/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARSERUSERHANDLER_H
#define PARSERUSERHANDLER_H

#include "config.h"
#include "ParserHandler.h"
#include "MC2String.h"
#include "UserEnums.h"
#include "ExtraAddInfo.h"
#include "InterfaceRequestData.h"
#include <set>

class UserItem;
class UserLicenceKey;
class UserElement;
class UserIDKey;
class UserUser;
class IPnPort;
class ClientSetting;
class UserRight;


/**
 * Class handling user operations.
 */
class ParserUserHandler : public ParserHandler {
   public:
      /**
       * Constructor.
       */
      ParserUserHandler( ParserThread* thread,
                         ParserThreadGroup* group );

      
      typedef vector< pair< UserEnums::URType, ExtraAddInfo > > ExtraTypes;
      typedef vector< UserElement* > UserElementVector;
            
      /**
       * The result of an operation.
       */
      enum errorCode {
         /// It is ok.
         OK          =  0,
         /// General error.
         ERROR       = -1,
         /// Timeout has occurred.
         TIMEOUT     = -2,
         /// The operation is not allowed.
         NOT_ALLOWED  = -3,
      };

      /**
       * Add a Wayfinder (trial) user.
       *
       * @param passwd Set to the password of the user.
       * @param userKey The licence to add user for, NULL if none.
       * @param userItem Set to the the new user.
       * @param startTime The start time of initial access.
       * @param endTime The end time of initial access. If same as 
       *                startTime then no start acces right is added.
       * @param regionID The region of initial access.
       * @param lang The language of the client.
       * @param clientType The clientType, NULL if none.
       * @param clientTypeOptions The clientTypeOptions, NULL if none.
       * @param programV Vector of three uint32.
       * @param activationCode The activation code used to create user,
       *                       empty if not so.
       * @param extraTypes Some extra types of UserRights to add.
       * @param createWFST The WFST to use, default TRIAL.
       * @param transactionDays The number of transaction days, default 0.
       * @param brand The brand of the user, defualt empty.
       * @param fixedUserName The fixed user name to use, default NULL.
       * @param extraComment Extra text added to operatorcomment.
       * @param extraElements Elements to add to user, elements are
       *                      owned by userItem after call.
       * @param userNamePrefix The explicitly set user name prefix, 
       *                       uses default if NULL. Overrides 
       *                       fixedUserName if not NULL.
       * @param email The email address to set in user, NULL if none.
       * @param name The name to set in user, split into first and last 
       *             name, NULL if none.
       * @param opt_in_name  The name of customer contact info to set.
       * @param opt_in_value The value to set for opt_in_name.
       * @param extraRights additional rights for user
       * @return 0 all is ok, 2 if timeout, 3 if error and 4 if an UserIDKey
       *         of type account isn't unique.
       */
      int createWayfinderUser( 
         MC2String& passwd,
         const UserLicenceKey* userKey, 
         UserItem*& userItem,
         uint32 startTime,
         uint32 endTime,
         uint32 regionID,
         LangTypes::language_t lang, 
         const char* clientType,
         const char* clientTypeOptions,
         const uint32* programV,
         const MC2String& activationCode,
         const ExtraTypes& extraTypes = ExtraTypes(),
         WFSubscriptionConstants::subscriptionsTypes createWFST = 
         WFSubscriptionConstants::TRIAL,
         int32 transactionDays = 0,
         const MC2String& brand = "",
         const char* fixedUserName = NULL,
         const char* extraComment = NULL,
         const UserElementVector& extraElements = UserElementVector(),
         const char* userNamePrefix = NULL,
         const char* email = NULL,
         const char* name = NULL, 
         const MC2String& opt_in_name = "",
         const MC2String& opt_in_value = "",
         const MC2String& extraRights = MC2String("") );

