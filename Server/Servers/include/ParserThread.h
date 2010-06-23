/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARSERTHREAD_H
#define PARSERTHREAD_H

#include "config.h"

#include "ISABThread.h"
#include "MapRequester.h"
#include "SortThreadRequestContainerByRequestID.h"
#include "SortSendRequestHolderByRequestID.h"
#include "ParserThreadConfig.h"
#include "ThreadRequestContainer.h"
#include "SubscriptionResource.h"
#include "RouteID.h"
#include "WFSubscriptionConstants.h"
#include "MC2Coordinate.h"
#include "UserEnums.h"
#include "RequestData.h"
#include "ItemTypes.h"
#include "IPnPort.h"
#include "ScopedArray.h"
#include "OperationTypes.h"
#include "TimeUtility.h"

#include <memory>

// Forwards
class ExpandRouteRequest;
class ThreadRequestContainer;
class RouteRequest;
class UserElement;
class UserUser;
class UserCellular;
class UserItem;
class RouteReplyPacket;
class MC2Coordinate;
class PushPacket;
class DataBuffer;
class MC2SimpleString;
class UserLicenceKey;
class TopRegionRequest;
class MC2BoundingBox;
class UserRight;
class ParserTokenHandler;
class ParserTileHandler;
class ParserMapHandler;
class SearchResultRequest;
class ParserExternalAuth;
class ParserUserHandler;
class ExternalSearchDesc;
class ExtService;
class TileMapQuery;
class ParserActivationHandler;
class ClientSetting;
class HttpInterfaceRequest;
class URLFetcher;
class ParserCWHandler;
class ParserRouteHandler;
class PacketContainer;
#ifndef SEARCH_PARSER_HANDLER_H
// Ugly fix for unit test to compile
class SearchParserHandler;
#endif
class ParserPoiReviewHandler;
class RouteStorageGetRouteReplyPacket;
class ParserPosPush;
class ParserDebitHandler;
class RouteAllowedMap;
class PurchaseOptions;
class ParserAppStoreHandler;
class InterfaceRequestData;


/**
 *    Abstract superclass for the parser threads in one server. The 
 *    subclasses of this class should parse the data from the client
 *    (of any kind), create a request, insert that into the server,
 *    wait for the answer, format outdata and send that back to the
 *    client.
 *
 */
class ParserThread : public ISABThread, public MapRequester {
   public:
      /**
       *    Creates a new ParserThread.
       *    @param group The ParserThreadGroup that this ParserThread
       *                 is part of.
       *    @param name The name of the thread.
       */
      ParserThread( ParserThreadGroupHandle group,
                    const char* name = "ParserThread" );


      /**
       *    Destructor deletes all local data. {\it {\bf NB!}
       *    Do not call this, use terminate().}
       */
      virtual ~ParserThread();

      /**
       *    This is a true virtual method that will do all the 
       *    actual parsing of the client-requests in the subclasses.
       */
      virtual void run() = 0;

      /**
       *    Sets everything that needs to be set in the reqCont and
       *    calls the groups putRequest(). {\tt Monitor method.}
       *    @param reqCont The Request to send.
       */
      void putRequest( ThreadRequestContainer* reqCont );

      /**
       *    Wrapper for putRequest( ThreadRequestContainer* ) this
       *    handles the ThreadRequestContainer.
       *    @param reqCont The Request to send.
       */
      void putRequest( Request* req );

      /**
       *    Puts multiple requests at the same time and returns when
       *    all are done.
       *    @param reqs The Requests to send.
       */
      void putRequests( const vector<RequestWithStatus*>& reqs );

      /**
       *    Returns true if the client uses latin-1 and not utf-8.
       *
       *    @return true if client uses latin-1, false if utf-8.
       */
      inline bool clientUsesLatin1() const;


      /**
       * Set if client uses iso-8859-1 and not utf-8. 
       *
       * @param clientUsesLatin1 If client uses iso-8859-1.
       */
      void setClientUsesLatin1( bool clientUsesLatin1 );


      /**
       * Wrapper for putRequest( Request* ) this
       * handles the PacketContainer by using a SinglePacketRequest.
       *
       * @param cont The PacketContainer to send.
       * @return The answer PacketContainer, may be NULL.
       */
      PacketContainer* putRequest( PacketContainer* cont );


      /**
       * Wrapper for putRequest( PacketContainer* ) this
       * handles the Packet and module_t by using a PacketContainer.
       *
       * @param pack The Packet to send.
       * @param module Where to send packet.
       * @return The answer PacketContainer, may be NULL.
       */
      PacketContainer* putRequest( Packet* pack, moduletype_t module );

      
      /**
       *    Returns a processed request. {\tt Monitor method.}
       *    @param reqCont The processed Request.
       */
      void putAnswer(ThreadRequestContainer* reqCont);


      /**
       * Sends a number of packets and returns the replies.
       *
       * @param reqs The packets to send, is cleared as packets are
       *             consumed.
       * @param reps The reply packets, is the same size as reqs was.
       */
      void putRequest( vector< PacketContainer* >& reqs, 
                       vector< PacketContainer* >& reps );

      /**
       * Send a number of packets and returns the replies.
       * 
       * @param packets When the function is called the vector should
       *                contain all the packets to send. When the
       *                function returns it will cotain all the
       *                returned packages.
       */
      void putRequest( vector< PacketContainer* >& packets );

