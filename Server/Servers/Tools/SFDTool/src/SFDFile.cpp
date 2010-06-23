/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SFDFile.h"
#include "TileCollectionNotice.h"
#include "SFDMultiBufferReader.h"
#include "DeleteHelpers.h"
#include <set>
#include <algorithm>
#include <iterator>

#if 0
SFDFile ::= SFDHeader SFDHeaderBuffsers SFDBuffers

SFDHeader ::= [See SFDSavableHeader] is like 368 bytes today. XXX: Needs more documentation!

SFDHeaderBuffsers ::= str_offset_buf strBuf map_buf_offset_buf mapBuf offsetBuf

SFDBuffers ::= SFDMultiBuffer*

str_offset_buf ::= Offset for each string into strBuf, starts at 0, strlen +1,.. There is SFDHeader::nbrStrings offsets, strings in strBuf, offsets in map_buf_offset_buf and tilemapformatdescs in mapBuf
strBuf ::= The tile params for the tilemapformatdescs in mapBuf starts at the offsets in str_offset_buf relative the start of the strBuf

map_buf_offset_buf ::= Offset for each tilemapformatdescs in mapBuf and the offset is relative the start of the file.

mapBuf ::= tilemapformatdescs

offsetBuf ::= Offset for each SFDMultiBuffer in SFDBuffers relative the start of the file. The SFDHeader::TileMapCollection bbox and details in tileindexes defines the number of offsets and what each offset is.

SFDMultiBuffer ::= [See SFDMultiBufferReader] XXX: Needs documentation!

#endif

/// destroys all items, with a buff() method, passed to operator()(T)
struct DeleteMapBuff {
   template <typename A>
   void operator () (A& item) {
      delete item.buff();
   }
};

/// delete all values in container with MapBuffs in it and clears it.
template <typename T>
void deleteMapBuffCont( T& val ) {
   for_each( val.begin(), val.end(),
             DeleteMapBuff() );
   val.clear();
}


SFDFile::SFDFile( const MC2SimpleString& name, bool debug ) 
      : m_header( name, debug )
{
}

SFDFile::~SFDFile() {
   clear();
}

void
SFDFile::clear( bool deleteBuffers ) {
   if ( deleteBuffers ) {
      // Delete
      deleteMapBuffCont( m_headerMaps );
      deleteMapBuffCont( m_maps );      
   }
   m_headerStrings.clear();
   m_headerMaps.clear();
   m_maps.clear();
}

void
SFDFile::assign( SFDFile& other ) {
   // Remove any old buffers in this first.
   clear();

   // Take all from other
   *this = other;

   // Remove references to buffers in other without deleting them.
   other.clear( false ); 
}

void
SFDFile::setName( const MC2SimpleString& name ) {
   m_header.setName( name );
}

