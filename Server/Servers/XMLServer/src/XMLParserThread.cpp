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
#include "HttpParserThreadGroup.h"
#include "StringUtility.h"
#include "SinglePacketRequest.h"
#include "UserData.h"
#include "XMLParserEntityResolver.h"
#include "HttpHeader.h"
#include "HttpBody.h"
#include "SearchReplyPacket.h"
#include "ParserTokenHandler.h"
#include "Properties.h"
#include "HttpInterfaceRequest.h"
#include "ParserExternalAuth.h"
#include "XMLExtServiceHelper.h"
#include "ParserExternalAuthHttpSettings.h"
#include "SearchParserHandler.h"
#include "XMLAuthData.h"
#include "ClientSettings.h"
#include "LangUtility.h"
#include "STLStringUtility.h"
#include "PurchaseOptions.h"
#include "XMLServerUtility.h"
#include "XMLSearchUtility.h"
#include "XMLServerElements.h"
#include "InfoTypeConverter.h"
#include "HttpHeaderLines.h"
#include "XMLTool.h"
#include "XSData.h"
#include "NetUtility.h"


#include <sstream>
#include <iomanip>

using XMLServerUtility::appendStatusNodes;
using namespace HttpHeaderLines;
using namespace XMLTool;

namespace {

/**
 * Creates a document.
 * @param documentType The documentType, default isab-mc2.
 * @param charset The character set to use, default iso-8859-1.
 * @param standAlone The information whether the document is 
 *                   standalone or not, default "".
 * @param xmlVersion The version of XML, default 1.0.
 */
DOMDocument* 
makeDocument( const char* documentType = "isab-mc2",
              const char* charset = "iso-8859-1",
              const char* standAlone = "",
              const char* xmlVersion = "1.0" ) {
   DOMImplementation* impl = 
      DOMImplementationRegistry::
      getDOMImplementation( X( "LS" ) ); // Load Save

   DOMDocument* reply = impl->
      createDocument( NULL,      // root element namespace URI.
                      X( documentType ),  // root element name
                      NULL ); // document type object (DTD).

   // Create the DOCTYPE
   reply->insertBefore( reply->createDocumentType( X( documentType ) ),
                        reply->getDocumentElement() );


   return reply;
}

/**
 * Makes a document with status_code and status_message.
 *
 * @param code String with the code of the status.
 * @param message String with the message of the status.
 * @param indent If indent.
 * @param documentType The documentType, default isab-mc2.
 * @param charset The character set to use, default iso-8859-1.
 * @param standAlone The information whether the document is 
 *                   standalone or not, default "".
 * @param xmlVersion The version of XML, default 1.0.
 */
DOMDocument* 
makeErrorDocument( const char* const code,
                   const char* const message,
                   bool indent,
                   const char* documentType = "isab-mc2",
                   const char* charset = "iso-8859-1",
                   const char* standAlone = "",
                   const char* xmlVersion = "1.0") {
   DOMDocument* errDoc = makeDocument( documentType, charset, 
                                       standAlone, xmlVersion );
   DOMElement* isabmc2 = errDoc->getDocumentElement();
   XMLServerUtility::
      appendStatusNodes( isabmc2, errDoc, 1, indent, code, message );
   // Indent and linebreak before ending isab-mc2 tag
   if ( indent ) {
      isabmc2->appendChild( errDoc->createTextNode( X( "\n" ) ) );
   }

   return errDoc;
}

/**
 * Sets the body to a xml-error-document.
 *
 * @param outHead The HTTP header of the reply.
 * @param outBody The content of the reply.
 * @param inHead  The in-header.
 * @param code String with the code of the status.
 * @param message String with the message of the status.
 * @param indent If indent.
 */
void 
makeErrorReply( HttpBody* outBody,
                HttpHeader* outHead,
                const HttpHeader& inHead,
                const char* const code, 
                const char* const message,
                bool indent,
                const char* documentType = "isab-mc2",
                const char* standAlone = "",
                const char* xmlVersion = "1.0" ) {
   
   DOMDocument* errReply = 
      makeErrorDocument( code, message, 
                         indent, documentType, 
                         inHead.getAcceptCharset().c_str(),
                         standAlone, 
                         xmlVersion );
   MC2String replyString =
      XMLTreeFormatter::makeStringOfTree( errReply,
                                          inHead.getAcceptCharset().c_str() );
   outHead->addHeaderLine( &XMLParserThread::ContentTypeStr,
                           new MC2String( "text/xml" ) );
   outBody->setBody( &replyString );
   outBody->setCharSet( inHead.getAcceptCharset().c_str() );
   mc2log << info << "XMLParserThread::makeErrorReply reply:"
          << endl;
   XMLServerUtility::printWithLineNumbers( mc2log, replyString.c_str() );
   // Return memory for reply
   errReply->release();
}

/**
 * Return the reply document name for a request name, returns
 * request name if not known.
 *
 * @param requestName The name of the request.
 * @return The reply document name for the request name.
 */
const char* getReplyDocumentName( const char* requestName ) {
   if ( strcmp( requestName, "request" ) == 0 ) {
      return "reply";
   } else {
      return requestName;
   }
}

}


bool equalVanillaMatch::operator()( const VanillaMatch* x,
                                    const VanillaMatch* y ) const {
           return (x->getMapID() == y->getMapID() && 
                   x->getMainItemID() == y->getMainItemID() );
};


const MC2String XMLParserThread::ContentTypeStr = "Content-Type";


XMLParserThread::XMLParserThread( HttpParserThreadGroup* group ) 
   : HttpParserThread( group ),
     m_infoTypes( new InfoTypeConverter() ) {

   m_extServiceHelper = new XMLExtServiceHelper(this,
                                                group );
   clearUserItem();
   m_authData = new XMLAuthData();
   // Static service and server strings
   char serverName[ 512 ];

   strcpy( serverName, NetUtility::getLocalHostName().c_str() );

   char* dotPos = strchr( serverName, '.' );
   if ( dotPos != NULL ) {
      // End hostname at first dot.
      *dotPos = '\0';
   }

   m_serverName = new char[ strlen( serverName ) + 1 ];
   strcpy( m_serverName, serverName );

#ifdef USE_XML
   m_parser = new XercesDOMParser();
   m_parser->setValidationScheme( XercesDOMParser::Val_Auto );
   ErrorHandler* errReporter = new XMLParserErrorReporter();
   m_parser->setErrorHandler( errReporter );
//   m_parser->setToCreateXMLDeclTypeNode( true );
   m_parser->setIncludeIgnorableWhitespace( false );
   m_parser->cacheGrammarFromParse( true ); // Cache this
   delete m_parser->getEntityResolver(); // Any old one
   m_parser->setEntityResolver( new XMLParserEntityResolver() );

   // Initialize with isab-mc2.dtd to be able to resuse grammar
   bool ok = true;
   const char* xmlReqStr = 
      "<?xml version='1.0' encoding='ascii' ?>\r\n"
      "<!DOCTYPE isab-mc2 SYSTEM 'isab-mc2.dtd'>\r\n"
      "<isab-mc2>\r\n"
      "<auth><auth_user></auth_user><auth_passwd></auth_passwd></auth>\r\n"
      "<user_login_request transaction_id='a'>"
      "<user_name></user_name><user_password></user_password>"
      "</user_login_request>\r\n"
      " <!-- comment -->\r\n"
      "</isab-mc2>\r\n";
   MemBufInputSource xmlBuff( (byte*)xmlReqStr, 
                              strlen( xmlReqStr ), "xmlInitRequest" );
   const char* pubXmlReqStr =
      "<?xml version='1.0' encoding='ascii' ?>\r\n"
      "<!DOCTYPE request SYSTEM 'public.dtd'>\r\n"
      "<request>\r\n"
      "<auth login='mylogin' password='mypass'/>\r\n"
      "<user_favorites_request transaction_id='U1'/>\r\n"
      "</request>\r\n";
   MemBufInputSource pubXmlBuff( (byte*)pubXmlReqStr, 
                                 strlen( pubXmlReqStr ), 
                                 "pubXmlInitRequest" );
   try {
      m_parser->parse( xmlBuff );
      m_parser->parse( pubXmlBuff );
   } catch ( const XMLException& e ) {
      mc2log << error 
             << "XMLParserThread::XMLParserThread an XMLerror occured "
             << "during parsing of initialize request: "
             << e.getMessage() << " line " 
             << e.getSrcLine() << endl;
      ok = false;
   } catch( const SAXParseException& e) {
      mc2log << error
             << "XMLParserThread::XMLParserThread an SAXerror occured "
             << "during parsing of initialize request: "
             << e.getMessage() << ", "
             << "line " << e.getLineNumber() << ", column " 
             << e.getColumnNumber() << endl;
      ok = false;
   }
   if ( !ok ) {
      mc2log << fatal << "XMLParserThread::XMLParserThread "
             << "initialization of xml parser failed, quiting." << endl;
      exit( 1 );
   }
   m_parser->cacheGrammarFromParse( false ); // Cache no more
   m_parser->useCachedGrammarInParse( true ); // Use cached grammar
#endif
#if 0
   HttpParserThreadGroup* group2 = (HttpParserThreadGroup*)10;
   if ( group ) {
      group2 = NULL; 
   }
   cout << group2->getCache();
#endif
}


XMLParserThread::~XMLParserThread() {
   delete m_extServiceHelper;
   delete [] m_serverName;
   delete m_authData;
#ifdef USE_XML
   delete m_parser->getErrorHandler();
   delete m_parser->getEntityResolver();
   delete m_parser;
#endif
}


bool 
XMLParserThread::handleHttpRequest( HttpHeader* inHead, 
                                    HttpBody* inBody,
                                    stringMap* paramsMap,
                                    HttpHeader* outHead, 
                                    HttpBody* outBody,
                                    uint32 now,
                                    const char* dateStr )
{
   // Check if its a XMLRequest in the HTTP request
   bool xmlRequest = false;
   bool xorIt = false;

   // Check for special files ( e.g. Echo )
   bool specialFileHandledResult = true;
   const bool specialFileHandled = handleSpecialFiles( 
      inHead, inBody, paramsMap, outHead, outBody, now,
      specialFileHandledResult );
   if ( specialFileHandled ) {
      // Special file handled - return.
      return specialFileHandledResult;
   }
   
   const MC2String* contentType = inHead->getHeaderValue( 
         ContentTypeStr.c_str() );
   if ( contentType == NULL ) {
      contentType = inHead->getHeaderValue( "Content-type" ); 
   }
   const MC2String* pageName = inHead->getPagename();

   if ( inHead->getMethod() == HttpHeader::POST_METHOD &&
        inBody->getBodyLength() >= 0 &&
        ( (contentType != NULL &&
           StringUtility::strcasecmp( contentType->c_str(), 
                                      "text/xml" ) == 0 ) ||
          (pageName != NULL && 
           (StringUtility::strcasecmp( pageName->c_str(), 
                                       "xmlfile" ) == 0 ||
            StringUtility::strcasecmp( pageName->c_str(), 
                                       "xs" ) == 0 ||
            StringUtility::strcasecmp( pageName->c_str(), 
                                       "xsdata" ) == 0)) ) )
   {
      xmlRequest = true;
      if ( StringUtility::strcasecmp( pageName->c_str(), 
                                      "xsdata" ) == 0 ) {
         xorIt = true;
      }
   }

   if ( xorIt ) {
      XSData::decodeXSBuffer( *inBody );
      // Treat it as text
      inBody->setBinary( false );
   }

   bool res = true;

   if ( !xmlRequest ) {
      // Let HttpParserThread handle it.
      res = HttpParserThread::handleHttpRequest( inHead, 
                                                 inBody,
                                                 paramsMap,
                                                 outHead, 
                                                 outBody,
                                                 now,
                                                 dateStr );
   } else { // Handle XMLRequest
      res = handleXMLHttpRequest( inHead, 
                                  inBody,
                                  paramsMap,
                                  outHead, 
                                  outBody,
                                   now,
                                  dateStr );
   }


   if ( xorIt ) {
      XSData::encodeXSBuffer( *outBody );

      // Set content type
      outHead->addHeaderLine( &CONTENT_TYPE, 
                              new MC2String( "application/binary" ) );
      // Remove Accept-Encoding to avoid encoding it again in HttpParserThread
      inHead->deleteHeaderLine( &ACCEPT_ENCODING );
   }

   return res;
}


#ifdef USE_XML


