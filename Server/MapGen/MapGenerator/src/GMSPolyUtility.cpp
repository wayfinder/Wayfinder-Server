/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSPolyUtility.h"
#include "OldGenericMap.h"
#include "PolygonDefects.h"
#include "GfxUtility.h"
#include "Stack.h"
#include "InsideUtil.h"
#include "NationalProperties.h"

#include "GMSMap.h"
#include "GMSWaterItem.h"
#include "TimeUtility.h"
#include "MapBits.h"
#include "Math.h"

uint32
GMSPolyUtility::rmPolygonDefectsAndUnnecessaryCoords( OldGenericMap* theMap )
{
   mc2dbg << "rmPolygonDefectsAndUnnecessaryCoords" << endl;
   uint32 nbrChanged = 0;
   
   uint32 startTime = TimeUtility::getCurrentTime();
   uint32 nbrClosedGfx = 0;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
         OldItem* item = theMap->getItem(z, i);
         if (item == NULL){
            continue;
         }

         GMSGfxData* gfx = item->getGfxData();
         if ( gfx == NULL ) {
            continue;
         }
         if ( gfx->getNbrPolygons()==1 && 
              gfx->getNbrCoordinates(0) <= 2 ) {
            continue;
         }
         
         // not closed polygons, simply remove identical coordinates
         if ( ! gfx->getClosed(0) ) {
            gfx->removeIdenticalCoordinates();
            continue;
         }

         // store item id in member variable for debug and error print
         GMSPolyUtility::myItemID = item->getID();
         GMSPolyUtility::myMapID = theMap->getMapID();

         // closed polygons, remove polygon defects
         nbrClosedGfx++;
         
         gfx->removeIdenticalCoordinates();
         GMSGfxData* newGfx = 
            rmPolygonDefectsAndUnnecessaryCoordsFromClosed( gfx );
         if ( newGfx != NULL ) {
            nbrChanged++;
            item->setGfxData( newGfx );
         }
      }
   }
   mc2dbg << "Nbr closed gfx=" << nbrClosedGfx 
          << " nbr changed=" << nbrChanged
          << ", took " << TimeUtility::getCurrentTime() - startTime 
          << " ms" << endl;

   return nbrChanged;
}

GMSGfxData*
GMSPolyUtility::rmPolygonDefectsAndUnnecessaryCoordsFromClosed( 
   GMSGfxData* gfx )
{
   if ( ! gfx->getClosed(0) ) {
      return NULL;
   }
   gfx->removeIdenticalCoordinates();
   
   GMSGfxData* newGfx = GMSGfxData::createNewGfxData(NULL);
   uint32 nbrNewPolys = 0;
   for (uint32 p=0; p < gfx->getNbrPolygons(); p++ ) {
      vector<MC2Coordinate> polygon;
      for (uint32 c = 0; c < gfx->getNbrCoordinates(p); c++ ){
         polygon.push_back(
            MC2Coordinate( gfx->getLat(p,c), gfx->getLon(p,c) ) );
      }
      
      vector<MC2Coordinate> output;   
      GfxUtil::removeSomePolygonDefects( output,
                                         polygon.begin(),
                                         polygon.end() );
      // If number of coords = 0, dont add polygon!
      if ( output.size() == 0 ) {
         mc2dbg1 << "Item " << GMSPolyUtility::myItemID
                 << " map " << GMSPolyUtility::myMapID
                 << " No coords in poly " << p << " after rmPolyDefects"
                 << endl;
      }
      if ( output.size() > 0 ) {
         newGfx->addPolygon();
         newGfx->addCoordinates( output.begin(), output.end() );
         newGfx->setClosed(nbrNewPolys, gfx->getClosed(p));
         nbrNewPolys++;
      }
   }

   // If the rm poly defects removed everything, return NULL
   // ( = keeps the original gfx data )
   if ( newGfx->getNbrPolygons() == 0 ) {
      return NULL;
   }
   
   // this check takes approx 25 percent of the total time
   if ( gfx->equals(newGfx) ) {
      return NULL;
   } else {
      mc2dbg8 << "changed item " << GMSPolyUtility::myItemID << " "
              << " nbrcoords " << gfx->getNbrCoordinates(0) 
              << " -> " << newGfx->getNbrCoordinates(0) << endl;
   }

   return newGfx;
}


void
GMSPolyUtility::eliminateSelfTouch( OldGenericMap* theMap,
                                    set<MC2Coordinate>&
                                       coordsAddedToReverseLine,
                                    ItemTypes::itemType elimItemType )
{
   mc2log << info << "GMSPolyUtility::eliminateSelfTouch for map " 
          << theMap->getMapID() 
          << " " << theMap->getMapName() << endl;

   // Eliminate self-touch
   uint32 startTime = TimeUtility::getCurrentTime();
   uint32 elimTime = 0;

   uint32 nbrGfxHadSelfTouchRemoved = 0;
   uint32 nbrGfx = 0;
   uint32 nbrCandidateGfx = 0;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
         OldItem* item = theMap->getItem(z, i);
         if (item == NULL){
            continue;
         }
         
         // Check item type
         ItemTypes::itemType type = item->getItemType();
         if (type == ItemTypes::municipalItem) {
            continue;
         }
         if ( (elimItemType != ItemTypes::numberOfItemTypes) &&
              (type != elimItemType) ) {
            continue;
         }
         
         // store item id in member variable for debug and error print
         GMSPolyUtility::myItemID = item->getID();
         GMSPolyUtility::myMapID = theMap->getMapID();

         GMSGfxData* gfx = item->getGfxData();
         if ( gfx == NULL ) {
            continue;
         }
         nbrGfx++;
         if ( ! gfx->getClosed(0) ) {
            continue;
         }

         if ( gfx->getNbrPolygons()==1 && 
              gfx->getNbrCoordinates(0) <= 2 ) {
            continue;
         }
         nbrCandidateGfx++;

         
         mc2dbg8 << "Item " << theMap->getMapID() << ":" << item->getID() 
              << " nbrPoly=" << gfx->getNbrPolygons()
              << " " << ItemTypes::getItemTypeAsString(item->getItemType())
              << " " << theMap->getFirstItemName(item) << endl;
         
         uint32 tmpTime = TimeUtility::getCurrentTime();
         uint32 totNbrCoords = gfx->getTotalNbrCoordinates();
         bool hasProblems = false;
         GMSGfxData* newGfx = eliminateSelfTouch( gfx, 
                                                  hasProblems,
                                                  coordsAddedToReverseLine );
         if( hasProblems ) {
            // Do not change this print, used for error detection
            // in the makemaps logs
            mc2log << error << here << " FCR_ELIMST: "
                   << " map " << GMSPolyUtility::myMapID << " item "
                   << GMSPolyUtility::myItemID
                   << " item type: " << int(elimItemType)
                   << endl;
         } 
         elimTime += TimeUtility::getCurrentTime() - tmpTime;

         if ( newGfx != NULL ) {
            item->setGfxData( newGfx );
            nbrGfxHadSelfTouchRemoved++;
            
            mc2dbg << "Item " << theMap->getMapID() << ";" << item->getID()
                 << " " << ItemTypes::getItemTypeAsString(item->getItemType())
                 << " " << theMap->getFirstItemName(item) 
                 << ", self-touch eliminated"
                 << " nbrPoly " << gfx->getNbrPolygons()
                 << "->" << newGfx->getNbrPolygons()
                 << " nbrCoords " << totNbrCoords
                 << "->" << newGfx->getTotalNbrCoordinates()
                 << " took " << TimeUtility::getCurrentTime() - tmpTime
                 << " ms" << endl;
         }

      }
   }
   
   mc2log << info << "eliminateSelfTouch nbrGfx=" << nbrGfx 
          << " nbrCandidateGfx=" << nbrCandidateGfx
          << " nbrGfxHadSelfTouchRemoved=" 
          << nbrGfxHadSelfTouchRemoved
          << ", took " << TimeUtility::getCurrentTime() - startTime 
          << " ms" << " (" << elimTime << " ms)"
          << endl;
} // eliminateSelfTouch

void
GMSPolyUtility::eliminateHoles( OldGenericMap* theMap,
                                set<MC2Coordinate>& coordsAddedToReverseLine,
                                ItemTypes::itemType elimItemType )
{
   mc2log << info << "GMSPolyUtility::eliminateHoles for map " 
          << theMap->getMapID() 
          << " " << theMap->getMapName() << endl;


   // 1. build poly hierarchy
   // 2. elim holes
   uint32 startTime = TimeUtility::getCurrentTime();
   uint32 elimTime = 0;
   uint32 nbrGfx = 0;
   uint32 nbrClosedMultiPolyGfx = 0;
   uint32 nbrGfxHadHolesRemoved = 0;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
         OldItem* item = theMap->getItem(z, i);
         if (item == NULL){
            continue;
         }
         
         // Check item type
         ItemTypes::itemType type = item->getItemType();
         if (type == ItemTypes::municipalItem) {
            continue;
         }
         if ( (elimItemType != ItemTypes::numberOfItemTypes) &&
              (type != elimItemType) ) {
            continue;
         }
         
         // store item id in member variable for debug and error print
         GMSPolyUtility::myItemID = item->getID();
         GMSPolyUtility::myMapID = theMap->getMapID();

         GMSGfxData* gfx = item->getGfxData();
         if ( gfx == NULL ) {
            continue;
         }
         nbrGfx++;
         if ( ! gfx->getClosed(0) ) {
            continue;
         }
         if ( gfx->getNbrPolygons()==1 ) {
            continue;
         }
         nbrClosedMultiPolyGfx++;
         
         // For debug
         /*
         // Items to examine
         //uint32 itemID = 134222462; //forest map 0x18, line wrong direction
         uint32 itemID = 134219084; // some water in 0x6
         if (item->getID() != itemID ){
            //continue;
         }
         */
         
         mc2dbg8 << "Item " << theMap->getMapID() << ":" << item->getID() 
              << " nbrPoly=" << gfx->getNbrPolygons()
              << " " << ItemTypes::getItemTypeAsString(item->getItemType())
              << " " << theMap->getFirstItemName(item) << endl;
         
         
         uint32 tmpTime = TimeUtility::getCurrentTime();
         uint32 totNbrCoords = gfx->getTotalNbrCoordinates();
         bool hasProblems = false;
         GMSGfxData* newGfx = eliminateHoles( gfx, 
                                              hasProblems, 
                                              coordsAddedToReverseLine);
         if( hasProblems ) {
            // Do not change this print, used for error detection
            // in the makemaps logs
            mc2log << error << here << " FCR_FALU: "
                   << " map " << GMSPolyUtility::myMapID << " item "
                   << GMSPolyUtility::myItemID
                   << " item type: " << int(elimItemType)
                   << endl;
         }
         elimTime += TimeUtility::getCurrentTime() - tmpTime;

         if ( newGfx != NULL ) {
            item->setGfxData( newGfx );
            nbrGfxHadHolesRemoved++;
            
            mc2dbg << "Item " << theMap->getMapID() << ":" << item->getID()
                 << " " << ItemTypes::getItemTypeAsString(item->getItemType())
                 << " " << theMap->getFirstItemName(item) 
                 << ", holes eliminated"
                 << " nbrPoly " << gfx->getNbrPolygons()
                 << "->" << newGfx->getNbrPolygons()
                 << " nbrCoords " << totNbrCoords
                 << "->" << newGfx->getTotalNbrCoordinates()
                 << " took " << TimeUtility::getCurrentTime() - tmpTime
                 << " ms" << endl;
         }

      }
   }
   mc2log << info << "eliminateHoles nbrGfx=" << nbrGfx 
          << " nbrClosedMultiPolyGfx=" << nbrClosedMultiPolyGfx
          << " nbrGfxHadHolesRemoved=" << nbrGfxHadHolesRemoved
          << ", took " << TimeUtility::getCurrentTime() - startTime 
          << " ms" << " (" << elimTime << " ms)"
          << endl;
} // eliminateHoles

