/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "SimpleBalancer.h"
#include "Properties.h"
#include "ModuleList.h"

#include "LoadMapPacket.h"
#include "DeleteMapPacket.h"
#include "SystemPackets.h"
#include "StatisticsPacket.h"
#include "MapStatistics.h"
#include "DeleteHelpers.h"
#include "StringUtility.h"
#include "StringTable.h"

#include <algorithm>
#include <vector>


namespace {
/// Make comma separated list of hex numbers
inline MC2String makeList( const set<uint32>& nbrs )
{
   MC2String res;
   const char* addStr = "";
   for ( set<uint32>::const_iterator it = nbrs.begin();
         it != nbrs.end();
         ++it ) {
      res += addStr;
      addStr = ",";
      char tmp[64];
      sprintf( tmp, "0x%x", *it );
      res += tmp;
   }
   return res;
}
}

SimpleBalancer::SimpleBalancer(const IPnPort& ownAddr,
                               const MC2String& moduleName,
                               StatisticsPacket* ownStatistics,
                               bool usesMaps,
                               bool multipleMaps)
      : m_ownAddr( ownAddr ),
        m_moduleName( moduleName )
{
   m_moduleUsesMaps  = usesMaps;
   m_multipleMaps    = multipleMaps;
   // Presetting the propvalues to invalid 
   
   m_nbrHeartBeat    = 0;

   // Default is not to use map balancing
   m_useMapBalancing = false;
   
   createModuleList( ownStatistics );

   // Max age of a map
   m_maxMapAgeMinutes = MAX_UINT32;
   

   readPropValues();
   
   m_moduleList->setUseMapBalancing( m_useMapBalancing && m_multipleMaps );

   mc2dbg2 << "[SimpleBalancer]: Will keep one instance of maps ["
           << ::makeList( m_mapsToKeepOneOf ) << "]" << endl;
   mc2dbg2 << "[SimpleBalancer]: Max age for maps " << m_maxMapAgeMinutes
           << " minutes" << endl;
}

SimpleBalancer::~SimpleBalancer() {
   if ( m_moduleList.get() ) {
      STLUtility::deleteValues( *m_moduleList );
   }
}

int
SimpleBalancer::getNbrModules() const
{
   return m_moduleList->size();
}

void
SimpleBalancer::becomeAvailable()
{
   ModuleNotice* myNotice = m_moduleList->findModule( m_ownAddr );

   // m_moduleList should always contain the module's own notice
   if ( myNotice == NULL ) {
      mc2log << error << "[SimpleBalancer] Couldn't find own notice!" << endl;
      mc2log << error << "Exiting.... unhappy!" << endl;
      exit(0);
   }

   myNotice->setLeader( false );

   // delete all notices except the one for this module
   for ( ModuleList::iterator itr = m_moduleList->begin(); 
         itr != m_moduleList->end(); ++itr ) {
      if ( *itr != myNotice ) {
         delete *itr;
      }
   }

   // remove all other modules from the list, but keep our own module notice
   m_moduleList->clear();
   m_moduleList->push_back( myNotice );
}

void
SimpleBalancer::becomeLeader()
{
}

int
SimpleBalancer::checkOldAge( PacketSendList& packets )
{
   int nbrDeleted = 0;
   if ( ! m_moduleUsesMaps ) {
      return nbrDeleted;
   }
   
   // Check if there is a a map that hasn't been used for a while.
   if ( m_maxMapAgeMinutes == MAX_UINT32 || m_maxMapAgeMinutes == 0 ) {
      return nbrDeleted;
   }

   float maxUseTime = m_maxMapAgeMinutes * 60000;

   set<uint32> mapsToKeepOneOf( m_mapsToKeepOneOf );

   // Go through all modules and find maps older than maxUseTime
   for (  ModuleList::iterator it = m_moduleList->begin();
          it != m_moduleList->end();
          ++it ) {
      ModuleNotice* mn = *it;

      // Don't disturb the module if it is loading or deleting.
      if ( mn->isDeleting() || mn->isLoading() ) {
         continue;
      }
      
      if ( mn->getLoad1() > 0.50 ) {
         // Too high load - wait until it gets lower.
         continue;
      }
      
      pair<uint32, uint32> oldest =
         mn->getStats().getOldestMapAndAge( mapsToKeepOneOf );

      if ( oldest.first == MAX_UINT32 ) {
         mc2dbg8 << "[TB]: No maps in module " << mn->getAddr() << endl;
         continue;
      }
      
      float timeSinceUse = TimeUtility::getCurrentTime() - oldest.second;

      if ( timeSinceUse > maxUseTime ) {
         mc2dbg << "[TB]: Should delete map "
                << prettyMapID( oldest.first )
                << " last use "
                << ( timeSinceUse / 60000 ) << " minutes" << endl;
         moduleDeleteMap( packets, oldest.first,
                          mn );
         // Only remove one
         return 1;
      }      
   }

   return 0;   
}