bool
XMLParserThread::doHttpAuth( UserItem*& user, MC2String& unauthorizedStr,
                             MC2String& errorCode, 
                             const MC2String& extType, 
                             LangTypes::language_t clientLang,
                             const MC2String& currentID,
                             const MC2String& newID,
                             bool checkTime )
{
   bool authorized = false;
   const MC2String USERAGENT = "User-Agent";
   if ( m_irequest->getRequestHeader()->getHeaderValue( &USERAGENT ) != 
        NULL )
   {
      if ( m_authData->clientType.empty() ) {
         m_authData->clientType = 
            *m_irequest->getRequestHeader()->getHeaderValue( &USERAGENT );
      }
   }

   int status = 0;
   UserItem* setUserItem = NULL;
   int ret = getExternalAuth()->handleHttpHeaderRequest( 
      m_irequest, user, extType, *m_authData, ""/*activationCode*/, 
      status, setUserItem, user ? user->getUser()->getLogonID() : "",
      currentID, newID );
   
   if ( ret == 0 ) {
      // Set user
      releaseUserItem( user );
      user = setUserItem;

      if ( m_authData->clientSetting->usesRights() ) {
         // checkUserAccess
         authorized = checkUserAccess( user->getUser(), "JAVA", checkTime );
         if ( ! authorized ) {
            // No access
            if ( checkUserAccess( user->getUser(), "JAVA", false ) &&
                 m_allowedRequests.find( "tunnel_request" ) != 
                 m_allowedRequests.end() ) {
               // Tunnel allowed when expired
               authorized = true;
            } else {
               unauthorizedStr = extType + " authentication: Access denied.";
               errorCode = "-402";
            }
         }
      } else {
         // Client do not use rights
         authorized = true;
      }
   } else if ( status == -2 ) {
      unauthorizedStr = extType + " authentication: timed out.";
      errorCode = "-3"; 
   } else if ( status == -3 ) {
      // "-402"
      unauthorizedStr = extType + " authentication: Expired.";
      errorCode = "-402"; 
   } else if ( status == -4 ) {
      unauthorizedStr = extType + " authentication: user not from "
         + extType + ".";
      errorCode = "-401"; 
   } else {
      unauthorizedStr = extType + " authentication: error.";
      errorCode = "-1"; 
   }

   if ( !authorized ) {
      mc2log << warn << "doHttpAuth " << extType;
      if ( user != NULL ) {
         mc2log << " user " 
                << user->getUser()->getLogonID() << "(" << user->getUIN() 
                << ") ";
      } else {
         mc2log << " unknown user ";
      }
      mc2log << "Not authenticated. Error: " << errorCode << " " 
             << unauthorizedStr << endl;
   }

   return authorized;
}


