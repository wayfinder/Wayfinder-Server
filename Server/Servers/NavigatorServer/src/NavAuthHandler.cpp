/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavAuthHandler.h"
#include "InterfaceParserThread.h"
#include "NavPacket.h"
#include "ClientSettings.h"
#include "NavParserThreadGroup.h"
#include "NavUserHelp.h"
#include "isabBoxSession.h"
#include "SendEmailPacket.h"
#include "NavExternalAuth.h"
#include "isabBoxInterfaceRequest.h"
#include "UserRight.h"
#include "UserData.h"
#include "ParserTokenHandler.h"
#include "STLStringUtility.h"
#include "ParserActivationHandler.h"
#include "ParserUserHandler.h"
#include "NavRequestData.h"
#include "Properties.h"


NavAuthHandler::NavAuthHandler( InterfaceParserThread* thread,
                                NavParserThreadGroup* group,
                                NavUserHelp* userHelp )
      : NavHandler( thread, group ), m_userHelp( userHelp ),
        m_externalAuth( NULL )
{
   static ParamExpectation exps[] = {
      //User ID - UIN or login, see also 8 and 9
      ParamExpectation( 1, NParam::String, 1, 255 ),
      //User Passwd 
      ParamExpectation( 2, NParam::String, 1, 255 ),
      //User licence, IMEI or serialno
      ParamExpectation( 3, NParam::Byte_array, 1, 255 ),
      //client type
      ParamExpectation( 4, NParam::String ),
      //client type options
      ParamExpectation( 5, NParam::String ),
      //client language
      ParamExpectation( 6, NParam::Uint16 ),
      //wayfindertype
      ParamExpectation( 7, NParam::Byte ),
      //user login
      ParamExpectation( 8, NParam::String, 1, 255 ),
      //user UIN
      ParamExpectation( 9, NParam::String, 1, 20 ),
      //Retry Time, only sent from the server
      ParamExpectation( 10, NParam::Uint32 ),
      //Transactions left, only sent from server
      ParamExpectation( 11, NParam::Uint32_array, 12, 12 ),
      //session key
      ParamExpectation( 14, NParam::String ),
      //uploaded files
      ParamExpectation( 16, NParam::Byte_array ),
      //Old user licence
      ParamExpectation( 17, NParam::Byte_array, 1, 255 ),
      //days left, only sent from server
      ParamExpectation( 18, NParam::Uint16 ),
      //Server Auth BOB
      ParamExpectation( 19, NParam::Byte_array ),
      //Server auth BOB checksum
      ParamExpectation( 20, NParam::Uint32 ),
      
      // Upgrade
      //activation code
      ParamExpectation( 2200, NParam::String, 1, 255 ),
      //Phone number
      ParamExpectation( 2201, NParam::String, 1, 255 ),
      //topregionid
      ParamExpectation( 2202, NParam::Uint32 ),
   };
   static const size_t nexps = sizeof(exps)/sizeof(*exps);
   m_expectations.insert(m_expectations.end(), exps, exps + nexps);

   //allocate here to avoid exception problems. 
   m_externalAuth = new NavExternalAuth( thread, group, userHelp );

}


NavAuthHandler::~NavAuthHandler() {
   delete m_externalAuth;
}


