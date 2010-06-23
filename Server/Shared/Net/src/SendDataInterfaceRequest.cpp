/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SendDataInterfaceRequest.h"
#include "SSLSocket.h"
#include "TCPSocket.h"
#include "SocketBuffer.h"
#include "Utility.h"
#include "StringUtility.h"
#include <regex.h>

#undef DEBUG

// Constants
uint32 SendDataInterfaceRequest::MAX_SOCKET_WAITTIME     = 60000000;


SendDataInterfaceRequest::SendDataInterfaceRequest( 
   const URL& peer, const byte* data, uint32 dataLen,
   const char* expectedReplyData,
#ifdef USE_SSL
   SSL_CONTEXT* SSLContext,
#endif
   uint32 queueTimeout
   )
      : SocketInterfaceRequest(),
        m_iostate( io_uninitalized ),
        m_requestBuffer( NULL ), m_requestSize( dataLen ),
        m_requestPos( 0 ), m_peer( peer ), m_queueTimeout( queueTimeout ),
        m_expectedReplyData( expectedReplyData )
{
   // Set timeouts
   m_timeout = MAX_SOCKET_WAITTIME / 1000; // ms
   m_totalTimeout = MAX_SOCKET_WAITTIME / 1000 / 100 / 2; // s

   // Logprefix
   const char randChars[80] = 
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
   snprintf( m_logPrefix, 255, "[%c%c%c] [%s:%d;%d] ", 
             randChars[(int)((strlen(randChars)+0.0)*rand()/(RAND_MAX+1.0))],
             randChars[(int)((strlen(randChars)+0.0)*rand()/(RAND_MAX+1.0))],
             randChars[(int)((strlen(randChars)+0.0)*rand()/(RAND_MAX+1.0))],
             peer.getHost(), peer.getPort(), m_requestSize );

   if ( MC2String( "ssl" )   == peer.getProto() || 
        MC2String( "https" ) == peer.getProto() ) {
#ifdef USE_SSL
      m_reqSock = new SSLSocket( SSLContext );
#else
      m_reqSock = new TCPSocket();
#endif
   } else {
      m_reqSock = new TCPSocket();
   }
   m_sockBuff = new SocketBuffer( m_reqSock );

   m_requestBuffer = new byte[ m_requestSize + 1 ];
   memcpy( m_requestBuffer, data, m_requestSize );
   reset();
}

SendDataInterfaceRequest::~SendDataInterfaceRequest() {
   delete m_sockBuff;
   delete m_reqSock;
   delete [] m_requestBuffer;
   m_requestBuffer = NULL;
   m_requestSize = 0;
}

void
SendDataInterfaceRequest::terminate() {
   // Nothing for now
}

int
SendDataInterfaceRequest::getPriority( const InterfaceHandleIO* g ) const {
   return SocketInterfaceRequest::getPriority( g );
}

void
SendDataInterfaceRequest::handleIO( bool readyRead, bool readyWrite ) {
   // No invitos here (Guess the movie)
   m_invIO = false;

   if ( m_iostate == initial_hand_shake ) {
      int res = m_reqSock->initialHandshaking();
      if ( res > 0 ) {
         // Ok done 
         m_iostate = writing_request;
      } else if ( res == -4 ) {
         // Want read
         m_invIO = true;
         return;
      } else if ( res == -5 ) {
         // Want write
         return;
      } else {
         mc2log << warn << m_logPrefix
                << " handleIO initialHandshake failed " << res 
                << " Peer " << m_peer << endl;
         m_iostate = io_error;
         setState( Error );
         return;
      }
   }

   switch( m_iostate ) {
      case writing_request:
         if ( readyWrite || (readyRead && m_invIO) ) {
            // Write some more
            int res = m_reqSock->write( m_requestBuffer + m_requestPos, 
                                        m_requestSize - m_requestPos );
            if ( res > 0 ) {
               m_requestPos += res;
               // If sent reply
               if ( m_requestPos >= m_requestSize ) {
                  // Set state etc.
                  m_iostate = reading_reply;
                  setState( Ready_To_IO_Request );
               }
            } else if ( res == -2 || res == -3 ) {
               // Timeout or EAGAIN -> try again later
            } else if ( res == -4 ) {
               // Write wants read
               m_invIO = true;
            } else {
               // Error / closed
               mc2log << warn << m_logPrefix << " handleIO "
                      << "write, request, failed " << res << " = " 
                      << (res == 0 ? "closed" : "error" ) 
                      << " Peer " << m_peer << endl;
               m_iostate = io_error;
               setState( Error );
#ifdef DEBUG
               // Debug
               dumpState( mc2log );
#endif
            }
         } // End if ok to call write
         break;

      case reading_reply:
         if ( readyRead || (readyWrite && m_invIO) ) {
            // Read some more
            const uint32 buffSize = 4096;
            byte buff[ buffSize ];
            int res = 1;
            while ( res > 0 && m_iostate == reading_reply ) {
               res = m_reqSock->read( buff, buffSize );
               if ( res > 0 ) {
                  m_replyBytes.insert( m_replyBytes.end(), buff, buff + res );
               } else if ( res == -2 || res == -3 ) {
                  // Timeout or EAGAIN -> try again later
               } else if ( res == -4 ) {
                  // Write wants read
                  m_invIO = true;
               } else {
                  if ( res == 0 ) {
                     m_iostate = io_done;
                     vector<byte> replyBytes = getReplyBytes();
                     replyBytes.push_back( '\0' );
                     const char* replyData = reinterpret_cast<char*>( 
                        &replyBytes.front() );
                     int c = (REG_EXTENDED|REG_ICASE);
                     if ( StringUtility::regexp( m_expectedReplyData.c_str(),
                                                 replyData, c, 0 ) ) {
                        setState( Done );
                     } else {
                        mc2log << warn << m_logPrefix << " handleIO "
                               << "reply doesn't match "
                               << " expected " << m_expectedReplyData.c_str()
                               << " Got " << replyData << endl;
                        setState( Error );
                     }
                  } else {
                     mc2log << warn << m_logPrefix << " handleIO "
                            << "read, reply, failed " << res << " = " 
                            << (res == 0 ? "closed" : "error" ) 
                            << " Peer " << m_peer << endl;
                     m_iostate = io_error;
                     setState( Error );
                  }
               }
            } // End while res > 0 and iostate is reading reply
         } // End if ok to call read
         break;

      case connecting:
      case initial_hand_shake:
      case io_done:
      case io_error:
      case io_uninitalized:
         mc2log << error << m_logPrefix<< " handleIO bad IO "
                << "state " << getIOStateAsString( m_iostate ) 
                << " Peer " << m_peer << endl;
         break;
   } // End switch m_iostate


   
}

