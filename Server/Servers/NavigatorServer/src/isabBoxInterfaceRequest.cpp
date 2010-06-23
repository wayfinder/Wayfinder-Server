/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "isabBoxInterfaceRequest.h"
#include "DataSocket.h"
#include "MC2CRC32.h"
#include "NavPacket.h"
#include "NavMessage.h"
#include "isabBoxNavMessage.h"
// Http stuff
#include "HttpInterfaceRequest.h"
#include "HttpParserThread.h"
#include "HttpBody.h"
#include "HttpHeader.h"
#include "HttpHeaderLines.h"
#include "STLStringUtility.h"

#include "HttpCodes.h"

const uint32 IsabBoxInterfaceRequest::startHeaderSize = 6;
const uint32 IsabBoxInterfaceRequest::maxReplyMsgSize = 131072;
const uint32 navPacketheaderLength = 10; // Should never be any need to change this


const uint32 protoverPos = 5;


#define CONSTRUCTOR_COMMON_INIT \
        /* m_startHeader[ startHeaderSize ] = {0},*/\
        m_requestBuff( NULL ), m_requestPos( 0 ),\
        m_requestSize( 0 ),\
        m_replyBuff( NULL ), m_replySize( 0 ), m_replyPos( 0 ),\
        m_navMess( NULL ), m_reqPack( NULL ), m_replyPack( NULL ),\
        m_httpRequest( NULL ), m_uncompressedRequestSize( 0 ), \
        m_uncompressedReplySize( 0 )


IsabBoxInterfaceRequest::IsabBoxInterfaceRequest( DataSocket* dataSocket ) 
      : NavInterfaceRequest(),
        m_dataSocket( dataSocket ), 
        m_session( static_cast< isabBoxSession* > ( 
                      dataSocket->getSession() ) ),
        CONSTRUCTOR_COMMON_INIT
{
   // Set timeouts
   m_timeout = 180000; // ms   (180 s)
   m_totalTimeout = 300; // s (5 min)
   MC2_ASSERT( dynamic_cast< isabBoxSession* > ( 
                  dataSocket->getSession() ) );
   // Set to start state
   setState( Ready_To_IO_Request );
   m_iostate = reading_stx;
}


