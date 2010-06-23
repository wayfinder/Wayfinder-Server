/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CoordinateOnItemPacket.h"
#include "StringTable.h"
#include "MC2BoundingBox.h"
#include "GfxConstants.h"

//---------------
// Request
//---------------

CoordinateOnItemRequestPacket::CoordinateOnItemRequestPacket(uint32 packetID, 
                                                             uint32 reqID,
                                                             bool bboxWanted,
                                                  bool allCoordinatesWanted)
      :  RequestPacket( MAX_PACKET_SIZE,
                        COORDINATEONITEM_REQUEST_PRIO,
                        Packet::PACKETTYPE_COORDINATEONITEMREQUEST,
                        packetID,
                        reqID,
                        MAX_UINT32 )
{
   setLength(REQUEST_HEADER_SIZE+COORDINATEONITEM_REQUEST_HEADER_SIZE);
   setNbrItems(0);
   setBBoxWanted(bboxWanted);
   setAllCoordinatesWanted(allCoordinatesWanted);
}


void CoordinateOnItemRequestPacket::add( uint32 itemID,
                                         uint32 offset )
{
   int position = REQUEST_HEADER_SIZE +
                  COORDINATEONITEM_REQUEST_HEADER_SIZE +
                  getNbrItems()*8;
   incWriteLong(position, itemID);
   incWriteLong(position, offset);

   setNbrItems( getNbrItems()+1);
   setLength(position);
}

void CoordinateOnItemRequestPacket::getData( int32 index,
                                             uint32& itemID,
                                             uint32& offset ) const
{
   int pos = REQUEST_HEADER_SIZE +
             COORDINATEONITEM_REQUEST_HEADER_SIZE +
            index*8;
   itemID = incReadLong(pos);
   offset = incReadLong(pos);
}

//---------------
// Reply
//---------------

CoordinateOnItemReplyPacket::CoordinateOnItemReplyPacket(
                                 const CoordinateOnItemRequestPacket* p,
                                 int nbrItems)
      : ReplyPacket( nbrItems * BYTES_PER_ITEM +
                     REPLY_HEADER_SIZE + COORDINATEONITEM_REPLY_HEADER_SIZE,
                     Packet::PACKETTYPE_COORDINATEONITEMREPLY,
                     p,
                     StringTable::OK)
{
   setNbrItems(0);
   setLength(REPLY_HEADER_SIZE + COORDINATEONITEM_REPLY_HEADER_SIZE);
}

void 
CoordinateOnItemReplyPacket::add( uint32 itemID,
                                  int32 lat, 
                                  int32 lon,
                                  const MC2BoundingBox* bbox)
{
   int pos = REPLY_HEADER_SIZE +
             COORDINATEONITEM_REPLY_HEADER_SIZE +
             getNbrItems()*BYTES_PER_ITEM;
   incWriteLong(pos, itemID );
   incWriteLong(pos, lat );
   incWriteLong(pos, lon );
   if ( bbox != NULL ) {
      incWriteLong(pos, bbox->getMaxLat());
      incWriteLong(pos, bbox->getMinLon());
      incWriteLong(pos, bbox->getMinLat());
      incWriteLong(pos, bbox->getMaxLon());
   } else {
      for(int i=0; i < 4; ++i ) {
         incWriteLong(pos, GfxConstants::IMPOSSIBLE);
      }
   }
   setNbrItems( getNbrItems()+1 );
   setLength( pos );
}

void 
CoordinateOnItemReplyPacket::getLatLong( int32 index, uint32& itemID,
                                         int32& lat, int32& lon ) const
{
   int position = REPLY_HEADER_SIZE +
                  COORDINATEONITEM_REPLY_HEADER_SIZE +
                  index*BYTES_PER_ITEM;
   itemID = incReadLong(position);
   lat = incReadLong(position);
   lon = incReadLong(position);
}
  
void
CoordinateOnItemReplyPacket::getLatLongAndBB( int32 index,
                                              uint32& itemID,
                                              int32& lat,
                                              int32& lon,
                                              MC2BoundingBox& bbox) const
{
   int pos = REPLY_HEADER_SIZE +
             COORDINATEONITEM_REPLY_HEADER_SIZE +
             index*BYTES_PER_ITEM;
   itemID = incReadLong(pos);
   lat = incReadLong(pos);
   lon = incReadLong(pos);
   int32 maxLat = incReadLong(pos);
   int32 minLon = incReadLong(pos);
   int32 minLat = incReadLong(pos);
   int32 maxLon = incReadLong(pos);
   bbox = MC2BoundingBox(maxLat, minLon, minLat, maxLon);
}



