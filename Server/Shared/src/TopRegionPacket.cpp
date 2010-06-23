/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2BoundingBox.h"
#include "Name.h"

#include "TopRegionPacket.h"

// ============================================================= Request =
TopRegionRequestPacket::TopRegionRequestPacket( uint16 reqID, 
                                                uint16 packetID )
   : RequestPacket( REQUEST_HEADER_SIZE,      // Size
                    TOP_REGION_REQUEST_PRIO, // prio
                    Packet::PACKETTYPE_TOPREGIONREQUEST,
                    packetID,
                    reqID,
                    MAX_UINT32 ) // mapID
{
}

// ============================================================= Reply =

TopRegionReplyPacket::TopRegionReplyPacket( 
                                    const TopRegionRequestPacket* packet,
                                    StringTable::stringCode status )
   : ReplyPacket( MAX_PACKET_SIZE, 
                  Packet::PACKETTYPE_TOPREGIONREPLY,
                  packet,
                  status )
{
   int pos = REPLY_HEADER_SIZE;
   incWriteLong( pos, 0 ); // Nbr of top regions.
   setLength( pos );
}

TopRegionReplyPacket::TopRegionReplyPacket(   
                          const TopRegionRequestPacket* packet,
                          const ItemIDTree& wholeTree,
                          const ConstTopRegionMatchesVector& topRegions )
   : ReplyPacket( getSizeInDataBuffer(topRegions, wholeTree),
                  Packet::PACKETTYPE_TOPREGIONREPLY,
                  packet,
                  StringTable::OK )
{
   int pos = REPLY_HEADER_SIZE;
   incWriteLong( pos, topRegions.size() ); // Nbr of top regions.
   
   // Write all the top regions.
   for ( ConstTopRegionMatchesVector::const_iterator it(topRegions.begin());
         it != topRegions.end(); ++it ) {
      const TopRegionMatch* tr = *it;
      incWriteLong( pos, tr->getType() );
      incWriteLong( pos, tr->getID() );
      incWriteBBox( pos, tr->getBoundingBox() );
      const NameCollection* names = tr->getNames();
      incWriteLong( pos, names->getSize() );
      // All languages:
      for ( uint32 i = 0; i < names->getSize(); i++ ) {
         incWriteLong( pos, names->getName( i )->getLanguage() );         
      }
      // All types:
      for ( uint32 i = 0; i < names->getSize(); i++ ) {
         incWriteLong( pos, names->getName( i )->getType() );         
      }
      // All names:
      for ( uint32 i = 0; i < names->getSize(); i++ ) {
         incWriteString( pos, names->getName( i )->getName() );         
      }
      // The map hierarchy for the top region
      tr->getItemIDTree().save(this, pos);
   }

   // And save the idtree of all maps.
   wholeTree.save(this, pos);
   
   
   setLength( pos );
}

int
TopRegionReplyPacket::
privateGetTopRegions( TopRegionMatchesVector& topRegions ) const
{
   int pos = REPLY_HEADER_SIZE;

   uint32 nbrRegions = incReadLong( pos );

   // Read all the top regions.
   for ( uint32 i = 0; i < nbrRegions; i++ ) {
      TopRegionMatch::topRegion_t type = 
          (TopRegionMatch::topRegion_t)incReadLong( pos );
      uint32 id = incReadLong( pos );
      MC2BoundingBox bbox;
      incReadBBox( pos, bbox );
      uint32 nbrNames = incReadLong( pos );

      // All languages:
      uint32* languages = new uint32[ nbrNames ];
      for ( uint32 i = 0; i < nbrNames; i++ ) {
         languages[ i ] = incReadLong( pos );
      }  

      // All types
      uint32* types = new uint32[ nbrNames ];
      for ( uint32 i = 0; i < nbrNames; i++ ) {
         types[ i ] = incReadLong( pos );
      }  
         
      // All names
      char** names = new char*[ nbrNames ];
      for ( uint32 i = 0; i < nbrNames; i++ ) {
         incReadString( pos, names[ i ] );
      }  

      TopRegionMatch* topRegion = new TopRegionMatch( id, type );
      topRegion->setBoundingBox( bbox );

      // Loop once more to fill in all the names
      for ( uint32 i = 0; i < nbrNames; i++ ) {
         topRegion->addName( names[ i ], 
                             LangTypes::language_t( languages[ i ] ), 
                             ItemTypes::name_t( types[ i ] ) );
      }

      // Read the itemIdtree
      topRegion->getItemIDTreeForWriting()->load(this, pos);
      
      // Delete what we have allocated
      delete [] languages; 
      delete [] types;
      delete [] names;
      
      // Add to top region vector.
      topRegions.push_back( topRegion );
   }
   return pos;
}

void
TopRegionReplyPacket::
getTopRegions( TopRegionMatchesVector& topRegions ) const
{
   privateGetTopRegions ( topRegions );
}

void
TopRegionReplyPacket::
getTopRegionsAndIDTree ( TopRegionMatchesVector & topRegions,
                         ItemIDTree& wholeTree ) const
{
   int pos = privateGetTopRegions ( topRegions );
   wholeTree.load( this, pos );
}

uint32
TopRegionReplyPacket::getSizeInDataBuffer( const ConstTopRegionMatchesVector& topRegions,
                                           const ItemIDTree& wholeTree ) const
{
   uint32 size = REPLY_HEADER_SIZE;
   size+=wholeTree.getSizeInPacket();
   size+=4; // Nbr top regiions
   
   for ( ConstTopRegionMatchesVector::const_iterator it(topRegions.begin());
         it != topRegions.end(); ++it ) {
      const TopRegionMatch* tr = *it;
      
      size+=4; // top region type.
      size+=4; // top region ID.
      size+=4*4; // top region bounding box.
      size+=4; // size of names.
      const NameCollection* names = tr->getNames();
      for ( uint32 i = 0; i < names->getSize(); i++ ) {
         size+=4; // name language
         size+=4; // name type
         size+= strlen((names->getName( i )->getName())) + 1; // name
         size+=3; // Compensate for any alignment.
      }
      size+=tr->getItemIDTree().getSizeInDataBuffer();
      size+=3; // Compensate for any alignment.
   }
   size+=wholeTree.getSizeInDataBuffer();
   size+=3; // Compensate for any alignment.

   mc2dbg8 << "TRRP size: " << size << endl;
   return size;
} // getSizeInDataBuffer
