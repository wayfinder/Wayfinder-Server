/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PACKETREADERTHREAD_H
#define PACKETREADERTHREAD_H

#include "config.h"
#include "PointerFifoTemplate.h"

class Packet;
class TCPSocket;
class DatagramReceiver;
class Fifo;
class ModulePushList;
class ModulePacketSenderReceiver;

/** 
 *   Class that receives the incoming messages to a module. The one
 *   and only task for this module is to read the packets from the
 *   network and insert them into the incoming packets queue.
 *
 */
class PacketReaderThread {
   /// Rate of heartbeat ($ms^{-1}$)
#  define HEARTBEAT_RATE 1000
   
   /// Rate of sending vote-packets during election (ms)
#  define ELECTION_WON_RATE 500
   
public:
   typedef PointerFifoTemplate<Packet> ReceiveQueue;
   /** 
    *    The only constructor for the packet reader thread.
    * 
    *    @param q             The queue with to insert the incomming 
    *                         packets.
    *    @param leaderIP      The IP-address to the leader-multicast
    *                         group in hex.
    *    @param leaderPort    The port used for leader-group.
    *    @param availableIP   The IP-address to the available-multicast
    *                         group in hex.
    *    @param availablePort The port used for available-group.
    */
   PacketReaderThread( ModulePacketSenderReceiver& packetSenderReceiver,
                       bool listenForTCP = true );


   /**
    * Destructor.
    */
   virtual ~PacketReaderThread();

   
   /**
    *    This is the method that performes all work. And that is:
    *    \begin{enumerate}
    *       \item Read one UDP from the network into a Packet.
    *       \item Insert the Packet into the queue.
    *       \item Repeat from 1.
    *    \end{enumeration}
    */
   virtual void run();
   
   void start();
   /**
    *    Change the necessary variables to become leader.
    */ 
   void becomeLeader();
   
   /**
    *    Change the necessary variables to become an available module.
    */
   void becomeAvailable();
   
   /**
    *    @name Get.
    *    @memo Get ip and port-addresses.
    *    @doc  Methods to get the IP-addresses and port numbers that
    *          are used in this module.
    */
   //@{
   /**
    *    Get the IP-address of the leader (the leader multicast 
    *    address).
    *    @return  IP-address of the leader.
    */
   uint32 getLeaderIP() const;
         
   /**
    *    Get the portnumber used by the laeader.
    *    @return  The port number used by the laeader.
    */
   uint16 getLeaderPort() const;

   /**
    *    Get the IP-address of the available (the available multicast 
    *    address).
    *    @return  IP-address of the available modules.
    */
   uint32 getAvailableIP() const;

   /**
    *    Get the portnumber used by the available modules.
    *    @return  The port number used by the available modules.
    */
   uint16 getAvailablePort() const;

   /**
    *    Returns the push port of this module.
    */
   uint32 getPushPort() const;
         
   /**
    *    Get the packet queue where this thread inserts the 
    *    packets, read from the network.
    *    @return  The queue where the packets, read from the network,
    *             is inserted.
    */
   ReceiveQueue& getPacketQueue();

   //@}
   /**
    *    Sets the push list. Done from the reader if push is wanted.
    *    Must be done before start().
    */
   void setPushList(ModulePushList* pushList);
         
private:

   /**
    *   This function will clean things up if the
    *   terminate function is called, since the threads
    *   shouldn't be destroyed with the destructor
    */
   void cleanUp();

   ModulePacketSenderReceiver& m_packetSenderReceiver;

   
   /** 
    *   The queue where this packet reader thread inserts the 
    *   packets.
    */
   ReceiveQueue& m_incomingPacketQueue;
   
};

#endif

