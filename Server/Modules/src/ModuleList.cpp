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

#include "ModuleList.h"
#include "StatisticsPacket.h"
#include "TimeUtility.h"
#include "MapStatistics.h"

#include <sstream>
#include <algorithm>

/*
 * ----------------------- ModuleNotice -------------------
 */


ModuleNotice::ModuleNotice(StatisticsPacket* sp) 
   : m_ipAndPort(sp->getOriginIP(), sp->getOriginPort())
{
   m_moduleMapStats = new MapStatistics;
   updateData(sp);
   m_isLeader = false;
}


ModuleNotice::~ModuleNotice()
{
   delete m_moduleMapStats;
}

uint32
ModuleNotice::getIP() const
{
   return m_ipAndPort.getIP();
}


uint16
ModuleNotice::getPort() const
{
   return m_ipAndPort.getPort();
}

const MapStatistics&
ModuleNotice::getStats() const
{
   return *m_moduleMapStats;
}


void
ModuleNotice::updateData(StatisticsPacket* sp)
{
   sp->getMapStatistics(*m_moduleMapStats);
   
   m_usedMem = m_moduleMapStats->calcUsedMem();
   m_timeForStatistics = TimeUtility::getCurrentTime();

   // Reset the number of extra packets.
   m_recentNbrOfReq = 0;
}

uint32
ModuleNotice::getTimeForLastStatistics() const
{
   return m_timeForStatistics;
}

uint32
ModuleNotice::getProcessTime() const
{
   return MAX(m_moduleMapStats->getProcessTime(), 1);
}

bool
ModuleNotice::isMapLoaded(uint32 mapID) const
{ 
   return m_moduleMapStats->isMapLoaded(mapID);
}

bool
ModuleNotice::isMapLoading( uint32 mapID ) const
{
   return m_moduleMapStats->isMapLoading( mapID );
}

bool
ModuleNotice::isMapLoadedOrLoading(uint32 mapID) const
{ 
   return m_moduleMapStats->isMapLoadedOrLoading(mapID);
}

void
ModuleNotice::printLoadedMaps(ostream& stream)
{
   stream << *m_moduleMapStats;
}

uint32
ModuleNotice::getNbrMaps() const
{
   return m_moduleMapStats->getNbrOfMaps();
}

uint64
ModuleNotice::getUsedMem() const
{
   return m_usedMem;
}

uint64
ModuleNotice::getMaxMem() const
{
   return m_moduleMapStats->getMaxMem();
}

uint64
ModuleNotice::getOptMem() const
{
   return m_moduleMapStats->getOptMem();
}

int64
ModuleNotice::getXSMem() const
{
   return getUsedMem() - getOptMem();
}

bool
ModuleNotice::isLeader() const
{
   return m_isLeader;
}

void
ModuleNotice::setLeader(bool leader)
{
   m_isLeader = leader;
}

bool
ModuleNotice::isLoading() const
{
   return m_moduleMapStats->moduleIsLoading();
}

bool
ModuleNotice::isDeleting() const
{
   return m_moduleMapStats->moduleIsDeleting();
}

uint32
ModuleNotice::getPacketsInQueue() const
{
   return m_moduleMapStats->getQueueLength();
}

void
ModuleNotice::markLoadingMap(uint32 mapID)
{
   // This function will add the map if it is needed.
   m_moduleMapStats->setStatus(mapID, MapElement::TOLD_TO_LOAD);
   m_moduleMapStats->updateLastUse( mapID );
}

void
ModuleNotice::addPacketToQueue( uint32 mapID )
{
   ++m_recentNbrOfReq;
   if ( mapID != MAX_UINT32 ) {
      m_moduleMapStats->updateLastUse( mapID );
   }
}

void
ModuleNotice::startDeletingMap(uint32 mapID)
{   
   m_moduleMapStats->setStatus(mapID, MapElement::TOLD_TO_DELETE);
}

void
ModuleNotice::addLoadedMap(uint32 mapID, uint32 size)
{
   m_moduleMapStats->finishLoadingMap(mapID, size);
   m_usedMem = m_moduleMapStats->calcUsedMem();
}

int
ModuleNotice::checkIfMapsPresent(set<uint32> &presentMaps,
                                 set<uint32> &doubles)
{
   int foundNbr = 0;
   //set<uint32> unique;
   for(set<uint32>::iterator si = presentMaps.begin();
       si != presentMaps.end(); si ++){
      if(isMapLoadedOrLoading(*si)){
         doubles.insert(*si);
         foundNbr++;
      }
   }
   set<MapElement> oldMaps;
   m_moduleMapStats->getAllMapInfo(oldMaps);
   if(!oldMaps.empty()){
      for(set<MapElement>::iterator mi = oldMaps.begin();
       mi != oldMaps.end(); mi ++){
         if(presentMaps.find((*mi).getMapID()) == presentMaps.end())
            presentMaps.insert((*mi).getMapID());
      }
   }
   return foundNbr;
}

