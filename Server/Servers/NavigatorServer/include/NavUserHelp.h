/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVUSERHELP_H
#define NAVUSERHELP_H


#include "config.h"
#include "InterfaceParserThreadConfig.h"
#include "MC2Coordinate.h"
#include "NavHandler.h"
#include "WFSubscriptionConstants.h"
#include "UserEnums.h"
#include "ExtraAddInfo.h"
#include "NavRequestData.h"

class UserLicenceKey;
class UserItem;
class ClientSetting;
class UserUser;
class UserElement;
class isabBoxSession;


/**
 * Class with help functions for user handling.
 *
 */
class NavUserHelp : public NavHandler {
   public:
      /**
       * Constructor.
       */
      NavUserHelp( InterfaceParserThread* thread,
                   NavParserThreadGroup* group );


      /**
       * Get the User Identification Number (UIN) from a identification
       * string. UIN, User logonID and phonenumber is supported values
       * of identification string.
       *
       * @param identification The identification string may be user 
       *        UIN, logonID or phonenumber.
       */
      uint32 getUIN( const char* identification );


      /**
       * Store the users position.
       *
       * @param UIN The user to store for.
       * @param pos The coordine to store.
       * @param source The source of the position.
       */
      void storeUserPosition( uint32 UIN, MC2Coordinate pos,
                              const char* source = "" );


      /**
       * Store the users position.
       *
       * @param UIN The user to store for.
       * @param lat The latitude to store, in MC2 coordinates.
       * @param lon The longitude to store, in MC2 coordinates.
       * @param navID The navigatorID, default 0.
       * @param identification The identifying string. Not used.
       */
      void storeUserPosition( uint32 UIN, int32 lat, int32 lon, 
                              uint32 navID = 0, 
                              const char* identification = "",
                              const char* source = "" );

      /**
       * Prints a licence key on out.
       */
      static ostream& printLicence( ostream& out, 
                                    const uint8* key, uint32 len );

      /**
       * Prints a licence key on out. If patamID is not in params nothing
       * is printed.
       */
      static ostream& printLicence( ostream& out, uint16 paramID,
                                    NParamBlock& params );


      /**
       * Appends a licence key on out.
       */
      static MC2String& printLicence( MC2String& out, 
                                      const uint8* key, uint32 len );

      /**
       * Prints a user licence on out.
       */
      static ostream& printLicence( ostream& out, 
                                    const UserLicenceKey& licence );

      /**
       * Appends a user licence on out.
       */
      static MC2String& printLicence( MC2String& out, 
                                      const UserLicenceKey& licence );

      /**
       * Prints a set of licence keys on out.
       */
      static ostream& printLicences( ostream& out, 
                                     const UserLicenceKeyVect& hwKeys,
                                     const MC2String& prefix = "" );

      /**
       * Appends a licence key on out.
       */
      static MC2String& printLicences( MC2String& out, 
                                       const UserLicenceKeyVect& hwKeys,
                                       const MC2String& prefix = "" );  

      /**
       * Checks if the licence matches the user, possibly adds it. 
       * 
       * @param userItem The user to check licence for.
       * @param key The licence bytes.
       * @param keyLength The length of key.
       * @param userName The logonID for the user. Used if dev key.
       * @param userPassword The logonPaswd for the user. Used if dev key.
       * @param changeToUserItem Set to a new UserItem to use.
       * @param noaddLicence Don't add licence under any circumstances.
       * @return 0 if ok, 1 if unauthorized, 2 if timeout, 3 if error,
       *         4 if other user has bk and unauthorized, 5 if change to
       *         other user(changeToUserItem set to newed UserItem).
       */
      int checkUserLicence( UserItem* userItem, const byte* key, 
                            uint32 keyLength, const char* userName,
                            const char* userPassword,
                            UserItem*& changeToUserItem,
                            bool noaddLicence = false );


      typedef vector< pair< UserEnums::URType, ExtraAddInfo > > ExtraTypes;
      typedef vector< UserElement* > UserElementVector;
      
      /**
       * Returns the user rights mask.
       */
      UserEnums::URType getUrMask( const ClientSetting* clientSetting );

      /**
       * Returns the user rights level.
       */
      UserEnums::userRightLevel getUrLevel( const ClientSetting* clientSetting );

