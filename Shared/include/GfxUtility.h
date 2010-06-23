/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFXUTILITY_H
#define GFXUTILITY_H

#include "config.h"
#include "Point.h"
#include "MC2Exception.h"

#include <vector>
#include <list>

class MC2Coordinate;
class LanguageCode;

/**
 *    Define the MC2BoundingBox for the getAdjacentRectangle-method and
 *    the isSegmentInMC2BoundingBox-method.
 *    MC2BoundingBox.h is included in the cpp-file.
 */
class MC2BoundingBox;


/**
 *    This static class contains a lot of utility-methods about 
 *    geographical data.
 *    
 *    @see     Utility.h Non-geographic utility-methods.
 */    
class GfxUtility {
public:
      /**
       *    Get the distance between two points.
       *    @return  The squared linear distance between two points.
       *             In meters squared.
       */
      static float64 squareP2Pdistance_linear(int32 lat1, int32 lon1, 
                                              int32 lat2, int32 lon2,
                                              float64 cosLat = 2.0);

      /**
       *    Get the distance between two points.
       *    @return  The squared linear distance between two points.
       *             In meters squared.
       */
      static float64 squareP2Pdistance_linear( const MC2Coordinate& p1,
                                               const MC2Coordinate& p2,
                                               float64 cosLat = 2.0 );

      /**
       *    Get the angle from the north-direction. Used to calculate 
       *    the angle described by the figure:
       *    @verbatim
                      (lat2, lon2)
                  |     /
                  |    /
                  |   /
                  |A /
                  |\/
                  |/ (lat1, lon1)
                  |
                  |
                  |
            @endverbatim
       *
       *    @param   lat1  The latitude of the startpoint for the line.   
       *    @param   lon1  The longitude of the startpoint for the line.   
       *    @param   lat2  The latitude of the endpoint for the line.   
       *    @param   lon2  The longitude of the endpoint for the line.   
       *    @return  Angle in radians from the north-direction in 
       *             clockwise direction. The return value is in the 
       *             interval $[0,2\pi[$.
       */
      static float64 getAngleFromNorth( int32 lat1, int32 lon1, 
                                        int32 lat2, int32 lon2,
                                        bool useCosLat = true);
      /// see the other getAngleFromNorth function.
      static float64 getAngleFromNorth( const MC2Coordinate& first,
                                        const MC2Coordinate& second );
      /// see the other getAngleFromNorth function.
      static float64 getAngleFromNorthNoCoslat( 
               int32 lat1, int32 lon1, int32 lat2, int32 lon2);

   /**
    * Makes sure an angle is in the interval [0, 2*pi[
    *
    * @param angle An angle, any real number.
    * @return a number in [0, 2*pi[
    */
   static double normalizeAngle( double angle );

   /**
    * Calculates the difference between two angles, using the shortest
    * path around the unit circle.
    */
   static double angleDifference( double angle1, double angle2 );

      /**
       *    Convert a string containing WGS 84 latitude or longitude to 
       *    an MC2 integer coordinate. The input string would look like 
       *    (N/S/E/W) D(D*)MMSS[.ddd], that is the period and the 
       *    following numbers are optional.
       *    @param   s  The null terminated lat or lon string in the 
       *                format specified by WGS 84.
       *    @return  An integer representing the position in the MC2 
       *             coordinate system.
       */
      static int32 convLatLonStringToInt(const char* s);

      /**
       *    Convert a string containing a SWEREF93 latitude or longitude to 
       *    a SWEREF93 radians coordinate. The input string would look like 
       *    D(D*).ddd(N/S/E/W).
       *    @param   s  The null terminated lat or lon string in the 
       *                format specified by WGS 84.
       *    @return  An float64 in SWEREF93 radians.
       */
      static float64 convLatLonStringToDbl(const char* s);


