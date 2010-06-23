/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARSERTHREADGROUP_H
#define PARSERTHREADGROUP_H


#include "config.h"
#include "ISABThread.h"
#include "ParserThreadConfig.h"
#include "ParserThread.h"
#include "MonitorCache.h"
#include "PointerFifoTemplate.h"
#include "CopyrightHolder.h"
#include "ServerTypes.h"
#include "NotCopyable.h"

#include <list>
#include <memory>
#include <boost/shared_ptr.hpp>

// Forward declarations
class ThreadRequestHandler;
class ThreadRequestContainer;
class CellularPhoneModelsElement;
class TopRegionRequest;
class UserItem;
class UserUser;
class ServerRegionIDs;
class UserSessionCache;
class PushPacket;
class PushService;
class ServerTileMapFormatDesc;
class STMFDParams;
class BitBuffer;
class SSL_CONTEXT;
class FOBC;
class ExtService;
class ExternalSearchDesc;
class ExternalSearchDescGenerator;
class ClientSettingStorage; 
class ClientSetting;  
class NamedServerLists;
class CategoriesDataHolder;
class NamedServerList;
struct PreCacheRouteData;
class ExpandedRoute;
class POIImageIdentificationTable;
class SFDHolder;
class SearchHeadingManager;
class TimedOutSocketLogger;

namespace CategoryTreeUtils {
class CategoryTree;
typedef boost::shared_ptr<CategoryTree> CategoryTreePtr;
class LocalCategoryTrees;
typedef boost::shared_ptr<LocalCategoryTrees> LocalCategoryTreesPtr;
}

/*
 * Holds data about a ParserThread's push service.
 */
struct ParserThreadPushData {
   ParserThreadPushData( ParserThreadHandle hand, bool store ) {
      handle = hand;
      storePackets = store;
   }

   ParserThreadHandle handle;
   bool storePackets;
   list< PushPacket* > storedPackets;
};

/*
 *    ISABThreadHandle Vector.
 */
typedef list<ParserThreadHandle> threadHandleStructure_t;

// PushService and resource to ParserThreadPushData multimap
typedef multimap< pair< uint32, SubscriptionResourceNotice >, 
   ParserThreadPushData > PushServiceThreadMap;



/**
 *    Superclass for classes that handles the ParserThreads. Acts as an 
 *    interface to these Threads for other classes.
 *
 */
class ParserThreadGroup : public ISABThreadGroup, private NotCopyable {
   public:
      /** 
       *    Creates a new thread group for the parser threads.
       * 
       *    @param threadGroupName  The name of this thread group.
       */
      ParserThreadGroup( ServerTypes::servertype_t serverType, 
                         const char* threadGroupName = "ParserThreadGroup" );
      
      /**
       *    Destructor, waits for all threads to finish.
       */
      virtual ~ParserThreadGroup();

      /**
       *    Set the request handler that should be used by this thread 
       *    group.
       */
      inline void setRequestHandler(ThreadRequestHandler* handler);

      /**
       *    Method for returning a processed reuqest to a ClientThread.
       *    @param reqCont The request container to return to the
       *                   ParserThread that sent it.
       */
      void putAnswer(ThreadRequestContainer* reqCont);


      /**
       * Add a PushService.
       * 
       * @param thread The ParserThreadHandle of the thread to return
       *               Push data to.
       * @param service The PushService to add. The object is owned by 
       *                this class after the call to this method.
       * @param storePackets If incomming PushPackets for this service
       *                     and thread should be stored until 
       *                     readyForPush() is called.
       * @param lastUpdateTime For all resources the lastUpdateTime is 
       *                       added to this vector.
       */
      void addPushService( ParserThreadHandle thread,
                           PushService* service,
                           bool storePackets,
                           vector<uint32>& lastUpdateTime );


