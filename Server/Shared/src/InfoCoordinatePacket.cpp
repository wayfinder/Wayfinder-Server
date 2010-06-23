/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "InfoCoordinatePacket.h"
#include "PacketHelpers.h"
#include "MC2Exception.h"

void InfoCoordinate::save(Packet* packet, int& pos) const
{
   packet->incWriteLong( pos, coord.lat );
   packet->incWriteLong( pos, coord.lon );
   packet->incWriteLong( pos, angle );   
}

void InfoCoordinate::load( const Packet* packet, int& pos) 
{
   packet->incReadLong( pos, coord.lat );
   packet->incReadLong( pos, coord.lon );
   packet->incReadLong( pos, angle );   
}


InfoCoordinateRequestPacket::
InfoCoordinateRequestPacket(uint32 mapID,
                            const MC2Coordinate& coord,
                            uint32 angle) : 
   RequestPacket(REQUEST_HEADER_SIZE + 4/*num*/ + 3*4, /*lat, lon, angle*/
                 DEFAULT_PACKET_PRIO,
                 PACKETTYPE_INFOCOORDINATEREQUEST,
                 0,0,MAX_UINT32 )
{
   InfoCoordCont infoCoord( 1, InfoCoordinate( coord,angle ) );
   encodeRequest( infoCoord );
}

InfoCoordinateRequestPacket::
InfoCoordinateRequestPacket(uint32 mapID,
                            const InfoCoordCont& coords) : 
   RequestPacket((REQUEST_HEADER_SIZE + 4/*num*/ + 
                  3*4*coords.size()), /*lat, lon, angle*/
                 DEFAULT_PACKET_PRIO,
                 PACKETTYPE_INFOCOORDINATEREQUEST,
                 0,0,MAX_UINT32 )
{
   encodeRequest( coords );
}

void 
InfoCoordinateRequestPacket::
encodeRequest( const InfoCoordinateRequestPacket::InfoCoordCont& coords )
{
   int pos = REQUEST_HEADER_SIZE;
   SaveLengthHelper slh(this, pos);
   slh.writeDummyLength( pos );
   incWriteLong( pos, coords.size() );
   for( InfoCoordCont::const_iterator it = coords.begin();
        it != coords.end(); ++it ){
      it->save( this, pos );
   }
   slh.updateLengthUsingEndPos( pos );
}


InfoCoordinateRequestPacket::InfoCoordCont::size_type 
InfoCoordinateRequestPacket::
readData(InfoCoordinateRequestPacket::InfoCoordCont& coords) const
{
   int pos = REQUEST_HEADER_SIZE;
   LoadLengthHelper llh( this, pos, "[InfoCoordinateRequestPacket]");
   llh.loadLength( pos );
   uint32 num = 0;
   incReadLong( pos, num );
   coords.reserve( num );
   for( uint32 i = 0; i < num; ++i ){
      InfoCoordinate coord;
      coord.load( this, pos );
      coords.push_back( coord );
   }
   llh.skipUnknown( pos );
   return coords.size();
}

void InfoCoordinateReplyData::save(Packet* packet, int& pos) const
{
   packet->incWriteLong( pos, mapID );
   packet->incWriteLong( pos, nodeID0 );
   packet->incWriteLong( pos, nodeID1 );
   packet->incWriteLong( pos, distance );
   packet->incWriteLong( pos, offset );
}

void InfoCoordinateReplyData::load(Packet* packet, int& pos)
{
   packet->incReadLong( pos, mapID );
   packet->incReadLong( pos, nodeID0 );
   packet->incReadLong( pos, nodeID1 );
   packet->incReadLong( pos, distance );
   packet->incReadLong( pos, offset );
}


void InfoCoordinateReplyData::merge( const InfoCoordinateReplyData& data )
{
   if( distance > data.distance ) {
      *this = data;
   }
}

ostream& operator<<(ostream& stream, const InfoCoordinateReplyData& icrd)
{
   return stream << hex
                 << "(0x" << setfill('0') << setw(8) << icrd.mapID 
                 << ",0x" << setfill('0') << setw(8) << icrd.nodeID0 
                 << ",Ox" << setfill('0') << setw(8) << icrd.nodeID1 
                 << "," << dec << icrd.distance << "," 
                 << icrd.offset << ")";
}


InfoCoordinateReplyPacket::
InfoCoordinateReplyPacket( const InfoCoordinateRequestPacket* request,
                           uint32 status, 
                           const InfoReplyCont& data ) :
   ReplyPacket(6666, Packet::PACKETTYPE_INFOCOORDINATEREPLY,
               request, status)
{
   encodeReply( data );
}

void 
InfoCoordinateReplyPacket::
encodeReply( const InfoCoordinateReplyPacket::InfoReplyCont& data )
{
   int pos = REPLY_HEADER_SIZE;
   SaveLengthHelper slh(this, pos);
   slh.writeDummyLength( pos );
   incWriteLong( pos, data.size() );
   for( InfoReplyCont::const_iterator it = data.begin();
        it != data.end(); ++it ){
      it->save( this, pos );
   }
   slh.updateLengthUsingEndPos( pos );
}

void
InfoCoordinateReplyPacket::
mergeData(InfoCoordinateReplyPacket::InfoReplyCont& data)
{
   int pos = REPLY_HEADER_SIZE;
   LoadLengthHelper llh( this, pos, "[InfoCoordinateReplyPacket]");
   llh.loadLength( pos );
   uint32 num = 0;
   incReadLong( pos, num );

   if(num != data.size()){
      throw MC2Exception("InfoCoordinateReplyPacket::mergeData size mismatch");
   }

   for( InfoCoordinateReplyPacket::InfoReplyCont::iterator it = data.begin();
        it != data.end(); ++it ){
      InfoCoordinateReplyData replyData;
      replyData.load( this, pos );
      it->merge( replyData );
   }
}
