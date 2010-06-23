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

#include "TileMapPacket.h"
#include "GFMDataHolder.h"
#include "ServerTileMapFormatDesc.h"
#include "BitBuffer.h"
#include "TileMapBufferHolder.h"

TileMapRequestPacket::TileMapRequestPacket( 
            const GFMDataHolder& gfmData,
            const STMFDParams& stmfdParams,
            bool removeNames,
            const resultMap_t& resultMapType,
            const MC2SimpleString& param,
            int pixelToMeter )
      : RequestPacket( MAX_PACKET_SIZE,
                       DEFAULT_PACKET_PRIO,
                       Packet::PACKETTYPE_TILEMAP_REQUEST,
                       Packet::NO_PACKET_ID, // Packet ID
                       Packet::MAX_REQUEST_ID,
                       Packet::MAX_MAP_ID )
{
   int pos = REQUEST_HEADER_SIZE;
   // Pixel to meter.
   incWriteLong( pos, (uint32) pixelToMeter );

   stmfdParams.save( this, pos );
   // The desc.
   incWriteString( pos, param.c_str() );

   incWriteByte( pos, removeNames );
   incWriteByte( pos, resultMapType );
   gfmData.save( this, pos );
   setLength( pos );
}
                  
void 
TileMapRequestPacket::get( GFMDataHolder& gfmData,
                           STMFDParams& stmfdParams,
                           bool& removeNames,
                           resultMap_t& resultMapType,
                           MC2SimpleString& param,
                           int& pixelToMeter ) const
{
   int pos = REQUEST_HEADER_SIZE;
   // Pixel to meter.
   pixelToMeter = (int) incReadLong( pos );
   stmfdParams.load( this, pos );
   // The desc.
   param = MC2SimpleString( incReadString( pos ) );
   removeNames = incReadByte( pos );
   resultMapType = resultMap_t( incReadByte( pos ) );
   gfmData.load( this, pos );
}
   
TileMapReplyPacket::TileMapReplyPacket( 
                       const TileMapRequestPacket* req,
                       const vector<TileMapBufferHolder>& tileMapBufferHolder ) 
      : ReplyPacket( MAX_PACKET_SIZE, // FIXME: Calculate this somehow.
                     Packet::PACKETTYPE_TILEMAP_REPLY,
                     req,
                     StringTable::OK )
{
   int pos = REPLY_HEADER_SIZE;
   incWriteLong( pos, tileMapBufferHolder.size() );
   
   // Calculate the approximate size.
   uint32 approxSize = 100 * tileMapBufferHolder.size();
   for ( uint32 i = 0; i < tileMapBufferHolder.size(); ++i ) {
      approxSize += 
         tileMapBufferHolder[ i ].getBuffer()->getCurrentOffset();
   }
   // updateSize requires that the length of the packet is set.
   setLength( pos );
   updateSize( approxSize, approxSize );
   
   for ( uint32 i = 0; i < tileMapBufferHolder.size(); ++i ) {
      const TileMapBufferHolder& holder = tileMapBufferHolder[ i ];
      holder.save( this, pos );
   }
   setLength( pos );
}

TileMapReplyPacket::TileMapReplyPacket( const TileMapRequestPacket* req,
                                        uint32 status )
      : ReplyPacket( REPLY_HEADER_SIZE,
                     Packet::PACKETTYPE_TILEMAP_REPLY,
                     req,
                     status )
{
   setLength( REPLY_HEADER_SIZE );
}

void
TileMapReplyPacket::get( vector<TileMapBufferHolder>& tileMapBufferHolder ) const
{
   int pos = REPLY_HEADER_SIZE;
   uint32 count = incReadLong( pos );
   tileMapBufferHolder.reserve( count );
   for ( uint32 i = 0; i < count; ++i ) {
      TileMapBufferHolder holder( this, pos );
      
      // Add to the vector.
      tileMapBufferHolder.push_back( holder );
   }
}

