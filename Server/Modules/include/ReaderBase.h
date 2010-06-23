/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PACKETSERDNER_H
#define PACKETSERDNER_H

#include "config.h"

#include "IPnPort.h"
#include "NetPacketQueue.h"
#include "PacketSendList.h"

#include <map>
#include <memory>

class Queue;
class Packet;
class DatagramSender;
class MapSafeVector;
class PacketSender;


/**
 *   Superclass to Readers. Introduced to avoid 
 *   that the reader enqueues packets in the packet
 *   queue without updating certain things, e.g. MapSafeVector.
 */
class ReaderBase {
public:
   /**
    *    Returns the length of the queue to the Processor.
    */
   int getQueueLength();
   
protected:
   /**
    *    Constructor.
    *    @param q The packet queue to the Processor.
    *    @param loadedMaps    The MapSafeVector to tell the Processor
    *                         about loading maps etc.
    *    @param ownAddr       The address of this module.
    *    @param leaderAddr    The multicast address of the leaders.
    *    @param availableAddr The multicast address of the availables.
    */
   ReaderBase( Queue* q,
               NetPacketQueue& sendQueue,
               MapSafeVector* loadedMaps,
               const IPnPort& ownAddr,
               const IPnPort& leaderAddr,
               const IPnPort& availableAddr,
               bool pushWanted );

   /**
    *    Destructor.
    */
   ~ReaderBase();

   /**
    *    Sends packet to the multicast address of the availables.
    */
   void sendToAvailables( const Packet& p);

   /**
    *    Sends packet to the multicast address of the leaders.
    */
   void sendToLeaders( const Packet& p );
   

   /**
    *    Sends a packet using TCP if the packet is large or
    *    UDP if it is small enough. Uses Packet::TCP_LIMIT_SIZE
    *    to choose protocol. Enqueues the packet in the queue if
    *    the receiver is == own address.
    *    @param p    The packet to send. Will be deleted.
    *    @param dest  Destination ip and port.
    */
   void sendAndDeletePacket(Packet* p,
                            const IPnPort& dest);


   void sendAndDeletePacket( Packet* packet,
                             uint32 ip, uint16 port ) { 
      sendAndDeletePacket( packet, IPnPort( ip, port ) );
   }

   /// sends all items in the list
   void sendAndDeletePackets( PacketSendList& packets );
   /// @return leader address
   const IPnPort& getLeaderAddr() const { return m_leaderAddr; }
   /// @return avail address
   const IPnPort& getAvailAddr() const { return m_availAddr; }
   /** 
    *   Used for communication between reader& processor.
    *   Contains e.g. which maps are loaded/loading etc.
    */
   MapSafeVector* m_loadedMaps;
   

protected:
   /// Own ip and port
   IPnPort m_ownAddr;

private:

   /**
    *   Puts the packet into the queue to the processor.
    *   Also checks if the packet is LoadMapRequestPacket
    *   or DeleteMapRequestPacket and updates MapSafeVector.
    */
   void enqueue(Packet* p);
   
   /** 
    *   The queue that the reader inserts the job-packets to 
    *   be processed.
    */
   Queue* m_packetQueue;

   /// Leader ip and port
   IPnPort m_leaderAddr;
   
   /// Available ip and port
   IPnPort m_availAddr;

   // The maps that are waiting for push from the InfoModule
   map<uint32, vector<Packet*> > m_waitingForPush;

   // True if the module is subscribing for push packets.
   bool m_pushWanted;

   // Timer used for push
   map<uint32, uint32> m_pushTime;

   auto_ptr<PacketSender> m_packetSender; //< sends packets
};

#endif
