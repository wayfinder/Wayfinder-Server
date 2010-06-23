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

#include "ClockWise.h"
#include "GfxPolygon.h"
#include "GfxRoadPolygon.h"
#include "GfxUtility.h"
#include "Intersect.h"
#include "Point.h"
#include "StringUtility.h"
#include "TileMapConfig.h"
#include "TileMapCreator.h"
#include "UTF8Util.h"
#include "TileMapCoord.h"
#include "GnuPlotDump.h"
#include "PolygonDefects.h"
#include "MC2String.h"
#include "MC2Exception.h"
#include "TileMapBufferHolder.h"
#include "POIImageIdentificationTable.h"
#include "EmptyTileMap.h"

TileMapCreator::TileMapCreator( const ServerTileMapFormatDesc& desc ):
   m_poiImageTable( new POIImageIdentificationTable( desc ) ) {
   
}

TileMapCreator::~TileMapCreator() {


}

TileFeature* 
TileMapCreator::createTileFeature( const ServerTileMapFormatDesc* desc,
                                   WritableTileMap* tileMap,
                                   const GfxFeature* gfxFeature,
                                   const GfxPolygon* gfxPolygon )
{
   // Creates right type. Checks that the gfxFeature contains enough
   // information to become a TileFeature (i.e. has a coordinates etc).
   int16 type = desc->getTileFeatureTypeFromGfxFeature( gfxFeature );

   WritableTileFeature* tileFeature = NULL;

   if ( type < 0 ) {
      return NULL;
   }
   bool isSCPOI = ServerTileMapFormatDesc::isSpecialCustomPOI( type );
      
      
   if ( gfxFeature->getType() == GfxFeature::POI &&
        isSCPOI ) {

      // handle special custom pois here
      const GfxPOIFeature* poiFeature = 
         static_cast<const GfxPOIFeature*>( gfxFeature );
      
      tileFeature = setupCustomPOI( *poiFeature, *tileMap, *desc, type );
      
      // failed to create a feature for this? :(
      if ( tileFeature == NULL ) {
         return NULL;
      }
   } else {
      
      // take care of normal pois and other features here.
      tileFeature = tileMap->getCurrentNewFeature( type );

      vector<TileFeatureArg*> args;
      desc->getArgsForFeatureType( type, args );
      tileFeature->setArgs( args );
   }
      // --------------------
      // --- Add geometry ---
      // --------------------
      if ( desc->hasArg( type, TileArgNames::coord ) ) {
         // One coordinate
         CoordArg* arg = static_cast<CoordArg*> 
            ( tileFeature->getArg( TileArgNames::coord ) );
         arg->setCoord( gfxPolygon->getLat( 0 ), 
                        gfxPolygon->getLon( 0 ),
                        *tileMap );
      } else if ( desc->hasArg( type, TileArgNames::coords ) ) {   
         // One polygon
         CoordsArg* arg = static_cast<CoordsArg*> 
            ( tileFeature->getArg( TileArgNames::coords ) );
         for ( GfxPolygon::const_iterator it = gfxPolygon->begin(),
                  end = gfxPolygon->end();
               it != end; ++it ) {
            arg->addCoord( it->lat, it->lon, *tileMap );
         }
      } 


      // handle normal poi images here
      if ( !isSCPOI && 
           desc->hasArg( type, TileArgNames::image_name ) ) {
         MC2_ASSERT( gfxFeature->getType() == GfxFeature::POI );
         const GfxPOIFeature* poiFeature = 
            static_cast<const GfxPOIFeature*>( gfxFeature );
          tileFeature->setArg( TileArgNames::image_name,
                               poiFeature->getCustomImageName().c_str() );
      }
      
      // ------------------------------------
      // --- SSI specific: level0, level1 ---
      // ------------------------------------
      switch ( gfxFeature->getType() ) {
         case ( GfxFeature::STREET_MAIN ) :
         case ( GfxFeature::STREET_FIRST ) :
         case ( GfxFeature::STREET_SECOND ) :
         case ( GfxFeature::STREET_THIRD ) :
         case ( GfxFeature::STREET_FOURTH ) :
         case ( GfxFeature::WALKWAY ): {
            const GfxRoadPolygon* gfxRoadPoly = 
               static_cast<const GfxRoadPolygon*> ( gfxPolygon );
            bool hasLevel0 = desc->hasArg( type, TileArgNames::level );
            bool hasLevel1 = desc->hasArg( type, TileArgNames::level_1 );
            int level0 = 0;
            int level1 = 0;
            if ( hasLevel0 ) {
               level0 = ServerTileMapFormatDesc::getRoadLevelWithOffset(
                                          gfxRoadPoly->getLevel0() );
            }
            if ( hasLevel1 ) {
               level1 = ServerTileMapFormatDesc::getRoadLevelWithOffset(
                                          gfxRoadPoly->getLevel1() );
            }

            if ( hasLevel0 && hasLevel1 ) {
               // If both levels exist, then use the lowest level for both levels.
               level0 = MIN(level0, level1);
               level1 = level0;
            }
            if ( hasLevel0 ) {
               tileFeature->setArg( TileArgNames::level, level0 );
            }
            if ( hasLevel1 ) {
               tileFeature->setArg( TileArgNames::level_1, level1 ); 
            }
            break;
         }
         default:
            break;
      }

      if ( gfxFeature->getType() == GfxFeature::EVENT ) {
         const GfxEventFeature* event =
            static_cast<const GfxEventFeature*>( gfxFeature );
         tileFeature->setArg( TileArgNames::time, event->getDate() );
         tileFeature->setArg( TileArgNames::ext_id, event->getID() );
         tileFeature->setArg( TileArgNames::duration, event->getDuration() );
      }
   return tileFeature;
}