      /**
       * Send a Request and return control immediately, call waitForRequest
       * to wait for the request to finish.
       *
       * @param reqCont The Request to send.
       */
      void sendRequest( ThreadRequestContainer* reqCont );


      /**
       * Send a Request and return control immediately, call waitForRequest
       * to wait for the request to finish.
       *
       * @param req The Request to send.
       */
      void sendRequest( Request* req );

      /**
       * Send a Request and return control immediately, call waitForRequest
       * to wait for the request to finish.
       *
       * @param cont The Request to send.
       */
      void sendRequest( PacketContainer* cont );

      /**
       * Send a Request and return control immediately, call waitForRequest
       * to wait for the request to finish.
       *
       * @param pack The Request to send.
       * @param module The module to send the packet too.
       */
      void sendRequest( Packet* pack, moduletype_t module );

      /**
       * Wait for a prevoiusly sent request in sendRequest.
       *
       * @param requestID The ID of the Request to wait for.
       */
      void waitForRequest( uint16 requestID );

      /**
       * Wait for a prevoiusly sent request in sendRequest.
       *
       * @param reqCont The Request to wait for.
       */
      void waitForRequest( const ThreadRequestContainer* reqCont );

      /**
       * Method for puting received PushPackets.
       * Implemented as dummy.
       * 
       * @param packet The PushPacket. Should NOT be deleted.
       * @param serviceID The serviceID of the PushPacket.
       * @param resource The resource of the PushPacket.
       */
      virtual void putPushPacket( PushPacket* packet, uint32 serviceID, 
                                  SubscriptionResource& resource );

