/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "BBoxPacket.h"
#include "PacketHelpers.h"
#include "StringTable.h"

BBoxReqPacketData::BBoxReqPacketData( const MC2BoundingBox& bbox,
                                      bool underview,
                                      bool country )
      : m_bbox(bbox),
        m_underview( underview ),
        m_country( country ),
        m_overview( false )
{
}

int
BBoxReqPacketData::getSizeInPacket() const
{
   return 4+4*4 + 4;
}

void
BBoxReqPacketData::save( Packet* packet, int& pos ) const
{
   // Save the length
   SaveLengthHelper slh( packet, pos );
   slh.writeDummyLength( pos );

   // Write the data
   packet->incWriteBBox( pos, m_bbox );
   
   packet->incWriteByte( pos, m_underview );
   packet->incWriteByte( pos, m_country );
   packet->incWriteByte( pos, m_overview );
   packet->incWriteByte( pos, 0 );

   slh.updateLengthUsingEndPos( pos );   
}

void
BBoxReqPacketData::load( const Packet* packet, int& pos )
{
    LoadLengthHelper llh( packet, pos, "[BBoxReqPacketData]" );
    llh.loadLength( pos );

    // Read the data
    packet->incReadBBox( pos, m_bbox );
    
    m_underview = packet->incReadByte( pos );
    m_country   = packet->incReadByte( pos );
    m_overview  = packet->incReadByte( pos );
    packet->incReadByte( pos );
    
    llh.skipUnknown( pos );   
}

ostream& operator<<( ostream& o, const BBoxReqPacketData& data )
{
   return o << "BBRPD: bbox = " << data.getBBox()
            << ", u = " << data.underview()
            << ", c = " << data.country();
}

// -- Request packet

BBoxRequestPacket::BBoxRequestPacket( const BBoxReqPacketData& data )
   : RequestPacket( MAX_PACKET_SIZE,
                    DEFAULT_PACKET_PRIO,
                    Packet::PACKETTYPE_BBOXREQUEST,
                    Packet::NO_PACKET_ID,
                    MAX_UINT16,           // Request ID
                    MAX_UINT32 )          // MapID
{
   int pos = REQUEST_HEADER_SIZE;
   data.save( this, pos );
   setLength( pos );
}

void
BBoxRequestPacket::get( BBoxReqPacketData& data ) const
{
   int pos = REQUEST_HEADER_SIZE;
   data.load( this, pos );
}


// -- Reply packet
BBoxReplyPacket::BBoxReplyPacket( const BBoxRequestPacket* req,
                                  const vector<uint32>& mapIDs )
      : ReplyPacket( 65536, // FIXME: Calculate this somehow.
                     Packet::PACKETTYPE_BBOXREPLY,
                     req,
                     StringTable::OK )
{
   int pos = REPLY_HEADER_SIZE;
   
   // Save the length
   SaveLengthHelper slh( this, pos );
   slh.writeDummyLength( pos );
   // Write back the bbox
   BBoxReqPacketData reqData;
   req->get( reqData );
   incWriteBBox( pos, reqData.getBBox() );
   // Write the covered maps
   incWriteLong( pos, mapIDs.size() );
   for ( uint32 i = 0; i < mapIDs.size(); ++i ) {
      incWriteLong( pos, mapIDs[i] );
   }
   slh.updateLengthUsingEndPos( pos );
   setLength( pos );
}

void
BBoxReplyPacket::get( MC2BoundingBox& bbox,
                      vector<uint32>& mapIDs ) const
{
   int pos = REPLY_HEADER_SIZE;

   // Load the length
   LoadLengthHelper llh( this, pos, "[BBoxReply]" );
   llh.loadLength( pos );

   // Read bbox
   incReadBBox( pos, bbox );
   
   vector<uint32> tmpIDs;

   int size = incReadLong( pos );
   tmpIDs.resize( size );
   for ( uint32 i = 0; i < tmpIDs.size(); ++i ) {
      incReadLong( pos, tmpIDs[i] );
   }
   
   mapIDs.swap( tmpIDs );
   
   llh.skipUnknown( pos );   
}

