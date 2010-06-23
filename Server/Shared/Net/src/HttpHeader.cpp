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
#include <time.h>
#include <iostream>
#include "HttpHeader.h"
#include "StringUtility.h"
#include "STLStringUtility.h"
#include "TCPSocket.h"
#include "ScopedArray.h"

static const char* httpResponsCodes[] = {
   "100 Continue",
   "101 Switching Protocols",
   "200 OK",
   "201 Created",
   "202 Accepted",
   "203 Non-Authoritative Information",
   "204 No Content",
   "205 Reset Content",
   "206 Partial Content",
   "300 Multiple Choices",
   "301 Moved Permanently",
   "302 Found",
   "303 See Other",
   "304 Not Modified",
   "305 Use Proxy",
   "307 Temporary Redirect",
   "400 Bad Request",
   "401 Unauthorized",
   "402 Payment Required",
   "403 Forbidden",
   "404 Not Found",
   "405 Method Not Allowed",
   "406 Not Acceptable",
   "407 Proxy Authentication Required",
   "408 Request Time-out",
   "409 Conflict",
   "410 Gone",
   "411 Length Required",
   "412 Precondition Failed",
   "413 Request Entity Too Large",
   "414 Request-URI Too Large",
   "415 Unsupported Media Type",
   "416 Requested range not satisfiable",
   "417 Expectation Failed",
   "500 Internal Server Error",
   "501 Not Implemented",
   "502 Bad Gateway",
   "503 Service Unavailable",
   "504 Gateway Time-out",
   "505 HTTP Version not supported",
   NULL // This is the end marker. Do not remove it.
};


const char* bodyFormat = "<html>\n"
"<head>\n"
"<title>%s"
"</title>\n"
"</head>\n"
"\n"
"<body>\n"
"<h1>%s</h1>\n"
"</body>\n"
"</html>\n";


HttpHeader::httpResponsMap 
HttpHeader::initHttpResponsMap() {
   uint32 index = 0;
   char* endPtr = NULL;
   uint32 code = 0;
   httpResponsMap hmap;

   while ( httpResponsCodes[ index ] != NULL ) {
      // Get number
      code = strtoul( httpResponsCodes[ index ], &endPtr, 10 );
      if ( endPtr == NULL || endPtr[ 0 ] != ' ' ) {
         mc2log << fatal << "initHttpResponsMap HTTP respons " << index 
                << " is bad " << MC2CITE( httpResponsCodes[ index ] ) 
                << endl;
         exit ( 1 );
      }
      // Make header and body
      MC2String header( httpResponsCodes[ index ] );
      
      char body[ 4096 ];
      sprintf( body, bodyFormat, httpResponsCodes[ index ],
               httpResponsCodes[ index ] );

      pair < httpResponsMap::iterator, bool > res = hmap.insert( 
            make_pair( code, make_pair( header, MC2String( body ) ) ) );
      if ( !res.second ) {
         mc2log << fatal << "initHttpResponsMap HTTP respons " << index 
                << " is a dupe " << MC2CITE( httpResponsCodes[ index ] ) 
                << endl;
         exit ( 1 );
      }
      index++;
   }
   
   return hmap;
}


HttpHeader::httpResponsMap HttpHeader::httpResponses =initHttpResponsMap();


const HttpHeader::methodTable_t
HttpHeader::methodTable[] = {
   { "GET",   GET_METHOD },
   { "POST",  POST_METHOD },
   { "HEAD",  HEAD_METHOD },
   { "OPTIONS", OPTIONS_METHOD },
   { "PUT",     PUT_METHOD },
   { "DELETE",  DELETE_METHOD },
   { "TRACE",   TRACE_METHOD },
   { "CONNECT", CONNECT_METHOD },
   { "",      NO_MATCH } // Vaktpost
};