      /**
       *    Get the IP-address of the computer where the client that are
       *    connected to this thread executes.
       *    @return  The IP-address of the peer computer, where the client
       *             executes. 0 or MAX_UINT32 is returned upon failure.
       */
      uint32 getPeerIP();


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
                                       uint16& destinationOffset );

      /**
       *   Returns a routeReplyPacket containing the stored
       *   route with the supplied id or NULL if not found.
       *   @param routeID The id of the route.
       *   @return A new RouteReplyPacket which should be deleted
       *           by the caller or NULL if failure.
       */
      RouteReplyPacket* getStoredRoute(const RouteID& routeID);
      

      /**
       * Stores a route.
       * If m_isCheckUser is set then this method returns without 
       * doing anything.
       *
       * @param req The RouteRequest with the route to store.
       * @param UIN The user of the request.
       * @param extraUserinfo Extra data about the user storing the route.
       * @param urmask The rights mask used for the route.
       * @param forceRouteID If to force a specific routeID, default is not
       *        to use forced routeIDs.
       * @param p Set to a RouteStorageGetRouteReplyPacket if not NULL.
       * @return True if stored route ok, false if not.
       */
      bool storeRoute( RouteRequest* req,
                       uint32 UIN,
                       const char* const extraUserinfo,
                       UserEnums::URType urmask = UserEnums::defaultMask,
                       const RouteID& forceRouteID = RouteID(),
                       RouteStorageGetRouteReplyPacket** p = NULL );


      /**
       * Updates validUntil of a route.
       * If m_isCheckUser is set then this method returns OK without 
       * doing anything.
       *
       * @param routeID The routeID to update.
       * @param createTime The time of the route to update.
       * @param validUntil The new validUntil value to set.
       * @return StringTable::OK if updated ok else error code.
       */
      StringTable::stringCode updateStoredRoute( uint32 routeID, 
                                                 uint32 createTime, 
                                                 uint32 validUntil );


      /**
       * Gets a stored route thougth routeID and createTime and expands it too.
       * 
       * @param routeID The routeID to search for.
       * @param createTime The time of the route to search for.
       * @param expandType The route expand type.
       * @param lang The prefered language.
       * @param abbreviate If roadnames should be abbreviated.
       * @param landmarks Should landmarks be added to the result.
       * @param removeAheadIfDiff Should Aheads that are for road name 
       *                          changes be removed.
       * @param nameChangeAsWP Should road name change force a waypoint.
       * @param routePack Set to the RouteReplyPacket if a stored route
       *        was found.
       * @param UIN Set to the UIN of the stored route if a stored route
       *        was found.
       * @param extraUserinfo Set to the extraUserinfo of the 
       *        stored route if a stored route was found.
       * @param validUntil Set to the validUntil of the stored route 
       *        if a stored route was found.
       * @param expReq The ExpandRouteRequest, might be NULL.
       * @param expandRouteCont The ExpandRouteReplyPacket, might be NULL.
       * @return The PacketContainer with the 
       *         RouteStorageGetRouteReplyPacket if found else NULL.
       */
      PacketContainer* getStoredRouteAndExpand( 
         uint32 routeID, uint32 createTime,
         uint32 expandType, StringTable::languageCode lang,
         bool abbreviate, bool landmarks, bool removeAheadIfDiff,
         bool nameChangeAsWP,
         RouteReplyPacket*& routePack,
         uint32& UIN,
         const char*& extraUserinfo,
         uint32& validUntil,
         int32& originLat, int32& originLon,
         uint32& originMapID, uint32& originItemID, 
         uint16& originOffset,
         int32& destinationLat, int32& destinationLon,
         uint32& destinationMapID, uint32& destinationItemID,
         uint16& destinationOffset,
         ExpandRouteRequest*& expReq,
         PacketContainer*& expandRouteCont );

      /**
       * Gets a stored route thougth routeID and expands it too.
       * 
       * @param routeID The routeID to search for.
       * @param expandType The route expand type.
       * @param lang The prefered language.
       * @param abbreviate If roadnames should be abbreviated.
       * @param landmarks Should landmarks be added to the result.
       * @param removeAheadIfDiff Should Aheads that are for road name 
       *                          changes be removed.
       * @param nameChangeAsWP Should road name change force a waypoint.
       * @param expReq The ExpandRouteRequest, might be NULL.
       * @param expandRouteCont The ExpandRouteReplyPacket, might be NULL.
       * @return A new RouteReplyPacket which should be deleted
       *           by the caller or NULL if failure.
       */
      RouteReplyPacket* getStoredRouteAndExpand( 
         RouteID routeID, uint32 expandType, 
         StringTable::languageCode lang, bool abbreviate, bool landmarks,
         bool removeAheadIfDiff, bool nameChangeAsWP,
         ExpandRouteRequest*& expReq, PacketContainer*& expandRouteCont );


      /**
       *    Tries to contact UserModule and get the user associated with
       *    a session.
       *    Note that the user is created inside this method and returned 
       *    via a outparameter. 
       *
       *    @param sessionID  The session ID
       *    @param sessionKey The session key
       *    @param userItem   Set to the user fot the session or NULL if 
       *                      no such user.
       *    @param useCache   If the cache should be used, if true then
       *                      the UserItem must be returned by calling
       *                      releaseUserItem, if false the returned
       *                      UserItem must be deleted by user. Default
       *                      false.
       *    @param wipeFromCache If to clean all caches in server and 
       *                         module from this user.
       *    @return True if all communication with UserModule was ok,
       *            false if communication/database error.
       */
      bool getUserBySession( const char* sessionID, const char* sessionKey,
                             UserItem*& userItem, bool useCache = false,
                             bool wipeFromCache = false );



      /**
       * Check if user has access to a top region id.
       *
       * @param regionID The top region id of the country.
       * @param user The user to check access for.
       * @param urmask The user right type mask to match right with.
       * @param checkEvenIfNotUsingRights If to bypass the client type setting
       *        usesRights that normally makes this method return true.
       * @return True if user has access false if not.
       */
      bool checkUserRegionAccess( 
         uint32 regionID, const UserUser* const user, 
         UserEnums::URType urmask = UserEnums::defaultMask,
         bool checkEvenIfNotUsingRights = false );


      /**
       * Get the allowed maps for a user.
       *
       * @param user The user to get accessable maps for.
       * @param maps Set to a stl-map with the allowed maps, with blocked 
       *             segments, or NULL if access to all maps.
       * @param urmask The user right type mask to match right with.
       * @return True if all is well, false if not.
       */
      bool getMapIdsForUserRegionAccess( 
         const UserUser* const user,
         RouteAllowedMap*& maps, 
         UserEnums::URType urmask = UserEnums::defaultMask );


      /**
       * Get the allowed maps for a user.
       *
       * @param user The user to get accessable maps for.
       * @param maps Set to a stl-set with the allowed map ids.
       * @param now The current time to check with when checking if an
       *            access is valid.
       * @param urmask The user right type mask to match right with.
       * @return True if all is well, false if not.
       */
      bool getMapIdsForUserRegionAccess( 
         const UserUser* const user, set<uint32>*& maps,
         uint32 now = TimeUtility::getRealTime(),
         UserEnums::URType urmask = UserEnums::defaultMask );


      /**
       * Get all the valid region IDs for a user.
       *
       * @param user The user to get valid regions for.
       * @param allMaps True if all maps allowed, false if not and
       *                regionIDs is filled with the valid region IDs.
       * @param regionIDs Filled with the user's valid regions.
       * @param now The current time to check with when checking if a
       *            region is valid.
       * @param urmask The user right type mask to match right with.
       * @return True if all is well, false if not.
       */
      bool getAllValidRegionIDs( 
         const UserUser* const user, bool& allMaps,
         vector< uint32 >& regionIDs,
         uint32 now = TimeUtility::getRealTime(),
         UserEnums::URType urmask = UserEnums::defaultMask );


      /**
       * Checks if a user has access to a service.
       *
       * @param user The user to check access for.
       * @param service The service to check for.
       * @param levelmask The level(s) that is acceptable, default
       *        ALL_LEVEL_MASK.
       * @param regionID The needed region, default MAX_UINT32 as
       *                 in no specific region is needed for access.
       * @param checkTime If to check if user has access now, default true.
       * @param isSingleMultiBitService If service is only one multibit 
       *                                service and not several one bit
       *                                services. Default false.
       */
      bool checkAccessToService( 
         const UserUser* const user,
         UserEnums::userRightService service,
         UserEnums::userRightLevel levelmask = UserEnums::ALL_LEVEL_MASK,
         uint32 regionID = MAX_UINT32,
         bool checkTime = true,
         bool isSingleMultiBitService = false ) const;


      /**
       * Find a matching right in user.
       *
       * @param rights The vector to add matching rights to.
       * @param user The user to find match in.
       * @param type The type to match, exact matching is done.
       */
      void getMatchingRights( vector<UserRight*>& rights,
                              const UserUser* user, 
                              UserEnums::URType type ) const;

      /**
       * Checks if user may, doesn't check region access.
       *
       * @param user The user to check.
       * @param clientSetting The settings the user uses.
       * @param may The service to check for in Iron user.
       * @return True if Iron user hasn't access to may service or not
       *         Iron client.
       */
      bool checkIfIronUserMay( 
         const UserUser* user, 
         const ClientSetting* clientSetting,
         UserEnums::userRightService may ) const;

      /**
       * Returns the user rights level.
       * @param clientSettins The settings to determine user rights level from.
       * @return user right level for client settings.
       */
      UserEnums::userRightLevel 
      getUrLevel( const ClientSetting* clientSetting ) const;

      /**
       * Checks if client setting is for a Iron client.
       *
       * @param clientSetting The settings to check.
       * @return True if Iron client.
       */
      bool checkIfIronClient( const ClientSetting* clientSetting ) const;

      /**
       * Check if client is of wolfram type.
       * @return true if wolfram client.
       */
      bool checkIfWolframClient( const ClientSetting* clientsetting ) const;

      /**
       * Checks if client setting is for a Lithium client.
       *
       * @param clientSetting The settings to check.
       * @return True if Lithium client.
       */
      bool checkIfLithiumClient( const ClientSetting* clientSetting ) const;

      /**
       * Checks if the client is allowed to consume the service.
       * It will return true if the client is allowed to use the service and
       * and false if the user can not or if something went wrong in the
       * communication. If the user can not consume the service the packages
       * will be set to the url for the client to present to the webserver.
       *
       * @param clientSetting The setting to check.
       * @param httpRequest The http request.
       * @param operationType The type of service to consume.
       * @param url An url with possible packages to purchase if the user is
       *            not allowed to consume the service.
       * @param checkedServiceIDs A set with the service ids we have already
       *                          queried, used to avoid rechecking.
       * @param serviceLocation The cordinate location for the service in case
       *                        a top region is not sent with the request.
       * @param topRegionID The top region id for the service requested.
       * @param clientLang The client's language type. Default is set to
       *                   English.
       * @param mayUseCachedCheckResult If the result of a previous check 
       *                                may be used as return for this call.
       * @return True if the client can consume the service, false if not.
       *         If false is returned there are two possible outcomes.
       *         One where something went wrong in the communication towards
       *         external access authority and then the url will be empty, 
       *         otherwhise and url will be created with the available
       *         packages to purchase.
       */
      bool checkService( const ClientSetting* clientSetting,
                         const HttpInterfaceRequest* httpRequest,
                         OperationType::operationType operationType,
                         PurchaseOptions& purchaseOptions,
                         set< MC2String >& checkedServiceIDs,
                         const MC2Coordinate& 
                         serviceLocation = MC2Coordinate(),
                         uint32 topRegionID = MAX_UINT32,
                         LangType::language_t clientLang = LangTypes::english,
                         bool mayUseCachedCheckResult = false );

      /**
       * Releases a UserItem locked in a call to getUser with useCache
       * enabled.
       * Calls ParserThreadGroup.
       *
       * @param userItem The UserItem that should be released from the 
       *                 cache
       */
      void releaseUserItem( UserItem* userItem );


      /**
       * The ID of the next request. Asks the ParserGroup.
       *
       * @return The next RequestID.
       */
      RequestData getNextRequestID();
      

      /**
       *    Tries to contact UserModule and get the user with the given 
       *    logonID. Notice that the user is created inside this
       *    method and returned via a outparameter. That means that a
       *    typical call to this method might look something like:
       *    \begin{verbatim}
            UserItem* theUser;
            if (getUser(userLogin, theUser) {
               if ( theUser != NULL ) {
                  // theUser points to a valid UserItem-object
                  ...
               } else {
                  // userLogin not found
                  ...
               }
            } else {
               // Something went wrong, theUser not valid
               ...
            }
            \end{verbatim}
       *
       *    @param logonID    The logonID of the user to get.
       *    @param userItem   Set to the user with the logonID or NULL if 
       *                      no such user.
       *    @param useCache   If the cache should be used, if true then
       *                      the UserItem must be returned by calling
       *                      releaseUserItem, if false the returned
       *                      UserItem must be delered by user. Default
       *                      false.
       *    @param wipeFromCache If to clean all caches in server and
       *                         module from this user.
       *    @return True if all communication with UserModule was ok,
       *            false if communication/database error.
       */
      bool getUser( const char* const logonID, UserItem*& userItem,
                    bool useCache = false,
                    bool wipeFromCache = false );


      /**
       *    Tries to contact UserModule and get the user with the given 
       *    UIN. Notice that the user is created inside this
       *    method and returned via a outparameter. 
       *    Calls ParserThreadGroup.
       *
       *    @param UIN        The UIN of the user to get.
       *    @param userItem   Set to the user with the logonID or NULL if 
       *                      no such user.
       *    @param useCache   If the cache should be used, if true then
       *                      the UserItem must be returned by calling
       *                      releaseUserItem, if false the returned
       *                      UserItem must be delered by user. Default
       *                      false.
       * @param wipeFromCache If to clean all caches in server and module
       *                      from this user.
       *    @return True if all communication with UserModule was ok,
       *            false if communication/database error.
       */
      bool getUser( uint32 UIN, UserItem*& userItem, 
                    bool useCache = false,
                    bool wipeFromCache = false );


      /**
       * Creates a new user from user with password passwd.
       *
       * @param user    The initial user data for the user.
       * @param passwd  The password of the user.
       * @param changerUser The user that does the adding, NULL if not
       *        known.
       * @return The UIN of the user, MAX_UINT32 if communication problem
       *         0 if not unique logonID, MAX_UINT32-2 if not unique UserIDKey
       *         of type account, and MAX_UINT32-1 if error.
       */
      uint32 createUser( UserUser* user, const char* const passwd,
                         const UserUser* changerUser );


      /**
       * Send all changes of given user to the user module.
       * Calls ParserThreadGroup.
       *
       * @param user The user with changes.
       * @param changerUser The user that does the change, NULL if not
       *        known.
       * @return True if changes are added ok, false if not.
       */
      bool changeUser( UserUser* user, const UserUser* changerUser );


      /**
       * Change a users password.
       *
       * @param user The user to change password for.
       * @param newPassword The new password.
       * @param oldPassword The old password is checked before changing.
       * @param checkPassword If to check the oldPassword before changing.
       * @param changerUser The user that does the change, NULL if not
       *        known.
       * @return 0 if ok, -1 on error, -2 on timeout, -3 on bad old 
       *         password
       */
      int32 changeUserPassword( const UserUser* user,
                                const char* newPassword,
                                const char* oldPassword,
                                bool checkPassword,
                                const UserUser* changerUser );



      /**
       *    Finds the UserCellular with phonenumber phonenumber.
       *    @param user          The user to find the phone in.
       *    @param phonenumber   The phonenumber to look for.
       *    @return The UserCellular with phonenumber phonenumber or
       *            NULL if no such cellular in user.
       */
      UserCellular* getUsersCellular( 
         const UserUser* const user, const char* const phonenumber ) const;


      /**
       * Sets all UserLicenceKeys with same key as userKey to removed in
       * user.
       *
       * @param user The user to remove UserLicenceKeys from.
       * @param userKey The key to remove.
       * @return The number of keys removed.
       */
      uint32 removeAllUserLicenceKey( UserUser* user, 
                                      UserLicenceKey* userKey );


      /**
       *    Finds the user that own the UserCellular with the phonenumber
       *    number.
       *    @param number The phonenumber to search for.
       *    @param userItem Set to the user with the cellular or NULL if 
       *                    no such cellular.
       *    @param useCache   If the cache should be used, if true then
       *                      the UserItem must be returned by calling
       *                      releaseUserItem, if false the returned
       *                      UserItem must be delered by user. Default
       *                      false.
       * @param wipeFromCache If to clean all caches in server and module
       *                      from this user.
       *    @return True if all communication with UserModule was ok,
       *            false if communication/database error.
       */
      bool getUserFromCellularNumber( const char* const number, 
                                      UserItem*& userItem,
                                      bool useCache = false,
                                      bool wipeFromCache = false );

      /**
       *    A low level method for finding a user that matches the 
       *    UserElement.
       *    @param elem The element to search with.
       *    @param userItem Set to the user that matches elem or NULL if 
       *                    no such user.
       *    @param nbrUsers   Set to the number of users found.
       *    @param useCache   If the cache should be used, if true then
       *                      the UserItem must be returned by calling
       *                      releaseUserItem, if false the returned
       *                      UserItem must be delered by user. Default
       *                      false.
       * @param wipeFromCache If to clean all caches in server and module
       *                      from this user.
       *    @return True if all communication with UserModule was ok,
       *            false if communication/database error.
       */
      bool getUserFromUserElement( const UserElement* elem, 
                                   UserItem*& userItem, uint32& nbrUsers,
                                   bool useCache = false,
                                   bool wipeFromCache = false );


      /**
       * Authenticate a user.
       *
       * @param userName The users logonID.
       * @param userPasswd The users password.
       * @param checkExpired If to check if user is expired, default
       *                     false.
       * @return The UIN of the user or 0 if invalid login, MAX_UIN32
       *         if communication problem and MAX_UINT32 -1 if expired
       *         user.
       */
      uint32 authenticateUser( const char* userName, 
                               const char* userPasswd,
                               bool checkExpired = false );


      /**
       * Authenticate a user using a session.
       *
       * @param sessionID The ID of the session.
       * @param sessionKey The key of the session.
       * @param checkExpired If to check if user is expired, default
       *                     false.
       * @return The UIN of the user or 0 if invalid login, MAX_UIN32 -2
       *         if communication problem, MAX_UINT32 -1 if expired
       *         user and MAX_UINT32 if expired session.
       */
      uint32 authenticateUserSession( const char* sessionID, 
                                      const char* sessionKey,
                                      bool checkExpired = false );


      /**
       * Alter and get a user's number transactions.
       *
       * @param UIN The User's Identifications Number.
       * @param transactionChange The change in number of transactions.
       * @param nbrTransactions Set to the number of transactions the user
       *                        has after this change.
       * @return True if changed ok, false if not. If false then 
       *         nbrTransactions is not set.
       */
      bool getAndChangeTransactions( uint32 UIN, int32 transactionChange,
                                     int32& nbrTransactions );


      /**
       * Get and optionally check a user's transaction days.
       *
       * @param UIN The User's Identification Number.
       * @param check If to check if a new day is needed.
       * @param nbrTransactionDays Input as the number of days change,
       *                           set to the number of transaction days
       *                           the user has left.
       * @param curTime Set to the time of when the current transaction
       *                day started.
       * @return The status, OK if all is well, NOT_ALLOWED if no days
       *         left and check is true and NOTOK if error, TIMEOUT_ERROR
       *         if timeout on communication.
       */
      StringTable::stringCode getAndChangeTransactionDays( 
         uint32 UIN, bool check, int32& nbrTransactionDays, 
         uint32& curTime );


      /**
       * Get WFST for a user, returns MAX_BYTE if no type for user.
       *
       * @param user The user to get WFST for.
       * @param urservice The user right service to match right with.
       * @param onlyActive If only currently active levels should be
       *                   returned.
       * @return The WFST for a user.
       */
      WFSubscriptionConstants::subscriptionsTypes 
         getSubscriptionTypeForUser( 
            const UserUser* user, 
            UserEnums::userRightService urservice,
            bool onlyActive = true,
            UserEnums::userRightLevel urlevel = UserEnums::ALL_LEVEL_MASK ) const;


      /**
       * Get the edgenodes for a map.
       *
       * @param mapID The map to get edge nodes for.
       * @param overMapID The overview map to translate edgenodeId to. Set
       *                  to mapID to avoid translation.
       * @param externalNodes Nodes are added to this vector.
       * @return StringTable::OK if all is ok, error code if not.
       */
      StringTable::stringCode getEdgeNodesFor( 
         uint32 mapID, uint32 overMapID, vector<uint32>& externalNodes );

      /**
       *    Gets many tilemaps on the format:
       *    <br />
       *    Zero-terminated string with params.<br />
       *    4 bytes big endian length <br />
       *    length bytes of data <br />
       *    <br />
       *    @param params A vector of parameters.
       *    @param startOffset The start offset in bytes.
       *    @param maxBytes    The approx maximum number of bytes to send.
       */
      DataBuffer* getTileMaps( const vector<MC2SimpleString>& params,
                               uint32 startOffset,
                               uint32 maxBytes );

      /**
       *    Gets the tilemaps that the query wants.
       *    @param query Query to fill in.
       */
      void getTileMaps( TileMapQuery& query );
      
      /**
       *    Gets one TileMap. It is recommended that getTileMaps
       *    with a query as parameter is used instead.
       */
      DataBuffer* getTileMap( const char* paramStr );

      
      /**
       *    Get a string that describes the type of server. E.g. this method
       *    coule return "XML" (the XMLParserThread) or "MS" 
       *    (MapCentralParserThread).
       *    @return  A string that describes the server that does the 
       *             debiting.
       */
      virtual const char* getServerType() = 0;
      
      /**
       * Sets the log-prefix.
       *
       * @param prefix The string used as prefix for all mc2-log printing.
       */
      void setLogPrefix( const char* prefix );

      /**
       * Get the current logprefix. May be NULL.
       */
      const char* getLogPrefix() const;

      /**
       * Get the TopRegionRequest from the ParserThreadGroup
       *
       * @return The current TopRegionRequest, may be NULL if modules
       *         aren't working.
       */
      const TopRegionRequest* getTopRegionRequest();


      /**
       * Get the ParserTokenHandler.
       */
      ParserTokenHandler* getTokenHandler();


      /**
       * Get the ParserExternalAuth.
       */
      ParserExternalAuth* getExternalAuth();


      /**
       * Get the ParserUserHandler.
       */
      ParserUserHandler* getUserHandler();


      /**
       * Get the ParserDebitHandler.
       */
      ParserDebitHandler* getDebitHandler();


      /**
       * Get the ParserActivationHandler.
       */
      ParserActivationHandler* getActivationHandler();


      /**
       *   Returns the TileHandler.
       */
      ParserTileHandler* getTileHandler() {
         return m_tileHandler.get();
      }

      /**
       *   Returns the ParserMapHandler.
       */
      ParserMapHandler& getMapHandler() {
         return *m_parserMapHandler;
      }

      /**
       *   Returns the ParserRouteHandler.
       */
      ParserRouteHandler& getRouteHandler() {
         return *m_routeHandler;
      }

      /**
       * @return the SearchParserHandler.
       */
      SearchParserHandler& getSearchHandler();

      /**
       * @return const SearchParserHandler.
       */
      const SearchParserHandler& getSearchHandler() const;

      /**
       * Returns the ParserPoiReviewHandler.
       */
      ParserPoiReviewHandler& getPoiReviewHandler() {
         return *m_poiReviewHandler;
      }

      /**
       * Returns const ParserPoiReviewHandler.
       */
      const ParserPoiReviewHandler& getPoiReviewHandler() const {
         return *m_poiReviewHandler;
      }

      /**
       * Returns the ParserPosPush.
       */
      ParserPosPush& getPosPush() {
         return *m_posPush;
      }

      /**
       * Returns const ParserPosPush.
       */
      const ParserPosPush& getPosPush() const {
         return *m_posPush;
      }

      /**
       * Get the ParserCWHandler.
       */
      ParserCWHandler* getCWHandler();

      /**
       * Return the URLFetcher.
       */
      URLFetcher* getURLFetcher();

      /**
       * Get the App Store handler.
       */
      ParserAppStoreHandler& getAppStore();

      /**
       * Sets the peer IP.
       *
       * @param IP The IP of the peer.
       */
      void setPeerIP( uint32 IP );

      /**
       *   Sets the currently active user.
       */
      void setUser( UserItem* user );

      /**
       * Set the client settings for the current user
       * @param settings
       */
      void setClientSetting( const ClientSetting* settings );

      /// @return client settings for the current user
      const ClientSetting* getClientSetting() const { 
         return m_clientSetting;
      }

      /// Set the request data.
      void setRequestData( const InterfaceRequestData* rd );

      /// @return request data.
      const InterfaceRequestData* getRequestData() const;

      /// @return server host + ip
      const IPnPort& getServerAddress() const;

      /// Get the group of this Thread.
      ParserThreadGroupHandle getGroup() const;

      /**
       *  @name Map IDs from coordinates and boundingboxes.
       * These functions are used to find which maps overlap a specifed
       * boundingbox. 
       */
      //@{
      /**
       * Finds all the maps that may encompass a specific coordinate.
       * @param maps      The IDs of found maps will be inserted into
       *                  this set. Note that the set will _not_ be
       *                  emptied by this function.
       * @param underview Check lowest level maps if true.
       * @param country   Check country level maps if true.
       * @param coord     The coordinate. 
       * @param radius xx Since we actually check whether maps and a
       *                  certain boundingbox overlap we use the
       *                  coordinate ac the center of a boundingbox
       *                  with the width and height 2*radius. radius
       *                  defaults to 200 meters,
       * @return The number of map IDs inserted into maps. This is the
       *         same as the difference between maps.size() before and
       *         after the function call.
       */
      std::set<uint32>::size_type
      getMapsFromCoordinate(std::set<uint32>& maps, 
                            bool underview, bool country, 
                            const MC2Coordinate coord, int radius = 200);
      /**
       * Finds all the maps that overlap a boundingbox.
       * @param maps      The IDs of found maps will be inserted into
       *                  this set. Note that the set will _not_ be
       *                  emptied by this function.
       * @param underview Check lowest level maps if true.
       * @param country   Check country level maps if true.
       * @param bbox      The boundingbox
       * @return The number of map IDs inserted into maps. This is the
       *         same as the difference between maps.size() before and
       *         after the function call.
       */
      std::set<uint32>::size_type
      getMapsFromBBox(std::set<uint32>& maps, 
                            bool underview, bool country, 
                            const MC2BoundingBox& bbox);
      /**
       *    Return the current user (default m_user).
       */
      UserItem* getCurrentUser();

      /**
       * Returns true if the current user is a user account used for
       * testing if the system is upp all the time.
       *
       * @return True if current user is a check user.
       */
      bool isCheckUser() const;

      /**
       * Return the name of this server type.
       *
       * @return String with the name of this server.
       */
      const MC2String& getServerName() const;

