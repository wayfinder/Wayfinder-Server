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

#include "IPnPort.h"
#include "DatagramSocket.h"
#include "NetUtility.h"
#include "Properties.h"

#include <stdlib.h>

#ifdef __linux
// Defines __u32 and __u8 used by errqueue
#include <asm/types.h>
// Msg error queue used by recvmsg
#include <linux/errqueue.h>
#endif

// ========================================================================
//                                                         DatagramSocket =

#ifndef _MSC_VER
// Not Visual Studio. Will crash if getprotobyname returns NULL.
// But the function is not threadsafe so we will get much stranger
// errors if we do not do this.
int DatagramSocket::c_udp_proto_nbr =
         (getprotobyname("udp") ?
          getprotobyname("udp")->p_proto :
          -1 );
#else
int DatagramSocket::c_udp_proto_nbr = -1;
#endif

DatagramSocket::DatagramSocket()
{
   if ( c_udp_proto_nbr <= 0 ) {
      // Not initialized, probably windows.
      c_udp_proto_nbr = getprotobyname("udp")->p_proto;

      // It didn't work here either.
      if ( c_udp_proto_nbr <= 0 ) {
         PANIC("Couldn't get protocol entry for protocol 'udp'", "");
      }
   }
   
   if ( ( fd = socket(AF_INET, SOCK_DGRAM, c_udp_proto_nbr) ) < 0 ) {
      mc2log << error
             <<"DatagramSocket::DatagramSocket() error creating socket"
             << endl << "FD " << fd << endl << " proto_ent->p_proto "
             << c_udp_proto_nbr << endl;
      PANIC("__socket: ", strerror(errno));
   }
   
   DEBUG_VERBOSE(mc2dbg << "Socket created, filedescriptor: " << fd << endl);
}


DatagramSocket::~DatagramSocket()
{
   int res = 0;
#ifndef __MSC_VER
   res = ::close(fd); 
#else
   res = ::closesocket(fd);
#endif
   if (res != 0) {
      mc2log << error
             << "DatagramSocket::~DatagramSocket() error - ::close returned "
             << res << endl;
      if (res < 0) {
         perror("~DatagramSocket");
      }
   }
}


// ========================================================================
//                                                         DatagramSender =

DatagramSender::DatagramSender() 
   : DatagramSocket()
{
   const int wmem = Properties::getUint32Property( 
      "SOCKET_SNDBUF_SIZE", 
      262144/*256 kB is a small size for MC2, linux default is max 128kB*/ );
   const socklen_t length = sizeof(wmem);
#ifdef _MFC_VER
   int r = setsockopt(fd, IPPROTO_IP, SO_SNDBUF, (char *)&wmem, length);
#else
   int r = setsockopt(fd, IPPROTO_IP, SO_SNDBUF, (void *)&wmem, length);
#endif

//   DEBUG8(
   if (r<0){
      mc2dbg1 << "DataGramSender(): __setsockopt(SO_SNDBUF): " 
              << strerror(errno) << endl;
   }
   /* FIXME: The buffer-length should be set -- but doesn't work now...

   cout << "Set: ttl=" << ttl << ", length=" << length << endl;

   r = getsockopt(fd, IPPROTO_IP, SO_SNDBUF, (void *)&ttl, &length);
   if (r<0)
      cout << "FEL!" << strerror(errno) << endl;
   cout << "Get: ttl=" << ttl << ", length=" << length << endl;
   */
}

DatagramSender::~DatagramSender()
{
}

bool
DatagramSender::send(const Packet *packet, uint32 ip, uint16 port )
{
	mc2dbg8 << "Send, ReqID: " << packet->getRequestID() 
           << ", PacketID: " << packet->getPacketID() << endl
           << " Length: " << packet->getLength() << endl ;
   DEBUG8(packet->dump( true ); );

   uint32 pos = 0;
   uint32 nbrPackets = 0;
   while ( pos < packet->getLength() ) {
      pos += Packet::MAX_UDP_PACKET_SIZE;
      if ( nbrPackets != 0 ) {
         pos -= HEADER_SIZE;
      }
      nbrPackets++;
   }
   // Should be sent by tcp if too large.
   MC2_ASSERT( nbrPackets == 1 );
   
   pos = 0;
   ((Packet*)packet)->setNbrPackets( nbrPackets );   
   for ( uint32 packetNbr = 0 ; packetNbr < nbrPackets ; packetNbr++ ) {
      mc2dbg8 << "PacketNbr: " << packetNbr << " of " << nbrPackets
              << endl;
      ((Packet*)packet)->setPacketNbr( packetNbr );   
      if ( packetNbr == 0 ) {
         mc2dbg8 << "send( " << pos << ", "
                 <<  MIN( Packet::MAX_UDP_PACKET_SIZE,
                          packet->getLength() - pos ) << " )" << endl; 
         if ( !send( packet->getBuf() + pos, 
                     MIN( Packet::MAX_UDP_PACKET_SIZE, 
                          packet->getLength() - pos ),
                     ip, port ) )
         {
            return false;
         }
      } else {
//#ifdef UDP_SEND_DELAY
         // If we don't wait packet is lost, sometimes
         usleep(100);
//#endif         
         if ( !send( packet->getBuf(), packet->getBuf() + pos, 
                     MIN( Packet::MAX_UDP_PACKET_SIZE - HEADER_SIZE, 
                          packet->getLength() - pos ),
                     ip, port ) ) 
         {
            return false;
         }
         pos -= HEADER_SIZE; // MAX_UDP_PACKET_SIZE - HEADER_SIZE is sent
      }
      pos += Packet::MAX_UDP_PACKET_SIZE;
   }

   mc2dbg8 << "Send done " << endl;
   return true;
}