      /**
       *    Calculate the closest distance from a vector (defined by 
       *    (x1,y1) and (x2,y2) ) to a point (xpos, ypos).
       *    @param x1 The x-part of the first coordinate of the vector.
       *    @param y1 The y-part of the first coordinate of the vector.
       *    @param x2 The x-part of the second coordinate of the vector.
       *    @param y2 The y-part of the second coordinate of the vector.
       *    @param xpos The x-part of the point to calculate distance to.
       *    @param ypos The y-part of the point to calculate distance to.
       *    @param cosF The cosine factor.
       *    @return The minimum distance to (xpos, ypos) from vector.
       */
      static uint64 closestDistVectorToPoint( int32 x1,   int32 y1,
                                              int32 x2,   int32 y2,
                                              int32 xpos, int32 ypos,
                                              double cosF );
      /**
       *    Calculate the closest distance from a vector (defined by 
       *    (x1,y1) and (x2,y2) ) to a point (xpos, ypos) and also the 
       *    point, on the line between (x1,y1) and (x2,y2), which is 
       *    closest to (xpos, ypos).
       *
       *    @param x1   The x-part of the first coordinate of the vector.
       *    @param y1   The y-part of the first coordinate of the vector.
       *    @param x2   The x-part of the second coordinate of the vector.
       *    @param y2   The y-part of the second coordinate of the vector.
       *    @param xpos The x-part of the point to calculate distance to.
       *    @param ypos The y-part of the point to calculate distance to.
       *    @param xOut Outparameter that is set to the x-part of the
       *                coordinate that are closest to (xpos, ypos)
       *    @param xOut Outparameter that is set to the y-part of the
       *                coordinate that are closest to (xpos, ypos)
       *    @param cosF The cosine factor.
       *    @return The minimum distance to (xpos, ypos) from vector
       */
      static uint64 closestDistVectorToPoint( int32 x1,   int32 y1,
                                              int32 x2,   int32 y2,
                                              int32 xpos, int32 ypos,
                                              int32& xOut, int32& yOut,
                                              double cosF );

      /**
       *    @name Get point on line
       */
      //@{
         /**
          *    Get a point located @c dist meters from <tt>(lat1,lon1)</tt> 
          *    and on the line from <tt>(lat1,lon1)</tt> to 
          *    <tt>(lat2,lon2)</tt>. Will return false if the distance from 
          *    <tt>(lat1,lon1)</tt> to <tt>(lat2,lon2)</tt> is less than 
          *    @c dist.
          *
          *    @param lat1    Latitude part of the first point on the line.
          *    @param lon1    Longitude part of the first point on the line.
          *    @param lat2    Latitude part of the second point on the line.
          *    @param lon2    Longitude part of the second point on the line.
          *    @param dist    The distance from <tt>(lat1,lon1)</tt> of the
          *                   returned coordinate.
          *    @param outLat  Outparameter that is set to the coordinate 
          *                   described above.
          *    @param outLon  Outparameter that is set to the coordinate 
          *                   described above.         
          *    @return True if the outparameters are updated, false otherwise.
          */
         static bool getPointOnLine(int32 lat1, int32 lon1, 
                                    int32 lat2, int32 lon2, float64 dist,
                                    int32& outLat, int32& outLon);

         /**
          *    Get a point located @c dist meters from <tt>(lat1,lon1)</tt> 
          *    and on the line from <tt>(lat1,lon1)</tt> to 
          *    <tt>(lat2,lon2)</tt>. Will return false if the distance from 
          *    <tt>(lat1,lon1)</tt> to <tt>(lat2,lon2)</tt> is less than 
          *    @c dist.
          *
          *    Use this method if the distance between <tt>(lat1,lon1)</tt>
          *    and <tt>(lat2,lon2)</tt> already is calculated.
          *
          *    @param lat1    Latitude part of the first point on the line.
          *    @param lon1    Longitude part of the first point on the line.
          *    @param lat2    Latitude part of the second point on the line.
          *    @param lon2    Longitude part of the second point on the line.
          *    @param dist    The distance from <tt>(lat1,lon1)</tt> of the
          *                   returned coordinate.
          *    @param outLat  Outparameter that is set to the coordinate 
          *                   described above.
          *    @param outLon  Outparameter that is set to the coordinate 
          *                   described above.         
          *    @return True if the outparameters are updated, false otherwise.
          */
         static bool getPointOnLine(int32 lat1, int32 lon1, 
                                    int32 lat2, int32 lon2, 
                                    float64 dist, float64 totDist,
                                    int32& outLat, int32& outLon);
      //@}


      
      /**
       *    The types of text representation of a lat or lon in printLatLon.
       */
      enum coordianteRepresentations {
         /** simpleRad, 13.1911N, precision is number decimals.
           * The coordinate is expressed in decimal degrees with a final 
           * letter for compass direction (N|S|W|E or N|S|V|Ö ...). */
         simpleRad = 0,
         /** wgs84 style, N 13° 19' 12.96", precision is number decimals.
           * The coordinate is expressed as: compass direction degreesï¿½ 
           * minutes' seconds". */
         wgs84Style = 1,
         /** wgs84Rad 
           * The coordinate is expressed in radians. */
         wgs84Rad = 2,
         /** wgs84Deg
           * The coordinate is expressed in decimal degrees. */
         wgs84Deg = 3,

