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
#include "ParserCWHandler.h"
#include "HttpHeader.h"
#include "HttpInterfaceRequest.h"
#include "XMLAuthData.h"
#include "XMLServerElements.h"
#include "URLAddToer.h"
#include "ClientSettings.h"
#include "XMLTool.h"
#include "NamedServerLists.h"

using namespace URLAddToer;
using XMLServerUtility::appendStatusNodes;
using namespace XMLTool;

bool 
XMLParserThread::xmlParseTunnelRequest( DOMNode* cur, 
                                        DOMNode* out,
                                        DOMDocument* reply,
                                        bool indent )
{
   bool ok = true;
   int indentLevel = 1;

   MC2String errorCode;
   MC2String errorMessage;

   // Create tunnel_reply element
   DOMElement* tunnel_reply = reply->createElement( X( "tunnel_reply" ) );
   // Transaction ID
   tunnel_reply->setAttribute( 
      X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
         X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( tunnel_reply );

   // Data
   const MC2String eol( "\r\n" );
   MC2String urlStr;
   MC2String postData;
   HttpHeader outHeaders;
   MC2String replyStr;
   uint32 fromByte = 0;
   uint32 toByte = MAX_UINT32;
   uint32 startByte;
   uint32 endByte;


   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;
   
   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(),
                              "transaction_id" ) ) 
      {
         // Handled above 
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "url" ) ) 
      {
         urlStr = tmpStr;
      } else {
         mc2log << warn << "XMLParserThread::xmlParseTunnelRequest "
                   "unknown attribute for tunnel_request element "
                << " Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }

   // Get children
   DOMNode* child = cur->getFirstChild();
   
   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(), "post_data" ) ) {
               MC2String tmpStr = XMLUtility::getChildTextStr( *child );
               if ( XMLString::equals( child->getAttributes()->
                       getNamedItem( X( "te" ) )->getNodeValue(),
                                       "base64" ) )
               {
                  // base64decode tmpStr
                  char decBuff[ tmpStr.size() + 1 ];
                  int res = StringUtility::base64Decode( 
                     tmpStr.c_str(), reinterpret_cast<byte*>( decBuff ) );
                  if ( res < 0 ) {
                     ok = false;
                     errorCode = "-1";
                     errorMessage = "post_data does not have correct "
                        "base64 encoding.";
                  } else {
                     decBuff[ res ] = '\0';
                     postData = decBuff;
                  }
               } else {
                  postData = tmpStr;
               }
            } else {
               mc2log << warn << "XMLParserThread::"
                         "xmlParseTunnelRequest "
                         "odd Element in tunnel_request element: " 
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break; 
         default:
            mc2log << warn << "XMLParserThread::"
                      "xmlParseTunnelRequest odd "
                      "node type in tunnel_request element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }


   if ( ok ) {
      // TODO: If no user (auth_activate_request) allow only some urls.
      //       The same for WFID client with no user.

      // WFID
      if ( m_authData->clientSetting->getWFID() || 
           m_authData->clientSetting->getNotWFIDButUsesServices() ) {
         // Add some to url
         bool first = urlStr.find( "?" ) == MC2String::npos;
         // su (server uin)
         if ( m_user != NULL ) {
            addToURL( first, urlStr, "su", m_user->getUser()->getUIN() );
         } else {
            addToURL( first, urlStr, "su", "" );
         }
         // hw keys
         // ADD key(s) hwd and hwdt for each key 
         for ( UserLicenceKeyVect::const_iterator key = 
                  m_authData->hwKeys.begin();
               key != m_authData->hwKeys.end() ; ++key ) {
            addToURL( first, urlStr, "hwd", (*key).getLicenceKeyStr() );
            addToURL( first, urlStr, "hwdt", (*key).getKeyType() );
         }

         // External auth
         if ( !m_authData->externalAuth.empty()  ) {
            // External auth failed
            if ( m_user == NULL ) {
               const char* extAuthErr = "error";
               if ( m_authData->externalAuthErrorCode == "-401" ) {
                  // -401 user not from
                  extAuthErr = "unauthorized";
               } else if ( m_authData->externalAuthErrorCode == "-402" ) {
                  // -402 Not billable/Expired
                  extAuthErr = "expired";
               }
               addToURL( first, urlStr, "extautherr", extAuthErr );
            } else if ( m_authData->clientSetting->usesRights() && 
                        ! checkAccessToService( 
                           m_user->getUser(), 
                           m_authData->urmask.service(),
                           m_authData->urmask.level() ) ) {
               addToURL( first, urlStr, "extautherr", "expired" );
            }
            addToURL( first, urlStr, "extauth", m_authData->externalAuth );
         }
         // Add server type srvt
         addToURL( first, urlStr, "srvt", NamedServerLists::XmlServerType );
      }

      int ures = getCWHandler()->getURL( 
         urlStr, postData, m_authData->ipnport.getIP(), fromByte, toByte,
         m_authData->clientLang, m_irequest->getRequestHeader(),
         outHeaders, replyStr, startByte, endByte );
      mc2dbg8 << "ures " << ures << endl;

      MC2String statusLine = *outHeaders.getStartLine();
      // Remove endline
      statusLine = StringUtility::trimStartEnd( statusLine );
      tunnel_reply->setAttribute( X( "status_line" ), X( statusLine ) );

      for ( HttpHeader::HeaderMap::const_iterator it = 
               outHeaders.getHeaderMap().begin() ; 
            it != outHeaders.getHeaderMap().end() ; ++it )
      {
         DOMElement* el = reply->createElement( X( "header" ) );
         el->setAttribute( X( "field" ), X( it->first ) );
         el->setAttribute( X( "value" ), X( *it->second ) );
         tunnel_reply->appendChild( el );
      }

      if ( !replyStr.empty() ) {
         using namespace XMLServerUtility;
         MC2String data = StringUtility::base64Encode( replyStr );
         attrVect attr; 
         attr.push_back ( stringStringStruct( "te", "base64" ) );

         appendElementWithText( tunnel_reply, reply, "body", data.c_str(), 
                                indentLevel + 1, false,  &attr );
      }

   } else {
      // An error in errorCode, errorMessage
      appendStatusNodes( tunnel_reply, reply, 
                         indentLevel + 1, false, 
                         errorCode.c_str(), errorMessage.c_str() );
      // Error handled 
      ok = true;
   }

   // Indent 
   if ( indent ) {
      XMLUtility::indentPiece( *tunnel_reply, indentLevel );
   }

   return ok;
}

#endif // USE_XML

