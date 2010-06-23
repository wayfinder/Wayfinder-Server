/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef __linux__
#   include <features.h>
#   ifndef __USE_XOPEN
#      define __USE_XOPEN
#   endif
#endif
#include "HttpParserThread.h"
#include "HttpFunctionHandler.h"
#include "HttpParserThreadGroup.h"
#include "SinglePacketRequest.h"
#include "HttpParserThreadUtility.h"
#include "HttpHeader.h"
#include "HttpBody.h"
#include "HttpInterfaceRequest.h"
#include "HttpFileHandler.h"

#include "DataBuffer.h"
#include "MC2SimpleString.h"
#include "HttpMapFunctions.h"

#include "STLStringUtility.h"
#include "UserData.h"
#include "UserSwitch.h"
#include "XSData.h"
#include "Utility.h"

// fstat includes
#ifdef __linux
   #include <sys/stat.h>
#endif
#ifdef __SVR4
   #include <sys/types.h>
   #include <sys/stat.h>
#endif
#ifdef WIN32
   #include <stat.h>
   // I think in WIN32
#endif
#include "zlib.h"

// Some usefull Http strings
#include "HttpHeaderLines.h"

#include "HttpCodes.h"

using namespace HttpHeaderLines;

HttpParserThread::HttpParserThread( HttpParserThreadGroup* group ):
   InterfaceParserThread( group, "HttpParserThread" ),
   m_functions( new HttpFunctionHandler() )
{
}


HttpParserThread::~HttpParserThread() {
   delete m_functions;
}


void 
HttpParserThread::addRequestName( const char* name ) {
   if ( !m_requestName.empty() ) {
      m_requestName.append( ", " ); 
   } 
   m_requestName.append( name );
}


uint32 
HttpParserThread::getCurrRequestStartTime() const {
   if ( m_irequest != NULL ) {
      return m_irequest->getStartTime();
   } else {
      return TimeUtility::getCurrentTime();
   }
}


void 
HttpParserThread::setStatusReply( uint32 statusCode ) {
   if ( m_irequest != NULL ) {
      m_irequest->setStatusReply( statusCode );
   } else {
      mc2log << error << "HttpParserThread::setStatusReply called but no "
             << "request to set reply in!" << endl;
   }
}


void 
HttpParserThread::setLogUserName( const char* userName ) {
   if ( m_irequest != NULL ) {
      m_irequest->setLogUserName( userName );
      m_logname = userName;
      setLogPrefix( m_irequest->getLogPrefix() );
   } else {
      mc2log << error << "HttpParserThread::setLogUserName called but no "
             << "irequest to set name in!" << endl;
   }
}


bool
HttpParserThread::checkHttpHeader( HttpInterfaceRequest* ireq ) {
   HttpHeader& inHead = *ireq->getRequestHeader();

   // Check HTTP method
   if ( inHead.getMethod() != HttpHeader::GET_METHOD &&
        inHead.getMethod() != HttpHeader::POST_METHOD &&
        inHead.getMethod() != HttpHeader::HEAD_METHOD )
   {
      mc2log << warn << "HttpParserThread method " 
             << inHead.getMethodString() << " not supported peer " 
             << ireq->getPeer() << " Startline: " 
             << *inHead.getStartLine()<< endl;
      vector< MC2String > extraHeaderFields;
      extraHeaderFields.push_back( ALLOW + ": " + "GET, HEAD, POST" );
      ireq->setStatusReply( HttpCode::METHOD_NOT_ALLOWED, 0,
                            &extraHeaderFields );
      return false;
   }

   // HTTP 1.1 requirements
   if ( inHead.getMinorVersion() == 1 && inHead.getMajorVersion() == 1 ) {
      // Must have Host:
      if ( inHead.getHeaderValue( &HOST ) == NULL ) {
         // 400
         mc2log << warn << "HttpParserThread HTTP 1.1 requires Host peer " 
                << ireq->getPeer() << endl;
         ireq->setStatusReply( HttpCode::BAD_REQUEST );
         return false;
      }
   }
   
   return true;
}


void
HttpParserThread::setKeepAlive( HttpInterfaceRequest* ireq, 
                                HttpHeader* outHead ) 
{
   HttpHeader& inHead = *ireq->getRequestHeader();

   // Set "Keep-Alive:" if client requested it
   const MC2String* connection = inHead.getHeaderValue( &CONNECTION );
   if ( inHead.getMinorVersion() > 0 && inHead.getMajorVersion() >= 1 ) {
      // HTTP 1.1 has persistent connections as default
      if ( (connection != NULL && 
            StringUtility::strcasecmp( connection->c_str(), 
                                       CLOSE.c_str() ) == 0) ||
           ireq->getNbrReuses() 
           >= HttpInterfaceRequest::MAX_NUMBER_SOCKET_REUSES )
      {
         outHead->addHeaderLine( &CONNECTION, new MC2String( CLOSE ) );
      }
   } else if ( connection != NULL &&
               StringUtility::strcasecmp( connection->c_str(), 
                                          KEEP_ALIVE.c_str() ) == 0 &&
               ireq->getNbrReuses() <
               HttpInterfaceRequest::MAX_NUMBER_SOCKET_REUSES )
   {
      outHead->addHeaderLine( &CONNECTION, 
                              new MC2String( KEEP_ALIVE ) );  
      outHead->addHeaderLine( &KEEP_ALIVE, new MC2String( "" ) );  
   } else {
      outHead->addHeaderLine( &CONNECTION, new MC2String( CLOSE ) );
   }
}


void
HttpParserThread::setHttpHeaderDate( HttpHeader* outHead, uint32 now ) {
   // Set date of request
   char dateStr[ 50 ];
   makeDateStr( dateStr, now );
   outHead->addHeaderLine( &DATE, new MC2String( dateStr ) );
}


void 
HttpParserThread::makeDateStr( char* dateStr, uint32 date ) {
   time_t t = date;
   struct tm result;
   struct tm* tmStruct = gmtime_r( &t, &result );
   tmStruct->tm_isdst = 1;
   strftime( dateStr, 30, "%a, %d %b %Y %H:%M:%S GMT", tmStruct );
}

uint32
HttpParserThread::dateStrToInt( const char* dateStr ) {
   struct tm tm; 
   strptime( dateStr, "%a, %d %b %Y %H:%M:%S GMT", &tm );
   time_t t = timegm( &tm );

   return uint32( t );
}

void
HttpParserThread::setDefaultResponseLine( HttpInterfaceRequest* ireq, 
                                          HttpHeader* outHead )
{
   HttpHeader& inHead = *ireq->getRequestHeader();

   if ( inHead.getMinorVersion() == 0 ) {
      outHead->setStartLine( new MC2String( "HTTP/1.0 200 OK\r\n" ) );
   } else { // 1.1
      outHead->setStartLine( new MC2String( "HTTP/1.1 200 OK\r\n" ) );
   }
}


