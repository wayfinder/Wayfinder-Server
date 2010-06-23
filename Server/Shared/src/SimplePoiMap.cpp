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

#include "SimplePoiMap.h"

#include "DrawingProjection.h"
#include "GfxFeatureMap.h"
#include "GfxFeature.h"
#include "GfxPolygon.h"
#include "MathUtility.h"
#include "ServerTileMapFormatDesc.h"
#include "POIImageIdentificationTable.h"
#include "SimplePoiDesc.h"

#if 0
#define spdebug mc2dbg
#else
#define spdebug mc2dbg8
#endif

namespace {
   /// One poi in the resulting map.
   class SimplePoiNotice {
   public:
      SimplePoiNotice( const MC2Point& calcedPoint,
                       uint32 tileType,
                       const GfxFeature* feature )
            : m_point( calcedPoint ),
              m_tileType( tileType ),
              m_feature( feature ) {
      }

      const MC2Point& getPoint() const {
         return m_point;
      }

      uint32 getType() const {
         return m_tileType;
      }

      const char* getName() const {
         return m_feature->getName();
      }
      
   private:
      MC2Point          m_point;
      uint32            m_tileType;
      const GfxFeature* m_feature;
   };
}

SimplePoiMap::SimplePoiMap( const GfxFeatureMap& featMap,
                            const ServerTileMapFormatDesc& desc,
                            const DrawingProjection& proj,
                            const POIImageIdentificationTable& imageTable  )
      : m_gfxMap( featMap ),
        m_desc( desc ),
        m_proj( proj ),
        m_imageTable( imageTable )
{

}

SimplePoiMap::~SimplePoiMap()
{

}

static inline void write8or16( BitBuffer& dest, uint32 nbr, bool sixteen )
{
   if ( sixteen ) {
      dest.writeNextBAShort( nbr );
   } else {
      dest.writeNextByte( nbr );
   }
}


static inline void writePoi( BitBuffer& dest,
                             int coordsAre16Bits,
                             int type16Bits,
                             const SimplePoiNotice& notice )
{
   // Poi type
   write8or16( dest, notice.getType(), type16Bits );
   // Coords
   write8or16( dest, notice.getPoint().getX(), coordsAre16Bits );
   write8or16( dest, notice.getPoint().getY(), coordsAre16Bits );
   // Name
   dest.writeNextString( notice.getName() );
   // Extra info
   dest.writeNextString( "" ); 
}

static inline void saveSimplePoiMap( BitBuffer& dest,
                                     const vector<SimplePoiNotice>& pois,
                                     int nbrBitsPerType,
                                     int nbrBitsPerCoord )
{
   const uint32 maxPoisPerBlock = 255;
   const bool type16Bits = nbrBitsPerType > 8;
   const bool coordsAre16Bits = nbrBitsPerCoord > 8;

   uint32 nbrPoisLeft = pois.size();
   
   // Write placeholder for the size
   BitBuffer lenBuf( dest.getCurrentOffsetAddress(), 4 );
   dest.writeNextBALong( 0 );
   
   // Types are 16 bits
   dest.writeNextBits( type16Bits, 1 );
   // Coordinates are 16 bit
   dest.writeNextBits( coordsAre16Bits, 1 );
   // Align to byte and zero
   dest.writeNextBits( 0, 6 );
   dest.alignToByte();
   
   // Write poi blocks
   vector<SimplePoiNotice>::const_iterator it = pois.begin();
   while ( nbrPoisLeft ) {
      uint32 nbrToWrite = MIN( maxPoisPerBlock, nbrPoisLeft );
      nbrPoisLeft -= nbrToWrite;
      // Number of pos in this block
      dest.writeNextByte( nbrToWrite );
      for ( uint32 i = 0; i < nbrToWrite; ++i ) {
         writePoi( dest, coordsAre16Bits, type16Bits, *it++);
      }
   }
   // Finish with a zero nbr pois
   dest.writeNextByte( 0 );

   // Update the length
   lenBuf.writeNextBALong( dest.getCurrentOffset() );
   dest.setSizeToOffset();
}

SharedBuffer*
SimplePoiMap::getAsNewBytes()
{
   getAsBytes();
   return m_bytes.release();
}