bool
SFDFile::load( BitBuffer& bitBuff ) {
   bool res = true;

   res = m_header.load( bitBuff );

   if ( ! res ) {
      return res;
   }

   mc2dbg << "pos after header " << bitBuff.getCurrentOffset() << endl;

   mc2dbg << "m_header.m_stringsAreNullTerminated " << m_header.stringsAreNullTerminated() << endl;
   mc2dbg << "m_header.m_strIdxEntrySizeBits " << m_header.getStrIdxEntrySizeBits() << endl;
   mc2dbg << "m_header.m_strIdxStartOffset " <<  m_header.getStrIdxStartOffset() << endl;
   mc2dbg << "m_header.m_nbrStrings " << m_header.getNbrStrings() << endl;
   mc2dbg << "m_header.m_strDataStartOffset " << m_header.getStrDataStartOffset() << endl;
   mc2dbg << "m_header.m_bufferIdxStartOffset " << m_header.getBufferIdxStartOffset() << endl;
   mc2dbg << "m_header.m_bufferDataStartOffset " << m_header.getBufferDataStartOffset() << endl;
   mc2dbg << "m_header.m_maxStringSize " << m_header.maxStringSize() << endl;
   // str_offset_buf
   mc2dbg << "pos before str_offset_buf " << bitBuff.getCurrentOffset() << endl;
   for ( uint32 i = 0 ; i < m_header.getNbrStrings() + 1 ; ++i ) {
      bitBuff.readNextBALong();
   }
   // strBuf
   mc2dbg << "pos before strBuf " << bitBuff.getCurrentOffset() << endl;
   for ( uint32 i = 0 ; i < m_header.getNbrStrings() ; ++i ) {
      m_headerStrings.push_back( bitBuff.readNextString() );
   }
   // buf_offset_buf
   mc2dbg << "pos before buf_offset_buf " << bitBuff.getCurrentOffset() << endl;
   vector<uint32> headerMapBuffOffset;
   for ( uint32 i = 0 ; i < m_header.getNbrStrings() + 1 ; ++i ) {
      headerMapBuffOffset.push_back( bitBuff.readNextBALong() );
   }
   // mapBuf
   mc2dbg << "pos before mapBuff " << bitBuff.getCurrentOffset() << endl;
   for ( uint32 i = 0 ; i < m_header.getNbrStrings() ; ++i ) {
      // Header TileMap (TileMapFormatDesc)
      int size = headerMapBuffOffset [ i + 1 ] - headerMapBuffOffset[ i ];
      BitBuffer* mapBuff = new BitBuffer( size );
      mapBuff->writeNextByteArray( bitBuff.readNextByteArray( size ), size );
      m_headerMaps.push_back( MapBuff( "", mapBuff ) );
   }
   mc2dbg << "pos after mapBuff " << bitBuff.getCurrentOffset() 
          << " buffOffset " << headerMapBuffOffset.back() << endl;
   // offsetBuf
   //  offset for each tile in the tile collection
   //  Get min, max from TileCollectionNotice...
   const vector<TileCollectionNotice>& tileCollection =
      m_header.getTileCollection();
   uint32 nbrOffsets = 0;
   vector<uint32> mapOffsets;
   vector<uint32> mapSizes; // Used if version > 0
   for ( uint32 i = 0 ; i < tileCollection.size() ; ++i ) {
      const vector<TilesForAllDetailsNotice>& tileDetail =
         tileCollection[ i ].getTilesForAllDetails();
      for ( uint32 j = 0 ; j < tileDetail.size() ; ++j ) {
         const vector<TilesNotice>& tileNotices = 
            tileDetail[ j ].getTilesNotices();
         for ( uint32 k = 0 ; k < tileNotices.size() ; ++k ) {
            int startLatIdx = tileNotices[ k ].getStartLatIdx();
            int endLatIdx = tileNotices[ k ].getEndLatIdx();
            int startLonIdx = tileNotices[ k ].getStartLonIdx();
            int endLonIdx = tileNotices[ k ].getEndLonIdx();
            int nbrLayers = tileNotices[ k ].getNbrLayers();
            nbrOffsets += 
               (endLatIdx - startLatIdx + 1) * (endLonIdx-startLonIdx + 1);
            mc2dbg << "  lat " << startLatIdx << " - " << endLatIdx << endl;
            mc2dbg << "  lon " << startLonIdx << " - " << endLonIdx << endl;
            mc2dbg << "  nbrLayers " << nbrLayers << endl;
            //mc2dbg << "nbrOffsets += " << ((endLatIdx - startLatIdx + 1) * (endLonIdx-startLonIdx + 1))  << endl;
            for ( int32 lat = startLatIdx ; lat <= endLatIdx ; ++lat ) {
               for ( int32 lon = startLonIdx ; lon <= endLonIdx ; ++lon ) {
                  mapOffsets.push_back( bitBuff.readNextBALong() );
                  if ( m_header.getVersion() > 0 ) {
                     mapSizes.push_back( bitBuff.readNextBALong() );
                  }
               } // End for all lon
            } // End for all lat
            mc2dbg << "TileNotice " << k << " done" << endl;
         } // End for all TilesNotice
         mc2dbg << "TileDetailNotice " << j << " done" << endl;
      } // End for all TilesForAllDetailsNotice
      
   } // End for all TileCollectionNotice
   mapOffsets.push_back( bitBuff.readNextBALong() );

   mc2dbg << "Nbr offsets = " << (nbrOffsets+1) << endl;
         
   mc2dbg << "pos after offsetBuff " << bitBuff.getCurrentOffset() << endl;


   uint32 nbrMultiBuffers = 0;
   uint32 nbrEmptyMultiBuffers = 0;
   uint32 nbrBytesInMultiBuffers = 0;
   for ( uint32 i = 0 ; i < tileCollection.size() ; ++i ) {
      const vector<TilesForAllDetailsNotice>& tileDetail =
         tileCollection[ i ].getTilesForAllDetails();
      for ( uint32 j = 0 ; j < tileDetail.size() ; ++j ) {
         const vector<TilesNotice>& tileNotices = 
            tileDetail[ j ].getTilesNotices();
         for ( uint32 det = 0 ; det < tileNotices.size() ; ++det ) {
            int startLatIdx = tileNotices[ det ].getStartLatIdx();
            int endLatIdx = tileNotices[ det ].getEndLatIdx();
            int startLonIdx = tileNotices[ det ].getStartLonIdx();
            int endLonIdx = tileNotices[ det ].getEndLonIdx();
            int nbrLayers = tileNotices[ det ].getNbrLayers();
            mc2dbg2 << "  lat " << startLatIdx << " - " << endLatIdx << endl;
            mc2dbg2 << "  lon " << startLonIdx << " - " << endLonIdx << endl;
            mc2dbg2 << "  nbrLayers " << nbrLayers << endl;

            for ( int32 lat = startLatIdx ; lat <= endLatIdx ; ++lat ) {
               for ( int32 lon = startLonIdx ; lon <= endLonIdx ; ++lon ) {
                  const TileMapParams param( 
                     9/*serverPrefix*/, 1/*gzip*/, 0/*layer*/, 
                     TileMapTypes::tileMapData, 0/*imp*//*importanceNbr*/, 
                     LangTypes::english,
                     lat/*tileIndexLat*/, lon, det/*detailLevel*/ ); // XXX: 
                  // If ver 1 use the size in offset buff, not the offset to
                  // next offset as size of buffer as offsets might not be
                  // sequential.
                  int size = 0;
                  if ( m_header.getVersion() > 0 ) {
                     size = mapSizes[ nbrMultiBuffers ];
                  } else {
                     size = mapOffsets[ nbrMultiBuffers + 1 ] - 
                        mapOffsets[ nbrMultiBuffers ];
                  }
                  nbrBytesInMultiBuffers += size;
                  BitBuffer* mapBuff = new BitBuffer( size );
                  bitBuff.reset();
                  bitBuff.readPastBytes( mapOffsets[ nbrMultiBuffers ] );
                  mapBuff->writeNextByteArray( 
                     bitBuff.readNextByteArray( size ), size );
                  m_maps.push_back( 
                     MapBuff( param.getAsString(), 
                              mapBuff ) );
                  nbrMultiBuffers++;
                  nbrEmptyMultiBuffers = nbrEmptyMultiBuffers;
                  
               } // End for all lon
            } // End for all lat
            mc2dbg2 << "TileNotice " << det << " done" << endl;
         } // End for all TilesNotice
         mc2dbg2 << "TileDetailNotice " << j << " done" << endl;
      } // End for all TilesForAllDetailsNotice
   } // End for all TileCollectionNotice

   mc2dbg << "pos after all multiBuff " << bitBuff.getCurrentOffset() 
          << " buffer is " << bitBuff.getBufferSize() << endl;

   mc2dbg << "nbrBytesInMultiBuffers " << nbrBytesInMultiBuffers
          << " nbrMultiBuffers " << nbrMultiBuffers << endl;

   return res;
}