private:
      /**
       * Helper function for getMapsFromBBox. Creates a
       * BBoxRequestPacket, inserts it into a PacketContainer and then
       * pushes that PacketContainer onto the back of a vector.
       * @param cont      The vector the new packet container will be
       *                  pushed onto.
       * @param bbox      The boundingbox parameter for the 
       *                  BBoxRequestPacket.
       * @param underview The underview parameter for the
       *                  BBoxRequestPacket.
       * @param country   The country parameter for the
       *                  BBoxRequestPacket.
       * @param mapset    The mapset parameter for the BBoxRequestPacket.
       */
      void
      pushBBoxRequestPacketContainer(std::vector<PacketContainer*>& cont,
                                     const MC2BoundingBox& bbox, 
                                     bool underview, bool country,
                                     uint32 mapset);
      //@}

      /**
       * Internal checkService. This method relies on that checkService
       * has checked the input variables.
       * @see CheckService.
       *
       * @param errorReason Set to a description of why the service
       *                    check failed.
       * @param topRegionID Changed as this sets the topregion if not set
       *                    before this call.
       * @return 0 If checked and user has access, 1 if no need to query 
       *         for access, 2 if already checked, 3 if checked and user
       *         has no access,
       *         -1 if error querying for access, -2 if error with input, 
       *         
       */
      int internalCheckService( const ClientSetting* clientSetting,
                                const HttpInterfaceRequest* httpRequest,
                                OperationType::operationType operationType,
                                PurchaseOptions& purchaseOptions,
                                set< MC2String >& checkedServiceIDs,
                                const MC2Coordinate& serviceLocation,
                                uint32& topRegionID,
                                LangType::language_t clientLang,
                                bool mayUseCachedCheckResult,
                                MC2String& errorReason );

      /**
       * Checks for a previous check for a operationType and topRegionID.
       * Called by internalCheckService.
       *
       * @param operationType The operationType to look for.
       * @param topRegionID The topRegionID to look for.
       * @param res Set to the prevoius check result.
       * @return True if a previous check was found.
       */
      bool hasCachedServiceCheck( OperationType::operationType operationType,
                                  uint32 topRegionID,
                                  int& checkRes );