bool 
XMLParserThread::checkAuthorization( const DOMDocument* doc,
                                     DOMDocument* reply,
                                     bool& indent,
                                     bool& development,
                                     const HttpHeader& inHeaders )
{
   mc2dbg4 << "XMLParserThread::checkAuthorization" << endl;
   volatile uint32 startProcessTime = TimeUtility::getCurrentMicroTime();
   

   int indentLevel = 1;
   const DOMNode* node = doc;
   bool authorized = false;
   MC2String unauthorizedStr = "unauthorized";
   MC2String errorCode = "-1";
   MC2String errorURL = "";
   MC2String newTokenStr;
   const MC2String userAgent( "User-Agent" );
   bool sendUpdate = false;
   bool justPrintError = false;
   bool clientUserIDDoesNotMatch = false;

   if ( m_irequest->getRequestHeader()->getHeaderValue( userAgent ) 
        != NULL )
   {
      m_authData->clientType = *m_irequest->getRequestHeader()
         ->getHeaderValue( userAgent );
   }

   m_authData->ipnport = m_irequest->getPeer();

   DOMNodeList* auths = doc->getElementsByTagName( X( "auth" ) );

   if ( auths->getLength() == 1 ) {
      // Get the auth element (only the first is valid isabmc2 
      //                       as described in the DTD)
      node = auths->item( 0 );

      // Check attributes to auth-element
      DOMNamedNodeMap* attributes = node->getAttributes();
      DOMNode* attribute;
   
      for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
         attribute = attributes->item( i );
      
         char* tmpStr = XMLUtility::transcodefromucs( 
            attribute->getNodeValue() );
         if ( XMLString::equals( attribute->getNodeName(),
                                 "indentingandlinebreaks" ) )
         {
            indent = StringUtility::checkBoolean( tmpStr );
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "development" ) ) 
         {
            development = StringUtility::checkBoolean( tmpStr );          
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "client_type" ) ) 
         {
            m_authData->clientType = tmpStr;
            m_authData->clientTypeSet = true;
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "client_lang" ) ) 
         {
            m_authData->clientLang = getStringAsLanguage( tmpStr );
            m_authData->stringtClientLang = 
               ItemTypes::getLanguageTypeAsLanguageCode(
                  m_authData->clientLang );
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "server_list_crc" ) ) 
         {
            char* tmpPtr = NULL;
            m_authData->server_list_crc = strtoul( tmpStr, &tmpPtr, 16 );
            if ( tmpPtr != tmpStr || tmpStr[ 0 ] == '\0' ) {
               m_authData->server_list_crcSet = true;
            } else {
               mc2log << warn << "XMLParserThread::checkAuthorization "
               "server_list_crc not valid " << tmpStr << endl;
               errorCode = "-1";
               unauthorizedStr = "server_list_crc not valid.";
            }
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "server_auth_bob_crc" ) ) 
         {
            char* tmpPtr = NULL;
            m_authData->server_auth_bob_crc = 
               strtoul( tmpStr, &tmpPtr, 16 );
            //string should be a number or empty
            if ( tmpPtr != tmpStr || tmpStr[ 0 ] == '\0' ) {
               m_authData->server_auth_bob_crcSet = true;
            } else {
               mc2log << warn << "XMLParserThread::checkAuthorization "
               "server_auth_bob_crc not valid " << tmpStr << endl;
               errorCode = "-1";
               unauthorizedStr = "server_auth_bob_crc not valid.";
            }
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "client_source" ) ) 
         {
            // "BBApi" can come here
         } else {
            mc2log << warn << "XMLParserThread::checkAuthorization "
                      "unknown attribute for auth element "
                   << "Name " << attribute->getNodeName()
                   << " Type " << attribute->getNodeType() << endl;
         }
         delete [] tmpStr;
      }

      // Check if the flag is overridden in the prop-file.
      if ( Properties::getUint32Property("XML_PRINT_XML",0) ) {
         development = true;
         indent = true;
      }

      m_authData->clientSetting = m_group->getSetting( 
         m_authData->clientType, "" );

      UserEnums::userRightLevel levelmask = 
         getUrLevel( m_authData->clientSetting  );

      if ( m_authData->clientTypeSet && 
           strcmp( m_authData->clientSetting->getClientType(), "default" )
           == 0 )
      {
         mc2log << info << "XMLParserThread::checkAuthorization: Unknown "
                << "client type \""
                << m_authData->clientType << "\" sending unauthorized." 
                << endl;
         unauthorizedStr = "Unknown client type.";
         errorCode = "-202";
         justPrintError = true;
      }
      
      // Get the auth_user and auth_passwd children from node
      DOMNode* auth_user = NULL;
      DOMNode* auth_passwd = NULL;
      // Optionally use session
      DOMNode* user_session_id = NULL;
      DOMNode* user_session_key = NULL;
      const char* serviceType = "";
      char* deleteServiceType = NULL;
      // Optionally use auth_activate_request
      DOMNode* auth_activate_request = NULL;
      // Token
      DOMNode* auth_token = NULL;
      DOMNode* auth_uin = NULL;
      uint32 authUIN = MAX_UINT32;

      DOMNode* child = node->getFirstChild();
      MC2String wantUIN;
      while ( child != NULL && !justPrintError ) {
         switch ( child->getNodeType() ) {
            case DOMNode::ELEMENT_NODE :
               if ( XMLString::equals( child->getNodeName(), 
                                       "auth_user" ) ) 
               {
                  auth_user = child;
               } else if ( XMLString::equals( child->getNodeName(),
                                              "auth_passwd" ) ) 
               {
                  auth_passwd = child;
               } else if ( XMLString::equals( child->getNodeName(),
                                              "user_session_id" ) )
               {
                  user_session_id = child;
               } else if ( XMLString::equals( child->getNodeName(),
                                              "user_session_key" ) )
               {
                  user_session_key = child;
               } else if ( XMLString::equals( child->getNodeName(),
                                              "user_service" ) ) 
               {
                  deleteServiceType = XMLUtility::getChildTextValue( 
                     child );
                  serviceType = deleteServiceType;
               } else if ( XMLString::equals( child->getNodeName(),
                                              "auth_activate_request" ) ) 
               {
                  auth_activate_request = child;
               } else if ( XMLString::equals( child->getNodeName(),
                                              "auth_token" ) ) 
               {
                  auth_token = child;
               } else if ( XMLString::equals( child->getNodeName(),
                                              "hardware_key" ) ) 
               {
                  m_authData->hwKeys.push_back( UserLicenceKey( MAX_UINT32 ) );
                  m_authData->hwKeys.back().setProduct( 
                     m_authData->clientSetting->getProduct() );
                  MC2String type;
                  XMLTool::getAttribValue( type, "type", child );
                  m_authData->hwKeys.back().setKeyType( type );
                  m_authData->hwKeys.back().setLicence( 
                     XMLUtility::getChildTextStr( *child ) );
               } else if ( XMLString::equals( child->getNodeName(), "uin"))
               {
                  auth_uin = child;
                  MC2String uinStr = XMLUtility::getChildTextStr( *child );
                  char* tmpPtr = NULL;
                  authUIN = strtoul( uinStr.c_str(), &tmpPtr, 10 );
                  if ( tmpPtr == uinStr.c_str() || *tmpPtr != '\0' ) {
                     // must be digits only
                     mc2log << info << "XMLParserThread::checkAuthorization: "
                            << "auth_uin is not a number. " << uinStr << endl;
                     unauthorizedStr = "auth_uin is not a number.";
                     errorCode = "-2";
                     justPrintError = true;
                  }
               } else if ( XMLString::equals( child->getNodeName(),
                                              "want_uin" ) ) {
                  wantUIN = XMLUtility::getChildTextStr( *child );
               } else if ( XMLString::equals( child->getNodeName(),
                                              "pp" ) ) {
                  // Store the phone property in m_authData
               } else {
                  mc2log << warn << "XMLParserThread::checkAuthorization "
                         << "odd Element in auth element: " 
                         << child->getNodeName() << endl;
               }
               break;
            case DOMNode::COMMENT_NODE :
               // Ignore comments
               break;
            default:
               mc2log << warn << "XMLParserThread::checkAuthorization "
                            "odd node type in auth element: "
                      << child->getNodeName() 
                      << " type " << child->getNodeType()<< endl;
               break;
         }
         child = child->getNextSibling();
      }

      // Set here where serviceType is set
      m_authData->urmask = UserEnums::URType( 
         levelmask, getRightService( serviceType ) );

      
      if ( justPrintError ) {
         // Don't try to authenticate
      } else if ( auth_user != NULL && auth_passwd != NULL ) {
         // Check user
         ScopedArray<char> userName( XMLUtility::getChildTextValue( auth_user ) );
         ScopedArray<char> userPasswd( XMLUtility::getChildTextValue( auth_passwd ) );

         uint32 UIN = authenticateUser( userName.get(), userPasswd.get(), true );

         if ( UIN != 0 && UIN != MAX_UINT32 && UIN != (MAX_UINT32 -1) ) {
            if ( getUser( UIN, m_user, true ) ) 
            {
               if ( m_user != NULL ) {
                  authorized = checkUserAccess( m_user->getUser(),
                                                serviceType, true,
                                                levelmask );
            
                  if ( authorized ) {
                     logUserAccepted( "XMLParserThread::checkAuthorization" );
                     if ( strcmp( "netsaint", 
                                  m_user->getUser()->getLogonID() ) 
                          == 0 )
                     {
                        // netsaint user shouldn't be debited nor store 
                        // routes.
                        m_isCheckUser = true;
                     }

                     // if auth contained a wanted user id and
                     // the current user is a "special" user that
                     // is allowed to switch to another user then
                     // we should switch user.
                     if ( ! wantUIN.empty() &&
                          m_user->getUser()->getExternalXmlService() ) {
                        // lets switch user to the wanted user                        
                        UserItem* newUser = NULL;
                        uint32 parsedWantUIN = 0;
                        bool parseOk = STLStringUtility::
                                       strtoul( wantUIN, parsedWantUIN ); 
                        if ( !parseOk  || 
                             !getUser( parsedWantUIN, newUser, true ) || 
                             newUser == NULL ) {
                           unauthorizedStr = "Unknown user.";
                           errorCode = "-202";
                           mc2log << info 
                                  << "XMLParserThread::checkAuthorization "
                                  << "failed to get user, UIN "
                                  << wantUIN << endl;
                        } else {
                           releaseUserItem( m_user );
                           m_user = newUser;
                           logUserAccepted( 
                              "XMLParserThread::checkAuthorization Switch" );
                        }
                     }
                  } else {
                     if ( checkUserAccess( m_user->getUser(),
                                           serviceType, false, 
                                           levelmask ) ) 
                     {
                        mc2log << info 
                               << "XMLParserThread::checkAuthorization "
                               << "User expired, userName " 
                               << m_user->getUser()->getLogonID()
                               << "(" << m_user->getUIN() << ")"  << endl;
                        unauthorizedStr = "Expired user.";
                        errorCode = "-206";
                     } else {
                        unauthorizedStr = "Access denied.";
                        errorCode = "-201";
                        mc2log << warn 
                               << "XMLParserThread::checkAuthorization " 
                               << "User has no access to service! "
                               << serviceType << " level "
                               << MC2HEX( levelmask )
                               << " UserName " 
                               << m_user->getUser()->getLogonID() 
                               << "(" << m_user->getUIN() << ")" << endl;
                     }
                  } // End else for if authorized
               } else {
                  unauthorizedStr = "Unknown user.";
                  errorCode = "-202";
                  mc2log << info << "XMLParserThread::checkAuthorization "
                     "failed to get user, UIN " << UIN << endl;
               }
            } else {
               unauthorizedStr = "Database connection error, failed to "
                  "get user data.";
               errorCode = "-3";
               mc2log << info << "XMLParserThread::checkAuthorization "
                         "failed to get user"
                      << "UserName " << userName.get() << "(" << UIN << ")"
                      << endl;
            }
         } else {
            if ( UIN == MAX_UINT32 ) {
               unauthorizedStr = 
                  "Database connection error, no connection.";
               errorCode = "-3";
            } else if ( UIN == (MAX_UINT32 -1) ) {
               unauthorizedStr = "Expired user.";
               errorCode = "-206";
            } else {
               unauthorizedStr = "Invalid login.";
               errorCode = "-203";
            }
            mc2log << info << "XMLParserThread::checkAuthorization "
                   << "authorization failed: " << unauthorizedStr << endl;
         }

      } else if ( user_session_id != NULL && user_session_key != NULL )
      {
         // Verify session
         char* userSessionID = XMLUtility::getChildTextValue( 
            user_session_id );
         char* userSessionKey = XMLUtility::getChildTextValue( 
            user_session_key );

         uint32 UIN = authenticateUserSession( userSessionID,
                                               userSessionKey, true );
         if ( UIN != 0 && UIN != MAX_UINT32 && UIN != (MAX_UINT32 -1) &&
              UIN != (MAX_UINT32 -2) ) 
         {
            if ( getUser( UIN, m_user, true ) ) {
               if ( m_user != NULL ) {
                  authorized = checkUserAccess( m_user->getUser(),
                                                serviceType, true, 
                                                levelmask );
                  if ( authorized ) {
                     logUserAccepted( "XMLParserThread::checkAuthorization" );
                  } else {
                     unauthorizedStr = "Access denied.";
                     errorCode = "-201";
                     mc2log << warn 
                            << "XMLParserThread::checkAuthorization " 
                               "User has no access to service "
                            << serviceType << " UserName " 
                            << m_user->getUser()->getLogonID() 
                            << "(" << m_user->getUIN() << ")" << endl;
                  }
               } else {
                  unauthorizedStr = "Unknown user.";
                  errorCode = "-202";
                  mc2log << info <<  "XMLParserThread::checkAuthorization "
                            "failed to get user" << endl;
               }
            } else {
               unauthorizedStr = "Database connection error, failed to "
                  "get user data.";
               errorCode = "-3";
               mc2log << info << "XMLParserThread::checkAuthorization "
                         "failed to get user"
                      << "UserName " 
                      << (m_user? 
                          m_user->getUser()->getLogonID() : 
                          "UNKNOWN")
                      << "(" << UIN << ")" << endl;
            }
  

         } else {
            if ( UIN == (MAX_UINT32 -2) ) {
               unauthorizedStr = 
                  "Database connection error, no connection.";
               errorCode = "-3";
            } else if ( UIN == 0 ) {
               unauthorizedStr = "Invalid session.";
               errorCode = "-204";
            } else if ( UIN == MAX_UINT32 ) {
               unauthorizedStr = "Session has expired, login again.";
               errorCode = "-205";
            } else if ( UIN == (MAX_UINT32 -1) ) {
               unauthorizedStr = "Expired user.";
               errorCode = "-206";
            }
            mc2log << info << "XMLParserThread::checkAuthorization "
               "verify user request failed: " << unauthorizedStr 
                   << endl;
         }
         
         delete [] userSessionID;
         delete [] userSessionKey;
      } else if ( auth_activate_request != NULL ) {
         mc2log << info 
                << "XMLParserThread::checkAuthorization, only "
                << "activate_request and tunnel_request allowed" << endl;
         m_allowedRequests.insert( "activate_request" );
         m_allowedRequests.insert( "auth" ); // This element
         // Allow to activate in Content Window
         m_allowedRequests.insert( "tunnel_request" );
         authorized = true;
      } else if ( auth_token != NULL && auth_uin != NULL ) {
         // Get user and check token
         char* uinStr = XMLUtility::getChildTextValue( auth_uin );
         char* tokenStr = XMLUtility::getChildTextValue( auth_token );
         uint32 startTime = TimeUtility::getCurrentTime();
         char* tmpPtr = NULL;

         uint32 uin = strtoul( uinStr, &tmpPtr, 10 );
         serviceType = "JAVA";
         m_authData->urmask = UserEnums::URType( 
            levelmask, getRightService( serviceType ) );

         if ( tmpPtr != uinStr && *tmpPtr == '\0' ) {//uinStr must be digits only
            if ( getUser( uin, m_user, true ) ) {
               if ( m_user != NULL ) {
                  MC2String nTokenStr = 
                     getTokenHandler()->makeTokenStr();
                  
                  UserItem* setUserItem = NULL;
                  if ( getUserHandler()->checkIronTrialTime( 
                          m_user->getUser(), m_authData->clientSetting,
                          setUserItem ) && setUserItem != NULL ) {
                     // Update m_user
                     releaseUserItem( m_user );
                     m_user = setUserItem;
                  }
                  authorized = checkUserAccess( m_user->getUser(),
                                                serviceType, true, 
                                                levelmask );
                  bool fakeAuthorization = false;
                  if ( !authorized && checkUserAccess( 
                          m_user->getUser(), serviceType, false, 
                          levelmask ) )
                  {
                     // Check if only user_cap then allow it
                     DOMNodeList* auths = doc->getElementsByTagName( 
                        X( "user_cap_request" ) );
                     DOMNodeList* tunnels = doc->getElementsByTagName( 
                        X( "tunnel_request" ) );
                     if ( auths->getLength() >= 1 || tunnels->getLength() >= 1)
                     {
                        m_allowedRequests.insert( "auth" ); // This element
                        m_allowedRequests.insert( "user_cap_request" );
                        m_allowedRequests.insert( "tunnel_request" );
                        authorized = true;
                        fakeAuthorization = true;
                     }
                  }
                  bool errorSet = false;
                  bool extAuthDone = false;
                  const ParserExternalAuthHttpSettings* peh = NULL;
                  // External check here

                  if ( !errorSet && !extAuthDone && getExternalAuth()->
                       isHttpHeaderRequestUser( m_user, &peh ) )
                  {
                     extAuthDone = true;
                     authorized = doHttpAuth( m_user, unauthorizedStr, 
                                              errorCode, peh->reqName,
                                              m_authData->clientLang,
                                              tokenStr, nTokenStr );
                     if ( !authorized ) {
                        errorSet = true;
                     }
                  } // End Http external user

                  // Add Other type of external user here


                  if ( errorSet ) {
                     // Done no more checks
                  } else if ( authorized ) {
                     // Check token
                     if ( getTokenHandler()->getToken( 
                             m_user->getUser(), tokenStr, 
                             m_authData->clientSetting ) != NULL )
                     {
                        // Check for expired token (too old)
                        if ( !getTokenHandler()->expiredToken( 
                                m_user->getUser(), tokenStr, 
                                m_authData->clientSetting ) )
                        {
                           // If new token used(is tokenStr) remove old
                           auto_ptr<UserUser> user( 
                              new UserUser( *m_user->getUser() ) );
                           int res = getTokenHandler()->removeAllButToken( 
                              user.get(), tokenStr,
                              m_authData->clientSetting );
                           if ( res == 0 ) {
                              res = getUserHandler()->updateUser( m_user );
                           }
                           
                           if ( res == 0 ) {
                              logUserAccepted(
                                 "XMLParserThread::checkAuthorization Token \""
                                 + MC2String( tokenStr )
                                 + "\"" );
                           } else {
                              // Error
                              unauthorizedStr = "Database connection "
                                 "error, failed to update user data.";
                              errorCode = "-1";
                              if ( res == -2 ) {
                                 errorCode = "-3";
                              }
                              mc2log << info << "XMLParserThread::"
                                     << "checkAuthorization "
                                     << "failed to add new token to "
                                     << "User " 
                                  << (m_user? 
                                      m_user->getUser()->getLogonID()
                                      : "UNKNOWN")
                                     << "(" << uin << ")" << endl;
                           } 
                        } else {
                           // Token too old, add new and send new
                           // Remove old new (client got new before).If any
                           auto_ptr<UserUser> user( 
                              new UserUser( *m_user->getUser() ) );
                           int res = getTokenHandler()->removeAllButToken( 
                              user.get(), tokenStr, 
                              m_authData->clientSetting );
                           if ( res == 0 ) {
                              res = getTokenHandler()->addNewToken(
                                 user.get(), nTokenStr,
                                 m_authData->clientSetting );
                           }
                           if ( res == 0 ) {
                              res = getUserHandler()->updateUser( m_user );
                           }
                           if ( res == 0 ) {
                              newTokenStr = nTokenStr;
                              authorized = false;
                              unauthorizedStr = "Expired token.";
                              errorCode = "-208";
                              mc2log << warn 
                                     << "XMLParserThread::"
                                     << "checkAuthorization "
                                     << "Expired Token, Token \""
                                     << tokenStr << "\" Sending new \""
                                     << newTokenStr << "\" User " 
                                     << m_user->getUser()->getLogonID()
                                     << "(" << m_user->getUIN() << ")"
                                     << endl;
                           } else {
                              // Error
                              unauthorizedStr = "Database connection "
                                 "error, failed to update user data.";
                              errorCode = "-1";
                              if ( res == -2 ) {
                                 errorCode = "-3";
                              }
                              mc2log << info << "XMLParserThread::"
                                     << "checkAuthorization "
                                     << "failed to add new token to "
                                     << "User " 
                                  << (m_user? 
                                      m_user->getUser()->getLogonID()
                                      : "UNKNOWN")
                                     << "(" << uin << ")" << endl;
                           }
                        }
                     } else {
                        unauthorizedStr = "Unknown token.";
                        errorCode = "-207";
                        mc2log << warn 
                               << "XMLParserThread::checkAuthorization "
                               << "Token not found in User, Token \""
                               << tokenStr << "\" TokenGroup \"" 
                               << getTokenHandler()->tokenGroup( 
                                  m_user->getUser(), 
                                  m_authData->clientSetting )
                               << "\"(" << m_authData->clientSetting->
                                  getClientType() << ") User " 
                               << m_user->getUser()->getLogonID() 
                               << "(" << m_user->getUIN() << ")"
                               << endl;
                        authorized = false;
                     }
                  } else {
                     if ( checkUserAccess( m_user->getUser(),
                                           serviceType, false, levelmask ))
                     {
                        mc2log << info 
                               << "XMLParserThread::checkAuthorization "
                               << "Token User expired, userName " 
                               << m_user->getUser()->getLogonID()
                               << "(" << m_user->getUIN() << ")"  << endl;
                        unauthorizedStr = "Expired user.";
                        errorCode = "-206";
                     } else {
                        unauthorizedStr = "Access denied.";
                        errorCode = "-201";
                        mc2log << warn 
                               << "XMLParserThread::checkAuthorization "
                               << "Token User has no access to service "
                               << serviceType << " UserName " 
                               << m_user->getUser()->getLogonID() 
                               << "(" << m_user->getUIN() << ")" << endl;
                     }
                  }

                  if ( authorized && !extAuthDone && !fakeAuthorization ) {
                     // Check client_type level matches user level
                     if ( getClientTypeLevel() == UserEnums::UR_GOLD ) {
                        if ( !checkUserAccess( m_user->getUser(),
                                               serviceType, 
                                               true/*checkTime*/,
                                               UserEnums::TG_MASK ) )
                        {
                           authorized = false;
                           unauthorizedStr = "Insufficient credit.";
                           errorCode = "-209";
                           mc2log << warn 
                                  << "XMLParserThread::checkAuthorization "
                                  << "Gold client but not gold user, "
                                  << "sending Insufficient credit."
                                  << endl;
                        }
                     }
                  }

               } else {
                  unauthorizedStr = "Unknown user.";
                  errorCode = "-202";
                  mc2log << info <<  "XMLParserThread::checkAuthorization "
                         << "Token user unknown \"" << uin << endl;
               }
            } else {
               uint32 endTime = TimeUtility::getCurrentTime();
               unauthorizedStr = "Database connection error, failed to "
                  "get user data.";
               errorCode = "-1";
               if ( endTime - startTime > 2000 ) {
                  errorCode = "-3";
               }
               mc2log << info << "XMLParserThread::checkAuthorization "
                         "Token failed to get user"
                      << "UserName " 
                      << (m_user? 
                          m_user->getUser()->getLogonID() : 
                          "UNKNOWN")
                      << "(" << uin << ")" << endl;
            }
               
         } else {
            mc2log << "XMLParserThread::checkAuthorization Token "
               "Bad uin \"" << uinStr << "\" token " << tokenStr << endl;
            unauthorizedStr = "Bad uin.";
            errorCode = "-1";
         }

         delete [] uinStr;
         delete [] tokenStr;
      } else if ( m_authData->hwKeys.size() > 0 ) {
         // Hardware identification

         // Find and set best
         getUserHandler()->setBestLicenceKey( m_authData->hwKeys, 
                                              m_authData->hwKey );

         // A "token" string. Used for timeout for checking with externalauth
         MC2String licenceKey; 
         getUserHandler()->makeLicenceKeyStr( licenceKey, m_authData->hwKey );
         bool ok = true;

         const ParserExternalAuthHttpSettings* peh = 
            getExternalAuth()->getNavHttpSetting( 
               m_authData->clientSetting->getClientType() );
         serviceType = "JAVA";
         m_authData->urmask = UserEnums::URType( 
            levelmask, getRightService( serviceType ) );

         if ( !ok ) {
            // Error set do nothing
         } else if ( peh != NULL ) {
            // External auth
            m_authData->externalAuth = peh->settingId;
            // Do Http auth
            authorized = doHttpAuth( m_user, unauthorizedStr, 
                                     errorCode, peh->settingId, 
                                     m_authData->clientLang,
                                     licenceKey, licenceKey,
                                     true/*checkTime*/ );
            if ( authorized ) {
               logUserAccepted( "XMLParserThread::checkAuthorization "
                                "hardware authentication ExternalAuth "
                                + peh->settingId
                                + " via client_type "
                                + m_authData->clientSetting->getClientType()
                                );
            } else {
               // Else error in errorMessage, errorCode
               ok = false;
            }

            // Check m_user uin with clients auth_uin, if present
            if ( ok && m_user != NULL && auth_uin != NULL ) {
               if ( m_user->getUIN() != authUIN ) {
                  ok = false;
                  authorized = false;
                  unauthorizedStr = "Access denied. "
                     "Needs to update uin in client.";
                  errorCode = "-201";
                  clientUserIDDoesNotMatch = true;
                  mc2log << warn 
                         << "XMLParserThread::checkAuthorization "
                         << "client uin (" << authUIN << ") is not external auth "
                         << "uin (" << m_user->getUIN() << ") sending error." 
                         << endl;
               }
            }

            // The error reason is present
            m_authData->externalAuthErrorCode = errorCode;

            // Check if only user_cap or tunnel then allow it
            if ( !ok ) {
               DOMNodeList* auths = doc->getElementsByTagName( 
                  X( "user_cap_request" ) );
               DOMNodeList* tunnels = doc->getElementsByTagName( 
                  X( "tunnel_request" ) );
               if ( (m_user != NULL && auths->getLength() >= 1) || 
                    tunnels->getLength() >= 1 ) {
                  m_allowedRequests.insert( "auth" ); // This element
                  if ( m_user != NULL ) {
                     m_allowedRequests.insert( "user_cap_request" );
                  }
                  m_allowedRequests.insert( "tunnel_request" );
                  authorized = true;
                  ok = true;
                  unauthorizedStr = "";
                  errorCode = "0";
               }
            }

         } else  {
            // Normal auth using hardware key
            ok = getUserFromLicenceKeys( m_authData->hwKeys, m_user, 
                                         "-215", errorCode, unauthorizedStr );

            // Check m_user uin with clients auth_uin, if present, but not for
            // tunnel requests
            DOMNodeList* tunnels = doc->getElementsByTagName( 
               X( "tunnel_request" ) );
            if ( ok && m_user != NULL && auth_uin != NULL && 
                 tunnels->getLength() == 0 ) {
               if ( m_user->getUIN() != authUIN ) {
                  ok = false;
                  authorized = false;
                  unauthorizedStr = "Access denied. "
                     "Needs to update uin in client.";
                  errorCode = "-201";
                  clientUserIDDoesNotMatch = true;
                  mc2log << warn 
                         << "XMLParserThread::checkAuthorization "
                         << "client uin (" << authUIN << ") is not owner of "
                         << "hardware key owner uin (" << m_user->getUIN() 
                         << ") sending error." << endl;
               }
            }

            if ( ok && m_user != NULL ) {
               // Have a user for the key
               // Checkup earth AUTO rights
               UserItem* setUserItem = NULL;
               if ( getUserHandler()->checkIronTrialTime( 
                       m_user->getUser(), m_authData->clientSetting,
                       setUserItem ) && setUserItem != NULL ) {
                  // Update m_user
                  releaseUserItem( m_user );
                  m_user = setUserItem;
               }

               authorized = checkUserAccess( m_user->getUser(),
                                             serviceType, true, 
                                             levelmask );
               // Check expired (but allow tunnel etc.)
               if ( !authorized && checkUserAccess( 
                       m_user->getUser(), serviceType, false, levelmask ) )
               {
                  // Check if only user_cap or tunnel then allow it
                  DOMNodeList* auths = doc->getElementsByTagName( 
                     X( "user_cap_request" ) );
                  if ( auths->getLength() >= 1 || tunnels->getLength() >= 1 ) {
                     m_allowedRequests.insert( "auth" ); // This element
                     m_allowedRequests.insert( "user_cap_request" );
                     m_allowedRequests.insert( "tunnel_request" );
                     authorized = true;
                  } else {
                     mc2log << info 
                            << "XMLParserThread::checkAuthorization "
                            << "hardware authentication  User expired, "
                            << "userName " << m_user->getUser()->getLogonID()
                            << "(" << m_user->getUIN() << ")"  << endl;
                     unauthorizedStr = "Expired user.";
                     errorCode = "-206";
                  }
               } else if ( !authorized ) {
                  unauthorizedStr = "Access denied.";
                  errorCode = "-201";
                  mc2log << warn 
                         << "XMLParserThread::checkAuthorization "
                         << "hardware authentication User has no access to "
                         << "service " << serviceType << " UserName " 
                         << m_user->getUser()->getLogonID() 
                         << "(" << m_user->getUIN() << ")" << endl;
               } // Else authorized
            } else if ( ok && m_user == NULL ) {
               // handle no user 
               // check for tunnel and wfid client type...
               // or server info
               DOMNodeList* tunnels = doc->getElementsByTagName( 
                  X( "tunnel_request" ) );
               DOMNodeList* serverInfo = doc->getElementsByTagName( 
                  X( "server_info_request" ) );
               DOMNodeList* activateRequest = doc->getElementsByTagName( 
                  X( "activate_request" ) );

               if ( ( m_authData->clientSetting->getWFID() &&
                      tunnels->getLength() >= 1 ) 
                    || serverInfo->getLength() >= 1 ||
                    activateRequest->getLength() >= 1 ) {
                  authorized = true;
                  m_allowedRequests.insert( "auth" ); // This element
                  m_allowedRequests.insert( "tunnel_request" );
                  m_allowedRequests.insert( "server_info_request" );
                  m_allowedRequests.insert( "activate_request" );
               } else {
                  if ( m_authData->clientSetting->getWFID() ) {
                     // WFID
                     unauthorizedStr = "Access denied. Non tunnel request.";
                     errorCode = "-201";
                     mc2log << warn 
                            << "XMLParserThread::checkAuthorization "
                            << "hardware authentication no User for licence "
                            << licenceKey << " and WFID client type for "
                            << "non tunnel request." << endl;
                  } else {
                     if ( tunnels->getLength() >= 1 || serverInfo->getLength() >= 1 ) {
                        authorized = true;
                        m_allowedRequests.insert( "auth" ); // This element
                        m_allowedRequests.insert( "tunnel_request" );
                        m_allowedRequests.insert( "server_info_request" );
                     } else {
                        // What here?
                        unauthorizedStr = "Access denied. Non tunnel request.";
                        errorCode = "-201";
                        mc2log << warn 
                               << "XMLParserThread::checkAuthorization "
                               << "hardware authentication no User for licence "
                               << licenceKey << " and non WFID client type for "
                               << "non tunnel request." << endl;
                     }
                  }
               }
            } // Else !ok and error is set

         } // End if ok
         
         if ( m_authData->clientSetting->getWFID() ) {
            // Send a suitable web page to goto
            // Tunnel adds su and hw-keys when client requests page.
            if ( errorCode == "-201" || errorCode == "-401" ) {
               errorURL = "http://firstpage/?srverr=clientunauthorized";
            } else if ( errorCode == "-206" || errorCode == "-402" ) {
               errorURL = "http://firstpage/?srverr=clientexpired";
            }
         } // End if wfid client type
      } else {
         mc2log << warn 
                << "XMLParserThread::checkAuthorization auth element"
                   " bad content!" << endl;
         mc2dbg4 << "auth_user " 
                 << (auth_user != NULL ? auth_user->getNodeName() : 
                     X("NULL")) << endl
                 << "auth_passwd " 
                 << (auth_passwd != NULL ? auth_passwd->getNodeName() : 
                     X("NULL")) << endl
                 << "user_session_id " 
                 << ( user_session_id != NULL ? 
                      user_session_id->getNodeName() : X("NULL") ) << endl
                 << "user_session_key " 
                 << ( user_session_key != NULL ? 
                      user_session_key->getNodeName() : X("NULL") )
                 << "hwKeys " 
                 << ( m_authData->hwKeys.size() > 0 ? 
                      m_authData->hwKeys[ 0 ].getLicenceKeyStr() : "NULL" ) 
                 << endl;
         
         unauthorizedStr = "No data to authenticate user with.";
         errorCode = "-1";
      }

      if ( authorized && !auth_activate_request && m_user != NULL ) {
         if ( !m_authData->backupUser && needsNewServerList() ) {
            authorized = false; // Send data
            sendUpdate = true;
            errorCode = "-210";
            unauthorizedStr = "New data.";
            errorURL = ""; // JIC
         }
         
         //check lock-in
         mc2dbg2 << "Checking Version Lock" << endl;
         if ( authorized && m_authData->clientTypeSet ) {
            const uint32 highest = 
               m_user->getUser()->getHighestVersionLock();
            const uint32 required = 
               m_authData->clientSetting->getVersionLock();
            mc2dbg2 << "VL: highest: " << highest
                    << " required: " << required << endl;
            if ( (required > highest) && 
                 ! m_user->getUser()->hasActive( UserEnums::URType(UserEnums::UR_TRIAL, UserEnums::UR_WF) ) ){  
               //User does not have high enough version lock and no
               //valid trial right
               mc2log << info << "VersionLock! User has " << highest 
                      << ", needs" << required << endl;

               //allow some requests even if version lock
               m_allowedRequests.insert( "activate_request" );
               m_allowedRequests.insert( "auth" ); // This element
               // Allow to activate in Content Window
               m_allowedRequests.insert( "tunnel_request" );
               m_allowedRequests.insert( "user_cap_request" );

               //Check if all requests in this document are allowed. 
               bool ok = true;
               for( DOMNode* auth_sib = node->getParentNode()->getFirstChild();
                    auth_sib != NULL && ok; 
                    auth_sib = auth_sib->getNextSibling() ) {
                  
                  ScopedArray<char> 
                     tmpStr( XMLUtility::transcodefromucs( auth_sib->getNodeName() ) );
                  ok = ( m_allowedRequests.find( tmpStr.get() ) != 
                         m_allowedRequests.end() ) ;
               }
               if(!ok){ //at least one forbidden request, signal -214
                  authorized = false;
                  errorCode = "-214";
                  unauthorizedStr = 
                     "You do not have access to this version. "
                     "Please use the Mobile shop to ugrade.";
                  errorURL = "http://versionlock/";
                  mc2dbg << "Version Lock block, errorCode " << errorCode
                         << ", errorMsg '" << unauthorizedStr << "', errorURL '" 
                         << errorURL << "'" << endl;
               }
            }
         }
              
         // Check last client.
         if ( authorized && m_authData->clientTypeSet ) {
            UserItem* setUserItem = NULL;
            MC2String version;
            MC2String extra;
            if ( m_irequest->getRequestHeader()->getHeaderValue( 
                    userAgent ) != NULL )
            {
               extra += userAgent; extra += ": ";
               extra += *m_irequest->getRequestHeader()
                  ->getHeaderValue( userAgent );
               // Get version from userAgent (jn-1-rel/1.35.2)
               const char* start = strstr( 
                  extra.c_str(), m_authData->clientType.c_str() );
               if ( start != NULL && 
                    *(start + m_authData->clientType.size()) == '/' )
               {
                  // After client_type and slash
                  const char* pos = start + 
                     m_authData->clientType.size() + 1;
                  // Until end or not (number or dot)
                  while ( *pos && (isdigit(*pos) || *pos == '.') ) {
                     version += *pos;
                     pos++;
                  }
               }
            }

            if ( getUserHandler()->checkAndUpdateLastClientAndUsage(
                       m_user->getUser(), setUserItem,
                       m_authData->clientType, "", version, 
                       extra, m_authData->ipnport ) )
            {
               releaseUserItem( m_user );
               m_user = setUserItem;
            }
         }
      }

      delete [] deleteServiceType;
   } else {
      mc2log << warn << "XMLParserThread::checkAuthorization "
                "not one auth element!" << endl;
      mc2dbg8 << "auths.getLength() " << auths->getLength() 
              << endl;
      unauthorizedStr = "Not one auth element in request.";
      errorCode = "-1";
   }

   if ( !authorized ) {
      // Send status
      DOMElement* isabmc2 = reply->getDocumentElement();
         
      appendStatusNodes( reply->getLastChild(), reply, indentLevel, indent,
                         errorCode.c_str(), unauthorizedStr.c_str(), 
                         NULL, //status_code_extended
                         errorURL.c_str() );
      if ( !newTokenStr.empty() ) {
         XMLServerUtility::
            appendElementWithText( isabmc2, reply, 
                                   "auth_token", 
                                   newTokenStr.c_str(),
                                   indentLevel, indent  );
      }
      if ( sendUpdate ) {
         XMLServerUtility::
            appendServerList( isabmc2, reply, indentLevel, indent,
                              getServerList("") );
         mc2log << info << "Sending server_list update to client." << endl;
      }

      // Indent and linebreak before ending isab-mc2 tag
      if ( indent ) {
         isabmc2->appendChild( reply->createTextNode( X( "\n" ) ) );
      }
   }

   uint32 stopProcessTime = TimeUtility::getCurrentMicroTime();
   mc2dbg2 << "XMLParserThread::checkAuthorization process time " 
           << (stopProcessTime - startProcessTime)/1000 << " ms" << endl;

   return authorized;
}