void 
HttpParserThread::handleInterfaceRequest( InterfaceRequest* ireq ) {
   MC2_ASSERT( dynamic_cast< HttpInterfaceRequest* > ( ireq ) );

   HttpInterfaceRequest* irequest = static_cast< HttpInterfaceRequest* > (
      ireq );
   m_irequest = irequest;
   m_requestName.clear();
   char dateStr[ 50 ];
   setLogPrefix( m_irequest->getLogPrefix() );
   HttpHeader& inHead = *irequest->getRequestHeader();
   HttpBody& inBody = *irequest->getRequestBody();
   const MC2String ContentTypeStr = "Content-Type";
   const MC2String CONTENTLENGTH = "Content-Length";
   const MC2String USERAGENT = "User-Agent";
   MC2String* tmpString = NULL;
   time_t now;  // Date
   MC2String userAgent;
   uint32 replySize = 0;

   if ( !checkHttpHeader( m_irequest ) ) {
      return;
   }

   HttpHeader* outHead = new HttpHeader;
   HttpBody* outBody = new HttpBody;

   // Clear m_logname
   m_logname = "";

   // Parameters from start line
   stringMap paramsMap;

   // Add any parameters from startline to paramsMap
   const MC2String* startLine = inHead.getStartLine();
   MC2String::size_type findPos = startLine->find( '?' );
   if ( findPos != MC2String::npos ) {
      MC2String::size_type endPos = startLine->find( ' ', findPos + 1 );
      MC2String parameters = startLine->substr( findPos + 1, 
                                                endPos - findPos - 1 );
      parseParameters( parameters, &paramsMap, &inHead );
   }

   // Absolute uri http://php/Map.php?a=b
   if ( StringUtility::strncasecmp( 
           "http://", inHead.getURLPath()->c_str(), 7 ) == 0 ||
        StringUtility::strncasecmp( 
           "https://", inHead.getURLPath()->c_str(), 8 ) == 0 )
   {
      // Remove the http[s]://[host]/ and replace by /
      uint32 protSize = 7;
      if ( StringUtility::strncasecmp( 
              "https://", inHead.getURLPath()->c_str(), 8 ) == 0 )
      {
         protSize = 8; 
      }
      findPos = inHead.getURLPath()->find( '/', protSize );
      MC2String host( inHead.getURLPath()->substr( 
                         protSize, findPos - protSize ) );
      if ( findPos != MC2String::npos ) {
         MC2String path( inHead.getURLPath()->substr( findPos ) );
         inHead.setURLPath( new MC2String( path ) );
         inHead.setPagename( new MC2String( 
                                path.substr( path.rfind( '/' ) + 1 ) ) );
      } else {
         // only host no path set '/'
         inHead.setURLPath( new MC2String( "/" ) );
         inHead.setPagename( new MC2String( "" ) );
      }
      // The host of the Absolute uri should be used
      inHead.addHeaderLine( &HOST, new MC2String( host ) );
   }
   // Remove ?... from page, url
   findPos = inHead.getURLPath()->find( '?' );
   if ( findPos != MC2String::npos ) {
      MC2String path( inHead.getURLPath()->substr( 0, findPos ) );
      inHead.setURLPath( new MC2String( path ) );

      MC2String::size_type pos = path.rfind( '/' );
      if ( pos != MC2String::npos ) {
         inHead.setPagename( new MC2String( path.substr( pos + 1) ) );
      } else {
         inHead.setPagename( new MC2String( "" ) );
      }
   }
   // path, default?. pagename
   if ( inHead.getPagename()->empty()  ){
      // Get and set default
      MC2String path( *inHead.getURLPath() );
      path.append( HttpParserThreadGroupHandle( 
                      static_cast<HttpParserThreadGroup*>( 
                         m_group.get() ) )->getDefaultPage() );
      inHead.setURLPath( new MC2String( path ) );

      MC2String::size_type pos = path.rfind( '/' );
      if ( pos != MC2String::npos ) {
         inHead.setPagename( new MC2String( path.substr( pos + 1 ) ) );
      } else {
         inHead.setPagename( new MC2String( "" ) );
      }
   }

   // Find file extension and set Content-Type if possible.
   const MC2String* pageName = inHead.getPagename();
   findPos = pageName->rfind( '.' ); // Last .   
   if ( findPos != MC2String::npos ) {
      MC2String ext = pageName->substr( findPos + 1 );
      MC2String lowerStr = StringUtility::copyLower( ext );
      MC2String fileType = HttpFileHandler::getFileType( lowerStr.c_str(),
                                                         NULL );
      outHead->addHeaderLine( ContentTypeStr, fileType );
   } else {
      outHead->addHeaderLine( ContentTypeStr, "" );
   }

   // Keep-Alive
   setKeepAlive( m_irequest, outHead );

   // Set HTTP version in outHead
   outHead->setHttpVersion( inHead.getMajorVersion(), 
                            inHead.getMinorVersion() );
   
   // Create Date:
   now = TimeUtility::getRealTime();
   makeDateStr( dateStr, now );
   setHttpHeaderDate( outHead, now );
  
   // Set this before to that handleHttpRequest can set it
   // to something else.
   setDefaultResponseLine( m_irequest, outHead );

   // Set userAgent
   if ( inHead.getHeaderValue( &USERAGENT ) != NULL ) {
      userAgent = *inHead.getHeaderValue( &USERAGENT );
   }

   // Set m_peerIP
   m_peerIP = m_irequest->getPeer().getIP();
   setServerAddress( m_irequest->getServerAddress() );



   // Call handleHttpRequest
   mc2dbg2 << "HttpParserThread::handleHttpRequest start "
           << inHead.getStartLine()->c_str() << endl;
   uint32 startProcessTime = TimeUtility::getCurrentMicroTime();

   if ( handleHttpRequest( &inHead, &inBody, &paramsMap,
                           outHead, outBody, now, dateStr ) )
   {
      uint32 bodyLength = outBody->getBodyLength();
      MC2String body( outBody->getBody(), bodyLength );

      // Check if change charset
      if ( !outBody->getBinary() ) {
         setBodyCharset( *outBody,
                         body,
                         &inHead, outHead, bodyLength );
      }
      
      // Check if encode content
      encodeBody( body, &inHead, outHead, outBody, bodyLength );

      // Set the length of the reply
      if ( bodyLength > 0) {
         char tmp[20];
         sprintf( tmp, "%d", bodyLength );
         tmpString = new MC2String( tmp );
      } else {
         tmpString = new MC2String( "0" );
      }
      outHead->addHeaderLine( &CONTENTLENGTH, tmpString);

      // Set the reply
      if ( !outBody->getBinary() ) {
         outBody->setBody( &body );
      } else {
         outBody->setBody( (const byte*)body.c_str(), body.size() );
      }

      irequest->setReply( outHead, outBody );

      // Add m_requestName to startline added to log
      MC2String startLine( StringUtility::trimStartEnd( 
                              *inHead.getStartLine() ) );
      if ( strchr( startLine.c_str(), '#' ) != NULL ) { 
         startLine.append( "," ); 
      } else {
         startLine.append( "#" );
      }
      startLine.append( m_requestName );

      // Get logname 
      HttpParserThreadGroupHandle( 
         static_cast<HttpParserThreadGroup*>( 
            m_group.get() ) )->httpLog( 
               NULL, irequest->getPeer().getIP(), now, 
               startLine.c_str(),
               outHead->getStartLineCode(), 
               outBody->getBodyLength(),
               userAgent.c_str(),
               "-",
               m_logname.empty() ? NULL : m_logname.c_str() );
      replySize = outBody->getBodyLength();
   } else {
      mc2log << info << "HttpParserThread failed to process "
             << m_requestName << " request "
             << inHead.getStartLine()->c_str() << endl;
      if ( irequest->getState() != InterfaceRequest::Ready_To_IO_Reply ) {
         // Not set then set it to 404
         irequest->setStatusReply( HttpCode::NOT_FOUND );
      }
      // Get logname 
      HttpParserThreadGroupHandle( 
         static_cast<HttpParserThreadGroup*>( 
            m_group.get() ) )->httpLog( 
               NULL, irequest->getPeer().getIP(), now,
               StringUtility::trimStartEnd(*inHead.getStartLine()).c_str(),
               HttpHeader::parseHTTPResponeLine( 
                  MC2String( 
                     (const char*) irequest->getReplyBuffer(),
                     MIN( 12, irequest->getReplySize() ) ).c_str() ),
               irequest->getReplySize(),
               userAgent.c_str(),
               "-",
               m_logname.empty() ? NULL : m_logname.c_str() );
      replySize = irequest->getReplySize();
      delete outHead;
      delete outBody;
   }
   uint32 stopProcessTime = TimeUtility::getCurrentMicroTime();

   uint32 requestTime = stopProcessTime - startProcessTime;
   requestTime /= 1000;


   // Calculate in size
   uint32 inbytes = 0;
   MC2String header;
   inHead.putHeader( &header );
   inbytes += header.size() + 2;
   inbytes += inBody.getBodyLength();

   // More like the one in NavigatorServer.
   mc2log << info << "handleRequest "
          << m_requestName << " "
          << StringUtility::trimStartEnd(*inHead.getStartLine())
          << " got " << inbytes << " bytes reply " 
          << irequest->getReplySize() << " bytes time in sys "
          << irequest->getTimeFromPutInQueueMs() << " ms time "
          << requestTime << " ms IO time " 
          << irequest->getTotalUsedIOTime() << " ms status "
          << irequest->getStatusReply() << endl;
   
//    if ( inHead.getMethod() == HttpHeader::POST_METHOD && 
//         paramsMap.size() > 0 ) 
//    {
//       printStringMap( (mc2log<< info), paramsMap, 
//                       "HttpParserThread POST parameters " );
//    }

   clearStringMap( &paramsMap );
   m_peerIP = 0;
   setServerAddress( IPnPort(0, 0) );
   setLogPrefix( "" );
}