HttpHeader::HttpHeader(MC2String* Header) {
   m_header = new MC2String(*Header);
   m_method = NO_MATCH;
   parseHeader(m_header);
   m_contentLength = 0;
   m_URLPath = NULL;
   m_pagename = NULL;
   m_startLine = NULL;
   m_majorVersion = 1;
   m_minorVersion = 0;
}


HttpHeader::HttpHeader() {
   m_header = NULL;
   m_contentLength = 0;
   m_URLPath = NULL;
   m_pagename = NULL;
   m_startLine = NULL;
   m_method = NO_MATCH;
   m_majorVersion = 1;
   m_minorVersion = 0;
}


HttpHeader::~HttpHeader() {
   clear();
}

void
HttpHeader::resetCached()
{
   m_acceptCharset = "";
}

ostream& operator<<( ostream& ostr, const HttpHeader& header )
{
   if ( header.m_startLine != NULL ) {
      ostr << "1 " << *header.m_startLine << endl;
      int nbr = 1;
      for( HttpHeader::HeaderMap::const_iterator it =
              header.m_Headers.begin();
           it != header.m_Headers.end();
           ++it ) {
         ostr << ++nbr << " " << it->first << ": "
              << *it->second << endl;
      }
   }
   return ostr;
}

const MC2String*
HttpHeader::getHeaderValue(const MC2String* HeaderType) const {
   HeaderMap::const_iterator headi = m_Headers.find(*HeaderType);
   if ( headi == m_Headers.end() ) {
      return NULL;
   } else {
      return (*headi).second; 
   }
}


const MC2String* 
HttpHeader::getHeaderValue(const char* HeaderType) const {
   MC2String* type = new MC2String( HeaderType );
   const MC2String* res = getHeaderValue( type );
   delete type;
   return res;
}

time_t
HttpHeader::getHeaderDateValue( const MC2String& headerName ) const
{
   const MC2String* tmpVal = getHeaderValue( headerName );
   if ( tmpVal == NULL ) {
      return 0;
   }
   struct tm tm; 
   if ( strptime( tmpVal->c_str(),
                  "%a, %d %b %Y %H:%M:%S GMT", &tm ) == NULL ) {
      return 0;
   }
   return timegm( &tm );
}

void
HttpHeader::addHeaderDateValue( const MC2String& headerName,
                                time_t date )
{
   char dateStr[1024];
   time_t t = date;
   struct tm result;
   struct tm* tmStruct = gmtime_r( &t, &result );
   tmStruct->tm_isdst = 1;
   strftime( dateStr, 30, "%a, %d %b %Y %H:%M:%S GMT", tmStruct );
   addHeaderLine( headerName, dateStr );
}

const MC2String&
HttpHeader::getAcceptCharset() const
{
   if ( m_acceptCharset != "" ) {
      return m_acceptCharset;
   }
   MC2String acceptStr = "Accept-Charset";

   const char* charset = "iso-8859-1";
   
   // Check for Accept-Charset
   const MC2String* accept = getHeaderValue( &acceptStr );
   if ( accept != NULL ) {
      // Check all accept lines
      uint32 pos = 0;
      
      const char* matchStr = HttpHeader::getFirstStringIn( accept, pos );
      while ( matchStr != NULL ) {
         MC2String str = StringUtility::copyLower( matchStr );

         if ( strstr( str.c_str(), "utf-8" ) != NULL ) {
            charset = "utf-8";
         } else if ( strstr( str.c_str(), "iso-8859-1" ) != NULL ) {
            charset = "iso-8859-1";
         } else if ( strstr( str.c_str(), "ascii" ) != NULL ) {
            charset = "us-ascii";
         } else if ( strstr( str.c_str(), "ansi_x3.4-1968" ) != NULL ) {
            charset = "us-ascii";
         }
         matchStr = HttpHeader::getNextStringIn( accept, pos );
      }
   }
   const_cast<HttpHeader*>(this)->m_acceptCharset = charset;
   return m_acceptCharset;   
}