bool ordIn( int l, int min, int max ) {
   return l <= max && l >= min;
}

bool
SFDFile::save( BitBuffer& bitBuff, int wfdVersion ) {
   bool res = true;

   bool useCrcToReduceBuffers = wfdVersion > 0;
   m_header.setVersion( wfdVersion );

   //uint32 buffStartPos = bitBuff.getCurrentOffset();

   // Header must be up to date!
   SharedBuffer header_buf( 10*1024 );
   // FIXME: Add variable data before finding out the size.
   // First find out the size of the header.
   uint32 headerSize = m_header.save( header_buf );
   header_buf.reset();
   uint32 headerStringsSize = 0;
   for ( uint32 i = 0 ; i < m_header.getNbrStrings() ; ++i ) {
      headerStringsSize += m_headerStrings[ i ].length() + 1;
   }
   uint32 headerMapOffsetsSize = (m_headerMaps.size() + 1)*4;
   uint32 headerMapBuffSize = 0;
   for ( mapBuff_t::const_iterator it = m_headerMaps.begin() ; 
         it != m_headerMaps.end() ; ++it ) {
      headerMapBuffSize += it->buff()->getBufferSize();
   }
   m_header.setStrIdxStartOffset( headerSize );
   m_header.setStrDataStartOffset( m_header.getStrIdxStartOffset() +
                                   (m_header.getNbrStrings() + 1)*4 );

   m_header.setBufferIdxStartOffset( m_header.getStrDataStartOffset() +
                                     headerStringsSize );
   m_header.setBufferDataStartOffset( m_header.getBufferIdxStartOffset() +
                                      headerMapOffsetsSize );
   m_header.setMultiBufferOffsetStartOffset( 
      m_header.getBufferDataStartOffset() + headerMapBuffSize );
   // The m_header.save() call above set multiBufferStartOffset
   // m_header.setMultiBufferStartOffset( );

   // Save header
   m_header.save( bitBuff );

   mc2dbg << "pos after header " << bitBuff.getCurrentOffset() << endl;

   mc2dbg << "m_header.m_stringsAreNullTerminated " << m_header.stringsAreNullTerminated() << endl;
   mc2dbg << "m_header.m_strIdxEntrySizeBits " << m_header.getStrIdxEntrySizeBits() << endl;
   mc2dbg << "m_header.m_strIdxStartOffset " <<  m_header.getStrIdxStartOffset() << endl;
   mc2dbg << "m_header.m_nbrStrings " << m_header.getNbrStrings() << endl;
   mc2dbg << "m_header.m_strDataStartOffset " << m_header.getStrDataStartOffset() << endl;
   mc2dbg << "m_header.m_bufferIdxStartOffset " << m_header.getBufferIdxStartOffset() << endl;
   mc2dbg << "m_header.m_bufferDataStartOffset " << m_header.getBufferDataStartOffset() << endl;
   mc2dbg << "m_header.m_maxStringSize " << m_header.maxStringSize() << endl;
   // str_offset_buf
   mc2dbg << "pos before str_offset_buf " << bitBuff.getCurrentOffset() << endl;

   mc2dbg << "getTileCollection size " << m_header.getTileCollection().size()
          << endl;

   // str_offset_buf
   uint32 strOffsetPos = 0;
   mc2dbg << "m_header.getNbrStrings() " << m_header.getNbrStrings()
          << " nbr strings " << m_headerStrings.size() << endl;
   for ( uint32 i = 0 ; i < m_header.getNbrStrings() ; ++i ) {
      bitBuff.writeNextBALong( strOffsetPos );
      strOffsetPos += m_headerStrings[ i ].length() + 1;
   }
   bitBuff.writeNextBALong( strOffsetPos );

   // strBuf
   for ( uint32 i = 0 ; i < m_header.getNbrStrings() ; ++i ) {
      bitBuff.writeNextString( m_headerStrings[ i ] );
   }

   // header map offset buf
   uint32 headerMapOffsetPos = bitBuff.getCurrentOffset() + 
      headerMapOffsetsSize;
   for ( mapBuff_t::const_iterator it = m_headerMaps.begin() ; 
         it != m_headerMaps.end() ; ++it ) {
      bitBuff.writeNextBALong( headerMapOffsetPos );
      headerMapOffsetPos += it->buff()->getBufferSize();
   }
   bitBuff.writeNextBALong( headerMapOffsetPos );

   // header mapBuf
   for ( mapBuff_t::const_iterator it = m_headerMaps.begin() ; 
         it != m_headerMaps.end() ; ++it ) {
      bitBuff.writeNextByteArray( it->buff()->getBufferAddress(),
                                  it->buff()->getBufferSize() );
   }
   mc2dbg << "pos after mapBuff " << bitBuff.getCurrentOffset() << endl;

   // offsetBuf
   uint32 offsetPos = bitBuff.getCurrentOffset() + (m_maps.size() + 1) * 
      ( wfdVersion == 0 ? 4 : 8 );
   const vector<TileCollectionNotice>& tileCollection =
      m_header.getTileCollection();
   mapBuff_t::const_iterator mapIt = m_maps.begin();
   typedef map< uint32, pair< uint32, uint32 > > addedCrcAndOffsetmap_t;
   addedCrcAndOffsetmap_t addedCrcAndOffset;
   for ( uint32 i = 0 ; i < tileCollection.size() ; ++i ) {
      const vector<TilesForAllDetailsNotice>& tileDetail =
         tileCollection[ i ].getTilesForAllDetails();
      for ( uint32 j = 0 ; j < tileDetail.size() ; ++j ) {
         const vector<TilesNotice>& tileNotices = 
            tileDetail[ j ].getTilesNotices();
         for ( uint32 k = 0 ; k < tileNotices.size() ; ++k ) {
            int startLatIdx = tileNotices[ k ].getStartLatIdx();
            int endLatIdx = tileNotices[ k ].getEndLatIdx();
            int startLonIdx = tileNotices[ k ].getStartLonIdx();
            int endLonIdx = tileNotices[ k ].getEndLonIdx();
            for ( int32 lat = startLatIdx ; lat <= endLatIdx ; ++lat ) {
               for ( int32 lon = startLonIdx ; lon <= endLonIdx ; ++lon ) {
                  MC2_ASSERT( mapIt->buff() != NULL );
                  uint32 offsetToWrite = offsetPos;
                  uint32 mapSize = mapIt->buff()->getBufferSize();
                  addedCrcAndOffsetmap_t::const_iterator findIt =
                     addedCrcAndOffset.find( mapIt->crc() );
                  if ( useCrcToReduceBuffers &&
                       findIt != addedCrcAndOffset.end() ) {
                     // Reuse old offset
                     offsetToWrite = findIt->second.first;
                     mapSize = findIt->second.second;
                  } else {
                     addedCrcAndOffset.insert( 
                        make_pair( mapIt->crc(), 
                                   make_pair( offsetPos, 
                                              mapIt->buff()->getBufferSize() ) ) );
                     offsetPos += mapIt->buff()->getBufferSize();
                  }
                  bitBuff.writeNextBALong( offsetToWrite );
                  if ( useCrcToReduceBuffers ) {
                     bitBuff.writeNextBALong( mapSize );
                  }
                  mapIt++;
               } // End for all lon
            } // End for all lat

         } // End for all TilesNotice
         
      } // End for all TilesForAllDetailsNotice
   } // End for all TileCollectionNotice
   bitBuff.writeNextBALong( offsetPos );
   if ( useCrcToReduceBuffers ) {
      bitBuff.writeNextBALong( offsetPos );
   }
   mc2dbg << "pos after offsetBuff " << bitBuff.getCurrentOffset() << endl;

   // Checking how many crcs that is same for debug print
   set<uint32> crcSet;
   uint32 nbrSameCrc = 0;
   for ( mapBuff_t::const_iterator it = m_maps.begin() ; it != m_maps.end() ;
         ++it ) {
      pair< set<uint32>::iterator, bool > res = crcSet.insert( it->crc() );
      if ( ! res.second ) {
         nbrSameCrc++;
      }
   }
   mc2dbg << "Same map crcs " << nbrSameCrc << endl;

   // MultiMaps
   mapIt = m_maps.begin();
   addedCrcAndOffset.clear();
   for ( uint32 i = 0 ; i < tileCollection.size() ; ++i ) {
      const vector<TilesForAllDetailsNotice>& tileDetail =
         tileCollection[ i ].getTilesForAllDetails();
      for ( uint32 j = 0 ; j < tileDetail.size() ; ++j ) {
         const vector<TilesNotice>& tileNotices = 
            tileDetail[ j ].getTilesNotices();
         for ( uint32 k = 0 ; k < tileNotices.size() ; ++k ) {
            int startLatIdx = tileNotices[ k ].getStartLatIdx();
            int endLatIdx = tileNotices[ k ].getEndLatIdx();
            int startLonIdx = tileNotices[ k ].getStartLonIdx();
            int endLonIdx = tileNotices[ k ].getEndLonIdx();
            for ( int32 lat = startLatIdx ; lat <= endLatIdx ; ++lat ) {
               for ( int32 lon = startLonIdx ; lon <= endLonIdx ; ++lon ) {
                  MC2_ASSERT( mapIt->buff() != NULL );
                  addedCrcAndOffsetmap_t::const_iterator findIt =
                     addedCrcAndOffset.find( mapIt->crc() );
                  if ( useCrcToReduceBuffers &&
                       findIt != addedCrcAndOffset.end() ) {
                     // Buffer already added
                  } else {
                     addedCrcAndOffset.insert( 
                        make_pair( mapIt->crc(), 
                                   make_pair( 
                                      bitBuff.getCurrentOffset()/*Not used*/,
                                      0/*Not used*/ ) ) );
                     bitBuff.writeNextByteArray( 
                        mapIt->buff()->getBufferAddress(),
                        mapIt->buff()->getBufferSize() );
                  }
                  mapIt++;
               } // End for all lon
            } // End for all lat

         } // End for all TilesNotice
         
      } // End for all TilesForAllDetailsNotice
   } // End for all TileCollectionNotice

   mc2dbg << "pos after all multiBuff " << bitBuff.getCurrentOffset() 
          << " buffer is " << bitBuff.getBufferSize() << endl;
   m_header.writeFileSize( bitBuff.getCurrentOffset(), bitBuff );

   return res;
}

