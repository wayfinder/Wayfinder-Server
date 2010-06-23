/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARSEREXTERNALAUTH_H
#define PARSEREXTERNALAUTH_H

#include "config.h"
#include "ParserHandler.h"
#include "MC2String.h"
#include "ParserUserHandler.h"

#include "PopupEnums.h"
#include "UserIDKey.h" // UserIDKey::idKey_t
#include "InterfaceRequestData.h"
#include "TimeUtility.h"

#include <set>

// Forward declarations.
class UserItem;
class SSL_CONTEXT;
class HttpHeader;
class InterfaceRequest;
class ClientSetting;
class UserUser;
class HttpInterfaceRequest;
class MC2Digest;
class ParserExternalAuthHttpSettings;


/**
 * Class holding the parameters to handleHttpHeaderRequest method.
 */
class handleHttpHeaderRequestParam {
public:
   handleHttpHeaderRequestParam( 
      InterfaceRequest* iireq, 
      const ParserExternalAuthHttpSettings* ipeh,
      InterfaceRequestData& ird,
      const MC2String& iactivationCode,
      const MC2String& iclientUserID, const MC2String& icurrentID,
      const MC2String& inewID, const UserItem* iclientUser,
      bool irecheckExernalAuth )
         : ireq( iireq ), peh( ipeh ), rd( ird ),
           activationCode( iactivationCode ), 
           clientUserID( iclientUserID ), currentID( icurrentID ),
           newID( inewID ), clientUser( iclientUser ), 
           recheckExernalAuth( irecheckExernalAuth )
      {
      }
   
   InterfaceRequest* ireq;
   const ParserExternalAuthHttpSettings* peh;
   InterfaceRequestData& rd;
   const MC2String& activationCode;
   const MC2String& clientUserID;
   const MC2String& currentID;
   const MC2String& newID;
   const UserItem* clientUser;
   bool recheckExernalAuth;
};


/**
 * Class for holding popup data for a client type.
 */
class popupData {
   public:
      /**
       * Constructor.
       */
   popupData( const MC2String& iclientTypeEnd, 
              StringTable::stringCode iexpired,
              StringTable::stringCode iwelcome, 
              StringTable::stringCode itrialMode, 
              StringTable::stringCode itrialMode2,
              const MC2String& ipopURL ) 
      : clientTypeEnd( iclientTypeEnd ), expired( iexpired ), 
        welcome( iwelcome ), trialMode( itrialMode ), 
        trialMode2( itrialMode2 ), popURL( ipopURL )
      {}

   MC2String clientTypeEnd;
   StringTable::stringCode expired;
   StringTable::stringCode welcome;
   StringTable::stringCode trialMode;
   StringTable::stringCode trialMode2;
   MC2String popURL;
};


/**
 * Class handling external authentication.
 *
 */
class ParserExternalAuth : public ParserHandler {
   public:
      /**
       * Constructor.
       */
      ParserExternalAuth( 
         ParserThread* thread,
         ParserThreadGroup* group );


      /**
       * Destructor.
       */
      virtual ~ParserExternalAuth();


      /**
       * Checks if a username is external auth type.
       * 
       * @return True if str is external auth type, false if not.
       */
      bool externalUserName( const MC2String& str ) const;


      /**
       * Checks if user is a external user.
       *
       * @param userItem The user item to check.
       * @return True if user is external auth, false if not.
       */
      bool isExternalUser( const UserItem* userItem ) const;


      /**
       * Checks if a username is an Http auth type.
       * 
       * @return True if str is Http auth type, false if not.
       */
      bool isHttpUserName( const MC2String& str ) const;


      /**
       * Checks if user is a external user with HttpHeaderRequest 
       * authentication.
       *
       * @param peh Set to the ParserExternalAuthHttpSettings of the
       *            user type if not NULL.
       */
      bool isHttpHeaderRequestUser( 
         const UserItem* userItem,
         const ParserExternalAuthHttpSettings** peh = NULL ) const;


      /**
       * Handles a HttpHeaderRequest authentication.
       *
       * @param ireq The request to authenticate.
       * @param peh The settings.
       * @param clientType The client identfier.
       * @param clientTypeOptions The client identfier options.
       * @param licenceKeys The licence keys of the client, empty if none.
       * @param programV The program version of the client, NULL if none.
       * @param activationCode The activation code used, empty if none.
       * @param status Set to the status of the reply. 
       *               -1 is NOT_OK.
       *               -2 is TIMEOUT.
       *               -3 is EXPIRED.
       *               -4 is UNAUTHORIZED.
       * @param setUserItem Set to a newed UserItem of the authenticated 
       *                    user.
       * @param clientUserID The user-id from the client.
       * @param currentID The current (hardware) id of the client.
       * @param newID     The next (hardware) id of the client. May be same
       *                  as currentID.
       * @param clientUser The user the client think it has, if any.
       */
      int handleHttpHeaderRequest( 
         const handleHttpHeaderRequestParam& p,
         int& status, UserItem*& setUserItem );


      /**
       * Handles a HttpHeaderRequest from a user or extType.
       * Parameters values same as in handleHttpHeaderRequest with 
       * ParserExternalAuthHttpSettings.
       * 
       * @param user The user to use, may be NULL, see setUserItem.
       * @param extType The type of external auth.
       */
      int handleHttpHeaderRequest( 
         InterfaceRequest* ireq, UserItem* user, const MC2String& extType,
         InterfaceRequestData& rd,
         const MC2String& activationCode,
         int& status, UserItem*& setUserItem, 
         const MC2String& clientUserID, const MC2String& currentID,
         const MC2String& newID );