bool
SimpleBalancer::updateStats(StatisticsPacket* packet,
                            PacketSendList& packetList)
{
   StatisticsPacket *sp = (StatisticsPacket *) packet;
   ModuleNotice* mn = m_moduleList->findModule(sp->getOriginIP(), 
                                               sp->getOriginPort());
   
   if (mn == NULL) {
      mn = new ModuleNotice(sp);
      m_moduleList->push_back( mn );
      mc2dbg << "[SimpleBalancer]: ADDING " << getModuleName() << "Module "
             << mn->getAddr() << endl;
      checkNoMultipleMaps( packetList );
   } else {
      mn->updateData( sp );
   }
   delete packet;

   if ( m_moduleUsesMaps ) {
      // Check if there are any maps that haven't been used for a
      // while
      checkOldAge( packetList );      
   }
   
   return true;
}

int
SimpleBalancer::timeout( StatisticsPacket* ownStatistics, 
                         PacketSendList& packets ) {
   ++m_nbrHeartBeat;
   mc2dbg8 << "[SimpleBalancer] isLeader == TRUE. Socket timed-out" 
           << endl;   
   if(m_nbrHeartBeat % 60 == 0)
      m_moduleList->dumpList(getModuleName().c_str(), m_moduleUsesMaps);

   // UpdateStats deletes the statistics packet.
   updateStats( ownStatistics,
                packets);

   ModuleNotice* mn = m_moduleList-> findModule(m_ownAddr);
   if ( mn ) {
      mn->setLeader(true);
   }
   if((mn != NULL) && (mn->isLeader())) {
      // Nothing
   } else {
      if ( mn == NULL ) {
         mc2log << error << "[SimpleBalancer]Couldn't find own notice!" << endl
                << "Exiting.... unhappy!" << endl;
         exit(0);
      } else {
         mc2dbg << "[SimpleBalancer]: My notice is not leader." << endl;
      }
   }
     

   // Remove modules that haven't sent replies to heartbeats.
   m_moduleList->checkTimeForStatistics();

   checkMemOverUse(packets);
   
   return 10000;
}

void
SimpleBalancer::getNoticeOrSendAck( PacketSendList& packetList,
                                    const RequestPacket* request,
                                    bool& ok )
{
   uint32 mapID = request->getMapID();
   if ( mapID != MAX_UINT32 ) {
      // Find the map in the allMaps - set
      bool foundMap = m_allMaps.count( mapID ) > 0;
      
      if( foundMap ) {
         // OK
         ok = true;
      } else {
         mc2log << error << "[SimpleBalancer] Requested map " 
                << prettyMapID( mapID ) 
                << " not found in allMaps vector!" << endl
                << "subtype = " << (int)request->getSubType()<< endl;
         mc2dbg << "Sender was: " << request->getOriginAddr() << endl;

         MC2_ASSERT( m_allMaps.size() != 0 );
         // Avoid looping forever when the packet comes back after the
         // ack is sent.
         // Don't abort, why should it loop? Doesn't loop
         // on MAPNOTFOUND. It's ERROR_MAP_LOADED that makes a new request.
         //abort();
         
         // Check that the sender isn't in the moduleList
         ModuleNotice* sender =
            m_moduleList->findModule(request->getOriginIP(),
                                     request->getOriginPort());
         if(sender == NULL) {
            AcknowledgeRequestReplyPacket* ackpack
               = new AcknowledgeRequestReplyPacket(request, 
                                                   StringTable::MAPNOTFOUND, 
                                                   MAX_UINT32);
            packetList.push_back( make_pair(request->getOriginAddr(),
                                            ackpack ) );
         }
         // Map nonexisting.
         ok = false;
      }

   } else {
      // mapID MAX_UINT32 always exists, since it means that
      // the map doesn't matter
      ok = true;
   }
}