bool 
XMLParserThread::xmlParseRequest( DOMDocument* request,
                                  DOMDocument* reply,
                                  bool indent,
                                  const HttpHeader& inHeader )
{
   mc2dbg4 << "XMLParserThread::xmlParseRequest" << endl;
   bool ok = false;

   // Looking for the isab-mc2/request element
   DOMElement* root = request->getDocumentElement();
   if ( XMLString::equals( root->getNodeName(), "isab-mc2" ) ) {
      ok = xmlParseIsabmc2( root, reply->getLastChild(),
                            reply, indent, inHeader );
   } else if ( XMLString::equals( root->getNodeName(), "request" ) ) {
      ok = xmlParsePublicRequest( root, reply->getLastChild(),
                                  reply, indent );
   } else {
      mc2log << warn << "XMLParserThread::xmlParseRequest document "
                "without isab-mc2 tag" << endl;
   }

   return ok;
}

#define REQNAME( a ) if ( !m_requestName.empty() ) { \
 m_requestName.append( ", " ); } m_requestName.append( a );


bool 
XMLParserThread::xmlParseIsabmc2( DOMNode* cur, 
                                  DOMNode* out,
                                  DOMDocument* reply,
                                  bool indent,
                                  const HttpHeader& inHeaders )
{

   mc2dbg8 << "XMLParserThread::xmlParseIsabmc2" << endl;
   bool ok = true;
   
   // Get isab-mc2
   DOMElement* isabmc2 = reply->getDocumentElement();

   // Go throu the elements and handle them.
   DOMNode* child = cur->getFirstChild();
   
   for (; child != NULL && ok ; child = child->getNextSibling() ) {
      if ( child->getNodeType() == DOMNode::COMMENT_NODE ||
           child->getNodeType() == DOMNode::TEXT_NODE ) {
         // ignore comments and stray texts
         continue;
      } else if ( child->getNodeType() != DOMNode::ELEMENT_NODE ) {
         ok = false;
         mc2log << warn << "XMLParserThread::xmlParseIsabmc2 odd "
                << "node type in isab-mc2 element: " 
                << child->getNodeName() 
                << " type " << child->getNodeType()<< endl;
         break;
      }
      const XMLCh* nodeName = child->getNodeName();
      // See if the element is a known type
      ScopedArray<char> tmpStr( XMLUtility::
                                transcodefromucs( nodeName ) );
      
      if ( !m_allowedRequests.empty() && 
           m_allowedRequests.find( tmpStr.get() ) ==
           m_allowedRequests.end() ) {
         // Not in allowed requests
         mc2log << warn << "XMLParserThread::xmlParseIsabmc2 "
                << "unallowed element in isab-mc2 element: " 
                << child->getNodeName() 
                << " type " << child->getNodeType()
                << " skipping it" << endl;
      } else if ( XMLString::equals( nodeName, "auth" ) ) {
         ok = xmlParseAuth( child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName, 
                                     "user_request" ) ) {
         REQNAME( "USER_EDIT" );
         ok = xmlParseUserRequest( child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "route_request" ) ) {
         REQNAME( "ROUTE" );
         ok = xmlParseRouteRequest( child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "search_request" ) ) {
         REQNAME( "SEARCH" );
         ok = xmlParseSearchRequest( child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "search_desc_request" ) ) {
         REQNAME( "SEARCH_DESC" );
         ok = xmlParseSearchDescRequest( child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "search_position_desc_request" ) ) {
         REQNAME( "SEARCH_POSITION_DESC" );
         ok = xmlParseSearchPositionDescRequest( child, isabmc2,
                                                 reply, indent );
      } else if (XMLString::equals( nodeName,
                                    "compact_search_request" ) ) { 
         REQNAME( "COMPACT_SEARCH" );
         ok = xmlParseCompactSearchRequest( child, isabmc2,
                                            reply, indent );
      } else if (XMLString::equals( nodeName,
                                    "one_search_request" ) ) { 
         REQNAME( "ONE_SEARCH" );
         ok = xmlParseOneSearchRequest( child, isabmc2,
                                        reply, indent );
      } else if (XMLString::equals( nodeName,
                                    "category_list_request" ) ) { 
         REQNAME( "CATEGORY_LIST" );
         ok = xmlParseCategoryListRequest( child, isabmc2,
                                           reply, indent );
      } else if (XMLString::equals( nodeName,
                                    "category_tree_request" ) ) { 
         REQNAME( "CATEGORY_TREE" );
         ok = xmlParseCategoryTreeRequest( child, isabmc2,
                                           reply, indent );
      } else if (XMLString::equals( nodeName,
                                    "copyright_strings_request" ) ) {
         REQNAME( "COPYRIGHT_STRINGS" );
         ok = xmlParseCopyrightStringsRequest( child, isabmc2,
                                               reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "expand_request" ) ) {
         REQNAME( "EXPAND" );
         ok = xmlParseExpandRequest( child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "send_sms_request" ) ) {
         REQNAME( "SEND_SMS" );
         ok = xmlParseSendSMSRequest( child, isabmc2, reply, 
                                      indent );
      } else if ( XMLString::equals( nodeName,
                                     "user_login_request" ) ) {
         REQNAME( "USER_LOGIN" );
         ok = xmlParseUserLoginRequest( child, isabmc2, reply, 
                                        indent );
      } else if ( XMLString::equals( nodeName,
                                     "user_verify_request" ) ) {
         REQNAME( "USER_VERIFY" );
         ok = xmlParseUserVerifyRequest( child, isabmc2, reply, 
                                         indent );   
      } else if ( XMLString::equals( nodeName,
                                     "user_logout_request" ) ) {
         REQNAME( "USER_LOGOUT" );
         ok = xmlParseUserLogoutRequest( child, isabmc2, reply, 
                                         indent );   
      } else if ( XMLString::equals( nodeName,
                                     "map_request" ) ) {
         REQNAME( "MAP_URL" );
         ok = xmlParseMapRequest( child, isabmc2, reply, 
                                  indent );
      } else if ( XMLString::equals( nodeName,
                                     "poi_info_request" ) ) {
         REQNAME( "POI_INFO" );
         ok = xmlParsePOIInfoRequest( child, isabmc2, reply, 
                                      indent );
       } else if ( XMLString::equals( nodeName,
                                     "poi_detail_request" ) ) {
         REQNAME( "POI_DETAIL" );
         ok = xmlParsePOIDetailRequest( child, isabmc2, reply, 
                                        indent );
      } else if ( XMLString::equals( nodeName,
                                     "simple_poi_desc_request" ) ) {
         REQNAME( "SIMPLE_POI_DESC_REQUEST" );
         ok = xmlParseSimplePOIDescRequest( child, isabmc2, reply,
                                            indent );
      } else if ( XMLString::equals( nodeName,
                                     "email_request" ) ) {
         REQNAME( "EMAIL" );
         ok = xmlParseEmailRequest( child, isabmc2, 
                                    reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "sms_format_request" ) ) {
         REQNAME( "SMS_FORMAT" );
         ok = xmlParseSMSFormatRequest( child, isabmc2, reply, 
                                        indent );
      } else if ( XMLString::equals( nodeName,
                                     "user_show_request" ) ) {
         REQNAME( "USER_SHOW" );
         ok = xmlParseUserShowRequest( child, isabmc2, reply, 
                                       indent );
      } else if ( XMLString::equals( nodeName,
                                     "user_cap_request" ) ) {
         REQNAME( "USER_CAP" );
         ok = xmlParseUserCapRequest( child, isabmc2, reply,
                                      indent );
      } else if ( XMLString::equals( nodeName,
                                     "user_favorites_request" ) ) {
         REQNAME( "USER_FAV" );
         ok = xmlParseUserFavoritesRequest( child, isabmc2, reply, 
                                            indent );
      } else if ( XMLString::equals( nodeName,
                                     "user_favorites_crc_request") ) {
         REQNAME( "USER_FAV_CRC" );
         ok = xmlParseUserFavoritesCRCRequest( child, isabmc2, reply,
                                               indent );
      } else if ( XMLString::equals( nodeName,
                                     "zoom_settings_request" ) ) {
         REQNAME( "ZOOM_SETTINGS" );
         ok = xmlParseZoomSettingsRequest( child, isabmc2, reply,
                                           indent );
      } else if ( XMLString::equals( nodeName,
                                     "sort_dist_request" ) ) {
         REQNAME( "SORT_DIST" );
         ok = xmlParseSortDistRequest( child, isabmc2, reply, 
                                       indent );
      } else if ( XMLString::equals( nodeName,
                                     "top_region_request" ) ) {
         REQNAME( "TOP_REGION" );
         ok = xmlParseTopRegionRequest( child, isabmc2, reply, 
                                        indent );
      } else if ( XMLString::equals( nodeName,
                                     "phone_manufacturer_request" ) ) {
         REQNAME( "PHONE_MANU" );
         ok = xmlParsePhoneManufacturerRequest( child, isabmc2, 
                                                reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "phone_model_request" ) ) {
         REQNAME( "PHONE_MODEL" );
         ok = xmlParsePhoneModelRequest( child, isabmc2, 
                                         reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "user_track_request" ) ) {
         REQNAME( "USER_TRACK" );
         ok = xmlParseUserTrackRequest( child, isabmc2, 
                                        reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "user_track_add_request" ) ) {
         REQNAME( "USER_TRACK_ADD" );
         ok = xmlParseUserTrackAddRequest( child, isabmc2, 
                                           reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "user_debit_log_request" ) ) {
         REQNAME( "USER_DEBIT_LOG" );
         ok = xmlParseUserDebitLogRequest( child, isabmc2, 
                                           reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "user_find_request" ) ) {
         REQNAME( "USER_FIND" );
         ok = xmlParseUserFindRequest( child, isabmc2, 
                                       reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "transactions_request" ) ) {
         REQNAME( "TRANSACTIONS" );
         ok = xmlParseTransactionsRequest( child, isabmc2, 
                                           reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "transaction_days_request" ) ) {
         REQNAME( "TRANSACTION_DAYS" );
         ok = xmlParseTransactionDaysRequest( child, isabmc2, 
                                              reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "activate_request" ) ) {
         REQNAME( "ACTIVATE" );
         ok = xmlParseActivateRequest( child, isabmc2, 
                                       reply, indent, inHeaders );
      } else if ( XMLString::equals( nodeName,
                                     "ext_services_request" ) ) {
         REQNAME( "EXTSERVICES" );
         ok = m_extServiceHelper->
            parseExtServicesReq( child, isabmc2, 
                                 reply, indent, inHeaders );
      } else if ( XMLString::equals( nodeName,
                                     "ext_search_request" ) ) {
         REQNAME( "EXTSEARCH" );
         ok = m_extServiceHelper->
            parseExtSearchReq( child, isabmc2, 
                               reply, indent, inHeaders );
      } else if ( XMLString::equals( nodeName,
                                     "tunnel_request" ) ) {
         REQNAME( "TUNNEL" );
         ok = xmlParseTunnelRequest( child, isabmc2, 
                                     reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "error_report" ) ) {
         REQNAME( "ERROR_REPORT" );
         ok = xmlParseErrorReport( child, isabmc2, 
                                   reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "poi_review_requests" ) ) {
         REQNAME( "POI_REVIEW" );
         ok = xmlParsePoiReviewRequests( child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "show_activationcode_request" ) ) {
         REQNAME( "SHOW_AC" );
         ok = xmlParseShowActivationcode( child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "expand_top_region_request" ) ) {
         REQNAME( "EXPAND_TOPREGION" );
         ok = xmlParseExpandTopRegion( child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "ad_debit_request" ) ){
         REQNAME( "AD_DEBIT" );
         ok = xmlParseAdDebitRequest( child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "client_type_info_request" ) ) {
         REQNAME( "CLIENT_TYPE_INFO" );
         ok = xmlParseClientTypeInfoRequest( child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "server_list_for_client_type_request" ) ){
         REQNAME( "SERVER_LIST_FOR_CLIENT_TYPE" );
         ok = xmlParseServerListForClientTypeRequest( 
            child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "create_wayfinder_user_request" ) ) {
         REQNAME( "CREATE_WAYFINDER_USER" );
         ok = xmlParseCreateWayfinderUserRequest( 
            child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "update_hardware_keys_request" ) ) {
         REQNAME( "UPDATE_HARDWARE_KEYS" );
         ok = xmlParseUpdateKeysRequest( child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "set_stored_user_data_request" ) ) {
         REQNAME( "SET_STORED_USER_DATA" );
         ok = xmlParseSetStoredUserData( child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "get_stored_user_data_request" ) ) {
         REQNAME( "GET_STORED_USER_DATA" );
         ok = xmlParseGetStoredUserData( child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "friend_finder_request" ) ) {
         REQNAME( "FRIEND_FINDER" );
         ok = xmlParseFriendFinder( child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName,
                                     "friend_finder_info_request" ) ) {
         REQNAME( "FRIEND_FINDER_INFO" );
         ok = xmlParseFriendFinderInfo( child, isabmc2, reply, indent );
      } else if ( XMLString::equals( nodeName, "cell_id_request" ) ) {
         REQNAME( "CELL_ID" );
         ok = xmlParseCellIDRequest( child, isabmc2, reply, indent );
      } else if (XMLString::equals( nodeName,
                                    "poi_search_request" ) ) {
         REQNAME( "POI_SEARCH" );
         ok = xmlParsePOISearchRequest( child, isabmc2,
                                        reply, indent );
      } else if ( XMLString::equals( nodeName, 
                                     "local_category_tree_request" ) ) {
         REQNAME( "LOCAL_CATEGORY_TREE" );
         ok = xmlParseLocalCategoryTreeRequest( child, isabmc2, reply, 
                                                indent );
      } else if ( XMLString::equals( nodeName, "server_info_request" ) ) {
         REQNAME( "SERVER_INFO" );
         ok = xmlParseServerInfoRequest( child, isabmc2, reply, indent );

      } else {
         ok = false;
         mc2log << warn  << "XMLParserThread::xmlParseIsabmc2 "
                << "odd Element in isab-mc2 element: " 
                << nodeName << endl;
      }
      
   }

   if ( indent ) {
      // Newline and indent before end isab-mc2 tag
      isabmc2->appendChild( reply->createTextNode( X( "\n" ) ) );
   }

   return ok;
}