bool
SFDFile::merge( SFDFile& other ) {
   bool res = true;

   // Merge Headers 
   // Asset on not same settings (stringsAreNullTerminated,
   // strIdxEntrySizeBits, readDebugParams) that we don't support to merge.
   MC2_ASSERT( m_header.stringsAreNullTerminated() == 
               other.m_header.stringsAreNullTerminated() );
   MC2_ASSERT( m_header.getStrIdxEntrySizeBits() == 
               other.m_header.getStrIdxEntrySizeBits() );
   MC2_ASSERT( m_header.readDebugParams() == 
               other.m_header.readDebugParams() );
   // Also assert on header strings (and header map buffers) not same.
   MC2_ASSERT( m_header.getNbrStrings() == other.m_header.getNbrStrings() );
   for ( uint32 i = 0 ; i < m_header.getNbrStrings() ; ++i ) {
      mc2dbg << "m_headerStrings[ i ] " << m_headerStrings[ i ] 
             << " other.m_headerStrings[ i ] " <<  other.m_headerStrings[ i ]
             << endl;
      MC2_ASSERT( m_headerStrings[ i ] == other.m_headerStrings[ i ] );
      // TileMapFormatDesc has timeStamp in them so a simple compare won't do
      // might be able to get crc stored in the TileMapFormatDesc and check.
      // MC2_ASSERT( m_headerMaps[ i ] == other.m_headerMaps[ i ] );
   }

   // Set creationTime (or keep it? Set in SFDSavableHeader::save anyway)

   // maxStringSize
   m_header.setMaxStringSize( max( m_header.maxStringSize(), 
                                   other.m_header.maxStringSize() ) );
   // initialCharacters
   mc2dbg << "m_initialCharacters ";
   const vector<char>& initialCharacters = m_header.getInitialCharacters();
   copy( initialCharacters.begin(), initialCharacters.end(), 
         ostream_iterator<char>(mc2dbg, " " ) );
   mc2dbg << " Other ";
   const vector<char>& otherInitialCharacters = 
      other.m_header.getInitialCharacters();
   copy( otherInitialCharacters.begin(), otherInitialCharacters.end(), 
         ostream_iterator<char>(mc2dbg, " " ) );
   mc2dbg << endl;
   
   // routeIDs (assert if present)
   const vector<RouteID>& routeIDs = m_header.getRouteIDs();
   const vector<RouteID>& otherRouteIDs = other.m_header.getRouteIDs();
   MC2_ASSERT( routeIDs.empty() );
   MC2_ASSERT( otherRouteIDs.empty() );   

   // Placement of other: above, below, left, right? (Assert if not adjacent)
   const vector<TileCollectionNotice>& tileCollection =
      m_header.getTileCollection();
   const vector<TileCollectionNotice>& otherTileCollection =
      other.m_header.getTileCollection();
   // Support only one TileCollection
   MC2_ASSERT( tileCollection.size() == 1 );
   MC2_ASSERT( tileCollection.size() == otherTileCollection.size() );
   const vector<TilesForAllDetailsNotice>& tileDetail = 
      tileCollection[ 0 ].getTilesForAllDetails();
   const vector<TilesForAllDetailsNotice>& otherTileDetail = 
      otherTileCollection[ 0 ].getTilesForAllDetails();
   MC2_ASSERT( tileDetail.size() == 1 );
   MC2_ASSERT( tileDetail.size() == otherTileDetail.size() );
   const vector<TilesNotice>& tileNotices = tileDetail[ 0 ].getTilesNotices();
   const vector<TilesNotice>& otherTileNotices = 
      otherTileDetail[ 0 ].getTilesNotices();
   // Size check for tileNotices? (8 detail levels today)
   MC2_ASSERT( tileNotices.size() == otherTileNotices.size() );
   const TilesNotice& tilesNotice0 = tileNotices[ 0 ];
   const TilesNotice& otherTilesNotice0 = otherTileNotices[ 0 ];
   const int thisHeight = tilesNotice0.getEndLatIdx() - 
      tilesNotice0.getStartLatIdx();
   const int thisWidth = 
      tilesNotice0.getEndLonIdx() - tilesNotice0.getStartLonIdx();
   const int otherHeight = 
      otherTilesNotice0.getEndLatIdx() - otherTilesNotice0.getStartLatIdx();
   const int otherWidth = 
      otherTilesNotice0.getEndLonIdx() - otherTilesNotice0.getStartLonIdx();
   mc2dbg << "thisHeight " << thisHeight << " otherHeight " << otherHeight 
          << endl;
   mc2dbg << "thisWidth " << thisWidth << " otherWidth " << otherWidth 
          << endl;
   // [end|start][Lat|Lon] same?
   const vector<TilesNotice>* firstTileNotices = NULL;
   const vector<TilesNotice>* secondTileNotices = NULL;

   if ( 
      otherTilesNotice0.getStartLatIdx() == tilesNotice0.getEndLatIdx() &&
      ordIn( otherTilesNotice0.getStartLonIdx(), 
             tilesNotice0.getStartLonIdx(), tilesNotice0.getEndLonIdx() ) &&
      ordIn( otherTilesNotice0.getEndLonIdx(), 
             tilesNotice0.getStartLonIdx(), tilesNotice0.getEndLonIdx() ) ) {
      mc2dbg << "Other is above" << endl;
      firstTileNotices = &tileNotices;
      secondTileNotices = &otherTileNotices;
   } else if (
      otherTilesNotice0.getEndLatIdx() == tilesNotice0.getStartLatIdx() &&
      ordIn( otherTilesNotice0.getStartLonIdx(), 
             tilesNotice0.getStartLonIdx(), tilesNotice0.getEndLonIdx() ) &&
      ordIn( otherTilesNotice0.getEndLonIdx(), 
             tilesNotice0.getStartLonIdx(), tilesNotice0.getEndLonIdx() ) ) {
      mc2dbg << "Other is below" << endl;
      firstTileNotices = &otherTileNotices;
      secondTileNotices = &tileNotices;
   } else if (
      otherTilesNotice0.getStartLonIdx() == tilesNotice0.getEndLonIdx() &&
      ordIn( otherTilesNotice0.getStartLatIdx(), 
             tilesNotice0.getStartLatIdx(), tilesNotice0.getEndLatIdx() ) &&
      ordIn( otherTilesNotice0.getEndLatIdx(), 
             tilesNotice0.getStartLatIdx(), tilesNotice0.getEndLatIdx() ) ) {
      mc2dbg << "Other is right" << endl;
      firstTileNotices = &tileNotices;
      secondTileNotices = &otherTileNotices;
   } else if (
      otherTilesNotice0.getEndLonIdx() == tilesNotice0.getStartLonIdx() &&
      ordIn( otherTilesNotice0.getStartLatIdx(), 
             tilesNotice0.getStartLatIdx(), tilesNotice0.getEndLatIdx() ) &&
      ordIn( otherTilesNotice0.getEndLatIdx(), 
             tilesNotice0.getStartLatIdx(), tilesNotice0.getEndLatIdx() ) ) {
      mc2dbg << "Other is left" << endl;
      firstTileNotices = &otherTileNotices;
      secondTileNotices = &tileNotices;
   } else if ( 
      ordIn( otherTilesNotice0.getStartLatIdx(), 
             tilesNotice0.getStartLatIdx(), tilesNotice0.getEndLatIdx() ) &&
      ordIn( otherTilesNotice0.getEndLatIdx(), 
             tilesNotice0.getStartLatIdx(), tilesNotice0.getEndLatIdx() ) &&
      ordIn( otherTilesNotice0.getStartLonIdx(), 
             tilesNotice0.getStartLonIdx(), tilesNotice0.getEndLonIdx() ) &&
      ordIn( otherTilesNotice0.getEndLonIdx(), 
             tilesNotice0.getStartLonIdx(), tilesNotice0.getEndLonIdx() ) ) {
      mc2dbg << "Other is contained in this (filling in missing area)" << endl;
      firstTileNotices = &tileNotices;
      secondTileNotices = &otherTileNotices;
   } else {
      mc2log << fatal << "[sfdf] sfds are not adjacent!" 
             << endl;
      mc2dbg << " thisStartLatIdx = " 
             << tileNotices[ 0 ].getStartLatIdx() << endl;
      mc2dbg <<" thisEndLatIdx = " 
             << tileNotices[ 0 ].getEndLatIdx() << endl;
      mc2dbg << " thisStartLonIdx = " 
             << tileNotices[ 0 ].getStartLonIdx() << endl;
      mc2dbg << " thisEndLonIdx = " 
             << tileNotices[ 0 ].getEndLonIdx() << endl;
      mc2dbg << " otherStartLatIdx = " 
             << otherTileNotices[ 0 ].getStartLatIdx() << endl;
      mc2dbg << " otherEndLatIdx = "
             << otherTileNotices[ 0 ].getEndLatIdx() << endl;
      mc2dbg << " otherStartLonIdx = "
             << otherTileNotices[ 0 ].getStartLonIdx() << endl;
      mc2dbg << " otherEndLonIdx = "
             << otherTileNotices[ 0 ].getEndLonIdx() << endl;
      MC2_ASSERT( 0 == 1 );
   }

   // Merge map buffers per row/column
   mapBuff_t newMaps;

   mapBuff_t::const_iterator firstMapIt = firstTileNotices == &tileNotices ? 
      m_maps.begin() : other.m_maps.begin();
   mapBuff_t::const_iterator secondMapIt = firstTileNotices == &tileNotices ? 
      other.m_maps.begin() : m_maps.begin();
   uint32 tileNoticeOffset = m_header.getMultiBufferOffsetStartOffset();
   // Detail (Same for both sfds)
   for ( uint32 det = 0 ; det < tileNotices.size() ; ++det ) {
      int firstStartLatIdx = (*firstTileNotices)[ det ].getStartLatIdx();
      int firstEndLatIdx = (*firstTileNotices)[ det ].getEndLatIdx();
      int firstStartLonIdx = (*firstTileNotices)[ det ].getStartLonIdx();
      int firstEndLonIdx = (*firstTileNotices)[ det ].getEndLonIdx();
      int secondStartLatIdx = (*secondTileNotices)[ det ].getStartLatIdx();
      int secondEndLatIdx = (*secondTileNotices)[ det ].getEndLatIdx();
      int secondStartLonIdx = (*secondTileNotices)[ det ].getStartLonIdx();
      int secondEndLonIdx = (*secondTileNotices)[ det ].getEndLonIdx();

      // All the latitudes and longitudess for both
      int minLat = min( firstStartLatIdx, secondStartLatIdx );
      int maxLat = max( firstEndLatIdx, secondEndLatIdx );
      int minLon = min( firstStartLonIdx, secondStartLonIdx );
      int maxLon = max( firstEndLonIdx, secondEndLonIdx );
      mc2dbg << "Lat " << minLat << " - " << maxLat << endl;
      mc2dbg << "Lon " << minLon << " - " << maxLon << endl;
      for ( int32 lat = minLat ; lat <= maxLat ; ++lat ) {
         for ( int32 lon = minLon ; lon <= maxLon ; ++lon ){
            bool inFirst = ordIn( lat, firstStartLatIdx, firstEndLatIdx ) &&
               ordIn( lon, firstStartLonIdx, firstEndLonIdx );
            bool inSecond = ordIn( lat, secondStartLatIdx, secondEndLatIdx ) &&
               ordIn( lon, secondStartLonIdx, secondEndLonIdx );
            // If lat,lon in first and buff not NULL use first
            if ( inFirst && firstMapIt->buff() != NULL ) {
               newMaps.push_back( *firstMapIt );
            } else if ( inSecond && secondMapIt->buff() != NULL ) {
            // else if lat,lon in second and buff not NULL use second
               newMaps.push_back( *secondMapIt );
            } else {
            // else use NULL buffer (and check when saving for NULL buffers
            //      and die with error, because sfds must be square!)
               newMaps.push_back( MapBuff( "", NULL, 0 ) );
            }
            if ( inFirst ) {
               firstMapIt++;
            }
            if ( inSecond ) {
               secondMapIt++;
            }
         } // End for all lon
      } // End for all lat

      // Merge tile collections.
      // Asserts for other attributes: nbrLayers impRange
      MC2_ASSERT( tileNotices[ det ].getNbrLayers() == 
                  otherTileNotices[ det ].getNbrLayers() );
      for ( int i = 0 ; i < tileNotices[ det ].getNbrLayers() ; ++i ) {
         MC2_ASSERT( tileNotices[ det ].getFirstImportanceNbr( i ) == 
                     otherTileNotices[ det ].getFirstImportanceNbr( i ) );
         MC2_ASSERT( tileNotices[ det ].getLastImportanceNbr( i ) == 
                     otherTileNotices[ det ].getLastImportanceNbr( i ) );
      }
      const_cast< TilesNotice& > ( tileNotices[ det ] ).updateBBox( 
         minLat, maxLat, minLon, maxLon );
      // new width/height to set new offset (the notices before this has set 
      // tileNoticeOffset so far)
      tileNoticeOffset = const_cast< TilesNotice& > ( tileNotices[ det ] ).
         updateOffset( tileNoticeOffset );
   } // End for all details

   // Clear this and other maps
   m_maps.clear();
   other.m_maps.clear();
   // Set newMaps in this
   m_maps = newMaps;

   return res;
}