bool
DatagramSender::send( const Packet* p, const IPnPort& addr )
{
   return send(p, addr.getIP(), addr.getPort());
}

bool 
DatagramSender::send( byte* header, byte* data, uint32 dataSize, 
                      uint32 ip, uint16 port )
{
   byte* buffer = new byte[ HEADER_SIZE + dataSize ];

   memcpy( buffer, header, HEADER_SIZE );
   memcpy( buffer + HEADER_SIZE, data, dataSize );
   bool sendRes = send( buffer, (HEADER_SIZE + dataSize), ip, port );
   delete [] buffer;
   return sendRes;
}

bool 
DatagramSender::send( byte* buffer, uint32 buffSize, 
                      uint32 ip, uint16 port ) 
{
   struct sockaddr_in addr;

   memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(ip);
   addr.sin_port = htons(port);

   if (sendto( fd,
               (char *)buffer,
               buffSize,
               0,
               (struct sockaddr *)&addr,
               sizeof(addr)   ) < 0)
   {
#ifdef SINGLE_VERSION
      int nbrRetries = 4;
      while ( (errno == ECONNREFUSED || errno == EINTR)
              && (nbrRetries > 0) ) { // Try again
         DEBUG4(
            if ( errno == ECONNREFUSED ) {
               mc2dbg << "DatagramSender::send SendTo ECONNREFUSED, retry"
                      << endl;
            } else if ( errno == EINTR ) {
               mc2dbg << "DatagramSender::send SendTo EINTR, retry"
                      << endl;
            }
         ); // DEBUG
         usleep(100);
         if (sendto( fd,
                     (char *)buffer,
                     buffSize,
                     0,
                     (struct sockaddr *)&addr,
                     sizeof(addr)   ) == 0) 
         {
            mc2dbg4 << "DatagramSender::send SendTo sent!" << endl;
            return true;
         } else {
            mc2dbg4 << "DatagramSender Sendto: " 
                    << strerror(errno) << endl;
         }
         nbrRetries--;
      }
#endif
      mc2dbg2 << "__sendto: " << strerror(errno) 
              << " sending to ip = " << ip << ", port = " << port 
              << endl;
      return false;
   }


   return true;
}


// ========================================================================
//                                                       DatagramReceiver =

DatagramReceiver::~DatagramReceiver()
{
}

DatagramReceiver::DatagramReceiver(uint16 port, PortChoice pc) 
   : DatagramSocket()
{
   const int rmem = Properties::getUint32Property( 
      "SOCKET_RCVBUF_SIZE", 
      262144/*256 kB is a small size for MC2, linux default is max 128kB*/ );
   const socklen_t length = sizeof(rmem);
#ifdef _MFC_VER
   int r = setsockopt(fd, IPPROTO_IP, SO_RCVBUF, (char *)&rmem, length);
#else
   int r = setsockopt(fd, IPPROTO_IP, SO_RCVBUF, (void *)&rmem, length);
#endif
   if (r<0){
      mc2log << warn << "DatagramReceiver(): __setsockopt(SO_RCVBUF): " 
              << strerror(errno) << endl;
   }

   int nbrTries = 1;

   if (pc == REUSEADDR ) {
      /* Allow several ports to listen on the same socket.
       * Specially usable if several processes on the same computer
       * is listening to a multicastaddress */
      unsigned int one = 1;
      if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                     (char *)&one, sizeof(one)) < 0) {
         PANIC("DatagramReceiver::DatagramReceiver -> __setsockopt: ",
               strerror(errno));
      }   
   } else if (pc == FINDFREEPORT) {
      nbrTries = NetUtility::MAX_NBR_TRIES_PORTFIND;
   }

   while (nbrTries > 0) {
      mc2dbg8 << "   DatagramReciever: Trying port = " << port << endl;

      struct sockaddr_in addr;
      memset(&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = htonl(INADDR_ANY);
      addr.sin_port = htons(port);
      errno = 0;
      if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
#ifdef _MFC_VER
         bool eadrinuse = true;         
#else
         bool eadrinuse = (errno == EADDRINUSE);
#endif
         if ( eadrinuse && (FINDFREEPORT) ) {
            
            // increase the portnumber and try again!
            port++;
            nbrTries--;
            mc2dbg8 << "   DatagramReciever: Port already in use "
                    << " trying the next one (" << port << ")" << endl;
         } else {
            PANIC("DatagramReceiver: bind: ", strerror(errno));
         }
      } else {         
         // Done -- port OK!
         nbrTries = -1;
      }
   }
   
   if (nbrTries == 0) {
      mc2dbg8 << "__bind: No free port found!" << endl; 
   }

   this->port = port;

   // Clear peer address
   memset(&m_peerAddr, 0, sizeof(m_peerAddr));

   mc2dbg8 << "   DatagramReceiver: fd = " << fd 
           << ", port " << port << endl;
}