void 
SimpleBalancer::moduleDeleteMap( PacketSendList& packetList,
                                 uint32 mapID, ModuleNotice* mn )
{
   if(!m_moduleUsesMaps) {
      mc2log << fatal << "[SimpleBalancer] " << getModuleName()
             << " does not delete maps. Returning ! "<<endl;
   }
   // Valid map id?
   if( m_allMaps.count( mapID ) == 0 ) {
      mc2log << error << "[SimpleBalancer]: Map " << prettyMapID( mapID )
             << " not found in allMaps vector!" << endl;
   } else {
      mn->startDeletingMap(mapID);
   }

   mc2dbg << "SimpleBalancer::moduleDeleteMap(): "
          << "module " << mn->getAddr() 
          << " deleting map " << prettyMapID( mapID )
          << endl;
   
   DeleteMapRequestPacket* p
         = new DeleteMapRequestPacket(mapID,
                                      m_ownAddr.getIP(), m_ownAddr.getPort());
   p->setArrivalTime();
   packetList.push_back( make_pair(IPnPort(mn->getIP(),
                                           mn->getPort()),
                                   p) );
}



void 
SimpleBalancer::moduleLoadMap( const RequestPacket* request,
                               PacketSendList& packetList,
                               uint32 mapID, ModuleNotice* mn )
{
   if(!m_moduleUsesMaps) {
      mc2log << fatal << "[SimpleBalancer] " << getModuleName()
             << " does not use maps. Exiting ! " << endl;
   }
      
   mn->markLoadingMap( mapID );
   
   mc2dbg2 << "[SimpleBalancer]: Module Loading Map = "
           << prettyMapID( mapID ) << endl;
   
   if( m_allMaps.count( mapID ) == 0 ) {
      mc2log << error << "[SimpleBalancer]Map " << prettyMapID( mapID )
             << " not found in allMaps vector!" << endl;
   }
   
   mc2dbg << "SimpleBalancer::moduleLoadMap(): module "
          << IPnPort(mn->getIP(), mn->getPort())
          << " loading map " << prettyMapID( mapID )
          << endl;
   
   LoadMapRequestPacket* p
      = new LoadMapRequestPacket(mapID,
                                 m_ownAddr.getIP(),    
                                 m_ownAddr.getPort(),
                                 mn->getIP(),
                                 mn->getPort());
   p->setArrivalTime();
   p->copyRequestInfoFrom( request );
   
   packetList.push_back( make_pair( IPnPort(mn->getIP(),
                                            mn->getPort()),
                                    p) );
}

int
SimpleBalancer::checkMemOverUse(PacketSendList& packets)
{
   if ( ! m_moduleUsesMaps ) {
      return 0;
   }
   
   // Get the modules sorted by memory usage.
   vector<ModuleNotice*> mods;
   m_moduleList->getModulesMostMemFirst( mods );
   
   if ( mods.empty() || mods.front()->getXSMem() <= 0 ) {
      // No modules use too much memory.
      return 0;
   }

   // Copy the set of maps to keep one of, since it will
   // be eaten if a module sees it.
   set<uint32> toKeepOne( m_mapsToKeepOneOf );
   int nbrDeleted = 0;
         
   for ( vector<ModuleNotice*>::iterator it = mods.begin();
         it != mods.end();
         ++it ) {
      ModuleNotice* deleteCandidate = *it;
      if ( deleteCandidate->getXSMem() <= 0 ) {
         // Since these are sorted we have nothing to do
         break;
      }
      if ( deleteCandidate->isDeleting() ) {
         // Take next module.
         mc2dbg8 << "[SimpleBalancer]: Module " << deleteCandidate->getAddr()
                 << " is already deleting - next please" << endl;
         continue;
      }
      // A map should be deleted
      mc2dbg << "[SimpleBalancer]: A map should be deleted in module "
             << deleteCandidate->getAddr() << endl;

      pair<uint32,uint32> oldest =
         deleteCandidate->getStats().getOldestMapAndAge( toKeepOne );

      if ( oldest.first == MAX_UINT32 ) {
         mc2log << warn << "[SimpleBalancer]: Could not find a map to delete"
                << " in " << deleteCandidate->getAddr() << " trying next"
                << endl;
         ++nbrDeleted;
         continue;
      } else {
         mc2dbg << "[SimpleBalancer]: Deleting map "
                << oldest.first << " in "
                << deleteCandidate->getAddr() << endl;
         moduleDeleteMap( packets, oldest.first, deleteCandidate);
      }      
   }
   return nbrDeleted;
}

