/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSGfxData.h"
#include "OldGenericMap.h"
#include <algorithm>
#include "GfxUtility.h"
#include "AllocatorTemplate.h"
#include <iostream>
#include <sstream>

GMSGfxData* 
GMSGfxData::createNewGfxData(DataBuffer* dataBuffer, OldGenericMap* theMap)
{
   // Will always return an GMSGfxData.
   GMSGfxData* gfx = theMap ?
      theMap->m_gfxDataAllocator->getNextObject() :
      new GMSGfxData;
      
   dataBuffer->alignToLong();

   gfx->setClosed(0, dataBuffer->readNextBool());     // closed (bool)
   dataBuffer->readNextByte();      // Type of Gfx, not used here!
   uint16 nbrPolygons = dataBuffer->readNextShort();  // nbrPolygons (uint16)

   gfx->m_polygons.resize( nbrPolygons );
   
   for(uint32 poly = 0; poly < nbrPolygons; poly++) {
      uint32 nbrCoords = dataBuffer->readNextLong();  // nbrCoords (uint32)
      DEBUG_DB(mc2dbg << "      nbrCoords in polygon: " << nbrCoords << endl);
      gfx->m_polygons[poly].first.reserve( nbrCoords );

      // Add the points
      MC2Coordinate coord;
      for(uint32 j = 0; j < nbrCoords; j++) {
         coord.lat = dataBuffer->readNextLong();
         coord.lon = dataBuffer->readNextLong();
         gfx->m_polygons[poly].first.push_back( coord );
      }

      // Set the length
      gfx->setLength(poly, dataBuffer->readNextFloat() );
      // Old maps do not have float length.
      gfx->updateLength( poly );
   }

   gfx->updateBBox();
   
   return gfx;
}

GMSGfxData*
GMSGfxData::createNewGfxData(OldGenericMap* theMap, bool createNewPolygon)
{  
   
   GMSGfxData* gfx = NULL;
   if (theMap != NULL) {
      gfx = theMap->m_gfxDataAllocator->getNextObject();
   } else { 
      gfx = new GMSGfxData();
   }
   //GMSGfxData* gfx = new GMSGfxData();

   if (createNewPolygon) {
      gfx->addPolygon();
   }

   return gfx;
}

GMSGfxData* 
GMSGfxData::createNewGfxData(OldGenericMap* theMap, const MC2BoundingBox* bbox)
{
   GMSGfxData* gfx = createNewGfxData(theMap);

   gfx->addCoordinate(bbox->getMinLat(), bbox->getMinLon(), true);
   gfx->addCoordinate(bbox->getMaxLat(), bbox->getMinLon());
   gfx->addCoordinate(bbox->getMaxLat(), bbox->getMaxLon());
   gfx->addCoordinate(bbox->getMinLat(), bbox->getMaxLon());
   gfx->setClosed(0, true);
   gfx->updateBBox();
   gfx->updateLength();
   return gfx;
}

GMSGfxData* 
GMSGfxData::createNewGfxData(OldGenericMap* theMap,                                                             const coordinate_type* firstLat,
                          const coordinate_type* firstLon,
                          uint16 nbrCoords,
                          bool closed)
{
   GMSGfxData* gfx = createNewGfxData(theMap);

   for (uint16 i=0; i < nbrCoords; i++) {
      gfx->addCoordinate(firstLat[i], firstLon[i], i==0);
   }
   gfx->setClosed(0, closed);
   gfx->updateBBox();
   gfx->updateLength();
   return gfx;
}


GMSGfxData* 
GMSGfxData::createGfxData(GMSGfxData** p_gfxs, 
                                       int nbrGfxs)
{
   // Create the GfxData to return, without any polygon!
   GMSGfxData* returnGfx = new GMSGfxData();

   // Add the first GfxData to the returnGfxData
   if ( (p_gfxs == NULL) || 
        (nbrGfxs < 1) || 
        (!returnGfx->addPolygon(p_gfxs[0]))) {
      MC2WARNING("GMSGfxData::createGfx Failed to add Gfx");
      delete returnGfx;
      return (NULL);
   }

   // Create a copy of the array with pointers to gfxs
   GMSGfxData** gfxs = new GMSGfxData*[nbrGfxs];
   for (int i=0; i<nbrGfxs; i++) {
      gfxs[i] = p_gfxs[i];
   }
   gfxs[0] = NULL; // Already added number 0 to returnGfx
   
   // Loop until all is set or nothing is changed
   //
   // This will not work when starting an an island or having
   // several maps that not are connected to the others!!!
   // 
   bool allDone = false;
   if (nbrGfxs == 1) {
      allDone = true;
   }
   cout << "Tying to concatinate " << nbrGfxs << " GfxData's" << endl;
   bool oneSet = true;
   while ((!allDone) && (oneSet)) {
      oneSet = false;
      for (int i=0; i<nbrGfxs; i++) {
         if (gfxs[i] != NULL) {
            cout << "---------------------" << endl;
            cout << "Tryes to add gfx nbr " << i << endl;

            if (returnGfx->addOutline(gfxs[i])) {
               oneSet = true;
               cout << "Gfx number " << i << " added" << endl;
               gfxs[i] = NULL;

            } else {
               cout << "Gfx number " << i << " does not fit in!" << endl;
            }
         }
      }

      // Check if all is done (allMunicipals[k] == NULL for all k)
      int k=0;
      while ((k < nbrGfxs) && (gfxs[k] == NULL)) {
         k++;
      }
      cout << "k = " << k << endl;
      if (k == nbrGfxs) {
         cout << "setting allDone to true!" << endl;
         allDone = true;
      }
   }

   DEBUG1(
      if (allDone) {
         cerr << "   ALL done" << endl;
      } else {
         for (int i=0; i<nbrGfxs; i++) {
            if (gfxs[i] != NULL) {
               cout << "   Failed to add gfx number " << i << endl;
            }
         }
      }
   );
   
   // Delete the memory allocated here
   delete gfxs;

   return (returnGfx);
}


GMSGfxData*
GMSGfxData::createNewGfxData(OldGenericMap* theMap, const GfxData* src)
{
   GMSGfxData* dest = createNewGfxData(theMap, false);

   for (uint16 p=0; p<src->getNbrPolygons(); ++p) {
      for (uint32 i=0; i<src->getNbrCoordinates(p); ++i) {
         dest->addCoordinate(src->getLat(p,i), src->getLon(p,i), i==0);
      }
      dest->setClosed(p, src->getClosed(p));
   }
   dest->updateBBox();
   dest->updateLength();
   return dest;
}



