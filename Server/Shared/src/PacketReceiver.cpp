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

#include "Packet.h"
#include "DatagramSocket.h"
#include "PacketReceiver.h"
#include "PacketReceiveSocket.h"
#include <stdlib.h>

PacketReceiver::PacketReceiver() {
}

PacketReceiver::~PacketReceiver() {
   mc2dbg4 << "~PacketReceiver" << endl;
}

void
PacketReceiver::terminate()
{
   mc2dbg4 << "PacketReceiver::terminate()" << endl;
   m_receiver.terminate();
}


void
PacketReceiver::addDatagramSocket(DatagramReceiver *sock) {
   m_receiver.addDatagramSocket( sock ); 
}

void
PacketReceiver::removeDatagramSocket(DatagramReceiver *sock) {
   m_receiver.removeDatagramSocket( sock );
}

void
PacketReceiver::addTCPSocket(TCPSocket *sock) {
   m_receiver.addTCPSocket( sock );
}

void
PacketReceiver::removeTCPSocket(TCPSocket *sock) {
   m_receiver.removeTCPSocket( sock );
   m_permanentSockets.erase( sock );
}

void
PacketReceiver::addPermanentTCPSocket(TCPSocket* sock) {
   m_receiver.addTCPSocket( sock );
   m_permanentSockets.insert( sock );
}

bool
PacketReceiver::isPermanent(TCPSocket* sock)
{
   return m_permanentSockets.find(sock) != m_permanentSockets.end();
}

void*
PacketReceiver::getSocketThatReceived() {
   return m_receiver.getSocketThatReceived();
}


Packet*
PacketReceiver::receiveAndCreatePacket(int32 micros,
                                       TCPSocket** tcpReceiverSock) 
{
   // Variables that must be set before creating the packet
   byte* buffer = NULL;
   /*ssize_t*/uint32 bufferLength = 0;
   int packetLength = 0;
   
   // Used to get the correct socket to read from
   TCPSocket* tcpSock;
   DatagramReceiver* datagramSock;

   // Reset tcpReceiverSock
   if ( tcpReceiverSock != NULL ) {
      *tcpReceiverSock = NULL;
   }
   
   if ( m_receiver.select( micros, tcpSock, datagramSock ) ) {
      if ( tcpSock != NULL ) {
         // Set the socket to read from depending on the state
         TCPSocket* readSocket = tcpSock;
         bool deleteReadSocket = false;
         bool keepSocket = false; // Override on deleteReadSocket

         if (tcpSock->getState() == TCPSocket::LISTEN) {
            uint32 acceptStartTime = TimeUtility::getCurrentTime();
            readSocket = tcpSock->accept();
            uint32 acceptTime = TimeUtility::getCurrentTime() - acceptStartTime;
            if ( acceptTime > 1000 ) {
               mc2dbg << "[PR]: Took " << acceptTime
                      << " to accept which is unacceptable" << endl;
            }
            // Check if the socket should be permanently added.
            if ( isPermanent( tcpSock ) ) {
               mc2dbg8 << "[PR]: Socket is permanent" << endl;
               // Yes - add the child socket too.
               addPermanentTCPSocket( readSocket );
            } else {
               PacketReceiveSocket* prs =  
                  dynamic_cast< PacketReceiveSocket* > ( readSocket );
               if ( prs == NULL ) {
                  mc2dbg8 << "[PR]: New socket is not PacketReceiveSocket"
                          << endl;
                  // No - disconnect after reading a packet.
                  deleteReadSocket = true;
               } else {
                  // Continue reading next time.
                  mc2dbg8 << "[PR]: I will read during the next round"
                          << endl;
                  // Add the socket, but don't restart the selector.
                  m_receiver.addTCPSocket( readSocket, true );
                  readSocket = NULL;
                  keepSocket = true;
               }
            }            
         }
            
         if (readSocket != NULL) {
            if ( dynamic_cast< PacketReceiveSocket* > ( readSocket ) ) {
               PacketReceiveSocket* prs =  
                  static_cast< PacketReceiveSocket* > ( readSocket );
               prs->readBytes();
               if ( prs->getReadState() == PacketReceiveSocket::done ) {
                  buffer = prs->getBuffer( true );
                  bufferLength = prs->getSize();
                  packetLength = prs->getSize();
                  if ( !isPermanent( readSocket ) ) {
                     deleteReadSocket = true;
                     removeTCPSocket ( readSocket );
                  }
               } else if ( prs->getReadState() == 
                           PacketReceiveSocket::reading_size ||
                           prs->getReadState() == 
                           PacketReceiveSocket::reading_packet )
               {
                  // This shouldn't happen anymore since it has been
                  // done above.
                  if ( readSocket != tcpSock && !isPermanent( tcpSock ) ) {
                     // Add new socket
                     MC2_ASSERT( false );
                     m_receiver.addTCPSocket( readSocket );
                  }
                  keepSocket = true;
               } else {
                  // Else error
                  if ( !isPermanent( readSocket ) ) {
                     deleteReadSocket = true;
                  }
               }
            } else if ( ! isPermanent( readSocket ) ) {
               // Should not hang forever if someone connects without
               // sending
               buffer = readPacket( readSocket, bufferLength, packetLength,
                                    10*1000*1000);
            } else {
               // Maybe timeout here too?
               buffer = readPacket( readSocket, bufferLength, packetLength);
            }
            if ( buffer == NULL && !keepSocket ) {
               if ( isPermanent(readSocket) ) {
                  // We do not want to receive more stuff on this one.
                  removeTCPSocket ( readSocket );
                  mc2dbg << "[PacketReceiver] Permanent socket closed"
                     " no need to crash" << endl;
               } else {
                  mc2log << warn << "PacketReceiver::receiveAndCreatePacket "
                         << "failed to read packet from socket." << endl;
                  if ( readSocket == tcpSock &&
                       tcpSock->getState() == TCPSocket::LISTEN ) {
                     mc2log << "PacketReceiver::receiveAndCreatePacket "
                            << "Failed to read from main socket: " 
                            << "Error: exiting with exit(2) instead of "
                            << "trying to fix the situation." << endl;
                     exit( 2 );
                  }
               } 
            }
         }
         // Check if the read-socket should be deleted or not.
         if ( keepSocket ) {
            // Keep it to read more later
         } else if (deleteReadSocket) {
            delete readSocket;
         } else if ( tcpReceiverSock != NULL ) {
            *tcpReceiverSock = readSocket;
         }
         
      } else if ( datagramSock != NULL ) {
         // Create buffer.
         bufferLength = Packet::MAX_UDP_PACKET_SIZE;
         buffer = new byte[bufferLength];
         packetLength = datagramSock->receive(buffer, bufferLength);
         if (packetLength < 0) {
            delete [] buffer;
            buffer = NULL;
         }
      }
   }

   // Return
   if (buffer != NULL) {
      Packet* returnPacket = Packet::makePacket(buffer, 
                                                bufferLength);
      returnPacket->setLength(packetLength);
      return returnPacket;
   } else {
      return NULL;
   }
}