bool 
XMLParserThread::xmlParsePublicRequest( DOMNode* cur, 
                                        DOMNode* out,
                                        DOMDocument* reply,
                                        bool indent )
{
   mc2dbg8 << "XMLParserThread::xmlParsePublicRequest" << endl;
   bool ok = true;
   
   // Get root element
   DOMElement* request = reply->getDocumentElement();

   // Go throu the elements and handle them.
   DOMNode* child = cur->getFirstChild();
   
   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(), "auth" ) ) {
               ok = xmlParseAuth( child, request, reply, indent );
            } else if ( XMLString::equals( child->getNodeName(), 
                                           "user_favorites_request" ) ) 
            {
               REQNAME( "PUB_USER_FAV" );
               ok = xmlParseUserFavoritesRequest( child, request, reply, 
                                                  indent );
            } else {
               ok = false;
               mc2dbg4 << "XMLParserThread::xmlParsePublicRequest "
                          "odd Element in request element: " 
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
            ok = false;
            mc2dbg4 << "XMLParserThread::xmlParsePublicRequest odd "
                       "node type in request element: " 
                    << child->getNodeName() 
                    << " type " << child->getNodeType()<< endl;
            break;
      }
      
      child = child->getNextSibling();
   }

   if ( indent ) {
      // Newline and indent before end request tag
      request->appendChild( reply->createTextNode( X( "\n" ) ) );
   }

   return ok;
}


