/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GfxMapFilter.h"

#include "GfxFeatureMap.h"
#include "GfxPolygon.h"
#include "GfxRoadPolygon.h"
#include "MapUtility.h"
#include "TileImportanceTable.h"
#include "ServerTileMapFormatDesc.h"
#include "TileMapParams.h"
#include "TileMapProperties.h"
#include "GfxPolygon.h"
#include "GfxDataFull.h"
#include "Stack.h"
#include "AbbreviationTable.h"
#include "MC2Coordinate.h"
#include "NodeBits.h"
#include "PropertyHelper.h"
#include <math.h>

/// Uniq set of gfx types.
typedef std::set< GfxFeature::gfxFeatureType > GfxFeatureTypeSet;

/**
 *    Will create a new GfxMap (which must be deleted by the caller)
 *    containing merged street features.
 *    Streets will be merged to be in the same polygon 
 *    if they share end coordinates and they have the same name.
 *    A new polygon is created for a crossing with other streets 
 *    with type present in visibleStreetTypes.
 *    
 *    Only streets of type streetTypeToMerge will be merged and
 *    present in the returned map.
 *
 *    @param   gfxMap               The map.
 *    @param   visibleStreetTypes   All visible types of streets.
 *    @param   alwaysIncludeTypes   Types that always should be 
 *                                  included in the resulting map.
 *    @param   streetTypeToMerge    The street type to merge.
 *    @param   maxCrossDist         Maximum cross distance for
 *                                  the filtering algorithm.
 *    @param   importance           The importance.
 *    @param   mapDesc              The tilemapformatdesc.
 *    @param   param                The TileMapParam for the
 *                                  importance.
 *    @return  A new gfxmap containing the merged streets.
 *             Must be deleted by the caller of this method!
 */
GfxFeatureMap* 
mergeAndFilterStreets( GfxFeatureMap& gfxMap,
                       const GfxFeatureTypeSet& visibleStreetTypes,
                       const GfxFeatureTypeSet& alwaysIncludeTypes,
                       const GfxFeatureTypeSet& streetTypesToMerge,
                       const TileImportanceNotice* importance,
                       const ServerTileMapFormatDesc& mapDesc,
                       uint32 maxCrossDist,
                       const TileMapParams& param );

/**
 *    Adds background features, i.e. the underlying ocean and
 *    land features. Tries to avoid overlapping features in order
 *    to speed up the drawing.
 *
 *    @param   gfxMap      The input gfxmap, containing land, buas
 *                         etc.
 *    @param   worldBBox   The world boundingbox, i.e. the bbox
 *                         for the tile.
 *    @param   resultMap   Map that the background (ocean and land)
 *                         features will be added to.
 */
void
addBackgroundFeatures( const GfxFeatureMap& gfxMap, 
                       const MC2BoundingBox& worldBBox, 
                       GfxFeatureMap& resultMap );
      
/// One set of merged node id:s.
typedef list< pair< bool, uint32 > > mergedNodes_t;

/**
 *    Help method, used to create which street types that will
 *    be present in the final tilemap.
 *    @param   visibleStreetTypes   [OUT] Will be filled to contain
 *                                  the visible street types.
 *    @param   mapDesc              The map description.
 *    @param   detailLevel          The detail level of the tilemap.
 *    @param   streetType           The street type to check if to be
 *                                  included among the visible street
 *                                  types.
 */
void
addVisibleStreetTypes( GfxFeatureTypeSet& visibleStreetTypes, 
                       const ServerTileMapFormatDesc& mapDesc,
                       int detailLevel,
                       int streetType ) {
   const TileImportanceNotice* streetImportance = 
               mapDesc.getImportanceOfType( TileMapTypes::c_mapLayer,
                                            streetType );
   if ( ( streetImportance != NULL ) && 
        ( streetImportance->getDetailLevel() >= detailLevel ) ) {
      
      switch ( streetType ) {
         case ( ServerTileMapFormatDesc::
                  STREET_MAIN_AND_MEDIUM_CITYCENTRES_AND_AIRPORTS ) :
            visibleStreetTypes.insert( GfxFeature::STREET_MAIN );
            visibleStreetTypes.insert( GfxFeature::STREET_FIRST );
            break;
         case ( ServerTileMapFormatDesc::
                  STREET_FIRST_AND_MINOR_CITYCENTRES ) :
            break;
         case ( ServerTileMapFormatDesc::STREET_SECOND_AND_FERRY ) :
            visibleStreetTypes.insert( GfxFeature::STREET_SECOND );
            break;
         case ( ServerTileMapFormatDesc::STREET_THIRD_AND_MORE ) :
            visibleStreetTypes.insert( GfxFeature::STREET_THIRD );
            break;
         case ( ServerTileMapFormatDesc::STREET_FOURTH_AND_RAILWAY ) :
            visibleStreetTypes.insert( GfxFeature::STREET_FOURTH );
            break;
         default:
            visibleStreetTypes.insert( 
                  GfxFeature::gfxFeatureType( streetType ) );
            break;
      }
   }
}

namespace {
void
removeNames( GfxFeatureMap* gfxMap, 
             GfxFeature::gfxFeatureType type )
{
   for ( uint32 i = 0; i < gfxMap->getNbrFeatures(); ++i ) {
      GfxFeature* feature = 
         const_cast<GfxFeature*> ( gfxMap->getFeature( i ) );
      if ( feature->getType() == type ) {
         feature->setName( "" );
         feature->setBasename( "" );
      }
   }
}

}

/**
 *    Will filter out everything but the features according to the
 *    importance notice.
 *
 *    In case the method's return value is not NULL, then that
 *    GfxMap will contain the filtered features. This GfxMap must
 *    be deleted by the caller.
 *    Otherwise the GfxMap submitted as parameter will have it's 
 *    features filtered.
 *    
 *    @param   gfxMap         The input gfxmap.
 *                            It will be modified, so it should not be used
 *                            afterwards.
 *    @param   importance     The importance notice.
 *    @param   mapDesc        The map format description.
 *    @param   param          The TileMapParam for the importance.
 *    @param   meterToPixelFactor Meter to pixel factor.
 *    @return  A new GfxMap that contains the filtered 
 *             features.
 */
