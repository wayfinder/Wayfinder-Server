/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "HttpInterfaceRequest.h"
#include "HttpHeader.h"
#include "HttpBody.h"
#include "TCPSocket.h"
#include "SocketBuffer.h"
#include "SSLSocket.h"
#include "HttpHeaderLines.h"
#include "InterfaceParserThread.h"
#include "ParserThreadGroup.h"
#include "InterfaceHandleIO.h"
#include "Utility.h"

#include "HttpCodes.h"

using namespace HttpHeaderLines;

#undef DEBUG

// Constants
uint32 HttpInterfaceRequest::MAX_SOCKET_WAITTIME        = 60000000;
uint32 HttpInterfaceRequest::MAX_REUSED_SOCKET_WAITTIME = 30000000;
uint32 HttpInterfaceRequest::MAX_NUMBER_SOCKET_REUSES   = 100;
const uint32 HttpInterfaceRequest::MAX_LINE_LENGTH      = 4096;


HttpInterfaceRequest::HttpInterfaceRequest( TCPSocket* reqSock, const IPnPort& serverAddress )
      : SocketInterfaceRequest(),
        m_reusedConnection( 0 ),
        m_iostate( io_uninitalized ), m_chunkedstate( reading_chunk_size ),
        m_requestHeader( new HttpHeader() ), m_bodyRequestLength( 0 ),
        m_requestBody( new HttpBody() ), m_replyHeader( NULL ),
        m_replyBody( NULL ), m_replyBuffer( NULL ), m_replySize( 0 ),
        m_replyPos( 0 ), m_IPnPort( 0 ,0 ),
        m_serverAddress( serverAddress )
{
   // Setup socket
   m_reqSock = reqSock;
   m_sockBuff = new SocketBuffer( reqSock );

   // Set timeouts
   m_timeout = MAX_SOCKET_WAITTIME / 1000; // ms
   m_totalTimeout = MAX_SOCKET_WAITTIME / 1000 / 100 / 2; // s
   
   reset();
   // New socket, new hand shake
   m_iostate = initial_hand_shake;
   // Get peer IP 'n Port
   if ( !m_reqSock->getPeerName( m_IPnPort.first, m_IPnPort.second ) ) {
      mc2log << "HttpInterfaceRequest no peer aborting" << endl;
      m_iostate = io_error;
      setState( Error );
   }

   // Logprefix
   const char randChars[80] = 
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
   strcpy( m_logPrefix, "[XXX][NA;NA] " );
   m_logPrefix[ 1 ] = 
      randChars[(int)((strlen(randChars)+0.0)*rand()/(RAND_MAX+1.0))];
   m_logPrefix[ 2 ] = 
      randChars[(int)((strlen(randChars)+0.0)*rand()/(RAND_MAX+1.0))];
   m_logPrefix[ 3 ] = 
      randChars[(int)((strlen(randChars)+0.0)*rand()/(RAND_MAX+1.0))];
   snprintf( m_logPrefix + 6, 45, "%d.%d.%d.%d:%d;NA] ", 
             int((m_IPnPort.first & 0xff000000) >> 24), 
             int((m_IPnPort.first & 0x00ff0000) >> 16), 
             int((m_IPnPort.first & 0x0000ff00) >>  8),
             int (m_IPnPort.first & 0x000000ff), 
             m_IPnPort.second );
}


HttpInterfaceRequest::~HttpInterfaceRequest() {
   reset();
   delete m_requestHeader;
   delete m_requestBody;
   delete m_sockBuff;
   // Factory decides what to do with m_reqSock
}


void
HttpInterfaceRequest::terminate() {
   // Check state
   if ( getState() == Ready_To_IO_Reply &&
        (m_iostate == writing_reply_header ||
         m_iostate == writing_reply_body ) &&
        (m_replyPos == 0 || m_replyPos == 1) &&
        m_replyHeader != NULL &&
        ( (m_replyHeader->getHeaderValue( &KEEP_ALIVE ) != NULL &&
           (*m_replyHeader->getHeaderValue( &KEEP_ALIVE ))[ 0 ] == '\0') ||
          ( m_requestHeader->getMinorVersion() > 0 &&
            m_requestHeader->getMajorVersion() >= 1 ) ) )
   {
      // Change header and stop Keep-Alive
      m_replyHeader->addHeaderLine( &CONNECTION, new MC2String( CLOSE ) );
      m_replyHeader->deleteHeaderLine( &KEEP_ALIVE );
      makeReplyBuffer();
   }
}


int 
HttpInterfaceRequest::getPriority( const InterfaceHandleIO* g ) const 
{
   const ParserThreadGroup* pg = dynamic_cast< const ParserThreadGroup* >
      ( g );
   if ( m_requestHeader != NULL && pg != NULL &&
        m_requestHeader->getHeaderValue( &X_FORWARDED_FOR ) != NULL &&
        m_requestHeader->getHeaderValue( &X_WF_ID ) != NULL &&
        *m_requestHeader->getHeaderValue( &X_WF_ID ) == 
        pg->getServerInstanceStr() )
   {
      return -1;
   } else {
      return SocketInterfaceRequest::getPriority( g );
   }
}