         /**
          * NMEA. 
          */
         nmea = 4,
      };

      /**
       * Prints a sweref93_LLA lat or lon coordinate (expressed in radians) 
       * into a string in different formats.
       * 
       * @param str        The string to print into.
       * @param latOrLon   The latitude or longitude in sweref93_LLA (radians).
       * @param lang       The language to print compassdirection in.
       * @param lat        If the latOrLon is a latitude, false if longitude.
       * @param precision  Valid numbers of decimals.
       * @param representation In which format should the coordinate be 
       *                       printed.
       * 
       * @return Number of chars written to str.
       */
      static uint32 printLatLon( char* str, 
                                 float64 latOrLon,
                                 bool lat,
                                 LanguageCode lang,
                                 uint32 precision = 3,
                                 coordianteRepresentations representation =
                                 simpleRad );
   /** Same as the printLatLon, but returns two strings directly for 
    * lat and lon
    * @see printLatLon
    */
   static void printLatLon( MC2String& latStr, MC2String& lonStr,
                            const MC2Coordinate& coord,
                            const LanguageCode& language,
                            coordianteRepresentations representation =
                            simpleRad );

      /**
       * Same as printLatLon but with default values for precision depending
       * on representation.
       * 
       * @param latOrLon The MC2 coordinate.
       * @see printLatLon.
       */
      static uint32 printLatLon( char* str, 
                                 int32 latOrLon,
                                 bool lat,
                                 const LanguageCode& lang,
                                 coordianteRepresentations representation );

      
      /**
       *    Find the intersection between two lines that can be described
       *    by the following coordinates:
       *    Line 1: (x1, y1) - (x2, y2) and \
       *    Line 2: (v1, w1) - (v2, w2).
       *    @param x1   x1 coordinate of line 1.
       *    @param y1   y1 coordinate of line 1.
       *    @param x2   x2 coordinate of line 1.
       *    @param y2   y2 coordinate of line 1.
       *
       *    @param v1   v1 coordinate of line 2.
       *    @param w1   w1 coordinate of line 2.
       *    @param v2   v2 coordinate of line 2.
       *    @param w2   w2 coordinate of line 2.
       *
       *    @param intersectX Output parameter set to the x coordinate
       *                      of the intersection of the two lines.
       *    @param intersectY Output parameter set to the y coordinate
       *                      of the intersection of the two lines.
       *    @return  True if the two lines intersects each other,
       *             false otherwise.
       */
      static bool getIntersection(int32 x1, int32 y1, int32 x2, int32 y2,
                                  int32 v1, int32 w1, int32 v2, int32 w2,
                                  int32& intersectX, int32& intersectY);

      /**
       *    Find the intersection between two lines that can be described
       *    by the following coordinates:
       *    Line 1: (x1, y1) - (x2, y2) and \
       *    Line 2: (v1, w1) - (v2, w2).
       *    @param x1   x1 coordinate of line 1.
       *    @param y1   y1 coordinate of line 1.
       *    @param x2   x2 coordinate of line 1.
       *    @param y2   y2 coordinate of line 1.
       *
       *    @param v1   v1 coordinate of line 2.
       *    @param w1   w1 coordinate of line 2.
       *    @param v2   v2 coordinate of line 2.
       *    @param w2   w2 coordinate of line 2.
       *
       *    @param intersectX Output parameter set to the x coordinate
       *                      of the intersection of the two lines.
       *    @param intersectY Output parameter set to the y coordinate
       *                      of the intersection of the two lines.
       *    @param percentFromx1y1 [Output] How many percent between 
       *                           (x1,y1) and (x2,y2) 
       *                           the intersection lies. 
       *    @param percentFromv1w1 [Output] How many percent between 
       *                           (v1,w1) and (v2,w2) 
       *                           the intersection lies. 
       *    @return  True if the two lines intersects each other,
       *             false otherwise.
       */
      static bool getIntersection(int32 x1, int32 y1, int32 x2, int32 y2,
                                  int32 v1, int32 w1, int32 v2, int32 w2,
                                  int32& intersectX, int32& intersectY,
                                  float64& percentFromx1y1,
                                  float64& percentFromv1w1 );