      /**
       * Removes a PushService entierly.
       *
       * @param thread The ParserThreadHandle of the thread to return
       *               Push data to.
       * @param serviceID The ID of the PushService to remove.
       * @return True if serviceID did exist, false if it didn't.
       */
      bool removePushService( ParserThreadHandle thread,
                              uint32 serviceID );


      /**
       * Removes a certain resource from a PushService.
       * The PushService is removed entierly if there remains no 
       * resourceIDs in it.
       *
       * @param thread The ParserThreadHandle of the thread to return
       *               Push data to.
       * @param serviceID The ID of the PushService to remove.
       * @param resource The resource to stop having push for.
       * @return True if resource did exist, false if it didn't.
       */
      bool removePushServiceResource( ParserThreadHandle thread,
                                      uint32 serviceID, 
                                      SubscriptionResource& resource );


      /**
       * Thread is now ready to receice Push for PushService.
       * All stoed PushPackets are sent.
       *
       * @param thread The ParserThreadHandle of the thread.
       * @param serviceID The ID of the PushService.
       * @param resource The resource that the thread is ready to recive
       *                 Push for.
       */
      void readyForPush( ParserThreadHandle thread, uint32 serviceID,
                         SubscriptionResource& resource );


      /**
       * Method for puting received PushPackets.
       * 
       * @param packet The PushPacket.
       * @param serviceID The serviceID of the PushPacket.
       * @param resource The resource of the PushPacket.
       */
      virtual void putPushPacket( PushPacket* packet, uint32 serviceID, 
                                  SubscriptionResource* resource );


      /**
       *    Remove one thread from this thread group.
       *    @param   handle   Handle to the thread that should be 
       *                      removed. The destructor of the thread is
       *                      called when all handles are removed.
       *    @return  True will be returned if the thread is removed,
       *             false upon error.
       */
      bool removeParserThread( ParserThreadHandle handle );

      /**
       *    Method for the parser threads to send a request.
       *    @param reqCont The Request to send.
       */
      void putRequest(ThreadRequestContainer* reqCont);
      
      /**
       *    The ID of the next request.
       *    @return The next RequestID.
       */
      uint16 getNextRequestID();

      /**
       *    Get a pointer to the cache in the server.
       *    @return A pointer to the cache in the server.
       */
      MonitorCache* getCache();

      /**
       *    The MCRequestHandler.
       */
      ThreadRequestHandler* m_handler;

      /**
       *    Structure with the active threads.
       */
      threadHandleStructure_t m_threads;

      /// The type of server
      ServerTypes::servertype_t m_serverType;

      /**
       *    If we should try to end.
       */
      bool m_running;

      /**
       *    The Monitor protecting m_threads.
       */
      ISABMonitor m_monitor;

      /**
       * Get the current CellularPhoneModelsElement.
       * Syncronized function.
       *
       * @param thread Handle to the thread calling this function.
       * @return The current CellularPhoneModelsElement. Caller
       *         must call releaseCacheElement with it.
       */
      CellularPhoneModelsElement* getCurrentCellularPhoneModels( 
         ParserThreadHandle thread );

      /**
       * Free a locked element.
       * @param element is the locked element to release.
       */
      inline void releaseCacheElement( CacheElement* element );


      /**
       * Tries to contact UserModule and get the user with the given 
       * UIN. Notice that the user is created inside this
       * method and returned via a outparameter. 
       * m_userCacheMutex method.
       *
       * @param UIN        The UIN of the user to get.
       * @param userItem   Set to the user with the logonID or NULL if 
       *                   no such user.
       * @param thread Handle to the thread calling this function.
       * @param useCache   If the cache should be used, if true then
       *                   the UserItem must be returned by calling
       *                   releaseUserItem, if false the returned
       *                   UserItem must be delered by user. Default
       *                   false.
       * @param wipeFromCache If to clean all caches in server and module
       *                      from this user.
       * @return True if all communication with UserModule was ok,
       *         false if communication/database error.
       */
      bool getUser( uint32 UIN, UserItem*& userItem, 
                    ParserThreadHandle thread,
                    bool useCache = false,
                    bool wipeFromCache = false );