GfxFeatureMap*
filterImportance( GfxFeatureMap* gfxMap, 
                  const TileImportanceNotice* importance,
                  const ServerTileMapFormatDesc& mapDesc,
                  const TileMapParams& param,
                  float64 meterToPixelFactor ) {
   int detailLevel = param.getDetailLevel();
//   bool highEnd = mapDesc.highEndDevice();
   
   // fixme: Put this in TileMapProperties.
   if ( detailLevel == 3 ) {
      // Remove the names for all roadclass one streets.
      // This is to allow merging of streets in for instance Paris.
      removeNames( gfxMap, GfxFeature::STREET_FIRST );
   } else if ( detailLevel == 4 || detailLevel == 5 ) {
      for ( uint32 i = 0, n = gfxMap->getNbrFeatures(); i < n; ++i ) {
         GfxFeature* feature =
            const_cast<GfxFeature*>( gfxMap->getFeature( i ) );
         if ( feature->getType() == GfxFeature::STREET_FIRST ||
              feature->getType() == GfxFeature::STREET_MAIN ) {
            feature->setName( "" );
            feature->setBasename( "" );
         }
      }
   } else if ( detailLevel == 6 || detailLevel == 7 ) {
      removeNames( gfxMap, GfxFeature::STREET_MAIN );
   } /*else if ( detailLevel == 1 && highEnd ) {
      // names are too small on this detail level, for 
      // street four.
      removeNames( gfxMap, GfxFeature::STREET_FOURTH );
      }*/

   
   if ( importance->getType() == 
         ServerTileMapFormatDesc::
            STREET_MAIN_AND_MEDIUM_CITYCENTRES_AND_AIRPORTS ||
        importance->getType() == 
            ServerTileMapFormatDesc::STREET_FIRST_AND_MINOR_CITYCENTRES ||
        importance->getType() == 
            ServerTileMapFormatDesc::STREET_SECOND_AND_FERRY || 
        importance->getType() == 
            ServerTileMapFormatDesc::STREET_THIRD_AND_MORE ||
        importance->getType() == 
            ServerTileMapFormatDesc::STREET_FOURTH_AND_RAILWAY || 
        importance->getType() == 
        ServerTileMapFormatDesc::ALL_ROUTE_FEATURES ){
      set<GfxFeature::gfxFeatureType> visibleStreetTypes;
      
      addVisibleStreetTypes( visibleStreetTypes, mapDesc, 
                             detailLevel, 
                       ServerTileMapFormatDesc::
                        STREET_MAIN_AND_MEDIUM_CITYCENTRES_AND_AIRPORTS );
      addVisibleStreetTypes( visibleStreetTypes, mapDesc, 
                             detailLevel, 
                       ServerTileMapFormatDesc::
                        STREET_FIRST_AND_MINOR_CITYCENTRES );
      addVisibleStreetTypes( visibleStreetTypes, mapDesc, 
                             detailLevel, 
                       ServerTileMapFormatDesc::STREET_SECOND_AND_FERRY );
      addVisibleStreetTypes( visibleStreetTypes, mapDesc, 
                             detailLevel, 
                       ServerTileMapFormatDesc::STREET_THIRD_AND_MORE );
      addVisibleStreetTypes( visibleStreetTypes, mapDesc, 
                             detailLevel, 
                       ServerTileMapFormatDesc::STREET_FOURTH_AND_RAILWAY );

      mc2dbg8 << "Street type = " << importance->getType() << endl;
      for ( set<GfxFeature::gfxFeatureType>::const_iterator it =
               visibleStreetTypes.begin(); it != visibleStreetTypes.end();
               ++it ) {
         mc2dbg8 << "Visible street type = " << *it << endl;
      }
      mc2dbg8 << " meterToPixelFactor = " << meterToPixelFactor << endl;
      uint32 maxCrossDist = 0;
      if ( detailLevel > 0 ) {
         maxCrossDist = int( TileMapProperties::streetFilterFactor / 
                             meterToPixelFactor );
      }
      if ( detailLevel > 3 ){
         maxCrossDist = static_cast<uint32>(maxCrossDist * 2); //(1.0/3);//9; //Hack more filtering, put in TileMapProperties.
      }
      mc2dbg8 << "Max cross dist det[" << detailLevel << "]:"
             << maxCrossDist << endl;

      set<GfxFeature::gfxFeatureType> alwaysIncludeTypes;

      set<GfxFeature::gfxFeatureType> mergeTypes;
      switch ( importance->getType() ) {
         case ( ServerTileMapFormatDesc::
                STREET_MAIN_AND_MEDIUM_CITYCENTRES_AND_AIRPORTS ) :
            mergeTypes.insert(GfxFeature::STREET_MAIN); 
            mergeTypes.insert(GfxFeature::STREET_FIRST);
            if ( false /*highEnd*/ ) {
               mergeTypes.insert(GfxFeature::STREET_SECOND);
               visibleStreetTypes.insert( GfxFeature::STREET_SECOND );
            }
            visibleStreetTypes.insert( GfxFeature::STREET_MAIN );
            visibleStreetTypes.insert( GfxFeature::STREET_FIRST );


            alwaysIncludeTypes.insert( GfxFeature::POI );
            alwaysIncludeTypes.insert( GfxFeature::EVENT );
            break;
         case ( ServerTileMapFormatDesc::
                  STREET_FIRST_AND_MINOR_CITYCENTRES ) :
            alwaysIncludeTypes.insert( GfxFeature::POI );
            alwaysIncludeTypes.insert( GfxFeature::EVENT );
            break;
         case ( ServerTileMapFormatDesc::STREET_SECOND_AND_FERRY ) :
            mergeTypes.insert(GfxFeature::STREET_SECOND); //Skip ferry for now.
            if ( false /*highEnd*/ ) {
               mergeTypes.insert(GfxFeature::STREET_THIRD);
            }

            break;
         
         case ( ServerTileMapFormatDesc::STREET_THIRD_AND_MORE ) :
            mergeTypes.insert(GfxFeature::STREET_THIRD);
            if ( false /*highEnd*/ ) {
               mergeTypes.insert(GfxFeature::STREET_FOURTH);
            }

            break;
            
         case ( ServerTileMapFormatDesc::STREET_FOURTH_AND_RAILWAY ) :
            mergeTypes.insert(GfxFeature::STREET_FOURTH);
            if ( false /*highEnd*/ ) {
               alwaysIncludeTypes.insert(GfxFeature::STREET_FOURTH);
            }
            alwaysIncludeTypes.insert( GfxFeature::RAILWAY );

            break;
         
         case ( ServerTileMapFormatDesc::ALL_ROUTE_FEATURES ) :
            mergeTypes.insert(GfxFeature::ROUTE);
            visibleStreetTypes.insert( GfxFeature::ROUTE );
            visibleStreetTypes.insert( GfxFeature::STREET_MAIN );
            visibleStreetTypes.insert( GfxFeature::STREET_FIRST );
            visibleStreetTypes.insert( GfxFeature::STREET_SECOND );
            visibleStreetTypes.insert( GfxFeature::STREET_THIRD );
            visibleStreetTypes.insert( GfxFeature::STREET_FOURTH );
            
            alwaysIncludeTypes.insert( GfxFeature::STREET_MAIN );
            alwaysIncludeTypes.insert( GfxFeature::STREET_FIRST );
            alwaysIncludeTypes.insert( GfxFeature::STREET_SECOND );
            alwaysIncludeTypes.insert( GfxFeature::STREET_THIRD );
            alwaysIncludeTypes.insert( GfxFeature::STREET_FOURTH );
            
            alwaysIncludeTypes.insert( GfxFeature::ROUTE_ORIGIN );
            alwaysIncludeTypes.insert( GfxFeature::ROUTE_DESTINATION );
            alwaysIncludeTypes.insert( GfxFeature::PARK_CAR );
            break;
         default:
            mergeTypes.insert(GfxFeature::gfxFeatureType(importance->getType()));
            break;
      }
      



      GfxFeatureMap* newMap = mergeAndFilterStreets(  
                    *gfxMap, 
                    visibleStreetTypes,
                    alwaysIncludeTypes,
                    mergeTypes,
                    importance,
                    mapDesc,
                    maxCrossDist, 
                    param );

      return newMap;
   }
  
   GfxFeatureMap* newMap = new GfxFeatureMap;
   MC2BoundingBox bbox;
   gfxMap->getMC2BoundingBox( &bbox );
   newMap->setMC2BoundingBox( &bbox );
  
   if ( importance->getType() == 
        ServerTileMapFormatDesc::LAND_AND_MAJOR_CITYCENTRES ) { 
      addBackgroundFeatures( *gfxMap, bbox, *newMap );
   }
   
   float64 sqMeterToSqPixelFactor = 
      meterToPixelFactor * meterToPixelFactor;

   while ( gfxMap->getNbrFeatures() > 0 ) {
      GfxFeature* feature = gfxMap->getAndRemoveLastFeature();
          
      if ( mapDesc.isGfxOfImportance( feature,
                                      importance,
                                      sqMeterToSqPixelFactor,
                                      param ) ) {
         // Add the feature.
         newMap->addFeature( feature );

      } else {
         // Delete it.
         delete feature;
      }

   }
   // Reverse the features in newMap since we added them in
   // reverse order.
   newMap->reverseFeatures();

   return newMap;
}

