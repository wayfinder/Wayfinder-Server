/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapDrawingCommon.h"

#include "MC2Point.h"
#include "GfxFeature.h"
#include "GfxData.h"
#include "DrawingProjection.h"

#include "MapUtility.h"
#include "MC2Coordinate.h"
#include "GfxPolygon.h"
#include "StringUtility.h"
#include "TimeUtility.h"
#include "MapSettings.h"
#include "GfxFeatureMap.h"
#include "ObjectBoxes.h"

namespace {

void addPOIBox( MapDrawingCommon::ObjectBoxes& boxes,
                const MC2BoundingBox& box,
                const DrawingProjection& projection ) {
   // need to move the poi collision box a bit
   // since its placed in the center
   int32 lonDiff = box.getLonDiff() / 2;
   int32 latDiff = box.getHeight() / 2;
   MC2BoundingBox poiBox( box.getMaxLat() + latDiff,
                          box.getMinLon() + lonDiff,
                          box.getMinLat() + latDiff, 
                          box.getMaxLon() + lonDiff );
   poiBox.increaseFactor( 0.2 );
   boxes.addBox( poiBox, projection );
}

}

namespace MapDrawingCommon {

bool isStreetFeature( const GfxFeature& feature ) {
    return 
       feature.getType() == GfxFeature::STREET_MAIN ||
       feature.getType() == GfxFeature::STREET_FIRST ||
       feature.getType() == GfxFeature::STREET_SECOND ||
       feature.getType() == GfxFeature::STREET_THIRD ||
       feature.getType() == GfxFeature::STREET_FOURTH ||
       feature.getType() == GfxFeature::WALKWAY;
}

MC2Point determineCentroid( const GfxFeature& feature,
                            const GfxData& featGfx,
                            const DrawingProjection& projection,
                            bool& toAdd ) {
   MC2Point centerPoint( 0, 0 );
   // Place text for a large area feature.
   // Place it at the centroid of the polygon.
   if ( MapUtility::isLargeAreaType( feature.getType() ) &&
        feature.getNbrPolygons() >= 1 &&
        feature.getPolygon( feature.getNbrPolygons() - 1 )->
        getNbrCoordinates() == 1 ) {
      // use the last polygon coordinate as center, 
      // this coordinate is calculated from the original
      // polygon in map module

      GfxPolygon* centerPoly = 
         feature.getPolygon( feature.getNbrPolygons() - 1 );
      MC2Coordinate centerCoord = centerPoly->getCoord( 0 );
      centerPoint = projection.getPoint( centerCoord );
      toAdd = true;

   } else {
      MC2Coordinate coord;
      toAdd = featGfx.getPolygonCentroid( 0, coord );
      centerPoint = MC2Point( coord.lon, coord.lat );
   }
   return centerPoint;
}

bool
mergeAndDrawStreetNames( GfxFeature::gfxFeatureType type,
                         uint32 scaleLevel, bool smallImage,
                         bool merge) {
   if (merge) {
      if ( (type == GfxFeature::STREET_FOURTH) &&
           (scaleLevel >= 10-uint(smallImage)) )
         // "detailed street"/"part of block" and more detailed
         return true;
      else if ( (type == GfxFeature::STREET_THIRD) &&
                (scaleLevel >= 8-uint(smallImage)) )
         // "block"/"district" and more detailed
         return true;
      else if ( ((type == GfxFeature::STREET_SECOND) ||
                 (type == GfxFeature::STREET_FIRST) ||
                 (type == GfxFeature::STREET_MAIN) ) &&
                (scaleLevel >= 7-uint(smallImage)) )
         // "district"/"small city" and more detailed
         return true;

      return false;

   } else {
      if ( (type == GfxFeature::STREET_FOURTH) &&
           (scaleLevel >= 9-uint(smallImage)) )
         // "part of block"/"block" and more detailed
         return true;
      else if ( (type == GfxFeature::STREET_THIRD) &&
                (scaleLevel >= 8-uint(smallImage)) )
         // "block"/"district" and more detailed
         return true;
      else if ( (type == GfxFeature::STREET_SECOND) &&
                (scaleLevel >= 7-uint(smallImage)) )
         // "district"/"small city" and more detailed
         return true;
      else if ( ((type == GfxFeature::STREET_FIRST) ||
                 (type == GfxFeature::STREET_MAIN) ) &&
                (scaleLevel >= 6-uint(smallImage)) )
         // "small city"/"city" and more detailed
         return true;

      return false;

   }

   return false;
}

bool roadSignName(const char* name) {
   // FIXME: More general check for Major roads
   if( (strlen( name ) <= 3)
       &&
       (StringUtility::onlyDigitsInString( name ) ||
        ((name[0] == 'E' || name[0] == 'M' || name[0] == 'A') &&
         StringUtility::onlyDigitsInString( name + 1 )) ) )
      {
         return true;
      } else if( (strlen( name ) <= 5) &&
                 (name[0] == 'I') && (name[1] == ' ') &&
                 StringUtility::onlyDigitsInString( name + 2 ) ) {
      return true;
   } else if( (strlen( name) <= 6) &&
              (name[0] == 'U') && (name[1] == 's') && (name[2] == ' ') &&
              StringUtility::onlyDigitsInString( name + 3 ) ) {
      return true;
   }
   return false;
}

/**
 * Vector of featurenotice-list iterators.
 */
typedef vector<list<featurenotice_t>::iterator> featItVector; 

/**
 * Map consisting of an int (poitype) as key and featItVector as
 * value.
 */
typedef map<int, featItVector> poiTypeMap;

/**
 * Pair consisting of an int (poitype) and a featItVector.
 */
typedef pair<int, featItVector> poiTypePair;

/**
 * Used for sorting poiTypePair based on the number of elements
 * in the featItVector.
 */
struct LessPOITypePair:
      public binary_function<const poiTypePair&, 
                             const poiTypePair&, 
                             bool> {
   bool operator()(const poiTypePair& x, const poiTypePair& y) {
      return (x.second.size() < y.second.size());
   }
};

/**
 * Help method to sortAndCreateFeatureNotices.
 * Makes sure that no more than maxNbrPOIs is included in 
 * poiNotices. Removes whole categories of poi:s if necessary.
 */
int filterPOINotices( const poiTypeMap& poiTypes,
                      list<featurenotice_t>& poiNotices,
                      int maxNbrPOIs ) {


   // Make sure not too many poi:s are added!!
   // Copy from map to vector so we can sort according to the number
   // of instances of poitypes.
   mc2dbg2 << "  Making sure not too many poi:s are displayed.."
           << endl;
   vector<poiTypePair> sortedPOITypes(poiTypes.size());
   copy(poiTypes.begin(), poiTypes.end(), sortedPOITypes.begin());
   // Sort
   sort(sortedPOITypes.begin(),
        sortedPOITypes.end(),
        LessPOITypePair());

   int sum = 0;
   for ( vector<poiTypePair>::iterator it = sortedPOITypes.begin();
         it != sortedPOITypes.end();
         it++ ) {
      mc2dbg2 << "[" << it->first << "] = " << it->second.size();

      if ( sum + int(it->second.size()) >= maxNbrPOIs ) {
         // Too many POIs. Remove this poi category.
         mc2dbg2 << " ... Removing POI category ... " << endl;

         for ( featItVector::iterator pIt = it->second.begin();
               pIt != it->second.end();
               pIt++ ) {
            poiNotices.erase(*pIt);
         }
      } else {
         sum+=it->second.size();
         mc2dbg2 << endl;
      }
   }
   return (sum);

}

/**
 * Notice consisting of a boundingbox and a featurenotice.
 */
struct poinotice_t {
   featurenotice_t m_featureNotice;
   MC2BoundingBox m_bbox;
};

void
sortAndCreateFeatureNotices( GfxFeatureMap* featureMap,
                             MapSettings* mapSettings,
                             vector<featurenotice_t> &notices,
                             bool singleDraw,
                             bool checkOverlappingObjects,
                             bool includeHiddenFeatures,
                             bool poiFiltering,
                             ObjectBoxes* objectBBoxes ) {
   ////////////////////////////////////////////////////////////////////////
   // Description of this method:
   //
   // The goal is to select which features to add to the GfxMap,
   // and make sure they are sorted in drawing order.
   // The features are represented as feature_notices and added to
   // the notices vector.
   //
   // In case singleDraw == true then all features are only added once.
   // Otherwise certain features may be added more than once, typically
   // streets who are added twice (first the outer street border then
   // the inner one).
   //
   // If specifed (checkOverlappingObjects == true) it is made sure that
   // certain objects are not overlapping each other. These objects can
   // typically be points of interests or traffic information symbols.
   // In order to avoid writing text on top of these at a later stage,
   // the boundingboxes of these are stored in the objectBBoxes vector.
   //
   // The following only applies when checkOverlappingObjects == true:
   //
   //    If poiFiltering == true then it is made sure that not too many poi
   //    are shown in the map. A threshold of how many poi:s that may
   //    be present is determined by taking into consideration the
   //    resolution of the screen and the average size of a poi.
   //    Then whole categories of poi:s are added until that threshold
   //    is reached. The categories (ie. resturants or hotels) containing
   //    the most number of poi:s are removed until the threshold is
   //    reached.
   //    If poiFiltering == false then simply all poi:s are included.
   //
   //    Let's say several objects (poi or traffic infos) are overlapping
   //    each other. In case the GfxMap only will be used to draw
   //    an image, then it is only important to include the one of the
   //    overlapping objects then set includeHiddenFeatures to false.
   //    However if we want to enable clicking on a object and get info
   //    about all objects that are covered by this, then set
   //    includeHiddenFeatures to true.
   //
   //    Example of the way aggregation of object is used (note that it
   //    will become the polygons that are sorted in this way):
   //
   //    <Restaurant MCDonalds (visible)>
   //    <Restaurant Pizza Hut (hidden)>
   //    <Postoffice (hidden)>
   //    <TrafficInfo - Accident at highway 16 (visible)>
   //    <Statoil (hidden)>
   //
   //    In this example the MCDonalds poi is visible and it covers
   //    a Pizza Hut poi and a postoffice (who are aggregated into
   //    the visible poi). Following that is a Traffic info (visible) which
   //    is overlapping a Statoil poi.
   //
   ////////////////////////////////////////////////////////////////////////
   uint32 startTime = TimeUtility::getCurrentTime();
   // If we wan't to remove overlapping objects, then objectBBoxes
   // mustn't be NULL
   MC2_ASSERT( !(checkOverlappingObjects && objectBBoxes == NULL) );

   // Height in pixels of a generic object.
   int32 POIHeight = 8;
   // Width in pixels of a generic object.
   int32 POIWidth = 8;
   // ... in mc2 pixels instead
   int32 POIHeightMC2 =
      mapSettings->getDrawingProjection()->getLatDiff( POIHeight );
   // ... in mc2 pixels instead
   int32 POIWidthMC2 =
      mapSettings->getDrawingProjection()->getLonDiff( POIWidth );

   // Temporary list containing poi notices.
   typedef list<featurenotice_t> POINotices;
   POINotices poiNotices;

   // Keeps track of where the different poi-types are located
   // in poiNotices.
   poiTypeMap poiTypes;

   for ( uint32 i = 0 ; i < featureMap->getNbrFeatures() ; i++ ) {
      const GfxFeature* curFeature = featureMap->getFeature( i );
      for ( uint32 p = 0 ; p < curFeature->getNbrPolygons() ; p++ ) {
         featurenotice_t notice;
         notice.m_visible = true;
         notice.m_drawOrder = MapUtility::getDrawOrder( curFeature, p );
         notice.m_border = true;
         notice.m_feature = curFeature;
         notice.m_polygonIndex = p;
         notice.m_featureIndex = i;
         notice.m_setting = mapSettings ? mapSettings->getSettingFor(
            curFeature->getType(), curFeature->getScaleLevel() ) : NULL;

         if ( checkOverlappingObjects ) {
            if ( curFeature->getType() == GfxFeature::POI ) {
               notice.m_poiStatus = DrawSettings::singlePOI;
               if ( poiFiltering ) {
                  // Make sure not too many pois are shown
                  // Store some extra info in order to do this.
                  const GfxPOIFeature* poi =
                     static_cast<const GfxPOIFeature*> (curFeature);
                  if( poi->getPOIType() != ItemTypes::cityCentre ) {
                     // Store iterators for a certain poi-type
                     poiTypes[ poi->getPOIType() ].push_back(
                        poiNotices.insert(poiNotices.end(), notice));
                  } else {
                     poiNotices.push_back( notice );
                  }
               } else {
                  // Simply add the poi.
                  poiNotices.push_back( notice );
               }
            } else if ( curFeature->getType() == GfxFeature::TRAFFIC_INFO ) {
               // No filtering of traffic infos.
               poiNotices.push_back( notice );
            } else {
               notices.push_back( notice );
            }

         } else {
            notices.push_back( notice );
         }

         if ( singleDraw && 
              MapDrawingCommon::isStreetFeature( *curFeature ) ) {
            // It is a road, add it twice
            featurenotice_t notice;
            notice.m_visible = true;
            notice.m_drawOrder =
               MapUtility::getDrawOrder( curFeature, p, false );
            notice.m_border = false;
            notice.m_feature = curFeature;
            notice.m_polygonIndex = p;
            notice.m_featureIndex = i;
            notices.push_back(notice);
            DEBUG8(curFeature->getPolygon(p)->dump( 1 ));
         }
      }
   }


   if ( checkOverlappingObjects ) {
      // Do we want to make sure that too many poi:s are not added?
      
      if (poiFiltering) {

         float64 poiFactor = 0.020;
         int maxNbrPOIs =
            int(  poiFactor *
                  featureMap->getScreenX() *
                  featureMap->getScreenY() /
               float64(POIWidth * POIHeight));
         mc2dbg4 << "  Max nbr pois allowed is " << maxNbrPOIs << endl;

         int sum = filterPOINotices(poiTypes,
                                    poiNotices,
                                    maxNbrPOIs);
         mc2dbg2 << "  Total nbr of poi:s " << sum << endl;
      }
      

      typedef vector<MC2BoundingBox>::const_iterator bboxIt;
      typedef list<poinotice_t>::iterator poiIt;
      list<poinotice_t> tmpPOINotices;
      int32 lat = MAX_UINT32, lon = MAX_UINT32;

      // Sort poiNotices according to drawing order.
      // By doing this we make sure that the most important symbol
      // will be visible in case of aggregation of other symbols.
      poiNotices.sort( LessFeatureNoticeOrder() );
      for ( list<featurenotice_t>::const_iterator it = poiNotices.begin();
            it != poiNotices.end();
            ++it ) {
         // Make sure the poi:s do not overlap each other.
         lat = it->m_feature->getPolygon(0)->getLat( 0 );
         lon = it->m_feature->getPolygon(0)->getLon( 0 );

         // Create a bbox of the POI. Approximate width and height.
         MC2BoundingBox poiBBox(lat + POIHeightMC2,
                                lon - POIWidthMC2,
                                lat - POIHeightMC2,
                                lon + POIWidthMC2 );

         // Make sure this poi won't overlap any other poi.
         poiIt pIt = tmpPOINotices.begin();
         bool overlaps = false;
         while ( pIt != tmpPOINotices.end() && ! overlaps ) {
            if ( pIt->m_featureNotice.m_visible &&
                 pIt->m_bbox.overlaps(poiBBox) ) {
               overlaps = true;
            } else {
               ++pIt;
            }
         }

         poinotice_t poiNotice;
         poiNotice.m_featureNotice = *it;
         if ( !overlaps ) {
            // The POI doesn't overlap.
            poiNotice.m_bbox = poiBBox;
            // Add last.
            tmpPOINotices.push_back(poiNotice);
         } else {
            // The poi overlaps another poi.
            poiNotice.m_featureNotice.m_visible = false;
            if ( ( pIt->m_featureNotice.m_poiStatus ==
                   DrawSettings::multiSamePOI ) ||
                 ( pIt->m_featureNotice.m_poiStatus ==
                   DrawSettings::singlePOI ) ) {
               if (pIt->m_featureNotice.m_feature
                     ->equalsType(it->m_feature)) {
                  // Same poi-types.
                  pIt->m_featureNotice.m_poiStatus =
                     DrawSettings::multiSamePOI;
               } else {
                  // Different poi-types
                  pIt->m_featureNotice.m_poiStatus =
                     DrawSettings::multiDifferentPOI;
               }
            }
            // Add after pIt
            tmpPOINotices.insert( ++pIt, poiNotice );
         }
      }

      // Go through everything in tmpPOINotices and
      // add to notices.
      for (poiIt pIt = tmpPOINotices.begin();
           pIt != tmpPOINotices.end();
           pIt++) {
         const GfxFeature* feature = pIt->m_featureNotice.m_feature;
         const GfxPOIFeature* poi = static_cast<const GfxPOIFeature*>(feature);
         mc2dbg4 << "added feature "
                 << pIt->m_featureNotice.m_feature->getName()
                 << " and visibility = " << pIt->m_featureNotice.m_visible
                 << endl;
         // Add all if hidden features should be included, otherwise only
         // the visible ones.
         if (includeHiddenFeatures || pIt->m_featureNotice.m_visible) {
            notices.push_back(pIt->m_featureNotice);
            if( poi->getPOIType() != ItemTypes::cityCentre ) {
               ::addPOIBox( *objectBBoxes, pIt->m_bbox,
                            *mapSettings->getDrawingProjection() );
            }
         }
      }
   }

   // Sorth the features according to drawing order.
   // Done last so overlap test can add without thinking about order
   // (See bug #775  and check the statoil + pin url in the note.)
   sort( notices.begin(), notices.end(), LessFeatureNoticeOrder() );

   mc2dbg4 << "  createAndSortFeatureNotices() took "
           << TimeUtility::getCurrentTime() - startTime << " ms." << endl;
}

} // MapDrawingCommon