typedef vector< vector<MC2Coordinate> > coordVectVect_t;

/// Snap to tilemap coords and remove duplicates.
static bool
snapToCoord( coordVectVect_t& polygons,
             const TileMap& tilemap )
{
   bool changedAnything = false;
   for ( uint32 i = 0; i < polygons.size(); ++i ) {
      vector<MC2Coordinate>& coords = polygons[ i ];

      for ( uint32 j = 0; j < coords.size(); ++j ) {
         TileMapCoord currCoord = coords[ j ];
         tilemap.snapCoordToPixel( currCoord );
         if ( currCoord != coords[ j ] ) {
            coords[ j ] = currCoord;
            changedAnything = true;
         }
      }
      // Remove consequtive identical coordinates.
      uint32 size = coords.size();
      coords.resize( std::distance( coords.begin(), 
                        std::unique( coords.begin(), coords.end() ) ) );
      if ( coords.size() != size ) {
         changedAnything = true;
      }
   }

   return changedAnything;
}

// Fix orientation, remove polygon defects and remove polygons 
// with less than 3 coords.
static void
fixOrientation( coordVectVect_t& polygons )
{
   for ( uint32 i = 0; i < polygons.size(); ++i ) {
      vector<MC2Coordinate>& coords = polygons[ i ];

      if ( coords.size() < 3 ) {
         // Erase.
         polygons.erase( polygons.begin() + i );
         // Stand still.
         --i;
         // Continue.
         continue;
      }

      // Must remove polygon defects first, otherwise clockwise will not work.
      vector<MC2Coordinate> fixedCoords;
      GfxUtil::removeSomePolygonDefects( fixedCoords, coords.begin(), coords.end() );
      coords.swap( fixedCoords );

      int clockwise = GfxUtil::isClockWise( coords.begin(), 
                                            coords.end() );

      if ( clockwise != 0 && clockwise != 1 ) {
         // Orientation could not be determined - remove poly
         if ( ! coords.empty() ) {
            mc2log << error << "[GfxUtility]: Orientation could not be determined" 
                   << " even if not empty" << endl;
            MC2_ASSERT( false );
         }

         // Erase.
         polygons.erase( polygons.begin() + i );
         // Stand still.
         --i;
         // Continue.
         continue;
      }

      // Take the next one if already clockwise.
      if (clockwise == 1 ) {
         continue;
      }
      
      // I put my thang down, flip it and reverse it.
      std::reverse( coords.begin(), coords.end() );

   }
}