/**
 *    Enlarge the bbox so that it covers all areas not covered
 *    by the feature. Maximum size of the bbox can be
 *    the worldBBox.
 *    @param   worldBBox   The world bbox. The bbox can max
 *                         be this large.
 *    @param   bbox        The bbox to enlarge.
 *    @param   feature     The feature.
 */
void enlargeBBox( const MC2BoundingBox& worldBBox, 
                  MC2BoundingBox& bbox, 
                  const GfxPolygon& polygon ) {
   if ( bbox == worldBBox ) {
      // The bbox can't get any larger than to cover the whole
      // world.
      return;
   }
   
   set<int> corners;
   // Go through all coords in the polygon.
   const GfxPolygon* poly = &polygon;
   MC2Coordinate coord;
   for ( uint32 i = 0; i < poly->getNbrCoordinates(); ++i ) {
      coord.lat = poly->getLat( i );
      coord.lon = poly->getLon( i );
      
      // Check if on corner.
      int corner = worldBBox.getCorner( coord );
      if ( corner != MC2BoundingBox::no_corner ) {
         // Yep, it's a corner. Store it.
         corners.insert( corner );
      } else {
         // Enlarge the bbox with all coords that are not corners.
         bbox.update( coord );
      }
   }

   // Update the bbox with all corners not found in the feature.
   for ( int c = 0; c < 4; ++c ) { 
      if ( corners.find( c ) == corners.end() ) {
         // Corner not present in feature.
         // Update bbox.
         bbox.update( 
               worldBBox.getCorner( MC2BoundingBox::corner_t( c )  ) );
      }
   }
}

void
enlargeBBox( const MC2BoundingBox& worldBBox, 
             MC2BoundingBox& bbox, 
             const GfxFeature& feature )
{
   for ( uint32 p = 0; p < feature.getNbrPolygons(); ++p ) {
      // Go through all coords in the polygon.
      const GfxPolygon* poly = feature.getPolygon( p );
      enlargeBBox( worldBBox, bbox, *poly );
   }
}

/**
 *    Reduces the boundingbox (from world bbox) to cover the 
 *    smallest area possible, that still covers any area 
 *    not already covered by the supplied features.
 *    I.e. find the smallest boundingbox that can be used
 *    as background for the features.
 *    @param   worldBBox   The initial boundingbox.
 *    @param   features    Set of features.
 *    @return  The reduced boundingbox.
 */
MC2BoundingBox
reduceBBox( const MC2BoundingBox& worldBBox,
            const vector<const GfxFeature*>& features ) {
   if ( features.empty() ) {
      // Was not possible to reduce the size of the bbox since no
      // features are present.
      return worldBBox;
   }

   MC2BoundingBox coveredBBox;
   // Enlarge the empty bbox so that it covers the features.
   for ( vector<const GfxFeature*>::const_iterator it = features.begin();
         it != features.end(); ++it ) {
      enlargeBBox( worldBBox, coveredBBox, **it );
   }
   return coveredBBox;
}  

/**
 *    Checks if the geometry of the closed feature totally
 *    covers the boundingbox. The method is not foolproof, but
 *    should work for most cases, or at least for some.
 *    @param   feature  The feature to check.
 *    @param   bbox     The boundingbox.
 *    @return  If the feature totally covered the bbox.
 */
bool
totallyCoversBBox( const GfxPolygon& polygon,
                   const MC2BoundingBox& bbox ) 
{
   
   const GfxPolygon* poly = &polygon;
   
   // It's assumed to cover the bbox if all bbox corners are present
   // in the polygon and that all other coordinates are lying on the
   // bbox border.
   
   set<int> corners;
   
   MC2Coordinate coord;
   for ( uint32 i = 0; i < poly->getNbrCoordinates(); ++i ) {
      coord.lat = poly->getLat( i );
      coord.lon = poly->getLon( i );

      if ( bbox.onBorder( coord ) ) {
         int corner = bbox.getCorner( coord );
         if ( corner != MC2BoundingBox::no_corner ) {
            corners.insert( corner );
         }
      } else {
         // Not on the border.
         return false;
      }
   }

   // All coordinates were on the border.
   // Check that we got 4 individual corners also.
   return corners.size() == 4;
}