      /**
       * Calculates the cosine factor using a center for a given latitude
       * range.
       *
       * @param minLat The lower latitude value.
       * @param maxLat The higher latitude value.
       * @return The coslat factor.
       */
      static float64 getCoslat( int32 minLat, int32 maxLat );

      /**
       *  Determines if a given point (x,y) is inside a given
       *  rectangle or not. NOTE: if point is on rectangular
       *  boundary, it is counted as INSIDE.
       *  
       *  @param minX x-coordinate of the left  veritical edge in rectangle
       *  @param maxX x-coordinate of the right veritical edge in rectangle
       *  @param minY y-coordinate of the lower horisontal edge in rectangle
       *  @param maxY y-coordinate of the upper horisontaledge in rectangle
       *  @return true if point is inside rectangle, false otherwise
       *
       */
      static bool pointInsideRect( int32 x, int32 y, int32 minX, int32 maxX,
                                   int32 minY, int32 maxY );

      /**
       *  Determines the part of a given line segment, (x1,y1) to (x2,y2),
       *  that is inside a given rectangle. 
       *
       *  @param (x1,y1) one endpoint in line segment to be clipped
       *  @param (x2,y2) second endpoint in line segment to be clipped
       *  @param minX x-coordinate of the left  veritical edge in rectangle
       *  @param maxX x-coordinate of the right veritical edge in rectangle
       *  @param minY y-coordinate of the lower horisontal edge in rectangle
       *  @param maxY y-coordinate of the upper horisontaledge in rectangle
       *  @param (x1Out,y1Out) outparameter  one endpoint in clipped
       *                                     line segment 
       *  @param  (x2Out,y2Out) outparameter second endpoint in clipped
       *                                     line segment 
       *
       *  @return true if at least one point of the line segment is inside
       *               the rectangle, false otherwise 
       */
       static bool getClippedLineInRect( int32 x1, int32 y1, int32 x2,
                                         int32 y2, int32 minX, int32 minY,
                                         int32 maxX, int32 maxY,
                                         int32& x1Out, int32& y1Out,
                                         int32& x2Out, int32& y2Out);
       /**
        *  Finds out where a given point is located relative a given
        *  bounding box. The sides of the bounding box is virtually
        *  extended so there are nine rectangles (eight unbounded ones)
        *  including the bounding box itself. 
        *
        *  @param  bbox       Bounding box used. 
        *
        *  @param  (lat,lon)  Point to determine the location for.
        *
        *  @param  vertPos    Outparameter. Vertical position of (lat,lon).
        *                     Can have the values -1, 0 or 1:
        *
        *                     -1: Somewhere below the bounding box
        *                      0: Inside, left or right of the bounding box
        *                      1: Somewhere above the bounding box
        *
        *  @param  horPos     Outparameter. Vertical position of (lat,lon).
        *                     Can have the values -1, 0 or 1:
        *
        *                     -1: Somewhere to the left of the bounding box
        *                      0: Inside, above or below the bounding box
        *                      1: Somewhere to the right of the bounding box   
        */
       static void getAdjacentRectangle(const MC2BoundingBox* bbox, int32 lat,
                                        int32 lon, int& vertPos, int& horPos);

       /*
        *  Finds out if a given segment (the whole or a part of) is inside 
        *  a given bounding box. The boundary is not included in the "inside"
        *  so a part of the segment must be in the interor of the bounding
        *  box in order to be counted as inside. 
        *
        *  @param  bbox     Bounding box used. 
        *
        *  @param  (y1,x1)  One endpoint of the segment
        *
        *  @param  (y2,x2)  Other endpoint of the segment
        *
        *  @return          True if some part of the segment is inside
        *                   the bounding box, false otherwise.
        */
       static bool isSegmentInMC2BoundingBox(const MC2BoundingBox* bbox,
                                             int32 y1, int32 x1, int32 y2,
                                             int32 x2);
       /**
        *  Method used to determine to which map a given segement shall
        *  belong to, if the given segment is on the boundary between
        *  two maps.
        * 
        *  @param (y1,x1)       Lat-lon coordinates of one endpoint
        *                       of the segment.
        *  @param (y2,x2)       Lat-lon coordinates of the second
        *                       endpoint of the segment.
        *  @param (yOut,xOut)   Outparameters. Lat-lon coordinates of a point
        *                       not lying on the given segment, and such that
        *                       points (y1,x1) (y2,x2) (yOut,xOut) are
        *                       clockwise oriented
        *                  
        */
       static void generatePointInMap(int32 y1, int32 x1, int32 y2, int32 x2,int32& yOut, int32& xOut);