      /**
       * Checks the user's Wayfidner Subscription Type. 
       *
       * @param wayfinderType The WFST sent from client.
       * @param userItem The user.
       * @param clientSetting The client settings.
       * @param userwfst Set to the user's WFST.
       * @return 0 - All ok no nothing, 1 - if unauthorized,
       *         2 - if timeout, 3 - if error, 4 Send userwfst to client.
       */
      int checkWFST( byte wayfinderType, UserItem* userItem, 
                     const ClientSetting* clientSetting,
                     byte& userwfst );

      /**
       * Makes a string with some extra information about user from params.
       */
      MC2String makeExtraUserInfoStr( const NParamBlock& params ) const;


      /**
       * Upgrades a user.
       *
       * @param changeToUserItem Set to the user to change to, caller must
       *                         release it.
       * @param mayChangeUser If it is ok to change user (by setting
       *                      changeToUserItem).
       * @param serverStr Set to the server name to use when status is 
       *                  REDIRECT.
       */
      uint8 handleNavUpgrade( const char* actCode,
                              uint32 topRegionID,
                              const char* phoneNumber,
                              UserUser* user,
                              const ClientSetting* clientSetting,
                              const uint32* cpvArray,
                              uint32 peerIP,
                              const UserLicenceKey& licenceKey,
                              byte& userWFType,
                              bool& topRegionOK,
                              bool& activationCodeOK,
                              bool& phonenumberOK,
                              UserItem*& changeToUserItem,
                              bool mayChangeUser,
                              MC2String& serverStr );


      /**
       * Handles uploaded files.
       *
       * @param userItem The user, may be NULL.
       * @param v The uploaded data.
       * @param key The licence key.
       * @param keyLength The length of key.
       */
      void handleUploadFiles( UserItem* userItem, const vector<byte>& v,
                              const byte* key, uint32 keyLength );


      /**
       * Removes upload files from user.
       *
       * @param userItem The user.
       * @param files Strings with the file names.
       */
      void removeUploadFiles( UserItem* userItem, 
                              const vector<MC2String>& files );


      /**
       * Get the centerpoint, and scale, for a user's regions.
       *
       * @param userItem The user.
       * @param scale Set to the scale of the centerpoint.
       * @return The centerpoint.
       */
      MC2Coordinate getCenterpointFor( UserItem* userItem, uint32& scale );


      /**
       * Get user from params, using paramID 9,8,1.
       */
      bool getUserFromParams( NParamBlock& params, MC2String& idStr,
                              UserItem*& userItem, bool useCache = true,
                              bool wipeFromCache = false );


      /**
       * Make PINs param.
       *
       * @param userItem The user.
       * @param p PINs appended to this.
       */ 
      void getUsersPINs( const UserItem* userItem, NParam& p );


      /**
       * Make PINs crc.
       *
       * @param userItem The user.
       */ 
      uint32 getUsersPINsCRC( const UserItem* userItem );

      /**
       * Add server list parameters.
       *
       * @param params To add params to.
       * @param settings The client setting.
       * @param http If using http encapsulated navigator proto.
       * @param addServerm If to add server list param not just crc.
       * @param printCRC If to print the crc with name before as hex without
       *                 newline.
       * @param fixedServerListName If set then this server list name is
       *        used.
       * @return If added server list or not.
       */
      bool addServerListParams( 
         NParamBlock& params,
         const ClientSetting& settings,
         bool http, bool addServerm, bool printCRC = false,
         const MC2String& fixedServerListName = "" ) const;


      /**
       * Get the client settings for the request.
       */
      const ClientSetting* getClientSetting( const NParamBlock& params,
                                             const UserUser* user ) const;


      /**
       * Save the changes in user and update the user in session.
       */
      bool updateSessionUser( UserUser* user, 
                              isabBoxSession* session );


      /**
       * Adds all licence/hardware keys to vector.
       *
       * @param params The block of parameters.
       * @param typeID The ID of the licence type paramter.
       * @param licenceID The ID of the licence paramter.
       * @param hwKeys The keys are added to this vector.
       * @param product The name of the product.
       * @return False if licence type is not a valid type or if type
       *         is not set for multi licence keys.
       */
      bool getLicencesAndTypes( const NParamBlock& params, uint16 typeID,
                                uint16 licenceID, 
                                vector< UserLicenceKey >& hwKeys,
                                const MC2String& product ) const;
};


#endif // NAVUSERHELP_H