GMSGfxData*
GMSPolyUtility::eliminateHoles( GMSGfxData* gfx, 
                                bool &hasProblems,
                                set<MC2Coordinate>& coordsAddedToReverseLine )
{
   // ***************************************************
   // *********** build poly hierarchy ******************

   hasProblems = false;
   multimap<uint32, uint32> holeHierarchy;
   bool holeHierarchyOK = getPolyHoleHierarchy(gfx, holeHierarchy,   
                                               hasProblems);
   if ( holeHierarchyOK ) {
      // ok
   } else { 
      // no holes in this poly
      return NULL;
   }


   // ***************************************************
   // **************** elim holes ***********************

   // find which polygons have holes
   multimap<uint32, uint32>::const_iterator hh;
   set<uint32> polysWithHoles;
   for ( hh = holeHierarchy.begin();
         hh != holeHierarchy.end(); hh++ ) {
      polysWithHoles.insert( hh->first );
   }
   mc2dbg8 << "polysWithHoles=" << polysWithHoles.size() << endl;
   if ( polysWithHoles.size() == 0 ) {
      return NULL;
   }


   // If some holes are touching each other, the algorithm for
   // hole elimination might result in cases that are impossible to
   // solve in the self-touch elimination
   // Detect if any holes of this gfx data are touching each other,
   // if they do return NULL
   bool holesAtTouching = holesAreTouching(gfx, holeHierarchy);
   if ( holesAtTouching ) {
      mc2log << error << here << " FCR  map " << GMSPolyUtility::myMapID 
             << " item " << GMSPolyUtility::myItemID 
             << " has holes touching" << endl;
      hasProblems = true;
      return NULL;
   }

   

   // Set of holes, fill it with info when a hole is eliminated
   set<uint32> holesEliminated; 

   uint32 convHullNbr = 0;
   set<uint32>::const_iterator p;
   for ( p = polysWithHoles.begin();
         p != polysWithHoles.end(); p++ ) {
      uint32 poly = *p;
      MC2BoundingBox polyBbox;
      gfx->getMC2BoundingBox(polyBbox, poly);
      mc2dbg4 << "Eliminate holes for poly " << poly
              << " nbrCoords=" << gfx->getNbrCoordinates(poly)
              << " " << polyBbox << endl;
      

      bool allHolesOfPolyEliminated = false;
      uint32 nbrHolesOfPolyEliminated = 0;
      uint32 holeElimRound = 0;
      // BEGIN ----- Repeat until no holes remain -----
      while ( ! allHolesOfPolyEliminated ) {
         holeElimRound++;
         
         // 1. create convex hull of all my holes
         //   (do not include holes that was already eliminated = added
         //    to the polygon boundry)
         GMSGfxData* convHull = 
            createConvexHullOfHolesOfOnePoly( gfx,
               poly, holeHierarchy, holesEliminated );
         mc2dbg8 << "convHull nbrCoords="
                 << ( convHull ?  int(convHull->getNbrCoordinates(0)) : -1 )
                 << endl;
         if ( convHull == NULL ) {
            mc2log << error << here << " FCR "
                   << " map " << GMSPolyUtility::myMapID << " item "
                   << GMSPolyUtility::myItemID
                   << " poly " << poly << " holeElimRound=" << holeElimRound
                   << " CONV HULL NULL" << endl;
            hasProblems = true;
            return NULL;
            // Return NULL instead of assert not to fail map gen
            //MC2_ASSERT(false);
         }
         convHullNbr++;
         // possible to print the convhull to mif
         /*
         char tmpstr[128];
         sprintf(tmpstr, "convHull_%d.mif", convHullNbr);
         ofstream fileX( tmpstr );
         convHull->printMif( fileX );
         */

         // 2. loop all my holes
         // a. Find for each hole a coord that is on the convex hull
         //    From this coord the line to the border should be drawn.
         //    Do some fillCoordMap with the convHull + the hole
         //    to detect the common coords for the hole
         // b. Draw the line (find the end coordinate)
         //    line length =  max(height,width) of the poly bbox
         // c. Find the pairs of coords on poly border that intersects
         //    with the line. Then select the closest intersection.
         // d. Eliminate the hole, by adding the intersection coord
         //    and the hole coords to the poly border
         //    (creating a self-touch)
         // e. Mark the hole as eliminated, possible update other params
         //    to correctly represent the changes in the gfx data
         //
         
         
         // 2.
         coordMap_t convHullCoords;
         GMSGfxData::fillCoordMap( convHull, 0, convHullCoords, 0);
         mc2dbg8 << "holeElimRound=" << holeElimRound
                 << " convHullCoords.size=" << convHullCoords.size() << endl;
         
         uint32 holeNumber = 0;
         for ( hh = holeHierarchy.lower_bound(poly);
               hh != holeHierarchy.upper_bound(poly); hh++ ) {
            
            uint32 hole = hh->second;
            holeNumber++;
            mc2dbg8 << " Hole holeNumber=" << holeNumber
                 << " / hole=" << hole << " holeNbrCoords=" 
                 << gfx->getNbrCoordinates(hole) << endl;
         
            set<uint32>::const_iterator 
               holeIter = holesEliminated.find(hole);
            if ( holeIter != holesEliminated.end() ) {
               mc2dbg8 << "  Hole already eliminated - skip" << endl;
               continue;
            }


            // 2.a. Find a hole coord that is on the holeConvHull
            
            // Add all coords for this hole into the map with the
            // coords of the conv hull
            coordMap_t hullAndHoleCoords = convHullCoords;
            GMSGfxData::fillCoordMap(
               gfx, hole, hullAndHoleCoords, holeNumber);

            // Find one coord that is shared between the convHull 
            // and the hole
            // (this always finds the most southern coord since 
            //  the set is sorted on latitude value)
            bool foundCoordOnHull = false;
            MC2Coordinate coordOnHull;
            uint32 convHullCoordIdx = MAX_UINT32;
            uint32 holeCoordIdx = MAX_UINT32;
            coordMap_t::const_iterator it = hullAndHoleCoords.begin();
            while ( !foundCoordOnHull && 
                     (it != hullAndHoleCoords.end()) ) {
               idxSet_t idxSet = it->second;
               if ( idxSet.size() == 2 ) {
                  foundCoordOnHull = true;
                  coordOnHull = it->first;
                  for ( idxSet_t::const_iterator id = idxSet.begin();
                        id != idxSet.end(); id++ ) {
                     if ( id->first == 0 ) {
                        convHullCoordIdx = id->second;
                     } else if ( id->first == holeNumber ) {
                        holeCoordIdx = id->second;
                     }
                  }

               } else if (idxSet.size() > 2 ) {
                  mc2log << error << here << " FCR "
                         << " map " << GMSPolyUtility::myMapID << " item "
                         << GMSPolyUtility::myItemID
                         << " More than 2 coordIdx has this coord "
                         << it->first 
                         << " in hole and convex hull of holes" << endl;
                  hasProblems = true;
                  return NULL;
                  // Return NULL instead of assert not to fail map gen
                  //MC2_ASSERT(false);
               } else {
                  it++;
               }
            }
            if ( foundCoordOnHull ) {
               mc2dbg8 << " coordOnHull = " << coordOnHull.lat
                    << ";" << coordOnHull.lon
                    << " convHullCoordIdx=" << convHullCoordIdx
                    << " holeCoordIdx=" << holeCoordIdx << endl;
            } else {
               mc2dbg8 << " Did not find coordOnHull for hole" << endl;
               continue;
            }

            // 2.b.
            // From the coord holeCoordIdx of hole (coordOnHull)
            // we should draw a line to the border of gfx poly
            // Line length =  max(height,width) of the poly bbox

            // Pic a direction from a start point to coordOnHull
            // = to convHullCoordIdx on the conv hull
            // 
            // Draw a baseline from convHullCoordIdx-1 to convHullCoordIdx+1
            // Find the point on that base line that is closest to
            // convHullCoordIdx. This point is the start point.

            // Normalize the vector from the start point to convHullCoordIdx
            // From convHullCoordIdx, add the normVec*length
            // (make sure the toCoord is outside the poly)
            
            int32 endLat; int32 endLon;
            bool lineOK = drawLineTowardsPolyBorder( 
                  convHull, coordOnHull, convHullCoordIdx, polyBbox,
                  endLat, endLon );
            if ( ! lineOK ) {
               mc2log << error << here << " FCR "
                      << " map " << GMSPolyUtility::myMapID << " item "
                      << GMSPolyUtility::myItemID
                      << " Failed to create line towards poly border"
                      << endl;
               // possible to print the convhull to mif
               //char tmpstr[128];
               //sprintf(tmpstr, "__convHull_%d_%d.txt", 
               //         GMSPolyUtility::myMapID, GMSPolyUtility::myItemID);
               //ofstream fileX( tmpstr );
               //convHull->printMif( fileX );

               hasProblems = true;
               return NULL;
               // Return NULL instead of assert not to fail map gen
               //MC2_ASSERT(false);
            }
            mc2dbg8 << " line from " << coordOnHull << " to " << endLat
                    << ";" << endLon << " (convHullNbr=" << convHullNbr
                    << ")" << endl;
            

            
            // 2.c. Find the (closest to coordOnHull) intersection point
            // between the line and the poly border

            MC2Coordinate interCoord;
            uint32 nextCoordIdx = MAX_UINT32;
            uint32 reversedLineNextCoordIdx = MAX_UINT32;
            bool interSectionFound =
               findPolyBorderIntersectionPoint( gfx, poly, 
                  coordOnHull, endLat, endLon,
                  interCoord, nextCoordIdx, reversedLineNextCoordIdx );
            if ( ! interSectionFound ) {
               MC2Coordinate newCoordOnHull( coordOnHull.lat,
                                             coordOnHull.lon );
               // Try moving coordOnHull north, east, south west
               // while maintaining it within poly. Tries to solve
               // the case where an intersection could not be found
               // because coordOnHull touched the poly border.
               
               int i = 0;
               while( !interSectionFound && i < 4) {

                  // reset newCoordOnHull
                  newCoordOnHull.lat = coordOnHull.lat;
                  newCoordOnHull.lon = coordOnHull.lon;

                  // Move north, east, south or north
                  if( i == 0 ) {
                     newCoordOnHull.lat++;
                  } else if ( i == 1 ) {
                     newCoordOnHull.lat--;
                  } else if ( i == 2 ) {
                     newCoordOnHull.lon++; 
                  } else {
                     newCoordOnHull.lon--; 
                  }
            
                  // Check if 1) newCoordOnHull is inside poly. Also check 
                  // 2) that it's outside convHull, and 3) that the old line 
                  // direction from newCoordOnHull doesn't overlap old 
                  // convex hull. If these things hold, try computing new
                  // intersection.
                  bool newCoordOnHullInsidePolygon = false;
                  bool newCoordOnHullOutsideOldConvexHull = false;

                  // 1) inside poly?
                  newCoordOnHullInsidePolygon = gfx->insidePolygon( 
                     newCoordOnHull.lat, 
                     newCoordOnHull.lon,
                     poly );

                  // 2) outside old convex hull? Has to be completely outside
                  // in order not to risk overlap of other coordinates.
                  if( convHull->insidePolygon( newCoordOnHull.lat, 
                                               newCoordOnHull.lon,
                                               0 ) == 0 ) {
                     newCoordOnHullOutsideOldConvexHull = true;
                  }

                  // 3) Line from newCoordOnHull doesn't overlap old
                  // convex hull?

                  // first find prev and next on convexHull
                  
                  uint32 nbrCoordsInConvHull = convHull->getNbrCoordinates(0);
                  int32 prevConvHullCoordIdx = convHullCoordIdx-1;
                  if ( prevConvHullCoordIdx < 0 ) {
                     prevConvHullCoordIdx = nbrCoordsInConvHull-1;
                  }

                  uint32 nextConvHullCoordIdx = convHullCoordIdx+1;
                  if ( nextConvHullCoordIdx >= nbrCoordsInConvHull ) {
                     nextConvHullCoordIdx = 0;
                  }


                  // Create temporary gfx data and store the segments
                  // that we want to check for overlapping
                  GMSGfxData* tmpGfx = NULL;
                  tmpGfx = GMSGfxData::createNewGfxData(NULL, true);
                
                  // Segments from conv hull (coordidxs: 0, 1, 2)
                  tmpGfx->addCoordinate(
                     convHull->getLat( 0, prevConvHullCoordIdx ),
                     convHull->getLon( 0, prevConvHullCoordIdx ) );
                  
                  tmpGfx->addCoordinate(
                     coordOnHull.lat,
                     coordOnHull.lon );
                  
                  tmpGfx->addCoordinate(
                     convHull->getLat( 0, nextConvHullCoordIdx ),
                     convHull->getLon( 0, nextConvHullCoordIdx ) );

                  // New line (coordidxs 3, 4)
                  tmpGfx->addCoordinate(
                     newCoordOnHull.lat,
                     newCoordOnHull.lon );

                  tmpGfx->addCoordinate(
                     endLat,
                     endLon );

                  // Check if we have overlap
                  bool newLineOverlapsConvHull = false;
                  newLineOverlapsConvHull = tmpGfx->segmentIntersect(
                     0, 0, 1, 3, 4);
                  
                  newLineOverlapsConvHull = tmpGfx->segmentIntersect(
                     0, 1, 2, 3, 4);

                  // Does 1), 2) and 3) hold? If so, try to compute
                  // new intersection.
                  if( newCoordOnHullInsidePolygon &&
                      newCoordOnHullOutsideOldConvexHull &&
                      !newLineOverlapsConvHull ) {
                     interSectionFound =
                        findPolyBorderIntersectionPoint( gfx, poly, 
                           newCoordOnHull, endLat, endLon,
                           interCoord, nextCoordIdx, 
                           reversedLineNextCoordIdx );
                  }

                  i++;
               }
               
               if ( interSectionFound ) {
                  
                  // Change geometry to the newCoordOnHull
                  coordOnHull.lat = newCoordOnHull.lat;
                  coordOnHull.lon = newCoordOnHull.lon;
                  
                  gfx->setCoordinate( hole, 
                                      holeCoordIdx, 
                                      newCoordOnHull.lat, 
                                      newCoordOnHull.lon );

                  mc2log << info << here << " FCR "
                         << " map " << GMSPolyUtility::myMapID << " item "
                         << GMSPolyUtility::myItemID 
                         << " Had to change geometry for " << poly 
                         << " hole " << hole 
                         << " holeCoordIdx " << holeCoordIdx 
                         << " in order to find line intersection" << endl;
               } else { // we have a serious problem
                  mc2log << error << here << " FCR "
                         << " map " << GMSPolyUtility::myMapID << " item "
                         << GMSPolyUtility::myItemID 
                         << " No intersection found for poly " << poly 
                         << " hole " << hole 
                         << " coordOnHull " << coordOnHull
                         << " end " << endLat << ";" << endLon << endl;
                  //char tmpstr[128];
                  //sprintf(tmpstr, "__convHull_%d_%d.txt", 
                  //         GMSPolyUtility::myMapID, GMSPolyUtility::myItemID);
                  //ofstream fileX( tmpstr );
                  //convHull->printMif( fileX );
                  
                  mc2dbg << "Calls findPolyBorderIntersectionPoint "
                         << "again with printDebug = true:" << endl;
                  findPolyBorderIntersectionPoint( gfx, poly, 
                     coordOnHull, endLat, endLon,
                     interCoord, nextCoordIdx, 
                     reversedLineNextCoordIdx, true); // print info
                  hasProblems = true;
                  return NULL;
                  // Return NULL instead of assert not to fail map gen
                  //MC2_ASSERT(false);
               }
            }
            mc2dbg8 << " intersection nextCoordIdx=" << nextCoordIdx
                    << " nextCoord=" << gfx->getLat(poly,nextCoordIdx)
                    << ";" << gfx->getLon(poly,nextCoordIdx)
                    << ", reversedLineNextCoordIdx="
                    << reversedLineNextCoordIdx
                    << ", interCoord=" << interCoord << endl;

            // If the poly border in this case is an old line previously
            // drawn to the true poly border, my current line has two
            // intersction points closest to the coordOnHull. The lines
            // has different directions. We want to eliminate the hole by
            // adding it to the border before coord nextCoordIdx.
            // To keep the shared coords on both lines, we want to add
            // the interCoord also on the reversed line, that is before
            // coord reversedLineNextCoordIdx.
            // Check here in which order to do the inserts.

            bool insertReversedBeforeHoleElim = false;
            bool insertReversedAfterHoleElim = false;
            if ( reversedLineNextCoordIdx != MAX_UINT32 ) {
               mc2dbg8 << " reversedLineNextCoordIdx " 
                       << reversedLineNextCoordIdx
                       << gfx->getLat(poly,reversedLineNextCoordIdx)
                       << ";" << gfx->getLon(poly,reversedLineNextCoordIdx)
                       << endl;
               if ( nextCoordIdx < reversedLineNextCoordIdx ) {
                  insertReversedBeforeHoleElim = true;
               }
               else if ( nextCoordIdx > reversedLineNextCoordIdx ) {
                  insertReversedAfterHoleElim = true;
               }
            }


            // 2.d. Eliminate the hole, by adding the intersection coord
            // and the hole coords to the poly border (creating a self-touch)
            // I simply want to insert the coords between certain coordIdxes
            // of the gfxdata poly.
            //
            // Start by adding the interCoord, followed by
            // the hole coords starting+ending with holeCoordIdx
            // Add the hole backwards to be able to keep
            // the nextCoordIdx of the poly constant
            // Add the interCoord to the reversed line if booleans says so

            // For the case where the poly border is an old line previously 
            // drawn to the original border, keep track of coordinates added
            // to the reversed line (later used in eliminateSelfTouches).

            uint32 startNumberCoords = gfx->getNbrCoordinates(poly);
            if ( insertReversedBeforeHoleElim ) {
               gfx->addCoordinateBeforeSpecificCoordIdx(
                  interCoord.lat, interCoord.lon,
                  poly, reversedLineNextCoordIdx);

               coordsAddedToReverseLine.insert( 
                  MC2Coordinate( interCoord.lat, interCoord.lon ) );
               mc2dbg8 << "Coord added to reversed line: " 
                      << "lat: " << interCoord.lat
                      << "lon: " << interCoord.lon
                      << endl;
            }
            bool insertOK =
               gfx->addCoordinateBeforeSpecificCoordIdx(
                  interCoord.lat, interCoord.lon,
                  poly, nextCoordIdx);
            insertOK =
               gfx->addCoordinateBeforeSpecificCoordIdx(
                  gfx->getLat(hole, holeCoordIdx),
                  gfx->getLon(hole, holeCoordIdx),
                  poly, nextCoordIdx);

            bool done = false;
            int32 addHoleCoordIdx = holeCoordIdx;
            while ( !done ) {
               addHoleCoordIdx--;
               if ( addHoleCoordIdx < 0 ) {
                  addHoleCoordIdx = gfx->getNbrCoordinates(hole)-1;
               }
               mc2dbg8 << "   adding hole coord " << addHoleCoordIdx
                    << " " << gfx->getLat(hole, addHoleCoordIdx)
                    << ";" << gfx->getLon(hole, addHoleCoordIdx)
                    << endl;
               
               gfx->addCoordinateBeforeSpecificCoordIdx(
                  gfx->getLat(hole, addHoleCoordIdx),
                  gfx->getLon(hole, addHoleCoordIdx),
                  poly, nextCoordIdx);
               if ( addHoleCoordIdx == int(holeCoordIdx) ) {
                  done = true;
               }
            }
            // Then add the interCoord again
            insertOK =
               gfx->addCoordinateBeforeSpecificCoordIdx(
                  interCoord.lat, interCoord.lon,
                  poly, nextCoordIdx);
            if ( insertReversedAfterHoleElim ) {
               gfx->addCoordinateBeforeSpecificCoordIdx(
                  interCoord.lat, interCoord.lon,
                  poly, reversedLineNextCoordIdx);
   
               coordsAddedToReverseLine.insert( 
                     MC2Coordinate( interCoord.lat, interCoord.lon ) );
               
               mc2dbg8 << "Coord added to reversed line: " 
                         << "lat: " << interCoord.lat
                         << "lon: " << interCoord.lon
                         << endl;
            }
            
    
            // 2.e. 
            // Mark the hole as eliminated
            // (insert into the holesEliminated set) 
            // need to keep poly Idxes etc for the continuation
            // of hole elim from all polys of this gfxdata.

            holesEliminated.insert( hole );
            nbrHolesOfPolyEliminated++;
            mc2dbg4 << "  Hole " << hole << " eliminated in round "
                 << holeElimRound << ", "
                 << "nbr coords in poly " << startNumberCoords
                 << " -> " << gfx->getNbrCoordinates(poly)
                 << " (now totally " << nbrHolesOfPolyEliminated << " of "
                 << holeHierarchy.count(poly) << " eliminated for poly "
                 << poly << ")" << endl;
            
         }
         
         
         if ( nbrHolesOfPolyEliminated == holeHierarchy.count(poly) ) {
            mc2dbg8 << " ALL holes eliminated for poly " << poly << endl;
            allHolesOfPolyEliminated = true;
         } else {
            mc2dbg8 << " MORE holes to eliminated for poly " << poly
                 << " = " 
                 << ( holeHierarchy.count(poly) - nbrHolesOfPolyEliminated )
                 << endl;
         }
         
      } // while allHolesOfPolyEliminated
      // END ----- Repeat until no holes remain -----

   }


   // Now I can remove the hole polygons that was eliminated
   // Create a new gfx data
   // Loop the polys of my gfx from the last to the first
   // If the poly is a hole that was eliminated - remove it!
   GMSGfxData* newGfx = GMSGfxData::createNewGfxData(NULL);
   set<uint32>::const_iterator holeIter;
   for ( uint32 polyNbr = 0;
         polyNbr < gfx->getNbrPolygons(); polyNbr++ ) {
      holeIter = holesEliminated.find(polyNbr);
      if ( holeIter == holesEliminated.end() ) {
         mc2dbg8 << "keep poly " << polyNbr << endl;
         if ( ! newGfx->addPolygon( gfx, false, polyNbr) ) {
            mc2log << error << here << " FCR "
                   << " map " << GMSPolyUtility::myMapID << " item "
                   << GMSPolyUtility::myItemID 
                   << " Problem adding gfx poly " << polyNbr 
                   << " to new gfx" << endl;
            hasProblems = true;
            return NULL;
            // Return NULL instead of assert not to fail map gen
            //MC2_ASSERT(false);
         }
         newGfx->setClosed( 
            newGfx->getNbrPolygons()-1, gfx->getClosed(polyNbr));
      } else {
         mc2dbg8 << "remove poly " << polyNbr << " = hole" << endl;
      }
   }
   newGfx->removeIdenticalCoordinates();
   newGfx->sortPolygons();
   newGfx->updateLength();

   return newGfx;
} // eliminateHoles in one gfx data

