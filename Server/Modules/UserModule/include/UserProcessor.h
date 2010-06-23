/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USER_PROCESSOR_H
#define USER_PROCESSOR_H

#include "Packet.h"
#include "Processor.h"
#include "Queue.h"
#include "UserPacket.h"
#include "DebitPacket.h"
#include "RouteStoragePacket.h"
#include "MapUpdatePacket.h"
#include "UserFavoritesPacket.h"
#include "AddUserTrackPacket.h"
#include "GetUserTrackPacket.h"
#include "SQLDataContainer.h"
#include "GetStoredUserDataPacket.h"
#include "SetStoredUserDataPacket.h"
#include "SQLTableData.h"
#include <map>
#include <set>
#include <memory>

// Forward
class TransactionRequestPacket;
class TransactionDaysRequestPacket;
class WFActivationRequestPacket;
class WFActivationReplyPacket;
class Cache;
class CharEncSQLConn;
class CharEncSQLQuery;
class UserCellular;
class UserLicenceKey;
class UserRegionAccess;
class UserRight;
class UserWayfinderSubscription;
class UserToken;
class UserPIN;
class UserIDKey;
class UserBuddyList;
class UserNavigator;
class SQLQuery;
class UserLastClient;
class PoiReviewItem;
class PoiReviewDetail;
class LicenceToRequestPacket;
class LeaderStatus;
class IDKeyToRequestPacket;

#ifdef PARALLEL_USERMODULE
class SessionCache;
#else
class UserSessionCache;
#endif

/**
 * Class for holding a debit waiting to be inserted into database.
 */
class DebitData {
public:
   DebitData( uint32 idate, uint32 iUIN, const MC2String& iserverID,
              const MC2String& iuserOrigin, uint32 imessageID,
              uint32 idebInfo, uint32 ioperationType, uint32 isentSize,
              const MC2String& ioperationDescription ) 
         : date( idate ), UIN( iUIN ), serverID( iserverID ),
           userOrigin( iuserOrigin ), messageID( imessageID ),
           debInfo( idebInfo ), operationType( ioperationType ),
           sentSize( isentSize ), 
           operationDescription( ioperationDescription )
      {}

   bool operator < ( const DebitData& o ) const {
      return date < o.date;
   }

   friend ostream& operator << ( ostream& o, const DebitData& d ) {
      o << "DebitData( " << d.date << ", " << d.UIN << ", " << d.serverID
        << ", " << d.userOrigin << ", " << d.messageID << ", " << d.debInfo
        << ", " << d.operationType << ", " << d.sentSize << ", " 
        << d.operationDescription << " )";
      return o;
   }

   uint32 date;
   uint32 UIN;
   MC2String serverID;
   MC2String userOrigin;
   uint32 messageID;
   uint32 debInfo;
   uint32 operationType;
   uint32 sentSize;
   MC2String operationDescription;
};


/**
 *    Processes UserRequestPackets. 
 *
 */
class UserProcessor : public Processor {
   public:
      /**
       * Creates a new UserProcessor with a vector of loaded maps
       *
       * @param loadedMaps The standard list of loaded maps for modules.
       * @param noSqlUpdate If not to update sql tables.
       * @param leaderStatus The module's leader status.
       */
      UserProcessor( MapSafeVector* loadedMaps, 
                     bool noSqlUpdate, 
                     const LeaderStatus* leaderStatus);

      virtual ~UserProcessor();
 
      
      /**
       * Returns the status of the module.
       *
       * @return 0.
       */
      int getCurrentStatus();
      
      
      /**
       *    Virtual function for loading a map. Should be called
       *    by handleRequest when that function gets a loadMapRequest
       *    and isn't virtual anymore.
       *    @param mapID The map to load.
       *    @param mapSize Outparameter describing the size of the
       *                   map.
       *    @return StringTable::OK if ok.
       */
      virtual StringTable::stringCode loadMap(uint32 mapID,
                                      uint32& mapSize) {
         mapSize = 1;
         return StringTable::OK;
      }

      /**
       *    Virtual function to be called when a map should
       *    be deleted.
       *    @param mapID The map to be deleted.
       *    @return StringTable::OK if ok.
       */
      virtual StringTable::stringCode deleteMap(uint32 mapID) {
         return StringTable::OK;
      }

      
      /**
       * How long an unused session is valid after last use in s.
       */
      static const int32 sessionValidTime;


      /**
       * How often a session is updated in the SQL-database s.
       * Is less than sessionValidTime.
       */
      static const int32 sessionUpdateTime;


      /**
       * How old a session has to be before being moved to history.
       * Is more than sessionValidTime.
       */
      static const int32 sessionOldTime;


      /**
       * Set if to use caching of user data.
       */
      void setUseUserCache( bool useUserCache );
      
      /**
       * Checks wether handling this request will result in only reads from
       * the database.
       * @param   request  The request to check.
       */
      static bool readOnly( const RequestPacket* request );

protected:
       /**
       * Handles a request and returns a reply, perhaps NULL.
       *
       * @param p The packet to handle.
       * @return A reply to p or NULL if p has unknown packet type.
       */
      Packet *handleRequestPacket( const RequestPacket& p,
                                   char* packetInfo );
      
private:
      /** Handles a GetUserDataRequestPacket and returns an answer */
      UserReplyPacket* handleGetUserDataRequestPacket( const 
         GetUserDataRequestPacket* p );

      
      /** Handles a AddUserRequestPacketand returns an answer */
      UserReplyPacket* handleAddUserRequestPacket( const AddUserRequestPacket* p);
      
      
      /** Handles a DeleteUserRequestPacketand returns an answer */
      UserReplyPacket* handleDeleteUserRequestPacket( const 
         DeleteUserRequestPacket* p );
      
      
      /** Handles a ChangeUserDataRequestPacketand returns an answer */
      UserReplyPacket* handleChangeUserDataRequestPacket( const 
         ChangeUserDataRequestPacket* p );
      
      
      /** Handles a CheckUserPasswordRequestPacket and returns an answer */
      UserReplyPacket* handleCheckUserPasswordRequestPacket( const 
         CheckUserPasswordRequestPacket* p );
      

