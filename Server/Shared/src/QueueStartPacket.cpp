/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "QueueStartPacket.h"
#include "StringTable.h"

QueueStartRequestPacket::QueueStartRequestPacket()
      : RequestPacket( MAX_PACKET_SIZE,
                       QUEUE_START_REQUEST_PRIO,
                       Packet::PACKETTYPE_QUEUESTARTREQUEST,
                       0, // Must be set before using
                       0,
                       MAX_UINT32)
{
   setLength( REQUEST_HEADER_SIZE + 16 );
   setFirstNodeID(MAX_UINT32);
   setDistance(MAX_UINT32);
}

QueueStartRequestPacket::QueueStartRequestPacket(uint32 firstNodeID,
                                                 uint32 distance)
      : RequestPacket( MAX_PACKET_SIZE,
                       QUEUE_START_REQUEST_PRIO,
                       Packet::PACKETTYPE_QUEUESTARTREQUEST,
                       0, // Must be set before using
                       0,
                       MAX_UINT32)
{
   setLength( REQUEST_HEADER_SIZE + 16 );
   setFirstNodeID(firstNodeID);
   setDistance(distance);
}

uint32
QueueStartRequestPacket::getFirstNodeID() const
{
   return readLong( REQUEST_HEADER_SIZE + QUEUE_START_REQUEST_NODE_ID );
}

uint32
QueueStartRequestPacket::getDistance() const
{
   return readLong( REQUEST_HEADER_SIZE + QUEUE_START_REQUEST_DISTANCE );
}

void
QueueStartRequestPacket::setFirstNodeID(uint32 nodeID)
{
   writeLong( REQUEST_HEADER_SIZE + QUEUE_START_REQUEST_NODE_ID, nodeID );
}

void QueueStartRequestPacket::setDistance(uint32 distance)
{
   writeLong( REQUEST_HEADER_SIZE +QUEUE_START_REQUEST_DISTANCE, distance );
}
   
// =======================================================================
//                                                QueueStartReplyPacket =


QueueStartReplyPacket::QueueStartReplyPacket(int size)
      : ReplyPacket( size,                    
                     Packet::PACKETTYPE_QUEUESTARTREPLY) {
   setNodeID(MAX_UINT32);
   setDistance(MAX_UINT32);
   setNextMapID(MAX_UINT32);
}
    
QueueStartReplyPacket::QueueStartReplyPacket(const QueueStartRequestPacket* p )
      : ReplyPacket( MAX_PACKET_SIZE,
                     PACKETTYPE_QUEUESTARTREPLY,
                     (RequestPacket*) p,
                     StringTable::OK) {
   
   setNodeID(MAX_UINT32);
   setDistance(MAX_UINT32);
   setNextMapID(MAX_UINT32);
}

void
QueueStartReplyPacket::setNodeID(uint32 lastNodeID)
{
   writeLong( REPLY_HEADER_SIZE +QUEUE_START_REPLY_NODE_ID, lastNodeID );
}


void
QueueStartReplyPacket::setDistance(uint32 passedDistance)
{
   writeLong( REPLY_HEADER_SIZE +QUEUE_START_REPLY_DISTANCE, passedDistance);
}

void
QueueStartReplyPacket::setNextMapID(uint32 nextMapID)
{
   writeLong( REPLY_HEADER_SIZE +QUEUE_START_REPLY_NEXT_MAP_ID, nextMapID);
}

bool
QueueStartReplyPacket::isDone() const
{
   return (readLong( REPLY_HEADER_SIZE +QUEUE_START_REPLY_NEXT_MAP_ID)
           == MAX_UINT32);
}

uint32
QueueStartReplyPacket::getNodeID() const
{
   return readLong( REPLY_HEADER_SIZE +QUEUE_START_REPLY_NODE_ID );
}
   

uint32
QueueStartReplyPacket::getDistance() const
{
   return readLong( REPLY_HEADER_SIZE +QUEUE_START_REPLY_DISTANCE );
}

uint32
QueueStartReplyPacket::getNextMapID() const
{
   return readLong( REPLY_HEADER_SIZE +QUEUE_START_REPLY_NEXT_MAP_ID );
}