bool 
XMLParserThread::xmlParseAuth( DOMNode* cur, 
                               DOMNode* out,
                               DOMDocument* reply,
                               bool indent ) 
{
   // Auth was checked first so this is ignored here
   
   return true;
}




bool 
XMLParserThread::handleXMLHttpRequest( HttpHeader* inHead, 
                                       HttpBody* inBody,
                                       stringMap* paramsMap,
                                       HttpHeader* outHead, 
                                       HttpBody* outBody,
                                       uint32 now,
                                       const char* dateStr )
{
#ifdef USE_XML
   MC2String charSet = inHead->getAcceptCharset();
   mc2dbg2 << "XMLParserThread::handleXMLHttpRequest" << endl;

   m_inHead = inHead;
   m_authData->reset();

   bool ok = true;
   MC2String errStr;
   bool indent = true; // If reply should contain indentation
   bool development = true; // If to be verbose, like loging reply
   uint32 linesAdded = 0;

   // Default is to debit, set to true in checkAuthorization
   m_isCheckUser = false;

   mc2log << info << "XMLParserThread::run xmlfile " << endl;
   XMLServerUtility::printWithLineNumbers( mc2log, inBody->getBody() );

   // Check if request is a xml and isab-mc2/request request.
   MC2String requestName;
   ok = checkAndFixHeader( inBody, errStr, linesAdded, requestName );
   if ( requestName.empty() ) {
      requestName = "isab-mc2";
   }
   MC2String replyName = ::getReplyDocumentName( requestName.c_str() );
   if ( !isalpha( replyName[ 0 ] ) ) {
      // Make it so
      replyName.insert( 0, "a" );
   }
   
   if ( !ok ) {
      ::makeErrorReply( outBody, outHead,
                        *inHead, "-2", errStr.c_str(), indent,
                        replyName.c_str() );
      return true;
   }

   // Make XML Document of request data
   MemBufInputSource xmlBuff( (byte*)inBody->getBody(), 
                              inBody->getBodyLength(), "xmlRequest" );

      
   try {
      mc2dbg8 << "About to parse (xerces)" << endl;
      uint32 startProcessTime = TimeUtility::getCurrentMicroTime();
      m_parser->parse( xmlBuff );
      uint32 stopProcessTime = TimeUtility::getCurrentMicroTime();
      mc2dbg2 << "XMLParserThread::handleHttpRequest xml request"
                 " xerces parse time "
              << (stopProcessTime - startProcessTime)/1000 << " ms" 
              << endl;
      mc2dbg8 << "Parse done (xerces)" << endl;
   } catch ( const XMLException& e ) {
      mc2log << warn 
             << "An XMLerror occured during parsing\n   Message: "
             << e.getMessage() << " line " 
             << (e.getSrcLine() - linesAdded) << endl;
      // Special case send e.getMessage() 
      char* eMsg = XMLUtility::transcodefromucs( e.getMessage() );
      errStr = eMsg;
      errStr.append( ", line " );
      char tmpStr[ 22 ];
      sprintf( tmpStr, "%d", e.getSrcLine() - linesAdded );
      errStr.append( tmpStr );
      mc2log << warn << "XMLParserThread::handleHttpRequest "
                "XMLException " << errStr << endl;
//             << "For XMLRequest:" << endl;
//      printWithLineNumbers( mc2log, inBody->getBody() );
      ok = false;
      delete [] eMsg;
   } catch( const SAXParseException& e) {
      mc2log << warn 
             << "An SAXerror occured during parsing\n   Message: "
             << e.getMessage() << ", "
             << "line " << (e.getLineNumber() - linesAdded) << ", column " 
             << e.getColumnNumber() << endl;
      // Special case send error 
      char* eMsg = XMLUtility::transcodefromucs( e.getMessage() );
      char tmpStr[44];
      sprintf( tmpStr, ", line %d, column %d",
               (int)e.getLineNumber() - linesAdded,
               (int)e.getColumnNumber() );
      errStr = eMsg;
      errStr.append( tmpStr );
      mc2log << warn << "XMLParserThread::handleHttpRequest "
         "SAXParseException " << errStr  << endl;
//             << "For XMLRequest:" << endl;
//      printWithLineNumbers( mc2log, inBody->getBody() );
      ok = false;
      delete [] eMsg;
   }
   mc2dbg8 << "Parse done " << endl;
   
   if ( !ok ) {
      mc2dbg << "Making error reply name: " << MC2CITE( requestName )
             << endl;
      ::makeErrorReply( outBody, outHead,
                      *inHead,
                      "-2", errStr.c_str(), indent,
                      replyName.c_str() );
      return true;
   }


   
   // Start handling Request
   DOMDocument* doc = m_parser->getDocument();
   DOMElement* root = doc->getDocumentElement();
   char* docname = XMLUtility::transcodefromucs( root->getTagName() );
   mc2dbg8 << "getDocument done " << endl;

   // If no Accept-Charset then use charset of incoming xml
   if ( inHead->getHeaderValue( HttpHeaderLines::ACCEPT_CHARSET ) == NULL ) {
      MC2String docCharSet = StringUtility::copyLower( 
         XMLUtility::transcodefrom( doc->getActualEncoding() ) );
      if ( !docCharSet.empty() ) {
         mc2log << info << "XMLParserThread::handleHttpRequest using "
                << "charSet from request document " << docCharSet << endl;
         charSet = docCharSet;
      }
   }

   DOMDocument* reply = ::makeDocument( replyName.c_str(), charSet.c_str() );
   outBody->setCharSet( charSet.c_str() );
   
   if ( XMLString::equals( root->getTagName(), "isab-mc2" ) ) {   
      // Check authorization
      if ( !checkAuthorization( doc, reply, indent, development, *inHead ) ) { 
         mc2dbg8 << "User unauthorized" << endl;
         MC2String replyString = 
            XMLTreeFormatter::makeStringOfTree( reply, charSet.c_str() );
         outHead->addHeaderLine( &ContentTypeStr, 
                                 new MC2String( "text/xml" ) );
         cerr << replyString << endl;
         outBody->setBody( &replyString );      
         releaseUserItem( m_user );
         clearUserItem();
         m_allowedRequests.clear();
         reply->release();
         delete [] docname;
         return true;
      }
   } else if ( XMLString::equals( root->getTagName(), "request" ) ) { 
      // Check auth
      if ( !checkPublicAuth( doc, reply, indent, development ) ) { 
         mc2dbg8 << "User unauthorized" << endl;
         MC2String replyString = 
            XMLTreeFormatter::makeStringOfTree( reply, charSet.c_str() );
         outHead->addHeaderLine( &ContentTypeStr, 
                                 new MC2String( "text/xml" ) );
         outBody->setBody( &replyString );
         
         releaseUserItem( m_user );
         clearUserItem();
         m_allowedRequests.clear();
         reply->release();
         delete [] docname;
         return true;
      }
   } else {
      // Unknown root element (document type)
      mc2log << error << "XMLParserThread::handleHttpRequest unknown "
             << "document " << MC2CITE( root->getTagName() ) << endl;
      ::makeErrorReply( outBody, outHead, *inHead,
                        "-2", "Unknown document type", 
                        indent, replyName.c_str() );
      delete [] docname;
      reply->release();
      return true;
   }
   
   // Set m_logname to users name
   if ( m_user ) {
      m_logname = m_user->getUser()->getLogonID();
      if ( m_authData ) {
         setClientSetting( m_authData->clientSetting );
         setRequestData( m_authData );
      }
   } else {
      m_logname = "";
   }
   setLogUserName( m_logname.c_str() );


   mc2dbg8 << "Auth ok " << endl;
   // Handle Requests in document
   using XMLServerUtility::printWithLineNumbers; 
   try {
      mc2dbg8 << "About to parse" << endl;
      ok = xmlParseRequest( doc, reply, indent, *inHead );
      mc2dbg8 << "Parse done" << endl;
   }
   catch ( const DOMException& e ) {
      char* error = XMLUtility::transcodefromucs( e.msg );
      mc2log << warn << "An error occured during creating reply" 
             << "   Message: " << error << endl;
      // Special case send error 
      ::makeErrorReply( outBody, outHead,
                        *inHead,
                        "-1", error, indent, 
                        replyName.c_str() );
      MC2String incompleteReplyDoc = XMLTreeFormatter::makeStringOfTree(
         reply, charSet.c_str() );
      mc2log << warn << "XMLParserThread::handleHttpRequest "
                "Internal server DOMException " << error << endl;
//             << "For XMLRequest:" << endl;
//      printWithLineNumbers( mc2log, inBody->getBody() );
      mc2log << "Incomplete reply:" << endl;
      printWithLineNumbers( mc2log, incompleteReplyDoc.c_str() );
      delete [] error;
               
      releaseUserItem( m_user );
      clearUserItem();
      m_allowedRequests.clear();
      reply->release();
      delete [] docname;
      return true;
   } 

   if ( !ok ) {
      // Get the set error and send it
      const char* errStr = 
         "Internal Server error, could not handle request.";
      ::makeErrorReply( outBody, outHead, *inHead,
                        "-1", errStr, indent, 
                        replyName.c_str() );
      MC2String incompleteReplyDoc = XMLTreeFormatter::makeStringOfTree(
         reply, charSet.c_str() );
      mc2log << warn << "XMLParserThread::handleHttpRequest "
                "Internal server error " << errStr << endl;
//             << "For XMLRequest:" << endl;
//      printWithLineNumbers( mc2log, inBody->getBody() );
      mc2log << "Incomplete reply:" << endl;
      printWithLineNumbers( mc2log, incompleteReplyDoc.c_str() );
      releaseUserItem( m_user );
      clearUserItem();
      m_allowedRequests.clear();
      reply->release();
      delete [] docname;
      return true;
   }

   // Put Answer together
   mc2dbg8 << "makeStringOfTree" << endl;
   uint32 startProcessTime = TimeUtility::getCurrentMicroTime();
   MC2String replyString = 
      XMLTreeFormatter::makeStringOfTree( reply, charSet.c_str() );
   uint32 stopProcessTime = TimeUtility::getCurrentMicroTime();
   mc2dbg2 << "XMLParserThread::handleHttpRequest make xml reply"
              " time "
           << (stopProcessTime - startProcessTime)/1000 << " ms" 
           << ", size " << replyString.size() << endl;

   if ( development ) {
      mc2log << info << "XMLParserThread::run ReplyTree:"
             << endl;
      printWithLineNumbers( mc2log, replyString.c_str() );
   } else {
      // Print stats about reply, that is done in HttpParserThread 
      // (time and size)
   }
      
   outHead->addHeaderLine( &ContentTypeStr, new MC2String( "text/xml" ) );
   outBody->setBody( &replyString );

   
   if ( Properties::getUint32Property( "XML_SERVER_REPLY_DTD_CHECK", 0 ) ) {
      MC2String requestName2;
      uint32 linesAdded2;
      HttpBody testBody( MC2String( outBody->getBody(), 
                                    outBody->getBodyLength() ) );
      checkAndFixHeader( &testBody, errStr, linesAdded2, requestName2 );
   
      // Check reply against dtd
      MemBufInputSource xmlReplyBuff( (byte*)testBody.getBody(),
                                      testBody.getBodyLength(), "xmlReply" );
      try {
         mc2dbg << "XMLParserThread::handleHttpRequest About to parse "
                 << "reply (xerces)" << endl;
         uint32 startProcessTime = TimeUtility::getCurrentMicroTime();
         m_parser->parse( xmlReplyBuff );
         uint32 stopProcessTime = TimeUtility::getCurrentMicroTime();
         mc2dbg << "XMLParserThread::handleHttpRequest xml reply "
                   "xerces parse time "
                << (stopProcessTime - startProcessTime)/1000 << " ms" 
                 << endl;
         mc2dbg << "XMLParserThread::handleHttpRequest Done parsing "
                 << "reply (xerces)" << endl;         
      } catch ( const XMLException& e ) {
         ostringstream x;
         x << "An XMLerror occured during parsing of reply" << endl
           << "   Message: "
           << e.getMessage() << " line " 
           << e.getSrcLine() << endl << ends;
         mc2log << x;
         outBody->addString( "<!--\n" );
         outBody->addString( x.str() );
         outBody->addString( "-->\n" );
      } catch( const SAXParseException& e) {
         ostringstream x;
         x << "An SAXerror occured during parsing of reply" << endl
           << "   Message: "
           << e.getMessage() << ", "
           << "line " << (int)e.getLineNumber() << ", column " 
           << (int)e.getColumnNumber() << endl;
         mc2log << x;
         outBody->addString( "<!--\n" );
         outBody->addString( x.str() );
         outBody->addString( "-->\n" );         
      }
   }

   // Return memory for reply
   reply->release();
   // Don't keep request in parser
   m_parser->resetDocumentPool();

   releaseUserItem( m_user );
   clearUserItem();
   m_authData->reset();
   m_allowedRequests.clear();
   delete [] docname;

#else
   // Send some answer as we can't parse without XML support
   // Send answer
   MC2String errStr = "Server configured without XML support, "
      "unable to handle request.";
   outBody->setBody( &errStr );
   outHead->addHeaderLine(&ContentTypeStr, 
                          new MC2String("text/plain"));  
#endif
   return true;
}