      /**
       * Send all chenges of given user to the user module.
       * m_userCacheMutex method.
       *
       * @param user The user with changes.
       * @param changerUser The user that does the change, NULL if not
       *        known.
       * @param thread Handle to the thread calling this function.
       * @return True if changes are added ok, false if not.
       */
      bool changeUser( UserUser* user, const UserUser* changerUser,
                       ParserThreadHandle thread );


      /**
       * Releases a UserItem locked in a call to getUser with useCache
       * enabled.
       * m_userCacheMutex method.
       *
       * @param userItem The UserItem that should be released from the 
       *                 cache.
       */
      void releaseUserItem( UserItem* userItem );


      /**
       * Remove cached user.
       *
       * @param UIN The Id of the User to remove from cache.
       */
      void removeUserFromCache( uint32 UIN );


      /**
       * Get the TopRegionRequest.
       * Syncronized function.
       *
       * @param thread Handle to the thread calling this function.
       * @return The current TopRegionRequest, may be NULL if modules
       *         aren't working.
       */
      const TopRegionRequest* getTopRegionRequest( 
         ParserThreadHandle thread );


      /**
       * The PushServiceThreadMap.
       */
      PushServiceThreadMap m_pushServiceThreadMap;


      /**
       * The mutex protecting m_pushServiceThreadMap.
       */
      ISABMutex m_pushServiceMutex;


      /**
       * Get the RegionIDs.
       */
      const ServerRegionIDs* getRegionIDs() const;


      /**
       * Get the edgenodes for a map.
       *
       * @param mapID The map to get edge nodes for.
       * @param overMapID The overview map to translate edgenodeId to. Set
       *                  to mapID to avoid translation.
       * @param externalNodes Nodes are added to this vector.
       * @param thread The thread calling this function.
       * @return StringTable::OK if all is ok, error code if not.
       */
      StringTable::stringCode getEdgeNodesFor( 
         uint32 mapID, uint32 overMapID, vector<uint32>& externalNodes,
         ParserThreadHandle thread );

      /**
       * Get the edgenodes for a map.
       *
       * @param mapIDs a vector of map ids to get nodes for
       * @param overMapID The overview map to translate edgenodeId to. Set
       *                  to mapID to avoid translation.
       * @param externalNodes Nodes are added to this vector.
       * @param thread The thread calling this function.
       * @return StringTable::OK if all is ok, error code if not.
       */
      StringTable::stringCode getEdgeNodesFor( const vector<uint32>& mapIDs,
                                               uint32 overMapID, 
                                               vector<uint32>& externalNodes,
                                               ParserThreadHandle thread );

      /**
       * Authenticate a user. Uses user login cache.
       *
       * @param userName The users logonID.
       * @param userPasswd The users password.
       * @param checkExpired If to check if user is expired.
       * @param thread The thread calling this function.
       * @return The UIN of the user or 0 if invalid login, MAX_UIN32
       *         if communication problem and MAX_UINT32 -1 if expired
       *         user.
       */
      uint32 authenticateUser( const char* userName, 
                               const char* userPasswd,
                               bool checkExpired,
                               ParserThreadHandle thread );


      /**
       * Authenticate a user using a session. Uses user session cache.
       *
       * @param sessionID The ID of the session.
       * @param sessionKey The key of the session.
       * @param checkExpired If to check if user is expired.
       * @param thread The thread calling this function.
       * @return The UIN of the user or 0 if invalid login, MAX_UIN32 -2
       *         if communication problem, MAX_UINT32 -1 if expired
       *         user and MAX_UINT32 if expired session.
       */
      uint32 authenticateUserSession( const char* sessionID, 
                                      const char* sessionKey,
                                      bool checkExpired,
                                      ParserThreadHandle thread );