      /** Handles a FindUserRequestPacket and returns an answer */
      UserReplyPacket* handleFindUserRequestPacket( const 
         FindUserRequestPacket* p );
   
      /** Handles a UserFavoritesRequestPacket and returns an answer */
      UserReplyPacket* handleUserFavoritesRequestPacket( const 
         UserFavoritesRequestPacket* p );

      /** Handles a DebitRequestPacket and returns an answer */
      DebitReplyPacket* handleDebitRequestPacket( const  DebitRequestPacket* p );

      /**
       * Inserts the debits in m_debitSet.
       */
      void addStoredDebits();

      /** Handles a GetCellularPhoneModelsPacket and returns an answer */
      GetCellularPhoneModelDataReplyPacket* 
         handleGetCellularPhoneModelsRequestPacket( const  
            GetCellularPhoneModelDataRequestPacket* p );
      
      
      /** 
       *  Handles a AddCellularPhoneModelRequestPacket 
       *  and returns an answer.
       */
      ReplyPacket* handleAddCellularPhoneModelRequestPacket
         ( const AddCellularPhoneModelRequestPacket* p);

      
      /** 
       * Handles a ChangeUserPasswordPequestPacket and returns an answer 
       */
      UserReplyPacket* handleChangeUserPasswordRequestPacket( const 
         ChangeUserPasswordRequestPacket* p);


      /** 
       *  Handles a ChangeCellularPhoneModelRequestPacket 
       *  and returns an answer.
       */
      ReplyPacket* handleChangeCellularPhoneModelRequestPacket
         ( const ChangeCellularPhoneModelRequestPacket* p);


      /** 
       *  Handles a VerifyUserRequestPacket
       *  and returns an answer.
       */
      ReplyPacket* handleVerifyUserRequestPacket
         ( const VerifyUserRequestPacket* p);


      /** 
       *  Handles a LogoutUserRequestPacket
       *  and returns an answer.
       */
      ReplyPacket* handleLogoutUserRequestPacket
         ( const LogoutUserRequestPacket* p);


      /** 
       *  Handles a SessionCleanUpRequestPacket
       *  and returns an answer.
       */
      ReplyPacket* handleSessionCleanUpRequestPacket
         ( const SessionCleanUpRequestPacket* p);


      /** Handles a ListDebitRequestPacket and returns an answer */
      ReplyPacket* handleListDebitRequestPacket
         ( const ListDebitRequestPacket* p); 

      
      /**
       * Handles a AddUserNavDestinationRequestPacket 
       * and returns an answer.
       */
      ReplyPacket* handleAddUserNavDestinationRequestPacket( const  
         AddUserNavDestinationRequestPacket* p );

      
      /**
       * Handles a DeleteUserNavDestinationRequestPacket
       * and returns an answer.
       */
      ReplyPacket* handleDeleteUserNavDestinationRequestPacket( const  
         DeleteUserNavDestinationRequestPacket* p );


      /**
       * Handles a ChangeUserNavDestinationRequestPacket
       * and returns an answer.
       */
      ReplyPacket* handleChangeUserNavDestinationRequestPacket( const  
         ChangeUserNavDestinationRequestPacket* p );


      /**
       * Handles a GetUserNavDestinationRequestPacket
       * and returns an answer.
       */
      ReplyPacket* handleGetUserNavDestinationRequestPacket( const  
         GetUserNavDestinationRequestPacket* p );


      /** Handles a AuthUserRequestPacket and returns an answer */
      UserReplyPacket* handleAuthUserRequestPacket( const 
         AuthUserRequestPacket* p );

      
      /**
       * Handles a RouteStorageAddRouteRequestPacket
       * and returns an answer.
       */
      ReplyPacket* handleRouteStorageAddRouteRequestPacket( const  
         RouteStorageAddRouteRequestPacket* p );


      /**
       * Handles a RouteStorageGetRouteRequestPacket
       * and returns an answer.
       */
      ReplyPacket* handleRouteStorageGetRouteRequestPacket( const  
         RouteStorageGetRouteRequestPacket* p );

      /**
       * Handles a CreateSessionRequestPacket.
       * and returns an answer.
       */
      ReplyPacket* handleCreateSessionRequestPacket( const 
         CreateSessionRequestPacket* p );

      /**
       *    Handles a MapUpdateRequestPacket and returns an answer.
       */
      ReplyPacket* handleMapUpdateRequestPacket( const 
         MapUpdateRequestPacket* p );

      /**
       *    Handle a AddUserTrackRequestPacket and return an answer.
       */
      ReplyPacket* handleAddUserTrackRequestPacket( const AddUserTrackRequestPacket* p);

      /**
       *    Handle a GetTrackTrackRequestPacket and return an answer.
       */
      ReplyPacket* handleGetUserTrackRequestPacket( const GetUserTrackRequestPacket* p);

      /**
       * Handle a TransactionRequestPacket and return an answer.
       */
      ReplyPacket* handleTransactionRequestPacket( const  
         TransactionRequestPacket* p );

      /**
       * Handle a TransactionDaysRequestPacket and return an answer.
       */
      ReplyPacket* handleTransactionDaysRequestPacket( const  
         TransactionDaysRequestPacket* p );

      /**
       * Handles a RouteStorageChangeRouteRequestPacket
       * and returns an answer.
       */
      ReplyPacket* handleRouteStorageChangeRouteRequestPacket( const  
         RouteStorageChangeRouteRequestPacket* p );

