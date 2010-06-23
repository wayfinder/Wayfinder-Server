/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileMapConfig.h"
#include "TileMap.h"
#include "TileFeature.h"
#include "TileMapParams.h"
#include "TileMapTypes.h"
#include "TileMapFormatDesc.h"
#include "MC2BoundingBox.h"
#include "GfxConstants.h"
#include "TileArgNames.h"

#include "GunzipUtil.h"

#ifdef MC2_SYSTEM
#include "GzipUtil.h"
#include "MathUtility.h"
#include "MC2CRC32.h"
#include <string.h>
#endif


TileMap::~TileMap()
{
   delete m_params;
   {
//        for ( vector<TileFeature*>::iterator it = m_features.begin();
//              it != m_features.end(); ++it ) {
//           delete *it;
//        }
   }
   {
      if ( m_primitives ) {
//         for ( int i = 0; i < m_nbrPrimitiveLevels; ++i ) {
//            primVect_t& curPrimVect = m_primitives[i];
//              for( primVect_t::iterator jt = curPrimVect.begin();
//                   jt != curPrimVect.end();
//                   ++jt) {
//                 delete *jt;
//              }
//         }
         delete [] m_primitives;
      }
   }
   delete m_otherTypeOfMapParams;
}

#ifdef MC2_SYSTEM
void
TileMap::saveTileMapData( BitBuffer& buf ) const
{
   saveHeader( buf );

   // Nbr features.
   buf.writeNextBits( m_features.size(), 31 );
   mc2dbg2 << "[TileMap]: save NbrFeatures = "
           << m_features.size() << endl;
   const TileFeature* prevFeature = NULL;
   uint32 i = 0;
   for ( vector<TileFeature>::const_iterator it =
         m_features.begin(); it != m_features.end(); ++it, ++i ) {
      it->save( buf, *this, prevFeature );
      // Blargh! Watch out for this.
      prevFeature = &(*it);
   }
}

void
TileMap::loadCategories( BitBuffer& buf, uint32 nbrBitsFeatureIdx ) {
   // Format: see saveCategories

   // Read Header

   buf.alignToByte();
   buf.readNextBALong(); // reserved
   // if "Have categories" bit is 0 then we do not have categories
   // to load.
   if ( buf.readNextBits( 1 ) == 0 ) {
      return;
   }

   // Read Category Header
   uint32 nbrBitsForCategorySize = buf.readNextBits( 4 );
   uint32 nbrBitsPerCategoryID = buf.readNextBits( 4 );

   m_poiCategoryMap.resize( buf.readNextBits( nbrBitsForCategorySize ) );

   // Read Categories
   for ( poiCategoryMap_t::size_type i = 0; 
         i < m_poiCategoryMap.size(); ++i ) {
      poiCategoryMap_t::value_type& item = m_poiCategoryMap[ i ];
      item.first = buf.readNextBits( nbrBitsFeatureIdx );
      item.second = buf.readNextBits( nbrBitsPerCategoryID );
   }
}

