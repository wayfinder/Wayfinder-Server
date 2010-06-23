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

#include "GfxData.h"

#include "AllocatorTemplate.h"
#include "DataBuffer.h"
#include "GenericMap.h"

#include "GfxUtility.h"
#include "GfxDataFull.h"
#include "GfxDataFactory.h"

#include "STLUtility.h"

#include "Stack.h"
#include "Math.h"

/**
 *    Help class for the getConvexHull() method in GfxData.
 *    Is used to sort the coordinates by the angle from the
 *    reference coordinate, and in case of ties, use the distance
 *    from the reference coordinate.
 *
 */
class ConvexHullNotice {
public:
   /**
    *    Constructor setting members.
    */
   ConvexHullNotice(float64 angle, uint32 index, float64 dist)
      : m_angle(angle), m_index(index), m_dist(dist) {}

   /**
    *   @name The operators needed for sorting and searching.
    *
    */
   //@{
   /// equal
   bool operator == (const ConvexHullNotice& elm) const {
      return ((m_angle == elm.m_angle) &&
              (m_dist == elm.m_dist));

   };

   /// not equal
   bool operator != (const ConvexHullNotice& elm) const {
      return ((m_angle != elm.m_angle) ||
              (m_dist != elm.m_dist));
   };

   /// greater
   bool operator > (const ConvexHullNotice& elm) const {
      if (m_angle > elm.m_angle)
         return (true);
      else if ( (m_angle == elm.m_angle) &&
                (m_dist > elm.m_dist) )
         return (true);
      else
         return (false);
   };

   /// less
   bool operator < (const ConvexHullNotice& elm) const {
      if (m_angle < elm.m_angle)
         return (true);
      else if ( (m_angle == elm.m_angle) &&
                (m_dist < elm.m_dist) )
         return (true);
      else
         return (false);
   };
   //@}

   /**
    *    @name The members. Publicly declared since this class
    *          is only a containerclass anyway.
    */
   //@{
   ///   The angle in radians from the reference coordinates.
   float64 m_angle;

   ///   The index of this coordinate pair in the GfxData.
   uint32 m_index;

   ///   The distance to the reference coordinate.
   float64 m_dist;
   //@}
};

GfxDataFull*
GfxData::createNewGfxData(GenericMap* theMap, bool createNewPolygon)
{
   GfxDataFull* gfx = NULL;
   if (theMap != NULL) {
      gfx = static_cast<GfxDataFull*>( theMap->
                                       getGfxDataFactory().
                                       create( gfxDataFull ) );
   } else {
      gfx = new GfxDataFull();
   }

   if (createNewPolygon) {
      gfx->addPolygon();
   }

   return gfx;
}

GfxData*
GfxData::createNewGfxData(GenericMap* theMap, const MC2BoundingBox* bbox)
{
   GfxDataFull* gfx = createNewGfxData(theMap);

   gfx->addCoordinate(bbox->getMinLat(), bbox->getMinLon(), true);
   gfx->addCoordinate(bbox->getMaxLat(), bbox->getMinLon());
   gfx->addCoordinate(bbox->getMaxLat(), bbox->getMaxLon());
   gfx->addCoordinate(bbox->getMinLat(), bbox->getMaxLon());
   gfx->setClosed(0, true);
   gfx->updateBBox();
   gfx->updateLength();
   return gfx;
}

GfxData*
GfxData::createNewGfxData(GenericMap* theMap,
                          const coordinate_type* firstLat,
                          const coordinate_type* firstLon,
                          uint16 nbrCoords,
                          bool closed)
{
   GfxDataFull* gfx = createNewGfxData(theMap);

   for (uint32 i=0; i < nbrCoords; i++) {
      gfx->addCoordinate(firstLat[i], firstLon[i], i==0);
   }
   gfx->setClosed(0, closed);
   gfx->updateBBox();
   gfx->updateLength();
   return gfx;
}