bool
HttpParserThread::handleTileMapRequest( HttpHeader* inHead, 
                                        HttpBody* inBody,
                                        stringMap* paramsMap,
                                        HttpHeader* outHead, 
                                        HttpBody* outBody,
                                        uint32 now)
{

   m_requestName = "TMAP";
   UserItem* userItem = NULL;
   const ClientSetting* clientSetting = NULL;
   if ( !HttpMapFunctions::setUserFromParams( 
           paramsMap, this, m_functions->getVariableContainer(), 
           userItem,
           clientSetting ) ) 
   {
      return false;
   }
   auto_ptr< UserSwitch > userSwitch;
   if ( userItem != NULL ) {
      userSwitch.reset( new UserSwitch( *this, userItem, clientSetting ) );
//       if ( !myThread->getMapIdsForUserRegionAccess( 
//               userItem->getUser(), allowedMaps ) )
//       {
//          mc2log << info << "HttpTMapFunctions::handleTileMapsRequest "
//                 << "failed to get allowed mapIDs for user "
//                 << userItem->getUser()->getLogonID() << "(" 
//                 << userItem->getUIN() << ")" << endl;
//       }
   }

   logLastClient();

   // Extract the request string from the url-path
   const MC2String* specString = inHead->getPagename();
   mc2log << info << "handleTileMapRequest " << *specString << endl;
   
   DataBuffer* buf = getTileMap( specString->c_str() );
   
   if ( buf == NULL ) {
      // Not ok set 503 (Perhaps 503 if we had have an errorcode)
      m_irequest->setStatusReply( HttpCode::SERVICE_UNAVAILABLE );
      return false;
   } 

   // Set the body.
   outBody->setBody( buf->getBufferAddress(),
                     buf->getCurrentOffset() );
   delete buf;
   outBody->setBinary( true );

   MC2String ext = STLStringUtility::fileExtension( *specString, true );
   if ( ext == "svg" ) {
      outHead->addHeaderLine( CONTENT_TYPE, "image/svg+xml" );
   } else if ( ext == "png" ) {
      outHead->addHeaderLine( CONTENT_TYPE, "image/png" );
   } else {
      outHead->addHeaderLine( CONTENT_TYPE,
                              "application/wfmap" );
   }
   // Set headers to allow cache:ing.
   struct stat fStat;

   // Set last-mod date to the output of date +"%s" the 22 of dec 2003.
   fStat.st_atime = fStat.st_mtime = fStat.st_ctime = 1072094201;
   setCacheHeaders( outHead, "GET", false, true, now, NULL, &fStat);

   return true;
}


