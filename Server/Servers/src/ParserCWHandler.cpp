/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ParserCWHandler.h"
#include "Properties.h"
#include "STLStringUtility.h"
#include "URL.h"
#include "HttpHeader.h"
#include "HttpHeaderLines.h"
#include "URLFetcher.h"
#include "ParserThread.h"
#include "DataBuffer.h"
#include "ParserThreadGroup.h"
#include "ArrayTools.h"
#include "NetUtility.h"

#include "HttpFunctionHandler.h"
#include "HttpProxyFunctions.h"
#include "HttpBody.h"

ParserCWHandler::ParserCWHandler( 
   ParserThread* thread, ParserThreadGroup* group )
      : ParserHandler( thread, group )
{
   // TODO: Read repstrs from config.
   const char* cwHostStr = Properties::getProperty(
      "CW_HOST", "http://localhost/" );
   // Strip http[s] from property
   MC2String cwHost( cwHostStr );
   cwHost.erase( 0, cwHost.find( ':' ) );
   m_repurls.insert( 
      make_pair( MC2String( "://startpage/" ), CWReplace( cwHost ) ) );
   m_repurls.insert( 
      make_pair( MC2String( "://showinfo/" ),
                 CWReplace( cwHost, "show_info.[ISO639-1].php" ) ) );
   m_repurls.insert( 
      make_pair( MC2String( "://onlinehelp/" ),
                 CWReplace( cwHost, "symbianhelp/" ) ) );
   m_repurls.insert( 
      make_pair( MC2String( "://extendederror/" ),
                 CWReplace( cwHost, "extendederror.[ISO639-1].php" ) ) );
   m_repurls.insert(
      make_pair( MC2String( "://versionlock/" ),
                 CWReplace( cwHost, "versionlock.php" ) ) );
   m_repurls.insert(
      make_pair( MC2String( "://upgrade/" ),
                 CWReplace( cwHost, "startup/"
                            "upgrade_page_wayfinder.[ISO639-1].php" ) ) );
   m_repurls.insert(
      make_pair( MC2String( "://TMap" ),
                 CWReplace( "://internalInThisProcess/TMap" ) ) );
   m_repurls.insert( 
      make_pair( MC2String( "://firstpage/" ),
                 CWReplace( cwHost + "startup2/", 
                            "firstpage.[ISO639-1].php" ) ) );
   m_repurls.insert( 
      make_pair( MC2String( "://friendfinder/" ),
                 CWReplace( "://rocl-web/~vnicoara/mc2web/services/", 
                            "friend_details.[ISO639-1].php" ) ) );
   m_repurls.insert( 
      make_pair( MC2String( "://startup/" ),
                 CWReplace( cwHost + "startup3/", 
                            "firstpage.[ISO639-1].php" ) ) );
   m_repurls.insert( 
      make_pair( MC2String( "://show_msg/" ),
                 CWReplace( cwHost + "startup2/", 
                            "show_msg.[ISO639-1].php" ) ) );
   m_repurls.insert( 
      make_pair( MC2String( "://cache_img/" ),
                 CWReplace( "://sendThruProxyServer/" ) ) );
}

void
ParserCWHandler::updateRequest( 
   MC2String& urlStr, LangTypes::language_t clientLang ) const
{
   const char* instance = Properties::getProperty( "WF_INSTANCE", "eu" );
   for ( repmap::const_iterator it = m_repurls.begin() ; 
         it != m_repurls.end() ; ++it )
   {
      if ( urlStr.find( (*it).first ) != MC2String::npos ) {
         MC2String rstr( (*it).second.replaceWith );
         STLStringUtility::replaceString( rstr, "[INSTANCE]", instance );
         STLStringUtility::replaceString( 
            urlStr, (*it).first, rstr.c_str() );
         if ( (*it).second.defaultPage != "" ) {
            // Check if no page then replace with defaultPage
            URL url( urlStr );
            MC2String file( url.getFile() );
            if ( file.size() > 0 && file[ file.size() -1 ] == '/' ) {
               MC2String path( url.getPath() );
               MC2String::size_type qPos = path.find( '?' );
               if ( qPos == MC2String::npos ) {
                  qPos = path.find( '#' );
               }
               MC2String params;
               if ( qPos != MC2String::npos ) {
                  params = path.substr( qPos );
               }
               MC2String defaultPage( url.getFile() );
               defaultPage.append( (*it).second.defaultPage );
               defaultPage.append( params ); // ?u=...
               // TODO: Convert clientLang to languageCode and back
               //       to get only translated languages (There is a webpage).
               STLStringUtility::replaceString( 
                  defaultPage, "[ISO639-1]", 
                  LangTypes::getLanguageAsISO639( clientLang ) );
               URL newUrl( url.getProto(), url.getHost(), url.getPort(), 
                           defaultPage.c_str() );
               urlStr = newUrl.getSpec();
            }
         }
      }
   }
}