      /**
       * Tries to contact UserModule and get the user associated with
       * a session.
       * Note that the user is created inside this method and returned 
       * via a outparameter. 
       * Uses user session cache.
       *
       * @param sessionID  The session ID
       * @param sessionKey The session key
       * @param userItem   Set to the user fot the session or NULL if 
       *                   no such user.
       * @param useCache   If the cache should be used, if true then
       *                   the UserItem must be returned by calling
       *                   releaseUserItem, if false the returned
       *                   UserItem must be deleted by user.
       * @param thread The thread calling this function.
       * @param wipeFromCache If to clean all caches in server and module
       *                      from this user.
       * @return True if all communication with UserModule was ok,
       *         false if communication/database error.
       */
      bool getUserBySession( const char* sessionID, const char* sessionKey,
                             UserItem*& userItem, bool useCache,
                             bool wipeFromCache,
                             ParserThreadHandle thread );

      /**
       *   Returns a ServerTileMapFormatDesc that is shared
       *   between all threads.
       *   @param settings The parameters that are needed to create the
       *                   ServerTileMapFormatDesc, currently the lang.
       *   @param thread for sending request
       */
      const ServerTileMapFormatDesc*
      getTileMapFormatDesc( const STMFDParams& settings,
                            ParserThreadHandle thread );

      /**
       *   Returns a new BitBuffer containing the ServerTileMapFormatDesc.
       *   The BitBuffer has to be deleted.
       *   @param settings The parameters that are needed to create the
       *                   ServerTileMapFormatDesc, currently the lang.
       *   @param thread for sending request
       */
      BitBuffer* newTileMapFormatDescBuffer( const STMFDParams& settings,
                                             ParserThreadHandle thread );

      /**
       *   Returns a new BitBuffer containing the CRC for the
       *   ServerTileMapFormatDesc which would be obtained with
       *   getTileMapFormatDescBuffer or getTileMapFormatDesc.
       *   @param settings The parameters that are needed to create the
       *                   ServerTileMapFormatDesc, currently the lang.
       *   @param thread for sending requests
       */
      BitBuffer* newTileMapFormatDescCRCBuffer( const STMFDParams& settings,
                                                ParserThreadHandle thread );


      /**
       * Gets a stored route thougth routeID and createTime.
       * 
       * @param routeID The routeID to search for.
       * @param createTime The time of the route to search for.
       * @param routePack Set to the RouteReplyPacket if a stored route
       *        was found.
       * @param UIN Set to the UIN of the stored route if a stored route
       *        was found.
       * @param extraUserinfo Set to the extraUserinfo of the 
       *        stored route if a stored route was found.
       * @param validUntil Set to the validUntil of the stored route 
       *        if a stored route was found.
       * @param thread The thread calling this function.
       * @return The PacketContainer with the 
       *         RouteStorageGetRouteReplyPacket if found else NULL.
       */
      PacketContainer* getStoredRoute( uint32 routeID,
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
                                       ParserThreadHandle thread );


      /**
       * Returns a routeReplyPacket containing the stored
       * route with the supplied id or NULL if not found.
       *
       * @param routeID The id of the route.
       * @param thread The thread calling this function.
       * @return A new RouteReplyPacket which should be deleted
       *         by the caller or NULL if failure.
       */
      RouteReplyPacket* getStoredRoute( const RouteID& routeID,
                                        ParserThreadHandle thread );

   
      /**
       * Adds a route id to be expanded to tilemap params
       * @param expRoute the expanded route
       * @param routeID id of the route
       * @param lang language for text in the route
       * @param layerIDs which ids to use
       * @param extraPixels size of the window in pixels
       */
      void preCacheRoute( const ExpandedRoute* expRoute,
                          const RouteID& routeID,
                          LangTypes::language_t lang,
                          const set<int>& layerIDs,
                          uint32 extraPixels = 250 );

      /**
       * Set the Periodic traffic info interval time sent to clients.
       */
      void setPeriodicTrafficUpdateInterval( uint32 interval );