GMSGfxData*
GMSPolyUtility::eliminateSelfTouch( GMSGfxData* origGfx, 
                                    bool& hasProblems,
                                    set<MC2Coordinate>& coordsAddedToReverseLine )
{
   hasProblems = false;
   uint32 startTime = TimeUtility::getCurrentTime();
   // remove identical coords before starting
   origGfx->removeIdenticalCoordinates();
   
   // Create a temporary gfx data and work on that. 
   // If anything changed in the end (notified by number of polygons 
   // in the gfx data), return the temporary gfx with self-touches 
   // eliminated, else return null
   GMSGfxData* gfx  = GMSGfxData::createNewGfxData(NULL, origGfx);
   uint32 origNbrPolygons = origGfx->getNbrPolygons();
   mc2dbg8 << "gfx originally has " << origNbrPolygons << " polygons" << endl;

   // Loop all polys backwards, and eliminate the self-touch-parts in them
   // Each elimination will split the polygon in 2 parts. One part will
   // replace the original polygon in the gfxdata, the other will be added
   // as a new polygon to (the end of) the gfxdata.
   // If a poly has no self-touch-part, mark it as done.
   // Repeat, until no poly has any self-touch-parts.
   set<uint16> polysWithoutSelfTouch;
   set<uint16>::const_iterator pwstIT;
   bool allSTremoved=false;

   uint32 nbrLoops = 0;
   while ( ! allSTremoved ) {
      nbrLoops++;
      mc2dbg8 << "Loop " << nbrLoops << " time="
              << TimeUtility::getCurrentTime() - startTime << " ms" << endl;
      uint16 nbrPolygons = gfx->getNbrPolygons();
      for ( int16 p = nbrPolygons-1; p >= 0; p-- ) {
         
         int clockWise = gfx->clockWise(p);
         mc2dbg8 << "poly " << p << " clockWise = " << clockWise << endl;
         if ( clockWise < 1 ) {
            // Might be a polygon that failed on eliminateHoles
            // so there are holes remaining
            mc2log << error << here << " FCR "
                   << " map " << GMSPolyUtility::myMapID << " item "
                   << GMSPolyUtility::myItemID 
                   << " Problem with clockwise in loop " 
                   << nbrLoops << " poly=" << p << " firstCoord=" 
                   << gfx->getLat(p,0) << ";" << gfx->getLon(p,0)
                   << endl;
            // We have no solution for this now - return NULL 
            // means that we will not eliminate any self-touch at all 
            // from this gfxdata
            hasProblems = true;
            return NULL;
            // this below is for debug purposes
            /*
            GMSGfxData* tmpGfx = GMSGfxData::createNewGfxData(NULL);
            tmpGfx->addPolygon(gfx, false, p); // not backwards
            tmpGfx->setClosed(0, gfx->getClosed(p));
            createAndSaveAMapFromGfxData( tmpGfx, 7777 );
            //MC2_ASSERT(false);
            */
         }
         pwstIT = polysWithoutSelfTouch.find(p);
         if ( pwstIT != polysWithoutSelfTouch.end() ) {
            mc2dbg8 << " poly " << p
                    << " has no more self-touch-parts to eliminate - skip"
                    << endl;
            continue;
         }
         else {
            mc2dbg8 << " poly " << p
                    << " may have self-touch-parts to eliminate" << endl;
         }

         // Check if gfx is self touching in polygon p
         // and collect the coord indexes and coords that are shared
         coordMap_t selfTouchingCoords;
         coordMap_t::const_iterator cit;
         bool isST = isSelfTouching( gfx, selfTouchingCoords, p );
         
         map<uint32, MC2Coordinate> selfTouchingCoordIdxs;
         map<uint32, MC2Coordinate>::const_iterator iit;
         if ( ! isST ) {
            // Poly is not self-touching or had some problems
            // - skip it now and forever
            polysWithoutSelfTouch.insert(p);
            mc2dbg8 << " poly " << p << " is not ST" << endl;
            continue;
         } else {
            // check that the result make sense, only 2 coord indexes may
            // share one corod for it to be correct.
            for ( cit = selfTouchingCoords.begin();
                  cit != selfTouchingCoords.end(); cit++) {
               idxSet_t idxSet = cit->second;
               if ( idxSet.size() != 2 ) {
                  if( idxSet.size() == 3 && 
                      coordsAddedToReverseLine.count( cit->first ) ) {  
                     // Then we're ok, the extra coordinate was added in 
                     // eliminate holes to maintain geometry for the 
                     // reversed line.
                  } else {
                     mc2log << error << here << " FCR "
                            << " map " << GMSPolyUtility::myMapID << " item "
                            << GMSPolyUtility::myItemID
                            << " Coord " << cit->first
                            << " is used " << idxSet.size()
                            << " times in poly " << p << endl;
                     hasProblems = true;
                     return NULL;
                     // return NULL not to fail map gen
                     //MC2_ASSERT(false); 
                  }
               }
               idxSet_t::const_iterator sit;
               for ( sit = idxSet.begin(); sit != idxSet.end(); sit++ ) {
                  selfTouchingCoordIdxs.insert(
                        make_pair( sit->second, cit->first));
               }
            }
         }
         uint32 nbrCoordsInPoly = gfx->getNbrCoordinates(p);
         mc2dbg8 << " nbrCoordsInPoly=" << nbrCoordsInPoly << endl;
         if ( selfTouchingCoordIdxs.size() == 0 ) {
            mc2log << error << here << " FCR "
                   << " map " << GMSPolyUtility::myMapID << " item "
                   << GMSPolyUtility::myItemID
                   << " poly " << p << "Problem " << endl;
            hasProblems = true;
            return NULL;
            // return NULL not to fail map gen
            //MC2_ASSERT(false); 
         }
         DEBUG8(
         cout << " poly has " << selfTouchingCoordIdxs.size()
              << " (of " << nbrCoordsInPoly << ")"
              << " coord idxs that share coord with someone" << endl;
         for ( iit = selfTouchingCoordIdxs.begin();
               iit != selfTouchingCoordIdxs.end(); iit++ ) {
            cout << " " << iit->first;
         }
         cout << endl; 
         );


         // Find a good start coord for this polygon, one that is on 
         // the border of the poly (go with a point on the conv hull of 
         // the poly, that is for sure part of the border)
         uint32 startCoordOnBorderIdx = getPointOnConvexHull( gfx, p );
         mc2dbg8 << " start coord " << startCoordOnBorderIdx << " "
                 << gfx->getLat(p,startCoordOnBorderIdx) << ";"
                 << gfx->getLon(p,startCoordOnBorderIdx) << endl;
         // Start here and go around to find a coordIdx that is shared
         // That will be a point on the polygon border.
         uint32 borderCoordIdx = startCoordOnBorderIdx;
         bool shared = false;
         while ( ! shared ) {
            iit = selfTouchingCoordIdxs.find( borderCoordIdx );
            if ( iit != selfTouchingCoordIdxs.end() ) {
               shared = true;
            } else {
               borderCoordIdx++;
               if ( borderCoordIdx >= nbrCoordsInPoly ) {
                  borderCoordIdx = 0;
               }
            }
         }
         if ( ! shared ) {
            // this cannot happen?
            cout << " poly " << p << " is not ST" << endl;
            mc2log << error << here << " FCR "
                   << " map " << GMSPolyUtility::myMapID << " item "
                   << GMSPolyUtility::myItemID
                   << "No shared coord in poly " << p << endl;
            hasProblems = true;
            return NULL;
            // return NULL not to fail map gen
            //MC2_ASSERT(false); 
         }

         // Now we have one coord on the polygon border that is shared
         mc2dbg8 << " first shared coord " << borderCoordIdx << " "
                 << selfTouchingCoordIdxs.find(borderCoordIdx)->second 
                 << endl;
         
         // BEGIN ----- findFirstSelfTouchPart ---------
         // while loop: search for self-touch until we have one or search 
         // until we are back at the original borderCoordIdx
         // - or stop at startCoordOnBorderIdx ??? (in that case also below)
         // -----------------------------------------------------------
         // Find a self-touch-part
         // Go forward from borderCoordIdx, compare with the shared in 
         // selfTouchingCoords
         // Save the prevCoord all the time until we have two shared coordIdxs
         // that points to the same coord. Then we have identified the
         // start and end of a self-touch-part.
         // Need to check that the self-touch-part is really a hole. If it is
         // a positive poly we do not need to eliminate it.
         uint32 firstSelfTouchCoord = borderCoordIdx;
         uint32 secondSelfTouchCoord = borderCoordIdx+1;
         if ( secondSelfTouchCoord >= nbrCoordsInPoly ) {
            secondSelfTouchCoord = 0;
         }
         bool cont = true;
         bool round = false;
         MC2Coordinate stBeginCoord;
         while ( cont ) {
            iit = selfTouchingCoordIdxs.find( secondSelfTouchCoord );
            if ( iit != selfTouchingCoordIdxs.end() ) {
               mc2dbg8 << " new shared " << firstSelfTouchCoord << "-" 
                       << secondSelfTouchCoord << endl;
               if ( iit->second == 
                  selfTouchingCoordIdxs.find(firstSelfTouchCoord)->second) {
                  cont = false;
                  stBeginCoord = iit->second;
                  mc2dbg8 << "  identified self-touch-loop from " 
                          << stBeginCoord << endl;
               } else {
                  mc2dbg8 << "  update first+second" << endl;
                  firstSelfTouchCoord=secondSelfTouchCoord;
                  secondSelfTouchCoord++;
               }
            } else {
               secondSelfTouchCoord++;
            }
            if ( secondSelfTouchCoord >= nbrCoordsInPoly ) {
               secondSelfTouchCoord = 0;
            }
            
            if ( secondSelfTouchCoord == borderCoordIdx ) {
               // or stop at startCoordOnBorderIdx ???
               mc2dbg8 << "   round" << endl;
               cont = false;
               round = true;
            }
         }
         if ( round ) {
            // we could not identify any self-touch-part
            // something is strange
            mc2log << error << here << " FCR "
                   << " map " << GMSPolyUtility::myMapID << " item "
                   << GMSPolyUtility::myItemID 
                   << " Could not identify self-touch-part"
                   << " for poly " << p << " nbrLoops="
                   << nbrLoops << endl;
            hasProblems = true;
            return NULL;
            // Return NULL instead of assert not to fail map gen
            //MC2_ASSERT(false);
         }
         
         // The poly part from firstSelfTouchCoord to secondSelfTouchCoord
         // defines one self-touch-part
         mc2dbg4 << " self-touch-part " << firstSelfTouchCoord << "->"
                 << secondSelfTouchCoord << " " << stBeginCoord << endl;
         // END ----- findFirstSelfTouchPart ---------
         
         
         // Need to know if the poly part is a  negative ("hole") or 
         // a positive self-touch
         // to decide how to continue with the elimination
         GMSGfxData* stGfx = GMSGfxData::createNewGfxData(NULL, true);
         uint32 c = firstSelfTouchCoord;
         while ( c != secondSelfTouchCoord ) {
            stGfx->addCoordinate( gfx->getLat(p,c), gfx->getLon(p,c));
            c++;
            if ( c >= nbrCoordsInPoly ) {
               c = 0;
            }
         }
         stGfx->setClosed(0, gfx->getClosed(p));
         stGfx->updateLength();
         stGfx->removeIdenticalCoordinates();
         int stClockWise = stGfx->clockWise(0);
         mc2dbg8 << " stGfx nbrC=" << stGfx->getNbrCoordinates(0) 
                 << " clockwise=" << stClockWise << endl;
         if ( stGfx->getNbrCoordinates(0) < 3 ) {
            mc2log << error << here << " FCR "
                   << " map " << GMSPolyUtility::myMapID << " item "
                   << GMSPolyUtility::myItemID 
                   << " The self-touch-part only has "
                   << stGfx->getNbrCoordinates(0) << " coords"
                   << " self-touch begin " << stBeginCoord << endl;
            hasProblems = true;
            return NULL;
            // Return NULL instead of assert not to fail map gen
            /*
            // For debug purposes
            GMSGfxData* tmpGfx = GMSGfxData::createNewGfxData(NULL);
            tmpGfx->addPolygon(gfx, false, p); // not backwards
            tmpGfx->setClosed(0, gfx->getClosed(p));
            createAndSaveAMapFromGfxData( tmpGfx, 7781 );
            */
            //MC2_ASSERT(false);
         }

         // --------- ELIMINATE POSITIVE SELF-TOUCH ---------
         if ( stClockWise == 1 ) {
            mc2dbg4 << " self-touch-part is a positive poly" << endl;
            // Simply snipp it of!
            // this section is kind of copied from negative self-touch
            // perhaps make it into one sub-method
            
            // Create new gfx datas with the 2 poly parts.
            // Possible improvement? (for easier debug): 
            // - The polypart that includes coord 0 should be rotated 
            //   to start from coord 0.
            // 
            // 1.
            // The part from firstSelfTouchCoord -> secondSelfTouchCoord
            // does not have any more self-touches
            // It is the stGfx, add it to the gfxdata
            stGfx->removeIdenticalCoordinates();
            mc2dbg8 << " stGfx nbrC=" << stGfx->getNbrCoordinates(0) << endl;
            gfx->addPolygon( stGfx );
            gfx->setClosed(nbrPolygons, gfx->getClosed(p));
            polysWithoutSelfTouch.insert(nbrPolygons);
            
            // 2
            // The part from secondSelfTouchCoord -> firstSelfTouchCoord
            // it might have more self-touches to eliminate
            // This one should replace poly p
            // 
            // Improvement: if firstSelfTouchCoord != borderCoordIdx
            // there is a line between the 2 poly parts not contributing
            // to the poly shape and area, remove the "polygon defects"
            // before replacing poly p
            GMSGfxData* replaceGfx = 
               GMSGfxData::createNewGfxData(NULL, true);
            uint32 c=secondSelfTouchCoord;
            while ( c != firstSelfTouchCoord ) {
               replaceGfx->addCoordinate( 
                     gfx->getLat(p,c), gfx->getLon(p,c));
               c++;
               if ( c >= nbrCoordsInPoly ) {
                  c = 0;
               }
            }
            replaceGfx->setClosed(0, gfx->getClosed(p));
            replaceGfx->updateLength();
            replaceGfx->removeIdenticalCoordinates();
            mc2dbg8 << " replaceGfx nbrC=" 
                    << replaceGfx->getNbrCoordinates(0) << endl;
            gfx->replacePolygon( p, replaceGfx, 0 );
            gfx->setClosed(p, gfx->getClosed(p));
            mc2dbg8 << " replace poly " << p  << " nbrC " << nbrCoordsInPoly 
                    << " -> " << gfx->getNbrCoordinates(p) << endl;
         }

         // --------- ELIMINATE NEGATIVE SELF-TOUCH ---------
         else if ( stClockWise == 0 ) {
            // The self-touch-part was negative = a hole
            // Eliminate this self-touch-part!

            // BEGIN ----- eliminateOneSelfTouchPart -----
            // params: gfx, p, stGfx, stBeginCoord
            //         firstSelfTouchCoord, secondSelfTouchCoord
            //         borderCoordIdx ??
    
            // Create a convHull of this stGfx, find a point on the conv hull
            // convHullCoordIdx, that is not the same as the start&end of 
            // the self-touch-part
            // This is the point from where to draw the line to the poly border
            // Find the coordIdx of this point in gfx poly p, polyCoordIdx

            GfxData* stTempHullGfx = stGfx->createNewConvexHull();
            if ( stTempHullGfx == NULL ) {
               mc2log << error << here << " FCR "
                      << " map " << GMSPolyUtility::myMapID << " item "
                      << GMSPolyUtility::myItemID
                      << " poly " << p << " self-touch begin " 
                      << stBeginCoord << " CONV HULL NULL" << endl;
               char tmpstr[128];
               sprintf(tmpstr, "__stGfx_%d_%d.txt", 
                        GMSPolyUtility::myMapID, GMSPolyUtility::myItemID);
               ofstream fileX( tmpstr );
               stGfx->printMif( fileX );
               hasProblems = true;
               return NULL;
               // return NULL not to fail map gen
               //MC2_ASSERT(false); 
            }
            GfxDataFull* stHullGfx = 
               static_cast<GfxDataFull*>( stTempHullGfx );
            stHullGfx->updateBBox();
            stHullGfx->setClosed(0, true);
            stHullGfx->removeIdenticalCoordinates();
            stHullGfx->updateLength();

            // find a good coord idx on the conv hull
            uint32 convHullCoordIdx = 0;
            bool foundGoodConvHullCoord = false;
            MC2Coordinate coordOnHull;
            while (!foundGoodConvHullCoord && 
                     convHullCoordIdx < stHullGfx->getNbrCoordinates(0)) {
               MC2Coordinate tmpCoordOnHull( 
                  stHullGfx->getLat(0,convHullCoordIdx), 
                  stHullGfx->getLon(0,convHullCoordIdx));
               if ( tmpCoordOnHull != stBeginCoord ) {
                  foundGoodConvHullCoord = true;
                  coordOnHull = tmpCoordOnHull;
               } else {
                  convHullCoordIdx++;
               }
            }
            if ( ! foundGoodConvHullCoord ) {
               mc2log << error << here << " ERROR" << endl;
               return NULL;
               // return NULL not to fail map gen
               //MC2_ASSERT(false); 
            }
            mc2dbg8 << " stHullGfx coordOnHull idx=" << convHullCoordIdx
                    << " " << coordOnHull << endl;

            // Find the coordIdx of this point in gfx poly p, polyCoordIdx
            coordMap_t stGfxCoords;
            GMSGfxData::fillCoordMap( stGfx, 0, stGfxCoords, 0);
            cit = stGfxCoords.find(coordOnHull);
            if ( cit == stGfxCoords.end() ) {
               // really strange
               mc2log << error << here << " FCR "
                      << " map " << GMSPolyUtility::myMapID << " item "
                      << GMSPolyUtility::myItemID
                      << " The self-touch-part does not"
                      << " share coord " << coordOnHull 
                      << " with the conv hull of the self-touch-part"
                      << endl;
               hasProblems = true;
               return NULL;
               // return NULL not to fail map gen
               //MC2_ASSERT(false); 
            }
            uint32 stGfxCoordIdx = cit->second.begin()->second;
            mc2dbg8 << "  stGfxCoordIdx " << stGfxCoordIdx << endl;
            // the stGfx starts at firstSelfTouchCoord
            uint32 polyCoordIdx = 
                  ((stGfxCoordIdx + firstSelfTouchCoord) % nbrCoordsInPoly);
            mc2dbg8 << " polyCoordIdx " << polyCoordIdx 
                    << " (firstSelfTouchCoord " << firstSelfTouchCoord
                    << ")" << endl;

            // Draw the line towards the poly border 
            // (uses same method as elimholes method)
            MC2BoundingBox polyBbox;
            gfx->getMC2BoundingBox(polyBbox, p);
            int32 endLat; int32 endLon;
            bool lineOK = drawLineTowardsPolyBorder( 
                  stHullGfx, coordOnHull, convHullCoordIdx, polyBbox,
                  endLat, endLon);
            if ( ! lineOK ) {
               mc2log << error << here << " FCR "
                      << " map " << GMSPolyUtility::myMapID << " item "
                      << GMSPolyUtility::myItemID
                      << " Failed to create line towards poly border"
                      << endl;
               hasProblems = true;
               return NULL;
               // Return NULL instead of assert not to fail map gen
               //MC2_ASSERT(false);
            }
            mc2dbg8 << " line from " << coordOnHull << " to " << endLat
                    << ";" << endLon << endl;
            
            // Find the intersection point on the polygon border
            // (uses same method as elimholes method, but it skips 
            //  comparing with border coordinates that are part of
            //  the self-touch-part )
            // BEGIN ------------ copied from elimHoles ----------------
            MC2Coordinate interCoord;
            uint32 nextCoordIdx = MAX_UINT32;
            uint32 reversedLineNextCoordIdx = MAX_UINT32;
            bool interSectionFound =
               findPolyBorderIntersectionPoint( gfx, p, 
                  coordOnHull, endLat, endLon,
                  interCoord, nextCoordIdx, reversedLineNextCoordIdx,
                  false, // no print info
                  secondSelfTouchCoord, firstSelfTouchCoord);
                  // only check for intersections on the poly border that is 
                  // not part of the self-touch-part
            if ( ! interSectionFound ) {
               // we have a serious problem
               mc2log << error << here << " FCR "
                      << " map " << GMSPolyUtility::myMapID << " item "
                      << GMSPolyUtility::myItemID 
                      << " No intersection found for poly " << p << endl;
               mc2dbg << "poly=" << p << " coordOnHull " << coordOnHull
                      << " end " << endLat << ";" << endLon << endl;
               mc2dbg << "Prints convHull of self-touch-part to stHullGfx.mif"
                      << endl;
               ofstream fileX( "stHullGfx.mif");
               GMSGfxData* tmpGfx = 
                  GMSGfxData::createNewGfxData(NULL, stHullGfx);
               tmpGfx->printMif( fileX );
               mc2dbg << "Calls findPolyBorderIntersectionPoint "
                      << "again with printDebug = true " << endl;
               findPolyBorderIntersectionPoint( gfx, p, 
                  coordOnHull, endLat, endLon,
                  interCoord, nextCoordIdx, reversedLineNextCoordIdx,
                  true, // print info
                  secondSelfTouchCoord, firstSelfTouchCoord );
               hasProblems = true;
               return NULL;
               // Return NULL instead of assert not to fail map gen
               //MC2_ASSERT(false);
            }
            mc2dbg8 << " intersection nextCoordIdx=" << nextCoordIdx
                    << " nextCoord=" << gfx->getLat(p,nextCoordIdx)
                    << ";" << gfx->getLon(p,nextCoordIdx)
                    << ", interCoord=" << interCoord << endl;
            // Fixme: Do something with the reversedLineNextCoordIdx
            // for eliminate self-touch, consider that the reversed line
            // is either in this poly or in another already snipped
            // polygon.
            // END ------------ copied from elimHoles ----------------
            
            delete stTempHullGfx;

// Fix this:
// code parts that cause V-gaps and in some cases cause ST-elim 
// to crash with bad_alloc
// Believe it has todo with the coord idxs not correct after this.
//            // Before removing the self touch, add interCoord to the
//            // intersecting reversed line (if there is one) in order make 
//            // sure we have the same geometry on the parallell lines.
//
//            mc2dbg8 << "p: " << p << endl;
//            mc2dbg8 << "reversedLineNextCoordIdx: "
//                   << reversedLineNextCoordIdx << endl;
//            mc2dbg8 << "nbrOfCoordinates: "
//                   << gfx->getNbrCoordinates( p ) << endl;
//            if ( reversedLineNextCoordIdx != MAX_UINT32 ) {
//               mc2dbg8 << "add coord: " << "lat: "
//                      << interCoord.lat << "lon: "
//                      << interCoord.lon << endl;
//               gfx->addCoordinateBeforeSpecificCoordIdx(
//                  interCoord.lat, interCoord.lon,
//                  p, reversedLineNextCoordIdx);
//
//               // Also, make sure all index-variables are correct after the 
//               // insert
//               if( reversedLineNextCoordIdx < polyCoordIdx ) {
//                  polyCoordIdx++;
//               }
//         
//               if( reversedLineNextCoordIdx < nextCoordIdx ) {
//                  nextCoordIdx++;
//               }
//            } else {
//               // Fixme:
//               // The newly drawn line may intersect a segment where there
//               // exist a reverse line, but where the reverse line is 
//               // in another poly. This may happen when we have 2 holes 
//               // belonging to the same self-touch (see Fig 4. in section 
//               // 'Eliminate self-touching polygons' of the documentation 
//               // for an example), and where the first hole have been 'self
//               // touch eliminated'. For this case we have to go through all 
//               // polys and see if the
//               // newly drawn line intersects a poly P in the same coordinate
//               // as interCoord, and in such case, add interCoord to P.
//               // NOTE; could not use findPolyBorderIntersectionPoint
//               // for this purpose as this function requires the start point
//               // to be inside P. 
//            }
// Fix this end

            // Create new gfx datas with the 2 poly parts, both of them
            // might have more self-touches to eliminate
            // Remove ident coords, if the interCoord was found to be the
            // same as a original coord in the gfx it is duplicated.
            //
            // Possible improvement?: (for easier debug):
            // - The polypart that includes coord 0 should be rotated 
            //   to start from coord 0.
            
            // 1 
            // This one should be added as new poly to (end of) gfx
            //  - start at polyCoordIdx (coordOnHull)
            //  - add until nextCoordIdx-1
            //  - add interCoord
            GMSGfxData* newGfx = GMSGfxData::createNewGfxData(NULL, true);
            uint32 c=polyCoordIdx;
            while ( c != nextCoordIdx ) {
               newGfx->addCoordinate( gfx->getLat(p,c), gfx->getLon(p,c));
               c++;
               if ( c >= nbrCoordsInPoly ) {
                  c = 0;
               }
            }
            newGfx->addCoordinate( interCoord.lat, interCoord.lon);
            newGfx->setClosed(0, gfx->getClosed(p));
            newGfx->updateLength();
            newGfx->removeIdenticalCoordinates();
            mc2dbg8 << " newGfx nbrC=" 
                    << newGfx->getNbrCoordinates(0) << endl;
            gfx->addPolygon( newGfx /*, false, 0*/ );
            gfx->setClosed(nbrPolygons, gfx->getClosed(p));
            delete newGfx;
            
            // 2
            // This one should replace poly p
            // - add interCoord
            // - continue from nextCoordIdx (it is on the poly border)
            // - add until polyCoordIdx
            GMSGfxData* replaceGfx = 
               GMSGfxData::createNewGfxData(NULL, true);
            replaceGfx->addCoordinate( interCoord.lat, interCoord.lon);
            c=nextCoordIdx;
            while ( c != polyCoordIdx ) {
               replaceGfx->addCoordinate( 
                  gfx->getLat(p,c), gfx->getLon(p,c));
               c++;
               if ( c >= nbrCoordsInPoly ) {
                  c = 0;
               }
            }
            // add polyCoordIdx
            replaceGfx->addCoordinate( 
               gfx->getLat(p,polyCoordIdx), gfx->getLon(p,polyCoordIdx));
            replaceGfx->setClosed(0, gfx->getClosed(p));
            replaceGfx->updateLength();
            replaceGfx->removeIdenticalCoordinates();
            mc2dbg8 << " replaceGfx nbrC=" << replaceGfx->getNbrCoordinates(0)
                    << endl;
            /*bool ok =*/ gfx->replacePolygon( p, replaceGfx, 0 );
            gfx->setClosed(p, gfx->getClosed(p));
            mc2dbg8 << " replace poly " << p  << " nbrC " << nbrCoordsInPoly 
                    << " -> " << gfx->getNbrCoordinates(p) << endl;
            delete replaceGfx;
            
            
            // END ----- eliminateOneSelfTouchPart -----
         }
         
         else {
            // problem with clockwise for the self-touch-part
            mc2log << error << here << " FCR "
                   << " map " << GMSPolyUtility::myMapID << " item "
                   << GMSPolyUtility::myItemID 
                   << " Problem with clockwise for the self-touch-part"
                   << endl;
            GMSGfxData* tmpGfx = GMSGfxData::createNewGfxData(NULL);
            tmpGfx->addPolygon(gfx, false, p); // not backwards
            tmpGfx->setClosed(0, gfx->getClosed(p));
            createAndSaveAMapFromGfxData( tmpGfx, 7776 );
            hasProblems = true;
            return NULL;
            // return NULL not to fail map gen
            //MC2_ASSERT(false); 
         }

         delete stGfx;

      }

      // check if all polys are in the polysWithoutSelfTouch set
      // if they are -> we are done!
      // else -> repeat looping polys of this gfxdata
      uint16 newNbrPolygons = gfx->getNbrPolygons();
      mc2dbg8 << "gfx now has " << newNbrPolygons << " polygons" << endl;
      uint16 k = 0;
      while ( (k < newNbrPolygons) && 
               (polysWithoutSelfTouch.find(k) !=
                  polysWithoutSelfTouch.end()) ) {
         k++;
      }
      if ( k == newNbrPolygons) {
         allSTremoved = true;
         mc2dbg8 << "allSTremoved" << endl;
      }
      else {
         mc2dbg8 << "not allSTremoved k=" << k << endl;
      }
      
   } // while allSTremoved
   
   
   // If something changed return the gfx, sort it first
   if ( gfx->getNbrPolygons() != origNbrPolygons ) {
      //ofstream fileX( "eliminated.mif");
      //gfx->printMif( fileX );
      gfx->removeIdenticalCoordinates();
      gfx->sortPolygons();
      gfx->updateLength();
      return gfx;
   }
   // else return NULL
   delete gfx;
   return NULL;

} // eliminateSelfTouch in one gfx data


