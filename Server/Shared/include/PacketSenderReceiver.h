/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PACKETSENDERRECEIVER_H
#define PACKETSENDERRECEIVER_H

#include "config.h"
#include "PointerFifoTemplate.h"
#include "NetPacketQueue.h"
#include <memory>

class Packet;
class SelectableSelector;
class Selectable;
class IPnPort;

/**
 * Sends and receives packages by selecting for read and/or write
 * on a set of selectables.
 * Uses a send and a receive queue.
 */
class PacketSenderReceiver {
public:
   /// selectable read or write type
   enum IOType { 
      READ =  ( 1 << 0 ), //< should be selected with read
      WRITE = ( 1 << 1 ) //< should be selected with write
   };
   /// The receive queue type
   typedef PointerFifoTemplate<Packet, SelectableSelector> ReceiveQueue;
   /// The send queue type
   typedef NetPacketQueue SendQueue;

   /**
    * Setup sender/reciver to listen to a specific port with
    * both udp and tcp.
    * @param port the port to listen to
    */
   explicit PacketSenderReceiver( uint32 port );

   virtual ~PacketSenderReceiver();

   /// Start the working thread
   virtual void start();
   /// stop the working thread
   virtual void stop();

   /// @return address of this sender receiver ( local ip + port )
   const IPnPort& getAddr() const;
   /// @return port that this sender/receiver is listening on
   uint16 getPort() const;

   /// @return reference to send queue
   SendQueue& getSendQueue();
   /// @return reference to receive queue
   ReceiveQueue& getReceiveQueue();

   /**
    * Adds a selectable to a "do not remove"-list.
    * The "do not remove"-list contains selectables that are not suppose
    * to be removed once they get a timeout.
    * @param sel the selectable to be added
    * @param iotype read and/or write type selectable, @see IOType
    */
   void addPermanentSelectable( Selectable& sel, int iotype );

   /**
    * Removes a selectable from the "do not remove"-list.
    * @see addPermanentSelectable
    * @param sel the selectable to be removed
    */
   void removePermanentSelectable( Selectable& sel );

   /**
    * Called  after read/write of sockets has been done.
    */
   virtual void selectDone();

   /**
    * @return true if address is cached as a tcp connection
    */
   bool hasCachedConnection( const IPnPort& addr ) const;

protected:
   /** Close and destroy idle and cached connections. 
    *  Does not remove or destroy permanent sockets.
    */
   void clearIdleAndCachedConnections();
private:
   struct Impl;
   auto_ptr<Impl> m_impl; ///< hidden implementation
};

#endif 