bool
HttpParserThread::handleTileMapsRequest( HttpHeader* inHead, 
                                         HttpBody* inBody,
                                         stringMap* paramsMap,
                                         HttpHeader* outHead, 
                                         HttpBody* outBody,
                                         uint32 now)
{
   m_requestName = "TMAPS";

   // POST /TMap
   // ...
   // Content-Length: ddd
   //
   // [startOffset 4 bytes][maxBytes 4 bytes][zero terminated specstrings]
   uint32 startOffset = 0;
   uint32 maxBytes = 0;
   vector<MC2SimpleString> tmapParams;
   const char* body = inBody->getBody();
   MC2String currParam;
   UserItem* userItem = NULL;
   const ClientSetting* clientSetting = NULL;
   if ( !HttpMapFunctions::setUserFromParams( 
           paramsMap, this, m_functions->getVariableContainer(), 
           userItem, clientSetting ) ) 
   {
      return false;
   }
   auto_ptr< UserSwitch > userSwitch;
   if ( userItem != NULL ) {
      userSwitch.reset( new UserSwitch( *this, userItem, clientSetting ) );
//       if ( !myThread->getMapIdsForUserRegionAccess( 
//               userItem->getUser(), allowedMaps ) )
//       {
//          mc2log << info << "HttpTMapFunctions::handleTileMapsRequest "
//                 << "failed to get allowed mapIDs for user "
//                 << userItem->getUser()->getLogonID() << "(" 
//                 << userItem->getUIN() << ")" << endl;
//       }
   }

   uint32 pos = 0;
   if ( inBody->getBodyLength() < (4 + 4 + 2) ) {
      mc2log << warn << "HPT::handleTileMapsRequest too short request "
             << inBody->getBodyLength() << "bytes" << endl;
      return false;
   }

   // startOffset
   startOffset = ntohl(*((uint32 *)&(body[pos])));
   pos += 4;
   // maxBytes
   maxBytes    = ntohl(*((uint32 *)&(body[pos])));
   pos += 4;

   mc2log << info << "handleTileMapsRequest: startOffset " << startOffset
          << " maxSize " << maxBytes;

   while ( pos < inBody->getBodyLength() ) {
      // Read params string
      if ( body[ pos ] == 0 ) {
         tmapParams.push_back( currParam.c_str() );
         mc2log << " " << currParam;
         currParam.clear();
      } else {
         currParam.append( 1, body[ pos ] );
      }
      pos++;
   }
   mc2log << endl;

   DataBuffer* buf = getTileMaps( tmapParams, startOffset, maxBytes );
   
   if ( buf == NULL ) {
      // Not ok set 500 (Perhaps 503 if we had have an errorcode)
      m_irequest->setStatusReply( HttpCode::INTERNAL_ERROR );
      return false;
   }


   // Set the body.
   outBody->setBody( buf->getBufferAddress(),
                     buf->getCurrentOffset() );
   delete buf;
   outBody->setBinary( true );

   MC2String contentType("Content-Type");
   outHead->addHeaderLine(&contentType,
                          new MC2String("application/wfmap") );
   // Set headers to allow cache:ing.
   struct stat fStat;

   // Set last-mod date to the output of date +"%s" the 22 of dec 2003.
   fStat.st_atime = fStat.st_mtime = fStat.st_ctime = 1072094201;
   setCacheHeaders( outHead, "POST", false, true, now, NULL, &fStat);
   
   return true;
}

bool
HttpParserThread::handleEchoRequest( HttpHeader* inHead, 
                                     HttpBody* inBody,
                                     stringMap* paramsMap,
                                     HttpHeader* outHead, 
                                     HttpBody* outBody,
                                     uint32 now )
{
   m_requestName = "ECHO";

   return copyEchoRequestToReply( inHead, inBody, outHead, outBody );
}

bool
HttpParserThread::handleXSMapRequest( HttpHeader* inHead, 
                                     HttpBody* inBody,
                                     stringMap* paramsMap,
                                     HttpHeader* outHead, 
                                     HttpBody* outBody,
                                     uint32 now )
{
   // Decode
   XSData::decodeXSBuffer( *inBody );
   // Handle
   // Treat it as text
   inBody->setBinary( false );
   bool res = handleTileMapsRequest( inHead, inBody, paramsMap,
                                     outHead, outBody, now );
   // Encode
   XSData::encodeXSBuffer( *outBody );
   // Set content type
   outHead->addHeaderLine( &CONTENT_TYPE, 
                           new MC2String( "application/binary" ) );
   // Remove Accept-Encoding to avoid encoding it again later
   inHead->deleteHeaderLine( &ACCEPT_ENCODING );

   return res;
}

bool
HttpParserThread::handleSpecialFiles( HttpHeader* inHead, 
                                      HttpBody* inBody,
                                      stringMap* paramsMap,
                                      HttpHeader* outHead, 
                                      HttpBody* outBody,
                                      uint32 now,
                                      bool& res )
{
   // Check for TileMap
   const MC2String* urlPath = inHead->getURLPath();
   
   if ( urlPath->find("/TMap/") == 0 ) {
      if ( inHead->getMethod() == HttpHeader::POST_METHOD ) {
         res = handleTileMapsRequest( inHead, inBody, paramsMap,
                                      outHead, outBody, now );
      } else {
         res = handleTileMapRequest( inHead, inBody, paramsMap,
                                     outHead, outBody, now );
      }
      return true; // handled the request, maybe set error reply
   } else if ( urlPath->find("/TMap") == 0 && 
               inHead->getMethod() == HttpHeader::POST_METHOD ) 
   {
      res = handleTileMapsRequest( inHead, inBody, paramsMap,
                                   outHead, outBody, now );
      return true; // handled the request, maybe set error reply
   } else if ( urlPath->find("/echo") == 0 ) {
      res = handleEchoRequest( inHead, inBody, paramsMap,
                               outHead, outBody, now );
      return true; // handled the request, maybe set error reply
   } else if ( urlPath->find("/XSMap") == 0 && 
               inHead->getMethod() == HttpHeader::POST_METHOD ) {
      res = handleXSMapRequest( inHead, inBody, paramsMap,
                                outHead, outBody, now );
      return true; // handled the request, maybe set error reply
   }
   // Not handled - return false.
   return false;
}


