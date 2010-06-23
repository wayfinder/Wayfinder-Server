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

#include "TileCollectionNotice.h"
#include "SharedBuffer.h"

#include "ArrayTools.h"


// ---- TilesNotice ----

TilesNotice::TilesNotice() : m_offset( -1 ),
                             m_nbrLayers( 0 ),
                             m_impRange( NULL )
{
   m_startLatIdx = m_startLonIdx = m_endLatIdx = m_endLonIdx = MAX_INT32;
   // Set the offset to an invalid value to indicate
   // that this TilesNotice is uninitialized.
}

TilesNotice::~TilesNotice() 
{
   delete[] m_impRange;
}

TilesNotice&
TilesNotice::operator = ( const TilesNotice& other )
{
   if ( this != &other ) {
      m_offset = other.m_offset;
      m_startLatIdx = other.m_startLatIdx;
      m_endLatIdx = other.m_endLatIdx;
      m_startLonIdx = other.m_startLonIdx;
      m_endLonIdx = other.m_endLonIdx;
      m_nbrLayers = other.m_nbrLayers;

      delete[] m_impRange;
      if ( m_nbrLayers > 0 ) {
         m_impRange = new impRange_t[ m_nbrLayers ];
         memcpy( m_impRange, other.m_impRange, 
                 sizeof( impRange_t ) * m_nbrLayers );
      } else {
         m_impRange = NULL;
      }
   }
   return *this;
}
   
TilesNotice::TilesNotice( const TilesNotice& other )
{
   m_impRange = NULL;
   *this = other;
}

void 
TilesNotice::load( SharedBuffer& buf )
{
   // Offset.
   m_offset = buf.readNextBALong();
   
   // start lat
   m_startLatIdx = buf.readNextBALong();
   // end lat
   m_endLatIdx = buf.readNextBALong();
   // start lon
   m_startLonIdx = buf.readNextBALong();
   // end lon 
   m_endLonIdx = buf.readNextBALong();

   // Size of m_impRange.
   m_nbrLayers = buf.readNextBAShort();
   MC2_ASSERT( m_nbrLayers > 0 );
   delete [] m_impRange;
   m_impRange = new impRange_t[ m_nbrLayers ];
   for ( uint32 i = 0; i < m_nbrLayers; ++i ) {
      m_impRange[ i ].m_layerID = buf.readNextBAShort();
      m_impRange[ i ].m_firstImp = buf.readNextBALong();
      m_impRange[ i ].m_lastImp = buf.readNextBALong();
   } 
}

#ifdef MC2_SYSTEM
void 
TilesNotice::save( SharedBuffer& buf ) const
{
   /* Format:
    * LONG    - offset 
    * LONG    - start lat index
    * LONG    - end lat index
    * LONG    - start lon index
    * LONG    - end lon index
    * SHORT   - number of layers
    * Layer Array {
    *   SHORT - layer id
    *   LONG  - first importance
    *   LONG  - last importance
    * }
    *
    */
   // Offset.
   buf.writeNextBALong( m_offset );

   // start lat
   buf.writeNextBALong( m_startLatIdx );
   // end lat
   buf.writeNextBALong( m_endLatIdx );
   // start lon
   buf.writeNextBALong( m_startLonIdx );
   // end lon 
   buf.writeNextBALong( m_endLonIdx );

   // Store m_impRangeByLayerID.
   // Nbr elements.
   buf.writeNextBAShort( m_nbrLayers );
   
   for ( uint32 i = 0; i < m_nbrLayers; ++i ) {
      // Layer id.
      buf.writeNextBAShort( m_impRange[ i ].m_layerID );
      // First importance.
      buf.writeNextBALong( m_impRange[ i ].m_firstImp );
      // Last importance.
      buf.writeNextBALong( m_impRange[ i ].m_lastImp );
      
   }
}

int TilesNotice::getNextOffset() const 
{
   int width = m_endLonIdx - m_startLonIdx + 1;
   int height = m_endLatIdx - m_startLatIdx + 1;
   return m_offset + width * height * offsetEntrySize;
}

uint32 
TilesNotice::updateOffset( uint32 startOffset ) 
{
   // -1 = uninitialized TilesNotice = very bad!
   MC2_ASSERT( m_offset != -1 );
   m_offset = startOffset;
   return getNextOffset();
}

void 
TilesNotice::updateWithParam( const TileMapParams& param )
{
   if ( m_offset == -1 ) {
      // Not yet initialized => initialize.
      m_offset = 0;
      m_startLatIdx = m_endLatIdx = param.getTileIndexLat();
      m_startLonIdx = m_endLonIdx = param.getTileIndexLon(); 
   } else {
      // Already initialized. Just update the indexes.
      m_startLatIdx = MIN( m_startLatIdx, 
                           param.getTileIndexLat() );
      m_endLatIdx = MAX( m_endLatIdx,
                         param.getTileIndexLat() );
      m_startLonIdx = MIN( m_startLonIdx, 
                           param.getTileIndexLon() );
      m_endLonIdx = MAX( m_endLonIdx,
                         param.getTileIndexLon() );
   }
   // Update the importance range in the notice.
   impRange_t* endIt = m_impRange + m_nbrLayers;
   impRange_t* findit = std::find( m_impRange, endIt, param.getLayer() );
   if ( findit == endIt ) {
      // Layer not added yet. Add.
      impRange_t impRange;
      impRange.m_layerID = param.getLayer();
      impRange.m_firstImp = impRange.m_lastImp = param.getImportanceNbr();
      m_impRange = ArrayTool::addElement( m_impRange, impRange, m_nbrLayers );
   } else {
      // Update the range.
      // First importance.
      findit->m_firstImp = MIN( findit->m_firstImp, 
                                param.getImportanceNbr() );
      // Last importance.
      findit->m_lastImp = MAX( findit->m_lastImp, 
                               param.getImportanceNbr() );
   }
}

