/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ParserThreadGroup.h"

#include "ParserThread.h"
#include "ThreadRequestHandler.h"
#include "ThreadRequestContainer.h"
#include "UserData.h"
#include "UserRight.h"

#include "CellularPhoneModelsElement.h"
#include "UserSessionCache.h"

#include "SinglePacketRequest.h"
#include "PushPacket.h"
#include "TopRegionRequest.h"
#include "ServerRegionIDs.h"
#include "EdgeNodesPacket.h"
#include "IDTranslationPacket.h"
#include "CopyrightRequest.h"
#include "UserPacket.h"
#include "RouteStoragePacket.h"

#include "ServerTileMapFormatDesc.h"
#include "SFDHolder.h"
#include "Properties.h"
#include "FOBC.h"
#include "NamedServerLists.h"
#include "ClientSettings.h"
#include "CategoriesData.h"
#include "ParserUserHandler.h"
#include "HttpHeader.h"
#include "ParserCWHandler.h"
#include "PropertyHelper.h"
#include "DeleteHelpers.h"

#include "PreCacheRouteData.h"
#include "PreCacheTileMapHandler.h"

#include "ExpandedRouteItem.h"
#include "ExpandedRoute.h"
#include "RouteRequest.h"
#include "DriverPref.h"
#include "CategoryTree.h"
#include "LocalCategoryTrees.h"
#include "POIImageIdentificationTable.h"
#include "ParserActivationHandler.h"
#include "HttpCodes.h"

#include "SearchHeadingManager.h"
#include "TimedOutSocketLogger.h"

#include <boost/lexical_cast.hpp>

// - TileMapFormatDescData
ParserThreadGroup::
TileMapFormatDescData::
TileMapFormatDescData( const STMFDParams& settings,
                       const CopyrightHolder& copyrights ):
   m_tmfd( new ServerTileMapFormatDesc( settings, copyrights ) ) {

   m_tmfd->setData();   
   // Make a 10 meg buffer.
   BitBuffer tmpBuf(10*1024*1024);
   
   m_tmfd->save( tmpBuf, &m_timeStampPos );

   TileMapFormatDescCRC crc( m_tmfd->getCRC() );

   // Make new buffer of correct size.
   m_tmfdBuffer = new BitBuffer( tmpBuf, true );

   // Save the crc
   tmpBuf.reset();
   crc.save( tmpBuf );
   m_crcBuffer = new BitBuffer( tmpBuf, true );   
}

ParserThreadGroup::
TileMapFormatDescData::~TileMapFormatDescData()
{
   delete m_tmfdBuffer;
   delete m_tmfd;
   delete m_crcBuffer;
}

BitBuffer*
ParserThreadGroup::
TileMapFormatDescData::newTMFDBuffer() const
{
   BitBuffer* tmpBuf = new BitBuffer( *m_tmfdBuffer );
   // Update the timestamp in the buffer.
   ServerTileMapFormatDesc::updateTimeStamp( *tmpBuf, m_timeStampPos );
   return tmpBuf;
}

BitBuffer*
ParserThreadGroup::
TileMapFormatDescData::newCRCBuffer() const
{
   return new BitBuffer( *m_crcBuffer );
}

const ServerTileMapFormatDesc*
ParserThreadGroup::TileMapFormatDescData::getTMFD() const
{
   return m_tmfd;
}


// - ParserThreadGroup

// The time betwen updates of m_phoneModels
const time_t ParserThreadGroup::MAX_PHONEMODELS_AGE = 1800;


// The time to keep a copy of the user cached
const uint32 ParserThreadGroup::MAX_USER_AGE = 60;


// The time to keep user sessions and logins cached
const uint32 ParserThreadGroup::MAX_SESSION_AGE = 60;

ParserThreadGroup::ParserThreadGroup( ServerTypes::servertype_t serverType,
                                      const char* threadGroupName)
      : ISABThreadGroup(threadGroupName),
        m_handler(NULL),
        m_running(true),
        m_phoneModels( NULL ),
        m_lastPhoneModelsUpdate( 0 ),
        m_topRegionRequest( NULL ),
        m_regionIDs( new ServerRegionIDs ),
        m_userSessionCache( new UserSessionCache() ),
        m_userLoginCache( new UserSessionCache() ),
#ifdef USE_SSL
        m_ctx( NULL ),
#endif
        m_fobc( new FOBC() ),
        m_clientSettingStorage( NULL ),
        m_categories( NULL ),
        m_preCacheRouteQueue(),
        m_preCacheTileMapHandler( new PreCacheTileMapHandler( this,
                                                              m_preCacheRouteQueue ) ),
        m_timedOutSocketLogger( NULL ),
        m_periodicTrafficUpdateInterval( MAX_UINT32 ),
        m_searchHeadingManager( new SearchHeadingManager() )
{
  

   m_preCacheTileMapHandler->start();

   m_serverType = serverType;

   m_namedServerLists = new NamedServerLists();

   const uint32 randSize = 10;
   char tmpStr[ randSize + 1 ];
   StringUtility::randStr( tmpStr, randSize );
   m_serverInstanceStr = tmpStr;

   m_localCategoryTrees = makeNewLocalCategoryTrees();

   const char* sfdPath = Properties::getProperty( "SFD_PATH" );
   if ( sfdPath != NULL ) {
      mc2dbg << "[ParserThreadGroup] Loading SFD files from: " 
             << sfdPath << endl;
      DebugClock sfdTime;
      m_SFDHolder.reset( new SFDHolder( sfdPath ) );
      mc2dbg << "SFD files loaded in " << sfdTime << endl;
   }

}


ParserThreadGroup::~ParserThreadGroup() 
{
   ISABSync sync( m_monitor );

   m_running = false;

   // Wait for all threads to terminate
   while ( !m_threads.empty() ) {
      ISABThread::yield();
   }

   delete m_topRegionRequest;
   delete m_regionIDs;
   delete m_userSessionCache;
   delete m_userLoginCache;
   delete m_fobc;
   delete m_namedServerLists;
   delete m_clientSettingStorage;

   for ( tfmdmap_t::iterator it = m_tileMapFormatDescMap.begin();
         it != m_tileMapFormatDescMap.end();
         ++it ) {
      delete it->second;
   }
   m_tileMapFormatDescMap.clear();

   for ( routeStorage_t::iterator rit = m_routeStorage.begin() ;
         rit != m_routeStorage.end() ; ++rit )
   {
      delete rit->second;
   }
   m_routeStorage.clear();

   mc2dbg1 << "ParserThreadGroup::~ParserThreadGroup "
           << "all threads done" << endl;
}


void
ParserThreadGroup::putAnswer(ThreadRequestContainer* reqCont) 
{
   ISABSync sync( m_monitor );

   // Find thread and return request
   if ( reqCont->getThread()->isAlive() ) {
      DEBUG8(cerr << "   Group got answer, returning to thread" << endl);
      reqCont->getThread()->putAnswer( reqCont );
      DEBUG8(cerr << "   Answer returned to thread" << endl);
   } else {
      MC2ERROR2("ParserThreadGroup::putAnswer thread is dead! ",
                cerr << reqCont->getThread() << endl;);
   }
}


void 
ParserThreadGroup::addPushService( ParserThreadHandle thread,
                                   PushService* service,
                                   bool storePackets,
                                   vector<uint32>& lastUpdateTime )
{
   vector<SubscriptionResourceNotice> resources;

   service->getAllResources( resources );

   m_pushServiceMutex.lock();
   for ( vector<SubscriptionResourceNotice>::iterator 
            it = resources.begin() ; it != resources.end() ; ++it )
   {
      m_pushServiceThreadMap.insert( make_pair(
         make_pair( service->getServiceID(), *it ), ParserThreadPushData(
            thread, storePackets ) ) );
   }
   m_pushServiceMutex.unlock();
   
   m_handler->addPushService( service, lastUpdateTime );
}


bool 
ParserThreadGroup::removePushService( ParserThreadHandle thread,
                                      uint32 serviceID )
{
   bool found = false;


   m_pushServiceMutex.lock();
   PushServiceThreadMap::iterator it = m_pushServiceThreadMap.begin();
   while( it != m_pushServiceThreadMap.end() ) {
      if ( it->first.first == serviceID && 
           it->second.handle.get() == thread.get() ) {
         // Remove
         m_pushServiceThreadMap.erase( it++ );
      } else {
         ++it;
      }
   }
   // Check if no one uses service any more
   bool serviceUsed = false;
   it = m_pushServiceThreadMap.begin();
   for ( ; it != m_pushServiceThreadMap.end() ; ++it ) {
      if ( it->first.first == serviceID ) {
         serviceUsed = true;
         break;
      }
   }
   if ( !serviceUsed ) {
      m_handler->removePushService( serviceID );
   }
   m_pushServiceMutex.unlock();

   return found;
}


