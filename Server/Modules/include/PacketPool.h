/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PACKETPOOL_H
#define PACKETPOOL_H

#include "config.h"
#include "Packet.h"
#include "JTC/JTC.h"

/**
  *   A very simple packet pool. Currently only handles one size of the
  *   packets. The packet pool uses monitors to make it threadsafe.
  */
class PacketPool : public  JTCMonitor {

   public:
      /**
        *   Created an empty PacketPool with the maximum possible size of 
        *   the packets (MAX_UINT16).
        */
      PacketPool();

      /**
        *   Created a PacketPool with the maximum possible size of 
        *   the packets (MAX_UINT16).
        *   @param   nbrPackets  The number of packets created when
        *                        started
        */
      PacketPool(uint32 nbrPackets);

      /**
        *   Created a PacketPool with a specified packet size
        *   (MAX_UINT16).
        *   @param   nbrPackets  The number of packets created when
        *                        started
        *   @param   packetSize  The size of the packets created in
        *                        the pool.
        */
      PacketPool(uint32 nbrPackets, uint16 packetSize);

      /**
        *   Used to get a free packet. If no free packet in the pool,
        *   a new is created and returned. Should replace "new".
        * 
        *   @return  A free packet, with the size specified when the
        *            packetpool created.
        */
      Packet* getPacket();

      /**
        *   Returns a packet to the PacketPool. Should replace "delete".
        *   @param   p  The packet to return.
        */
      void returnPacket(Packet* p);

      /**
        *   Please note that this is mainly for debugging -- this method
        *   takes linear time!
        *   @return  The number of packet currently in this packet pool
        */
      uint32 getCurNbrPackets();


   private:
      /**
        *   Pointer to the first packet in the pool.
        */
      Packet* firstPacket;

      /**
        *   The size of the packets created in this PacketPool.
        */
      uint16 packetSize;
};

#endif