/**
 *   Compare the amount of memory used by modules.
 */
class XSMemComp {
public:
   /// Returns true if a uses less memory than b
   bool operator()(const ModuleNotice* a, const ModuleNotice* b) const {
      return a->getXSMem() < b->getXSMem();
   }
   
};

/**
 *   Slow comparator that compares map ages.
 */
class OldestMapLessComp {
public:
   /// Constructor. Puts the ages of the maps into a map.
   OldestMapLessComp(const set<uint32>& keepMaps )
         : m_keepMaps( keepMaps ) {
   }

   /// Returns the age of the oldest map in modNotice
   uint32 getOldestMapAge(const ModuleNotice* modNotice) const {

      // Blaargh! Have to copy this one.
      set<uint32> avoid( m_keepMaps );

      // Find the oldest map in the module pretending that the
      // keep maps are very new.
      pair<uint32,uint32> oldest =
         modNotice->getStats().getOldestMapAndAge( avoid, true );

      if ( oldest.first == MAX_UINT32 ) {
         // No maps were loaded in the module.
         return 0;
      } else {
         return oldest.second;
      }
      return 0;
   }
   
   /// Returns true if a has older map than b
   bool operator()(const ModuleNotice* a, const ModuleNotice* b) const {
      return getOldestMapAge(a) < getOldestMapAge(b);
   }
private:
   const set<uint32>& m_keepMaps;
};

ModuleNotice*
SimpleBalancer::getModuleUsingLeastMemPrefNotLoading( uint32 mapID )
{
   if ( m_moduleList->size() == 0 ) {
      return NULL;
   }
   
   // Put the notices into a vector for convenience
   vector<const ModuleNotice*> modules;
   bool allLoading = m_moduleList->allModulesLoading();
   vector<ModuleNotice*> tmpStorage;

   const vector<ModuleNotice*>& candidates =
      m_moduleList->removeWorst( tmpStorage, *m_moduleList );
   
   for( ModuleList::const_iterator it = candidates.begin();
        it != candidates.end();
        ++it ) {
      const ModuleNotice* mn = *it;
      // Add the module if all modules are loading or the module is
      // not loading
      bool okLoad = mn->getLoad1() < 20.00;
      if ( allLoading || ( ( ! mn->isLoading() ) && okLoad ) ) {
         // Don't consider the ones that already have the map.
         if ( ! mn->isMapLoaded( mapID ) ) {
            modules.push_back(mn);
         }
      }
   }

   if ( modules.empty() ) {
      return NULL;
   }
   
   // Sort the modules. Less mem first.
   std::sort(modules.begin(), modules.end(), XSMemComp() );

#if 1
   // Now keep the ones that have approximately the same amount of
   // memory free.
   // XXX: I have doubts about this algorithm, leastMemMod->getUsedMem() == 0?
   //      Then only modules with 0 used men can load!?
   const ModuleNotice* leastMemMod = modules.front();
   if ( leastMemMod->getOptMem() == 0 ) {
      // Should not happen. Const cast uhggh!
      return const_cast<ModuleNotice*>(leastMemMod);
   }

   // This way the modules get more equal the more mem they use.
   double bestPercentage = double(leastMemMod->getUsedMem()) /
                           double(leastMemMod->getOptMem());
   // This means that e.g. modules with 50% mem use will be compared
   // to maps with up to 101% mem usage.
   double okPercentage = bestPercentage * 2;
   if ( okPercentage > 1.01 ) {
      // Wait for the maps to be deleted.
      okPercentage = 1.01;
   }

   
   vector<const ModuleNotice*>::iterator it = modules.begin();

   bool okSoFar = true;
   while ( okSoFar && it != modules.end() ) {
      const ModuleNotice* curNotice = *it;
      if ( curNotice->getOptMem() == 0 ) {
         // Stupid setting.
         ++it;
         continue;
      }
      double percentage = double(curNotice->getUsedMem()) /
                          double(curNotice->getOptMem());
      okSoFar = percentage <= okPercentage;
      if ( okSoFar ) {
         // Ok. Next.
         ++it;
      }
   }
#else
   // Consider all modules.
   vector<const ModuleNotice*>::iterator it = modules.end();
#endif   

   // Take the module with the oldest map from the beginning to the
   // first one that wasn't ok. Will not count the maps that we should
   // keep one of.
   // Also add the ugly const_cast
   return const_cast<ModuleNotice*>(
      *std::min_element(modules.begin(), it,
                        OldestMapLessComp(m_mapsToKeepOneOf)));
   
}