const SharedBuffer*
SimplePoiMap::getAsBytes() const
{
   // 1. Get the poi types from ServerTileMapFormatDesc
   typedef ServerTileMapFormatDesc::imageByType_t imageByType_t;
   const imageByType_t& imagesByType = m_desc.getUsedPoiNamesByType();
  
   // 2. Go through the map and check for max coord. Save the
   //    features that can be kept.
   vector<SimplePoiNotice> keptNotices;
   keptNotices.reserve( m_gfxMap.getNbrFeatures() );
   PixelBox pixelBox;
   bool haveSpecialPOI = false;
   const PixelBox& projPixBox = m_proj.getPixelBox();
   for ( GfxFeatureMap::const_iterator it = m_gfxMap.begin();
         it != m_gfxMap.end();
         ++it ) {
      const GfxFeature* fiat = *it;
      uint32 tileType = m_desc.getTileFeatureTypeFromGfxFeature( fiat );
      //fiat->dump();

      bool haveImage = false;
      // do special stuff for special custom pois
      if ( fiat->getType() == GfxFeature::POI &&
           ServerTileMapFormatDesc::isSpecialCustomPOI( tileType ) ) {
         const GfxPOIFeature* poi = static_cast<const GfxPOIFeature*>( fiat );
         // change tiletype number
         tileType = getSpecialFeatureType( poi->getPOIType(),
                                           poi->getCustomImageName().c_str() );
         // failed to find tile type, lets try default poi type
         if ( tileType == MAX_UINT16 ) {
            tileType = m_desc.getTileFeatureForPOI( poi->getPOIType() );
         } else {
            haveSpecialPOI = true; // force 16 bit types
            // we must have an image if decoding of custom image name
            // was successful
            haveImage = true;
         }
      }

      if ( ! haveImage ) {
         haveImage = imagesByType.find( tileType ) != imagesByType.end();
      }

      if ( haveImage ) {

         MC2Coordinate coord = *(fiat->getPolygon(0)->begin());
         MC2Point point = m_proj.getPoint( coord );

         spdebug << "[SPM]: Coord is " << coord << endl;
         spdebug << "[SPM]: Point is " << point << endl;

         if ( projPixBox.pointInside( point ) ) {
            // Save feature here
            spdebug << "[SPM]: Saved feature "
                   << MC2CITE( fiat->getName() ) << endl;
            keptNotices.push_back( SimplePoiNotice( point, tileType, fiat ) );
            pixelBox.update( point );
         }
      } else {
         spdebug << "[SPM]: Nicht Gut: " 
                 << tileType << endl;
      }
   }
   spdebug << "[SPM]: Box is " << pixelBox << endl;
   spdebug << "[SPM]: Proj pixbox is " << projPixBox << endl;
   
   // Nice. Only positive values.
   // 3. Get the number of bits needed for each coordinate.
   int nbrCoordBits = MAX( MathUtility::getNbrBits( pixelBox.getHeight() ),
                           MathUtility::getNbrBits( pixelBox.getLonDiff() ) );
   spdebug << "[SPM]: Nbr coord bits needed " << nbrCoordBits << endl;

   // 4. Write the map.

   m_bytes.reset( new BitBuffer(10*1024*1024) );
   
   saveSimplePoiMap( static_cast< BitBuffer& >( *m_bytes ), keptNotices,
                     haveSpecialPOI ? 16 :
                     MathUtility::getNbrBits( imagesByType.size() ), 
                     // force 16 bit for feature type 
                     // since we might have 
                     nbrCoordBits );

   spdebug << db_dump( *m_bytes ) << endl;
   
   return m_bytes.get();
}

/**
 * Determine the new feature type for the special custom poi type
 * with custom image.
 * @param featureType special custom poi type
 * @return new feature type id for simple poi desc table.
 */
uint16 SimplePoiMap::
getSpecialFeatureType( uint32 poiType,
                       const MC2String& customImage ) const {
   
   uint32 codeIndex = m_imageTable.getCodeIndex( poiType, customImage );
   if ( codeIndex == MAX_UINT32 ) {
      return MAX_UINT16;
   }
   return SimplePoiDesc::SPECIAL_FEATURE_OFFSET + codeIndex;
}
