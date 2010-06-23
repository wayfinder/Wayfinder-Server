/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SortDistPacket.h"

#define REQUEST_NBR_ITEMS_POS REQUEST_HEADER_SIZE
#define REQUEST_SORT_DIST_HEADER (REQUEST_HEADER_SIZE+4)

#define REPLY_NBR_ITEMS_POS REPLY_HEADER_SIZE
#define REPLY_SORT_DIST_HEADER (REPLY_HEADER_SIZE+4)

SortDistanceRequestPacket::SortDistanceRequestPacket()
: RequestPacket( MAX_PACKET_SIZE, 
                 SORTDIST_REQUEST_PRIO,
                 Packet::PACKETTYPE_SORTDISTREQUEST,
                 0, // Packet ID
                 0, // RequestID
                 MAX_UINT32 ) // MapID
{
   writeLong( REQUEST_NBR_ITEMS_POS, 0 );
   setLength( REQUEST_SORT_DIST_HEADER );
}

int SortDistanceRequestPacket::getNbrItems()
{
   return readLong( REQUEST_NBR_ITEMS_POS );
}

void SortDistanceRequestPacket::add( uint32 mapID, uint32 itemID, uint16 offset )
{
   int nbrItems = getNbrItems();
   int pos = REQUEST_SORT_DIST_HEADER + nbrItems*12; 
   incWriteLong( pos, mapID );
   incWriteLong( pos, itemID );
   incWriteLong( pos, offset );
   setLength( pos );
   writeLong(REQUEST_NBR_ITEMS_POS, nbrItems+1 );
}

void SortDistanceRequestPacket::getData( int index, uint32& mapID, uint32& itemID, uint16& offset )
{
   int nbrItems = getNbrItems();
   if( index < nbrItems ){
      int pos = REQUEST_SORT_DIST_HEADER + index*12;
      mapID  = incReadLong( pos);
      itemID = incReadLong( pos );
      offset = (uint16)incReadLong( pos );
   }
   else{
      mapID = MAX_UINT32;
      itemID = MAX_UINT32;
      offset = MAX_UINT16;
   }
}


////////////////////////////////////////////////////
// The reply packet.
////////////////////////////////////////////////////

SortDistanceReplyPacket::SortDistanceReplyPacket()
   : ReplyPacket( MAX_PACKET_SIZE, Packet::PACKETTYPE_SORTDISTREPLY )
{
   setNbrItems(0);
   setLength( REPLY_SORT_DIST_HEADER );
}
      
void SortDistanceReplyPacket::setNbrItems( int nbr )
{
   writeLong( REPLY_NBR_ITEMS_POS, nbr );
}

int SortDistanceReplyPacket::getNbrItems()
{
   return readLong( REPLY_NBR_ITEMS_POS );
}
      
void SortDistanceReplyPacket::add( int index, uint32 mapID, uint32 itemID, uint32 distance, uint32 sortIndex )
{
   int nbrItems = getNbrItems();
   if( index < nbrItems ){
      int pos = REQUEST_SORT_DIST_HEADER + index*16;
      incWriteLong( pos, mapID );
      incWriteLong( pos, itemID );
      incWriteLong( pos, distance );
      incWriteLong( pos, sortIndex );
   }
}

void SortDistanceReplyPacket::getData( int index, uint32& mapID, uint32& itemID, uint32& distance, uint32& sortIndex )
{
   int nbrItems = getNbrItems();
   if( index < nbrItems ){
      int pos = REQUEST_SORT_DIST_HEADER + index*16;
      mapID  = incReadLong( pos);
      itemID = incReadLong( pos );
      distance = incReadLong( pos );
      sortIndex = incReadLong( pos );
   }
   else{
      mapID = MAX_UINT32;
      itemID = MAX_UINT32;
      distance = MAX_UINT32;
      sortIndex = MAX_UINT32;
   }   
}
