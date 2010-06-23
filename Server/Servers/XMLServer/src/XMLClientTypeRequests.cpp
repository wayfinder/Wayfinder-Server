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
#include "ClientSettings.h"
#include "XMLServerElements.h"
#include "NamedServerLists.h"

using namespace XMLTool;


bool 
XMLParserThread::xmlParseClientTypeInfoRequest( DOMNode* cur, 
                                                DOMNode* out,
                                                DOMDocument* reply,
                                                bool indent )
try {
   bool ok = true;
   int indentLevel = 1;
   MC2String errorCode;      //errorcode if one should be sent in reply.
   MC2String errorMessage;   //errorMessage if one should be sent in reply

   DOMElement* cti_reply = reply->createElement( 
      X( "client_type_info_reply" ) );
   out->appendChild( cti_reply );

   MC2String transaction_id;
   getAttrib( transaction_id, "transaction_id", cur );
   cti_reply->setAttribute( X( "transaction_id" ), X( transaction_id ) );

   MC2String client_type;
   MC2String client_type_options;

   getAttribValue( client_type, "client_type", cur );
   getAttribValue( client_type_options, "client_type_options", cur );

   const ClientSetting* setting = m_group->getSetting( 
      client_type.c_str(), client_type_options.c_str() );

   MC2String phoneModel;
   MC2String imageExtension;
   MC2String extraRights;

   if ( !m_user->getUser()->getEditUserRights() ) {
      // SECURITY!
      mc2log << warn 
             << "XMLParserThread::xmlParseClientTypeInfoRequest "
             << "Access denied for " << m_user->getUser()->getLogonID()
             << endl;
      errorCode = "-1";
      errorMessage = "Access denied.";
      ok = false;
   } else if ( setting != NULL ) {
      // Add data to reply
      errorCode = "0";
      errorMessage = "";
      phoneModel = setting->getPhoneModel();
      imageExtension = setting->getImageExtension();
      extraRights = setting->getExtraRights();
   } else {
      errorCode = "-700";
      errorMessage = "No such client type!";
      XMLServerUtility::
         appendStatusNodes( cti_reply, reply, 0, false,
                            errorCode.c_str(), errorMessage.c_str() );
   }

   cti_reply->setAttribute( X( "phoneModel" ), X( phoneModel ) );
   cti_reply->setAttribute( X( "imageExtension" ), X( imageExtension ) );
   cti_reply->setAttribute( X( "extraRights" ), X( extraRights ) );

   if ( indent ) {
      XMLUtility::indentPiece( *cti_reply, indentLevel );
   }
   return ok;

} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "[ClientTypeInfo]  " << e.what() << endl;
   XMLServerUtility::
      appendStatusNodes( out->getFirstChild(), reply, 1, false,
                         "-1", e.what() );
   return false;

} 


bool 
XMLParserThread::xmlParseServerListForClientTypeRequest( 
   DOMNode* cur, 
   DOMNode* out,
   DOMDocument* reply,
   bool indent )
try {
   bool ok = true;
   int indentLevel = 1;
   MC2String errorCode;      //errorcode if one should be sent in reply.
   MC2String errorMessage;   //errorMessage if one should be sent in reply

   DOMElement* slct_reply = reply->createElement( 
      X( "server_list_for_client_type_reply" ) );
   out->appendChild( slct_reply );

   MC2String transaction_id;
   getAttrib( transaction_id, "transaction_id", cur );
   slct_reply->setAttribute( X( "transaction_id" ), X( transaction_id ) );

   MC2String client_type;
   MC2String client_type_options;
   MC2String uin;
   MC2String srvt;

   getAttribValue( client_type, "client_type", cur );
   getAttribValue( client_type_options, "client_type_options", cur );
   getAttribValue( uin, "uin", cur );
   getAttribValue( srvt, "srvt", cur );

   const ClientSetting* setting = m_group->getSetting( 
      client_type.c_str(), client_type_options.c_str() );

   if ( !m_user->getUser()->getEditUserRights() ) {
      // SECURITY!
      mc2log << warn 
             << "XMLParserThread::xmlParseServerListForClientTypeRequest "
             << "Access denied for " << m_user->getUser()->getLogonID()
             << endl;
      errorCode = "-1";
      errorMessage = "Access denied.";
      ok = false;
   } else if ( setting != NULL ) {
      // Add data to reply
      errorCode = "0";
      errorMessage = "";
      // Add server list
      if ( srvt.empty() ) {
         srvt = NamedServerLists::XmlServerType;
      }

      MC2String serverListName( setting->getServerListName() );
      if ( setting->getServerListName().empty() ) {
         serverListName = NamedServerLists::DefaultGroup;
      }

      XMLServerUtility::appendServerList( 
         slct_reply, reply, indentLevel, indent,
         m_group->getServerList( serverListName, srvt ) );
   } else {
      errorCode = "-700";
      errorMessage = "No such client type!";
      XMLServerUtility::
         appendStatusNodes( slct_reply, reply, 0, false,
                            errorCode.c_str(), errorMessage.c_str() );
   }

   if ( indent ) {
      XMLUtility::indentPiece( *slct_reply, indentLevel );
   }
   return ok;

} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "[ServerListForClientType]  " << e.what() << endl;
   XMLServerUtility::
      appendStatusNodes( out->getFirstChild(), reply, 1, false,
                         "-1", e.what() );
   return false;

} 


#endif // USE_XML