bool
SimpleBalancer::getModulePackets(PacketSendList& packets,
                                 RequestPacket* request)
{
   // Make sure that the request is not NULL
   MC2_ASSERT(request != NULL);
   
   if( (!m_moduleUsesMaps) || (request->getMapID() == MAX_UINT32) )
   {
      ModuleNotice* mn = m_moduleList->getBestModule(request);
      if( mn != NULL ) {
         // Send the request to the module that uses maps
         // since this module dont use them.

         mn->addPacketToQueue( MAX_UINT32 );
         packets.push_back( make_pair( IPnPort( mn->getIP(), mn->getPort() ),
                                       request ) );
      }
      return true;
   }

   // Module has maps from this point on.
   
   // Add the request to the map.
   uint32 mapID = request->getMapID();
   

   // 1.
   // Check if the map id is possible at all and send
   // an ack if it is not found in the allmaps vector.
   bool ok = false;
   getNoticeOrSendAck( packets, request, ok );
   if ( ok == false ) {
      delete request;
      request = NULL;
      return true;
   }
   
   bool mapHasBeenLoaded = false;
   // 2. Check if there is a module with the map loaded.
   //    Return the best module with the mapID   
   ModuleNotice* mn = m_moduleList->getBestModuleWithMapLoaded(mapID);
   if ( mn == NULL ) {
      // Check if a module is loading the map.
      mn = m_moduleList->getBestModuleLoadingMap(mapID);
      if ( mn == NULL ) {
         // No-one has loaded the map - tell someone to do it.
         // We will still keep the behaviour that we refuse requests
         // until there is a module that can load a map. This is due
         // to the strangeness in memory balance that can occur if
         // the guessed size of the maps is wrong or a module uses
         // much time to load a map.
         if ( !m_moduleList->allModulesLoading() ) {
            mn = getModuleUsingLeastMemPrefNotLoading( mapID );
         }
         // All modules are loading.
         if ( mn == NULL ) {
            mc2dbg << "[SimpleBalancer]: All modules are loading - req "
                   << " on map " << prettyMapID( mapID ) 
                   << " was refused" << endl;
            sendAcknowledge(packets, request, 10003, true);
            // Copy the header into the list
            request->setLength(REQUEST_HEADER_SIZE);
            m_refusedDueToLoading.push_back(
               static_cast<RequestPacket*>(request->getClone(false)));
            
         } else {
            if (request->getSubType() != Packet::PACKETTYPE_DELETEMAPREQUEST &&
                request->getSubType() != Packet::PACKETTYPE_ACKNOWLEDGE ) {
               // Load map, but not if it should be deleted.            
               // Can probably only be sent from ModuleTestServer
               // \label{loadmap}
               // Also no not load maps for ACKNOWLEDGE.
               moduleLoadMap(request, packets, mapID, mn);
               mapHasBeenLoaded = true;
               mn->addPacketToQueue( mapID );
            }
         }
      }
   }

   // No module found
   if ( mn == NULL ) {
      delete request;
      return true;
   }

   // Send the request packet to the module.
   if ( request->getSubType() != Packet::PACKETTYPE_LOADMAPREQUEST ) {
      packets.push_back( make_pair( mn->getAddr(), request ) );
   } else {
      // Check if it is a LOADMAP so we get the right behaviour
      // even when using ModuleTestServer
      packets.insert( packets.begin(), 
                      make_pair( mn->getAddr(), request ) );
      mapHasBeenLoaded = true;
   }
   mn->addPacketToQueue( mapID );
   request = NULL;

   // Checks if there are doubles of maps if this is not allowed
   if(!m_multipleMaps){
      checkNoMultipleMaps(packets);
   }
   
   
   // 3. Check if there is need for any delete-map requests.
   //    Delete the oldest map in the module with the most maps loaded.
   //    FIXME: If more than one module have the same mem-use -
   //           check map age. (Should not happen when real map size is
   //           used since all maps will probably have different sizes).
   if ( mapHasBeenLoaded ) {
      checkMemOverUse(packets);
   }
   
   return true;
}