bool
HttpParserThread::handleHttpRequest( HttpHeader* inHead, 
                                     HttpBody* inBody,
                                     stringMap* paramsMap,
                                     HttpHeader* outHead, 
                                     HttpBody* outBody,
                                     uint32 now,
                                     const char* dateStr ) 
{
   // WARNING! This function may be recursively called by
   // HttpProxyFunctions
   MC2String* tmpString = NULL;
   struct stat fStat;
   MC2String ContentTypeStr = "Content-Type";
   int length = 0;

   // Parse parameters, if any, only if 
   // "application/x-www-form-urlencoded".
   // FIXME: handle "multipart/form-data"
   const MC2String* contentType = inHead->getHeaderValue( &ContentTypeStr );
   if ( inBody->getBodyLength() > 0 && ! inBody->getBinary() &&
        ( contentType == NULL ||
          StringUtility::strcasecmp( contentType->c_str(),
                                     "application/x-www-form-urlencoded" )
          == 0 ) )
   {
      MC2String params = inBody->getBody();
      parseParameters( params, paramsMap, inHead );
   }

   // Check for special files ( e.g. TileMap )
   bool specialFileHandledResult = true;
   const bool specialFileHandled = handleSpecialFiles( 
      inHead, inBody, paramsMap, outHead, outBody, now,
      specialFileHandledResult );
   if ( specialFileHandled ) {
      // Special file handled - return.
      return specialFileHandledResult;
   }
   // Get file
   const MC2String* constString = inHead->getURLPath();
   byte* file = HttpParserThreadGroupHandle( 
      static_cast<HttpParserThreadGroup*>( 
         m_group.get() ) )->getFile( constString, length, &fStat );
   if ( file == NULL ) {
      return false; // next request	  
   }
   bool ok = true;
   // Create reply
   if ( fStat.st_size > 13 && 
        (strncmp((char*)file, "<!--ISAB_TAGS", 13) == 0) ) 
   { 
      // 13 = strlen("<!--ISAB_TAGS");

      setCacheHeaders( outHead, inHead->getMethodString(), 
                       true, false,
                       now, dateStr, &fStat );

      // Find the tags and run the functions
      tmpString = new MC2String( (char*)file, length );
      // DEBUG8(cerr << "Begin parse page" << endl;);
      ok = parsePage( *tmpString, inHead, inBody, 
                      paramsMap, outHead, outBody );
      // DEBUG8(cerr << "Done parse page" << endl;);
      delete tmpString;
      tmpString = NULL;
   } else { // Send raw file   
      outBody->setBody( file, length );

      setCacheHeaders( outHead, inHead->getMethodString(), 
                       false, true, now, dateStr, &fStat );
   }
  
   delete [] file;

   return ok;
}

bool
HttpParserThread::copyEchoRequestToReply( HttpHeader* inHead, 
                                          HttpBody* inBody,
                                          HttpHeader* outHead, 
                                          HttpBody* outBody ) {
   // Transfer header fields to reply
   // The fields that are dangerus to transfer
   static const MC2String fields[] = { CONTENT_LENGTH, KEEP_ALIVE, CONNECTION,
                                       EXPECT, PROXY_CONNECTION };
   typedef set<MC2String> FieldSet;
   static const FieldSet fieldSet( fields, fields + NBR_ITEMS( fields ) );
   for ( HttpHeader::HeaderMap::const_iterator it = 
            inHead->getHeaderMap().begin() ;
         it != inHead->getHeaderMap().end(); ++it )
   {
      if ( fieldSet.find( it->first ) == fieldSet.end() ) {
         outHead->addHeaderLine( it->first, *it->second );
      }
   }
   // Transfer all X-WAYF-... possibly overwriting prevoius values.
   for ( HttpHeader::HeaderMap::const_iterator it = 
            inHead->getHeaderMap().begin() ;
         it != inHead->getHeaderMap().end(); ++it )
   {
      if ( it->first.find( "X-WAYF-" ) == 0 ) {
         outHead->addHeaderLine( it->first.substr( 7 ), *it->second );
      }
   }

   // Use inBody as outBody
   inBody->setBinary( true ); // Raw copy
   outBody->setBody( reinterpret_cast< const byte* >( inBody->getBody() ), 
                     inBody->getBodyLength() );
   outBody->setBinary( true );
   
   return true;
}

const char* 
HttpParserThread::getServerType() {
   return "HTTP";
}


void 
HttpParserThread::parseParameters( MC2String& params, stringMap* paramsMap, 
                                   HttpHeader* inHead ) 
{
   MC2String::size_type pos;
   MC2String::size_type findPos;
   MC2String line;
   MC2String param;
   MC2String* value;
   MC2String tmpString;
   bool convertCharset = false;
   const char* charSet = NULL;
   // Charset conversion
   // Check for Content-Type and charset
   const MC2String* contentType = NULL;
   if ( (contentType = inHead->getHeaderValue( "Content-Type" )) 
        != NULL || 
        (contentType = inHead->getHeaderValue( "Content-type" )) 
        != NULL ) 
   { 
      if ( contentType->find( "charset=" ) != MC2String::npos ) {
         uint32 pos = contentType->find( "charset=" ) + 8;
         charSet = contentType->c_str() + pos;
         convertCharset = true;
      } else {
         charSet = "iso-8859-1";
         convertCharset = true;
      }
         
   }

   if ( !params.empty() ) { // Got something
      DEBUG8(cerr << "parameters are: #" << params << "#" << endl;);
      pos = 0;
      while ( pos < params.size() ) {
         findPos = params.find_first_of("=&", pos);    
         if ( findPos == MC2String::npos ) break;
         if ( (params[findPos] == '=') && (findPos > pos) ) {
            param = params.substr(pos, (findPos - pos));
            pos = findPos + 1; // Skip '='
            findPos = params.find_first_of("&=\r\n", pos);
            if ( ( (findPos == MC2String::npos) ||
                   (params[findPos] == '&') || 
                   (params[findPos] == '\0') ||
                   (params[findPos] == '\r') ||
                   (params[findPos] == '\n') )
                 ) {
               tmpString = params.substr(pos, (findPos-pos));
               value = HttpParserThreadUtility::unURLEncode(tmpString);
               if ( convertCharset ) {
                  HttpParserThreadUtility::charsetToMC2( *value, charSet );
               }
	       
               // Add paramter and it's value
               DEBUG4(cerr << "HttpParserThread::parseParameters: " 
                      << param << "=#" << (*value) << "#" << endl;);
               stringMap::iterator pI = paramsMap->find(param);
               if (pI != paramsMap->end()) {
                  *pI->second += '\0' + *value;
                  delete value;
               } else
                  (*paramsMap)[ param ] = value;
            } else {
               DEBUG8(cerr << "HttpParserThread::parseParameters "
                      "odd parmetervalue: " 
                      << params.substr(pos, (findPos - pos + 1)) 
                      << " for param " << param 
                      << " in line #" << params << "#" << endl
                      << "Endchar pos = " << findPos 
                      << " char #" << params[findPos] << "#"  << endl;);
            }
         } else {
            DEBUG8(cerr << "HttpParserThread::parseParameters" 
                   << " odd parameter and value: "
                   << params.substr(pos, (findPos - pos + 1)) << endl;);
         }
         pos = findPos == MC2String::npos? findPos: findPos+1;
      } // / while ( pos < params.size() )
   } // / if ( !params.empty() )
}