bool
totallyCoversBBox( const GfxFeature& feature,
                   const MC2BoundingBox& bbox ) {

   // One polygon must totally cover the bbox to return true.
   for ( uint32 i = 0; i < feature.getNbrPolygons(); ++i ) {
      if ( totallyCoversBBox( *feature.getPolygon( i ), bbox ) ) {
         return true;
      }
   }

   return false;
}

/**
 *    Adds background features, i.e. the underlying ocean and
 *    land features. Tries to avoid overlapping features in order
 *    to speed up the drawing.
 *
 *    @param   gfxMap      The input gfxmap, containing land, buas
 *                         etc.
 *    @param   worldBBox   The world boundingbox, i.e. the bbox
 *                         for the tile.
 *    @param   resultMap   Map that the background (ocean and land)
 *                         features will be added to.
 */
void
addBackgroundFeatures( const GfxFeatureMap& gfxMap, 
                       const MC2BoundingBox& worldBBox, 
                       GfxFeatureMap& resultMap ) {
   mc2dbg8 << "[GFM]: addBackgroundFeatures" << endl;

   // Collect the unique buas.
   typedef vector<const GfxFeature*> vec_t;
   vec_t buas;
   gfxMap.getUniqueFeatures( buas, GfxFeature::BUILTUP_AREA );
   
   {for ( vec_t::const_iterator it = buas.begin(); it != buas.end(); 
         ++it ) {
      if ( totallyCoversBBox( **it, worldBBox ) ) {
         // Bua covers everything, no need to add land or ocean.
         mc2dbg8 << "[GFM]: Bua covers everything " << worldBBox << endl;
         return;
      }
   }}
   
   // Check land.
   // Collect unique lands.
   vec_t lands;
   gfxMap.getUniqueFeatures( lands, GfxFeature::LAND );

   {for ( vec_t::const_iterator it = lands.begin();
         it != lands.end(); ++it ) {
      if ( totallyCoversBBox( **it, worldBBox ) ) {
         mc2dbg8 << "[GFM]: Land covers everything, no need for ocean " 
                 << worldBBox << endl;
         // Land fully covers world.
         // Reduce the bbox so that it's as small as possible but
         // still covers the buas.
         MC2BoundingBox landBBox = reduceBBox( worldBBox, buas );
//         if ( landBBox != worldBBox ) {
//            mc2dbg8 << "[GFM]: Replaced land covering all bbox " << worldBBox
//                   << " with this bbox " << landBBox << endl;
//         } 
         resultMap.addFeature( 
               new GfxFeature( GfxFeature::LAND, 
                               landBBox,
                               (*it)->getName() ) );
         
         // No need to add ocean, since the land covers it.
         return;
   }}
   
   // No land fully covers the bbox.
   {for ( vec_t::const_iterator jt = lands.begin();
         jt != lands.end(); ++jt ) {
         // Add the land feature (that didn't fully cover the bbox).
         // FIXME: Move it from gfxMap to resultMap, and don't copy it.
         resultMap.addFeature( (*jt)->clone() );
         mc2dbg8 << "[GFM]: Adding nonfull land << " << worldBBox << endl;
      }
   }}

   // Add ocean.
   // The one and only ocean fully covers the bbox.

   // Reduce the bbox so that it's as small as possible but
   // still covers the lands.
   MC2BoundingBox oceanBBox = reduceBBox( worldBBox, lands );
//   if ( oceanBBox != worldBBox ) {
//      mc2dbg8 << "[GFM]: Replaced ocean covering all bbox " << worldBBox
//             << " with this bbox " << oceanBBox << endl;
//   } else {
//      mc2dbg8 << "[GFM]: Adding full ocean << " << oceanBBox << endl;
//   }
   
   // Split the ocean if it's covers the dateline.
   if ( oceanBBox.getMinLon() > oceanBBox.getMaxLon() ) {
      // Split the bbox in two
      MC2BoundingBox bbox1(oceanBBox);
      bbox1.setMaxLon( MAX_INT32 );
      MC2BoundingBox bbox2(oceanBBox);
      bbox2.setMinLon( MIN_INT32 );
      resultMap.addFeature( 
            new GfxFeature( GfxFeature::OCEAN, bbox1 ) );
      resultMap.addFeature( 
            new GfxFeature( GfxFeature::OCEAN, bbox2 ) );
   } else {
      resultMap.addFeature( 
            new GfxFeature( GfxFeature::OCEAN, oceanBBox ) );
   }
}

/**
 *    Recursive help method to mergeStreets.
 *
 *    @param   nodeByCoord       Map containing streetnodes (sorted)
 *                               indexed by coordinates.
 *    @param   mergedNodes       [IN/OUT] List of merged nodes
 *                               that will be filled with more 
 *                               nodes if merging was possible.
 *    @param   processedStreets  All processed street ids so far.
 *    @param   gfxMap            The gfxmap.
 *    @param   feature           The street feature that we're trying
 *                               to merge.
 *    @param   featureID         The feature id of the feature.
 *    @param   nodeNbr           Which of the nodes (0 or 1) that
 *                               we aretrying to merge.
 *    @param   forward           If merging in forward or backward
 *                               direction.
 *    @param   dontUseRamps      If set to true, no streets with the
 *                               ramp attribute are included in the
 *                               merging.
 */
