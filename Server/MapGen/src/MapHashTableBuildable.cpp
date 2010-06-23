/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapHashTableBuildable.h"

#include "Item.h"
#include "GfxData.h"

MapHashTableBuildable::
MapHashTableBuildable( uint32 minLon, uint32 maxLon,
                       uint32 minLat, uint32 maxLat,
                       uint32 nbrCellsX, uint32 nbrCellsY,
                       GenericMap* map ):
   MapHashTable( minLon, maxLon,
                 minLat, maxLat,
                 nbrCellsX, nbrCellsY, map ) {
   m_cells.resize( m_nbrVerticalCells );
   for ( uint32 x = 0; x < m_nbrVerticalCells; ++x ) {
      m_cells[ x ].resize( m_nbrHorizontalCells );
   }
}


void MapHashTableBuildable::
addItem( const MC2BoundingBox* bbox, uint32 itemID ) {

   uint32 hHashStart, vHashStart, hHashEnd, vHashEnd;
   
   //Get indexes for hashcells to put elements in
   getHashIndex( bbox->getMinLon(), bbox->getMinLat(), hHashStart, vHashStart );
   getHashIndex( bbox->getMaxLon(), bbox->getMaxLat(), hHashEnd, vHashEnd );

   for( uint32 v = vHashStart; v <= vHashEnd; v++ ) {
      for( uint32 h = hHashStart; h <= hHashEnd; h++ ) {
         m_cells[ v ][ h ].push_back( itemID );
      }
   }
}


void MapHashTableBuildable::addItem( const Item* item ) {
   if ( item == NULL ) {
      return;
   }

   const GfxData* gfx = getGfxData( item );
   if ( gfx == NULL ) {
      return;
   }

   MC2BoundingBox box( false, false );
   gfx->getMC2BoundingBox( box );
   addItem( &box, item->getID() );
}

void MapHashTableBuildable::setItemIDs( ItemIDArray& newIDs, 
                                        const CellOffsetMap& offsetMap ) {
   getItemIDs().swap( newIDs );
   // setup original cells
   for ( uint32 v = 0; v < m_nbrVerticalCells; ++v ) {
      for ( uint32 h = 0; h < m_nbrHorizontalCells; ++h ) {

         MapHashCell& cell = static_cast<MapHashCell&>(*m_hashCell[ v ][ h ] );
         CellOffsetMap::const_iterator offset = 
            offsetMap.find( m_cells[ v ][ h ] );
         MC2_ASSERT( offset != offsetMap.end() );

         cell.m_allItemIDIndex = (*offset).second;
         cell.m_allItemIDSize = (*offset).first.size();
         cell.m_nbrElements = cell.m_allItemIDSize;
      }
   }

   m_cells.clear();
}