int 
HttpInterfaceRequest::getBytes( byte* buff, size_t length ) {
   int res = m_sockBuff->read( buff, length );
   if ( res > 0 ) {
//       mc2dbg << "HttpInterfaceRequest::getBytes read " << res
//              << " bytes" << endl;
   } else if ( res == -2 || res == -3 ) {
      // Timeout or EAGAIN -> try again later
   } else if ( res == -4 ) {
      // Read wants write
      m_invIO = true;
   } else {
      // Error / closed
      if ( res == 0 && m_reusedConnection > 0 && 
           m_iostate == reading_start_line &&
           m_currLine.empty() )
      {
         // Other end done with reused connection
         m_iostate = io_done;
         setState( Done );
      } else {
         mc2log << warn << "HttpInterfaceRequest::handleIO state "
                << getIOStateAsString( m_iostate )
                << " read failed " << res << " = " 
                << (res == 0 ? "closed" : "error" ) 
                << " Peer " << m_IPnPort << endl;
         m_iostate = io_error;
         setState( Error );
#ifdef DEBUG
         // Debug
         dumpState( mc2log );
#endif
      }
   }

   return res;
}


bool 
HttpInterfaceRequest::keepConnection( uint32 statusNbr ) const {
   return 
      // If client wants persistent connection
      ( ( m_requestHeader != NULL &&
          m_requestHeader->getMinorVersion() > 0 &&
          m_requestHeader->getMajorVersion() >= 1 &&
          (!(m_requestHeader != NULL &&
             m_requestHeader->getHeaderValue( &CONNECTION ) !=
             NULL &&
             *m_requestHeader->getHeaderValue( &CONNECTION ) == CLOSE ) ) )
        ||
        ( m_requestHeader != NULL &&
          m_requestHeader->getHeaderValue( &CONNECTION ) != NULL && 
          StringUtility::strcasecmp( 
             m_requestHeader->getHeaderValue( &CONNECTION )->c_str(),
             KEEP_ALIVE.c_str() ) == 0 ) )
      &&
      // if not closed by us
      ( m_replyHeader == NULL ||
        !(m_replyHeader->getHeaderValue( &CONNECTION ) != NULL &&
          *m_replyHeader->getHeaderValue( &CONNECTION ) == CLOSE) )
      && 
      // If status code doesn't need closed connection
      ! ( (statusNbr >= 400 && statusNbr < 500 &&
           !(m_requestHeader != NULL && 
             m_requestHeader->getMajorVersion() == 1
             && m_requestHeader->getMinorVersion() == 1 && 
             statusNbr == HttpCode::NOT_FOUND) ) ||
          statusNbr == HttpCode::NOT_IMPLEMENTED ||
          statusNbr == HttpCode::SERVICE_UNAVAILABLE );
}