      /**
       *   Handles a WFActivationRequestPacket.
       */
      ReplyPacket* handleWFActivationRequestPacket(
         const WFActivationRequestPacket* req);


      /**
       * Add or modify a review or grade a poi.
       */
      ReplyPacket* handlePoiReviewAdd( const RequestPacket& p );

      /**
       * Remove a review.
       */
      ReplyPacket* handlePoiReviewDelete( const RequestPacket& p );

      /**
       * List reviews.
       */
      ReplyPacket* handlePoiReviewList( const RequestPacket& p );

      /**
       * Grade a poi.
       */
      uint32 gradePoi( const PoiReviewItem& poi, const PoiReviewDetail& d,
                       CharEncSQLQuery* sqlQuery );

      /**
       * Handle periodic packet.
       */
      ReplyPacket* handlePeriodic( const RequestPacket& p );

      /**
       * Handle GetStoredUserDataRequestPacket
       */
      UserReplyPacket* handleGetStoredUserDataRequestPacket( const
            GetStoredUserDataRequestPacket& p );
      
      /**
       * Handle SetStoredUserDataRequestPacket
       */
      UserReplyPacket* handleSetStoredUserDataRequestPacket( const
            SetStoredUserDataRequestPacket& p );

      /**
       * Handle licence to request.
       */
      UserReplyPacket* handleLicenceToRequestPacket( 
         const LicenceToRequestPacket& p );

      /**
       * Handle id key to request.
       */
      UserReplyPacket* handleIDKeyToRequestPacket( 
         const IDKeyToRequestPacket& p );

      /** Asks the database for a UserUser with UIN UIN and returns it. */
      UserUser* getUserUser( uint32 UIN );
      

      /**
       *  Asks the database for a user's cellulars. 
       *
       *  @param UIN The user.
       *  @param cellulars Is set to the cellulars.
       *  @return the number of cellulars for the user. -1 if error.
       */
      int32 getUserCellular( uint32 UIN, UserCellular**& cellulars );

      /**
       * Asks the database for a user's licencies. 
       *
       * @param UIN The user.
       * @param licencies Licence keys added to this.
       * @param sqlQuery A query to use, new is created if NULL.
       * @return The number of licencies for the user. -1 if error.
       */
      int32 getUserLicence( uint32 UIN, vector<UserLicenceKey*>& licencies,
                            CharEncSQLQuery* sqlQuery = NULL );

      /**
       * Asks the database for a user's licencies in the old key table. 
       *
       * @param UIN The user.
       * @param licencies Old licence keys added to this.
       * @param sqlQuery A query to use, new is created if NULL.
       * @return The number of old licencies for the user. -1 if error.
       */
      int32 getOldUserLicence( uint32 UIN, vector<UserLicenceKey*>& licencies,
                               CharEncSQLQuery* sqlQuery = NULL );


      /**
       *  Asks the database for a user's regino accesses. 
       *
       *  @param UIN The user.
       *  @param licencies Is set to the licencies.
       *  @return The number of licencies for the user. -1 if error.
       */
      int32 getUserRegionAccess( uint32 UIN, 
                                 UserRegionAccess**& accesses );
    
      /**
       *  Asks the database for a user's rights. 
       *
       *  @param UIN The user.
       *  @param rights Is set to the rights.
       *  @return The number of rights for the user. -1 if error.
       */
      int32 getUserRights( uint32 UIN, vector<UserRight*>& rights );


      /**
       *  Asks the database for a user's wayfinder subscription data. 
       *
       *  @param UIN The user.
       *  @param subsr Is set to the subscription.
       *  @return The number of subscrintins for the user. -1 if error.
       */
      int32 getUserWayfinderSubscription( 
         uint32 UIN, vector<UserWayfinderSubscription*>& subcr );

    
      /**
       *  Asks the database for a user's tokens. 
       *
       *  @param UIN The user.
       *  @param tokens The user's tokens are added to this, caller 
       *                deletes.
       *  @return The number of tokens for the user. -1 if error.
       */
      int32 getUserTokens( uint32 UIN, vector<UserToken*>& tokens );

    
      /**
       *  Asks the database for a user's PINs. 
       *
       *  @param UIN The user.
       *  @param PINs The user's PINs are added to this, caller 
       *              deletes.
       *  @return The number of PINs for the user. -1 if error.
       */
      int32 getUserPINs( uint32 UIN, vector<UserPIN*>& PINs );

    
      /**
       *  Asks the database for a user's idKeys. 
       *
       *  @param UIN The user.
       *  @param els The user's idKeys are added to this, caller 
       *             deletes.
       * @param sqlQuery A query to use, a new one is used if NULL.
       *  @return The number of idKeys for the user. -1 if error.
       */
      int32 getUserIDKeys( uint32 UIN, vector<UserIDKey*>& els,
                           CharEncSQLQuery* sqlQuery = NULL );

    
      /**
       *  Asks the database for a user's lastClients. 
       *
       *  @param UIN The user.
       *  @param els The user's lastClients are added to this, caller 
       *             deletes.
       *  @return The number of lastClients for the user. -1 if error.
       */
      int32 getUserLastClients( uint32 UIN, vector<UserLastClient*>& els );


      /**
       *   Copy the current row in the CharEncSQLQuery to a new 
       *   CellularPhoneModel
       *   @param sqlQuery The query to use
       *   @return Pointer to a new CellularPhoneModel
       */
      CellularPhoneModel* fillCellularPhoneModel(CharEncSQLQuery* 
                                                 sqlQuery);

      /**
       *  Checks a logonID and password against the database and returns 
       *  the UIN of the user, 0 if login failed.
       *
       *  @param logonID String with users loginname.
       *  @param logonPasswd String with users login password.
       *  @param checkExpired If to check if user is expired.
       *  @return The UIN of the user, 0 if login failed and MAX_UINT32 -1
       *          if user is expired.
       */
      uint32 getUser( const char* logonID, const char* logonPasswd,
                      bool checkExpired = false );