bool 
ParserThreadGroup::removePushServiceResource( 
   ParserThreadHandle thread, uint32 serviceID, 
   SubscriptionResource& resource )
{
   bool found = false;

   pair< uint32, SubscriptionResourceNotice > serviceResource (
      make_pair( serviceID, SubscriptionResourceNotice( resource ) ) );
   
   m_pushServiceMutex.lock();
   PushServiceThreadMap::iterator it = m_pushServiceThreadMap.lower_bound(
      serviceResource );
   while( it != m_pushServiceThreadMap.end() &&
          it->first == serviceResource ) 
   {
      // Check for thread
      if ( it->second.handle.get() == thread.get() ) {
         // Remove
         m_pushServiceThreadMap.erase( it++ );
      } else {
         ++it;
      }
   }
   // Check if no one uses service,resource any more 
   it = m_pushServiceThreadMap.lower_bound( serviceResource );
   if ( it == m_pushServiceThreadMap.end() || 
        it->first != serviceResource )
   {
      m_handler->removePushServiceResource( serviceID, resource );
   }
   m_pushServiceMutex.unlock();

   return found;
}


void 
ParserThreadGroup::readyForPush( ParserThreadHandle thread, 
                                 uint32 serviceID, 
                                 SubscriptionResource& resource )
{
   mc2dbg1 << "PTG::readyForPush serviceID " << serviceID 
           << " resource ";
#ifdef DEBUG_LEVEL_1
   resource << mc2log; mc2log << endl;
#endif
   pair< uint32, SubscriptionResourceNotice > serviceResource(
      make_pair( serviceID, SubscriptionResourceNotice( resource ) ) ); 
   ::list< PushPacket* > storedPackets;

   m_pushServiceMutex.lock();
   PushServiceThreadMap::iterator it = m_pushServiceThreadMap.lower_bound(
      serviceResource );
   
   while( it != m_pushServiceThreadMap.end() &&
          it->first == serviceResource ) 
   {
      // Check for thread
      if ( it->second.handle.get() == thread.get() ) {
         // Add all stored packets
         it->second.storePackets = false;
         storedPackets.swap( it->second.storedPackets );
         break;
      }
      ++it;
   }
   m_pushServiceMutex.unlock();

   // Call putPushPacket on thread
   while ( !storedPackets.empty() ) {
      ::list< PushPacket* >::reference ref = storedPackets.front();
      thread->putPushPacket( ref, serviceID, resource );

      delete ref;
      storedPackets.pop_front();
   }
}


void 
ParserThreadGroup::putPushPacket( PushPacket* packet, uint32 serviceID, 
                                  SubscriptionResource* resource )
{
   mc2dbg1 << "ParserThreadGroup::putPushPacket serviceID "
           << serviceID << " resource ";
#ifdef DEBUG_LEVEL_1
   *resource << mc2log; 
#endif
   mc2dbg1 << endl;
   pair< uint32, SubscriptionResourceNotice > serviceResource (
      make_pair( serviceID, SubscriptionResourceNotice( *resource ) ) );

   m_pushServiceMutex.lock();
   PushServiceThreadMap::iterator it = m_pushServiceThreadMap.lower_bound(
      serviceResource );
   while( it != m_pushServiceThreadMap.end() && 
          it->first == serviceResource ) 
   {
      if ( it->second.handle->isAlive() ) {
         if ( it->second.storePackets ) {
            it->second.storedPackets.push_back( static_cast<PushPacket*>( 
               packet->getClone() ) );
         } else {
            it->second.handle->putPushPacket( 
               packet, serviceID, *resource );
         }
         ++it;
      } else {
         // This should never happen, client should deregister, but...
         mc2log << warn << "PTG::putPushPacket dead thread found in "
                << "pushServiceThreadMap." << endl;
         // Remove it
         m_pushServiceThreadMap.erase( it++ );
         // Check if no one alse uses service,resource
         PushServiceThreadMap::iterator fit = 
            m_pushServiceThreadMap.lower_bound( serviceResource );
         if ( fit == m_pushServiceThreadMap.end() || 
              fit->first != serviceResource )
         {
            m_handler->removePushServiceResource( serviceID, *resource );
         }
      }      
   }
   m_pushServiceMutex.unlock();

   delete packet;
   delete resource;
}


void
ParserThreadGroup::putRequest(ThreadRequestContainer* reqCont ) 
{
   ISABSync sync( m_monitor );

   DEBUG8(cerr << "     Inserting request container into handler!" 
          << endl);
   m_handler->insert( reqCont );
   DEBUG8(cerr << "     Request container inserted" << endl);
}

bool 
ParserThreadGroup::removeParserThread( ParserThreadHandle handle ) {
   // Remove any Push services
   m_pushServiceMutex.lock();
   PushServiceThreadMap::iterator it = m_pushServiceThreadMap.begin();
   while( it != m_pushServiceThreadMap.end() ) {
      if ( it->second.handle.get() == handle.get() ) {
         // Remove
         m_pushServiceThreadMap.erase( it++ );
      } else {
         ++it;
      }
   }
   m_pushServiceMutex.unlock();

   ISABSync sync( m_monitor );

   mc2dbg4 << "ParserThreadGroup::removeParserThread(): nbrHandles before: "
           << m_threads.size() << endl;
   bool handleDeleted = false;
   threadHandleStructure_t::iterator p = m_threads.begin();
   while (p != m_threads.end()) {
//      if (*p == handle) {     // the equality operator is not used, instead
//      the compiler uses "operator bool()" when it does the test
      if ((*p).get() == handle.get()) {
         handleDeleted = true;
         mc2dbg4 << "ParserThreadGroup::removeParserThread(): deleting a "
                    "handle: " << *handle << ", iter: " << **p
                 << endl;
         m_threads.erase(p++);
      } else {
         ++p;
      }
   }

   mc2dbg4 << "ParserThreadGroup::removeParserThread(): nbrHandles after: "
           << m_threads.size() << endl;
   return (handleDeleted);
}


uint16 
ParserThreadGroup::getNextRequestID() 
{
   ISABSync sync( m_monitor );
   return m_handler->getNextRequestID();
}


MonitorCache* 
ParserThreadGroup::getCache() 
{
   return (m_handler->getCache());
}


void 
ParserThreadGroup::setNamedServerLists( const char* namedServerList ) {
   if ( namedServerList != NULL) {
      m_namedServerLists->parseServerFile( namedServerList );
   }
}


const MC2String&
ParserThreadGroup::getNamedServerList( const MC2String& listName, 
                                       const MC2String& type ) const
{
   m_serverListMutex.lock();
   const MC2String& ret = m_namedServerLists->findServerList(
      listName, type );
   m_serverListMutex.unlock();
   return ret;
}


const NamedServerList* 
ParserThreadGroup::getServerList( const MC2String& listName, 
                                  const MC2String& type ) const
{
   return m_namedServerLists->findNamedServerList( listName, type );
}


uint32 
ParserThreadGroup::getNamedServerListCRC( const MC2String& listName, 
                                          const MC2String& type ) const
{
   m_serverListMutex.lock();
   uint32 ret = m_namedServerLists->getServerListsCRC( listName, type );
   m_serverListMutex.unlock();
   return ret;
}


bool
ParserThreadGroup::replaceNamedServerList( const MC2String& name,
                                           const MC2String& type,
                                           const MC2String& serverList )
{
   m_serverListMutex.lock();
   bool res = m_namedServerLists->replaceServerList( 
      name, type, serverList );
   m_serverListMutex.unlock();
   return res;
}


void 
ParserThreadGroup::setClientSettingStorage(const char* clientSettings )
{
   ISABSync sync(m_clientSettingMutex);
   delete m_clientSettingStorage;
   m_clientSettingStorage = new ClientSettingStorage( this );
   m_clientSettingStorage->parseSettingsFile( clientSettings );
}


const ClientSetting* 
ParserThreadGroup::getSetting( 
   const char* clientType, const char* clientOptions ) const
{
   m_clientSettingMutex.lock();
   const ClientSetting* cs = m_clientSettingStorage->getSetting( 
      clientType, clientOptions );
   m_clientSettingMutex.unlock();
   return cs;
}


void 
ParserThreadGroup::getCreateWFTime( const ClientSetting* setting,
                                    uint32& endTime ) const 
{
   endTime = ParserUserHandler::addTime( 
      setting->getCreateRegionTimeYear(),
      setting->getCreateRegionTimeMonth(),
      setting->getCreateRegionTimeDay(),
      setting->getExplicitCreateRegionTime(), endTime ); 
}