protected:
   void clearUserItem() {
      m_user = NULL;
      m_clientSetting = NULL;
      m_requestData = NULL;
   }

      void setServerAddress(const IPnPort& address);

      /**
       * Wait for a specific put request to return.
       * m_monitor must be locked to call this.
       */
      void internalWaitForRequest( uint16 requestID );

      /**
       *    The ParserThreadGroup that this ParserThread is part of.
       */
      ParserThreadGroupHandle m_group;
      
      /**
       *    The Monitor used for RequestHandling.
       */
      ISABMonitor m_monitor;
      
      /**
       *    The autheticated user of the isab-mc2 request.
       */
      UserItem* m_user;

      /**
       * The current user's client setting, set when it is a client 
       * doing the request.
       */
      const ClientSetting* m_clientSetting;

      /// The reqest data.
      const InterfaceRequestData* m_requestData;

      /**
       *    The name of this server to use when debiting.
       */
      MC2String m_serverName;
      
      /**
       *    The peers IP.
       */
      uint32 m_peerIP;
      
      /**
       *    The peers port.
       */
      uint16 m_peerPort;

      /**
       * The current mc2-log prefix.
       */
      ScopedArray<char> m_logPrefix;

      /**
       * If no debiting nor any storing should be made for user.
       * Check user checks the system reguarly.
       */
      bool m_isCheckUser;

      /**
       * The class that handles tokens.
       */
      auto_ptr<ParserTokenHandler> m_tokenHandler;


      /**
       * The class that handles external authentication.
       */
      auto_ptr<ParserExternalAuth> m_externalAuth;


      /**
       * The class that handles users.
       */
      auto_ptr<ParserUserHandler> m_userHandler;

      /**
       * The class that handles debits.
       */
      auto_ptr<ParserDebitHandler> m_debitHandler;


      /**
       * The class that handles activations.
       */
      auto_ptr<ParserActivationHandler> m_activationHandler;

      /**
       *   The class that handles the tile maps.
       */
      auto_ptr<ParserTileHandler> m_tileHandler;

      /**
       *   Object that handles map functions.
       */
      auto_ptr<ParserMapHandler> m_parserMapHandler;
   
      /**
       * The class that helps with route functions.
       */
      auto_ptr<ParserRouteHandler> m_routeHandler;

      /**
       * The class that helps with Content Window handling.
       */
      auto_ptr<ParserCWHandler> m_cwHandler;

      /**
       * The class that helps with combied searches.
       */
      auto_ptr<SearchParserHandler> m_searchHandler;

      /**
       * The class that handles poi reviews.
       */
      auto_ptr<ParserPoiReviewHandler> m_poiReviewHandler;

      /**
       * The class that handles position pushes.
       */
      auto_ptr<ParserPosPush> m_posPush;

      /**
       * The class that handles App Store actions.
       */
      auto_ptr<ParserAppStoreHandler> m_appStoreHandler;

      /**
       * If client uses iso-8859-1.
       */
      bool m_clientUsesLatin1;

      /**
       * The URLFetcher.
       */
      auto_ptr<URLFetcher> m_urlFetcher;

   private:

   IPnPort m_serverAddress;

   /// The set of requests in progress.
   typedef set< SendRequestHolder*, 
                SortSendRequestHolderByRequestID > ReqHolderSet;
   /**
    * The requests in progress that is sent by sendRequest methods.
    */
   ReqHolderSet m_requests;

   /// The set of done requests.
   typedef set< const ThreadRequestContainer*, 
                SortThreadRequestContainerByRequestID > ReqContSet;

   /**
    * The currently done requests.
    */
   ReqContSet m_replies;

   /// The last checksum from search headings.
   uint32 m_searchHeadingCRC;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline ParserTokenHandler* 