void
mergeStreet( const map<MC2Coordinate, set<uint32> >& nodeByCoord,
             mergedNodes_t& mergedNodes, 
             set<uint32>& processedStreets, 
             const GfxFeatureMap& gfxMap,
             const GfxFeature& feature, 
             uint32 featureID,
             byte nodeNbr,
             bool forward,
             bool dontUseRamps ) {
   const GfxPolygon* poly = feature.getPolygon( 0 );

   MC2Coordinate coord;
   if ( nodeNbr == 0 ) {
      coord.lat = poly->getLat( 0 );
      coord.lon = poly->getLon( 0 );
   } else {
      // Node 1.
      uint32 lastCoord = uint32( poly->getNbrCoordinates() - 1 );
      coord.lat = poly->getLat( lastCoord );
      coord.lon = poly->getLon( lastCoord );
   }
   
   uint32 nbrOtherStreets = 0;
   uint32 nextNodeID = MAX_UINT32;
   const GfxFeature* nextFeature = NULL;
   
   map<MC2Coordinate, set<uint32> >::const_iterator findIt = 
      nodeByCoord.find( coord );
  
   if ( findIt == nodeByCoord.end() ) {
      // No other nodes at this coordinate. Done.
      return;
   }

   const set<uint32>& nodes = findIt->second;

   // Collect features connected to this node, filter out unwanted ones, 
   // and sort the ones we want in length order.
   multimap<double, const GfxFeature*> featureByLength;
   for ( set<uint32>::const_iterator it = nodes.begin();
         it != nodes.end(); ++it ) {
      uint32 otherFeatureID = REMOVE_UINT32_MSB( *it );
      if ( otherFeatureID != featureID ) {
         const GfxFeature* otherFeature = 
            gfxMap.getFeature( otherFeatureID );

         // Check if the feature type and the names are the same.
         if ( ( feature.getType() == otherFeature->getType() ) &&
              ( feature.getBasename() == otherFeature->getBasename() ) &&
              ( processedStreets.find( otherFeatureID ) == 
                processedStreets.end() ) ) {
            
            
            // Calculate length of all feature polygons, and check if this is
            // a ramp
            double featureLength = 0;
            bool isRamp = false;
            for ( uint32 polyIdx=0; polyIdx<otherFeature->getNbrPolygons();
                  polyIdx++){
               
               // Increment length
               GfxPolygon* gfxPoly = otherFeature->getPolygon(polyIdx);
               featureLength += gfxPoly->getLength();

               // Check ramp
               GfxRoadPolygon* gfxRoadPoly = 
                  dynamic_cast<GfxRoadPolygon*>(gfxPoly);
               if ( gfxRoadPoly != NULL && gfxRoadPoly->isRamp() ){
                  isRamp = true;
               }
            }
            if ( dontUseRamps && isRamp ){ // Hack, not merging ramps.
               // Do nothing because this is a ramp.
            }
            else {
               featureByLength.insert(make_pair(featureLength, otherFeature));
               ++nbrOtherStreets;
            }
         }
      }
      
      for ( multimap<double, const GfxFeature*>::reverse_iterator 
               featIt = featureByLength.rbegin(); 
            featIt != featureByLength.rend(); ++featIt ){
         
         const GfxFeature* otherFeature = featIt->second;
      // Type and name are the same. This feature can be grouped
      // together with the current one.
         
         if ( nextNodeID == MAX_UINT32 ) {
            // Merge with the first found node. This will allow
            // for better merging of route features since these
            // features are already sorted. It will not make any
            // difference for ordinary streets.
            nextNodeID = *it;
            nextFeature = otherFeature;
         }
      }
   }


   if ( nextNodeID != MAX_UINT32 ) {
      // Found the next node id for the street.
      
      // It's a crossing if more than one street was found.
      bool crossing = nbrOtherStreets > 1;
      if ( forward ) {
         // Forward, add to the back.
         mergedNodes.push_back( make_pair( crossing, nextNodeID ) );
      } else {
         // Backwards, add to the front.
         mergedNodes.front().first = crossing; 
         mergedNodes.push_front( 
               make_pair( false, TOGGLE_UINT32_MSB( nextNodeID ) ) );
      }
      processedStreets.insert( REMOVE_UINT32_MSB( nextNodeID ) );

      // Continue merging again. Toggle the node id.
      byte toggledNextNodeNbr = 1;
      if ( MapBits::isNode0( TOGGLE_UINT32_MSB( nextNodeID ) ) ) {
         toggledNextNodeNbr = 0;
      }
      // Recursive call. Try continue merging the next node.
      mergeStreet( nodeByCoord,
                   mergedNodes,
                   processedStreets,
                   gfxMap,
                   *nextFeature,
                   REMOVE_UINT32_MSB( nextNodeID ),
                   toggledNextNodeNbr,
                   forward,
                   dontUseRamps );
   } 

}



void dumpGfxData( const GfxData* gfxData ){
   for ( int32 p = 0; p< gfxData->getNbrPolygons(); p++ ){
      for ( uint32 c = 0; c < gfxData->getNbrCoordinates(p); c++ ){
         cout << gfxData->getCoordinate(p, c) << endl;
      }
   }
}

/**
 *    Help method to mergeAndFilterStreets.
 *    Will filter the GfxData and it as a new polygon to the
 *    GfxFeature.
 *    
 *    @param   tmpGfx   GfxData containing the geometry for the
 *                      feature.
 *    @param   feature  The feature to add a new polygon to.
 *    @param   maxCrossDist   Maximum cross distance for the
 *                            filtering algorithm.
 *    @param   streetDistCutOff The distance cutoff point for straight lines.
 */
void filterAndAddToFeature( GfxData* tmpGfx,
                            GfxFeature* feature,
                            uint32 maxCrossDist,
                            float64 streetDistCutOff ) {

   for ( uint32 p = 0; p < tmpGfx->getNbrPolygons(); ++p ) {

      // Filter and add.
      Stack filteredIndeces;
      tmpGfx->openPolygonFilter( &filteredIndeces,
                                 p, maxCrossDist, MAX_UINT32, true );
      mc2dbg8 << "maxCrossDist = " << maxCrossDist << endl;
      mc2dbg8 << "Filtering resulted in decrease of coordinates from "
         << tmpGfx->getNbrCoordinates( p )
         << " to " << filteredIndeces.getStackSize() << endl;

      // Add the filtered indeces to the gfxfeature.
      uint32 startIndex = 0;
      if ( p > 0 ) {
         // Skip the first coordinate for all but the first polygon.
         startIndex = 1;
      }

      // Add coordinates and remove some unnecessary coords for some
      // almost straight lines.
      for ( uint32 i = startIndex; 
            i < filteredIndeces.getStackSize();) {

         uint32 index1 = filteredIndeces.getElementAt( i );

         feature->addCoordinateToLast( tmpGfx->getLat( p, index1 ),
                                       tmpGfx->getLon( p, index1 ) );

         uint32 nextIndex = i;
         // If the the cutoff distance is zero, then dont even
         // bother with trying to straighten out the lines.
         if ( streetDistCutOff != 0 ) {
            nextIndex =
               GfxMapFilter::
               getFirstAngleDifferenceIndex( filteredIndeces,
                                             *tmpGfx,
                                             p, i,
                                             streetDistCutOff );
         }

         if ( nextIndex != i ) {
            i = nextIndex;
         } else {
            ++i;
         }
      }
   }
}