IsabBoxInterfaceRequest::IsabBoxInterfaceRequest( 
   HttpInterfaceRequest* ireq,
   HttpHeader*& outHead, 
   HttpBody*& outBody ) 
      : NavInterfaceRequest(),
        CONSTRUCTOR_COMMON_INIT
{
   MC2_ASSERT( dynamic_cast< DataSocket* > ( ireq->getSelectable() ) );
   m_dataSocket = static_cast< DataSocket* > ( ireq->getSelectable() ); 
   m_session = static_cast< isabBoxSession* > ( 
      m_dataSocket->getSession() );
   m_httpRequest = ireq;
   m_startQueueTime = ireq->getStartQueueTime();
   // Set to process state
   setState( Ready_To_Process );
   m_iostate = io_done;

   // Some Http stuff
   if ( HttpParserThread::checkHttpHeader( ireq ) ) {
      HttpParserThread::setKeepAlive( ireq, outHead );
      HttpParserThread::setHttpHeaderDate( outHead );
      HttpParserThread::setDefaultResponseLine( ireq, outHead );

      // Get Http body and use as NavRequest
      HttpBody& inBody = *ireq->getRequestBody();
      HttpHeader& inHead = *ireq->getRequestHeader();

      // Ping request for testing connectivity in things like browsers.
      if ( ireq->getRequestHeader()->getPagename() != NULL && 
           (*ireq->getRequestHeader()->getPagename() == "ping" ||
            ireq->getRequestHeader()->getPagename()->substr( 0, 5 ) == 
            "ping?" ) ) {
         outBody->addString( "<html>\n"
                             "<head>\n"
                             "<title>Pong"
                             "</title>\n"
                             "</head>\n"
                             "\n"
                             "<body>\n"
                             "<h1>Pong</h1>\n"
                             "</body>\n"
                             "</html>\n" );
         outHead->addHeaderLine( HttpHeaderLines::CONTENT_LENGTH, 
                                 STLStringUtility::uint2str( 
                                    outBody->getBodyLength() ) );
         outHead->addHeaderLine( HttpHeaderLines::CONTENT_TYPE, 
                                 "text/html" );
         ireq->setReply( outHead, outBody );
         // Out is now owned by ireq
         outHead = NULL;
         outBody = NULL;
         mc2log << info << "IsabBoxInterfaceRequest::IsabBoxInterfaceRequest "
                << "Got " << *ireq->getRequestHeader()->getPagename() 
                << " request sending pong reply." << endl;
         return;
      }

      // Check for special test request 
      if ( inHead.getHeaderValue( "X-WF-ReqType" ) != NULL &&
           *inHead.getHeaderValue( "X-WF-ReqType" ) == "NAV_ECHO_REQ" ) {
         // Echo the request body in reply
         HttpParserThread::copyEchoRequestToReply( &inHead, &inBody, 
                                                   outHead, outBody );
         outHead->addHeaderLine( HttpHeaderLines::CONTENT_LENGTH, 
                                 STLStringUtility::uint2str( 
                                    outBody->getBodyLength() ) );
         outHead->addHeaderLine( HttpHeaderLines::CONTENT_TYPE, 
                                 "application/binary" );
         ireq->setReply( outHead, outBody );
         // Out is now owned by ireq
         outHead = NULL;
         outBody = NULL;
         mc2log << info << "IsabBoxInterfaceRequest::IsabBoxInterfaceRequest "
                << "Got a X-WF-ReqType: NAV_ECHO_REQ in "
                << *ireq->getRequestHeader()->getPagename() 
                << " request sending back body." << endl;
         return;
      }

      // Make requestBuff
      m_requestSize = inBody.getBodyLength();
      m_requestBuff = new byte[ m_requestSize ];
      memcpy( m_requestBuff, inBody.getBody(), m_requestSize );

//      mc2dbg << "Request" << endl;
//      Utility::hexDump( cerr, m_requestBuff, m_requestSize );

      // Set a Content-Type to be nice to some proxies that needs one to
      // feel safe.
      outHead->addHeaderLine( HttpHeaderLines::CONTENT_TYPE, 
                              "application/binary" );

      // Set protoVer
      if ( m_requestSize > protoverPos ) {
         m_protoVer = m_requestBuff[ protoverPos ];
      } else {
         // Just something
         m_protoVer = 0xa;
      }
      if ( m_requestSize < navPacketheaderLength + 4/*CRC*/ ) {
          mc2log << warn << m_session->m_logPrefix
                 << "IsabBoxInterfaceRequest::IsabBoxInterfaceRequest ";
          if ( inHead.getMethod() == HttpHeader::POST_METHOD ) {
             mc2log << "too short packet " << m_requestSize << " bytes "
                    << "setting Http 400 error.";
             ireq->setStatusReply( HttpCode::BAD_REQUEST );
          } else {
             mc2log << "not POST request (" << inHead.getMethodString()
                    << ") setting Http 404 error.";
             if ( inHead.getStartLine() != NULL ) {
                mc2log << " Startline " << *inHead.getStartLine();
             }
             ireq->setStatusReply( HttpCode::NOT_FOUND );
          }
          mc2log << endl;
      } else if ( m_protoVer < 0xa ) {
         mc2log << warn << m_session->m_logPrefix
                << "IsabBoxInterfaceRequest::IsabBoxInterfaceRequest "
                << "too low protover 0x" << hex << int(m_protoVer) << dec 
                << " setting Http 400 error."<< endl;
         ireq->setStatusReply( HttpCode::BAD_REQUEST );
      }
   } else {
      // Not good http, error set in ireq
   }
}