bool
GMSPolyUtility::getPolyHoleHierarchy( 
   GMSGfxData* gfx, multimap<uint32, uint32>& holeHierarchy, 
   bool& hasProblems )
{
   if ( ! gfx->closed() ) {
      return false;
   }
   uint16 nbrPolys = gfx->getNbrPolygons();
   if ( nbrPolys==1 ) {
      return false;
   }

   // Check if this gfxdata has any holes
   // 1. Create bbox for every polygon (for check overlapping)
   // 2. Check orientation for every poly, and store whether the poly
   //    is positive or a hole.
   vector<MC2BoundingBox> polyboxes;
   vector<int> polyClockwise;
   set<uint16> holes; set<uint16> polys;
   polyboxes.reserve(nbrPolys);
   polyClockwise.reserve(nbrPolys);
   for (uint16 p=0; p < nbrPolys; p++ ) {
      MC2BoundingBox box;
      gfx->getMC2BoundingBox(box, p);
      int clockWise = gfx->clockWise(p);
      mc2dbg8 << " p=" << p << " cw=" << clockWise << " box " << box
              << " " << gfx->getLat(p, 0) << ";" << gfx->getLon(p, 0)
              << endl;
      polyboxes[p] = box;
      polyClockwise[p] = clockWise;
      if (clockWise == 1) { polys.insert(p); }
      else if (clockWise == 0) { holes.insert(p); }
      else { 
         mc2log << error << here << " FCR "
                << " map " << GMSPolyUtility::myMapID
                << " item " << GMSPolyUtility::myItemID 
                << " Could not get clockWise for poly " << p << endl;
         hasProblems = true;
         return false; 
         // Return false instead of assert not to fail map gen
         //MC2_ASSERT(false);
      }
   }

   if ( holes.size() == 0 ) {
      // no holes
      polyboxes.clear();
      polyClockwise.clear();
      return false;
   }

   mc2dbg4 << "Gfx has " << holes.size() << " holes (of "
           << nbrPolys << ")" << endl;

   // For every hole check which is my positive poly, i.e. which poly
   // the hole is inside
   //  a: bboxes overlaps
   //  b: one coord from hole is inside poly
   // One hole can be inside several polys (poly inside poly), then
   // choose the poly that is inside the other poly (has the smallest box)
   set<uint16>::const_iterator hole;
   set<uint16>::const_iterator poly;
   set<uint16> polysThatMustHaveHoles;
   for ( hole=holes.begin(); hole != holes.end(); hole++ ) {
      int32 holeLat = gfx->getLat(*hole, 0);
      int32 holeLon = gfx->getLon(*hole, 0);

      mc2dbg8 << " Hole " << *hole
              << " " << holeLat << ";" << holeLon << endl;
      set<uint16> polyCandidates;
      set<uint16> polyMaybeCandidates;
      // Loop the polys to find the ones that include the hole
      for ( poly=polys.begin(); poly != polys.end(); poly++ ) {
         if ( ! polyboxes[*hole].overlaps(polyboxes[*poly])) {
            continue;
         }
         mc2dbg8 << "  hole bbox overlapped by poly " << *poly << endl;
         // use gfx:insidePolygon or insideUtil:inside ?? Bug in insideutil?
         int inside = gfx->insidePolygon(holeLat, holeLon, *poly);
         if ( inside == 0 ) { // outside
            continue;
         }
         if ( inside == 1 ) { // on border
            // Try moving the hole coord east, west, south, north 
            // to find position completely inside the polygon (that is,
            // not on border).

            int i = 0;
            bool newCoordInsidePolygon = false;
            MC2Coordinate newCoord( holeLat,
                                    holeLon );
             
            while( !newCoordInsidePolygon && i < 4) {

               // reset newCoordOnHull
               newCoord.lat = holeLat;
               newCoord.lon = holeLon;

               // Move north, east, south or north
               if( i == 0 ) {
                  newCoord.lat++;
               } else if ( i == 1 ) {
                  newCoord.lat--;
               } else if ( i == 2 ) {
                  newCoord.lon++; 
               } else {
                  newCoord.lon--; 
               }
         
               // Check if newCoord is inside poly.
               newCoordInsidePolygon = gfx->insidePolygon( 
                  newCoord.lat, 
                  newCoord.lon,
                  *poly );

               i++;
            }
 
            if( newCoordInsidePolygon ) {
               // Change geometry to the newCoordOnHull
               holeLat = newCoord.lat;
               holeLon = newCoord.lon;
              
               gfx->setCoordinate( *hole, 
                                   0, 
                                   newCoord.lat, 
                                   newCoord.lon );

               mc2dbg1 << here << " FCR "
                      << " map " << GMSPolyUtility::myMapID << " item "
                      << GMSPolyUtility::myItemID 
                      << " Had to change geometry for poly " << *poly 
                      << " hole " << *hole 
                      << " holeCoordIdx: 0" 
                      << " in order to avoid hole touching border." 
                      << " New coordinate, lat, lon: " << newCoord.lat
                      << " ," << newCoord.lon  
                      << endl;

            } else {
               // the hole coord is on the border of the poly
               mc2dbg8 << "  hole is maybe inside (" << inside << ") poly "
                       << *poly << endl;
               polyMaybeCandidates.insert(*poly);
               mc2log << warn << here << " FCR "
                      << " map " << GMSPolyUtility::myMapID
                      << " item " << GMSPolyUtility::myItemID 
                      << " Hole " << *hole 
                      << " is on the border of poly " << *poly 
                      << " coord " << holeLat << ";" << holeLon 
                      << endl;
               // If the insideUtil:inside says inside we perhaps should use it?
               // skip for now - the holes of this gfx will not be eliminated
               bool tmpInside = InsideUtil::inside( 
                  gfx->polyBegin(*poly), gfx->polyEnd(*poly), 
                  MC2Coordinate(holeLat, holeLon) );
               cout << " the tmpInside = " << int(tmpInside) << endl;
               hasProblems = true;
               continue;
            }
         }
         mc2dbg8 << "  hole is inside (" << inside << ") poly "
                 << *poly << endl;
         polyCandidates.insert(*poly);
      }
      if ( polyCandidates.size() == 0 ) {
         mc2log << error << here << " FCR "
                << " map " << GMSPolyUtility::myMapID
                << " item " << GMSPolyUtility::myItemID
                << " Hole " << *hole << " " << holeLat << ";" 
                << holeLon << " is not inside any poly!" << endl;
         uint32 hullIndex = getPointOnConvexHull( gfx, *hole );
         mc2dbg << "The coord that was used as hullIndex in clockwise: "
                << gfx->getLat(*hole,hullIndex) << ";"
                << gfx->getLon(*hole,hullIndex) << endl;
         hasProblems = true;
         return false; 
         // Return false instead of assert not to fail map gen
         //MC2_ASSERT(false);
         // Possible improvement?? 
         // - if something in the polyMaybeCandidates, use it ???
      }
      if ( polyCandidates.size() > 1 ) {
         // find out which is the smallest poly (poly in poly)
         mc2dbg8 << " SEVERAL polys for hole" << endl;
         uint64 minBoxArea = MAX_UINT64;
         uint16 polyForHole = MAX_UINT16;
         for ( poly = polyCandidates.begin();
               poly != polyCandidates.end(); poly++ ) {
            uint64 boxArea = polyboxes[*poly].getArea();
            mc2dbg8 << "  poly " << *poly << " area=" << boxArea << endl;
            if ( boxArea < minBoxArea ) {
               minBoxArea = boxArea;
               polyForHole = *poly;
            }
         }
         mc2dbg8 << " CORRECT poly for hole " << polyForHole << endl;
         holeHierarchy.insert(make_pair(polyForHole, *hole));
         // and the other polys must have holes in them!
         // (hole in poly in hole in poly)
         for ( poly = polyCandidates.begin();
               poly != polyCandidates.end(); poly++ ) {
            if ( *poly != polyForHole ) {
               polysThatMustHaveHoles.insert(*poly);
            }
         }
      }
      if (polyCandidates.size() == 1) {
         poly = polyCandidates.begin();
         mc2dbg8 << " SINGLE poly for hole " << *poly << endl;
         holeHierarchy.insert(make_pair(*poly, *hole));
      }
   }
   
   // Check that all polys marked to have holes, really do have holes
   mc2dbg8 << "Gfx " << polysThatMustHaveHoles.size() 
           << " polys must have holes" << endl;
   multimap<uint32, uint32>::const_iterator it;
   for ( poly=polysThatMustHaveHoles.begin();
         poly != polysThatMustHaveHoles.end(); poly++ ) {
      it = holeHierarchy.find(*poly);
      if ( it == holeHierarchy.end() ){
         mc2log << error << here << " FCR "
                << " map " << GMSPolyUtility::myMapID
                << " item " << GMSPolyUtility::myItemID
                << " polysThatMustHaveHoles poly " << *poly
                << " has no hole!" << endl;
         hasProblems = true;
         return false; 
         // Return false instead of assert not to fail map gen
         //MC2_ASSERT(false);
      }
   }

   // The holeHierarchy now only includes the polygons that have holes
   // That is ok, we do not want to add all other polys as well.
   uint32 nbrPolysWithHoles = 0;
   for ( poly=polys.begin(); poly != polys.end(); poly++ ) {
      if (holeHierarchy.count(*poly) > 0 ) {
         nbrPolysWithHoles++;
      }
      mc2dbg8 << " poly " << *poly << " has "
              << holeHierarchy.count(*poly) << " holes" << endl;
   }
   mc2dbg8 << "Item has " << nbrPolysWithHoles << " polys with holes ("
           << holeHierarchy.size() << " poly-hole pairs)" << endl;
   
   return (holeHierarchy.size() > 0);
} // getPolyHoleHierarchy


