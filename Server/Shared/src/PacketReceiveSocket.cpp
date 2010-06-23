/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PacketReceiveSocket.h"

#include <vector>

PacketReceiveSocket::PacketReceiveSocket()
      : TCPSocket(), m_readState( reading_size ), 
        m_size( 0 ), m_readSize( 0 ), m_buffer( NULL )
        
{
}


PacketReceiveSocket::~PacketReceiveSocket() {
   delete [] m_buffer;
}


TCPSocket*
PacketReceiveSocket::accept() {
   SOCKET tmpsock;
   PacketReceiveSocket* newSock = NULL;

   if ( (tmpsock = ::accept(m_sock, (struct sockaddr *)0, 0)) == 
        SOCKET_ERROR ) 
   {
      if ( errno == EAGAIN ) {
         // Not any sockets in queue, try again after a select
         mc2dbg2 << "PacketReceiveSocket::accept() EAGAIN: "
                 << "No socket right now." << endl;
      } else {
         mc2log << error << "PacketReceiveSocket::accept() failure: " 
                << strerror( errno ) << endl;
      }
      return NULL;
   }

   newSock = new PacketReceiveSocket( tmpsock );
   // Set non blocking
   newSock->setBlocking( false );

   return newSock;
}


PacketReceiveSocket::PacketReceiveSocket( SOCKET sock, int backlog ) 
      : TCPSocket( sock, backlog ), m_readState( reading_size ), 
        m_size( 0 ), m_readSize( 0 ), m_buffer( NULL )
{
}


byte*
PacketReceiveSocket::getBuffer( bool steal ) {
   byte* res = m_buffer;

   if ( steal ) {
      m_buffer = NULL;
   }

   return res;
}


void
PacketReceiveSocket::readBytes() {
   int res = 1;

   while ( res > 0 ) {
      switch( m_readState ) {
         case reading_size : {
            res = read( (byte*)(&m_sizeBuf) + m_readSize, 4 - m_readSize );
            if ( res > 0 ) {
               m_readSize += res;
               if ( m_readSize >= 4 ) {
                  // Change the byte ordering of packetLength
                  m_size = ntohl( m_sizeBuf );
                  m_readSize = 0;
                  m_readState = reading_packet;
                  // TODO: Size check 
                  m_buffer = new byte[ m_size ];
               }
            } else if ( res == -2 || res == -3 ) {
               // Timeout or EAGAIN -> try again later
            } else {
               mc2log << warn << "PacketReceiveSocket:readBytes: state "
                      << "reading_size read failed " << res << " = " 
                      << (res == 0 ? "closed" : "error" ) << endl;
               m_readState = error;
            }
         } break;
         
         case reading_packet : {
            res = read( m_buffer + m_readSize, m_size - m_readSize );
            if ( res > 0 ) {
               m_readSize += res;
               if ( m_readSize >= m_size ) {
                  // Done
                  m_readState = done;
                  res = 0;
               }
            } else if ( res == -2 || res == -3 ) {
               // Timeout or EAGAIN -> try again later
            } else {
               mc2log << warn << "PacketReceiveSocket:readBytes: state "
                      << "reading_packet read failed " << res << " = " 
                      << (res == 0 ? "closed" : "error" ) << endl;
               m_readState = error;
            }
         } break;
         
         case done : {
            mc2log << error << "PacketReceiveSocket::readBytes "
                   << "called in state done" << endl;
            res = 0;
         } break;
         
         case error : {
            mc2log << error << "PacketReceiveSocket::readBytes "
                   << "called in state error" << endl;
            res = 0;
         } break;
      }

   } // End while res > 0 
}