IsabBoxInterfaceRequest::~IsabBoxInterfaceRequest() {
   // The m_dataSocket might be permanent, don't delete it
   reset();
}


void
IsabBoxInterfaceRequest::terminate() {
   // Check state
   if ( getState() == Ready_To_IO_Reply &&
        m_iostate == writing_reply  )
   {
      m_dataSocket->getSession()->setCallDone( 1 );
      mc2log << info << m_session->m_logPrefix
             << "IsabBoxInterfaceRequest::terminate setting Call done ("
             << m_session->getCallDone() << ")" << endl;
   }
}


Selectable* 
IsabBoxInterfaceRequest::getSelectable() {
   return m_dataSocket;
}


void
IsabBoxInterfaceRequest::handleIO( bool readyRead, bool readyWrite ) {
//     mc2dbg << "IsabBoxInterfaceRequest::handleIO( " << readyRead << ", "
//            << readyWrite << " ) io state " 
//            << getIOStateAsString( m_iostate ) << " " << this << endl;
   switch( m_iostate ) {
      case reading_stx:
      case reading_header:
         if ( readyRead ) {
            // Read start of header bytes.
            int res = m_dataSocket->read( m_startHeader + m_requestPos, 
                                          startHeaderSize - m_requestPos );
//             mc2dbg << "IsabBoxInterfaceRequest read " << res << endl;
            // Handle and set state
            if ( res > 0 ) {
               m_requestPos += res;
               if ( m_requestPos >= startHeaderSize ) {
                  // Ok, check and create m_requestBuff
                  if ( m_startHeader[ 0 ] == 0x02 ) { // STX
                     // Extract length
                     const int sizePos = 1;
                     m_requestSize = 0;
                     m_requestSize |= (m_startHeader[ sizePos     ] << 24);
                     m_requestSize |= (m_startHeader[ sizePos + 1 ] << 16);
                     m_requestSize |= (m_startHeader[ sizePos + 2 ] << 8);
                     m_requestSize |=  m_startHeader[ sizePos + 3 ];
                     m_protoVer = m_startHeader[ protoverPos ];
                     if ( m_requestSize > m_requestPos &&
                          m_requestSize <= NParamBlock::maxMsgSize )
                     {
                        // Change to reading_request_data
                        // Make m_requestBuff (m_requestSize big)
                        // Copy m_startHeader to m_requestBuff
                        m_iostate = reading_request_data;
                        m_requestBuff = new byte[ m_requestSize ];
                        memcpy( m_requestBuff, m_startHeader, 
                                startHeaderSize );
//                          mc2dbg << "IsabBoxInterfaceRequest read header"
//                                 << endl;
                     } else {
                        mc2log << warn << m_session->m_logPrefix
                               << "IsabBoxInterfaceRequest::handleIO "
                               << "bad size of request " << m_requestSize
                               << endl;
                        m_iostate = io_error;
                        setState( Error );
                        m_session->setCallDone( 2 );
                     }
                     
                  } else {
                     mc2log << warn << m_session->m_logPrefix
                            << "IsabBoxInterfaceRequest::handleIO "
                            << "not STX first aborting" << endl;
                     m_iostate = io_error;
                     setState( Error );
                     m_session->setCallDone( 2 );
                  }
               }
            } else if ( res == -2 || res == -3 ) {
               // Timeout or EAGAIN -> try again later
            } else {
               // Error / closed
               // if closed when reading first byte on nonfirst request
               // not so bad error. (protover 8 has keep-alive on socket
               // after poll_server_final)
               if ( res == 0 && m_requestPos == 0 && 
                    m_session->getUser() != NULL ) 
               {
                  m_iostate = io_done;
                  setState( Done );
                  m_session->setCallDone( 1 );
                  mc2log << info << m_session->m_logPrefix
                         << "IsabBoxInterfaceRequest::handleIO Call done ("
                         << m_session->getCallDone() << ")" << endl;
               } else {
                  mc2log << warn << m_session->m_logPrefix
                         << "IsabBoxInterfaceRequest::handleIO "
                         << "read, header, failed " << res << " = " 
                         << (res == 0 ? "closed" : "error" ) << endl;
                  m_iostate = io_error;
                  setState( Error );
                  m_session->setCallDone( 2 );
               }
            }
            
         } // end if readyRead
         break;
      case reading_request_data:
         if ( readyRead ) {
            // Read rest of msg
            int res = m_dataSocket->read( m_requestBuff + m_requestPos, 
                                          m_requestSize - m_requestPos );
            // Handle and set state
            if ( res > 0 ) {
               m_requestPos += res;
               // If read all change state etc.
               if ( m_requestPos >= m_requestSize ) {
                  // Read all!
                  m_iostate = io_done;
                  setState( Ready_To_Process );
//                    mc2dbg << "IsabBoxInterfaceRequest read body, "
//                           << "Ready_To_Process" << endl;
                  // Send the STX byte here to indicate to client
                  // that connection is ok
                  byte STX = 0x02;
                  int wres = m_dataSocket->write( &STX, 1 );
                  if ( wres != 1 ) {
                     mc2log << warn << m_session->m_logPrefix
                            << "IsabBoxInterfaceRequest::"
                            << "handleIO write of STX after read request "
                            << "failed " << wres << " = " 
                            << (wres == 0 ? "closed" : "error" ) << endl;
                     m_iostate = io_error;
                     setState( Error );
                     m_session->setCallDone( 2 );
                     res = -1;
                  } else {
                     mc2dbg8 << m_session->m_logPrefix
                             << "IsabBoxInterfaceRequest::handleIO "
                             << " sent STX ok." << endl;
                     m_replyPos = 1; // STX sent already
                  }
               } // Else read more
            } else if ( res == -2 || res == -3 ) {
               // Timeout or EAGAIN -> try again later
            } else {
               // Error / closed
               mc2log << warn << m_session->m_logPrefix
                      << "IsabBoxInterfaceRequest::handleIO "
                      << "read, request, failed " << res << " = " 
                      << (res == 0 ? "closed" : "error" ) << endl;
               m_iostate = io_error;
               setState( Error );
               m_session->setCallDone( 2 );
            }
         } // end if readyRead
         break;
      case writing_reply:
         if ( readyWrite ) {
            // Write some more
            int res = m_dataSocket->write( m_replyBuff + m_replyPos, 
                                           m_replySize - m_replyPos );
            if ( res > 0 ) {
               m_replyPos += res;
               // If sent reply
               if ( m_replyPos >= m_replySize ) {
                  // Set state etc.
                  if ( m_dataSocket->getSession()->getCallDone() ) {
                     // No more then done
                     m_iostate = io_done;
                     setState( Done );
//                      mc2dbg << "IsabBoxInterfaceRequest::handleIO "
//                             << "done writing reply and all done!"
//                             << " bytes " << m_replyPos << endl;
                     // The factory desides what to do with the socket.
//                     m_dataSocket->close();
                  } else {
                     // More read again setState( Ready_To_IO_Request )
//                      mc2dbg << "IsabBoxInterfaceRequest::handleIO "
//                             << "done writing reply going to read request"
//                             << " bytes " << m_replyPos << endl;
                     reset();
                  }
               }
            } else if ( res == -2 || res == -3 ) {
               // Timeout or EAGAIN -> try again later
            } else {
               // Error / closed
               mc2log << warn << m_session->m_logPrefix
                      << "IsabBoxInterfaceRequest::handleIO "
                      << "write, reply, failed " << res << " = " 
                      << (res == 0 ? "closed" : "error" ) << endl;
               m_iostate = io_error;
               setState( Error );
               m_session->setCallDone( 2 );
            }
         } // end if readyWrite
         break;

      case io_done:
      case io_error:
      case io_uninitalized:
         mc2log << error << m_session->m_logPrefix
                << "IsabBoxInterfaceRequest::handleIO bad IO "
                << "state " << getIOStateAsString( m_iostate ) << endl;
         break;
   };
}