      /**
       * Asks the database for a users buddylists. 
       *
       * @param UIN The user in question.
       * @param buddies Set to the buddies of user.
       * @return the number of buddyLists for the user. -1 if error
       */
      int32 getUserBuddyLists( uint32 UIN, UserBuddyList**& buddies );
      

      /**
       * Asks the databse for a users navigators.
       *
       * @param UIN The user in question.
       * @param buddies Set to the Navigators of user.
       * @return the number of Navigators for the user. -1 if error.
       */
      int32 getUserNavigators( uint32 UIN, UserNavigator**& navis );

      
      /**
       * Looks for Users which matches the data in user
       *
       * @param user UserUser with changed fields to search with.
       * @param users The found users' UINs.
       * @param logonIDs The found users' logonIDs.
       * @param sqlQuery A query to use, new is created if NULL.
       * @return Nbr users found.
       */
      uint32 getUINsFromUserUser( const UserUser* user, 
                                  vector<uint32>& users, 
                                  vector<MC2String>& logonIDs,
                                  CharEncSQLQuery* sqlQuery = NULL );
      
      
      /**
       * Looks for Users which matches the data in cellular
       *
       * @param cellular UserCellular with changed fields to search with.
       * @param users The found users' UINs.
       * @param logonIDs The found users' logonIDs.
       * @param sqlQuery A query to use, new is created if NULL.
       * @return Nbr users found.
       */
      uint32 getUINsFromUserCellular( const UserCellular* cellular, 
                                      vector<uint32>& users, 
                                      vector<MC2String>& logonIDs,
                                      CharEncSQLQuery* sqlQuery = NULL );

      
      /**
       * Looks for Users which matches the data in licence.
       *
       * @param licence UserLicenceKey with changed fields to search with.
       * @param users The found users' UINs.
       * @param logonIDs The found users' logonIDs.
       * @param sqlQuery A query to use, new is created if NULL.
       * @return Nbr users found.
       */
      uint32 getUINsFromUserLicenceKey( const UserLicenceKey* licence, 
                                        vector<uint32>& users, 
                                        vector<MC2String>& logonIDs,
                                        CharEncSQLQuery* sqlQuery = NULL );

      
      /**
       * Looks for Users which matches the type in subscription.
       *
       * @param subscr UserWayfinderSubscription with changed fields to
       *               search with.
       * @param users The found users' UINs.
       * @param logonIDs The found users' logonIDs.
       * @param sqlQuery A query to use, new is created if NULL.
       * @return Nbr users found.
       */
      uint32 getUINsFromUserWayfinderSubscription( 
         const UserWayfinderSubscription* subscr, 
         vector<uint32>& users, 
         vector<MC2String>& logonIDs,
         CharEncSQLQuery* sqlQuery = NULL );

      
      /**
       * Looks for Users which matches the data in key.
       *
       * @param key UserIDKey with changed fields to search with.
       * @param users The found users' UINs.
       * @param logonIDs The found users' logonIDs.
       * @param sqlQuery A query to use, new is created if NULL.
       * @return Nbr users found.
       */
      uint32 getUINsFromUserIDKey( const UserIDKey* key, 
                                   vector<uint32>& users, 
                                   vector<MC2String>& logonIDs,
                                   CharEncSQLQuery* sqlQuery = NULL );

      
      /**
       * Returns an unused id in a table.
       *
       * @param tableName The name of the table
       * @param colName   The name of the id column
       * @param instance  "Key space instance", used for US/Europe cluster
       * @return Returns an unused id. 0 if error.
       */
      uint32 getNewUniqueID(const char* tableName, const char* colName,
                            int instance = -1);
      
      /**
       * Adds UserUser data as ',' separated list.
       *
       * @param target String to add text to.
       * @param user The UserUser to add.
       * @param UIN The UIN of the user.
       * @param passwd The Password for the user.
       * @return the size of added data, not including terminating '\0'.
       */
      uint32 addUserUserData( char* target, UserUser* user,
                              uint32 UIN, const char* passwd );
      
      
      /**
       * Returns an unused cellularID
       *       
       * @return Returns an unused cellularID or 0 if error.
       */
      uint32 getNewCellularID();
      
      
      /**
       * Returns an unused licenceID
       *       
       * @return Returns an unused licenceID or 0 if error.
       */
      uint32 getNewLicenceID();


      /**
       * Returns an unused region access id.
       *       
       * @return Returns an unused region access id or 0 if error.
       */
      uint32 getNewRegionAccessID();
      
      
      /**
       * Adds UserCellular data as ',' separated list.
       *
       * @param target String to add text to.
       * @param ID The ID of the cellular.
       * @param UIN The UIN of the user owning this cellular.
       * @param cellular The UserCellular to add.
       * @return the size of added data, not including terminating '\0'.
       */
      uint32 addUserCellularData( char* target, uint32 ID, uint32 UIN, 
                                  UserCellular* cellular );


      /**
       * Adds UserLicenceKey data as ',' separated list.
       *
       * @param target String to add text to.
       * @param ID The ID of the licence.
       * @param UIN The UIN of the user owning this licence.
       * @param licence The UserLicenceKey to add.
       * @return The size of added data, not including terminating '\0'.
       */
      uint32 addUserLicenceData( char* target, uint32 ID, uint32 UIN, 
                                 UserLicenceKey* licence );


      /**
       * Adds UserRegionAccess data as ',' separated list.
       *
       * @param target String to add text to.
       * @param ID The ID of the region access.
       * @param UIN The UIN of the user owning this region access.
       * @param access The UserRegionAccess to add.
       * @return The size of added data, not including terminating '\0'.
       */
      uint32 addUserRegionAccessData( char* target, uint32 ID, uint32 UIN, 
                                      UserRegionAccess* access );