// Note: this is probably not a trustworthy method!
bool
GMSGfxData::addOutline(GMSGfxData* externGfx)
{

   // Start with checking the distance between the boundingboxes
   MC2BoundingBox thisBBox, extBBox;
   if (!this->getMC2BoundingBox(thisBBox) ||
       !externGfx->getMC2BoundingBox(extBBox)) {
      cerr << "addOutline:: Error getting boundingboxes." << endl;
      return false;
   }
   
   if(!thisBBox.overlaps(extBBox)) {
      DEBUG4(cerr << "addOutline:: The GfxData's did not overlap!" << endl);
      return (false);
   }

   // Make sure that the coordinates are unique
   this->removeIdenticalCoordinates();
   externGfx->removeIdenticalCoordinates();
   
   DEBUG4(cerr << "FU: Outline, coord 0 = (" << getLat(0,0) 
               << ", " << getLon(0,0) << ")" << endl);

   bool retVal = true;
   const uint32 COMMON_PART_EXTRA_FACTOR = 1;

   // The maximum possible sizes
   DEBUG4(cout << "new vecor-sizes = " << dec 
               << externGfx->getNbrCoordinates(0) << "(ext) + " 
               << this->getNbrCoordinates(0) << "(int)" << endl);
   
   point_vector_type newPolygon;
   newPolygon.reserve(externGfx->getNbrCoordinates(0)
         + this->getNbrCoordinates(0));

   uint64 dist = 0;

   // Two cases: 1. The first side should _not_ be include in the new polygon
   //            2. The first side should be included in the new polygon
   int32 firstOwnCommonCoord = 0;
   int32 lastOwnCommonCoord = 0;
   if ( externGfx->squareDistTo(getLat(0,0), getLon(0,0), 0 ) < 
          MAX_MUNICIPAL_SQUARE_DISTANCE ) {
      // Case 1
      DEBUG4(cout << "Case 1 (first point not to be included)" << endl);
      /*
      while (firstOwnCommonCoord < int32(getNbrCoordinates(0)) &&
            (dist = externGfx->squareDistTo(
                                 getLat(firstOwnCommonCoord), 
                                 getLon(firstOwnCommonCoord), 
                                 0) ) < 
          MAX_MUNICIPAL_SQUARE_DISTANCE ) {
         firstOwnCommonCoord++;
      }
      // Compensate because the loop does not stop until it is to late
      if ( (dist > MAX_MUNICIPAL_SQUARE_DISTANCE) &&
           (firstOwnCommonCoord < int32(getNbrCoordinates(0))))
         firstOwnCommonCoord--;
      */

      firstOwnCommonCoord = -1;
      while (firstOwnCommonCoord < int(getNbrCoordinates(0)-1) &&
            (dist = externGfx->squareDistTo(
                                 getLat(0,firstOwnCommonCoord+1), 
                                 getLon(0,firstOwnCommonCoord+1), 
                                 0) ) < 
          MAX_MUNICIPAL_SQUARE_DISTANCE ) {
         firstOwnCommonCoord++;
      }
      if (dist < MAX_MUNICIPAL_SQUARE_DISTANCE) {
         firstOwnCommonCoord++;
      }
      
      /*lastOwnCommonCoord = getNbrCoordinates(0)-1;
      while (lastOwnCommonCoord > 0 &&
              (dist = externGfx->squareDistTo(
                                 getLat(lastOwnCommonCoord), 
                                 getLon(lastOwnCommonCoord), 
                                 0) ) < 
            MAX_MUNICIPAL_SQUARE_DISTANCE ) {
         lastOwnCommonCoord--;
      }
      // Compensate because the loop does not stop until it is to late
      if ( (dist > MAX_MUNICIPAL_SQUARE_DISTANCE) &&
           (lastOwnCommonCoord >= 0))
         lastOwnCommonCoord++;
      */

      lastOwnCommonCoord = getNbrCoordinates(0);
      while (lastOwnCommonCoord > 0 &&
              (dist = externGfx->squareDistTo(
                                 getLat(0,lastOwnCommonCoord-1), 
                                 getLon(0,lastOwnCommonCoord-1), 
                                 0) ) < 
            MAX_MUNICIPAL_SQUARE_DISTANCE ) {
         lastOwnCommonCoord--;
      }
      if (dist < MAX_MUNICIPAL_SQUARE_DISTANCE) {
         lastOwnCommonCoord--;
      }

   } else {
      // Case 2
      DEBUG4(cout << "Case 2 (first point to be included)" << endl);
      firstOwnCommonCoord = getNbrCoordinates(0)-1;
      while ( (firstOwnCommonCoord > 0 &&
              (dist = externGfx->squareDistTo(  
                                 getLat(0,firstOwnCommonCoord), 
                                 getLon(0,firstOwnCommonCoord), 
                                 0) ) > 
            MAX_MUNICIPAL_SQUARE_DISTANCE ) ) {
         firstOwnCommonCoord--;
      }

      lastOwnCommonCoord = 0;
      while ( (lastOwnCommonCoord < int32(getNbrCoordinates(0))) &&
              ((dist = externGfx->squareDistTo(
                                 getLat(0,lastOwnCommonCoord),
                                 getLon(0,lastOwnCommonCoord), 
                                 0) ) > 
               MAX_MUNICIPAL_SQUARE_DISTANCE )) {
         lastOwnCommonCoord++;
      }
   }

   DEBUG4(cout << "lastOwnCommonCoord = " << lastOwnCommonCoord
               << ", firstOwnCommonCoord = " << firstOwnCommonCoord
               << ", getNbrCoords() = " << getNbrCoordinates()
               << endl);

   // Check that the two gfx's fits together ok
   // 1.  lastOwnCommonCoord OK?
   if ( (lastOwnCommonCoord < 0) || 
        (lastOwnCommonCoord >= int32(getNbrCoordinates(0))) ) {
      DEBUG1(cout << "FAILURE! (1): lastOwnCommonCoord < 0 "
                  << "or >= getNbrCoordinates(0)" << endl);
      retVal = false;
   }

   // 2.  firstOwnCommonCoord OK?
   if ( (firstOwnCommonCoord < 0) ||
        (firstOwnCommonCoord >= int32(getNbrCoordinates(0)) )) {
      DEBUG1(cout << "FAILURE! (2): firstOwnCommonCoord >= nbrCoordinates"
                  << "or <  0" << endl);
      retVal = false;
   }

   DEBUG1(  if (retVal) {
               cout << "   lastOwnCommonCoord = "
               << " (" << getLat(0,lastOwnCommonCoord) << "," 
                       << getLon(0,lastOwnCommonCoord) << endl
               << "   firstOwnCommonCoord = " 
               << " (" << getLat(0,firstOwnCommonCoord) << "," 
                       << getLon(0,firstOwnCommonCoord) << endl;
             }                                                    )
   
   // 3. Are the combination of firstOwnCoordinate and lastOwnCoordinate
   //    acceptable (not just one point?)?
   if ( abs(firstOwnCommonCoord - lastOwnCommonCoord) < 1) {
      DEBUG1(cout << "FAILURE! (3): firstOwnCommonCoord == lastOwnCoomonCoord"
                  << endl);
      retVal = false;
   }

   // 4.  Does the common part of the polygones fit (last->first)?
   if (retVal) {
      int32 i = lastOwnCommonCoord;
      while ((i != firstOwnCommonCoord) && (retVal)){
         dist = externGfx->squareDistTo(getLat(0,i), getLon(0,i), 0);
         if ( dist > MAX_MUNICIPAL_SQUARE_DISTANCE*COMMON_PART_EXTRA_FACTOR ) {
            DEBUG1(cout << " FAILURE! (4) i == " << i << ", dist = " 
                        << sqrt(dist) << "lat = " << getLat(0,i) 
                        << ", lon = " << getLon(0,i) 
                        << ", lastOwnCommonCoord = "
                        << lastOwnCommonCoord << ", firstOwnCommonCoord = "
                        << firstOwnCommonCoord << endl;
            );
            retVal = false;
         }
         if (i < (int32) (getNbrCoordinates(0)-1))
            i++;
         else
            i = 0;
      }
   }

   // 5.  Are the polygones separated in the "uncommon" part (first->last)?
   if (retVal) {
      int32 i = firstOwnCommonCoord;
      if (i < (int32) (getNbrCoordinates(0)-1))
         i++;
      else
         i = 0;
      while ((i != lastOwnCommonCoord) && (retVal)) {
         dist = externGfx->squareDistTo(getLat(0,i), getLon(0,i), 0);
         if ( dist < MAX_MUNICIPAL_SQUARE_DISTANCE ) {
            DEBUG1(cout << " FAILURE! (5) i == " << i << ", dist = " 
                        << sqrt(dist) << "lat = " << getLat(0,i) 
                        << ", lon = " << getLon(0,i) 
                        << ", lastOwnCommonCoord = "
                        << lastOwnCommonCoord << ", firstOwnCommonCoord = "
                        << firstOwnCommonCoord << endl;
            );
            retVal = false;
         }
         if (i < (int32) (getNbrCoordinates(0)-1))
            i++;
         else
            i = 0;
      }
   }

   if (retVal) {
      int32 i = firstOwnCommonCoord;
      while (i != lastOwnCommonCoord) {
         //newLat[curNewCoord] = getLat(i);
         //newLon[curNewCoord] = getLon(i);
         newPolygon.push_back( MC2Coordinate(getLat(0,i), getLon(0,i)));
         //curNewCoord++;
         if (i < (int32) (getNbrCoordinates(0)-1))
            i++;
         else
            i = 0;
      }
      // Copy the last coordinate!
      newPolygon.push_back( MC2Coordinate(getLat(0,lastOwnCommonCoord), 
                                          getLon(0,lastOwnCommonCoord)));
      DEBUG4(cout << "   All own coords copied (" << firstOwnCommonCoord 
                  << " --> " << lastOwnCommonCoord << ") = " 
                  << abs(lastOwnCommonCoord-firstOwnCommonCoord) << endl);


      // Two cases: 1. The first side should _not_ be included in the 
      //               new polygon
      //            2. The first side should be included in the new polygon
      int32 firstExtCommonCoord;
      int32 lastExtCommonCoord;
      if ( squareDistTo(externGfx->getLat(0,0), externGfx->getLon(0,0), 0 ) < 
           MAX_MUNICIPAL_SQUARE_DISTANCE ) {
         // Case 1
         DEBUG4(cout << "II Case 1 (first external point not to be included)" 
                     << endl);

         // - - - - - - - - - - - - - - - - - - - - - - firstExtCommonCoord
         firstExtCommonCoord = 0;
         while ( (firstExtCommonCoord < 
                     int32(externGfx->getNbrCoordinates(0))-1) &&
                 (dist = squareDistTo(externGfx->getLat(0,firstExtCommonCoord), 
                                      externGfx->getLon(0,firstExtCommonCoord),
                                      0) ) <
                  MAX_MUNICIPAL_SQUARE_DISTANCE ) {
            firstExtCommonCoord++;
         }
         // Compensate because the loop does not stop until it is to late
         if ( (dist > MAX_MUNICIPAL_SQUARE_DISTANCE) &&
              (firstExtCommonCoord > 0) ) {
            firstExtCommonCoord--;
         }


         // - - - - - - - - - - - - - - - - - - - - - - lastExtCommonCoord
         lastExtCommonCoord = externGfx->getNbrCoordinates(0)-1;
         while (lastExtCommonCoord > 0 &&
                 (dist = squareDistTo(externGfx->getLat(0,lastExtCommonCoord), 
                                      externGfx->getLon(0,lastExtCommonCoord), 
                                      0)) < 
               MAX_MUNICIPAL_SQUARE_DISTANCE ) {
            lastExtCommonCoord--;
         }
         // Compensate because the loop does not stop until it is to late
         if ( (dist > MAX_MUNICIPAL_SQUARE_DISTANCE) &&
              (lastExtCommonCoord < int32(externGfx->getNbrCoordinates(0))-1)){
            lastExtCommonCoord++;
         }

      } else {
         // Case 2
         DEBUG4(cout << "II Case 2 external (first point included)" << endl);
         float64 d1, d2;
         // - - - - - - - - - - - - - - - - - - - - - - firstExtCommonCoord
         firstExtCommonCoord = externGfx->getNbrCoordinates(0) - 1;
         /*while (firstExtCommonCoord > 0 &&
                 (dist = squareDistTo(  
                              externGfx->getLat(firstExtCommonCoord), 
                              externGfx->getLon(firstExtCommonCoord), 
                              0) ) > 
               MAX_MUNICIPAL_SQUARE_DISTANCE ) {
            firstExtCommonCoord--;
         }*/
         do {
            firstExtCommonCoord--;
            d1 = GfxUtility::squareP2Pdistance_linear(
                                 getLat(0,lastOwnCommonCoord),
                                 getLon(0,lastOwnCommonCoord),
                                 externGfx->getLat(0,firstExtCommonCoord),
                                 externGfx->getLon(0,firstExtCommonCoord));

            d2 = GfxUtility::squareP2Pdistance_linear(
                                 getLat(0,firstOwnCommonCoord),
                                 getLon(0,firstOwnCommonCoord),
                                 externGfx->getLat(0,firstExtCommonCoord),
                                 externGfx->getLon(0,firstExtCommonCoord));
         } while ( (firstExtCommonCoord > 0) &&
                   (d1 > 2*MAX_MUNICIPAL_SQUARE_DISTANCE) &&
                   (d2 > 2*MAX_MUNICIPAL_SQUARE_DISTANCE));

         // - - - - - - - - - - - - - - - - - - - - - - lastExtCommonCoord
         lastExtCommonCoord = 0;
         /*while (lastExtCommonCoord < externGfx->getNbrCoordinates(0) -1 &&
               (dist = squareDistTo(externGfx->getLat(lastExtCommonCoord), 
                                    externGfx->getLon(lastExtCommonCoord), 
                                    0) ) > 
               MAX_MUNICIPAL_SQUARE_DISTANCE ) {
            lastExtCommonCoord++;
         }*/
         do {
            lastExtCommonCoord++;
            d1 = GfxUtility::squareP2Pdistance_linear(
                                    getLat(0,lastOwnCommonCoord),
                                    getLon(0,lastOwnCommonCoord),
                                    externGfx->getLat(0,lastExtCommonCoord),
                                    externGfx->getLon(0,lastExtCommonCoord));

            d2 = GfxUtility::squareP2Pdistance_linear(
                                    getLat(0,firstOwnCommonCoord),
                                    getLon(0,firstOwnCommonCoord),
                                    externGfx->getLat(0,lastExtCommonCoord),
                                    externGfx->getLon(0,lastExtCommonCoord));
         } while ( (lastExtCommonCoord < 
                        int(externGfx->getNbrCoordinates(0)-1)) &&
                   (d1 > 2*MAX_MUNICIPAL_SQUARE_DISTANCE) &&
                   (d2 > 2*MAX_MUNICIPAL_SQUARE_DISTANCE));
      }

      // Check that the external coordinate indices are valid
      if ((firstExtCommonCoord < 0 ) ||
          (lastExtCommonCoord >= int(externGfx->getNbrCoordinates(0)))) {
         MC2ERROR2("firstExtCommonCoord<0 || lastExtCommonCoord>=N",
                   cerr << "   firstExtCommonCoord=" << firstExtCommonCoord 
                        << ", lastExtCommonCoord=" << lastExtCommonCoord 
                        << ", externGfx->getNbrCoords(0)="
                        << externGfx->getNbrCoordinates(0) << endl;);
         retVal = false;
      } else {

         DEBUG4(cout << "   copy from external coord " << firstExtCommonCoord 
                     << " to " << lastExtCommonCoord << endl
                     << "   lastExtCommonCoord = "
                     << " (" << externGfx->getLat(0,lastExtCommonCoord) << "," 
                             << externGfx->getLon(0,lastExtCommonCoord) << ")" 
                     << endl
                     << "   firstExtCommonCoord = " 
                     << " (" << externGfx->getLat(0,firstExtCommonCoord) << "," 
                             << externGfx->getLon(0,firstExtCommonCoord) << ")" 
                     << endl);


         // Check what point of the external gfx to be the next one
         float64 d1 = GfxUtility::squareP2Pdistance_linear(
                                 getLat(0,lastOwnCommonCoord),
                                 getLon(0,lastOwnCommonCoord),
                                 externGfx->getLat(0,lastExtCommonCoord),
                                 externGfx->getLon(0,lastExtCommonCoord));

         float64 d2 = GfxUtility::squareP2Pdistance_linear(
                                 getLat(0,lastOwnCommonCoord),
                                 getLon(0,lastOwnCommonCoord),
                                 externGfx->getLat(0,firstExtCommonCoord),
                                 externGfx->getLon(0,firstExtCommonCoord));

         // Take care of the special case when _all_ coordinates in one
         // of the polygons are close to the other! In this case, both
         // d1 and d2 are greater than MAX_MUNICIPAL_SQUARE_DISTANCE 
         if ( (d1 > MAX_MUNICIPAL_SQUARE_DISTANCE) &&
              (d2 > MAX_MUNICIPAL_SQUARE_DISTANCE) ){
            DEBUG1(cerr << "addOutline: Both distances larger than limit, "
                        << "d1=" << d1 << ", d2=" << d2 << endl);
            
            retVal = true;

            // Find out if it is the externGfx that should be removed or
            // it is this one
            uint32 i = 0;
            bool allCoordsClose = true;
            while ( (i < externGfx->getNbrCoordinates(0)) &&
                    (!allCoordsClose)) {
               if (squareDistTo(externGfx->getLat(0,i), externGfx->getLon(0,i)) >
                    MAX_MUNICIPAL_SQUARE_DISTANCE) {
                  DEBUG1(cerr << "   Found distance greater than limit, i="
                              << i << endl);
                  allCoordsClose = false;
               }
               i++;
            }

            // If all coordinates in the external gfx are close to this one,
            // none of them should be included in the resulting gfx!
            if (!allCoordsClose) {
               MC2ERROR("GMSGfxData:addOutline. Not all coordinates close "
                        "but distance grater than limit");
               DEBUG1(cerr << "   i=" << i << ", d1=" << d1 
                           << ", d2=" << d2 << endl);
            }

         } else if (d1 < d2) {
            // The distance to the lastExt-coordinate is the smallest
            DEBUG4(cout << "   copy from lastExt --> firstExt" << endl);
            i = lastExtCommonCoord;
            while (i != firstExtCommonCoord) {
               //newLat[curNewCoord] = externGfx->getLat(i);
               //newLon[curNewCoord] = externGfx->getLon(i);
               newPolygon.push_back(MC2Coordinate(externGfx->getLat(0,i),
                                                  externGfx->getLon(0,i)));
               //curNewCoord++;
               if (i > 0) 
                  i--;
               else
                  i = externGfx->getNbrCoordinates(0) - 1;
            }
            //newLat[curNewCoord] = externGfx->getLat(firstExtCommonCoord);
            //newLon[curNewCoord] = externGfx->getLon(firstExtCommonCoord);
            newPolygon.push_back(MC2Coordinate(
                  externGfx->getLat(0,firstExtCommonCoord),
                  externGfx->getLon(0,firstExtCommonCoord)));
            //curNewCoord++;
            DEBUG4(cout << "   All ext coords copied (" << lastExtCommonCoord 
                        << " --> " << firstExtCommonCoord << ")" << endl);
         } else {
            DEBUG4(cout << "   copy from firstExt --> lastExt" << endl);
            i = firstExtCommonCoord;
            while (i != lastExtCommonCoord) {
               //newLat[curNewCoord] = externGfx->getLat(i);
               //newLon[curNewCoord] = externGfx->getLon(i);
               newPolygon.push_back(MC2Coordinate(externGfx->getLat(0,i),
                                                  externGfx->getLon(0,i)));
               //curNewCoord++;
               if (i < (int32) (externGfx->getNbrCoordinates(0) - 1) )
                  i++;
               else
                  i = 0;
            }
            //newLat[curNewCoord] = externGfx->getLat(lastExtCommonCoord);
            //newLon[curNewCoord] = externGfx->getLon(lastExtCommonCoord);
            newPolygon.push_back(MC2Coordinate(
                  externGfx->getLat(0,lastExtCommonCoord),
                  externGfx->getLon(0,lastExtCommonCoord)));
            //curNewCoord++;
            DEBUG4(cout << "   All ext coords copied (" << firstExtCommonCoord 
                        << " --> " << lastExtCommonCoord << ")" << endl);
         }

         DEBUG4(cout << "d1 = " << d1 << ", d2 = " << d2 << endl);

         DEBUG4(cout << "   lat, lon delted" << endl);

         m_polygons.clear();

         m_polygons.push_back(make_pair(newPolygon, 0));
         updateLength();
         updateBBox();

         DEBUG4(cout << "   lat, lon filled" << endl);
      } // if (retVal)
   } // if (retVal)
   return (retVal);
}
 