GMSGfxData*
GMSPolyUtility::createConvexHullOfHolesOfOnePoly( 
   GMSGfxData* gfx, uint32 poly, multimap<uint32, uint32> holeHierarchy,
   set<uint32> excludeTheseHoles )
{
   uint32 nbrHolesForPoly = holeHierarchy.count(poly);
   mc2dbg8 << "GMSPolyUtility::createConvexHullOfHolesOfOnePoly "
           << "nbrHolesForPoly=" << nbrHolesForPoly << endl;
   if ( nbrHolesForPoly == 0 ) {
      return NULL;
   }

   // Collect all hole coordinates
   // do not include coords of holes that are defined in excludeTheseHoles
   GMSGfxData* allHoleCoords = GMSGfxData::createNewGfxData(NULL, true);
   multimap<uint32, uint32>::const_iterator hh;
   set<uint32>::const_iterator holeIter;
   for ( hh = holeHierarchy.lower_bound(poly);
         hh != holeHierarchy.upper_bound(poly); hh++ ) {
      uint32 hole = hh->second;
      holeIter = excludeTheseHoles.find(hole);
      if ( holeIter != excludeTheseHoles.end() ) {
         // skip this hole
      } else {
         // use coords from this hole
         GfxData::const_iterator cIt = gfx->polyBegin(hole);
         while ( cIt != gfx->polyEnd(hole) ){
            allHoleCoords->addCoordinate(cIt->lat, cIt->lon);
            ++cIt;
         }
      }
   }
   allHoleCoords->updateBBox();

   
   Stack* stack = new Stack;
   GMSGfxData* convHull = NULL;
   if (allHoleCoords->getConvexHull(stack, 0)) {
      convHull = GMSGfxData::createNewGfxData(NULL, true);
      
      uint32 size = stack->getStackSize();
      for (uint32 i = 0; i < size; i++) {
         uint32 idx = stack->pop();
         convHull->addCoordinate(allHoleCoords->getLat(0,idx), 
                                 allHoleCoords->getLon(0,idx));
      }
   } else {
      // Something went wrong.
      mc2log << error << here << " FCR "
             << " map " << GMSPolyUtility::myMapID
             << " item " << GMSPolyUtility::myItemID
             << " Could not create convex hull, allHoleCoords nbrC=" 
             << allHoleCoords->getNbrCoordinates(0) 
             << " coord0=" 
             << allHoleCoords->getLat(0,0) << ";" << allHoleCoords->getLon(0,0)
             << endl;
      return NULL;
      // Return NULL instead of assert not to fail map gen
      //MC2_ASSERT(false);
   }
   convHull->updateBBox();
   convHull->setClosed(0, true);
   convHull->removeIdenticalCoordinates();
   convHull->updateLength();
   delete stack;
   delete allHoleCoords;

   return convHull;
   
} // createConvexHullOfHolesOfOnePoly

