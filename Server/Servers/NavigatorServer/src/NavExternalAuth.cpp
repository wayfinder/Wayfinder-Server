/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavExternalAuth.h"
#include "InterfaceParserThread.h"
#include "NavPacket.h"
#include "NavUserHelp.h"
#include "isabBoxSession.h"
#include "NavParserThreadGroup.h"
#include "ClientSettings.h"
#include "STLStringUtility.h"
#include "NavMessage.h"
#include "isabBoxInterfaceRequest.h"
#include "ParserExternalAuth.h"
#include "ParserExternalAuthHttpSettings.h"
#include "HttpInterfaceRequest.h"
#include "HttpHeader.h"
#include "UserData.h"



NavExternalAuth::NavExternalAuth( 
   InterfaceParserThread* thread,
   NavParserThreadGroup* group,
   NavUserHelp* userHelp )
      : NavHandler( thread, group ), m_userHelp( userHelp )
{
   m_expectations.push_back( 
      ParamExpectation( 1, NParam::String, 1, 255 ) );
   m_expectations.push_back( 
      ParamExpectation( 3, NParam::Byte_array, 1, 255 ) );
   m_expectations.push_back( ParamExpectation( 4, NParam::String ) );
}


bool
NavExternalAuth::isExternalAuth( NavRequestPacket* req,
                                 IsabBoxInterfaceRequest* ireq ) 
{
   return isHttpRequest( req );
}


#define recentIDKeyAndTime( caseA, caseB ) \
            MC2String idKey; \
            NavUserHelp::printLicence( idKey, rd.hwKey );\
            \
            for ( uint32 i = 0 ; i < user->getNbrOfType( \
                     UserConstants::TYPE_ID_KEY ) ; ++i ) \
            { \
               UserIDKey* t = static_cast< UserIDKey* >( \
                  user->getElementOfType( \
                     i, UserConstants::TYPE_ID_KEY ) );\
               if ( t->getIDType() == UserIDKey::hardware_id_and_time ) {\
                  int32 time = 0;\
                  MC2String::size_type findPos = t->getIDKey().find( ':' );\
                  if ( findPos != MC2String::npos ) {\
                     time = strtol( t->getIDKey().substr( \
                                       findPos + 1 ).c_str(), NULL, 0 );\
                  }\
                  if ( /*StringUtility::*/strncmp( idKey.c_str(), \
                                                   t->getIDKey().c_str(),\
                                                   idKey.size() ) == 0 ) \
                  {\
                     /* Same Hw.id.*/\
                     caseA;\
                  } else {\
                     /* Other Hw.id.*/\
                     caseB;\
                  }\
               }\
            }


int
NavExternalAuth::handleExternalAuth( NavRequestData& rd,
                                     bool& reusedConnection,
                                     MC2String& externalAuthName )
{
   // rd.req, rd.reply, rd.session, rd.ireq,

   int res = 0;
   // The HttpSetting
   const ParserExternalAuthHttpSettings* peh = NULL;

   // Time now 
   uint32 now = TimeUtility::getRealTime();

   // Check if unknown client type
   if ( rd.params.getParam( 4 ) != NULL &&
        strcmp( rd.clientSetting->getClientType(), "default" ) == 0 ) 
   {
      rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER );
      mc2log << info << "handleExternalAuth: Unknown client type \""
             << rd.clientType << "\" sending UNAUTHORIZED. " 
             << " Parameters: ";
      rd.params.dump( mc2log, true, true );
      res = -1;
   }

   // Blocked client type
   if ( rd.clientSetting->getBlockDate() < now ) {
      // Blocked no need to check more
      rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER );
      mc2log << info << "handleExternalAuth: Blocked client type "
             << rd.clientType << " Parameters: ";
      rd.params.dump( mc2log, true, true );
      res = -1;
   }

   if ( res != 0 ) {
      // Error set do nothing
   } else if ( res == 0 &&
               (peh = m_thread->getExternalAuth()->getNavHttpSetting(
                  rd.clientType )) != NULL )
   {
      // Handle the Http request
      externalAuthName = peh->settingId;
      res = handleHttpHeaderRequest( rd, reusedConnection, peh );
   } else {
      // Not external try normal
      res = -2;
   }

   return res;
}