int
ParserCWHandler::getURL( const MC2String& urlStr, 
                         const MC2String& postData,
                         uint32 peerIP, uint32 fromByte, uint32 toByte,
                         LangTypes::language_t clientLang,
                         const HttpHeader* inHeaders,
                         HttpHeader& outHeaders, MC2String& reply,
                         uint32& startByte, uint32& endByte )
{
   uint32 startTime = TimeUtility::getCurrentTime();

   // Work with url
   MC2String serverURLStr( urlStr );
   mc2dbg2 << "serverURLStr " << serverURLStr << endl;
   updateRequest( serverURLStr, clientLang );

   bool fetchedThruProxy = false;
   if ( serverURLStr.find( "://sendThruProxyServer" ) != MC2String::npos ) {
      // This url should be fetched thru the proxy server
      
      // Remove 'sendThruProxyServer' from URL
      STLStringUtility::replaceString( serverURLStr, 
                                       "://sendThruProxyServer/",
                                       "://" );

      URL url(serverURLStr);
      HttpBody outBody;
      HttpVariableContainer myVar;
      myVar.https = ( serverURLStr.find( "https://" ) != MC2String::npos );
      
      // Fetch the url thru proxy server. If proxy fails it will be fetched without
      // proxy below.
      fetchedThruProxy =
         HttpProxyFunctions::fetchThruProxy( url, &outHeaders, &outBody, m_thread, &myVar );
      if ( fetchedThruProxy ) {
         // Successfully fetched thru proxy
         reply.append( outBody.getBody(), outBody.getBodyLength() );      
      } 
   }
   
   // Work with url
   mc2dbg2 << "serverURLStr " << serverURLStr << endl;
   updateRequest( serverURLStr, clientLang );
   URL url2( serverURLStr );
   int ures = 0;
   uint32 timeout = Properties::getUint32Property( "CW_TIMEOUT", 
                                                   5000 );
   
   if ( ! fetchedThruProxy ) {
      if ( serverURLStr.find( "internalInThisProcess" ) != MC2String::npos ) {
         // Handle internally
         reply.clear();
         if ( serverURLStr.find("/TMap/") != MC2String::npos ) {
            MC2String urlParam = STLStringUtility::basename( url2.getFile() );
            if ( !urlParam.empty() ) {
               DataBuffer* d = m_thread->getTileMap( urlParam.c_str() );
               if ( d != NULL ) {
                  ures = 200;
                  reply.append( reinterpret_cast<char*>( d->getBufferAddress() ),
                                d->getCurrentOffset() );
                  outHeaders.setStartLine( 200 );
               } else {
                  outHeaders.setStartLine( 503 );
               }
               delete d;
            }
         }
      }  else {
         // Send request using the parserthreads urlfetcher
         URLFetcher* f = m_thread->getURLFetcher();
         HttpHeader sendHeaders; // Extra headers to send
         // TODO: Add byterange using fromByte and toByte if not 0,MAX_UINT32
         //       so we don't download whole file all the time.
         MC2String peerIPstr = NetUtility::ip2str( peerIP );
         sendHeaders.addHeaderLine( "X-Forwarded-For", peerIPstr );
         
         static const MC2String notForwardHeadersStr[] = {
            "X-WAYF-CT",
            HttpHeaderLines::CONTENT_TYPE,
            HttpHeaderLines::CONTENT_LENGTH,
            HttpHeaderLines::CONNECTION,
            HttpHeaderLines::TRANSFER_ENCODING,
            HttpHeaderLines::X_FORWARDED_FOR,
            HttpHeaderLines::PROXY_CONNECTION,
            HttpHeaderLines::HOST,
            HttpHeaderLines::TE,
            HttpHeaderLines::TRAILER,
            HttpHeaderLines::KEEP_ALIVE,
            HttpHeaderLines::PROXY_AUTHENTICATE,
            HttpHeaderLines::PROXY_AUTHORIZATION,
            HttpHeaderLines::UPGRADE,
         };
         // TODO: Also remove all headers in Connection: header. Like
         //       "Connection: Keep-Alive, Trailer" should delete those two.
         static const set< MC2String, strNoCaseCompareLess > notForwardHeaders( 
            BEGIN_ARRAY( notForwardHeadersStr ), 
            END_ARRAY( notForwardHeadersStr ) );
         
         if ( inHeaders != NULL ) {
            const HttpHeader::HeaderMap& headers = inHeaders->getHeaderMap();
            for ( HttpHeader::HeaderMap::const_iterator it = headers.begin() ;
                  it != headers.end() ; ++it ) {
               if ( notForwardHeaders.find( it->first ) == 
                    notForwardHeaders.end() ) {
                  sendHeaders.addHeaderLine( it->first, *it->second );
               }
            }
         }
         
         if ( postData.empty() ) {
            ures = f->get(  reply, outHeaders, url2, 
                            timeout, &sendHeaders );
         } else {
            if ( inHeaders->getHeaderValue( "X-WAYF-CT" ) != NULL ) {
               sendHeaders.addHeaderLine( 
                  HttpHeaderLines::CONTENT_TYPE,
                  *inHeaders->getHeaderValue( "X-WAYF-CT" ) );
            } else {
               sendHeaders.addHeaderLine( 
                  HttpHeaderLines::CONTENT_TYPE, 
                  "application/x-www-form-urlencoded" );
            }
            ures = f->post( reply, outHeaders, url2, postData, 
                            timeout, &sendHeaders );
         }
         // Reset user agent
         //f->setDefaultUserAgent();
      }
   } // if (! fetchedThruProxy )
   
   // Remove chunked-encoding from reply (if present)
   const MC2String teh( "Transfer-Encoding" );
   const MC2String* te = outHeaders.getHeaderValue( &teh );
   if ( te != NULL && ( te->find( "chunked") != MC2String::npos ) ) {
      outHeaders.deleteHeaderLine( &teh );
   }
      
   // Check if web updated user
   const MC2String wfidh( "X-WFID-UPDATE" );
   const MC2String* wfid = outHeaders.getHeaderValue( &wfidh );
   if ( wfid != NULL ) {
      // Remove the uin from user cache
      uint32 uin = STLStringUtility::strtoul( *wfid );
      m_group->removeUserFromCache( uin );

      // And remove the header
      outHeaders.deleteHeaderLine( &wfidh );
   }

   // Make reply
   const MC2String eol( "\r\n" );
   if ( fromByte > toByte ) {
      toByte = fromByte;
   }
   const uint32 maxBytes = toByte - fromByte;
   uint32 lastByte = uint32( MAX( int32(reply.size()) - 1, 0 ) );
   startByte = MIN( fromByte, lastByte );
   endByte = startByte + MIN( lastByte - startByte, maxBytes );

   if ( ures > 0 && reply.size() > 0 && 
        (endByte != lastByte || startByte != 0) ) 
   {
      // Set byte range in reply
      MC2String rangeStr( "bytes " );
      STLStringUtility::uint2str( startByte, rangeStr );
      rangeStr.append( "-" );
      STLStringUtility::uint2str( endByte, rangeStr );
      rangeStr.append( "/" );
      STLStringUtility::uint2str( reply.size(), rangeStr ); // Real size
      outHeaders.addHeaderLine( "Content-Range", rangeStr );
      rangeStr = "";
      STLStringUtility::uint2str( endByte - startByte + 1, rangeStr );
      // Set right content length
      outHeaders.addHeaderLine( "Content-Length", rangeStr );
      // Set startline with 206 Partial Content
      MC2String responce( outHeaders.getStartLine()->substr( 0, 9 ) );
      responce.append( "206 Partial Content" );
      responce.append( eol );
      outHeaders.setStartLine( new MC2String( responce ) );
   } else if ( ures < 0 || outHeaders.getStartLine() == NULL ) {
      if ( TimeUtility::getCurrentTime() - startTime >= timeout ) {
         outHeaders.setStartLine( 503 );
      } else {
         outHeaders.setStartLine( 500 );
      }
   } else {
      // Set startline with eol
      outHeaders.setStartLine( 
         new MC2String( StringUtility::trimStartEnd( 
                           *outHeaders.getStartLine() ) + eol ) );
   }

   return ures;
}
