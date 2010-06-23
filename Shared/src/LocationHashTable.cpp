/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "LocationHashTable.h"
#include "Math.h"

#include <math.h>
#include <set>
/* ************************************************/
/* ************  LocationHashCell  ****************/
/* ************************************************/

LocationHashCell::LocationHashCell()
{
}

LocationHashCell::~LocationHashCell()
{
}

void 
LocationHashCell::setMC2BoundingBox(int32 hmin, int32 hmax,
                                    int32 vmin, int32 vmax)
{
   m_rightHorizontal = hmax;
   m_leftHorizontal  = hmin;
   m_topVertical     = vmax;
   m_bottomVertical  = vmin;
}

bool 
LocationHashCell::containsValidElement() const 
{
   return (m_nbrElements > 0);
}



uint32 
LocationHashCell::getClosest(int32 hpos, int32 vpos, 
                             bool dummy, uint64 &dist, bool areaItem)
{
   uint64 maxDist = MAX_UINT64;
	uint64 newdist;
   uint64 mindist;
   uint32 foundIndex = MAX_UINT32;
   uint32 t; 

   if (dist == MAX_UINT64) {
      // Calculate maxDist (the searchradius, the minimum distance)
      // Calc the minimum of all max-dist-to-bbox.
      for( t = 0; t < m_nbrElements; t++) {
         newdist = maxSquaredistToBoundingbox(t, hpos, vpos);
         if( newdist < maxDist ) {
            maxDist = newdist;
            foundIndex = t;
         }
      }
   } else {
      maxDist = dist;
   }

   if (maxDist < MAX_UINT64) {
      //Calculate the distance for all elements within the searchradius
//      dist = MAX_UINT64;//minSquaredist(foundIndex, hpos, vpos); // calculate distance of a good candidate.
	   for( t = 0; t < m_nbrElements; t++) {
         mindist = minSquaredistToBoundingbox(t, hpos, vpos, dist);
         if (mindist < MAX_UINT64 && mindist <= maxDist) {
//		   if (mindist < dist) {
            newdist = minSquaredist(t, hpos, vpos);
			   if( dist > newdist ) {
               if ( !areaItem || 
                    (areaItem && isInsidePolygon( t, hpos, vpos ) ) ){
                  foundIndex = t;
      		      dist = newdist;
                  maxDist = dist;
               }
			   }
		   } 
	   }
      return foundIndex;
   }
   else
      // Unable to find any matches.
      return MAX_UINT32;
}

void
LocationHashCell::getWithinRadius( int32 hpos, int32 vpos, 
                                   uint64 sqRadius,
                                   set<uint32>& proximityResult)
{
	for( uint32 t = 0; t < m_nbrElements; t++) {
      //If the maximum distance is less than the radius just add the item.
      uint32 itemID = getItemID(t);
      if ( proximityResult.find( itemID )  != proximityResult.end() ) {
         continue; // Next, please
      }
      uint64 maxDist = maxSquaredistToBoundingbox(t, hpos, vpos);
      if ( maxDist <= sqRadius ) {
         proximityResult.insert( itemID );
      } else {
         //If the minimum distance is less than the radius, look closer
         uint64 minDist = minSquaredistToBoundingbox(t, hpos, vpos);
         if( minDist <= sqRadius ) {
            uint64 dist = minSquaredist(t, hpos, vpos);
            if( dist <= sqRadius ) {
               proximityResult.insert( itemID );
            }
         }
      }
   }
}

void
LocationHashCell::getWithinMC2BoundingBox(const MC2BoundingBox* bbox,
                                          Vector* proximityResult)
{
	for( uint32 t = 0; t < m_nbrElements; t++) {
      
      if (insideMC2BoundingBox(t, bbox)) {
         proximityResult->addLast(getItemID(t));
      }
   }
}

void
LocationHashCell::getWithinMC2BoundingBox(const MC2BoundingBox* bbox,
                                          set<uint32>& proximityResult)
{
	for( uint32 t = 0; t < m_nbrElements; t++) {
      
      if (insideMC2BoundingBox(t, bbox)) {
         proximityResult.insert(getItemID(t));
      }
   }
}