bool
NavAuthHandler::handleAuth( NavRequestData& rd ) {
   //check that parameters are sane
   if ( !checkExpectations( rd.params, rd.reply ) ) {
      return false;
   }

   // Check auth
   bool errorSet = false;
   bool reusedConnection = false;

//   uint16 days = MAX_UINT16;

   // Time now 
   const uint32 now = TimeUtility::getRealTime();
   
   bool wfid = rd.clientSetting->getWFID();

   MC2String version = rd.getClientVersionAsString();
   
   if ( rd.params.getParam( 7 ) != NULL ) { //wayfindertype
      rd.wayfinderType = rd.params.getParam( 7 )->getByte();
      rd.wayfinderTypeSet = true;
   }

   // Hardware keys
   MC2String product = rd.clientSetting->getProduct();
   
   bool licenceTypeOK = m_userHelp->getLicencesAndTypes( 
      rd.params, 29, 3, rd.hwKeys, product );
   bool oldLicenceTypeOK = m_userHelp->getLicencesAndTypes( 
      rd.params, 30, 17, rd.oldHwKeys, product );
   if ( !licenceTypeOK || !oldLicenceTypeOK ) {
      rd.reply->setStatusCode( 
         NavReplyPacket::NAV_STATUS_PARAMBLOCK_INVALID );
      MC2String errStr;
      errStr.append( "Licence Key Type not valid. paramID " );
      STLStringUtility::uint2str( !licenceTypeOK ? 29 : 30, errStr );
      rd.reply->setStatusMessage( errStr );
      errorSet = true;
   } else {
      m_thread->getUserHandler()->setBestLicenceKey( rd.hwKeys, rd.hwKey );
   }
 
   // External authentication
   bool externalAuth = false;
   MC2String externalAuthName;
   if ( errorSet ) {
      // Error nothing to do.
   } else if ( m_externalAuth->isExternalAuth( rd.req, rd.ireq ) ) {
      int res = m_externalAuth->handleExternalAuth( 
         rd, reusedConnection, externalAuthName );
      rd.externalAuthStatusCode = rd.reply->getStatusCode();
      if ( res == 0 ) {
         externalAuth = true;
      } else if ( res == -1 ) {
         // Error set in reply
         errorSet = true;
         // And it is an externalAuth error
         externalAuth = true;
      } else if ( res == -2 ) {
         // Try normal
      }
   } else {
      //      Remove reserved user-names ("tme-") from params so
      //      user can't use external user in normal WF.
      //      This is so we keep the external auth users separated from 
      //      normal users.
      int res = m_externalAuth->removeExternalAuthUserNames( rd.params );

      if ( res != 0 ) {
         errorSet = true;
         rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
         if ( res == -2 ) {
            rd.reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
         }
      }

      // Check client type
      if ( rd.params.getParam( 4 ) != NULL &&
           strcmp( rd.clientSetting->getClientType(), "default" ) == 0 ) 
      {
         rd.reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER );
         mc2log << info << "handleAuth: Unknown client type \""
                << rd.clientType << "\" sending UNAUTHORIZED. " 
                << " Parameters: ";
         rd.params.dump( mc2log, true, true );
         errorSet = true;
      }
   }

   // Set externalAuth in requestdata
   rd.externalAuth = externalAuthName;

   if ( errorSet ) {
      // Error set above, do nothing.
   } else if ( externalAuth ) {
      // Auth done via external authentication
   } //   1. Check if blocked client
   else if ( rd.clientSetting->getBlockDate() < now ) {
      // Blocked no need to check more
      rd.reply->setStatusCode( 
         NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER );
      mc2log << info << "handleAuth: Blocked client type "
             << rd.clientType << " Parameters: ";
      rd.params.dump( mc2log, true, true );
   } // 2. SessionKey and previous user 
   else if ( rd.session->getUser() != NULL && 
             rd.params.getParam( 14 ) != NULL ) //sessionkey
   {
      if ( rd.session->getSessionKey().compare( 
              rd.params.getParam( 14 )->getString( //sessionkey
                 m_thread->clientUsesLatin1() ) ) == 0 ) 
      {
         m_thread->setLogPrefix( rd.session->m_logPrefix );
         UserItem* userItem = rd.session->getUser();
         mc2log << info << "User authenticated using sessionKey "
                << rd.params.getParam( 14 )->getString(
                      m_thread->clientUsesLatin1())
                << " login: " 
                << userItem->getUser()->getLogonID() 
                << "(" << userItem->getUIN() << ")"
                << " clientType \"" << rd.clientType 
                << "\" options \"" << rd.clientTypeOptions
                << "\" V" << version << " PV " 
                << int(rd.req->getProtoVer()) << " WF "
                << WFSubscriptionConstants::subscriptionTypeToString(
                   WFSubscriptionConstants::subscriptionsTypes( 
                      rd.wayfinderType ) );
         NavUserHelp::printLicences( mc2log, rd.hwKeys, " Licences: " );
         mc2log << endl;
         reusedConnection = true;
      } else {
         mc2log << warn << info << "handleAuth: sessionKey doesn't "
                << "match "
                << MC2CITE( rd.params.getParam( 14 )->getString(
                               m_thread->clientUsesLatin1()) )
                << " != " << MC2CITE( rd.session->getSessionKey() )
                << " UNAUTHORIZED Parameters: ";
         rd.req->getParamBlock().dump( mc2log, true, true ); 
         mc2log << endl;
         rd.reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER );
         errorSet = true;
      }
   } // 3. MSISDN and Licence Key
   else if ( false && rd.params.getParam( 3 ) != NULL 
               /*&& rd.params.getParam( MSISDN ) != NULL */ )
   {
      // Check MSISDN and Licence
      // TODO: Implement and remove && false in if-statement above
   } // 4.a UserName, Licence Key and Old Licence Key
   else if ( ( rd.params.getParam( 1 ) != NULL ||  //User ID
               rd.params.getParam( 8 ) ||          //User Login
               rd.params.getParam( 9 ) ) &&        //User UIN
             rd.params.getParam( 3 ) != NULL &&    //User Lincense
             rd.params.getParam( 17 ) != NULL &&   //Old User Licence
             rd.req->getType() == NavPacket::NAV_CHANGED_LICENCE_REQ )
   {
      // Changed licence key
      
      // Get user by UIN, Login, or User ID
      UserItem* userItem = NULL;
      MC2String idStr;
      getUser( userItem, idStr, errorSet, rd.req, rd.reply, 
               "Changed licence", true, true /*Really uptodate*/ );

      if ( !errorSet ) { //user found
         rd.session->setUser( new UserItem( *userItem ) );
         m_thread->setLogPrefix( rd.session->m_logPrefix );
         mc2log << info << "handleAuth: Calling handleChangedLicence "
                << endl;
         // Call handleChangedLicence
         handleChangedLicence( rd );

         if ( !rd.reply->getStatusCode() == NavReplyPacket::NAV_STATUS_OK )
         {
            if ( rd.reply->getStatusCode() == 
                 NavReplyPacket::
                 CHANGED_LICENCE_REPLY_OLD_LICENCE_NOT_IN_ACCOUNT ||
                 rd.reply->getStatusCode() == 
                 NavReplyPacket::
                 CHANGED_LICENCE_REPLY_OLD_LICENCE_IN_OTHER_ACCOUNT )
            {
               // C. Neither new or old key -> call handleWhoAmI
               mc2log << "handleAuth: Changed licence: says old licence "
                      << "not in account and not new either. Calling "
                      << "handleWhoAmIReq to fix it all up." << endl;
               rd.session->setUser( NULL );
               if ( handleWhoAmIReq( rd ) ) {
                  mc2log << "handleAuth: Changed licence: handleWhoAmIReq "
                         << "Fixed up the user." << endl;
                  rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_OK );
               } else {
                  errorSet = true;
               }
            } else {
               errorSet = true;
               mc2log << warn << "handleAuth: handleChangedLicence "
                      << "couldn't fix user up." << endl;
            }
         } // End if not status ok 

      } // End if not errorSet
      else if ( rd.reply->getStatusCode() == 
                NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER )
      {
         // Ok, no such user -> try with WhoAmIReq
         rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_OK );
         errorSet = false;
         if ( handleWhoAmIReq( rd ) ) {
         } else {
            errorSet = true;
            mc2log << warn << "handleAuth: Changed licence "
                   << "handleWhoAmIReq returned error." << endl;
         }
      }
         
      if ( !errorSet ) {
         // User authenticated!
         mc2log << info << "User authenticated using Changed licence, " 
                << "licence, login: " 
                << rd.session->getUser()->getUser()->getLogonID() 
                << "(" << rd.session->getUser()->getUIN() << ")"
                << " clientType \"" << rd.clientType 
                << "\" options \"" << rd.clientTypeOptions
                << "\" V" << version << " PV " 
                << int(rd.req->getProtoVer()) << " WF "
                << WFSubscriptionConstants::subscriptionTypeToString(
                   WFSubscriptionConstants::subscriptionsTypes( 
                      rd.wayfinderType ) );
         NavUserHelp::printLicences( mc2log, rd.hwKeys, " Licences: " );
         mc2log << endl;
      } else {
         mc2log << " Params: ";
         // Dump vars
         rd.params.dump( mc2log, true, true );
      }

      // Gotten above
      m_thread->releaseUserItem( userItem );
   } // 4. UserName and Licence Key   
   else if ( ( rd.params.getParam( 1 ) != NULL || //User ID         
               rd.params.getParam( 8 )||          //User Login      
               rd.params.getParam( 9 ) ) &&       //User UIN        
             rd.params.getParam( 3 )              //User Lincense   
             != NULL )                            
   {
      // Find user for licence
      UserItem* userItem = NULL;
      uint32 nbrUsers = 0;
      
      uint32 startTime = TimeUtility::getCurrentTime();
      bool commOk = m_thread->getUserHandler()->getUserFromLicenceKeys(
         rd.hwKeys, userItem, nbrUsers );
      uint32 endTime = TimeUtility::getCurrentTime();

      if ( !commOk || (nbrUsers == 1 && userItem == NULL) ) {
         mc2log << warn << "handleAuth: error getting user from licence: ";
         NavUserHelp::printLicence( mc2log, rd.hwKey );
         mc2log << " error: ";
         if ( endTime - startTime > 3000 ) {
            rd.reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
            mc2log << "Timeout";
         } else {
            rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
            mc2log << "Error";
         }
         mc2log << endl;
         errorSet = true;
      }

      if ( !errorSet ) {
         if ( nbrUsers == 1 ) {
            rd.session->setUser( new UserItem( *userItem ) );
            m_thread->setLogPrefix( rd.session->m_logPrefix );
            // Check if userID is the owner's userID
            bool userIDMatches = false;
            MC2String clientID;

            userIDMatches = checkUserIDMatches( rd, clientID );

            if ( wfid && !userIDMatches ) {
               userIDMatches = true;
               mc2log << warn << "handleAuth: WFID client has other userID ("
                      << clientID << ") than user (" << userItem->getUIN()
                      << ") returning error." << endl;
               rd.reply->setStatusCode( 
                  NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER );
               errorSet = true;
            }

            if ( !userIDMatches ) {
               mc2log << warn << "handleAuth: changing client to user "
                      << userItem->getUser()->getLogonID() << "("
                      << userItem->getUIN() << ") From";
               if ( rd.params.getParam( 1 ) != NULL ) {
                  mc2log << " userID " << 
                     rd.params.getParam( 1 )->getString( 
                        m_thread->clientUsesLatin1() );
               }
               if ( rd.params.getParam( 8 ) != NULL ) {
                  mc2log << " userLogon " << rd.params.getParam( 8 )
                     ->getString( m_thread->clientUsesLatin1() );
               }
               if ( rd.params.getParam( 9 ) != NULL ) {
                  mc2log << " UIN " << rd.params.getParam( 9 )->getString( 
                     m_thread->clientUsesLatin1() );
               }
               mc2log << endl;
               // Change client to user
               // Set rd.params
               rd.rparams.addParam( 
                  NParam( 8, userItem->getUser()->getLogonID(), 
                          m_thread->clientUsesLatin1() ) );
               char tmpStr[ 25 ];
               sprintf( tmpStr, "%u", userItem->getUIN() );
               rd.rparams.addParam( 
                  NParam( 9, tmpStr, m_thread->clientUsesLatin1() ) );
               rd.rparams.addParam( 
                  NParam( 2, "ChangeMe", m_thread->clientUsesLatin1() ) );
            }

         } else if ( nbrUsers == 0 ) {
            // Hmm, let handleWhoAmIReq make a new user.
            mc2log << info << "handleAuth: ununsed licence create "
                   << "new user." << endl;
            if ( handleWhoAmIReq( rd ) ) {
               // User authenticated!
            } else {
               errorSet = true;
            }
         } else { // Many
            // If user from userID has licence, unauth if not.
            // Get user
            MC2String idStr;
            getUser( userItem, idStr, errorSet, rd.req, rd.reply, 
                     "Many with licence" );

            if ( !errorSet ) {
               UserLicenceKey userKey( rd.hwKey );
               if ( m_thread->getUserHandler()->findUserLicenceKey( 
                       userItem->getUser(), &userKey ) )
               {
                  rd.session->setUser( new UserItem( *userItem ) );
                  m_thread->setLogPrefix( rd.session->m_logPrefix );
               } else { // User hasn't licence and many others do
                  mc2log << warn << "handleAuth: user "
                         << userItem->getUser()->getLogonID() << "("
                         << userItem->getUIN() << ") hasn't licence and "
                         << "many others do. Licence: ";
                  NavUserHelp::printLicence( mc2log, rd.hwKey );
                  mc2log << endl;
                  errorSet = true;
                  rd.reply->setStatusCode( 
                     NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER );
               }
            } // End if not error set
            else if ( rd.reply->getStatusCode() == 
                      NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER )
            {
               // Ok, no such user -> try with WhoAmIReq
               rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_OK );
               errorSet = false;
               if ( handleWhoAmIReq( rd ) ) {
               } else {
                  errorSet = true;
                  mc2log << warn << "handleAuth: Many users with licence ";
                  NavUserHelp::printLicence( mc2log, rd.hwKey );
                  mc2log << " and handleWhoAmIReq returned error." << endl;
               }
            }

         } // End if many users with licence
         
      } // End if not error set

      if ( !errorSet ) {
         mc2log << info << "User authenticated using UserName "
                << "and licence, login: " 
                << rd.session->getUser()->getUser()->getLogonID() 
                << "(" << rd.session->getUser()->getUIN() << ")"
                << " clientType \"" << rd.clientType 
                << "\" options \"" << rd.clientTypeOptions
                << "\" V" << version << " PV " 
                << int(rd.req->getProtoVer()) << " WF "
                << WFSubscriptionConstants::subscriptionTypeToString(
                   WFSubscriptionConstants::subscriptionsTypes( 
                      rd.wayfinderType ) ) << " Licences: ";
         NavUserHelp::printLicences( mc2log, rd.hwKeys );
         mc2log << endl;
      } else {
         mc2log << " Params: ";
         // Dump vars
         rd.params.dump( mc2log, true, true );
      }

      // Gotten above
      m_thread->releaseUserItem( userItem );
   } // 5. User Login and User Password
   else if ( rd.params.getParam( 8 ) != NULL &&  //User login
             rd.params.getParam( 2 ) != NULL )   //User Passwd
   {
      MC2String userName = rd.params.getParam( 8 )->getString(
         m_thread->clientUsesLatin1());
      MC2String userPassword = rd.params.getParam( 2 )->getString(
         m_thread->clientUsesLatin1() );
      uint32 UIN = m_thread->authenticateUser( userName.c_str(),
                                               userPassword.c_str() );
      if ( UIN != 0 && UIN != MAX_UINT32 && UIN != (MAX_UINT32 -1) ) {
         UserItem* userItem = NULL;
         if ( m_thread->getUser( UIN, userItem, true ) ) {
            if ( userItem != NULL ) {
               // Ok!
               rd.session->setUser( new UserItem( *userItem ) );
               m_thread->setLogPrefix( rd.session->m_logPrefix );
               mc2log << info << "User accepted using login and passwd, "
                      << "login: "
                      << userItem->getUser()->getLogonID() 
                      << "(" << userItem->getUIN() << ")" 
                      << " clientType \"" << rd.clientType 
                      << "\" options \"" << rd.clientTypeOptions
                      << "\" V" << version << " PV " 
                      << int(rd.req->getProtoVer()) << " WF "
                      << WFSubscriptionConstants::subscriptionTypeToString(
                         WFSubscriptionConstants::subscriptionsTypes( 
                            rd.wayfinderType ) ) << endl;
            } else {
               // We should never get here, we have valid UIN, 
               // but jic.
               rd.reply->setStatusCode( 
                  NavReplyPacket::NAV_STATUS_NOT_OK );
               errorSet = true;
            }
         } else {
            rd.reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
            errorSet = true;
         }
         m_thread->releaseUserItem( userItem );
      } else {
         if ( UIN == MAX_UINT32 ) {
            mc2log << warn << "handleAuth: " 
                   << "timeout checking User userName " 
                   << userName << " pass " << userPassword << endl;
            rd.reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
            errorSet = true;
         } else {
            mc2log << warn << "handleAuth: " 
                   << "UNAUTHORIZED User userName " 
                   << userName << " pass " << userPassword << endl;
            rd.reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER );
            errorSet = true;
         }
      }
   } // 6. Only Licence Key (IMEI) and request type is NAV_WHOAMI_REQ
   else if ( rd.params.getParam( 3 ) != NULL && //User Licence
             (rd.req->getType() == NavPacket::NAV_WHOAMI_REQ ||
              rd.req->getType() == NavPacket::NAV_UPGRADE_REQ ||
              true/*Only IMEI is good 20041220*/) )
   {
      // Handle WHOAMI_REQ (or UPGRADE_REQ after) Or any request 041220
      if ( handleWhoAmIReq( rd ) ) {
         // User authenticated!
         mc2log << info << "User authenticated using only " 
                << "licence, login: " 
                << rd.session->getUser()->getUser()->getLogonID() 
                << "(" << rd.session->getUser()->getUIN() << ")"
                << " clientType \"" << rd.clientType 
                << "\" options \"" << rd.clientTypeOptions
                << "\" V" << version << " PV " 
                << int(rd.req->getProtoVer()) << " WF "
                << WFSubscriptionConstants::subscriptionTypeToString(
                   WFSubscriptionConstants::subscriptionsTypes( 
                      rd.wayfinderType ) ) << " Licence: ";
         NavUserHelp::printLicence( mc2log, rd.hwKey );
         mc2log << endl;
      } else {
         errorSet = true;
         mc2log << " Params: ";
         // Dump vars
         rd.params.dump( mc2log, true, true );
      }      
   } // 7. Else unauthorized
   else {
      mc2log << "UNAUTHORIZED!" << endl;
      rd.reply->setStatusCode( 
         NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER );
      mc2log << info << "handleAuth: Nothing to authenticate with";
      NavUserHelp::printLicences( mc2log, rd.hwKeys, " Licences: " );
      mc2log << " Parameters: ";
      rd.params.dump( mc2log, true, true );
      errorSet = true;
   }



   // Check if client sent some upload files
   if ( rd.params.getParam( 16 ) != NULL &&  //uploaded files
        rd.session->getUser() != NULL )
   {
      const NParam* plicence = rd.params.getParam( 3 );
      vector< byte > data;
      vector< const NParam* > pf;
      rd.params.getAllParams( 16, pf );
      for ( uint32 i = 0 ; i < pf.size() ; ++i ) {
         data.insert( data.end(), pf[ i ]->getBuff(), 
                      pf[ i ]->getBuff() + pf[ i ]->getLength() );
      }
      m_userHelp->handleUploadFiles( 
         rd.session->getUser(), data,
         plicence ? plicence->getBuff() : NULL,
         plicence ? plicence->getLength() : 0 );
   }

   // Check if redirect
   if ( m_group->getForceRedirect() ) {
      // Add ServerList 
      // Auth bob not needed (not backup - really should be on other 
      // server).
      m_userHelp->addServerListParams( 
         rd.rparams, *rd.clientSetting, rd.ireq->getHttpRequest() != NULL, 
         true );

      mc2log << info << "handleAuth: Redirecting client" << endl;
      // And set redirect
      rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_REDIRECT );
      // And close after this
      rd.session->setCallDone( 1 ); // No more on this
      // And we are done
      errorSet = true;
   }

   // Get the best possible clientSetting and update user params in rd.
   if ( !errorSet && rd.session->getUser() != NULL ) {
      rd.clientSetting = m_userHelp->getClientSetting(
         rd.params, rd.session->getUser()->getUser() );
      rd.urmask = m_userHelp->getUrMask( rd.clientSetting );
   }

   //check lock-in
   if ( !errorSet ) {
      mc2dbg2 << "checking version lock" << endl;
      errorSet = ! checkVersionLock( rd );
      if ( errorSet ) {
         mc2dbg4 << "VERSION LOCK!" << endl;
         rd.reply->setStatusCode(NavReplyPacket::NAV_STATUS_EXTENDED_ERROR);
         //add the "external URL"
         rd.rparams.addParam( NParam( 26, "http://versionlock/",
                                      m_thread->clientUsesLatin1() ) );
      }
   }

   // Check if tokens
   if ( !errorSet && rd.session->getUser() != NULL && !externalAuth && 
        !reusedConnection && !wfid &&
        (rd.req->getType() != NavPacket::NAV_UPGRADE_REQ &&
         rd.req->getType() != NavPacket::NAV_TUNNEL_DATA_REQ) &&
        m_thread->getTokenHandler()->getNbrTokens( 
           rd.session->getUser()->getUser(), rd.clientSetting ) > 0 &&
        m_thread->getUserHandler()->getNbrProductKeys( 
           rd.session->getUser()->getUser(), rd.clientSetting->getProduct() ) 
        <= 1 )
   {
      // User has tokens force reactivation
      mc2log << info << "handleAuth: User has tokens forcing reactivation "
             << "by sending UNAUTHORIZED." << endl;
      rd.reply->setStatusCode( 
         NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER );
      //add the "external URL"
      rd.rparams.addParam( NParam( 26, "http://upgrade/",
                                   m_thread->clientUsesLatin1() ) );
      rd.session->setUser( NULL ); // Deletes old one
      // May keep logonID m_thread->setLogPrefix(session->m_logPrefix);
      errorSet = true;
   }

   // Check if to add Earth right or Trial time
   if ( !errorSet && rd.session->getUser() != NULL
        && rd.clientSetting->usesRights() ) {
      if ( !checkIronTrialTime( rd ) ) 
      {
         rd.session->setUser( NULL ); // Deletes old one
         // May keep logonID m_thread->setLogPrefix(session->m_logPrefix);
         errorSet = true;
      }
   }

   // Check if too long trial time
   if ( !errorSet && rd.session->getUser() != NULL &&
        rd.clientSetting->usesRights() &&
        m_thread->getSubscriptionTypeForUser( rd.session->getUser()
           ->getUser(), UserEnums::UR_WF ) == 
        WFSubscriptionConstants::TRIAL && rd.params.getParam( 4 ) != NULL )
   {
      if ( !checkTrialTime( rd.session->getUser(), rd.req, rd.reply,
                            rd.session, rd.clientSetting ) )
      {
         rd.session->setUser( NULL ); // Deletes old one
         // May keep logonID m_thread->setLogPrefix(session->m_logPrefix);
         errorSet = true;
      }
   }
   
   // Check transactions
   if ( !errorSet && rd.session->getUser() != NULL ) {
      if ( !checkTransactions( rd.session->getUser(), rd.req, rd.reply ) ) 
      {
         rd.session->setUser( NULL ); // Deletes old one
         // May keep logonID m_thread->setLogPrefix(session->m_logPrefix);
         errorSet = true;
      }
   }

   // Check WFST (If too high,low send to client or even Unauth)
   if ( !errorSet && rd.session->getUser() != NULL && !reusedConnection ) {
      if ( !checkUserWFST( rd.session->getUser(), rd.req, rd.reply,
                           rd.clientSetting ) ) 
      {
         rd.session->setUser( NULL ); // Deletes old one
         // May keep logonID m_thread->setLogPrefix(session->m_logPrefix);
         errorSet = true;
      }
   }

   // Check for expired
   if ( !errorSet && rd.session->getUser() != NULL &&
         rd.clientSetting->usesRights() ) {
      if ( !checkUserExpired( rd.session->getUser(), rd.req, rd.reply ) ) {
         mc2log << info << "handleAuth: User expired." << endl;
         rd.session->setUser( NULL ); // Deletes old one
         // May keep logonID m_thread->setLogPrefix(session->m_logPrefix);
         errorSet = true;
      }
   }

   // Check if tell client to upload files
   if ( !errorSet && rd.session->getUser() != NULL && 
        rd.params.getParam( 16 ) == NULL ) //uploaded files
   {
      // Check if initials have magic keyword
      const char* keyWord = "WfGEt:";
      char* tmpPtr = StringUtility::strstr( 
         rd.session->getUser()->getUser()->getInitials(), keyWord );
      if ( tmpPtr != NULL ) {
         // Found!
         MC2String files = tmpPtr + strlen( keyWord );
         if ( files.size() > 0 ) {
            rd.reply->getParamBlock().addParam( 
               NParam( 15, files.c_str(), m_thread->clientUsesLatin1() ) );
            mc2log << info << "handleAuth: : Requesting client to upload "
                   << "files: " << MC2CITE( files ) << endl;
            // Remove to avoid sending several times.
            vector<MC2String> fileVect;
            uint32 pos = 0;
            while ( pos < files.size() ) {
               MC2String::size_type findPos = files.find( ",", pos );
               fileVect.push_back( files.substr( pos, findPos - pos ) );
               if ( findPos != MC2String::npos ) {
                  pos = findPos + 1;
               } else {
                  pos = findPos;
               }
            }
            m_userHelp->removeUploadFiles( rd.session->getUser(), fileVect );
         }
      }
   }

   // Check last client.
   if ( !errorSet && rd.session->getUser() != NULL && !reusedConnection &&
        !((rd.req->getType() == NavPacket::NAV_SERVER_INFO_REQ ||
           rd.req->getType() == NavPacket::NAV_TOP_REGION_REQ ||
           rd.req->getType() == NavPacket::NAV_LATEST_NEWS_REQ  ||
           rd.req->getType() == NavPacket::NAV_CATEGORIES_REQ  ||
           rd.req->getType() == NavPacket::NAV_CALLCENTER_LIST_REQ  ||
           rd.req->getType() == NavPacket::NAV_SERVER_LIST_REQ  ||
           rd.req->getType() == NavPacket::NAV_SERVER_AUTH_BOB_REQ  ||
           rd.req->getType() == NavPacket::NAV_NEW_PASSWORD_REQ) &&
          rd.params.getParam( 11 ) == NULL) )
   {
      // NAV_SERVER_INFO_REQ doesn't send version (sends minimal params)
      // In one case.
      UserItem* setUserItem = NULL;
      MC2String extra;
      if ( m_thread->getUserHandler()->checkAndUpdateLastClientAndUsage(
              rd.session->getUser()->getUser(), setUserItem,
              rd.clientType, rd.clientTypeOptions, version, extra, 
              rd.ipnport ) )
      {
         rd.session->setUser( new UserItem( *setUserItem ) );
         m_thread->releaseUserItem( setUserItem );
      }
   }

   // Set sessionKey if no set by client. Not for WFID clients
   // Disabled to get client type in all requests.