      /**
       * Add a Wayfinder (trial) user for a licence key (IMEI).
       *
       * @param passwd Set to the password of the user.
       * @param userKey The licence to add user for.
       * @param userItem Set to the the new user.
       * @param lang The language of the client.
       * @param clientType The clientType.
       * @param clientTypeOptions The clientTypeOptions.
       * @param programV Vector of three uint32.
       * @param activationCode The activation code used to create user,
       *                       empty if not so.
       * @param extraTypes Some extra types of UserRights to add, the 
       *                   uint32 is the endtime, using default if 0.
       * @param fixedUserName The fixed user name to use, default NULL.
       * @param extraComment Extra text added to operatorcomment.
       * @param extraElements Elements to add to user, elements are
       *                      owned by userItem after call.
       * @return 0 all is ok, 2 if timeout, 3 if error.
       */
      int createWayfinderUser( 
         MC2String& passwd,
         const UserLicenceKey* userKey, 
         UserItem*& userItem,
         LangTypes::language_t lang, 
         const char* clientType,
         const char* clientTypeOptions,
         const uint32* programV,
         const MC2String& activationCode = "",
         const ExtraTypes& extraTypes = ExtraTypes(),
         const char* fixedUserName = NULL,
         const char* extraComment = NULL,
         const UserElementVector& extraElements = UserElementVector() );

      /**
       * Add a Wayfinder (trial) user for a licence key.
       * Checks for hardware key activation code, IMEI:00440000113342.
       *
       * @param passwd Set to the password of the user.
       * @param server Set to the server name if redirect.
       * @param userKey The licence to add user for, NULL if none.
       * @param userItem Set to the the new user.
       * @param startTime The start time of initial access.
       * @param endTime The end time of initial access. If same as 
       *                startTime then no start acces right is added.
       * @param regionID The region of initial access.
       * @param lang The language of the client.
       * @param clientType The clientType, NULL if none.
       * @param clientTypeOptions The clientTypeOptions, NULL if none.
       * @param programV Vector of three uint32.
       * @param activationCode The activation code used to create user,
       *                       empty if not so.
       * @param extraTypes Some extra types of UserRights to add.
       * @param createWFST The WFST to use, default TRIAL.
       * @param transactionDays The number of transaction days, default 0.
       * @param brand The brand of the user, defualt empty.
       * @param fixedUserName The fixed user name to use, default NULL.
       * @param extraComment Extra text added to operatorcomment.
       * @param extraElements Elements to add to user, elements are
       *                      owned by userItem after call.
       * @param userNamePrefix The explicitly set user name prefix, 
       *                       uses default if NULL. Overrides 
       *                       fixedUserName if not NULL.
       * @param email The email address to set in user, NULL if none.
       * @param name The name to set in user, split into first and last 
       *             name, NULL if none.
       * @param opt_in_name  The name of customer contact info to set.
       * @param opt_in_value The value to set for opt_in_name.
       * @param extraRights additional rights for user
       * @param autoKey The licence to check for auto activation, NULL if none.
       * @return 0 all is ok, 2 if timeout, 3 if error, 4 if redirect,
       *         5 if may not create user.
       */
      int createWayfinderUserWithAutoAC( 
         MC2String& passwd,
         MC2String& server,
         const UserLicenceKey* userKey, 
         UserItem*& userItem,
         uint32 startTime,
         uint32 endTime,
         uint32 regionID,
         LangTypes::language_t lang, 
         const char* clientType,
         const char* clientTypeOptions,
         const uint32* programV,
         const MC2String& activationCode,
         const ExtraTypes& extraTypes = ExtraTypes(),
         WFSubscriptionConstants::subscriptionsTypes createWFST = 
         WFSubscriptionConstants::TRIAL,
         int32 transactionDays = 0,
         const MC2String& brand = "",
         const char* fixedUserName = NULL,
         const char* extraComment = NULL,
         const UserElementVector& extraElements = UserElementVector(),
         const char* userNamePrefix = NULL,
         const char* email = NULL,
         const char* name = NULL, 
         const MC2String& opt_in_name = "",
         const MC2String& opt_in_value = "",
         const MC2String& extraRights = MC2String(""),
         const UserLicenceKey* autoKey = NULL );


