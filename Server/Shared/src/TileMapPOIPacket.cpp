/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileMapPOIPacket.h"
#include "TileMapBufferHolder.h"
#include "POISetProperties.h"

uint32 tileMapBufferHolderVectorSize( const TileMapBufferHolderVector& v );

/**
 * @param a the container to calculate size for
 * @return size in bytes that the container requires in a packet.
 */
uint32 acpContSize( const TileMapPOIRequestPacket::ACPCont& cont ) {
   uint32 size = 4; // nbr poi sets

   for ( TileMapPOIRequestPacket::ACPCont::const_iterator it = 
            cont.begin() ; it != cont.end() ; ++it ) {
      size += it->first.getSizeInPacket();
      size += tileMapBufferHolderVectorSize( *it->second );
   }

   return size;
}

uint32 tileMapBufferHolderVectorSize( const TileMapBufferHolderVector& v ) {
   uint32 size = 4; // vector size
   for ( TileMapBufferHolderVector::const_iterator
            tit = v.begin(); tit != v.end() ; ++tit ) {
      size += tit->getSizeInPacket();
   }

   return size;   
}

TileMapPOIRequestPacket::
TileMapPOIRequestPacket( const ACPCont& data ):
   RequestPacket( REQUEST_HEADER_SIZE + acpContSize( data ),
                  DEFAULT_PACKET_PRIO,
                  Packet::PACKETTYPE_TILEMAPPOI_REQUEST,
                  Packet::NO_PACKET_ID, // Packet ID
                  Packet::MAX_REQUEST_ID,
                  Packet::MAX_MAP_ID )
{
   int pos = REQUEST_HEADER_SIZE;

   // Nbr POI sets
   incWriteLong( pos, data.size() );
   for ( ACPCont::const_iterator it = data.begin(); it != data.end() ; ++it ) {
      // POISet 
      it->first.save( this, pos );
      // Nbr TileMapBufferHolderVector
      incWriteLong( pos, it->second->size() );

      for ( TileMapBufferHolderVector::const_iterator valueIt = 
               it->second->begin(); valueIt != it->second->end(); ++valueIt ) {
         valueIt->save( this, pos );
      }
   }

   setLength( pos );
}

void
TileMapPOIRequestPacket::getData( ACPData& data ) const {
   int pos = REQUEST_HEADER_SIZE;

   uint32 nbrPOISets = incReadLong( pos );
   for ( uint32 setIdx = 0 ; setIdx < nbrPOISets ; ++setIdx ) {
      POISet poiSet( UserEnums::UR_WF );
      poiSet.load( this, pos );

      uint32 nbrTMapBufferHolderVectors = incReadLong( pos );

      data.push_back( make_pair( poiSet, TileMapBufferHolderVector( 
                                 nbrTMapBufferHolderVectors, 
                                 TileMapBufferHolder( NULL, 
                                                      NULL, 0, true ) ) ) );
      // load the buffer
      for ( uint32 holderIdx = 0;
            holderIdx < nbrTMapBufferHolderVectors; 
            ++holderIdx ) {
         data.back().second[ holderIdx ].load( this, pos );
      }
   }
}
                  

TileMapPOIReplyPacket::
TileMapPOIReplyPacket( const TileMapPOIRequestPacket& req,
                       uint32 status,
                       const TileMapBufferHolderVector& a ):
   ReplyPacket( REPLY_HEADER_SIZE + tileMapBufferHolderVectorSize( a ),
                Packet::PACKETTYPE_TILEMAPPOI_REPLY,
                &req,
                status ) {

   int pos = REQUEST_HEADER_SIZE;

   incWriteLong( pos, a.size() );
   for ( TileMapBufferHolderVector::const_iterator
            tit = a.begin(); tit != a.end() ; ++tit ) {
      tit->save( this, pos );
   }

   setLength( pos );
}

void
TileMapPOIReplyPacket::getData( TileMapBufferHolderVector& data ) const {
   int pos = REPLY_HEADER_SIZE;

   // Nbr TileMapBufferHolder
   uint32 nbr = incReadLong( pos );
   for ( uint32 i = 0 ; i < nbr ; ++i ) {
      data.push_back( TileMapBufferHolder( this, pos ) );
   }
}
