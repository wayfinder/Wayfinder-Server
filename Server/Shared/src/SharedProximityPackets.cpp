/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SharedProximityPackets.h"
#include <stdlib.h>

//////////////////////////////////////////////////////////////
// ProximityPositionRequestPacket
//////////////////////////////////////////////////////////////
ProximityPositionRequestPacket::ProximityPositionRequestPacket( 
   uint16 packetID,
   uint16 reqID ) 
      : RequestPacket( REQUEST_HEADER_SIZE + 58 + 255, // Header + string
                       SHARED_PROXIMITY_REQUEST_PRIO,
                       PACKETTYPE_PROXIMITYPOSITIONREQUESTPACKET,
                       packetID,
                       reqID,
                       MAX_UINT32 ) // MapID
{
}


void
ProximityPositionRequestPacket::encodeRequest( 
   PositionRequestPacketContent packetContent,
   int32 latitude,
   int32 longitude,
   uint32 distance,
   uint32 itemID,
   uint16 offset,
   proximitySearchItemsType 
   proximityItems,
   uint16 maxNumberHits,
   bool useString,
   const char* searchString,
   uint16 categoryType,
   SearchTypes::StringMatching matchType,
   SearchTypes::StringPart stringPart,
   SearchTypes::SearchSorting sortingType,
   uint8 nbrSortedHits,
   uint32 requestedLanguage,
   uint32 regionsInMatches,
   uint32 innerRadius,
   uint16 startAngle,
   uint16 stopAngle )
{
   DEBUG2(cerr << "ProximityPositionRequestPacket::encode()" << endl;);
   int i = REQUEST_HEADER_SIZE;
   
   incWriteLong(i, latitude);
   incWriteLong(i, longitude);
   incWriteLong(i, distance);
   incWriteLong(i, itemID);
   incWriteShort(i, offset);
   incWriteShort(i, packetContent);
   incWriteByte(i, useString);
   incWriteByte(i, nbrSortedHits);
   incWriteShort(i, categoryType);
   incWriteLong(i, matchType);
   incWriteLong(i, stringPart);
   incWriteLong(i, sortingType);
   incWriteLong(i, proximityItems);
   incWriteLong(i, requestedLanguage );
   incWriteLong(i, regionsInMatches );
   incWriteLong(i, innerRadius );
   incWriteShort(i, startAngle );
   incWriteShort(i, stopAngle );
   incWriteShort(i, maxNumberHits);
   if ( strlen(searchString) > 254 ) 
      PANIC("ProximityPositionRequestPacket::encodeRequest", 
            "String overflow");
   incWriteString(i, searchString);
   setLength(i);
}


void
ProximityPositionRequestPacket::decodeRequest( 
   PositionRequestPacketContent& packetContent,
   int32& latitude,
   int32& longitude,
   uint32& distance,
   uint32& itemID,
   uint16& offset,
   proximitySearchItemsType& 
   proximityItems,
   uint16& maxNumberHits,
   bool& useString,
   char*& searchString,
   uint16& categoryType,
   SearchTypes::StringMatching& matchType,
   SearchTypes::StringPart& stringPart,
   SearchTypes::SearchSorting& sortingType,
   uint8& nbrSortedHits,
   uint32& requestedLanguage,
   uint32& regionsInMatches,
   uint32& innerRadius,
   uint16& startAngle,
   uint16& stopAngle ) 
{
   DEBUG2(cerr << "ProximityPositionRequestPacket::decode()" << endl;);
   int i = REQUEST_HEADER_SIZE;  

   latitude = incReadLong(i);
   longitude = incReadLong(i);
   distance = incReadLong(i);
   itemID = incReadLong(i);
   offset = incReadShort(i);
   packetContent = PositionRequestPacketContent(incReadShort(i));
   useString = (bool)(incReadByte(i) != 0);
   nbrSortedHits = (bool)(incReadByte(i) != 0);
   categoryType = incReadShort(i);
   matchType = static_cast<SearchTypes::StringMatching>(incReadLong(i));
   stringPart = static_cast<SearchTypes::StringPart>(incReadLong(i));
   sortingType = static_cast<SearchTypes::SearchSorting>(incReadLong(i));
   proximityItems = proximitySearchItemsType(incReadLong(i));
   requestedLanguage = incReadLong( i );
   regionsInMatches = incReadLong( i );
   innerRadius = incReadLong( i );
   startAngle = incReadShort( i );
   stopAngle = incReadShort( i );
   maxNumberHits = incReadShort(i);
   incReadString(i, searchString);
}