#if 0
   if ( !errorSet && !wfid &&
        rd.params.getParam( 14 ) == NULL && //sessionkey
        rd.ireq->getHttpRequest() == NULL ) 
   {
      const uint32 sessionKeyLength = 4;
      char sessionKey[ sessionKeyLength + 1 ];
      const char* sKey = sessionKey;
      if ( rd.session->getSessionKey().size() > 0 ) {
         sKey = rd.session->getSessionKey().c_str();
      } else {
         StringUtility::randStr( sessionKey, sessionKeyLength );
         rd.session->setSessionKey( sessionKey );
      }
      rd.reply->getParamBlock().addParam( 
         NParam( 14, sKey, m_thread->clientUsesLatin1() ) );
   }
#endif

   // WFID
   if ( wfid ) {
      // Some WFID changes
      if ( errorSet && rd.req->getType() == NavPacket::NAV_TUNNEL_DATA_REQ ) {
         // Some errors are ignored, let user get login page
         uint8 status = rd.reply->getStatusCode();
         if ( status == NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER ) {
            rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_OK );
            rd.reply->setStatusMessage( "" );
            rd.rparams.removeParam( 26 ); // external URL
            errorSet = false; // Not "error" per say
            mc2log << info << "User unauthorized letting WFID user tunnel " 
                   << "anyway: " 
                   << " clientType \"" << rd.clientType 
                   << "\" options \"" << rd.clientTypeOptions
                   << "\" V" << version << " PV " 
                   << int(rd.req->getProtoVer()) << " WF "
                   << WFSubscriptionConstants::subscriptionTypeToString(
                      WFSubscriptionConstants::subscriptionsTypes( 
                         rd.wayfinderType ) );
            NavUserHelp::printLicences( mc2log, rd.hwKeys, " Licences: " );
            mc2log << endl;
         }
      }
      if ( rd.rparams.getParam( 26 ) == NULL ) {
         // Send a suitable web page to goto
         if ( rd.reply->getStatusCode() == 
              NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER ) {
            rd.rparams.addParam( 
               NParam( 26, "http://firstpage/?srverr=clientunauthorized",
                       m_thread->clientUsesLatin1() ) );
         } else if ( rd.reply->getStatusCode() == 
                     NavReplyPacket::NAV_STATUS_EXPIRED_USER ) {
            rd.rparams.addParam( 
               NParam( 26, "http://firstpage/?srverr=clientexpired",
                       m_thread->clientUsesLatin1() ) );
         }
      }
   } // End if WFID client

   return !errorSet;
}