      /**
       * Add time from year, month and day. If all zero then inf. time.
       * 
       * @param years The number of years to add.
       * @param months The number of months to add.
       * @param days The number of days to add.
       * @param explicitDate The explicit date to return, not used if
       *                     MAX_UINT32.
       * @param now The time to add to.
       * @return The resulting time.
       */
      static uint32 addTime( int32 years, int32 months, int32 days, 
                             uint32 explicitDate, uint32 now );

      /**
       * Get the id and time from an UserIDKey.
       * This method first checks if it is an hardware_id_and_time.
       *
       * @param el The UserIDKey.
       * @param time Set to the time.
       * @param id Set to the hardware id.
       * @return True if el is hardware_id_and_time and has valid content.
       */
      static bool getHardwareIdAndTime( const UserIDKey* el, uint32& time,
                                        MC2String& id );

      /**
       * Get the id and time from an UserIDKey of type hardware_id_and_time
       * or service_id_and_time.
       *
       * @param el The UserIDKey.
       * @param time Set to the time.
       * @param id Set to the id.
       * @param res Set to the result of the check.
       * @return True if el is of right type and has valid content.
       */
      static bool getServiceIdResAndTime( const UserIDKey* el, uint32& time,
                                          MC2String& id, uint32& res );

      /**
       * Get a UserIDKey from a user of a certain type and key.
       *
       * @param user The user to find in.
       * @param type The UserIDKey::idKey_t type to find.
       * @param key The key to match.
       */
      static UserIDKey* getIDKey( const UserUser* user, uint32 type,
                                  const MC2String& key );

      /**
       * Get a UserIDKey from a user of a certain type and key.
       *
       * @param user The user to find in.
       * @param idKey The id key to match.
       * @return The matching idKey in user or NULL if no such id key.
       */
      static UserIDKey* getIDKey( const UserUser* user, 
                                  const UserIDKey* idKey );

      /**
       * Remove all UserIDKey of a certain type.
       *
       * @param user The user to remove UserIDKeys.
       * @param type The type from UserIDKey to remove.
       * @return The number of removed keys.
       */
      uint32 removeAllUseIDKeyOfType( UserUser* user, uint32 type );

      /**
       * Check and update a user's LastClient and LatestUsage if needed.
       *
       * @param user The user to check and update.
       * @param userItem Is released and set to an updated user if update
       *                 needed.
       * @param clientType The client type.
       * @param clientTypeOptions The client type options.
       * @param version The version string.
       * @param extra The extra information.
       * @param origin The IP of the client.
       * @return If updated userItem.
       */
      bool checkAndUpdateLastClientAndUsage( 
         const UserUser* user,
         UserItem*& userItem,
         const MC2String& clientType, 
         const MC2String& clientTypeOptions, 
         const MC2String& version, const MC2String& extra,
         const IPnPort& origin );

      /**
       * 
       * @param user The user to modify, this object is not modified.
       * @param clientSetting The client settings.
       * @param setUserItem Set to an updated user if update
       *                    needed.
       * @return True if ok false if error.
       */
      bool checkIronTrialTime( const UserUser* user, 
                               const ClientSetting* clientSetting,
                               UserItem*& setUserItem );

      /**
       * Make the password.
       *
       * @param passwd The string to set.
       */
      static void makePassword( MC2String& passwd );

      /**
       * Send a welcome email to user.
       *
       * @param user The user to send to.
       * @return 0 if all is ok, -1 if error, -2 on timeout.
       */
      int sendWelcomeEmail( const UserUser* user );

      /**
       * Get the favorite crc for a user.
       *
       * @param uin The user to get favorites crc for.
       * @return The crc for the user's favories.
       */
      uint32 getUserFavCRC( uint32 uin );