int 
HttpInterfaceRequest::handleRequestBytes( bool readyRead, bool readyWrite )
{
   byte ch = 0;
   int res = 1;
   if ( ! ((readyRead || m_sockBuff->hasBytes()) || 
           (readyWrite && m_invIO)) ) 
   {
      // Nothing to do
      return -1;
   }

   if ( m_iostate == reading_start_line ) {
      while ( res > 0 && m_iostate == reading_start_line ) {
         // Read more
         res = getBytes( &ch, 1 );

         if ( res <= 0 ) {
            // No more right now
         } else if ( ch == '\n' ) { // Check if line ends
//            mc2dbg << "HttpInterfaceRequest::handleRequestBytes eol"
//                   << endl;
            m_iostate = reading_header;

            if ( m_currLine.size() > 0 &&
                 m_currLine[ m_currLine.size() -1 ] == '\r' ) 
            {
               // Remove it
               m_currLine.erase( m_currLine.size() -1, 1 );
            }
            mc2dbg2 << "Start line: " << m_currLine << endl;
            if ( m_currLine.empty() ) {
               // Empty line before start line is ok and should be ignored
               m_iostate = reading_start_line; // Still no start line
            } else if ( !m_requestHeader->addRequestLine( 
                           m_currLine.c_str() ) ) 
            {
               if ( m_currLine.size() == 11 && 
                    /*StringUtility::*/strncmp( 
                       m_currLine.c_str(), " / HTTP/1.", 10 ) == 0 )
               {
                  // Special end of keep-alive from client
                  // " / HTTP/1.X"
                  mc2dbg2 << "HttpInterfaceRequest::"
                          << "handleRequestBytes end of Keep-Alive: "
                          << m_currLine << endl;
               } else {
                  // Error, abort, abort, abort
                  mc2log << warn << "HttpInterfaceRequest::"
                         << "handleRequestBytes bad start line " 
                         << m_currLine << " Peer " << m_IPnPort << endl;
               }
               res = -1;
               setStatusReply( HttpCode::BAD_REQUEST );
            } else {
               // Else ok, check version
               if ( !( (m_requestHeader->getMinorVersion() == 0 &&
                        m_requestHeader->getMajorVersion() == 1) ||
                       (m_requestHeader->getMinorVersion() == 1 && 
                        m_requestHeader->getMajorVersion() == 1) ) )
               {
                  // Not 1.0 nor 1.1
                  mc2log << warn << "HttpInterfaceRequest::"
                         << "handleRequestBytes HTTP version " 
                         << m_requestHeader->getMajorVersion() << "." 
                         << m_requestHeader->getMinorVersion()
                         << " not supported Peer " << m_IPnPort << endl;
                  setStatusReply( HttpCode::HTTP_VERSION_NOT_SUPPORTED );
                  res = -1;
               }
            } 
            // Clear m_currLine
            m_currLine = "";
         } else {
            // Add to m_currLine
            m_currLine += ch;
            if ( m_currLine.size() > MAX_LINE_LENGTH ) {
               mc2log << warn << "HttpInterfaceRequest::"
                      << "handleRequestBytes start line too long "
                      << MC2CITE( m_currLine ) << " Peer " << m_IPnPort
                      << endl;
               setStatusReply( HttpCode::REQUEST_URI_TOO_LONG );
               res = -1;
            }
         }
      } // End while res > 0 and iostate is reading startline
   } // End iostate == reading_start_line

   

   if ( m_iostate == reading_header && res > 0 ) {
      while ( res > 0 && m_iostate == reading_header ) {
         // Read more
         res = getBytes( &ch, 1 );

         if ( res <= 0 ) {
            // No more right now
         } else if ( ch == '\n' ) { // Check if line ends
            if ( m_currLine.size() > 0 && 
                 m_currLine[ m_currLine.size() -1 ] == '\r' ) 
            {
               // Remove it
               m_currLine.erase( m_currLine.size() -1, 1 );
            }

            //      Multiple message-header fields with the same field-name
            //      MAY be present in a message ... by appending each
            //      subsequent field-value to the first, each separated by
            //      a comma.
            // Add this feature to HttpHeader!?

            if ( !m_currLine.empty() && 
                 (m_currLine[ 0 ] == ' ' || m_currLine[ 0 ] == '\t' ) )
            {
               // If line start with SP or HT add it to last 
               m_currLine = m_lastHeaderLine + m_currLine;
            }
            mc2dbg2 << "Header line: " << m_currLine << endl;

            if ( !m_requestHeader->addHeaderLine( 
                    m_currLine.c_str(), true/*appendMultipleHeaders*/ ) ) {
               // Empty line?
               if ( m_currLine.empty() ) {
//                 mc2dbg << "HttpInterfaceRequest::handleRequestBytes eoh"
//                         << endl;
                  // End of header

#ifdef DEBUG
                  // Debug
                  MC2String header;
                  m_requestHeader->putHeader( &header );
                  mc2dbg << "Got header:" << endl << header << endl;
#endif

                  // What now?
                  if ( m_chunkedstate == reading_chunk_trailer ) {
                     // We have been reading chunked trailing header lines
                     // We are now done
                     m_iostate = io_done;
                     setState( Ready_To_Process );
                     res = -1;
                  } else if ( m_requestHeader->getMethod() == 
                       HttpHeader::POST_METHOD || 
                       m_requestHeader->getContentLength() > 0 ||
                       m_requestHeader->getHeaderValue( 
                          &TRANSFER_ENCODING ) != NULL )
                  {
                     // Get body. GET doesn't have a body. HAHAHA!
                     // Will be zeroed if chunked.
                     m_bodyRequestLength = 
                        m_requestHeader->getContentLength();

                     if ( m_requestHeader->getHeaderValue( 
                             &TRANSFER_ENCODING ) != NULL ) 
                     {
                        if ( StringUtility::strcasecmp( 
                                m_requestHeader->getHeaderValue( 
                                   &TRANSFER_ENCODING )->c_str(), 
                                CHUNKED.c_str() ) == 0 )
                        { // TODO: chunked,identity or chunked;q=0.1?
                           m_iostate = reading_request_body_chunked;
                           // Zero the request length, since some proxies
                           // might send both Chunked and content-length.
                           m_bodyRequestLength = 0; 
                        } else if ( StringUtility::strcasecmp( 
                                       m_requestHeader->getHeaderValue( 
                                          &TRANSFER_ENCODING )->c_str(), 
                                       IDENTITY.c_str() ) == 0 )
                        {
                           m_iostate = reading_request_body;
                        } else {
                           mc2log << warn << "HttpInterfaceRequest::"
                                  << "handleRequestBytes Unknown TE "
                                  << MC2CITE( 
                                     m_requestHeader->getHeaderValue( 
                                        &TRANSFER_ENCODING ) ) << endl;
                           setStatusReply( HttpCode::NOT_IMPLEMENTED );
                           res = -1;
                        }
                     } else {
                        m_iostate = reading_request_body;
                        if ( m_requestHeader->getContentLength() == 0 ) {
                           // Done process
                           m_iostate = io_done;
                           setState( Ready_To_Process );
                           res = -1;
                        }
                     }

                     if ( m_bodyRequestLength < (MAX_LINE_LENGTH<<8) ) {
                        // Make room for it
                        m_requestBody->checkSize( m_bodyRequestLength );

                        // Expect header line? (100 Continue) and V 1.1
                        if ( (m_requestHeader->getMinorVersion() == 1 &&
                              m_requestHeader->getMajorVersion() == 1) &&
                             m_requestHeader->getHeaderValue( &EXPECT ) 
                             != NULL &&
                             m_iostate != io_done/*Have body already*/ )
                        {
                           if ( StringUtility::strncasecmp( 
                                   StringUtility::trimStart( 
                                      m_requestHeader->getHeaderValue( 
                                         &EXPECT )->c_str() ), 
                                   "100-continue", 12 ) == 0 )
                           {
                              // Send HTTP/1.1 100 Continue
                              // Try to send 100 header if not set 417
                              MC2String data = getCurrHTTPVer();
                              data.append( HttpHeader::getHttpStartLine( 
                                              100 ) );
                              data.append( "\r\n" ); // End line
                              data.append( "\r\n" ); // End header
                              int wres = m_reqSock->write( 
                                 (const byte*)data.data(), data.size() );
                              if ( wres != int32( data.size() ) ) {
                                 mc2log << warn << "HttpInterfaceRequest::"
                                        << "handleRequestBytes failed 100-"
                                        << "Continue respons Peer " 
                                        << m_IPnPort << endl;
                                 setStatusReply( HttpCode::EXPECTATION_FAILED);
                                 res = -1; 
                              } else {
                                 mc2dbg2 << "HttpInterfaceRequest::"
                                         << "handleRequestBytes sent 100-"
                                         << "Continue" << endl;
                              }
                           } else {
                              // Bad Expect
                              mc2log << warn << "HttpInterfaceRequest::"
                                     << "handleRequestBytes bad Expect "
                                     << "line " << MC2CITE( 
                                        m_requestHeader->getHeaderValue(
                                           &EXPECT ) ) << " Peer " 
                                     << m_IPnPort << endl;
                              setStatusReply( HttpCode::EXPECTATION_FAILED );
                              res = -1;
                           }
                        }
                     } else {
                        mc2log << warn 
                               << "HttpInterfaceRequest::"
                               << "handleRequestBytes bad Content-"
                               << "Length " 
                               << m_bodyRequestLength << " Peer " 
                               << m_IPnPort << endl;
                        setStatusReply( HttpCode::REQUEST_ENTITY_TOO_LARGE );
                        res = -1;
                     }
                  } else {
                     // Done process
                     m_iostate = io_done;
                     setState( Ready_To_Process );
                     res = -1;
                  }
               } else {
                  // Error, abort, abort, abort
                  mc2log << warn << "HttpInterfaceRequest::"
                         << "handleRequestBytes bad header line "
                         << MC2CITE( m_currLine ) << " Peer " << m_IPnPort
                         << endl;
                  setStatusReply( HttpCode::BAD_REQUEST );
                  res = -1;
               }
            } // End if not header line added ok
            // Clear m_currLine
            m_lastHeaderLine = m_currLine;
            m_currLine = "";
         } else {
            // Add to m_currLine
            m_currLine += ch;
            if ( m_currLine.size() > MAX_LINE_LENGTH ) {
               mc2log << warn << "HttpInterfaceRequest::"
                      << "handleRequestBytes header line too long "
                      << MC2CITE( m_currLine ) << " Peer " << m_IPnPort
                      << endl;
               setStatusReply( HttpCode::BAD_REQUEST );
               res = -1;
            }
         }
      } // End while res > 0 and iostate is reading header
   } // End if iostate is reading header and res > 0


   if ( m_iostate == reading_request_body && res > 0 ) {
      while ( res > 0 && m_iostate == reading_request_body ) {
         // Get some
         res = getBytes( 
            (byte*)m_requestBody->m_body + m_requestBody->m_pos,
            m_bodyRequestLength - m_requestBody->m_pos );

         if ( res > 0 ) {
            m_requestBody->m_pos += res;
            // Check if done
            if ( m_requestBody->
                 getBodyLength() >= 
                 m_bodyRequestLength )
            {
               // Read all
               m_iostate = io_done;
               setState( Ready_To_Process );
/*
               // Send the "HTTP" here to indicate to client
               // that connection is ok
               const char* http = "HTTP";
               int wres = m_reqSock->write( (const byte*)http, 1 );
               if ( wres != 1 ) {
                  mc2log << warn << "HttpInterfaceRequest::"
                         << "handleRequestBytes write of HTTP after "
                         << "read request "
                         << "failed " << wres << " = " 
                         << (wres == 0 ? "closed" : "error" ) 
                         << " Peer " << m_IPnPort << endl;
                  m_iostate = io_error;
                  setState( Error );
                  res = -1;
#ifdef DEBUG
                  // Debug
                  dumpState( mc2log );
#endif
               } else {
                  mc2dbg8 << "HttpInterfaceRequest::handleRequestBytes "
                          << "sent HTTP ok." << endl;
                  m_replyPos = 1; // "HTTP" sent already
               }
*/
            } // Not done yet
         }
      } // End while res > 0 and iostate is reading request body
   } // End if iostate is reading request body and res > 0

   if ( m_iostate == reading_request_body_chunked && res > 0 ) {
      while ( res > 0 && m_iostate == reading_request_body_chunked ) {
         switch( m_chunkedstate ) {
            case reading_chunk_size :
               // Read more
               res = getBytes( &ch, 1 );

               if ( res <= 0 ) {
                  // No more right now
               } else if ( ch == '\n' ) { // Check if line ends
                  if ( m_currLine.size() > 0 &&
                       m_currLine[ m_currLine.size() -1 ] == '\r' ) 
                  {
                     // Remove it
                     m_currLine.erase( m_currLine.size() -1, 1 );
                  }
                  // Get the chunk size (is in HEX)
                  char* endPtr = NULL;
                  uint32 size = strtoul( m_currLine.c_str(), &endPtr, 16 );
                  if ( endPtr == NULL || 
                       (*endPtr != ';' && *endPtr != '\0') )
                  {
                     // Error bad size
                     mc2log << warn << "HttpInterfaceRequest::"
                            << "handleRequestBytes bad chunk-size " 
                            << m_currLine << " Peer " << m_IPnPort << endl;
                     setStatusReply( HttpCode::BAD_REQUEST );
                     res = -1;
                  } else {
                     // Ok
                     if ( size == 0 ) {
                        // last-chunk
                        m_iostate = reading_header;
                        m_chunkedstate = reading_chunk_trailer;
                        m_currLine = "";
                        // Check if already has request in m_sockBuff!
                        handleRequestBytes( readyRead, readyWrite );
                        res = -1;
                     } else {
                        // Check if too large 
                        if ( size >= (MAX_LINE_LENGTH<<8) ||
                             (m_bodyRequestLength + size) >= 
                             (MAX_LINE_LENGTH<<8) )
                        {
                           mc2log << warn 
                                  << "HttpInterfaceRequest::"
                                  << "handleRequestBytes chunked "
                                  << "length " << size << " total "
                                  << m_bodyRequestLength << " too large "
                                  << "Peer " << m_IPnPort << endl;
                           setStatusReply( HttpCode::REQUEST_ENTITY_TOO_LARGE);
                           res = -1;
                        } else {
                           m_bodyRequestLength += size;
                           m_chunkedstate = reading_chunk;
                           // Make room for it
                           m_requestBody->checkSize( m_bodyRequestLength );
                        }
                     }
                  }

                  // Clear m_currLine 
                  m_currLine = "";
               } else {
                  // Add to m_currLine
                  m_currLine += ch;
               }

               break;
            case reading_chunk :
               // Get some
               res = getBytes( 
                  (byte*)m_requestBody->m_body + m_requestBody->m_pos,
                  m_bodyRequestLength - m_requestBody->m_pos );

               if ( res > 0 ) {
                  m_requestBody->m_pos += res;
                  // Check if done
                  if ( m_requestBody->
                       getBodyLength() >= 
                       m_bodyRequestLength )
                  {
                     // Read all
                     m_chunkedstate = reading_chunk_crlf;
                  } // Not done yet
               }
               break;
            case reading_chunk_crlf:
               // Read one
               res = getBytes( &ch, 1 );
               if ( ch == '\n' ) {
                  // Done with chunk
                  m_currLine = "";
                  m_chunkedstate = reading_chunk_size;
               } else if ( ch == '\r' ) {
                  // CR
                  if ( m_currLine.empty() ) {
                     m_currLine += ch;
                  } else {
                     // Too many CR
                     mc2log << warn << "HttpInterfaceRequest::"
                            << "handleRequestBytes more than one CR after "
                            << "chunk " << " Peer " << m_IPnPort << endl;
                     res = -1;
                     setStatusReply( HttpCode::BAD_REQUEST );
                  }
               } else {
                  // Bad char
                  mc2log << warn << "HttpInterfaceRequest::"
                         << "handleRequestBytes not CRLF after "
                         << "chunk " << int(ch) << " Peer " << m_IPnPort
                         << endl;
                  res = -1;
                  setStatusReply( HttpCode::BAD_REQUEST );
               }
               break;
            case reading_chunk_trailer :
               // We never gets here... trailing entity-header are read in
               // m_iostate == reading_header above
               break;
         }
      } // End while res > 0 and iostate is reading request body chunked
   } // End if iostate is reading request body chunked and res > 0

   return res;
}


