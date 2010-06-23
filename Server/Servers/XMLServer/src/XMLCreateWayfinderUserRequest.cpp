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


#ifdef USE_XML
#include "XMLTool.h"
#include "UserLicenceKey.h"
#include "ClientSettings.h"
#include "XMLServerElements.h"
#include "NamedServerLists.h"
#include "ParserUserHandler.h"
#include "XMLAuthData.h"

using namespace XMLTool;


bool 
XMLParserThread::xmlParseCreateWayfinderUserRequest( DOMNode* cur, 
                                                     DOMNode* out,
                                                     DOMDocument* reply,
                                                     bool indent )
try {
   bool ok = true;
   int indentLevel = 1;
   MC2String errorCode;      //errorcode if one should be sent in reply.
   MC2String errorMessage;   //errorMessage if one should be sent in reply

   DOMElement* cwu_reply = reply->createElement( 
      X( "create_wayfinder_user_reply" ) );
   out->appendChild( cwu_reply );

   MC2String transaction_id;
   getAttrib( transaction_id, "transaction_id", cur );
   cwu_reply->setAttribute( X( "transaction_id" ), X( transaction_id ) );

   // Get input
   MC2String client_type;
   getAttribValue( client_type, "client_type", cur );
   MC2String client_type_options;
   getAttribValue( client_type_options, "client_type_options", cur );
   MC2String logon;
   getAttribValue( logon, "logon", cur );
   MC2String password;
   getAttribValue( password, "password", cur );
   MC2String activation_code;
   getAttribValue( activation_code, "activation_code", cur );
   uint32 activationRegionID = MAX_INT32; // The selected region for ac
   getAttribValue( activationRegionID, "top_region_id", cur );
   MC2String client_lang_str;
   getAttribValue( client_lang_str, "client_lang", cur );
   LangTypes::language_t client_lang = getStringAsLanguage( 
      client_lang_str.c_str() );

   const ClientSetting* setting = m_group->getSetting( 
      client_type.c_str(), client_type_options.c_str() );
   m_authData->clientSetting = setting;

   // Hardware key(s)
   DOMNodeList* hwdNodes = static_cast<DOMElement*>( cur )
      ->getElementsByTagName( X( "hardware_key" ) );
   for ( uint32 i = 0 ; i < hwdNodes->getLength() ; ++i ) {
      m_authData->hwKeys.push_back( UserLicenceKey( MAX_UINT32 ) );
      m_authData->hwKeys.back().setProduct( 
         m_authData->clientSetting->getProduct() );
      MC2String type;
      XMLTool::getAttribValue( type, "type", hwdNodes->item( i ) );
      m_authData->hwKeys.back().setKeyType( type );
      m_authData->hwKeys.back().setLicence( 
         XMLUtility::getChildTextStr( *hwdNodes->item( i ) ) );
   }
   // Find and set best
   getUserHandler()->setBestLicenceKey( m_authData->hwKeys, 
                                        m_authData->hwKey );

   // The created user's UIN
   uint32 uin = 0;
   // If redirect this is the server
   MC2String server;
   
   if ( !ok ) {
      // Error above
   } else if ( !m_user->getUser()->getEditUserRights() ) {
      // SECURITY!
      mc2log << warn 
             << "XMLParserThread::xmlParseCreateWayfinderUserRequest "
             << "Access denied for " << m_user->getUser()->getLogonID()
             << endl;
      errorCode = "-1";
      errorMessage = "Access denied.";
      ok = false;
   } else if ( m_authData->clientSetting != NULL ) {
      // Check if licenceKey is already used! Before adding it to new user.
      uint8 status = 0;
      UserItem* userItem = NULL;
      uint32 nbrUsers = 0;
      UserLicenceKey userKey( m_authData->hwKey );
      ok = getUserFromUserElement( &userKey, 
                                   userItem, nbrUsers, true );
      if ( !ok || nbrUsers != 0 ) {
         ok = false;
         if ( nbrUsers != 0 ) {
            if ( nbrUsers == 1 && userItem != NULL ) {
               mc2log << warn 
                      << "XMLParserThread::xmlParseCreateWayfinderUserRequest "
                      << "Licence key already used removing key from "
                      << userItem->getUser()->getLogonID()
                      << "(" << userItem->getUIN() << ")"<< endl;
               UserUser cuser( *userItem->getUser() );
               removeAllUserLicenceKey( &cuser, &m_authData->hwKey );
               if ( !changeUser( &cuser, NULL ) ) {
                  mc2log << warn 
                         << "XMLParserThread::"
                         << "xmlParseCreateWayfinderUserRequest "
                         << "Licence key already used failed to remove key "
                         << "from user, returning error." << endl;
                  status = 3;
               } else {
                  ok = true; // Ok to create user
               }
            } else if ( nbrUsers == 1 && userItem == NULL ) {
               mc2log << warn 
                      << "XMLParserThread::xmlParseCreateWayfinderUserRequest "
                      << "Licence key already used "
                      << "failed to get user, returning error." << endl;
               status = 2;
            } else {
               mc2log << warn 
                      << "XMLParserThread::xmlParseCreateWayfinderUserRequest "
                      << "Licence key already used by " << nbrUsers 
                      << " users, returning error." << endl;
               status = 3;
            }
         } else {
            mc2log << warn 
                   << "XMLParserThread::xmlParseCreateWayfinderUserRequest "
                   << "Licence key check failed returning error." << endl;
            status = 3;
         }

         if ( status == 2 ) {
            errorCode = "-3";
            errorMessage = "Timeout creating user.";
         } else {
            errorCode = "-1";
            errorMessage = "Error creating user";
         }

         releaseUserItem( userItem );
         userItem = NULL;
      }

      if ( ok ) {
         // Create user
         userItem = NULL;
         uint32 now = TimeUtility::getRealTime();
         const uint32 createStartTime = now - 24*60*60;
         const uint32 createEndTime = ParserUserHandler::addTime( 
            setting->getCreateRegionTimeYear(),
            setting->getCreateRegionTimeMonth(),
            setting->getCreateRegionTimeDay(),
            setting->getExplicitCreateRegionTime(), now );
         UserLicenceKey userKey( m_authData->hwKey );

         ok = createWayfinderUserForXML( 
            errorCode, errorMessage, password, server, &userKey, 
            userItem,
            createStartTime, createEndTime, setting->getCreateRegionID(),
            client_lang, client_type.c_str(), client_type_options.c_str(), 
            /*cpvArray*/NULL, activation_code, activationRegionID,
            ParserUserHandler::ExtraTypes(), setting->getCreateLevel(),
            setting->getCreateTransactionDays(), setting->getBrand(), 
            logon.c_str(), NULL/*extraComment*/, 
            ParserUserHandler::UserElementVector(), 
            NULL,  // userNamePrefix
            NULL, NULL, // email, name
            "", "", // opt in name, opt in value
            setting->getExtraRights(),
            &userKey /*The key for auto IMEI activation*/, 
            true/*Yes check and use ac if possible*/ );
         status = ok ? 0 : 3;
         if ( errorCode == "-211" ) {
            if ( getServerList( server ) == NULL ) {
               // No server list so can't change, fix configuration
               mc2log << warn 
                   << "XMLParserThread::xmlParseCreateWayfinderUserRequest "
                      << "No server list " << server
                      << " returning error." << endl;
               errorCode = "-1";
               errorMessage = "Failed to create user ";
            }
         }
      }

      if ( status == 0 ) {
         // Set the reply value
         uin = userItem->getUIN();
      }

      releaseUserItem( userItem );
   } else {
      errorCode = "-700";
      errorMessage = "No such client type!";
      ok = false;
   }

   if ( !ok ) {
      XMLServerUtility::
         appendStatusNodes( cwu_reply, reply, 0, false,
                            errorCode.c_str(), errorMessage.c_str() );
      if ( errorCode == "-211" ) {
         XMLServerUtility::
            appendServerList( cwu_reply, reply, indentLevel, indent,
                              getServerList( server ) );
      }
      ok = true; // Handled error
   }

   // Set data in reply
   if ( uin != 0 ) {
      cwu_reply->setAttribute( X( "uin" ), XUint32( uin ) );
   }

   if ( indent ) {
      XMLUtility::indentPiece( *cwu_reply, indentLevel );
   }
   return ok;

} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "[CreateWayfinderUser]  " << e.what() << endl;
   XMLServerUtility::
      appendStatusNodes( out->getFirstChild(), reply, 1, false,
                         "-1", e.what() );
   return false;

} 

#endif // USE_XML
