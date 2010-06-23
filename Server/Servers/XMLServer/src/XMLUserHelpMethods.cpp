/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLParserThread.h"
#include "UserLicenceKey.h"
#include "ParserUserHandler.h"
#include "XMLAuthData.h"
#include "ClientSettings.h"

#ifdef USE_XML
#include "ParserActivationHandler.h"
#include "XMLAuthData.h"
#include "HttpInterfaceRequest.h"
#include "HttpHeader.h"
#include "HttpHeaderLines.h"


bool
XMLParserThread::getUserFromLicenceKeys( 
   const UserLicenceKeyVect& hwKeys,
   UserItem*& hardwareUser, 
   const MC2String& noSuchLicenceKeyErrorCode,
   MC2String& errorCode,
   MC2String& errorMessage,
   bool wipeFromCache )
{
   bool ok = true;
   uint32 nbrUsers = 0;
   if ( !getUserHandler()->getUserFromLicenceKeys( 
           hwKeys, hardwareUser, nbrUsers, wipeFromCache ) ) {
      if ( nbrUsers == 1 && hardwareUser == NULL ) {
         ok = false;
         errorCode = "-1";
         errorMessage = "Failed to get user";
      } else if( nbrUsers > 1 ) {
         // More than one user
         ok = false;
         errorCode = noSuchLicenceKeyErrorCode; 
         errorMessage = "Too many users for this hardware key.";
      } else {
         ok = false;
         errorCode = "-1";
         errorMessage = "Database error";
      }
   } else {
      // Ok
      if( nbrUsers == 0 ) {
         hardwareUser = NULL;
         mc2log << "XMLParserThread::getUserFromLicenceKey No user "
                << "found for licence keys." << endl;
      } else {
         mc2log << "XMLParserThread::getUserFromLicenceKey Found user "
                << hardwareUser->getUIN() << endl;
      }
   }

   return ok;
}


bool
XMLParserThread::createWayfinderUserForXML( 
   MC2String& errorCode, 
   MC2String& errorMessage,
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
   uint32 activationRegionID,
   const ParserUserHandler::ExtraTypes& extraTypes,
   WFSubscriptionConstants::subscriptionsTypes createWFST,
   int32 transactionDays,
   const MC2String& brand,
   const char* fixedUserName,
   const char* extraComment,
   const ParserUserHandler::UserElementVector& extraElements,
   const char* userNamePrefix,
   const char* email,
   const char* name, 
   const MC2String& opt_in_name,
   const MC2String& opt_in_value,
   const MC2String& extraRights,
   const UserLicenceKey* autoKey,
   bool checkAndUseAC )
{
   bool ok = true;

   int status = 0;

   MC2String rights;
   uint32 ownerUIN = 0;
   MC2String ac = activationCode;
   if ( checkAndUseAC && !ac.empty() ) {
      // Check if used and for redirect
      int res = getActivationHandler()->getActivationData( 
         ac.c_str(), rights, ownerUIN, server );
      if ( res == 0 ) {
         mc2dbg4 << "createWayfinderUserForXML Got activation data "
                 << rights << " Owner " << ownerUIN << endl;
         if ( !server.empty() ) {
            errorCode = "-211";
            ok = false;
         } else if ( ownerUIN != 0 ) {
            ac = "";
         }
      } else {
         mc2log << warn << "createWayfinderUserForXML Failed to get "
                << "activation data for " << MC2CITE(ac) << endl;
         ok = false;
         errorCode = "-1";
         errorMessage = "Problem with activation code "; 
         if ( res == -2 ) {
            errorCode = "-3";
            errorMessage.append( "Timeout" );
         } else if ( res == -5 ) {
            errorCode = "-302";
            errorMessage.append( "Bad activation code" );
         } // Else -1
      }
   }

   if ( ok ) {
      status = getUserHandler()->createWayfinderUserWithAutoAC(
         passwd, server, userKey, userItem, startTime, endTime, regionID, lang,
         clientType, clientTypeOptions, programV, ac, extraTypes,
         createWFST, transactionDays, brand, fixedUserName, extraComment,
         extraElements, userNamePrefix, email, name, 
         opt_in_name, opt_in_value, extraRights, autoKey );

      if ( status == 0 ) {
         if ( checkAndUseAC && !ac.empty() ) {
            // Add activation code to user
            auto_ptr<UserUser> cuser( new UserUser( *userItem->getUser() ) );
            MC2String userAgent;
            MC2String userInput;
            if ( m_irequest->getRequestHeader()->getHeaderValue( 
                    HttpHeaderLines::USER_AGENT ) != NULL ) 
            {
               userAgent = *m_irequest->getRequestHeader()->getHeaderValue(
                  HttpHeaderLines::USER_AGENT );
            }

            int res = getActivationHandler()->activateUser( 
               cuser.get(), m_authData->clientSetting, ac.c_str(),
               ""/*phoneNumber*/, activationRegionID,
               m_irequest->getPeer().getIP(), 
               userAgent.c_str(), userInput.c_str(), "UNKNOWN", ownerUIN,
               server, false/*allowSpecialCodes*/ );

            if ( res != 0 ) {
               ok = false;
               errorCode = "-1";
               errorMessage = "Failed to activate user ";
               mc2log << "createWayfinderUserForXML Failed to activate user "
                      << "with AC " << MC2CITE(ac) << " ";
               if ( res == -2 ) {
                  errorCode = "-3";
                  mc2log << "Timeout";
                  errorMessage.append( "Timeout" );
               } else if ( res == -3 ) {
                  errorCode = "-304";
                  mc2log << "Wrong phone number";
                  errorMessage.append( "Wrong phone number" );
               } else if ( res == -4 ) {
                  errorCode = "-303";
                  mc2log << "Used activation code. By (" << ownerUIN << ")";
                  errorMessage.append( "Used activation code" );
               } else if ( res == -5 ) {
                  errorCode = "-302";
                  mc2log << "Bad activation code";
                  errorMessage.append( "Bad activation code" );
               } else if ( res == -6 ) {
                  errorCode = "-305";
                  mc2log << "Extension not allowed";
                  errorMessage.append( "Extension not allowed" );
               } else if ( res == -7 ) {
                  errorCode = "-306";
                  mc2log << "Creation not allowed";
                  errorMessage.append( "Creation not allowed" );
               } else if ( res == -10 ) {
                  errorCode = "-302";
                  mc2log << "Must choose region, sending Bad "
                         << "activation code";
                  errorMessage.append( "Bad activation code" );
               } // Else just error
               mc2log << endl;
            }
         }
      } else if ( status == 2 ) {
         errorCode = "-3";
         errorMessage = "Timeout while creating user.";
         ok = false;
      } else if ( status == 4 ) {
         errorCode = "-211";
         ok = false;
      } else if ( status == 5 ) {
         errorCode = "-1";
         errorMessage = "May not create user.";
         ok = false;
      } else { // if ( status == 3 )
         errorCode = "-1";
         errorMessage = "Error while creating user.";
         ok = false;
      }
   } // End if ok

   return ok;
}


#endif // USE_XML