      /**
       * Get the Periodic traffic info interval time sent to clients.
       */
      uint32 getPeriodicTrafficUpdateInterval() const;


#ifdef USE_SSL
      /**
       * Return the SSL Context object.
       */
      SSL_CONTEXT* getSSLContext() const;


      /**
       * Set the SSL Context object.
       */
      void setSSLContext( SSL_CONTEXT* ctx );
#endif


      /**
       * Get the FOBC.
       */
      FOBC* getFOBC();


      /**
       * Parse the contents of a serverlist file and add the specified
       * lists to the NamedServerLists object.
       *
       * @param namedServerLists The contents of a serverlist file.
       */
      void setNamedServerLists(const char* namedServerList);


      /**
       * Fetch the serverlist associated with a certain name and type.
       * The name is associated with certain client types in
       * navclientsettings.  The type is associated with a kind of
       * server, such as NavigatorServer, XML-server or
       * http-NavigatorServer.
       *
       * @param listName The name of the list.
       * @param type     The type of the list. 
       * @return The server list, or empty string if no list is found. 
       */
      const MC2String& getNamedServerList( const MC2String& listName, 
                                           const MC2String& type ) const;

      /**
       * Fetch the serverlist associated with a certain name and type.
       * The name is associated with certain client types in
       * navclientsettings.  The type is associated with a kind of
       * server, such as NavigatorServer, XML-server or
       * http-NavigatorServer.
       *
       * @param listName The name of the list.
       * @param type     The type of the list. 
       * @return The server list, or NULL if no list is found. 
       */
      const NamedServerList* getServerList( const MC2String& listName, 
                                            const MC2String& type ) const;


      /**
       * Fetch the CRC of the serverlist associated with a certain name
       * and type.  The name is associated with certain client types in
       * navclientsettings.  The type is associated with a kind of
       * server, such as NavigatorServer, XML-server or
       * http-NavigatorServer.
       *
       * @param listName The name of the list.
       * @param type     The type of the list. 
       * @return The CRC of the server list, or 0 if no list is found. 
       */
      uint32 getNamedServerListCRC( const MC2String& listName, 
                                    const MC2String& type ) const;


      /**
       * Replace a named serverlist. If the name-type pair was not
       * present already the serverlist is added.
       *
       * @name name       The name of the serverlist.
       * @name type       The type of the serverlist.
       * @name serverlist The server list.
       * @return true if changed ok, false if not.
       */
      bool replaceNamedServerList( const MC2String& name,
                                   const MC2String& type,
                                   const MC2String& serverlist );


      /**
       * Set ClientSettingStorage.
       */
      void setClientSettingStorage( const char* clientSettings );


      /**
       * Get ClientSettingsStorage.
       */
      const ClientSettingStorage* getClientSettingStorage() const;


      /**
       * Get the clientsetting for a clientType.
       */
      const ClientSetting* getSetting( 
         const char* clientType, const char* clientOptions ) const;


      /**
       * Get the clientsetting for a clientType.
       */
      const ClientSetting* getSetting( 
         const MC2String& clientType, 
         const MC2String& clientOptions ) const;


      /**
       * Get the amount of seconds for create wf user using a specific
       * ClientSetting. May return MAX_INT32.
       * Time is added to endTime unless a explicit time then endTime
       * is set to that.
       *
       * @param setting The ClientSetting to use.
       * @param endTime Set to the create time, used as "now".
       */
      void getCreateWFTime( const ClientSetting* setting,
                            uint32& endTime ) const ;


      /**
       * Set the categories files for languages.
       * Syncronized function.
       *
       * @param categoriesFiles The directory with category files.
       */
      void setCategories( const char* categoriesFiles );


      /**
       * Get the categories files directory.
       *
       * @return The directory with category files.
       */
      const char* getCategorieFiles() const;


   /**
    * Get the CategoriesDataHolder.
    */
   CategoriesDataHolder* getCategories();

   /// @return category tree
   const CategoryTreeUtils::CategoryTreePtr getCategoryTree() const;