uint32
HttpHeader::getContentLength() const {
   return m_contentLength;
}

void
HttpHeader::setContentLength(uint32 Length) {
   m_contentLength = Length;
}

void
HttpHeader::addHeaderLine( const MC2String* headerType, 
                           MC2String* headerValue,
                           bool appendMultipleHeaders )
{
   resetCached();
   mc2dbg4 << "Adding " << *headerType << " : " << *headerValue << endl;
   HeaderMap::iterator headI = m_Headers.find( *headerType );
   if ( headI != m_Headers.end() ) {
      if ( appendMultipleHeaders ) {
         const char* sep = ",";
         (*headI).second->append( sep );
         (*headI).second->append( *headerValue );
         delete headerValue;
      } else {
         delete (*headI).second;
         headI->second = headerValue;
      }
   } else {
      m_Headers[ *headerType ] = headerValue;
   }
}

void
HttpHeader::addHeaderLine( const MC2String& headerType,
                           const MC2String& headerValue,
                           bool appendMultipleHeaders ) {
   addHeaderLine( &headerType, new MC2String( headerValue ), 
                  appendMultipleHeaders );
}

void
HttpHeader::addHeaderLine( const char* headerType,
                           const char* headerValue,
                           bool appendMultipleHeaders ) {
   MC2String type( headerType );
   addHeaderLine( &type, new MC2String( headerValue ), 
                  appendMultipleHeaders );
}

bool
HttpHeader::addHeaderLine( const char* inLine, bool appendMultipleHeaders ) {
   resetCached();
   MC2String line(inLine);
   uint32 valuePos = line.find(':');
   if ( valuePos < line.size() ) { // OK
      bool ok = true;
      MC2String typeString(line.substr(0, valuePos));
      valuePos++;
      while( line[valuePos] == ' ' ) { // Skip spaces
         valuePos++;
      }
      MC2String valueString(line.substr(valuePos,
                              (line.size() - valuePos)));
      
      // Set contentlength if it is "content-length"
      if ( StringUtility::strcasecmp( typeString.c_str(), 
                                      "content-length") == 0 ) 
      {
         char* endptr = NULL;
         int contentLength = strtol( valueString.c_str(), &endptr, 10 );
         if ( endptr != valueString.c_str() ) {
            setContentLength( contentLength );
         } else {
            mc2dbg2 << "HttpHeader::addHeaderLine - Content-Length not "
                    << "a number " << MC2CITE( valueString ) << endl;
            ok = false;
         }
      }
      // Add the line with the old method.
      addHeaderLine( &typeString, new MC2String( valueString ), 
                     appendMultipleHeaders );
      return ok;
   } else {
      return false;
   }
}