void
TileMap::saveCategories( BitBuffer& buf,
                         uint32 nbrBitsForFeatures ) const {
   /*
    * Format ( first align by byte ):
    *
    *            +------------------------------+---------------------------+
    *            |            Data              |     Size in bits          |
    *            +------------------------------+---------------------------+
    * Header     | Reserved, Do not use         |        32                 |
    *            | Have categories              |        1                  |
    *            +------------------------------+---------------------------+
    *              If "Have categories" == 1:
    *            +------------------------------+---------------------------+
    * Categories | #bits for categories         |        4                  |
    * header     | #bits for Category ID        |        4                  |
    *            | Category size                | #bits for categories      |
    *            +------------------------------+---------------------------+
    *              For all categories:
    *            +------------------------------+---------------------------+
    * Categories | Feature ID                   | #bits for features        |
    *            | Category ID                  | #bits for category ID     |
    *            +------------------------------+---------------------------+
    */
   buf.alignToByte();
   buf.writeNextBALong( 0 );
   if ( m_poiCategoryMap.empty() ) {
      buf.writeNextBits( 0, 1 ); // do not have categories
   } else {
      buf.writeNextBits( 1, 1 ); // have categories
      uint32 nbrBitsForCategories =
         MathUtility::getNbrBits( m_poiCategoryMap.size() );

      // determine maximum category ID
      poiCategoryMap_t::value_type::second_type maxNumCategoryID = 0;
      for ( poiCategoryMap_t::size_type i = 0; 
            i < m_poiCategoryMap.size(); ++i ) {
         maxNumCategoryID = max( maxNumCategoryID,
                                 m_poiCategoryMap[ i ].second );
      }

      uint32 nbrBitsForCategoryID = MathUtility::
         getNbrBits( maxNumCategoryID );
      // write header
      buf.writeNextBits( nbrBitsForCategories, 4 );
      buf.writeNextBits( nbrBitsForCategoryID, 4 );
      buf.writeNextBits( m_poiCategoryMap.size(), nbrBitsForCategories );

      // write categories
      for ( poiCategoryMap_t::size_type i = 0; 
            i < m_poiCategoryMap.size(); ++i ) {
         buf.writeNextBits( m_poiCategoryMap[ i ].first,
                            nbrBitsForFeatures );
         buf.writeNextBits( m_poiCategoryMap[ i ].second,
                            nbrBitsForCategoryID );
      }
   }
}


inline void
TileMap::saveExtendedStringTable( BitBuffer& buf,
                                  uint32 nbrBitsPerFeature,
                                  uint32 nbrBitsForStringIndex,
                                  uint32 startOfStringIndex ) const {

   /*
    *
    * Format:
    *
    * +------------------------------+-----------------------+
    * |             Data             |    nbr bits           |
    * +------------------------------+-----------------------+
    * | Have extended strings        |     1                 |
    * +------------------------------+-----------------------+
    *   If "Have extended strings" == 1 then:
    * +------------------------------+-----------------------+
    * | #bits per string type        |     4                 |
    * | #bits for table size         |     4                 |
    * | Table size                   | #bits for table size  |
    * +------------------------------------------------------+
    *   Table:
    * +----------------------------------------------------------------+
    * | Same feature index           |   1  (no bit for first index)   |
    * +----------------------------------------------------------------+
    *
    *  If "same feature index" == 0 then read ( else jump to next table):
    * +------------------------------+-----------------------+
    * | Feature index                | #bits per feature     |
    * +------------------------------------------------------+
    *
    * +------------------------------------------------------+
    * | String type                  | #bits per string type |
    * | String index                 | #bits per index       |
    * +------------------------------------------------------+
    *
    */

   if ( m_extendedStrings.empty() ) {
      // write zero and no more
      buf.writeNextBits( 0, 1 );
      return;
   } else {
      // we do have strings, write '1' and continue
      buf.writeNextBits( 1, 1 );
   }
   uint32 nbrBitsForTableSize = MathUtility::
      getNbrBits( m_extendedStrings.size() );
   uint32 nbrBitsPerName =
      MathUtility::getNbrBits( ExtendedTileString::INVALID_TYPE );

   buf.writeNextBits( nbrBitsPerName, 4 );
   buf.writeNextBits( nbrBitsForTableSize, 4 );
   buf.writeNextBits( m_extendedStrings.size(), nbrBitsForTableSize );

   uint32 strIdx = startOfStringIndex;

   ExtendedTileString::FeatureIndex oldFeatureIndex = 0;

   for ( ExtendedStringTable::const_iterator it =
            m_extendedStrings.begin();
         it != m_extendedStrings.end(); ++it, ++strIdx ) {
      const ExtendedTileString& str = *it;

      // Write same feature bit and/or feature index
      if ( it != m_extendedStrings.begin() ) {
         if ( oldFeatureIndex != str.getFeatureIndex() ) {
            buf.writeNextBits( 0, 1 );
            buf.writeNextBits( str.getFeatureIndex(), nbrBitsPerFeature );
            oldFeatureIndex = str.getFeatureIndex();
         } else {
            buf.writeNextBits( 1, 1 );
         }
      } else {
         buf.writeNextBits( str.getFeatureIndex(), nbrBitsPerFeature );
         oldFeatureIndex = str.getFeatureIndex();
      }

      // write type and string index
      buf.writeNextBits( str.getType(), nbrBitsPerName );
      buf.writeNextBits( strIdx, nbrBitsForStringIndex );
   }
}