   /**
    * @return container for the location specific category trees.
    */
   const CategoryTreeUtils::LocalCategoryTreesPtr 
      getLocalCategoryTrees() const;

   /**
    * Get the server instance string. New for each start.
    */
   const MC2String& getServerInstanceStr() const;
   
   /// key type for requesting news
   typedef pair<MC2String, LangTypes::language_t> NewsKey;
   /// return type from getNewsString
   typedef pair<MC2String, int32> NewsStatus; 

   /**
    * @param thread working thread
    * @param key the news key
    * @return news string and status code.
    */
   NewsStatus getNewsString( ParserThread& thread, const NewsKey& key );

      /**
       * The PreCacheTileMapHandler is done, no more requestes will be sent.
       */
      virtual void preCacheDone( ParserThreadHandle preCahce );

      /**
       * The the size of edgenodesData.
       */
      uint32 getEdgenodesDataSize() const;

      /**
       * The the size of UserSessionCache.
       */
      uint32 getUserSessionCacheSize() const;

      /**
       * The the size of UserLoginCache.
       */
      uint32 getUserLoginCacheSize() const;

      /**
       * The the size of routeStorage.
       */
      uint32 getRouteStorageSize() const;

      /**
       * The the size of NewsMap.
       */
      uint32 getNewsMapSize() const;
   /**
    *
    * @param thread Working thread.
    * @param bbox Bounding box for copyright area.
    * @param language Requested language for the copyright strings.
    * @return copyright string for the bounding box.
    */
   MC2String getCopyright( ParserThread* thread, 
                           const MC2BoundingBox& box, 
                           LangTypes::language_t lang );
   /// @return poi image file
   MC2String getPOIImage( const MC2String& imageCode,
                          ParserThreadHandle thread );

   inline const POIImageIdentificationTable& getPOIImageIdTable() const;

   /**
    * Get the Holder for precached SFD files.
    */
   SFDHolder* getSFDHolder();

   /**
    * Get the Holder for precached SFD files.
    */
   const SFDHolder* getSFDHolder() const;

   /// @return SearchHeadingManager
   SearchHeadingManager& getSearchHeadingManager() {
      return *m_searchHeadingManager;
   }

   /**
    * Returns the logger handler for timed out sockets.
    */
   TimedOutSocketLogger* getTimedOutSocketLogger() { 
      return m_timedOutSocketLogger; 
   }


private:

   /** 
    * Creates a news item in the news map.
    * Not thread safe!, should only be used in getNewsString
    */
   NewsStatus createNewsItem( ParserThread& thread, const NewsKey& key );

   /**
    *   Updates the user rights caches per region and map
    *   for the user.
    *   @param user   User to update.
    *   @param thread The thread calling this function.
    *   @return true if successful.
    */
   bool updateRightsCache( UserUser* user,
                           ParserThreadHandle thread );
   
   /// Adds extra rights to user
   /// @param user User to update
   /// @param now Time for now
   /// @param thread The thread calling this function.
   void addExtraRights( UserUser* user, uint32 now, ParserThreadHandle thread );
      
   /**
    * The CellularPhoneModels.
    */
   CellularPhoneModelsElement* m_phoneModels;

   /**
    * The time of the last update of m_phoneModels.
    */
   time_t m_lastPhoneModelsUpdate;

      /**
       * The maximum age of m_phoneModels in seconds. 
       * Currently 1800s = ½h.
       */
      static const time_t MAX_PHONEMODELS_AGE;

      /**
       * The mutex used to lock getCurrentCellularPhoneModels.
       */
      ISABMutex m_phoneModelsMutex;


      /**
       * The TopRegionRequest.
       */
      TopRegionRequest* m_topRegionRequest;


      /**
       * The mutex used to lock getTopRegionRequest.
       */
      ISABMutex m_topRegionsMutex;