void
LocationHashCell::getAllItems(Vector* proximityResult)
{
   // Let's allocate enough memory already.
   proximityResult->setAllocSize(proximityResult->getSize() + m_nbrElements);
   
   for( uint32 t = 0; t < m_nbrElements; ++t) {
      uint32 itemID = getItemID(t);
      if (isItemAllowed(itemID))
         proximityResult->addLast(itemID);
   }
}

void
LocationHashCell::getAllItems(set<uint32>& proximityResult)
{
   for( uint32 t = 0; t < m_nbrElements; ++t) {
      uint32 itemID = getItemID(t);
      if (isItemAllowed(itemID))
         proximityResult.insert(itemID);
   }
}

uint32 
LocationHashCell::getMemoryUsage() const 
{
   uint32 size = sizeof(LocationHashCell);
   return size;
}




bool 
LocationHashCell::isInsidePolygon(uint32 index, const int32 hpos, 
                                  const int32 vpos )
{
   // Overrided in subclasses not abstract for now!!!!!!!!
   return true;
}


/* ************************************************/
/* ************  LocationHashTable  ***************/
/* ************************************************/

LocationHashTable::LocationHashTable( ):
   m_nbrVerticalCells( 0 ),
   m_nbrHorizontalCells( 0 ),
   m_hashCell( 0 ),
   m_verticalShift( 0 ),
   m_horizontalShift( 0 ),
   m_topVertical( 0 ),
   m_bottomVertical( 0 ),
   m_rightHorizontal( 0 ),
   m_leftHorizontal( 0 ) 
{
}

LocationHashTable::LocationHashTable( int32 hmin, int32 hmax, int32 vmin,
                                      int32 vmax, uint32 nbrVerticalCellsx,
                                      uint32 nbrHorizontalCellsx )
{
   m_rightHorizontal = hmax;
   m_leftHorizontal = hmin;
   m_topVertical = vmax;
   m_bottomVertical = vmin;

   //calculate vertical parameter for hashfunction
   m_nbrVerticalCells = nbrVerticalCellsx; 
   m_verticalShift = (int) (
      -log( ( (double) m_nbrVerticalCells ) / 
      ( m_topVertical - m_bottomVertical) ) / log(2.0) + 1 );

   //calculate the actual number of vertical cells
   m_nbrVerticalCells = ( m_topVertical - m_bottomVertical) 
                        >> m_verticalShift;
   m_nbrVerticalCells++;

   //calculate horizontal parameter for hashfunction
   m_nbrHorizontalCells = nbrHorizontalCellsx; 
   m_horizontalShift = (int) (
      -log( ( (double) m_nbrHorizontalCells) / 
      ( m_rightHorizontal - m_leftHorizontal ) ) / log(2.0) + 1 );

   //calculate the actual number of horizontal cells
   m_nbrHorizontalCells = 
      ( m_rightHorizontal - m_leftHorizontal ) >> m_horizontalShift;
   m_nbrHorizontalCells++;

}

LocationHashTable::~LocationHashTable()
{
}

void 
LocationHashTable::addToNeighbours(uint64 elmDist, int32 vpos, int32 hpos,
   int32 v, int32 h, int32 dv, int32 dh,
   uint32 vIndex[], uint32 hIndex[], uint64 sqDist[], int32 &nbr)
{
   if( (v + dv >= 0) && (v + dv < (int32)m_nbrVerticalCells) &&
       (h + dh >= 0) && (h + dh < (int32)m_nbrHorizontalCells) ) {

      if( m_hashCell[v + dv][h + dh]->containsValidElement()) {
         uint64 sqV, sqH;

         if( dv == 1 )
            sqV = SQUARE( int64(vpos - m_hashCell[v][h]->getMaxVertical()) );
         else if( dv == -1 )
            sqV = SQUARE( int64(vpos - m_hashCell[v][h]->getMinVertical()) );
         else
            sqV = 0;

         if( dh == 1 )
            sqH = SQUARE( int64(getHorizontalFactor()*
                          (m_hashCell[v][h]->getMaxHorizontal() - hpos)) );
         else if( dh == -1 )
            sqH = SQUARE( int64(getHorizontalFactor()*
                          (hpos - m_hashCell[v][h]->getMinHorizontal())) );
         else
            sqH = 0;

         uint64 borderDist = sqV + sqH;

         if( elmDist >= borderDist ) {
            sqDist[nbr] = borderDist;
            vIndex[nbr] = v + dv;
            hIndex[nbr] = h + dh;
            nbr++;
         }
      }
   }
}