#if 0
   // Code for reading a SFDMultiBuffer for a detail, lat and lon.

   mc2dbg << "multiBuff  " << nbrMultiBuffers++ << " lat " << lat << "/" << endLatIdx << " lon " << lon << "/" << endLonIdx << " det " << det << "/" << tileNotices.size() /*<< " imp " << imp << "/" << tileNotices[ k ].getLastImportanceNbr( 0*lay* )*/ /*<< " lay " << lay << "/" << nbrLayers*/ << " tileNotice " << det << "/" << tileNotices.size() << " tileDetails " << j << "/" << tileDetail.size() << " " << " tileCollection " << i << endl;
   mc2dbg << "pos before multiBuff " << bitBuff.getCurrentOffset() << endl;
   // multiBuf
   auto_ptr<SFDMultiBufferReader> reader( new SFDMultiBufferReader( &bitBuff, param, &m_header ) );
   mc2dbg << "reader.hasNext() " << reader->hasNext() << endl;
   while ( reader->hasNext() ) {
      LangTypes::language_t lang = LangTypes::english; // XXX:
      // Data
      SFDMultiBufferReader::bufPair_t data = reader->readNext( lang );
      m_maps.push_back( 
         MapBuff( data.first.getAsString(), 
                  data.second ) );
      if ( data.second != NULL ) {
         nbrBytesInMultiBuffers += data.second->getBufferSize();
      } else {
         nbrEmptyMultiBuffers++;
      }
   }
   mc2dbg << "pos after multiBuff " << bitBuff.getCurrentOffset() 
          << " buffer is " << bitBuff.getBufferSize() << endl;
#endif
