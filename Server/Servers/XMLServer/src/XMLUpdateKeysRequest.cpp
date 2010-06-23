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
#include "XMLServerElements.h"
#include "ParserUserHandler.h"
#include "XMLAuthData.h"
#include "ClientSettings.h"

using namespace XMLTool;


bool 
XMLParserThread::xmlParseUpdateKeysRequest( DOMNode* cur, 
                                            DOMNode* out,
                                            DOMDocument* reply,
                                            bool indent )
try {
   bool ok = true;
   int indentLevel = 1;
   MC2String errorCode;      //errorcode if one should be sent in reply.
   MC2String errorMessage;   //errorMessage if one should be sent in reply

   DOMElement* uhk_reply = reply->createElement( 
      X( "update_hardware_keys_reply" ) );
   out->appendChild( uhk_reply );

   MC2String transaction_id;
   getAttrib( transaction_id, "transaction_id", cur );
   uhk_reply->setAttribute( X( "transaction_id" ), X( transaction_id ) );

   // Get input
   uint32 uin;
   getAttribValue( uin, "uin", cur );
   MC2String client_type;
   getAttribValue( client_type, "client_type", cur );
   MC2String client_type_options;
   getAttribValue( client_type_options, "client_type_options", cur );
   const ClientSetting* setting = m_group->getSetting( 
      client_type.c_str(), client_type_options.c_str() );
   // Hardware key(s)
   DOMNodeList* hwdNodes = static_cast<DOMElement*>( cur )
      ->getElementsByTagName( X( "hardware_key" ) );
   for ( uint32 i = 0 ; i < hwdNodes->getLength() ; ++i ) {
      m_authData->hwKeys.push_back( UserLicenceKey( MAX_UINT32 ) );
      m_authData->hwKeys.back().setProduct( 
         setting->getProduct() );
      MC2String type;
      XMLTool::getAttribValue( type, "type", hwdNodes->item( i ) );
      m_authData->hwKeys.back().setKeyType( type );
      m_authData->hwKeys.back().setLicence( 
         XMLUtility::getChildTextStr( *hwdNodes->item( i ) ) );
   }
   // Find and set best
   getUserHandler()->setBestLicenceKey( m_authData->hwKeys, 
                                        m_authData->hwKey );

   // Get the user
   UserItem* userItem = NULL;
   bool commOk = getUser( uin, userItem, true );

   if ( !ok ) {
      // Error above
   } else if ( !m_user->getUser()->getEditUserRights() ) {
      // SECURITY!
      MC2String logonIDString;
      
      mc2log << warn 
             << "XMLParserThread::xmlParseUpdateKeysRequest "
             << "Access denied to user " 
             << (userItem ? userItem->getUser()->getLogonID() : 
                 "[No such user]")
             << "(" << uin << ")"
             << " for " << m_user->getUser()->getLogonID()
             << "(" << m_user->getUIN() << ")"
             << endl;
      errorCode = "-1";
      errorMessage = "Access denied.";
      ok = false;
   } else if ( userItem != NULL ) {
      // Licence to user
      int status = getUserHandler()->licenceTo( 
         userItem->getUser(), &m_authData->hwKey );
      ok = status == 0;

      if ( status == 0 ) {
         errorCode = "0";
         errorMessage = "";
      } else if ( status == -2 ) {
         errorCode = "-3";
         errorMessage = "Timeout updating user.";
      } else {
         errorCode = "-1";
         errorMessage = "Error updating user";
      }
   } else {
      // Check getUser status
      errorCode = "-1";
      if ( !commOk ) {
         errorMessage = "Database connection error, no connection.";
      } else {
         errorMessage = "No such user!";
      }

      ok = false;
   }

   releaseUserItem( userItem );

   if ( !ok ) {
      XMLServerUtility::
         appendStatusNodes( uhk_reply, reply, 0, false,
                            errorCode.c_str(), errorMessage.c_str() );
      ok = true; // Handled error
   }

   if ( indent ) {
      XMLUtility::indentPiece( *uhk_reply, indentLevel );
   }
   return ok;

} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "[UpdateKeys]  " << e.what() << endl;
   XMLServerUtility::
      appendStatusNodes( out->getFirstChild(), reply, 1, false,
                         "-1", e.what() );
   return false;

} 

#endif // USE_XML
