/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include <memory>

#include "URLFetcherNoSSL.h"

#include "TCPSocket.h"

#include "URL.h"
#include "HttpHeader.h"
#include "HttpHeaderLines.h"
#include "DataBuffer.h"
#include "FileUtils.h"

class StringDataBuffer : public DataBuffer {
public:
   StringDataBuffer( MC2String& str ) {
      m_str.swap( str );
      m_buf = reinterpret_cast<uint8*>( &( m_str[0] ) );
      m_pos = m_buf;
      m_bufSize = m_str.length();
      m_selfAlloc = false;
   }
private:
   /// String that holds the data.
   MC2String m_str;
};

static inline uint32 ms_to_us( uint32 ms )
{
   if ( ms == MAX_UINT32 ) {
      return ms;
   } else {
      return ms * 1000;
   }
}

URLFetcherNoSSL::URLFetcherNoSSL() {
   m_sock = NULL;
   setDefaultUserAgent();
}

URLFetcherNoSSL::~URLFetcherNoSSL()
{
   delete m_sock;
}

int
URLFetcherNoSSL::get( MC2String& result,
                      const URL& url,
                      uint32 timeout_ms,
                      const HttpHeader* inHeaders )
{
   HttpHeader head;
   return fetch( result, head, url, "", timeout_ms, inHeaders );
}

URLFetcherNoSSL::dbPair_t
URLFetcherNoSSL::get( const URL& url,
                      uint32 timeout_ms,
                      const HttpHeader* inHeaders )
{
   HttpHeader head;
   return get( head, url, timeout_ms, inHeaders);
}

URLFetcherNoSSL::dbPair_t
URLFetcherNoSSL::get( HttpHeader& header,
                      const URL& url,
                      uint32 timeout_ms,
                      const HttpHeader* inHeaders )
{
   MC2String buf;
   int getres = get( buf, header, url, timeout_ms, inHeaders );
   return dbPair_t( getres,
                    new StringDataBuffer( buf ) );
}

int
URLFetcherNoSSL::get( MC2String& result,
                      HttpHeader& headers,
                      const URL& url,
                      uint32 timeout_ms,
                      const HttpHeader* inHeaders )
{
   return fetch( result, headers, url, "", timeout_ms, inHeaders );
}

int 
URLFetcherNoSSL::post( MC2String& result,
                       HttpHeader& headers,
                       const URL& url,
                       const MC2String& postData,
                       uint32 timeout_ms,
                       const HttpHeader* inHeaders )
{
   return fetch( result, headers, url, postData, timeout_ms, inHeaders );
}

int
URLFetcherNoSSL::post( MC2String& result,
                       const URL& url,
                       const MC2String& postData,
                       uint32 timeout_ms,
                       const HttpHeader* inHeaders )
{
   HttpHeader headers;
   return fetch( result, headers, url, postData, timeout_ms, inHeaders );
}

int
URLFetcherNoSSL::postWithLogging( MC2String& result,
                                  const URL& url,
                                  const MC2String& postData,
                                  uint32 timeout_ms,
                                  const HttpHeader* inHeaders,
                                  const MC2String& callingFunction ) {
   HttpHeader headers;
   return fetch( result, headers, url, postData, timeout_ms, inHeaders, true,
                 callingFunction );
}

int
URLFetcherNoSSL::readHeaders( HttpHeader& header,
                         TCPSocket& sock,
                         uint32 timeout_ms )
{
   // Read headers
   MC2String answer;
   char buf;
   bool headersDone = false;
   // Read answer (FIXME) Add timeout
   // FIXME: Add contentlength too
   answer.reserve( 1024 );
   int nbrLines = 0;
   while ( sock.readExactly((byte*)&buf, 1, ms_to_us(timeout_ms) ) == 1 ) {
      if ( buf != '\r' && buf != '\n' ) {
         answer += buf;
      }
      if ( buf == '\n' ) {
         if ( answer.empty() ) {
            headersDone = true;
            break;
         } else {
            if ( nbrLines++ == 0 ) {
               header.setStartLine( new MC2String( answer ) );
            } else {
               bool res = header.addHeaderLine( answer.c_str() );
               if ( ! res ) {
                  mc2log << warn
                         << "[URLFetcherNoSSL]: Error while parsing header line "
                         << answer << endl;
                  return -5;
               }
            }
         }
         answer = "";
      }
   }

   if ( ! headersDone ) {
      mc2log << "[URLFetcherNoSSL]: End of file while parsing headers" << endl;
      return -6;
   }
   return 0;
}