void addMapRightsForRegion( const TopRegionRequest& topReg, 
                            uint32 topRegionID, 
                            const MapRights& startRights,
                            UserUser::mapRightMap_t& rightMap ) {
   const TopRegionMatch* topMatch =
      topReg.getTopRegionWithID( topRegionID );
   if ( topMatch == NULL ) {
      // User has region that is not on this cluster.
      return;
   }
   MapRights rights = startRights;
   set<uint32> bottomMaps;
   topMatch->getItemIDTree().getLowestLevelMapIDs( bottomMaps );
   for ( set<uint32>::const_iterator mapit = bottomMaps.begin();
         mapit != bottomMaps.end(); ++mapit ) {         
      vector<uint32> overviews;
      // Only the fully covered maps will be returned here, I think.
      topMatch->getItemIDTree().getOverviewMapsFor( *mapit, overviews );
      // First check for other rights on the map. (Could happen if the
      // regions overlap).
      UserUser::mapRightMap_t::const_iterator findit =
         rightMap.find( *mapit );
      if ( findit != rightMap.end() ) {
         // Already has user rights. Don't forget them.
         rights |= findit->second.second;
      }
      mc2dbg8 << "[ParserThreadGroup]: Map " << MC2HEX(*mapit);
      if ( overviews.empty() ) {
         rightMap[*mapit] = make_pair( *mapit, rights );
         mc2dbg8 << " has no full overview maps" << endl;
      } else {
         // FIXME: Only one fully covered overview map is supported
         rightMap[*mapit] = make_pair( overviews[0], rights );
         mc2dbg8 << " has " << MC2HEX( overviews[0] ) << " above" << endl;
      }
      mc2dbg8 << "[ParserThreadGroup]:   MapRights are " << rights << endl;
   }
}

void 
ParserThreadGroup::addExtraRights( UserUser* user, 
                                   uint32 now, 
                                   ParserThreadHandle thread ) 
{
   // Depending if user is special in some way, has a specific brand,
   // add some using for example activation right string.
   // thread->getActivationHandler()->addUserRights( user, "", "", MAX_UINT32, 
   //    "GOLD(0,2097213)", "Right not to be in DB", now, MAX_UINT32 );

   // The demo clients gets free access
   if ( user->getBrand() == MC2String( "DEMO" ) ) {
      thread->getActivationHandler()->addUserRights( 
         user, "", "", MAX_UINT32, "GOLD(0,2097181),SCDB_SPEEDCAM(0,2097181)", 
         "Right not to be in DB", now, MAX_UINT32 );
   }
}

bool
ParserThreadGroup::updateRightsCache( UserUser* user,
                                      ParserThreadHandle thread )
{
   mc2dbg4 << "[ParserThreadGroup]::updateRightsCache for "
          << user->getLogonID() << endl;
   uint32 now = TimeUtility::getRealTime();

   // The TopRegionRequest is needed.
   const TopRegionRequest* topReg = getTopRegionRequest( thread );
   if ( topReg == NULL ) {
      mc2log << error << "[ParserThreadGroup] Could not get top region request" 
             << endl;
      return false;
   }

   // Add the top regions from the TopRegionRequest
   vector<uint32> topRegionIDs;   
   for ( uint32 i = 0; i < topReg->getNbrTopRegions(); ++i ) {
      topRegionIDs.push_back( topReg->getTopRegion(i)->getID() );
   }

   // Add some rights that should not be in db.
   addExtraRights( user, now, thread );

   // First update the regions. The UserUser can to that almost by
   // itself.
   user->updateRegionRightsCache( *getRegionIDs(), 
                                  now,
                                  topRegionIDs );

   // This is the target map
   UserUser::mapRightMap_t rightMap;   
   
   const UserUser::regionRightMap_t& userRegs = user->getRegionRightsCache();
   for ( UserUser::regionRightMap_t::const_iterator regit = userRegs.begin();
         regit != userRegs.end();
         ++regit ) {
      addMapRightsForRegion( *topReg, (*regit).first, (*regit).second, 
                             rightMap );
   }

   // Swap in the cache into the user.
   user->swapInMapRightsCache( rightMap );
   
   return true;
}


bool 
ParserThreadGroup::getUser( uint32 UIN, UserItem*& userItem, 
                            ParserThreadHandle thread,
                            bool useCache,
                            bool wipeFromCache )
{
   bool ok = false;

   if ( useCache ) {
      m_userCacheMutex.lock();
      UserItem* userEl = static_cast<UserItem*>(
         getCache()->find( UIN, CacheElement::USER_ELEMENT_TYPE ) );
      if ( userEl != NULL ) {
         if ( TimeUtility::getRealTime() - userEl->getTimeStamp() < 
              MAX_USER_AGE && !wipeFromCache ) 
         {
            userItem = userEl;
            ok = true;
         } else { // Too stale, get new
            getCache()->remove( userEl ); // Remove stale from cache
            getCache()->releaseCacheElement( userEl ); // Find locked it
         }
      }
   }

   if ( !ok ) { // Get user from UM
      PacketContainer* cont = new PacketContainer( 
         new GetUserDataRequestPacket( 
            0, 0, UIN, UserConstants::TYPE_ALL, wipeFromCache ) );
      cont->setModuleType( MODULE_TYPE_USER );
      SinglePacketRequest* req = new SinglePacketRequest( 
         getNextRequestID(), cont );
      ThreadRequestContainer* reqCont = 
         new ThreadRequestContainer( req );
      mc2dbg8 << "About to send GetUserRequest for " << UIN << endl;
      thread->putRequest( reqCont );
      mc2dbg8 << "GetUserRequest returned" << endl;
      userItem = NULL;
      PacketContainer* getAnsCont = req->getAnswer();
      if ( getAnsCont != NULL ) {
         GetUserDataReplyPacket* getAns = 
            static_cast< GetUserDataReplyPacket* > ( 
               getAnsCont->getPacket() );
         UserItem* item = new UserItem( getAns );
         if ( item->getValid() ) {
            // All is ok

            // Update the region rights cache.
            // Note that the region cache depends on that
            // there is a time limit on the User-cache
            // since it takes the regions/rights that are allowed now.
            UserUser* user = item->getUser();
            ok = updateRightsCache( user, thread );
            
            if ( ok ) {
               userItem = item;
               if ( useCache ) {
                  getCache()->addAndLock( userItem );
               }
            } else {
               mc2log << error
                      << "[PTG]: Failed to update region cache for user"
                      << endl;
               userItem = NULL;
               delete item;
            }
            mc2dbg8 << "ParserThreadGroup::getUser all ok" << endl;
         } else {
            mc2dbg8 << "ParserThreadGroup::getUser GetUser UserItem not OK"
                    << endl;
            // got an answer, so the request is OK, but no user found
            ok = true;
            userItem = NULL;
            delete item;
         }
      } else {
         mc2dbg8 << "ParserThreadGroup::getUser GetUser no answer" << endl;
      }
      delete reqCont;
      delete req;
      delete getAnsCont;
   }

   if ( useCache ) {
      m_userCacheMutex.unlock();
   }
   
   return ok;
}


bool 
ParserThreadGroup::changeUser( UserUser* user, 
                               const UserUser* changerUser,
                               ParserThreadHandle thread )
{
   if ( user == NULL ) {
      mc2log << warn << "ParserThreadGroup::changeUser "
             << "Tries to change NULL-user!!!" << endl;
      return false;
   }
   
   bool ok = false;

   // Add data to packet
   ChangeUserDataRequestPacket* p = new ChangeUserDataRequestPacket(
      0, 0, user->getUIN(), changerUser ? changerUser->getUIN(): 0 );
   if ( user->getNbrChanged() > 0 ) {
      p->addChangedElement( user );
   }
   
   vector<UserElement*> changed_elements;
   user->getAllChangedElements( changed_elements );
   for ( uint32 i = 0, n = changed_elements.size(); i < n; ++i ) {
      p->addChangedElement( changed_elements[i] );
   }

   // Send packet
   SinglePacketRequest* req = new SinglePacketRequest( 
      getNextRequestID(), 
      new PacketContainer( 
         p, 0, 0, MODULE_TYPE_USER ) ); 
   ThreadRequestContainer* reqCont = new ThreadRequestContainer(
      req );
   mc2dbg8 << "About to send ChangeUserRequest" << endl;
   thread->putRequest( reqCont );
   mc2dbg8 << "ChangeUserRequest returned" << endl;

   // Check answer
   PacketContainer* ansCont = req->getAnswer();
   if ( ansCont != NULL &&
        static_cast< ChangeUserDataReplyPacket* > (
           ansCont->getPacket() )->getStatus() == StringTable::OK )
   {
      ok = true;
   }

   delete ansCont;
   delete reqCont;
   delete req;

   // Remove all cached version of this user
   removeUserFromCache( user->getUIN() );

   return ok;
}


void 
ParserThreadGroup::releaseUserItem( UserItem* userItem ) {
   if ( userItem != NULL ) {
      m_userCacheMutex.lock();
      getCache()->releaseCacheElement( userItem );
      m_userCacheMutex.unlock();
   }   
}