bool
HttpParserThread::parsePage( const MC2String& page, 
                             HttpHeader* inHead, 
                             HttpBody* inBody,
                             stringMap* paramsMap,
                             HttpHeader* outHead, 
                             HttpBody* outBody ) 
{
   MC2String::size_type pos,
       findPos,
       tmpPos,
       length = 0;
   uint32 nbrParams = 0;
   MC2String command;
   MC2String param;
   char c;
   stringVector procParams;
   stringVector deleteableStrings;
   HttpFunctionNote* funcNote;
   bool ok = true;

   // Set HTTP or HTTPS in variablecontainer
   m_functions->getVariableContainer()->https = m_irequest->isHttps();

   length = page.size();
   // Skip the  <!--ISAB_TAGS ... -->
   findPos = page.find("<!--ISAB_TAGS");
   if ( findPos != MC2String::npos) {
      findPos =  page.find("-->", findPos);
      // Skip the "-->"
      pos = findPos == MC2String::npos? findPos : findPos + 3; 
      if ( (pos < length) && 
           ( (page[pos] == '\r') || (page[pos] == '\n') ) ) {
         pos++;
         // Skip any linefeed too if carrigereturn
         if ( (pos < length) && 
              (page[pos-1] == '\r') &&
              (page[pos] == '\n') ) // Skip CRLF
            pos++;
      }
   } else {
      pos = 0;
   }

   while (pos < length) { // Decode
      procParams.clear();
      nbrParams = 0;

      findPos = page.find("<ISAB_", pos);
      // Add the request data before <ISAB_
      outBody->addString(page.substr(pos, (findPos - pos)));
      if (findPos == MC2String::npos) { // No more functions 
         pos = findPos;
         break; 
      }
      pos =  findPos + 6; // Skip <ISAB_
      // Skip spaces
      while ( (pos < length) && (page[pos] == ' ') ) pos++;
      // Get functionname
      findPos = pos;
      while ( (findPos < length) &&
              (page[findPos] != '(') &&
              (page[findPos] != '>') ) {
         findPos++;
      }
      // Remove spaces before '(' or '>'
      tmpPos = findPos;
      while( (tmpPos < length) && (page[tmpPos-1] == ' ') ) tmpPos--; 
      command = page.substr(pos, (tmpPos - pos));
      pos = findPos; // On the '(' or '>'
      // Get parameters
      if ( (pos < length) && (page[pos] == '(') ) { // Start of parameters
         pos++;
         while ( (pos < length) &&
                 (page[pos] != ')') &&
                 (page[pos] != '>' ) ) 
         { // More parameters
            // Skip spaces
            while ( (pos < length) && (page[pos] == ' ') ) pos++;
            // Exctract parameter
            findPos = pos;
            if ( page[findPos] == '"' ) { // Handle strings
               findPos++; // Skip '"'
               while ( (findPos < length) && (page[findPos] != '"') )
                  findPos++;
               if ( (findPos < length) && (page[findPos] == '"') )
                  findPos++;
            } else {
               while ( (findPos < length) && (
                  (isalnum(page[findPos])) ||
                  (page[findPos] == '_') ||
                  (page[findPos] == '*') ||
                  (page[findPos] == '.')
                  ) )  // isalnum = A-z,a-z,0-9
                  findPos++;
            }
            param = page.substr(pos, (findPos - pos));
            pos = findPos;
            // Add paramter to parametervector
            if ( !param.empty() ) { // Add the parametervalue
               if ( (param.size() > 1) && // Can hold two '"'
                    (param[0] == '"') && 
                    (param[param.size() - 1] == '"') ) { // A string
                  // This string should be deleted at the end of function
                  deleteableStrings.push_back(
                     new MC2String( param.substr(1, 
                                              param.size() - 2)));
                  procParams.push_back(deleteableStrings.back());
                  nbrParams++;
               } else { // A parameter
                  stringMap::iterator fIt = paramsMap->find(param);
                  if ( fIt != paramsMap->end() ) {
                     procParams.push_back(fIt->second);
                     nbrParams++;
                  }
               }
            }
            // skip to ',' or end of parameters = ')' 
            // or end of function call '>'
            while ( (pos < length) && 
                    (c = page[pos]) && // Yes assign if pos < length
                    (c != ',') && 
                    (c != ')') &&
                    (c != '>') ) 
            {
               pos++;
            }
            // Skip any ','
            if ( (pos < length) && (page[pos] == ',') ) pos++;
         } // / while More parameters
      } // / if (page[pos] == '(') { // Start of parameters
      // Skip until end of ISAB-tag '>'
      while ( (pos < length) && (page[pos] != '>') ) pos++;
      pos ++; // Skip the '>'

      // Skip linebreak if directly after '>'
      if ( (pos < length) &&
           ( (page[pos] == '\r') || (page[pos] == '\n') ) ) {
         pos++;
         // Skip any linefeed too if carrigereturn
         if ( (pos < length) && 
              (page[pos-1] == '\r') &&
              (page[pos] == '\n') ) // Skip CRLF
            pos++;
      }       
      // Parse command
      funcNote = m_functions->getFunctionNoteOf( command );
      if ( funcNote != NULL ) { // Function Exists
         if ( funcNote->getMinArguments() <= nbrParams ) {
            DEBUG8(cerr << "About to call " << command << endl;);
            if ( !funcNote->getFunction()( &procParams, 
                                           nbrParams,
                                           paramsMap,
                                           inHead, 
                                           outHead, 
                                           inBody, 
                                           outBody,
                                           this,
                                           m_functions->
                                           getVariableContainer()) )
            {
               pos = MC2String::npos;
               ok = false;
            }
         }
      } else { // No such function
         MC2WARNING2( "HttpParserThread::parsePage unable to find command",
                      cerr << "   No function named " << command 
                      << " found" << endl;);
         //m_outBody->addString("No function named '" + command + 
         //                     "'");
      }
   } // / while (pos < length) { // Decode

   // Delete deleteableStrings
   clearStringVector(&deleteableStrings); 

   return ok;
}


void 
HttpParserThread::clearStringVector(stringVector* v) {
   MC2String* str;

   while ( !v->empty() ) {
      str = v->back();
      delete str;
      v->pop_back();
   }
}


void
HttpParserThread::clearStringMap( stringMap* params ) {
   for ( stringMap::iterator it = params->begin() ; 
         it != params->end();  ++it ) {
      delete it->second;
   }
   params->clear();
}