//////////////////////////////////////////////////////////////
// ProximityReplyPacket
//////////////////////////////////////////////////////////////

ProximityReplyPacket::ProximityReplyPacket( ProximityPositionRequestPacket*
                                            inPacket) 
      : ReplyPacket(MAX_PACKET_SIZE,
                    PACKETTYPE_PROXIMITYREPLYPACKET,
                    inPacket,
                    StringTable::OK ) // Status
{
}



ProximityReplyPacket::ProximityReplyPacket( ProximityItemSetRequestPacket*
                                            inPacket)
      : ReplyPacket(MAX_PACKET_SIZE,
                    PACKETTYPE_PROXIMITYREPLYPACKET,
                    inPacket,
                    StringTable::OK ) // Status
{
}


void
ProximityReplyPacket::setNumberMaps(uint16 numberMaps) {
   writeShort(REPLY_HEADER_SIZE, numberMaps);
}


uint16
ProximityReplyPacket::getNumberMaps() {
   return readShort(REPLY_HEADER_SIZE);
}


void 
ProximityReplyPacket::setMoreHits(uint8 moreHits) {
   writeByte(REPLY_HEADER_SIZE + 4, moreHits); 
}


uint8 
ProximityReplyPacket::getMoreHits() {
   return readByte(REPLY_HEADER_SIZE + 4); 
}


void
ProximityReplyPacket::setMapID(uint16 index, uint32 mapID) {
   writeLong(REPLY_HEADER_SIZE + 4 + 1 + index*8, mapID);
}


uint32
ProximityReplyPacket::getMapID(uint16 index) {
   return readLong( REPLY_HEADER_SIZE + 4 + 1 + index*8 );
}


int
ProximityReplyPacket::setNumberItems(uint16 index, uint32 nbrItems) {
   writeLong(REPLY_HEADER_SIZE + 4 + 1 + index*8 + 4, nbrItems); 
   
   int position = REPLY_HEADER_SIZE + 4 + 1 + getNumberMaps()*8;
   for ( uint32 i = 0 ; i < index ; i++ ) {
      position += 4*getNumberItems(i);
   }
   setLength(position + nbrItems*4);
   return position;
}


uint32
ProximityReplyPacket::getNumberItems(uint16 index) {
   return readLong( REPLY_HEADER_SIZE + 4 + 1 + index*8 + 4 );
}


bool
ProximityReplyPacket::addItem(uint32 itemID, int& position) {
   if ( (position+sizeof(uint32)) < MAX_PACKET_SIZE ) {
      incWriteLong(position, itemID);
      return true;
   } else {
      return false;
   }
}


int
ProximityReplyPacket::getFirstItemPosition(uint16 index) {
   int position = REPLY_HEADER_SIZE + 4 + 1 + getNumberMaps()*8;
   for ( uint32 i = 0 ; i < index ; i++ ) {
      position += 4*getNumberItems(i);
   }  
   return position;
}


uint32
ProximityReplyPacket::getNextItem(int& position) {
   uint32 itemID = incReadLong(position);
   return itemID;
}


//////////////////////////////////////////////////////////////
// ProximityItemSetRequestPacket
//////////////////////////////////////////////////////////////

ProximityItemSetRequestPacket::ProximityItemSetRequestPacket( 
   uint16 packetID,
   uint16 reqID ) 
      : RequestPacket( MAX_PACKET_SIZE,
                       SHARED_PROXIMITY_REQUEST_PRIO,
                       PACKETTYPE_PROXIMITYITEMSETREQUESTPACKET,
                       packetID,
                       reqID,
                       MAX_UINT32 ) // MapID
{
}


