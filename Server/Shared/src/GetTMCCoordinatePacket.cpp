/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GetTMCCoordinatePacket.h"

GetTMCCoordinateRequestPacket::GetTMCCoordinateRequestPacket(
                                                uint16 packetID,
                                                uint16 reqID ):
   RequestPacket( REQUEST_HEADER_SIZE + 26,
                  DEFAULT_PACKET_PRIO,
                  PACKETTYPE_GETTMCCOORREQUEST,
                  packetID,
                  reqID,
                  Packet::MAX_MAP_ID ) {
   int position = REQUEST_HEADER_SIZE;
   incWriteLong(position, 0);
   setLength(position);
}

GetTMCCoordinateRequestPacket::GetTMCCoordinateRequestPacket() :
   RequestPacket( REQUEST_HEADER_SIZE + 26,
                  DEFAULT_PACKET_PRIO,
                  PACKETTYPE_GETTMCCOORREQUEST,
                  0, // packet id
                  0, // request id
                  Packet::MAX_MAP_ID ) {
   int position = REQUEST_HEADER_SIZE;
   incWriteLong(position, 0);
   setLength(position);
}

bool
GetTMCCoordinateRequestPacket::addPoints(MC2String firstLocation,
                                         MC2String secondLocation,
                                         int32 extent,
                                         TrafficDataTypes::direction direction)
{
   int pos = REQUEST_HEADER_SIZE;

   // Check if the buffer is big enough, if not resize it
   uint32 tmcCordSize = 2*4 + firstLocation.size()+1 + secondLocation.size()+1;
   if ( pos + tmcCordSize >= getBufSize() ) {
      resize( ( getBufSize() + tmcCordSize ) * 2 );
   }

   incWriteLong(pos, extent);
   incWriteLong(pos, direction);
   incWriteString(pos, firstLocation.c_str());
   incWriteString(pos, secondLocation.c_str());
   setLength(pos);

   return true;
}

bool
GetTMCCoordinateRequestPacket::getPoints(MC2String &firstLocation,
                                         MC2String &secondLocation,
                                         int32 &extent,
                                         TrafficDataTypes::direction &direction) const
{
   int pos = REQUEST_HEADER_SIZE;
   extent = incReadLong(pos);
   direction = TrafficDataTypes::direction(incReadLong(pos));

   char* temp;
   incReadString(pos, temp);
   firstLocation = temp;
   incReadString(pos, temp);
   secondLocation = temp;
  
   return true;
}

// Reply packet

GetTMCCoordinateReplyPacket::GetTMCCoordinateReplyPacket(
                                                 const RequestPacket* p,
                                                 uint32 status,
                                                 uint32 size)
      : ReplyPacket(REPLY_HEADER_SIZE + size + 12,
                    PACKETTYPE_GETTMCCOORREPLY,
                    p,
                    status)
{
   int position = REPLY_HEADER_SIZE;
   incWriteLong(position, 0);  // Nbr of coordinates
   setLength(position);
}


void
GetTMCCoordinateReplyPacket::addCoordinatesToPacket(
   vector< pair<int32, int32> > firstCoords,
   vector< pair<int32, int32> > secondCoords)
{
   vector< pair<int32, int32> >::iterator it;

   int pos = REPLY_HEADER_SIZE;
   uint32 n1 = firstCoords.size();
   incWriteLong(pos, n1);
   for(it = firstCoords.begin(); it != firstCoords.end(); it++) {
      pair<int32, int32> coordPair = *it;
      int32 lat = coordPair.first;
      int32 lon = coordPair.second;
      incWriteLong(pos, lat);
      incWriteLong(pos, lon);
   }

   uint32 n2 = secondCoords.size();
   incWriteLong(pos, n2);
   for(it = secondCoords.begin(); it != secondCoords.end(); it++) {
      pair<int32, int32> coordPair = *it;
      int32 lat = coordPair.first;
      int32 lon = coordPair.second;
      incWriteLong(pos, lat);
      incWriteLong(pos, lon);
   }   
   setLength(pos);
}

void
GetTMCCoordinateReplyPacket::getCoordinates(
   vector< pair<int32, int32> > &firstCoords,
   vector< pair<int32, int32> > &secondCoords) const
{
   int pos = REPLY_HEADER_SIZE;
   uint32 n1 = incReadLong(pos);
   for(uint32 i = 0; i < n1; i++) {
      int32 lat = int32(incReadLong(pos));
      int32 lon = int32(incReadLong(pos));
      firstCoords.push_back(pair<int32, int32>(lat, lon));
   }
   uint32 n2 = incReadLong(pos);
   for(uint32 i = 0; i < n2; i++) {
      int32 lat = int32(incReadLong(pos));
      int32 lon = int32(incReadLong(pos));
      secondCoords.push_back(pair<int32, int32>(lat, lon));
   }
}

void GetTMCCoordinateReplyPacket::
getMC2Coordinates( vector< MC2Coordinate >& firstCoords,
                   vector< MC2Coordinate >& secondCoords ) const {
   int pos = REPLY_HEADER_SIZE;
   uint32 nbrOfCoords1 = incReadLong( pos );
   for( size_t i = 0; i < nbrOfCoords1; ++i ) {
      int32 lat = int32( incReadLong( pos ) );
      int32 lon = int32( incReadLong( pos ) );
      firstCoords.push_back( MC2Coordinate( lat, lon ) );
   }
   uint32 nbrOfCoords2 = incReadLong( pos );
   for( size_t i = 0; i < nbrOfCoords2; ++i ) {
      int32 lat = int32( incReadLong( pos ) );
      int32 lon = int32( incReadLong( pos ) );
      secondCoords.push_back( MC2Coordinate( lat, lon ) );
   }
}