ParserThread::getTokenHandler() {
   return m_tokenHandler.get();
}


inline ParserExternalAuth* 
ParserThread::getExternalAuth() {
   return m_externalAuth.get();
}


inline ParserUserHandler* 
ParserThread::getUserHandler() {
   return m_userHandler.get();
}

inline ParserDebitHandler* 
ParserThread::getDebitHandler() {
   return m_debitHandler.get();
}

inline ParserActivationHandler* 
ParserThread::getActivationHandler() {
   return m_activationHandler.get();
}

inline ParserCWHandler*
ParserThread::getCWHandler() {
   return m_cwHandler.get();
}

inline ParserAppStoreHandler&
ParserThread::getAppStore() {
   return *m_appStoreHandler;
}

inline void 
ParserThread::setPeerIP( uint32 IP ) {
   m_peerIP = IP;
}


inline bool
ParserThread::clientUsesLatin1() const {
   return m_clientUsesLatin1;
}


inline void
ParserThread::setClientUsesLatin1( bool clientUsesLatin1 ) {
   m_clientUsesLatin1 = clientUsesLatin1;
}


inline URLFetcher*
ParserThread::getURLFetcher() {
   return m_urlFetcher.get();
}

inline void 
ParserThread::setServerAddress(const IPnPort& address) { 
   m_serverAddress = address; 
}

inline const IPnPort&
ParserThread::getServerAddress() const {
   return m_serverAddress;
}

inline bool
ParserThread::isCheckUser() const {
   return m_isCheckUser;
}

inline const MC2String&
ParserThread::getServerName() const {
   return m_serverName;
}

#endif // PARSERTHREAD_H

