/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RouteExpandItemPacket.h"
#include "StringTable.h"


// - - - - - - - - - - - - - - - - - - - - - - RouteExpandItemRequestPacket

RouteExpandItemRequestPacket::
RouteExpandItemRequestPacket(uint32 packetID,
                             uint32 reqID,
                             uint32 userData)
      :  RequestPacket( MAX_PACKET_SIZE,
                        ROUTEEXPANDITEM_REQUEST_PRIO,
                        Packet::PACKETTYPE_ROUTEEXPANDITEMREQUEST,
                        packetID,
                        reqID,
                        MAX_UINT32 )
{
   int pos = REQUEST_HEADER_SIZE;
   incWriteLong(pos, userData);
   incWriteLong(pos, 0);        // NbrItems
   incWriteLong(pos, 0);        // Language. Prolly unused.

   // Must set the length, I think.
   setLength(pos);
}

void RouteExpandItemRequestPacket::add( uint32 itemID,
                                        uint16 offset )
{
   // Calc the position
   int position = ITEMS_START_POS + ITEM_SIZE * getNbrItems();
   incWriteLong(position, itemID);
   incWriteShort(position, offset);
   incWriteShort(position, 0);   // Pad to make sure everything is aligned
   setNbrItems( getNbrItems()+1);
   setLength(position);
}

void RouteExpandItemRequestPacket::getData( int32 index,
                                            uint32& itemID,
                                            uint16& offset ) const
{
   int position = ITEMS_START_POS + ITEM_SIZE * index;
   itemID = incReadLong(position);
   offset = incReadShort(position);
   incReadShort(position); // Read past the unused short
}


// - - - - - - - - - - - - - - - - - - - - - - - RouteExpandItemReplyPacket

RouteExpandItemReplyPacket::RouteExpandItemReplyPacket(
   const RouteExpandItemRequestPacket* p)
      : ReplyPacket( MAX_PACKET_SIZE,
                     Packet::PACKETTYPE_ROUTEEXPANDITEMREPLY,
                     p,
                     StringTable::OK)
{
   // Transfer some data from the request.
   setMapID(p->getMapID());
   setUserData(p->getUserData());
   setNbrItems(0);
   setLength(ITEMS_START_POS);
}

void
RouteExpandItemReplyPacket::addItem( uint16 nbrPos,
                                     uint32 expandedItemID,
                                     ItemTypes::itemType parentType,
                                     const uint32* ssItemID,
                                     const uint16* offset,
                                     const int32* lat,
                                     const int32* lon,
                                     const uint32* parentItemID)
{
   int position = getLength();
   // Calculate approx size.
   int approxSize = position + nbrPos * 6 * 4 + 4 + 10;
   if ( approxSize > (int)getBufSize() ) {
      // To small buffer - double it (and some more)
      resize( position + approxSize );
      mc2dbg << "[REIPack]: Resizing packet " << endl;
   }
   incWriteShort(position, nbrPos);
   incWriteByte(position, parentType );
   incWriteByte(position, 0);         // Reset the pad
   incWriteLong(position, expandedItemID);
   for (uint32 i = 0; i < nbrPos; i++) {
      incWriteLong(position, ssItemID[i]);
      incWriteShort(position, offset[i]);
      // Be sure to zero this one out.
      incWriteShort(position, 0 );
      incWriteLong(position, lat[i]);
      incWriteLong(position, lon[i]);
      incWriteLong(position, parentItemID[i]);
   }
   setNbrItems( getNbrItems()+1 );
   setLength(position);
}

void
RouteExpandItemReplyPacket::getItem(int& position,
                                    uint16& nbrPos,
                                    uint32& expandedItemID,
                                    ItemTypes::itemType& parentType,
                                    uint32*& ssItemIDs,
                                    uint16*& offsets,
                                    int32*& lats,
                                    int32*& lons,
                                    uint32*& parentItemID)
{
   nbrPos    = quickIncReadShort(position);
   parentType = ItemTypes::itemType( incReadByte(position) );
   incReadByte(position);        // Read past the pad
   expandedItemID = incReadLong(position);
   ssItemIDs = new uint32[nbrPos];
   offsets = new uint16[nbrPos];
   lats = new int32[nbrPos];
   lons = new int32[nbrPos];
   parentItemID = new uint32[nbrPos];
   for (uint32 i = 0; i < nbrPos; i++) {
      ssItemIDs[i] = quickIncReadLong(position);
      offsets[i] = quickIncReadShort(position);
      lats[i] = incReadLong(position);
      lons[i] = quickIncReadLong(position);
      parentItemID[i] = quickIncReadLong(position);
   }
}