bool
DatagramReceiver::receive(Packet *packet)
{
	if (packet == NULL) {
		mc2log << error << "DatagramReceiver::receive no packet" << endl;
      return (false);
   }
   
   int length = receive( packet->getBuf(), packet->getBufSize() );

   if ( length < 0 ) {
      packet->setLength( 0 );
      return false;
   } else {
      packet->setLength( length );
      return true;
   }
}


int32
DatagramReceiver::receive( byte* buff, uint32 buffSize ) 
{
   int length;

   // Clear peer address
   uint32 addrlen = sizeof(m_peerAddr);
   memset(&m_peerAddr, 0, addrlen);

#ifdef EINTR
   do {
#endif // EINTR
#ifndef _MSC_VER
      errno = 0;
      length = recvfrom(fd, (char *)buff,
                        buffSize, 0,
                        (struct sockaddr *)&m_peerAddr, &addrlen);
#else
      length = recvfrom(fd, (char *)buff,
                        buffSize, 0,
                        (struct sockaddr *)&m_peerAddr, (int*)&addrlen);
      if ( length < 0 && errno == EINTR )
         mc2log << warn << "DatagramReceiver::receive - got EINTR -retrying"
                << endl;
#endif // _MSC_VER
#ifdef EINTR
   } while ( length < 0 && errno == EINTR);
#endif // EINTR
   
   if ( length < 0 ) {
#ifdef __unix
      mc2log << error
             << "DatagramReceiver: recvfrom: "
             << strerror(errno) << endl;
#endif // __unix
   }      
   return length;
}


bool
DatagramReceiver::receive(Packet *packet, uint32 micros)
{
   int length = receive( packet->getBuf(), packet->getBufSize(), micros );
   if ( length < 0 ) {
      packet->setLength( 0 );
      return false;
   } else {
      packet->setLength( length ); 
      return true;
   }
}


int32
DatagramReceiver::receive( byte* buff, uint32 buffSize, uint32 micros ) 
{
   fd_set rfds;
   struct timeval tv;
   int retval;

   /* Add our fd to the set */
   FD_ZERO(&rfds);
   FD_SET(fd, &rfds);

   /* Put the microseconds into tv */
   tv.tv_sec = micros / 1000000;
   tv.tv_usec = micros % 1000000;
   
   /* Do the selection */
#ifndef _MFC_VER
   retval = select(getdtablesize(), &rfds, NULL, NULL, &tv);
#else
   retval = select(0, &rfds, NULL, NULL, &tv);
#endif

   if ( retval < 0 ){
      mc2dbg1 << "__select: " << strerror(errno) << endl;
   }
   if( retval > 0 && FD_ISSET(fd, &rfds)) {
      return receive(buff, buffSize);
   } else {// Timeout
      return -1;
   }  
}


bool
DatagramReceiver::joinGroup(uint32 ip)
{
#ifndef SINGLE_VERSION
   struct ip_mreq mreq;

	mc2dbg8 << "IP: " << ip << endl;
	
   mreq.imr_multiaddr.s_addr = htonl(ip);
   mreq.imr_interface.s_addr = htonl(INADDR_ANY);
   if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq,
                  sizeof(mreq)) < 0)
      PANIC("DatagramReceiver::joinGroup -> __setsockopt: ", strerror(errno));
#else
   if ( setsockopt(fd, IPPROTO_IP, SO_DEBUG, NULL, 0) == -1 ) {
      mc2dbg1 << "DatagramReceiver::joinGroup failed setting SO_DEGUG"
              << endl;
      return false;
   }
      
#endif
   return true;
}

int DatagramReceiver::getSOCKET() 
{
	return fd;
}

uint16
DatagramReceiver::getPort()
{
   return (port);
}

bool
DatagramReceiver::getPeerName(uint32& IP, uint16& port) 
{
   if ( m_peerAddr.sin_port != 0 ) { // Is set
      IP = ntohl( m_peerAddr.sin_addr.s_addr );
      port = ntohs( m_peerAddr.sin_port );
      return true;
   } else {
      return false;
   }
}

void
DatagramReceiver::clear() {
   uint32 bufSize = Packet::MAX_UDP_PACKET_SIZE;
   byte* buffer = new byte[ bufSize ];

   while ( receive( buffer, bufSize, 0 ) > 0 ) {
      DEBUG8(mc2dbg << "DatagramReceiver::clear dumping packet ";
             uint32 IP = 0;
             uint16 port = 0;
             if ( getPeerName( IP, port ) ) {
                mc2dbg << "From: " << IP << ":" << port << endl;
             }
             mc2dbg << endl;
             );
   }
   delete [] buffer;
}