      /**
       * Makes sure licence is in user.
       * Does not change user.
       *
       * @param user The user that should have licence.
       * @param licence The licence.
       * @return 0 if ok, -1 if error, -2 if timeout and -3 if may not
       *         change licences.
       */
      int licenceTo( const UserUser* user, const UserLicenceKey* licence );

      /**
       * Get the oldest time for a user's rights.
       *
       * @param user The user to get oldest time of any right for.
       * @return The oldest time for any right. Returns MAX_UINT32 if
       *         user has no rights.
       */
      uint32 getOldestRightTime( const UserUser* user ) const;

      /**
       * Get the endtime for a user.
       *
       * @param user The user to get end time for.
       * @param clientSetting Settings for user.
       * @param levelmask The levels that are allowed in check.
       * @return The time when the user expire.
       */
      uint32 getEndTimeForUser( 
         const UserUser* user, const ClientSetting* clientSetting,
         UserEnums::userRightLevel levelmask ) const;

      /**
       * Get the number of days left for a user.
       *
       * @param user The user to get end time for.
       * @param clientSetting Settings for user.
       * @return The number of whole days left.
       */
      uint32 getNbrDaysLeftForUser( 
         const UserUser* user, const ClientSetting* clientSetting,
         uint32 maxNbrDays = 30 ) const;

      /**
       * Get a update user.
       *
       * @param userItem Set to a new up to date UserItem.
       * @return 0 If ok, -1 if error, -2 if timeout.
       */
      int updateUser( UserItem*& userItem );

      /**
       * The accepted types of licenceKey/hardwareKey types.
       */
      static const set< MC2String > validHardwareKeyTypes;

      /**
       * The licence/hardware key type IMEI. 
       */
      static const MC2String imeiType;

      /**
       * The licence/hardware key type IMSI. 
       */
      static const MC2String imsiType;

      /**
       * The licence/hardware key type for iPhone device id.
       */
      static const MC2String iPhoneDevIDType;

      /**
       * The licence/hardware key type for clients using MSISDN
       * as a hardware key when e.g. connecting over Wi-Fi.
       */
      static const MC2String customerMSISDNType;

      /**
       * Make a combined type and key for a hardware key.
       *
       * @param str The string to put the combined string into.
       * @param hardwareType The type of key.
       * @param hardwareKey The key string.
       */
      bool makeLicenceKeyStr( MC2String& str,
                              const MC2String& hardwareType, 
                              const MC2String& hardwareKey ) const;

      /**
       * Make a combined type and key for a hardware key.
       *
       * @param hwKey The Licence/Hardware key.
       */
      bool makeLicenceKeyStr( MC2String& str,
                              const UserLicenceKey& hwKey ) const;

      /**
       * Sets the best licence/hardware key from a list.
       */
      bool setBestLicenceKey( const vector< UserLicenceKey >& hwKeys, 
                              UserLicenceKey& hwKey );

      /**
       * Returns a matching UserLicenceKey from user.
       *
       * @param user The user to check.
       * @param userKey The key to find.
       * @return Pointer to a matching UserLicenceKey, may be NULL if no
       *         mathcing key.
       */
      UserLicenceKey* findUserLicenceKey( 
         UserUser* user, const UserLicenceKey* userKey ) const;

      /**
       * Returns a matching UserLicenceKey from user.
       *
       * @param user The user to check.
       * @param userKey The key to find.
       * @return Pointer to a matching UserLicenceKey, may be NULL if no
       *         mathcing key.
       */
      const UserLicenceKey* findUserLicenceKey( 
         const UserUser* user, const UserLicenceKey* userKey ) const;

      /**
       * Get the number of licence keys for a product that a user has.
       *
       * @param user The user too look in.
       * @param product The product to look for.
       * @return The number of licence keys with product.
       */
      int getNbrProductKeys( const UserUser* user, 
                             const MC2String& product ) const;