inline void
TileMap::loadExtendedStringTable( BitBuffer& buf,
                                  uint32 nbrBitsPerFeature,
                                  uint32 nbrBitsForStringIndex ) {
   // Format: see saveExtendedStringTable

   if ( buf.readNextBits( 1 ) == 0 ) {
      // no extended string table
      return;
   }

   uint32 nbrBitsForStringType = buf.readNextBits( 4 );
   uint32 nbrBitsForTableSize = buf.readNextBits( 4 );
   uint32 tableSize = buf.readNextBits( nbrBitsForTableSize );
   m_extendedStrings.resize( tableSize );


   ExtendedTileString::FeatureIndex featureIndex = 0;

   for ( uint32 i = 0; i < tableSize; ++i ) {

      // read data

      // if i = 0 then we must read featureIndex or
      // if first bit is 0 we must also read a new featureIndex
      if ( i == 0 || buf.readNextBits( 1 ) == 0 ) {
         featureIndex = buf.readNextBits( nbrBitsPerFeature );
      }

      uint32 stringType = buf.readNextBits( nbrBitsForStringType );
      Strings::size_type stringIndex =
         buf.readNextBits( nbrBitsForStringIndex );
      // add to extended strings
      m_extendedStrings.
         push_back( ExtendedTileString( featureIndex,
                                        static_cast<ExtendedTileString::Type>
                                        ( stringType ),
                                        m_strings[ stringIndex ].c_str() ) );
   }
}


