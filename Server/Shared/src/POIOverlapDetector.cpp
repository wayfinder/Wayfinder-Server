/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "POIOverlapDetector.h"
#include "GfxFeatureMap.h"
#include "GfxFeature.h"
#include "GfxPolygon.h" 
#include "GfxConstants.h"
#include "TileMapParams.h"
#include "ServerTileMap.h"
#include "ServerTileMapFormatDesc.h"
#include "OverlapDetector.h"
#include "MC2BoundingBox.h"

void POIOverlapDetector::removeOverlappedPOIs( GfxFeatureMap& gfxMap, 
                                               const TileMapParams& params,
                                               WritableTileMap& refTileMap,
                                               const ServerTileMapFormatDesc& desc,
                                               POICache& poiCache,
                                               avoidSet_t& avoidSet,
                                               int pixelMargin )
{
   // Get the most zoomed in scale for the tilemap
   pair<uint16, uint16> scaleRange = desc.getScaleRange( params );
   double lowestScale = scaleRange.first + 1;
   mc2dbg8 << "Lowest scale range: " << lowestScale << endl;
   mc2dbg8 << "Detail level: " << params.getDetailLevel() << endl;

   MC2BoundingBox tileBox;
   desc.getBBoxFromTileIndex( params.getLayer(), 
                              tileBox,
                              params.getDetailLevel(),
                              params.getTileIndexLat(),
                              params.getTileIndexLon() );

   // cosLat for the tileBox
   float64 cosLat = tileBox.getCosLat();

   std::multimap<int16, const GfxFeature*> featureList;
   for ( uint32 i = 0; i < gfxMap.getNbrFeatures(); ++i ) {
      const GfxFeature* feature = (gfxMap.getFeature( i ) );
      // Get the TileFeatureType
      int16 tft = desc.getTileFeatureTypeFromGfxFeature( feature );
      // Get the name of the feature
      MC2String name = 
         desc.getPOIImageName( tft );
      if ( name != "" ) {
         // Feature contains an image, store it in the map
         featureList.insert( make_pair( tft, feature ) );
         mc2dbg8 << "Adding feature with name: " << name << endl;
      }
   }
   OverlapDetector<MC2BoundingBox> overlapDetector;
   typedef multimap<int16, const GfxFeature*>::iterator it_t;
   it_t lower_it = featureList.lower_bound( MIN_INT16 );
   MC2String name;
   while ( lower_it != featureList.end() ) {
      // Clear the vector in the overlapDetector, we should only
      // compare features within the same category
      overlapDetector.clear();

      // For each TileFeatureType (Key in the multimap) in the map
      it_t upper_it = featureList.upper_bound( lower_it->first );
      for ( it_t it = lower_it; it != upper_it; ++it ) {
         name = desc.getPOIImageName(
                  desc.getTileFeatureTypeFromGfxFeature( it->second ) );    

         // Name should not be empty, if name is empty something is very wrong 
         // since all features not having an image are ignored in previous for
         // loop.
         MC2_ASSERT( name != "" );

         // GfxFeature should contain exactly one GfxPolygon
         MC2_ASSERT( it->second->getNbrPolygons() == 1 );
         GfxPolygon* polygon = it->second->getPolygon( 0 );
         
         // polygon should not be null and should contain exactly 
         // one coordinate 
         MC2_ASSERT( polygon != NULL && polygon->getNbrCoordinates() == 1 );
         TileMapCoord coord( polygon->getLat( 0 ), 
                             polygon->getLon( 0 ) );

         mc2dbg8 << "Bitmapcoord BEFORE coslat calc: " << coord << endl;
         // Calc the new longtitude for the center coordinate, corrected
         // by the cosLat factor for the whole tielBox
         coord.lon = int32( tileBox.getMinLon() + 
            ( coord.lon - tileBox.getMinLon() ) * cosLat );

         mc2dbg8 << "Bitmapcoord AFTER coslat calc: " << coord << endl;
         
         // Snap the coordinate to the correct pixel
         refTileMap.snapCoordToPixel( coord );

         POIData poiData;
         if ( !poiCache.find( name, poiData ) ) {
            // Feature does not exist in the cache
            mc2dbg8 << "Feature with name: " << name  
                   << " does not exist in the cache!" << endl;
            // Let the cache calculate the actual size of the visible rect
            // and the offset point.
            if ( !poiCache.createAndInsert( name, poiData ) ) {
               mc2dbg8 << "Could not create poi data from image, continue!" << endl;
               continue;  
            }
         }
         
         // Calc the poi size from meter to mc2
         int32 heightInMC2 = int32( ( poiData.m_height + pixelMargin ) * lowestScale * 
                                    GfxConstants::METER_TO_MC2SCALE );
         int32 widthInMC2  = int32( ( poiData.m_width + pixelMargin ) * lowestScale * 
                                    GfxConstants::METER_TO_MC2SCALE );
         
         // Create the new center coordinate
         MC2Coordinate center = coord;
         center.lon -= int32( poiData.m_offset.getX() * 
            lowestScale * GfxConstants::METER_TO_MC2SCALE );
         center.lat -= int32( poiData.m_offset.getY() * 
            lowestScale * GfxConstants::METER_TO_MC2SCALE );

         // Calc the bounding box and send it to the overlapDetector
         // for checking if it is ok or not
         MC2BoundingBox bbox( MC2Coordinate( center.lat - heightInMC2 / 2 ,
                                             center.lon - widthInMC2 / 2 ),
                              MC2Coordinate( center.lat + heightInMC2 / 2, 
                                             center.lon + widthInMC2 / 2 ) );

         mc2dbg8 << "MC2BoundingBox for the feature: " << bbox << endl;
         mc2dbg8 << "Center for the bbox = " << bbox.getCenter() << endl;
         mc2dbg8 << "Original coord for the feature = " << coord << endl;
         bool toBeIncluded = overlapDetector.addIfNotOverlapping( bbox );
         if ( !toBeIncluded ) {
            mc2dbg8 << "The feature should NOT be in the map!" << endl;
            avoidSet.insert( it->second );
         } else {
            mc2dbg8 << "The feature should be in the map!" << endl;            
         }
      }
      lower_it = upper_it;
   }
}