bool
GMSGfxData::updateLength( uint16 poly ) {
   return GfxDataFull::updateLength( poly );
}


GMSGfxData*
GMSGfxData::removeInsideCoordinates(GfxDataFull* complexGfx)
{
   mc2dbg2 << "complex, nbrPoly=" << complexGfx->getNbrPolygons()
           << " totNbrCoords=" << complexGfx->getTotalNbrCoordinates() << endl;
   complexGfx->removeIdenticalCoordinates();
   mc2dbg4 << " removeIdenticalCoordinates -> " 
           << complexGfx->getTotalNbrCoordinates() << endl;

   uint16 nbrPolygons = complexGfx->getNbrPolygons();

   if (nbrPolygons > 1) {
      
      // For each polygon, check if any vertices are on the boundry of the
      // comlexGfx. If so, add the vertices to a polygonGfx in the gfxs-vector.
      GfxData** gfxs = new GfxData*[nbrPolygons];
      uint16 nbrGfxs = 0;
      uint32 totNbrCoords = 0;
      uint32 totNbrPoly = 0;
      for (uint16 p = 0; p < nbrPolygons; p++) {
         uint32 nbrCoords = complexGfx->getNbrCoordinates(p);
         bool firstEqualsLast = 
            ((complexGfx->getLat(p,0) == complexGfx->getLat(p,nbrCoords-1)) &&
             (complexGfx->getLon(p,0) == complexGfx->getLon(p,nbrCoords-1)));
         mc2dbg4 << " " << p << ":" << complexGfx->getNbrCoordinates(p) 
                 << " firstEqualsLast=" << firstEqualsLast << endl;
         mc2dbg4 << "  clockwise=" << complexGfx->clockWise(p) << endl;

         GfxDataFull* pgfx = createNewGfxData(NULL, false);

         int32 pLat, pLon, revpLat, revpLon;
         uint32 c = 0;
         int32 firstlat = complexGfx->getLat(p,c);
         int32 firstlon = complexGfx->getLon(p,c);
         int32 secondlat, secondlon;
         bool createdPol = false;
         bool addedCoords = false;
         while ( ((c+1) < complexGfx->getNbrCoordinates(p)) ||
                 (!firstEqualsLast && 
                  ((c+1) <= complexGfx->getNbrCoordinates(p))) ) {

            if ((c+1) < complexGfx->getNbrCoordinates(p)) {
               secondlat = complexGfx->getLat(p, c+1);
               secondlon = complexGfx->getLon(p, c+1);
            } else {
               secondlat = complexGfx->getLat(p, 0);
               secondlon = complexGfx->getLon(p, 0);
            }
           
            mc2dbg8 << c  << " first (" 
                    << firstlat << "," << firstlon << ")"
                    << " second ("
                    << secondlat << "," << secondlon << ")" << endl;
                
            if ( !((firstlat == secondlat) && (firstlon == secondlon)) ) {
               
               // Generate one point on both sides of the segment between
               // first coord and second coord.
               GfxUtility::generatePointInMap(
                     firstlat, firstlon, secondlat, secondlon, pLat, pLon);
               GfxUtility::generatePointInMap(
                     secondlat, secondlon, firstlat, firstlon, 
                     revpLat, revpLon);

               if (!complexGfx->insidePolygon(pLat, pLon) ||
                   !complexGfx->insidePolygon(revpLat, revpLon)) {
                  // The segment is on the boundry of complexGfx
                  if (!addedCoords) {
                     // add the first coord to new polygon.
                     pgfx->addCoordinate(firstlat, firstlon, true);
                     addedCoords = true;
                     createdPol = true;
                     mc2dbg8 << " adding coord (new poly)";
                  }
                  // add the second coord
                  pgfx->addCoordinate(secondlat, secondlon);
                  mc2dbg8 << " adding coord" << endl;

               // if addedCoords and then there is a segment that is NOT
               // on the boundry, set addedCoords to false.
               // -> if more coords from this polygon is on the boundry 
               //    they should be in a new polygon in the pgfxData
               } else if (addedCoords) {
                  addedCoords = false;
               }
               
            } else {
               mc2dbg8 << " Duplicated coords (first == second), skip" << endl;
            }
               

            firstlat = secondlat;
            firstlon = secondlon;
            c++;

         }

         if (createdPol) {
            mc2dbg4 << "for poly " << p << ": polygonGfxs, " 
                    << pgfx->getNbrPolygons() << " polys and "
                    << pgfx->getTotalNbrCoordinates() << " coords" << endl;
            totNbrPoly += pgfx->getNbrPolygons();
            totNbrCoords += pgfx->getTotalNbrCoordinates();
            gfxs[nbrGfxs] = pgfx;
            nbrGfxs++;
         } else {
            delete pgfx;
         }
         
      }

      mc2dbg2 << "Created " << nbrGfxs << " polygonGfxs with tot " 
              << totNbrPoly << " polys and " << totNbrCoords 
              << " coords" << endl;
     
      // The gfxData to return
      GMSGfxData* returnGfx = createNewGfxData(NULL, true);
      
      /*
      // Simple loop for adding all the pgfxs to the returnGfx, without
      // considering order.
      for (uint32 g = 0; g < nbrGfxs; g++) {
         for (uint16 p = 0; p < gfxs[g]->getNbrPolygons(); p++) {
            if (!returnGfx->add(gfxs[g], false, p)) {
               mc2log << here << "error adding gfx=" << g 
                              << " poly=" << p << endl;
            }
         }
      }*/
      
      // Add the pgfxs to the returnGfx in the correct order!
      // Match the last ccord in returnGfx with the first|last ccord of 
      // the other pgfxs (all polygons). If the same coord add pgfx(p)
      // to returnGfx.
      // If no match, create a new Polygon and add the first one not yet added
      // and loop again.

      uint32 added[totNbrPoly];
      for (uint32 p = 0; p < totNbrPoly; p++) {
         added[p] = 0;
      }

      // add the first polygon of the first pgfx in gfxs
      returnGfx->add(gfxs[0]); //p=0 as default
      added[0] = 1;
      int32 lastLat = returnGfx->getLastLat(returnGfx->getNbrPolygons()-1);
      int32 lastLon = returnGfx->getLastLon(returnGfx->getNbrPolygons()-1);
      
      //
      bool allDone = false;
      if (totNbrPoly == 1) {
         allDone = true;
      } else {
         mc2dbg2 << "To concatinate all pgfxDatas, totNbrPoly=" 
                 << totNbrPoly << endl;
      }
      while ( !allDone ) {
         
         // Loop until all is set or nothing is changed
         // 
         bool oneAdded = true;
         while ((!allDone) && (oneAdded)) {
            oneAdded = false;
            uint32 polyNbr = 0;
            for (uint32 i=0; i<nbrGfxs; i++) {
               for (uint16 p = 0; p < gfxs[i]->getNbrPolygons(); p++) {
                  if (added[polyNbr] == 0) {
                     mc2dbg4 << "Tryes pgfx " << i << ":" << p 
                             << ", polyNbr=" << polyNbr << endl;
                     
                     uint32 nbrC = gfxs[i]->getNbrCoordinates(p);
                     bool addThis = false;
                     bool backwards = false;
                     if ((gfxs[i]->getLat(p, 0) == lastLat) && 
                         (gfxs[i]->getLon(p, 0) == lastLon)) {
                        addThis = true;
                     } else if ((gfxs[i]->getLat(p, nbrC-1) == lastLat) &&
                                (gfxs[i]->getLon(p, nbrC-1) == lastLon)) {
                        addThis = true;
                        backwards = true;
                     }
                     
                     if (addThis) {
                        if (returnGfx->add(gfxs[i], backwards, p)) {
                           oneAdded = true;
                           mc2dbg4 << " Gfx " << i << ":" << p 
                                   << " added" << endl;
                           added[polyNbr] = 1;
                           lastLat = returnGfx->getLastLat(
                                          returnGfx->getNbrPolygons()-1);
                           lastLon = returnGfx->getLastLon(
                                          returnGfx->getNbrPolygons()-1);
                        } else {
                           mc2dbg2 << "Failed to add gfx " << i << ":" << p 
                                   << ", polyNbr=" << polyNbr << endl;
                        }
                     }
                  }
                  // increase polyNbr
                  polyNbr++;
               }
            }

            // Check if all is done (added[k] == 1 for all k)
            uint32 k=0;
            while ((k < totNbrPoly) && (added[k] == 1)) {
               k++;
            }
            if (k == totNbrPoly) {
               allDone = true;
            }
         }

         if (!allDone) {
            // Find the first gfxpolygon that is not yet added.
            uint32 gfx = 0;
            uint32 poly = 0;
            uint32 polyNbr = 0;
            uint32 addGfx = 0;
            uint32 addGfxPoly = 0;
            uint32 addPolyNbr = 0;
            bool nbrSet = false;
            while ( !nbrSet && (gfx < nbrGfxs)) {
               poly = 0;
               while ( !nbrSet && (poly < gfxs[gfx]->getNbrPolygons())) {
                  if (added[polyNbr] == 0) {
                     mc2dbg4 << " Gfx " << gfx << ":" << poly 
                             << " not yet added" << endl;
                     addGfx = gfx;
                     addGfxPoly = poly;
                     addPolyNbr = polyNbr;
                     nbrSet = true;
                  }
                  poly++;
                  polyNbr++;
               }
               gfx++;
            }
            // Add the gfxpolygon as a new Polygon to the returnGfx 
            // since it is isolated from the already added ones
            returnGfx->setClosed(returnGfx->getNbrPolygons()-1, true);
            mc2dbg4 << "Adding new poly, gfx " << addGfx << ":" << addGfxPoly 
                    << ", polyNbr=" << addPolyNbr << " to returnGfx" << endl;
            if (returnGfx->addPolygon(gfxs[addGfx], false, addGfxPoly)) {
               added[addPolyNbr] = 1;
               lastLat = returnGfx->getLastLat(returnGfx->getNbrPolygons()-1);
               lastLon = returnGfx->getLastLon(returnGfx->getNbrPolygons()-1);
            }
         }
         
      }

      returnGfx->setClosed(returnGfx->getNbrPolygons()-1, true);
      returnGfx->updateLength();

      returnGfx->removeIdenticalCoordinates();
      mc2dbg2 << "Created returnGfx with " << returnGfx->getNbrPolygons()
              << " polygons and " << returnGfx->getTotalNbrCoordinates() 
              << " coords" << endl;
      
      delete gfxs;
      return returnGfx;
      
   } else {

      return (GMSGfxData*) complexGfx;
   }
}