void
ParserThreadGroup::removeUserFromCache( uint32 UIN ) {
   m_userCacheMutex.lock();
   CacheElement* el = getCache()->find( 
      UIN, CacheElement::USER_ELEMENT_TYPE );
   while ( el != NULL ) {
      getCache()->remove( el );
      getCache()->releaseCacheElement( el );
      el = getCache()->find( UIN, CacheElement::USER_ELEMENT_TYPE );
   }

   m_userCacheMutex.unlock();
}


CellularPhoneModelsElement* 
ParserThreadGroup::getCurrentCellularPhoneModels(
   ParserThreadHandle thread )
{
   m_phoneModelsMutex.lock();

   time_t now = TimeUtility::getRealTime();

   if ( m_phoneModels == NULL || 
        (now - m_lastPhoneModelsUpdate) > MAX_PHONEMODELS_AGE ||
        m_phoneModels->getNbrManufacturers() == 0 )
   {
      // Get new CellularPhoneModels
      CellularPhoneModels* newModels = NULL;
      GetCellularPhoneModelDataRequestPacket* p
         = new GetCellularPhoneModelDataRequestPacket( 0, 0 );

      SinglePacketRequest* request = new SinglePacketRequest(
         getNextRequestID(),
         new PacketContainer( p, 0, 0, MODULE_TYPE_USER ) );

      ThreadRequestContainer* reqCont = 
         new ThreadRequestContainer( request );
      reqCont->setThread( thread );

      // Process the request
      mc2dbg8 << "About to send GetCellularPhoneModelDataRequest" << endl;
      thread->putRequest( reqCont );
      mc2dbg8 << "GetCellularPhoneModelDataRequest returned" << endl;

      PacketContainer* ansCont = request->getAnswer();

      if ( ansCont != NULL ) {
         GetCellularPhoneModelDataReplyPacket* reply =
            static_cast< GetCellularPhoneModelDataReplyPacket* > (
               ansCont->getPacket() );

         if( reply->getNbrCellularPhoneModels() > 0 ) {
            CellularPhoneModels* models = reply->getAllPhoneModels();
            if ( models != NULL && models->getValid() ) {
               newModels = models;
            } else {
               delete models;
            }
         }
      }
      delete ansCont;
      delete reqCont;
      delete request;

      if ( newModels == NULL ) {
         // Error couldn't get phoneModels check database!
         mc2log << warn << "ParserThreadGroup::"
                   "getCurrentCellularPhoneModels "
                   "couldn't get CellularPhoneModels check database!"
                << endl;
         newModels = new CellularPhoneModels();
         newModels->addPhoneModel( new CellularPhoneModel( 
            "UNKNOWN", "Default",
            16, UserConstants::EOLTYPE_AlwaysCRLF,
            3, true, 50, 50, 
            CellularPhoneModel::SMSCAPABLE_YES,
            CellularPhoneModel::SMSCONCATENATION_NO,
            CellularPhoneModel::SMSGRAPHICS_NO,
            CellularPhoneModel::WAPCAPABLE_NO,
            "", 2000, "" ) );
      }

      // Remove old from cache
      if ( m_phoneModels != NULL ) {
         getCache()->remove( m_phoneModels );
         getCache()->releaseCacheElement( m_phoneModels );
      }
      // Add to cache
      m_phoneModels = new CellularPhoneModelsElement( newModels );
      getCache()->addAndLock( m_phoneModels );
      m_lastPhoneModelsUpdate = now;
   }

   CellularPhoneModelsElement* res = m_phoneModels;
   m_phoneModels->addLock();

   m_phoneModelsMutex.unlock();
   return res;
}


const TopRegionRequest* 
ParserThreadGroup::getTopRegionRequest( ParserThreadHandle thread ) {
   TopRegionRequest* topRequest = NULL;
   ISABSync sync( m_topRegionsMutex );
   if ( m_topRegionRequest == NULL ) {
      // Get a new one
      topRequest = new TopRegionRequest( getNextRequestID() );
      thread->putRequest( topRequest );
      if ( topRequest->getStatus() == StringTable::OK) {
         if (topRequest->isCacheable()) {
            m_topRegionRequest = topRequest;
         } else {
            // this will leak memory, but it's better than returning NULL
            // FIXME somehow.
            /* delete topRequest;
            topRequest = NULL; */
            mc2log << warn << "[PTG] TopRegionRequest not cacheable" << endl;
         }
      } else {
         delete topRequest;
         topRequest = NULL;
         mc2log << warn << "[PTG] TopRegionRequest not OK, returning NULL"
                << endl;
      }
   } else {
      topRequest = m_topRegionRequest;
   }
   return topRequest;
}

StringTable::stringCode
ParserThreadGroup::getEdgeNodesFor( const vector<uint32>& mapIDs, uint32 overMapID, 
                                    vector<uint32>& externalNodes,
                                    ParserThreadHandle thread )
{

   // 
   // First we try to find external nodes in cache
   // else we generate cache.
   // So in the first case we need to find which 
   // ids have cache and which dont.
   // So to keep track of which had cache we use
   // the vector nonCachedMapIDs which is later
   // used to create new cache
   //
   vector<uint32> nonCachedMapIDs;
   // worst case scenario; nothing is cached, so
   // we reserve space for this.
   nonCachedMapIDs.reserve( mapIDs.size() );

   // stage 1: 
   // Search cache
   {
      // lock cache
      ISABSync sync( m_edgenodesMutex );
      // go through all map ids and search for cache
      for ( uint32 i = 0; i < mapIDs.size(); ++i ) {
         // Find in cache
         edgenodesMap::const_iterator findIt = 
            m_edgenodesData.find( make_pair( mapIDs[i], overMapID ) );

         if ( findIt != m_edgenodesData.end() ) {
            // Found in cache
            mc2dbg8 << "Found " << prettyMapID( mapIDs[ i ] ) << " in "
                    << "edgenodes cache" << endl;
            externalNodes.insert( externalNodes.end(),
                                  findIt->second.begin(),
                                  findIt->second.end() );
       
         } else {
            // no cache, lets generate cache for this id in
            // the next stage.
            nonCachedMapIDs.push_back( mapIDs[ i ] );
         }
      }
   }

   // stage 2: 
   // Generate cache for those items that didnt have any.
   // a) First all edge node requests are generated for each map id
   //    so they can be sent away all at the same time with
   //    a multi request. 
   //
   // b) translate each edge node if needed,
   //    else add them to cache
   // 
   //
   // If any request fails the function will return and
   // edge node generation will fail.

   
   // create edge node request for each map id
   typedef STLUtility::AutoContainer< vector<PacketContainer*> > PCVector;
   PCVector containers;
   containers.resize( nonCachedMapIDs.size() );
   if ( !nonCachedMapIDs.empty() ) {
      mc2dbg << "getEdgeNodesFor: About to send EdgeNodesRequest "
             << " on map(s): ";
   }
   for ( uint32 i = 0; i < nonCachedMapIDs.size(); ++i ) { 
      mc2dbg << MC2HEX( nonCachedMapIDs[ i ] ) << ", ";
         
      EdgeNodesRequestPacket* edgeNodePacket = 
         new EdgeNodesRequestPacket( 0, 0, nonCachedMapIDs[ i ],
                                     0, OrigDestInfoList() );
      containers[ i ] = new PacketContainer( edgeNodePacket, 
                                             0, 0, MODULE_TYPE_ROUTE );
   }
   if ( !nonCachedMapIDs.empty() ) {
      mc2dbg << endl;
   }

   thread->putRequest( containers );

   // ok at this point all request are ok,
   // now lets see if any of the edge nodes need to translated,

   // must have same size since we extract map id from nonCachedMapIDs
   MC2_ASSERT( containers.size() == nonCachedMapIDs.size() );
   
   // holds packet containers to translation requests
   PCVector translationRequests;

   // holds map ids for each translation request
   // so they can be used when each translation is added to cache
   vector<uint32> translationMapIDs;
   // holds node lists for each translation request
   vector<OrigDestInfoList> nodeLists;

   StringTable::stringCode status = StringTable::OK;

   for ( uint32 i = 0; i < containers.size(); ++i ) {
      if ( containers[ i ] == NULL ) {
         status = StringTable::TIMEOUT_ERROR;
         continue;
      }

      EdgeNodesReplyPacket* reply = static_cast< EdgeNodesReplyPacket* >
         ( containers[ i ]->getPacket() );
      if ( reply->getStatus() != StringTable::OK ) {
         // a request failed!
         status =  StringTable::stringCode( reply->getStatus() );
         continue;
      }

      OrigDestInfoList nodeList;
           
      reply->getEdgeNodes( nodeList );

      // need to translate if map id is not overview map id
      if ( nonCachedMapIDs[ i ] != overMapID ) {
         mc2dbg4 << "nonCachedMapIDs[ i ] != overMapID "
                 <<  nonCachedMapIDs[ i ] << " != " << overMapID << endl;

         IDTranslationRequestPacket* idp =
            new IDTranslationRequestPacket( 0, 0,
                                            overMapID,
                                            false,
                                            nodeList );
         translationRequests.
            push_back( new PacketContainer( idp, 0, 0, MODULE_TYPE_ROUTE ) );
         // store map id too, so we can map it to packet container later
         // in the loop where we add the translated id to cache
         // ( the same goes for nodeList.)
         translationMapIDs.push_back( nonCachedMapIDs[ i ] );
         nodeLists.push_back( nodeList );

      } else { // no need to translate, just add it to cache
         vector<uint32> nodes;
         for ( OrigDestInfoList::const_iterator it = nodeList.begin();
               it != nodeList.end() ; ++it ) {
            nodes.push_back( it->getNodeID() );
         }
         // Cache nodes
         ISABSync sync( m_edgenodesMutex );
         uint32 mapID = nonCachedMapIDs[ i ];
         m_edgenodesData.
            insert( make_pair( make_pair( mapID, overMapID ), nodes ) );
         // Set outdata
         externalNodes.insert( externalNodes.end(),
                               nodes.begin(),
                               nodes.end() );

      }
   }

   MC2_ASSERT( nodeLists.size() == translationRequests.size() );
   MC2_ASSERT( translationMapIDs.size() == translationRequests.size() );

   // send id translation requests
   thread->putRequest( translationRequests );
   // holds return code, default value "success"
   StringTable::stringCode res = StringTable::OK;
            
   // now lets go through each translation request
   // and add them to cache, and if any request failed: exit function
   for ( uint32 idTranslationIdx = 0; 
         idTranslationIdx < translationRequests.size(); 
         ++idTranslationIdx ) {      
      if ( translationRequests[ idTranslationIdx ] == NULL ) {
         status = res;
         continue;
      }

      PacketContainer& idCont = *translationRequests[ idTranslationIdx ];
      IDTranslationReplyPacket* ir = 
         static_cast< IDTranslationReplyPacket* >( idCont.getPacket() );
      // did the request fail?
      if ( ir->getStatus() != StringTable::OK ) {
         res = StringTable::stringCode( ir->getStatus() );
         mc2log << warn << "IDTranslationRequest failed status: " 
                << StringTable::getString( res, StringTable::ENGLISH )
                << endl;
         status = res;
         continue;
      }
          
      // successful request; add to cache
            
      OrigDestInfoList overNodeList;
            
      ir->getTranslatedNodes( 0, overNodeList, 
                              nodeLists[ idTranslationIdx ] );
      vector<uint32> nodes;
      for ( OrigDestInfoList::const_iterator it = 
               overNodeList.begin();
            it != overNodeList.end() ; ++it ) {
         if ( it->getNodeID() != MAX_UINT32 ) {
            // Present on higher map
            nodes.push_back( it->getNodeID() );
         }
      }
      // Cache nodes
      ISABSync sync( m_edgenodesMutex );
      uint32 mapID = translationMapIDs[ idTranslationIdx ];
      m_edgenodesData.
         insert( make_pair( make_pair( mapID, overMapID ), nodes ) );
      // Set outdata
      externalNodes.insert( externalNodes.end(),
                            nodes.begin(),
                            nodes.end() );
   } // End for all translationRequests

   if ( status != StringTable::OK ) {
      return status;
   }

   return res;
      
}