void
ProximityItemSetRequestPacket::encodeRequest(
   uint16 numberMaps,
   proximitySearchItemsType proximityItems,
   uint16 maxNumberHits,
   bool useString,
   const char* searchString,
   uint16 categoryType,
   SearchTypes::StringMatching matchType,
   SearchTypes::StringPart stringPart,
   SearchTypes::SearchSorting sortingType,
   uint8 nbrSortedHits )
{
   DEBUG2(cerr << "ProximityItremSetRequestPacket::encode()" << endl;);
   int i = REQUEST_HEADER_SIZE;

   incWriteLong(i, numberMaps);
   incWriteByte(i, useString);
   incWriteByte(i, nbrSortedHits);
   incWriteShort(i, categoryType);
   incWriteLong(i, matchType);
   incWriteLong(i, stringPart);
   incWriteLong(i, sortingType);
   uint32 length = 4*(strlen(searchString)/4+1);
   incWriteLong(i, length );
   incWriteLong(i, proximityItems);
   incWriteLong(i, maxNumberHits);
   incWriteString(i, searchString);
   setLength(i);
}


void
ProximityItemSetRequestPacket::decodeRequest(
   uint16& numberMaps,
   proximitySearchItemsType& proximityItems,
   uint16& maxNumberHits,
   bool& useString,
   char*& searchString,
   uint16& categoryType,
   SearchTypes::StringMatching& matchType,
   SearchTypes::StringPart& stringPart,
   SearchTypes::SearchSorting& sortingType,
   uint8& nbrSortedHits ) 
{
   DEBUG2(cerr << "ProximityItremSetRequestPacket::decode()" << endl;);
   int i = REQUEST_HEADER_SIZE;

   numberMaps = incReadLong(i);
   useString = (bool)(incReadByte(i)!=0);
   nbrSortedHits = incReadByte(i);
   categoryType = incReadShort(i);
   matchType = static_cast<SearchTypes::StringMatching>(incReadLong(i));
   stringPart = static_cast<SearchTypes::StringPart>(incReadLong(i));
   sortingType = static_cast<SearchTypes::SearchSorting>(incReadLong(i));
   incReadLong(i); // searchStringLength
   proximityItems = proximitySearchItemsType(incReadLong(i));
   maxNumberHits = incReadShort(i);
   incReadString(i, searchString);
}


uint16
ProximityItemSetRequestPacket::getNumberMaps() {
   return readLong(REQUEST_HEADER_SIZE);
}


void
ProximityItemSetRequestPacket::setMapID(uint16 index, uint32 mapID) {
   // Header + stringLength + mapPos
   int position = REQUEST_HEADER_SIZE + 28 + 
      readLong(REQUEST_HEADER_SIZE + 20) + index*8; 

   incWriteLong(position, mapID);
}


uint32
ProximityItemSetRequestPacket::getMapID(uint16 index) {
   // Header + stringLength + mapPos
   int position = REQUEST_HEADER_SIZE + 28 + 
      readLong(REQUEST_HEADER_SIZE + 20) + index*8; 
   
   return incReadLong(position);  
}


int
ProximityItemSetRequestPacket::setNumberItems(uint16 index, 
                                              uint32 nbrItems) 
{
   int position = REQUEST_HEADER_SIZE + 28 + 
      readLong(REQUEST_HEADER_SIZE + 20) + index*8 + 4;


   writeLong(position, nbrItems);

   position = REQUEST_HEADER_SIZE + 28 + 
      readLong(REQUEST_HEADER_SIZE + 20) + getNumberMaps()*8;

   for ( uint32 i = 0 ; i < index ; i++ ) {
      position += 4*getNumberItems(i);
   }
   setLength(position + nbrItems*4);
   return position;
}


uint32 
ProximityItemSetRequestPacket::getNumberItems(uint16 index) {
   int position = REQUEST_HEADER_SIZE + 28 + 
      readLong(REQUEST_HEADER_SIZE + 20) + index*8 + 4;  
   return readLong(position);
}


bool
ProximityItemSetRequestPacket::addItem(uint32 itemID, int& position) {
   if ( (position+sizeof(uint32)) < MAX_PACKET_SIZE ) {
      incWriteLong(position, itemID);
      return true;
   } else {
      return false;
   }
}


int
ProximityItemSetRequestPacket::getFirstItemPosition(uint16 index) {
   int position = REQUEST_HEADER_SIZE + 28 + 
      readLong(REQUEST_HEADER_SIZE + 20) + getNumberMaps()*8;

   for ( uint32 i = 0 ; i < index ; i++ ) {
      position += 4*getNumberItems(i);
   }

   return position;
}


uint32
ProximityItemSetRequestPacket::getNextItem(int& position) {
   return incReadLong(position);
}