      /**
       * Get the default HttpHeaderRequest settings for type.
       */
      const ParserExternalAuthHttpSettings* getHttpSetting( 
         const MC2String& type ) const;


      /**
       * Get matching HttpHeaderRequest settings for type.
       */
      const ParserExternalAuthHttpSettings* getNavHttpSetting( 
         const MC2String& type ) const;


      /**
       * Checks if the logonID starts with prefix + "-". 
       */
      static bool checkUserNamePrefix( const MC2String& prefix, 
                                       const MC2String& logonID );

      /**
       * Checks if the user's logonID starts with prefix + "-". 
       */
      static bool checkUserNamePrefix( const MC2String& prefix, 
                                       const UserItem* userItem );

      /**
       * @name Popup messages in clients methods.
       * @memo Methods for popup messages in clients.
       * @doc  Methods for popup messages in clients.
       */
      //@{
         /**
          * Method to get popup for a user and client type.
          *
          * @param user The user to get popup for.
          * @param rd The request data with client settings.
          * @param popOnce Set the show once message.
          * @param popMessage Set to the message to pop up in client.
          * @param popURL Set to the url to popup 
          */
         bool getPopup( const UserUser* user, 
                        const InterfaceRequestData& rd,
                        MC2String& popOnce,
                        MC2String& popMessage, MC2String& popURL,
                        PopupEnums::popup_url_t& popURLType ) const;
      //@}

      /**
       * Get the userHeader from Http field.
       *
       * @param hreq The HttpInterfaceRequest.
       * @param peh The settings.
       * @param userHeaderStr Set to be userHeader's values sometimes.
       * @return The userHeader.
       */
      const MC2String* getUserHeader( 
         const HttpInterfaceRequest* hreq,
         const ParserExternalAuthHttpSettings* peh,
         MC2String& userHeaderStr ) const;

   private:
#ifdef USE_SSL
      /// The ssl context.
      SSL_CONTEXT* m_ctx;
#endif


      /**
       * Get the contect of a node.
       */
      MC2String::size_type getNodeValue( const MC2String& name, 
                                         const MC2String& data,
                                         MC2String& value, 
                                         uint32 startPos = 0 ) const;

      /**
       * Get all in a node.
       */
      MC2String::size_type getAllInNode( const MC2String& name,
                                         const MC2String& data,
                                         MC2String& value,
                                         uint32 startPos = 0 ) const;

      /**
       * Get an attribute in a node.
       */
      int getAttribute( const MC2String& name,
                        const MC2String& data,
                        MC2String& value,
                        uint32 startPos = 0 ) const;


      /**
       * An externalAuthFunction method for IMSI users.
       */
      int externalIMSIAuth( uint32 IP, const handleHttpHeaderRequestParam& p,
                            MC2String& userID );

      /**
       * An externalAuthFunction method for type of hw key.
       */
      int externalHWKeyAuth( uint32 IP, const handleHttpHeaderRequestParam& p,
                             MC2String& userID,
                             const MC2String& keyType );

      /**
       * An externalAuthFunction method for iPhone device id type of hw key.
       */
      int externalDevIDAuth( uint32 IP, const handleHttpHeaderRequestParam& p,
                             MC2String& userID );


      /**
       * Checks if an ip matches.
       * 
       * @param IP the IP to match.
       * @param ipStr The IP as string, set by this method.
       * @param p The request parameters.
       * @return True if IP matches p's requirements.
       */
      bool ipmatches( uint32 IP, MC2String& ipStr, 
                      const handleHttpHeaderRequestParam& p );

      /**
       * Get a part of a string.
       */
      bool stringPart( const MC2String& str, const MC2String& partStart,
                       const MC2String& partEnd, MC2String& val ) const;

      /**
       * Get the IDKey of type and that starts with string.
       */
      const UserIDKey* getUserIDKey( 
         const UserUser* user,
         const char* str,
         UserIDKey::idKey_t type = UserIDKey::account_id_key ) const;

      /**
       * Get the non const IDKey of type and that starts with string.
       */
      UserIDKey* getUserIDKey( 
         UserUser* user, 
         const char* str, 
         UserIDKey::idKey_t type = UserIDKey::account_id_key ) const;

      /**
       * Checks if clientUser has been check recently.
       */
      void checkedRecently( const UserItem* clientUser, 
                            const handleHttpHeaderRequestParam& p,
                            uint32 now,
                            bool& accessNow,
                            bool& hasCheckedRecently );

      /**
       * Checks if userHeader is not NULL and if it should be 
       * encrypted and updates it and userHeaderStr.
       */
      void addEncryptionToUserHeader( 
         const MC2String*& userHeader,
         MC2String& userHeaderStr,
         const ParserExternalAuthHttpSettings* peh ) const;

      class HttpSettingsCmp {
        public:
         bool operator() ( const ParserExternalAuthHttpSettings* a,
                           const ParserExternalAuthHttpSettings* b ) const;
      };


      typedef set< ParserExternalAuthHttpSettings*, HttpSettingsCmp > 
         HttpSettings;


      /// The array of default HttpHeaderRequest settings by prefix.
      HttpSettings m_httpSettings;


      /// The ParserExternalAuthHttpSettings to find with.
      mutable ParserExternalAuthHttpSettings* m_findSettings;

      typedef map< MC2String, popupData > popupDataCont;

      /**
       * The popupdata map.
       */
      popupDataCont m_popupDatas;

      /**
       * The maker of digests.
       */
      auto_ptr<MC2Digest> m_mc2Digest;
};


// =======================================================================
//                                     Implementation of inlined methods =

#endif // PARSEREXTERNALAUTH_H