StringTable::stringCode 
ParserThreadGroup::getEdgeNodesFor( uint32 mapID, uint32 overMapID, 
                                    vector<uint32>& externalNodes,
                                    ParserThreadHandle thread )
{
   {
      ISABSync sync( m_edgenodesMutex );
      // Find in cache
      edgenodesMap::const_iterator findIt = m_edgenodesData.find(
         make_pair( mapID, overMapID ) );
      if ( findIt != m_edgenodesData.end() ) {
         // Found in cache
         externalNodes.insert( externalNodes.end(),
                               findIt->second.begin(),
                               findIt->second.end() );
         return StringTable::OK;
      }
   }
   
   StringTable::stringCode res = StringTable::OK;
   {
      // Get it and cache it
      OrigDestInfoList nodeList;
      EdgeNodesRequestPacket* p = new EdgeNodesRequestPacket( 
         0, 0, mapID, 0, nodeList );
      p->setMapID( mapID );
      
      mc2dbg << "getEdgeNodesFor: About to send EdgeNodesRequest "
             << " on map " << MC2HEX( mapID ) << endl;
      auto_ptr<PacketContainer> cont(
         thread->putRequest( p , MODULE_TYPE_ROUTE ) );
      mc2dbg8 << "EdgeNodesRequest returned" << endl;

      if ( cont.get() != NULL &&
           static_cast< ReplyPacket* >( cont->getPacket() )->getStatus() 
           == StringTable::OK )
      {
         EdgeNodesReplyPacket* r = static_cast< EdgeNodesReplyPacket* >( 
            cont->getPacket() );

         r->getEdgeNodes( nodeList );
      
         if ( overMapID != mapID ) {
            auto_ptr<PacketContainer> icont;
            // TRANSLATE nodeIDs TO overview mapID IDs!!!
            {
               IDTranslationRequestPacket* idp =
                  new IDTranslationRequestPacket( 0, 0,
                                                  overMapID,
                                                  false,
                                                  nodeList );
               mc2dbg8 << "About to send IDTranslationRequest" << endl;
               icont.reset (
                  thread->putRequest( idp, MODULE_TYPE_ROUTE ) );
               mc2dbg8 << "IDTranslationRequest returned" << endl;
            }
         
            if ( icont.get() != NULL &&
                 static_cast< ReplyPacket* >( icont->getPacket() )
                 ->getStatus() == StringTable::OK )
            {
               OrigDestInfoList overNodeList;
               IDTranslationReplyPacket* ir = static_cast< 
                  IDTranslationReplyPacket* > ( icont->getPacket() );
               ir->getTranslatedNodes( 0, overNodeList, nodeList );
               vector<uint32> nodes;
               for ( OrigDestInfoList::const_iterator it = 
                        overNodeList.begin();
                     it != overNodeList.end() ; ++it )
               {
                  if ( it->getNodeID() != MAX_UINT32 ) {
                     // Present on higher map
                     nodes.push_back( it->getNodeID() );
                  }
               }
               // Cache nodes
               ISABSync sync( m_edgenodesMutex );
               m_edgenodesData.insert( 
                  make_pair( make_pair( mapID, overMapID ), nodes ) );
               // Set outdata
               externalNodes.insert( externalNodes.end(),
                                     nodes.begin(),
                                     nodes.end() );
            } else {
               if ( icont.get() != NULL ) {
                  res = StringTable::stringCode( static_cast<ReplyPacket*>( 
                  icont->getPacket() )->getStatus() );
                  mc2log << warn << "IDTranslationRequest failed status: " 
                         << StringTable::getString( res, 
                                                    StringTable::ENGLISH )
                         << endl;
               } else {
                  mc2log << "IDTranslationRequest failed no reply" << endl;
                  res = StringTable::TIMEOUT_ERROR;
               }
            }
         } else { // mapID == overMapID
            vector<uint32> nodes;
            for ( OrigDestInfoList::const_iterator it = nodeList.begin();
                  it != nodeList.end() ; ++it )
            {
               nodes.push_back( it->getNodeID() );
            }
            // Cache nodes
            ISABSync sync( m_edgenodesMutex );
            m_edgenodesData.insert( 
               make_pair( make_pair( mapID, overMapID ), nodes ) );
            // Set outdata
            externalNodes.insert( externalNodes.end(),
                                  nodes.begin(),
                                  nodes.end() );
         }
      } else { // If edgenodereply not ok
         if ( cont.get() != NULL ) {
            res = StringTable::stringCode( 
               static_cast<ReplyPacket*>( cont->getPacket() )
               ->getStatus() );
         } else {
            res = StringTable::TIMEOUT_ERROR;
         }
      }
   }

   return res;
}


