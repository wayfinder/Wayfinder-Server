/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TESTPACKETS_H
#define TESTPACKETS_H

#include "Packet.h"
#include "NetUtility.h"

inline Packet* createShutdownPacket() {

   RequestPacket* shutdowPack =
      new RequestPacket( 1024,
                         0, // prio
                         Packet::PACKETTYPE_SHUTDOWN,
                         0x7F, 0xAC, 1 ); // packetID, requestID, mapID
   shutdowPack->setArrivalTime();
   return shutdowPack;
}

/// Adds a cacheble packet to the queue
/// @param seed Value for data buffer, so cache elements can differ.
inline Packet* createCachablePacket( int seed ) {
   Packet* genericPack =
      new RequestPacket( 1024,
                         0, // prio
                         Packet::PACKETTYPE_TILEMAP_REQUEST,
                         0x7F, 0xAB, 1 ); // packetID, requestID, mapID
   int pos = REQUEST_HEADER_SIZE;
   genericPack->incWriteLong( pos, seed );
   genericPack->setLength( pos );
   genericPack->setOriginAddr( IPnPort( NetUtility::getLocalIP(), 5678 ) );
   genericPack->setArrivalTime();
   return genericPack;
}

inline Packet* createDirectDispatchPacket() {
   Packet* genericPack =
      new RequestPacket( 1024,
                         0, // prio
                         Packet::PACKETTYPE_PERIODIC_REQUEST,
                         0x7F, 0xAB, 1 ); // packetID, requestID, mapID
   genericPack->setOriginAddr( IPnPort( NetUtility::getLocalIP(), 5678 ) );
   genericPack->setArrivalTime();

   return genericPack;
}

#endif // TESTPACKETS_H