LocationHashCell* 
LocationHashTable::getClosest(int32 hpos, int32 vpos, 
                              uint32 &offset, uint64 &dist, bool areaItem)
{
   uint32 vHash, hHash;     //hashindex to a cell
   uint64 elmDistance = MAX_UINT64; //squaredistance to an element
   uint64 sqDistance[8];    //distance to adjacent cells
   uint32 vNeighbour[8];    //vertical index for adjacent cells
   uint32 hNeighbour[8];    //horizontal index for adjacent cells
   int nbrNeighbours = 0;   //number of neighbours to cell[vHash][hHash]
   uint64 sqTemp;           //temporary storage
   uint32 tempOffset;       //temporary storage
   LocationHashCell* tempCell = NULL;   //temporary storage
   
   getHashIndex(hpos, vpos, hHash, vHash); 
               
   //First get the distance to the closest element in cell[vHash][hHash]
   offset = m_hashCell[vHash][hHash]->
            getClosest(hpos, vpos, false, elmDistance, areaItem );

   //Now store neighbours closer than elmDistance
   //Lower neighbour
   addToNeighbours(elmDistance, vpos, hpos, vHash, hHash, -1, 0, 
                   vNeighbour, hNeighbour, sqDistance, nbrNeighbours);

   //Right neighbour
   addToNeighbours(elmDistance, vpos, hpos, vHash, hHash, 0, 1, 
                   vNeighbour, hNeighbour, sqDistance, nbrNeighbours);

   //Upper neighbour
   addToNeighbours(elmDistance, vpos, hpos, vHash, hHash, 1, 0, 
                   vNeighbour, hNeighbour, sqDistance, nbrNeighbours);

   //Left neighbour
   addToNeighbours(elmDistance, vpos, hpos, vHash, hHash, 0, -1, 
                   vNeighbour, hNeighbour, sqDistance, nbrNeighbours);

   //Lower right neighbour
   addToNeighbours(elmDistance, vpos, hpos, vHash, hHash, -1, 1, 
                   vNeighbour, hNeighbour, sqDistance, nbrNeighbours);

   //Lower left neighbour
   addToNeighbours(elmDistance, vpos, hpos, vHash, hHash, -1, -1, 
                   vNeighbour, hNeighbour, sqDistance, nbrNeighbours);

   //Upper right neighbour
   addToNeighbours(elmDistance, vpos, hpos, vHash, hHash, 1, 1, 
                   vNeighbour, hNeighbour, sqDistance, nbrNeighbours);

   //Upper left neighbour
   addToNeighbours(elmDistance, vpos, hpos, vHash, hHash, 1, -1, 
                   vNeighbour, hNeighbour, sqDistance, nbrNeighbours);

   //If there are no elements in the cells we must use a different algorithm.
   if ( (m_hashCell[vHash][hHash]->containsValidElement()) || 
        (nbrNeighbours > 0)) {
      //Deal with cells closer than elmDistance
      if( nbrNeighbours > 0 ) {
         //Sort the neighbours by closest distance (bubblesort)
         bool swapped = true;
         int j = nbrNeighbours - 1;
         uint32 iTemp;

         while( (j > 0) && swapped ) {
            swapped = false;
            for(int i = 0; i < j; i++ ) {
               if( sqDistance[i] > sqDistance[i+1] ) {
                  //swap sqDistance
                  sqTemp          = sqDistance[i];
                  sqDistance[i]   = sqDistance[i+1];
                  sqDistance[i+1] = sqTemp;

                  //swap vertical cell index
                  iTemp           = vNeighbour[i];
                  vNeighbour[i]   = vNeighbour[i+1];
                  vNeighbour[i+1] = iTemp;

                  //swap horizontal cell index
                  iTemp   = hNeighbour[i];
                  hNeighbour[i]   = hNeighbour[i+1];
                  hNeighbour[i+1] = iTemp;

                  swapped = true;
               }
            }
            j--;
         }

         //Now find the closest distance to an element in the neighbour-cell
         uint32 hTemp = hNeighbour[0];
         uint32 vTemp = vNeighbour[0];
         sqTemp = elmDistance;
         tempOffset = m_hashCell[vTemp][hTemp]->
                      getClosest(hpos, vpos, false, sqTemp, areaItem );

         if( nbrNeighbours == 1 ) {
            //Only one neighbour - easy!
            if( sqTemp < elmDistance ) {
               offset = tempOffset;
               dist = sqTemp;
               return m_hashCell[vTemp][hTemp];
            }
            else {
               dist = elmDistance;
               return m_hashCell[vHash][hHash];
            }
         }
         else {
            //Several neighbours - a bit trickier.
            if( (sqTemp <= elmDistance) && (sqTemp <= sqDistance[1]) ) {
               //The closest neighbour is the winner!
               offset = tempOffset;
               dist = sqTemp;
               return m_hashCell[vTemp][hTemp];
            }
            else {
               //Sigh! need to check on the other neighbours

               //Calculate the distances to the closest element in the
               //neighbours
               if( sqTemp < elmDistance ) {
                  dist = sqTemp;
                  tempCell = m_hashCell[vTemp][hTemp];
                  offset = tempOffset;
               }
               else {
                  dist = elmDistance;
                  tempCell = m_hashCell[vHash][hHash];
               }

               //Check ALL neighbours, there is probably a more clever solution
               for( j = 1; j < nbrNeighbours; j++ ) {
                  hTemp = hNeighbour[j];
                  vTemp = vNeighbour[j];
                  sqTemp = dist;
                  tempOffset = m_hashCell[vTemp][hTemp]->
                               getClosest(hpos, vpos, false, sqTemp, areaItem );

                  if( sqTemp < dist ) {
                     dist = sqTemp;
                     tempCell = m_hashCell[vTemp][hTemp];
                     offset = tempOffset;
                  }
               }
               return tempCell;
            }
         }
      }
      else {
         dist = elmDistance;
         return m_hashCell[vHash][hHash]; //no neighbours, return cell
      }
   }
   else {
      dist = elmDistance;//MAX_INT64;
      uint32 nbrCellsToRight = (m_nbrHorizontalCells - 1) - hHash ;
      uint32 nbrCellsToTop = (m_nbrVerticalCells - 1) - vHash ;
      // Find maximum of number of cells in four directions
      uint32 nbrOfLayers = MAX(MAX(hHash, nbrCellsToRight),
                               MAX(vHash, nbrCellsToTop));
      uint32 v, h;
      uint32 layer = 2;
      bool elemFound = false;
      while ((layer <= nbrOfLayers) && (!elemFound)) {
         // If a layer is non-empty, one of the elements in it must
         // be the closest one. 
         int32 maxCol = MIN(layer, nbrCellsToRight);
         int32 minCol = -int32(MIN(layer, hHash));
         int32 maxRow = MIN(layer, vHash);
         int32 minRow = -int32(MIN(layer, nbrCellsToTop));
         // upper row
         if (minRow + layer == 0) {
             v = vHash + layer;
             for (h = minCol + hHash; h <= maxCol + hHash; h++){
               sqTemp = dist;
               tempOffset = m_hashCell[v][h]->
                  getClosest(hpos, vpos, false, sqTemp, areaItem);
               if ( (tempOffset < MAX_UINT32) && (sqTemp < dist )) {
                  if (!elemFound)
                     elemFound = true;
                  dist = sqTemp;
                  tempCell = m_hashCell[v][h];
                  offset = tempOffset;
               }
             }
             
         }
         // right column
         if (maxCol == (int32)layer) {
            h = hHash + layer;
            for (v = vHash - maxRow + 1; v < vHash - minRow; v++){
               sqTemp = dist;
               tempOffset = m_hashCell[v][h]->
                  getClosest(hpos, vpos, false, sqTemp, areaItem);
               if ( (tempOffset < MAX_UINT32) && (sqTemp < dist )) {
                  if (!elemFound)
                     elemFound = true;
                  dist = sqTemp;
                  tempCell = m_hashCell[v][h];
                  offset = tempOffset;
               }
            }
         }
         // bottom row
         if (maxRow == (int32)layer){
             v = vHash - layer;
             for (h = minCol + hHash; h <= maxCol + hHash; h++){
               sqTemp = dist;
               tempOffset = m_hashCell[v][h]->
                  getClosest(hpos, vpos, false, sqTemp, areaItem);
               if ( (tempOffset < MAX_UINT32) && (sqTemp < dist )) {
                  if (!elemFound)
                     elemFound = true;
                  dist = sqTemp;
                  tempCell = m_hashCell[v][h];
                  offset = tempOffset;
               }
             }
             
         } 
         // left column
         if (minCol + layer == 0) {
             h = hHash - layer;
             for (v = vHash - maxRow + 1; v < vHash - minRow; v++){
               sqTemp = dist;
               tempOffset = m_hashCell[v][h]->
                  getClosest(hpos, vpos, false, sqTemp, areaItem);
               if ( (tempOffset < MAX_UINT32) && (sqTemp < dist )) {
                  if (!elemFound)
                     elemFound = true;
                  dist = sqTemp;
                  tempCell = m_hashCell[v][h];
                  offset = tempOffset;
               }
            }
         }
         ++layer;
      }
      return tempCell;
   }
}