       /**
        *
        * 
        */
       static float64 convertToDrawAngle(float64 angleFromNorth);
       
      /**
       *  Given two int32 numbers x and y, let x have the value of y and
       *  vice versa.
       *  @param x     int32 number
       *  @param y     int32 number
       */

      static void swapInt32(int32& x, int32& y);

       /**
        *  Checks if two points are located on the same boundary of
        *  a given rectangle. If the two points are equal, the method
        *  is used to determine if a given point is located somewhere
        *  on the rectangular boundary.
        *
        *  @param (x1,y1)     First point to check
        *  @param (x2,y2)     Second point to check
        *  @param  minX       x-coordinate of left vertical edge in rectangle
        *  @param  maxX       x-coordinate of right vertical edge in rectangle
        *  @param  minY       y-coordinate of lower horizontal edge.
        *  @param  maxY       y-coordinate of upper  horizontal edge.
        *  @return            True if the two points are on the same boundary,
        *                     false otherwise.
        */
       static bool twoPointsOnCommonRectBoundary(int32 x1, int32 y1,
                                                 int32 x2, int32 y2,
                                                 int32 minX, int32 maxX, 
                                                 int32 minY, int32 maxY);
     
       /**
        *  Checks if a given point is located on one of the four
        *  corners of a rectangle.
        *
        *  @param  (x,y)     Point to check
        *  @param  minX       x-coordinate of left vertical edge in rectangle
        *  @param  maxX       x-coordinate of right vertical edge in rectangle
        *  @param  minY       y-coordinate of lower horizontal edge.
        *  @param  maxY       y-coordinate of upper  horizontal edge.
        *  @return            true if the point is  on one of the corners,
        *                     false otherwise.
        */
     static bool pointOnRectCorner(int32 x, int32 y, 
                                   int32 minX, int32 maxX,
                                   int32 minY, int32 maxY );

      /**
       * From a max display size in pixels and a boundingbox
       * the boundingbox is streetched to fix the display.
       *
       * @param bbox   The bounding box.
       * @param width The maximun width of the display.
       * @param height The maximun height of the display.
       */
      static void getDisplaySizeFromBoundingbox( MC2BoundingBox& bbox,
                                                 uint16& width,
                                                 uint16& height );
      
      /**
       * From a max display size in pixels and a boundingbox
       * the boundingbox is streetched to fix the display.
       *
       * @param minLat Upper left latitude of boundingbox.
       * @param minLon Upper left longitude of boundingbox.
       * @param maxLat Lower right latitude of boundingbox.
       * @param maxLon Lower right longitude of boundingbox.
       * @param width The maximun width of the display.
       * @param height The maximun height of the display.
       */
      static void getDisplaySizeFromBoundingbox( int32& minLat,
                                                 int32& minLon,
                                                 int32& maxLat,
                                                 int32& maxLon,
                                                 uint16& width,
                                                 uint16& height );

      /**
       * Calculates the aspect correct view of an arc.
       *
       * @param minLat Upper left latitude of boundingbox.
       * @param minLon Upper left longitude of boundingbox.
       * @param maxLat Lower right latitude of boundingbox.
       * @param maxLon Lower right longitude of boundingbox.
       * @param width The width of the display.
       * @param height The height of the display.
       * @param lat The center of arc.
       * @param lon The center of arc.
       * @param innerRadius The inner radius.
       * @param outerRadius The outer radius.
       * @param startAngle The start angle in degrees.
       * @param stopAngle The stop angle in degrees.
       * @param cx Set center of arc x-value, may be negative or 
       *           larger than height.
       * @param cy Set center of arc y-value, may be negative or 
       *           larger than height.
       * @param iR Set to the inner radius.
       * @param oR Set to the outer radius.
       */
      static void getArcAspectCoordinates( 
         int32 minLat,
         int32 minLon,
         int32 maxLat,
         int32 maxLon,
         uint32 width,
         uint32 height,
         int32 lat, int32 lon,
         uint16 innerRadius, uint16 outerRadius,
         uint16 startAngle , uint16 stopAngle,
         int& cx, int& cy,
         int& iR, int& oR );