      /**
       * The maximum age of cached UserItems in seconds. 
       * Currently 180s = 3min.
       */
      static const uint32 MAX_USER_AGE;


      /**
       * The mutex used to lock user cache handling.
       */
      ISABMutex m_userCacheMutex;


      /**
       * The region ids.
       */
      ServerRegionIDs* m_regionIDs;


      /**
       * The mutex used to lock edge nodes.
       */
      ISABMutex m_edgenodesMutex;


      typedef map< pair< uint32, uint32 >, vector< uint32 > > edgenodesMap;
      /**
       * The cached edge nodes data.
       */
      edgenodesMap m_edgenodesData;


      /**
       * The maximum age of cached sessions in seconds. 
       * Currently 60s = 1min.
       */
      static const uint32 MAX_SESSION_AGE;


      /**
       * The cached user sessions.
       */
      UserSessionCache* m_userSessionCache;


      /**
       * The mutex used to lock user session cache.
       */
      ISABMutex m_userSessionCacheMutex;


      /**
       * The cached user logins.
       */
      UserSessionCache* m_userLoginCache;


      /**
       * The mutex used to lock user login cache.
       */
      ISABMutex m_userLoginCacheMutex;

      /// Class containing the stuff that is cached for TMFD:s.
      class TileMapFormatDescData {
        public:
         /** 
          * Creates the buffers needed.
          * @param settings for tmfd
          * @param copyrights 
          */
         TileMapFormatDescData( const STMFDParams& settings,
                                const CopyrightHolder& holder );
         /// Destructor.
         ~TileMapFormatDescData();
         /// Returns a copy of the buffer that contains the tfmd.
         BitBuffer* newTMFDBuffer() const;
         /// Returns a copy of the buffer containing the CRC.
         BitBuffer* newCRCBuffer() const;
         /// Returns the ServerTileMapFormatDesc.
         const ServerTileMapFormatDesc* getTMFD() const;
      private:
         BitBuffer* m_tmfdBuffer;
         /// Position of the timestamp in the m_tmfdBuffer
         uint32 m_timeStampPos;
         BitBuffer* m_crcBuffer;
         ServerTileMapFormatDesc* m_tmfd;
      };

      /// Type of map to put TileMapFormatDescData into
      typedef map<STMFDParams, TileMapFormatDescData*> tfmdmap_t;

      /// The cache of TileMapFormatDescData
      tfmdmap_t m_tileMapFormatDescMap;

      /**
       *   Returns the iterator to the TileMapFormatDescData
       *   for the supplied settings. Inserts into cache if necessary.
       *   The mutex must be locked when calling the function.
       * @param settings for the tile map format description
       * @param thread for sending requests
       */
      tfmdmap_t::const_iterator getTMFDIterator( const STMFDParams& settings,
                                                 ParserThreadHandle thread );
      
      /**
       *   The mutex that is used when using the TileMapFormatDesc
       *   cache.
       */
      ISABMutex m_tileMapFormatDescMutex;


      /// Type of storage for the routes.
      typedef map<RouteID, PacketContainer*> routeStorage_t;
      
      /**
       *    Testing to store the routes in a map.
       */
      routeStorage_t m_routeStorage;

      /**
       *   The mutex that is used for route storage.
       */
      ISABMutex m_routeStorageMutex;

#ifdef USE_SSL
      /// The ssl context.
      SSL_CONTEXT* m_ctx;
#endif


      /// The FOBC.
      FOBC* m_fobc;

      /**
       * The holder of named server lists.
       */
      NamedServerLists* m_namedServerLists;

      /**
       * The mutex used to lock NamedServerLists.
       */
      ISABMutex m_serverListMutex;

      /**
       * The storage of client settings.
       */
      ClientSettingStorage* m_clientSettingStorage;

      /**
       * The mutex used to lock NavClientSettingStorage.
       */
      ISABMutex m_clientSettingMutex;


      /**
       * The directory with categories.
       */
      const char* m_categories;

      /**
       * The server instance string.
       */
      MC2String m_serverInstanceStr;

