#ifndef INSIDEUTIL_H
#define INSIDEUTIL_H

/**
 *    Contains static method for determining if a point is inside a polygon.
 *    Copyright 2001, softSurfer (www.softsurfer.com)
 *    This code may be freely used and modified for any purpose
 *    providing that this copyright notice is included with it.
 *    SoftSurfer makes no warranty for this code, and cannot be held
 *    liable for any real or imagined damage resulting from its use.
 *    Users of this code must verify correctness for their application.
 *    http://softsurfer.com/Archive/algorithm_0103/algorithm_0103.htm
 */
class InsideUtil {
   public:
      /**
       *    Tests if a point is left/on/right of an infinite line.
       *    @param   p0 The first point of the infinite line.
       *    @param   p1 The second point of the infinte line.
       *    @param   p2 The point to check.
       *
       *    @return >0 for P2 left of the line through P0 and P1
       *            =0 for P2 on the line
       *            <0 for P2 right of the line.
       */
      template<class P>
      static int isLeft( const P& p0, const P& p1, const P& p2 ) 
      {
         double res =
            double( (double)GfxUtil::getCoordX( p1 ) - GfxUtil::getCoordX( p0 ) ) * 
                  ( (double)GfxUtil::getCoordY( p2 ) - GfxUtil::getCoordY( p0 ) ) -
            double( (double)GfxUtil::getCoordX( p2 ) - GfxUtil::getCoordX( p0 ) ) * 
                  ( (double)GfxUtil::getCoordY( p1 ) - GfxUtil::getCoordY( p0 ) );
       
         if ( res > 0.5 ) {
            return 1;
         } else if ( res < -0.5 ) {
            return -1;
         } else {
            return 0; 
         }
      }


      /**
       *    Contains static method for determining if a point is 
       *    inside a polygon. Use the other inside if you have an
       *    MC2Coordinate or MC2Point, since they have XYHelpers
       *    defined in their class definition.
       *  
       *    @param   begin    Iterator of points containing 
       *                      the beginning of the polygon.
       *    @param   end      End iterator of the polygon, 
       *                      i.e. one position after the last point.
       *    @param   p        The point to check.
       *    @param   helper   The coordinate helper class for the point. 
       *                      Usually P::XYHelper will work.
       *    @return  True if the point was inside the polygon, 
       *             false otherwise.
       */
      template<class POINT_ITERATOR, class P, class XY_HELPER>
      static bool inside( const POINT_ITERATOR& begin,
                          const POINT_ITERATOR& end, 
                          const P& p, 
                          const XY_HELPER& helper )
      {
         int wn = 0;    // the winding number counter
        
         // loop through all edges of the polygon
         POINT_ITERATOR it = begin;
         POINT_ITERATOR prevIt = it;
         ++it;
         while( it != end ) {
            // edge from V[i] to V[i+1]
            if ( helper.getY( *prevIt ) <= 
                 helper.getY( p ) ) {   // start y <= P.y
               if ( helper.getY( *it ) > 
                    helper.getY( p ) )  // an upward crossing
                  if (InsideUtil::isLeft( 
                           *prevIt, *it, p ) > 0)// P left of edge
                     ++wn;              // have a valid up intersect
            } else {                    // start y > P.y (no test needed)
               if ( helper.getY( *it ) <= 
                    helper.getY( p ) )       // a downward crossing
                  if (InsideUtil::isLeft( 
                           *prevIt, *it, p ) < 0)// P right of edge
               --wn;                        // have a valid down intersect
            }
            prevIt = it;
            ++it;
         }
         // Odd winding number means it's inside.
         return wn & 1;
      }
      
      /**
       *    Contains static method for determining if a point is 
       *    inside a polygon. Uses the default XYHelper of the
       *    coordinate class sent in, for custom XYHelper, see
       *    the other inside function.
       *    <br />
       *    Example:
       *    const vector<MC2Coordinate>& vectorOfCoordinates = something();
       *    InsideUtil::inside(vectorOfCoordinates.begin(),
       *                       vectorOfCoordinates.end(),
       *                       MC2Coordinate(21123,123213) );
       *
       *    @param   begin    Iterator of points containing 
       *                      the beginning of the polygon.
       *    @param   end      End iterator of the polygon, 
       *                      i.e. one position after the last point.
       *    @param   p        The point to check.
       *    @return  True if the point was inside the polygon, 
       *             false otherwise.
       */
      template<class POINT_ITERATOR, class P>
      static bool inside( const POINT_ITERATOR& begin,
                          const POINT_ITERATOR& end, 
                          const P& p ) {
         typename P::XYHelper helper;
         return inside(begin, end, p, helper);
      }

};


#endif

