/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARSEREXTERNALAUTHHTTPSETTINGS_H
#define PARSEREXTERNALAUTHHTTPSETTINGS_H

#include "config.h"
#include "MC2String.h"
#include "ParserUserHandler.h"
#include "StringUtility.h"
#include "ExtraAddInfo.h"


// Forward declarations
class ParserExternalAuth;
class ParserThread;
class SSL_CONTEXT;
class HttpInterfaceRequest;
class handleHttpHeaderRequestParam;


/**
 * Class that hold the http settings for external authentication.
 *
 */
class ParserExternalAuthHttpSettings {
public:
   /**
    * @param reqName Name to use in prints. Like "app"
    * @param userHeader The name of the header with the user id.
    *                   Like "x-up-subno". Used in order.
    * @param userHeaderListPartStart If a part like 
    *        "x-up-calling-line-id=" in
    *        "x-up-calling-line-id=466784444;".
    * @param userHeaderListPartEnd The end of id Like ";".
    * @param userHeaderListPrefix Prefix to add to userHeader value.
    *        Eg. country prefix to phone nbr "46" to 703122345.

    * @param logonIDHeader The name of the header with the logonid, if
    *                      empty then userHeader(user id) is used when
    *                      creating user's logonid.
    * @param logonIDHeaderListPartStart If logonID is part of header.
    * @param logonIDHeaderListPartEnd The end of id Like ";".
    * @param statusHeader The name of the header with the status to
    *                     check. Empty string disables check.
    * @param okStatus The value of statusHeader to be ok.
    * @param noStatusHeaderOK If statusHeader is set and still there
    *                         is no such header is it ok?
    * @param ips The IP of the request must match one of the IP stringss
    *            in ips using the uint16 number of bytes to compare,
    *            1-4 are valid number of bytes.
    * @param userloginprefix The prefix or suffix of user.
    * @param extraTypes The extra rights to add when creating user.
    * @param addLicence If to add licence sent by client to user.
    * @param addLicenceAsID If add licence as IDKey not BinaryKey.
    * @param idAsKey If to add the user id in http-header as a 
    *                id-key and identify user by this id-key.
    * @param addTime If to add rights automatically to user when 
    *                expired or not.
    * @param addOrigin The origin of the rights added to user.
    * @param addRegionID The regionID to add.
    * @param addTypes The rights to add.
    * @param addYears The number of years to add.
    * @param addMonths The number of monthss to add.
    * @param addDays The number of days to add.
    * @param addRegionID The regionID to check for and add rights for.
    * @param clientType The substring to look for when matching client 
    *                   types.
    * @param clientTypeEndsStr If clientType substring must be at the
    *                          end to match.
    * @param userNamePrefix The create user prefix, if non empty then
    *                       a logonID with this prefix and some random
    *                       chars is made.
    * @param extAuthFunc The function in ParserExternalAuth that can
    *                    authenticate a user.
    * @param extAuthFuncCheckTime The time between extAuthFunc checks,
    *                             0 means to always check.
    * @param userHeaderForServices The header to add the found userHeader
    *                              and value to for use by services.
    * @param keepUpdatedHWKey If to keep an updated idkey with the hw key.
    * @param 
    */



   /**
    * Method that.
    * 
    * @return 0 If all is ok, -3 if not authenticated, -1 if error,
    *         -2 if timeout, -4 if not billable.
    */
   typedef int (ParserExternalAuth::*externalAuthFunction)( 
      uint32 IP, const handleHttpHeaderRequestParam& p, MC2String& userID );

   /**
    * Method that is called before any auth is done.
    *
    * @return 0 If all is ok, -3 if not authenticated, -1 if error,
    *         -2 if timeout, -4 if not billable.
    */
   typedef int (ParserExternalAuth::*preAuthFunction)( 
      uint32 IP, handleHttpHeaderRequestParam& p, 
      const MC2String*& userHeader, MC2String& userHeaderStr,
      UserItem*& userItem, UserItem*& setUserItem,
      bool& setCheckTime );