bool insideCity(MC2Coordinate coord){
   // Settings for Paris
   MC2Coordinate center(582883502, 28023783);
   uint32 westLon = 25135981;
   uint32 northLat = 584331547;

   uint32 lonDiff = center.lon - westLon;
   uint32 latDiff = northLat - center.lat;

   MC2BoundingBox box1(center.lat + latDiff, center.lon - lonDiff/2,
                       center.lat - latDiff, center.lon + lonDiff/2);
   if ( box1.inside( coord ) ){
      return true;
   }
   MC2BoundingBox box2(center.lat + latDiff/2, center.lon - lonDiff,
                       center.lat - latDiff/2, center.lon + lonDiff);
   if ( box2.inside( coord ) ){
      return true;
   }
   return false;
}


/**
 *    Will create a new GfxMap (which must be deleted by the caller)
 *    containing merged street features.
 *    Streets will be merged to be in the same polygon 
 *    if they share end coordinates and they have the same name.
 *    A new polygon is created for a crossing with other streets 
 *    with type present in visibleStreetTypes.
 *    
 *    Only streets of type streetTypeToMerge will be merged and
 *    present in the returned map.
 *
 *    @param   gfxMap               The map.
 *    @param   visibleStreetTypes   All visible types of streets.
 *    @param   alwaysIncludeTypes   Types that always should be 
 *                                  included in the resulting map.
 *    @param   streetTypeToMerge    The street type to merge.
 *    @param   maxCrossDist         Maximum cross distance for
 *                                  the filtering algorithm.
 *    @param   importance           The importance.
 *    @param   mapDesc              The tilemapformatdesc.
 *    @param   param                The TileMapParam for the
 *                                  importance.
 *    @return  A new gfxmap containing the merged streets.
 *             Must be deleted by the caller of this method!
 */