float
ModuleNotice::getLoad1() const
{
   return m_moduleMapStats->getLoad1();
}

/*
 * ----------------------- ModuleList -------------------
 */

ModuleList::ModuleList()
{
   m_useMapBalancing = false;
}

ModuleNotice*
ModuleList::findModule(const IPnPort& addr)
{
   for( iterator it = begin();
        it != end();
        ++it ) {
      if ( (*it)->getAddr() == addr ) {
         return *it;
      }
   }
   // Not found.
   return NULL;
}


ModuleNotice*
ModuleList::findModule(uint32 ip, uint16 port)
{
   return findModule(IPnPort(ip, port));
}


bool
ModuleList::allModulesLoading() const
{
   for( const_iterator it = begin();
        it != end();
        ++it ) {
     if ( ! (*it)->isLoading() ) {
         return false;
      }
   }
   return true;
}

ModuleNotice*
ModuleList::getBestModule( const vector<ModuleNotice*>& notices )
{
   ModuleNotice* best = NULL;
   uint32 bestValue   = MAX_UINT32;
   float bestLoad     = 10000;
   // Does not use maps. Just takes the shortest queue.
   for ( int i = 0, n = notices.size(); i < n; ++i ) {
      
      ModuleNotice* mn = notices[i];

      // Take shortest queue. 
      uint32 value = mn->getPacketsInQueue() + mn->getRequests();

      if ( ( value < bestValue ) ||
           // If queues are the same take least load.
           ( ( value == bestValue ) && ( mn->getLoad1() < bestLoad ) ) ) {
         bestValue = value;
         best      = mn;
         bestLoad  = mn->getLoad1();
      }
   }
   return best;
}

const vector<ModuleNotice*>&
ModuleList::removeWorst( vector<ModuleNotice*>& out_notices,
                         const vector<ModuleNotice*>& in_notices ) const
{
   if ( in_notices.empty() ) {
      return in_notices;
   }

   // TESTING
   
   if ( m_useMapBalancing ) {
      out_notices.clear();
      out_notices.reserve( in_notices.size() );
      // Remove the worst modules.
      // Could lead to trouble since the loading
      // code doesn't know about this.
      
      float64 loadSum = 0;
      for ( int i = 0, n = in_notices.size(); i < n; ++i ) {
         float64 curLoad = in_notices[i]->getLoad1();
         loadSum += curLoad;
      }
      float64 avgLoad = loadSum / in_notices.size();

      // The avg load must be at least one to be important.
      avgLoad = MAX( avgLoad, 1.0 );

      // Now we have the avg load.
      float64 maxAllowed = avgLoad * 2;
      // Now add the notices
      for ( int i = 0, n = in_notices.size(); i < n; ++i ) {
         float64 curLoad = in_notices[i]->getLoad1();
         // If the number of requests are less than the load, then
         // the load must be going down.
         uint32 totPacks = in_notices[i]->getPacketsInQueue() +
                              in_notices[i]->getRequests();
         if ( ( curLoad < maxAllowed ) ||
              ( totPacks < curLoad ) ) {
            out_notices.push_back( in_notices[i] );
         }
      }
      return out_notices;
   } else {
      return in_notices;
   }
}

ModuleNotice*
ModuleList::getBestModuleWithMapLoaded(uint32 mapID)
{
   vector<ModuleNotice*> storage;
   const vector<ModuleNotice*>& candidates = removeWorst( storage, *this );

   vector<ModuleNotice*> new_candidates;
   
   for( vector<ModuleNotice*>::const_iterator it = candidates.begin();
        it != candidates.end();
        ++it ) {
      ModuleNotice* mn = *it;
      if ( mn->isMapLoaded(mapID) ) {
         new_candidates.push_back( mn );
      }
   }

   return getBestModule( new_candidates );
}

ModuleNotice*
ModuleList::getBestModuleLoadingMap(uint32 mapID)
{
   // Don't check the queue this time.
   // But remove the overloaded ones.
   vector<ModuleNotice*> storage;
   const vector<ModuleNotice*>& candidates = removeWorst( storage, *this );
   
   for( vector<ModuleNotice*>::const_iterator it = candidates.begin();
        it != candidates.end();
        ++it ) {
      if ( (*it)->isMapLoadedOrLoading( mapID ) ) {
         return *it;
      }
   }
   // No module found.
   return NULL;
}

int
ModuleList::countModulesWithMap( uint32 mapID ) const
{
   int count = 0;
   for( const_iterator it = begin();
        it != end();
        ++it ) {
      if ( (*it)->isMapLoadedOrLoading( mapID ) ) {
         ++count;
      }
   }
   return count;
}