bool
URLFetcherNoSSL::compareAndSetLast( const URL& url )
{
   // FIXME: Also check the ip, since there can be virtual hosts.
   bool same = true;

   mc2dbg8 << "[URLFetcherNoSSL]: m_lastHost = "
           << m_lastHost << ", m_lastPort = " << m_lastPort
           << ", m_lastProto = " << m_lastProto << endl;
   
   if ( m_lastPort  != (uint32)url.getPort() ||
        m_lastHost  != url.getHost() ||
        m_lastProto != url.getProto() ) {
      same = false;
   }
   m_lastPort  = url.getPort();
   m_lastHost  = url.getHost();
   m_lastProto = url.getProto();
   
   mc2dbg8 << "[URLFetcherNoSSL]: m_lastHost = "
           << m_lastHost << ", m_lastPort = " << m_lastPort
           << ", m_lastProto = " << m_lastProto << endl;

   return same;
}


int
URLFetcherNoSSL::fetch( MC2String& result,
                        HttpHeader& header,
                        const URL& oldURL,
                        const MC2String& postData,
                        uint32 timeout_ms,
                        const HttpHeader* inHeaders,
                        const bool logSocketInfo,
                        const MC2String& callingFunction )
{
   const bool useKeepAlive = false;

   result.clear();
   if ( ! oldURL.isValid() ) {
      return -15;
   }

   // special case for "file://"
   if ( oldURL.getProto() != NULL && 
        strcasecmp( oldURL.getProto(), "file" ) == 0 ) {
      if ( !postData.empty() ) {
         mc2log << error << "[URLFetcherNoSSL]: Can't post to "
                << oldURL << endl;
         return -15;
      }
      else {
         return fetchFile( result, oldURL.getPath() );
      }
   }

   bool useHttps = strcmp(oldURL.getProto(), "https") == 0;
   bool useHttp = strcmp(oldURL.getProto(), "http") == 0;
   bool useSSL = useHttps;

   URL url( oldURL );
   if ( ! useSSL ) {
      url = URL(m_proxyAddress + oldURL.getSpec());
      if ( ! url.isValid() ) {
         return -15;
      }
   }

   // compareAndSetLast must be first, since we want to setlast.
   bool reuse = useKeepAlive && compareAndSetLast( url ) && m_sock;
   if ( reuse ) {
      mc2dbg << "[URLFetcherNoSSL]: Could re-use socket now" << endl;
   } else {
      delete m_sock;
      m_sock = NULL;
   }
   
   
   // Add a non-blocking read and check if the socket is
   // connected. If 0 bytes are read it is EOF, if -1 and EAGAIN
   // it should still be connected.
   if ( reuse ) {
      m_sock->setBlocking( false );
      uint8 buf;
      int res = m_sock->read( &buf, 1, ms_to_us( timeout_ms ) );
      if ( res == -3 ) {
         mc2dbg << "[URLFetcherNoSSL]: EAGAIN - nice - reusing " << endl;
         m_sock->setBlocking( true );
      } else {
         reuse = false;
         delete m_sock;
         m_sock = NULL;
         mc2dbg << "[URLFetcherNoSSL]: res = " << res << endl;
      }
   } 
   
   if ( ! reuse ) {
      // Open new socket.
      TCPSocket* sock = NULL;
      if ( useSSL ) {
         sock = createSSLSocket();
         if ( sock == NULL ) {
            mc2log << error << "[URLFetcherNoSSL]: SSLSocket not supported"
                   << endl;
            return -100;
         }
      } else {
         sock = new TCPSocket();
      }
      m_sock = sock;

      // Check wether we need to log socket information or not.
      if ( logSocketInfo ) {
         m_sock->setLogging( true );
         m_sock->setPrintOnDestruction( true );
         m_sock->setLoggPrefix( "[URLFetcherNoSSL]:" + callingFunction + " " );
      }
   }

   if ( ! reuse ) {
      if ( m_sock->open() == false ) {
         mc2log << error << "[URLFetcherNoSSL]: Could not open socket! ("
                << strerror(errno) << ")"
                << endl;
         return -16;
      }
      if ( !m_bindAddress.empty() ) {
         mc2dbg << "[URLFetcherNoSSL]: bindAddress " << m_bindAddress << endl;
         if ( m_sock->bind( m_bindAddress.c_str() ) == false ) {
            mc2log << error << "[URLFetcherNoSSL]: Could not bind to \""
                   << m_bindAddress << "\" (" << strerror(errno) << ")" 
                   << endl;
            return -18;
         }
      }
      if ( m_sock->connect( url.getHost(), url.getPort(), 
                                 ms_to_us( timeout_ms ) ) == false ) 
      {
         mc2log << error << "[URLFetcherNoSSL]: Could not connect ("
                << strerror(errno) << ")" << endl;
         return -17;
      }
   }

   MC2String userAgentStr( m_userAgent );
   if ( inHeaders != NULL && inHeaders->getHeaderValue( 
           HttpHeaderLines::USER_AGENT ) != NULL ) {
      userAgentStr += " (";
      userAgentStr += *inHeaders->getHeaderValue( 
         HttpHeaderLines::USER_AGENT );
      userAgentStr += ")";
   }

   // Make request string
   {
      const char* method = "GET";
      if ( ! postData.empty() ) {
         method = "POST";
      }
      const char* newline = "\r\n";
      MC2String req;
      req.reserve( 1024 + postData.length() );
      req = req + method + " ";
      if ( getProxyAddress().empty() || useSSL ) {
         req += url.getPath();
      } else {
         MC2String withoutFirstSlash(url.getPath());
         size_t slashPos = withoutFirstSlash.find_first_not_of('/');
         req += ( withoutFirstSlash.c_str() + slashPos );
      }

      req += MC2String(" HTTP/1.1") + newline;
      req = req + "User-Agent: " + userAgentStr + newline;
      char portBuf[60];
      // if http and 80 or https and 443 then no port (Apache says 400)
      if ( (url.getPort() == 80  && useHttp) || 
           (url.getPort() == 443 && useHttps) )
      {
         portBuf[ 0 ] = '\0';
      } else {
         sprintf( portBuf, ":%u", url.getPort() );         
      }
      req = req + "Host: " + url.getHost() + portBuf + newline;
      if ( useKeepAlive ) {
         req = req + "Connection: Keep-Alive" + newline;
      } else {
         req = req + "Connection: close" + newline;
      }
      if ( ! postData.empty() ) {
         char tmp[200];
         sprintf(tmp, "%u", (unsigned int)(postData.length()) );
         req = req + "Content-Length: " + tmp + newline;
      }
      if ( inHeaders != NULL ) {
         for ( HttpHeader::HeaderMap::const_iterator it = 
                  inHeaders->getHeaderMap().begin() ;
               it != inHeaders->getHeaderMap().end() ; ++it )
         {
            if ( it->first == HttpHeaderLines::USER_AGENT ) {
               // Added above in special way
               continue;
            }
            req = req + it->first + ": " + *it->second + newline;
         }
      }
      req += newline;
      
      if ( ! postData.empty() ) {
         req += postData;
      }

      // Send it 
      
      if ( m_sock->writeAll( (byte*)req.c_str(), req.length() ) != 
           (int)req.length() ) {
         mc2log << error << "[URLFetcherNoSSL]: Could not send request ("
                << strerror(errno) << ")" << endl;
         return -4;
      }
   }

   int headerRes = readHeaders( header, *m_sock, timeout_ms );
   if ( headerRes != 0 ) {
      return headerRes;
   }

   uint32 contentlength = header.getContentLength();
   int toReserve = contentlength ? contentlength : 1024;
   MC2String answer;
   answer.reserve( toReserve + 1 );

   const MC2String* te = header.getHeaderValue( "Transfer-Encoding" );
   
   if ( te && ( te->find( "chunked") != MC2String::npos ) ) {
      int res = readBodyChunked( answer, *m_sock, header, timeout_ms );
      if ( res != 0 ) {
         mc2log << warn << "[URLFetcherNoSSL]: Error when fetching chunked "
                << MC2CITE(url.getSpec()) << endl;
         return res;
      }     
   } else {
      // Not chunked
      int res = readBody( answer, *m_sock, contentlength, timeout_ms );
      
      if ( res != 0 ) {
         mc2log << warn << "[URLFetcherNoSSL]: Didn't get EOF errno="
                << strerror(errno) << endl;
         return -7;
      }
   }

   if ( contentlength != 0 && contentlength != answer.size() ) {
      mc2log << warn << "[URLFetcherNoSSL]: contentlength = "
             << contentlength << " != answer.size() = "
             << answer.size()
             << strerror(errno) << endl;
      return -8;
   }

//    mc2dbg << "Header:" << endl;
//    mc2dbg << header << endl;

   // Cannot use keep-alive right now, since there must be something
   // that closes the socket after a while.
   if ( useKeepAlive ) {
      // If we get down here, we could re-use the socket.
      const MC2String* h = header.getHeaderValue( "Connection" );
      MC2String conn = h ? 
         StringUtility::copyUpper( *header.getHeaderValue( "Connection" ) )
         : "";
   
      if ( strstr( conn.c_str(), "KEEP-ALIVE" ) != NULL ) {
         mc2dbg << "[URLFetcherNoSSL]: Saving old socket" << endl;
         // m_sock already points to a connection
      }
   }
   
   // Put the answer into result.
   result.swap( answer );
   return header.getStartLineCode();
}