GfxFeatureMap* 
mergeAndFilterStreets( GfxFeatureMap& gfxMap,
                       const GfxFeatureTypeSet& visibleStreetTypes,
                       const GfxFeatureTypeSet& alwaysIncludeTypes,
                       const GfxFeatureTypeSet& streetTypesToMerge,
                       const TileImportanceNotice* importance,
                       const ServerTileMapFormatDesc& mapDesc,
                       uint32 maxCrossDist,
                       const TileMapParams& param ) {
   // Create a multimap with coordinates and feature id.
  
   // Create a map that has coordinates as key and a (sorted) set
   // of node ids present at the coordinates.
   // The node ids are sorted so that better merging of routes are
   // encouraged. Route features are already in the correct order in
   // the feature map.
   map< MC2Coordinate, set<uint32> > nodeByCoord;
   
   for ( uint32 i = 0; i < gfxMap.getNbrFeatures(); ++i ) {
      
      const GfxFeature* feature = gfxMap.getFeature( i );

      if ( visibleStreetTypes.find( feature->getType() ) != 
           visibleStreetTypes.end() ) {
         // This is a feature that will be visible.

         if ( feature->getNbrPolygons() != 1 ) {
            mc2dbg << "[GfxMapFilter]: feature has "
                   << feature->getNbrPolygons() << " polys " << endl;
               
         }
         MC2_ASSERT( feature->getNbrPolygons() == 1 );
         const GfxPolygon* poly = feature->getPolygon( 0 );
         if ( poly->getNbrCoordinates() > 0 ) {
            // First coord.
            MC2Coordinate coord( poly->getLat( 0 ), poly->getLon( 0 ) );
            nodeByCoord[ coord ].insert( i );
            
            // Last coord.
            uint32 lastCoord = uint32( poly->getNbrCoordinates() - 1 );
            coord.lat = poly->getLat( lastCoord );
            coord.lon = poly->getLon( lastCoord );
            nodeByCoord[ coord ].insert( TOGGLE_UINT32_MSB( i ) );
         }
      }      
   }

   // All the currently processed streets.
   set<uint32> processedStreets;

      
   vector<mergedNodes_t> allMergedNodes;
   
   bool dontUseRamps = false;
   int32 detailLevel = param.getDetailLevel();
   if (detailLevel >= 3 ){ // Fixme: Make using ramps a constant TileMapProperties
      dontUseRamps = true;
   }

   // Now merge
   for ( uint32 i = 0; i < gfxMap.getNbrFeatures(); ++i ) {
      
      const GfxFeature* feature = gfxMap.getFeature( i );
     
      // Check if the street is of the correct type 
      // and not already processed.
      if ( ( streetTypesToMerge.find(feature->getType()) !=
             streetTypesToMerge.end() ) &&
           ( processedStreets.find( i ) == processedStreets.end() ) ) {

         mergedNodes_t mergedNodes;
         mergedNodes.push_back( make_pair( false, i ) );
         // Mark it as processed.
         processedStreets.insert( i );

         // Start with node 1 in order to promote merging of 
         // route features.

         // Merge node 1
         mergeStreet( nodeByCoord,
                      mergedNodes, 
                      processedStreets, 
                      gfxMap,
                      *feature, 
                      i,
                      1, 
                      true,
                      dontUseRamps);
         
         // Merge node 0
         mergeStreet( nodeByCoord,
                      mergedNodes, 
                      processedStreets, 
                      gfxMap,
                      *feature, 
                      i,
                      0, 
                      false,
                      dontUseRamps);
         
 
         // Add to allMergedNodes.
         allMergedNodes.push_back( mergedNodes );
         
        
      }
   } 

   DEBUG8(
   // Dump everything
   for ( vector<mergedNodes_t>::const_iterator it = allMergedNodes.begin();
         it != allMergedNodes.end(); ++it ) {
      
      mc2dbg << "Merged street:" << endl;
      for ( mergedNodes_t::const_iterator jt = it->begin(); jt != it->end();
            ++jt ) {

         mc2dbg << "ID = " << jt->second << ", crossing = " << jt->first;
         const GfxFeature* feature = 
            gfxMap.getFeature( REMOVE_UINT32_MSB( jt->second ) );
         const GfxPolygon* poly = feature->getPolygon( 0 );
         uint32 lastCoord = uint32( poly->getNbrCoordinates() - 1 );
         if ( MapBits::isNode0( jt->second ) ) {
            mc2dbg << " ( " << poly->getLat( 0 ) << ", "
               << poly->getLon( 0 ) << " ) - ( "
               << poly->getLat( lastCoord ) << ", "
               << poly->getLon( lastCoord ) << " )" << endl;
         } else {
            mc2dbg << "( " << poly->getLat( lastCoord ) << ", "
               << poly->getLon( lastCoord ) << " ) - ( "
               << poly->getLat( 0 ) << ", "
               << poly->getLon( 0 ) << " )" << endl;
         }

      }
   }
   );


   // The new gfxmap that will contain the merged streets.
   GfxFeatureMap* newGfxMap = new GfxFeatureMap();
   MC2BoundingBox bbox;
   gfxMap.getMC2BoundingBox( &bbox );
   newGfxMap->setMC2BoundingBox( &bbox );
   
   // feature length by feature index in newGfxMap
   map<uint32, uint32> streetLengthsByIdx;

   // After a few experiments, 300 seems like a good value.
   // Roundabouts and ramps are detected in the loop below, which then set
   // the cutoff to 0.
   float64 streetDistCutOff =
      PropertyHelper::get<float64>( "STREET_DIST_CUT_OFF", 300.0 );

   // Merge the streets.
   for ( vector<mergedNodes_t>::const_iterator it = allMergedNodes.begin();
         it != allMergedNodes.end(); ++it ) {
      GfxFeature* feature = NULL;
      GfxDataFull* tmpGfx = GfxData::createNewGfxData( NULL, false );
      for ( mergedNodes_t::const_iterator jt = it->begin(); jt != it->end();
            ++jt ) {
         bool crossing = jt->first;
         uint32 nodeID = jt->second;
         const GfxFeature* oldFeature = 
            gfxMap.getFeature( REMOVE_UINT32_MSB( nodeID ) );
            
         const GfxPolygon* oldPoly = oldFeature->getPolygon( 0 );

         bool createNewPoly = false;
         if ( feature == NULL ) {           
            // Hack.
            char abbrevName[1024];
            abbrevName[ 0 ] = 0;
            const char* nameToAdd = oldFeature->getName();
            if ( ! oldFeature->getNameIsAbbreviated() ) {
               // Abbreviate the names.
               LangTypes::language_t langType = oldFeature->getLangType();
               AbbreviationTable::abbreviate( 
                        nameToAdd,
                        abbrevName,
                        langType,
                        AbbreviationTable::beginningAndEnd );
               nameToAdd = abbrevName;
            }
            
            feature = GfxFeature::createNewFeature( 
                  oldFeature->getType(),
                  nameToAdd );
            // Now add the new polygon.
            feature->addNewPolygon( false, oldPoly->getNbrCoordinates() );
            // keep the base name too, so we can compare streets later
            feature->setBasename( oldFeature->getBasename().c_str() );

            // Copy the attributes from the old polygon to the new.
            const GfxRoadPolygon* oldRoadPoly = 
               dynamic_cast<const GfxRoadPolygon*> ( oldPoly );
            if ( oldRoadPoly != NULL ) {
               GfxRoadPolygon* roadPoly = 
                  static_cast<GfxRoadPolygon*> ( feature->getPolygon( 0 ) );

               // Roundabout and ramps should not be filtered in the
               // "straightout"-algorithm, so set the cutoff to 0
               if ( roadPoly->isRoundabout() || roadPoly->isRamp() ) {
                  streetDistCutOff = 0;
               }

               roadPoly->setParams( oldRoadPoly->getPosSpeedlimit(),
                                    oldRoadPoly->getNegSpeedlimit(),
                                    oldRoadPoly->isMultidigitialized(),
                                    oldRoadPoly->isRamp(),
                                    oldRoadPoly->isRoundabout(),
                                    oldRoadPoly->getLevel0(),
                                    oldRoadPoly->getLevel1(),
                                    oldRoadPoly->getEntryRestrictions( 0 ),
                                    oldRoadPoly->getEntryRestrictions( 1 ));
               if ( dontUseRamps && oldRoadPoly->isRamp() ){ 
                  // Hack, not adding ramps.
                  // Ramps have not been merged with anything, so this should
                  // be OK.
                  delete feature;
                  feature = NULL;
                  continue;
               }
            }
               
            newGfxMap->addFeature( feature );
            createNewPoly = true;
            tmpGfx->addPolygon();
            mc2dbg8 << "new feature and pol" << endl;
         } else if ( crossing ) {
            // New polygon.
            createNewPoly = true;
            tmpGfx->addPolygon();
            mc2dbg8 << "new pol" << endl;
         }
           
         if ( MapBits::isNode0( nodeID ) ) {
            // Add in normal order.

            // Skip the first coordinate if unless it's a new polygon. 
            uint32 startIdx = 1;
            if ( createNewPoly ) {
               startIdx = 0;
            }
            for ( uint32 i = startIdx; 
                  i < oldPoly->getNbrCoordinates(); ++i ) {
               tmpGfx->addCoordinate( oldPoly->getLat( i ),
                                      oldPoly->getLon( i ) );
               mc2dbg8 << "Add coord: " << oldPoly->getLat( i ) << ","
                      << oldPoly->getLon( i ) << endl;
            }
         } else {
            // Add in reverse order.
            
            // Skip the first coordinate if unless it's a new polygon. 
            int32 startIdx = oldPoly->getNbrCoordinates() - 2;
            if ( createNewPoly ) {
               ++startIdx;
            }
            for ( int32 i = startIdx; i >= 0; --i ) {
               tmpGfx->addCoordinate( oldPoly->getLat( i ),
                                      oldPoly->getLon( i ) );
               mc2dbg8 << "Add coord: " << oldPoly->getLat( i ) << ","
                      << oldPoly->getLon( i ) << endl;
               
            }
         }         
      }
      if ( feature != NULL ) {
         
         // Hack use this only on detail 4.
         // Put the distance in the utility.
         // Calculate feature length
         float64 gfxLength = 0; // Length in meters.
         MC2Coordinate fistCoord, lastCoord;
         if ( tmpGfx->getNbrPolygons() > 0 && 
              tmpGfx->getNbrCoordinates(0) > 0 ){
            fistCoord = tmpGfx->getCoordinate(0,0);
            uint32 nbrLastCoords =
               tmpGfx->getNbrCoordinates(tmpGfx->getNbrPolygons()-1);
            if ( nbrLastCoords > 0 ){
               lastCoord = 
                  tmpGfx->getCoordinate(tmpGfx->getNbrPolygons()-1, 
                                        nbrLastCoords-1);
            }
         }
         for ( uint32 polyIdx=0; polyIdx < tmpGfx->getNbrPolygons();
               polyIdx++){
            // Increment length
            gfxLength += tmpGfx->getLength(polyIdx);
            mc2dbg8 << "Gfx calc for poly: " << polyIdx << " = " 
                   << tmpGfx->getLength(polyIdx) << endl;
         }
         uint32 nbrFeatures = newGfxMap->getNbrFeatures();
         streetLengthsByIdx.insert(make_pair(nbrFeatures-1,
                                             static_cast<uint32>(gfxLength)));
         //streetLengths.insert(static_cast<uint32>(gfxLength));
         // Not adding too small items for high detail levels.
         int detailLevel = param.getDetailLevel();
         if ( (detailLevel > 3) &&  // detail level threshold as constant.
              ( (gfxLength < 500) ) // length in meters. Fixme: set as constant in TileMapProperties
              //||//
                //  feature->getType() != GfxFeature::STREET_MAIN &&
                //insideCity(fistCoord) && insideCity(lastCoord) ) 
              ){
            
            // Remove the last added feature if it is too small.
            if (nbrFeatures > 0){
               newGfxMap->removeFeature(nbrFeatures-1);
            }
         }
         else {
         
            // Filter and add the coordinates.
            filterAndAddToFeature( tmpGfx, feature, maxCrossDist,
                                   streetDistCutOff );
         }
      }
      delete tmpGfx;



   }
   
   // dumping merged streets lengths.
   // Find threshold for 1000 largest items.
   /* uint32 count = 0;
   uint32 totNbr = streetLengths.size();
   uint32 featSizeThreshold = MAX_UINT32;
   for ( multiset<uint32>::const_iterator it = streetLengths.begin();
         it != streetLengths.end(); ++it){
      mc2dbg << "str[" << count << "]: " << *it << endl;
      if ( (totNbr - count) == 1000 ){
         featSizeThreshold = *it;
      }
      count++;
   }
   // Remove too small features.
   for ( uint32 f=0; f<newGfxMap->getNbrFeatures(); f++){
      //const GfxFeature* feature = newGfxMap->getFeature(f);
      multimap<uint32, uint32>::const_iterator findIt = streetLengthsByIdx.find(f);
      MC2_ASSERT(findIt != streetLengthsByIdx.end());
      if ( findIt->second < featSizeThreshold ){
         newGfxMap->removeFeature(f);
      }
   } */

   // Check if to add the types found in alwaysIncludeTypes.
   if ( ! alwaysIncludeTypes.empty() ) {
      for ( uint32 i = 0; i < gfxMap.getNbrFeatures(); ++i ) {
         const GfxFeature* feature = gfxMap.getFeature( i );
         if ( alwaysIncludeTypes.find( feature->getType() ) !=
              alwaysIncludeTypes.end() ) {
            // Check if this feature is to be added.
            if ( mapDesc.isGfxOfImportance( 
                     feature, importance, MAX_UINT32, param ) ) { 
               newGfxMap->addFeature( feature->clone() );
            }
         }
      }
   }
   
   return newGfxMap;
   
}