      /**
       * Adds UserRight data as ',' separated list.
       *
       * @param target String to add text to.
       * @param ID The ID of the right.
       * @param UIN The UIN of the user owning this right.
       * @param right The UserRight to add.
       * @return The size of added data, not including terminating '\0'.
       */
      uint32 addUserRightData( char* target, uint32 ID, uint32 UIN, 
                               UserRight* right );


      /**
       * Adds UserWayfinderSubscription data as ',' separated list.
       *
       * @param target String to add text to.
       * @param ID The ID of the wayfinder subscription.
       * @param UIN The UIN of the user owning this wayfinder subscription.
       * @param access The UserWayfinderSubscription to add.
       * @return The size of added data, not including terminating '\0'.
       */
      uint32 addUserWayfinderSubscriptionData( 
         char* target, uint32 ID, uint32 UIN, 
         UserWayfinderSubscription* wfsubscr );


      /**
       * Adds UserToken data as ',' separated list.
       *
       * @param target String to add text to.
       * @param ID The ID of the token.
       * @param UIN The UIN of the user owning this token.
       * @param token The UserToken to add.
       * @return The size of added data, not including terminating '\0'.
       */
      uint32 addUserTokenData( char* target, uint32 ID, uint32 UIN, 
                               UserToken* token );


      /**
       * Adds UserPIN data as ',' separated list.
       *
       * @param target String to add text to.
       * @param ID The ID of the PIN.
       * @param UIN The UIN of the user owning this PIN.
       * @param PIN The UserPIN to add.
       * @return The size of added data, not including terminating '\0'.
       */
      uint32 addUserPINData( char* target, uint32 ID, uint32 UIN, 
                             UserPIN* PIN );


      /**
       * Adds UserIDKey data as ',' separated list.
       *
       * @param target String to add text to.
       * @param ID The ID of the element.
       * @param UIN The UIN of the user owning this element.
       * @param el The UserIDKey to add.
       * @return The size of added data, not including terminating '\0'.
       */
      uint32 addUserIDKeyData( char* target, uint32 ID, uint32 UIN, 
                               UserIDKey* el );


      /**
       * Adds UserLastClient data as ',' separated list.
       *
       * @param target String to add text to.
       * @param ID The ID of the element.
       * @param UIN The UIN of the user owning this element.
       * @param el The UserLastClient to add.
       * @return The size of added data, not including terminating '\0'.
       */
      uint32 addUserLastClientData( char* target, uint32 ID, uint32 UIN, 
                                    UserLastClient* el );


      /**
       * Adds UserUser changed data as "fieldname" = value ',' separated 
       * list
       * @return the size of the added data, not includeing the terminating
       *         '\0' byte.
       */
      uint32 addChangedUserUserData( char* target, UserUser* user );
      

      /**
       * Adds UserCellular changed data as "fieldname" = value ',' 
       * separated list
       * @return the size of the added data, not includeing the terminating
       *         '\0' byte.
       */
      uint32 addChangedUserCellularData( char* target, 
                                         UserCellular* cellular );


      /**
       * Adds UserLicenceKey changed data as "fieldname" = value ',' 
       * separated list
       * @return the size of the added data, not includeing the terminating
       *         '\0' byte.
       */
      uint32 addChangedUserLicenceData( char* target, 
                                        const UserLicenceKey* licence );


      /**
       * Adds UserRegionAccess changed data as "fieldname" = value ',' 
       * separated list
       * @return the size of the added data, not includeing the terminating
       *         '\0' byte.
       */
      uint32 addChangedUserRegionAccessData( char* target, 
                                             UserRegionAccess* access );


      /**
       * Adds UserRight changed data as "fieldname" = value ',' 
       * separated list
       * @return the size of the added data, not includeing the terminating
       *         '\0' byte.
       */
      uint32 addChangedUserRightData( char* target, UserRight* right );


      /**
       * Adds UserWayfinderSubscription changed data as 
       * "fieldname" = value ',' separated list.
       *
       * @return the size of the added data, not includeing the terminating
       *         '\0' byte.
       */
      uint32 addChangedUserWayfinderSubscriptionData( 
         char* target, UserWayfinderSubscription* subscr );


      /**
       * Adds UserToken changed data as "fieldname" = value ',' 
       * separated list
       * @return the size of the added data, not includeing the terminating
       *         '\0' byte.
       */
      uint32 addChangedUserTokenData( char* target, UserToken* token );


      /**
       * Adds UserPIN changed data as "fieldname" = value ',' 
       * separated list
       * @return the size of the added data, not includeing the terminating
       *         '\0' byte.
       */
      uint32 addChangedUserPINData( char* target, UserPIN* PIN );

      /**
       * Adds UserIDKey changed data as "fieldname" = value ',' 
       * separated list
       * @return the size of the added data, not includeing the terminating
       *         '\0' byte.
       */
      uint32 addChangedUserIDKeyData( char* target, 
                                      const UserIDKey* el ) const;

      /**
       * Adds UserLastClient changed data as "fieldname" = value ',' 
       * separated list
       * @return the size of the added data, not includeing the terminating
       *         '\0' byte.
       */
      uint32 addChangedUserLastClientData( char* target, 
                                           UserLastClient* el );

      
      /**
       * Returns an unused BuddyListID
       * @return An unused buddyListID, 0 if error.
       */ 
      uint32 getNewBuddyListID();

      
      /**
       * Adds UserBuddyList data as ',' separated list.
       * @return the size of added data, not including terminating '\0'.
       */
      uint32 addUserBuddyListData( char* target, UserBuddyList* buddy );
      
      
      /**
       * Adds UserBuddyList changed data as "fieldname" = value ',' 
       * separated list
       * @return the size of the added data, not includeing the terminating
       *         '\0' byte.
       */
      uint32 addChangedUserBuddyListData( char* target, 
                                          UserBuddyList* buddy );

      
      /**
       * Adds debit data vales into target
       * @return the size of the added data, not includeing the terminating
       *         '\0' byte.
       */
      uint32 addSMSDebitData( char* target, uint32 debitTime, 
                              const char* phoneNumber, 
                              const char* serverPhoneNumber, 
                              uint32 messageID,
                              uint32 operationType, uint32 sentSMS,
                              const char* operationDescription );