void
HttpParserThread::setBodyCharset( HttpBody& outBody,
                                  MC2String& body,
                                  HttpHeader* inHead,
                                  HttpHeader* outHead,
                                  uint32& bodyLength )
{
   DEBUG4(cerr << "HttpThread::setBodyCharset " << endl;);
   bool iso = false;
   bool utf8 = false;
   MC2String acceptStr = "Accept-Charset";
   
   // Check the inHead for Accept-Charset
   const MC2String accept = inHead->getAcceptCharset();
   if ( accept == "utf-8" ) {
      utf8 = true;
   } else {
      iso = true;
   }

   const char* charset = NULL;
   if ( outBody.charSetMayBeChanged() ) {
      if ( utf8 ) {
         charset = "utf-8";
         body = UTF8Util::mc2ToUtf8( body );
         bodyLength = body.size();
      } else { // ISO and default
         charset = "iso-8859-1";         
         body = UTF8Util::mc2ToIso( body );
         bodyLength = body.size();
      }
   } else {
      charset = outBody.getCharSet();
   }

   // Set charset in Content-Type
   // Content-Type: text/html; charset=ISO-8859-1
   MC2String ContentType = "Content-Type";
   MC2String* contentType = 
      const_cast< MC2String* > ( outHead->getHeaderValue( &ContentType ) );
   
   if ( contentType == NULL ) {
      // Add empty type
      outHead->addHeaderLine( &ContentType, new MC2String );
      contentType = 
         const_cast< MC2String* > ( outHead->getHeaderValue( &ContentType ) );
   }
   
   contentType->append( "; charset=" );
   contentType->append( charset );
}


void 
HttpParserThread::encodeBody( MC2String& body, 
                              HttpHeader* inHead,
                              HttpHeader* outHead,
                              HttpBody* outBody,
                              uint32& bodyLength )
{
   const MC2String AcceptEncoding = "Accept-Encoding";
   const MC2String ContentEncoding = "Content-Encoding";
   bool acceptGzip = false;
   bool acceptXGzip = false;
   bool acceptXWayf = false;
   bool acceptDeflate = false;

   // Check the inHead for Accept-Encoding
   const MC2String* accept = inHead->getHeaderValue( &AcceptEncoding );
   if ( accept != NULL ) {
      // Check all accept lines
      uint32 pos = 0;
      
      const char* matchStr = HttpHeader::getFirstStringIn( accept, pos );
      while ( matchStr != NULL ) {
         
         MC2String str = StringUtility::copyLower( matchStr );
         
         // Exclude some content types from compression
         bool excluded = false;

         const MC2String* contentType = outHead->getHeaderValue( 
            HttpHeaderLines::CONTENT_TYPE );
         if ( contentType != NULL && 
              (*contentType == HttpFileHandler::getFileType( "png", NULL ) ||
               *contentType == HttpFileHandler::getFileType( "gif", NULL )) ) {
            excluded = true;
         }

         if ( excluded ) {
            // No compression
         } else if ( strstr( str.c_str(), "gzip" ) != NULL ) {
            acceptGzip = true;
         } else if ( strstr( str.c_str(), "x-gzip" ) != NULL ) {
            acceptXGzip = true;
         } else if ( strstr( str.c_str(), "x-wayf" ) != NULL ) {
            acceptXWayf = true;
         } else if ( strstr( str.c_str(), "deflate" ) != NULL ) {
            acceptDeflate = true;
         }
         
         matchStr = HttpHeader::getNextStringIn( accept, pos );
      }
      
      if ( acceptGzip || acceptXGzip || acceptXWayf ) {
         // Encode with zlib (gzip)
         uint32 startTime = TimeUtility::getCurrentMicroTime();
         ulong compLen = body.size(); // Not larger than uncompressed
         int compPos = 0;
         byte compData[ compLen ];


         // Zlib starts
         int err;
         int pos = 0;
         bool ok = true;
         int level = Z_DEFAULT_COMPRESSION; //Z_BEST_SPEED
         int strategy = Z_DEFAULT_STRATEGY;
         z_stream s;
         uint32 crc = crc32( 0L, Z_NULL, 0);
         static int gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */
#define OS_CODE  0x03  /* Default Unix */
#define MEM_LEVEL 8

         s.next_in = s.next_out = Z_NULL;
         s.avail_in = s.avail_out = 0;

         s.msg = NULL;

         s.zalloc = (alloc_func)0;
         s.zfree = (free_func)0;
         s.opaque = (voidpf)0;


         // windowBits is passed < 0 to suppress zlib header
         // WARNING: Undocumented feature in zlib!
         // TODO: Use GzipUtil
         err = deflateInit2( &s, level, Z_DEFLATED, -MAX_WBITS, 
                             MEM_LEVEL, strategy );
         if ( err != Z_OK ) {
            mc2log << warn << "HttpParserThread::encodeBody "
                   << "zlib::deflateInit2 failed (" << err << ")" << endl;
            ok = false;
         }

         // Set indata
         s.next_in = (byte*)body.data();
         s.avail_in = body.size();

         // Make gzip header
         sprintf( (char*)compData, "%c%c%c%c%c%c%c%c%c%c", 
                  gz_magic[0], gz_magic[1],
                  Z_DEFLATED, 0 /*flags*/, 0,0,0,0 /*time*/, 0 /*xflags*/,
                  OS_CODE );
         compPos = 10L;
         
         // Set outbuff
         s.next_out = compData + compPos;
         s.avail_out = compLen - compPos;

         // Compress
         err = deflate( &(s), Z_FINISH ); // Finnish as in flush to outbuff
         if ( err != Z_OK && err != Z_STREAM_END ) {
            mc2log << warn << "HttpParserThread::encodeBody "
                   << "zlib::deflate failed (" << err << ")" << endl;
            ok = false;
         }

         // Update crc
         crc = crc32(crc, (const byte *)body.data(), body.size() );
         
         // Append gzip end (crc+uncompressed length)
         if ( s.avail_out >= 8 ) {
            *s.next_out++ = crc;
            *s.next_out++ = crc>>8;
            *s.next_out++ = crc>>16;
            *s.next_out++ = crc>>24;
            s.total_out += 4;
            int tot = s.total_in;
            *s.next_out++ = tot;
            *s.next_out++ = tot>>8;
            *s.next_out++ = tot>>16;
            *s.next_out++ = tot>>24;
            s.total_out += 4;
         } else {
            mc2dbg1 << "HttpParserThread::encodeBody No space for "
                    << "gzip end (crc+length)" << endl;
            ok = false;
            s.next_out = compData + compLen;
         }

         pos = s.total_in;
         compPos = (s.next_out - compData);

         // Clean zlib stream
         err = deflateEnd ( &s );
         if ( err != Z_OK ) {
            mc2log << warn << "HttpParserThread::encodeBody "
                   << "zlib::deflateEnd failed (" << err << ")" << endl;
         }

         uint32 stopTime = TimeUtility::getCurrentMicroTime();
         if ( ok && uint32(pos) == body.size() && compPos > 0 &&
              uint32(compPos) <= compLen  ) 
         {
            mc2dbg1 << "HttpParserThread::encodeBody gzip done " 
                    << body.size() << " compressed " << compPos 
                    << " in " << (stopTime - startTime) << "us" << endl;
            body.assign( (const char*)compData, compPos );
            bodyLength = body.length();
            MC2_ASSERT( uint32(compPos) == body.length() );

            // Set Content-Encoding 
            if ( acceptGzip ) {
               outHead->addHeaderLine( &ContentEncoding, 
                                       new MC2String( "gzip" ) );
            } else if ( acceptXGzip ) {
               outHead->addHeaderLine( &ContentEncoding, 
                                       new MC2String( "x-gzip" ) );
            } else { // acceptXWayf
               outHead->addHeaderLine( &ContentEncoding, 
                                       new MC2String( "x-wayf" ) );
            }
            outBody->setBinary( true ); // gziped now
         } else {
            if ( uint32(compPos) >= compLen ) {
               mc2dbg1 << "HttpParserThread::encodeBody gzip Compression"
                       << " ratio too high compression buffer full."
                       << " Compression time used " 
                       << (stopTime - startTime) << "us" << endl;
            } else {
               mc2log << warn << "HttpParserThread::encodeBody failed "
                      << "to gzip body, time used " 
                      << (stopTime - startTime) << "us, body:" << endl;
               Utility::hexDump( mc2log, (byte*)body.c_str(), 
                                 body.size() );
            }
         }
      } else if ( acceptDeflate ) {
         // Encode with zlib (deflate)
         uint32 startTime = TimeUtility::getCurrentMicroTime();
         ulong compLen = 12 + uint32( ceil( body.size()*1.01 ) );// Must be
         byte compData[ compLen ];

         if ( compress2( compData, &compLen, 
                         (const byte*)body.c_str(), body.size(),
                         Z_BEST_SPEED ) == Z_OK && compLen <= body.size() )
         {
            uint32 stopTime = TimeUtility::getCurrentMicroTime();
            mc2dbg1 << "HttpParserThread::encodeBody deflating done " 
                    << body.size() << " compressed " << compLen 
                    << " in " << (stopTime - startTime) << "us" << endl;
            body.assign( (const char*)compData, compLen );
            bodyLength = body.length();

            // Set Content-Encoding
            outHead->addHeaderLine( &ContentEncoding, 
                                    new MC2String( "deflate" ) );
            outBody->setBinary( true ); // deflated now
         } else {
            uint32 stopTime = TimeUtility::getCurrentMicroTime();
            if ( compLen > body.size() ) {
               mc2dbg1 << "HttpParserThread::encodeBody deflate "
                       << "Compression ratio too high compression "
                       << "buffer full. Compression time used " 
                       << (stopTime - startTime) << "us" << endl;
            } else {
               mc2log << warn << "HttpParserThread::encodeBody "
                      << "zlib::compress failed. time used "
                      << (stopTime - startTime) << "us, body:" << endl;
               Utility::hexDump( mc2log, (byte*)body.c_str(), 
                                 body.size() ); 
            }
         }
      } // Else nothing to do
   } // Rfc 2068 says that no Accept-Encoding means that all is accepted
    //  but we don't what to do that.
}