GMSGfxData*
GMSGfxData::mergeTwoPolygons( GMSGfxData* otherGfx,
                              GMSGfxData* polygonGfxs[],
                              uint16 nbrPolygons,
                              uint16 p, uint16 mergep,
                              bool mergePolygonsWithMultipleTouches,
                              bool& twoPolygonsHaveMultipleTouches )
{
   GMSGfxData* myGfx = this;
   
   // The polygons should be closed
   if ( !myGfx->getClosed(0) || !otherGfx->getClosed(0) ) {
      return NULL;
   }

   // The polygons should have at least 3 coords
   uint32 myNbrCoords = myGfx->getNbrCoordinates(0);
   uint32 otherNbrCoords = otherGfx->getNbrCoordinates(0);
   if ( (myNbrCoords < 3) || (otherNbrCoords < 3) ) {
      return NULL;
   }

   // The polygons need to overlap if to be merged
   MC2BoundingBox myBBox;
   myGfx->getMC2BoundingBox( myBBox );
   MC2BoundingBox otherBBox;
   otherGfx->getMC2BoundingBox( otherBBox );
   if ( ! myBBox.overlaps( otherBBox) ) {
      DEBUG4(cout << "mergeTwoPolygons bbox does not overlap" << endl;);
      return NULL;
   }

   // Orientation must match for the polygons to be merged
   if ( myGfx->clockWise(0) != otherGfx->clockWise(0) ) {
      DEBUG4(cout << "mergeTwoPolygons orientation differ" << endl;);
      return NULL;
   }

   // If the polygons are equal, just return one of them
   if ( myGfx == otherGfx ) {
      DEBUG4(cout << "mergeTwoPolygons gfx polygons equal" << endl;);
      return myGfx;
   }
   
   // Collect coordinates from the 2 polygons
   typedef set< pair<uint32,uint32> > idxSet_t;
   typedef map< MC2Coordinate, idxSet_t > coordMap_t;
   coordMap_t gfxCoords;
   fillCoordMap( myGfx, 0, gfxCoords, 0);
   fillCoordMap( otherGfx, 0, gfxCoords, 1);
   
   /*
   // Check that at least 2 coords are shared between the 2 polygons
   uint32 nbrSharedCoords = 0;
   for ( coordMap_t::const_iterator it = gfxCoords.begin();
         it != gfxCoords.end(); it++ ) {
      idxSet_t idxSet = it->second;
      if ( idxSet.size() >= 2 ) {
         // check that it is shared among the two polygons, not just
         // shared with one of the polys.
         nbrSharedCoords++;
      }
   }
   if ( nbrSharedCoords < 2 ) {
      return NULL;
   }
   cout << "mergeTwoPolygons nbrSharedCoords=" << nbrSharedCoords << endl;
   */
   
   // Check coords in my polygon, find a good start coord (one that is not 
   // shared with the other polygon), and make sure that the polygons share 
   // at least 2 coords
   // It it not possible to start at a coord that is just my own if my polygon
   // is the filling of a "hole" in other polygon. Necessary?

   GMSGfxData* mergeGfx = GMSGfxData::createNewGfxData(NULL, true);
   
   // Start at coord 0 in my poly
   bool polygonsMerged = false;
   uint32 myIdx = 0;
   uint32 startIdx = myIdx;
   bool done = false;
   mc2dbg4 << " adding to mergeGfx from myGfx c=" << startIdx 
        << " (" << myGfx->getLat(0, myIdx) << "," 
        << myGfx->getLon(0, myIdx) << ")" << endl;
   while ( ! done ) {
      // Begin adding coordinates from myGfx to the mergeGfx
      int32 lat = myGfx->getLat(0, myIdx);
      int32 lon = myGfx->getLon(0, myIdx);
      mergeGfx->addCoordinate(lat, lon);
      mc2dbg8 << "  adding my " << myIdx << " " << lat << " " << lon 
              << " mergeGfx=" << mergeGfx->getNbrCoordinates(0) << endl;

      // Check if the other polygon has this coord
      uint32 otherIdx = MAX_UINT32;
      MC2Coordinate coord(lat, lon);
      coordMap_t::const_iterator it = gfxCoords.find(coord);
      if ( (it != gfxCoords.end()) && (it->second.size() > 1) ) {
         idxSet_t idxSet = it->second;
         idxSet_t::const_iterator setIt = idxSet.begin();
         while ( (otherIdx == MAX_UINT32) && (setIt != idxSet.end()) ) {
            if ( setIt->first == 1 ) {
               otherIdx = setIt->second;
            }
            setIt++;
         }
      }
      
      // The polygons share this coordinate
      if ( otherIdx != MAX_UINT32 ) {
         // Check how many coordinates that are shared between the polygons
         // Mark the coordIdx in each polygon where the shared part starts
         // and ends.
         uint32 mySharedStart = myIdx;
         uint32 otherSharedStart = otherIdx;
         mc2dbg4 << " shared coord mySharedStart=" << mySharedStart
                 << " otherSharedStart=" << otherSharedStart
                 << " " << it->first << endl;

         uint32 mySharedEnd = myIdx;
         uint32 otherSharedEnd = otherIdx;
         bool shared = true;
         while ( shared ) {
            myIdx = (myIdx+1) % myNbrCoords;
            otherIdx = (otherIdx-1+otherNbrCoords) % otherNbrCoords;
            if ( (myGfx->getLat(0,myIdx) == otherGfx->getLat(0,otherIdx)) &&
                 (myGfx->getLon(0,myIdx) == otherGfx->getLon(0,otherIdx)) ) {
               mySharedEnd = myIdx;
               otherSharedEnd = otherIdx;
            } else {
               shared = false;
            }
         }
         mc2dbg4 << " shared coord mySharedEnd=" << mySharedEnd
                 << " otherSharedEnd=" << otherSharedEnd 
                 << " (" << myGfx->getLat(0,mySharedEnd) << ","
                 << myGfx->getLon(0,mySharedEnd) << ")" << endl;

         if ( mySharedStart == mySharedEnd &&
              !surroundedByOtherPolys( otherGfx,
                                       polygonGfxs,
                                       nbrPolygons,
                                       p, mergep, 
                                       mySharedEnd,  
                                       myNbrCoords,
                                       otherSharedEnd,
                                       otherNbrCoords ) ) {
            // Only one coord shared, and merge point not surrounded by other 
            // polys: do not merge the polygons here, continue checking next coord 
            // in myGfx
            // Note; normally we don't want to merge polys that only share one
            // coord as this produces a self-touching polygon. However, if the
            // shared coord is surrounded by other polys, the self-touching
            // poly will disappear in forthcoming merges. Sometimes this is
            // what we want since the self-touching poly may prevent multiple
            // touches in intermediate steps (see test mergePolygonsTest???). 

            myIdx = mySharedStart;
         }
         else {
            // Now we are really merging
            polygonsMerged = true;
            bool foundMultipleTouches = false;      
      
            // Add otherGfx otherSharedStart+1 -> otherSharedEnd to mergeGfx
            mc2dbg8 << " adding from other " << otherSharedStart+1 << " -> "
                    << otherSharedEnd << " (" << otherNbrCoords << ")" << endl;
            uint32 nbrFromOther;
            
            if( otherSharedEnd == otherSharedStart ) {
               nbrFromOther = otherNbrCoords;
            } else {
               nbrFromOther = (otherSharedEnd-otherSharedStart + 
                  otherNbrCoords) % otherNbrCoords;
            }

            uint32 c = otherSharedStart;
            for ( uint32 i = 0; i < nbrFromOther; i++ ) {
               c = (c+1) % otherNbrCoords;
               mergeGfx->addCoordinate(
                  otherGfx->getLat(0,c), otherGfx->getLon(0,c));

               // Check if the two polygons have multiple touches, that is,
               // check if the other polygon has this coord.
               // Note that otherSharedEnd will be added to mergeGfx, and 
               // thus should not be tested for multiple touching.
               if( !foundMultipleTouches && 
                   c != otherSharedEnd ) {
                  MC2Coordinate coord( otherGfx->getLat(0,c), 
                                       otherGfx->getLon(0,c) );
                  coordMap_t::const_iterator it = gfxCoords.find(coord);
                  if ( (it != gfxCoords.end()) && (it->second.size() > 1) ) {
                     idxSet_t idxSet = it->second;
                     idxSet_t::const_iterator setIt = idxSet.begin();
                     while ( (!foundMultipleTouches) && 
                             (setIt != idxSet.end()) ) {
                        if ( setIt->first == 0 ) {
                           foundMultipleTouches = true;
                           twoPolygonsHaveMultipleTouches = true;
                        }
                        setIt++;
                     }
                  }
               }

   
               if( foundMultipleTouches && 
                   !mergePolygonsWithMultipleTouches ) {
                  delete mergeGfx;
                  return NULL;
               }

               mc2dbg8 << "  adding other " << c << " "
                       << otherGfx->getLat(0,c) << " " << otherGfx->getLon(0,c) 
                       << " mergeGfx=" << mergeGfx->getNbrCoordinates(0) << endl;
            }
 

            // Add rest of myGfx mySharedEnd+1 -> startIdx-1 to mergeGfx
            if ( (mergeGfx->getLastLat(0) == myGfx->getLat(0,startIdx)) &&
                 (mergeGfx->getLastLon(0) == myGfx->getLon(0,startIdx)) ) {
               // If we already are at startIdx, please don't add more
               // from myGfx! (we started from a shared coordinate corner)
               mc2dbg8 << " don't add from my " << mySharedEnd+1 << " -> "
                       << startIdx << " (" << myNbrCoords << ")" << endl;
            } else {
               mc2dbg4 << " adding from my " << mySharedEnd+1 << " -> "
                       << startIdx << " (" << myNbrCoords << ")" << endl;
               uint32 nbrFromMy =
                  (startIdx-1 - mySharedEnd + myNbrCoords) % myNbrCoords;
               c = mySharedEnd;
               for ( uint32 i = 0; i < nbrFromMy; i++ ) {
                  c = (c+1) % myNbrCoords;
                  mergeGfx->addCoordinate(
                     myGfx->getLat(0,c), myGfx->getLon(0,c));
                  mc2dbg8 << "  adding my " << c << " " << myGfx->getLat(0,c)
                          << " " << myGfx->getLon(0,c) << " mergeGfx=" 
                          << mergeGfx->getNbrCoordinates(0) << endl;
               }
            }

            done = true;
         }
      }
      
      // go to next coord of my polygon, check if we are done.
      myIdx = (myIdx+1) % myNbrCoords;
      if ( myIdx == startIdx ) {
         mc2dbg8 << "myIdx=startIdx says done" << endl;
         done = true;
      }
   } // end while (!done)

   if ( !polygonsMerged ) {
      mc2dbg8 << "Polygons were not merged" << endl;
      delete mergeGfx;
      return NULL;
   }

   // Set this gfx to be closed, otherwise removeDeadEnds will not work
   mergeGfx->setClosed(0, true);

   uint32 nbrc = mergeGfx->getNbrCoordinates(0);
   // Remove any dead-ends and duplicates from the mergeGfx
   mergeGfx->removeDeadEndCoordinates();
   mergeGfx->removeIdenticalCoordinates();
   mergeGfx->updateLength();
   mc2dbg4 << "Created mergeGfx " << mergeGfx->getNbrCoordinates(0) 
           << " (" << nbrc << ")" << endl;

   return mergeGfx;
}