bool
HttpHeader::addRequestLine(const char* inLine) {
   resetCached();
   // Maybe its that line with GET in it?
   int reqLen = strlen(inLine);
   ScopedArray<char> reqLine( new char[reqLen + 1] );
   char* reqEnd = reqLine.get() + reqLen;
   strcpy( reqLine.get(), inLine );
   char* spacePos = strchr( reqLine.get(), ' ');
   if ( spacePos == NULL ) {
      // Malformed request. There should be a method followed
      // by a space and a URI.
      mc2dbg2 << "HttpHeader::addHeaderLine - no space on line"
              << endl;
      return false;
   }
   char* uriPos = spacePos+1;
   if ( uriPos >= reqEnd ) {
      // No room for URI
      return false;
   }
   *spacePos = '\0'; // Terminate reqLine so that the method is there.
   m_method = findMethod( reqLine.get() );
   if ( m_method == NO_MATCH ) {
      // Unknown method
      mc2dbg8 << "HttpHeader::addHeaderLine - unknown method" << endl;
      return false;
   }
   // Now get the URI
   spacePos = strchr( uriPos, ' ');
   char* http_pos = NULL;
   // If there is a httpversion use it later.
   if ( spacePos != NULL ) {
      http_pos = spacePos + 1;
      // Zero terminate the URI.
      *spacePos = '\0';
   }
   MC2String* path = new MC2String(uriPos);
   MC2String* page = new MC2String(path->substr(path->rfind('/') + 1 ) );
   setURLPath(path);
   setPagename(page);
   
   if ( http_pos != NULL ) {
      bool ok = false;
      char* currPos = http_pos;
      if ( StringUtility::strncasecmp( currPos, "HTTP/", 5 ) == 0 ) {
         // Starts with HTTP/
         currPos = currPos + 5;
         char* endptr = NULL;
         uint32 tmp = strtol( currPos, &endptr, 10 ); /* Base 10 */
         if ( endptr != currPos ) { /* Is a number */
            m_majorVersion = tmp;
            currPos = endptr; /* Skip the majorVersion */
            if ( *currPos == '.' ) { /* majorVersion. */
               currPos++;
               tmp = strtol( currPos, &endptr, 10 ); /* Base 10 */
               if ( endptr != currPos ) { /* Is a number */
                  m_minorVersion = tmp;
                  if ( StringUtility::trimStart( endptr )[ 0 ] == '\0' ) {
                     // Nothing after HTTP-Version
                     ok = true;
                  } else {
                     // Rubbish after HTTP-Version
                  }
               }
            }
         }
      } else {
         if ( StringUtility::trimStart( http_pos )[ 0 ] == '\0' ) {
            // No version at all
            ok = true;
         } else {
            // Rubbish after URI
         }
      }
      if ( !ok ) {
         mc2dbg8 << "HttpHeader::addHeaderLine - bad HTTP-Version" << endl;
         return false;
      }
   } else {
      m_majorVersion = 1;
      m_minorVersion = 0;
   }

   // This function does not handle parameters.
   // If you add POST parameters - be careful. Navigatorserver
   // must have the data of the post field exactly as sent.
   
   mc2dbg2 << "Method is " << methodTable[ m_method ].methodString << endl
           << "URI is " << *path << endl << "pagename is " << *page << endl; 
   setStartLine(new MC2String(inLine));

   return true;
}

HttpHeader::MethodType
HttpHeader::findMethod(const char* method) const {
   int i = 0;
   do {
      if(StringUtility::strcasecmp(methodTable[i].methodString, method ) == 0 )
         return methodTable[i].method;
   } while ( methodTable[i++].method != NO_MATCH );
   return NO_MATCH;
}

HttpHeader::MethodType
HttpHeader::getMethod() const
{
   return m_method;
}


void
HttpHeader::setMethod( MethodType method ) {
   m_method = method;
}


const char* 
HttpHeader::getMethodString() const 
{
   return methodTable[ m_method ].methodString;
}


uint32 
HttpHeader::getMajorVersion() const {
   return m_majorVersion;
}


uint32 
HttpHeader::getMinorVersion() const {
   return m_minorVersion;
}


uint32
HttpHeader::nbrStrings( const MC2String* str ) {
   uint32 nbr = 0;
   uint32 pos = 0;
   const char* tmp = NULL;
   
   tmp = getFirstStringIn( str, pos );
   while ( tmp != NULL ) {
      nbr++;
      tmp = getNextStringIn( str, pos );
   }
   return nbr;
}


const char* 
HttpHeader::getFirstStringIn( const MC2String* str, uint32& pos ) {
   pos = 0;
   return str->c_str();
}


const char*
HttpHeader::getNextStringIn( const MC2String* str, uint32& pos ) {
   uint32 findPos = str->find( '\0', pos ); ;
   if ( findPos < str->size() ) {
      pos = findPos + 1;
      return ( str->c_str() + pos );
   } else {
      return NULL;
   }
}