inline Vector*
copySetToVector(const set<uint32>& theSet)
{
   Vector* resVect = new Vector(theSet.size());
   for ( set<uint32>::const_iterator it = theSet.begin();
         it != theSet.end();
         ++it ) {
      resVect->push_back(*it);
   }
   return resVect;
}

void
LocationHashTable::getAllWithinRadius(set<uint32>& resSet,
                                      int32 hpos, int32 vpos,
                                      int32 radius)
{
   //hashindex to a cell
   uint32 vHashStart, hHashStart, vHashEnd, hHashEnd;

   uint64 sqRadius = uint64( SQUARE(int64(radius)) );

   
         
   //Check if the circle's boundingbox overlaps cells
   getHashIndex(hpos - radius, vpos - radius, hHashStart, vHashStart);
   getHashIndex(hpos + radius, vpos + radius, hHashEnd  , vHashEnd  );

   if( (hHashStart == hHashEnd) && (vHashStart == vHashEnd) ) {
      //The easy one, just one cell is overlapped

      m_hashCell[vHashStart][hHashStart]->
         getWithinRadius( hpos, vpos, sqRadius, resSet);
   } else {
      //A bit trickier, start getting itemIDs from overlapping cells

      for( uint32 h = hHashStart; h <= hHashEnd; h++ ) {
         for( uint32 v = vHashStart; v <= vHashEnd; v++ ) {
            //Test if the cell REALLY is overlapped
            if( isCellWithinRadius(m_hashCell[v][h], vpos, hpos, sqRadius) ) {
               // Add the results to the result vector.
               m_hashCell[v][h]->getWithinRadius(hpos, vpos, sqRadius,
                                                 resSet);
            }
         }
      }
   }
}