void
IsabBoxInterfaceRequest::timedout() {
   mc2log << warn << m_session->m_logPrefix 
          << "IsabBoxInterfaceRequest::timedout after " << getUsedIOTime()
          << "ms when " << getStateAsString( getState() ) << endl;
   mc2dbg2 << " used IO time " << getUsedIOTime() << " timeout " 
           << getTimeout() << " start IO time " << getIOStartTime() 
           << " total timeout " << getTotalTimeout()
           << endl;
   // Darn!
   if ( getState() == Ready_To_IO_Request ) {
      m_iostate = io_error;
      setState( Timeout_request );
   } else if ( getState() == Ready_To_IO_Reply ) {
      m_iostate = io_error;
      setState( Timeout_reply );
   } else {
      mc2log << error << m_session->m_logPrefix
             << "IsabBoxInterfaceRequest::timedout not in "
             << "request or reply state " << getStateAsString( getState() )
             << endl;
      m_iostate = io_error;
      setState( Error );
   }
}


bool 
IsabBoxInterfaceRequest::isIdle() const {
   if ( getState() == Ready_To_IO_Request &&
        m_requestPos == 0 )
   {
      return true;
   } else {
      return false;
   }
}


NavMessage*
IsabBoxInterfaceRequest::getRequestMessage() {
   if ( m_navMess != NULL ) {
      return m_navMess;
   }
   if ( m_protoVer >= 0xa ) {
      mc2log << error << m_session->m_logPrefix
             << "IsabBoxInterfaceRequest::getRequestMessage called on "
             << "protoVer >= 0xa, returning NULL" << endl;
      return NULL;
   }

//    mc2dbg << "IsabBoxInterfaceRequest::getRequestMessage making mess"
//           << endl;
   // Make request from buffer
   bool ok = true;
   // "Unencrypt" buffer
   if ( m_protoVer >= 0x05 ) {
      // Apply magic byte on all body bytes

      int headerLength = isabBoxTypeMessage::getHeaderSize( 
         m_protoVer, false ); // is request
      decodeBuffer( m_requestBuff + headerLength, 
                    m_requestSize - headerLength );

      // Check CRC
      uint32 crc = isabBoxNavMessageUtil::readCRC( m_requestBuff, 
                                                   m_protoVer );
      // Set CRC bits to zero
      isabBoxNavMessageUtil::writeCRC( m_requestBuff, m_protoVer, 0 );
      // Calculate CRC
      uint32 realCrc = MC2CRC32::crc32( m_requestBuff, m_requestSize );
      if ( realCrc != crc ) {
         if ( m_protoVer > 0x08 ) {
            // Not implemented in client up to v8, and possibly more
            // so it is never right
            mc2log << warn << m_session->m_logPrefix
                   << "IsabBoxInterfaceRequest::getRequestMessage "
                   << "crc is incorrent. 0x" 
                   << hex << crc << " should be 0x" << realCrc 
                   << dec << endl;
         }
         // TODO: Set res to -1 to indicate bad packet change when
         //       client has crc.
         // Wrong CRC return error
         // ok = false;
         // set state
      } else {
         mc2dbg2 << "IsabBoxInterfaceRequest::getRequestMessage "
                 << "crc is corrent. 0x" << hex
                 << crc << dec << endl;
      }
   } // end protover >= 0x05

   // Make message
   if ( ok ) {
      // Try to make a packet out of the message.
      const char addressString[] = "10101010";
      NavAddress senderAddr( NavAddress::ADDRESS_TYPE_ISABBOX,
                                addressString );
      isabBoxTypeMessage typeMess( senderAddr, m_requestBuff, 
                                   m_requestSize, m_session);
      // Check for too new clients (unsupported versions)
      if ( typeMess.getProtoVer() > NavMessage::MAX_PROTOVER ) {
         mc2log << error << m_session->m_logPrefix
                << "IsabBoxInterfaceRequest::getRequestMessage"
                << " client has too high protoVer: " 
                << int(typeMess.getProtoVer()) 
                << " server supports max: " 
                << int(NavMessage::MAX_PROTOVER) 
                << " sending error message to client." << endl;
         // Error
         isabBoxReplyMessage* reply = new isabBoxReplyMessage( 
            senderAddr,
            typeMess.getType(),
            NavMessage::NAV_STATUS_PROTOVER_NOT_SUPPORTED,
            m_session,
            NavMessage::MAX_PROTOVER );
         reply->setReqID( typeMess.getReqID() );
         // setReplyMessage sets new state
         setReplyMessage( reply );
         m_session->setCallDone( 1 ); // No more on this
         ok = false;
      }

      if ( ok ) {
         IncomingNavMessage* navMess = NULL;
         switch ( typeMess.getType() ) {
            default:
               navMess = new IncomingNavMessage( 
                  senderAddr, 
                  NavMessage::MessageType( typeMess.getType() + 1 ),
                  false/*isRequest*/,
                  m_session );
               break;
         }
         if ( ok ) {
//            mc2dbg << "About to call convertFromBytes" << endl;
            navMess->convertFromBytes( m_requestBuff, m_requestSize );
//            mc2dbg << "convertFromBytes done" << endl;
            m_navMess = navMess;
         } else {
            // Only get here if the request was undecodable.
            m_session->setCallDone( 1 );
         }
            
      } // End if ok
   } // End if ok

   return m_navMess;
}