bool
NavAuthHandler::handleUpgrade( NavRequestData& rd )
{
   if ( !checkExpectations( rd.params, rd.reply ) ) {
      return false;
   }

   bool ok = true;

   // The user
   UserUser* user = rd.session->getUser()->getUser();


   // Start parameter printing
//   mc2log << info << "handleUpgrade:";

   
   const char* actCode = NULL;
   MC2String actCodeStr;
   if ( rd.params.getParam( 2200 ) != NULL ) {
      actCodeStr = rd.params.getParam( 2200 )->getString(
         m_thread->clientUsesLatin1() );
      actCode = actCodeStr.c_str();
   }

   const char* phoneNumber = NULL;
   MC2String phoneNumberStr;
   if ( rd.params.getParam( 2201 ) != NULL ) {
      phoneNumberStr = rd.params.getParam( 2201 )->getString(
         m_thread->clientUsesLatin1() );
      phoneNumber = phoneNumberStr.c_str();
   }

   uint32 topRegionID = MAX_INT32;
   if ( rd.params.getParam( 2202 ) != NULL ) {
      topRegionID = rd.params.getParam( 2202 )->getUint32();
   }

   MC2String email;
   MC2String name;
   if ( rd.params.getParam( 2203 ) != NULL ) {
      email = rd.params.getParam( 2203 )->getString(
         m_thread->clientUsesLatin1() );
   }
   if ( rd.params.getParam( 2204 ) != NULL ) {
      name = rd.params.getParam( 2204 )->getString(
         m_thread->clientUsesLatin1() );
   }

   MC2String options;
   if ( rd.params.getParam( 2205 ) != NULL ) {
      options = rd.params.getParam( 2205 )->getString(
         m_thread->clientUsesLatin1() );
   }

   bool topRegionOK = true;
   bool activationCodeOK = true;
   bool phonenumberOK = true;
   bool emailOK = true;
   bool nameOK = true;
   byte userWFType = m_thread->getSubscriptionTypeForUser( 
      user, UserEnums::UR_WF );
   MC2String serverStr;
   uint8 status = NavReplyPacket::NAV_STATUS_OK;
   bool sendBools = false;

   if ( ok && !(email.empty() && name.empty()) ) {
      bool ironClient = false;
      if ( m_thread->checkIfIronClient( rd.clientSetting ) ) {
         // Earth "upgrade"
         ironClient = true;
      } // Else Trial registration
        
      // Set email and name
      UserUser* cuser = new UserUser( *user );

      if ( !StringUtility::validEmailAddress( email.c_str() ) ) {
         emailOK = false;
      }

      if ( emailOK && nameOK ) {
         if ( !email.empty() ) {
            cuser->setEmailAddress( email.c_str() );
         }
         if ( !name.empty() ) {
            // Split name on space and see
            vector<MC2String> splitStr;
            StringUtility::tokenListToVector( splitStr, name, ' ' );
            if ( splitStr.size() > 1 ) {
               cuser->setFirstname( splitStr[ 0 ].c_str() );
               cuser->setLastname( splitStr[ 1 ].c_str() );
            } else if ( splitStr.size() > 0 ) {
               cuser->setFirstname( splitStr[ 0 ].c_str() );
            }
         }
         if ( !options.empty() ) {
            // Set some in customerContactInfo
            vector<MC2String> splitStr;
            StringUtility::tokenListToVector( splitStr, options, ';' );
            for ( uint32 i = 0 ; i < splitStr.size() ; ++i ) {
               if ( !cuser->setCustomerContactInfoField( splitStr[ i ] ) )
               {
                  mc2log << warn << "handleUpgrade: Bad option from client"
                         << ": " << MC2CITE( splitStr[ i ] ) << " from "
                         << MC2CITE( options ) << endl;
               }
            }
         }
         uint32 startTime = TimeUtility::getCurrentTime();
         if ( cuser->getNbrChanged() > 0 ) {
            if ( !m_thread->changeUser( cuser, NULL/*changer*/ ) ) {
               rd.reply->setStatusCode( 
                  NavReplyPacket::NAV_STATUS_NOT_OK );
               mc2log << warn << "handleUpgrade: "
                      << "Failed to update user "
                      << "with new email \"" << email << "\" name \"" 
                      << name << "\".";
               uint32 endTime = TimeUtility::getCurrentTime();
               if ( (endTime-startTime) > 2000 ) {
                  mc2log << " Timeout.";
                  rd.reply->setStatusCode( 
                     NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
               }
               mc2log << endl;
            } else {
               // Else changed ok
               mc2log << info << "handleUpgrade: "
                      << "added email \"" << email << "\" name \"" << name
                      << "\" to user." << endl;
            }
         }

         if ( rd.reply->getStatusCode() == NavReplyPacket::NAV_STATUS_OK ) 
         {
            if ( ironClient ) {
               // Send email
               // 20080228 Now mail will be sent from CRM system
               sendBools = true;
               userWFType = WFSubscriptionConstants::IRON;
            } else {
               sendBools = true;
               userWFType = m_thread->getSubscriptionTypeForUser( 
                  user, UserEnums::UR_WF );
            }
         }
      } else { // End if mail and name ok
         // Set this
         sendBools = true;
      }

      delete cuser;
   } else if ( actCode != NULL && phoneNumber != NULL ) {
      UserItem* changeToUserItem = NULL;
      status = m_userHelp->handleNavUpgrade( 
         actCode, topRegionID, phoneNumber, user,
         rd.clientSetting,
         rd.clientV, rd.session->getPeerIP(), rd.hwKey,
         userWFType, topRegionOK, activationCodeOK, phonenumberOK,
         changeToUserItem, true/*mayChangeUser */, serverStr );
      if ( m_thread->checkIfIronClient( rd.clientSetting ) ) {
         userWFType = WFSubscriptionConstants::IRON;
      }

      if ( changeToUserItem != NULL ) {
         rd.session->setUser( new UserItem( *changeToUserItem ) );
         m_thread->setLogPrefix( rd.session->m_logPrefix );
         user = rd.session->getUser()->getUser();
         m_thread->releaseUserItem( changeToUserItem );

         // Set params
         rd.rparams.addParam( NParam( 8, user->getLogonID(),
                                      m_thread->clientUsesLatin1() ) );
         char tmpStr[ 25 ];
         sprintf( tmpStr, "%u", user->getUIN() );
         rd.rparams.addParam( NParam( 9, tmpStr, 
                                      m_thread->clientUsesLatin1() ) );
         rd.rparams.addParam( NParam( 2, "ChangeMe",
                                      m_thread->clientUsesLatin1() ) );
      }

      if ( status == NavReplyPacket::NAV_STATUS_OK ) {
         sendBools = true;
         mc2log << info << "handleUpgrade: reply: "
                << " topRegionOK " << topRegionOK
                << " phonenumberOK " << phonenumberOK
                << " activationCodeOK " << activationCodeOK 
                << " WFType " << WFSubscriptionConstants::
            subscriptionTypeToString( WFSubscriptionConstants::
                                      subscriptionsTypes( userWFType ) );
      } else if ( status == NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT ) {
         rd.reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
         mc2log << info << "handleUpgrade: reply: Timeouterror" << endl;
      } else if ( status == NavReplyPacket::NAV_STATUS_REDIRECT ) {
         // Add ServerList 
         // Auth bob not needed (not backup - really should be on other 
         // server).
         if ( m_userHelp->addServerListParams( 
                 rd.rparams, *rd.clientSetting, 
                 rd.ireq->getHttpRequest() != NULL, true, false, serverStr ) )
         {
            // And set redirect       
            rd.reply->setStatusCode( status );
            // And close after this
            rd.session->setCallDone( 1 ); // No more on this
         } else {
            mc2log << error << "handleUpgrade: No serverlist for "
                   << serverStr << " sending error." << endl;
            rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
         }
      } else if ( status == NavReplyPacket::UPGRADE_MUST_CHOOSE_REGION ) {
         if ( rd.reqVer > 1 ) {
            rd.reply->setStatusCode( status );
         } else {
            mc2log << warn << "handleUpgrade: Old client no support for "
                   << "choosing regionID, sendign bad AC." << endl;
            sendBools = true;
            activationCodeOK = false;
         }
      } else {
         rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
         mc2log << info << "handleUpgrade: reply: Error" << endl;
      }
   } else {
      mc2log << warn << "handleUpgrade: Missing input";
      if ( actCode == NULL ) {
         mc2log << " ActivationCode missing";
         rd.reply->setStatusMessage( "ActivationCode" );
      }
      if ( phoneNumber == NULL ) {
         mc2log << " PhoneNumber missing";
         rd.reply->setStatusMessage( "Phonenumber" );
      }
      mc2log << endl;
      rd.reply->setStatusCode( 
         NavReplyPacket::NAV_STATUS_MISSING_PARAMETER );
   }


   if ( sendBools ) {
      rd.rparams.addParam( NParam( 2300, byte(topRegionOK) ) );
      rd.rparams.addParam( NParam( 2301, byte(activationCodeOK) ) );
      rd.rparams.addParam( NParam( 2302, byte(phonenumberOK) ) );
      rd.rparams.addParam( NParam( 2303, userWFType ) );
      uint32 scale = 100000;
      Nav2Coordinate coord( m_userHelp->getCenterpointFor(
                               rd.session->getUser(), scale ) ); 
      NParam center( 2304 );
      center.addInt32( coord.nav2lat );
      center.addInt32( coord.nav2lon );
      center.addUint32( scale );
      rd.rparams.addParam( center );
      mc2log << " Center " << MC2Coordinate( coord )
             << endl;
      rd.rparams.addParam( NParam( 2305, byte(emailOK) ) );
      rd.rparams.addParam( NParam( 2306, byte(nameOK) ) );
   }

   return ok;
}


bool
NavAuthHandler::handleWhoAmIReq( NavRequestData& rd ) {
   bool ok = true;
   if ( rd.session->getUser() != NULL ) {
      // Have user using it.
      rd.reply->getParamBlock().addParam( 
         NParam( 8, rd.session->getUser()->getUser()->getLogonID(),
                 m_thread->clientUsesLatin1() ) );
      char tmp[20];
      sprintf( tmp, "%u", rd.session->getUser()->getUIN() );
      rd.reply->getParamBlock().addParam( 
         NParam( 9, tmp, m_thread->clientUsesLatin1() ) );
      return ok;
   }

   if ( rd.hwKeys.empty() ) {
      mc2log << warn << "handleWhoAmIReq: no licence to get user for, "
             << "returning UNAUTHORIZED. Params " << endl;
      rd.params.dump( mc2log, true, true );
      rd.reply->setStatusCode( 
         NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER );
      return false;
   }
   
   UserItem* userItem = NULL;
   uint32 nbrUsers = 0;
   UserLicenceKey userKey( rd.hwKey );

   if ( m_thread->getUserHandler()->getUserFromLicenceKeys(
           rd.hwKeys, userItem, nbrUsers ) )
   {
      // If not then add and return username and password
      if ( nbrUsers == 1 && userItem == NULL ) {
         // Failed to get user
         rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
         mc2log << warn << "handleWhoAmIReq: failed to "
                << "get user(s) with licence: ";
         NavUserHelp::printLicence( mc2log, rd.hwKey );
         mc2log << endl;
         ok = false;
      } else if ( nbrUsers == 0 && !rd.clientSetting->getWFID() ) {
         MC2String passwd;
         MC2String server;

         // Make create  settings.
         uint32 now = TimeUtility::getRealTime();
         uint32 startTime = now - 24*60*60;
         uint32 endTime = now;
         m_group->getCreateWFTime( rd.clientSetting, endTime );
         WFSubscriptionConstants::subscriptionsTypes createWFST = 
            WFSubscriptionConstants::subscriptionsTypes(
               rd.clientSetting->getCreateLevel() );
         const UserLicenceKey* imeiKey = rd.getLicenceKeyType( 
            ParserUserHandler::imeiType );

         int status = 
            m_thread->getUserHandler()->createWayfinderUserWithAutoAC( 
               passwd, server, &userKey, userItem, startTime, endTime,
               rd.clientSetting->getCreateRegionID(), rd.clientLang, 
               rd.clientType.c_str(), rd.clientTypeOptions.c_str(),
               rd.clientV, ""/*activationCode*/, 
               ParserUserHandler::ExtraTypes(), createWFST,
               rd.clientSetting->getCreateTransactionDays(),
               rd.clientSetting->getBrand(), NULL/*fixedUserName*/, 
               NULL/*extraComment*/, ParserUserHandler::UserElementVector(),
               NULL,  // userNamePrefix
               NULL, NULL, // email, name
               "", "", // opt in name, opt in value
               rd.clientSetting->getExtraRights(),
               imeiKey /*The key for auto IMEI activation*/ );
         if ( status == 0 ) {
            // Set user in session (to a copy)
            rd.session->setUser( new UserItem( *userItem ) );
            m_thread->setLogPrefix( rd.session->m_logPrefix );
            // Set UserName and Password in reply params
            rd.reply->getParamBlock().addParam( 
               NParam( 8, userItem->getUser()->getLogonID(),
                       m_thread->clientUsesLatin1() ) );
            char tmpStr[ 256 ];
            sprintf( tmpStr, "%u", userItem->getUIN() );
            rd.rparams.addParam( 
               NParam( 9, tmpStr, m_thread->clientUsesLatin1() ) );
            rd.rparams.addParam( 
               NParam( 2, passwd, m_thread->clientUsesLatin1() ) );
         } else if ( status == 2 ) {
            rd.reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
            ok = false;
         } else if ( status == 4 ) {
            rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_REDIRECT );
            m_userHelp->addServerListParams( 
               rd.rparams, *rd.clientSetting, 
               rd.ireq->getHttpRequest() != NULL, true, false, server );
            ok = false;
         } else if ( status == 5 ) {
            rd.reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER );
            ok = false;
         } else { // if ( status == 3 )
            rd.reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_NOT_OK );
            ok = false;
         }
      } else if ( nbrUsers == 0 && rd.clientSetting->getWFID() ) {
         // No user and WFID, then let web login user
         mc2log << warn << "handleWhoAmIReq: No "
                << "user with licence " << rd.hwKey << " and WFID not creating"
                << " user." << endl;
         rd.reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER );
         ok = false;
      } else if ( nbrUsers == 1 ) {
         // User with Licence already exists
         rd.reply->getParamBlock().addParam( 
            NParam( 8, userItem->getUser()->getLogonID(),
                    m_thread->clientUsesLatin1() ) );
         char* tmpStr = new char[ 256 ];
         sprintf( tmpStr, "%u", userItem->getUIN() );
         rd.rparams.addParam( 
            NParam( 9, tmpStr, m_thread->clientUsesLatin1() ) );
         rd.rparams.addParam( 
            NParam( 2, "ChangeMe", m_thread->clientUsesLatin1() ) );
         delete [] tmpStr;
         // Set user in session (to a copy)
         rd.session->setUser( 
            new UserItem( *userItem ) );
         m_thread->setLogPrefix( 
            rd.session->m_logPrefix );
      } else { 
         // Not 0 or 1 user then it is 2+
         mc2log << warn << "handleWhoAmIReq: More than one "
                << "user with same licence sending error, "
                << "licence: ";
         NavUserHelp::printLicence( mc2log, rd.hwKey );
         mc2log << endl;
         rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
         ok = false;
      }
   } else {
      // Data com. problem
      mc2log << warn << "handleWhoAmIReq: failed to "
             << "find user(s) with licence: ";
      NavUserHelp::printLicence( mc2log, rd.hwKey );
      mc2log << endl;
      rd.reply->setStatusCode( 
         NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
      ok = false;
   }

   m_thread->releaseUserItem( userItem );

   return ok;
}