GMSGfxData*
GMSGfxData::mergePolygons(GfxData* complexGfx)
{
   uint16 nbrPolygons = complexGfx->getNbrPolygons();
   if ( nbrPolygons < 2 ) {
      return NULL;
   }

   if ( ! complexGfx->getClosed(0) ) {
      return NULL;
   }
   
   // Create a separate gfxData of each of the polys
   GMSGfxData* polygonGfxs[nbrPolygons];
   for (uint16 p = 0; p < nbrPolygons; p++ ) {
      GMSGfxData* polyGfx = GMSGfxData::createNewGfxData(NULL, true);
      if (polyGfx->add(complexGfx, false, p)) {
         polyGfx->setClosed(0, complexGfx->getClosed(p));
         polyGfx->updateLength();
         polyGfx->removeDeadEndCoordinates();
         polygonGfxs[p] = polyGfx;
         mc2dbg8 << "pGfx " << p << ": " << polyGfx->getNbrCoordinates(0)
                 << " clockWise=" << polyGfx->clockWise(0)
                 << " (" << complexGfx->clockWise(p) << ")" << endl;
         if ( ! complexGfx->getClosed(p) ) {
            cout << " complexGfx poly " << p << " is not closed..." << endl;
         }
      } else {
         cout << " failed to add complexGfx poly " << p 
              << " to the polygonGfxs vector" << endl;
      }
      
   }
   
   // Mark all poly gfxs as not merged so far
   bool polyMerged[nbrPolygons];
   for ( uint16 p = 0; p < nbrPolygons; p++ ) {
      polyMerged[p] = false;
   }

   // Merge the polygonGfxs
   mc2dbg4 << "GMSGfxData::mergePolygons: Trying to concatinate "
           << nbrPolygons << " gfxData polys." << endl;
   bool complexIsChanged = false;
   bool oneMerged = false;
   bool allDone = false;
   bool mergePolygonsWithMultipleTouches = false;
   bool twoPolygonsHaveMultipleTouches = false;
   uint16 p = 0;
   while ( !allDone && 
           (p < (nbrPolygons -1)) ) { // something to merge into this poly
      mc2dbg8 << "p=" << p << endl;
      if ( ! polyMerged[p] ) {
         
         // Start merging into polyGfx p
         GMSGfxData* pGfx = polygonGfxs[p];
         uint16 mergep = p + 1;
         while ( mergep < nbrPolygons ) {
            mc2dbg8 << "p=" << p << " mergep=" << mergep << endl;
//            mc2dbg1 << "p=" << p << " " << pGfx->getLat(0,0) << ";" 
//                    << pGfx->getLon(0,0)
//                    << " mergep=" << mergep << " " 
//                    << polygonGfxs[mergep]->getLat(0,0) << ";"
//                    << polygonGfxs[mergep]->getLon(0,0) << endl;
            if ( ! polyMerged[mergep] ) {
                
               GMSGfxData* newGfx =
                  pGfx->mergeTwoPolygons( polygonGfxs[mergep],
                                          polygonGfxs,
                                          nbrPolygons,
                                          p, mergep,
                                          mergePolygonsWithMultipleTouches,
                                          twoPolygonsHaveMultipleTouches );

               if ( newGfx != NULL ) {

                  complexIsChanged = true;
                  oneMerged = true;
                  polyMerged[mergep] = true;
                  pGfx = newGfx;
                  polygonGfxs[p] = newGfx;
                  mc2dbg4 << "poly " << mergep << " added to " << p << " (" 
                          << pGfx->getNbrCoordinates(0) << " coords)" << endl;
               } else {
                  mc2dbg4 << "poly " << mergep << " not added to " << p << endl;
               }
            }
            mergep++;
         }
      }

      if ( oneMerged ) {
         oneMerged = false;
         mergePolygonsWithMultipleTouches = false;
         twoPolygonsHaveMultipleTouches = false; 
         
         // Check if we are done
         // (all polys after the one we started from are merged)
         uint32 k=p+1;
         while ( (k < nbrPolygons) && polyMerged[k] ) {
            k++;
         }
         if (k == nbrPolygons) {
            allDone = true;
         }
         mc2dbg4 << "completed one loop for p=" << p << " k=" << k 
                 << " allDone=" << allDone << endl;
      } else if ( !oneMerged && 
                  !mergePolygonsWithMultipleTouches && 
                  twoPolygonsHaveMultipleTouches ) {
         // Try one more round (that is, do not increase p++), but this
         // time merge the multiple touches.
         mergePolygonsWithMultipleTouches = true;
         twoPolygonsHaveMultipleTouches = false; 
      } else if ( !oneMerged && 
                  mergePolygonsWithMultipleTouches ) {
         // Something is wrong, merging multiple touches must
         // have failed (since mergePolygonsWithMultipleTouches is only
         // true if we multipletouches in the last round which should have
         // have been merged in this one).
         mc2dbg << warn 
                << "Did not merge found polygons with multiple touches"  
                << endl;
         mergePolygonsWithMultipleTouches = false;
         twoPolygonsHaveMultipleTouches = false; 
         p++;
      } else {
         mergePolygonsWithMultipleTouches = false;
         twoPolygonsHaveMultipleTouches = false;          
         p++;
         //MC2_ASSERT( false );
      }

   }

   // Check if we actually merged any of the item polygons
   if ( ! complexIsChanged ) {
      return NULL;
   }

   // Create the gfxData to return
   GMSGfxData* returnGfx = createNewGfxData(NULL, true);
   // Add all not-merged polygonGfxs to the returnGfx
   bool firstAdded = false;
   for (uint16 p = 0; p < nbrPolygons; p++ ) {
      if ( !polyMerged[p] ) {
         if ( firstAdded ) {
            returnGfx->addPolygon( polygonGfxs[p], false, 0 );
         } else {
            returnGfx->add( polygonGfxs[p], false, 0 );
            firstAdded = true;
         }
      }
   }
   returnGfx->setClosed(0, true);
   returnGfx->updateLength();
   mc2dbg4 << "GMSGfxData::mergePolygons: returnGfx nbrp=" 
           << returnGfx->getNbrPolygons()
           << " nbrc=" << returnGfx->getTotalNbrCoordinates() << endl;

   // delete gfxs created here ?
   //for (uint16 p = 0; p < nbrPolygons; p++ ) {
   //   delete polygonGfxs[p];
   //}
   
   return returnGfx;
}