void
IsabBoxInterfaceRequest::setReplyMessage( OutGoingNavMessage* reply ) {
   delete m_reply;
   m_reply = reply;
   // Set reqID
   if ( m_navMess != NULL ) {
      m_reply->setReqID( m_navMess->getReqID() );
   }
   // Make reply buffer and fill it
   delete [] m_replyBuff;
   uint32 maxMessageSize = MAX( reply->getSession()->getMaxBufferLength(),
                                maxReplyMsgSize );
   if ( reply->getReplySize() > 0 && 
        reply->getReplySize() > maxMessageSize )
   {
      maxMessageSize = reply->getReplySize();
   }
   m_replyBuff = new byte[ maxMessageSize ];
   if ( m_reply->convertToBytes( m_replyBuff, maxMessageSize ) ) {
      m_replySize = m_reply->getLength();
      if ( reply->getProtoVer() >= 0x05 ) {
         // Calculate CRC
         uint32 crc = MC2CRC32::crc32( m_replyBuff, m_replySize );
         // Write CRC
         isabBoxNavMessageUtil::writeCRC( m_replyBuff, 
                                          reply->getProtoVer(), crc );
#ifdef DEBUG_LEVEL_2
         mc2log.setf( ios::uppercase );
         mc2log << info << m_session->m_logPrefix
                << "CRC for message: 0x" << hex << setw( 8 ) 
                << setfill( '0' ) << crc << dec << endl;
         mc2log.unsetf( ios::uppercase ); // Unset
#endif
         // XOR body of packet
         // Apply magic byte on all body bytes
         int headerLength = isabBoxTypeMessage::getHeaderSize( 
            reply->getProtoVer(), true ); // is reply
         encodeBuffer( m_replyBuff + headerLength, 
                       m_replySize - headerLength );
      }

      // Ready to send reply
      m_iostate = writing_reply;
      setState( Ready_To_IO_Reply );
   } else {
      mc2log << error << m_session->m_logPrefix
             << "IsabBoxInterfaceRequest::setReplyMessage convertToBytes "
             << "failed." << endl;
      m_iostate = io_uninitalized;
      setState( Error );
   }
}