bool 
XMLParserThread::checkPublicAuth( 
   const DOMDocument* doc, DOMDocument* reply,
   bool& indent, bool& development )
{
   mc2dbg4 << "XMLParserThread::checkPublicAuth" << endl;
   volatile uint32 startProcessTime = TimeUtility::getCurrentMicroTime();

   int indentLevel = 1;
   const DOMNode* node = doc;
   bool authorized = false;
   const char* unauthorizedStr = "unauthorized";
   MC2String errorCode = "-1";

   DOMNodeList* auths = doc->getElementsByTagName( X( "auth" ) );

   if ( auths->getLength() == 1 ) {
      // Get the auth element (only the first is valid request 
      //                       as described in the DTD)
      node = auths->item( 0 );

      // Check attrubutes to auth-element
      DOMNamedNodeMap* attributes = node->getAttributes();
      DOMNode* attribute;
      MC2String userName;
      MC2String userPasswd;

      for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
         attribute = attributes->item( i );
      
         char* tmpStr = XMLUtility::transcodefromucs( 
            attribute->getNodeValue() );
         if ( XMLString::equals( attribute->getNodeName(),
                                 "login" ) )
         {
            userName = tmpStr;
         } else if ( XMLString::equals( attribute->getNodeName(),
                                        "password" ) ) 
         {
            userPasswd = tmpStr;
         } else {
            mc2log << warn << "XMLParserThread::checkPublicAuth "
                      "unknown attribute for auth element "
                   << "Name " << attribute->getNodeName()
                   << " Type " << attribute->getNodeType() << endl;
         }
         delete [] tmpStr;
      }
      
      if ( !userName.empty() && !userPasswd.empty() ) {
         // Check user
         uint32 UIN = authenticateUser( userName.c_str(), 
                                        userPasswd.c_str(), true );

         if ( UIN != 0 && UIN != MAX_UINT32 && UIN != (MAX_UINT32 -1) ) {
            if ( getUser( UIN, m_user, true ) ) 
            {
               if ( m_user != NULL ) {
                  authorized = 
                     ( m_user->getUser()->getHTMLService() || 
                       m_user->getUser()->getExternalXmlService() ||
                       checkUserAccess( m_user->getUser(), "HTML", true ) );
            
                  if ( authorized ) {
                     logUserAccepted( "XMLParserThread::checkPublicAuth" );
                  } else {
                     unauthorizedStr = "Access denied.";
                     errorCode = "-201";
                     mc2log << warn 
                            << "XMLParserThread::checkPublicAuth " 
                               "User has no access to service!"
                            << "UserName " 
                            << m_user->getUser()->getLogonID() 
                            << "(" << m_user->getUIN() << ")" << endl;
                  }
               } else {
                  unauthorizedStr = "Unknown user.";
                  errorCode = "-202";
                  mc2log << info << "XMLParserThread::checkPublicAuth "
                     "failed to get user, UIN " << UIN << endl;
               }
            } else {
               unauthorizedStr = "Database connection error, failed to "
                  "get user data.";
               errorCode = "-3";
               mc2log << info << "XMLParserThread::checkPublicAuth "
                         "failed to get user"
                      << "UserName " << userName << "(" << UIN << ")"
                      << endl;
            }
         } else {
            if ( UIN == MAX_UINT32 ) {
               unauthorizedStr = 
                  "Database connection error, no connection.";
               errorCode = "-3";
            } else if ( UIN == (MAX_UINT32 -1) ) {
               unauthorizedStr = "Expired user.";
               errorCode = "-206";
            } else {
               unauthorizedStr = "Invalid login.";
               errorCode = "-203";
            }
            mc2log << info << "XMLParserThread::checkPublicAuth "
                   << "authorization failed: " << unauthorizedStr << endl;
         }
      } else { // No login and password
         mc2log << warn 
                << "XMLParserThread::checkPublicAuth auth element"
                << " bad content!" << endl;
         mc2dbg4 << "userName " << MC2CITE( userName )
                 << "userPasswd " << MC2CITE( userPasswd )
                 << endl;
         
         unauthorizedStr = "No data to authenticate user with.";
         errorCode = "-1";
      }
   } else {
      mc2log << warn << "XMLParserThread::checkPublicAuth "
                "not one auth element!" << endl;
      mc2dbg8 << "auths.getLength() " << auths->getLength() 
              << endl;
      unauthorizedStr = "Not one auth element in request.";
      errorCode = "-1";
   }

   if ( !authorized ) {
      // Send status
      DOMElement* root = reply->getDocumentElement();
         
      appendStatusNodes( reply->getLastChild(), reply, indentLevel, indent,
                         errorCode.c_str(), unauthorizedStr );
      // Indent and linebreak before ending reply tag
      if ( indent ) {
         root->appendChild( reply->createTextNode( X( "\n" ) ) );
      }
   }

   uint32 stopProcessTime = TimeUtility::getCurrentMicroTime();
   mc2dbg2 << "XMLParserThread::checkPublicAuth process time " 
           << (stopProcessTime - startProcessTime)/1000 << " ms" << endl;

   return authorized;
}



#endif // USE_XML