const char*
HttpHeader::getStringIn( uint32 index, const MC2String* str ) {
   uint32 pos;
   uint32 i = 0;
   const char* tmp;

   tmp = getFirstStringIn( str, pos );
   while ( i < index && tmp != NULL ) {
      tmp = getNextStringIn( str, pos );
      i++;
   }

   return tmp;
}


bool
HttpHeader::deleteHeaderLine( const MC2String* HeaderType ) {
   HeaderMap::iterator headIt = m_Headers.find( *HeaderType );
   if ( headIt == m_Headers.end() ) {
      return false;
   } else {
      delete headIt->second;
      m_Headers.erase(headIt); 
      return true;
   }  
}


void
HttpHeader::setStartLine(MC2String* Line)
{
   delete m_startLine;
   m_startLine = Line;
   // Seems like the startline should contain the CR+LF
   if ( m_startLine &&
        m_startLine->rfind("\r\n") != m_startLine->length() - 2 ) {
      mc2dbg8 << "[HttpHeader]: Adding \"\\r\\n\"" << endl;
      m_startLine->append( "\r\n" );
   }
}


void
HttpHeader::setStartLine( uint32 code ) {
   MC2String* line = new MC2String( getCurrHTTPVer() );
   line->append( getHttpStartLine( code ) );
   setStartLine( line );
}

const char* 
HttpHeader::getCurrHTTPVer() const {
   const char* httpVer = "HTTP/1.0 ";
   if ( getMajorVersion() == 1 && getMinorVersion() == 1 ) {
      httpVer = "HTTP/1.1 ";
   }

   return httpVer;
}


const MC2String*
HttpHeader::getStartLine() const {
   return m_startLine;
}


uint32 
HttpHeader::getStartLineCode() const {
   uint32 code = 0;

   if ( m_startLine != NULL ) {
      code = parseHTTPResponeLine( m_startLine->c_str() );
   }

   return code;
}


void
HttpHeader::writeHeader(TCPSocket* sock) const {
   if (m_startLine != NULL) 
      sock->writeAll( (byte*)m_startLine, m_startLine->size() );
   for ( HeaderMap::const_iterator it = m_Headers.begin() ; 
         it != m_Headers.end(); ++it ) {
      // it->first + ": " + (*it->second) + "\r\n";
      sock->writeAll( (byte*)it->first.c_str(), it->first.size() );
      sock->writeAll( (byte*)": ", 2);
      sock->writeAll( (byte*)(*it->second).c_str(), it->second->size() );
      sock->writeAll( (byte*)"\r\n", 2 );
   }
   sock->writeAll( (byte*)"\r\n", 2 );
}


void
HttpHeader::putHeader(MC2String* headerString) const {
   if (m_startLine != NULL)
      headerString->append(*m_startLine);
   for ( HeaderMap::const_iterator it = m_Headers.begin() ; 
         it != m_Headers.end() ; ++it ) {
      headerString->append((*it).first + ": " + (*(*it).second) + "\r\n");
   } 
}


void
HttpHeader::replacePagename( const MC2String& newPageName ) {
   MC2String *startLine = new MC2String( *getStartLine() );
   MC2String *urlPath = new MC2String( *getURLPath() );
   MC2String* newPagename = new MC2String( newPageName );

   STLStringUtility util;
   util.replaceString( *startLine,
                       *getPagename(),
                       *newPagename );
   util.replaceString( *urlPath,
                       *getPagename(),
                       *newPagename );
   
   setStartLine( startLine );
   setPagename( newPagename );
   setURLPath( urlPath );
}

void
HttpHeader::clear() {
   mc2dbg8 << "HttpHeader::clear()" << endl;
   delete m_header;
   m_header = NULL;
   m_contentLength = 0;
   delete m_startLine;
   m_startLine = NULL;
   delete m_URLPath;
   m_URLPath = NULL;
   delete m_pagename;
   m_pagename = NULL;
   m_method = NO_MATCH;
   m_majorVersion = 1;
   m_minorVersion = 0;
   for ( HeaderMap::iterator it = m_Headers.begin() ; 
         it != m_Headers.end(); ++it ) {
      //    mc2dbg8 << "HttpHeader::clear removing " 
      //      << (*it).first << " value: " << (*(*it).second) << endl;
      delete (*it).second;
   }
   m_Headers.clear();
}