Vector* 
LocationHashTable::getAllWithinRadius(int32 hpos, int32 vpos,
                                      int32 radius, bool &shouldKill)
{
   shouldKill = true;
   set<uint32> resSet;
   getAllWithinRadius(resSet, hpos, vpos, radius);
   if ( resSet.empty() ) {
      return NULL;
   } else {
      return copySetToVector(resSet);
   }
}

Vector*
LocationHashTable::getAllWithinMC2BoundingBox(const MC2BoundingBox* bbox, 
                                              bool& shouldKill)
{
   // Should always kill.
   shouldKill = true;
   if (bbox == NULL) {
      return NULL;
   }
   set<uint32> resSet;
   getAllWithinMC2BoundingBox(resSet, *bbox);
   if ( resSet.empty() ) {
      return NULL;
   } else {
      return copySetToVector(resSet);
   }
}

void
LocationHashTable::getAllWithinMC2BoundingBox(set<uint32>& resIDs,
                                              const MC2BoundingBox& bbox)
   
{
   //hashindex to a cell
   uint32 vHashStart, hHashStart, vHashEnd, hHashEnd;

   //Check if the boundingbox overlaps cells
   getHashIndex(bbox.getMinLon(), bbox.getMinLat(), 
                hHashStart, vHashStart);
   getHashIndex(bbox.getMaxLon(), bbox.getMaxLat(), 
                hHashEnd, vHashEnd  );

   uint32 h = 0;
   uint32 v = 0;

   // Add all items that are in the cells that are not 
   // on the "boundary"
   for (h = hHashStart+1; h < hHashEnd; h++) {
      for (v = vHashStart+1; v < vHashEnd; v++) {
         m_hashCell[v][h]->getAllItems(resIDs);
      }
   }
   
   // Add the items that are on the "boundary" cells, but 
   // this time we need to check if they really are inside 
   // the bounding box or not.
   
   // The upper and lower row
   for (h = hHashStart; h <= hHashEnd; h++ ) {
      
      bool cont = true;
      v = vHashStart;
      
      while (cont) {
         m_hashCell[v][h]->getWithinMC2BoundingBox(&bbox, resIDs);
         
         if (v != vHashEnd) {
            v = vHashEnd;
         } else {
            // Exit loop
            cont = false;
         }
      }
   }

   // The left and right column, but do not include the corner cells.
   for (v = vHashStart+1; v < vHashEnd; v++ ) {
     
      bool cont = true;
      h = hHashStart;
      
      while (cont) {
         // Put the stuff in the result vector.
         m_hashCell[v][h]->getWithinMC2BoundingBox(&bbox, resIDs);

         if (h != hHashEnd) {
            h = hHashEnd;
         } else {
            // Exit loop
            cont = false;
         }
      }
   }
}