      /**
       * Calculates an turn arrow. NB documentation is reverse enginered
       * from source.
       * 
       *    @verbatim
         the turn  
                  |\¯¯¯¯¯¯¯¯¯¯¯¯¯ after turn
                  | \ cornerDist
                  |  \   angle \ ¯ arrowLength
                  |   \    /¯¯¯/ ¯
                  |    \  /\   
                  |     \/
                  |     /\  shortDistance
                  |    /
                  |   /  
                  |   |  longDistance 
                  |   |_
     before turn  |
            @endverbatim
       *
       * @param src The three turn points, before turn, the turn and
       *            finally after turn.
       * @param dst Set to the four(five if addCenterPoint) spline control
       *            points of the turn arrow line, see addCenterPoint.
       * @param arrow Set to the three points of the arrow, left, center
       *        and then right.
       * @param cornerDistance How far out the turn arrow line should be.
       * @param pointDistance1 Distance before center of spline.
       * @param pointDistance2 Distance after center of spline.
       * @param addCenterPoint If the center of the spline should be added
       *        to dst after the first two spline control points.
       * @param arrowLength The length of the arrow.
       * @param angle The angle of the arrow.
       * @param useCosLat If the horisontal lengths should be cossine
       *        adjusted, default true.
       */
      static void 
         calcArrowPoints( POINT* src, POINT* dst, POINT* arrow,
                          int32 cornerDistance, 
                          int32 shortDistance, 
                          int32 longDistance,
                          bool addCenterPoint,
                          int32 arrowLength,
                          int32 angle,
                          bool useCosLat = true );
      
      
      /**
       * Draws a spline.
       * @param x0 The x value of the first controlpoint.
       * @param y0 The y value of the first controlpoint.
       * @param x1 The x value of the second controlpoint.
       * @param y1 The y value of the second controlpoint.
       * @param x2 The x value of the third controlpoint.
       * @param y2 The y value of the third controlpoint.
       * @param x3 The x value of the fourth controlpoint.
       * @param y3 The y value of the fourth controlpoint.
       * @param resX The vector to insert x values into.
       * @param resY The vector to insert y values into.
       * @param bias The amount to move along the spline between each
       *             sample, default 0.02 (1/50 50 points along the curve).
       *             The bias must be in the range ]0.0,1.0].
       * @param endAngle Set to the angle at the endpoint of the spline if
       *                 endAngle is not NULL.
       */
      static void makeSpline( int x0, int y0, int x1, int y1, 
                              int x2, int y2, int x3, int y3,
                              vector<int>& resX, vector<int>& resY,
                              float32 bias = 0.02,
                              double* endAngle = NULL );

      /**
       * Calculate the radius of a circle given three coordinates
       * (on the circle border).
       * 
       * @param   lat0  The first latitude on the circle border.
       * @param   lon0  The first longitude on the circle border.
       * @param   lat1  The second latitude on the circle border.
       * @param   lon1  The second longitude on the circle border.
       * @param   lat2  The third latitude on the circle border.
       * @param   lon2  The third longitude on the circle border.
       * @return  The radius in mc2-units of the circle. MAX_UINT32 if
       *          no radius could be calculated (ie. infinite radius).
       */
      static float64 calcRadius( int32 lat0, int32 lon0,
                                 int32 lat1, int32 lon1,
                                 int32 lat2, int32 lon2 );

      /**
       *    Clips the specifed closed polygon to a boundingbox and
       *    adds it to the clippedGfxData.
       *    
       *    The polygon is clipped by first reducing the number of
       *    vertices outside the boundingbox, and then
       *    Sutherland-Hodgeman clipping is performed.
       *
       *    Note that the result is always a single polygon, even
       *    if the polygon to be clipped is concave. This is the
       *    drawback of using this method.
       *
       *    @param   bbox     The boundingbox to clip the polygon to.
       *    @param   vertices [IN/OUT] The polygon to be clipped.
       *    @return  True if the resulting clipped polygon is not empty,
       *             false otherwise.
       */
      static bool clipPolyToBBoxFast( const MC2BoundingBox* bbox,
                                      vector<POINT>& vertices );

      /**
       *    Clips the first polygon in the polygons vector against
       *    the clipPolygon.
       *
       *    The algorithm used is described in:
       *    "Efficient Clipping of Arbitrary polygons:" by Gunther Greiner
       *    and Kai Hormann from Computer Graphics Group, 
       *    University of Erlangen.
       *
       *    The algorithm handles selfintersecting polygons, and can
       *    result in multiple clipped polygons, in case the polygon
       *    to be clipped is concave.
       *
       *    The polygon to be clipped may not have coordinates that are
       *    touching a clip border.
       *
       *    @param   clipPolygon The polygon containing the polygon
       *                         to clip against. The polygon must be closed, 
       *                         i.e. the first and last coordinate 
       *                         must be the same.
       *    @param   polygons    [IN/OUT] The first polygon of the vector
       *                         is the polygon that will be clipped.
       *                         The resulting polygons will be added
       *                         to the vector, replacing the original
       *                         polygon. The input polygon must 
       *                         also be closed.
       *
       *    @return  The number of resulting polygons.
       */
      static int clipToPolygon( const vector<POINT>& clipPolygon, 
                               vector< vector<POINT> >& polygons );