ModuleNotice*
ModuleList::getModuleUsingLeastMem()
{
   // Look for the one using the least memory but
   // one that isn't already loading a map.
   // Since there is only one pos for loading map
   // we can only load one map.
   int64 minMem = MAX_INT64;
   ModuleNotice* bestModule = NULL;
   for( iterator it = begin();
         it != end();
         ++it ) {      
      ModuleNotice* mn = *it;
      if ( mn->getXSMem() < minMem ) {
            minMem = mn->getXSMem();
            bestModule = mn;
      }
   }
   return bestModule;
}

ModuleNotice*
ModuleList::getModuleUsingLeastMemPrefNotLoading()
{
   // Look for the one using the least memory but
   // one that isn't already loading
   int64 minMem = MAX_INT64;
   ModuleNotice* bestModule = NULL;
   for ( iterator it = begin();
         it != end();
         ++it ) {
     ModuleNotice* mn = *it;
     if ( ! mn->isLoading() ) {
         if ( mn->getXSMem() < minMem ) {
            minMem = mn->getXSMem();
            bestModule = mn;
         }
      }
   }
   if ( bestModule != NULL ) {
      return bestModule;
   } else {
      return getModuleUsingLeastMem();
   }
}

ModuleNotice*
ModuleList::getModuleUsingMostMem()
{
   // Look for the one using the most memory  
   int64 maxMem = MIN_INT64;
   ModuleNotice* bestModule = NULL;
   for ( iterator it = begin();
         it != end();
         ++it ) {
       ModuleNotice* mn = *it;
       if ( mn->getXSMem() > maxMem ) {
         maxMem = mn->getXSMem();
         bestModule = mn;
      }
   }
   return bestModule;
}

namespace {
   class XSMemSorter {
   public:
      bool operator()( const ModuleNotice* a, const ModuleNotice* b ) {
         return a->getXSMem() > b->getXSMem();
      }
   };
}

void
ModuleList::getModulesMostMemFirst( vector<ModuleNotice*>& mods ) const
{
   mods.insert( mods.end(), begin(), end() );
   std::sort( mods.begin(), mods.end(), XSMemSorter() );
}

void
ModuleList::checkTimeForStatistics()
{
   bool removedNotice = false;
   uint32 curTime = TimeUtility::getCurrentTime();
   for ( iterator it = begin();
         it != end(); ) {
      uint32 timeSinceLastStats = curTime - (*it)->getTimeForLastStatistics();
      if ( timeSinceLastStats > MAX_TIME_NO_STATISTICS ) {
         mc2log << warn << "[ML]: REMOVING MODULE "
                << (*it)->getAddr() << " no stats for "
                << (timeSinceLastStats/1000.0) << " secs" << endl;
         removedNotice = true;
         delete *it;
         it = erase(it);
      } else {
         ++it;
      }
   }
   if(removedNotice) {
      resetNumberOfRequests();
   }
}

ModuleNotice*
ModuleList::getBestModule(Packet *p)
{
   // Module does not use maps or the packet has no map id.
   return getBestModule( *this );
}

void
ModuleList::dumpList(const char* moduleName,
                     bool maps)
{

   uint32 i=0;
   for ( iterator it = begin();
         it != end();
         ++it ) {
      ModuleNotice* mn = *it;
      stringstream strstr;
      if( mn->isLeader() ) {
         strstr << "[ML] " << moduleName << "Leader ";
      } else {
         strstr << "[ML] " << moduleName << "Module ";
      }
      strstr << i << " (" << mn->getAddr() << ")"
             << ", p time = " << mn->getProcessTime()
             << ", q = " << mn->getPacketsInQueue();
      if(maps) {
         strstr << ", xs  = " << mn->getXSMem();
         strstr << ", Maps: ";
         mn->printLoadedMaps(strstr);
      }
      strstr << endl << ends;
      mc2log << info << strstr.str();
      i++;      
   }
}

void
ModuleList::resetNumberOfRequests()
{
   for ( iterator it = begin();
         it != end();
         ++it ) {
      ModuleNotice* mn = *it;
      mn->resetRequests();
   }
}

pair<ModuleNotice*, uint32>
ModuleList::getOldestMap( set<uint32>& keepers )
{
   pair<ModuleNotice*, uint32> retVal(NULL, MAX_UINT32);
   uint32 oldestSoFar = MAX_UINT32;
   for ( iterator it = begin();
         it != end();
         ++it ) {
      ModuleNotice* mn = *it;
      const MapStatistics& mapStats = mn->getStats();
      pair<uint32,uint32> curOldest = mapStats.getOldestMapAndAge( keepers );
      if ( curOldest.second < oldestSoFar ) {
         retVal.first  = mn;              // module notice
         retVal.second = curOldest.first; // map id
      }
   }
   return retVal;
}