bool 
GMSGfxData::printMif(ofstream& outfile) const 
{
   if (closed()){
      outfile << "Region " << getNbrPolygons() << endl; 
      for (uint32 p=0; p<getNbrPolygons(); p++) {
         outfile << getNbrCoordinates(p) << endl;
         for (uint32 i=0; i<getNbrCoordinates(p); i++) {
            outfile << getLat(p, i) << " "
                    << getLon(p, i) << endl;
         }
      }
   } else {
      uint32 p = 0; //nbrPolygons - 1 
                    //(only one polygon when pline or point)
      uint32 c = getNbrCoordinates(p);
      if (c == 1) {
         outfile << "Point ";
         outfile << getLat(p, c-1) << " " 
                 << getLon(p, c-1) << endl;
      } else {
         outfile << "Pline ";
         outfile << getNbrCoordinates(p) << endl;
         for (uint32 i=0; i<getNbrCoordinates(p); i++) {
            outfile << getLat(p, i) << " "
                    << getLon(p, i) << endl;
         }
      }
   }
   return (true);
}

bool
GMSGfxData::readMifHeader(ifstream& infile,
                       CoordinateTransformer::format_t& coordsys,
                       bool& normalOrderOfCoordinates,
                       uint32& utmzone,
                       int32& falseNorthing, int32& falseEasting)
{
   // The default coordinate system is mc2
   coordsys = CoordinateTransformer::mc2;
   // The default order of coordinates is "normal" i.e.
   // lat,lon or x,y (northing, easting).
   normalOrderOfCoordinates = true;
   //The default utmzone
   utmzone = 0;
   //Default standard addition is no addition.
   falseNorthing = 0;
   falseEasting = 0;

   char* buff = new char[128];
   buff[0] = '\0';

   bool done = false;

   while ( !infile.eof() && !done ) {
      if (strcasecmp("coordsys", buff) == 0) {
         // The optional COORDSYS is found. Read the coordinate system
         infile >> buff;
         mc2dbg1 << "Found optional coord sys \"" << buff << "\"" << endl;
         if (strstr(buff, "wgs84_lonlat_deg") != NULL) {    
            coordsys = CoordinateTransformer::wgs84deg;
            normalOrderOfCoordinates = false;
         } else if (strstr(buff, "wgs84_latlon_deg") != NULL) {
            coordsys = CoordinateTransformer::wgs84deg;
         } else if (strstr(buff, "rt90_lonlat") != NULL){
            coordsys = CoordinateTransformer::rt90_2_5gonV_rh70_XYH;
            normalOrderOfCoordinates = false;
         } else if (strstr(buff, "rt90") != NULL) {
            coordsys = CoordinateTransformer::rt90_2_5gonV_rh70_XYH;
         } else if (strstr(buff, "utm") != NULL) {
            coordsys = CoordinateTransformer::utm;
            if (strstr(buff, "lonlat") != NULL) {
               normalOrderOfCoordinates = false;
            }
            infile >> buff;
            utmzone = strtoul(buff, NULL, 10);
         } else if (strstr(buff, "mc2_lonlat") != NULL) {
            coordsys = CoordinateTransformer::mc2;
            normalOrderOfCoordinates = false;
         } else if (strstr(buff, "mc2") != NULL) {
            coordsys = CoordinateTransformer::mc2;
         }
      } else if (strcasecmp(buff, "falseNorthing") == 0){
         infile >> buff;
         falseNorthing = strtol(buff, NULL, 10);
      } else if (strcasecmp(buff, "falseEasting") == 0){
         infile >> buff;
         falseEasting = strtol(buff, NULL, 10);
      } else if (strcasecmp(buff, "Data") == 0) {
         // Done with the header
         mc2dbg4 << "Found \"Data\", done with the header." << endl;
         done = true;
      } else {
         // Continue reading the header
         infile >> buff;
      }
   } // while
   
   delete [] buff;

   if (!done) {
      mc2log << fatal << "No \"Data\" tag in the mif header." << endl;
      return false;
   }

   mc2dbg << "[ReadMifHeader] "
          << "coordsys=" << int(coordsys) << " normalorder="
          << normalOrderOfCoordinates << " utmzone=" << utmzone
          << " falseNorthing=" << falseNorthing
          << " falseEasting=" << falseEasting << endl;
   
   return true;
}