uint32
ParserThreadGroup::authenticateUser( 
   const char* userName, 
   const char* userPasswd,
   bool checkExpired,
   ParserThreadHandle thread )
{
   m_userLoginCacheMutex.lock();
   bool ok = false;
   uint32 UIN = 0;
   uint32 now = TimeUtility::getRealTime();
   const char* cachedKey = NULL;
   uint32 cachedUIN = 0;
   uint32 cachedTime = 0;
   if ( m_userLoginCache->getSession( userName, cachedKey, cachedUIN,
                                      cachedTime ) )
   {
      // Check cachedKey 
      if ( StringUtility::strcmp( cachedKey, userPasswd ) == 0 ) {
         // Check cachedTime
         if ( now - cachedTime < MAX_SESSION_AGE ) {
            // Ok!
            ok = true;
            UIN = cachedUIN;
         } else {
            m_userLoginCache->removeSession( userName );
         }
      } else {
         m_userLoginCache->removeSession( userName );
      }
   }

   if ( !ok ) {   
      AuthUserRequestPacket* p = new AuthUserRequestPacket(
         0, 0, userName, userPasswd, checkExpired );
      SinglePacketRequest* req = new SinglePacketRequest( 
         getNextRequestID(), 
         new PacketContainer( p, 0, 0, MODULE_TYPE_USER ) ); 
      mc2dbg8 << "About to send AuthUserRequest" << endl;
      thread->putRequest( req );
      mc2dbg8 << "AuthUserRequest returned" << endl;
      PacketContainer* ansCont = req->getAnswer();
      if ( ansCont != NULL ) {
         UIN = static_cast< AuthUserReplyPacket* > ( 
         ansCont->getPacket() )->getUIN();
         if ( UIN != 0 && UIN != MAX_UINT32 && UIN != MAX_UINT32-1 ) {
            // Add to cache
            m_userLoginCache->addSession( userName, userPasswd, 
                                          UIN, now );
         }
      } else {
         UIN = MAX_UINT32;
      }

      delete ansCont;
      delete req;
   }

   m_userLoginCacheMutex.unlock();
   return UIN;
}



uint32
ParserThreadGroup::authenticateUserSession( 
   const char* sessionID, 
   const char* sessionKey,
   bool checkExpired,
   ParserThreadHandle thread )
{
   bool ok = false;
   uint32 UIN = 0;
   int32 now = TimeUtility::getRealTime();
   const char* cachedKey = NULL;
   uint32 cachedUIN = 0;
   uint32 cachedTime = 0;
   {
      // Lock cache while searching it
      ISABSync synkare( m_userSessionCacheMutex );
      if ( m_userSessionCache->getSession( sessionID, cachedKey, cachedUIN,
                                           cachedTime ) ) {
         // Check cachedKey 
         if ( StringUtility::strcmp( cachedKey, sessionKey ) == 0 ) {
            // Check cachedTime
            if ( now - int32(cachedTime) < int32(MAX_SESSION_AGE) ) {
               // Ok!
               ok = true;
               UIN = cachedUIN;
            } else {
               m_userSessionCache->removeSession( sessionID );
            }
         } else {
            m_userSessionCache->removeSession( sessionID );
         }
      }
      // Unlock cache when communicating with the UserModule
   }
   
   if ( !ok ) {
      // Check with the UserModule
      mc2dbg8 << "About to send VerifyUserRequest" << endl;
      auto_ptr<PacketContainer> ansCont(
         thread->putRequest( new VerifyUserRequestPacket(
            0, 0, sessionID, sessionKey, checkExpired ), MODULE_TYPE_USER ) );
      mc2dbg8 << "VerifyUserRequest returned" << endl;

      if ( ansCont.get() != NULL && static_cast< VerifyUserReplyPacket* > ( 
              ansCont->getPacket() )->getStatus() == StringTable::OK ) {
         UIN = static_cast< VerifyUserReplyPacket* > ( 
            ansCont->getPacket() )->getUIN();
         if ( UIN != 0 && UIN != MAX_UINT32 && UIN != MAX_UINT32-1 ) {
            // Add to cache while mutex is locked.
            ISABSync synken( m_userSessionCacheMutex );
            m_userSessionCache->addSession( sessionID, sessionKey, 
                                            UIN, now );
         }
      } else {
         UIN = MAX_UINT32 -2;
      }      
   }

   return UIN;
}


bool
ParserThreadGroup::getUserBySession( 
   const char* sessionID, const char* sessionKey,
   UserItem*& userItem, bool useCache,
   bool wipeFromCache,
   ParserThreadHandle thread )
{  
   bool ok = false;
   uint32 UIN = authenticateUserSession( sessionID, sessionKey,
                                         false, thread );
   
   if ( UIN != 0 &&                // if invalid login
        UIN != MAX_UINT32 &&      // if expired session
        UIN != MAX_UINT32 - 1 && // if expired user
        UIN != MAX_UINT32 - 2   // Timeout
        ) {
      ok = getUser( UIN, userItem, thread, useCache, wipeFromCache );
   } else { // invalid or expired session
      ok = true;
   }

   return ok;
}

BitBuffer*
ParserThreadGroup::newTileMapFormatDescBuffer( const STMFDParams& settings,
                                               ParserThreadHandle thread )
{
   m_tileMapFormatDescMutex.lock();
   tfmdmap_t::const_iterator it = getTMFDIterator( settings, thread );
   BitBuffer* tmpBuf = it->second->newTMFDBuffer();
   m_tileMapFormatDescMutex.unlock();
   return tmpBuf;
}

BitBuffer*
ParserThreadGroup::newTileMapFormatDescCRCBuffer( const STMFDParams& settings,
                                                  ParserThreadHandle thread )
{
   m_tileMapFormatDescMutex.lock();
   tfmdmap_t::const_iterator it = getTMFDIterator( settings, thread );
   BitBuffer* tmpBuf = it->second->newCRCBuffer();
   m_tileMapFormatDescMutex.unlock();
   return tmpBuf;
}

const ServerTileMapFormatDesc*
ParserThreadGroup::getTileMapFormatDesc( const STMFDParams& settings,
                                         ParserThreadHandle thread )
{
   m_tileMapFormatDescMutex.lock();
   tfmdmap_t::const_iterator it = getTMFDIterator( settings, thread );
   const ServerTileMapFormatDesc* tmp = it->second->getTMFD();
   m_tileMapFormatDescMutex.unlock();
   return tmp;
}

CopyrightHolder ParserThreadGroup::getCopyrights( ParserThreadHandle thread,
                                                  const LangType& language ) {

   // get copyright strings
   auto_ptr<CopyrightRequest> 
      req( new CopyrightRequest( getNextRequestID(), language ) );
   thread->putRequest( req.get() );
   if ( req->getStatus() != StringTable::OK ) {
      mc2log << fatal << "[PTG] Failed to get copyrights!" << endl;
      MC2_ASSERT( false );
      return CopyrightHolder();
   }
   return req->getCopyrightHolder();
}

ParserThreadGroup::tfmdmap_t::const_iterator
ParserThreadGroup::getTMFDIterator( const STMFDParams& settings, 
                                    ParserThreadHandle thread )
{

   // Mutex must be locked when we get here.
   tfmdmap_t::const_iterator it = m_tileMapFormatDescMap.find( settings );
   if ( it != m_tileMapFormatDescMap.end() ) {
      mc2dbg8 << "[PTG]: Found cached TMFD" << endl;
      return it;
   } else if ( m_tileMapFormatDescMap.
               find( STMFDParams( LangTypes::invalidLanguage, false ) ) ==
               m_tileMapFormatDescMap.end() ) {
      // Make the invalidLanguage (DXXX) TileMapFormatDesc first
      // because the clients requires that the timestamp of DXXX is lower
      // or equal to the timestamp of the language-specific (DX-eng) one.
      // No need to lock mutex here as no threads are started yet.
      STMFDParams invalid( LangTypes::invalidLanguage, false );

      TileMapFormatDescData* desc =
         new TileMapFormatDescData( invalid, 
                                    getCopyrights( thread, 
                                                   LangTypes::invalidLanguage ) );

      m_tileMapFormatDescMap.insert( make_pair( invalid, desc ) );

      // init poi image table too
      const ServerTileMapFormatDesc* serverDesc = desc->getTMFD();

      m_poiImageTable.reset( new POIImageIdentificationTable( *serverDesc ) );

      const char* imagePath = Properties::getProperty( "IMAGES_PATH" );
      if ( imagePath == NULL ) {
         imagePath = "./";
      }
      // We MUST have the images!
      MC2_ASSERT( m_poiImageTable->validateImageFilesOnDisc( imagePath ) );

      // in case we actually asked for the "invalid" setting; 
      return getTMFDIterator( settings, thread );
   }


   TileMapFormatDescData* desc =
      new TileMapFormatDescData( settings, 
                                 getCopyrights( thread, settings.getLanguage() ) );
   // Must insert new
   m_tileMapFormatDescMap.insert( make_pair( settings, desc ) );

   mc2dbg << "[PTG]: Created new TMFD for " << settings << endl;
   return getTMFDIterator( settings, thread );
}

MC2String 
ParserThreadGroup::getPOIImage( const MC2String& imageCode,
                                ParserThreadHandle thread ) {

   MC2_ASSERT( m_poiImageTable.get() );
   return m_poiImageTable->decode( imageCode );
}