void
SendDataInterfaceRequest::timedout() {
   // Darn!
   mc2log << warn << m_logPrefix << "timedout state " 
          << getStateAsString( getState() ) << " Used IO time "
          << getUsedIOTime() << "ms of " << getTimeout() << "ms "
          << "Total IO time " << getTotalUsedIOTime() << "ms "
          << "Start time " << getIOStartTime()
          << " Peer " << m_peer 
          << endl;
   if ( getState() == Ready_To_IO_Request ) {
      m_iostate = io_error;
      setState( Timeout_request );
   } else if ( getState() == Ready_To_IO_Reply ) {
      m_iostate = io_error;
      setState( Timeout_reply );
   } else {
      mc2log << error << m_logPrefix
             << "timedout not in "
             << "request or reply state " << getStateAsString( getState() )
             << " Peer " << m_peer << endl;
      m_iostate = io_error;
      setState( Error );
   }
}

void
SendDataInterfaceRequest::handleOverloaded( int overLoad ) {
   reset();
   m_iostate = io_error;
   setState( Error );
}

bool
SendDataInterfaceRequest::isIdle() const {
   return false;
}

void
SendDataInterfaceRequest::reset() {
   m_requestPos = 0;
   m_invIO = false;
   m_replyBytes.clear();
   bool socketOk = true;
   if ( m_reqSock->getState() != TCPSocket::CLOSED &&
        m_reqSock->getState() != TCPSocket::UNKNOWN ) {
      m_reqSock->close();
   }
   if ( ! m_reqSock->open() ) {
      mc2log << m_logPrefix << " Couldn't create socket." << endl;
      socketOk = false;
   }

   // Set non blocking
   m_reqSock->setBlocking( false );

   if ( socketOk && 
        ! m_reqSock->connect( m_peer.getHost(), m_peer.getPort() ) )
   {
      mc2log << m_logPrefix << " Couldn't connect to " << m_peer.getHost() 
             << ":" << m_peer.getPort() << endl;
      socketOk = false;
   }

   if ( socketOk ) {
      setState( Ready_To_IO_Reply );
      m_iostate = initial_hand_shake;
   } else {
      setState( Error );
      m_iostate = io_error;
   }
}

void
SendDataInterfaceRequest::setMaxSocketWaitTime( uint32 val ) {
   MAX_SOCKET_WAITTIME = val;   
}

ostream&
SendDataInterfaceRequest::dumpState( ostream& out ) const {
   out << "SendDataInterfaceRequest state:" << endl;
   out << " IOState " << getIOStateAsString( m_iostate ) << endl;
   
   if ( m_requestBuffer ) {
      Utility::hexDump( out, m_requestBuffer, m_requestSize, 
                        " requestBuffer " );
   } else {
      out << " No requestBuffer " << endl;
   }
   out << " requestSize " << m_requestSize << endl;
   out << " requestPos " << m_requestPos << endl;
   out << " invIO " << m_invIO << endl;
   out << " peer " << m_peer << endl;
   out << " logPrefix " << m_logPrefix << endl;

   return out;
}

const char*
SendDataInterfaceRequest::getIOStateAsString( io_state_t state ) {
   switch( state ) {
      case connecting:
         return "connecting";
      case initial_hand_shake:
         return "initial_hand_shake";
      case writing_request:
         return "writing_request";
      case reading_reply:
         return "readin_reply";
      case io_done:
         return "io_done";
      case io_error:
         return "io_error";
      case io_uninitalized:
         return "io_uninitalized";
   }

   return "Unknown state";
}