void
HttpInterfaceRequest::handleIO( bool readyRead, bool readyWrite ) {
//    mc2dbg << "HttpInterfaceRequest::handleIO " << readyRead
//           << ", " << readyWrite << " currline " << m_currLine << endl;
   // No invitos here (Guess the movie)
   m_invIO = false;

   if ( m_iostate == initial_hand_shake ) {
      int res = m_reqSock->initialHandshaking();
      if ( res > 0 ) {
         // Ok done 
         m_iostate = reading_start_line;
      } else if ( res == -4 ) {
         // Want read
         return;
      } else if ( res == -5 ) {
         // Want write
         m_invIO = true;
         return;
      } else {
         mc2log << warn << "HttpInterfaceRequest::"
                << "handleIO initialHandshake failed " << res 
                << " Peer " << m_IPnPort << endl;
         m_iostate = io_error;
         setState( Error );
         return;
      }
   }

   switch( m_iostate ) {
      case reading_start_line:
      case reading_header:
      case reading_request_body:
      case reading_request_body_chunked:
         handleRequestBytes( readyRead, readyWrite );
         break;

      case writing_reply_header:
      case writing_reply_body:
         if ( readyWrite || (readyRead && m_invIO) ) {
            // Write some more
            int res = m_reqSock->write( m_replyBuffer + m_replyPos, 
                                        m_replySize - m_replyPos );
            if ( res > 0 ) {
               m_replyPos += res;
               // If sent reply
               if ( m_replyPos >= m_replySize ) {
                  // Set state etc.
                  if ( keepConnection( m_replyCode ) ) {
                     // More read again setState( Ready_To_IO_Request )
//                      mc2dbg << "HttpInterfaceRequest::handleIO "
//                            << "done writing reply going to read request"
//                             << " bytes " << m_replyPos << endl;
                     reset();
                     m_reusedConnection++;
                     // Check if already has request in m_sockBuff!
                     handleRequestBytes( readyRead, readyWrite );
                  } else {
                     // No more then done
                     m_iostate = io_done;
                     setState( Done );
                     // No don't delete the factory desides what to do.
                     // The factory desides what to do with the socket.
//                     m_reqSock->close();
                  }
               }
            } else if ( res == -2 || res == -3 ) {
               // Timeout or EAGAIN -> try again later
            } else if ( res == -4 ) {
               // Write wants read
               m_invIO = true;
            } else {
               // Error / closed
               mc2log << warn << "HttpInterfaceRequest::handleIO "
                      << "write, reply, failed " << res << " = " 
                      << (res == 0 ? "closed" : "error" ) 
                      << " Peer " << m_IPnPort << endl;
               m_iostate = io_error;
               setState( Error );
#ifdef DEBUG
               // Debug
               dumpState( mc2log );
#endif
            }
         } // End if ok to call write
         break;

      case io_done:
      case io_error:
      case io_uninitalized:
      case initial_hand_shake:
         mc2log << error << "HttpInterfaceRequest::handleIO bad IO "
                << "state " << getIOStateAsString( m_iostate ) 
                << " Peer " << m_IPnPort << endl;
         break;
   };
}