int
URLFetcherNoSSL::fetchFile( MC2String& result,
                            const MC2String& path ) {
   char* buf;
   if ( FileUtils::getFileContent( path.c_str(), buf ) ) {
      result = buf;
      delete[] buf;
      return 200; // HTTP OK
   }
   return -1;
}

int
URLFetcherNoSSL::readBody( MC2String& target,
                           TCPSocket& sock,
                           int contentlength,
                           uint32 timeout_ms )
{
   // Now read body data. This part could probably be optimized.
   char buf;
   int res = 1;
   if ( contentlength == 0 ) {
      // Read until closed
      while ( ( res = sock.read((byte*)&buf, 1,
                                ms_to_us ( timeout_ms ) ) ) == 1 ) {
         target += buf;
      }
      return res;
   } else {
      // Read contentlength bytes.
#if 0
      while ( contentlength &&
              ( res = sock.read((byte*)&buf, 1,
                                ms_to_us(timeout_ms ) ) ) == 1 ) {
         target += buf;
         if ( --contentlength == 0 ) {
            mc2dbg8 << "[URLFetcherNoSSL::readBody]: str = "
                    << MC2CITE( target ) << endl;
            return 0;
         }
      }
#else
      // No need for a really big buffer, it seems. read returns if there
      // is a packet.
      DataBuffer bigBuf( 65536 );
      while ( contentlength &&
              ( res = sock.read(bigBuf.getBufferAddress(),
                                MIN( contentlength,
                                     (int)bigBuf.getBufferSize() ),
                                ms_to_us(timeout_ms) ) ) >= 1 ) {
         
         target.append( reinterpret_cast<char*>( bigBuf.getBufferAddress()),
                        res );
         contentlength -= res;
         if ( contentlength == 0 ) {
            return contentlength;
         }
      }
#endif
      if ( res < 0 ) return res;
      return -10; // We should not get here
   }
}

