/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExpandItemPacket.h"

// =======================================================================
//                                               ExpandItemRequestPacket =

#define EXPANDITEMREQUEST_PRIO      0
#define EXPANDITEMREQUEST_LENGTH    REQUEST_HEADER_SIZE + 8

ExpandItemRequestPacket::ExpandItemRequestPacket()
   : RequestPacket(EXPANDITEMREQUEST_LENGTH)
{
   initEmptyPacket();
}

ExpandItemRequestPacket::ExpandItemRequestPacket(uint32 packetID, 
                                                 uint32 reqID,
                                                 uint32 mapID)
   : RequestPacket(EXPANDITEMREQUEST_LENGTH,
                   EXPANDITEMREQUEST_PRIO,
                   PACKETTYPE_EXPANDITEMREQUEST,
                   packetID,
                   reqID,
                   mapID)
{
   initEmptyPacket();
}

void 
ExpandItemRequestPacket::initEmptyPacket()
{
   setSubType(PACKETTYPE_EXPANDITEMREQUEST);
   int pos = REQUEST_HEADER_SIZE;
   incWriteLong(pos, 0);   // ItemID
   incWriteLong(pos, 0);   // Expansion type
   setLength(pos);
}

void 
ExpandItemRequestPacket::setItemID(uint32 itemID)
{
   writeLong(REQUEST_HEADER_SIZE, itemID);
}

uint32 ExpandItemRequestPacket::getItemID() const
{
   return (readLong(REQUEST_HEADER_SIZE));
}

void ExpandItemRequestPacket::setExpansionType(expansion_t type)
{
   writeLong(REQUEST_HEADER_SIZE + 4, (uint32) type);
}


ExpandItemRequestPacket::expansion_t 
ExpandItemRequestPacket::getExpansionType() const
{
   return ( (ExpandItemRequestPacket::expansion_t) 
            readLong(REQUEST_HEADER_SIZE + 4));
}


// =======================================================================
//                                                 ExpandItemReplyPacket =

#define EXPANDITEMREPLY_PRIO      0
#define EXPANDITEMREPLY_LENGTH    MAX_PACKET_SIZE

ExpandItemReplyPacket::ExpandItemReplyPacket()
   : ReplyPacket(EXPANDITEMREPLY_LENGTH,
                 PACKETTYPE_EXPANDITEMREPLY)
{
   initEmptyPacket();
}

ExpandItemReplyPacket::ExpandItemReplyPacket(const ExpandItemRequestPacket* req)
   : ReplyPacket(EXPANDITEMREPLY_LENGTH,
                 PACKETTYPE_EXPANDITEMREPLY,
                 req,
                 0)
{
   initEmptyPacket();
}

void ExpandItemReplyPacket::addItem(uint32 itemID)
{
   uint32 nbrItems = getNbrItems();
   int pos = REPLY_HEADER_SIZE+4 + nbrItems*4;
   incWriteLong(pos, itemID);
   setLength(pos);
   writeLong(REPLY_HEADER_SIZE, nbrItems+1);
}

uint32 ExpandItemReplyPacket::getItem(uint32 i) const
{
   if (i >= getNbrItems())
      return (MAX_UINT32);
   else
      return (readLong(REPLY_HEADER_SIZE+4 + 4*i));
}

uint32 ExpandItemReplyPacket::getNbrItems() const
{
   return (readLong(REPLY_HEADER_SIZE));
}

void ExpandItemReplyPacket::initEmptyPacket()
{
   writeLong(REPLY_HEADER_SIZE, 0); // NbrItems
}