namespace GfxMapFilter {

/**
 * Determines the next index to use in the polygon, i.e whether to remove
 * any indeces in between to do a straighter line.
 *
 * @param filteredIndeces Contains index in to gfx polygon.
 * @param gfx Contains the coordinates
 * @param poly Polygon index in Gfx to filter.
 * @param startIndexIn Start index in filteredIndeces.
 * @param distanceCutOff The maximum distance before
 * @return next index to be used in filteredIndeces.
 */
uint32 getFirstAngleDifferenceIndex( const Stack& filteredIndeces,
                                     const GfxData& gfx,
                                     uint32 poly,
                                     uint32 startIndexIn,
                                     float64 distanceCutOff ) {
   if ( startIndexIn + 2 >= filteredIndeces.getStackSize() ) {
      return startIndexIn + 1;
   }

   uint32 startIndex = startIndexIn;
   MC2Coordinate startCoord = gfx.getCoordinate( poly, startIndexIn );
   float64 dist = 0;
   while ( startIndex + 2 < filteredIndeces.getStackSize() &&
           dist <= distanceCutOff ) {
      // Calculate the distance from line (index1 -> index3) to point (index2)
      uint32 index1 = filteredIndeces.getElementAt( startIndexIn );
      uint32 index2 = filteredIndeces.getElementAt( startIndex + 1 );
      uint32 index3 = filteredIndeces.getElementAt( startIndex + 2 );
      dist =
         sqrt( GfxUtility::
         closestDistVectorToPoint( gfx.getLon( poly, index1 ),
                                   gfx.getLat( poly, index1 ),
                                   gfx.getLon( poly, index3 ),
                                   gfx.getLat( poly, index3 ),
                                   gfx.getLon( poly, index2 ),
                                   gfx.getLat( poly, index2 ),
                                   GfxUtility::getCoslat( startCoord.lat,
                                                          startCoord.lat ) ));
      if ( dist > distanceCutOff ) {
         break;
      }
      startIndex += 2;
   }

   if ( startIndex >= filteredIndeces.getStackSize() ) {
      startIndex = filteredIndeces.getStackSize() - 1;
   }

   return startIndex;
}

GfxFeatureMap* 
filterGfxMap( GfxFeatureMap& gfxMap, 
              const ServerTileMapFormatDesc& mapDesc,
              const TileMapParams& param,
              float64 meterToPixelFactor,
              bool removeNames )
{
   // Remove names if requested.
   if ( removeNames ) {
      for ( uint32 i = 0; i < gfxMap.getNbrFeatures(); ++i ) {
         GfxFeature* feature = 
            const_cast<GfxFeature*> (gfxMap.getFeature( i ));
         feature->setName( "" );
         feature->setBasename( "" );
      }
   }
   
   const TileImportanceNotice* importance = 
      mapDesc.getImportanceNbr( param );
  
   // Remove all features but those that match the selected importance.
   // Also merge and filter features.
   GfxFeatureMap* newGfxMap = filterImportance( &gfxMap, 
                                                importance,
                                                mapDesc,
                                                param,
                                                meterToPixelFactor );
 
   // Remove duplicate features.
   newGfxMap->removeDuplicates();
   
   // Return the new gfxmap that has to be deleted by the caller.
   return newGfxMap;
}

} // GfxMapFilter
