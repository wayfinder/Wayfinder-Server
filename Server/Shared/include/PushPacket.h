/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ___PUSH_PACKET_H_QQQZZZ
#define ___PUSH_PACKET_H_QQQZZZ

#include "config.h"
#include "Packet.h"

#define PUSH_HEADER_SIZE (REQUEST_HEADER_SIZE + 8)

/**
 *   Superclass for packets with push contents.
 *   @packetdesc
 *   @row REQUEST_HEADER_SIZE     @sep 4 bytes @sep serviceID @endrow
 *   @row REQUEST_HEADER_SIZE + 4 @sep 4 bytes @sep timeStamp @endrow
 *   @endpacketdesc
 */
class PushPacket : public RequestPacket {
public:

   /**
    *   Constructor.
    *   @param bufSize   The size of the packet buffer.
    *   @param subType   The type of packet.
    *   @param mapID     The map id if mapID:s are used. If map id:s are
    *                    not used it shoud be MAX_UINT32.
    *   @param serviceID The service ID for the push service.
    *   @param timeStamp Time of newest update in packet.
    */
   PushPacket( uint32 bufSize,
               uint16 subType,
               uint32 mapID,
               uint32 serviceID,
               uint32 timeStamp);

   /**
    *   Returns the service ID of the contents of the packet.
    */
   inline uint32 getServiceID() const;

   /**
    *   Returns the timestamp for the last update in the packet.
    */
   inline uint32 getTimeStamp() const;

   /** Position of the service ID */
   static const int SERVICE_ID_POS = REQUEST_HEADER_SIZE;

   /** Position of time stamp */
   static const int TIME_STAMP_POS = SERVICE_ID_POS + 4;
   
};

inline uint32
PushPacket::getServiceID() const
{
   return readLong(SERVICE_ID_POS);
}

inline uint32
PushPacket::getTimeStamp() const
{
   return readLong(TIME_STAMP_POS);
}

#endif