   /**
    * Method that is called on an authenticated user.
    *
    * @param p The inputs.
    * @param userItem The authenticated user.
    * @param taintedUser Set this to true if userItem is modified and
    *        needs update before use.
    * @param status Set to error status.
    * @return 0 If all is ok, -1 if error and status is set to a value.
    */
   typedef int (ParserExternalAuth::*PostAuthFunction)( 
      const handleHttpHeaderRequestParam& p, 
      UserItem*& userItem, bool& taintedUser,
      int& status );

   /**
    * Method that creates a user.
    *
    * @param p
    * @param userID
    * @return
    */
   typedef int ( ParserExternalAuth::*CreateWFUserFunction )( 
      const handleHttpHeaderRequestParam& p,
      const MC2String& userID );

   /**
    * Method that verifies a userHeader.
    *
    * @param p The inputs.
    * @param IP The IP of the client to check.
    * @param userHeader Set to NULL if userHeader is not verfied.
    * @param userHeaderStr Cleared if userHeader is not verfied.
    * @return 0 if verfied ok, -4 if userHeader is not verfied.
    */
   typedef int (ParserExternalAuth::*VerifyIDFunction)( 
      const handleHttpHeaderRequestParam& p,
      uint32 IP, const MC2String*& userHeader, MC2String& userHeaderStr );

   /**
    * Method to encrypt user id.
    *
    * @param userID The users id.
    * @return The encrypted user id.
    */
   typedef MC2String ( ParserExternalAuth::*EncryptIDFunction )(
      const MC2String& userID ) const;

   
   /**
    * Method that can do things with the http parameters.
    *
    * @param p The http header requests.
    * @param IP the IP address of the client.
    * @param hreq The http request of the client.
    * @return 0.
    */
   typedef int ( ParserExternalAuth::*HttpHeaderFunction )(
      handleHttpHeaderRequestParam& p, 
      uint32 IP, HttpInterfaceRequest* hreq );

   typedef vector< pair< UserEnums::URType, ExtraAddInfo > > AddTypes;
   typedef vector<MC2String> userHeaderV;
   
   MC2String reqName;
   MC2String settingId;
   userHeaderV userHeader;
   userHeaderV userHeaderListPartStart;
   userHeaderV userHeaderListPartEnd;
   userHeaderV userHeaderListPrefix;
   userHeaderV logonIDHeader;
   userHeaderV logonIDHeaderListPartStart;
   userHeaderV logonIDHeaderListPartEnd;
   MC2String statusHeader;
   MC2String okStatus;
   bool noStatusHeaderOK;
   vector< pair<MC2String, uint16> > ips;
   MC2String userloginprefix;
   AddTypes extraTypes;
   bool addLicence;
   bool addLicenceAsID;
   bool idAsKey;
   bool addTime;
   MC2String addOrigin;
   uint32 addRegionID;
   AddTypes addTypes;
   uint32 addYears;
   uint32 addMonths;
   uint32 addDays;
   uint32 blockedDate;
   LangTypes::language_t lang;
   WFSubscriptionConstants::subscriptionsTypes createLevel;
   uint32 createYears;
   uint32 createMonths;
   uint32 createDays;
   uint32 createExplicitTime;
   uint32 createRegionID;
   int32 createTransactionDays;
   MC2String brand;
   MC2String clientType;
   bool clientTypeEndsStr;
   MC2String userNamePrefix;
   externalAuthFunction extAuthFunc;
   int32 extAuthFuncCheckTime;
   preAuthFunction preAuthFunc;
   VerifyIDFunction verifyIDFunction;
   CreateWFUserFunction createWFUserFunction;
   EncryptIDFunction encryptIDFunction;
   bool usesHwKeys;
   MC2String userHeaderForServices;
   bool keepUpdatedHWKey;
   HttpHeaderFunction httpHeaderFunction;
   PostAuthFunction postAuthFunction;

   bool operator < ( const ParserExternalAuthHttpSettings& o ) const {
      return StringUtility::strcasecmp( settingId.c_str(),
                                        o.settingId.c_str() ) < 0;
   }
};


#endif // PARSEREXTERNALAUTHHTTPSETTINGS_H