bool
NavAuthHandler::handleChangedLicence( NavRequestData& rd )
{
   if ( !checkExpectations( rd.params, rd.reply ) ) {
      return false;
   }

   bool ok = true;

   // The user
   UserUser* user = rd.session->getUser()->getUser();

   // Start parameter printing
   mc2log << info << "handleChangedLicence:";

   const NParam* plicence3 = NULL;
   const NParam* plicence17 = NULL;

   if ( rd.params.getParam( 3 ) ) {
      plicence3 = rd.params.getParam( 3 );
      mc2log << " New licence ";
      NavUserHelp::printLicence( 
         mc2log, plicence3->getBuff(), plicence3->getLength() ); 
   }

   if ( rd.params.getParam( 17 ) ) {
      plicence17 = rd.params.getParam( 17 );
      mc2log << " Old licence ";
      NavUserHelp::printLicence( 
         mc2log, plicence17->getBuff(), plicence17->getLength() ); 
   }

   mc2log << endl;

   // If external auth don't use licence keys, just return ok.
   if ( m_externalAuth->isExternalAuth( rd.req, rd.ireq ) &&
        plicence3 != NULL ) 
   {
      // Set 3 in rd.rparams, to indicate new value to client
      rd.rparams.addParam( *plicence3 );
      mc2log << warn << "handleChangedLicence: externalAuth returning "
             << "ok" << endl;
      return true;
   }

   // This only works for old clients with IMEI
   // The client sends only one key or this works in undefined ways
   if ( rd.hwKeys.size() == 1 && rd.getLicenceKeyType( 
           rd.hwKeys, ParserUserHandler::imeiType ) != NULL &&
        rd.oldHwKeys.size() == 1 && rd.getLicenceKeyType( 
           rd.oldHwKeys, ParserUserHandler::imeiType ) != NULL ) {
      // Ok both is one IMEI
   } else {
      mc2log << warn << "handleChangedLicence Bad number or type of licences."
             << endl;
      rd.reply->setStatusMessage( "Bad number or type of licence(s)" );
      rd.reply->setStatusCode( 
         NavReplyPacket::NAV_STATUS_MISSING_PARAMETER );
      return false;
   }

   
   if ( plicence3 == NULL || plicence17 == NULL ) {
      // Ok, no way I can change to/from NULL
      mc2log << warn << "handleChangedLicence Missing old or new licence."
             << endl;
      ok = false;
      if ( plicence3 == NULL ) {
         rd.reply->setStatusMessage( "New licence" );
      } else {
         rd.reply->setStatusMessage( "Old licence" );
      }
      rd.reply->setStatusCode( 
         NavReplyPacket::NAV_STATUS_MISSING_PARAMETER );
   } else {
      // Get user for 3 and the user for 17.
      UserItem* userItem3 = NULL;
      uint32 nbrUsers3 = 0;
      bool user3Ok = false;
      UserItem* userItem17 = NULL;
      uint32 nbrUsers17 = 0;
      bool user17Ok = false;
      UserLicenceKey* userKey = new UserLicenceKey( MAX_UINT32 );
      userKey->setLicence( plicence3->getBuff(), plicence3->getLength() );
      userKey->setKeyType( ParserUserHandler::imeiType );
      
      user3Ok = m_thread->getUserFromUserElement( 
         userKey, userItem3, nbrUsers3, true, true );
      // And 17
      userKey->setLicence( plicence17->getBuff(), plicence17->getLength());
      userKey->setKeyType( ParserUserHandler::imeiType );
      user17Ok = m_thread->getUserFromUserElement( 
         userKey, userItem17, nbrUsers17, true, true );

      if ( !user3Ok || (nbrUsers3 == 1 && userItem3 == NULL) ) {
         // Data.com. error
         ok = false;
         rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
         mc2log << warn << "handleChangedLicence: failed to "
                << "get user(s) with new licence: ";
         NavUserHelp::printLicence( mc2log, plicence3->getBuff(), 
                                    plicence3->getLength() );
         mc2log << endl;
      }

      if ( !user17Ok || (nbrUsers17 == 1 && userItem17 == NULL) ) {
         // Data.com. error
         ok = false;
         rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
         mc2log << warn << "handleChangedLicence: failed to "
                << "get user(s) with old licence: ";
         NavUserHelp::printLicence( mc2log, plicence17->getBuff(), 
                                    plicence17->getLength() );
         mc2log << endl;
      }

      // Check if user already has been updated
      userKey->setLicence( plicence3->getBuff(), plicence3->getLength() );
      userKey->setKeyType( ParserUserHandler::imeiType );
      UserLicenceKey* findKey = m_thread->getUserHandler()->findUserLicenceKey(
         user, userKey );
      if ( findKey != NULL ) {
         // Hmm found new key in user, no need to update.
         mc2log << warn << "handleChangedLicence Found new key in user. "
                << "Not changing user." << endl;
         // And set 3 in rd.rparams, to indicate new key in user
         rd.rparams.addParam( *plicence3 );
         ok = false; // Not really but don't change users
      }

      if ( ok ) {
         if ( nbrUsers17 > 1 ) {
            // Many users with old licence
            ok = false;
            rd.reply->setStatusCode( 
               NavReplyPacket::
               CHANGED_LICENCE_REPLY_MANY_USERS_WITH_OLD_LICENCE );
            mc2log << warn << "handleChangedLicence: many users with old "
                   << "licence: ";
            NavUserHelp::printLicence( mc2log, plicence17->getBuff(), 
                                       plicence17->getLength() );
            mc2log << endl;
         } else if ( userItem17 == NULL ) {
            // No user with old licence
            ok = false;
            rd.reply->setStatusCode( 
               NavReplyPacket::
               CHANGED_LICENCE_REPLY_OLD_LICENCE_NOT_IN_ACCOUNT );
            mc2log << warn << "handleChangedLicence: no user with old "
                   << "licence: ";
            NavUserHelp::printLicence( mc2log, plicence17->getBuff(), 
                                       plicence17->getLength() );
            mc2log << endl;
         }
         
         if ( ok && nbrUsers3 > 1 ) {
            // Many users with new licence
            ok = false;
            rd.reply->setStatusCode( 
               NavReplyPacket::
               CHANGED_LICENCE_REPLY_MANY_USERS_WITH_NEW_LICENCE );
            mc2log << warn << "handleChangedLicence: many users with new "
                   << "licence: ";
            NavUserHelp::printLicence( mc2log, plicence3->getBuff(), 
                                       plicence3->getLength() );
            mc2log << endl;
         }

         if ( ok /*&& userItem17 != NULL*/ ) {
            // Check if userItem is user for 17 if not then error "Last key
            //    not in accout"
            if ( userItem17->getUIN() != user->getUIN() ) {
               ok = false;
               rd.reply->setStatusCode( 
                  NavReplyPacket::
                  CHANGED_LICENCE_REPLY_OLD_LICENCE_IN_OTHER_ACCOUNT );
               mc2log << warn << "handleChangedLicence: old licence in "
                      << "other user " 
                      << userItem17->getUser()->getLogonID() << "("
                      << userItem17->getUIN() << ")"
                      << " licence: ";
               NavUserHelp::printLicence( mc2log, plicence17->getBuff(), 
                                          plicence17->getLength() );
               mc2log << endl;  
            }
         }

         if ( ok && !user->mayChangeDevice() ) {
            ok = false;
            rd.reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_UNAUTH_OTHER_HAS_LICENCE );
            mc2log << warn << "handleChangedLicence: User has no device "
                   << "changes left" << endl;
         }

         if ( ok && userItem3 != NULL &&
              !userItem3->getUser()->mayChangeDevice() ) 
         {
            ok = false;
            rd.reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_UNAUTH_OTHER_HAS_LICENCE );
            mc2log << warn << "handleChangedLicence: Other User: "
                   << userItem3->getUser()->getLogonID() << "("
                   << userItem3->getUIN() << ") may not change device "
                   << "from licence: ";
            NavUserHelp::printLicence( mc2log, plicence3->getBuff(), 
                                       plicence3->getLength() );
            mc2log << endl;
         }

         if ( ok ) {
            //       switch 3 and 17 between 3 and 17 users
            //       if no 3 user then just change from 17 to 3 in user
            

            // First fix 17 user (current User)
            // Add new, remove old
            // Really change userItem to it can be used in next request on 
            // socket
            userKey->setLicence( plicence17->getBuff(),
                                 plicence17->getLength());
            userKey->setKeyType( ParserUserHandler::imeiType );
            uint32 nbrRemoved = m_thread->removeAllUserLicenceKey(
             user, userKey );
            UserLicenceKey* newKey = new UserLicenceKey( 0 );
            newKey->setLicence( plicence3->getBuff(), 
                                plicence3->getLength() );
            newKey->setKeyType( ParserUserHandler::imeiType );
            user->addElement( newKey );
            user->useDeviceChange();

            if ( m_userHelp->updateSessionUser( user, rd.session ) ) {
               // Ok 
               user = rd.session->getUser()->getUser();
               mc2log << info << "handleChangedLicence: Changed "
                      << user->getLogonID() << "("
                      << user->getUIN() << ") ok. Added licence ";
               NavUserHelp::printLicence( mc2log, plicence3->getBuff(), 
                                          plicence3->getLength() );
               if ( nbrRemoved > 0 ) {
                  mc2log << " Removed licence ";
                  NavUserHelp::printLicence( mc2log, plicence17->getBuff(),
                                             plicence17->getLength() );
               }
               mc2log << endl;
            } else {
               ok = false;
               rd.reply->setStatusCode( 
                  NavReplyPacket::NAV_STATUS_NOT_OK );
               mc2log << warn << "handleChangedLicence: Failed to change "
                      << "licences in "
                      << user->getLogonID() << "("
                      << user->getUIN() << ")" << endl;
            }
            
            if ( ok ) {
               // Second fix 3 user (if any)
               if ( userItem3 != NULL ) {
                  // Add old, remove new
                  UserItem* userItem3C = new UserItem( *userItem3 );
                  userKey->setLicence( plicence3->getBuff(),
                                       plicence3->getLength());
                  userKey->setKeyType( ParserUserHandler::imeiType );
                  m_thread->removeAllUserLicenceKey( userItem3C->getUser(),
                                                     userKey );
                  newKey = new UserLicenceKey( 0 );
                  newKey->setLicence( plicence17->getBuff(),
                                      plicence17->getLength() );
                  newKey->setKeyType( ParserUserHandler::imeiType );
                  userItem3C->getUser()->addElement( newKey );
                  
                  if ( m_thread->changeUser( userItem3C->getUser(), 
                                             user/*changer*/ ) )
                  {
                     // Ok
                     mc2log << info << "handleChangedLicence: Changed "
                            << userItem3->getUser()->getLogonID() << "("
                            << userItem3->getUIN() << ") ok. Added "
                            << "licence ";
                     NavUserHelp::printLicence( 
                        mc2log, plicence17->getBuff(), 
                        plicence17->getLength() );
                     if ( nbrRemoved > 0 ) {
                        mc2log << " Removed licence ";
                        NavUserHelp::printLicence( 
                           mc2log, plicence3->getBuff(),
                           plicence3->getLength() );
                     }
                     mc2log << endl;
                  } else {
                     ok = false;
                     mc2log << error << "handleChangedLicence: Failed to "
                            << "change licences in " 
                            << userItem3->getUser()->getLogonID() << "("
                            << userItem3->getUIN() << ") Licence ";
                     NavUserHelp::printLicence( 
                        mc2log, plicence17->getBuff(), 
                        plicence17->getLength() );
                     mc2log << " Must be added and Licence ";
                     NavUserHelp::printLicence( 
                        mc2log, plicence3->getBuff(),
                        plicence3->getLength() );
                     mc2log << " Must be removed!" << endl;
                     // Oh, need help to solve this!
                     // Send email with above print
                     char tmpStr[ 200 ];
                     MC2String subject( "Fixup user " );
                     subject.append( userItem3->getUser()->getLogonID() );
                     sprintf( tmpStr, "(%d)", userItem3->getUIN() );
                     subject.append( tmpStr );
                     subject.append( " Add licence " );
                     NavUserHelp::printLicence( subject, 
                                                plicence17->getBuff(), 
                                                plicence17->getLength() );
                     subject.append( " Remove licence " );
                     NavUserHelp::printLicence( subject, 
                                                plicence3->getBuff(),
                                                plicence3->getLength() );
                     subject.append( " Other user " );
                     subject.append( userItem17->getUser()->getLogonID() );
                     sprintf( tmpStr, "(%d)", userItem17->getUIN() );
                     subject.append( tmpStr );
                     // Client must know for certain what to do.
                     // Both 3 and 17 user has same key 3 neither 17
                     // But 17/current user works and hopefully 3 user
                     // gets manualy updated.
//                     rd.reply->setStatusCode( 
//                        NavReplyPacket::NAV_STATUS_NOT_OK );
                     SendEmailRequestPacket* p = 
                        new SendEmailRequestPacket( 0 );
                     if ( p->setData( 
                             "mc2_error@localhost.localdomain", 
                             Properties::getProperty( 
                                "DEFAULT_RETURN_EMAIL_ADDRESS", 
                                "please_dont_reply@localhost.localdomain" ),
                             subject.c_str(), subject.c_str() /*body*/ ) )
                     {
                        PacketContainer* rp = new PacketContainer( 
                           p, 0, 0 , MODULE_TYPE_SMTP );
                        PacketContainer* pc = m_thread->putRequest( rp );
                        if ( pc != NULL && static_cast< ReplyPacket* >( 
                                pc->getPacket() )->getStatus() == 
                             StringTable::OK )
                        {
                           mc2log << info << "handleChangedLicence: email "
                                  << "with instructions sent." << endl;
                        }
                        delete pc;
                     } else {
                        mc2log << error << "handleChangedLicence "
                               << "SendEmailRequestPacket::setData failed."
                               << endl;
                     }
                  } // End else for if changeUser userItem3 ok

                  delete userItem3C;
               }
               
            }

            if ( ok ) {
               mc2log << info << "handleChangedLicence: Updated ok."
                      << endl;
               // And set 3 in rd.rparams, to indicate new value
               rd.rparams.addParam( *plicence3 );
            }

         } // End if ok

      } // Else error set above


      m_thread->releaseUserItem( userItem17 );
      m_thread->releaseUserItem( userItem3 );
      delete userKey;
   } // End else for if 3 or 17 NULL

   return ok;
}