void
SimpleBalancer::reactToMapLoaded(PacketSendList& packetList,
                                 LoadMapReplyPacket* replyPacket)
{
   // I would like to avoid the "All modules are loading"
   mc2dbg << "[SimpleBalancer]: React to map loaded" << endl;
   // SearchModule never returns OK...
   if ( replyPacket->getStatus() == StringTable::OK ) {
      uint32 loaderIP = replyPacket->getMapLoaderIP();
      uint32 loaderPort = replyPacket->getMapLoaderPort();
      ModuleNotice* mn = m_moduleList->findModule(loaderIP, 
                                                  loaderPort);
      if ( mn == NULL ) {
         mc2dbg << "[SimpleBalancer]: Someone has loaded a map - don't "
                << "know who" << endl; 
      } else {
         mc2dbg << "[SimpleBalancer]: " << mn->getAddr()
                << " has loaded map " 
                << prettyMapIDFill( replyPacket->getMapID() ) << endl;
         mn->addLoadedMap(replyPacket->getMapID(),
                          replyPacket->getLoadedMapSize());
      }
   } else if ( replyPacket->getStatus() != StringTable::ERROR_MAP_LOADED ) {
      mc2dbg << "[SimpleBalancer]: Got loadMapReply with status != OK. (SM?)."
             << endl;
   }
   
   const int nbrRefused = m_refusedDueToLoading.size();
   if ( nbrRefused != 0 ) {
      mc2dbg << "[SimpleBalancer]: Considering "
             << nbrRefused << " wakeup acks to servers"
             << endl;
      int nbrSent = 0;
      for( int i = 0; i < nbrRefused; ++i ) {
         RequestPacket* curReq = m_refusedDueToLoading[i];
          ++nbrSent;
          sendMapLoadedAcknowledge(packetList, curReq);          
      }
      STLUtility::deleteValues( m_refusedDueToLoading );
      m_refusedDueToLoading.clear();
      mc2dbg << "[SimpleBalancer]: Sent "
             << nbrSent << " wakeup acks to servers"
             << endl;
   } 
    
}

void
SimpleBalancer::checkNoMultipleMaps(PacketSendList& packets)
{
   if ( m_multipleMaps ) {
      return;
   }
   mc2dbg1 << "[SimpleBalancer] checking for multiple maps" << endl;
   // MapStatistics::getLoadedMaps(set<uint32>& maps);
   //bool multiplies = false;
   set<uint32> presentMaps;
   set<uint32> doubles;
   int nbrdbls = 0;
   
   
  for( ModuleList::iterator it = m_moduleList->begin();
       it != m_moduleList->end();
       ++it ) {
       nbrdbls = (*it)->checkIfMapsPresent(presentMaps, doubles);
   }

   if(!doubles.empty()){
      // Deleate all the double maps from all modules.
      for( ModuleList::iterator it = m_moduleList->begin();
           it != m_moduleList->end();
           ++it ) {
        ModuleNotice* mn = *it;
         for(set<uint32>::iterator si = doubles.begin();si != doubles.end();
             si++){
            if(mn->isMapLoadedOrLoading(*si)){
               moduleDeleteMap(packets, *si, mn);
            }
         }  
      }
   }
}