bool
TileMap::save( BitBuffer& inbuf, uint32* outCrc, uint32 forcedCRC ) const
{
   /* Format:
    * +----------------------------------+------------------------------+
    * |             Data                 |  size in number of bits      |
    * +----------------------------------+------------------------------+
    *
    * +----------------------------------+
    * |    Main Data structure           |
    * +----------------------------------+------------------------------+
    * | Tile map string (empty)          |         String               |
    * | < Type specific table >          | See Data Specific Table      |
    * |                                  | or String Specific Table 1   |
    * |                                  |                              |
    * | ( Align to byte )                |          0-7                 |
    * | CRC                              |          32                  |
    * | Empty Importances                |          16                  |
    * | < String Type Specific Table 2 > | See String Specific Table 2  |
    * +----------------------------------+------------------------------+
    * | End                              |
    * +----------------------------------+
    *
    *
    * +----------------------------------+
    * | String Specific Table 1:         |
    * +----------------------------------+------------------------------+
    * | # of bits for string index       |           4                  |
    * | # of bits for feature index      |           4                  |
    * | # of features                    | # of bits for feature index  |
    * | # of features in text order      | # of bits for feature index  |
    * | # of strings                     | # of bits for string index   |
    * +----------------------------------+------------------------------+
    * | For ( # of features in text order ):                            |
    * +----------------------------------+------------------------------+
    * | Feature Index                    | # bits per feature index     |
    * | Feature String Index             | # bits per string index      |
    * +----------------------------------+------------------------------+
    * | String table                     | String * ( # of strings )    |
    * +----------------------------------+------------------------------+
    *
    *
    * +----------------------------------+
    * |String Specific Table 2           |
    * +----------------------------------+------------------------------+
    * | If this tilemap has features and is a string tilemap:           |
    * +----------------------------------+------------------------------+
    * | < Categories >                   |  See saveCategories          |
    * | < Extended String table >        |  See saveExtendedStringTable |
    * +----------------------------------+------------------------------+
    *
    *
    * +----------------------------------+
    * | Data Specific Table              |
    * +----------------------------------+------------------------------+
    * | # of features                    |           31                 |
    * +----------------------------------+------------------------------+
    * | for ( # of features ):                                          |
    * +----------------------------------+------------------------------+
    * | Feature                          | See Tile Feature Table       |
    * +----------------------------------+------------------------------+
    *
    *
    * +----------------------------------+
    * | Tile Feature Table               |
    * +----------------------------------+------------------------------+
    * | Same type                        |           1                  |
    * +----------------------------------+------------------------------+
    * | if "Same type" == 0:                                            |
    * -----------------------------------+------------------------------+
    * | Type                             |           8                  |
    * +----------------------------------+------------------------------+
    * |                                  |                              |
    * +----------------------------------+------------------------------+
    * | < Feature Argument >             | See TileFeatureArg.cpp       |
    * +----------------------------------+------------------------------+
    *
    */
   // Make new buffer to that reset will rewind to the start
   // of this map.
   // WARNING! Must skip bytes at the end too!
   BitBuffer buf(inbuf.getBufferAddress() + inbuf.getCurrentOffset(),
                  inbuf.getBufferSize() - inbuf.getCurrentOffset());
   TileMapTypes::tileMap_t tileMapType = m_params->getTileMapType();
   bool useGzip = m_params->useGZip();

   buf.alignToByte();

   // Write an empty string so that the new client can use the
   // old server.
   //buf.writeNextString(m_params->getAsString().c_str());
   buf.writeNextString("");

   // Calculate the number of bits needed for representing
   // the feature index.
   uint32 featureSize = max( m_features.size(),
                             m_strIdxByFeatureIdx.size() );
   uint32 nbrBitsFeatureIdx =
      MathUtility::getNbrBits( featureSize );
   // Calculate the number of bits needed for representing
   // the string index.
   uint32 nbrBitsStrIdx = MathUtility::
      getNbrBits( m_strings.size() + m_extendedStrings.size() );

   uint32 startOfExpandedStrings = m_strings.size();

   if ( tileMapType == TileMapTypes::tileMapData ) {
      saveTileMapData( buf );

   } else if ( tileMapType == TileMapTypes::tileMapStrings ) {
      // Save the strings.

      buf.writeNextBits( nbrBitsStrIdx, 4 );

      // Calculate the number of bits needed for representing
      // the feature index.

      buf.writeNextBits( nbrBitsFeatureIdx, 4 );

      // Write number of features.
      buf.writeNextBits( featureSize, nbrBitsFeatureIdx );

      // Write number of features with strings.
      buf.writeNextBits( m_featureIdxInTextOrder.size(),
                         nbrBitsFeatureIdx );

      // Write number of strings.
      buf.writeNextBits( m_strings.size() +
                         m_extendedStrings.size(), nbrBitsStrIdx );
      // Write pairs of ( feature index, string index ) in
      // the order as specified by m_featureIdxInTextOrder.
      for ( vector<uint32>::const_iterator it =
               m_featureIdxInTextOrder.begin();
            it != m_featureIdxInTextOrder.end(); ++it ) {
         // Feature index.
         buf.writeNextBits( *it, nbrBitsFeatureIdx );

         // String index.
         buf.writeNextBits( m_strIdxByFeatureIdx[ *it ], nbrBitsStrIdx );
      }

      // Write the strings.
      buf.alignToByte();
      for ( Strings::const_iterator it = m_strings.begin();
            it != m_strings.end(); ++it ) {
         buf.writeNextString( it->c_str() );
      }
      for ( ExtendedStringTable::const_iterator it = m_extendedStrings.begin();
            it != m_extendedStrings.end(); ++it ) {
         buf.writeNextString( it->getData().c_str() );
      }
   }

   // All is written. Now align to next byte.
   buf.alignToByte();

   uint32 crc = MAX_UINT32;

   // Add CRC for the features, if present.
   if ( forcedCRC != MAX_UINT32 ) {
      buf.writeNextBALong( forcedCRC );
   } else if ( ! m_features.empty() ) {
      BitBuffer crcBuf( inbuf.getBufferSize() );

      // Check the size of the saved tilemap.
      saveTileMapData( crcBuf );
      crcBuf.alignToByte();
      // Only memset the necessary amount of data since this is slow.
      // Clear the buffer.
      memset( crcBuf.getBufferAddress(), 0, crcBuf.getCurrentOffset() );
      crcBuf.reset();

      // Save again on a zeroed buffer.
      saveTileMapData( crcBuf );
      crcBuf.alignToByte();
      crc = MC2CRC32::crc32( crcBuf.getBufferAddress(),
                             crcBuf.getCurrentOffset() );


      mc2dbg8 << "[TMap]: saving crc " << hex << crc << dec << endl;
      buf.writeNextBALong( crc );
   }

   if ( outCrc != NULL ) {
      *outCrc = crc;
   }

   // Write the empty importances. 16 bits must be enough!
   buf.writeNextBAShort( m_emptyImportances );

   if ( featureSize > 0 &&
        tileMapType == TileMapTypes::tileMapStrings ) {
      // Write category data
      saveCategories( buf, nbrBitsFeatureIdx );
      // Write extended string table
      saveExtendedStringTable( buf,
                               nbrBitsFeatureIdx,
                               nbrBitsStrIdx,
                               startOfExpandedStrings );
   }
   buf.alignToByte();

   const int mapSize = buf.getCurrentOffset();
   // Compress the buffer if gzip should be used.
   if ( useGzip ) {
      mc2dbg8 << "[TMap]: Trying gzip" << endl;
      int inLen = mapSize;
      // Outbuf should be as large as inbuf
      // The problem is that gzip does not stop if the buffer is too
      // small
      BitBuffer* compBuf = new BitBuffer(inLen*2+1024);
      int outLen =
         GzipUtil::gzip(compBuf->getBufferAddress(),
                        compBuf->getBufferSize(),
                        buf.getBufferAddress(),
                        inLen, 9);
      if ( (outLen > 0) && (outLen < inLen) ) {
         // Ugglehack!!
         buf.reset();
         buf.alignToByte();
         buf.writeNextByteArray(compBuf->getBufferAddress(), outLen);
         mc2dbg8 << "[TMap]: Gzip has reduced the size of the map from "
                << inLen << " to " << outLen << " bytes" << endl;
      }
      delete compBuf;
   }

   // Align the buffer to the next byte since we might not have written
   // a complete byte last.
   buf.alignToByte();
   inbuf.readPastBytes(buf.getCurrentOffset());
   inbuf.alignToByte();

   //dump( mc2dbg );
   return true;

}
#endif

