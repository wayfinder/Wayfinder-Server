/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapTextPlacement.h"

#include "MapSettings.h"
#include "GfxFeature.h"
#include "GfxData.h"
#include "GfxPolygon.h"
#include "ImageDraw.h"
#include "DrawingProjection.h"
#include "GfxDataFull.h"
#include "MC2Point.h"
#include "GfxRoadPolygon.h"
#include "GfxFeatureMap.h"

#include "StringUtility.h"
#include "MapFontSettings.h"
#include "ScopedArray.h"
#include "MapDrawingCommon.h"
#include "AbbreviationTable.h"
#include "TimeUtility.h"
#include "MapUtility.h"
#include "DebugClock.h"
#include "PixelBox.h"
#include "StreetTextPlacer.h"

#include "ObjectBoxes.h"

#include "Math.h"

#include <set>

namespace {

template <typename T>
T max4( T a, T b, T c, T d ) {
   return max( a, max( b, max( c, d ) ) );
}

template <typename T>
T min4( T a, T b, T c, T d ) {
   return min( a, min( b, min( c, d ) ) );
}

}

namespace MapTextPlacement {

struct ImportanceTextFeatureNoticeOrder:
   public binary_function<const featuretextnotice_t&, 
                          const featuretextnotice_t&, bool> {
   bool operator()(const featuretextnotice_t& x, 
                   const featuretextnotice_t& y) {
            
      // if both textnotices are builtup areas, 
      // sort according to scaleLevel (and length)
      if ( x.m_textImportanceLevel == BUILTUP_AREA &&
           y.m_textImportanceLevel == BUILTUP_AREA ) {
         if ( x.m_scaleLevel == y.m_scaleLevel ) {
            return (x.m_length > y.m_length);
         }

         return x.m_scaleLevel < y.m_scaleLevel;

      } else if ( x.m_textImportanceLevel == CITY_CENTRE &&
                  y.m_textImportanceLevel == CITY_CENTRE ) {
         // If both are city centres, sort after length     
         const GfxPOIFeature* xPoi = 
            static_cast<const GfxPOIFeature*>(x.m_feature);
         const GfxPOIFeature* yPoi = 
            static_cast<const GfxPOIFeature*>(y.m_feature);

         if( xPoi->getExtraInfo() != yPoi->getExtraInfo() ) {
            return xPoi->getExtraInfo() < yPoi->getExtraInfo();
         }
      }
      // Sort STREETS according to length to get good 
      // name/roadsigns locations
      else if ( ((x.m_textImportanceLevel == STREET_MAIN) &&
                 (y.m_textImportanceLevel == STREET_MAIN)) ||
                ((x.m_textImportanceLevel == STREET_FIRST) && 
                 (y.m_textImportanceLevel == STREET_FIRST)) ||
                ((x.m_textImportanceLevel == STREET_SECOND) && 
                 (y.m_textImportanceLevel == STREET_SECOND)) ||
                ((x.m_textImportanceLevel == STREET_THIRD) && 
                 (y.m_textImportanceLevel == STREET_THIRD)) ||
                ((x.m_textImportanceLevel == STREET_FOURTH) && 
                 (y.m_textImportanceLevel == STREET_FOURTH)) ) {
         return (x.m_length > y.m_length);
      } 

      if ( x.m_textImportanceLevel == y.m_textImportanceLevel ) {
         return strcmp( x.m_feature->getName(),
                        y.m_feature->getName() ) < 0;
      }
             

      return x.m_textImportanceLevel < y.m_textImportanceLevel; 

   }
};
// used for sorting notices when merging street text features
struct NameTextFeatureNoticeOrder:
      public binary_function<const featuretextnotice_t&, 
                             const featuretextnotice_t&, bool> {
   bool operator()(const featuretextnotice_t& x, 
                   const featuretextnotice_t& y) {

      if (x.m_textImportanceLevel == y.m_textImportanceLevel) {
         const char* xname = x.m_feature->getName();
         const char* yname = y.m_feature->getName();
               
         if (strlen(xname) == 0)
            return false;
         else 
            if (strlen(yname) == 0)
               return true;
            else
               return ( strcasecmp(xname, yname) < 0);
      } else {
         return (x.m_textImportanceLevel <
                 y.m_textImportanceLevel );
      }
   }
};



namespace Collision {

bool containsWithin( const MC2BoundingBox& lhs,
                     const MC2BoundingBox& rhs ) {
#define LESS_THAN( a,b ) ( ( ( (a) - (b) ) < 0 ) )
#define IN_RANGE(a,b,c) ( LESS_THAN(a,b) && LESS_THAN(b,c) )

   return (rhs.getMaxLat() > lhs.getMinLat()) &&
      (lhs.getMaxLat() > rhs.getMinLat() ) &&
      ( IN_RANGE( lhs.getMinLon(), rhs.getMinLon(), lhs.getMaxLon() ) ||
        IN_RANGE( lhs.getMinLon(), rhs.getMaxLon(), lhs.getMaxLon() ) ||
        IN_RANGE( rhs.getMinLon(), lhs.getMinLon(), rhs.getMaxLon() ) ||
        IN_RANGE( rhs.getMinLon(), lhs.getMaxLon(), rhs.getMaxLon() ) );
#undef IN_RANGE
#undef LESS_THAN

}


/**
 * Do collision test with "currTextBBox" against already added collision boxes.
 *
 * @param currTextBBox the box to test collision against the others
 * @param gfxTextArray a set of text bounding boxes as gfx data to test against
 *                     "currTextBBox"
 * @param objectBBoxes bounding boxes from other types of objects to test
 *                     against "currTextBBox"
 * @return true if there was a collision
 */
inline bool 
collisionTest( MC2BoundingBox currTextBBox,
               vector<GfxData*>& gfxTextArray,
               MapDrawingCommon::ObjectBoxes& objectBBoxes,
               const DrawingProjection& proj ) {

   PixelBox pixelBox = MapDrawingCommon::createPixelBox( currTextBBox, proj );

   // Check if the text boundary crosses any
   // points of interests or any streets
   for ( MapDrawingCommon::ObjectBoxes::SizeType boxIndex = 0;
         boxIndex < objectBBoxes.getNbrBoxes();
         ++boxIndex ) {
      if ( containsWithin( objectBBoxes.getPixelBox( boxIndex ), pixelBox ) ) {
         return true;
      }
   }

   // Check if the text boundary crosses any previous
   // text boundary
                  
   for ( uint32  i = 0; i < gfxTextArray.size(); ++i ) {
      //First check if the boundingboxes overlap.
      const GfxData* existingTextGfx = gfxTextArray[ i ];
      if ( existingTextGfx != NULL ) {
         MC2BoundingBox existingTextBBox;
         existingTextGfx->getMC2BoundingBox( existingTextBBox );
         if ( containsWithin( currTextBBox, existingTextBBox ) ) {
            return true;
         }
      }
   }
   // no collision
   return false;

}
} // Collision


/**
 *    Help function for mergeStreetFeatures.
 *    @param streetnotices    The selected street notices.
 *    @param mergedStreets    List with notices that will be merged.
 *    @param processedStreets Set with ids of notices that
 *                            has been processed.
 *    @param firstStreetWithCurName The first notice that has the
 *                            same name as the notice to process.
 *    @param curStreet        The notice to process.
 *    @param curNodeNbr       Which node of the stret notice to process.
 *    @param forward          Tells if merging in the beginning (false)
 *                            or end (true) of the already merged notices.
 *    @param startNode        Outparam, which is the first node of
 *                            the first notice among the ones to merge.
 */
void mergeStreetFeature( vector<featuretextnotice_t>& streetnotices,
                         list<featureTextNoticeIt>& mergedStreets,
                         set<uint32>& processedStreets,
                         featureTextNoticeIt firstStreetWithCurName,
                         featureTextNoticeIt curStreet,
                         byte curNodeNbr,
                         bool forward,
                         byte& startNode);

/**
 *    Merges selected street feature text notices so that 
 *    more names can be drawn in the image.
 *    The merged notices are then inserted into the notices vector.
 */
bool
mergeStreetFeatures( vector<featuretextnotice_t>& streetnotices,
                     vector<featuretextnotice_t>& notices );
 
void
mergeStreetFeature( vector<featuretextnotice_t>& streetnotices,
                    list<featureTextNoticeIt>& mergedStreets,
                    set<uint32>& processedStreets,
                    featureTextNoticeIt firstStreetWithCurName,
                    featureTextNoticeIt curStreet,
                    byte curNodeNbr,
                    bool forward,
                    byte& startNode) {

   //The number of other street features that share curNodeNbr with curStreet
   uint32 nbrNeighbourStreets = 0;
   featureTextNoticeIt nextStreet;
   uint32 sharedNextNode = 0;

   // Find the street features that share node with curStreet
   featureTextNoticeIt f = firstStreetWithCurName;
   //featureTextNoticeIt f = curStreet;
   const char* curName = curStreet->m_feature->getName();
   uint32 curCoord = 0;
   if (curNodeNbr == 1)
      curCoord = curStreet->m_gfxData->getNbrCoordinates(0) - 1;
   int32 curLat = curStreet->m_gfxData->getLat(0,curCoord);
   int32 curLon = curStreet->m_gfxData->getLon(0,curCoord);
   mc2dbg4 << curStreet->m_id << " " << curStreet->m_feature->getName()
           << " node " << int(curNodeNbr)
           << " (" << curLat << "," << curLon << ")" << endl;
   while ((f < streetnotices.end()) &&
          (nbrNeighbourStreets < 2) &&
          (strcasecmp(curName, f->m_feature->getName()) == 0)) {
      if (curStreet->m_id != f->m_id) {
         // do not compare with curStreet.
         // FIXME: check also that f is not yet processed ?
         mc2dbg4 << "checking " << f->m_id << " "
                 << f->m_feature->getName() << endl;
         for (uint32 node = 0; node < 2; node++) {
            uint32 coord = 0;
            if (node == 1)
               coord = f->m_gfxData->getNbrCoordinates(0) - 1;
            int32 lat = f->m_gfxData->getLat(0,coord);
            int32 lon = f->m_gfxData->getLon(0,coord);
            mc2dbg8 << " n=" << node << " c=" << coord
                 << " (" << lat << "," << lon << ")" << endl;
            if (curLat == f->m_gfxData->getLat(0,coord) &&
                curLon == f->m_gfxData->getLon(0,coord)) {
               mc2dbg4 << " shared coord in node " << node << endl;
               nbrNeighbourStreets++;
               nextStreet = f;
               sharedNextNode = node;
            }
         }
      }
      f++;
   }

   // If there was only one feature sharing the node with curStreet,
   // check that they should be merged, i.e. nextStreet not already
   // processed and same roadClass(=featuretype=textImportance) && scaleLevel
   // TODO: and not too small angle
   // TODO: and not too long ?(poi symbols in the middle of streets)
   if ( (nbrNeighbourStreets == 1) &&
        (processedStreets.find(nextStreet->m_id) ==
         processedStreets.end()) ) {

      // type=textImp already taken care of in sorting...
      bool toMerge = ( (curStreet->m_feature->getType() ==
                        nextStreet->m_feature->getType()) &&
                       (curStreet->m_feature->getScaleLevel() ==
                        nextStreet->m_feature->getScaleLevel()) );
      if (toMerge) {
         // OK - add to list
         mc2dbg4 << " to merge" << endl;
         if (forward) {
            mergedStreets.push_back(nextStreet);
         } else {
            mergedStreets.push_front(nextStreet);
         }
         // Add as processed
         processedStreets.insert(nextStreet->m_id);
         byte nextNodeNbr = 0;
         if (sharedNextNode == 0)
            nextNodeNbr = 1;

         if (!forward)
            startNode = byte(nextNodeNbr);

         // Continue merging
         mergeStreetFeature(streetnotices, mergedStreets, processedStreets,
                            firstStreetWithCurName, nextStreet,
                            nextNodeNbr, forward, startNode);
      }
   }

   DEBUG4(
   mc2dbg4 << " For " << curStreet->m_id << " node=" << int(curNodeNbr)
           << " nbrNeighbours=" << nbrNeighbourStreets;
   if (nbrNeighbourStreets > 0)
      mc2dbg4 << " 1st=" << nextStreet->m_id;
   mc2dbg4 << endl;);

}


/**
 *    Merges selected street feature text notices so that 
 *    more names can be drawn in the image.
 *    The merged notices are then inserted into the notices vector.
 */
bool
mergeStreetFeatures( vector<featuretextnotice_t>& streetnotices,
                     vector<featuretextnotice_t>& notices ) {
   uint32 startTime = TimeUtility::getCurrentTime();
   uint32 mergeTime;
   uint32 totalMergeTime = 0;

   // Sort the street notices according to textImportanceLevel and name
   sort( streetnotices.begin(), streetnotices.end(),
         NameTextFeatureNoticeOrder() );
   mergeTime = TimeUtility::getCurrentTime() - startTime;
   totalMergeTime += mergeTime;
   mc2dbg2 << "mergeStreetFeatures sorting name took "
           << mergeTime << " ms" << endl;

   // set ids and build gfxData
   startTime = TimeUtility::getCurrentTime();
   uint32 nbrFeatures=0;
   for ( featureTextNoticeIt f = streetnotices.begin();
         f < streetnotices.end(); f++) {
      f->m_id = nbrFeatures++;
      GfxDataFull* gfx = GfxData::createNewGfxData( NULL, true );
      if (f->m_feature->getNbrPolygons() != 1) {
         mc2dbg4 << warn << here << " Street feature " << f->m_id
                 << " " << f->m_feature->getName()
                 << " has " << f->m_feature->getNbrPolygons()
                 << " polygons" << endl;
      }
      // Add polygons of the street feature to the gfxData, add p 1+
      // only if they fit into the prior polygon (same node)
      uint32 p = 0;
      bool addNextCoord = true;
      while ( addNextCoord && (p < f->m_feature->getNbrPolygons()) ) {
         GfxPolygon* poly = f->m_feature->getPolygon(p);
         for (uint32 c=0; c < poly->getNbrCoordinates(); c++) {
            if (p == 0) {
               gfx->addCoordinate(poly->getLat(c), poly->getLon(c));
            } else {
               if (c == 0) {
                  int32 lat = poly->getLat(c);
                  int32 lon = poly->getLon(c);
                  if (gfx->getLat(0,gfx->getNbrCoordinates(0)-1) != lat ||
                      gfx->getLon(0,gfx->getNbrCoordinates(0)-1) != lon)
                     addNextCoord = false;
               } else if (addNextCoord) {
                  gfx->addCoordinate(poly->getLat(c), poly->getLon(c));
               }
            }
         }
         p++;
      }
      gfx->removeIdenticalCoordinates();
      f->m_gfxData = boost::shared_ptr<GfxData>( gfx );
   }
   mc2dbg8 << " Added id and gfxData to " << nbrFeatures
           << " street features" << endl;
   mergeTime = TimeUtility::getCurrentTime() - startTime;
   totalMergeTime += mergeTime;
   mc2dbg2 << "mergeStreetFeatures set ids & gfxData took "
           << mergeTime << " ms" << endl;

   startTime = TimeUtility::getCurrentTime();
   // Set containing the processed street features
   set<uint32> processedStreets;
   // List with street features that will be merged
   typedef list<featureTextNoticeIt> mergedStreets_t;
   vector<mergedStreets_t> allMergedStreets(nbrFeatures);
   // Map with info about which is the startnode of the first feature
   // in each list.
   map<uint32,byte> startNodeInEachList;

   const char* prevName = NULL;
   textImportanceLevel prevImportance = OTHER;
   // FIXME: With what should firstStreetWithCurName inited?
   featureTextNoticeIt firstStreetWithCurName = streetnotices.begin();
   for ( featureTextNoticeIt f = streetnotices.begin();
         f < streetnotices.end(); f++) {
      if (processedStreets.find(f->m_id) == processedStreets.end()) {
         // The street feature is not yet processed

         mergedStreets_t mergedStreets;

         mergedStreets.push_back( f );
         processedStreets.insert( f->m_id );

         // find first notice for each name and importance...
         if ((prevName == NULL) ||
             (prevImportance != f->m_textImportanceLevel) ||
             (strcasecmp(prevName, f->m_feature->getName()) != 0)) {
            mc2dbg8 << " new firstStreetWithCurName, prevN=" << prevName
                    << " curN=" << f->m_feature->getName()
                    << ", prevImp=" << prevImportance << " curImp="
                    << f->m_textImportanceLevel << endl;
            firstStreetWithCurName = f;
            prevName = f->m_feature->getName();
            prevImportance = static_cast<textImportanceLevel>
               ( f->m_textImportanceLevel );
         }

         // Merge
         mc2dbg4 << "--- street to merge (node 0) " << f->m_id << endl;
         byte startNode = 0;
         mergeStreetFeature(streetnotices,
                            mergedStreets, processedStreets,
                            firstStreetWithCurName, f,
                            0, false,  // node0, forward=false
                            startNode);
         mergeStreetFeature(streetnotices,
                            mergedStreets, processedStreets,
                            firstStreetWithCurName, f,
                            1, true,   // node1, forward=true
                            startNode);

         allMergedStreets.push_back( mergedStreets );
         startNodeInEachList.insert(
               pair<uint32,byte>(mergedStreets.front()->m_id, startNode));
         mc2dbg4 << " pair(" << mergedStreets.front()->m_id << ","
                 << int(startNode) << ")" << endl;
      }
   }
   mergeTime = TimeUtility::getCurrentTime() - startTime;
   totalMergeTime += mergeTime;
   mc2dbg1 << "mergeStreetFeatures merging preparation took "
           << mergeTime << " ms" << endl;

   mc2dbg4 << "--- Ready to do actual merging ---" << endl;

   // Merge features
   startTime = TimeUtility::getCurrentTime();
   for ( vector<mergedStreets_t>::iterator it = allMergedStreets.begin();
         it != allMergedStreets.end(); ++it ) {
      DEBUG4(
      if (it->size() > 0) {
         mc2dbg4 << "to merge " << it->size() << " streets: ";
         for (list<featureTextNoticeIt>::iterator jt = it->begin();
              jt != it->end(); ++jt )
            mc2dbg4 << (*jt)->m_id << " ";
         mc2dbg4 << it->front()->m_feature->getName() << endl;
      });

      if (it->size() > 0) {

         // create a new notice and copy members from
         // the first notice in the list.
         list<featureTextNoticeIt>::iterator jt = it->begin();
         featuretextnotice_t newNotice;
         featuretextnotice_t curNotice = **jt;
         newNotice.m_scaleLevel = curNotice.m_scaleLevel;
         newNotice.m_textImportanceLevel = curNotice.m_textImportanceLevel;
         newNotice.m_feature = curNotice.m_feature; // type & name are used
         newNotice.m_setting = curNotice.m_setting;
         newNotice.m_id = curNotice.m_id; // not used anymore...
         // m_length calc below, m_curvedtextOut not applicable now

         GfxDataFull* gfx = GfxData::createNewGfxData( NULL, true );

         if (it->size() == 1) {
            // move coords from the street notice's gfxdata
            gfx->add( curNotice.m_gfxData.get() );

         } else if (it->size() > 1) {
            // merge all street notices' gfxdatas into the new gfx
            map<uint32, byte>::iterator sN =
                     startNodeInEachList.find(curNotice.m_id);
            if (sN == startNodeInEachList.end()) {
               mc2log << warn << "could not find startNode for street "
                      << curNotice.m_id << endl;
            } else {
               byte startNode = (*sN).second;
               int32 lastLat, lastLon;
               if (startNode == 0) {
                  // add the notice gfxdata forward
                  gfx->add( curNotice.m_gfxData.get() );
               } else {
                  // add the notice gfxdata backwards
                  gfx->add( curNotice.m_gfxData.get(), true );
               }
               lastLat = gfx->getLat(0, gfx->getNbrCoordinates(0)-1);
               lastLon = gfx->getLon(0, gfx->getNbrCoordinates(0)-1);

               // add the rest of the gfxdatas
               jt++;

               while (jt != it->end()) {

                  curNotice = **jt;
                  if ( (lastLat == curNotice.m_gfxData->getLat(0,0) &&
                        lastLon == curNotice.m_gfxData->getLon(0,0)) ) {
                     // add the notice gfxdata forward
                     gfx->add( curNotice.m_gfxData.get() );
                  } else {
                     // add the notice gfxdata backwards
                     gfx->add( curNotice.m_gfxData.get(), true );
                  }
                  lastLat = gfx->getLat(0, gfx->getNbrCoordinates(0)-1);
                  lastLon = gfx->getLon(0, gfx->getNbrCoordinates(0)-1);

                  jt++;
               }
            }
         }
         // calculate length of new notice
         gfx->removeIdenticalCoordinates();
         newNotice.m_length = gfx->getLength(0);
         newNotice.m_gfxData = boost::shared_ptr<GfxData>( gfx );

         // insert into notices vector
         notices.push_back(newNotice);
         mc2dbg4 << "inserting new street notice, "
                 << newNotice.m_feature->getName()
                 << " ftype=" << newNotice.m_feature->getType()
                 << " length=" << newNotice.m_length << endl;
      }
   }
   mergeTime = TimeUtility::getCurrentTime() - startTime;
   totalMergeTime += mergeTime;
   mc2dbg1 << "mergeStreetFeatures merging took "
           << mergeTime << " ms" << endl;

   mc2dbg1 << "mergeStreetFeatures total took "
           << totalMergeTime << " ms" << endl;
   return true;
}

inline 
void drawBUA( const GfxFeature* feature, 
              const featureTextNoticeIt& f, 
              ImageDraw* image,
              MapSettings* mapSettings ) {

   // Don't know if this is correct, but seems
   // better that not initializing Buasettings at all
   auto_ptr<DrawSettings> BUAsettings( new DrawSettings() );
   const MapSetting* mapSettingBUA = f->m_setting;
   BUAsettings->reset();
   if ( feature->getType() == GfxFeature::BUILTUP_AREA_SQUARE) {
      if ( mapSettingBUA != NULL ) {
         // Found setting use it
         BUAsettings->m_style = mapSettingBUA->m_drawStyle;
         BUAsettings->m_symbol = mapSettingBUA->m_drawSymbol;
         BUAsettings->m_color = mapSettingBUA->m_drawColor;
         BUAsettings->m_lineWidth = mapSettingBUA->m_lineWidth;
      } else{
         BUAsettings->m_style = DrawSettings::SYMBOL;
         BUAsettings->m_symbol = DrawSettings::SQUARE_3D_SYMBOL;
      }
   } else {
      if ( mapSettingBUA != NULL ) {
         // Found setting use it
         BUAsettings->m_style = mapSettingBUA->m_drawStyle;
         BUAsettings->m_symbol = mapSettingBUA->m_drawSymbol;
         BUAsettings->m_color = mapSettingBUA->m_drawColor;
         BUAsettings->m_lineWidth = mapSettingBUA->m_lineWidth;
      } else{
         BUAsettings->m_style = DrawSettings::SYMBOL;
         BUAsettings->m_symbol = DrawSettings::SMALL_CITY_SYMBOL;
      }
   }

   if ( !image->drawGfxFeaturePolygon( feature, feature->getPolygon(0),
                                       BUAsettings.get(),
                                       mapSettings->getImageSet() ) ) {
      mc2dbg << "MapDrawer::drawGfxFeatureMap"
         " feature not drawn for "<< feature->getName()
             << endl << endl;
   }
}


featuretextnotice_t::~featuretextnotice_t() {
}

/**
 * Find a good place to place textX and textY in a water area.
 * @param featGfx the water feature to try to position textX and textY in.
 * @param textX X.
 * @param textY Y.
 * @param tryNbr current try number, will be increased.
 */
void placeWater( const GfxDataFull* featGfx,
                 int32& textX, int32& textY,
                 int& tryNbr ) {
   int tol = 500;
   // check if we have a river
   if( !( SQUARE(featGfx->getLength(0))*
          GfxConstants::SQUARE_METER_TO_SQUARE_MC2SCALE/
          fabs(featGfx->polygonArea(0)) > tol ) ){
      return;
   }

   uint16 closestPoly = 0;
   uint32 closestI = MAX_UINT32;
   float64 closestT = MAX_UINT32;

   // find the point on featGfx closest to (textX, textY)
   // closestPoly will always be 0 since wo only consider that
   uint64 sqDistToBoundary = featGfx->
      squareDistWithOffsetToLine_mc2( textY, textX,
                                      closestPoly,
                                      closestI,
                                      closestT,
                                      0 );
   sqDistToBoundary = sqDistToBoundary;
   // FIXME: Will only use the first polyon...
   uint32 n = featGfx->getNbrCoordinates(0);
   int32 x1 = featGfx->getLon(closestPoly,closestI);
   int32 y1 = featGfx->getLat(closestPoly, closestI);
   int32 x2 = 0, y2 = 0;
   int32 boundaryLat, boundaryLon;
   if (closestT > 0) {
      // The closest point is between node 'i' and node '(i+1)%nbrCoordinates'
      x2 = featGfx->getLon(0, (closestI+1)%n);
      y2 = featGfx->getLat(0, (closestI+1)%n);
      GfxUtility::
         getPointOnLine(y1,x1,
                        y2,x2,
                        ::sqrt(GfxUtility::
                               squareP2Pdistance_linear( y1, x1, 
                                                         y2, x2 ) )*closestT,
                        boundaryLat, boundaryLon );
   } else{
      boundaryLon = x1;
      boundaryLat = y1;
   }
   int32 y3,x3,y4,x4,y5,x5;
   int32 maxLat = featGfx->getMaxLat() + 1;
   int32 minLat = featGfx->getMinLat() - 1;
   // check if centroid is inside the polygon
   if(featGfx->insidePolygon(textY, textX, 0) == 2){
      y4 = boundaryLat;
      x4 = boundaryLon;
      y5 = textY;
      x5 = textX;
   } else {
      y4 = textY;
      x4 = textX;
      y5 = boundaryLat;
      x5 = boundaryLon;
   }
   int32 diffLon = x5 - x4;
   int32 diffLat = y5 - y4;
   if (diffLon == 0){
      x3 = x4;
      if (diffLat > 0)
         y3 = maxLat;
      else
         y3 = minLat;
   } else if (diffLat == 0){
      y3 = y4;
      int32 maxLon = featGfx->getMaxLon() + 1;
      int32 minLon = featGfx->getMinLon() - 1;
      if (diffLon > 0)
         x3 = maxLon;
      else
         x3 = minLon;
   } else {
      int32 maxLon = featGfx->getMaxLon() + 1;
      int32 minLon = featGfx->getMinLon() - 1;
      if (diffLat > 0)
         y3 = maxLat;
      else
         y3 = minLat;
      x3 = int32(rint(int64(diffLon)*(y3-y4)/(double(diffLat)))) + x4;
      if (!((x3 - minLon > 0)&&((x3 - maxLon < 0)))){
         if (diffLon > 0)
            x3 = maxLon;
         else
            x3 = minLon;
         y3 = int32(rint(int64(diffLat)*(x3-x4)/(double(diffLon)))) + y4;
      }
   }
   //clip with polygon
   float64 minSqDist = MAX_FLOAT64;
   float64 sqDistance = MAX_FLOAT64;
   uint32 coord = (closestI+1)%n;
   int32 intersectX, intersectY;
   int32 bestIntersectX = 0;
   int32 bestIntersectY = 0;
   x1 = featGfx->getLon(0,coord);
   y1 = featGfx->getLat(0,coord);
   x2 = featGfx->getLon(0,(coord+1)%n);
   y2 = featGfx->getLat(0,(coord+1)%n);
   // FIXME: What happens to bestIntersectY and X
   // if the loop isn't entered?
   for (uint32 index = 1; index < n - 1 ; index++){
      if (GfxUtility::getIntersection(x1, y1, x2, y2,
                                      boundaryLon, boundaryLat,
                                      x3, y3,
                                      intersectX, intersectY)){
         sqDistance =
            GfxUtility::
            squareP2Pdistance_linear( boundaryLat, boundaryLon,
                                      intersectY, intersectX,
                                      1 );
         if (sqDistance < minSqDist){
            minSqDist = sqDistance;
            bestIntersectX = intersectX;
            bestIntersectY = intersectY;
         }
      }
      x1 = x2;
      y1 = y2;
      coord = (coord + 1)%n;
      x2 = featGfx->getLon(0,(coord+1)%n);
      y2 = featGfx->getLat(0,(coord+1)%n);
   }
   textX = int32(double(boundaryLon + bestIntersectX)/2);
   textY = int32(double(boundaryLat + bestIntersectY)/2);
   tryNbr = 5;
}

bool
getTextLocation( GfxFeature* feature,
                 const vector<featuretextnotice_t>& sortednotices,
                 const vector<featuretextnotice_t>::const_iterator
                 currentNoticeIt,
                 GfxDataFull*& textGfx,
                 uint32 scaleLevel,
                 uint32 screenX,
                 uint32 screenY,
                 MC2BoundingBox& bbox,
                 int& tryNbr,
                 uint32 nbrBuaTextsAdded,
                 const DrawingProjection* projection,
                 const ImageDraw* image )
{
   const featuretextnotice_t& notice = *currentNoticeIt;

   bool toAdd = true;

   mc2dbg8 << "getTextLocation name \"" << feature->getName()
           << "\" s " << feature->getScaleLevel()
           << " #polys " << feature->getNbrPolygons() 
           << " type: " << GfxFeature::getFeatureTypeAsString( *feature )
           << " try nbr: " << tryNbr << endl;

   if ( feature->getNbrPolygons() == 0 ) {
      // Nothing to do without coordinates
      return false;
   }

   // Checks if feature really should be here
   bool smallImage = isSmallImage( screenX );

   if (((feature->getType() == GfxFeature::WATER ) && (scaleLevel < 4)) ||

       ((feature->getType() == GfxFeature::INDIVIDUALBUILDING) &&
        ((scaleLevel < 9 && feature->getPolygon(0)->getLength() < 600) ||

         (scaleLevel < 8 && feature->getPolygon(0)->getLength() < 1200))) ||

       (smallImage && (feature->getType() == GfxFeature::BUILTUP_AREA) &&
        (nbrBuaTextsAdded >= 6))) {
      return false;
   }

   int polygonIndex = 0;
   if ( feature->getNbrPolygons() > 1 ) {
      // TODO: Select "best" polygon
      if ((notice.m_textImportanceLevel == BUILTUP_AREA) &&
          (notice.m_scaleLevel <=4)) {
         int bestPolyIndex = feature->getNbrPolygons();
         float64 bestLength = 0;
         for ( polygonIndex = 0;
               polygonIndex < int32(feature->getNbrPolygons());
               polygonIndex++) {
            float64 length = feature->getPolygon(polygonIndex)->getLength();
            if (length > bestLength) {
               bestPolyIndex = polygonIndex;
               bestLength = length;
            }
         }
         polygonIndex = bestPolyIndex;
      } else  {
         while ( polygonIndex < int32(feature->getNbrPolygons()) &&
                 feature->getPolygon(
                    polygonIndex )->getNbrCoordinates() < 1 )
         {
            polygonIndex++;
         }
      }
   }
   if ( polygonIndex == int32( feature->getNbrPolygons() ) ) {
      // No "best" polygon
      polygonIndex = 0;
   }

   // Sanity check on "best" polygon
   if ( feature->getPolygon(polygonIndex)->getNbrCoordinates() == 0 ) {
      mc2dbg << "[MapDrawer]: Polygon " << polygonIndex << " has 0 coordinates"
             << endl;

      return false;
   }


   GfxPolygon* poly = feature->getPolygon( polygonIndex );
   if ( poly->getNbrCoordinates() < 1 &&
        !( feature->getType() == GfxFeature::BUILTUP_AREA_SQUARE ) &&
        !( feature->getType() == GfxFeature::BUILTUP_AREA_SMALL ))
   {
      mc2dbg4 << "Polygon " << polygonIndex << " to few coordinates "
              << poly->getNbrCoordinates() << endl;

      return false;
   }


   bool closed = false;
   bool backgroundArea = false;
   const char* fontName = "Vera.ttf";
   int fontSize = 8;
   if ( toAdd ) {
      GDUtils::Color::CoolColor color;
      MapFontSettings::
         getFontSettings( notice.m_feature,
                          notice.m_setting,
                          fontName, fontSize,
                          color, scaleLevel, smallImage );
      switch ( feature->getType() ) {
      case GfxFeature::BUILTUP_AREA:
      case GfxFeature::BUILTUP_AREA_SQUARE:
      case GfxFeature::BUILTUP_AREA_SMALL:
      case GfxFeature::LAND:
         backgroundArea = true;
         closed = true;
         break;
      case GfxFeature::POI:
      case GfxFeature::WATER_LINE:
         break;
      case GfxFeature::CARTOGRAPHIC_GROUND:
      case GfxFeature::CARTOGRAPHIC_GREEN_AREA:
      case GfxFeature::PARK:
      case GfxFeature::NATIONALPARK:
      case GfxFeature::FOREST:
      case GfxFeature::BUILDING:
      case GfxFeature::INDIVIDUALBUILDING:
      case GfxFeature::WATER:
      case GfxFeature::ISLAND:
      case GfxFeature::PEDESTRIANAREA:
      case GfxFeature::AIRCRAFTROAD:
         closed = true;
         break;
      default:
         mc2dbg8 << "MapDrawer::getTextLocation default type "
                 << (int)feature->getType() 
                 << " not to add." << endl;
         return false;
      }
   }

   // Draw the texts for the city centres
   if( toAdd && feature->getType() == GfxFeature::POI ) {
      const GfxPOIFeature* poi =
         static_cast<const GfxPOIFeature*>(feature);
      ItemTypes::pointOfInterest_t poiType = poi->getPOIType();

      if( poiType == ItemTypes::cityCentre ) {
         // Make only the first letter capital
         mc2TextIterator it( feature->getName() );
         MC2String lower = StringUtility::copyLower( it );
         feature->setName(
            StringUtility::makeFirstInWordCapital( lower ).c_str() );

         const char* poiName = poi->getName();
         GfxDataTypes::dimensions dim = 
            image->getStringDimension( fontSize, fontName, poiName,
                                       1.0, 1.0 );
         int32 nameWidth = dim.width;
         int32 nameHeight = dim.height;

         GfxPolygon* poly = poi->getPolygon(0);
         if( poly->getNbrCoordinates() == 0 ) {
            // No coordinates
            return false;
         }

         if( projection->getProjectionType() ==
             DrawingProjection::cosLatProjection ) {
            int32 lat = poly->getLat(0);
            int32 lon = poly->getLon(0);

            textGfx= GfxData::createNewGfxData( NULL, true );

            int32 addLat, addLon;
            // Upper left corner
            addLat = lat;
            addLon = lon - projection->getLonDiff( nameWidth / 2 );
            textGfx->addCoordinate( addLat, addLon );

            // Upper right coord
            addLat = lat;
            addLon = lon + projection->getLonDiff( nameWidth / 2 );
            textGfx->addCoordinate( addLat, addLon );

            // Lower right coord
            addLat = lat - projection->getLatDiff( nameHeight * 2 );
            addLon = lon + projection->getLonDiff( nameWidth / 2 );
            textGfx->addCoordinate( addLat, addLon );

            // Lower left coord
            addLat = lat - projection->getLatDiff( nameHeight * 2 );
            addLon = lon - projection->getLonDiff( nameWidth / 2 );
            textGfx->addCoordinate( addLat, addLon );

            feature->setFontSize( fontSize );
            feature->setTextLat( lat -
                                 projection->getLatDiff( nameHeight * 2 ) );
            feature->setTextLon( lon -
                                 projection->getLonDiff( nameWidth / 2 ) );
            feature->setDisplayText( true );
            feature->setDrawTextStart( 90 );

            MC2BoundingBox newBBox;
            textGfx->getMC2BoundingBox( newBBox );
         } else {
            MC2Point point = projection->
               getGlobalPoint( poly->getCoord( 0 ) );

            textGfx = GfxData::createNewGfxData( NULL, true );

            MC2Point tempPoint( 0,0 );

            // Upper left corner
            tempPoint = MC2Point( point.getX() - nameWidth / 2,
                                  point.getY() );
            textGfx->addCoordinate( projection->
                                    getGlobalCoordinate( tempPoint ) );

            // Upper right coord
            tempPoint = MC2Point( point.getX() + nameWidth / 2,
                                  point.getY() );
            textGfx->addCoordinate( projection->
                                    getGlobalCoordinate( tempPoint ) );

            // Lower right coord
            // offset the text a bit so the city center can be drawn visible
            // 50% should be enough....
            uint32 yOffset = nameHeight / 2;
            tempPoint = MC2Point( point.getX() + nameWidth / 2,
                                  point.getY() - nameHeight - yOffset );
            textGfx->addCoordinate( projection->
                                    getGlobalCoordinate( tempPoint ) );

            // Lower left coord, this is where we place the text
            tempPoint = MC2Point( point.getX() - nameWidth / 2,
                                  point.getY() - nameHeight - yOffset );
            MC2Coordinate textCoord = 
               projection->getGlobalCoordinate( tempPoint );
            textGfx->addCoordinate( textCoord );
            
            feature->setFontSize( fontSize );

            feature->setTextCoord( textCoord );
            feature->setDisplayText( true );
            feature->setDrawTextStart( 90 );

            MC2BoundingBox newBBox;
            textGfx->getMC2BoundingBox( newBBox );
         }
         return true;
      } else {
         // We only want texts for city centres.
         return false;
      }
   }

   if ( backgroundArea ) {
      mc2dbg8 << "backgroundArea " << feature->getName()
              << " featurescalelevel " << feature->getScaleLevel()
              << " scalelevel " << scaleLevel << endl;
   }

   // FIXME: Check if 3 is right
   // Don't draw the text if the feature has too high scaleLevel
   // compared to the current = if the diff is more than 2 levels.
   // Exception: big buas are ok to differ 3
   bool ok = ( (feature->getType() == GfxFeature::BUILTUP_AREA) &&
               (feature->getScaleLevel() <=3 ) &&
               (scaleLevel - feature->getScaleLevel() <= 3) );
   if ( toAdd && backgroundArea &&
        !( feature->getScaleLevel() + 3 > scaleLevel ) && !ok )
   {
      mc2dbg4 << "Areafeature to high scalelevel "
              << feature->getScaleLevel()
              << " current " << scaleLevel << endl;

      return false;
   }

   // FontSize is set in switch above
   const char* name = feature->getName();
   // Check if name is ok
   if ( strlen( name ) == 0 || strcmp( name, "MISSING" ) == 0 ) {
      mc2dbg2 << here << "Name is bad \"" << name << "\"" << endl;
      return false;
   }



   uint32 firstTextLength = 0;
   uint32 secondTextLength = 0;
   MC2String firstLine;
   MC2String secondLine;
   bool twoLines = false;
   // Check if the feature is allowed to
   // split its name into two lines
   if ( shouldSplitText( *feature ) ) {
      const char* lastSpace = strrchr( name, ' ' );
      // have space?, we can not split without space
      if ( lastSpace ) {
         firstTextLength = lastSpace - name + 1; // + 1 for space
         secondTextLength = strlen( name ) - firstTextLength;

         MC2String firstName( name, firstTextLength );
         firstLine = firstName;
         MC2String secondName( name + firstTextLength,
                               secondTextLength );
         secondLine = secondName;
                               
         twoLines = true;
      }
   } else {
      if ( useAltLangText( *feature ) && strcmp( name, feature->getBasename().c_str() ) != 0 ) {
         // This feature prints two text. Both the man name and the name in an
         // alternative language i.e. english
         firstTextLength = strlen( name );
         firstLine = name;
         secondTextLength = feature->getBasename().length();
         secondLine = feature->getBasename();
         twoLines = true;
      }
   }

   GfxDataTypes::dimensions dim = 
      image->getStringDimension( fontSize,
                                 fontName,
                                 name, 
                                 1.0, 1.0 );

   int32 nameWidth = dim.width;
   int32 nameHeight = dim.height;
   int32 charHeight = nameHeight;
   int32 charWidth = dim.width / strlen( name );


   GfxDataFull* featGfx = NULL;
   if ( toAdd ) {
      // The bbox for the text
      textGfx= GfxData::createNewGfxData( NULL, true );
      // Create the GfxData of the polygon ("best polygon").
      featGfx = GfxData::createNewGfxData( NULL, true );
      int32 lat = poly->getLat( 0 );
      int32 lon = poly->getLon( 0 );
      MC2Coordinate coord( lat, lon );
      MC2Point point = projection->getPoint( coord );
      featGfx->addCoordinate( point.getY(), point.getX() );
      for ( uint32 i = 1; i < poly->getNbrCoordinates(); i++ ) {
         lat = poly->getLat( i, lat );
         lon = poly->getLon( i, lon );
         MC2Coordinate coord( lat, lon );
         MC2Point point = projection->getPoint( coord );
         featGfx->addCoordinate( point.getY(), point.getX() );
      }
      featGfx->updateLength();

      // Make sure that the length of the feature is longer than the
      // length of the text.
      // (Length is the total length around the polygon)

      if ( feature->getType() != GfxFeature::BUILTUP_AREA &&
           feature->getType() != GfxFeature::POI &&
           feature->getType() != GfxFeature::LAND ) {

         if ( (( featGfx->getLength(0) * GfxConstants::METER_TO_MC2SCALE )
              < nameWidth) &&
              !( feature->getType() == GfxFeature::BUILTUP_AREA_SQUARE ) &&
              !( feature->getType() == GfxFeature::BUILTUP_AREA_SMALL ) &&
              !twoLines )
         {
            mc2dbg8 << "nameWidth too large " << nameWidth << " >= "
                    << featGfx->getLength(0) << endl;
            toAdd = false;
         }
         else if( (( featGfx->getLength(0) * GfxConstants::METER_TO_MC2SCALE )
                   < max( firstTextLength, secondTextLength )) &&
                  !( feature->getType() == GfxFeature::BUILTUP_AREA_SQUARE ) &&
                  !( feature->getType() == GfxFeature::BUILTUP_AREA_SMALL ) &&
                  twoLines )
         {
            mc2dbg8 << "nameWidth too large " << nameWidth << " >= "
                    << featGfx->getLength(0) << endl;
            toAdd = false;
         }
      }
   }

   // Check Park, Water etc.
   if ( toAdd && !backgroundArea && closed &&
        feature->getType() == GfxFeature::WATER ) {
      // If covers entire screen don't add
      // TODO: Text Should be added in water
      if ( featGfx->getMinLat() < 0 &&
           featGfx->getMaxLat() > int32(screenY) -1 &&
           featGfx->getMinLon() < 0 &&
           featGfx->getMaxLon() > int32(screenX) -1 ) {
         //  WATER too big
         toAdd = false;
      }
   }

   int32 textLat = 0;
   int32 textLon = 0;
   int drawTextStart = 0;

   // drawTextStart and textLat,textLon (Y=lat X=lon)
   if ( toAdd ) {
      int32 deltaX = 0;    // to decrease textX with dX
      int32 deltaY = 0;    // to increase textY with dY
      int32 textX = 0;
      int32 textY = 0;
      if ( ( closed && backgroundArea ) || closed) {
         featGfx->setClosed( 0, true );
         drawTextStart = 90; // Right angle
         MC2BoundingBox polyBBox;
         featGfx->getMC2BoundingBox(polyBBox);

         if ( feature->getType()==GfxFeature::BUILTUP_AREA_SQUARE ||
              feature->getType()==GfxFeature::BUILTUP_AREA_SMALL ){
            polyBBox.getCenter(textY,textX);
            toAdd = true;
            tryNbr=5;
            deltaX = nameWidth;
            deltaY = -nameHeight;
         }
         else {
            MC2Point center = 
               MapDrawingCommon::
               determineCentroid( *feature, *featGfx, *projection, toAdd );

            textX = center.getX();
            textY = center.getY();

            if (!toAdd){
               tryNbr = 5;
            } else {
               if( !twoLines ) {
                  deltaX = int32(rint((float64)nameWidth / 2 ));
                  deltaY = 0;
               } else { 
                  // text is double size in height plus some extra pixels
                  
                  GfxDataTypes::dimensions firstDim = 
                     image->getStringDimension( fontSize, fontName,
                                                firstLine.c_str(),
                                                1.0, 1.0 );
                  GfxDataTypes::dimensions secondDim = 
                     image->getStringDimension( fontSize, fontName,
                                                secondLine.c_str(),
                                                1.0, 1.0 );
                  nameWidth = max( firstDim.width, secondDim.width );

                  
                  // +5 for some extra space inbetween
                  nameHeight = firstDim.height + secondDim.height + 5;
                  deltaX = int32(rint((float64)nameWidth / 2));
                  deltaY = nameHeight / 2; 
                }
            }
         }

      } else if ( false && closed ) { 
         // TODO: check this some more, 
         // if enabled this breaks the invidiual building placement!
         // It looks almost like the code in the first if-statement...


         // Place text for an area feature.
         // Place it at the centroid of the polygon.
         // TODO: If WATER Place it along the border to avoid getting
         //       the name on islands etc.
         drawTextStart = 90; // Right angle
         featGfx->setClosed(0, true);
         
         int32 highValidLat = int32(screenY) - nameHeight - int32(rint(::fabs(deltaY)));
         int32 lowValidLat = nameHeight - int32(rint(::fabs(deltaY)));
         int32 lowValidLon = deltaX;
         int32 highValidLon = int32(screenX) - deltaX;
         
         MC2BoundingBox validArea( highValidLat, lowValidLon,
                                   lowValidLat, highValidLon );
         
         MC2BoundingBox polyBBox;
         featGfx->getMC2BoundingBox(polyBBox);
         toAdd = validArea.getInterSection(polyBBox, validArea);
         if (toAdd){
            highValidLat = validArea.getMaxLat();
            lowValidLat = validArea.getMinLat();
            highValidLon = validArea.getMaxLon();
            lowValidLon = validArea.getMinLon();

            if (tryNbr == 0){
               MC2Point center = 
                  MapDrawingCommon::
                  determineCentroid( *feature, *featGfx, *projection, toAdd );
               textX = center.getX();
               textY = center.getY();
            }

            if ( feature->getType() == GfxFeature::WATER ){
               placeWater( featGfx, textX, textY, tryNbr );
            }

            if( !twoLines ) {
               deltaX = int32(rint((float64)nameWidth / 2 ));
               deltaY = 0;
            } else {
               deltaX = int32(rint((float64) max( firstTextLength,
                                                  secondTextLength )
                                   * charWidth / 2));
               deltaY = -int32(rint((float64) charHeight*1.5 ));
            }
            //check if location is valid (not rivers)
            // do not move text if small image!

            if ( smallImage ) {
               tryNbr = 5;
            }
            //
            // Do not try another time for small images!
            // smallImages ~ mercator projection
            //
            if ( !smallImage && 
                 (( textY + deltaY - nameHeight < 0 ) ||
                  ( textY - deltaY - nameHeight < 0 ) ||
                  ( textY + deltaY + nameHeight > int32(screenY) ) ||
                  ( textY - deltaY + nameHeight > int32(screenY) ) ||
                  ( textX - deltaX < 0 ) ||
                  ( textX + deltaX > int32(screenX) ) )) {
               tryNbr++;
               // pick a valid point (textX, textY)
               if (tryNbr == 1){
                  textY = int32(rint(double(lowValidLat+3*highValidLat)/4));
                  textX = int32(rint(double(highValidLon+3*lowValidLon)/4));
               }
               else if (tryNbr == 2){
                  // pick a valid point (textX, textY)
                  textY = int32(rint(double(lowValidLat+3*highValidLat)/4));
                  textX = int32(rint(double(3*highValidLon+lowValidLon)/4));
               }
               else if (tryNbr == 3){
                  // pick a valid point (textX, textY)
                  textY = int32(rint(double(3*lowValidLat+highValidLat)/4));
                  textX = int32(rint(double(3*highValidLon+lowValidLon)/4));
               }
               else {
                  textY = int32(rint(double(3*lowValidLat+highValidLat)/4));
                  textX = int32(rint(double(highValidLon+3*lowValidLon)/4));
               }
            }

         }
      } else {
         nameHeight = 0;
         nameWidth = 0;
         // add open polygons if their MC2BoundingBox overlaps the map
         MC2BoundingBox polygonBBox;
         featGfx->getMC2BoundingBox(polygonBBox);
         // change to input coordinates
         polygonBBox.setMaxLat(int(rint(bbox.getMinLat() +
            projection->getLatDiff(
               screenY - 1 - polygonBBox.getMaxLat() ) ) ) );
         polygonBBox.setMinLat(int(rint(bbox.getMinLat() +
            projection->getLatDiff(
               screenY - 1 - polygonBBox.getMinLat() ) ) ) );

         polygonBBox.setMaxLon(int(rint(bbox.getMinLon() +
            projection->getLonDiff(
               polygonBBox.getMaxLon() ) ) ) );
         polygonBBox.setMinLon(int(rint(bbox.getMinLon() +
            projection->getLonDiff(
               polygonBBox.getMinLon() ) ) ) );
         toAdd = polygonBBox.overlaps(bbox);
      }

      if ( toAdd ) { 
         // Assume the text looks something like this which then can be
         // rotated 0 - pi degrees around the upper left coordinate.
         // ______________
         // | Baravagen  |
         // |------------|
         if( projection->getProjectionType() ==
             DrawingProjection::cosLatProjection ) {
             // Upper left coord in input scale
            int32 lat = int( projection->getLatDiff( int( rint( screenY - 1 -
                                    (textY + deltaY - nameHeight*1.5)))) +
                                   bbox.getMinLat() );
            int32 lon = int( projection->getLonDiff( int( rint(
                                    textX - deltaX - charWidth/3))) +
                                    bbox.getMinLon() );
            textGfx->addCoordinate( lat, lon );

            // Upper right coord
            lat = int( projection->getLatDiff( int( rint( (screenY - 1 -
                              (textY + deltaY - nameHeight*1.5) ) ) ) ) +
                             bbox.getMinLat() );
            lon = int( projection->getLonDiff( int( rint(
                       (textX + deltaX + charWidth/3)))) + bbox.getMinLon() );
            textGfx->addCoordinate( lat, lon);

            // Lower right coord
            lat = int( projection->getLatDiff( int( rint( (screenY - 1 -
                          (textY /*- deltaY*/ + nameHeight/**1.5*/))))) +
                          bbox.getMinLat() );
            lon = int( projection->getLonDiff( int( rint(
                          (textX + deltaX + charWidth/3)))) +
                           bbox.getMinLon() );
            textGfx->addCoordinate( lat, lon);

            // Lower left coord
            lat = int( projection->getLatDiff( int( rint( (screenY - 1 -
                              (textY /*- deltaY*/ + nameHeight/**1.5*/))))) +
                             bbox.getMinLat() );
            lon = int( projection->getLonDiff( int( rint(
                              (textX - deltaX - charWidth/3)))) +
                              bbox.getMinLon() );
            textGfx->addCoordinate( lat, lon );

            textGfx->setClosed(0, true);
            // textX,Y is the LOWER left corner when drawn by GDImageDraw
            // (the textGfx box is created considering this)
            textX -= deltaX;
            textY += deltaY;
            textLat = int( projection->getLatDiff( int( rint(
                              (screenY - 1 - (textY))))) +
                              bbox.getMinLat() );
            textLon = int( projection->getLonDiff( int( rint( (textX)) )) +
                              bbox.getMinLon() );

         } else { 
            // this works best for mercator projection
            // TODO: test other projections too...

            // Upper left coord in input scale
            MC2Point bboxTopLeftPoint =
               projection->
               getGlobalPoint( bbox.getCorner( MC2BoundingBox::top_left ) );
            // Enlarge the box by 3 pixels on each side,
            // this will make sure we do not place them too close.
            // The collision testing will also be easier, especially at
            // higher zoom levels, where pixels means more.
            const int32 INC_BOX = 3;
            MC2Point topLeftPoint = bboxTopLeftPoint + 
               MC2Point( textX - deltaX - INC_BOX,
                         - (int)(textY + deltaY - nameHeight*1.5) + INC_BOX);
            
            MC2Point bottomRightPoint = topLeftPoint + 
               MC2Point( nameWidth + INC_BOX, -nameHeight - INC_BOX);
            
            MC2Coordinate topLeftCoord = 
               projection->getGlobalCoordinate( topLeftPoint );
            MC2Coordinate bottomRightCoord = 
               projection->getGlobalCoordinate( bottomRightPoint ); 
            MC2BoundingBox realTextBox( topLeftCoord, bottomRightCoord );

            textGfx->addCoordinate( realTextBox.
                                    getCorner(MC2BoundingBox::top_left ) );
            textGfx->addCoordinate( realTextBox.
                                    getCorner(MC2BoundingBox::top_right ) );
            textGfx->addCoordinate( realTextBox.
                                    getCorner(MC2BoundingBox::bottom_right ) );
            MC2Coordinate bottomLeftCoord = realTextBox.
               getCorner( MC2BoundingBox::bottom_left );
            textGfx->addCoordinate( bottomLeftCoord );
            textGfx->setClosed( 0, true );
            // textX,Y is the LOWER left corner when drawn by GDImageDraw
            // (the textGfx box is created considering this)
            textX -= deltaX;
            textY += deltaY;
            textLat = bottomLeftCoord.lat;
            textLon = bottomLeftCoord.lon;
            tryNbr = 5;
         }
      }
   
   }

   

   // Finally set data
   if ( toAdd ) {
      // Set textGfx and feature
      feature->setFontSize( fontSize );
      feature->setTextLat( textLat );
      feature->setTextLon( textLon );
      feature->setDisplayText( true );
      feature->setDrawTextStart( drawTextStart );
      // textGfx is set on the way
   }
   if ( ! toAdd ) {
      delete textGfx;
      textGfx = NULL;
   }
   delete featGfx;
   return toAdd;
}

namespace {
/**
 * Calculates boundingboxes around a given bounding box.
 * Used to block off everything around an area in the text placement.
 *
 * @param bbox   The area to get boxes around.
 * @param factor Controls the size of the surrounding boxes,
 *               will be multiplied by the width and height of bbox. 
 */
vector<MC2BoundingBox> getBoxesAround( const MC2BoundingBox& bbox,
                                       int factor = 10 ) {
   vector<MC2BoundingBox> result;

   int32 width = bbox.getWidth();
   int height = bbox.getHeight();

   // a box to the left of bbox
   int lat1 = bbox.getMaxLat()+height*factor;
   int lon1 = bbox.getMinLon()-width*factor;
   int lat2 = bbox.getMinLat()-height*factor;
   int lon2 = bbox.getMinLon();

   result.push_back( MC2BoundingBox( MC2Coordinate( lat1, lon1 ),
                                     MC2Coordinate( lat2, lon2 ) ) );
   
   // same box but now on the other side
   lon1 = bbox.getMaxLon();
   lon2 = bbox.getMaxLon()+width*factor;

   result.push_back( MC2BoundingBox( MC2Coordinate( lat1, lon1 ),
                                     MC2Coordinate( lat2, lon2 ) ) );

   // a box above bbox
   lat1 = bbox.getMaxLat()+height*factor;
   lon1 = bbox.getMinLon();
   lat2 = bbox.getMaxLat();
   lon2 = bbox.getMaxLon();

   result.push_back( MC2BoundingBox( MC2Coordinate( lat1, lon1 ),
                                     MC2Coordinate( lat2, lon2 ) ) );

   // same box but below
   lat1 = bbox.getMinLat();
   lat2 = bbox.getMinLat()-height*factor;

   result.push_back( MC2BoundingBox( MC2Coordinate( lat1, lon1 ),
                                     MC2Coordinate( lat2, lon2 ) ) );

   return result;
}
}

bool 
getTextLocationRotated( GfxFeature* feature,
                                   featuretextnotice_t& notice,
                                   uint32 scaleLevel,
                                   uint32 screenX,
                                   uint32 screenY,
                                   MC2BoundingBox& bbox,
                                   ImageDraw* image,
                                   MapDrawingCommon::ObjectBoxes& objectBBoxes,
                                   vector<GfxData*>& gfxTextArray,
                                   vector<featuretextnotice_t>& addednotices,
                                   uint32& nbrRoadSigns,
                                   const DrawingProjection* projection,
                                   uint32 streetWidth )
{
   // roadSigns & road names on county_level and above does not work
   if (scaleLevel <= 2) {
      return false;
   }

   bool toAdd = true;
   int fontSize = 10;
   bool smallImage = isSmallImage(screenX);
   GfxFeature::gfxFeatureType featureType = feature->getType();

   // 1. Some stuff moved/copied from getTextLocation
   // No names on ferrys
   if (featureType == GfxFeature::FERRY) {
      return false;
   }

   // get rid off too small streets immediately
   if ((featureType == GfxFeature::STREET_FOURTH) &&
       (scaleLevel <= 8-uint(smallImage))) {   // block/district
      return false;
   } else if ((featureType == GfxFeature::STREET_THIRD) &&
       (scaleLevel <= 7-uint(smallImage))) {   // district/small city
      return false;
   } else if ((featureType == GfxFeature::STREET_SECOND) &&
       (scaleLevel <= 6-uint(smallImage))) {   // small city/city
      return false;
   }

   // Check if name is ok
   const char* name = feature->getName();
   if ( strlen( name ) == 0 || strcmp( name, "MISSING" ) == 0 ) {
      mc2dbg2 << "Name is bad \"" << name << "\"" << endl;
      return false;
   }
   // Check if same name is allready added
   // Needs the abbreviated name here since that is stored in addednotices
   // FIXME: Not hardcoded language!
   ScopedArray<char> abbName( new char[strlen(name) + 1] );
   AbbreviationTable::abbreviate(name, abbName.get(), LangTypes::swedish,
                                 AbbreviationTable::anywhere);
   for ( featureTextNoticeConstIt f = addednotices.begin();
         f != addednotices.end();
         ++f ) {
      if ( strcmp( abbName.get(), f->m_feature->getName() ) == 0 ) {
         mc2dbg4 << "Name allready added " << name << endl;
         return false;
      }
   }

   // 2. get roadSigns correct
   bool roadSign = false;
   if (toAdd) {
      if ( MapDrawingCommon::roadSignName( name ) ) {
         roadSign = true;
         toAdd = false;
      }
      if (roadSign && !smallImage) {
         uint32 maxNbrSigns = 8;
         if (scaleLevel >= 4)
            maxNbrSigns = 5;
         if (nbrRoadSigns < maxNbrSigns) {
            // Find and set textLat and textLon for the roadSign
            // lat & lon in the middle of the ssi (poly0).
            // Don't add roadSign on ramp or roundabout
            GfxPolygon* poly = feature->getPolygon(0);
            GfxRoadPolygon* roadPoly = dynamic_cast<GfxRoadPolygon*>(poly);
            if (poly != NULL  &&
                !((roadPoly!= NULL) &&
                  (roadPoly->isRamp() || roadPoly->isRoundabout()))) {
               int32 midlat, midlon, foo;
               float64 polyLength = notice.m_length;
               if (polyLength <= 0)
                  polyLength = poly->getLength();
               if (poly->getPointOnPolygon(
                     polyLength/2.0, foo, midlat, midlon)) {

                  // a roadSign is 30*15 pixels, moving the sign 30 units
                  // west seems to center the sign over the roadsegment.
                  float64 mc2unitsPerPixelX = bbox.getWidth() / screenX;
                  float64 mc2unitsPerPixelY = bbox.getHeight() / screenY;
                  int32 textLat = midlat;
                  int32 textLon = midlon - int(rint(mc2unitsPerPixelX * 30));

                  // Build a roadSign bbox and
                  // check that it does not overlap the objectBBoxes
                  GfxDataFull signGfx;
                  signGfx.addPolygon();
                  // Upper left coord in input scale
                  signGfx.addCoordinate(
                        textLat + int(rint(mc2unitsPerPixelY * 15)),
                        textLon );
                  // Upper right coord
                  signGfx.addCoordinate(
                        textLat + int(rint(mc2unitsPerPixelY * 15)),
                        textLon + int(rint(mc2unitsPerPixelX * 30) * 2) );
                  // Lower right coord
                  signGfx.addCoordinate(
                        textLat - int(rint(mc2unitsPerPixelY * 15)),
                        textLon + int(rint(mc2unitsPerPixelX * 30) * 2) );
                  // Lower left coord
                  signGfx.addCoordinate(
                        textLat - int(rint(mc2unitsPerPixelY * 15)),
                        textLon );
                  signGfx.updateLength();
                  signGfx.setClosed(0, true);

                  MC2BoundingBox signBBox;
                  signGfx.getMC2BoundingBox(signBBox);
                  if (signBBox.inside(bbox)) {
                     bool cont = true;

                     for ( MapDrawingCommon::
                              ObjectBoxes::SizeType boxIndex = 0;
                           boxIndex < objectBBoxes.getNbrBoxes();
                           ++boxIndex ) {
                        if ( objectBBoxes.getWorldBox( boxIndex ).
                             overlaps( signBBox ) ) {
                           cont = false;
                           break;
                        }
                     }

                     if (cont) {
                        feature->setTextLat(textLat);
                        feature->setTextLon(textLon);
                        mc2dbg4 << "Add roadSign '" << name << "'" << endl;
                        toAdd = true;
                        nbrRoadSigns++;
                        objectBBoxes.addBox( signBBox, *projection );
                     }
                  }
               }
            }
         }
      }
   }

   // Check that we have an appropriate scaleLevel for
   // drawing names of the streets
   if (toAdd && !roadSign) {
      if (!MapDrawingCommon::
          mergeAndDrawStreetNames( featureType, scaleLevel,
                                   smallImage, false)) {
         toAdd = false;
      }
   }

   //3. drawRotatedText
   if ( ! toAdd || roadSign ) {
      return toAdd;
   }


   const char* fontName = "Vera.ttf";
   uint32 textPlacingTime, startTime;
   startTime = TimeUtility::getCurrentMicroTime();
   // TODO: Temporary code. Must be replaced later on!!
   // We need GfxData, but since we don't have it, we
   // create it from the GfxFeature. Unefficient!!
   // ================================================
   // This is the code for creating a GfxData
   // that has the same polygon structure as the
   // GfxFeature. Note however that the current
   // textplacement algorithm does not handle multiple
   // polygons correctly. That is why the code below
   // is commented.
   //                  GfxData tmpGfx;
   //                  for (uint32 poly = 0;
   //                       poly < feature->getNbrPolygons();
   //                       poly++) {
   //                     GfxPolygon* polygon = feature->getPolygon(poly);
   //                     int32 lat = polygon->getLat(0);
   //                     int32 lon = polygon->getLon(0);
   //                     tmpGfx.addCoordinate(lat,lon,true);
   //                     for (uint32 coords = 1;
   //                          coords < polygon->getNbrCoordinates();
   //                          coords++) {
   //                        tmpGfx.addCoordinate(
   //                           lat = polygon->getLat(coords, lat),
   //                           lon = polygon->getLon(coords, lon),
   //                           false);
   //                     }
   //                  }
   //
   // Very temporary code!!!
   //
   // Create a GfxData only consisting of one
   // polygon, regardless of how many polygons that
   // the GfxFeature has. We know that currently
   // the only way several polygons can exist in the
   // GfxFeature is due to drawing of bridges/tunnels
   // etc. We take advantage of that these multiple polygons
   // are not actually "true" polygons in the sense that
   // the pen is NOT lifted between the different polygons.
   // Therefore we create a GfxData only containing one
   // polygon, since the textplacement currently only
   // handles one polygon anyway. Fix for release_1_2.
   GfxDataFull tmpGfx;
   tmpGfx.addPolygon();
   GfxPolygon* polygon = feature->getPolygon(0);
   int32 lat = polygon->getLat(0);
   int32 lon = polygon->getLon(0);
   tmpGfx.addCoordinate(lat,lon);
   for (uint32 poly = 0;
        poly < feature->getNbrPolygons();
        poly++) {
      polygon = feature->getPolygon(poly);
      // Note that we skip the first coordinate in each
      // polygon (except for the first polygon) in
      // order to avoid duplicate coordinates.
      // TODO: This must be changed when GfxFeature
      // with "real" multiple polygons should be supported
      // (ie. when we use streets instead of
      // streetsegments).
      for (uint32 coords = 1;
           coords < polygon->getNbrCoordinates();
           coords++) {
         tmpGfx.addCoordinate( lat = polygon->getLat( coords, lat ),
                               lon = polygon->getLon( coords, lon ) );
      }
   }
   tmpGfx.updateLength();

   GfxData* useGfx = &tmpGfx;
   // use the gfxData that was created in mergeStreetFeatures
   if (notice.m_gfxData.get() != NULL) {
      useGfx = notice.m_gfxData.get();
   }

   // ================================================
   // Create dimension vector:
   vector<GfxDataTypes::dimensions> dimVec;
   // abbName already fetched above.
   // char* abbName = new char[strlen(name) + 1];
   fontSize = 9;

   if ( ! image->getGlyphDimensions( fontSize,
                                     fontName,
                                     abbName.get(),
                                     dimVec,
                                     bbox.getWidth() / float64(screenX),
                                     bbox.getHeight() / float64(screenY)) ) {
      // The fonts and glyphs could not be accessed.
      // Lets guess the dimensions instead then.
      // Hopefully we will not end up here..
      mc2log << error << here
             << " Could not get correct dimensions of glyphs."
             << " Guessing dimensions." << endl;
      int width =
         int32(7 * bbox.getWidth() / float64(screenX));
      int height =
         int32(10 * bbox.getHeight() / float64(screenY));
      // Create one dim for each character.
      for (mc2TextIterator it = mc2TextIterator( abbName.get() );
           *it != 0; ++it ) {
         GfxDataTypes::dimensions dim;
         dim.width = width;
         dim.height = height;
         dimVec.push_back(dim);
      }
   }


   // The outdata.
   vector<GfxDataTypes::textPos> textOut;

   auto_ptr<GfxDataFull> tmpGfx2( GfxData::createNewGfxData( NULL, true ) );

   for ( int16 polyIndex = 0;
         polyIndex < useGfx->getNbrPolygons();
         ++polyIndex ){

      vector<MC2BoundingBox> around = getBoxesAround( bbox );
      MapDrawingCommon::ObjectBoxes::WorldBoxes worldBoxes = 
         objectBBoxes.getWorldBoxes();

      for ( size_t i = 0; i < around.size(); ++i ) {
         worldBoxes.insert( worldBoxes.begin(), around[ i ] );
      }

      if ( ! useGfx->getTextPosition( worldBoxes,
                                      polyIndex, tmpGfx2.get(),
                                      gfxTextArray ) ){
         continue;
      }

      StreetTextPlacer streetTextPlacer;
      if ( ! streetTextPlacer.placeStreetText( tmpGfx2->polyBegin( polyIndex ),
                                               tmpGfx2->polyEnd( polyIndex ),
                                               dimVec, textOut ) ) {
         continue;
      }

      mc2dbg4 << "Add text name " << feature->getName()
              << " (abb " << abbName.get() << ")" << endl;

      // text placed out, calculate MC2BoundingBoxes for each letter
      for ( uint16 j = 0; j < textOut.size(); ++j ) {

         float64 angle = textOut[ j ].angle;
         int32 lat = textOut[ j ].lat;
         int32 lon = textOut[ j ].lon;
         uint32 width = dimVec[ j ].width;
         uint32 height = dimVec[ j ].height;

         /*
          *             x + w * cos( angle - PI / 2 )
          *             y + h * sin( angle - PI / 2 )
          *               \
          *                \      w
          *                 +-----------+ x2 + w * cos( angle - pi / 2 )
          *                 |           | y2 + h * sin( angle - pi / 2 )
          *                 |           |
          *                 |           |
          *                 |           | Letter box
          *                 |           |
          *\.               | h         | h
          * \.              |           |
          *  \.             |           |
          *   \.--+  angle  |           |
          *    \.  \        |_          |_
          *     \.  |       | |         | |
          *      -----------+-----------+---
          *               x, y         x2 = x + w * cos( angle )
          *                            y2 = y + h * sin( angle )
          *
          * Where "h" is height of glyph and "w" is the width.
          * "x" and "y" is the text placement position
          * 
          * The x2,y2 point is  calculated along the main line which
          *
          */
         int32 lowRightLon = lon + int32( width * cos( angle ) );
         int32 lowRightLat = lat + int32( height * sin( angle  ) ) ;

         int32 lonDist = static_cast<int32>( width * cos( angle + M_PI /2));
         int32 latDist = static_cast<int32>( height * sin( angle - M_PI /2));

         int32 highRightLon = lowRightLon + lonDist;
         int32 highRightLat = lowRightLat - latDist;

         int32 highLeftLon = lon + lonDist;
         int32 highLeftLat = lat - latDist;

         /* 
          * Now calculate the bounding box that the 
          * rotated box needs.
          *
          *      +------------------+
          *      |         +        |
          *      |        / \       | The real bounding box
          *      |       /   \      |
          *      |      /     \     |
          *      |     /       \    |
          *      |    /         \   |
          *      |   /           \  |
          *      |  /  rotated    + |
          *      | +     box     /  |
          *      |  \           /   |
          *      |   \         /    |
          *      |    \       /     |
          *      |     \     /      |
          *      |      \   /       |
          *      |       \ /        |
          *      |        +         |
          *      +------------------+
          *
          */

         // do a min of lat,lon and max of lat,lon to get each side
         MC2BoundingBox 
            textbox( ::max4( lat, lowRightLat, highRightLat, highLeftLat ),
                     ::min4( lon, lowRightLon, highRightLon, highLeftLon ),
                     ::min4( lat, lowRightLat, highRightLat, highLeftLat ), 
                     ::max4( lon, lowRightLon, highRightLon, highLeftLon ) );
                     
         // prevent other texts from being placed out too close
         textbox.increaseFactor( 0.9 );

         // if the textbox is not totaly inside, then we 
         // must give up textplacement for mercartor projection
         // since we can not place text across tiles.
         if ( ! textbox.inside( bbox ) ) {
            return false;
         }
         // dont add this until we actually calculate the box correctly
         objectBBoxes.addBox( textbox, *projection );
      }

      textPlacingTime = TimeUtility::getCurrentMicroTime() - startTime;
      mc2dbg8 << "Placing text took " << textPlacingTime
              << " us." << endl;
      // set textOut in the textnotice
      notice.m_curvedTextOut = textOut;

      // update name to abbName in feature
      feature->setName( abbName.get() );
      feature->setFontSize( fontSize );
      
      return true;
   }

   return false;

}

void 
placeOtherText( MapSettings* mapSettings,
                const vector<featuretextnotice_t> &notices,
                const GfxFeatureMap* featureMap,
                GfxFeature* feature,
                const featureTextNoticeIt& f, 
                const DrawingProjection* projection,
                ImageDraw* image,
                vector<GfxData*>& gfxTextArray,
                MapDrawingCommon::ObjectBoxes& objectBBoxes,
                vector<featuretextnotice_t>& addednotices,
                uint32& nbrBuaTextsAdded ) {

   int tryNbr = 0;
   int maxNbrTries = 1;
   bool locationFound = false;
   bool cont = false;
   while ( ! locationFound && tryNbr < maxNbrTries ) {
      auto_ptr<GfxDataFull> textGfx;
      MC2BoundingBox bbox = projection->getBoundingBox();
      {
         GfxDataFull* tmpGfx = NULL;
         locationFound = MapTextPlacement::
            getTextLocation( feature, 
                             notices, f,
                             tmpGfx,
                             featureMap->getScaleLevel(),
                             featureMap->getScreenX(),
                             featureMap->getScreenY(),
                             bbox,
                             tryNbr,
                             nbrBuaTextsAdded,
                             projection,
                             image );
         textGfx.reset( tmpGfx );
      }

      if ( (f->m_setting ? f->m_setting->m_textOnMap : true ) &&
           locationFound) {
         MC2BoundingBox currTextBBox;
         textGfx->getMC2BoundingBox( currTextBBox );

         // Text is ok to try to add, do collision test
         cont = ! Collision::collisionTest( currTextBBox,
                                            gfxTextArray, objectBBoxes, *projection );
      } else {
         cont = false;
      }

      if (cont) {
         // The text did not overlap any other text.
         feature->setDisplayText( true );
         // Added texts
         featuretextnotice_t notice;
         notice.m_scaleLevel = feature->getScaleLevel();
         notice.m_feature = feature;
         addednotices.push_back( notice );
         if ( feature->getType() == GfxFeature::BUILTUP_AREA ) {
            nbrBuaTextsAdded++;
         }

         if ( textGfx.get() != NULL ){

            MC2BoundingBox polBBox;
            textGfx->getMC2BoundingBox( polBBox );
            
            objectBBoxes.addBox( polBBox, *projection );
 
            // if the textGfx box does not overlap our draw/projection box then
            // there is no need to draw it, but it should still be in
            // the collision set ( see above ) since other text that
            // are drawn must be tested against it so each tile get the
            // same set of features to draw between the tile borders
            if ( polBBox.overlaps(projection->getBoundingBox()) ) {

               if ( !((feature->getType() == GfxFeature::BUILTUP_AREA_SQUARE) ||
                      (feature->getType() == GfxFeature::BUILTUP_AREA_SMALL))){
                  // Water must have the entire name inside the projection box
                  // else we end up with odd text placement on different tiles,
                  // for instance rivers does not have the entire polygon
                  // included so it would be difficult to calculate the
                  // centroid in the same place.
                  // A good example is the river "Gota Alv" in Gothenburg,
                  // Sweden, it goes through the city.
                  if ( feature->getType() == GfxFeature::WATER &&
                       ! polBBox.inside( projection->getBoundingBox() ) ) {
                     feature->setDisplayText( false );
                  } else {
                     gfxTextArray.push_back(textGfx.release());
                  }
               }
            } else {
               // it's not inside our draw box!
               // do not try to draw it
               // ( it should still be in the collision set )
               feature->setDisplayText( false );
            }
         }

         if ( feature->getType() == GfxFeature::BUILTUP_AREA_SQUARE ||
              feature->getType() == GfxFeature::BUILTUP_AREA_SMALL ) {
            drawBUA( feature, f, image, mapSettings );
         }

         tryNbr = 5;
         locationFound = true;

      } else {
         locationFound = false;
         ++tryNbr;
         feature->setDisplayText( false );
      }
   }//while
}

void 
placeStreetText( MapSettings* mapSettings,
                 const GfxFeatureMap* featureMap,
                 GfxFeature* feature,
                 featureTextNoticeIt& f, 
                 const DrawingProjection* projection,
                 ImageDraw* image,
                 vector<GfxData*>& gfxTextArray,
                 MapDrawingCommon::ObjectBoxes& objectBBoxes,
                 vector<featuretextnotice_t>& addednotices,
                 uint32& nbrRoadSigns ) {

   uint32 scaleLevel = featureMap->getScaleLevel();
   MC2BoundingBox bbox = projection->getBoundingBox();
   uint32 streetWidth = 3;
   DrawSettings settings;
   if ( MapUtility::
        getDrawSettings( feature->getType(),
                         scaleLevel,
                         mapSettings,
                         &settings,
                         feature,
                         false,
                         NULL, // no poly
                         featureMap ) ) {
      streetWidth = settings.m_lineWidth;
   }
   
   if ( MapTextPlacement::
        getTextLocationRotated( feature, *f, scaleLevel,
                                (uint32)featureMap->getScreenX(),
                                (uint32)featureMap->getScreenY(),
                                bbox, image,
                                objectBBoxes,
                                gfxTextArray,
                                addednotices,
                                nbrRoadSigns,
                                projection,
                                streetWidth ) ) {
      
      feature->setDisplayText( true );
      featuretextnotice_t notice;
      notice.m_scaleLevel = feature->getScaleLevel();
      notice.m_feature = feature;
      addednotices.push_back( notice );
      
      mc2dbg4 << "Add text name " << feature->getName()
              << endl;
   }

}

bool initText( vector<featuretextnotice_t> &notices,
               MapDrawingCommon::ObjectBoxes& objectBBoxes,
               GfxFeatureMap* featureMap,
               ImageDraw* image,
               MapSettings* mapSettings,
               vector<GfxData*>& gfxTextArray,
               const DrawingProjection* projection ) {
   DebugClock startClock;
   uint32 routeableTime = 0;
   uint32 otherTime = 0;

   //   uint32 scaleLevel = featureMap->getScaleLevel();
   // Added texts TODO: Better structure!
   vector<featuretextnotice_t> addednotices;

   uint32 nbrBuaTextsAdded = 0;
   uint32 nbrRoadSigns = 0;
   uint32 nbrRouteable = 0;
   uint32 nbrOthers = 0;

   for ( featureTextNoticeIt f = notices.begin() ;
         f != notices.end(); ++f ) {

      GfxFeature* feature = const_cast<GfxFeature*> ( f->m_feature );
      // no need to do text placement with notices that does not have 
      // any text
      if ( strlen( feature->getName() ) == 0 ) {
         continue;
      }

      if ( (feature->getType() != GfxFeature::BUILTUP_AREA) &&
           (feature->getType() != GfxFeature::BUILTUP_AREA_SMALL) &&
           (feature->getType() != GfxFeature::BUILTUP_AREA_SQUARE ) ) {

         // The GfxData of the boundary of the name, added as text
         // textGfx for streets and ferries are not stored
         if (! ( MapDrawingCommon::isStreetFeature( *feature ) ||
               (feature->getType() == GfxFeature::FERRY))){
            DebugClock otherClock;
            nbrOthers++;
            MapTextPlacement::
               placeOtherText( mapSettings,
                               notices,
                               featureMap, feature, f,
                               projection, image,
                               gfxTextArray,
                               objectBBoxes,
                               addednotices,
                               nbrBuaTextsAdded );

            otherTime += otherClock.getTime();
         } else{
            // STREET or FERRY
            DebugClock routableClock;
            nbrRouteable++;
            MapTextPlacement::
               placeStreetText( mapSettings,
                                featureMap, feature, f,
                                projection, image,
                                gfxTextArray,
                                objectBBoxes,
                                addednotices,
                                nbrRoadSigns );
            routeableTime += routableClock.getTime();
         }
      }
   }

   mc2dbg1 << "initText() took " << startClock << endl;
   mc2dbg2 << " init routeable (" << nbrRouteable << ") took "
           << routeableTime << " ms" << endl << " init other ("
           << nbrOthers << ") took " << otherTime << " ms" << endl;

   return true;
}

      

bool
initializeText( GfxFeatureMap* featureMap,
                MapSettings* mapSettings,
                vector<featuretextnotice_t> &notices,
                MapDrawingCommon::ObjectBoxes& objectBBoxes,
                ImageDraw* image,
                vector<GfxData*>& gfxTextArray,
                const DrawingProjection* projection) {
   uint32 startTime = TimeUtility::getCurrentTime();
   // Set the text-related variables in the featureNotices
   uint32 screenX = featureMap->getScreenX();
   bool smallImage = MapTextPlacement::isSmallImage(screenX);

   // Collect streets separately to merge them
   vector<featuretextnotice_t> streetnotices;

   for ( uint32 i = 0 ; i < featureMap->getNbrFeatures() ; i++ ) {
      const GfxFeature* curFeature = featureMap->getFeature( i );
      // Don't draw the texts of the BUA:s, we use the city centres
      // instead from now on.
      if ( (curFeature->getType() != GfxFeature::BUILTUP_AREA) &&
           (curFeature->getType() != GfxFeature::BUILTUP_AREA_SMALL) &&
           (curFeature->getType() != GfxFeature::BUILTUP_AREA_SQUARE ) )
      {
         featuretextnotice_t notice;
         notice.m_scaleLevel = curFeature->getScaleLevel();
         notice.m_feature = curFeature;
         notice.m_setting = mapSettings->getSettingFor(
            curFeature->getType(), curFeature->getScaleLevel() );
         // lenght is for sorting the notices,
         // e.g. for bua to draw name on largest buas first.
         // e.g. for ssi to draw name|roadsign on the longest street segments.
         notice.m_length = 0;

         bool includeThisNotice = true;

         GfxFeature::gfxFeatureType featureType = curFeature->getType();
         switch( featureType ) {
            case ( GfxFeature:: STREET_MAIN ) :
               notice.m_textImportanceLevel = STREET_MAIN;
               // no names when scale is county level or more zoomed out
               if (featureMap->getScaleLevel() > 2)
                  notice.m_length +=
                     notice.m_feature->getPolygon(0)->getLength();
               break;
            case ( GfxFeature:: STREET_FIRST ) :
               notice.m_textImportanceLevel = STREET_FIRST;
               // no names when scale is county level or more zoomed out
               if (featureMap->getScaleLevel() > 2)
                  notice.m_length +=
                     notice.m_feature->getPolygon(0)->getLength();
               break;
            case ( GfxFeature:: STREET_SECOND ) :
               notice.m_textImportanceLevel = STREET_SECOND;
               break;
            case ( GfxFeature:: STREET_THIRD ) :
               notice.m_textImportanceLevel = STREET_THIRD;
               break;
            case ( GfxFeature:: STREET_FOURTH ) :
               notice.m_textImportanceLevel = STREET_FOURTH;
               break;


            case ( GfxFeature:: BUILTUP_AREA ) :
               notice.m_textImportanceLevel = BUILTUP_AREA;
               for (uint32 p=0; p < notice.m_feature->getNbrPolygons(); p++) {
                  notice.m_length +=
                     notice.m_feature->getPolygon(p)->getLength();
               }
               break;
            case ( GfxFeature:: POI ) : {
               const GfxPOIFeature* poi = static_cast<const GfxPOIFeature*>(notice.m_feature);
               if( poi->getPOIType() == ItemTypes::cityCentre ) {
                  notice.m_textImportanceLevel = CITY_CENTRE;
               } else {
                  notice.m_textImportanceLevel = OTHER;
               }
            }
               break;
            case ( GfxFeature:: NATIONALPARK ) :
               // Disable national park names until there is
               // a better solution to merging national park with
               // the same name together.
               includeThisNotice = false;
               notice.m_textImportanceLevel = PARK;
            break;
            case ( GfxFeature:: CARTOGRAPHIC_GROUND ):
            case ( GfxFeature:: CARTOGRAPHIC_GREEN_AREA ):
            case ( GfxFeature:: PARK ) :

            case ( GfxFeature:: FOREST ) :
               notice.m_textImportanceLevel = PARK;
               break;
            case ( GfxFeature:: BUILDING ) :
               notice.m_textImportanceLevel = BUILDING;
               break;
            case ( GfxFeature:: INDIVIDUALBUILDING ) :
               notice.m_textImportanceLevel = INDIVIDUALBUILDING;
               break;
            case ( GfxFeature:: WATER ) :
               notice.m_textImportanceLevel = WATER;
               break;
            case ( GfxFeature:: WATER_LINE ) :
               notice.m_textImportanceLevel = WATER_LINE;
               break;
            case ( GfxFeature:: ISLAND ) :
               notice.m_textImportanceLevel = ISLAND;
               // Disable island names until there is a better
               // solution to very large islands, such as
               // Sicily.
               includeThisNotice = false;

               break;
            case ( GfxFeature:: PEDESTRIANAREA ) :
               notice.m_textImportanceLevel = PEDESTRIANAREA;
               break;
            case ( GfxFeature::AIRCRAFTROAD ) :
               notice.m_textImportanceLevel = AIRCRAFTROAD;
               break;
            case ( GfxFeature:: LAND ) :
               notice.m_textImportanceLevel = LAND;
               // Special for USA, we don't want the country name to be
               // placed randomly, so don't include at all...
               if ( strstr(curFeature->getName(), "USA") != NULL )
                  includeThisNotice = false;
               break;
            case ( GfxFeature:: BUILTUP_AREA_SQUARE ) :
               notice.m_textImportanceLevel = BUILTUP_AREA_SQUARE;
               break;
            case ( GfxFeature:: BUILTUP_AREA_SMALL ) :
               notice.m_textImportanceLevel = BUILTUP_AREA_SMALL;
               break;
            case ( GfxFeature:: FERRY ) :
               notice.m_textImportanceLevel = FERRY;
               break;
            default:
               notice.m_textImportanceLevel = OTHER;
               break;
         }
         if ( (strlen(curFeature->getName()) > 0) &&
              (MapDrawingCommon::
               mergeAndDrawStreetNames(featureType,
                                       featureMap->getScaleLevel(),
                                       smallImage, true)) &&
              (!MapDrawingCommon::roadSignName(curFeature->getName())) ) {
            // to merge street notices if the scaleLevel is
            // "detailed street" and "part of block"+"block" level if not rc=4
            // and "district" level if not rc=3
            // if small image allow one more scale level
            streetnotices.push_back( notice );

         } else {
            if ( includeThisNotice ) {
               notices.push_back( notice );
            }
            // else
            // NOT including text notice (USA land feature string)
         }
      }
   }
   mc2dbg << "MapDrawer::initializeText: "
          << "creating featuretextnotice vector took "
          << TimeUtility::getCurrentTime() - startTime << " ms" << endl
          << " notices.size=" << notices.size() << " streetnotices.size="
          << streetnotices.size() << endl;

   // merge street features so there is more room to draw the text.
   if (streetnotices.size() > 0)
      mergeStreetFeatures(streetnotices, notices);

   // Sort the vector to be able to draw the feature texts in the
   // correct order
   sort( notices.begin(), notices.end(),
         ImportanceTextFeatureNoticeOrder() );

   initText( notices, objectBBoxes, featureMap, image,
             mapSettings, gfxTextArray, projection );

   mc2dbg << "MapDrawer::initializeText() took "
          << TimeUtility::getCurrentTime() - startTime << " ms" << endl;
   return true;
}

/// @return true if the feature should have its text split into 
///         two lines.
bool shouldSplitText( const GfxFeature& feature ) {
   // no street or poi features should use
   // text spliting in to two lines
   // and the text length have to be larger than 
   // 12 ( the magic number... )
   return 
      ! MapDrawingCommon::isStreetFeature( feature ) &&
      feature.getType() != GfxFeature::POI &&
      strlen( feature.getName() ) > 12;
}

bool useAltLangText( const GfxFeature& feature ) {
   return feature.getType() == GfxFeature::LAND;
}

bool isSmallImage( uint32 screenWidth ) {
   return screenWidth <= 250;
}


}