      /**
       * Adds debit data vales into target
       * @return the size of the added data, not includeing the terminating
       *         '\0' byte.
       */
      uint32 addDebitData( char* target, 
                           uint32 debitTime, 
                           uint32 UIN,
                           const char* serverID, 
                           const char* userOrigin, 
                           uint32 messageID,
                           uint32 debInfo,
                           uint32 operationType, 
                           uint32 sentSize,
                           const char* operationDescription );


      /**
       * Asks the database for cellular phone models and retuns them.
       */
      CellularPhoneModels* getCellularPhoneModels
         ( uint32 nbrCellularModels, CellularPhoneModel* cellular);


      /**
       * Ask the database for a cellular phone model and returns it.
       */
      CellularPhoneModel* getCellularPhoneModel( const char* modelName );

      
      /**
       * Adds CellularModel data as a ',' separated list.
       * @return  the size of the added data, not includeing the 
       *          terminating '\0' byte.
       */
      uint32 addCellularModelData( char* target, 
                                   CellularPhoneModel* model );
      

      /**
       * Turns an unix time, seconds since 0:00 1 Jan 1970 into a
       * SQL DATE and and INT representing the seconds since midnight. 
       * Eg. '2000-01-21' and 32465  ('09:01:50)'.
       * @param time is the unix time.
       * @param date is where to print the date, at least 12 chars long.
       * @param time is where to print the time, at least 5 chars long.
       * @return length of string added
       */
      int makeSQLDate( uint32 time, char* dateStr, char* timeStr );
      
      
      /**
       * Set a session value.
       * @param session The session to alter.
       * @param name The name of the sessionvalue.
       * @param value The value of the sessionname.
       * @return true if updated succesfully, false if not.
       */
      bool setSessionValue( const char* session,
                            const char* name, 
                            const char* value );


      /**
       * Make a session row.
       * @param UIN User of session.
       * @param now Time in UTC.
       * @param id Filled with the ID of the session. 
       *        At least 31 chars long.
       * @param key Filled with the KEY of the session.       
       *        At least 31 chars long.
       * @return true if inserted ok, false if not.
       */
      bool makeSession( uint32 UIN, uint32 now, char* id, char* key );
      