bool
GMSPolyUtility::findPolyBorderIntersectionPoint( 
   GMSGfxData* gfx, uint32 poly, 
   MC2Coordinate startCoord, int32 endLat, int32 endLon,
   MC2Coordinate& interCoord, uint32 &nextCoordIdx,
   uint32 &reversedLineNextCoordIdx,
   bool printDebugs,
   uint32 polyStartIdx, uint32 polyEndIdx )
{
   // There is a line from a start coord (which is inside the polygon)
   // to a end coord which is outside the polygon.
   // Decide the point on the polygon border which intersects with the line
   
   // Find the pairs of coords on poly border that intersects with the line
   // Find the closest intersection
   //    The border found for some intersection can be the line between
   //    an old (hole) startcoord and the poly border. Then there are 
   //    two intersections with the same dist to start coord.
   //    Howto decide which is the correct? Check left/right side!
   //    Save also info for the other intersection, to insert the interCoord
   //    for both lines (share all coords)

   // Loop coord pairs in poly and check if they intersect
   // with the line
   typedef pair< MC2Coordinate, MC2Coordinate > mc2coordpair_t;
   map< mc2coordpair_t, MC2Coordinate > borderIntersections;
   map< mc2coordpair_t, uint32 > borderIntersectionsIdx;
   multimap< MC2Coordinate, uint32 > otherBorderIntersections;
   GfxData::const_iterator begin = gfx->polyBegin( poly );
   GfxData::const_iterator end   = gfx->polyEnd( poly );
   GfxData::const_iterator cur = begin;
   GfxData::const_iterator next = begin;
   next++;
   uint32 tmpNextCoordIdx = 0;
   tmpNextCoordIdx++;
   
   bool loopAllBorder = true;
   if ( polyStartIdx != MAX_UINT32 ) {
      loopAllBorder = false;
      cur += polyStartIdx;
      next += polyStartIdx;
      tmpNextCoordIdx += polyStartIdx;
      if ( polyEndIdx == MAX_UINT32 ) {
         mc2log << error << here << "Item " << GMSPolyUtility::myItemID
                << " poly " << poly << " polyStartIdx=" << polyStartIdx
                << ", but no given polyEndIdx" << endl;
         return false;
         // return false not to fail map gen
         //MC2_ASSERT(false); 
      }
   }
   if ( polyEndIdx != MAX_UINT32 ) {
      loopAllBorder = false;
      if ( polyStartIdx == MAX_UINT32 ) {
         mc2log << error << here << "Item " << GMSPolyUtility::myItemID
                << " poly " << poly << " polyEndIdx=" << polyEndIdx
                << ", but no given polyStartIdx" << endl;
         return false;
         // return false not to fail map gen
         //MC2_ASSERT(false); 
      }
   }
   
   bool cont = true;
   while ( cont ) {
      if ( next == end ) {
         next = begin;
         tmpNextCoordIdx = 0;
      }
      if ( cur == end ) {
         cur = begin;
      }

      int32 interLat; int32 interLon;
      bool intersects = 
         GfxUtility::getIntersection(
            cur->lat, cur->lon, next->lat, next->lon,
            startCoord.lat, startCoord.lon, endLat, endLon,
            interLat, interLon );
      if ( intersects ) {
         if ( printDebugs ) {
         mc2dbg << "  intersection with nextCoordIdx=" 
                 << tmpNextCoordIdx << " " << cur->lat << ";" << cur->lon 
                 << " " << next->lat << ";" << next->lon 
                 << " -> " << interLat << ";" << interLon << endl;
         }
         // The line intersects with the poly border part from cur to next.
         // The start coord needs to be on the right side of
         // the poly border part for the line to exit the polygon.
         int isLeft = InsideUtil::isLeft( *cur, *next, startCoord );
         if ( printDebugs ) {
         mc2dbg << "  intersection cur-next-start, isLeft = " << isLeft
                 << " (dist = " 
                 << sqrt ( GfxUtility::squareP2Pdistance_linear(
                     interLat, interLon, startCoord.lat, startCoord.lon ) )
                 << " m)" << endl;
         }
         // only if not isLeft -> add to candidates.
         if ( isLeft == 0 ) {
            mc2log << error << "Item " << GMSPolyUtility::myItemID
                   << " What to do?? - the start point is isLeft=0 "
                   << "- it is on the line from cur-next " << endl;
            return false;
            // Return false instead of assert not to fail map gen
            //MC2_ASSERT (false);
         }
         MC2Coordinate interCoord(interLat, interLon);
         pair<MC2Coordinate, MC2Coordinate> borderPair( *cur, *next );
         if ( isLeft < 0 ) {
            // this is the intersection we want
            borderIntersections.insert(
               make_pair( borderPair, interCoord) );
            borderIntersectionsIdx.insert(
               make_pair( borderPair, tmpNextCoordIdx) );
         } else {
            // this is the other intersection where we want to insert one
            // coordinate to make all coords shared along the lines
            otherBorderIntersections.insert(
               make_pair( interCoord, tmpNextCoordIdx) );
         }
      }


      if ( ! loopAllBorder ) {
         if ( tmpNextCoordIdx == polyEndIdx ) {
            cont = false;
         }
      }

      cur++;
      next++;
      tmpNextCoordIdx++;
   
      if ( loopAllBorder ) {
         if (cur == end) {
            cont = false;
         }
      }
   }
   if ( printDebugs ) {
   mc2dbg << "  borderIntersections.size="
           << borderIntersections.size() << endl;
   }

   if ( borderIntersections.size() < 1 ) {
      // no intersection found
      return false;
   }
   else if ( borderIntersections.size() == 1 ) {
      // single intersection found
      interCoord = borderIntersections.begin()->second;
      nextCoordIdx = borderIntersectionsIdx.begin()->second;
   }
   else if ( borderIntersections.size() > 1 ) {
      // several intersections found, compare distance to start coord
      bool foundOne = false;
      float64 distToIntersection = MAX_UINT64;
      map< mc2coordpair_t, MC2Coordinate >::const_iterator biIt;
      map< mc2coordpair_t, uint32 >::const_iterator biidxIt;
      for ( biIt = borderIntersections.begin();
            biIt != borderIntersections.end(); biIt++ ) {
         // calc dist start coord -> biIt->second
         // if dist is smaller -> save biidxIt->second
         float64 someDist = 
            GfxUtility::squareP2Pdistance_linear(
               biIt->second.lat, biIt->second.lon,
               startCoord.lat, startCoord.lon );
         if ( printDebugs ) {
         mc2dbg << "  " << biIt->second.lat << ";" << biIt->second.lon
                << " dist=" << someDist << " min=" << distToIntersection
                << endl;
         }
         
         if ( someDist < distToIntersection ) {
            foundOne = true;
            distToIntersection = someDist;
            interCoord = biIt->second;
            biidxIt = borderIntersectionsIdx.find(biIt->first);
            if ( biidxIt != borderIntersectionsIdx.end() ) {
               nextCoordIdx = biidxIt->second;
            }
            if ( printDebugs ) {
            mc2dbg << "  use it! " << interCoord 
                   << " nextCoordIdx " << nextCoordIdx << endl;
            }
         }

      }
      if ( ! foundOne ) {
         mc2log << error << "Item " << GMSPolyUtility::myItemID
                << " of " << borderIntersections.size()
                << " intersection candidates I found none good!" << endl;
         return false;
         // Return false instead of assert not to fail map gen
         //MC2_ASSERT(false);
      }
   }
   
   
   // Find the correct coord index for the other intersection,
   // where we want to insert one coordinate to make all coords
   // shared along the line
   uint32 nbrOthers = otherBorderIntersections.count(interCoord);
   if ( nbrOthers > 1 ) {
      // strange
   }
   if ( nbrOthers == 1 ) {
      multimap< MC2Coordinate, uint32 >::const_iterator oth =
         otherBorderIntersections.find(interCoord);
      reversedLineNextCoordIdx = oth->second;
   }
   
   return true;
} // findPolyBorderIntersectionPoint

