/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DisturbancePacket.h"
#include "StringTable.h"

DisturbanceRequestPacket::DisturbanceRequestPacket(
                                        uint32 packetID,
                                        uint32 requestID,
                                        uint32 startTime,
                                        uint32 endTime,
                                        uint32 mapID,
                                        uint32 originIP,
                                        uint16 originPort)
      : RequestPacket( MAX_PACKET_SIZE,
                       DISTURBANCE_REQUEST_PRIO,
                       Packet::PACKETTYPE_DISTURBANCEREQUEST,
                       packetID,
                       requestID,
                       MAX_UINT32 )
{
   setOriginIP(originIP);
   setOriginPort(originPort);

   int position = REQUEST_HEADER_SIZE;
   incWriteLong(position, startTime);
   incWriteLong(position, endTime);
   incWriteLong(position, mapID);
   setLength(position);
}

DisturbanceRequestPacket::~DisturbanceRequestPacket()
{
   
}

uint32
DisturbanceRequestPacket::getStartTime() const
{
   return readLong(START_TIME_POS);
}

uint32
DisturbanceRequestPacket::getEndTime() const
{
   return readLong(END_TIME_POS);
}

uint32
DisturbanceRequestPacket::getRequestedMapID() const
{
   return readLong(MAP_ID_POS);
}


DisturbanceReplyPacket::
DisturbanceReplyPacket( const DisturbanceRequestPacket* p,
                        const IDToDisturbanceMap& distMap,
                        bool removeDisturbances,
                        bool removeAll ):
   ReplyPacket( DisturbancePacketUtility::
                calcPacketSize( REPLY_HEADER_SIZE, distMap ),
                PACKETTYPE_DISTURBANCEREPLY,
                (RequestPacket*) p,
                StringTable::OK)
{
   int position = REPLY_HEADER_SIZE;
   DisturbancePacketUtility::writeToPacket( distMap,
                                            this, position,
                                            removeDisturbances,
                                            removeAll);
}

DisturbanceReplyPacket::~DisturbanceReplyPacket()
{

}

void
DisturbanceReplyPacket::getDisturbances(
                              map<uint32,DisturbanceElement*> &distMap,
                              bool &removeDisturbances,
                              bool &removeAll) const
{
   int position = REPLY_HEADER_SIZE;
   DisturbancePacketUtility::getDisturbances( distMap,
                                              this, position,
                                              removeDisturbances,
                                              removeAll);
}