void
TilesNotice::updateBBox( int startLatIdx, int endLatIdx, 
                         int startLonIdx, int endLonIdx ) {
   m_startLatIdx = startLatIdx;
   m_endLatIdx = endLatIdx;
   m_startLonIdx = startLonIdx;
   m_endLonIdx = endLonIdx;
}

#endif   
// ---- TilesForAllDetailsNotice ----

void 
TilesForAllDetailsNotice::load( SharedBuffer& buf )
{
   // Start detail.
   m_startDetail = buf.readNextBAShort();

   // The notices.
   uint32 size = buf.readNextBAShort();
   m_tilesNotice.reserve( size );

   for ( uint32 i = 0; i < size; ++i ) {
      TilesNotice notice;
      notice.load( buf );
      m_tilesNotice.push_back( notice );
   }
}

#ifdef MC2_SYSTEM   
void 
TilesForAllDetailsNotice::save( SharedBuffer& buf ) const
{
   // Start detail.
   buf.writeNextBAShort( m_startDetail );

   // The notices.
   buf.writeNextBAShort( m_tilesNotice.size() );

   for ( uint32 i = 0; i < m_tilesNotice.size(); ++i ) {
      m_tilesNotice[ i ].save( buf );
   }
}

uint32 
TilesForAllDetailsNotice::updateOffset( uint32 startOffset ) 
{
   uint32 nextOffset = startOffset;
   for ( uint32 i = 0; i < m_tilesNotice.size(); ++i ) {
      nextOffset = m_tilesNotice[ i ].updateOffset( nextOffset ); 
   }
   return nextOffset;
}

void
TilesForAllDetailsNotice::compactDetailLevels()
{
   // Find the first detail level that contains data, i.e. offset is not -1.
   m_startDetail = 0;
   for ( uint32 i = 0; i != m_tilesNotice.size(); ++i ) {
         
      TilesNotice& notice = m_tilesNotice[ i ];
      if ( notice.m_offset != -1 ) {
         // First real detail found.
         // Set m_startDetail to correct value.
         m_startDetail = i;
         break;
      }
   }
   
   // Store the non empty detail levels in a new vector.
   vector<TilesNotice> tmpNotices;
   for ( uint32 j = m_startDetail; j < m_tilesNotice.size(); ++j ) {
      tmpNotices.push_back( m_tilesNotice[ j ] );
   }

   // And switch this vector with the one in TilesForAllDetailsNotice so
   // that it will not begin with any empty detaillevels.
   m_tilesNotice.swap( tmpNotices );
}
#endif

// ---- TileCollectionNotice ----


uint32 
TileCollectionNotice::load( SharedBuffer& buf )
{
   // Load the m_indexByLayerID map.
   uint32 size = buf.readNextBAShort();
   {for ( uint32 i = 0; i < size; ++i ) {
      // Layer ID.
      int layerID = buf.readNextBAShort();

      // Index.
      int index = buf.readNextBAShort();
     
      m_indexByLayerID[ layerID ] = index;
   }}
  
   // Size of m_tilesForAllDetails.
   size = buf.readNextBAShort();

   m_tilesForAllDetails.reserve( size );

   // Load all the TilesForAllDetailsNotices. 
   {for ( uint32 i = 0; i < size; ++i ) {
      TilesForAllDetailsNotice notice;
      notice.load( buf );
      m_tilesForAllDetails.push_back( notice );
   }}

   return buf.getCurrentOffset();
}

#ifdef MC2_SYSTEM
uint32 
TileCollectionNotice::updateOffset( uint32 startOffset ) {
   uint32 nextOffset = startOffset;
   for ( uint32 i = 0; i  < m_tilesForAllDetails.size(); ++i ) {
      nextOffset = m_tilesForAllDetails[ i ].updateOffset( nextOffset );
   }
   // Note that we have to reserve extra space an extra offset last,
   // so that it is possible to calculate the size of one databuf by
   // subtracting two offsets.
   return nextOffset + TilesNotice::offsetEntrySize;
}

uint32 
TileCollectionNotice::save( SharedBuffer& buf ) const
{
   // Store the m_indexByLayerID map.
   buf.writeNextBAShort( m_indexByLayerID.size() );
   for ( map<int,int>::const_iterator it = m_indexByLayerID.begin();
         it != m_indexByLayerID.end(); ++it ) {

      // Layer ID.
      buf.writeNextBAShort( it->first );

      // Index.
      buf.writeNextBAShort( it->second );
   }
  
   // Size of m_tilesForAllDetails.
   buf.writeNextBAShort( m_tilesForAllDetails.size() );

   // Store all the TilesForAllDetailsNotice. 
   
   for ( uint32 i = 0; i < m_tilesForAllDetails.size(); ++i ) {
      m_tilesForAllDetails[ i ].save( buf );
   }

   return buf.getCurrentOffset();
}

void TileCollectionNotice::makeCompactDetail() 
{
   for_each( m_tilesForAllDetails.begin(), m_tilesForAllDetails.end(),
             mem_fun_ref( &TilesForAllDetailsNotice::compactDetailLevels ) );
}
#endif

const impRange_t* 
TileCollectionNotice::getImpRange( const TileMapParams& param ) const
{
   const TilesNotice* notice = getNotice( param );
   if ( notice != NULL ) {
      // The notice is found.
      return &notice->getImpRange( param );
   } else {
      // Not found.
      return NULL;
   }
}