bool 
NavAuthHandler::checkUserExpired( 
   UserItem* userItem, NavRequestPacket* req, NavReplyPacket* reply )
{
   bool ok = true;

   if ( req->getType() != NavPacket::NAV_UPGRADE_REQ &&
        req->getType() != NavPacket::NAV_TUNNEL_DATA_REQ
        /*&& req->getType() != NavPacket::NAV_WHOAMI_REQ 20041220 Client without userID sends WHOAMI and UPGRADE but so does Trial that just do paramSync*/ )
   {
      UserUser* user = userItem->getUser();
      uint32 now = TimeUtility::getRealTime();

      const ClientSetting* clientSetting = 
         m_userHelp->getClientSetting( req->getParamBlock(), user );
      UserEnums::userRightLevel levelmask = 
         m_userHelp->getUrLevel( clientSetting );
      
      ok = false;
      if ( user->getNbrOfType( UserConstants::TYPE_RIGHT ) > 0 ) {
         ok = m_thread->checkAccessToService( user, UserEnums::UR_WF,
                                              levelmask );
      } else { // XXX: Else use old region_access remove when users upd
      for ( uint32 i = 0 ; 
            i < user->getNbrOfType( 
               UserConstants::TYPE_REGION_ACCESS ) ; i++ )
      {
         UserRegionAccess* region = static_cast< 
            UserRegionAccess* > (
               user->getElementOfType( 
                  i, UserConstants::TYPE_REGION_ACCESS ) );
         if ( region->getEndTime() > now  &&
              region->getStartTime() <= now ) 
         {
            // Ok found a valid region access
            ok = true;
            break;
         }
      }
      }
      
      if ( !ok ) {
         mc2log << info << "checkUserExpired: User has no "
                << "regions left. Sending EXPIRED" << endl;
         reply->setStatusCode( NavReplyPacket::NAV_STATUS_EXPIRED_USER );
      }

      if ( ok ) {
         if ( user->getValidDate() < now && user->getValidDate() != 0 ) {
            mc2log << warn << "checkUserExpired: " 
                   << "EXPIRED User validDate " << endl;
            ok = false;
            reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_EXPIRED_USER );
         }
      }

   } // Else upgrade request and no need to check yet

   return ok;
}