bool
TileMap::load( BitBuffer& inbuf,
               const TileMapFormatDesc& desc,
               const MC2SimpleString& paramString)
{
   BitBuffer buf(inbuf.getBufferAddress() + inbuf.getCurrentOffset(),
                  inbuf.getBufferSize() - inbuf.getCurrentOffset());

   uint32 origOffset = inbuf.getCurrentOffset();

   // FIXNE: Ugglehack! use readNextByte and reset instead.
   if ( buf.getBufferAddress()[0] == 0x1f &&
        buf.getBufferAddress()[1] == 0x8b ) {
      // Gzipped !
      int origLength =
         GunzipUtil::origLength(buf.getBufferAddress(), buf.getBufferSize());
      if ( origLength < 0 ) {
         mc2dbg2 << "[TileMap]: origLength < 0" << endl;
      }
      BitBuffer gunzippedBuf(origLength);
      int res =
         GunzipUtil::gunzip(gunzippedBuf.getBufferAddress(),
                            gunzippedBuf.getBufferSize(),
                            buf.getBufferAddress(),
                            buf.getBufferSize());
      mc2dbg4 << "[TileMap]: gunzipres = " << res << endl;
      inbuf.readPastBytes(res);
      bool retVal = load(gunzippedBuf, desc, paramString);
      if ( retVal ) {
         m_loadSize = origLength; // Return the unzipped size.
      } else {
         m_loadSize = 0;
      }
      return retVal;
   }
   // FIXME: Clean up!!
   delete m_params;
   buf.alignToByte();
   // It used to read the params string here but since it took
   // too much room it is supplied when loading instead.
   // Read a string so that it is possible to use and old server.
   buf.readNextString();
   m_params = new TileMapParams( paramString.c_str() );

   // Set the reference coordinate and mc2 scale to the map.
   m_mc2Scale =
      int( desc.getCoordAndScaleForTile(
                                    m_params->getLayer(),
                                    m_params->getDetailLevel(),
                                    m_params->getTileIndexLat(),
                                    m_params->getTileIndexLon(),
                                    m_referenceCoord ) *
            GfxConstants::METER_TO_MC2SCALE );
   // Adjust the reference coord so it is a multiple of the mc2scale.
   snapCoordToPixel( m_referenceCoord );

   TileMapTypes::tileMap_t tileMapType = m_params->getTileMapType();

   int nbrOneCoordArgs = 0;
   uint32 nbrBitsFeatureIdx = 0;
   uint32 nbrFeatures = 0;
   uint32 nbrBitsStrIdx = 0;
   if ( tileMapType == TileMapTypes::tileMapData ) {
      loadHeader( buf );

      // Load nbr features.
      uint32 xnbrFeatures = buf.readNextBits(31);
      m_features.resize( xnbrFeatures );
      m_pixelBoxes.resize( xnbrFeatures );
      m_vectorProxies.resize( xnbrFeatures );
      mc2dbg4 << "[TileMap]: load NbrFeatures = " << hex
             << xnbrFeatures << dec << endl;
      TilePrimitiveFeature* prevFeature = NULL;
      for ( uint32 i = 0; i < xnbrFeatures; ++i ) {
         // Read type and create tilemapfeature of correct
         // dynamic type.
         int nbrCoordsBefore = m_allCoords.size() + nbrOneCoordArgs;
         TileFeature::createFromStream( m_features[i],
                                        buf, desc, *this, prevFeature );
         TileFeature* feature = &(m_features[i]);

         feature->setFeatureNbr( i );
         // Set empty coordinate "vector"
         feature->m_pixelBox     = &m_pixelBoxes[i];
         feature->m_screenCoords = &m_vectorProxies[i];
         *(feature->m_screenCoords) =
            VectorProxy<MC2Point>( m_allScreenCoords,
                                   nbrCoordsBefore,
                                   nbrCoordsBefore );
         if ( feature->getArg( TileArgNames::coord ) ) {
            ++nbrOneCoordArgs;
         }
         prevFeature = feature;
      }

   } else if ( tileMapType == TileMapTypes::tileMapStrings ) {
      // Load the strings.

      // Read the number of bits needed for representing
      // the string index.
      nbrBitsStrIdx = buf.readNextBits( 4 );

      // Read the number of bits needed for representing
      // the feature index.
      nbrBitsFeatureIdx = buf.readNextBits( 4 );

      // Read the number of features.
      nbrFeatures = buf.readNextBits( nbrBitsFeatureIdx );

      // Read the number of features with strings.
      uint32 nbrStringFeatures = buf.readNextBits( nbrBitsFeatureIdx );

      // Read the number of strings.
      uint32 nbrStrings = buf.readNextBits( nbrBitsStrIdx );

      // Resize the vectors.
      m_featureIdxInTextOrder.resize( nbrStringFeatures );
      m_strings.resize( nbrStrings );
      // Default is -1, i.e. that the feature has no strings.
      m_strIdxByFeatureIdx.resize( nbrFeatures, -1 );

      // Read pairs of ( feature index, string index ) in
      // the order as specified by m_featureIdxInTextOrder.

      for ( uint32 i = 0; i < nbrStringFeatures; ++i ) {
         // Feature index.
         uint32 featureIdx = buf.readNextBits( nbrBitsFeatureIdx );

         // String index.
         uint32 strIdx = buf.readNextBits( nbrBitsStrIdx );

         // Add to m_strIdxByFeatureIdx.
         m_strIdxByFeatureIdx[ featureIdx ] = strIdx;

         // Add to m_featureIdxInTextOrder.
         m_featureIdxInTextOrder[ i ] = featureIdx;
      }

      // Read the strings.
      buf.alignToByte();

      for ( uint32 i = 0; i < nbrStrings; ++i ) {
         // FIXME: We could also let the strings point into the
         //        buffer! Spooky.
         m_strings[ i ] = MC2SimpleString( buf.readNextString() );
      }
   }
   buf.alignToByte();

   // Read CRC if features are present.
   m_crc = MAX_UINT32;
   if ( ! empty() ) {
      // Make sure there is room for CRC in the buffer.
      if ( ( buf.getBufferSize() - buf.getCurrentOffset() ) >= 4 ) {
         m_crc = buf.readNextBALong(); // CRC
      }
   }

   // Read the empty importances. 16 bits must be enough!
   if ( ( buf.getBufferSize() - buf.getCurrentOffset() ) >= 2 ) {
      m_emptyImportances = buf.readNextBAShort();

      // if string type and we have feature idx to string table
      if ( tileMapType == TileMapTypes::tileMapStrings &&
           ! m_featureIdxInTextOrder.empty() ) {
         if ( buf.getBufferSize() - buf.getCurrentOffset() >= 2 ) {
            loadCategories( buf, nbrBitsFeatureIdx );
         }
         if ( buf.getCurrentBitOffset() != 0 ||
              buf.getBufferSize() - buf.getCurrentOffset() >= 2 ) {
            loadExtendedStringTable( buf,
                                     nbrBitsFeatureIdx,
                                     nbrBitsStrIdx );
         }
      }
   }

   buf.alignToByte();

   inbuf.readPastBytes(buf.getCurrentOffset());

   m_loadSize = inbuf.getCurrentOffset() - origOffset;

   //dump(mc2dbg);

   // Make it possible to set the values in the coords vector
   m_allScreenCoords.reserve( m_allCoords.size() + nbrOneCoordArgs );
   mc2dbg2 << "[TMAp]: Nbr coords = "
          << ( m_allCoords.size() + nbrOneCoordArgs ) << endl;

   return true;
}