PacketContainer* 
ParserThreadGroup::getStoredRoute( uint32 routeID,
                                   uint32 createTime,
                                   RouteReplyPacket*& routePack,
                                   uint32& UIN,
                                   const char*& extraUserinfo,
                                   uint32& validUntil,
                                   int32& originLat,
                                   int32& originLon,
                                   uint32& originMapID,
                                   uint32& originItemID,
                                   uint16& originOffset,
                                   int32& destinationLat,
                                   int32& destinationLon,
                                   uint32& destinationMapID,
                                   uint32& destinationItemID,
                                   uint16& destinationOffset,
                                   ParserThreadHandle thread )
{
   RouteID rid( routeID, createTime );

   PacketContainer* foundCont = NULL;
   Packet* pack = NULL;
   m_routeStorageMutex.lock();
   routeStorage_t::const_iterator it = m_routeStorage.find( rid );
   if ( it != m_routeStorage.end() ) {
      foundCont = it->second;
   }

   if ( foundCont != NULL ) {
      // Found it in the map. 
      PacketContainer* cont = foundCont;
      RouteStorageGetRouteReplyPacket* r = static_cast<
         RouteStorageGetRouteReplyPacket*> ( cont->getPacket() );
      routePack =  r->getRoutePacket();
      UIN = r->getUIN();
      extraUserinfo = r->getExtraUserinfo();
      validUntil = r->getValidUntil();
      originLat = r->getOriginLat();
      originLon = r->getOriginLon();
      originMapID = r->getOriginMapID();
      originItemID = r->getOriginItemID();
      originOffset = r->getOriginOffset();
      destinationLat = r->getDestinationLat();
      destinationLon = r->getDestinationLon();
      destinationMapID = r->getDestinationMapID();
      destinationItemID = r->getDestinationItemID();
      destinationOffset = r->getDestinationOffset();
      
      // Set pack here return it below
      pack = r->getClone(false);

      // Do some cleanup. Assume that the oldest route will be least
      // used.
      if ( m_routeStorage.size() > 
           Properties::getUint32Property(
              "ROUTE_STORAGE_CACHE_MAX_NBR", 100 ) ) 
      {
         routeStorage_t::iterator toDel = m_routeStorage.begin();
         delete toDel->second;
         m_routeStorage.erase( toDel );
      }
   }

   m_routeStorageMutex.unlock();

   if ( pack != NULL ) {
      return new PacketContainer( pack, 0, 0, MODULE_TYPE_INVALID );
   } else {
      // Must request a new one.
      PacketContainer* cont = thread->putRequest( 
         new RouteStorageGetRouteRequestPacket( routeID, createTime ),
         MODULE_TYPE_USER );

      if ( cont != NULL &&
           static_cast< ReplyPacket* >( cont->getPacket() )->getStatus() 
           == StringTable::OK ) {
         // Ok.
      } else {
         mc2dbg2 << "ParserThreadGroup::getStoredRoute failed to get "
                 << "stored route."
                 << "routeID " << routeID 
                 << " createTime " << createTime << endl;
         delete cont;
         cont = NULL;
      }
      
      // Check if complete route or we have to remake the route
      if ( cont && static_cast<RouteStorageGetRouteReplyPacket*> ( 
              cont->getPacket() )->getRoutePacketLength() == 0 ) {
         // Remake the route and store it
         bool ok = true;
         RouteStorageGetRouteReplyPacket* r = static_cast<
            RouteStorageGetRouteReplyPacket*> ( cont->getPacket() );
         uint32 userUIN = r->getUIN();
         DriverPref driverPref;
         r->getDriverPrefs( driverPref );

         // User? This user? check userUIN and currentUser.
         UserItem* userItem = thread->getCurrentUser();
         UserItem* releaseUserItem = NULL;
         if ( userItem == NULL || userUIN != userItem->getUIN() ) {
            if ( !thread->getUser( userUIN, releaseUserItem, true  ) ) {
               mc2log << warn << "ParserThreadGroup::getStoredRoute failed "
                      << "to get user (" << userUIN << ")" << endl;
               ok = false;
            } else {
               userItem = releaseUserItem;
            }
         }

         // Get AURA
         UserEnums::URType urmask = r->getUrmask();
         RouteAllowedMap* maps = NULL;
         if ( ok ) {
            if ( !thread->getMapIdsForUserRegionAccess( 
                    userItem->getUser(), maps, urmask ) )
            {
               ok = false;
               mc2log << warn << "ParserThreadGroup::getStoredRoute "
                      << "getMapIdsForUserRegionAccess failed." << endl;
            }
         }

         if ( ok ) {
            auto_ptr<RouteRequest> rr( 
               new RouteRequest( userItem->getUser(),
                                 thread->getNextRequestID(),
                                 (ROUTE_TYPE_STRING | ROUTE_TYPE_NAVIGATOR |
                                  ROUTE_TYPE_ITEM_STRING | ROUTE_TYPE_GFX),
                                 StringTable::ENGLISH, false, 0,
                                 thread->getTopRegionRequest() ) );
            rr->addOriginCoord( 
               r->getOriginLat(), r->getOriginLon(), r->getOriginAngle() );
            rr->addDestinationCoord( 
               r->getDestinationLat(), r->getDestinationLon() );
            
            rr->setRouteParameters( driverPref.getIsStartTime(),
                                    driverPref.getCostA(),
                                    driverPref.getCostB(),
                                    driverPref.getCostC(),
                                    driverPref.getCostD(),
                                    driverPref.getVehicleRestriction(),
                                    driverPref.getTime(),
                                    driverPref.useUturn(),
                                    true/*abbreviate*/,
                                    false/*addLandmarks*/,
                                    driverPref.avoidTollRoads(),
                                    driverPref.avoidHighways() );
            // If to check route w. disturbances
            if ( driverPref.getCostC() != 0 ) {
               // This makes route_with_disturbances work
               // One doesn't think so but it does...
               rr->setCompareDisturbanceRoute( true );
            }
            rr->setAllowedMaps( maps );

            thread->putRequest( rr.get() );

            if ( rr->getStatus() == StringTable::OK ) {
               RouteStorageGetRouteReplyPacket* p = NULL;
               ok = thread->storeRoute( rr.get(), userUIN, 
                                        ""/*extraUserinfo*/,
                                        urmask,
                                        RouteID( routeID, createTime ),
                                        &p );
               // Add to cache, makes sure we don't goes into loop
               if ( p != NULL ) {
                  delete cont;
                  cont = new PacketContainer( p, 0, 0, MODULE_TYPE_INVALID );
               } else {
                  ok = false;
               }
            } else {
               ok = false;
               mc2log << "ParserThreadGroup::getStoredRoute reroute of "
                      << "stored route failed."
                      << endl;
               rr->dumpState();
            }
         } // End if ok to start routing

         thread->releaseUserItem( releaseUserItem );
         if ( !ok ) {
            delete cont;
            cont = NULL;
         }

      } // End if we have to remake the route

      if ( cont ) {
         m_routeStorageMutex.lock();
         pair<routeStorage_t::iterator, bool> insres = 
            m_routeStorage.insert( make_pair( rid, cont ) );
         m_routeStorageMutex.unlock();
         if ( !insres.second ) {
            // Already present, use it and delete this
            delete cont; 
         }
         // The next call will find it in the map...
         return getStoredRoute( 
            routeID, createTime, routePack, UIN, extraUserinfo,
            validUntil, originLat, originLon, originMapID,
            originItemID, originOffset, destinationLat,
            destinationLon, destinationMapID, destinationItemID,
            destinationOffset, thread );
      } else {
         return NULL;
      }
   } // End else for if pack not null
}


RouteReplyPacket*
ParserThreadGroup::getStoredRoute( const RouteID& routeID,
                                   ParserThreadHandle thread )
{
   RouteReplyPacket* routePack;
   uint32 UIN;
   const char* extraUserinfo;
   uint32 validUntil;
   int32 originLat;
   int32 originLon;
   uint32 originMapID;
   uint32 originItemID;
   uint16 originOffset;
   int32 destinationLat;
   int32 destinationLon;
   uint32 destinationMapID;
   uint32 destinationItemID;
   uint16 destinationOffset;
   PacketContainer* pc = getStoredRoute( routeID.getRouteIDNbr(),
                                         routeID.getCreationTime(),
                                         routePack,
                                         UIN,
                                         extraUserinfo,
                                         validUntil,
                                         originLat,
                                         originLon,
                                         originMapID,
                                         originItemID,
                                         originOffset,
                                         destinationLat,
                                         destinationLon,
                                         destinationMapID,
                                         destinationItemID,
                                         destinationOffset,
                                         thread );
   if ( pc == NULL ) {
      return NULL;
   }
   // I dont know.
   if ( pc->getPacket() == routePack ) {
      routePack = static_cast<RouteReplyPacket*> (
         routePack->getClone( false ) );
      routePack->setStartOffset( originOffset );
      routePack->setEndOffset( destinationOffset );
   }
   delete pc;
   return routePack;
}                           
void ParserThreadGroup::preCacheRoute( const ExpandedRoute* expRoute,
                                       const RouteID& routeID,
                                       LangTypes::language_t lang,
                                       const set<int>& layerIDs,
                                       uint32 extraPixels ) 