void
HttpInterfaceRequest::timedout() {
   // Darn!
   mc2log << warn << "HttpInterfaceRequest::timedout state " 
          << getStateAsString( getState() ) << " Used IO time "
          << getUsedIOTime() << "ms of " << getTimeout() << "ms "
          << "Total IO time " << getTotalUsedIOTime() << "ms "
          << "Start time " << getIOStartTime()
          << " Peer " << m_IPnPort 
          << endl;
   if ( getState() == Ready_To_IO_Request ) {
      m_iostate = io_error;
      setState( Timeout_request );
   } else if ( getState() == Ready_To_IO_Reply ) {
      m_iostate = io_error;
      setState( Timeout_reply );
   } else {
      mc2log << error 
             << "HttpInterfaceRequest::timedout not in "
             << "request or reply state " << getStateAsString( getState() )
             << " Peer " << m_IPnPort << endl;
      m_iostate = io_error;
      setState( Error );
   }
}


void
HttpInterfaceRequest::handleOverloaded( int overLoad ) {
   reset();
   // Make 503 reply and set states
   setStatusReply( HttpCode::SERVICE_UNAVAILABLE,
                   overLoad*3 ); // 3 is a nice number (3s/req)
   mc2log << info << "HttpInterfaceRequest::handleOverloaded sending 503 "
          << "with " << overLoad*3 << " seconds retry time Peer "
          << m_IPnPort << endl;
}