GfxData*
GfxData::createNewGfxData(GenericMap* theMap, const GfxData* src)
{
   GfxDataFull* dest = createNewGfxData(theMap, false);

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

void
GfxData::save(DataBuffer& dataBuffer) const
{
   dataBuffer.alignToLong();

   dataBuffer.writeNextBool(getClosed(0));
   dataBuffer.writeNextByte(byte(getGfxDataType()) );
   dataBuffer.writeNextShort((uint16) getNbrPolygons()); // Nbr polygons
   DEBUG_DB(mc2dbg << "   GfxData::save " << getNbrPolygons() << " pologons"
            << endl;);

   for (uint32 p=0; p<getNbrPolygons(); ++p) {
      dataBuffer.writeNextLong(getNbrCoordinates(p));
      DEBUG_DB(mc2dbg << "      GfxData::save " << getNbrCoordinates(p)
               << " coordinates in pologon" << endl;);

      for (uint32 i=0; i<getNbrCoordinates(p); ++i) {
         dataBuffer.writeNextLong(getLat(p, i));
         dataBuffer.writeNextLong(getLon(p, i));
      }

      // Write updated length.
      dataBuffer.writeNextFloat( calcPolyLength( p ) );
   }

}

ostream& operator << (ostream& ostr, const GfxDataTypes::dimensions& dim ) {
   ostr << "(" << dim.height << ", " << dim.width << ")";
   return ostr;
}

ostream& operator <<( ostream& ostr, const GfxDataTypes::textPos& pos ) {
   ostr << "(" << pos.lat << ", " << pos.lon << ", " << pos.angle << ")";
   return ostr;
}


namespace {

/// @return non squared distance and with no coslat.
float64 normalDistance( const MC2Coordinate& start,
                  const MC2Coordinate& end ) {
   return ::sqrt( GfxUtility::squareP2Pdistance_linear( start, end, 1.0 ) );
}

}

bool
GfxData::getTextPosition( const vector<MC2BoundingBox>& objectBBoxes,
                          uint16 polygon, GfxDataFull* tmpGfx2,
                          const vector<GfxData*>& gfxTextArray ) const
{

   /*
    *
    * intersect box
    *  |
    *  \  +---------------+  intersect box
    *   \ |   gfx box     | /
    *    \|               |/
    * +---+-----+ +-------+----+
    * |   |.....| |.......|    |
    * |   |.....| |.......|    |
    * |   +-----+-+-------+    |
    * | obj box | |  obj box   |
    * +---------+ |            |
    *             |            |
    *             +------------+
    *
    *  From each object box create a new
    *  set of intersecting boxes on the current
    *  gfx data which will later be used to
    *  determine if the actuall intersect box
    *  were colliding with the text.
    *
    */

   uint32 n = getNbrCoordinates(polygon);
   vector<MC2BoundingBox> intersectedBBoxes;
   MC2BoundingBox gfxBBox;
   getMC2BoundingBox( gfxBBox, polygon );

   {
      MC2BoundingBox intersectBox;
      // check intersections with elements in objectBBoxes and in gfxTextArray,
      // store the intersections in a vector
      for (vector<MC2BoundingBox>::const_iterator it = objectBBoxes.begin();
           it != objectBBoxes.end();
           ++it ) {
         if ((*it).getInterSection(gfxBBox, intersectBox)) {
            intersectBox.updateCosLat();
            intersectedBBoxes.push_back( intersectBox );
         }
      }
   }

   {
      MC2BoundingBox intersectBox, tempBBox2;
      const GfxData* existingTextGfx;
      for ( uint32 i = 0; i < gfxTextArray.size(); ++i ) {
         if ( (existingTextGfx = gfxTextArray[i]) != NULL ) {
            existingTextGfx->getMC2BoundingBox(tempBBox2);
            if ( tempBBox2.getInterSection( gfxBBox, intersectBox ) ) {
               intersectBox.updateCosLat();
               intersectedBBoxes.push_back( intersectBox );
            }
         }
      }
   }

   // Check if there were overlappings
   if ( intersectedBBoxes.empty() ) {
      for (uint32 j = 0; j < getNbrCoordinates(polygon); j++)
         tmpGfx2->addCoordinate(getLat(polygon,j), getLon(polygon,j));
      tmpGfx2->updateLength();
      return true;
   }

   // Overlappings exist
   vector<MC2BoundingBox> candidates;
   candidates.clear();
   float64 distance, maxDistance;
   distance = maxDistance = 0;
   MC2BoundingBox bb = gfxBBox;
   uint32 firstNode = n - 1;
   uint32 lastNode = 0;
   MC2Coordinate roadStart;
   MC2Coordinate roadEnd;
   int bestboxNr = 0;
   bool calcExtraStartNode = false, calcExtraEndNode = false;


   MC2BoundingBox tempBBox;
   for ( vector<MC2BoundingBox>::iterator it = intersectedBBoxes.begin();
         it != intersectedBBoxes.end();
         ++it ) {

      if ( ! (*it).getInterSection(bb,tempBBox) ) {
         continue;
      }
      candidates.clear();
      // add candidats with non-zero areas
      MC2BoundingBox candidateBBox = MC2BoundingBox(bb.getMaxLat(), bb.getMinLon(),
                                                    tempBBox.getMaxLat(),bb.getMaxLon());
      if ((candidateBBox.getLonDiff() != 0) && (candidateBBox.getHeight() != 0))
         candidates.push_back(candidateBBox);
      candidateBBox = MC2BoundingBox(bb.getMaxLat(), tempBBox.getMaxLon(),
                                     bb.getMinLat(), bb.getMaxLon());
      if ((candidateBBox.getLonDiff() != 0) && (candidateBBox.getHeight() != 0))
         candidates.push_back(candidateBBox);
      candidateBBox = MC2BoundingBox(tempBBox.getMinLat(), bb.getMinLon(),
                                     bb.getMinLat(), bb.getMaxLon());
      if ((candidateBBox.getLonDiff() != 0) && (candidateBBox.getHeight() != 0))
         candidates.push_back(candidateBBox);
      candidateBBox = MC2BoundingBox(bb.getMaxLat(), bb.getMinLon(),
                                     bb.getMinLat(), tempBBox.getMinLon());
      if ((candidateBBox.getLonDiff() != 0) && (candidateBBox.getHeight() != 0))
         candidates.push_back(candidateBBox);

      if ( candidates.empty() ) {
         return false;
      }

      bestboxNr = 0;
      maxDistance = 0;
      for ( uint32 i = 0; i < candidates.size(); ++i ) {
         distance = 0;
         int32 tempFirstNode = 0;
         tempBBox = candidates[ i ];

         // clip the street with tempBBox

         // for each segment collision test against candidate box
         for ( uint32 coord = 0; coord < n - 1; ++coord ) {
            MC2Coordinate startCoord = getCoordinate( polygon, coord );
            MC2Coordinate endCoord = getCoordinate( polygon, coord + 1 );
            MC2Coordinate clipped;
            /*
             *  First case: The entire segment is contain in the
             *              candidate box.
             *
             *  +--------------+
             *  |   segment    |
             *  |          o   |
             *  |         /    | candidate box
             *  |        /     |
             *  |       /      |
             *  |      /       |
             *  |     /        |
             *  |    /         |
             *  |   /          |
             *  |  o           |
             *  +--------------+
             *
             */
            if ( tempBBox.contains( startCoord ) &&
                 tempBBox.contains( endCoord ) ) {

               // distance is the entire segment in the box
               distance += ::normalDistance( startCoord, endCoord );

               if (distance > maxDistance) {
                  maxDistance = distance;
                  firstNode = tempFirstNode;
                  lastNode = coord + 1;
                  bestboxNr = i;
                  calcExtraStartNode = false;
                  calcExtraEndNode = false;
               }

            } else if ( tempBBox.
                        clipToFirstIntersectionWithEdge( startCoord,
                                                         endCoord,
                                                         clipped ) ) {
               /*
                *  Second case: Clip the line to the first intersection
                *               with the candidate box. In this case
                *               start to "A".
                *
                *         +--------------+
                * start   |              |     end   Line "R"
                *  o------A--------------B------o
                *         |              |
                *         |              |
                *         |              |
                *         |              |
                * start   |  end         |           Line "S"
                *  o------A---o          |
                *         |              |
                *         |              |
                *         |              |
                *         |       start  |     end   Line "T"
                *         |          o---B------o
                *         |              |
                *         +--------------+
                *
                * Line "R", "S" and "T" illustrate different cases that 
                * Needs to be handled.
                *
                */

               // if the start point is the same as the cut point
               // then the cut point must be "B" in the illustration.
               if ( startCoord == clipped ) {
                  // lets find out where "B" is by changing the
                  // start and end points
                  tempBBox.clipToFirstIntersectionWithEdge( endCoord,
                                                            startCoord,
                                                            clipped );
               }

               if ( ! tempBBox.contains( startCoord ) &&
                    tempBBox.contains( endCoord ) ) {

                  // This is the case with Line "S"
                  // where start is outside and end coord is inside
                  // the box.

                  // calculate the distance inside the box
                  distance = ::normalDistance( clipped, endCoord );

                  tempFirstNode = coord + 1;
                  if (distance > maxDistance) {
                     firstNode = tempFirstNode;
                     lastNode = firstNode;
                     maxDistance = distance;
                     bestboxNr = i;
                     calcExtraStartNode = false;
                     calcExtraEndNode = false;
                  }
               } else if ( ! tempBBox.contains( endCoord ) &&
                           tempBBox.contains( startCoord ) ){

                  // This is the case with Line "T"
                  // where start is inside and end coord is outside
                  // the box.

                  // calculate the distance inside the box
                  distance += ::normalDistance( clipped, startCoord );

                  if (distance > maxDistance) {
                     firstNode = tempFirstNode;
                     maxDistance = distance;
                     lastNode = coord;
                     bestboxNr = i;
                     calcExtraStartNode = false;
                     calcExtraEndNode = false;
                  }
                  distance = 0;
               } else {

                  // This is the case with Line "R"
                  // Both start and end coordinates are outside the box.

                  // Determine point "B" for this line.
                  MC2Coordinate secondClip;
                  if ( tempBBox.
                       clipToFirstIntersectionWithEdge( endCoord,
                                                        startCoord,
                                                        secondClip )) {
                     // calculate distance that are inside the box
                     distance = ::normalDistance( clipped, secondClip );

                     if ( distance > maxDistance ) {
                        maxDistance = distance;
                        firstNode = n - 1;
                        lastNode = 0;
                        calcExtraStartNode = true;
                        calcExtraEndNode = true;
                        roadStart = clipped;
                        roadEnd = secondClip;
                        bestboxNr = i;
                     }
                     distance = 0;
                  }
               }
            }//else
         }
      }
      bb = candidates[bestboxNr];

   }//for

   bb.updateCosLat();

   //Find out if new start- and endpoints must be calculated
   if ( (firstNode > 0) && (firstNode <= lastNode) && (!calcExtraStartNode) &&
        (maxDistance > 0) ){
      if ( bb.
           clipToFirstIntersectionWithEdge( getCoordinate( polygon,
                                                           firstNode - 1 ),
                                            getCoordinate( polygon, firstNode),
                                            roadStart ) ) {
         calcExtraStartNode = true;
      }
   }

   if ( (lastNode < n - 1) &&
        (firstNode <= lastNode) && (!calcExtraEndNode) &&
        (maxDistance > 0) ){
      if ( bb.
           clipToFirstIntersectionWithEdge( getCoordinate( polygon,
                                                           lastNode + 1 ),
                                            getCoordinate( polygon, lastNode),
                                            roadEnd ) ) {
         calcExtraEndNode = true;
      }
   }

   if ( calcExtraStartNode ) {
      tmpGfx2->addCoordinate( roadStart );
   }

   for ( uint32 j = firstNode; j <= lastNode; ++j ) {
      tmpGfx2->addCoordinate( getCoordinate( polygon, j ) );
   }

   if ( calcExtraEndNode ) {
      tmpGfx2->addCoordinate( roadEnd );
   }

   tmpGfx2->updateLength();

   if ( tmpGfx2->getNbrCoordinates( 0 ) < 2 ) {
      return false;
   }

   return true;
}

// ******************* END NEW *******************************

GfxData*             // static method
GfxData::getArcApproximation(int32 lat, int32 lon,
                             int startAngle, int stopAngle,
                             int outerRadius, int innerRadius,
                             float64 angleIncrement)
{
   mc2dbg1 << "Creating GfxData" << endl;
   // Get the angles in radians and make sure that
   // startAngle < stopAngle
   float64 alpha = startAngle * M_PI / 180.0;
   float64 beta = stopAngle * M_PI / 180.0;
   if (beta < alpha)
      beta += M_PI*2.0;

   // Calculate the cosLat-factor for the centre
   float64 invCosLat = 1.0 / cos(lat*GfxConstants::invRadianFactor);
   mc2dbg4 << "invCosLat = " << invCosLat << ", cosLat = "
           << 1.0/invCosLat << endl;

   // Where to store the coordinates
   Vector lats(512, 128);
   Vector lons(512, 128);

   // The first line
   float64 cosAlpha = cos(alpha);
   float64 sinAlpha = sin(alpha) * invCosLat;
   lats.addLast( uint32(lat + cosAlpha*innerRadius) );
   lons.addLast( uint32(lon + sinAlpha*innerRadius) );
   lats.addLast( uint32(lat + cosAlpha*outerRadius) );
   lons.addLast( uint32(lon + sinAlpha*outerRadius) );

   // Outer arc
   float64 curAngle = alpha + angleIncrement;
   while (curAngle < beta) {
      lats.addLast( uint32(lat + cos(curAngle)*outerRadius) );
      lons.addLast( uint32(lon + sin(curAngle)*outerRadius*invCosLat) );
      curAngle += angleIncrement;
   }

   // The last point of the outer arc
   lats.addLast( uint32(lat + cos(beta)*outerRadius) );
   lons.addLast( uint32(lon + sin(beta)*outerRadius*invCosLat) );

   // Add the inner arc, if innerRadius > 0
   curAngle = beta;
   while (curAngle > alpha) {
      lats.addLast( uint32(lat + cos(curAngle)*innerRadius) );
      lons.addLast( uint32(lon + sin(curAngle)*innerRadius*invCosLat) );
      curAngle -= angleIncrement;
   }

   // Create the gfxData that is an approximation of the area and
   // return it
   /*GfxData* retGfx = new GfxDataFull((int32*) lats.getBuffer(),
                                 (int32*) lons.getBuffer(),
                                 lats.getSize(),
                                 true);*/
   GfxData* retGfx = createNewGfxData(NULL,
                                      (int32*) lats.getBuffer(),
                                      (int32*) lons.getBuffer(),
                                      lats.getSize(),
                                      true);
   return (retGfx);
}


GfxDataFull*
GfxData::removeTooCloseCoordinates(const uint32 polygon, const int64 minSqDist)
{
   //GfxData* filteredPolygon = new GfxDataFull(true);
   GfxDataFull* filteredPolygon = createNewGfxData(NULL, true);
   uint32 n = getNbrCoordinates(polygon);
   uint32 polyIndex = 0;
   uint32 nextIndex = 1;
   filteredPolygon->addCoordinate(getLat(0, polyIndex),
                                  getLon(0, polyIndex));

   while (nextIndex != 0) {
      while((dist(polygon, polyIndex, nextIndex) < minSqDist) &&
            (nextIndex != 0))
         nextIndex = (nextIndex + 1) % n;

      if ((nextIndex != 0) &&
          // dont add the last but one vertex if this vertex is too close
          // to the start vertex 0 (always added)
          (!((nextIndex == n - 1) &&
             dist(polygon, nextIndex, 0) < minSqDist))) {
         filteredPolygon->addCoordinate(getLat(0, nextIndex),
                                        getLon(0, nextIndex));
      }
      if (nextIndex != 0){
         polyIndex = nextIndex;
         nextIndex = (nextIndex + 1) % n;
      }
   }
   mc2dbg4 << "Nbr of coordinates in original polygon: " << n << " " << endl;
   mc2dbg4 <<"Nbr of coordinates in filtered polygon: "
           << filteredPolygon->getNbrCoordinates(0) << " " << endl;
   return filteredPolygon;
}


bool
GfxData::getPolygonCentroid(const uint32 polygon,
                            int32& centroidLat,
                            int32& centroidLon) const
{
   centroidLat = centroidLon = MAX_INT32;
   uint32 n = getNbrCoordinates(polygon);
   if ( (n < 3) || (!closed()) ) {
      mc2log << error
             << "GfxData::getPolygonCentroid Polygon has less than"
             << "three nodes or is not closed, number of nodes = "
             << n << " closed=" << closed() << endl;
      return(false);
   }

   double latSum, lonSum, areaSum;
   latSum = lonSum = areaSum = 0;

   // This computation seems to be a combination of
   // computation of the area of the polygon and
   // the computation of the centroid when the area
   // is known. The area computation will not work
   // for self intersecting polygons.
   for (uint32 polyIndex = 0; polyIndex < n; polyIndex++){
      int32 x1 = getLon(polygon, polyIndex);
      int32 y1 = getLat(polygon, polyIndex);
      int32 x2 = getLon(polygon, (polyIndex + 1)%n);
      int32 y2 = getLat(polygon, (polyIndex + 1)%n);
      double commonTerm = double(x1)*y2-double(x2)*y1;
      latSum += y1*commonTerm+y2*commonTerm;
      lonSum += x1*commonTerm+x2*commonTerm;
      areaSum += commonTerm;
   }
   centroidLat = int32(rint(double(latSum)/(3*areaSum)));
   centroidLon = int32(rint(double(lonSum)/(3*areaSum)));

   // FIXME: The code above does not work when the polygon intersects
   //        itself like the number 8.
   //        Until that works, take the center of the bounding box
   //        for the polygons that aren't inside the bbox.
   MC2BoundingBox bbox;
   getMC2BoundingBox(bbox);
#define CENTROID_DOES_NOT_ALWAYS_WORK
#ifdef  CENTROID_DOES_NOT_ALWAYS_WORK
   // This is the code that we will use until the other code
   // works.
   if ( ! bbox.contains( centroidLat, centroidLon ) ) {
      // Change the coordinates to the center of the bbox.
      mc2dbg << "[GfxData]: centroid failed - using bbox-center" << endl;
      bbox.getCenter( centroidLat, centroidLon );
   }
#else
   // This is the code that should be there.
   MC2_ASSERT( bbox.contains( centroidLat, centroidLon ) );
#endif

   return(true);
}

bool
GfxData::convexPolygon(const uint32 polygon) const
{
   uint32 prevIndex, nextIndex;
   uint32 n = getNbrCoordinates(polygon);
   bool convex = true;
   // Triangles are always convex
   if ( n > 3 ) {
      for(uint32 currIndex = 0; currIndex < n; currIndex++) {
         if (currIndex == 0)
            prevIndex = n - 1;
         else
            prevIndex = currIndex - 1;
         nextIndex = currIndex + 1;
         if (nextIndex == n)
            nextIndex = 0;
         // check if the angle (prevIndex, currIndex, nextIndex) is
         // greater than PI
         // Nobody is greater than pi!
         if (orientation(polygon, prevIndex, currIndex, nextIndex ) == -1) {
            convex = false;
            break;
         }
      }
   } else if ( n < 3 ) {
      mc2dbg4 << "GfxData::convexPolygon Polygon is only a point or a line"
              << endl;
      convex = false;
   }
   return convex;
}// END checkIfConvexPolygon



uint64
GfxData::getBBoxArea_mc2() const
{
   MC2BoundingBox bbox;
   getMC2BoundingBox(bbox);
   return bbox.getArea();
}


float64
GfxData::getBBoxArea() const
{
   return getBBoxArea_mc2() * GfxConstants::SQUARE_MC2SCALE_TO_SQUARE_METER;
}

bool
GfxData::insideBBox(int32 lat, int32 lon) const
{
   MC2BoundingBox bbox;
   getMC2BoundingBox(bbox);
   return bbox.inside(lat, lon);
}



uint32
GfxData::stepIndex(int step, int32 index, int32 nbrCoord) const
{
   int32 newIdx = index + step;
   if ( newIdx < 0 ) {
      newIdx += nbrCoord;
   } else if ( newIdx >= nbrCoord ) {
      newIdx -= nbrCoord;
   }
   return newIdx;
}

bool
GfxData::checkConcaveCorner( uint16 polyIndex, int32 currentIndex )
{
   int32 prevIndex =
      stepIndex( -1, currentIndex, getNbrCoordinates( polyIndex ) );
   int32 nextIndex =
      stepIndex( 1, currentIndex, getNbrCoordinates( polyIndex ) );

   int32 ux = getLon(polyIndex, nextIndex) -
               getLon(polyIndex, currentIndex);
   int32 uy = getLat(polyIndex, nextIndex) -
               getLat(polyIndex, currentIndex);
   int32 vx = getLon(polyIndex, prevIndex) -
               getLon(polyIndex, currentIndex);
   int32 vy = getLat(polyIndex, prevIndex) -
               getLat(polyIndex, currentIndex);
   // Calculate z-component of cross product between u and v
   if ( ((int64)ux*vy)-((int64)uy*vx) < 0 ) {
      return false;
   } else {
      return true;
   }
}

bool
GfxData::closedPolygonFilter( Stack* newPoly, uint32 polyIndex,
                            uint32 pMaxLatDist, uint32 pMaxWayDist)
{
   // Make sure that inparameters are OK
   int32 nbrCoords = getNbrCoordinates( polyIndex );
   if (newPoly == NULL) {
      mc2log << error << "GfxData::closedPolygonFilter stack == NULL" << endl;
      return (false);
   }
   else if ( nbrCoords < 3 ) {
      mc2log << warn << "GfxData::closedPolygonFilter too few coordinates"
             << endl;
      for (int32 i=0; i<nbrCoords; i++) {
         newPoly->push(i);
      }
      return (true);
   }

   // Recalculate distances (from meters to SQUARE(mc2)
   uint64 maxSqMC2Dist = uint64( SQUARE(uint64(pMaxLatDist)) *
                                 GfxConstants::SQUARE_METER_TO_SQUARE_MC2SCALE);
   uint64 maxSqMC2WayDist = uint64(SQUARE(uint64(pMaxWayDist)) *
                                   GfxConstants::SQUARE_METER_TO_SQUARE_MC2SCALE);

   int32 prevAddedIndex = 0;
   newPoly->push( prevAddedIndex );

   int32 currentIndex = prevAddedIndex + 1;

   while ( currentIndex < nbrCoords ) {

      if ( ! checkConcaveCorner( polyIndex, currentIndex ) ) {

         // Point is on convex hull.
         // No need to skip any points.

      } else {
         // Point is NOT on convex hull.

         // Step forward til we find a point on the convex hull or
         // the distances are too large.
         ++currentIndex;

         while ( ( currentIndex < nbrCoords ) &&
                 checkConcaveCorner( polyIndex, currentIndex ) &&
                 ( maxCrossDistance( polyIndex, prevAddedIndex, currentIndex )
                   <= maxSqMC2Dist ) &&
                 ( uint64( dist( polyIndex, prevAddedIndex, currentIndex ) )
                   <= maxSqMC2WayDist ) ) {
            // Skip point.
            ++currentIndex;
         }
      }
      // Add point if we didn't reach the end of the polygon.
      if ( currentIndex < nbrCoords ) {
         newPoly->push( currentIndex );
         prevAddedIndex = currentIndex;
         ++currentIndex;
      }
   }

   return true;
}



uint16
GfxData::getAverageNbrMaxTries(const uint16 polygon) const
{
  // Can be used in method getRandomCoordinateInside below.
  double polyArea = fabs(polygonArea(polygon));
  MC2BoundingBox bb;
  getMC2BoundingBox(bb, polygon);
  uint64 boundingBoxArea = bb.getArea();
  return uint16(double(boundingBoxArea)/polyArea);
}
// END getAverageNbrMaxTries

bool
GfxData::getRandomCoordinateInside(coordinate_type& lat,
      coordinate_type& lon, uint32 maxTries) const
{
   if(!closed())
      return false;

   uint32 nbrAttempts = 0;
   while(nbrAttempts < maxTries) {
      float64 randValue = float64(random()) / float64(RAND_MAX);
      lat = getMinLat()
         + coordinate_type((getMaxLat() - getMinLat()) * randValue);
      lon = getMinLon()
         + coordinate_type((getMaxLon() - getMinLon()) * randValue);
      if(insidePolygon(lat, lon) == 2)
         return true;
      else
         ++nbrAttempts;
   }
   return false;
}


int
GfxData::insidePolygon(MC2Coordinate coord,
                       uint16 usePolygon) const
{
   return insidePolygon(coord.lat, coord.lon, usePolygon);
}


int
GfxData::insidePolygon(coordinate_type lat, coordinate_type lon,
                       uint16 usePolygon) const
{
   coordinate_type minLat, maxLat, minLon, maxLon;
   getBBox(minLat, maxLat, minLon, maxLon);

   if( !closed() ||
       getNbrCoordinates(0) == 0 ||
       (lat < minLat || lat > maxLat ||
        lon - maxLon > 0 || lon - minLon < 0)) {
      return 0;
   }

   uint16 firstPolygon = 0;
   uint16 lastPolygon = getNbrPolygons();
   if ((usePolygon < getNbrPolygons() ) && usePolygon != MAX_UINT16) {
      firstPolygon = usePolygon;
      lastPolygon = firstPolygon + 1;
   }

   mc2dbg8 << "INSIDEPOLYGON: firstPolygon = " << firstPolygon
           << ", lastPolygon = " << lastPolygon << endl;

   // All calculations are being done in a local coordinate system
   // centered in the testpoint.
   for (uint16 p=firstPolygon; p<lastPolygon; ++p) {
      if (getNbrCoordinates(p) == 0)
         return 0;

      coordinate_type d_lat1 = getLastLat(p) - lat;
      coordinate_type d_lon1 = getLastLon(p) - lon;

      if(d_lat1 == 0 && d_lon1 == 0) //Take care of case with only one point
         return 1;

      int32 counter = 0;
      for (uint32 i = 0; i < getNbrCoordinates(p); ++i) {
         coordinate_type d_lat2 = getLat(p, i) - lat;
         coordinate_type d_lon2 = getLon(p, i) - lon;
         if(d_lat1 != d_lat2 || d_lon1 != d_lon2) {
            //If 2 consecutive polygon points are identical we loop on.
            if((d_lat1 <= 0 && d_lat2 >= 0)
                  || (d_lat1 >= 0 && d_lat2 <= 0)) {
               // If exactly one is negative
               int64 volprod = ((int64)d_lon1) * (d_lat2 - d_lat1)
                  - ((int64)d_lat1) * (d_lon2 - d_lon1);
               if(volprod == 0) {
                  // Here we have 3 possibilities,
                  // 1 ) Either the point is on the line.
                  //     Here we should return 1
                  // 2 ) It is a zero length line, but case is eliminated
                  //     above.
                  // 3 ) Line pt1-pt2 is parallell to line from pt to pt1
                  //     and the line is not crossing the point (case 1)
                  //     This iplies that d_lat1 = d_lat2 = 0 and we should
                  //     proceed.
                  if( (d_lon1 <= 0 && d_lon2 >= 0) ||
                           (d_lon1 >= 0 && d_lon2 <= 0 )){
                     return 1; //The the point is on the line and thus inside.
                  }
                  //else
                  //   ;  //proceed as usual
               }
               else if((d_lat1 == 0) || (d_lat2 == 0)) {
                  // A situation were the line jumps to or from a point at
                  // zero d_lat
                  if(volprod > 0)   // Case not clockwise
                     counter++;
                  else              // Case clockwise
                     counter--;
               }
               else {   // The line fully crosses the line dlat = 0
                  if(volprod > 0)   // Case no clockwise
                     counter += 2;
                  else              // Case clockwise
                     counter -= 2;
               }
            }
         }

         // Setting current point as previos point!
         d_lat1 = d_lat2;
         d_lon1 = d_lon2;
      }
#ifdef __linux
      //if(counter % 4) {
      //   mc2dbg << "Error in GfxData::inside() counter = "
      //        << counter << endl;
      //}
#endif

      // If we are at the outside the counter will be zero!
      // (At the inside it will be like +-4 (hopefully)) or
      //  4+-4k, k integer for windup!
      if(counter != 0)
         return 2;
   }

   // If we are outside all the subpolygons we then have to return false.
   return  0;
}

int
GfxData::nbrCornersInsidePolygon(const MC2BoundingBox& bbox,
                                 int maxCorners,
                                 uint16 usePolygon) const
{
   // FIXME: Check the bbox of the GfxData?
   int res = 0;
   if ( insidePolygon(bbox.getMaxLat(), bbox.getMinLon() ) ) {
      ++res;
      if ( res >= maxCorners ) {
         return res;
      }
   }
   if ( insidePolygon(bbox.getMinLat(), bbox.getMinLon() ) ) {
      ++res;
      if ( res >= maxCorners ) {
         return res;
      }
   }
   if ( insidePolygon(bbox.getMaxLat(), bbox.getMaxLon() ) ) {
      ++res;
      if ( res >= maxCorners ) {
         return res;
      }
   }
   if ( insidePolygon(bbox.getMinLat(), bbox.getMaxLon() ) ) {
      ++res;
   }
   return res;
}

uint64
GfxData::squareDistWithOffsetToLine_mc2(
      coordinate_type lat, coordinate_type lon,
      uint16& closestPolygon, uint32& closestI, float64& closestT,
      uint16 usePolygon) const
{
   uint64 minDist = MAX_UINT64;
   uint32 minI = MAX_UINT32;

   coordinate_type minLat, maxLat, minLon, maxLon;
   getBBox(minLat, maxLat, minLon, maxLon);
   float64 bbcoslat = cos( 2 * M_PI / POW2_32 * (maxLat / 2 + minLat / 2));

   if( (getNbrPolygons() == 0) || (getNbrCoordinates(0) == 0))
      return MAX_UINT64;

   uint16 firstPolygon = 0;
   uint16 lastPolygon = getNbrPolygons();
   if (usePolygon < getNbrPolygons() && usePolygon != MAX_UINT16) {
      firstPolygon = usePolygon;
      lastPolygon = firstPolygon + 1;
   }

   // FIXME: Is this correct?
   uint16 minPolygon = lastPolygon;
   uint32 currentPointIndex = 0;

   for (uint16 p=firstPolygon; p != lastPolygon; p++) {
      currentPointIndex = 0;
      coordinate_type lat1 = 0, lon1 = 0;
      GfxData::const_iterator firstPoint = polyBegin(p);
      if (closed()) {
         lat1 = getLastLat(p);
         lon1 = getLastLon(p);
      } else if ( getNbrCoordinates(p) == 1 ) {
         // A single point is a short line.
         lat1 = firstPoint->lat;
         lon1 = firstPoint->lon;
      } else {
         lat1 = firstPoint->lat;
         lon1 = firstPoint->lon;
         ++firstPoint;
         ++currentPointIndex;
      }

      coordinate_type delta_lat1 = lat - lat1;
      coordinate_type delta_lon1 = coordinate_type((lon - lon1) * bbcoslat);

      uint64 square1 = SQUARE(int64(delta_lat1)) + SQUARE(int64(delta_lon1));
      uint64 dist = square1;
      if(dist < minDist) {
         minDist = dist;
         minI = currentPointIndex;
         minPolygon = p;
      }

      GfxData::const_iterator end = polyEnd( p );
      for (GfxData::const_iterator j = firstPoint; j != end; ++j ) {
         coordinate_type lat2 = j->lat;
         coordinate_type lon2 = j->lon;
         coordinate_type delta_lat2 = lat - lat2;
         coordinate_type delta_lon2 = coordinate_type(
               (lon - lon2) * bbcoslat);

         uint64 square2 = SQUARE(int64(delta_lat2))
            + SQUARE(int64(delta_lon2));

         coordinate_type delta_latp = lat1 - lat2;
         coordinate_type delta_lonp = coordinate_type(
               (lon1 - lon2) * bbcoslat);

         uint64 squarep = SQUARE(int64(delta_latp))
            + SQUARE(int64(delta_lonp));

         if(square2 >= (square1 + squarep)) {
            ;
         } else if(square1 >= (square2 + squarep)) {
            dist = square2;
         } else {
            float64 nominator = float64(delta_lat1) * delta_lonp -
               float64(delta_lon1) * delta_latp;
            dist = uint64(SQUARE(nominator) / squarep);
         }

         lat1 = lat2;
         lon1 = lon2;
         delta_lat1 = delta_lat2;
         delta_lon1 = delta_lon2;
         square1 = square2;

         if(dist < minDist) {
            minDist = dist;
            minI = currentPointIndex;
            minPolygon = p;
         }

         ++currentPointIndex;
      }
   }

   //if(closestI != MAX_UINT32) {
   if(minI != MAX_UINT32) {
      // The outparameters does not handle when more than one polygon
      /*if (minPolygon > 1) {
         mc2log << fatal << here << " outparameters does not support "
                << "more than one polygon. Outparameters unset!" << endl;
         return uint64(minDist);
      }*/

      closestPolygon = minPolygon;
      closestI = minI;
      uint32 nextI = minI + 1;

      /*
      uint32 c = 0;
      for (uint16 p = 0; p<minPolygon; ++p) {
         for (uint32 j = 0; j<getNbrCoordinates(p); ++j) {
            c++;
         }
      }
      if (nextI == (c + getNbrCoordinates(minPolygon)))
         nextI = c;
      */

      if ( nextI == getNbrCoordinates(minPolygon) ){
         nextI = 0;
      }

      closestT = ((lat - getLat(minPolygon,minI)) *
                  (getLat(minPolygon,nextI) - getLat(minPolygon,minI)) +
                  (lon - getLon(minPolygon,minI)) *
                  (getLon(minPolygon,nextI) - getLon(minPolygon,minI))) /
                 (SQUARE(float64(getLat(minPolygon,nextI) -
                                 getLat(minPolygon,minI))) +
                  SQUARE(float64(getLon(minPolygon,nextI) -
                                 getLon(minPolygon,minI))));
      if(closestT < 0)
         closestT = 0;
      if(closestT > 1)
         closestT = 1;
   }

   return uint64(minDist);
}


uint64
GfxData::squareDistWithOffsetToLine(coordinate_type lat,
      coordinate_type lon, uint16& closestPolygon, uint32& closestI,
      float64& closestT, uint16 usePolygon) const
{
   uint64 dist_mc2 = squareDistWithOffsetToLine_mc2(lat, lon, closestPolygon,
                                                    closestI, closestT,
                                                    usePolygon);
   return uint64(dist_mc2 * GfxConstants::SQUARE_MC2SCALE_TO_SQUARE_METER);
};


uint64
GfxData::squareDistToLine_mc2(coordinate_type lat, coordinate_type lon,
      uint16 usePolygon) const
{
   uint16 closestPoly;
   uint32 closestCoord;
   float64 trash2;
   return squareDistWithOffsetToLine_mc2(lat, lon, closestPoly, closestCoord,
                                         trash2, usePolygon);
}

uint64
GfxData::squareDistToLine(coordinate_type lat, coordinate_type lon,
      uint16 usePolygon) const
{
   uint16 closestPoly;
   uint32 closestCoord;
   float64 trash2;
   return squareDistWithOffsetToLine(lat, lon, closestPoly, closestCoord,
                                         trash2, usePolygon);
}

uint64
GfxData::squareDistTo(coordinate_type lat, coordinate_type lon,
      uint16 usePolygon) const
{
   if(insidePolygon(lat, lon, usePolygon))
      return 0;

   return squareDistToLine(lat, lon, usePolygon);
}

uint64
GfxData::minSquareDistTo(const GfxData* otherGfx) const
{
   uint64 minDist = MAX_UINT64;
   for (uint16 p=0; (p < otherGfx->getNbrPolygons()) && ( minDist > 0);++p) {
      for (uint32 i=0; i<otherGfx->getNbrCoordinates(p); ++i) {
         uint64 curDist = squareDistTo(otherGfx->getLat(p, i),
                                       otherGfx->getLon(p, i));
         if (curDist < minDist)
            minDist = curDist;
         if (minDist == 0)
            return minDist;
      }
   }

   return minDist;
}

int64
GfxData::signedSquareDistTo(coordinate_type lat, coordinate_type lon,
      uint16 usePolygon) const
{
   int inside = insidePolygon(lat, lon, usePolygon);
   if(inside == 0)
      return squareDistToLine(lat, lon, usePolygon);

   if(inside == 1)
      return 0;

   return -squareDistToLine(lat, lon, usePolygon);
}

int64
GfxData::signedSquareDistTo_mc2(coordinate_type lat, coordinate_type lon,
      uint16 usePolygon) const
{
   int inside = insidePolygon(lat, lon, usePolygon);
   if(inside == 0)
      return squareDistToLine_mc2(lat, lon, usePolygon);
   
   if(inside == 1)
      return 0;

   return -squareDistToLine_mc2(lat, lon, usePolygon);
}


int
GfxData::getSegment(uint16 offset) const
{
   if(getNbrPolygons() != 1)
      return -1;

   /* else */
   uint32 nbrCoords = getNbrCoordinates(0);
   uint32 current = 0;
   float64 total = 0;
   float64 where = (getLength(0) / 0xffff) * offset;
   while(current < nbrCoords && total < where) {
      total += ::sqrt(GfxUtility::squareP2Pdistance_linear(
               getLat(0,current),
               getLon(0,current),
               getLat(0,current + 1),
               getLon(0,current + 1)));
      ++current;
   }
   return current;
}

float64
GfxData::getAngle(uint16 offset) const
{
   int segNbr = getSegment(offset);
   if(segNbr >= 0 && getNbrCoordinates(0) > 1) {
      if(segNbr > 0)
         segNbr--;
      return ((180.0/M_PI) * GfxUtility::getAngleFromNorth(
               getLat(0,segNbr),
               getLon(0,segNbr),
               getLat(0,segNbr + 1),
               getLon(0,segNbr + 1)));
   }

   /* else */
   return -1;
}

uint16
GfxData::getOffset(int32 lat, int32 lon,
                   int32& latOnPoly, int32 &lonOnPoly) const
{
   const uint32 polygonNbr = 0;
   const float64 cosLat = cos( (float64) (lat) * M_PI /
                               ((float64) 0x80000000) );
   mc2dbg4 << "cosLat = " << cosLat << ", nbrPolygons="
               << getNbrPolygons() << endl;

   // Set latOnPoly, lonOnPoly and calulate closestPointIndex
   uint64 minDist = MAX_UINT64;
   uint32 closestPointIndex = 0;
   uint32 i;
   for (i=0; i<getNbrCoordinates(polygonNbr)-1; i++) {
      int32 tmpLat, tmpLon;
      uint64 tmpDist =
         GfxUtility::closestDistVectorToPoint(
               getLon(polygonNbr,i), getLat(polygonNbr,i),
               getLon(polygonNbr,i+1), getLat(polygonNbr,i+1),
               lon, lat,
               tmpLon, tmpLat,
               cosLat);
      mc2dbg4 << "   tmpDist = " << tmpDist << "="
              << ::sqrt(tmpDist)*GfxConstants::MC2SCALE_TO_METER << " m" << endl
              << "   (" << tmpLat << "," << tmpLon <<")" << endl;

      if (tmpDist < minDist) {
         minDist = tmpDist;
         latOnPoly = tmpLat;
         lonOnPoly = tmpLon;
         closestPointIndex = i;
         mc2dbg8 << "      Udating minDist, i=" << i << " ("
                 << latOnPoly << "," << lonOnPoly <<")" << endl;
      }
   }
   mc2dbg4 << "   closestPointIndex = " << closestPointIndex
           << " = (" << getLat(polygonNbr,closestPointIndex) << ","
           << getLon(polygonNbr,closestPointIndex) << endl
           << "   (latOnPoly,lonOnPoly)=("
           << latOnPoly << "," << lonOnPoly << ")" << endl;

   // Caluclate the legnth to closestPointIndex
   float64 curLength = 0;
   for (i=0; i<closestPointIndex; i++) {
      mc2dbg8 << "      " << i << " curLength = " << curLength << endl;
      curLength += ::sqrt( GfxUtility::squareP2Pdistance_linear(
         getLat(polygonNbr,i), getLon(polygonNbr,i),
         getLat(polygonNbr,i+1), getLon(polygonNbr,i+1), cosLat));
   }
   mc2dbg8 << "   curLength = " << curLength << endl;

   // Calculate the length fomr last point to latOnPoly, lonOnPoly
   curLength += ::sqrt( GfxUtility::squareP2Pdistance_linear(
            getLat(polygonNbr,closestPointIndex),
            getLon(polygonNbr,closestPointIndex),
            latOnPoly, lonOnPoly , cosLat));


   // Calculate total length in this projection
   float64 totLength = 0;
   for (uint32 j=0; j<getNbrCoordinates(polygonNbr)-1; j++) {
      totLength += ::sqrt( GfxUtility::squareP2Pdistance_linear(
            getLat(polygonNbr,j), getLon(polygonNbr,j),
            getLat(polygonNbr,j+1), getLon(polygonNbr,j+1), cosLat));
   }
   mc2dbg8 << "   curLength = " << curLength << ", totLength="
           << totLength << ", getLength=" << getLength(polygonNbr)
           << ", ratio=" << (float64(curLength)/getLength(polygonNbr))
           << endl;

   // Calucluate the offset and return (if-statement due to possible
   // truncate-errors)
   uint16 offset = MAX_UINT16;
   //if (curLength < getLength(polygonNbr)) {
   if (curLength < totLength) {
      //offset = uint16( (float64(curLength) / getLength(polygonNbr))
      offset = uint16( (float64(curLength) / totLength)
                           * MAX_UINT16);
   }
   mc2dbg8 << "Returning offset = " << offset << endl;
   return (offset);
}


bool
GfxData::equals(uint16 thisPolygon, uint32 thisCoord,
                const GfxData* otherGfx,
                uint16 otherPolygon, uint32 otherCoord,
                uint16 factor) const
{
   if (  thisPolygon < getNbrPolygons() &&
         otherPolygon < otherGfx->getNbrPolygons() &&
         thisCoord < getNbrCoordinates(thisPolygon) &&
         otherCoord < otherGfx->getNbrCoordinates(otherPolygon)) {
      return ( (getLat(thisPolygon,thisCoord) / factor) ==
               (otherGfx->getLat(otherPolygon,otherCoord) / factor) ) &&
             ( (getLon(thisPolygon,thisCoord) / factor) ==
               (otherGfx->getLon(otherPolygon,otherCoord) / factor) );
   }
   else {
      return false;
   }
}

bool
GfxData::equals(const GfxData* otherData) const

{
   // For two polygons to be equal, two necessary conditions (and can be
   // checked fast) are that the bounding boxes coincide and that the
   // number of coordinates is the same. This assumes that all identical
   // coordinates have already been removed.
   //if (!(*(this->getMC2BoundingBox()) == *(otherData->getMC2BoundingBox())) ||
   if ((this->getNbrPolygons() != otherData->getNbrPolygons()) ||
       (this->getNbrCoordinates(0) != otherData->getNbrCoordinates(0)) )
      return false;
   else {
      // NB! Only checking first polygon!!!
      uint32 n = otherData->getNbrCoordinates(0);
      int32 x0 = this->getLon(0,0);
      int32 y0 = this->getLat(0,0);
      uint32 otherDataStartIndex = n;
      // find the index (if there is one) in otherData which has
      // the same coordinates as index 0 in the first polygon.
      for (uint32 i = 0; i < n; i++) {
         if ( (otherData->getLon(0,i) == x0) && (otherData->getLat(0,i) == y0))
            otherDataStartIndex = i;
      }
      if (otherDataStartIndex == n)
         return false;
      else {
         bool allCoordsEqual = true;
         uint32 polyIndex = 1;
         uint32 otherPolyIndex = (otherDataStartIndex + 1) % n;
         while ((allCoordsEqual) && (polyIndex < n)) {
            allCoordsEqual = ((this->getLon(0,polyIndex) ==
                               otherData->getLon(0,otherPolyIndex)) &&
                           (this->getLat(0,polyIndex) ==
                            otherData->getLat(0,otherPolyIndex)));
            polyIndex = polyIndex + 1;
            otherPolyIndex = (otherPolyIndex + 1)%n;
         }
         return allCoordsEqual;
      }
   }
}
// END equals

uint32
GfxData::getCoordinate(uint16 offset,
                       coordinate_type& lat,
                       coordinate_type& lon) const
{

   const uint32 nbrCoords = getNbrCoordinates( 0 );

   MC2_ASSERT( nbrCoords != 0 );

   if ( nbrCoords == 1 || offset == 0 ) {
      lat = getLat( 0, 0 );
      lon = getLon( 0, 0 );
      // The next index of the coordinate would be 1.
      return 1;
   }
   if ( offset == MAX_UINT16 &&
        getNbrPolygons() == 1 ) {
      lat = getLastLat( 0 );
      lon = getLastLon( 0 );
      return 1;
   }
   const float64 eps = 1e-6;

   // Get the distance from node 0
   float64 lengthPercent = static_cast<float64>( offset ) / 0xFFFF;
   float64 where = getLength( 0 ) * lengthPercent;

   if ( where < eps ) {
      lat = getLat( 0, 0 );
      lon = getLon( 0, 0 );
      // The next index of the coordinate would be 1.
      return 1;
   }
   if ( nbrCoords == 2 ) {

      uint32 segmentNbr = 0;

      int32 afterLat = getLat( 0, segmentNbr );
      int32 afterLon = getLon( 0, segmentNbr );

      lat = afterLat -
         static_cast<coordinate_type>
         ( ( afterLat - getLat( 0, segmentNbr + 1 ) ) * lengthPercent );
      lon = afterLon -
         static_cast<coordinate_type>
         ( ( afterLon - getLon( 0, segmentNbr + 1 ) ) * lengthPercent );

      return 1;
   }

   float64 total = 0;
   float64 lastDist = 0;
   uint32 segmentNbr = 0;

   while ( segmentNbr < nbrCoords - 1 && total < where ) {
      lastDist = ::sqrt(
            GfxUtility::squareP2Pdistance_linear(
               getLat( 0, segmentNbr ), getLon( 0,  segmentNbr ),
               getLat( 0, segmentNbr + 1 ), getLon( 0, segmentNbr + 1 )));
      total += lastDist;
      ++segmentNbr;
   }



   if ( segmentNbr == nbrCoords ) {
      mc2dbg << "[getCoordinate]: where = " << where
             << "getLength(0) = " << getLength(0) << endl;
   }

   // Now offset is between segmentNbr-1 and segmentNbr. And the
   // distances are like (X is at given offset):
   //
   // ( afterLat, afterLon )
   // segmentNbr-1                           segmentNbr
   //     +---------X-----------------------------+
   //                                           total
   //               |<------ total - where ------>|

   coordinate_type afterLat = getLat( 0, segmentNbr );
   coordinate_type afterLon = getLon( 0, segmentNbr );

   float64 relativeDistance = ( total - where ) / lastDist;

   lat = afterLat -
      static_cast<coordinate_type>
      ( ( afterLat - getLat( 0, segmentNbr - 1 ) ) * relativeDistance );
   lon = afterLon -
      static_cast<coordinate_type>
      ( ( afterLon - getLon( 0, segmentNbr - 1 ) ) * relativeDistance );

   return segmentNbr;
}


bool
GfxData::getBBox(coordinate_type& minLat, coordinate_type& maxLat,
                 coordinate_type& minLon, coordinate_type& maxLon) const
{
   minLat = getMinLat();
   maxLat = getMaxLat();
   minLon = getMinLon();
   maxLon = getMaxLon();
   return true;
}

bool
GfxData::getConvexHull(Stack* stack, uint16 polygon)
{
   // Brief description of the algorithm used (Grahams scan).
   //
   // (1) ===============================================================
   // First we find the coordinate that is furthest to the east
   // (minimum longitude). Call this the reference coordinate
   //
   // (2) ===============================================================
   // Sort all other coordinates by the angle to the reference coordinate.
   // The smallest angle is first, largest last. Break ties by choosing
   // the closest coordinate first.
   //
   // (3) ===============================================================
   // Add the last coordinate and the two first coordinates to the stack.
   //
   //
   // (4) ===============================================================
   // Then go through the sorted coordinates clockwise. If a coordinate
   // is making a strict right turn compared to the coordinates in the
   // stack, then add it to the stack. Otherwise pop the stack and
   // continue.
   
   // Do not use coslat on calculating the angles in this method.
   // The coslat varies with the coord-pairs and may lead to an incorrect
   // result including points into the convex hull that should not be there

   // Check stack
   if (stack == NULL) {
      mc2log << error << "GfxData::getConvexHull stack == NULL" << endl;
      return (false);
   }

   // Check gfxData
   if (getNbrCoordinates(polygon) < 3) {
      mc2log << error
             << "GfxData::getConvexHull too few coordinates in polygon"
             << endl;
      return (false);
   }

   // (1) ===============================================================
   // Find the coordinate furthest west and south (that is, minimum 
   // longitude and lattitude, lexicographically).
   uint32 minLonIdx = 0;
   int32 minLon = MAX_INT32;
   int32 minLat = MAX_INT32;
   for (uint32 i = 0; i < getNbrCoordinates(polygon); i++) {
      if (getLon(polygon, i) < minLon) {
         minLon = getLon(polygon, i);
         minLat = getLat(polygon, i);
         minLonIdx = i;
      } else if( getLon(polygon, i) == minLon) {
         if (getLat(polygon, i) < minLat) {
            minLon = getLon(polygon, i);
            minLat = getLat(polygon, i);
            minLonIdx = i;
         }
      }
   }

   // Coordinate pair of the coordinate furthest west and south. These 
   // are the reference coordinates.
   int32 refLon = minLon;
   int32 refLat = getLat(polygon, minLonIdx);

   // (2) ===============================================================
   // Sort the coordinates with respect to angle from reference coordinate
   // and break ties by choosing the smallest distance first.
   // Do not use coslat on calculating the angles.
   
   std::vector< ConvexHullNotice* > hullVec;
   hullVec.reserve( 100 );

   hullVec.push_back(new ConvexHullNotice(0, minLonIdx, 0));
   for (uint32 i = 0; i < getNbrCoordinates(polygon); i++) {
      int32 lat = getLat(polygon, i);
      int32 lon = getLon(polygon, i);
      // Don't include the reference point and points with the exact
      // same coordinates as the reference point
      if ((i != minLonIdx) && !(refLat == lat && refLon == lon)) {
         float64 angle =
            GfxUtility::getAngleFromNorthNoCoslat(refLat, refLon, lat, lon);
         float64 dist =
            GfxUtility::squareP2Pdistance_linear(refLat, refLon, lat, lon);

         // Add notice to hullVec
         hullVec.push_back( new ConvexHullNotice( angle, i, dist ) );
      }
   }

   // Sort
   std::sort( hullVec.begin(), hullVec.end(), STLUtility::RefLess() );

   uint32 size = hullVec.size();

   // (3) ===============================================================
   // Add the three first coordinates to the stack.
   ConvexHullNotice* preTopNotice = hullVec[size - 1];
   stack->push(preTopNotice->m_index);

   preTopNotice = hullVec[0];
   //stack->push(topNotice->m_index);
   stack->push(preTopNotice->m_index);

   uint32 i = 1;
   // Find the next notice that is not equal to preTopNotice.
   // This check is done since unique coordinates are not required.
   while ( (i < size) &&
           ( *hullVec[i] == *preTopNotice) ) {
      i++;
   }

   if (i == size)
      return (false);
   // else push that notice onto the stack.
   ConvexHullNotice* topNotice = hullVec[i];

   stack->push(topNotice->m_index);

   // Now the stack contains the first three coordinates of the convex hull.
   // Continue with checking which of the others that also belong there.
   // Do not use coslat on calculating the angles.
   // (4) ===============================================================
   i++;
   while (i < size) {
      
      float64 stackAngle = getAngleNoCoslat(
                     preTopNotice, topNotice, polygon);
      ConvexHullNotice* checkNotice = hullVec[i];

      // Since we don't require unique coordinates, must check that we
      // don't try to calculate the angle between
      // two identical coordinate pairs.
      if (*topNotice != *checkNotice) {

         float64 checkAngle = getAngleNoCoslat(
                     topNotice, checkNotice, polygon);

         // Check the angle. For the special case that preTopNotice
         // is equal to the last point in hullVec we have that we
         // always have a strict right turn since the reference point
         // was chosen to the furthest south west (lexicographically).
         if( checkAngle > stackAngle ||
             preTopNotice->m_index == hullVec[ size-1 ]->m_index ) {
            // Strict right turn. Push index to stack!
            stack->push(checkNotice->m_index);

            // Update the stack notices
            preTopNotice = topNotice;
            topNotice = checkNotice;

            // Increase index.
            i++;
         } else {
            // Remove the last index from the stack
            stack->pop();

            // Update the stack notices
            topNotice = preTopNotice;

            uint32 idx = stack->getFromTop(1);
            // Search for the ConvexHullNotice with that index.
            // Start at i - 1 and search backwards in the array.
            // Note that we also have to check the last element
            // in hullVec as this is the first one to be placed 
            // on stack.
            int32 j = i - 1;
            bool found = false;
            while ( (j >= -1) && (!found) ) {
               if( j >= 0 ) {
                  if ( hullVec[j]->m_index == idx ) {
                     found = true;
                  } else {
                     j--;
                  }
               } else { // j == -1
                  // We have to check the last element in hullVec also, as 
                  // this is the first one to be placed in the stack.
                  if ( hullVec[size-1]->m_index == idx ) {
                     found = true;
                     j = size - 1;
                  } else {
                     j--;
                  }
               }
            }
            /*
            while ( (j >= 0) && (!found) ) {
               if ( hullVec[j]->m_index == idx ) {
                  found = true;
               } else {
                  j--;
               }
            }
            */
            if (!found) {
               mc2log << warn << here << " failed to build convex hull"
                      << endl;

               return (false);
            }
            preTopNotice = hullVec[j];

            // Don't increase index.
         }
      } else {
         // Move on to the next coordinate..
         i++;
      }
   }

   // Free mem.
   STLUtility::deleteValues( hullVec );

   return (true);
}

static inline GfxDataFull*
createConvexHull( const GfxData* gfx, const set<int>& polygonSet )
{
   // This code has been moved from GfxTileFeatureMapProcessor
   // that's why it is static.
   mc2dbg8 << "[GfxData]: "
           <<"Entering convexhull stuff"
           << endl;
   // Create a GfxData with no map and the first polygon created.
   GfxDataFull* deleteGfxData = GfxData::createNewGfxData( NULL, true );

   for ( uint32 p = 0; p < gfx->getNbrPolygons(); ++p ) {
      if ( polygonSet.empty() || ( polygonSet.find(p) != polygonSet.end() ) ) {
         for ( uint32 i = 0; i < gfx->getNbrCoordinates(p); ++i ) {
            deleteGfxData->addCoordinate( gfx->getLat(p, i),
                                          gfx->getLon(p, i ) );
         }
      }
   }

   deleteGfxData->updateBBox();

   mc2dbg8 << "[GfxData]: createConvexHull "
           << deleteGfxData->getNbrCoordinates(0)
           <<" coordinates added" << endl;

   // Swap gfx-data if the convex hull succeded.
   GfxDataFull* retGfx = NULL;
   Stack deleteStack;
   if ( deleteGfxData->getConvexHull( &deleteStack, 0 ) ) {
      retGfx = GfxData::createNewGfxData( NULL, true );
      for ( int i = 0; i < int(deleteStack.getStackSize()); ++i ) {
         retGfx->addCoordinate(
            deleteGfxData->getLat( 0, deleteStack.getElementAt(i) ),
            deleteGfxData->getLon( 0, deleteStack.getElementAt(i) ) );
      }
      // Try adding the first coordinate last again. It may be needed
      // for the filters.
      retGfx->addCoordinate( retGfx->getLat( 0, 0 ),
                             retGfx->getLon( 0, 0 ) );
      retGfx->updateBBox();
      retGfx->setClosed( 0, true );
      retGfx->updateLength();
   }
   mc2dbg8 << "[GfxData]: Convex hull has been run "
           << " returning nbrCoordinates = "
           << ( retGfx ?  int(retGfx->getNbrCoordinates(0)) : -1 )
           << endl;

   delete deleteGfxData;

   return retGfx;
}

GfxData*
GfxData::createNewConvexHull() const
{
   // Use all polygons with the empty set
   return createConvexHull(this, set<int>());
}

GfxData*
GfxData::getConvexHull(const GfxData* pointSet)
{
  //GfxData* convexHullGfx = new GfxDataFull(true);
  GfxDataFull* convexHullGfx = GfxData::createNewGfxData(NULL, true);

  const uint16 p = 0;

  int32 minY = MAX_INT32;
  int32 maxY = 0;
  int32 latVal, taljare;
  uint32 maxPos = 0;
  uint32 minPos = 0;
  uint32 nextIndex, currIndex, insertPos, turn;
  uint32 n = pointSet->getNbrCoordinates(p);
  double currMaxCosAngle, maxCosAngle;

  // Find points with highest and lowest y-coordinate.
  for ( uint32 k = 0;  k < n; k++ ) {
     latVal = pointSet->getLat(p,k);
     if ( (int32)(latVal - minY) < 0 ) {
        minY = latVal;
        minPos = k;
     }
     if ( (int32)(latVal - maxY  > 0)) {
        maxY = latVal;
        maxPos = k;
     }
  }
  convexHullGfx->addCoordinate(minY,pointSet->getLon(p, minPos));
  currIndex = minPos;
  insertPos = maxPos;
  turn = 0;
  bool validCandidate;
  while ( insertPos != minPos) {
     nextIndex = currIndex;
     insertPos = nextIndex;
     maxCosAngle = -2;
     if ( currIndex == maxPos )
        turn = 1;
     for (uint32 j = 0; j < n - 1; j++) {
        nextIndex = nextIndex + 1;
        if ( nextIndex == n)
           nextIndex = 0;
        if ( turn == 0) {
           validCandidate = pointSet->getLat(p,nextIndex) -
              pointSet->getLat(p,currIndex) > 0;
           taljare = pointSet->getLon(p,maxPos) - pointSet->getLon(p,nextIndex);
        }
        else {
           validCandidate = pointSet->getLat(p,currIndex) -
              pointSet->getLat(p,nextIndex) > 0;
           taljare = pointSet->getLon(p,nextIndex) - pointSet->getLon(p,maxPos);
        }
        currMaxCosAngle = taljare/::sqrt(dist(0, currIndex, nextIndex));;
        if (( currMaxCosAngle > maxCosAngle ) && ( validCandidate )){
           maxCosAngle = currMaxCosAngle;
           insertPos = nextIndex;
        }
     }
     if ( insertPos != minPos)
        convexHullGfx->addCoordinate(pointSet->getLat(p,insertPos),
                                     pointSet->getLon(p,insertPos));
     currIndex = insertPos;
  }
  convexHullGfx->setClosed(0,true);
  return convexHullGfx;
}


float64
GfxData::getAngleNoCoslat(ConvexHullNotice* notice1,
                  ConvexHullNotice* notice2, uint16 polygon)
{
   int32 lat1, lon1, lat2, lon2;
   lat1 = getLat(polygon, notice1->m_index);
   lon1 = getLon(polygon, notice1->m_index);
   lat2 = getLat(polygon, notice2->m_index);
   lon2 = getLon(polygon, notice2->m_index);

   // Do not use coslat on calculating the angles.
   return (GfxUtility::getAngleFromNorthNoCoslat(lat1, lon1, lat2, lon2));
}

bool
GfxData::getSimplifiedPolygon(Stack* stack, uint16 polygon,
                              uint32 maxDist, uint32 minDist) const
{
   // Check outparameter
   if (stack == NULL) {
      mc2log << error << "GfxData::getSimplifiedPolygon stack == NULL" << endl;
      return (false);
   }

   // Fill the stack with data
   simplifyPoly( stack, polygon,
                 SQUARE(uint64(maxDist)), SQUARE(uint64(minDist)));

   return (true);
}


// = = = = = = = = = = = = = = = = = = = = Filtering stuff
bool
GfxData::simplifyPoly( Stack* newPoly, uint32 polyIndex,
                       int64 maxDist, int64 minDist ) const
{                                                        //Version 1!!!
	int step;
	uint32 startIndex, currentIndex, prevIndex;
   // Debug variable for going clockwise(-1) or counter clockwise(1)
	int8 direction = -1;

	uint32 maxIndex = getNbrCoordinates(polyIndex) - 1;
	startIndex = getPointOnConvexHull(polyIndex);

   int clockW = clockWise(polyIndex, startIndex);
	if ( clockW == 1 ) {
		step = -1*direction;
   } else if ( clockW == 0 ) {
      mc2dbg8 << "anti" << endl;
      step = 1*direction;
   } else {
      return false;
   }

	newPoly->push( startIndex );
	prevIndex = startIndex;
	currentIndex = stepIndex(step, prevIndex, maxIndex+1);
	newPoly->push( currentIndex );
	currentIndex = stepIndex(step, currentIndex, maxIndex+1);
   uint32 loops=0;
   GfxData::const_iterator beginPoly = polyBegin( polyIndex );
	while( newPoly->top( ) != startIndex && newPoly->top( ) != currentIndex) {
      loops=loops+1;
      int64 distance = dist(beginPoly, prevIndex, currentIndex);

		// Case: We have encountered a distance longer than maximum
      //       feature distance
		if ( distance > maxDist ) {
			if ( newPoly->top( ) != prevIndex ) {
				prevIndex = newPoly->top( );
				currentIndex = stepIndex(step, prevIndex, maxIndex+1);
			}
			else {
				newPoly->push( currentIndex );
				prevIndex = currentIndex;
				currentIndex = stepIndex(step, prevIndex, maxIndex+1);
			}
		}
		// Then we have the case of the two points being too close,
      // simply skip the point
		// This case often corresponds to two points being in the same
      // pixel on the screen
		else if ( distance <= minDist && newPoly->top( ) == prevIndex ) {
			newPoly->pop();
			newPoly->push( currentIndex );
			currentIndex = stepIndex(step, currentIndex, maxIndex+1);
		}
		// Finally checking the convex requirements if distance is about right
		else {
			// Calculating the cross product of the new point and the
         // last accepted point
			if ( newPoly->top( ) != prevIndex)
				if ( currentIndex == startIndex ){
					prevIndex = newPoly->top( );
					currentIndex = stepIndex( step, prevIndex, maxIndex+1 );
				}
				else {
               int32 x1 = beginPoly[newPoly->top()].lon -
                          beginPoly[prevIndex].lon;
               int32 y1 = beginPoly[newPoly->top()].lat -
                          beginPoly[prevIndex].lat;
               int32 x2 = beginPoly[currentIndex].lon -
                          beginPoly[prevIndex].lon;
               int32 y2 = beginPoly[currentIndex].lat -
                          beginPoly[prevIndex].lat;

					int64 crossprod = (((int64)x1)*y2 - ((int64)y1)*x2)*direction;
               // Keep hull convex by not cutting the polygon
					if ( crossprod > 0 ) {
						currentIndex = stepIndex(step, currentIndex, maxIndex+1);
					}
					else {
						// Otherwise we just move our positions forward and
                  // skip a point
						newPoly->pop();
						newPoly->push( currentIndex );
						currentIndex = stepIndex(step, currentIndex, maxIndex+1);
					}
				}
			// If we are dealing with two consecutive points we can just
         // accept and go on
			else {
				newPoly->push( currentIndex );
				currentIndex = stepIndex(step, currentIndex, maxIndex+1);
			}
		}
	}

	if ( currentIndex == newPoly->top() ) {
		while ( currentIndex != MAX_UINT32 )
			currentIndex = newPoly->pop();
   }
   else {
		newPoly->pop();
   }

   // Check if filtered polygon is simple.

   if ( newPoly->getStackSize() >= 5 )
      findBoundaryLoops2(newPoly, polyIndex);

   return true;
}

uint32
GfxData::getPointOnConvexHull(int polyIndex) const
{
   // Use iterators to avoid many virtual calls
   // (It is actually faster this way)
   const_iterator polyStart = polyBegin(polyIndex);
	int32 maxY = polyStart[0].lat;

   const_iterator polyStop  = polyEnd(polyIndex);
   const_iterator maxYIt = polyStart;
   for( const_iterator it = polyStart + 1; it != polyStop; ++it ) {
      if ( maxY < it->lat ) {
         maxY = it->lat;
         maxYIt = it;
      } else if ( maxY == it->lat ) {
         if ( it->lon > maxYIt->lon ) {
            maxYIt = it;
         }
      }
   }

   return maxYIt - polyStart;
}

int
GfxData::clockWise(int polyIndex, int startIndex) const
{

   uint32 hullIndex = getPointOnConvexHull(polyIndex);

   // Setting next index and previous index for later cross product
   uint32 nextIndex = hullIndex + 1;
   if( nextIndex == getNbrCoordinates(polyIndex)) {
      nextIndex = 0;
   }

   uint32 prevIndex;
   if( hullIndex == 0 )
      prevIndex = getNbrCoordinates(polyIndex) - 1;
   else
      prevIndex = hullIndex - 1;

   if ( prevIndex == hullIndex || prevIndex == nextIndex )
      return -1; // One or two point polygon

   // The sign in a cross product determines the orientation
   int32 x1 = getLon(polyIndex, prevIndex) - getLon(polyIndex, hullIndex);
   int32 y1 = getLat(polyIndex, prevIndex) - getLat(polyIndex, hullIndex);
   int32 x2 = getLon(polyIndex, nextIndex) - getLon(polyIndex, hullIndex);
   int32 y2 = getLat(polyIndex, nextIndex) - getLat(polyIndex, hullIndex);
   int64 crossprod = ((int64)x1)*y2 - ((int64)y1)*x2;

   if (crossprod > 0)
      return 1; // Clockwise
   else if (crossprod < 0)
      return 0; // Counterclockwise
   else
      return -1; // This polygon is corrupt

}

int
GfxData::clockWiseByArea(uint32 polyIndex) const
{
   double area = polygonArea(polyIndex);
   if ( area > 0 )
      return 1;          // Clockwise
   else if ( area < 0 )
      return 0;         // Counterclockwise
   else
       return -1; // This polygon is corrupt
}

int64
GfxData::dist( int32 polyIndex, int32 first, int32 second ) const
{
   return (int64) (
         GfxUtility::squareP2Pdistance_linear( getLat(polyIndex, first),
                                               getLon(polyIndex, first),
                                               getLat(polyIndex, second),
                                               getLon(polyIndex, second)) );
}

int64
GfxData::dist( const_iterator polyIter, int32 first, int32 second ) const
{
   return (int64) (
      GfxUtility::squareP2Pdistance_linear( polyIter[first].lat,
                                            polyIter[first].lon,
                                            polyIter[second].lat,
                                            polyIter[second].lon) );
}


double
GfxData::getCosRelativeAngle( int32 polyIndex, uint32 p ,
                              uint32 tip, uint32 q )
{

   float64 cosF = 0.56;
   float64 ux = cosF*(getLon(polyIndex, p) - getLon(polyIndex, tip));
   int32 uy = getLat(polyIndex, p ) - getLat(polyIndex, tip);
   float64 vx = cosF*(getLon(polyIndex, q) - getLon(polyIndex, tip));
   int32 vy = getLat(polyIndex, q) - getLat(polyIndex, tip);

   //Scalar product between vectors u (from "tip" to p) and v (from "tip" to q)
   float64 scalarProd  = ux*vx + uy*vy;
   float64 distProd = (dist(polyIndex, p, tip ))*
      (dist(polyIndex, q, tip ))*
      (SQUARE(GfxConstants::SQUARE_METER_TO_SQUARE_MC2SCALE));
   return (scalarProd/::sqrt(distProd));
}


int32
GfxData::orientation(uint32 polyIndex, uint32 x, uint32 y, uint32 z) const
{
   float64 cosFxy = cos((2*M_PI/ POW2_32 * ((getLat(polyIndex,x)/2)+(getLat(polyIndex,y)/2))));
   float64 cosFyz = cos((2*M_PI/ POW2_32 * ((getLat(polyIndex,y)/2)+(getLat(polyIndex,z)/2))));
   float64 xy1 = cosFxy*(getLon(polyIndex,y) - getLon(polyIndex,x));
   int32 xy2 = getLat(polyIndex,y) - getLat(polyIndex,x);
   float64 yz1 = cosFyz*(getLon(polyIndex,z) - getLon(polyIndex,y));
   int32 yz2 = getLat(polyIndex,z) - getLat(polyIndex,y);
   // z-component of cross product beteween vectors xy and yz
   float64 zComp = xy1*yz2-yz1*xy2;
   if (zComp > 0)
      return -1;
   else if (zComp < 0)
      return 1;
   else
      return 0;
}
int32
GfxData::orientation(int32 lat1, int32 lon1, int32 lat2, int32 lon2,
                     int32 lat3, int32 lon3) const
{
   float64 cosF12 = cos((2*M_PI/ POW2_32 * ((lat1/2)+(lat2/2))));
   float64 cosF23 = cos((2*M_PI/ POW2_32 * ((lat2/2)+(lat3/2))));
   float64 xy1 = cosF12*(lon2 - lon1);
   int32 xy2 = lat2 - lat1;
   float64 yz1 = cosF23*(lon3 - lon2);
   int32 yz2 = lat3 - lat2;
   // z-component of cross product beteween vectors xy and yz
   float64 zComp = xy1*yz2-yz1*xy2;
   if (zComp > 0)
      return -1;
   else if (zComp < 0)
      return 1;
   else
      return 0;
}

bool
GfxData::segmentIntersect( uint32 polyIndex,
                           uint32 p1, uint32 p2, uint32 q1, uint32 q2 ) const
{
   //test if (p1,p2,q1) and (p1,p2,q2) have different orientation:
   if (orientation(polyIndex, p1, p2, q1) *
       orientation(polyIndex, p1, p2, q2) < 0){
      // Different signs, i.e. different orientation
      // test (q1,q2,p1) and (q1,q2,p2).
      if (orientation(polyIndex, q1,q2,p1)*
          orientation(polyIndex, q1,q2,p2)<0)
         return true;
      else
         return false;
   }
   else
     return false;
}

double
GfxData::triangleArea(double d1, double d2, double d3) const
{
   if (( (d1 + d2) < d3 ) || ( (d2 + d3) < d1 ) || ( (d1 + d3) < d2 )) {
      mc2log << error << "GfxData::triangleArea Unbounded figure" << endl;
      return -1;
   }
   else {
      double p = ( d1 + d2 + d3 )/2;
      return ::sqrt( p*( p - d1)*( p - d2)*( p - d3));
   }
}

double
GfxData::polygonArea(uint32 polyIndex) const
{
   // gets the area of a polygon in MC2 units.
   uint32 n = getNbrCoordinates(polyIndex);
   if (n < 3) {
      mc2log << warn
             << "GfxData::polygonArea   GfxData has less than three nodes"
             << endl;
      return 0;
   }
   int32* xVector = new int32[n];
   int32* yVector = new int32[n];
   int32 x0 = getLon(polyIndex,0);
   int32 y0 = getLat(polyIndex,0);
   xVector[0] = 0;
   yVector[0] = 0;
   int64 sum = 0;
   for (uint32 i = 1; i < n; i++)
   {
      // shift the polygon by subtracting (x0,y0) => better numerics.
      xVector[i] = getLon(polyIndex,i) - x0;
      yVector[i] = getLat(polyIndex,i) - y0;
      sum = sum + int64(yVector[i-1])*xVector[i] -
         int64(xVector[i-1])*yVector[i];
   }
   int64 finalSum = sum + int64(yVector[n-1])*xVector[0] -
      int64(xVector[n-1])*yVector[0];
   if (finalSum < 0)
      mc2dbg4 <<
         "GfxData::polygonArea Coordinates are oriented anti-clockwise" <<endl;
   delete [] xVector;
   delete [] yVector;
   return finalSum/2;
}//END polygonArea


void
GfxData::findBoundaryLoops2(Stack* newPoly, uint32 polyIndex) const
{
   // Make sure that we have at least 4 coordinates
   if (newPoly->getStackSize() < 5) {
      return;
   }
   bool cross;
   uint32  prevInd,succInd, store, pos, stop;
   uint32 nbrCoord = newPoly->getStackSize();
   uint32* polyVec = new uint32[nbrCoord];
   for ( uint32 pos = 0; pos < nbrCoord; pos++ )
      polyVec[nbrCoord - 1 -pos] = newPoly->pop();
   uint32 currFirstPos = 0;
   uint32 currSecPos = 1;
   uint32 count = 0;
   for ( uint32 i = 0; i < nbrCoord - 1; i++) {
      currFirstPos = i;
      currSecPos = i + 1;
      prevInd = (i+1)%nbrCoord;
      succInd = (prevInd+1)%nbrCoord;
      cross = false;
      while (succInd != i) {
         prevInd = succInd;
         succInd = (succInd + 1) % nbrCoord;
         if ( segmentIntersect(polyIndex, polyVec[currFirstPos],
                               polyVec[currSecPos],polyVec[prevInd],
                               polyVec[succInd])) {
            mc2dbg8 << "%oegla" << endl << count << endl;
            // change order of inbetween points
            count = 0;
            pos = (currSecPos + 1) % nbrCoord;
            while (polyVec[pos] !=  polyVec[prevInd] ) {
               pos = (pos + 1) % nbrCoord;
               count = count + 1;
            }
            store = polyVec[currSecPos] ;
            polyVec[currSecPos] = polyVec[prevInd] ;
            polyVec[prevInd] = store;

            if ( count > 1) {
               pos = currSecPos;
               stop = prevInd;
               mc2dbg8 << "pos " << pos << endl << "stop " << stop << endl;
               for ( uint32 k = 1; k <= ( count / 2); k++ ) {
                  pos = ( pos + 1 ) % nbrCoord;
                  if ( stop > 0 )
                     stop = stop - 1;
                  else
                     stop = nbrCoord - 1;
                  store = polyVec[pos] ;
                  polyVec[pos] = polyVec[stop] ;
                  polyVec[stop] = store;
               }
            }
         }

      }
   }
   mc2dbg8 << "klar" << endl;
   for ( uint32 pos = 0; pos < nbrCoord; pos++ )
       newPoly->push(polyVec[pos]);
   delete [] polyVec;
}



double
GfxData::crossDistance(uint32 polyIndex, uint32 k, uint32 a, uint32 b)
{
   GfxData::const_iterator it = polyBegin(polyIndex);
   int32 x1 = ( it + a )->lon;
   int32 y1 = ( it + a )->lat;
   int32 x2 = ( it + b )->lon;
   int32 y2 = ( it + b )->lat;
   int32 x3 = ( it + k )->lon;
   int32 y3 = ( it + k )->lat;

   float64 cosLat = cos( (2*M_PI/ POW2_32 *
                         (y1/2+y2/2)));
   if ( x1 == x2 ) {
      double toSquare = cosLat*abs(x3-x1);
      return SQUARE(toSquare);
   } else {
      const double lineCoeff = (y2 - y1)/(cosLat*(x2 - x1));
      const double top = (lineCoeff*cosLat*(x3-x1)-(y3-y1));
      return SQUARE(top) / ( SQUARE( lineCoeff ) + 1 );
   }
}

double
GfxData::maxCrossDistance(uint32 polyIndex, uint32 a,  uint32 b)
{
   double maxDist = 0;
   double p = 0;
   for ( uint32 k=a+1; k<b; k++) {
     p = crossDistance(polyIndex, k, a, b);
     if ( p > maxDist )
        maxDist = p;
   }
   return maxDist;
}

uint32
GfxData::getBestIndexBetween( uint16 polyIndex, uint32 a, uint32 b )
{
   MC2_ASSERT( a <= b );

   uint32 bestIndex = b;
   float64 minTotalDist = MAX_FLOAT64;

   // Let k be all values between a and b.
   for ( uint32 k = a + 1; k < b; ++k ) {

      float64 currentDist = 0;

      // Between a and k.
      for ( uint32 i = a + 1; i < k; ++i ) {
         currentDist += crossDistance( polyIndex, i, a, k );
      }

      // Between k and b.
      for ( uint32 i = k + 1; i < b; ++i ) {
         currentDist += crossDistance( polyIndex, i, k, b );
      }

      // Check if the total distance is less than the minimum
      // total distance and we therefore found the current best index.
      if ( currentDist < minTotalDist ) {
         minTotalDist = currentDist;
         bestIndex = k;
      }
   }

   return bestIndex;
}


bool
GfxData::openPolygonFilter( Stack* newPoly, uint32 polyIndex,
                            uint32 pMaxLatDist, uint32 pMaxWayDist,
                            bool minimizeError,
                            uint32 startIndex,
                            uint32 endIndex )
{
   // Make sure that inparameters are OK
   if (newPoly == NULL) {
      mc2log << error << "GfxData::openPolygonFilter stack == NULL" << endl;
      return (false);
   }
   else if ( getNbrCoordinates(polyIndex) < 3 ) {
      mc2dbg2 << "GfxData::openPolygonFilter too few coordinates" << endl;
      for (uint32 i=0; i<getNbrCoordinates(polyIndex); i++) {
         newPoly->push(i);
      }
      return (true);
   }


   // Recalculate distances (from meters to SQUARE(mc2)
   uint64 maxSqMC2Dist = uint64( SQUARE(uint64(pMaxLatDist)) *
                         GfxConstants::SQUARE_METER_TO_SQUARE_MC2SCALE);
   uint64 maxSqMC2WayDist = uint64(SQUARE(uint64(pMaxWayDist)) *
                            GfxConstants::SQUARE_METER_TO_SQUARE_MC2SCALE);

   // "Normal" polygon to filter
   uint32 maxIndex = endIndex;
   if ( endIndex == MAX_UINT32 ) {
      maxIndex = getNbrCoordinates(polyIndex) - 1;
   }
   uint32 prevIndex = startIndex;
   newPoly->push( prevIndex );   // Add index 0 to make sure that is included
   //uint64 maxSqWayDist = SQUARE(uint64(maxWayDist));
   while ( prevIndex < maxIndex ) {
      uint32 currentIndex = prevIndex + 1;
      while (( currentIndex <= maxIndex)&&
             ( maxCrossDistance(polyIndex, prevIndex, currentIndex)
               <= maxSqMC2Dist ) &&
             uint64(dist(polyIndex,prevIndex,currentIndex))
             <= (maxSqMC2WayDist)) {
         ++currentIndex;
      }

      if ( minimizeError ) {
         // Select the point so that the crossdistance error is minimized.
         if ( currentIndex <= maxIndex ) {
            prevIndex =
               getBestIndexBetween( polyIndex, prevIndex, currentIndex );
         } else {
            prevIndex = maxIndex;
         }
      } else {
         // Don't minimize the error. Just choose the point before the
         // crossdistance error reached the threshold.
         prevIndex = currentIndex - 1;
      }

      newPoly->push( prevIndex );
   }// while

   // Test to make sure that the last index is included
   if (newPoly->top() != maxIndex) {
      newPoly->push(maxIndex);
      mc2dbg4 << "openPolygonFilter, Added maxIndex!!!" << endl;
   }

   return true;
}

namespace {
/**
 * The simple variant of Douglas-Peucker (without path hulls).
 * See http://en.wikipedia.org/wiki/Ramer-Douglas-Peucker_algorithm.
 *
 * This function performs the Douglas Peucker algorithm between startIndex
 * and lastIndex, the caller is however expected to add lastIndex to the
 * results (to simplify the recursive subdivision, we don't want the middle
 * point to be added twice).
 */
void douglasPeucker( vector<uint32>& results,
                     const vector<MC2Coordinate>& points,
                     int startIndex,
                     int lastIndex,
                     uint64 epsilonSquared ) {
   if ( lastIndex <= startIndex+1 ) { // base case, stop the recursion
      results.push_back( startIndex );
   }
   else {
      // find out of there is a point between startIndex and lastIndex
      // which is further than epsilon from the line between startIndex
      // and lastIndex.
      uint64 maxDist = 0;
      int worstIndex = 0;
      for ( int i = startIndex + 1; i < lastIndex; ++i ) {
         float64 cosLat = cos( (2*M_PI/ POW2_32 * points[i].lat));
         uint64 dist = 
            GfxUtility::closestDistVectorToPoint( points[startIndex].lon,
                                                  points[startIndex].lat,
                                                  points[lastIndex].lon,
                                                  points[lastIndex].lat,
                                                  points[i].lon,
                                                  points[i].lat,
                                                  cosLat );
         if ( dist > maxDist ) {
            worstIndex = i;
            maxDist = dist;
         }
      }

      if ( maxDist > epsilonSquared ) {
         // run the algorithm recursively on the stuff before and
         // after worstIndex
         douglasPeucker( results,
                         points,
                         startIndex,
                         worstIndex,
                         epsilonSquared );
         douglasPeucker( results,
                         points,
                         worstIndex,
                         lastIndex,
                         epsilonSquared );
      }
      else {
         // no more recursion needed
         results.push_back( startIndex );
      }
   }
}
}

bool
GfxData::douglasPeuckerPolygonFilter(Stack& newPoly, 
                                     uint32 polyIndex,
                                     uint32 epsilon ) const {

   if ( getNbrCoordinates( polyIndex ) < 3 ) {
      // The poly is just a point to start with
      return false;
   }

   // Keep the whole polygon if epsilon == 0
   if ( epsilon == 0 ) {
      for ( uint32 i = 0; i < getNbrCoordinates( polyIndex ); i++ ) {
         newPoly.push( i );
      }
      return true;
   }

   vector<uint32> results;
   vector<MC2Coordinate> points( polyBegin( polyIndex ), 
                                 polyEnd( polyIndex ) );

   uint64 epsilonMC2Scale = 
      static_cast<uint64>( epsilon * GfxConstants::METER_TO_MC2SCALE );

   // do the actual filtering...
   douglasPeucker( results,
                   points,
                   0,
                   points.size()-1,
                   SQUARE( epsilonMC2Scale ) );
   
   // douglasPeucker expects the caller to add the last coordinate
   results.push_back( points.size()-1 );

   // The resulting poly is just a point
   if ( results.size() < 3 ) {
      return false;
   }

   // Put the results in the stack
   for ( size_t i = 0; i < results.size(); ++i ) {
      newPoly.push( results[ i ] );
   }

   return true;
}

// ***********************************************
// *   Functions for triangulations of polygons  *
// ***********************************************

double
GfxData::distToTriangleSide(uint32 polyIndex, uint32 testIndex,
                            uint32 prevIndex, uint32 currIndex,
                            uint32 nextIndex)
   // determines if testIndex is within the triangle formed
   // by the other three parameters
{
   double distToLongsideMeasure;
   int8 o1 = orientation(polyIndex, prevIndex, nextIndex, testIndex);
   int8 o2 = orientation(polyIndex, nextIndex, currIndex, testIndex);
   int8 o3 = orientation(polyIndex, currIndex, prevIndex, testIndex);

   if (( o1 == o2 ) && ( o2 == o3 )) {
      double dist1 = ::sqrt(dist(polyIndex, prevIndex, testIndex));
      double dist2 = ::sqrt(dist(polyIndex, nextIndex, testIndex));
      double longDist = ::sqrt(dist(polyIndex, prevIndex, nextIndex));
      // Triangle area proportional to perpendicular distance
      distToLongsideMeasure = triangleArea(dist1,dist2,longDist);
      // Very thin triangle gives area result -1....due to the
      // approximate distance values between two points.
      if ( distToLongsideMeasure == -1)
         distToLongsideMeasure = 0.00000001;
   }
   else
      distToLongsideMeasure = -1;

   return distToLongsideMeasure ;
}

void
GfxData:: removePolygonVertex(int32 polyVec[],uint32 nbrCoordInVec,
                              uint32 VertexPos)

{
   for ( uint32 k = VertexPos; k < (nbrCoordInVec - 1); k++ )
      polyVec[k] =  polyVec[k+1];
   polyVec[nbrCoordInVec - 1] = -1;
   // nbrCoordInVec = nbrCoordInVec - 1 after calling this function
}

uint32
GfxData:: nbrCoordInSplittedPolygon(int32 polyVec[])
{
   uint32 pos = 0;
   uint32 count = 0;
   while (polyVec[pos] >= 0  ) {
      count = count + 1;
      pos = pos + 1;
   }
   return count;
}


void
GfxData::splitPolygon( int32 polyVec[], uint32 nbrCoordInPoly,
                       uint32 startIndexPos, uint32 stopIndexPos )
{
   uint32 n = nbrCoordInPoly;
   uint32 pos = startIndexPos + 1;
   int32 stopIndex = polyVec[stopIndexPos];

   if ( pos == n )
      pos = 0;
   while (  polyVec[pos] != stopIndex ) {
         removePolygonVertex( polyVec, n, pos );
          if ( pos == ( n - 1 ) )
             pos = 0;
          n = n - 1;
   }
}

bool
GfxData::triangulatePolygon( Stack* stack, uint32 polyIndex)

{
   // Description of the algorithm used.
   //
   // (1) ===============================================================
   //
   //  First, determine if the vertices are ordered clockwise or
   //  anti-clockwise. Then, the indices are stored in a vector
   //  such that an increasing vector position always corresponds to
   //  a step in the correct direction.
   //
   // (2) ===============================================================
   //
   //  Always study three consecutive indices the polygon vector. Call
   //  them P (previous), P (current) and N (next). Three cases are
   //  possible:
   //
   //         (2a) ======================================================
   //
   //         P,C and N form a concave corner (angle greater than 180
   //         degrees measured fron inside of polygon). In this case,
   //         just proceed one step forward with P,C and N until a
   //         situation with a convex corner appears instead.
   //
   //         (2b) ======================================================
   //
   //         P,C and N form a convex corner (angle less than 180
   //         degrees measured fron inside of polygon). If no other ver-
   //         tex in the polygon is contained in the triangle formed by
   //         P,C and N, store the triangle in a vector and remove vertex
   //         C from the polygon. Move C and N one step forward and repeat
   //         the procedure on the remaining polygon.
   //
   //         (2c) ======================================================
   //
   //         Most difficult case: one or several vertices in polygon are
   //         contained in the triangle formed by P,C and N. The polygon
   //         should now be splitted in two parts by "drawing a line"
   //         from C to one of the mentioned vertices such that the line
   //         does not intersect any segment in the polygon. The vertex
   //         with greatest perpendicular distance to the line between
   //         P and N is chosen. Repeat the procedure on the two new
   //         polygons.
   //
   // (3) ===============================================================
   //
   // End of algorithm: all polygons are triangles. These are stored in
   // a vector, first three elements correspond to the first triangle etc.

   // Check orientation of coordinates
   uint32  startIndex = getPointOnConvexHull(polyIndex);
   int32  convexOrientation;
   if ( clockWise(polyIndex, startIndex) == 1 )
		convexOrientation = 1; // three consecutive points neg. oriented
	else if ( clockWise(polyIndex, startIndex) == 0 )
	   convexOrientation = -1;
   else
      return false;

   uint32 nbrCoordInBigPoly = stack->getStackSize();
   uint32 nbrCoord = nbrCoordInBigPoly;
   // Known that (nbrCoord - 2) triangles will be created.
   uint32* triangleVec= new uint32[3*(nbrCoordInBigPoly - 2)];
   static const uint32 maxCol = 10;
   typedef int max_col_vect[maxCol+1];
   max_col_vect* polyVec = new max_col_vect[nbrCoordInBigPoly];
   int32* polyVec1 = new int[nbrCoordInBigPoly];
   int32* polyVec2 = new int[nbrCoordInBigPoly];
   uint32 x = 0;
   uint32 stopIndexPos=0;
   uint32 nextIndex;
   uint32 j,k,pos,prevIndex,currIndex,startIndexPos,
      nextIndexPos,testIndexPos,testIndex,storePos;
   // store original polygon in first column
	for ( pos = 0; pos < nbrCoordInBigPoly; pos++ ) {
      if( convexOrientation == 1 )
         storePos = (nbrCoordInBigPoly - 1) - pos;
      else
         storePos = pos;
      polyVec[storePos][0] = stack->pop();
   }
   for ( k=0; k<3*(nbrCoordInBigPoly - 2); k++)
      triangleVec[k]=0;
   uint32 currCol = 0;
   double maxDistMeasure;
   double distance = 0;
   uint32 col = currCol;
   bool done = false;

   // start looking for convex vertices in polyVec.
   while ( !done) {
      maxDistMeasure = -1;
      prevIndex =  polyVec[0][currCol];
      currIndex =  polyVec[1][currCol];
      nextIndex =  polyVec[2][currCol];
      startIndexPos = 1;
      nextIndexPos = 2;
      while (( maxDistMeasure < 0 ) && ( nbrCoord > 3)){
         // check all points in the polygon if they are located in
         // triangle (prevIndex, currIndex, nextIndex)
         if (orientation(polyIndex, prevIndex, currIndex, nextIndex)
             == convexOrientation){
            testIndexPos =  (nextIndexPos + 1) % nbrCoord;
            testIndex = polyVec[testIndexPos][currCol];
            startIndex = currIndex;
            if ( nextIndexPos > 0)
               startIndexPos = nextIndexPos - 1;
            else
               startIndexPos = nbrCoord - 1;

            while ( testIndex != prevIndex) {
               distance = distToTriangleSide( polyIndex, testIndex, prevIndex,
                                              currIndex, nextIndex);
               if ( distance > maxDistMeasure) {
                  maxDistMeasure = distance;
                  stopIndexPos = testIndexPos;
               }
               testIndexPos =  (testIndexPos + 1) % nbrCoord;
               testIndex = polyVec[testIndexPos][currCol];
            }
            if ( maxDistMeasure < 0 ){
               // no other point inside triangle, extract triangle
               // to triangleVec
               // and move forward
               for ( uint32 i=0; i < nbrCoordInBigPoly; i++)
                  polyVec1[i] = polyVec[i][currCol];

               removePolygonVertex( polyVec1, nbrCoord,startIndexPos);
               triangleVec[x] = prevIndex;
               triangleVec[x+1] = currIndex;
               triangleVec[x+2] = nextIndex;
               x=x+3;
               currIndex = nextIndex;
               if ( nextIndexPos == nbrCoord - 1)
                  nextIndexPos = 0;
               else if (nextIndexPos ==  0 )
                  nextIndexPos = 1;
               nbrCoord = nbrCoord - 1;
               for ( uint32 i=0; i < nbrCoordInBigPoly; i++)
                  polyVec[i][currCol] = polyVec1[i];
               nextIndex = polyVec[nextIndexPos][currCol];
            } // if
         }
         else
            // Concave vertex, move all positions forward.
            // In a polygon, there must be a convex vertex somewhere...
         {
            prevIndex = currIndex;
            currIndex = nextIndex;
            nextIndexPos = ( nextIndexPos + 1) % nbrCoord;
            nextIndex = polyVec[nextIndexPos][currCol];
         }
      } // while
      if ( nbrCoord == 3) {
         // only triangle left of the polygon, store it
         triangleVec[x] = prevIndex;
         triangleVec[x+1] = currIndex;
         triangleVec[x+2] = nextIndex;
         x=x+3;
         currCol = currCol + 1;
         done = ( currCol > col);
         for ( uint32 i=0; i < nbrCoordInBigPoly; i++)
            polyVec1[i] = polyVec[i][currCol];
         nbrCoord=nbrCoordInSplittedPolygon(polyVec1);
      }
      else
      {
         // split polygon
         for ( uint32 i=0; i < nbrCoordInBigPoly; i++) {
            polyVec1[i] = polyVec[i][currCol];
            polyVec2[i] = polyVec[i][currCol];
         }
         splitPolygon( polyVec1, nbrCoord, startIndexPos, stopIndexPos );
         splitPolygon( polyVec2, nbrCoord, stopIndexPos, startIndexPos );
         nbrCoord = nbrCoordInSplittedPolygon(polyVec1);
         for ( pos = (currCol + 1); pos <=col; pos++) {
            j = (currCol + 1) + col - pos;
            for (  k=0; k<nbrCoordInBigPoly; k++)
               polyVec[k][j+1] = polyVec[k][j];
         }
         // insert the new splitted polygons
         for ( k=0; k<nbrCoordInBigPoly; k++)
            polyVec[k][currCol] = polyVec1[k];
         for ( k=0; k<nbrCoordInBigPoly; k++)
            polyVec[k][currCol+1] = polyVec2[k];
         col = col + 1;
      } // else
   } // while (!done)
   for ( k=0; k<3*(nbrCoordInBigPoly - 2); k++)
      stack->push(triangleVec[3*(nbrCoordInBigPoly - 2) - k - 1]);

   delete [] triangleVec;
   delete [] polyVec;
   delete [] polyVec1;
   delete [] polyVec2;

   return true;
}

// ***************************************
// End of triangulation functions        *
// ***************************************


uint32
GfxData::getMemoryUsage() const
{
   static bool writtenErrorMessage = false;
   if (!writtenErrorMessage)
      mc2log << fatal << "GfxData::getMemoryUsage() NOT IMPLEMENTED" << endl;
   writtenErrorMessage = true;
   uint32 bbsize = sizeof(GfxData);
   //bbsize += m_polygons.size() * sizeof(polygon_type);
   //bbsize += getNbrCoordinates() * sizeof(point_type);
   return bbsize;
}

uint32
GfxData::getSizeInDataBuffer() const
{
   // Closed and nbr polygons.
   uint32 size = 4;
   // Each polygon contains length and nbr of coordinates.
   size += getNbrPolygons() * ( 4 + 4 );
   // The coordinates, lat and lon.
   size += getTotalNbrCoordinates() * ( 4 + 4 );

   return size;
}



float64
GfxData::calcPolyLength( uint32 polyIdx ) const
{
   MC2_ASSERT( polyIdx < getNbrPolygons() );

   float64 length = 0;

   if ( getNbrCoordinates( polyIdx ) < 2 ) {
      return length;
   }

   GfxData::const_iterator begin = polyBegin( polyIdx );
   GfxData::const_iterator cur = begin;
   GfxData::const_iterator end = polyEnd( polyIdx );
   GfxData::const_iterator prev = cur;
   ++cur;

   // Calculate the distances for unclosed
   for ( ; cur != end; ++cur, ++prev ) {
      length += ::sqrt(GfxUtility::squareP2Pdistance_linear( *prev, *cur ));
   }
   // Now add the distance back to the start if closed.
   if ( getClosed( polyIdx ) ) {
      // Prev shoud be the last coord since cur is end
      length += ::sqrt(GfxUtility::squareP2Pdistance_linear( *begin, *prev ) );
   }

   return length;
}

bool
GfxData::getMC2BoundingBox(MC2BoundingBox& bb, uint16 p) const
{
   bb.reset();

   uint32 startPoly = p;
   uint32 endPoly   = p + 1;
   if ( p == MAX_UINT16 ) {
      // All polygons
      startPoly = 0;
      endPoly   = getNbrPolygons();
   } else {
      MC2_ASSERT( p < getNbrPolygons() );
   }

   for ( uint32 poly = startPoly; poly != endPoly; ++poly ) {
      GfxData::const_iterator begin = polyBegin( poly );
      GfxData::const_iterator end   = polyEnd( poly );
      for ( GfxData::const_iterator it = begin; it != end; ++it ) {
         bb.update( it->lat, it->lon, false );
      }
      bb.updateCosLat();
   }
   return true;
}


void
GfxData::fillCoordMap( const GfxData* gfx, uint32 poly,
                       coordMap_t& gfxCoords,
                       uint32 polyIdxToStore )
{
   
   GfxData::const_iterator end = gfx->polyEnd( poly );
   uint32 coordIdx = 0;
   for ( GfxData::const_iterator git = gfx->polyBegin( poly );
         git != end; git++ ) {
      // Build id pair from gfxID, coordIdx
      pair<uint32,uint32> idxPair( polyIdxToStore, coordIdx );
      coordMap_t::iterator it = gfxCoords.find( *git );

      if ( it == gfxCoords.end() ) {
         idxSet_t idxSet;
         idxSet.insert( idxPair );
         gfxCoords.insert( make_pair( *git, idxSet ) );
      }
      else {
         idxSet_t& idxSet = (*it).second;
         idxSet.insert( idxPair );
      }
      coordIdx++;
   }
}




bool
GfxData::getMultiCoords(multimap< uint32,uint32 >&
                        selfTouchCoords) const 
{
   MC2_ASSERT(selfTouchCoords.size() == 0); // It does not make sence to have
   // this one filled already.   


   // Gfx data with only one polygon and less than 3 coords
   // cannot be self-touching (e.g. virtual 0-length ssi)
   if ( getNbrPolygons()==1 && getNbrCoordinates(0) <= 2 ) {
      return false;
   }
   
   // first:  poly index
   // second: coord index
   typedef set< pair<uint32,uint32> > idxSet_t;



   // Collect coordinates from all polygons
   //
   // Coordinate indexes sorted by coordinate value.
   typedef map< MC2Coordinate, idxSet_t > coordMap_t;
   coordMap_t allgfxCoords;
   for (uint16 p=0; p<getNbrPolygons(); p++) {
      fillCoordMap( this, p, allgfxCoords, p);
   }

   // Check which coordinates share a common coordinate value.
   for ( coordMap_t::const_iterator it = allgfxCoords.begin();
         it != allgfxCoords.end(); it++ ) {
      idxSet_t idxSet = it->second;
      if ( idxSet.size() >= 2 ) {
         for (idxSet_t::const_iterator idxIt = it->second.begin();
              idxIt != it->second.end(); ++idxIt ){
            selfTouchCoords.insert(make_pair(idxIt->first, idxIt->second));
         }
      }
   }
   return (selfTouchCoords.size() > 0);
} // getMultiCoords