try {
      
   mc2dbg << "[ParserThreadGroup] Precache route." << routeID << endl;
   //
   // Determine if we should precache by checking properties;
   //
   // if PRECACHETILE_ENABLE == 1 and queue size < PRECACHETILE_MAX_QUEUE_SIZE
   // then do precache.
   //

   if ( ! StringConvert::
        convert<bool>( Properties::getProperty( "PRECACHETILE_ENABLE",
                                                "false" ) ) ) {
      mc2dbg << "[ParserThreadGroup] No precache of route tiles." << endl;
      return;
   } else if ( (uint32)m_preCacheRouteQueue.getSize() >= 
               Properties::getUint32Property( "PRECACHETILE_MAX_QUEUE_SIZE",
                                              128 )  ) {
      mc2dbg << "[ParserThreadGroup] PreCache queue is too large. "
             << "Will no precache route tiles." << endl;
      return;
   }

   //
   // setup precache route data structure which
   // will be inserted in the queue
   //
   auto_ptr<PreCacheRouteData> data( new PreCacheRouteData );
   data->m_id = routeID;
   data->m_language = lang;
   data->m_layerIDs = layerIDs;
   data->m_extraPixels = extraPixels;

   //
   // Get all route items (turn descriptions)
   // and get roads for each turn description 
   //
   for ( uint32 i = 0; i < expRoute->getNbrExpandedRouteItems(); ++i ) {
      const ExpandedRouteItem* routeItem = 
         expRoute->getExpandedRouteItem( i );
      MC2_ASSERT( routeItem != NULL );
      
      // Get all roads for the turn description.
      for ( uint32 j = 0; j < routeItem->getNbrExpandedRouteRoadItems();
            ++j ) { 
         const ExpandedRouteRoadItem* routeRoadItem = 
            routeItem->getExpandedRouteRoadItem( j );
         MC2_ASSERT( routeRoadItem != NULL );

         auto_ptr<PreCacheRouteData::SubData> 
            subData(new PreCacheRouteData::SubData);
         subData->m_coords.insert( subData->m_coords.begin(),
                                   routeRoadItem->coordsBegin(),
                                   routeRoadItem->coordsEnd() );

         subData->m_speedLimit = routeRoadItem->getPosSpeedLimit();
         data->m_routes.push_back( subData.release() );
      }
   }

   m_preCacheRouteQueue.enqueue( data.release() );

} catch ( const StringConvert::ConvertException& e ) {
   mc2dbg << error << e.what() << endl;
}

uint32 
ParserThreadGroup::getPeriodicTrafficUpdateInterval() const {
   return m_periodicTrafficUpdateInterval;
}

void
ParserThreadGroup::setPeriodicTrafficUpdateInterval( uint32 interval ) {
   m_periodicTrafficUpdateInterval = interval;
}


#ifdef USE_SSL
SSL_CONTEXT*
ParserThreadGroup::getSSLContext() const {
   return m_ctx;
}


void
ParserThreadGroup::setSSLContext( SSL_CONTEXT* ctx ) {
   m_ctx = ctx;
}
#endif

ParserThreadGroup::NewsStatus 
ParserThreadGroup::createNewsItem( ParserThread& thread, 
                                   const NewsKey& key ) try { 

   // fetch news string from some server

   MC2String reply;
   HttpHeader outHeaders;
   uint32 startByte = 0, endByte = 0;
   int fetchStatus = 
      thread.getCWHandler()->
      getURL( PropertyHelper::get<MC2String>("NEWS_SERVER_URL")
              + key.first ,  // url + client type
              "",               // post data
              thread.getPeerIP(),
              0, MAX_UINT32, // from byte, to byte
              key.second,  // language
              NULL, // inHeaders, no headers available here, and no great need
              outHeaders, reply,
              startByte, endByte );

   // set news string and status code for return
   if ( fetchStatus >= 0 && fetchStatus != HttpCode::NOT_FOUND ) {
      m_news[ key ] = News( reply, TimeUtility::getCurrentTime() ); 
      return NewsStatus( reply, fetchStatus );
   }
   
   return NewsStatus( "", fetchStatus );
   
} catch ( const PropertyException& e ) {

   mc2log << error << "[ParserThreadGroup] Property problem: " << e.what()
          << endl;
   mc2log << error << "Check mc2.prop file." << endl;
   return NewsStatus( "", -1 );
}

ParserThreadGroup::NewsStatus 
ParserThreadGroup::getNewsString( ParserThread& thread, 
                                  const NewsKey& key ) {
   ISABSync lock( m_newsMutex );
   NewsMap::iterator newsIt = m_news.find( key );
   // If the last time we got news were more than (default) 30min ago
   // then fetch it again. NEWS_CACHE_TIMEOUT is used from mc2.prop if
   // it exist else 30 min default timeout will be used.
   // ( uint64 so it does not overflow too often. )
   if ( newsIt == m_news.end() ||
        TimeUtility::getCurrentTime() - newsIt->second.m_timestamp
        >= Properties::
        getUint32Property( "NEWS_CACHE_TIMEOUT", 30 ) * 1000*60 ) {

      return createNewsItem( thread, key );
   } 

   return NewsStatus( newsIt->second.m_info, 0 );
}

void
ParserThreadGroup::preCacheDone( ParserThreadHandle preCahce ) {
   // Subclasses that handles threads might use this.
}

uint32
ParserThreadGroup::getEdgenodesDataSize() const {
   ISABSync sync( m_edgenodesMutex );
   return m_edgenodesData.size();
}

uint32
ParserThreadGroup::getUserSessionCacheSize() const {
   ISABSync sync( m_userSessionCacheMutex );
   return m_userSessionCache->size();
}

uint32
ParserThreadGroup::getUserLoginCacheSize() const {
   ISABSync sync( m_userLoginCacheMutex );
   return m_userLoginCache->size();
}

uint32
ParserThreadGroup::getRouteStorageSize() const {
   ISABSync sync( m_routeStorageMutex );
   return m_routeStorage.size();
}

uint32
ParserThreadGroup::getNewsMapSize() const {
   ISABSync sync( m_newsMutex );
   return m_news.size();
}

MC2String
ParserThreadGroup::getCopyright( ParserThread* thread, 
                                 const MC2BoundingBox& bbox, 
                                 LangTypes::language_t lang ) 
{
   return 
      CopyrightHandler( getTileMapFormatDesc( STMFDParams( lang, false ),
                                              thread )->getCopyrightHolder() ).
      getCopyrightString( bbox );
}

CategoryTreeUtils::LocalCategoryTreesPtr
ParserThreadGroup::makeNewLocalCategoryTrees() const {
   CategoryTreeUtils::LocalCategoryTreesPtr newLocalCategoryTrees;

   const char* catFileProperty = "CATEGORY_TREE_FILE";
   const char* catFilename = Properties::getProperty( catFileProperty );
   
   const char* defConfigFileProperty = "CATEGORY_DEFAULT_CONFIG_PATH";
   const char* defConfFilename = 
      Properties::getProperty( defConfigFileProperty, 
                               "Categories/category_tree_default_configuration.xml" );

   const char* regionFileProperty = "CATEGORY_REGION_CONFIG_PATH";
   const char* regionFilename = 
      Properties::getProperty( regionFileProperty,
                               "Categories/category_tree_region_configuration.xml");
   
   if ( catFilename && strlen( catFilename ) > 0 &&
        defConfFilename && strlen( defConfFilename ) > 0 &&
        regionFilename && strlen( regionFilename ) > 0 ) {

      mc2dbg << "[ParserThreadGroup] Loading category trees: " 
             << catFilename << endl;
      
      DebugClock categoryTreeTime;
      // the LocalCategoryTrees constructor will throw if something was wrong
      newLocalCategoryTrees.reset( 
         new CategoryTreeUtils::LocalCategoryTrees( catFilename,
                                                    defConfFilename,
                                                    regionFilename ) );
      mc2dbg << "Category trees loaded in " << categoryTreeTime << endl;
   } else {
      const char* property = ( catFilename == NULL || strlen( catFilename ) == 0 ) ?
         catFileProperty : regionFileProperty;
      throw MC2Exception( "CategoryTree", 
                          "No or empty property "
                          + MC2String( property ) );
   }
   
   return newLocalCategoryTrees;
}

const CategoryTreeUtils::CategoryTreePtr
ParserThreadGroup::getCategoryTree() const {
   return m_localCategoryTrees->getFullTree();
}

const CategoryTreeUtils::LocalCategoryTreesPtr
ParserThreadGroup::getLocalCategoryTrees() const {
   return m_localCategoryTrees;
}