bool
HttpInterfaceRequest::isIdle() const {
   if ( getState() == Ready_To_IO_Request &&
        m_iostate == reading_start_line && 
        m_currLine.empty() )
   {
      // Still nothing
      return true;
   } else {
      return false;
   }
}


void
HttpInterfaceRequest::setReply( HttpHeader* outHead, HttpBody* outBody ) {
   delete m_replyHeader;
   m_replyHeader = outHead;
   m_replyCode = m_replyHeader->getStartLineCode();
   delete m_replyBody;
   m_replyBody = outBody;
   setState( Ready_To_IO_Reply );
   m_iostate = writing_reply_header;
   // Make reply buffer
   makeReplyBuffer();
}


bool
HttpInterfaceRequest::setStatusReply( 
   uint32 statusNbr, uint32 retryTime,
   vector< MC2String >* extraHeaderFields )
{
#ifdef DEBUG
   // Debug
   mc2dbg << "Reply Status " << statusNbr << endl;
   // Debug
   dumpState( mc2log );
#endif
   bool correctResult = false;
   const MC2String* header = NULL;
   const MC2String* body   = NULL;
   correctResult = HttpHeader::getHttpErrorPage( statusNbr, header, body );

   uint32 extraSize = 0;
   if ( extraHeaderFields != NULL ) {
      for ( uint32 i = 0 ; i < extraHeaderFields->size() ; ++i ) {
         extraSize += (*extraHeaderFields)[ i ].size() + 2;
      }
   }
   delete [] m_replyBuffer;
   m_replyBuffer = new byte[ 1024 + header->size() + body->size() + 
                             extraSize ];

   char date[ 43 ];
   time_t now = TimeUtility::getRealTime();
   struct tm result;
   struct tm *tmStruct = gmtime_r( &now, &result );
   // Date: Mon, 16 Aug 1999 09:25:03 GMT
   strftime( date, 43, "Date: %a, %d %b %Y %H:%M:%S GMT\r\n", tmStruct );
   uint32 dateLen = strlen( date );
   
   // Put the response together
   const char* httpVer = getCurrHTTPVer();
   const uint32 httpVerSize = strlen( httpVer );
   memcpy( m_replyBuffer + m_replySize, httpVer, httpVerSize );
   m_replySize += httpVerSize;
   memcpy( m_replyBuffer + m_replySize, header->c_str(), header->size() );
   m_replySize += header->size();
   memcpy( m_replyBuffer + m_replySize, "\r\n", 2 );
   m_replySize += 2;

   // date
   memcpy( m_replyBuffer + m_replySize, date, dateLen );
   m_replySize += dateLen;
   if ( statusNbr == 503 ) {
      // Retry-After
      m_replySize += sprintf( (char*)m_replyBuffer + m_replySize, 
                              "%s: %d\r\n", 
                              RETRY_AFTER.c_str(), retryTime );
   }
   if ( !keepConnection( statusNbr ) ) {
      // Connection: close
      memcpy( m_replyBuffer + m_replySize, CONNECTION.c_str(), 
              CONNECTION.size() );
      m_replySize += CONNECTION.size();
      memcpy( m_replyBuffer + m_replySize, ": ", 2 );
      m_replySize += 2;
      memcpy( m_replyBuffer + m_replySize, CLOSE.c_str(), 
              CLOSE.size() );
      m_replySize += CLOSE.size();
      memcpy( m_replyBuffer + m_replySize, "\r\n", 2 );
      m_replySize += 2;
   }
   m_replySize += sprintf( (char*)m_replyBuffer + m_replySize, 
                           "%s: %lu\r\n", 
                           CONTENT_LENGTH.c_str(), 
                           static_cast<unsigned long>( body->size() ) );
   // Extra fields
   if ( extraHeaderFields != NULL ) {
      for ( uint32 i = 0 ; i < extraHeaderFields->size() ; ++i ) {
         memcpy( m_replyBuffer + m_replySize, 
                 (*extraHeaderFields)[ i ].c_str(), 
                 (*extraHeaderFields)[ i ].size() );
         m_replySize += (*extraHeaderFields)[ i ].size();
         memcpy( m_replyBuffer + m_replySize, "\r\n", 2 );
         m_replySize += 2;
      }
   }
   // End header
   memcpy( m_replyBuffer + m_replySize, "\r\n", 2 );
   m_replySize += 2;
   // Add body
   memcpy( m_replyBuffer + m_replySize, body->c_str(), body->size() );
   m_replySize += body->size();

   m_replyPos = 0;
   if ( correctResult ) {
      m_replyCode = statusNbr;
   } else {
      m_replyCode = 500;
   }

   // Set state
   m_iostate = writing_reply_header;
   setState( Ready_To_IO_Reply );

   return correctResult;
}