void
PacketReceiver::forceRestart() 
{
   m_receiver.forceRestart();
}


Packet* 
PacketReceiver::readAndCreatePacket( TCPSocket* readSocket,
                                     uint32 timeout ) {
   uint32 bufferLength = 0;
   int packetLength = 0;
   byte* buffer = readPacket( readSocket, bufferLength, packetLength,
                              timeout);
   if ( buffer != NULL ) {
      Packet* returnPacket = Packet::makePacket( buffer, 
                                                 bufferLength );
      returnPacket->setLength( packetLength );
      return returnPacket;
   } else {
      return NULL;
   }
}


byte* 
PacketReceiver::readPacket( TCPSocket* readSocket, 
                            uint32& bufferLength, int& packetLength,
                            uint32 timeout)
{
   byte* buffer = NULL;
   // Read the length of the packet (four bytes send before 
   // the actual packet).
   const uint32 lengthSizeTCP = 4;
   errno = 0; // So that we can be sure that it is set by read.
   int res = 0;
   bufferLength = res =
      readSocket->readExactly( (byte*)&packetLength, lengthSizeTCP, timeout );
   if ( bufferLength != lengthSizeTCP ) {
      uint32 peerIP   = 0;
      uint16 peerPort = 0;
      readSocket->getPeerName(peerIP, peerPort);
      if ( res != -2 ) {
         mc2log << warn << "PacketReceiver::readPacket "
                << "couldn't read packetSize: from "
                << prettyPrintIP(peerIP) << ":"
                << peerPort << " \""
                << strerror(errno) << "\"" << endl;
      } else {
         mc2log << warn << "PacketReceiver::readPacket - timeout while "
            "reading packlen from " << prettyPrintIP(peerIP) << ":"
                << peerPort << endl;
                
      } 
      
      readSocket->close();      
   } else {
      // Change the byte ordering of packetLength
      packetLength = ntohl( packetLength );
      bufferLength = packetLength;
      buffer = MAKE_UINT32_ALIGNED_BYTE_BUFFER( bufferLength );

      mc2dbg2 << "To read " << packetLength << " bytes" << endl;
      uint32 startReadTime = TimeUtility::getCurrentTime();
      ssize_t nbrRead =
         readSocket->readExactly( buffer, bufferLength, timeout );
      uint32 readTime = TimeUtility::getCurrentTime() - startReadTime;
      if ( readTime > 1000 ) {
         mc2dbg << "[PacketReceiver]: Took " << readTime
                << " to read packet" << endl;
      }
      mc2dbg2 << "Read " << nbrRead << " bytes" << endl;
      if (nbrRead != (ssize_t) packetLength) {
         uint32 peerIP   = 0;
         uint16 peerPort = 0;
         readSocket->getPeerName(peerIP, peerPort);
         mc2log << warn << "PacketReceiver::readPacket "
                << "failed to read packetdata from "
                << prettyPrintIP(peerIP) << ":"
                << peerPort
                << ", read returned "
                << nbrRead
                << " " << strerror(errno)
                << endl;
         delete [] buffer;
         buffer = NULL;
         readSocket->close();
      }
   }
   
   return buffer;
}