      /**
       * Get the licence keys for a product that a user has.
       *
       * @param user The user too look in.
       * @param product The product to look for.
       * @return A list with pointers to the licence keys with product in user.
       *         Keys are not copied.
       */
      ConstUserLicenceKeyPVect getProductKeys( const UserUser* user, 
                                               const MC2String& product ) const;

      /**
       * Get the licence keys for a product that a user has.
       *
       * @param user The user too look in.
       * @param product The product to look for.
       * @return A list with pointers to the licence keys with product in user.
       *         Keys are not copied.
       */
      UserLicenceKeyPVect getProductKeys( UserUser* user, 
                                          const MC2String& product ) const;

      /**
       * Makes an activation UserAgent string.
       *
       * @param clientType The clientType.
       * @param clientTypeOptions The clientTypeOptions.
       * @param cpvArray Client program version array, length 3 required.
       * @return The UserAgent string.
       */
      MC2String makeUserAgentString( const char* clientType,
                                     const char* clientTypeOptions,
                                     const uint32* cpvArray );

      /**
       * Get user from list of licence keys.
       */
      bool getUserFromLicenceKeys( const UserLicenceKeyVect& hwKeys,
                                   UserItem*& hardwareUser,
                                   uint32& nbrUsers,
                                   bool wipeFromCache = false );

      /**
       * Get the user with the UserIDKey of type account and with a
       * key value.
       *
       * @param idKeyValue The key value to find.
       * @param userItem Set to the user that matches elem or NULL if 
       *                 no such user.
       * @param nbrUsers   Set to the number of users found.
       * @param useCache   If the cache should be used, if true then
       *                   the UserItem must be returned by calling
       *                   releaseUserItem, if false the returned
       *                   UserItem must be delered by user. Default
       *                   false.
       * @param wipeFromCache If to clean all caches in server and module
       *                    from this user.
       * @return True if all communication with UserModule was ok,
       *         false if communication/database error.
       */
      bool getUserFromIDKeyValue( const MC2String idKeyValue, 
                                  UserItem*& userItem, uint32& nbrUsers,
                                  bool useCache = false,
                                  bool wipeFromCache = false );

      /**
       * Makes sure id key is in user.
       * Does not change user.
       *
       * @param user The user that should have the idKey.
       * @param idKey The id key.
       * @param removeIDkey Optional idKey to remove in the user.
       * @return Can return OK, ERROR or TIMEOUT.
       */
      errorCode idKeyTo( const UserUser* user, const UserIDKey* idKey,
                         const UserIDKey* removeIDkey = NULL  );

   /**
    * Get Stored User Data for a key and user.
    *
    * @param uin The user to get stored data for.
    * @param key The key of the data to get.
    * @param value Set to the value of the stored data for key.
    * @return 0 if key is found and value is set, -1 if error,
    *         -2 if timeout and 1 if no such key stored.
    */
   int getStoredUserData( uint32 uin, const MC2String& key,
                          MC2String& value );

   /**
    * Set Stored User Data for a user.
    *
    * @param uin The user to set stored data for.
    * @param key The key of the data to set.
    * @param value The value to set for the key.
    * @return 0 if data is set, -1 if error and
    *         -2 if timeout.
    */
   int setStoredUserData( uint32 uin, const MC2String& key,
                          const MC2String& value );

   /**
    * Check if this rights length is same as create time in settings.
    *
    * @param r The right to check.
    * @param clientSetting The setting to match r with.
    */
   bool sameTimeSpan( const UserRight* r,
                      const ClientSetting* clientSetting ) const;

private:
   /**
    * Check and if latest usage of the server needs to be updated
    * @param user The user to check.
    * @param clientType The client type.
    * @return If update is needed
    */
   bool checkLatestUsage(
      const UserUser*& user,
      const MC2String& clientType );
   
};


/// Method for printing UserLicenceKeys.
ostream& operator<<( ostream& out, const UserLicenceKey& ul );


#endif // PARSERUSERHANDLER_H