void
HttpInterfaceRequest::reset() {
   m_currLine = "";
   m_lastHeaderLine = "";
   m_requestHeader->clear();
   m_bodyRequestLength = 0;
   m_requestBody->clear();
   delete m_replyHeader;
   m_replyHeader = NULL;
   delete m_replyBody;
   m_replyBody = NULL;
   delete [] m_replyBuffer;
   m_replyBuffer = NULL;
   m_replySize = 0;
   m_replyPos = 0;
   m_invIO = false;
   m_replyCode = 0;
   setState( Ready_To_IO_Request );
   m_iostate = reading_start_line;
   m_chunkedstate = reading_chunk_size;
}


bool 
HttpInterfaceRequest::isHttps() const {
#ifdef USE_SSL
   return dynamic_cast< const SSLSocket* > ( m_reqSock ) != NULL;
#else 
   return false;
#endif
}


void 
HttpInterfaceRequest::setLogUserName( const char* userName ) {
   char* lastSemicolon = strrchr( m_logPrefix, ';' );
   lastSemicolon[ 0 ] = '\0';
   strcat(m_logPrefix, ";");
   
   StringUtility::strlcat( m_logPrefix, 
                           userName != NULL ? userName : "NA", 45 );
   strcat(m_logPrefix, "] ");
}