uint32 
LocationHashTable::getMemoryUsage() const 
{
   uint32 size = sizeof(LocationHashTable);
   if ( m_hashCell != NULL ) {
      for (uint32 vert=0; vert < m_nbrVerticalCells; vert++)
            if ( m_hashCell[vert] != NULL ) 
               for(uint32 horiz=0; horiz < m_nbrHorizontalCells; horiz++) 
                  if ( m_hashCell[vert][horiz] != NULL ) 
                     size += m_hashCell[vert][horiz]->getMemoryUsage();
   }
   return size;
}


bool 
LocationHashTable::isCellWithinRadius( LocationHashCell* cell,
                                       int32 lat_32, int32 lon_32, 
                                       uint64 sqR )
{
   double cosLat = getHorizontalFactor();
   uint64 minD;

   //Calculate minumum squaredistance to the cell's boundingbox
   if (lon_32 - cell->getMinHorizontal() < 0) {
      if (lat_32 < cell->getMinVertical() )
         minD = uint64( SQUARE( int64(cell->getMinVertical() - lat_32) ) + 
            SQUARE( int64(cosLat*(cell->getMinHorizontal() - lon_32)) ) );

      else if (lat_32 > cell->getMaxVertical())
         minD = uint64( SQUARE( int64(lat_32 - cell->getMaxVertical()) ) + 
            SQUARE( int64(cosLat*(cell->getMinHorizontal() - lon_32)) ) );

      else
         minD = uint64(
            SQUARE( int64(cosLat*(cell->getMinHorizontal() - lon_32)) ) );

   } else if (lon_32 - cell->getMaxHorizontal() > 0 ) {
      if (lat_32 < cell->getMinVertical())
         minD = uint64( SQUARE( int64(cell->getMinVertical() - lat_32) ) + 
            SQUARE( int64(cosLat*(lon_32 - cell->getMaxHorizontal())) ) );

      else if (lat_32 > cell->getMaxVertical())
         minD = uint64( SQUARE( int64(lat_32 - cell->getMaxVertical()) ) + 
            SQUARE( int64(cosLat*(lon_32 - cell->getMaxHorizontal())) ) );

      else
         minD = uint64(
            SQUARE( int64(cosLat*(lon_32 - cell->getMaxHorizontal())) ) ) ;

   } else {
      if (lat_32 < cell->getMinVertical())
         minD = uint64( SQUARE( int64(cell->getMinVertical() - lat_32) ) );

      else if (lat_32 > cell->getMaxVertical())
         minD = uint64( SQUARE( int64(lat_32 - cell->getMaxVertical()) ) );

      else
         minD = 0;  //Inside
   }

   //Test if the cell is within the radius
   if( minD <= sqR )
      return true;
   else
      return false;
}