static void
cleanPolygons( coordVectVect_t& polygons,
               const TileMap& tilemap ) throw (MC2Exception)
{
   bool runagain = true;
   snapToCoord( polygons, tilemap );
   while ( runagain ) {
      // Fix orientation.
      fixOrientation( polygons );

      if ( polygons.empty() ) {
         mc2dbg << "[TMC::cleanPolygons]: no polys after fixOrientation" 
                << endl;
      }
      // Clean self intersections.
      GfxUtility::cleanSelfintersecting( polygons );
      // Make new polygons of self touching stuff.
      bool newPolyIntroduced = GfxUtility::splitSelfTouching( polygons );
      // Snap.
      bool snappedAnything = snapToCoord( polygons, tilemap );

      runagain = snappedAnything || newPolyIntroduced;
   }
}

static void
featureToVector( coordVectVect_t& polygons, const GfxFeature& feature ) 
{
   polygons.clear();
   polygons.resize( feature.getNbrPolygons() ); 
   for( uint32 p = 0; p < feature.getNbrPolygons(); ++p ) {
      GfxPolygon* poly = feature.getPolygon( p );
      polygons[ p ].insert( polygons[ p ].end(), poly->begin(), poly->end() );
   }
}

static void
vectorToFeature( GfxFeature& feature, const coordVectVect_t& polygons ) 
{
   feature.resizeNbrPolygons( polygons.size() );

   for ( uint32 i = 0; i < polygons.size(); ++i ) {
      feature.getPolygon( i )->setCoords( polygons[ i ] );
   }
}

namespace {

void addCategories( WritableTileMap& tileMap,
                    const GfxFeature& gfxFeature,
                    int32 firstFeatureIndex ) {
   // The firstFeatureIndex must be valid and
   // there are only two types of features that holds
   // categories:
   if ( firstFeatureIndex == -1 ||
        ( gfxFeature.getType() != GfxFeature::POI &&
          gfxFeature.getType() != GfxFeature::EVENT ) ) {
      // no categories to add.
      return;
   }
   // Get the correct category vector
   const GfxPOIFeature::Categories* cats = NULL;
   if ( gfxFeature.getType() == GfxFeature::POI ) {
      const GfxPOIFeature& poiFeature =
         static_cast<const GfxPOIFeature&>( gfxFeature );
      cats = &poiFeature.getCategories();
   } else if ( gfxFeature.getType() == GfxFeature::EVENT ) {
      const GfxEventFeature& event =
         static_cast<const GfxEventFeature&>( gfxFeature );
      cats = &event.getCategories();
   }
   if ( cats != NULL ) {
      GfxPOIFeature::Categories::const_iterator it = cats->begin();
      for ( ; it != cats->end(); ++it ) {
         tileMap.addCategory( firstFeatureIndex, *it );
      }
   }
}

void addExtendedStrings( WritableTileMap& tileMap,
                         const GfxEventFeature& event,
                         int32 firstFeatureIndex ) {
   GfxEventFeature::Strings::const_iterator strIt = event.getStrings().begin();
   for ( ; strIt != event.getStrings().end(); ++strIt ) {
      tileMap.
         addExtendedString( ExtendedTileString( firstFeatureIndex,
                                                static_cast<ExtendedTileString::Type>( strIt->getType() ),
                                                strIt->getString() ) );
   }
}

}

