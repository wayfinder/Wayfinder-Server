/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapUpdatePacket.h"
#include "StringTable.h"

#define ADDMAPUPDATEREQUESTPACKET_PRIO 4

MapUpdateRequestPacket::MapUpdateRequestPacket()
   : RequestPacket(MAX_PACKET_SIZE,
                   ADDMAPUPDATEREQUESTPACKET_PRIO,
                   Packet::PACKETTYPE_ADDMAPUPDATE_REQUEST,
                   MAX_UINT16,
                   MAX_UINT16,
                   MAX_UINT32)
{
   setLength(REQUEST_HEADER_SIZE);
}

MapUpdateRequestPacket::MapUpdateRequestPacket(uint32 reqID, 
                                                     uint32 packetID)
   : RequestPacket(MAX_PACKET_SIZE,
                   ADDMAPUPDATEREQUESTPACKET_PRIO,
                   Packet::PACKETTYPE_ADDMAPUPDATE_REQUEST,
                   packetID,
                   reqID,
                   MAX_UINT32)
{
   setLength(REQUEST_HEADER_SIZE);

}


MapUpdateReplyPacket::MapUpdateReplyPacket()
   : ReplyPacket(REPLY_HEADER_SIZE,
                 Packet::PACKETTYPE_ADDMAPUPDATE_REPLY)
{
   setLength(REPLY_HEADER_SIZE);
}


MapUpdateReplyPacket::MapUpdateReplyPacket(
                           const MapUpdateRequestPacket* req,
                           StringCode status)
   : ReplyPacket(REPLY_HEADER_SIZE,
                 Packet::PACKETTYPE_ADDMAPUPDATE_REPLY,
                 req,
                 status)
{
   setLength(REPLY_HEADER_SIZE);
}