      /**
       *    Removes self-intersections. Will fill holes in falukorvs.
       */
      static int cleanSelfintersecting( vector< vector<POINT> >& polygons )
         throw (MC2Exception);
      
      /**
       *    Removes self-intersections. Will fill holes in falukorvs.
       *    Currently copies all the coordinates into vectors of
       *    POINT, runs cleanSelfintersecting and then copies the
       *    result back.
       */
      static int cleanSelfintersecting(
         vector< vector<MC2Coordinate> >& polygons ) throw (MC2Exception);
  
      /// Splits self touching polygons. Returns true if splitting was done.
      static bool splitSelfTouching( vector< vector<POINT> >& polygons ) 
         throw (MC2Exception);
      static bool splitSelfTouching( 
               vector< vector<MC2Coordinate> >& polygons )
         throw (MC2Exception);
      
      /**
       *    Clips the first polygon in the polygons vector against
       *    the boundingbox.
       *
       *    The algorithm used is described in:
       *    "Efficient Clipping of Arbitrary polygons:" by Gunther Greiner
       *    and Kai Hormann from Computer Graphics Group, 
       *    University of Erlangen.
       *
       *    The algorithm handles selfintersecting polygons, and can
       *    result in multiple clipped polygons, in case the polygon
       *    to be clipped is concave. 
       * 
       *    Before clipping is performed, 
       *    the number of vertices outside the boundingbox
       *    is reduced and the vertices are slightly perturbed so that
       *    no the polygon to be clipped does not contain any coordinates
       *    on the clip polygon boundary.
       *
       *    @param   bbox        The boundingbox to clip against.
       *    @param   polygons    [IN/OUT] The first polygon of the vector
       *                         is the polygon that will be clipped.
       *                         The resulting polygons will be added
       *                         to the vector, replacing the original
       *                         polygon. The input polygon must 
       *                         also be closed.
       *    @return  The number of resulting polygons.
       */
      static int clipToPolygon( const MC2BoundingBox& bbox, 
                                vector< vector<POINT> >& polygons );
      
      /**
       *    
       *    Tests if a point is left/on/right of an infinite line.
       *    @param   p0 The first point of the infinite line.
       *    @param   p1 The second point of the infinte line.
       *    @param   p2 The point to check.
       *
       *    @return >0 for P2 left of the line through P0 and P1
       *            =0 for P2 on the line
       *            <0 for P2 right of the line.
       */
      inline static int isLeft( const POINT& p0, 
                                const POINT& p1, 
                                const POINT& p2 );

      /**
       *    Clips the first polyline (open polygon) 
       *    in the polygons vector against the boundingbox.
       *
       *    @param   bbox        The boundingbox to clip against.
       *    @param   polygons    [IN/OUT] The first polygon of the vector
       *                         is the polygon that will be clipped.
       *                         The resulting polygons will be added
       *                         to the vector, replacing the original
       *                         polygon. 
       *    @return  The number of resulting polygons.
       */
      static int clipPolylineToBBox( const MC2BoundingBox& bbox, 
                                     vector< vector<POINT> >& polygons );
   private:

      /**
       *    Find the intersection between two lines that can be described
       *    by the following coordinates:
       *    Line 1: (x1, y1) - (x2, y2) and \
       *    Line 2: (v1, w1) - (v2, w2).
       *    @param x1   x1 coordinate of line 1.
       *    @param y1   y1 coordinate of line 1.
       *    @param x2   x2 coordinate of line 1.
       *    @param y2   y2 coordinate of line 1.
       *
       *    @param v1   v1 coordinate of line 2.
       *    @param w1   w1 coordinate of line 2.
       *    @param v2   v2 coordinate of line 2.
       *    @param w2   w2 coordinate of line 2.
       *
       *    @param intersectX Output parameter set to the x coordinate
       *                      of the intersection of the two lines.
       *    @param intersectY Output parameter set to the y coordinate
       *                      of the intersection of the two lines.
       *    @param percentFromx1y1 [Output] If not NULL, 
       *                           how many percent between 
       *                           (x1,y1) and (x2,y2) 
       *                           the intersection lies. 
       *    @param percentFromv1w1 [Output] If not NULL,
       *                           how many percent between 
       *                           (v1,w1) and (v2,w2) 
       *                           the intersection lies. 
       *    @return  True if the two lines intersects each other,
       *             false otherwise.
       */
      static bool getIntersection(int32 x1, int32 y1, int32 x2, int32 y2,
                                  int32 v1, int32 w1, int32 v2, int32 w2,
                                  int32& intersectX, int32& intersectY,
                                  float64* percentFromx1y1,
                                  float64* percentFromv1w1 );
      