TileMap* 
TileMapCreator::createTileMap( const TileMapParams& params,
                               const ServerTileMapFormatDesc* desc,
                               const TileImportanceNotice& importance,
                               GfxFeatureMap* gfxMap )
{
   // A feature must only contain one polygon here.  
   typedef set<const GfxFeature*> avoidSet_t;
   avoidSet_t avoidSet;
   
   WritableTileMap refTileMap( params, 
                               *desc,
                               importance.getType(),
                               0 );

   // Check for self-intersections and if the features should be added
   // to the map
   for ( GfxFeatureMap::iterator it = gfxMap->begin(), end = gfxMap->end();
         it != end;
         ++it ) {
      const GfxFeature* gfxFeature = *it;
      
      // Check that it will become a valid tilefeature.
      int16 type = desc->getTileFeatureTypeFromGfxFeature( gfxFeature );
      if ( type < 0 ) {
         avoidSet.insert( gfxFeature );
         continue;
      }

      if ( ! desc->closedFeatureType( type ) ) {
         // No more checks for non-closed polygons.
         continue;
      }

      // Feature -> vector.
      coordVectVect_t polygons;
      featureToVector( polygons, *gfxFeature );
      // Remove self intersections, make new polys if self touching etc.
      try { 
         cleanPolygons( polygons, refTileMap ); 
      
      } catch ( const MC2Exception& e ) {
         mc2log << error << "[TMC]: Exception caught for " 
                << params.getAsString() << endl;
         mc2log << error << e << endl;
         avoidSet.insert( gfxFeature );
         continue;
      }
      // Back to feature.
      vectorToFeature( **it, polygons );

      if ( polygons.empty() ) {
         mc2dbg << "[TMC]: Empty polygons after cleanPolygons!" << endl;
         avoidSet.insert( gfxFeature );
         continue;
      }
   }

   // Check for and remove ovelapping poi:s 
   POIOverlapDetector::removeOverlappedPOIs( *gfxMap,
                                             params,
                                             refTileMap, // For snapping coords.
                                             *desc, 
                                             m_poiDataCache,
                                             avoidSet );
   
   // Count the number of polygons.
   int nbrPolys = 0;
   for ( uint32 i = 0; i < gfxMap->getNbrFeatures(); ++i ) {
      const GfxFeature* gfxFeature = gfxMap->getFeature( i );
      if ( avoidSet.find( gfxFeature ) != avoidSet.end() ) {
         continue;
      }
      // Check that it will become a valid tilefeature.
      if ( desc->getTileFeatureTypeFromGfxFeature( gfxFeature ) >= 0 ) {
         nbrPolys += gfxFeature->getNbrPolygons();
      }
   }
   

   // Create the new TileMap.
   WritableTileMap* tileMap =
      new WritableTileMap( params, 
                           *desc,
                           importance.getType(),
                           nbrPolys );

   // Add features from the gfxMap to the TileMap.
   int nbrNotAddedFeatures = 0;
   for ( uint32 i = 0; i < gfxMap->getNbrFeatures(); ++i ) {
      const GfxFeature* gfxFeature = gfxMap->getFeature( i );

      if ( avoidSet.find( gfxFeature ) != avoidSet.end() ) {
         continue;
      }

      int firstFeatureIndex = -1; // first valid feature in the current gfx 
      for ( uint32 p = 0; p < gfxFeature->getNbrPolygons(); ++p ) {
         GfxPolygon* poly = gfxFeature->getPolygon( p );
         TileFeature* tileFeature = 
            createTileFeature( desc, tileMap, gfxFeature, poly );

         if ( tileFeature == NULL ) {
            ++nbrNotAddedFeatures;
            continue;
         }
         // Add tile feature.
         int featureIndex = tileMap->add( tileFeature );
         if ( firstFeatureIndex == -1 ) {
            firstFeatureIndex = featureIndex;
         }

         // ----------------
         // --- Add name ---
         // ----------------
         if ( desc->hasName( tileFeature->getType() ) ) {
            const char* name1 = gfxFeature->getName();
            MC2String name =
               StringUtility::makeFirstInWordCapital( name1,
                                                      true ); // Romans uc.
            // if type is event or traffic info, then keep original name
            if ( gfxFeature->getType() == GfxFeature::EVENT || 
                 gfxFeature->getType() == GfxFeature::TRAFFIC_INFO) {
               name = name1;
            }
            if ( ! name.empty() ) {
               // Add the name as UTF-8.
               tileMap->addName( featureIndex,
                                 UTF8Util::mc2ToUtf8( name ).c_str() );
            }
         }
      }

      // add categories, if any
      ::addCategories( *tileMap, *gfxFeature, firstFeatureIndex );

      if ( gfxFeature->getType() == GfxFeature::EVENT ) {
         ::addExtendedStrings( *tileMap,
                               static_cast<const GfxEventFeature&>(*gfxFeature),
                               firstFeatureIndex );
      }
   }
   
   // Removed not added features from m_features in timeMap
   tileMap->nbrNotAddedFeatures( nbrNotAddedFeatures );

   // All features added.
   tileMap->complete();

   mc2dbg8 << "[TMC]: createTileMap ENDS" << endl;

   return tileMap; 
}