uint32
TileMap::getBufSize() const
{
   return m_loadSize;
}

bool
TileMap::createPrimitives(const TileMapFormatDesc& desc)
{
   if ( m_primitives != NULL ) {
      return false;
   }

   m_minLevelOfPrimitives = desc.getMinLevel();
   m_nbrPrimitiveLevels = desc.getMaxLevel() - desc.getMinLevel() + 1;
   m_primitives = new primVect_t[ m_nbrPrimitiveLevels ];

   // Not implemented yet.
   vector<int> nbrFeaturesPerLevel;
   desc.countNbrFeaturesPerLevel( nbrFeaturesPerLevel, m_features );

   for( uint32 j = 0; j < nbrFeaturesPerLevel.size(); ++j ) {
      m_primitives[j].reserve( nbrFeaturesPerLevel[j] );
   }

   vector<int> nbrAddedFeaturesPerLevel;
   nbrAddedFeaturesPerLevel.resize( nbrFeaturesPerLevel.size() );

   int nbrFeatures = m_features.size();
   vector<TilePrimitiveFeature*> primitives;
   for ( int i = 0; i < nbrFeatures; ++i ) {
      desc.getFeaturesForFeature(primitives,
                                 &m_features[i]);
      for( vector<TilePrimitiveFeature*>::iterator jt = primitives.begin();
           jt != primitives.end();
           ++jt ) {
         int level = 13;
         TilePrimitiveFeature* curPrim = *jt;
         const SimpleArg* levelArg =
            static_cast<const SimpleArg*>
            (curPrim->getArg(TileArgNames::level));
         if ( levelArg != NULL ) {
            level = levelArg->getValue();
         }
         // Copy the primitive into the vector. This may or may not be
         // very good, but let's try it.
         m_primitives[ level - m_minLevelOfPrimitives ].push_back(*curPrim);
//              [nbrAddedFeaturesPerLevel[level - m_minLevelOfPrimitives]++] =
//              *curPrim;
         delete curPrim;

      }
      primitives.clear();
   }

   // Go through the primivitives and update the m_parentFeature for
   // the complex features so that it points down to one of the
   // primitives.
   uint32 primNbr = 0;
   for ( int l = 0; l < m_nbrPrimitiveLevels; ++l ) {
      for ( uint32 i = 0; i < m_primitives[ l ].size(); ++i ) {
         TilePrimitiveFeature& curPrim = m_primitives[ l ][ i ];
         m_features[ curPrim.getFeatureNbr() ].setFeatureNbr( primNbr++ );
      }
   }

#ifdef MC2_SYSTEM
   for ( int k = 0; k < m_nbrPrimitiveLevels; ++k ) {
      int vectorSize = m_primitives[k].size();
      int counted = nbrFeaturesPerLevel[k];
      MC2_ASSERT( vectorSize == counted );
   }
#endif
   return true;
}