bool
GMSGfxData::findNextMifFeature(ifstream& infile,
                               bool& region,
                               bool& line,
                               bool& point,
                               uint32& nbrPolygons)
{
   // Read from mif file until next valid mif feature is found
   // valid features: Region, Pline, Line, Point
   bool retVal = true;
   
   // Re-set output values.
   // The Pline is considered to be the normal mif feature.
   region = false;
   line = false;
   point = false;
   nbrPolygons = 1;
   
   uint32 maxNbrChars = 128;
   char* buff = new char[maxNbrChars + 1];
   buff[0] = '\0';
   infile >> buff;

   // Find next "Region" or "Pline" or Point

   if (strcasecmp(buff, "VERSION") == 0) {
      // another mif header in the mif file, read past it
      bool done = false;
      while (!infile.eof() && !done) {
         infile >> buff;
         if (strcasecmp(buff, "Data") == 0) {
            done = true;
            mc2dbg2 << "Read past an inside mif header" << endl;
         }
      }
      delete [] buff;
      return (findNextMifFeature(infile, region, line, point, nbrPolygons));
   }

   // Mif cosmetic features, e.g. 'Pen (2,2,16776960)' or 'PEN(2,2,0)'
   if (strncasecmp(buff, "Pen", 3) == 0) {
      // read the 'Pen (X,X,X)' line
      infile.getline(buff, maxNbrChars);  // '(2,2,16776960)' or eol
      delete [] buff;
      return (findNextMifFeature(infile, region, line, point, nbrPolygons));
   }
   if (strncasecmp(buff, "Brush", 5) == 0) {
      // read the 'Brush (X,X,X)' line
      infile.getline(buff, maxNbrChars);
      delete [] buff;
      return (findNextMifFeature(infile, region, line, point, nbrPolygons));
   }
   if (strncasecmp(buff, "Center", 6) == 0) {
      // read the 'Center coord1 coord2' line
      infile.getline(buff, maxNbrChars);
      delete [] buff;
      return (findNextMifFeature(infile, region, line, point, nbrPolygons));
   }
   if (strncasecmp(buff, "Symbol", 6) == 0) {
      // read the 'Symbol (X,X,X)' line
      infile.getline(buff, maxNbrChars);
      delete [] buff;
      return (findNextMifFeature(infile, region, line, point, nbrPolygons));
   }
   if (strncasecmp(buff, "Smooth", 6) == 0) {
      // the 'Smooth' was read with this buff
      delete [] buff;
      return (findNextMifFeature(infile, region, line, point, nbrPolygons));
   }
   if (strncasecmp(buff, "Font", 4) == 0) {
      // read the 'Font ("xxx",x,x,x)' line
      infile.getline(buff, maxNbrChars);
      delete [] buff;
      return (findNextMifFeature(infile, region, line, point, nbrPolygons));
   }
   
   if (strcasecmp(buff, "Region") == 0) {
      region = true;
      infile >> nbrPolygons;
   } else if (strcasecmp(buff, "Pline") == 0) {
      nbrPolygons = 1;
   } else if (strcasecmp(buff, "Line") == 0) {
      line = true;
      nbrPolygons = 1;
   } else if (strcasecmp(buff, "Point") == 0) {
      nbrPolygons = 1;
      point = true;

   // Handle read past key lines for known, but not handled, mif features
   // The items with these features will not be added to mcm maps in GMS -r
   } else if ( (strcasecmp(buff, "none") == 0) ||
               (strcasecmp(buff, "Rect") == 0) ||
               (strcasecmp(buff, "Roundrect") == 0) ||
               (strcasecmp(buff, "Ellipse") == 0) ) {
      mc2dbg << "Reading mif feature " << buff << endl;
      infile.getline(buff, maxNbrChars);
      retVal = false;
      // if Roundrect, the degree of rounding can be either 
      // on this first line or on a second line
   } else if ( strcasecmp(buff, "Text") == 0 ) {
      mc2dbg << "Reading mif feature " << buff << endl;
      infile.getline(buff, maxNbrChars); // eol
      infile.getline(buff, maxNbrChars); // The text string
      infile.getline(buff, maxNbrChars); // The text coordinates
      retVal = false;
      
   } else {
      // no valid mif feature (or end of file)
      if ( !infile.eof() ) {
         mc2log << fatal << "No valid mif feature to read '" << buff 
                << "'" << endl;
         retVal = false;
         MC2_ASSERT(false);
      } else {
         mc2dbg << "mif file eof" << endl;
         retVal = false;
      }
   }
   
   delete [] buff;

   return retVal;
}