inline WritableTileFeature* 
TileMapCreator::
setupCustomPOI( const GfxPOIFeature& poi,
                WritableTileMap& tileMap,
                const ServerTileMapFormatDesc& desc,
                int32 tileFeatureType ) const {

   // find real tile feature from poi type
   TileFeature::tileFeature_t realFeatureType = 
      desc.getTileFeatureForPOI( poi.getPOIType() );
   // get image code
   MC2String imageName = m_poiImageTable->
      encode( poi.getPOIType(), poi.getCustomImageName().c_str() );

   // if we fail to find image code, lets revert back to 
   // the real feature type as the new feature.
   if ( imageName.empty() ) {
      if ( realFeatureType != TileFeature::nbr_tile_features ) {
         mc2dbg << "[TMC] Creating default feature instead ;(" << endl;
         mc2dbg << "[TMC] original feature = " << tileFeatureType
                << " new feature = " << realFeatureType << endl;
         WritableTileFeature* tileFeature = 
            tileMap.getCurrentNewFeature( realFeatureType );
         vector<TileFeatureArg*> args;
         desc.getArgsForFeatureType( realFeatureType, args );
         tileFeature->setArgs( args ); 
         return tileFeature;
      }
      // else we failed...
      return NULL;
   }
   WritableTileFeature* tileFeature = 
      tileMap.getCurrentNewFeature( tileFeatureType );
   
   
   vector<TileFeatureArg*> args;
   desc.getArgsForFeatureType( tileFeatureType, args );
   tileFeature->setArgs( args );
   
   tileFeature->setArg( TileArgNames::real_feature_type, realFeatureType );
   // no need to check has arg here, this kind of feature 
   // type must have TileArgNames::image_name
   // if ( desc.hasArg( tileFeatureType, TileArgNames::image_name ) )
   tileFeature->setArg( TileArgNames::image_name, imageName.c_str() );
   return tileFeature;

}

TileFeature*
TileMapCreator::createOceanFeature( WritableTileMap& tileMap,
                                    const TileMapFormatDesc* desc,
                                    const MC2BoundingBox& mapBBox ) {

   WritableTileFeature* ocean = tileMap.getCurrentNewFeature( TileFeature::ocean );

   vector<TileFeatureArg*> args;

   desc->getArgsForFeatureType( TileFeature::ocean, args );
   ocean->setArgs( args );
   CoordsArg* arg = 
      static_cast<CoordsArg*> ( ocean->getArg( TileArgNames::coords ) );
   // add in clockwise order
   arg->addCoord( mapBBox.getMaxLat(), mapBBox.getMinLon(),
                  tileMap );
   arg->addCoord( mapBBox.getMaxLat(), mapBBox.getMaxLon(),
                  tileMap );
   arg->addCoord( mapBBox.getMinLat(), mapBBox.getMaxLon(),
                  tileMap );
   arg->addCoord( mapBBox.getMinLat(), mapBBox.getMinLon(),
                  tileMap );
   return ocean;
}

TileMapBufferHolder*
TileMapCreator::createOceanTileMap( const MC2SimpleString& str,
                                    const ServerTileMapFormatDesc& desc ) {
   TileMapParams param( str );

   // If the layer ID is invalid then we need to create an empty tile map
   // instead of the ocean tile map.
   // For example some b0rk client might send: TQ2sAPOST
   if ( ! desc.isValidLayerID( param.getLayer() ) ||
        ! desc.isValidImportance( param ) ) {
      
      mc2dbg << "[TMC] Param: " << str 
             << " layer is invalid! Will create empty tmap instead of ocean."  << endl;

      EmptyTileMap m( param );
      auto_ptr<BitBuffer> buf( new BitBuffer( str.length() + 100 ) );
      uint32 crc;
      m.save( *buf, &crc );
      // addBuffer must have same getBufferSize as getCurrentOffset
      buf->setSizeToOffset();
      buf->reset();

      return new TileMapBufferHolder( str.c_str(),
                                      buf.release(), crc,
                                      false );
   }

   MC2BoundingBox box;
   desc.getBBoxFromTileIndex( param.getLayer(),
                              box,
                              param.getDetailLevel(),
                              param.getTileIndexLat(), param.getTileIndexLon() );

   WritableTileMap tmap( param, desc,
                         desc.getImportanceNbr( param )->getType(),
                         1 );

   createOceanFeature( tmap, &desc, box );

   tmap.complete();

   // force crc 
   BitBuffer* buf = new BitBuffer( 1024*1024*10 ); // 10Mbytes
   uint32 crc = MAX_UINT32;
   tmap.save( *buf, &crc );
   buf->setSizeToOffset();
   buf->reset();
   // holder now owns buf
   return new TileMapBufferHolder( str.c_str(), buf, crc, false );
}