bool
NavAuthHandler::checkTransactions( 
   UserItem* userItem, NavRequestPacket* req, NavReplyPacket* reply )
{
   bool ok = true;
   UserUser* user = userItem->getUser();
   
   if ( req->getType() == NavPacket::NAV_UPGRADE_REQ ) {
      // Upgrade request and no need to check yet
   } else if ( user->getTransactionBased() == UserConstants::TRANSACTIONS )
   {
      // Get nbr transactions left for user.
      int32 nbrTransactions = 0;
      if ( m_thread->getAndChangeTransactions( user->getUIN(), 0, 
                                               nbrTransactions ) ) 
      {
         mc2log << info << "checkTransactions: Sending nbr "
                << "transactions left: " << nbrTransactions
                << endl;
         reply->getParamBlock().addParam( 
            NParam( 12, MAX( nbrTransactions, 0 ) ) );
      } else {
         mc2log << warn << "checkTransactions: "
                << "getAndChangeTransactions failed "
                << "can't send nbr transactions left in params."
                << endl;
      }
   } else if ( user->getTransactionBased() == 
               UserConstants::TRANSACTION_DAYS )
   {
      // Get nbr transaction days left for user.
      int32 nbrTransactions = 0;
      uint32 curTime = 0;
      StringTable::stringCode status = 
         m_thread->getAndChangeTransactionDays( 
            user->getUIN(), true, nbrTransactions, curTime );
      if ( status == StringTable::OK ) {
         mc2log << info << "checkTransactions: Sending nbr "
                << "transaction days left: " << nbrTransactions
                << " curTime " << curTime << endl;
         reply->getParamBlock().addParam( 
            NParam( 12, (1<<31) | MAX( nbrTransactions, 0 ) ) );
         reply->getParamBlock().addParam( NParam( 13, curTime ) );
      } else if ( status == StringTable::NOT_ALLOWED ) {
         mc2log << warn << "checkTransactions: "
                << "getAndChangeTransactionDays no more days. Sending "
                << "EXPIRED" << endl;
         reply->setStatusCode( NavReplyPacket::NAV_STATUS_EXPIRED_USER );
         ok = false;
      } else if ( status == StringTable::TIMEOUT_ERROR ) {
         mc2log << warn << "checkTransactions: "
                << "getAndChangeTransactionDays timeout."
                << endl;
         reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
         ok = false;
      } else {
         mc2log << warn << "checkTransactions: "
                << "getAndChangeTransactionDays failed "
                << "can't let user pass."
                << endl;
         reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
         ok = false;
      }
   }

   return ok;
}