void
HttpParserThread::setCacheHeaders( HttpHeader* outHead, 
                                   const MC2String& command,
                                   bool generatedPage, bool cacheElement,
                                   uint32 date,
                                   const char* dateStr,
                                   struct stat* fStat )
{
   MC2String typeString( "" );
   struct tm *tmStruct;
   char cStr[1024];
   MC2String ContentType = "Content-Type";
   const MC2String* type = outHead->getHeaderValue( &ContentType );

   if ( generatedPage && 
        (type == NULL || strncmp( type->c_str(), "text/", 5 ) == 0 ) ) 
   {
      DEBUG8(cerr << "setCacheHeaders no cache" << endl;);
      // Generated text page
      if ( command.compare( "POST" ) != 0 ) {
         // Set expires date to date to avoid caching
         typeString = "Expires";
         outHead->addHeaderLine(&typeString, new MC2String(dateStr));
         // Set Last-modified to now
         typeString = "Last-modified";
         outHead->addHeaderLine(&typeString, new MC2String(dateStr));
         // Set Cache-Control to no-cache
         typeString = "Cache-Control";
         outHead->addHeaderLine( &typeString, 
                                 new MC2String( "no-cache" ) );
      }
   } else {
      DEBUG8(cerr << "setCacheHeaders cache" << endl;);
      // Set expires date to a year into the future to get page cached
      typeString = "Expires";
      time_t anotherTime = date;
      if ( cacheElement && !generatedPage ) {
         // = 365*24*60*60 = about a year
         anotherTime += 31536000;
      } else {
         anotherTime += 3600;
      }
      // Syncronized getTime
      //m_Server->stringFormatTime(cStr, now);
      struct tm result;
      tmStruct = gmtime_r(&anotherTime, &result );
      strftime(cStr, 30, "%a, %d %b %Y %H:%M:%S GMT", tmStruct);
      outHead->addHeaderLine(&typeString, new MC2String(cStr));
      // Set Last-modified
      typeString = "Last-modified";
      if ( generatedPage ) {
         anotherTime = date;
      } else {
         anotherTime = fStat->st_mtime;
      }
      
      if ( cacheElement ) { // Unknown exprire date
         cStr[ 0 ]  = '\0';
      } else {
         
         tmStruct = gmtime_r( &anotherTime, &result );
         strftime(cStr, 30, "%a, %d %b %Y %H:%M:%S GMT", 
                  tmStruct);
         outHead->addHeaderLine(&typeString, new MC2String(cStr));
      }
      if ( !cacheElement ) {
         // Set Cache-Control to no-cache
         typeString = "Cache-Control";
         outHead->addHeaderLine( &typeString, 
                                 new MC2String( "no-cache" ) );
      } else {
         // Set Cache-Control to public
         typeString = "Cache-Control";
         outHead->addHeaderLine( &typeString, 
                                 new MC2String( "public" ) );
      }
   }
}


void 
HttpParserThread::printStringMap( ostream& out, stringMap& strMap,
                                  const char* preLineString ) const 
{
   out << preLineString;
   for ( stringMap::iterator it = strMap.begin() ; 
         it != strMap.end() ; ++it )
   {
      if ( it != strMap.begin() ) out << ", ";
      out << it->first << "=" << *it->second;
   }
   out << endl;
}

void
HttpParserThread::logLastClient() {
   if ( m_user == NULL ) {
      return;
   }

   UserElement* el = 
      m_user->getUser()->getElementOfType( 0, 
                                           UserConstants::TYPE_LAST_CLIENT );

   if ( el != NULL ) {
      UserLastClient* last = static_cast< UserLastClient* >( el );
      
      mc2log << info << "Last client: "
             << "clientType \"" << last->getClientType()
             << "\" options \"" << last->getClientTypeOptions()
             << "\" V" << last->getVersion() << endl;
   } else {
      mc2log << info << "No information about last client available" << endl;
   }
}
