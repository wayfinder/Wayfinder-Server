/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef URL_FETCHERNOSSL_H
#define URL_FETCHERNOSSL_H

#include "config.h"

#include <utility>
#include <memory>

#include "MC2String.h"
#include "NotCopyable.h"

class DataBuffer;
class HttpHeader;
class TCPSocket;
class URL;

/**
 *   URL-fetcher that supports http, but not https.
 *   Use this if you don't want to link with ssl.
 */
class URLFetcherNoSSL: private NotCopyable {
public:

   /**
    *   Inits the URLFetcherNoSSL.
    */
   URLFetcherNoSSL();
   
   /**
    *   Deletes the socket.
    */
   virtual ~URLFetcherNoSSL();
   
   /**
    *   Connects to the server and gets the result into
    *   the string result.
    *   @param result Where to put the data.
    *   @return status code from http. Negative numbers for other errors.
    */
   int get( MC2String& result,
            const URL& url,
            uint32 timeout_ms = MAX_UINT32,
            const HttpHeader* inHeaders = NULL );

   /// Pair to return in functions returning DataBuffers.
   typedef pair<int, DataBuffer*> dbPair_t;
   
   /**
    *   Connects to the server and gets the result into
    *   the string result.
    *   @return status code from http and a DataBuffer which
    *           isn't NULL, but may be empty.
    */
   dbPair_t get( const URL& url,
                 uint32 timeout_ms = MAX_UINT32,
                 const HttpHeader* inHeaders = NULL);
   /**
    *   Connects to the server and gets the result into
    *   the string result.
    *   @return status code from http and a DataBuffer which
    *           isn't NULL, but may be empty.
    */
   dbPair_t get( HttpHeader& headers,
                 const URL& url,
                 uint32 timeout_ms = MAX_UINT32,
                 const HttpHeader* inHeaders = NULL );
   
   /**
    *   Connects to the server and gets the result into
    *   the string result.
    *   @param result  Where to put the data.
    *   @param headers Place to store the headers.
    *   @param url     The url.
    *   @return status code from http. Negative numbers for other errors.
    */
   int get( MC2String& result,
            HttpHeader& headers,
            const URL& url,
            uint32 timeout_ms = MAX_UINT32,
            const HttpHeader* inHeaders = NULL );

   /**
    *   Connects to the server and posts postdata.
    *   @param result   Result is put here.
    *   @param url      Url to post to the server.
    *   @param postData Data to post.
    *   @param timeout_ms Timeout in milli seconds, default no timeout.
    *   @param inHeaders Extra headers to send, default none.
    */
   int post( MC2String& result,
             const URL& url,
             const MC2String& postData,
             uint32 timeout_ms = MAX_UINT32,
             const HttpHeader* inHeaders = NULL );


   /**
    *   Connects to the server and posts postdata.
    *   @param result   Result is put here.
    *   @param headers  Place to store the headers.
    *   @param url      Url to post to the server.
    *   @param postData Data to post.
    *   @param timeout_ms Timeout in milli seconds, default no timeout.
    *   @param inHeaders Extra headers to send, default none.
    */
   int post( MC2String& result,
             HttpHeader& headers,
             const URL& url,
             const MC2String& postData,
             uint32 timeout_ms = MAX_UINT32,
             const HttpHeader* inHeaders = NULL );

   /**
    *   Connects to the server and posts postdata.
    *   This method will log extra information when used.
    *   @param result   Result is put here.
    *   @param url      Url to post to the server.
    *   @param postData Data to post.
    *   @param timeout_ms Timeout in milli seconds, default no timeout.
    *   @param inHeaders Extra headers to send, default none.
    *   @param callingFunction Extra logging information.
    */
   int postWithLogging( MC2String& result,
             const URL& url,
             const MC2String& postData,
             uint32 timeout_ms = MAX_UINT32,
             const HttpHeader* inHeaders = NULL,
             const MC2String& callingFunction = MC2String() );

   /**
    *   Returns the User-Agent: string used.
    */
   const MC2String& getUserAgent() const;

   /**
    * @return proxy address
    */
   const MC2String& getProxyAddress() const;

   /**
    *   Sets the user agent of the fetcher.
    */
   void setUserAgent( const MC2String& userAgent );

   /**
    * Resets the user agent to default.
    */
   void setDefaultUserAgent();

   /**
    *   Sets the bind address of the fetcher.
    */
   void setBindAddress( const MC2String& bindAddress );

   void setProxyAddress(const MC2String& proxyAddress );

   /**
    * Takes over the socket connection and resets socket member.
    * 
    * @return member socket m_sock
    */
   TCPSocket* takeOwnershipForSocket();

protected:
   /// Creates an SSLSocket or returns NULL
   virtual TCPSocket* createSSLSocket() { return NULL; }
   
   /// User agent
   MC2String m_userAgent;

private:

   /**
    *   Gets or posts, depending on if postData is empty or not.
    */
   int fetch( MC2String& result,
              HttpHeader& headers,
              const URL& url,
              const MC2String& postData,
              uint32 timeout_ms,
              const HttpHeader* inHeaders,
              const bool logSocketInfo = false,
              const MC2String& callingFunction = MC2String() );

   /**
    *   Gets the contents of a file.
    *   @param result   Where the contents is put.
    *   @param path     The path to the file.
    */
   int fetchFile( MC2String& result,
                  const MC2String& path );

   /**
    *   Reads the headers into <code>header</code>.
    */
   int readHeaders( HttpHeader& header, TCPSocket& sock, uint32 timeout_ms );
   
   /**
    *   Reads the body when chunked mode is not used.
    */
   int readBody( MC2String& target, TCPSocket& sock, int contentLength,
                 uint32 timeout_ms );

   /**
    *   Reads a chunked body.
    */
   int readBodyChunked( MC2String& target, TCPSocket& sock,
                        HttpHeader& header,
                        uint32 timeout_ms );

   /**
    *   Sets m_lastHost etc. and returns true if no change.
    */
   bool compareAndSetLast( const URL& url );
   
   /// Last socket used.
   TCPSocket* m_sock;

   /// bindAddress
   MC2String m_bindAddress;

   /// Last host
   MC2String m_lastHost;
   /// Last port
   uint32 m_lastPort;
   /// Last protocol
   MC2String m_lastProto;
   /// proxy server
   MC2String m_proxyAddress;
};

#endif