int
URLFetcherNoSSL::readBodyChunked( MC2String& target, TCPSocket& sock,
                             HttpHeader& header,
                             uint32 timeout_ms )
{
   uint32 lengthSum = 0;
   mc2dbg << "[URLFetcherNoSSL]: Reading chunked body" << endl;
   // Read until the chunk length is zero or an error occurs.
   for ( ;; ) {
      MC2String tmpLenght;
      tmpLenght.reserve( 16 );
      char buf;
      int res;
      // FIXME: Error checking here.
      while ( ( res = sock.read((byte*)&buf, 1, ms_to_us(timeout_ms))) == 1 ) {
         if ( buf != '\r' && buf != '\n' ) {
            tmpLenght += buf;
         }
         if ( buf == '\n' ) {
            break;
         }
      }
      mc2dbg8 << "[URLFetcherNoSSL]: tmpLength = "
             << MC2CITE(tmpLenght) << endl;
      if ( res < 0 ) return res;
      if ( res == 0 ) return -9; // EOF is an error here
      
      // Get the first hex digits as length.
      char* endPtr;
      int length = strtoul( tmpLenght.c_str(), &endPtr, 16 );
      lengthSum += length;
      
      if ( endPtr == tmpLenght.c_str() ) {
         return -11;
      }
      mc2dbg8 << "[URLFetcherNoSSL]: Length as hex " << MC2HEX(length) << endl;
      
      // Now read chunk data
      if ( length > 0 ) {
         readBody( target, sock, length, timeout_ms );
      }
      // Read CRLF
      while ( ( res = sock.read((byte*)&buf, 1,
                                ms_to_us(timeout_ms) ) ) == 1 ) {
         if ( buf != '\r' && buf != '\n' ) {
            mc2log << "[URLFetcherNoSSL]: Not CRLF after chunk body!!" << endl;
            // Must be CR or LF here
            return -12;
         }
         if ( buf == '\n' ) break;
      }
      if ( length == 0 ) {
         // Should read trailer here if needed
         // But we don't request them.
         if ( lengthSum == target.length() ) {
            return 0;
         } else {
            mc2log << "[URLFetcherNoSSL]: Sum of chunklengths != target.length() "
                   << endl;
            return -14;
         }
      }

   }   
}

const MC2String&
URLFetcherNoSSL::getUserAgent() const
{
   return m_userAgent;
}

void
URLFetcherNoSSL::setUserAgent( const MC2String& userAgent )
{
   m_userAgent = userAgent;
}

void
URLFetcherNoSSL::setDefaultUserAgent() {
   m_userAgent = "MC2-URLFetcherNoSSL/1.1";
#ifdef __linux__
   m_userAgent += " (linux)";
#endif
}

void
URLFetcherNoSSL::setBindAddress( const MC2String& bindAddress ) {
   m_bindAddress = bindAddress;
}

const MC2String&
URLFetcherNoSSL::getProxyAddress() const 
{
   return m_proxyAddress;
}


void
URLFetcherNoSSL::setProxyAddress( const MC2String& address ) 
{
   m_proxyAddress = address;
}

TCPSocket*
URLFetcherNoSSL::takeOwnershipForSocket() {
   TCPSocket *sock = m_sock;
   m_sock = NULL;
   return sock;
}