#ifdef MC2_SYSTEM
void
TileMap::dump( ostream& stream ) const {
  for ( vector<TileFeature>::const_iterator it =
            m_features.begin(); it != m_features.end(); ++it ) {
      (it)->dump( stream );
   }
   stream << "Nbr strings " << m_strings.size() << endl;
   uint32 i = 0;
   for ( vector<MC2SimpleString>::const_iterator kt = m_strings.begin();
         kt != m_strings.end(); ++kt ) {
      stream << "  String " << i << " " << *kt << endl;
      ++i;
   }
   stream << "String index by feature index." << endl;
   for ( uint32 i = 0; i < m_strIdxByFeatureIdx.size(); ++i ) {
      stream << " feat index " << i << ", str index "
             << m_strIdxByFeatureIdx[ i ] << endl;
   }
   stream << "Feature index in text order." << endl;
   for ( uint32 i = 0; i < m_featureIdxInTextOrder.size(); ++i ) {
      stream << m_featureIdxInTextOrder[ i ] << endl;
   }
   stream << "Categories by features. " << endl;
   for ( uint32 i = 0; i < m_poiCategoryMap.size(); ++i ) {
      stream << "feat index " << m_poiCategoryMap[ i ].first
             << " category id " << m_poiCategoryMap[ i ].second
             << endl;
   }
   stream << "TileMap:" << endl;
   stream << "Nbr features " << m_features.size() << endl;

}
#endif