uint32
GMSPolyUtility::getPointOnConvexHull(GMSGfxData* gfx, int polyIndex)
{
   // call the (private) GfxData::getPointOnConvexHull
   uint32 result = gfx->getPointOnConvexHull( polyIndex );
   return result;
}

uint32 GMSPolyUtility::myItemID = MAX_UINT32;
uint32 GMSPolyUtility::myMapID = MAX_UINT32;


bool
GMSPolyUtility::isSelfTouching( GMSGfxData* gfx, 
   coordMap_t& selfTouchingCoords, uint16 polyNbr )
{
   // Find out if there are any shared coordinates in any of the
   // polygons of the gfxData. If one polygon is given with polyNbr
   // we should check only that one.

   // Gfx data with only one polygon and less than 3 coords
   // cannot be self-touching (e.g. virtual 0-length ssi)
   if ( gfx->getNbrPolygons()==1 && gfx->getNbrCoordinates(0) <= 2 ) {
      return false;
   }

   // If polyNbr is given it must be within numberOfPolygons
   if ( polyNbr != MAX_UINT16 &&
        ( polyNbr >= gfx->getNbrPolygons()) ) {
      return false;
   }
   
   // decide which polys to loop
   uint16 startPoly = 0;
   uint32 endPoly = gfx->getNbrPolygons()-1;
   if ( polyNbr != MAX_UINT16 ) {
      startPoly = polyNbr;
      endPoly = polyNbr;
   }
   
   uint32 totNbrSharedIntraCoords = 0;
   for (uint16 p=startPoly; p<=endPoly; p++) {
      // Collect coordinates from my polygon
      coordMap_t gfxCoords;
      GMSGfxData::fillCoordMap( gfx, p, gfxCoords, 0);
      
      // Check if some coordinate(s) are shared within myself
      uint32 nbrSharedCoords = 0;
      for ( coordMap_t::const_iterator it = gfxCoords.begin();
            it != gfxCoords.end(); it++ ) {
         idxSet_t idxSet = it->second;
         if ( idxSet.size() >= 2 ) {
            if ( idxSet.size() == 2 &&
                 it->first.lat == gfx->getLat(p,0) &&
                 it->first.lat == 
                        gfx->getLat(p,gfx->getNbrCoordinates(p)-1) && 
                 it->first.lon == gfx->getLon(p,0) &&
                 it->first.lon == 
                        gfx->getLon(p,gfx->getNbrCoordinates(p)-1) ) {
               // my first coord is equal to my last coord
               // ok
               mc2dbg8 << "IST " << GMSPolyUtility::myItemID
                       << " intra fake shared p="
                       << p << " " << it->first << endl;
            } else {
               selfTouchingCoords.insert( make_pair(it->first, idxSet) );
               nbrSharedCoords++;
               mc2dbg8 << "IST " << GMSPolyUtility::myItemID
                       << " intra shared p=" 
                       << p << " size=" << idxSet.size()
                       << " "
                       << it->first << endl;
               for ( idxSet_t::const_iterator id = idxSet.begin();
                     id != idxSet.end(); id++ ) {
                  mc2dbg8 << " " << id->first << ":" << id->second << endl;
               }
            }
         }
      }
      totNbrSharedIntraCoords += nbrSharedCoords;
   }

   if ( totNbrSharedIntraCoords > 0 ) {
      mc2dbg2 << "IST totNbrSharedIntraCoords=" 
              << totNbrSharedIntraCoords << endl;
   }
   return (totNbrSharedIntraCoords > 0);
} // isSelfTouching


bool 
GMSPolyUtility::drawLineTowardsPolyBorder( 
   GfxData* convHull, MC2Coordinate coordOnHull, 
   uint32 convHullCoordIdx, MC2BoundingBox polyBbox,
   int32& endLat, int32& endLon, bool printDebugs )
{
   uint32 tmpTime = TimeUtility::getCurrentTime();
   uint32 nbrCoordsInConvHull = convHull->getNbrCoordinates(0);
   int32 prevConvHullCoordIdx = convHullCoordIdx-1;
   if ( prevConvHullCoordIdx < 0 ) {
      prevConvHullCoordIdx = nbrCoordsInConvHull-1;
   }
   uint32 nextConvHullCoordIdx = convHullCoordIdx+1;
   if ( nextConvHullCoordIdx >= nbrCoordsInConvHull ) {
      nextConvHullCoordIdx = 0;
   }
   const float64 coslat = cos( (float64) (coordOnHull.lat) * M_PI / 
                                ((float64) 0x80000000) );
   int32 startLat; int32 startLon;
   uint64 pointDist =
      GfxUtility::closestDistVectorToPoint(
         convHull->getLon(0,prevConvHullCoordIdx),
         convHull->getLat(0,prevConvHullCoordIdx),
         convHull->getLon(0,nextConvHullCoordIdx),
         convHull->getLat(0,nextConvHullCoordIdx),
         coordOnHull.lon, coordOnHull.lat,
         startLon, startLat,
         coslat);
   bool startCoordOK = true;
   if ( (pointDist == 0) ||
        ((startLat == coordOnHull.lat) &&
         (startLon == coordOnHull.lon)) ) {
      // prev-convHull-next was on a straight line, try increasing next
      nextConvHullCoordIdx++;
      if ( nextConvHullCoordIdx >= nbrCoordsInConvHull ) {
         nextConvHullCoordIdx = 0;
      }
      pointDist =
         GfxUtility::closestDistVectorToPoint(
            convHull->getLon(0,prevConvHullCoordIdx),
            convHull->getLat(0,prevConvHullCoordIdx),
            convHull->getLon(0,nextConvHullCoordIdx),
            convHull->getLat(0,nextConvHullCoordIdx),
            coordOnHull.lon, coordOnHull.lat,
            startLon, startLat,
            coslat);
      if ( (pointDist == 0) ||
           ((startLat == coordOnHull.lat) &&
            (startLon == coordOnHull.lon)) ) {
         // Prev-convHull-next was still on a straight line, try different
         // method to find startLat and startLon; let startLat and
         // startLon be an arbitrary point on the right of prev-next.
         // Note that we can still use nextConvHullCoordIdx++ since we
         // had a straight line.
         int32 resultLat;
         int32 resultLon;
         GfxUtility::generatePointInMap( 
                  convHull->getLat(0,prevConvHullCoordIdx),
                  convHull->getLon(0,prevConvHullCoordIdx),
                  convHull->getLat(0,nextConvHullCoordIdx),
                  convHull->getLon(0,nextConvHullCoordIdx),
                  resultLat, resultLon);
         startLat = resultLat;
         startLon = resultLon;

         // Check result
         pointDist =
            GfxUtility::closestDistVectorToPoint(
               convHull->getLon(0,prevConvHullCoordIdx),
               convHull->getLat(0,prevConvHullCoordIdx),
               convHull->getLon(0,nextConvHullCoordIdx),
               convHull->getLat(0,nextConvHullCoordIdx),
               startLon, startLat,
               coslat);
         
         if ( (pointDist == 0) ||
           ((startLat == coordOnHull.lat) &&
            (startLon == coordOnHull.lon)) ) {
            startCoordOK = false;
         }
      }
   }
   if ( printDebugs ) {
      cout << " coordOnHull=" << coordOnHull 
           << " prev=" << convHull->getLat(0,prevConvHullCoordIdx)
           << ";" << convHull->getLon(0,prevConvHullCoordIdx)
           << " next=" << convHull->getLat(0,nextConvHullCoordIdx)
           << ";" << convHull->getLon(0,nextConvHullCoordIdx)
           << " startCoord="
           << startLat << ";" << startLon << " pointDist=" << pointDist
           << endl;
   }
   if ( ! startCoordOK ) {
      mc2log << error << here << " FCR "
             << " map " << GMSPolyUtility::myMapID << " item "
             << GMSPolyUtility::myItemID
             << " Did not find a good startCoord for the line direction"
             << ", coordOnHull " << coordOnHull << endl;
      return false;
      // Probably something wrong with the convHull of the hole or
      // self-touch part. Before changing the getAngleFromNorth metod
      // not to use conslat, the convex hull included incorrect coords
      // in some cases. That may have caused also this problem.
   }
   int32 deltaLat = coordOnHull.lat - startLat;
   int32 deltaLon = coordOnHull.lon - startLon;
   mc2dbg8 << "  line direction from " << startLat << ";"
           << startLon << " (deltaLat=" << deltaLat 
           << " deltaLon=" << deltaLon << ")" << endl;
   if ( printDebugs ) {
      cout << "  line direction from " << startLat << ";"
           << startLon << " (deltaLat=" << deltaLat 
           << " deltaLon=" << deltaLon << ")" << endl;
   }
   // length of drawstart 
   // (no need of coslat, I want length in mc2-units)
   float64 drawLineStartDist = 
      sqrt( SQUARE( (float64) deltaLat) + 
              SQUARE( (float64) deltaLon) );
   // normalize
   float64 deltaLatNorm = deltaLat / drawLineStartDist;
   float64 deltaLonNorm = deltaLon / drawLineStartDist;
   // add bbox width/height and calculate draw line end point
   uint32 lineLength = 
      ( polyBbox.getLonDiff() + polyBbox.getHeight());
   

   // Need to check that the coord in the end point of the line
   // is a valid coordinate
   // The latitude must not be beyond the north/south pole
   // The longitude must not be on the other side of the date-line
   
   // for latitude it must no be less than -MAX_INT32/2 or
   // more than MAX_INT32/2
   endLat = coordOnHull.lat + int(deltaLatNorm*lineLength);
   int32 tmpEndLat = abs(endLat);
   if ( tmpEndLat > (MAX_INT32/2 -1) ) {
      mc2log << error << here << " FCR "
             << " map " << GMSPolyUtility::myMapID << " item "
             << GMSPolyUtility::myItemID
             << " Invalid latitude (" << endLat << ") for end of line" 
             << ", coordOnHull " << coordOnHull
             << " start=" << startLat << ";" << startLon
             << endl;
      mc2dbg8 << "FCR  coordOnHull=" << coordOnHull << " start="
             << startLat << ";" << startLon 
             << " pointDist=" << pointDist
             << " deltaLat=" << deltaLat
             << " drawLineStartDist=" << drawLineStartDist
             << " deltaLatNorm=" << deltaLatNorm 
             << " lineLength=" << lineLength << endl
             << " tmpEndLat=" << tmpEndLat
             << " MAX_INT32=" << MAX_INT32
             << " MAX_INT32/2=" << MAX_INT32/2
             << " MAX_INT32/2-1=" << (MAX_INT32/2 -1)
             << endl;
      return false;
   }
   
   int64 tmpEndLon = abs (coordOnHull.lon + int(deltaLonNorm*lineLength) );
   if ( tmpEndLon > MAX_INT32-1) {
      mc2log << error << here << " FCR "
             << " map " << GMSPolyUtility::myMapID << " item "
             << GMSPolyUtility::myItemID
             << " Invalid longitude for end of line" << endl;
      return false;
   }
   endLon = coordOnHull.lon + int(deltaLonNorm*lineLength);
   mc2dbg8 << "  line length=" << lineLength 
           << " start " << coordOnHull.lat << ";" << coordOnHull.lon
           << " end " << endLat << ";" << endLon << endl;
   if ( polyBbox.contains(endLat,endLon) ) {
      mc2log << error << here << " FCR "
             << " map " << GMSPolyUtility::myMapID << " item "
             << GMSPolyUtility::myItemID
             << " Need to increase the length so the "
             << "end coord is outside the polyBbox" << endl;
      return false;
      // Return false instead of assert not to fail map gen
      //MC2_ASSERT(false);
   }
   mc2dbg8 << "  decide line took "
        << TimeUtility::getCurrentTime() - tmpTime << " ms"
        << endl;
   
   return true;
} // drawLineTowardsPolyBorder

void 
GMSPolyUtility::createAndSaveAMapFromGfxData( 
   GMSGfxData* gfx, uint32 mapID )
{
   mc2dbg << "GMSPolyUtility::createAndSaveAMapFromGfxData" << endl;
   // print the problem gfx to a mif file
   ofstream fileX( "gfxWithProblems.mif");
   gfx->printMif( fileX );

   // create a map with only one item with the problem gfx
   // the map needs a map gfx data, use the problem gfx for that as well
   GMSMap* theMap = static_cast<GMSMap*>
      (GMSMap::createMap( mapID, "./", false ) ); // do not load any old map
   theMap->getItemNames()->addString("MISSING");
   GMSWaterItem* wi = new GMSWaterItem();
   wi->setGfxData(gfx);
   theMap->addItem(wi, 4);
   theMap->setGfxData(gfx);
   theMap->save();
}

bool 
GMSPolyUtility::holesAreTouching( 
   GMSGfxData* gfx, 
   multimap<uint32, uint32> holeHierarchy )
{
   // From holeHierarchy, find out which poly are holes,
   // Loop all polys of the gfx data, fill a coordMap with the coords
   // of the polys that are holes
   
   multimap<uint32, uint32>::const_iterator hh;
   set<uint32> polysAreHoles;
   for ( hh = holeHierarchy.begin();
         hh != holeHierarchy.end(); hh++ ) {
      polysAreHoles.insert( hh->second );
   }
   mc2dbg8 << "polysAreHoles=" << polysAreHoles.size() << endl;
   if ( polysAreHoles.size() == 0 ) {
      return false;
   }

   
   // Collect coordinates from my polygon
   coordMap_t gfxCoords;
   set<uint32>::const_iterator p;
   for ( p = polysAreHoles.begin();
         p != polysAreHoles.end(); p++ ) {
      uint32 poly = *p;
      
      GMSGfxData::fillCoordMap( gfx, poly, gfxCoords, poly);
   }

   // Check if any coord is used more than once
   // Print info for checking in mcm map (debug8)
   uint32 nbrHolesTouchesCoords = 0;
   for ( coordMap_t::const_iterator it = gfxCoords.begin();
         it != gfxCoords.end(); it++ ) {
      idxSet_t idxSet = it->second;
      if ( idxSet.size() >= 2 ) {
         // Check which polys share this coord
         // If it is different polys, then we have holes touching each other
         set<uint32> polysSharingTheCoord;
         for ( idxSet_t::const_iterator id = idxSet.begin();
               id != idxSet.end(); id++ ) {
            polysSharingTheCoord.insert(id->first);
            mc2dbg8 << "holes share coord " 
                    << id->first << ":" << id->second << endl;
         }
         if ( polysSharingTheCoord.size() > 1 ) {
            nbrHolesTouchesCoords++;
            mc2dbg << "FCR  map " << GMSPolyUtility::myMapID 
                   << " item " << GMSPolyUtility::myItemID 
                   << " has holes touching in " 
                   << it->first.lat << ";" << it->first.lon << endl;
            //return true;
            //no, loop to get all hole-touches printed to log file
         }
      }
   }
   
   return ( nbrHolesTouchesCoords > 0 );
}

