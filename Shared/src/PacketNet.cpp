/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PacketNet.h"

#include "Packet.h"
#include "TCPSocket.h"
#include "IPnPort.h"
#include "DebugClock.h"
#include "DataBuffer.h"
#include "ScopedArray.h"
#include "URL.h"

#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

namespace PacketUtils {

bool sendViaTCP( const Packet& packet,
                 const IPnPort& destination,
                 uint32 timeoutMicros ) 
{
   mc2dbg8 << "[PacketUtils::sendViaTCP] timeout = " << timeoutMicros << endl;

   IPnPort dest( destination );
   if ( dest == IPnPort( 0, 0 ) ) {
      dest = packet.getOriginAddr();
   }

   // still invalid addr?
   if ( dest == IPnPort( 0, 0 ) ) {
      mc2log << warn << "[PacketUtils::sendViaTCP]: address is zero" << endl;
      return false;
   }

   TCPSocket sock;
   sock.open();

   DebugClock connectClock;
   mc2dbg << "Trying to connect to: " << dest << endl;
   if ( sock.connect( dest, 1000 ) ) { 
      mc2dbg << "Connect done." << endl;
      if ( connectClock.getTime() > 1000 ) {
         mc2log << warn << "[Packet]: Took " << connectClock
                << " to connect to "
                << dest << " in sendViaTCP" << endl;
      }
      sock.setBlocking( false );
      return sendViaTCP( packet, sock, timeoutMicros );
   } else {
      // Failed to connect
      mc2log << warn << "[PacketUtils::sendviaTCP] failed to connect to "
             << dest << endl;
   }

   return false;
}

bool sendViaTCP( const Packet& pack,
                 TCPSocket& sock,
                 uint32 timeoutMicros ) {

   mc2dbg8 << "[PacketNet]: 2 sendViaTCP timeout = " << timeoutMicros << endl;
   if ( timeoutMicros == MAX_UINT32 ) {
      timeoutMicros = TCPSOCKET_DEFAULT_TIMEOUT;
   }  
   
   // Should not send LEADERIP_REQ via tcp
   // Beware that packets can be sent using TCP in other ways
   // too, so this check does not cover all cases.
   if ( pack.getSubType() == Packet::PACKETTYPE_LEADERIP_REQUEST ) {
      mc2log << warn << "[Packet]: Sending LEADERIP_REQ via TCP"
             << endl;
   }
   // The socket must be connected before calling this method!!!
   DataBuffer sizeBuff(4);
   int length = pack.getLength();
   sizeBuff.writeNextLong(length);

   DebugClock sendTime;


#ifdef USE_CORK
#ifdef __linux__
{
   mc2dbg8 << "[Packet]: Corking socket" << endl;
   int on = 1;
   if (setsockopt(sock.getSOCKET(),
                  SOL_TCP,
                  TCP_CORK, &on, sizeof(on)) < 0 ) {
      mc2log << warn << "TCPSocket::setupSocket, failed to set TCP_CORK; "
              << strerror(errno) << endl;
   }
}
#endif
#endif
   
   // Send the packetlength

   byte* curByte = sizeBuff.getBufferAddress();
   int bytesLeft = 4;
   while ( bytesLeft ) {
      int written = sock.write( curByte, bytesLeft, timeoutMicros );
      if ( written <= 0 ) {
         if ( written != 0 ) {
            mc2log << warn << "[Packet]: failed to send packet length via TCP"
                   << endl;
         }
         return false;
      }
      bytesLeft -= written;
      curByte   += written;
    }

   // Send the packet buffer
   bytesLeft = length;
   curByte = pack.getBuf();
   while ( bytesLeft ) {
      int written = sock.write( curByte, bytesLeft, timeoutMicros );
      if ( written <= 0 ) {
         if ( written != 0 ) {
            mc2log << warn << "[Packet]: failed to send packet body via TCP"
                   << endl;
         }
         return false;
      }
      bytesLeft -= written;
      curByte   += written;
   }
#ifdef USE_CORK
#ifdef __linux__
{
   mc2dbg8 << "[Packet]: Uncorking socket" << endl;
   int on = 0;
   if (setsockopt(sock.getSOCKET(),
                  SOL_TCP,
                  TCP_CORK, &on, sizeof(on)) < 0 ) {
      mc2log << warn << "TCPSocket::setupSocket, failed to set TCP_CORK; "
              << strerror(errno) << endl;
   }
}
#endif
#endif
   // All went well.
   if ( sendTime.getTime() > 1000 ) {
      mc2log << warn << "[Packet]: Took " << sendTime 
             << " ms to send packet of length " << length << endl;
   }

   return true;
}

Packet*
receivePacket( TCPSocket& socket, uint32 timeout ) 
{

   const int lengthSizeTCP = 4;
   uint32 packetSize = 0;
   
   const int bufferLength = 
      socket.readExactly( (byte*)&packetSize, lengthSizeTCP, 
                          timeout );
   if ( bufferLength != lengthSizeTCP ) {
      IPnPort addr;
      socket.getPeerName( addr.first, addr.second );
      if ( bufferLength == -2 ) {
         mc2log << warn << "[PacketUtils::receivePacket] could not read "
                << "packetSize from: " << addr 
                << ", error = " << strerror( errno ) << endl;
      } else {
         mc2log << warn << "[PacketUtils::receivePacket] timeout while "
                << "reading packet length from " << addr << endl;
      }
      socket.close();
      return NULL;
   }

   ScopedArray<byte> buffer( MAKE_UINT32_ALIGNED_BYTE_BUFFER( bufferLength ) );
   
   ssize_t readSize = socket.readExactly( buffer.get(), bufferLength, 
                                          timeout );
   if ( readSize != bufferLength ) {
      IPnPort addr;
      socket.getPeerName( addr.first, addr.second );

      mc2log << warn << "[PacketUtils::receivePacket] failed to read packet "
             << "data from " << addr 
             << ", error = " << strerror( errno ) << endl;

      socket.close();

      return NULL;
   }

   return Packet::makePacket( buffer.release(), bufferLength );
}

void saveURL( Packet& p, int& pos, const URL& url ) {
   p.incWriteString( pos, url.getSpec() );
}

URL loadURL( const Packet& p, int& pos ) {
   return URL( p.incReadString( pos ) );
}

}