int
NavExternalAuth::removeExternalAuthUserNames( NParamBlock& params ) const {
   int res = 0;

   // Remove all that starts with external  in param 1 and 8
   uint16 paramIDs[ 2 ] = { 1, 8 };
   for ( uint32 i = 0 ; i < sizeof( paramIDs ) / sizeof(*paramIDs) ; ++i )
   {
      if ( params.getParam( paramIDs[ i ] ) != NULL ) {
         MC2String p1 = params.getParam( paramIDs[ i ] )->getString(
            m_thread->clientUsesLatin1() );
         if ( m_thread->getExternalAuth()->externalUserName( p1 ) ) {
            // Remove!
            params.removeParam( paramIDs[ i ] );
         }
      }
   }

   // Some has changeable usernames and uses
   // uin to identify... param 1 userID, 9 UIN
   if ( params.getParam( 1 ) != NULL || params.getParam( 9 ) != NULL ) {
      // Possibly more to remove
      UserItem* userItem = NULL;
      bool commOk = true; // If true and NULL userItem then Comm. error
      MC2String idStr = "";
      uint32 startTime = TimeUtility::getCurrentTime();
      commOk = m_userHelp->getUserFromParams( params, idStr, userItem );
      uint32 endTime = TimeUtility::getCurrentTime();
      
      if ( !commOk ) {
         mc2log << warn << "removeExternalAuthUserNames: error getting "
                << "user, for external ids check, from " 
                << MC2CITE( idStr.c_str() ) << " error: ";
         if ( endTime - startTime > 3000 ) {
            res = -2;
            mc2log << "Timeout";
         } else {
            res = -1;
            mc2log << "Error";
         }
         mc2log << endl;
      }

      if ( commOk && userItem != NULL ) {
         if ( m_thread->getExternalAuth()->isExternalUser( userItem ) )
         {
            params.removeParam( 1 );
            params.removeParam( 9 );
         }
      }
      
      m_thread->releaseUserItem( userItem );
   } // End if has param 1 or 9.

   return res;
}


bool
NavExternalAuth::isHttpRequest( NavRequestPacket* req ) const
{
   bool res = false;

   if ( req->getParamBlock().getParam( 4 ) != NULL ) {
      MC2String clientType = req->getParamBlock().getParam( 4 )->getString(
         m_thread->clientUsesLatin1() );
      if ( m_thread->getExternalAuth()->getNavHttpSetting( 
              clientType.c_str() ) != NULL )
      {
         res = true;
      }
   }

   return res;
}