bool
GMSPolyUtility::mergeWaterItemsInCOMap( OldGenericMap* theCOMap )
{
   uint32 startTime = TimeUtility::getCurrentTime();
   // For waters do:
   // 1. merge items 2. merge item polygons 3. elim holes 4. elim selftouch
   mc2dbg << "GMSPolyUtility::mergeWaterItemsInCOMap map "
          << theCOMap->getMapName() << " "
          << theCOMap->getMapID() << endl;

   if ( ! MapBits::isCountryMap(theCOMap->getMapID()) ) {
      mc2dbg << "not a co map!" << endl;
      return false;
   }

   // First check if we really should merge etc the waters of this map
   if ( ! NationalProperties::mergeSameNameCloseItems(
         theCOMap->getCountryCode(), theCOMap->getMapOrigin(),
         ItemTypes::waterItem, theCOMap->getMapID() ) ) {
      mc2dbg << "national properties says no merge!" << endl;
      return false;
   } 

   mc2dbg << "Start merging closed items" << endl;
   uint32 nbrMerged = 
      mergeSameNameCloseItems( theCOMap, ItemTypes::waterItem, true );
   mc2dbg << "Merged for " << nbrMerged << " items" << endl;
   if ( nbrMerged == 0 ) {
      // nothing happened, return
      return false;
   }


   // The mergeItemPolygons etc will corrupt the polygons
   // since it will destroy the hole/selftouch elimination that was done in
   // the underview maps. And then running elimHoles+elimST on corrupt
   // polygons is not a good idea
   // So skip this forever!!!
//   mc2dbg << "Start merging item polygons" << endl;
//   nbrMerged = GMSPolyUtility::mergeItemPolygons( 
//                                 theCOMap, ItemTypes::waterItem );
//   mc2dbg << "Merged for " << nbrMerged << " items" << endl;
//
//   mc2dbg << "Eliminate holes" << endl;
//   eliminateHoles( theCOMap, ItemTypes::waterItem );
//   mc2dbg << "Eliminate self-touch" << endl;
//   eliminateSelfTouch( theCOMap, ItemTypes::waterItem );

   mc2dbg1 << "GMSPolyUtility::mergeWaterItemsInCOMap ends, took "
           << uint32(TimeUtility::getCurrentTime()-startTime) 
           << " ms" << endl;

   return true;
} // mergeWaterItemsInCOMap

uint32
GMSPolyUtility::mergeSameNameCloseItems( 
      OldGenericMap* theMap, ItemTypes::itemType itemType,
      bool processClosedPolygons )
{

   mc2log << info << "Called mergeSameNameCloseItems for " 
          << ItemTypes::getItemTypeAsString(itemType) 
          << " processClosedPolygons=" << int(processClosedPolygons)
          << endl;
   
   uint32 nbrMergedItems = 0; // return value
   
   // Check in-parameters.
   if ( (itemType == ItemTypes::builtUpAreaItem) ||
        (itemType == ItemTypes::municipalItem) ||
        (itemType == ItemTypes::zipCodeItem) ||
        (itemType == ItemTypes::cityPartItem) ){
      mc2log << error << "mergeSameNameCloseItems is not supposed "
             << "to be used for items being used as group, item type:" 
             << itemType << endl;
      MC2_ASSERT(false);
   }


   // Put all item merge candidates in a set.
   set<OldItem*> itemsToMerge;
   for (uint32 z=0; z< NUMBER_GFX_ZOOMLEVELS; z++) {
      // Check all items on current zoomlevel
      for ( uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
         OldItem* item  = theMap->getItem(z, i);
         if ((item == NULL) || 
             (item->getItemType() != itemType)) {
            continue;
         }
         GfxData* gfx = item->getGfxData();
         if ( gfx == NULL ) {
            continue;
         }
         bool closed = gfx->getClosed(0);
         if ( processClosedPolygons && ! closed ) {
            continue;
         }
         if ( ! processClosedPolygons && closed ) {
            continue;
         }
         itemsToMerge.insert(item);
      }
   }
   mc2dbg << "Total number of items to try to merge:" << itemsToMerge.size()
          << endl;
   

   // For each item, find all other items with the same names, and put them
   // in one set for each name combination for further processing.
   vector< set<OldItem*> > sameNameItemsSets;
   set<OldItem*> processedItems;
   for (set<OldItem*>::iterator itemIt = itemsToMerge.begin();
        itemIt != itemsToMerge.end(); ++itemIt ){
      set<OldItem*> sameNameItems;
      if ( processedItems.find(*itemIt) != processedItems.end() ){
         // This item has already been processed.
         continue;
      }
      processedItems.insert(*itemIt);
      //
      // Compare to all other items left.
      for (set<OldItem*>::iterator cmpIt = itemsToMerge.begin();
           cmpIt != itemsToMerge.end(); ++cmpIt ){
         if ( *cmpIt == *itemIt ){
            // same item.
            continue;
         }
         if ( processedItems.find(*cmpIt) != processedItems.end() ){
            // This item has already been processed.
            continue;
         }
         if ( (*itemIt)->hasSameNames(*cmpIt)) {
            sameNameItems.insert(*cmpIt);
            processedItems.insert(*cmpIt);
         }
      }
      if ( sameNameItems.size() > 0 ){
         sameNameItems.insert(*itemIt);
         sameNameItemsSets.push_back(sameNameItems);
         mc2dbg4 << "   Added same name set with " << sameNameItems.size() 
                 << " items." << endl;
      }
   }
   mc2dbg << "Total number of same name sets: " << sameNameItemsSets.size()
          << endl;


   // Check which items with the same name that are close to each other and
   // store them in itemsToMergeSets.
   vector< set<OldItem*> >itemsToMergeSets;
   uint32 maxMergeDistMeters = 20; // 20 meters
   uint64 maxMergeDist = 
      static_cast<int64>(GfxConstants::METER_TO_MC2SCALE*maxMergeDistMeters);
   for (uint32 i=0; i<sameNameItemsSets.size(); i++){
      set<OldItem*>& sameNameSet = sameNameItemsSets[i];
      mc2dbg8 << "for each same-name-set " << endl;
      // Handle a set of common names.
      while ( sameNameSet.size() > 0 ){
         mc2dbg8 << "   while still items to process" << endl;
         // Try to merge the first item still left to other items.
         OldItem* item1 = *(sameNameSet.begin());
         sameNameSet.erase(sameNameSet.begin());

         // Collect all items that are close enough for merging.
         set<OldItem*>mergeCandidates;
         mergeCandidates.insert(item1);
         bool stillGrowing = true;
         while (stillGrowing){
            mc2dbg8 << "      while still growing" << endl;
            stillGrowing = false;

            set<OldItem*>processed;
            set<OldItem*>::iterator item2It = sameNameSet.begin();
            while ( item2It != sameNameSet.end() ){
               mc2dbg8 << "         while" << endl;
               GfxData* gfx2 = (*item2It)->getGfxData();
               MC2BoundingBox bbox2;
               gfx2->getMC2BoundingBox(bbox2);
               bbox2.increaseMeters(maxMergeDistMeters);
               
               // Check closest distance between gfx2 and all items
               // collected for merge so far (at least item1).
               uint64 minDist = MAX_UINT64;
               for (set<OldItem*>::const_iterator distIt = 
                       mergeCandidates.begin();
                    distIt!=mergeCandidates.end(); ++distIt){
                  mc2dbg8 << "            for all collected items" << endl;
                  GfxData* toMergeGfx = (*distIt)->getGfxData();

                  // First check if they are close enough for a distance 
                  // calculation
                  MC2BoundingBox bboxToMerge;
                  toMergeGfx->getMC2BoundingBox(bboxToMerge);
                  if(bboxToMerge.overlaps(bbox2)){
                     
                     // Check if this distance is closer than any other 
                     // distance already stored.
                     uint64 tmpMinDist = toMergeGfx->minSquareDistTo(gfx2);
                     if ( tmpMinDist < minDist ){
                        minDist = tmpMinDist;
                     }
                  }
               }
               if (minDist < maxMergeDist ){
                  // The distance was short enough!
                  mergeCandidates.insert(*item2It);
                  processed.insert(*item2It);
                  stillGrowing = true;
               }
               ++item2It;
            }

            // Erase processed items from the set
            for (set<OldItem*>::const_iterator procIt = processed.begin();
                 procIt != processed.end(); ++procIt){
               mc2dbg8 << "         for items to erase" << endl;
               set<OldItem*>::iterator rmIt = sameNameSet.find(*procIt);
               sameNameSet.erase(rmIt);
            }
         }
         
         // Store items to merge.
         if (mergeCandidates.size() > 1){
            itemsToMergeSets.push_back(mergeCandidates);
         }
      } 
   } // for all same name items sets.  
   mc2dbg << "Total number of groups of items to merge: " 
          << itemsToMergeSets.size()
          << endl;


   // Merge all items in each items-to-merge-set.
   for (uint32 i=0; i<itemsToMergeSets.size(); i++){
      set<OldItem*>& mergeSet = itemsToMergeSets[i];
      
      // First item ( the one we keep )
      OldItem* item1 = (*mergeSet.begin());
      GMSGfxData* gfx1 = item1->getGfxData();
      nbrMergedItems++;
      mergeSet.erase(mergeSet.begin());
      mc2dbg << "Merging into item 0x" << hex << item1->getID() << ":";
      
      for (set<OldItem*>::const_iterator itemIt = mergeSet.begin();
           itemIt != mergeSet.end(); ++itemIt){
         OldItem* item2 = *itemIt;
         mc2dbg << " 0x" << item2->getID();
         nbrMergedItems++;

         // Merge gfx data
         GMSGfxData* gfx2 = item2->getGfxData();
         for (uint32 i=0; i < gfx2->getNbrPolygons(); i++) {
            if (! gfx1->addPolygon(gfx2, 
                                   false, // backwards
                                   i) ){
               mc2dbg << endl;
               mc2log << error << "mergeSameNameCloseItems " <<
                  "Could not add polygon when merging gfx data.";
               exit(1);
            }
         }

         // Add groups from merged item.
         for (uint32 g=0; g<item2->getNbrGroups(); g++){
            theMap->addRegionToItem(item1, item2->getGroup(g));
         }
         
         // Delete merged item from map.
         theMap->removeItem(item2->getID());
      }
      mc2dbg << dec << endl;
      
   } // for all items to merge sets.
   mc2dbg << "Merged totally " << nbrMergedItems << " items, resulting in "
          << itemsToMergeSets.size() << " larger items" << endl;
   
   return nbrMergedItems;
} // mergeSameNameCloseItems
   
uint32
GMSPolyUtility::mergeItemPolygons( OldGenericMap* theMap, 
                                   ItemTypes::itemType mergeItemType )
{
   uint32 nbrMerged = 0;
   uint32 start = TimeUtility::getCurrentTime();
   // Loop all items and merge polygons within items
   for(uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < theMap->getNbrItemsWithZoom(z); i++) {
         
         OldItem* item = theMap->getItem(z, i);
         
         if (item == NULL) {
            continue;
         }

         GfxData* gfx = item->getGfxData();
         if ( (gfx == NULL) || (gfx->getNbrPolygons() < 2) ) {
            continue;
         }

         ItemTypes::itemType type = item->getItemType();

         // Check if merge item type was given
         if ( (mergeItemType != ItemTypes::numberOfItemTypes) &&
              (type != mergeItemType) ) {
            continue;
         }
         
         // If we use index areas, skip merge the index area buas
         // since they are not used for display
         if ( NationalProperties::useIndexAreas(
                  theMap->getCountryCode(),
                  theMap->getMapOrigin()) &&
              (type == ItemTypes::builtUpAreaItem) &&
              (! theMap->itemNotInSearchIndex(item->getID())) ) {
            continue;
         }
         
         // Merge: only some standard item types
         if ( !  ((type==ItemTypes::waterItem) ||
                  (type==ItemTypes::parkItem) ||
                  (type==ItemTypes::forestItem) ||
                  (type==ItemTypes::builtUpAreaItem) ||
                  (type==ItemTypes::municipalItem)) ) {
            continue;
         }


         mc2dbg8 << "merge polygons of item " << item->getID() << endl;
         uint32 startTime = TimeUtility::getCurrentTime();
         GMSGfxData* newGfx = GMSGfxData::mergePolygons(gfx);
         if ( newGfx != NULL ) {
            item->setGfxData( newGfx );
            nbrMerged++;
            mc2dbg << "Merged polygons of item " << item->getID()
                   << " (" << ItemTypes::getItemTypeAsString(type)
                    << ") " << theMap->getFirstItemName(item)
                    << " p=" << newGfx->getNbrPolygons()
                          << "(" << gfx->getNbrPolygons() << ")"
                    << " c=" << newGfx->getTotalNbrCoordinates() 
                          << "(" << gfx->getTotalNbrCoordinates() << ")"
                    << " time=" << (TimeUtility::getCurrentTime() - startTime)
                    << " ms" << endl;
         }
      }
   }

   mc2dbg << "Merged item polygons of " << nbrMerged << " items in "
          << (TimeUtility::getCurrentTime() - start) << " ms" << endl;
   return nbrMerged;
} // mergeItemPolygons

