/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PacketSenderReceiver.h"
#include "CommandlineOptionHandler.h"
#include "NetPacket.h"
#include "MC2CRC32.h"
#include "Properties.h"
#include "MC2String.h"
#include "Packet.h"
#include "NetUtility.h"
#include "PacketNet.h"
#include "LoadMapPacket.h"
#include "DeleteMapPacket.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
/*
 * Usage:
 * Setup two or more PacketSenderTests programs as follows:
 * (using dev-2 ip for both programs)
 *
 * 1) ./PacketSenderTest -D "10.11.4.66:3841" -S 1
 *
 * 2) ./PacketSenderTest -D "10.11.4.66:3840" -S 1
 *
 * The first server will aquire port 3840 and send to port 3841
 * The second one will get port 3841 and send to port 3840
 * This way both will receive packets from each other and
 * do crc checks on them.
 *
 * To test close down:
 * 
 * # ./PacketSenderTest -D ip:port -K
 * This sends shutdown packet to ip:port which will start the
 * shutdown process.
 */
int main( int argc, char **argv ) {
   

   int32 preferredPort = 3840;
   char* destination = NULL;
   int32 sleepTime = 0;
   bool sendKill = false;
   bool listenOnly = false;
   uint32 numPackets = 0;
   uint32 loadMapID = MAX_UINT32;
   uint32 deleteMapID = MAX_UINT32;

   CommandlineOptionHandler coh(argc, argv);
   coh.setSummary("");
   coh.addOption( "-t", "--port",
                  CommandlineOptionHandler::uint32Val,
                  1, &preferredPort, "3840",
                  "listen port");

   coh.addOption("-D", "--destination",
                 CommandlineOptionHandler::stringVal,
                 1, &destination, "",
                 "Packet destination" );

   coh.addOption("-S", "--sleep",
                 CommandlineOptionHandler::uint32Val,
                 1, &sleepTime, "0",
                 "sleep time");
   coh.addOption("-K", "--kill",
                 CommandlineOptionHandler::presentVal,
                 1, &sendKill, "F",
                 "Send shutdown packet to destination");

   coh.addOption("-l", "--listen",
                 CommandlineOptionHandler::presentVal,
                 1, &listenOnly, "F",
                 "Dont sent packets, only listen." );
   coh.addOption("-n", "-numPackets",
                 CommandlineOptionHandler::uint32Val,
                 1, &numPackets, "0",
                 "Number of packets to send." );
   coh.addOption("", "--loadMap",
                 CommandlineOptionHandler::uint32Val,
                 1, &loadMapID, "0xFFFFFFFF",
                 "load a specific map id" );
   coh.addOption("", "--deleteMap",
                 CommandlineOptionHandler::uint32Val,
                 1, &deleteMapID, "0xFFFFFFFF",
                 "delete a specific map id" );

   if ( !coh.parse() ) {
      cerr << "PositioningModule: Error on commandline! (-h for help)" 
           << endl;
      return 0;
   }

   if ( !Properties::setPropertyFileName( coh.getPropertyFileName() ) ) {
      cerr << "No such file or directory: '"
           << coh.getPropertyFileName() << "'" << endl;
      return 0;
   }

   //
   // parse ip and port
   //
   MC2String ipStr( destination );
   MC2String portStr( destination );
   portStr.erase( 0, portStr.find_first_of(":") + 1);
   ipStr.erase( ipStr.find_first_of(":"), ipStr.size() );

   in_addr addr;
   if ( inet_aton( ipStr.c_str(), &addr ) == 0 ) {
      mc2dbg << error << " Can not find host: " 
             << ipStr << endl;
      return 0;
   }

   IPnPort destinationAddr( htonl( addr.s_addr ),
                            atoi( portStr.c_str() ) );

   mc2dbg << "Sending to " << destinationAddr << endl;

   if ( sendKill ) {
      // 
      // Send a shutdown packet to destination and exit program
      //
      Packet killPacket( 256, 0, // size, prio
                         Packet::PACKETTYPE_SHUTDOWN, // sub type
                         NetUtility::getLocalIP(), 0, // ip, port
                         31337, 31337 ); // packetID,deb
                         
      PacketUtils::sendViaTCP( killPacket, destinationAddr );
      return 0;
   } else if ( loadMapID != MAX_UINT32 ) {
      LoadMapRequestPacket loadMap( loadMapID,
                                    0, 0,
                                    destinationAddr.getIP(),
                                    destinationAddr.getPort() );

      PacketUtils::sendViaTCP( loadMap, destinationAddr );
      return 0;
   } else if ( deleteMapID != MAX_UINT32 ) {
      DeleteMapRequestPacket deleteMap( deleteMapID,
                                        destinationAddr.getIP(),
                                        destinationAddr.getPort() );

      PacketUtils::sendViaTCP( deleteMap, destinationAddr );
      return 0;
   }


   //
   // Init threads and packet sender/receiver
   //
   JTCInitialize init;

   PacketSenderReceiver senderReceiver( preferredPort );
   const uint32 packetSize = Packet::MAX_UDP_PACKET_SIZE - 128;
   Packet packetToSend( packetSize,  // size
                        0,    // prio
                        Packet::PACKETTYPE_TESTREQUEST, // sub type
                        NetUtility::getLocalIP(), // ip
                        senderReceiver.getPort(), // port
                        31337, 31337 ); // packetID, deb
   packetToSend.setLength( packetSize );

   memset( packetToSend.getBuf() + HEADER_SIZE, 
           0x7F, packetSize - HEADER_SIZE );
   
   const uint32 crc = MC2CRC32::crc32( packetToSend.getBuf() + HEADER_SIZE, 
                                       packetToSend.getLength() - HEADER_SIZE, 0 );
   // start listening on sockets
   senderReceiver.start();

   uint32 sendNum = 0; // number of sends
   // used to toggle udp/tcp send
   bool sendWithUDP = false;

   while ( 1 ) {
      if ( ! listenOnly ) { 
         //
         // send test packet to destination and
         // toggle udp/tcp send so we send a packet with TCP
         // and next time we send it with UDP
         //
         senderReceiver.getSendQueue().
            enqueue( new NetPacket( packetToSend.getClone(), 
                                    destinationAddr,
                                    sendWithUDP ? 
                                    NetPacket::UDP : NetPacket::TCP ) );
         sendWithUDP = ! sendWithUDP;
      }
      sendNum++;
      // get test packets from senders
      auto_ptr<Packet> testPacket;

      while ( 1 ) {
         // get packet, use no timelimit when we are in listen mode
         testPacket.reset( senderReceiver.
                           getReceiveQueue().
                           dequeue( listenOnly ? MAX_UINT32 : 100 ) );
         if ( testPacket.get() == NULL ) {
            break;
         }

         mc2dbg << " Got Packet: " << testPacket->getSubTypeAsString() << endl;
         mc2dbg << " Length: " << testPacket->getLength() << endl;

         //
         // If we got shutdown request, then end program.
         //
         if ( testPacket->getSubType() == Packet::PACKETTYPE_SHUTDOWN ) {
            break;
         }
              

         if ( testPacket->getLength() != packetToSend.getLength() ) {
            mc2dbg << "Length differs!" <<endl;
            MC2_ASSERT( false );
         }
         uint32 packetCRC = 
            MC2CRC32::crc32( testPacket->getBuf() + HEADER_SIZE, 
                             testPacket->getLength() - HEADER_SIZE, 0 );
         if ( packetCRC != crc ) {
            mc2dbg << fatal << "CRC differs!" << endl;
            MC2_ASSERT( false );
         }
      }

      //
      // If we got shutdown request, then end program.
      //
      if ( testPacket.get() &&
           testPacket->getSubType() == Packet::PACKETTYPE_SHUTDOWN ) {
         break;
      }
      mc2dbg << "send#" << sendNum << endl;
      mc2dbg << "queue sizes: " << senderReceiver.getSendQueue().getSize() 
             << ", " << senderReceiver.getReceiveQueue().getSize() << endl;

      // we are done sending packets
      if ( numPackets != 0 && sendNum >= numPackets ) {
         break;
      }
      sleep( sleepTime );
   }
}