      /**
       *    Clip the vertices against a boundary of a boundingbox.
       *                            
       *    Part of the Sutherland-Hodgeman clipping algorithm.
       *
       *    @param   boundaryOutcode   The Cohen-Sutherland
       *                               outcode of boundary of 
       *                               the boundingbox to clip to, ie. 
       *                               MC2BoundingBox::TOP/BOTTOM/LEFT/RIGHT
       *    @param   bbox              The boundingbox.
       *    @param   vertices          Vector of the vertices to clip.
       *                               The vector is cleared afterwards.
       *    @param   outcodes          Vector of the outcodes of the 
       *                               vertices to clip.
       *                               The vector is cleared afterwards.
       *    @param   resVertices       The resulting clipped vertices 
       *                               are added to this vector.
       *    @param   resOutcodes       The resulting outcodes of the
       *                               clipped vertices are added to this
       *                               vector.
       *    @return  True if the resulting clipped vertices is a valid
       *             polygon, false otherwise.
       */
      static bool clipToBoundary( const byte boundaryOutcode, 
                                  const MC2BoundingBox* bbox,
                                  vector<POINT>& vertices,
                                  vector<byte>& outcodes,   
                                  vector<POINT>& resVertices,
                                  vector<byte>& resOutcodes );
      
      /**
       *    Help-method to clipToBoundary(..).
       *    Clips the two vertices against the boundary of a boundingbox.
       *
       *    @param   prevVertex  The previous vertex.
       *    @param   currVertex  The next vertex.
       *    @param   prevInside  If prevVertex is inside the bbox boundary.
       *    @param   currInside  If currVertex is inside the bbox boundary.
       *    @param   currOutcode The Cohen-Sutherland outcode of 
       *                         currVertex.
       *    @param   bbox        The boundingbox.
       *    @param   boundaryOutcode   The Cohen-Sutherland
       *                               outcode of boundary of 
       *                               the boundingbox to clip to, ie. 
       *                               MC2BoundingBox::TOP/BOTTOM/LEFT/RIGHT
       *    @param   resVertices  The resulting clipped vertices 
       *                          are added to this vector.
       *    @param   resOutcodes  The resulting outcodes of the
       *                          clipped vertices are added to this
       *                          vector.
       */
      static void clipSegment( const POINT& prevVertex,
                               const POINT& currVertex,
                               bool prevInside,
                               bool currInside,
                               byte currOutcode,
                               const MC2BoundingBox* bbox,
                               byte boundaryOutcode,
                               vector<byte>& resOutcodes,
                               vector<POINT>& resVertices );

  public:
      /**
       *    Reduces the number of vertices outside the boundingbox and
       *    computes the Cohen-Sutherland outcodes for each of those
       *    vertices.
       *
       *    @param   bbox           The boundingbox to reduce the polygon.
       *    @param   vertices       The input polygon.
       *    @param   reducedVertices   [OUT] The resulting reduced polygon.
       *    @param   outcodes       The Cohen-Sutherland outcodes of the   
       *                            reduced vertices are added to this 
       *                            vector<byte>.
       *    @return  True if the vectors got filled with any vertices / 
       *             outcodes, false otherwise.
       */
     static bool reduceByBBox( const MC2BoundingBox* bbox,
                               const vector<POINT>& vertices,
                               vector<POINT>& reducedVertices,
                               vector<byte>& outcodes );     
};

//======================================================================
//                                    Implementation of inline methods =


inline bool
GfxUtility::pointInsideRect( int32 x, int32 y, int32 minX, int32 maxX,
                             int32 minY, int32 maxY )
{
   if (((minX <= x) && (x <= maxX)) && ((minY <= y) && (y <= maxY)))
      return true;
   else
      return false;
}


#endif