void
HttpInterfaceRequest::setMaxSocketWaitTime( uint32 val ) {
   MAX_SOCKET_WAITTIME = val;
}


void
HttpInterfaceRequest::setMaxSocketReuses( uint32 val ) {
   MAX_NUMBER_SOCKET_REUSES = val;
}


ostream&
HttpInterfaceRequest::dumpState( ostream& out ) const {
   out << "HttpInterfaceRequest state:" << endl;
   out << " IOState " << getIOStateAsString( m_iostate ) << endl;
   if ( m_iostate == reading_request_body_chunked ) {
      out << "  chunkedstate " << int(m_chunkedstate) << endl;
   }
   
   out << " currLine " << m_currLine << endl;
   out << "  lastHeaderLine " << m_lastHeaderLine << endl;
   if ( m_requestHeader ) {
      MC2String header;
      m_requestHeader->putHeader( &header );
      out << " requestHeader " << header << endl;
   } else {
      out << " No requestHeader " << endl;
   }
   out << " bodyRequestLength " << m_bodyRequestLength << endl;
   if ( m_requestBody ) {
      out << " requestBody " << m_requestBody->getBody() << endl;
   } else {
      out << " No requestBody " << endl;
   }
   if ( m_replyHeader ) {
      MC2String header;
      m_replyHeader->putHeader( &header );
      out << " replyHeader " << header << endl;
   } else {
      out << " No replyHeader " << endl;
   }
   if ( m_replyBody ) {
      out << " replyBody " << m_replyBody->getBody() << endl;
   } else {
      out << " No replyBody " << endl;
   }
   if ( m_replyBuffer ) {
      Utility::hexDump( out, m_replyBuffer, m_replySize, " replyBuffer " );
   } else {
      out << " No replyBuffer " << endl;
   }
   out << " replySize " << m_replySize << endl;
   out << " replyPos " << m_replyPos << endl;
   out << " m_reusedConnection " << m_reusedConnection << endl;
   out << " invIO " << m_invIO << endl;
   out << " IPnPort " << m_IPnPort << endl;
   out << " replyCode " << m_replyCode << endl;
   out << " logPrefix " << m_logPrefix << endl;

   return out;
}


const char* 
HttpInterfaceRequest::getIOStateAsString( io_state_t state ) {
   switch( state ) {
      case initial_hand_shake:
         return "initial_hand_shake";
      case reading_start_line:
         return "reading_start_line";
      case reading_header:
         return "reading_header";
      case reading_request_body:
         return "reading_request_body";
      case reading_request_body_chunked:
         return "reading_request_body_chunked";
      case writing_reply_header:
         return "writing_reply_header";
      case writing_reply_body:
         return "writing_reply_body";
      case io_done:
         return "io_done";
      case io_error:
         return "io_error";
      case io_uninitalized:
         return "io_uninitalized";
   }

   return "Unknown state";
}


void 
HttpInterfaceRequest::makeReplyBuffer() {
   MC2String header;
   m_replyHeader->putHeader( &header );
   // End header
   header.append( "\r\n" );
   m_replySize = header.size() + m_replyBody->getBodyLength();
   delete [] m_replyBuffer;
   m_replyBuffer = new byte[ m_replySize + 1 ];
   memcpy( m_replyBuffer, header.c_str(), header.size() );
   memcpy( m_replyBuffer + header.size(), m_replyBody->getBody(),
           m_replyBody->getBodyLength() );
   m_replyPos = 0;
#ifdef DEBUG
   // Debug
   MC2String rb( (char*)m_replyBody->getBody(), m_replyBody->getBodyLength() );
   mc2dbg << "Reply data: " << endl
          << header << rb << endl;
#endif
}


const char* 
HttpInterfaceRequest::getCurrHTTPVer() const {
   const char* httpVer = "HTTP/1.0 ";
   if ( m_requestHeader != NULL ) {
      httpVer = m_requestHeader->getCurrHTTPVer();
   }

   return httpVer;
}