void
HttpHeader::setURLPath(MC2String* newURLPath) {
   delete m_URLPath;
   m_URLPath = newURLPath;
}


const MC2String*
HttpHeader::getURLPath() const {
   return m_URLPath;
}


void
HttpHeader::setPagename(MC2String* newPagename) {
   delete m_pagename;
   m_pagename = newPagename;
}


const MC2String*
HttpHeader::getPagename() const {
   return m_pagename;
}


void
HttpHeader::parseHeader(MC2String* Header) {
   if ( Header != NULL ) {
      uint32 pos = 0;
      uint32 findPos = Header->find_first_of("\n\r");
      uint32 valuePos;
      MC2String Line;
      MC2String TypeString;
      MC2String ValueString;
      while ( pos < Header->size() ) {
         Line = Header->substr(pos, (findPos - pos) );
         valuePos = Line.find(':');
         if ( valuePos < Line.size() ) { // Ok
            TypeString = Line.substr(0, valuePos); 
            valuePos++; // Skipp ':'
            while ( Line[valuePos] == ' ' ) { // Skipp space
               valuePos++; 
            }
            ValueString = Line.substr(valuePos, (Line.size() - valuePos));
            m_Headers[TypeString] = new MC2String(ValueString);
         } else {
            mc2dbg4 << "parseHeader Odd headerline without :" << endl;
         }
         if ( (*Header)[findPos +1] == '\n' ) {
            findPos++;
         }
         pos = ++findPos; // Skip the stop-character
         findPos = Header->find_first_of("\n\r", pos);
      }
   }
}


const MC2String&
HttpHeader::getHttpStartLine( uint32 code ) {
   httpResponsMap::const_iterator resp = httpResponses.find( code );
   if ( resp != httpResponses.end() ) {
      return resp->second.first;
   } else {
      return httpResponses[ 500 ].first;
   }
}

bool
HttpHeader::getHttpErrorPage( uint32 code, const MC2String*& header,
                              const MC2String*& body ) 
{
   httpResponsMap::const_iterator resp = httpResponses.find( code );
   if ( resp != httpResponses.end() ) {
       header = &resp->second.first;
       body   = &resp->second.second;
      return true;
   } else {
      header = &httpResponses[ 500 ].first;
      body   = &httpResponses[ 500 ].second;
      return false;
   }
}



uint32 
HttpHeader::parseHTTPResponeLine( const char* data ) {
   uint32 code = 0;
   int tmp = 0;
   char* endptr = 0;
   const char* currPos = data;
   int length = strlen( data );

   if ( length >= 12 ) { /* Enough to hold HTTP/1.X NBR */
      if ( strncmp(currPos, "HTTP/", 5) == 0 ) { /* Starts with HTTP/ */
         currPos = currPos + 5;
         tmp = strtol(currPos, &endptr, 10); /* Base 10 */
         if ( endptr != currPos ) { /* Is a number */
            if ( tmp == 1 ) { /* HTTP Version 1 */
               currPos++; /* Skip the 1 */
               if ( *currPos == '.' ) { /* 1. */
                  currPos++;
                  tmp = strtol(currPos, &endptr, 10); /* Base 10 */
                  if ( endptr != currPos ) { /* Is a number */
                     if ( (tmp == 1) || (tmp == 0) ) { /* Ver 1.0 || 1.1 */
                        currPos++; /* Skip 0 or 1 */
                        if ( *currPos == ' ' ) {
                           currPos++;
                           tmp = strtol(currPos, &endptr, 10);
                           if ( endptr != currPos ) { /* Is a number */
                              code = tmp;
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }

   return code;
}