void
IsabBoxInterfaceRequest::reset() {
   delete m_navMess;
   m_navMess = NULL;
   delete m_reply;
   m_reply = NULL;
   delete [] m_requestBuff;
   m_requestBuff = NULL;
   m_requestPos = 0;
   m_requestSize = 0;
   m_uncompressedRequestSize = 0;
   m_protoVer = 0;
   delete [] m_replyBuff;
   m_replyBuff = NULL;
   m_replySize = 0;
   m_replyPos = 0;
   m_uncompressedReplySize = 0;
   delete m_reqPack;
   m_reqPack = NULL;
   delete m_replyPack;
   m_replyPack = NULL;
   setState( Ready_To_IO_Request );
   m_iostate = reading_stx;
}


uint32 
IsabBoxInterfaceRequest::getRequestSize() const {
   return m_requestSize;
}

uint32
IsabBoxInterfaceRequest::getUncompressedRequestSize() const {
   return m_uncompressedRequestSize;
}

uint32 
IsabBoxInterfaceRequest::getReplySize() const {
   return m_replySize;
}

uint32
IsabBoxInterfaceRequest::getUncompressedReplySize() const {
   return m_uncompressedReplySize;
}

NavRequestPacket*
IsabBoxInterfaceRequest::getRequestPacket() {
   if ( m_reqPack != NULL ) {
      return m_reqPack;
   }
   if ( m_requestSize < navPacketheaderLength + 4/*CRC*/ ) {
      mc2log << warn << m_session->m_logPrefix
             << "IsabBoxInterfaceRequest::getRequestPacket to short "
             << "packet " << m_requestSize << " bytes, returning NULL" 
             << endl;
      return NULL;
   }
   if ( m_protoVer < 0xa ) {
      mc2log << error << m_session->m_logPrefix
             << "IsabBoxInterfaceRequest::getRequestPacket called on "
             << "protoVer < 0xa, returning NULL" << endl;
      return NULL;
   }
   if ( m_requestBuff[ 0 ] != 0x02 ) { // STX
      mc2log << warn << m_session->m_logPrefix
             << "IsabBoxInterfaceRequest::getRequestPacket not STX "
             << "first, returning NULL" << endl;
      return NULL;
   }

   bool ok = true;

   // Check for too new clients (unsupported versions)
   if ( ok && m_protoVer > NavPacket::MAX_PROTOVER ) {
      mc2log << error << m_session->m_logPrefix
             << "IsabBoxInterfaceRequest::getRequestPacket"
             << " client has too high protoVer: 0x" << hex
             << int(m_protoVer) 
             << " server supports max: 0x" 
             << int(NavPacket::MAX_PROTOVER) << dec
             << " sending error message to client." << endl;
         // Error
      ok = false;
      NavReplyPacket* reply = new NavReplyPacket( 
         m_protoVer, 0, 0, 0, 
         NavReplyPacket::NAV_STATUS_PROTOVER_NOT_SUPPORTED,
         "Protover too high" /*StringTable:: but no lang here*/ );
      // setReplyMessage sets new state
      setReplyPacket( reply );
      m_session->setCallDone( 1 ); // No more on this
   }

   if ( ok ) {
      // "Unencrypt" buffer
      decodeBuffer( m_requestBuff + navPacketheaderLength, 
                    m_requestSize - navPacketheaderLength );
      
      // Check CRC
      int pos = m_requestSize - 4;
      uint32 crc = isabBoxNavMessageUtil::incReadLong( 
         m_requestBuff, pos );

      // Calculate CRC
      uint32 realCrc = MC2CRC32::crc32( m_requestBuff, 
                                        m_requestSize - 4/*crc*/ );
      if ( realCrc != crc ) {
         mc2log << warn << m_session->m_logPrefix
                << "IsabBoxInterfaceRequest::getRequestPacket "
                << "crc is incorrent. 0x" 
                << hex << crc << " should be 0x" << realCrc 
                << dec << endl;
         ok = false;
         NavReplyPacket* reply = new NavReplyPacket( 
            m_protoVer, 0, 0, 0, NavReplyPacket::NAV_STATUS_CRC_ERROR,
            "CRC incorrect" /*StringTable:: but no lang here*/ );
         // setReplyMessage sets new state
         setReplyPacket( reply );
         m_session->setCallDone( 1 ); // No more on this
      }
   }

   // Make packet
   if ( ok ) {
      int pos = 6;
      uint16 type = isabBoxNavMessageUtil::incReadShort( 
         m_requestBuff, pos );
      byte reqID  = m_requestBuff[ 8 ];
      byte reqVer = m_requestBuff[ 9 ];

      m_reqPack = new NavRequestPacket( 
         m_protoVer, type, reqID, reqVer, 
         m_requestBuff + navPacketheaderLength, 
         m_requestSize - navPacketheaderLength - 4/*crc*/,
         &m_uncompressedRequestSize );
      m_uncompressedRequestSize += navPacketheaderLength + 4;
      if ( !m_reqPack->getParamBlock().getValid() ) {
         mc2log << warn << m_session->m_logPrefix
                << "IsabBoxInterfaceRequest::getRequestPacket "
                << "ParamBlock invalid." << endl;
         ok = false;
         NavReplyPacket* reply = new NavReplyPacket( 
            m_protoVer, type, reqID, reqVer, 
            NavReplyPacket::NAV_STATUS_PARAMBLOCK_INVALID,
            "ParamBlock invalid" /*StringTable:: but no lang here*/ );
         // setReplyMessage sets new state
         setReplyPacket( reply );
         m_session->setCallDone( 1 ); // No more on this
         delete m_reqPack;
         m_reqPack = NULL;
      }
   }

   return m_reqPack;
}