      /**
       * Verify a session and update last access time.
       * @param sessionID  The sessionID.
       * @param sessionKey The sessionKey
       * @param updateAccess If last access time should be updated.
       * @param UIN of user or 0 if no such session, MAX_UINT32 if session
       *         has expired and not changed if verify falied.
       * @param updateAccess If session use time may be updated, default
       *                     true.
       * @param checkExpired If to check 
       * @return If verify succeded and UIN is set.
       */
      bool verifySession( const char* sessionID, const char* sessionKey,
                          uint32& UIN,
                          bool updateAccess = true,
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
       *         left and check is true and NOTOK if error.
       */
      StringTable::stringCode getAndCheckTransactionDays( 
         uint32 UIN, bool check, int32& nbrTransactionDays, 
         uint32& curTime );

      /**
       * Set userUIN for a id in ISABUserLicence.
       *
       * @param uin The uin to set in the key.
       * @param ownerUIN The current owner of the key.
       * @param changerUIN The UIN of the user changing.
       * @param licence The key with the id to change uin for.
       * @param sqlQuery A query to use.
       * @return If ok false if not.
       */
      bool setUserForIDinUserLicence( uint32 uin, uint32 ownerUIN,
                                      uint32 changerUIN,
                                      const UserLicenceKey* licence,
                                      CharEncSQLQuery* sqlQuery );

      /**
       * Delete licence with id from ISABUserLicenceKey.
       *
       * @param uin The uin for the key.
       * @param changerUIN The UIN of the user changing.
       * @param licence The key with the id to delete.
       * @param sqlQuery A query to use.
       * @return If ok false if not.
       */
      bool deleteUserLicence( uint32 uin, uint32 changerUIN,
                              const UserLicenceKey* licence,
                              CharEncSQLQuery* sqlQuery );

      /**
       * Delete licence with id from ISABUserLicence.
       *
       * @param uin The uin for the key.
       * @param changerUIN The UIN of the user changing.
       * @param licence The key with the id to delete.
       * @param sqlQuery A query to use.
       * @return If ok false if not.
       */
      bool deleteOldUserLicence( uint32 uin, uint32 changerUIN,
                                 const UserLicenceKey* licence,
                                 CharEncSQLQuery* sqlQuery );

      /**
       * Add a licence key.
       *
       * @param uin The uin for the key.
       * @param changerUIN The UIN of the user changing.
       * @param licence The key to add.
       * @param sqlQuery A query to use.
       * @return If ok false if not.
       */
      bool addUserLicence( uint32 uin, uint32 changerUIN,
                           UserLicenceKey* licence, 
                           CharEncSQLQuery* sqlQuery );

      /**
       * Add the changes from licence to database.
       *
       * @param uin The uin for the key.
       * @param changerUIN The UIN of the user changing.
       * @param licence The key to add changes for.
       * @param sqlQuery A query to use.
       * @return If ok false if not.
       */
      bool changeUserLicence( uint32 uin, uint32 changerUIN,
                              const UserLicenceKey* licence, 
                              CharEncSQLQuery* sqlQuery );


      /**
       * Delete ID key with id from ISABUserIDKey.
       *
       * @param uin The uin for the key.
       * @param changerUIN The UIN of the user changing.
       * @param key The key with the id to delete.
       * @param sqlQuery A query to use.
       * @return If ok false if not.
       */
      bool deleteUserIDKey( uint32 uin, uint32 changerUIN,
                            const UserIDKey* key,
                            CharEncSQLQuery* sqlQuery );

      /**
       * Set userUIN for a id in ISABUserIDKey.
       *
       * @param uin The uin to set in the key.
       * @param ownerUIN The current owner of the key.
       * @param changerUIN The UIN of the user changing.
       * @param key The key with the id to change uin for.
       * @param sqlQuery A query to use.
       * @return If ok false if not.
       */
      bool setUserForIDinUserIDKey( uint32 uin, uint32 ownerUIN,
                                    uint32 changerUIN,
                                    const UserIDKey* key,
                                    CharEncSQLQuery* sqlQuery );

      /**
       * Add an id key.
       *
       * @param uin The uin for the key.
       * @param changerUIN The UIN of the user changing.
       * @param key The key to add.
       * @param sqlQuery A query to use.
       * @return If ok false if not.
       */
      bool addUserIDKey( uint32 uin, uint32 changerUIN,
                         UserIDKey* idKey, 
                         CharEncSQLQuery* sqlQuery );

      /**
       * Remove a user from user cache.
       */
      void removeUserFromCache( uint32 uin );

      /**
       * Adds an entry into the userchangelog table.
       *
       * @param user The user that has changed.
       * @param changerUIN The UIN of the user changing.
       * @param sqlQuery The CharEncSQLQuery to use.
       */
      void addUserChangeToChangelog( UserUser* user, uint32 changerUIN,
                                     CharEncSQLQuery* sqlQuery );


      /**
       * Adds into the userregionchangelog table.
       *
       * @param user The user that has changed regions.
       * @param sqlQuery The CharEncSQLQuery to use.
       */
      void addUserRegionTolog( UserUser* user, CharEncSQLQuery* sqlQuery );


      /**
       * Adds into the userrightchangelog table.
       *
       * @param user The user that has changed right.
       * @param changerUIN The UIN of the user changing.
       * @param sqlQuery The CharEncSQLQuery to use.
       */
      void addUserRightTolog( UserUser* user, uint32 changerUIN,
                              CharEncSQLQuery* sqlQuery );


      /**
       * Adds into the usertokenchangelog table.
       *
       * @param user The user that has changed token.
       * @param changerUIN The UIN of the user changing.
       * @param sqlQuery The CharEncSQLQuery to use.
       */
      void addUserTokenTolog( UserUser* user, uint32 changerUIN,
                              CharEncSQLQuery* sqlQuery );


      /**
       * Adds into the userPINchangelog table.
       *
       * @param user The user that has changed PIN.
       * @param changerUIN The UIN of the user changing.
       * @param sqlQuery The CharEncSQLQuery to use.
       */
      void addUserPINTolog( UserUser* user, uint32 changerUIN,
                            CharEncSQLQuery* sqlQuery );


      /**
       * Adds into the UserIDKeychangelog table.
       *
       * @param user The user that has changed elements.
       * @param changerUIN The UIN of the user changing.
       * @param sqlQuery The CharEncSQLQuery to use.
       */
      void addUserIDKeyTolog( UserUser* user, uint32 changerUIN,
                              CharEncSQLQuery* sqlQuery );
   
      /**
       * Adds into the UserIDKeychangelog table.
       *
       * @param uin The user for the key.
       * @param key The key to save to change log.
       * @param changerUIN The UIN of the user changing.
       * @param sqlQuery The CharEncSQLQuery to use.
       */
      void addUserIDKeyTolog( uint32 uin, 
                              const UserIDKey* key,
                              uint32 changerUIN,
                              CharEncSQLQuery* sqlQuery );
      
      /**
       * Adds into the UserLastClientchangelog table.
       *
       * @param user The user that has changed elements.
       * @param changerUIN The UIN of the user changing.
       * @param sqlQuery The CharEncSQLQuery to use.
       */
      void addUserLastClientTolog( UserUser* user, uint32 changerUIN,
                                   CharEncSQLQuery* sqlQuery );


      /**
       * Adds into the UserLicencechangelog table.
       *
       * @param uin The user for the licence.
       * @param licence The licence to save to change log.
       * @param changerUIN The UIN of the user changing.
       * @param sqlQuery The CharEncSQLQuery to use.
       */
      void addOldUserLicenceTolog( uint32 uin, 
                                   const UserLicenceKey* licence, 
                                   uint32 changerUIN,
                                   CharEncSQLQuery* sqlQuery );


      /**
       * Adds into the UserLicenceKeychangelog table.
       *
       * @param uin The user for the licence.
       * @param licence The licence to save to change log.
       * @param changerUIN The UIN of the user changing.
       * @param sqlQuery The CharEncSQLQuery to use.
       */
      void addUserLicenceTolog( uint32 uin, 
                                const UserLicenceKey* licence, 
                                uint32 changerUIN,
                                CharEncSQLQuery* sqlQuery );


      /**
       *   Adds a table to the internal table vectors
       *   @param index The index to use
       *   @param name The name of the table
       *   @param createQuery The SQL query that creates the query
       *   @param extraQuery Any additional query to run after creation of
       *                     table (such as index creation)
       */
      void addTable(int index, const char* name, const char* createQuery,
                    const char* extraQuery = NULL);

      /**
       *   Initializes the table vectors
       *   @return The number of tables, 0 if something went wrong
       */
      int initTables();

      /**
       *   Checks the databse to make sure it can be used. Creates tables
       *   that do not exist.
       *
       *   @param noSqlUpdate If not to update sql tables.
       *   @return true if successfull
       */
      bool initialCheckDatabase( bool noSqlUpdate );

     
      /**
       * Add default User ISABUser.
       */
      bool addDefaultUser();


      /**
       * Add default Right.
       */
      bool addDefaultRight();

  
      /**
       *   Adds a phone model to the ISABCellularModels table
       *   @param model Pointer to a CellularPhoneModel to add
       *   @return true if successfull
       */
      bool addPhoneModel(CellularPhoneModel* model);

      /**
       * Adds default phone models to CellularModels table
       */
      bool addDefaultPhoneModels();

      /**
       * Only to be used in database check in initialCheckDatabase.
       */
      bool checkForColumnInISABUserUser( 
         CharEncSQLQuery* sqlQuery, const char* columnName, 
         const char* columnDefinition, const char* afterColumnName );

      /**
       * Only to be used in database check in initialCheckDatabase.
       */
      bool checkForColumnInISABUserUINTable( 
         CharEncSQLQuery* sqlQuery, const MC2String& tableName,
         const char* columnName, 
         const char* columnDefinition, const char* afterColumnName,
         bool& added );

      /**
       * Write a route storage string.
       */
      bool makeRouteStorageString( char* s, const char* preFix,
                                   const RouteReplyPacket* routePack,
                                   uint32 routeID,
                                   uint32 createTime,
                                   uint32 UIN,
                                   const char* const extraUserinfo,
                                   uint32 validUntil,
                                   int32 originLat,
                                   int32 originLon,
                                   uint32 originMapID,
                                   uint32 originItemID,
                                   uint16 originOffset,
                                   int32 destinationLat,
                                   int32 destinationLon,
                                   uint32 destinationMapID,
                                   uint32 destinationItemID,
                                   uint16 destinationOffset ) const;

      /**
       * Make a route storage path string. Uses ROUTE_STORAGE_PATH 
       * propery.
       *
       * @param routeID The routeID.
       * @param createTime The createTime.
       * @return The file path or empty string if error.
       */
      MC2String makeRouteStoragePath( uint32 routeID,
                                      uint32 createTime ) const;

      /**
       * Makes a byte buffer from the stored route packet in buff.
       *
       * @param buff The buffer to make route buffer from.
       * @param routePacketLength The expected length of the route buffer.
       * @return The route packet buffer or NULL if not ok data.
       */
      byte* storedRouteToBuffer( const char* buff, 
                                 uint32 routePacketLength,
                                 uint32 routeID,
                                 uint32 createTime ) const;

      /**
       * Adds a column to m_isabuseruser.
       */
      void setISABUserUserColumnName( const MC2String& name, bool isNumeric, 
                                      bool isKey );

      /**
       * Get an INT column for a UIN from ISABUserUser.
       */
      bool getISABUserUserColumnValue( 
         CharEncSQLQuery* sqlQuery, const MC2String& whereTag, uint32 uin,
         const MC2String& column, int32& number ) const;

      /**
       * Set an INT column for a UIN from ISABUserUser.
       */
      bool setISABUserUserColumnValue( 
         CharEncSQLQuery* sqlQuery, const MC2String& whereTag, uint32 uin,
         uint32 changerUIN, const MC2String& column, int32 number );
      
      /**
       * Gets a logon ID from a given UIN.
       * @param sqlQuery   The query to use.
       * @param UIN        The user for which to find the logon ID.
       * @param logonID    Out parameter for the logon ID.
       * @return wether finding the logonID was successful.  
       */
      bool getLogonIDForUIN( CharEncSQLQuery* sqlQuery,
                             uint32 UIN,
                             MC2String& logonID );
      
      /**
       * Creates the tables which this module expects to find 
       * if they don't exist.
       */
      bool createNonExistentTables();
      
      /**
       * Updates an old database to the current format, for instance
       * by adding new columns etc.
       */
      void sqlUpdate();
      
      /**
       * Does some testing on the database.
       */
      bool testDatabase();

      /** The database connection */
      CharEncSQLConn* m_sqlConnection;      
      
      /**
       *   The number of tables we use
       */
      int m_numTables;

      /**
       *   The names of the tables we use
       */
      char** m_tableNames;

      /**
       *   The SQL queries used to create our tables
       */
      char** m_tableCreateQueries;

      /**
       *   The SQL queries used in addition to table creation if applicable
       */
      char** m_tableExtraQueries;

      /**
       * The cached user sessions.
       */
#ifdef PARALLEL_USERMODULE
      SessionCache* m_sessionCache;
#else
      UserSessionCache* m_userSessionCache;
#endif

      /**
       * The cached user logins.
       */
#ifdef PARALLEL_USERMODULE
      SessionCache* m_loginCache;
#else
      UserSessionCache* m_userLoginCache;
#endif

      /**
       * The cache to cache UserItems in.
       */
      Cache* m_cache;


      /**
       * If to use user cache.
       */
      bool m_useUserCache;


      typedef multiset< DebitData > debitSet;
      /**
       * The currently stored debits waiting to be inserted.
       */
      debitSet m_debitSet;

      /** The table data for stored user data */
      auto_ptr< SQLTableData > m_storedUserTableData;

      /// The ISABUserUser table.
      auto_ptr< SQLTableData > m_isabuseruser;
      
      /// Set to true after the initial database check has been done.
      bool m_doneInitialDatabaseCheck;
      
      /// Used to check whether this module is leader
      const LeaderStatus* m_leaderStatus;
      
      /// Whether the initial database check should modify the database
      bool m_noSqlUpdate;
};


inline void
UserProcessor::setUseUserCache( bool useUserCache ) {
   m_useUserCache = useUserCache;
}

#endif