void
TileMap::snapCoordToPixel( TileMapCoord& coord ) const
{
   coord.setCoord(
         m_mc2Scale * (coord.getLat() / m_mc2Scale),
         m_mc2Scale * (coord.getLon() / m_mc2Scale) );
}

const TileFeature*
TileMap::getParentFeature(const TilePrimitiveFeature* prim) const
{
   int parentIdx = prim->getFeatureNbr();
   if ( parentIdx < 0 || parentIdx > (int)m_features.size() ) {
      return NULL;
   } else {
      return &m_features[parentIdx];
   }
}

const TileMapParams*
TileMap::getStringMapParams(LangTypes::language_t lang) const
{
   if ( m_params->getTileMapType() == TileMapTypes::tileMapStrings ) {
      return NULL;
   } else {
      if ( m_otherTypeOfMapParams != NULL ) {
         return m_otherTypeOfMapParams;
      }
      // Pretend that this function is const.
      TileMap* non_const_this = const_cast<TileMap*>(this);
      non_const_this->m_otherTypeOfMapParams = new TileMapParams(*m_params);
      non_const_this->m_otherTypeOfMapParams->
         setTileMapType(TileMapTypes::tileMapStrings);
      non_const_this->m_otherTypeOfMapParams->
         setLanguageType(lang);
      return m_otherTypeOfMapParams;
   }
}

const TileMapParams*
TileMap::getDataMapParams() const
{
   if ( m_params->getTileMapType() == TileMapTypes::tileMapData ) {
      return NULL;
   } else {
      if ( m_otherTypeOfMapParams != NULL ) {
         return m_otherTypeOfMapParams;
      }
      // Pretend that this function is const.
      TileMap* non_const_this = const_cast<TileMap*>(this);
      delete non_const_this->m_otherTypeOfMapParams;
      non_const_this->m_otherTypeOfMapParams = new TileMapParams(*m_params);
      non_const_this->m_otherTypeOfMapParams->
         setTileMapType(TileMapTypes::tileMapData);
      non_const_this->m_otherTypeOfMapParams->
         setLanguageType(LangTypes::swedish);
      return m_otherTypeOfMapParams;
   }
}

const TileMapParams*
TileMap::getParam() const {
   return m_params;
}

const MC2SimpleString*
TileMap::getStringForFeature(int featureNbr) const
{
   if ( m_strings.empty() || featureNbr >= (int)m_strIdxByFeatureIdx.size()) {
      return NULL;
   } else {
      int strIdx = m_strIdxByFeatureIdx[featureNbr];
      if ( strIdx < 0 ) {
         return NULL;
      } else {
         return &m_strings[strIdx];
      }
   }
}

const TileFeature*
TileMap::getFeature(int featureNbr) const
{
   if ( m_features.empty() ||
         featureNbr >= (int)m_features.size() ) {
      return NULL;
   } else {
      return &m_features[ featureNbr ];
   }
}