int
NavExternalAuth::handleHttpHeaderRequest( 
   NavRequestData& rd,
   bool& reusedConnection,
   const ParserExternalAuthHttpSettings* settings )
{
   int res = 0;

   ParserExternalAuthHttpSettings s( *settings );

   // Some standard client parameters
   UserItem* userItem = NULL;
   MC2String idStr;
   int status = 0;
   UserItem* setUserItem = NULL;
   
   if ( !m_userHelp->getUserFromParams( rd.params, idStr, userItem ) ) {
      mc2log << info << "handleExternalAuth: " << s.reqName 
             << " Communication error. idStr " << idStr << endl;
      status = -1;
      res = -1;
   }
   
   s.lang = rd.clientLang;

   MC2String clientUserID;
   if ( userItem != NULL ) {
      clientUserID = userItem->getUser()->getLogonID();
   } else {
      if ( rd.params.getParam( 1 ) ) {
         clientUserID = rd.params.getParam( 1 )->getString(
            m_thread->clientUsesLatin1() );
      }
      if ( rd.params.getParam( 8 ) ) {
         clientUserID = rd.params.getParam( 8 )->getString(
            m_thread->clientUsesLatin1() );
      }
   }

   s.createYears = rd.clientSetting->getCreateRegionTimeYear();
   s.createMonths = rd.clientSetting->getCreateRegionTimeMonth();
   s.createDays = rd.clientSetting->getCreateRegionTimeDay();
   s.createRegionID = rd.clientSetting->getCreateRegionID();
   s.createTransactionDays = rd.clientSetting->getCreateTransactionDays();

   InterfaceRequest* theReq = rd.ireq;
   if ( rd.ireq->getHttpRequest() != NULL ) {
      theReq = rd.ireq->getHttpRequest();
   }

   if ( res == 0 ) {
      MC2String licenceKey; // Used for timeout for checking with externalauth
      m_thread->getUserHandler()->makeLicenceKeyStr( licenceKey, rd.hwKey );
      res = m_thread->getExternalAuth()->handleHttpHeaderRequest( 
         handleHttpHeaderRequestParam( 
            theReq, &s, rd, ""/*activationCode*/, clientUserID, 
            licenceKey, licenceKey, userItem, 
            rd.req->getType() == NavPacket:: NAV_SERVER_INFO_REQ ),
         status, setUserItem );
   }

   // Print and set result
   if ( res == 0 ) {
      // UserItem is set
      rd.session->setUser( new UserItem( *setUserItem ) );
      m_thread->releaseUserItem( setUserItem );
      
      
      // Set username if none in client or not right
      const NParam* p = rd.params.getParam( 1 );
      if ( s.idAsKey ) {
         // If idAsKey then use param 9 UIN
         if ( rd.params.getParam( 9 ) != NULL ) {
            p = rd.params.getParam( 9 );
         }
         MC2String UINStr;
         STLStringUtility::uint2str( rd.session->getUser()->getUIN(), UINStr );
         if ( p == NULL || UINStr != p->getString( 
                 m_thread->clientUsesLatin1() ) )
         {
            rd.rparams.updateParam( 
               NParam( 9, UINStr, m_thread->clientUsesLatin1()) );
            rd.rparams.updateParam( 
               NParam( 2, "ChangeMe", m_thread->clientUsesLatin1() ) );
         }
      } else {
         if ( rd.params.getParam( 8 ) != NULL ) {
            p = rd.params.getParam( 8 );
         }
         if ( p == NULL || p->getString( 
                 m_thread->clientUsesLatin1() ) != rd.session->getUser()
              ->getUser()->getLogonID() )
         {
            rd.rparams.updateParam( 
               NParam( 8, rd.session->getUser()->getUser()
                       ->getLogonID(), m_thread->clientUsesLatin1() ) );
            rd.rparams.updateParam( 
               NParam( 2, "ChangeMe", m_thread->clientUsesLatin1() ) );
         }
      }

      // Print logon info
      m_thread->setLogPrefix( rd.session->m_logPrefix );
      mc2log << info << "handleExternalAuth: " << s.reqName 
             << " user: "
             << rd.session->getUser()->getUser()->getLogonID() 
             << "(" << rd.session->getUser()->getUIN() << ")"
             << " clientType \"" << rd.clientType 
             << "\" options \"" << rd.clientTypeOptions
             << "\" V" << rd.clientV[ 0 ] << "."
             << rd.clientV[ 1 ] << "." << rd.clientV[ 2 ]  
             << " PV " << int(rd.req->getProtoVer())
             << " WF "
             << WFSubscriptionConstants::subscriptionTypeToString(
                WFSubscriptionConstants::subscriptionsTypes( 
                   rd.wayfinderType ) );
      NavUserHelp::printLicences( mc2log, rd.hwKeys, " Licences: " );
      mc2log << endl;
   } else {
      // Set status in reply
      if ( status == -4 ) {
         rd.reply->setStatusCode(
            NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER );
      } else if ( status == -3 ) {
         rd.reply->setStatusCode(
            NavReplyPacket::NAV_STATUS_EXPIRED_USER );
      } else if ( status == -2 ) {
         rd.reply->setStatusCode(
            NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
      } else {
         rd.reply->setStatusCode(
            NavReplyPacket::NAV_STATUS_NOT_OK );
      }
      // Dump vars
      rd.params.dump( mc2log, true, true );
   }


   m_thread->releaseUserItem( userItem );

   return res;
}