   /**
    * news info + timestamp when the info
    * was last updated.
    */
   struct News {
      News():m_timestamp( 0 ) { }

      News( const MC2String& info,
            uint32 timestamp ):
         m_info( info ),
         m_timestamp( timestamp ) { }

      MC2String m_info; //< news string
      uint32 m_timestamp; //< last update time
   };

   /// maps news to a key
   typedef map< NewsKey, News > NewsMap; 

   ISABMutex m_newsMutex; //< lock for m_news

   NewsMap m_news; //< news mapping

   /**
    * Get copyright strings for a language.
    * @param thread request thread
    * @param language for the copyright strings
    * @return translated copyright strings with bounding boxes
    */
   CopyrightHolder getCopyrights( ParserThreadHandle thread,
                                  const LangType& language );

protected:
   /// holds job queue for precache route tile handler
   PointerFifoTemplate<PreCacheRouteData> m_preCacheRouteQueue;
   ParserThreadHandle m_preCacheTileMapHandler;

   /**
    * The CategoriesDataHolder.
    */
   CategoriesDataHolder* m_categoriesData;

   /**
    * InterfaceIO that logs activity on timed out sockets.
    */
   TimedOutSocketLogger* m_timedOutSocketLogger;

private:
   /// The Periodic traffic info interval time sent to clients.
   uint32 m_periodicTrafficUpdateInterval;

   /**
    * Create a new LocalCategoryTrees. 
    * @throw MC2Exception if there is a problem with the files.
    */
   CategoryTreeUtils::LocalCategoryTreesPtr makeNewLocalCategoryTrees() const;

   /// The container for the location specific category trees
   CategoryTreeUtils::LocalCategoryTreesPtr m_localCategoryTrees;

   /// holds the special poi images   
   auto_ptr<POIImageIdentificationTable> m_poiImageTable;

   /**
    * The holder for premade tile caches.
    */
   auto_ptr<SFDHolder> m_SFDHolder;

private:

   /// Search Descriptor containing headings and crc for headings.
   auto_ptr< SearchHeadingManager > m_searchHeadingManager;
};


// =======================================================================
//                                     Implementation of inlined methods =
inline const POIImageIdentificationTable& 
ParserThreadGroup::getPOIImageIdTable() const {
   return *m_poiImageTable;
}

inline void 
ParserThreadGroup::setRequestHandler(ThreadRequestHandler* handler) 
{
   m_handler = handler;
}


inline void 
ParserThreadGroup::releaseCacheElement( CacheElement* element ) {
   getCache()->releaseCacheElement( element );
}


inline const ServerRegionIDs* 
ParserThreadGroup::getRegionIDs() const {
   return m_regionIDs;
}


inline FOBC*
ParserThreadGroup::getFOBC() {
   return m_fobc;
}


inline void 
ParserThreadGroup::setCategories( const char* categoriesFiles ) {
   m_categories = categoriesFiles;
}


inline const char*
ParserThreadGroup::getCategorieFiles() const {
   return m_categories;
}


inline CategoriesDataHolder* 
ParserThreadGroup::getCategories() {
   return m_categoriesData;
}

inline const ClientSettingStorage* 
ParserThreadGroup::getClientSettingStorage() const {
   return m_clientSettingStorage;
}


inline const ClientSetting* 
ParserThreadGroup::getSetting( 
   const MC2String& clientType, 
   const MC2String& clientOptions ) const
{
   return getSetting( clientType.c_str(), clientOptions.c_str() );
}


inline const MC2String& 
ParserThreadGroup::getServerInstanceStr() const {
   return m_serverInstanceStr;
}

inline SFDHolder*
ParserThreadGroup::getSFDHolder() {
   return m_SFDHolder.get();
}

inline const SFDHolder*
ParserThreadGroup::getSFDHolder() const {
   return m_SFDHolder.get();
}

#endif // PARSERTHREADGROUP_H

