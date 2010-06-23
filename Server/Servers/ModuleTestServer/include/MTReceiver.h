/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MTRECEIVER_H
#define MTRECEIVER_H

#include "config.h"
#include "DatagramSocket.h"
#include "TCPSocket.h"
#include "ISABThread.h"
#include "MTLog.h"
#include "ServerProximityPackets.h"


#define DEFAULT_MT_SERVER_PORT   8180

 /**
   *  This class describes a server used for testing the modules.
   */
class MTReceiver : public ISABThread
{
   public :

       /**
         *  Default constructor
         */
      MTReceiver(MTLog* log);


       /**
         *  Destrucor
         */
      virtual ~MTReceiver();


       /**
         *  @return  The port of the receiver
         */
      uint16 getPort();


       /**
         *  The receives packets and decodes them. Main loop of the thread.
         */
      void run();

      /**
       * The timestamp of the last received packet.
       */
      uint32 getLastPacketTimeStamp();

   private :

      void decodeCoveredIDsReply(Packet* packet);
      void decodeCoordinateOnItemReply(Packet* packet);

      /**
       */
      void decodeCoordinateReply(Packet* packet);

      /**
       */
      void decodeAllMapReply(Packet* packet);

      /**
       */
      void decodeSMSReply(Packet* packet);
      
      void decodeEmailReply(Packet* packet);


      
      void decodeTestReply(Packet* packet);

      void decodeMunicipalReply(Packet* packet);

      void decodeLoadMapReply(Packet* packet);

      void decodeDeleteMapReply(Packet* packet);

       /**
         */
      void decodeMapReply(Packet* packet);


       /**
         */
      void decodeItemReply(Packet* packet);

      void decodeExpandItemReply(Packet* packet);


       /**
         */
      void decodeRouteReply(Packet* packet);


       /**
         */
      void decodeVanillaSearchReply(Packet* packet);


       /**
         *  Decodes an Overview Search Reply Packet
         *  @param   packet   The packet to decode
         */     
      void decodeOverviewSearchReplyPacket(Packet* packet);


      /**
       * Handles a CheckUserPasswordReplyPacket
       */
      void decodeCheckUserPasswordReply( Packet* p );


      /**
       * Handles a SessionCleanUpReplyPacket
       */
      void decodeSessionCleanUpReply( Packet* p );


      /**
       * handles a GetUserNavDestinationReplyPacket.
       */
      void decodeGetUserNavDestinationReplyPacket( Packet* packet );


      /**
       * handles a AddUserNavDestinationReplyPacket.
       */
      void decodeAddUserNavDestinationReplyPacket( Packet* packet );


      /**
       * Handles a AddDisturbanceReplyPacket.
       */
      void decodeAddDisturbanceReply( Packet* p );


      /**
       * Handles a RemoveDisturbanceReplyPacket.
       */
      void decodeRemoveDisturbanceReply( Packet* p );


      /**
       * Handles a RemoveDisturbanceReplyPacket.
       */
      void decodeGetDisturbanceReply( Packet* p );

      /**
       * Handles a DisturbanceSubscriptionReplyPacket.
       */
      void decodeDisturbanceSubscriptionReply( Packet* p );

      /**
       * Handles a UpdateTrafficCostRequestPacket.
       */
      void decodeUpdateTrafficCostRequest( Packet* p );

       /**
         *  Retrives a search map via TCP and prints it i clear text.
         *  @param   ip    The ip address of the map module.
         *  @param   port  The port of the map module.
         *  @return  True if successfull, false otherwise.
         */
      void getSearchMap(uint32 ip, uint16 port);


       /**
         *  Retrives a route map via TCP and prints it i clear text.
         *  @param   ip    The ip address of the map module.
         *  @param   port  The port of the map module.
         *  @return  True if successfull, false otherwise.
         */
      void getRouteMap(uint32 ip, uint16 port);


       /**
         *  Retrives a graphics map via TCP and prints it i clear text.
         *  @param   ip    The ip address of the map module.
         *  @param   port  The port of the map module.
         *  @return  True if successfull, false otherwise.
         */
      void getGfxMap(uint32 ip, uint16 port);

       /**
         *  Retrives a stringtable "map" via TCP and prints it i clear text.
         *  @param   ip    The ip address of the map module.
         *  @param   port  The port of the map module.
         *  @return  True if successfull, false otherwise.
         */
      void getStringTableMap(uint32 ip, uint16 port);

      /**
       *    Receives an EdgeNodesReplyPacket and prints some info.
       */
      void decodeEdgeNodesReply(Packet* packet);

      /**
       *    Receives a RouteExpandItemReplyPacket and prints some info.
       */
      void decodeRouteExpandItemPacket(Packet* packet);
      
       /**
         *  The socket used to receive UDP packets
         */
      DatagramReceiver* UDPReceiver;

       /**
         *  The log object
         */
      MTLog* log;

      /**
       * The time of the last received packet
       */
      uint32 m_packetTimeStamp;

      /// The mean time for the test replies to arrive.
      uint32 m_meanResponseTime;

      /// The number of responses that took more than 1000 ms.
      uint32 m_failedResponses;

      /// Total number of responses.
      uint32 m_nbrResponses;

}; // MTServer

#endif // MTRECEIVER_H