void
SimpleBalancer::sendAcknowledge(PacketSendList& packetList,
                                RequestPacket* pack,
                                uint32 eta, bool refuse)
{
   mc2dbg1 << "[SimpleBalancer] Sending Acknowledge with eta : " << eta 
           << endl;

   IPnPort dest(pack->getOriginIP(),pack->getOriginPort());

   AcknowledgeRequestReplyPacket* ackpack;
   if(!refuse) {      
      ackpack =
         new AcknowledgeRequestReplyPacket(pack, 
                                           StringTable::OK, 
                                           eta);

   } else {      
      ackpack = new AcknowledgeRequestReplyPacket(pack, 
                                                  StringTable::OK, //  For now
                                                  eta);
   }
   packetList.push_back( make_pair( dest, ackpack ) );
}

void
SimpleBalancer::sendMapLoadedAcknowledge(PacketSendList& packetList,
                                         RequestPacket* pack)
{
   AcknowledgeRequestReplyPacket* ackPack =
      new AcknowledgeRequestReplyPacket(pack,
                                        // Not an error, just map loaded.
                                        StringTable::ERROR_MAP_LOADED,
                                        307653); // Not important
   IPnPort dest(pack->getOriginIP(),pack->getOriginPort());
   packetList.push_back( make_pair( dest, ackPack ) );
}

void
SimpleBalancer::createModuleList( StatisticsPacket* ownStatistics )
{
   m_moduleList.reset( new ModuleList() );
   
   // Create and add a new leader notice to the list.
   ModuleNotice* leaderNotice = new ModuleNotice( ownStatistics );
   m_moduleList->push_back( leaderNotice );
   m_moduleList->setUseMapBalancing( m_useMapBalancing && m_moduleUsesMaps );
}

bool
SimpleBalancer::readPropValues(const MC2String& prefix,
                               const MC2String& suffix )
{
   mc2dbg8 << "[SimpleBalancer] readPropValues prefix = "
           << MC2CITE(prefix) << ", suffix = " << MC2CITE(suffix) << endl;
   bool success = true;

   m_maxMapAgeMinutes =
      Properties::getUint32Property( prefix +
                                     "MODULE_MAX_MAP_AGE_MINUTES"+suffix,
                                     m_maxMapAgeMinutes );

   m_useMapBalancing = Properties::getUint32Property( prefix +
                                                      "MODULE_USE_MAPBALANCING"
                                                      + suffix,
                                                      m_useMapBalancing );
   

   // This is a bit ugly. Use the default list etc.
   vector<uint32> keepMaps;
   MC2String defaultString = ::makeList( m_mapsToKeepOneOf );
   Properties::getUint32ListProperty( keepMaps,
                                      prefix + "MODULE_KEEP_ONE_MAP_OF"+suffix,
                                      defaultString.c_str() );
   m_mapsToKeepOneOf.clear();
   m_mapsToKeepOneOf.insert( keepMaps.begin(), keepMaps.end() );

   uint32 mapSet = Properties::getMapSet();
   if ( suffix.empty() ) {
      if ( mapSet != MAX_UINT32 ) {
         // Overwrite with the correct map set set
         char tmp[64];
         sprintf(tmp, "_%u", mapSet );
         success = success && readPropValues( prefix, tmp );
      }
   } else {
      return success;
   }
   
   // Add ROUTE_ for routemodule etc. and overwrite the values. 
   if ( prefix[0] == '\0' ) {
      // I'm keeping this because it is so funny.
      char* upperName = StringUtility::newStrDup(
         StringUtility::copyUpper(getModuleName()).c_str() );
      bool retVal = success &&
         readPropValues(MC2String(MC2String(upperName) + "_").c_str());
      delete [] upperName;
      return retVal;
   } else {
      return success;
   }
}

const MC2String& SimpleBalancer::getModuleName() const {
   return m_moduleName;
}

bool SimpleBalancer::usesMaps() const {
   return m_moduleUsesMaps;
}

void SimpleBalancer::setAllMaps( const set<MapID>& allMaps ) {
   m_allMaps = allMaps;
}

InfoModuleSimpleBalancer::InfoModuleSimpleBalancer( 
   const IPnPort& ownAddr,
   const MC2String& moduleName,
   StatisticsPacket* ownStatistics )
      : SimpleBalancer(ownAddr, moduleName, ownStatistics, true, false)
{
}