bool 
XMLParserThread::checkAndFixHeader( HttpBody* body, MC2String& errStr, 
                                    uint32& linesAdded, 
                                    MC2String& requestName ) 
{
   // Check if request is a xml and isab-mc2/request request.

   // If starts with <?xml (utf-16 [LE fffe, BE feff] 3c00 
   //   (Byte order mark, BOM 0xFEFF, and '<'))
   //   find ?> then check for whitespace <!DOCTYPE 
   //      check if isab-mc2/request
   //         find end > and check if SYSTEM [isab-mc2|public].dtd is 
   //            present, add if not.
   // else if starts with whitespace <isab-mc2 or <request then add xml 
   //         header
   // else not a valid request

   bool ok = true;

   if ( body->getBodyLength() > 9 ) { // Some data
      // Check for BOM
      const char* bodyData = body->getBody();
      bool prependXmlHeader = false;
      bool replaceDocType = false;
      int reqType = 0; // 0 == isab-mc2, 1 == request
      if ( (byte(bodyData[ 0 ]) == 0xff && 
            byte(bodyData[ 1 ]) == 0xfe ) || // LE
           (byte(bodyData[ 0 ]) == 0xfe && 
            byte(bodyData[ 1 ]) == 0xff ) )   // BE
      {
         // Add check here for utf-16 content, when needed
      } else { // More like ascii (utf-8/iso-8959)
         if ( strncmp( "<?xml version=", bodyData, 14 ) == 0 ) {
            char* endStr = StringUtility::strstr( bodyData + 14, "?>" );
            if ( endStr != NULL ) {
               endStr += 2; // ?>
               if ( strncmp( "<!DOCTYPE", 
                             StringUtility::trimStart( endStr ), 9 ) == 0 )
               {
                  // Ok has DOCTYPE but is it right?
                  // <!DOCTYPE isab-mc2 SYSTEM "isab-mc2.dtd" >
                  // or
                  // <!DOCTYPE request SYSTEM "public.dtd">
                  endStr = StringUtility::trimStart( endStr ); // White
                  endStr += 9; // <!DOCTYPE
                  char* endDocStr = strstr( endStr, ">" );
                  int docIsabCmp = strncmp( 
                     StringUtility::trimStart( endStr ), "isab-mc2", 8 );
                  int docReqCmp = strncmp( 
                     StringUtility::trimStart( endStr ), "request", 7 );
                  if ( endDocStr != NULL && 
                       (docIsabCmp == 0 || docReqCmp == 0) )
//docIsabStr != NULL &&
//                       (docIsabStr < endDocStr ) )
                  {
                     if ( docIsabCmp == 0 ) {
                        reqType = 0;
                     } else {
                        reqType = 1;
                     }
                     MC2String docTypeStr ( endStr, (endDocStr-endStr) + 1 );
                     uint32 maxStrlen = (endDocStr-endStr) + 1;
                     char docType[ maxStrlen ];
                     char system[ maxStrlen ];
                     char isabDtd[ maxStrlen ];
                     if ( sscanf( docTypeStr.c_str(), "%s%s%s", docType, 
                                  system, isabDtd  ) == 3 ) 
                     {
                        // Ok check content
                        if ( !(reqType == 0 && 
                               strcmp( docType, "isab-mc2" ) == 0 ) &&
                             !(reqType == 1 && 
                               strcmp( docType, "request" ) == 0) )
                        {
                           ok = false;
                           errStr = "Unknown document type in DOCTYPE.";
                        }
                        if ( ok && strcmp( system, "SYSTEM" ) != 0 ) {
                           replaceDocType = true;
                        }
                        if ( ok ) {
                           if ( reqType == 0 && 
                                strstr( isabDtd, "isab-mc2.dtd" ) == NULL )
                           {
                              replaceDocType = true;
                           }
                           if ( reqType == 1 && 
                                strstr( isabDtd, "public.dtd" ) == NULL )
                           {
                              replaceDocType = true;
                           }
                        }
                     } else if ( strncmp( StringUtility::trimStart( 
                                             docTypeStr.c_str() ), 
                                          "isab-mc2", 8 ) == 0 )
                     {
                        replaceDocType = true;
                     } else if ( strncmp( StringUtility::trimStart( 
                                             docTypeStr.c_str() ), 
                                          "request", 7 ) == 0 )
                     {
                        replaceDocType = true;
                     } else {
                        errStr = "Unknown document type.";
                        ok = false;
                     }

                     endStr = StringUtility::trimStart( endStr );
                     char* endName = endStr;
                     while ( *endName != '>' && *endName != '\0' &&
                             *endName != '<' && !isspace( *endName ) && 
                             /*StringUtility::*/isgraph(*endName) )
                     {
                        endName++;
                     }
                     requestName.assign( endStr, (endName - endStr) + 1 );
                     if ( requestName[ requestName.size() -1 ] == '>' ||
                          requestName[ requestName.size() -1 ] == '<' ||
                          isspace( requestName[ requestName.size() -1 ] )
                          || !StringUtility::isAlNum(
                             requestName[ requestName.size() -1 ] ) )
                     {
                        requestName.erase( requestName.size() -1 );
                     }
                     // Check for isab-mc2 or request
                     if ( ok && 
                          ( (reqType == 0 &&
                             strncmp( "<isab-mc2", 
                                      StringUtility::trimStart( 
                                         endDocStr + 1 ), 9 ) == 0 ) ||
                            (reqType == 1 &&
                             strncmp( "<request", 
                                      StringUtility::trimStart( 
                                         endDocStr + 1 ), 8 ) == 0 ) ) )
                     {
                        // Check if '>' after root-element name
                        endStr = StringUtility::trimStart( endDocStr + 1 );
                        if ( reqType == 0 ) {
                           endStr += 9; // <isab-mc2
                        } else {
                           endStr += 8; // <request
                        }
                        if ( StringUtility::trimStart( endStr )[ 0 ] 
                             == '>' )
                        {
                           // Ok tired of checking, let parser check 
                           // from here
                        } else {
                           // Hmm, something after root tag before '>'
                           ok = false;
                           errStr = "Unknown name on document element.";
                        }
                     } else if ( ok ) {
                        ok = false;
                        errStr = "Couldn't find document start after xml"
                           " header.";
                     }
                  } else {
                     errStr = "Problem with DOCTYPE: ";
                     if ( endDocStr == NULL ) {
                        errStr.append( "Couldn't find end of DOCTYPE." );
                     } else if ( docIsabCmp != 0 || docReqCmp != 0 ) {
                        errStr.append( "Not a supported DOCTYPE." );
                        endStr = StringUtility::trimStart( endStr );
                        char* endName = endStr;
                        while ( *endName != '>' && *endName != '\0' &&
                                *endName != '<' && !isspace( *endName ) && 
                                /*StringUtility::*/isgraph(*endName) )
                        {
                           endName++;
                        }
                        requestName.assign( endStr, 
                                            (endName - endStr) + 1 );
                        if ( requestName[ requestName.size() -1 ] == '>' ||
                             requestName[ requestName.size() -1 ] == '<' ||
                             isspace( requestName[ requestName.size() -1] )
                             || !/*StringUtility::*/isgraph(
                                requestName[ requestName.size() -1 ] ) )
                        {
                           requestName.erase( requestName.size() -1 );
                        }
                     } else {
                        errStr.append( "Couldn't understand DOCTYPE." );
                     }
                     ok = false;
                  }
               } else {
                  errStr = "Couldn't find DOCTYPE after xml header line.";
                  ok = false;
               }
            } else {
               errStr = "Request contains no end of xml header line.";
               ok = false;
            }
         } else if ( strncmp( "<isab-mc2", StringUtility::trimStart( 
                                 bodyData ), 9 ) == 0 )
         { // <isab-mc2
            // Check if '>' after root-element name
            char* endStr = StringUtility::trimStart( bodyData + 9 );
            requestName.assign( 
               StringUtility::trimStart( bodyData ) + 1,
               endStr - StringUtility::trimStart( bodyData ) -1 );
            if ( StringUtility::trimStart( endStr )[ 0 ] == '>' ) {
               // Prepend xml header
               prependXmlHeader = true;
               reqType = 0;
            } else {
               // Hmm, something after root tag before '>'
               ok = false;
               errStr = "Unknown name on document element.";
            }
         } else if ( strncmp( "<request", StringUtility::trimStart( 
                                 bodyData ), 8 ) == 0 )
         { // <request
            // Check if '>' after root-element name
            char* endStr = StringUtility::trimStart( bodyData + 8 );
            requestName.assign( 
               StringUtility::trimStart( bodyData ) + 1,
               endStr - StringUtility::trimStart( bodyData ) -1 );
            if ( StringUtility::trimStart( endStr )[ 0 ] == '>' ) {
               // Prepend xml header
               prependXmlHeader = true;
               reqType = 1;
            } else {
               // Hmm, something after root tag before '>'
               ok = false;
               errStr = "Unknown name on document element.";
            }
         } else {
            const char* startStr = bodyData;
            if ( startStr[ 0 ] == '<' ) {
               startStr++;
            }
            const char* endStr = startStr;
            while ( *endStr != '>' && *endStr != '\0' && *endStr != '<' &&
                    !isspace( *endStr ) && 
                    StringUtility::isAlNum(*endStr) ) 
            {
               endStr++;
            }
            requestName.assign( startStr, (endStr - startStr) + 1 );
            if ( requestName[ requestName.size() -1 ] == '>' ||
                 requestName[ requestName.size() -1 ] == '<' ||
                 isspace( requestName[ requestName.size() -1 ] ) ||
                 !StringUtility::isAlNum(
                    requestName[ requestName.size() -1 ] ) )
            {
               requestName.erase( requestName.size() -1 );
            }
            errStr = "Request has unknown start tag.";
            ok = false;
         }

         if ( ok ) {
            if ( prependXmlHeader || replaceDocType ) {
               // Ok make the request ok for parser
               const MC2String docType( reqType == 0 ? 
                  "<!DOCTYPE isab-mc2 SYSTEM \"isab-mc2.dtd\">\r\n" :
                  "<!DOCTYPE request SYSTEM \"public.dtd\">\r\n" );
               MC2String newBody( body->getBody() );
               if ( prependXmlHeader ) {
                  MC2String header( 
                     "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\r\n" );
                  header.append( docType );
                  newBody.insert( 0, header );
                  linesAdded = 2;
               } else if ( replaceDocType ) {
                  MC2String::size_type docStart = newBody.find( "<!DOCTYPE" );
                  MC2String::size_type docEnd = newBody.find( ">", docStart );
                  if ( docStart != MC2String::npos && 
                       docEnd != MC2String::npos )
                  {
                     newBody.replace( docStart, (docEnd - docStart) + 1,
                                      docType );
                     linesAdded = 1;
                  } else {
                     ok = false;
                     errStr = "DOCTYPE start and end could not be found.";
                  }
               }

               body->setBody( &newBody );
            } // Else let it pass as is            
         } // End if ok

      }

   } else {
      ok = false;
      errStr = "Request too short.";
   }

   return ok;
}


UserEnums::userRightLevel
XMLParserThread::getClientTypeLevel() const {
   if ( m_authData->clientType.find( "jn-" ) != MC2String::npos ) {
      return UserEnums::UR_GOLD;
   } else if ( m_authData->clientType.find( "jc-" ) != MC2String::npos )
   {
      return UserEnums::UR_SILVER;
   } else {
      return UserEnums::UR_NO_LEVEL;
   }
}

LangTypes::language_t
XMLParserThread::getStringAsLanguage( const char* langStr ) const {
   return LangUtility::getStringAsLanguage( langStr );
}

StringTable::languageCode
XMLParserThread::getLanguageCode( const char* langStr ) const {
   return LangUtility::getLanguageCode( langStr );
}


UserEnums::userRightService
XMLParserThread::getRightService( const char* service ) const {
   UserEnums::userRightService serv = UserEnums::UR_XML;
   if ( StringUtility::strcasecmp( service, "HTML" ) == 0 ||
        StringUtility::strcasecmp( service, "WAP" ) == 0 ) 
   {
      serv = UserEnums::UR_MYWAYFINDER; 
   } else if ( StringUtility::strcasecmp( service, "JAVA" ) == 0 ) {
      serv = UserEnums::userRightService( UserEnums::UR_WF );
   }
   return serv;
}


const char*
XMLParserThread::getServerType()
{
   return ("XML");
}

const InfoTypeConverter& XMLParserThread::getInfoTypeConverter() const {
   return *m_infoTypes;
}


void XMLParserThread::logUserAccepted( const MC2String& prefix ) {
   // this function should only be called after a user has been authorized
   // so m_user should be != NULL
   MC2_ASSERT( m_user != NULL );

   mc2log << info
          << prefix
          << " User accepted. UserName "
          << m_user->getUser()->getLogonID()
          << "(" << m_user->getUIN() << ")" << endl;
   logLastClient();
}


bool
XMLParserThread::checkAllowedToSearch( DOMNode* root,
                                       DOMDocument* reply,
                                       CompactSearch& params,
                                       bool indent
                                       ) {
   PurchaseOptions purchaseOptions( m_authData->clientType );
   set< MC2String > checkedServiceIDs;
   if ( ! checkService( getClientSetting(), getHttpInterfaceRequest(),
                        OperationType::SEARCH_RIGHT,
                        purchaseOptions,
                        checkedServiceIDs,
                        params.m_location.m_coord,
                        params.m_topRegionID,
                        m_authData->clientLang,
                        params.m_round >= 1 ) ) {
      // User not allowed to search, present purchase options
      using namespace PurchaseOption;
      mc2log << warn << "[XMLParserThread]: checkService failed.";
      if ( purchaseOptions.getReasonCode() == NEEDS_TO_BUY_APP_STORE_ADDON ) {
         // User not allowed to route, present purchase options
         // return the uri to the client so it can send it to the web server
         // FIXME: App store id not supported by xml-protocol nor 
         //        java-clients.
         mc2log << " User has no accsess for searching."
                << " Returning purchase options. ";
         appendStatusNodes( root, reply, 1, false, "-1", 
                            "Searching not allowed.",
                            NULL, purchaseOptions.getURL().c_str() );
      } else {
         mc2log << " General error.";
         if ( purchaseOptions.getReasonCode() == SERVICE_NOT_FOUND ) {
            // Service id not possible to purchase
            mc2log << " Service id not possible to purchase";
            
            MC2String noServiceIdURL( "http://show_msg/?txt=" );
            noServiceIdURL += StringUtility::URLEncode(
               StringTable::getString( StringTable::WF_NO_BILL_AREA,
                                       m_authData->clientLang ) );
            
            appendStatusNodes( root, reply, 1, false, "-1",
                               StringTable::
                               getString( StringTable::WF_NO_BILL_AREA,
                                          m_authData->clientLang ),
                               NULL,
                               noServiceIdURL.c_str() );
         } else {
            appendStatusNodes( root, reply, 1, false, "-1",
                               "General error." );
         }
      }
      mc2log << endl;
      
      if ( indent ) {
         XMLUtility::indentPiece( *root, 1 );
      }
      
      return false;
   }
   
   return true;
}