Vector* 
LocationHashTable::removeDups(Vector* proximityResult)
{
#define LOCATION_HT_TEST_SET_IN_REMOVEDUPS
#ifdef  LOCATION_HT_TEST_SET_IN_REMOVEDUPS
   mc2dbg << "[LHT]: Nbr elements before = " << proximityResult->size()
          << endl;
   // New Method   
   set<uint32> allIDs;
   uint32 vectSize = proximityResult->getSize();
   for(uint32 j=0; j < vectSize; ++j ) {
      allIDs.insert((*proximityResult)[j]);
   }

   // Reuse the old vector. It should be of about the right size.
   proximityResult->reset();

   set<uint32>::const_iterator it(allIDs.begin());
   while ( it != allIDs.end() ) {
      proximityResult->addLast(*it++);
   }
   mc2dbg << "[LHT]: Nbr elements after = " << proximityResult->size()
          << endl;
   return proximityResult;
   
#else
   // Method without set, also new.
   // Will sort the vector and remove doubles.
   proximityResult->removeDoubles();
   return proximityResult;
#endif
}


void 
LocationHashTable::getHashIndex(int32 hpos, int32 vpos,
                                uint32 &hHash, uint32 &vHash) const
{
   int32 vHashTemp = ( vpos - m_bottomVertical ) >> m_verticalShift;

   //vHash must be within limits
   if( vHashTemp < 0 ) {
//      almostAssert(false); // someone possibly initiated the bounding box incorrectly
      vHash = 0;
   } else if( vHashTemp > (int32)m_nbrVerticalCells - 1 ) {
//      almostAssert(false); // someone possibly initiated the bounding box incorrectly
      vHash = m_nbrVerticalCells-1;
   } else {
      vHash = vHashTemp;
   }

   int32 hHashTemp = ( hpos - m_leftHorizontal ) >> m_horizontalShift;

   //hHash must be within limits
   if( hHashTemp < 0 ) {
//      almostAssert(false); // someone possibly initiated the bounding box incorrectly
      hHash = 0;
   } else if( hHashTemp > (int32)m_nbrHorizontalCells-1 ) {
//      almostAssert(false); // someone possibly initiated the bounding box incorrectly
      hHash = m_nbrHorizontalCells-1;
   } else {
      hHash = hHashTemp;
   }
}

void 
LocationHashTable::invHashIndex(int32 &hpos, int32 &vpos,
                                uint32 hHash, uint32 vHash)
{
   vpos = ( vHash << m_verticalShift   ) + m_bottomVertical;
   hpos = ( hHash << m_horizontalShift ) + m_leftHorizontal;
}

double 
LocationHashTable::getHorizontalFactor()
{
   return 1.0;
}