bool
NavAuthHandler::checkUserWFST( UserItem* userItem, NavRequestPacket* req, 
                               NavReplyPacket* reply,
                               const ClientSetting* clientSetting )
{
   bool ok = true;
   byte wayfinderType = MAX_BYTE;
   
   if ( req->getParamBlock().getParam( 7 ) != NULL ) {
      wayfinderType = req->getParamBlock().getParam( 7 )->getByte();
   }
   
   byte userwfst = MAX_BYTE;
   int res = m_userHelp->checkWFST( wayfinderType, userItem, clientSetting,
                                    userwfst );
   
   if ( res == 4 ) {
      // Send current subscription level
      reply->getParamBlock().addParam( NParam( 7, userwfst ) );
   } else if ( res == 1 ) {
      reply->setStatusCode( 
         NavReplyPacket::NAV_STATUS_WF_TYPE_TOO_HIGH_LOW );
      ok = false;
   } else if ( res == 2 ) {
      reply->setStatusCode( 
         NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
      ok = false;
   } else if ( res == 3 ) {
      reply->setStatusCode( 
         NavReplyPacket::NAV_STATUS_NOT_OK );
      ok = false;
   }
   
   return ok;
}


bool
NavAuthHandler::checkTrialTime( UserItem* userItem, NavRequestPacket* req, 
                                NavReplyPacket* reply,
                                isabBoxSession* session,
                                const ClientSetting* clientSetting )
{
   bool ok = true;
   UserUser* user = userItem->getUser();
   if ( m_thread->getSubscriptionTypeForUser( user, UserEnums::UR_WF ) == 
        WFSubscriptionConstants::TRIAL )
   {
      // Get time added when created
      uint32 now = TimeUtility::getRealTime();
      uint32 endTime = now;
      m_group->getCreateWFTime( clientSetting, endTime );
      UserEnums::URType trialWf( UserEnums::UR_TRIAL,
                                 UserEnums::UR_WF );
      if ( endTime < MAX_INT32 && endTime != 0 ) { // Wörth checking
         bool changed = false;
         if ( user->getNbrOfType( UserConstants::TYPE_RIGHT ) > 0 ) {
            for ( uint32 i = 0 ; 
                  i < user->getNbrOfType( UserConstants::TYPE_RIGHT ) ; 
                  i++ ) 
            {
               UserRight* r = static_cast<UserRight*> ( 
                  user->getElementOfType( i, UserConstants::TYPE_RIGHT ) );
               if ( !r->isDeleted() && r->getUserRightType() == trialWf &&
                    (/*r->getEndTime() > endTime || Let user have it, trial per client type is the real fix. */
                     r->getEndTime() == 0 ) &&
                    r->getUserRightType().getLevelAsWFST() != 
                    WFSubscriptionConstants::IRON )
                  // Iron rights should not be limited
               {
                  // Waring! Trial user has longer access time 
                  //         than allowed, make it look like it 
                  //         started now.
                  mc2log << warn << "checkTrialTime: TRIAL user "
                         << "has right longer than allowed "
                         << "startTime " << r->getStartTime()
                         << " endtime " << r->getEndTime() 
                         << " type "
                         << r->getUserRightType()
                         << " regionID " << hex << r->getRegionID() << dec 
                         << " Setting new endtime to " << endTime
                         << endl;
                  changed = true;
                  r->setDeleted( true );
                  MC2String origin( "AUTO: Check TRIAL time. Was (" );
                  origin.append( r->getOrigin() );
                  origin.append( ")" );
                  // Add new with new time
                  UserRight* newRight = new UserRight(
                     0, now, r->getUserRightType(), r->getRegionID(),
                     now, endTime, false, origin.c_str() );
                  user->addElement( newRight );
               }
            }

         } else { // XXX: Old region way, remove when users are transformed
         for ( uint32 i = 0 ; i < user->getNbrOfType(
                  UserConstants::TYPE_REGION_ACCESS ) ; i++ )
         {
            UserRegionAccess* region = static_cast< UserRegionAccess* > (
               user->getElementOfType( 
                  i, UserConstants::TYPE_REGION_ACCESS ) );
            if ( region->getEndTime() > endTime ) {
               // Waring! Trial user has longer access time 
               //         than allowed, make it look like it 
               //         started now.
               mc2log << warn << "checkTrialTime: TRIAL user "
                      << "has region access longer than allowed "
                      << "startTime " << region->getStartTime()
                      << " endtime " << region->getEndTime()
                      << " regionID " << hex 
                      << region->getRegionID() << dec 
                      << " Setting new endtime to " << endTime
                      << " regionID " << hex
                      << clientSetting->getCreateRegionID() << dec
                      << endl;
               changed = true;
               region->remove();
               // Add new with new time
               UserRegionAccess* newRegion = new UserRegionAccess(
                  0, clientSetting->getCreateRegionID(), 
                  now, endTime );
               user->addElement( newRegion );
            }
         }
         } // End else use old region way
         if ( changed ) {
            m_userHelp->updateSessionUser( user, session );
         }
      } // End if endTime < MAX_INT32
   } // End if TRIAL user

   return ok;
}


bool
NavAuthHandler::checkIronTrialTime( NavRequestData& rd ) {
   bool ok = true;
   UserItem* setUserItem = NULL;

   ok = m_thread->getUserHandler()->checkIronTrialTime( 
      rd.session->getUser()->getUser(), rd.clientSetting, setUserItem );

   if ( !ok ) {
      mc2log << warn << "checkIronTrialTime failed." << endl;
      rd.reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
   } else if ( setUserItem ) {
      rd.session->setUser( new UserItem( *setUserItem ) );
      m_thread->releaseUserItem( setUserItem );
   }
   
   return ok;
}

bool
NavAuthHandler::checkVersionLock( NavRequestData& rd ) 
{

   if( rd.req->getType() == NavPacket::NAV_UPGRADE_REQ ||
       rd.req->getType() == NavPacket::NAV_TUNNEL_DATA_REQ ) {
      //do not check version lock for these types of messages.
      mc2dbg2 << "[NavAuth] no VL check for TUNNEL and UPGRADE" << endl;
      return true;
   }

   //if the user has active trial rights, do not check version lock.
   const UserUser& user = *rd.session->getUser()->getUser();
   if( user.hasActive( UserEnums::URType(UserEnums::UR_TRIAL, UserEnums::UR_WF) ) ){
      mc2dbg2 << "[NavAuth] no VL while trial" << endl;
      return true;
   }
   
   //no active trial rights, check the version lock.
   const uint32 highest = user.getHighestVersionLock();
   const uint32 required = rd.clientSetting->getVersionLock();
   if ( ! (required <= highest) ) {
      mc2log << info << "[NavAuth] version lock: " << "required(" 
             << required << ") > highest(" << highest << ")" << endl;
   }
   return required <= highest;
}

void
NavAuthHandler::getUser( UserItem*& userItem, MC2String& idStr, 
                         bool& errorSet,
                         NavRequestPacket* req, NavReplyPacket* reply,
                         const char* printName, bool useCache,
                         bool wipeFromCache )
{
   // In case it is set
   m_thread->releaseUserItem( userItem );
   userItem = NULL;

   bool commOk = true; // If true and NULL userItem then Comm. error
   uint32 startTime = TimeUtility::getCurrentTime();
   commOk = m_userHelp->getUserFromParams( req->getParamBlock(), idStr,
                                           userItem, useCache, wipeFromCache );
   uint32 endTime = TimeUtility::getCurrentTime();
      
   if ( !commOk ) { //communication failure
      mc2log << warn << "handleAuth: " << printName 
             << "error getting user from "
             << MC2CITE( idStr.c_str() ) << " error: ";
      if ( endTime - startTime > 3000 ) {
         reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
         mc2log << "Timeout";
      } else {
         reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_NOT_OK );
         mc2log << "Error";
      }
      mc2log << endl;
      errorSet = true;
   }

   if ( !errorSet ) {
      if ( userItem == NULL ) {
         // No such user sorry.
         mc2log << warn << "handleAuth: " << printName 
                << " No such user: " << MC2CITE( idStr.c_str() ) << endl;
         reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER );
         errorSet = true;
      }
   }
}

bool
NavAuthHandler::checkUserIDMatches( const NavRequestData& rd, 
                                    MC2String& clientID ) const {
   bool userIDMatches = false;

   if ( rd.params.getParam( 1 ) != NULL || 
        rd.params.getParam( 9 ) ) 
   {
      clientID = rd.params.getParam( 1 ) != NULL ? 
         rd.params.getParam( 1 )->getString( 
            m_thread->clientUsesLatin1() ) : 
         rd.params.getParam( 9 )->getString( 
            m_thread->clientUsesLatin1() );
      // UIN
      MC2String uinStr;
      STLStringUtility::uint2str( rd.session->getUser()->getUIN(), uinStr );
      if ( StringUtility::strcmp( uinStr.c_str(), 
                                  clientID.c_str() ) == 0 )
      {
         userIDMatches = true;
      }
   } else if ( rd.params.getParam( 8 ) != NULL ) {
      // LogonID
      clientID = rd.params.getParam( 8 )
         ->getString( m_thread->clientUsesLatin1() );
      if ( StringUtility::strcmp( clientID.c_str(), 
                                  rd.session->getUser()->getUser()->
                                  getLogonID() ) == 0 )
      {
         userIDMatches = true;
      }
   }
   
   return userIDMatches;
}