void 
IsabBoxInterfaceRequest::setReplyPacket( NavReplyPacket* reply ) {
   delete m_replyPack;
   m_replyPack = reply;

   vector< byte > buff;

   bool mayUseGzip = false;
   if ( m_reqPack != NULL ) {
      mayUseGzip = m_reqPack->getParamBlock().mayUseGzip();
   }
   m_replyPack->writeTo( buff, mayUseGzip, &m_uncompressedReplySize );

   // Make reply buffer
   m_replySize = buff.size();
   delete [] m_replyBuff;
   m_replyBuff = new byte[ buff.size() ];
   memcpy( m_replyBuff, &buff.front(), buff.size() );
   // Yes status code/message is encoded, so no plaintext is seen.

   // "Encrypt" body
   encodeBuffer( m_replyBuff + navPacketheaderLength,
                 m_replySize - navPacketheaderLength );

   // Ready to send reply
   m_iostate = writing_reply;
   setState( Ready_To_IO_Reply );
}


const char* 
IsabBoxInterfaceRequest::getIOStateAsString( io_state_t state ) {
   switch ( state ) {
      case reading_stx:
         return "reading_stx";
      case reading_header:
         return "reading_header";
      case reading_request_data:
         return "reading_request_data";
      case writing_reply:
         return "writing_reply";
      case io_done:
         return "done";
      case io_error:
         return "error";
      case io_uninitalized:
         return "uninitalized";
   }

   return "Unknown state";
}


void
IsabBoxInterfaceRequest::decodeBuffer( byte* buff, uint32 len ) {
#ifndef USE_MAGICBYTE
   uint32 magicPos = 0;
#endif
   for ( uint32 i = 0 ; i < len ; i++ ) {
#ifdef USE_MAGICBYTE
      buff[ i ] = buff[ i ] ^ MAGICBYTE;
#else
      buff[ i ] = buff[ i ] ^ MAGICBYTES[ magicPos ];
      magicPos++;
      if ( magicPos >= MAGICLENGTH ) {
         magicPos = 0;
      }
#endif
   }
}


void 
IsabBoxInterfaceRequest::encodeBuffer( byte* buff, uint32 len ) {
   // XOR is the same
   decodeBuffer( buff, len );
}
