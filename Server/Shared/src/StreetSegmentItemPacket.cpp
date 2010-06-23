/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "StreetSegmentItemPacket.h"


StreetSegmentItemRequestPacket::StreetSegmentItemRequestPacket(
                                       uint16 packetID,
                                       uint16 requestID,
                                       uint32 index,
                                       int32 lat,
                                       int32 lon,
                                       TrafficDataTypes::direction dir,
                                       uint32 originIP,
                                       uint16 originPort)

      : RequestPacket( REQUEST_HEADER_SIZE + 16,
                       DEFAULT_PACKET_PRIO,
                       Packet::PACKETTYPE_STREET_SEGMENT_ITEMREQUEST,
                       packetID,
                       requestID,
                       MAX_UINT32 )
{
   setOriginIP(originIP);
   setOriginPort(originPort);
   int pos = REQUEST_HEADER_SIZE;
   incWriteLong(pos, index);
   incWriteLong(pos, lat);
   incWriteLong(pos, lon);
   incWriteLong(pos, dir);
   
   setLength(pos);
}

uint32
StreetSegmentItemRequestPacket::getIndex() const
{
   return uint32(readLong(REQUEST_HEADER_SIZE));
}

int32
StreetSegmentItemRequestPacket::getLatitude() const
{
   return int32(readLong(REQUEST_HEADER_SIZE + 4));
}

int32
StreetSegmentItemRequestPacket::getLongitude() const
{
   return int32(readLong(REQUEST_HEADER_SIZE + 8));
}

TrafficDataTypes::direction
StreetSegmentItemRequestPacket::getDirection() const
{
   return TrafficDataTypes::direction(readLong(REQUEST_HEADER_SIZE + 12));
}

// Reply packet

StreetSegmentItemReplyPacket::StreetSegmentItemReplyPacket(
                                  const StreetSegmentItemRequestPacket* p,
                                  uint32 status,
                                  uint32 index,
                                  uint32 distance,
                                  uint32 mapID,
                                  int32 lat,
                                  int32 lon,
                                  uint32 firstNodeID,
                                  uint32 secondNodeID,
                                  uint32 firstAngle,
                                  uint32 secondAngle)

      : ReplyPacket( REPLY_HEADER_SIZE + 2000,
                     Packet::PACKETTYPE_STREET_SEGMENT_ITEMREPLY,
                     p, status)
{
   int pos = REPLY_HEADER_SIZE;
   incWriteLong(pos, index);
   incWriteLong(pos, distance);
   incWriteLong(pos, mapID);
   incWriteLong(pos, lat);
   incWriteLong(pos, lon);
   incWriteLong(pos, firstNodeID);
   incWriteLong(pos, secondNodeID);
   incWriteLong(pos, firstAngle);
   incWriteLong(pos, secondAngle);
   incWriteLong(pos, 0); // Nbr maps
   setLength(pos);
}


uint32
StreetSegmentItemReplyPacket::getIndex() const
{
   return uint32(readLong(REPLY_HEADER_SIZE));
}

uint32
StreetSegmentItemReplyPacket::getDistance() const
{
   return uint32(readLong(REPLY_HEADER_SIZE + 4));
}

uint32
StreetSegmentItemReplyPacket::getMapID() const
{
   return uint32(readLong(REPLY_HEADER_SIZE + 8));
}

int32
StreetSegmentItemReplyPacket::getLatitude() const
{
   return int32(readLong(REPLY_HEADER_SIZE + 12));
}

int32
StreetSegmentItemReplyPacket::getLongitude() const
{
   return int32(readLong(REPLY_HEADER_SIZE + 16));
}

uint32
StreetSegmentItemReplyPacket::getFirstNodeID() const
{
   return uint32(readLong(REPLY_HEADER_SIZE + 20));
}

uint32
StreetSegmentItemReplyPacket::getSecondNodeID() const
{
   return uint32(readLong(REPLY_HEADER_SIZE + 24));
}

uint32
StreetSegmentItemReplyPacket::getFirstAngle() const
{
   return uint32(readLong(REPLY_HEADER_SIZE + 28));
}

uint32
StreetSegmentItemReplyPacket::getSecondAngle() const
{
   return uint32(readLong(REPLY_HEADER_SIZE + 32));
}






