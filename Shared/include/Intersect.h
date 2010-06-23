/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INTERSECTS_H
#define INTERSECTS_H

#include "config.h"
#include "MC2Point.h"
#include "ClockWise.h"
#include "InsideUtil.h"
#include "GnuPlotDump.h"

#include <math.h>

namespace GfxUtil {

/**
 *    Calculates the angle between the three points.
 *    The resulting angle is between 0 and 360 degrees,
 *    and increases counter clockwise.
 */
template<class POINTLIKE>   
float calcAngle( const POINTLIKE& pl0,
                 const POINTLIKE& pl1,
                 const POINTLIKE& pl2 ) 
{
   
   MC2Point p0 = getPoint( pl0 );
   MC2Point p1 = getPoint( pl1 );
   MC2Point p2 = getPoint( pl2 );

   int isLeft = InsideUtil::isLeft( p0, p1, p2 );

   // Make two vectors from the points.
   MC2Point v0 = p0 - p1;
   MC2Point v1 = p2 - p1;

   if ( isLeft == 0 ) {
      // Neither left or right, means 180 or 0 degrees.
      if ( v0.dot64( v1 ) < 0 ) {
         // Opposing vectors => negative dot product => 180 degrees.
         return 180.0;
      } else {
         return 0.0;
      }
   }

   // Get the cosangle from dot product formula.
   float cosangle = v0.dot64( v1 ) / 
      sqrt( double(v0.dot64( v0 ) ) * v1.dot64( v1 ) );

   // Calculate the abs value of the angle (can max be 180).
   float absangle = float( acos( cosangle ) / M_PI * 180 );

   // The resulting angle shall increase ccw.
   float angle = absangle;
   if ( isLeft > 0 ) {
      angle = 360 - angle;
   }

   return angle;
}
   
/**
 *    Check if the closed polygon is self intersecting in such a 
 *    way that the orientation of the polygon is not constant, i.e.
 *    it is twisted.
 *    Does not work for polylines.
 *    @return  If the polygon is self intersecting.
 */
template<class ITERATOR>
bool selfIntersecting( const ITERATOR& begin,
                       const ITERATOR& end )
{
   // Only works for polygons known to be closed, i.e. not polylines or so.
   
   ITERATOR firstIt = begin;
   if ( firstIt == end ) {
      // There were too few
     return false;
   }
   ITERATOR middleIt = firstIt;
   ++middleIt;
   if ( middleIt == end ) {
      // There were too few
     return false;
   }
   ITERATOR nextIt = middleIt;
   ++nextIt;
   if ( nextIt == end ) {
      // There were too few
     return false;
   }
   float angleSum = 0;
   int nbrCorners = 0;
   int clockwiseRes = isClockWise( begin, end );
   if ( clockwiseRes < 0 ) {
      mc2dbg << "isClockWise failed" << endl;
   }
   bool wasClockwise = clockwiseRes;
   for ( ; middleIt != end; ++firstIt, ++middleIt, ++nextIt ) {
      if ( nextIt == end ) {
         if ( *middleIt != *begin ) {
            // Polygon not manually closed.
            return false;
         }
         nextIt = begin;
         ++nextIt;
      }

      // XXX: This hopefully works also when calcAngle returns 0?
      float angle = calcAngle( *firstIt, *middleIt, *nextIt );

      ++nbrCorners;

      if ( ! wasClockwise ) {
         // Exterior angle.
         mc2dbg8 << "Exterior angle." << endl;
         angle = (360 - angle );
      } else {
         // Interior angle.
         mc2dbg8 << "Interior angle." << endl;
      }
      
      angleSum += angle;

      mc2dbg8 << ", angle = " << angle << endl;
   }
   int expectedAngle = (nbrCorners - 2) * 180;
   mc2dbg8 << "checkSelfIntersect: corners = " << nbrCorners 
          << ", expectedAngle = " << expectedAngle
          << ", angleSum = " << angleSum 
          << ", wasClockwise = " << wasClockwise << endl;

   bool selfIntersect = ! (fabs(angleSum - expectedAngle) < 90);
   if ( selfIntersect ) {
      mc2dbg8 << "GfxUtil::selfIntersecting returns true. Corners = " << nbrCorners 
             << ", expectedAngle = " << expectedAngle
             << ", angleSum = " << angleSum 
             << ", wasClockwise = " << wasClockwise << endl;
#if 0
      cout << "EOF" << endl;
      cout << "cat > before.gnu << EOF" << endl;
      cout << GnuPlotDump::gp_dump( begin, end ) << endl;
      cout << "EOF" << endl;
      cout << "gnuplot" << endl;
      cout << "plot \"before.gnu\" with linespoints";
      cout << endl << flush;
#endif
   }
   return selfIntersect;

}   
/**
 *    Calls GfxUtility::getIntersection
 */
bool getIntersection(int32 x1, int32 y1, int32 x2, int32 y2,
                     int32 v1, int32 w1, int32 v2, int32 w2,
                     int32& intersectX, int32& intersectY,
                     float64& percentFromx1y1,
                     float64& percentFromv1w1);

/**
 *   Returns true if a polygon is self-intersecting.
 *   Complexity is O(n^2)
 */
template<class ITERATOR>
bool selfIntersectingSlowAndNotWorking( const ITERATOR& begin,
                                        const ITERATOR& end )
{
   // The iterator copying can probably be reduced, but
   // since the complexity is N^2
   for ( ITERATOR it = begin; it != end; ++it ) {
      ITERATOR nextIt = it;
      ++nextIt;
      if ( nextIt == end ) {
         // Done
         break;
      }
      for ( ITERATOR jt = nextIt; jt != end; ++jt ) {
         if ( jt == nextIt ) {
            continue;
         }
         ITERATOR nextJt = jt;
         ++nextJt;
         if ( nextJt == end ) {
            break;
         }
         if ( it == begin ) {
            ITERATOR nextNextJt = nextJt;
            ++nextNextJt;
            if ( nextNextJt == end && (*nextJt) == (*begin) ) {
               // Last coord same as first and "it" is at start               
               break;
            }
         }
         int32 intersectX;
         int32 intersectY;
         float64 alpha1;
         float64 alpha2;
         //MC2_ASSERT( *it != *jt );
         //MC2_ASSERT( *nextIt != *nextJt );
         //MC2_ASSERT( *it != *nextJt );
         //MC2_ASSERT( *jt != *nextIt );
         if ( getIntersection( getCoordX(*it),
                               getCoordY(*it),
                               getCoordX(*nextIt),
                               getCoordY(*nextIt),
                               getCoordX(*jt),
                               getCoordY(*jt),
                               getCoordX(*nextJt),
                               getCoordY(*nextJt),
                               intersectX,
                               intersectY,
                               alpha1,
                               alpha2 ) ) {
            mc2dbg << "[Intersect]: intersection at ("
                   << intersectY << "," << intersectX << ")"
                   << " alpha1 = " << alpha1 
                   << " alpha2 = " << alpha2 
                   << endl;
            return true;
         }
      }
   }
   return false;
}


}

#endif
