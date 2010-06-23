/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "HttpProxyFunctions.h"

#include "HttpParserThread.h"
#include "UserData.h"
#include "HttpBody.h"
#include "HttpHeader.h"
#include "Properties.h"
#include "URL.h"
#include "URLFetcherNoSSL.h"
#include "DataBuffer.h"
#include "STLStringUtility.h"
#include "HttpFunctionHandler.h"
#include "NetUtility.h"
#include "HttpHeaderLines.h"
#include "HttpInterfaceRequest.h"
#include "ParserThreadGroup.h"

#include "HttpCodes.h"

using namespace HttpHeaderLines;



namespace {
template<class ITERATOR>
MC2String transferParams( MC2String orig,
                          const stringMap& origParams,
                          const ITERATOR& beginAllowed,
                          const ITERATOR& endAllowed )
{
   // Start with question-mark. Change to & after first addition
   const char* sep = "?";
   // Keep the order of the allowed ones.
   for ( ITERATOR it = beginAllowed;
         it != endAllowed;
         ++it ) {
      MC2String arg( *it );
      stringMap::const_iterator find_it = origParams.find( arg );
      if ( find_it != origParams.end() ) {
         orig += sep;
         orig += arg;
         orig += "=";
         orig += *find_it->second;
         sep = "&";
      }
   }
   return orig;
}
}

bool HttpProxyFunctions::htmlProxy( stringVector* params,
                                    int paramc, 
                                    stringMap* paramsMap,
                                    HttpHeader* inHead, 
                                    HttpHeader* outHead,
                                    HttpBody* inBody,
                                    HttpBody* outBody,
                                    HttpParserThread* myThread,
                                    HttpVariableContainer* myVar )
{

   vector<MC2String> exploded = 
      STLStringUtility::explode( " ", 
                                 *inHead->getStartLine() );
   // exploded is now:
   // [0] = "GET", [1] = our pagename+options

   MC2String urlString = exploded[ 1 ];
   STLStringUtility::replaceString( urlString,
                                    *inHead->getPagename(), *(*params)[ 0 ] );

   size_t offset = urlString.find_first_not_of('/');
   if ( offset != MC2String::npos ) {
      urlString.erase( 0, offset );
   }

   // TEMPORARY!!! Remove when param transfer function is done

   // Works for mercator only - read which params to transfer
   // in the file on disk.
   if ( params->size() > 1 ) {
      
      MC2String::size_type queryStart =
         urlString.find_first_of( '?' );
      if ( queryStart != MC2String::npos ) {
         urlString.erase( queryStart );
      }
      // get allowed params from argument
      vector<MC2String> allowedParams = 
         STLStringUtility::explode( ",", *(*params)[ 1 ] );

      // compose new url string with allowed parameters
      urlString = ::transferParams( urlString, *paramsMap,
                                    allowedParams.begin(), 
                                    allowedParams.end() );
   }
   
   // END TEMPORARY!!
   

   MC2String httpStr("http");
   if ( myVar->https ) {
      httpStr += 's';
   }
   
   httpStr += "://";

   IPnPort address = myThread->getServerAddress();
   if ( address.getIP() == 0 ) {
      address.first = NetUtility::getLocalIP();
   }

   URL url( httpStr + 
            address.toString() +
            "/" +
            urlString );


   bool ok = fetchThruProxy(url, outHead, outBody, myThread, myVar);
   
   if ( ! ok ) {
      // try without proxy
      mc2dbg << "[HttpProxyFunc: Going to fetch without proxy: " 
             << url.getSpec() << endl;
         
      // this should not happen
      if ( *(*params)[ 0 ] == *inHead->getPagename() ) {
         mc2dbg << "[HttpProxyFunc]: param[ 0 ] = " << *(*params)[ 0 ] 
                << " is also the same as current inHeader->getPagename()." 
                << endl;
         return false;
      }
         
      inHead->replacePagename( *(*params)[ 0 ] );

      char dateStr[ 50 ];
      uint32 now = TimeUtility::getRealTime();
      HttpParserThread::makeDateStr( dateStr, now );

      return myThread->handleHttpRequest( inHead, inBody,
                                          paramsMap, 
                                          outHead, outBody,
                                          now,
                                          dateStr );
   }

   return ok;
}


bool
HttpProxyFunctions::fetchThruProxy( const URL& url,
                                    HttpHeader* outHead,                                   
                                    HttpBody* outBody,
                                    ParserThread* myThread,
                                    HttpVariableContainer* myVar) {
   URLFetcherNoSSL fetcher;
   // return value, true if fetch is successfull
   bool ok = false;

   const char* squid_prop = Properties::getProperty("INTERNAL_SQUID_URL");
   // no need to do proxy if we know about https
   if ( ! myVar->https && squid_prop != NULL ) {
      fetcher.setProxyAddress( squid_prop );
      const uint32 proxyTimeout = Properties::getUint32Property(
         "PROXY_TIMEOUT", 60000 );

      mc2dbg << "[HttpProxyFunc]: Going to fetch: "
             << fetcher.getProxyAddress() + url.getSpec() 
             << endl;

      HttpHeader fetHead;
      HttpHeader extraHeaders;
      extraHeaders.addHeaderLine( 
         X_WF_ID, myThread->getGroup()->getServerInstanceStr() );
      URLFetcherNoSSL::dbPair_t ret = fetcher.get( 
         fetHead, url, proxyTimeout, &extraHeaders );
      if ( (ret.first == HttpCode::OK ||
            ret.first == HttpCode::NOT_FOUND ||
            ret.first == HttpCode::SERVICE_UNAVAILABLE) &&
           ret.second != NULL ) {
         outBody->setBody( ret.second->getBufferAddress(), 
                           ret.second->getBufferSize() );
         const MC2String* hitOrMissTmp = fetHead.getHeaderValue( &X_CACHE );
         MC2String hitOrMiss = hitOrMissTmp ? *hitOrMissTmp : "";
         mc2dbg << "[HttpProxyFunc]: proxy fetch successful "
                << hitOrMiss << endl;
         ok = true;
         // Copy fetHead headers into outHead
         // ARRGH! squid supports only HTTP 1.0 not 1.1 as we do.
         outHead->setStartLine( ret.first ); 
         // Remove confusing proxy headers.
         fetHead.deleteHeaderLine( &PROXY_CONNECTION );
         fetHead.deleteHeaderLine( &CONNECTION );
         fetHead.deleteHeaderLine( &KEEP_ALIVE );
         fetHead.deleteHeaderLine( &X_CACHE );
         fetHead.deleteHeaderLine( &AGE );
         fetHead.deleteHeaderLine( &TRANSFER_ENCODING ); // chunked
         const HttpHeader::HeaderMap& h = fetHead.getHeaderMap();
         for ( HttpHeader::HeaderMap::const_iterator it = h.begin() ;
               it != h.end(); ++it )
         {
            outHead->addHeaderLine( it->first, *it->second );
         }
      } else {
         mc2dbg << "[HttpProxyFunc]: proxy fetch unsuccessfull: "
                << ret.first<< endl;
      }
      // might still be allocated
      delete ret.second;
   }


   return ok;
}