bool
GMSGfxData::createGfxFromMif(ifstream& infile,
                             CoordinateTransformer::format_t coordsys,
                             bool normalOrderOfCoordinates,
                             uint32 utmzone,
                             int32 falseNorthing, int32 falseEasting)
{
   // is this gfx a region or a pline
   bool region = false;
   bool line = false;
   bool point = false;
   uint32 nbrPolygons = 1;
   
   bool validMifFeature = 
         findNextMifFeature(infile, region, line, point, nbrPolygons);
   if ( !validMifFeature ) {
      return false;
   }

   if ( region && (nbrPolygons == 0) ) {
      mc2log << error << here << "Read gfx \"Region 0\" from mif file"
             << " exit!" << endl;
      MC2_ASSERT(false);
   }

   DEBUG4(
      if (region)
      mc2dbg << "Found region, nbrPolygons = " << nbrPolygons << endl;
      else if (point)
         mc2dbg << "Found point" << endl;
      else if (line)
         mc2dbg << "Found line" << endl;
      else
         mc2dbg << "Found pline" << endl;
   );

   // Read all the polygons
   uint32 p = 0;
   while ((p < nbrPolygons) && infile && !infile.eof()) {
      
      // Read the number of coordinates
      uint32 nbrCoordinates = MAX_UINT32;
      if (point) {
         nbrCoordinates = 1;
      } else if (line) {
         nbrCoordinates = 2;
      } else {
         infile  >> nbrCoordinates;
      }
      if ((nbrCoordinates > (MAX_UINT32/2)) //&& infile
         && !infile.eof()) {
         mc2log << error << here << " Could not read nbr coordinates "
                << "for polygon " << p << endl
                << "nbrCoordinates = " << nbrCoordinates << endl;
         return false;
      }
      mc2dbg2 << p << "   nbrCoordinates = " << nbrCoordinates << endl;

      uint32 i = 0;
      while ((i < nbrCoordinates) && infile && !infile.eof())  {
         float64 flat, flon;
         if (normalOrderOfCoordinates) {
            infile >> flat;
            infile >> flon;
         } else {
            infile >> flon;
            infile >> flat;
         }
         //remove any standard addition
         flat = flat - falseNorthing;
         flon = flon - falseEasting;
         
         // Convert to MC2 coordinates
         int32 lat, lon;
         CoordinateTransformer::transformToMC2(coordsys, flat, flon, 0,
                                               lat, lon, utmzone);
         mc2dbg8<< "      " << lat << " " << lon << endl;
         
         // Add the coordinate, create a new polygon if
         // first coordinate
         if ( !addCoordinate(lat, lon, (i == 0)) ) {
            mc2log << fatal << "Coordinate not added to the gfxData, " 
                            << "polygonNbr=" << p << " coordinateNbr=" 
                            << i << "." << endl;
            return false;
         }
         ++i;
      }
      // done
      if ( i != nbrCoordinates ) {
         mc2log << fatal 
                << "createGfxFromMif: nbrCoords specified in mif file="
                << nbrCoordinates
                << " does not match the true nbr coords that was read="
                << i << " for poly " << p+1 << " (of the specified "
                << nbrPolygons << ")" << endl;
         if ( getNbrCoordinates(0) > 0 ) {
            mc2dbg << "The first coord of this corrupt mif feature is "
                   << getLat(0,0) << " " << getLon(0,0) << endl;
         }
         MC2_ASSERT(false);
      }
      ++p;
   }

   // if region, set this gfx closed.
   if (region) {
      for (uint32 p=0; p < getNbrPolygons(); p++) {
         setClosed(p,true);
      }
      DEBUG2(cout << "   Setting gfx closed." << endl);
      sortPolygons();
   }

   //Removing duplicated coordinates and updating the length
   uint32 nbrCoordsInMif = getTotalNbrCoordinates();
   removeIdenticalCoordinates();
   updateLength();

   // Please do not remove the node coordinates of Plines!
   if ( !region && !point && (nbrCoordsInMif > 1) && 
        (getTotalNbrCoordinates() == 1) ) {
      mc2log << warn << " Removed identical node coord of Pline/line feature"
             << " - re-adding it" << endl;
      addCoordinate( getLat(0,0), getLon(0,0) );
      removeIdenticalCoordinates(); // why?
      updateLength();
   }

   // Print status
   DEBUG2(
      mc2dbg << "Filled gfx with " << getNbrPolygons()
             << " polygons." << endl;
      mc2dbg << "NbrCoordinates: ";
      for (uint32 p= 0; p < getNbrPolygons(); p++)
         mc2dbg << getNbrCoordinates(p) << " ";
      mc2dbg << endl;
   );
   
   return true;
}

bool
GMSGfxData::createFromMif(ifstream& infile)
{
   CoordinateTransformer::format_t coordsys;
   bool normalCoordinateOrder;
   uint32 utmzone;
   int32 falseNorthing, falseEasting;
   
   if ( !readMifHeader(infile, coordsys, normalCoordinateOrder,
                       utmzone, falseNorthing, falseEasting) ) {
      mc2log << fatal << "GfxData::createFromMif could not read mif header."
                      << endl;
      return false;
   }
   if ( !createGfxFromMif(infile, coordsys, normalCoordinateOrder,
                          utmzone, falseNorthing, falseEasting) ) {
      mc2log << fatal << "GfxData::createFromMif could not create gfx data."
                      << endl;
      return false;
   }
   
   mc2dbg1 << "Filled the gfx with " << getNbrPolygons() 
           << " polygons, " << getNbrCoordinates(0) << " coordinates in first."
           << endl;

   return true;
}



bool 
GMSGfxData::surroundedByOtherPolys( GMSGfxData* otherGfx,
                                    GMSGfxData* polygonGfxs[],
                                    uint16 nbrPolygons,
                                    uint32 p, uint32 mergep,
                                    uint32 mySharedEnd,
                                    uint32 myNbrCoords,
                                    uint32 otherSharedEnd,
                                    uint32 otherNbrCoords )
{

   // Find coord indexes of coords adjacent to the shared coord.
   uint32 mySharedEndPrev = 
      ( mySharedEnd - 1 + myNbrCoords ) % myNbrCoords;
   
   uint32 mySharedEndNext = 
      ( mySharedEnd + 1) % myNbrCoords;

   uint32 otherSharedEndPrev = 
      ( otherSharedEnd - 1 + otherNbrCoords ) % otherNbrCoords;

   uint32 otherSharedEndNext = 
      ( otherSharedEnd + 1) % otherNbrCoords;

   

   // Go through all polys (except the two we are merging) and see
   // if each adjacent coord pair is in a poly.
   bool surrounded = false;
   bool found1 = false;
   bool found2 = false;
   bool found3 = false;
   bool found4 = false;

   for (uint16 i = 0; i < nbrPolygons; i++) {
      if( i == p || i == mergep) {
         continue;
      }
      
      if ( coordPairInPoly( polygonGfxs[i], 
                            getLat(0, mySharedEnd), 
                            getLon(0, mySharedEnd),
                            getLat(0, mySharedEndPrev), 
                            getLon(0, mySharedEndPrev) ) ) {
         found1 = true;
      }

      if ( coordPairInPoly( polygonGfxs[i], 
                            getLat(0, mySharedEnd), 
                            getLon(0, mySharedEnd),
                            getLat(0, mySharedEndNext), 
                            getLon(0, mySharedEndNext) ) ) {
         found2 = true;
      }

      if ( coordPairInPoly( polygonGfxs[i], 
                            otherGfx->getLat(0, otherSharedEnd), 
                            otherGfx->getLon(0, otherSharedEnd),
                            otherGfx->getLat(0, otherSharedEndPrev), 
                            otherGfx->getLon(0, otherSharedEndPrev) ) ) {
         found3 = true;
      }

      if ( coordPairInPoly( polygonGfxs[i], 
                            otherGfx->getLat(0, otherSharedEnd), 
                            otherGfx->getLon(0, otherSharedEnd),
                            otherGfx->getLat(0, otherSharedEndNext), 
                            otherGfx->getLon(0, otherSharedEndNext) ) ) {
         found4 = true;
      }
   }
   
   if( found1 && found2 && found3 && found4) {
      surrounded = true;
   } else {
      surrounded = false;
   }

   return surrounded;
}

bool 
GMSGfxData::coordPairInPoly( GMSGfxData* gfxData,
                             coordinate_type coord1Lat,
                             coordinate_type coord1Lon,
                             coordinate_type coord2Lat,
                             coordinate_type coord2Lon ) {
   
   uint32 nbrCoords = gfxData->getNbrCoordinates(0);
  
   bool foundPair = false; 
   for( uint32 i = 0; i < nbrCoords; i++) {
      if( gfxData->getLat( 0, i ) == coord1Lat && 
          gfxData->getLon( 0, i ) == coord1Lon) {
         uint32 next = ( i + 1 ) % nbrCoords;
         uint32 prev = ( i - 1 + nbrCoords ) % nbrCoords;
         if( (gfxData->getLat( 0, next ) == coord2Lat &&
              gfxData->getLon( 0, next ) == coord2Lon ) ||
             (gfxData->getLat( 0, prev ) == coord2Lat &&
              gfxData->getLon( 0, prev ) == coord2Lon) ) {
            foundPair = true;
         }
      }
   }

   return foundPair;   
}





