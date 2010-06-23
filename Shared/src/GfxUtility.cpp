/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GfxUtility.h"
#include "GfxConstants.h"
#include "MC2BoundingBox.h"
#include "ClipUtil.h"
#include "InsideUtil.h"
#include "GnuPlotDump.h"
#include "Intersect.h"
#include "MC2Exception.h"
#include "CoordinateTransformer.h"
#include "StringTable.h"

#include "Math.h"

#include <stdlib.h>
#include <set>
#include <algorithm>

// Forward
class Vertex;

/**
 *    List of vertices.
 *
 *    Class needed by the implementation of the algorithm  
 *    "Efficient Clipping of Arbitrary Polygons", by
 *    Gunther Greiner and Kai Hormann, i.e. GfxUtility::clipToPolygon(...).
 *
 */
class VertexList : public list<Vertex*>
{
   public:

      /**
       *    Destructor. NB, will delete all Vertex objects in the list.
       */
      virtual ~VertexList();

      /**
       *    Inserts the intersection Vertex in this list, between
       *    the two iterators, according to it's alpha value.
       *    
       *    @param   first             The first iterator.
       *    @param   last              The last (end) iterator.
       *    @param   intersectVertex   The Vertex to insert.
       *                               Must have a valid alpha value.
       */
      VertexList::iterator insertIntersectVertex( 
            VertexList::iterator first,
            VertexList::iterator last,
            Vertex* intersectVertex );

      /**
       *    Checks if the specified point is inside the polygon or not.
       *    @param   p  The point to check.
       *    @param   If the polygon was inside the polygon.
       */
      bool inside( const POINT& p ) const;

      /**
       *    Set ids.
       */
      void renumber();
      
      /**
       *    Dump the vertex list to the specified stream.
       *    @param   stream   The stream.
       */
      void dump( ostream& stream ) const;

   private:

      /**
       *    Winding number test for a point in a polygon.
       *    @param   p  The point to check.
       *    @return  The winding number. 
       *             0 only if p is outside the polygon.
       */
      int windingNumber( const POINT& p ) const;
};

/**
 *    A vertex.
 *
 *    Class needed by the implementation of the algorithm  
 *    "Efficient Clipping of Arbitrary Polygons", by
 *    Gunther Greiner and Kai Hormann, i.e. GfxUtility::clipToPolygon(...).
 *
 */
class Vertex : public POINT {

   public:
      
      /**
       *    Initializes the member variables.
       */
      void init();

      /**
       *    Constructor.
       */
      Vertex();

      /**
       *    Constructor with a point.
       *    @param   point A point.
       */
      Vertex( const POINT& point);

      /**
       *    Dump the vertex to the specified stream.
       *    @param   stream   The stream.
       */
      void dump( ostream& stream ) const;

      /**
       *    Vertex ID. Use to know what happens.
       */
      int m_id;
      
      /**
       *    If the vertex is a intersection or not.
       */
      bool m_intersect;

      /**
       *    Iterator to neighbor vertexlist in case of an intersection.
       */
      VertexList::iterator m_neighbor;

      /**
       *    Alpha value for the intersection. I.e. only valid if 
       *    it is a intersection. Specifies how many percent between
       *    the previous and next vertex the intersection vertex lies.
       */
      float64 m_alpha;

      /**
       *    If true, the vertex is entering the polygon.
       *    If false, the vertex is exiting the polygon.
       *    Only valid if it is an intersection.
       */
      bool m_enteringPoly;
      
      /**
       *    Helper class to get coords.
       */
      class XYHelper {
         public:
            /**
             * @return  X coord from the vertex.
             */
            int getX( const Vertex* vertex ) const {
               return vertex->x;
            }

            /**
             * @return  Y coord from the vertex.
             */
            int getY( const Vertex* vertex ) const {
               return vertex->y;
            }
      };
};

// --- Implementation of VertexList ---

VertexList::~VertexList()
{
   for ( VertexList::iterator it = begin();
         it != end(); ++it ) {
      delete *it;
   }
}

VertexList::iterator 
VertexList::insertIntersectVertex( 
      VertexList::iterator first,
      VertexList::iterator last,
      Vertex* intersectVertex )
{
   for ( VertexList::iterator it = first; it != last; ++it ) {
      if ( it == first ) {
         continue;
      }
      if ( intersectVertex->m_alpha < (*it)->m_alpha ) {
         // Insert the vertex before the iterator.
         return insert( it, intersectVertex );
      } 
   }

   return insert( last, intersectVertex );
}
      
void 
VertexList::renumber()
{
   int id = 0;
   for ( iterator it = begin(); it != end(); ++it ) {
      (*it)->m_id = id;
      ++id;
   }
}

void
VertexList::dump( ostream& stream ) const
{
   for ( const_iterator it = begin(); it != end(); ++it ) {
      (*it)->dump( stream );
   }
}

static ostream& 
operator << ( ostream& stream, const VertexList& vertexList )
{
   vertexList.dump( stream );
   return stream;
}

// --- Implementation of Vertex ---

void 
Vertex::init() 
{
   m_intersect = false;
   m_alpha = 0;
   m_enteringPoly = false;
   m_id = -1;
}

Vertex::Vertex() 
{
   init();
}

Vertex::Vertex( const POINT& point) {
   this->x = point.x;
   this->y = point.y;
   init();
}

void
Vertex::dump( ostream& stream ) const
{
   stream << "Vertex: [" << m_id << "] , "
          << this->x << ", " << this->y;
   if ( m_intersect ) {
      stream << " neighbor [" << (*m_neighbor)->m_id << "]";
      stream << " alpha = " << m_alpha;
      stream << " entering poly = " << m_enteringPoly;
   }
   stream << endl;
}

static ostream& 
operator << ( ostream& stream, const Vertex& vertex )
{
   vertex.dump( stream );
   return stream;
}

float64
GfxUtility::squareP2Pdistance_linear(int32 lat1, int32 lon1, 
                                     int32 lat2, int32 lon2,
                                     float64 cos_lat)
{  
   if (cos_lat >= 1) {
      //Local linearisation being done at the mean latitude.
      cos_lat= cos( (float64) (lat1/2+lat2/2) * M_PI /
                    ((float64) 0x80000000) );
   }
   float64 delta_lat_sq =  SQUARE( (float64) (lat2 - lat1) );
   float64 delta_lon_sq =  SQUARE( ((float64) (lon2-lon1)) * cos_lat);
   return ( (delta_lat_sq + delta_lon_sq) * 
            GfxConstants::SQUARE_MC2SCALE_TO_SQUARE_METER);
}

float64
GfxUtility::squareP2Pdistance_linear(const MC2Coordinate& p1,
                                     const MC2Coordinate& p2,
                                     float64 cos_lat )
{
   return squareP2Pdistance_linear( p1.lat, p1.lon,
                                    p2.lat, p2.lon,
                                    cos_lat );
}


//            (lat2, lon2)
//         |     /
//         |    /
//         |   /
//         |A /
//         |\/
//         |/ (lat1, lon1)
//         |
//         |
//         |
// @return  Angle in radians from the north-direction in clockwise direction.
//          The return value is in the interval [0,2PI[
float64
GfxUtility::getAngleFromNorth(int32 lat1, int32 lon1, 
                              int32 lat2, int32 lon2, bool useCosLat )
{
   float64 coslat = 1;
   if ( useCosLat ) {
      coslat = cos( 
       GfxConstants::invRadianFactor * float64(lat2/2 + lat1/2));
      //Bringing in coslat. Without coslat one meter north and one meter
      //east will in Linköping give 60 degrees and not 45 as expected.
   }
   float64 A = atan2( coslat*(lon2-lon1), (float64) (lat2 - lat1) );
   if (A < 0)
      A += M_PI*2;
   
   if (A<0 || A>M_PI*2)
      PANIC("A out of range", "!");

   return (A);

}

float64 GfxUtility::getAngleFromNorth( const MC2Coordinate& first,
                                       const MC2Coordinate& second ) {
   return getAngleFromNorth( first.lat, first.lon,
                             second.lat, second.lon );
}

float64
GfxUtility::getAngleFromNorthNoCoslat(int32 lat1, int32 lon1, 
                                      int32 lat2, int32 lon2)
{

   return  getAngleFromNorth( lat1, lon1, lat2, lon2, false );

}

double
GfxUtility::normalizeAngle( double angle ) {
   double normalized = fmod( static_cast<double>( angle ), 2*M_PI );
   if ( normalized < 0 ) { 
      normalized += 2*M_PI;
   }   
   return normalized;
}

double 
GfxUtility::angleDifference( double angle1, double angle2 ) {
   angle1 = normalizeAngle( angle1 );
   angle2 = normalizeAngle( angle2 );
   double smallest = min( angle1, angle2 );
   double largest  = max( angle1, angle2 );

   return min( largest-smallest, ( 2*M_PI-largest )+smallest );
}

int32
GfxUtility::convLatLonStringToInt(const char* s)
{
   // Make a copy of the string
   int size = strlen(s);
   char* tmpBuf = new char[size + 1];
   strcpy(tmpBuf, s);
   
   // Format of indata (N/S/E/W) D(D*)MMSS[.ddd]
   // or D(D*).dd (N/S/E/W)
   
   // Find out the position of the period. Since the period is optional, 
   // if it is not found the periodPos will be size.
   uint8 periodPos = 0;
   while ((periodPos < size) && (s[periodPos] != '.')) {
      periodPos++;
   }
   // Convert from string to int by null terminating tmpBuf where
   //we want the number to end.
   double retVal = 0;
   
   if ( size > 3 && (periodPos - 4) >= 0 ) { // We index periodPos - 4
      retVal = GfxConstants::secondFactor * 
                      (strtod(&tmpBuf[periodPos - 2], (char**)NULL));
      tmpBuf[periodPos - 2] = '\0';
      retVal += GfxConstants::minuteFactor * (strtol(&tmpBuf[periodPos - 4],
                                       (char**)NULL, 10));
      tmpBuf[periodPos - 4] = '\0';
      retVal += GfxConstants::degreeFactor * 
                    (strtol(&tmpBuf[1], (char**)NULL, 10));
      
      bool changeSign = false;
      // Check the sign.
      switch (tmpBuf[0]) {
         case ('N'):
         case ('E'):
            changeSign = false;
            break;
         case ('S'):
         case ('W'):
            changeSign = true;
            break;
      }
      if (changeSign) {
         retVal = - retVal;
      }
   }

   delete [] tmpBuf;
   return (int32(retVal));
}

float64
GfxUtility::convLatLonStringToDbl(const char* s)
{
   // Make a copy of the string
   int size = strlen(s);
   char sign = s[size-1];
   char* tmpBuf = new char[size + 1];
   strcpy(tmpBuf, s);
   
   char* endPtr = &tmpBuf[size-1];
   float64 retVal = strtod(tmpBuf, &endPtr);
   
   bool changeSign = false;
   // Check the sign.
   switch (sign) {
      case ('N'):
      case ('E'):
         changeSign = false;
         break;
      case ('S'):
      case ('W'):
         changeSign = true;
         break;
   }
   if (changeSign) {
      retVal = - retVal;
   }
   delete [] tmpBuf;
   return retVal * M_PI/180;
}


uint32 
GfxUtility::printLatLon( char* str, 
                         float64 latOrLon,
                         bool lat,
                         LanguageCode lang,
                         uint32 precision,
                         coordianteRepresentations representation )
{
   const char* compas = NULL;
   if ( representation == nmea ) {
      lang = StringTable::ENGLISH; // nmea is always in english
   }

   if ( lat ) {
      if ( latOrLon < 0 ) {
         compas = StringTable::getString( 
            StringTable::DIR_SOUTH,
            StringTable::getShortLanguageCode( lang ) );
      } else {
         compas = StringTable::getString( 
            StringTable::DIR_NORTH,
            StringTable::getShortLanguageCode( lang ) );
      }
   } else {
      if ( latOrLon < 0 ) {
         compas = StringTable::getString( 
            StringTable::DIR_WEST,
            StringTable::getShortLanguageCode( lang ) );
      } else {
         compas = StringTable::getString( 
            StringTable::DIR_EAST,
            StringTable::getShortLanguageCode( lang ) );
      }
   }

   uint32 nbrChars = 0;
   switch( representation ) {
      case simpleRad: {
         char format[ 20 ];
         sprintf( format, "%%.%df%%s", precision );
         
         nbrChars = sprintf( str, format,
                             latOrLon * 180 / M_PI,
                             compas );
      }
      break;
      
      case wgs84Style: {
         if ( latOrLon < 0 ) {
            latOrLon = -latOrLon;
         }
         float64 ordinate = latOrLon * 180 / M_PI;
         int deg = int(ordinate);
         float64 fmin = (ordinate-deg)*60;
         int min = int(fmin);
         float64 fsec = (fmin-min)*60;

         char* format = new char[ 40 + precision ];
         sprintf( format, 
#ifdef MC2_UTF8
                  "%%s %%dÂ° %%.2d' "
#else
                  "%%s %%dï¿½ %%.2d' "
#endif
                  "%%0%d.%df\"", 3 + precision , precision );

         nbrChars = sprintf( str, format, compas, deg, min, fsec );
         delete [] format;
      }
      break;

      case wgs84Rad: {
         char format[ 20 ];
         sprintf( format, "%%.%df", precision );
         nbrChars = sprintf( str, format,
                             latOrLon );
      }
      break;
      case wgs84Deg: {
         char format[ 20 ];
         sprintf( format, "%%.%df", precision );
         nbrChars = sprintf( str, format,
                             latOrLon * 180 / M_PI );
      }
      break;
      case nmea : {
         // [N|S]ddmm.mmmm  [E|W]dddmm.mmmm
         char format[ 20 ];
         const char* degFormat = "%%s%%0.%dd%%0%d.%df";
         sprintf( format, degFormat, lat ? 2 : 3, 
                  3 + precision, precision );
         if ( latOrLon < 0 ) {
            latOrLon = -latOrLon;
         }
         float64 ordinate = latOrLon * GfxConstants::radianTodegreeFactor;
         int deg = int( ordinate );
         float64 fmin = ( ordinate - deg ) * 60;
         
         nbrChars = sprintf( str, format, compas, deg, fmin );
      }
      break;
   }

   return nbrChars;
}
void 
GfxUtility::
printLatLon( MC2String& latStr, MC2String& lonStr,
             const MC2Coordinate& coord,
             const LanguageCode& language,
             coordianteRepresentations representation ) {
   char latBuff[128];
   char lonBuff[128];
   printLatLon( latBuff, coord.lat, true,
                language,
                representation );
   printLatLon( lonBuff, coord.lon, false,
                language,
                representation );
   latStr = latBuff;
   lonStr = lonBuff;
}

uint32
GfxUtility::printLatLon( char* str, 
                         int32 latOrLon,
                         bool lat,
                         const LanguageCode& lang,
                         coordianteRepresentations representation )
{
   uint32 precision = 4;
   if ( representation == GfxUtility::wgs84Style ) {
      precision = 4;
   } else if ( representation == GfxUtility::wgs84Rad ) {
      precision = 10;
   } else if ( representation == GfxUtility::wgs84Deg ) {
      precision = 8;
   } else if ( representation == GfxUtility::nmea ) {
      precision = 4;
   }
   float64 olat;
   float64 olon;
   float64 oalt;
   CoordinateTransformer::transformFromMC2(
      latOrLon, latOrLon, CoordinateTransformer::sweref93_LLA,
      olat, olon, oalt );

   return printLatLon( str, lat ? olat : olon, lat, lang, 
                       precision, representation );
}

bool
GfxUtility::getIntersection(int32 x1, int32 y1, int32 x2, int32 y2,
                            int32 v1, int32 w1, int32 v2, int32 w2,
                            int32& intersectX, int32& intersectY,
                            float64* percentFromx1y1,
                            float64* percentFromv1w1 )
{
   // Can handle vertical lines.
   
   // x = x1 + t*alpha
   // y = y1 + t*beta
   // 
   // v = v1 + s*gamma
   // w = w1 + s*theta
   //
   // alpha, beta, gamma, theta as stated below

   int32 alpha    = x2 - x1;
   int32 beta     = y2 - y1;
   int32 gamma    = v2 - v1;
   int32 theta    = w2 - w1;

   int64 denominator = int64(theta)*alpha - int64(gamma)*beta; 
   
   // Check if the lines are parallell
   if (denominator == 0) {
      return (false);
   }

   // Solve the equation system
   float64 s = float64(int64(y1 - w1)*alpha + int64(v1 - x1)*beta) / 
               denominator;

   float64 t = float64( int64(v1 - x1)*theta + int64(y1 - w1)*gamma ) /
               denominator;

   
   if ( ( s < 0.0 ) || ( s > 1.0 ) ) {
      mc2dbg8 << "[GU]: s out of range - no intersect" << endl;
      return false;
   }
   if ( ( t < 0.0 ) || ( t > 1.0 ) ) {
      mc2dbg8 << "[GU]: percentFromx1y1 out of range - no intersect" << endl;
      return false;
   }

   if ( percentFromx1y1 ) {
      *percentFromx1y1 = t;
   }
   if ( percentFromv1w1 ) {
      *percentFromv1w1 = s;
   }

   intersectX = int32(v1 + rint(s*gamma));
   intersectY = int32(w1 + rint(s*theta));

   return (true);
}

bool
GfxUtility::getIntersection(int32 x1, int32 y1, int32 x2, int32 y2,
                            int32 v1, int32 w1, int32 v2, int32 w2,
                            int32& intersectX, int32& intersectY,
                            float64& percentFromx1y1,
                            float64& percentFromv1w1 )
{
   return getIntersection( x1, y1, x2, y2, 
                           v1, w1, v2, w2,
                           intersectX, intersectY, 
                           &percentFromx1y1,
                           &percentFromv1w1 );
}

bool
GfxUtility::getIntersection(int32 x1, int32 y1, int32 x2, int32 y2,
                            int32 v1, int32 w1, int32 v2, int32 w2,
                            int32& intersectX, int32& intersectY)
{
   return getIntersection( x1, y1, 
                           x2, y2, 
                           v1, w1, 
                           v2, w2, 
                           intersectX, intersectY, NULL, NULL );
}


uint64 
GfxUtility::closestDistVectorToPoint( int32 x1, int32 y1,
                                      int32 x2, int32 y2,
                                      int32 xP, int32 yP,
                                      double cosF )
{
   uint64 dist;

   //ex and ey are unitvectors
   //
   //Vector V1 = (x2 - x1) * ex + (y2 - y1) * ey
   //Vector V2 = (xP -Removed all  x1) * ex + (yP - y1) * ey

   //Calculate the scalarproduct V1 * V2
   int64 scalarProd = int64( cosF * (x2 - x1) * cosF * (xP - x1) +
                      double( y2 - y1 ) * double( yP - y1 ));

   if( (x1 - x2 == 0) && (y1 == y2) )
   {
      //V1 has a length of zero, use plain pythagoras
      dist = SQUARE( int64( cosF*(xP - x1) ) ) + 
             SQUARE( int64( yP - y1 ) );
   }
   else if ( scalarProd < 0 )
   {
      //The angle between V1 and V2 is more than pi/2,
      //this means that (xP, yP) is closest to (x1, y1) 
      dist = SQUARE( int64( cosF*(xP - x1) ) ) +
             SQUARE( int64( yP - y1 ) );
   }
   else if ( scalarProd > int64( SQUARE( int64( cosF*(x2 - x1) ) ) +
                                 SQUARE( int64( y2 - y1 ) ) ) )
   {
      //The scalarproduct V1 * V2 is larger than the length of V1
      //this means that (xP, yP) is closest to (x2, y2) 
      dist = SQUARE( int64( cosF*(xP - x2) ) ) +
             SQUARE( int64( yP - y2 ) );
   }
   else 
   {
      //Use the formula for minimumdistance from a point to a line
      //dist = ( V2x * V1y - V2y * V1x )^2 / ( (V1x ^ 2) + (V1y ^ 2) )
      dist = uint64( SQUARE( double(y2 - y1) * cosF*(xP - x1) + cosF*(x1 - x2) * double(yP - y1) ) /
             ( SQUARE( double(y2 - y1) ) + SQUARE( cosF*(x1 - x2) ) ) );
   }

   return dist;
}
uint64
GfxUtility::closestDistVectorToPoint( int32 x1, int32 y1,
                                      int32 x2, int32 y2,
                                      int32 xP, int32 yP,
                                      int32& xOut, int32& yOut,
                                      double cosF )
{
   uint64 dist;

   //ex and ey are unitvectors
   //
   //Vector V1 = (x2 - x1) * ex + (y2 - y1) * ey
   //Vector V2 = (xP -Removed all  x1) * ex + (yP - y1) * ey

   //Calculate the scalarproduct V1 * V2
   int64 scalarProd = int64( cosF * (x2 - x1) * cosF * (xP - x1) +
                      double( y2 - y1 ) * double( yP - y1 ));

   if ( (x1 - x2 == 0) && (y1 == y2) )
   {
      DEBUG8(cerr << "   CASE 0" << endl);
      //V1 has a length of zero, use plain pythagoras
      dist = SQUARE( int64( cosF*(xP - x1) ) ) + 
             SQUARE( int64( yP - y1 ) );
      xOut = x1;
      yOut = y1;      
   }
   else if ( scalarProd < 0 )
   {
      DEBUG8(cerr << "   CASE 1" << endl);
      //The angle between V1 and V2 is more than pi/2,
      //this means that (xP, yP) is closest to (x1, y1) 
      dist = SQUARE( int64( cosF*(xP - x1) ) ) +
         SQUARE( int64( yP - y1 ) );
      xOut = x1;
      yOut = y1;
   }
   else if ( scalarProd > int64( SQUARE( int64( cosF*(x2 - x1) ) ) +
                                 SQUARE( int64( y2 - y1 ) ) ) )
   {
      DEBUG8(cerr << "   CASE 2" << endl);
      //The scalarproduct V1 * V2 is larger than the length of V1
      //this means that (xP, yP) is closest to (x2, y2) 
      dist = SQUARE( int64( cosF*(xP - x2) ) ) +
             SQUARE( int64( yP - y2 ) );
      xOut = x2;
      yOut = y2;
   }
   else 
   {
      //Use the formula for minimumdistance from a point to a line
      //dist = ( V2x * V1y - V2y * V1x )^2 / ( (V1x ^ 2) + (V1y ^ 2) )
      dist = uint64( SQUARE( double(y2 - y1) * cosF*(xP - x1) +
                             cosF*(x1 - x2) * double(yP - y1) ) /
             ( SQUARE( double(y2 - y1) ) + SQUARE( cosF*(x1 - x2) ) ) );
      
      // Find intersection point (xOut,yOut) by writing the equations
      // of the two lines L1 and L2 defined below and solving the
      // upcoming (2*2) equation system .
      // "k" is the slope of the line L1 between (x1,y1) and (x2,y2),
      //  k = (y1 - y2)/(cosF*(x1 - x2));
      //  Since the line L2, between (xP,yP) and (xOut,yOut), is perpendicular
      //  to L1, the slope of L2 is -1/k (known from High School)
      
      //  |y - y1 = k(x - x1)cosF      ( equation for line between (x1,y1) and (x2,y2) 
      //  |y - yP = -1/k*(x - xP)cosF  ( equation for line between (xP,yP) and (xOut,yOut)
      //
      //  |y - k*cosF*x  = -k*x1*cosF + y1 <=>
      //  |y + cosF/k*x  = cosF/a*xP + yp   <=>
      //
      //  |1  -k*cosF || y |    | -k*x1*cosF + y1 |
      //  |           ||   | =  |                 | 
      //  |1   cosF/k || x |    |  cosF/a*xP + yp |
      //
      //  This is a linear equation system on the form Az = b,
      //  where
      //       |1  -k*cosF |      | y |       | -k*x1*cosF + y1 |
      //  A =  |           | , z =|   | , b = |                 |
      //       |1   cosF/k |      | x |       | cosF/a*xP + yp  |
      //
      //  The solution is obtained as z = inv(A)*b where inv(A) is the inverse of A.

      
      if ( x1 == x2 ) {
         // vertical line between (x1,x2) and (y1,y2)
         DEBUG8(cerr << "   CASE 3.1" << endl);
         xOut = x1;
         yOut = yP;

      }
      else if ( y1 == y2 ) {
         // horizontal line between (x1,x2) and (y1,y2)
         xOut = xP;
         yOut = y1;
      }
      else {
          // "lutning" k
         double k = double(y1 - y2)/(cosF*(x1 - x2));    // slope 
         float64 oneOverDeterminant = k/cosF/(SQUARE(k) + 1);
         // after simplification of z = inv(A)*b
         yOut = int32(oneOverDeterminant*(SQUARE(cosF)*(xP - x1) + cosF/k*y1
                                          +cosF*k*yP));
         xOut = int32(oneOverDeterminant*(yP - y1 + cosF/k*xP + cosF*k*x1));
        
         DEBUG8(cerr << "   CASE 3.2" << endl);

         //xOut = int32(1/(SQUARE(k) + 1)*(SQUARE(k)*x1 + xP + k*(yP-y1) ));
         //yOut = int32(1/(SQUARE(k) + 1)*(y1 + k*cosF*(xP-x1) + SQUARE(k)*yP));

         DEBUG8(
          cerr << "   k=" << k << endl;
          cerr << "   (x1,y1)=(" << x1 << "," << y1 << endl;
          cerr << "   (x2,y2)=(" << x2 << "," << y2 << endl;
          cerr << "   (xP,yP)=(" << xP << "," << yP << endl;
          cerr << "   (xOut,yOut)=(" << xOut << "," << yOut << endl;
          );
      }
   }
   return (dist); 
}

bool
GfxUtility::getPointOnLine(int32 lat1, int32 lon1,
                           int32 lat2, int32 lon2, float64 dist,
                           int32& outLat, int32& outLon)
{
   float64 totDist = sqrt(GfxUtility::squareP2Pdistance_linear(
                                       lat1, lon1, lat2, lon2));
   return getPointOnLine(lat1, lon1, lat2, lon2, dist, totDist,
                         outLat, outLon);
   // Notice that the return-statement is above this debugprinting...
   mc2dbg8 << here << " totDist=" << totDist << ", dist=" << dist
           << "1(" << lat1 << "," << lon1 << ") 2(" << lat2 << "," << lon2
           << ") n(" << outLat << "," << outLon << ")" << endl;
}

bool
GfxUtility::getPointOnLine(int32 lat1, int32 lon1,
                           int32 lat2, int32 lon2,
                           float64 dist, float64 totDist,
                           int32& outLat, int32& outLon)
{
   if (totDist <= dist) {
      // (outLat, outLon) could not be fitted on the line
      mc2log << error << here << " totDist <= dist" << endl;
      return false;
   } else {
      outLat = int32( float64(dist*(lat2-lat1) + totDist*lat1) /
                      float64(totDist));
      outLon = int32( float64(dist*(lon2-lon1) + totDist*lon1) /
                      float64(totDist));

      // Test
      DEBUG2(
         float64 s = sqrt(GfxUtility::squareP2Pdistance_linear(
                               lat1, lon1, outLat, outLon));
         mc2dbg1 << here << " s=" << s << ", dist=" << dist
                 << ", totDist=" << totDist << endl;
      );
      return true;
   }
}


void
GfxUtility::generatePointInMap(int32 y1, int32 x1, int32 y2, int32 x2,
                               int32& yOut, int32& xOut) 
{
   // Special case when segment is degenerated to a point
   DEBUG1(
      if ((x1 - x2 == 0) && (y1 - y2 == 0))
          mc2dbg1 << "GfxUtility::generatePointInMap "
                  << "The first and second endpoints of this segment coincide!"
                  << endl;
   );
   int32 yDiff = y2 - y1;
   int32 midLon = int32(double(x1 + x2)/2);
   if (yDiff != 0) {
      int32 xDiff = x2 - x1;
      double k = -double(xDiff)/yDiff;
      double midLat = double(y1 + y2)/2;
      int32 xOutTemp = midLon + 2;
      int32 yOutTemp = (int32)(midLat + 2*k);
      if (int64(xDiff)*(yOutTemp - y1)-int64(yDiff)*(xOutTemp - x1) > 0){
         xOut = xOutTemp; 
         yOut = yOutTemp;
      }
      else {
         xOut = midLon - 2;
         yOut = (int32)(midLat - 2*k);
      }
   }
   else {
      xOut = midLon;
      if (x2 - x1 > 0)
         yOut = y1 + 1;
      else
         yOut = y1 - 1;
   }
}

float64
GfxUtility::convertToDrawAngle(float64 angleFromNorth) 
{
   float64 tempAngle = angleFromNorth;
   // 1. check indata
   while (tempAngle < 0)
      tempAngle = tempAngle + 2*M_PI;
   while(tempAngle >= 2*M_PI)
      tempAngle = tempAngle - 2*M_PI;
   // 2. Now 0=< tempAngle <=2*Pi, convert
   if (tempAngle <= M_PI/2)
      return(M_PI/2 - tempAngle);
   else
      return(5*M_PI/2 - tempAngle);
}


   
void
GfxUtility::swapInt32(int32& x, int32& y)
{
   int32 temp = x;
   x = y;
   y = temp;
}

bool
GfxUtility::twoPointsOnCommonRectBoundary( int32 x1, int32 y1, int32 x2,
                                           int32 y2, int32 minX, int32 maxX,
                                           int32 minY, int32 maxY )
{
   if ( (pointInsideRect( x1, y1, minX, maxX, minY, maxY )) &&
        (pointInsideRect( x2, y2, minX, maxX, minY, maxY ))){
      if ( ( (x1 == x2)&&(x1 == minX) ) || ( (x1 == x2)&&(x1 == maxX) ) ||
           ( (y1 == y2)&&(y1 == minY) ) || ( (y1 == y2)&&(y1 == maxY) ) )
         return true;
      else
         return false;
   }
   else
      return false;       
}

bool
GfxUtility::pointOnRectCorner( int32 x, int32 y, int32 minX, int32 maxX,
                        int32 minY, int32 maxY )
{
   if ((( x == minX) && (  y == minY)) ||
       (( x == minX) && (  y == maxY)) ||
       (( x == maxX) && (  y == maxY)) ||
       (( x == maxX) && (  y == minY)))
      return true;
   else
      return false;
}

bool
GfxUtility::getClippedLineInRect( int32 x1, int32 y1, int32 x2, int32 y2,
                                  int32 minX, int32 maxX, int32 minY,
                                  int32 maxY, int32& x1Out, int32& y1Out,
                                  int32& x2Out, int32& y2Out)
   
{
   // The old method did not seem to work.
   // The new version uses clipPolylineToBBox instead.

   vector< vector< POINT > > clippedPolygon;

   clippedPolygon.push_back( vector<POINT>() );
   vector<POINT>& inpoly = clippedPolygon.front();

   POINT p;
   p.x = x1;
   p.y = y1;   
   inpoly.push_back( p );
   p.x = x2;
   p.y = y2;
   inpoly.push_back( p );

   MC2BoundingBox bbox;
   bbox.update( minX, minY );
   bbox.update( maxX, maxY );

   if ( clipPolylineToBBox( bbox, clippedPolygon ) ) {
      if ( ( clippedPolygon.size() == 1 ) &&
           ( clippedPolygon[ 0 ].size() == 2 ) ) {
         // Everything seems to be in order.
         x1Out = clippedPolygon[ 0 ][ 0 ].x;
         y1Out = clippedPolygon[ 0 ][ 0 ].y;
         x2Out = clippedPolygon[ 0 ][ 1 ].x;
         y2Out = clippedPolygon[ 0 ][ 1 ].y;
         return true;
      }
   }
   return false;
}

void
GfxUtility::getAdjacentRectangle(const MC2BoundingBox* bbox, int32 lat,
                                 int32 lon, int& vertPos, int& horPos)
{
  if (lat - bbox->getMaxLat() >= 0) 
      vertPos = 1;
   else if (lat - bbox->getMinLat() > 0)
      vertPos = 0;
   else
      vertPos = -1;
  if (lon - bbox->getMaxLon() >= 0) 
      horPos = 1;
   else if (lon - bbox->getMinLon() > 0)
      horPos = 0;
   else
      horPos = -1; 
}


bool      
GfxUtility::isSegmentInMC2BoundingBox(const MC2BoundingBox* bbox, int32 y1,
                                      int32 x1, int32 y2, int32 x2)
{
   if ((x1 == x2) && (y1 == y2)) {
      MC2ERROR("GfxUtility::isSegmentInMC2BoundingBox Input is not a segment, only a point!");
      return false;
   }
   // Partiton the xy-plane into 9 rectangles (8 unbounded)
   int vPos1, hPos1, vPos2, hPos2;
   // find which adjacent rectangle (y1, x1) and (y2, x2) belong to.
   // (vPos,Pos) = (1,1) means upper right,
   // (vPos,hPos) = (-1,-1) means lower left,
   // (vPos,hPos) = (0,0) means inside the bounding box itself, etc.
   getAdjacentRectangle(bbox, y1, x1, vPos1, hPos1);
   getAdjacentRectangle(bbox, y2, x2, vPos2, hPos2);
   
   // one point inside bbox
   if ((bbox->inside(y1,x1)) || (bbox->inside(y2,x2)))
      return true;
   
   // both points above, below, to the left or to the right of bbox
   else if ( ((vPos1 == vPos2) && (vPos1 != 0)) ||
             ((hPos1 == hPos2) && (hPos1 != 0))    )
      return false;
   
   // Horizontal or vertical line through box
   else if ( (vPos1 == vPos2) ||
             (hPos1 == hPos2))
      return true;
   else {
      // most difficult case, must check intersections
      
      // points to compare with (y1,x1) and (y2,x2) later
      int32 pLat, pLon, qLat, qLon;           
      int16 prod = (int16)vPos1*hPos1;
      if (prod > 0) {
         pLat = bbox->getMaxLat();
         pLon = bbox->getMinLon();
         qLat = bbox->getMinLat();
         qLon = bbox->getMaxLon();
      }
      else if (prod < 0) {
         pLat = bbox->getMinLat();
         pLon = bbox->getMinLon();
         qLat = bbox->getMaxLat();
         qLon = bbox->getMaxLon();
      }
      else {
         if (hPos1 == 0) {
            if (abs(bbox->getMinLat() - y1) < abs(bbox->getMaxLat() - y1))
               pLat = bbox->getMinLat();
            else 
               pLat = bbox->getMaxLat();
            pLon = bbox->getMinLon();
            qLat = pLat;
            qLon = bbox->getMaxLon();
         }
         else {
            // vPos1 = 0
            if (abs(bbox->getMinLon() - x1) < abs(bbox->getMaxLon() - x1))
               pLon = bbox->getMinLon();
            else
               pLon = bbox->getMaxLon();
            pLat = bbox->getMinLat();
            qLon = pLon;
            qLat = bbox->getMaxLat();
         }
      }
      // check if we have intersection.
      int64 crossprod1 = (int64)(qLon - pLon)*(y2 - qLat) -
         (int64)(x2 - qLon)*(qLat - pLat);
      int64 crossprod2 = (int64)(qLon - pLon)*(y1 - qLat)-
         (int64)(x1 - qLon)*(qLat - pLat);
      if ( ((crossprod1 >= 0) && (crossprod2 >= 0)) ||
           ((crossprod1 <= 0) && (crossprod2 <= 0)))
         return false;
      else {
         crossprod1 = (int64)(x2 - x1)*(pLat - y2)-
            (int64)(pLon - x2)*(y2 - y1);
         crossprod2 = (int64)(x2 - x1)*(qLat - y2)-
            (int64)(qLon - x2)*(y2 - y1);
         if ( ((crossprod1 >= 0) && (crossprod2 >= 0)) ||
              ((crossprod1 <= 0) && (crossprod2 <= 0)))
            return false;
         else
            return true;
      }
   }
}

void 
GfxUtility::getDisplaySizeFromBoundingbox( MC2BoundingBox& bbox,
                                           uint16& width,
                                           uint16& height )
{
   int32 minLat = bbox.getMinLat();
   int32 minLon = bbox.getMinLon();
   int32 maxLat = bbox.getMaxLat();
   int32 maxLon = bbox.getMaxLon();

   GfxUtility::getDisplaySizeFromBoundingbox( minLat, minLon, 
                                              maxLat, maxLon,
                                              width, height );
   bbox.setMinLat( minLat );
   bbox.setMinLon( minLon );
   bbox.setMaxLat( maxLat );
   bbox.setMaxLon( maxLon );
}


void
GfxUtility::getDisplaySizeFromBoundingbox( int32& minLat,
                                           int32& minLon,
                                           int32& maxLat,
                                           int32& maxLon,
                                           uint16& width,
                                           uint16& height )
{
   MC2BoundingBox bbox( maxLat, minLon,
                     minLat, maxLon );

   // width and height should have same proportions as 
   // bbox.width and bbox.height
   float64 bboxHeight = bbox.getHeight();
   float64 bboxWidth = bbox.getWidth();
   if ( bboxHeight == 0.0 ) {
      bboxHeight = 1.0;
   }
   if ( bboxWidth == 0.0 ) {
      bboxWidth = 1.0;
   }
   float64 factor = bboxHeight / bboxWidth * width / height;
   if ( factor < 1 ) {
      // Compensate for that the display is higher than the bbox
//      height = uint16( height * factor );
      int32 extraHeight = 
         int32( rint( ( (bboxHeight / factor ) - 
                        bboxHeight ) / 2 ) );
      minLat -= extraHeight;
      maxLat += extraHeight;
      bbox.setMinLat( minLat );
      bbox.setMaxLat( maxLat );
   } else {
      // Compensate for that the display is wider than the bbox
//      width = uint16( width / factor );
      uint32 lonDiff = bbox.getLonDiff();
      if ( lonDiff == 0 ) {
         lonDiff = 1;
      }
      int32 extraWidth = 
         int32( rint( ( (lonDiff * factor ) - 
                        lonDiff ) / 2 ) );
      minLon -= extraWidth;
      maxLon += extraWidth;
      bbox.setMinLon( minLon );
      bbox.setMaxLon( maxLon );
   }

   factor = bboxHeight / bboxWidth * width / height;
}


void 
GfxUtility::getArcAspectCoordinates( 
         int32 minLat,
         int32 minLon,
         int32 maxLat,
         int32 maxLon,
         uint32 width,
         uint32 height,
         int32 lat, int32 lon,
         uint16 innerRadius, uint16 outerRadius,
         uint16 startAngle, uint16 stopAngle,
         int& cx, int& cy,
         int& iR, int& oR )
{
   MC2BoundingBox bbox( maxLat, minLon,
                     minLat, maxLon );   

   float64 xScale = float64( width - 1) / bbox.getLonDiff();
   float64 yScale = float64( height - 1) / bbox.getHeight();
   
   // Make sure that the image will not have strange proportions.
   float64 factor = float64( bbox.getHeight()) / bbox.getWidth()
      * (width - 1) / (height - 1);
   if (factor < 1) {
      // Compensate for that the image is wider than it is high
      yScale *= factor;
   } else {
      // Compensate for that the image is higher than it is wide
      xScale /= factor;
   }

   // Center
   cx = int( rint( xScale * ( lon - bbox.getMinLon() ) ) );
   cy = int( rint( yScale * ( lat - bbox.getMinLat() ) ) );

   // Radius
   iR = int( rint( xScale * innerRadius ) );
   oR = int( rint( yScale * outerRadius ) );
}


void 
GfxUtility::calcArrowPoints( POINT* src, POINT* dst, POINT* arrow,
                             int32 cornerDistance, 
                             int32 shortDistance, 
                             int32 longDistance,
                             bool addCenterPoint,
                             int32 arrowLength,
                             int32 angle,
                             bool useCosLat )
{
   DEBUG4(cerr << "calcArrowPoints " 
          << src[0].x << "," << src[0].y << ","
          << src[1].x << "," << src[1].y << ","
          << src[2].x << "," << src[2].y << endl);

   // Calc the acording cos lat factor.
   double cosLat = useCosLat? cos( (2*M_PI/ POW2_32 * (src[1].y))) : 1.0; 

   // Translate so that src[1] is in origo.
   double v1x = (src[0].x - src[1].x)*cosLat;
   double v1y = src[0].y - src[1].y;
   double v2x = (src[2].x - src[1].x)*cosLat;
   double v2y = src[2].y - src[1].y;

   // Normalize vector 1 and 2
   // Note! the value steps are needed since large values will make sqrt return invalid
   // results if the calculations are done inside the function call.
   double value = SQUARE( v1x ) + SQUARE( v1y );
   double length1 = sqrt( value );
   value = SQUARE( v2x ) + SQUARE( v2y );
   double length2 = sqrt( value );

   // Normalized vector 
   double e1x = v1x/length1;
   double e1y = v1y/length1;
   double e2x = v2x/length2;
   double e2y = v2y/length2;
      
   // Get the intersecton vector.
   double v3x = e1x + e2x;
   double v3y = e1y + e2y;
   value = SQUARE( v3x ) + SQUARE( v3y );
   double length3 = sqrt( value );

   double e3x = 0;
   double e3y = 0;
   if( length3 == 0 ) {
      // They intersect at src[1], e.i. strait line.
      e3x = 0.5;
      e3y = 0.5;
   } else {
      e3x = v3x/length3;
      e3y = v3y/length3;
   }
    
   // Translated corner point.
   double p3x = e3x*cornerDistance + src[1].x*cosLat;
   double p3y = e3y*cornerDistance + src[1].y;

   int i=0;
   dst[i].x   = (int)(( p3x + e1x*longDistance)/cosLat + 0.5);
   dst[i++].y = (int)( p3y + e1y*longDistance);
   dst[i].x   = (int)(( p3x + e1x*shortDistance)/cosLat + 0.5);
   dst[i++].y = (int)( p3y + e1y*shortDistance);

   if( addCenterPoint ){
      dst[i].x   = (int)p3x;
      dst[i++].y = (int)p3y;
   }

   dst[i].x   = (int)(( p3x + e2x*shortDistance)/cosLat + 0.5);
   dst[i++].y = (int)( p3y + e2y*shortDistance);
   dst[i].x   = (int)(( p3x + e2x*longDistance)/cosLat + 0.5);
   dst[i++].y = (int)( p3y + e2y*longDistance);

   double tanAngle = tan((M_PI*angle)/180);
   double p2xPrim = e2y*tanAngle;
   double p2yPrim = e2x*tanAngle;
   double v4x = p2xPrim - e2x;
   double v4y = p2yPrim - e2y;
   value = SQUARE( v4x ) + SQUARE( v4y );
   double length4 = sqrt( value );
   double e4x = v4x/length4;
   double e4y = v4y/length4;

   double v5x = -p2xPrim - e2x;
   double v5y = -p2yPrim - e2y;
   value = SQUARE( v5x ) + SQUARE( v5y );
   double length5 = sqrt( value );
   double e5x = v5x/length5;
   double e5y = v5y/length5;

   arrow[0].x = (int32)(dst[i-1].x + e4x*arrowLength);
   arrow[0].y = (int32)(dst[i-1].y + e4y*arrowLength);
   arrow[1].x = dst[i-1].x;
   arrow[1].y = dst[i-1].y;
   arrow[2].x = (int32)(dst[i-1].x + e5x*arrowLength);
   arrow[2].y = (int32)(dst[i-1].y + e5y*arrowLength);
}


// Help function to makeSpline
float64 b( int i, float64 t ) {
   switch ( i ) {
      case 0:
         return (1-t)*(1-t)*(1-t);
      case 1:
         return 3*t*(1-t)*(1-t);
      case 2:
         return 3*t*t*(1-t);
      case 3:
         return t*t*t;
   }
   return 0.0; // We only get here if i is invalid
}


float64 
GfxUtility::calcRadius( int32 lat0, int32 lon0,
                        int32 lat1, int32 lon1,
                        int32 lat2, int32 lon2 )
{
   // Radius = a*c / (2*h)
   // where h = c*sin(alpha) 
   //
   // => Radius = a / (2 * sin(alpha))


   //         
   //         c        
   // A ------------- B
   //   \ )alpha   /
   //    \        /
   //     \      /
   //   b  \    / a
   //       \  / 
   //        \/
   //         C
   //
   // h is the normal from b, heading to point B.
   //
   // Let A = (lat0, lon0)
   //     B = (lat1, lon1)
   //     C = (lat2, lon2)
   
   float64 cosLat = cos( (2*M_PI/ POW2_32 * ((lat2/2)+(lat1/2))));
   float64 a = sqrt( SQUARE(lat2-lat1) + SQUARE((lon2-lon1)*cosLat) );

   float64 alpha = GfxUtility::getAngleFromNorth(lat0, lon0, lat2, lon2) -
                   GfxUtility::getAngleFromNorth(lat0, lon0, lat1, lon1);

   float64 sinAlpha = sin(alpha);
   
   if (sinAlpha != 0) {
      return ( a / (2 * sinAlpha) );
   } else {
      return (MAX_FLOAT64);
   }
      
}


void
GfxUtility::makeSpline( int x0, int y0, int x1, int y1, 
                        int x2, int y2, int x3, int y3,
                        vector<int>& resX, vector<int>& resY,
                        float32 bias,
                        double* endAngle )
{
   int nbrPoints = int( ceil( 1/bias ) + 1 );
   resX.reserve( nbrPoints );
   resY.reserve( nbrPoints );

   int controlX[4];
   int controlY[4];      

   float64 t = 0.0; // How far we are into the spline
   
   controlX[ 0 ] = x0;
   controlX[ 1 ] = x1;
   controlX[ 2 ] = x2;
   controlX[ 3 ] = x3;

   controlY[ 0 ] = y0;
   controlY[ 1 ] = y1;
   controlY[ 2 ] = y2;
   controlY[ 3 ] = y3;

   float64 px=0.0;
   float64 py=0.0;
   do {
      px=0.0;
      py=0.0;
      for ( int j = 0; j < 4 ; j++ ) {
         px += b( j, t ) * controlX[ j ];
         py += b( j, t ) * controlY[ j ];
      }
      resX.push_back( int( rint( px ) ) );
      resY.push_back( int( rint( py ) ) );

      t += bias;
   } while ( t <= 1.0 );

   if ( endAngle != NULL ) {   
      float64 px2=0.0;
      float64 py2=0.0;
      t = 1.0;
      for ( int j = 0; j < 4 ; j++ ) {
         px2 += b( j, t ) * controlX[ j ];
         py2 += b( j, t ) * controlY[ j ];
      }
      *endAngle = atan2( py - py2, px2 - px );
      if ( *endAngle < 0.0 ) {
         *endAngle += M_PI*2;
      }
   }
}


// **************************************
// *   Functions for clipping polygons  *
// **************************************
bool
GfxUtility::clipPolyToBBoxFast( const MC2BoundingBox* bbox, 
                                vector<POINT>& vertices )
{
   
   uint32 nbrVertices = vertices.size();

   if (nbrVertices < 3) {
      return (false);
   }
  
 
   vector<byte> outcodes1;
   outcodes1.reserve(nbrVertices);
   // Calculate the outcodes.
   for ( vector<POINT>::const_iterator it = vertices.begin();
         it != vertices.end(); ++it ) {
      outcodes1.push_back( 
            bbox->getCohenSutherlandOutcode( it->y, it->x ) );
   }

   vector<byte> outcodes2;
   vector<POINT> vertices2;

   // Clip using Sutherland-Hodgeman clipping.
   // Clip against a bbox boundary and feed the output as input when
   // clipping against the next bbox boundary...
   
   // Clip to left boundary
   clipToBoundary(MC2BoundingBox::LEFT, bbox, vertices, outcodes1,
                  vertices2, outcodes2);
   
   // Clip to right boundary
   clipToBoundary(MC2BoundingBox::RIGHT, bbox, vertices2, outcodes2,
                  vertices, outcodes1);
    
   // Clip to top boundary
   clipToBoundary(MC2BoundingBox::TOP, bbox, vertices, outcodes1,
                  vertices2, outcodes2);
    
   // Clip to bottom boundary
   bool retVal = 
      clipToBoundary(MC2BoundingBox::BOTTOM, bbox, vertices2, outcodes2,
                     vertices, outcodes1);

   return (retVal);
}
                     
#define ASSERT_OR_DUMP( x ) if ( x ) {} else { dump( clipPolygon, originalSubjPoly); MC2_ASSERT( x ); } 

static void
dump( const vector<POINT>& clip, const vector<POINT>& subj ) 
{
   cerr << "WILL CRASH! DUMPING ORIGINAL IN DATA:" << endl;
   cerr << "cat > clip.gnu << EOF" << endl;
   
   for ( uint32 i = 0; i < clip.size(); ++i ) {
      cerr << clip[ i ].x << " " << clip[ i ].y << endl;
   }
   cerr << "EOF" << endl;
   
   cerr << "cat > subj.gnu << EOF" << endl;

   for ( uint32 i = 0; i < subj.size(); ++i ) {
      cerr << subj[ i ].x << " " << subj[ i ].y << endl;
   }
   cerr << "EOF" << endl;
   cerr << "gnuplot" << endl;
   cerr << "plot \"clip.gnu\" with linespoints, \"subj.gnu\" with linespoints";
   cerr << endl; 
}

//#define DEBUG_CLIPPING

#ifdef DEBUG_CLIPPING
   #define clipdbg mc2dbg
#else
   #define clipdbg mc2dbg8
#endif   

static inline int
insertIntersections( VertexList& subjectPoly,
                     VertexList& clipPoly )
{
   int nbrIntersect = 0;
   // clipIsSubj means that we are looking for intersections
   // between the polygon and itself
   VertexList::iterator sIt = subjectPoly.begin();
   VertexList::iterator prevSIt = sIt;
   ++sIt;
   int forbiddenAlpha = 1;
   while ( sIt != subjectPoly.end() ) {
      VertexList::iterator cIt = clipPoly.begin();
      VertexList::iterator prevCIt = cIt;
      ++cIt;
      if ( &subjectPoly == &clipPoly ) {
         forbiddenAlpha = 1000;
         // Same subject and clip. We are looking for intersections
         // within a polygon. Do not consider adjacent lines
         cIt = sIt;
         ++cIt;
         if ( cIt == clipPoly.end() ) {
            // Done
            break;
         }
         prevCIt = cIt;
         ++cIt;
         if ( cIt == clipPoly.end() ) {
            // Done
            break;
         }
      }
//#undef clipdbg
//#define clipdbg mc2dbg 
      while ( cIt != clipPoly.end() ) {
         int32 intersectX = MAX_INT32; 
         int32 intersectY = MAX_INT32;
         float64 clipAlpha = 0; // percent from first clip vertex.
         float64 subjAlpha = 0; // percent from first subj vertex.
         if ( GfxUtility::getIntersection( (*prevSIt)->x, (*prevSIt)->y,
                                           (*sIt)->x, (*sIt)->y,
                                           (*prevCIt)->x, (*prevCIt)->y,
                                           (*cIt)->x, (*cIt)->y,
                                           intersectX, intersectY,
                                           subjAlpha,
                                           clipAlpha ) && (clipAlpha != forbiddenAlpha) ) {
            clipdbg << "Intersection found at " << intersectX << ", "
                    << intersectY << endl;
            clipdbg << "Subject alpha = " << subjAlpha << endl;
            clipdbg << "Clip alpha = " << clipAlpha << endl;
            clipdbg << "Subj: " << (*prevSIt)->x << ", "
                    << (*prevSIt)->y << " - " 
                    << (*sIt)->x << ", " << (*sIt)->y
                    << endl;
            clipdbg << "Clip: " << (*prevCIt)->x << ", "
                    << (*prevCIt)->y << " - " 
                    << (*cIt)->x << ", " << (*cIt)->y
                    << endl;
//#undef clipdbg
//#define clipdbg mc2dbg8
            // A false intersection can occur when a line which goes
            // straight out and straight in again through the clip area.
            // The first time the line is clipped, one clip intersection
            // is added.
            // When the line comes back, it will intersect both:
            // (pointA - intersection) and (intersection - pointB),
            // leading to 3 (odd) numbers of intersections.
            // This will lead to loops until the memory runs out.
            //
            // We hope that the false intersections are excluded since
            // intersections with alpha exactly 1 are not considered
            // as intersections (since there should be an intersection
            // with alpha 0 also).
            POINT intersectPoint;
            intersectPoint.x = intersectX;
            intersectPoint.y = intersectY;
            
            Vertex* subjVertex = new Vertex( intersectPoint ); 	 
            Vertex* clipVertex = new Vertex( intersectPoint ); 	 
            subjVertex->m_intersect = true; 	 
            clipVertex->m_intersect = true; 	 
            
            // Set alpha value. 	 
            subjVertex->m_alpha = subjAlpha; 	 
            clipVertex->m_alpha = clipAlpha; 	 
            
            // Link the two intersection points and add to the lists. 	 
            clipVertex->m_neighbor = 	 
               subjectPoly.insertIntersectVertex( prevSIt, sIt, subjVertex );
            subjVertex->m_neighbor = 	 
               clipPoly.insertIntersectVertex( prevCIt, cIt, clipVertex );
            ++nbrIntersect;
         }

         prevCIt = cIt;
         ++cIt;
      }
      prevSIt = sIt;
      ++sIt;
   }
   return nbrIntersect;
}


//#define CSI_DEBUG 1
#undef csi_dbg
#ifdef CSI_DEBUG
#define CSI_DUMP_IF_NOFAIL 1
#define csi_cout cout
#define csi_dbg mc2dbg
#else
#define CSI_DUMP_IF_NOFAIL 0
#define csi_dbg mc2dbg8
#define csi_cout mc2dbg8
#endif

static void 
advanceCircular( VertexList& subjectPoly,
                 VertexList::iterator& subjIt,
                 int dir )
{
   if ( dir < 0 ) {
      if ( subjIt == subjectPoly.begin() ) {
         subjIt = subjectPoly.end();
      }
   }
   std::advance( subjIt, dir );
   if ( dir > 0 ) {
      if ( subjIt == subjectPoly.end() ) {
         subjIt = subjectPoly.begin();
      }
   }
}

static VertexList::iterator 
moveToNextCoord( VertexList& list, VertexList::iterator it, int dir )
                 throw (MC2Exception)
{
   VertexList::iterator startIt = it;

   while ( **it == **startIt ) {
      advanceCircular( list, it, dir );
      
      if ( it == startIt ) {
         // This means we looped and has happened 
         // that we didn't think of.
         throw MC2Exception( "GfxUtil::moveToNextCoord loops! "
                             "runBin would have restarted me" );
      }
   }
   return it;
}

static float
calcTurnAngle( const POINT& prevSubj,
               const POINT& cur,
               const POINT& nextNeighbor,
               bool clockwise ) 
{
   // Calc angle.
   float angle = GfxUtil::calcAngle( prevSubj, cur, nextNeighbor );

   // The output from calcAngle is an angle that increases ccw.
   // Or if it happens so, that calcAngle returns 0, then we would like 360,
   // adhering to: "Too little data, so I make big".
   if ( clockwise || angle == 0 ) {
      // Adjust the angle so that it increases clockwise.
      angle = 360 - angle;
   }
   csi_dbg << "[GfxUtility]: Angle between "
           << prevSubj << ", " << cur << ", " << nextNeighbor
           << " is " << angle << endl;

   return angle;
}

/// Move to next outline.
static VertexList::iterator
moveToNextOutlineVertex( VertexList& subjectPoly,
                         const VertexList::iterator& subjItIn, 
                         bool clockwise, 
                         int& dir ) throw (MC2Exception)
{
    VertexList::iterator subjIt = subjItIn;
   if ( true || (*subjIt)->m_intersect ) {
      // The previous, real coordinate.
      VertexList::iterator prevSubjIt = 
         moveToNextCoord( subjectPoly, subjIt, -dir );
      
      // The first of many intersecting coordinates.
      VertexList::iterator it = subjIt;
      
      // The next one.
      VertexList::iterator nextIt =
         moveToNextCoord( subjectPoly, subjIt, dir );

      // First check the angle between the previous and next one
      // and let this be the best candidate.
      VertexList::iterator outlineIt = nextIt;
      // Calc angle.
      float minAngle = calcTurnAngle( **prevSubjIt, **it, **nextIt, clockwise );
      
      const int origDir = dir;
      // Go through the list as long as we have the same coordinate.
      for ( ; POINT(**it) == POINT(**subjIt); 
            advanceCircular( subjectPoly, it, origDir ) ) {
         // not Temporary
         if ( ! (*it)->m_intersect ) {
            continue;
         }
         MC2_ASSERT( (*it)->m_intersect );

         // Switch to neighbor.
         VertexList::iterator neighbor = (*it)->m_neighbor;

         // Try both directions for the neighbor.
         for ( int i = 0; i < 2; ++i ) {
            int neighborDir = (i == 0 ? -1 : 1);
            VertexList::iterator nextNeighbor = 
               moveToNextCoord( subjectPoly, neighbor, neighborDir );

            // Calc angle.
            float angle = calcTurnAngle( **prevSubjIt, **it, **nextNeighbor, clockwise );

            // The outline of the polygon is desired.
            // The angle increases accordingly to the clockwiseness, 
            // so the best candidate will have the smallest angle.
            if ( angle < minAngle ) {
               // Update.
               outlineIt = nextNeighbor;
               minAngle = angle;
               dir = neighborDir;
            }
         }
      }
      if ( outlineIt != nextIt ) {
         csi_dbg << "[GfxUtility]: Switching poly "
                << **subjIt << " -> " << **outlineIt << endl;
         csi_dbg << "[GfxUtility]: Switching dir "
                << origDir << " -> " << dir << endl;
         csi_dbg << "[GfxUtility]: Min angle = " << minAngle << endl;
      } else {
         csi_dbg << "[GfxUtility]: Not switching "
                 << **subjIt << endl;
      }  
      csi_dbg << "[GfxUtility]: prevSubjIt = " << **prevSubjIt << endl;
      csi_dbg << "[GfxUtility]: nextIt = " << **nextIt << endl;
      return outlineIt;
   } else {
      csi_dbg << "[GfxUtility]: Not switching (OLD) "
              << **subjIt << endl;
      advanceCircular( subjectPoly, subjIt, dir );
   }         
   return subjIt;
}

void addUniquePoints( const vector< POINT >& vertices,
                      VertexList& subjectPoly ) {
   POINT lastPoint;
   for ( vector<POINT>::const_iterator it = vertices.begin();
         it != vertices.end(); ++it ) {
      if ( it == vertices.begin() || lastPoint != *it ) {
         // Skip repeated points.
         subjectPoly.push_back( new Vertex( *it ) );
         lastPoint = *it;
      }
   }
}

int
GfxUtility::cleanSelfintersecting( vector< vector<POINT> >& polygons ) 
                                   throw (MC2Exception)
{
   
   csi_dbg << "[GfxUtility::cleanSelfintersecting]: ------- " << endl;
   // If failed - dump
   bool failed = false;
   bool dump  = false;
   vector< vector<POINT> > res;
   for ( vector<vector<POINT> >::const_iterator it = polygons.begin();
         it != polygons.end();
         ++it ) {
      VertexList subjectPoly;
      // Reference to the original polygons.
      const vector<POINT>& vertices1 = *it;
      vector<POINT> vertices( vertices1.begin(), vertices1.end() );
      vertices.resize( std::distance( vertices.begin(),
                                      std::unique( vertices.begin(),
                                                   vertices.end() ) ) );
     
      // Create subject polygon from the vertices.
      ::addUniquePoints( vertices, subjectPoly );

      if ( *subjectPoly.back() != *subjectPoly.front() ) {
         subjectPoly.push_back( new Vertex( POINT(*subjectPoly.front() ) ) );
      }

      // size_t origSize = subjectPoly.size();
      csi_dbg << subjectPoly << endl;
      csi_cout << GnuPlotDump::octave_dump( subjectPoly.begin(),
                                            subjectPoly.end() );
      
      // Insert the intersections
      int nbrIntersect = 0;
      nbrIntersect += insertIntersections( subjectPoly,
                                           subjectPoly );

      csi_dbg << "After intersections." << endl;
      csi_dbg << subjectPoly << endl;
      csi_cout << GnuPlotDump::octave_dump( subjectPoly.begin(),
                                            subjectPoly.end() );
      
      csi_dbg << "[GfxUtility]: Number of intersections " << nbrIntersect
             << endl;
      // First version will only copy back the points after insertions are
      // added
      res.push_back( vector<POINT>() );

      vector<POINT>& polyToAdd = res.back();
      // More difficult loop
      VertexList::iterator subjIt = 
         GfxUtil::getNorthestCorner( subjectPoly.begin(),  
                                     subjectPoly.end() );

      // Clockwiseness is checked on the vertices since does not contain
      // consecutive duplicate points.
      int clockwiseRes = GfxUtil::isClockWise( vertices.begin(),
                                               vertices.end() );
                                               
      MC2_ASSERT( clockwiseRes >= 0 );
      csi_dbg << "Clockwise result (int) = " << clockwiseRes << endl;
      bool clockwise = clockwiseRes > 0;
      
      int dir = 1;

      // Move back to the next real coordinate.
      VertexList::iterator stopIt = 
         moveToNextCoord( subjectPoly, subjIt, -dir );
      // And go forward again so that stopIt will be a possible 
      // output from moveToNextOutlineVertex.
      stopIt = moveToNextCoord( subjectPoly, stopIt, dir ); 
      subjIt = stopIt;
      
      
      bool first = true;
      size_t maxSize = subjectPoly.size();
      while ( first || subjIt != stopIt ) {
         first = false;
         polyToAdd.push_back( **subjIt );

         subjIt = moveToNextOutlineVertex( subjectPoly, subjIt,
                                           clockwise, dir );
         if ( polyToAdd.size() > maxSize ) {
            mc2log << error
                   << "[GfxUtility]: cleanSelfintersecting failed" << endl;
            failed = true;
            dump = true;
            break;
         }
      }

      if ( CSI_DUMP_IF_NOFAIL || dump ) {
         // Something was wrong.
         vector<POINT> polyToDraw = polyToAdd;
         if ( polyToDraw.front() != polyToDraw.back() ) {
            polyToDraw.push_back( polyToDraw.front() );
         }
         if ( failed ) {
            mc2log << error << "[GfxUtility]: Dumping failed poly" << endl;
         }
         // Intersections were inserted.
         cout << "cat > after.gnu << EOF" << endl;
         cout << GnuPlotDump::gp_vec_dump( polyToDraw.begin(),
                                           polyToDraw.end() ) << endl;
         cout << "EOF" << endl;
         cout << "cat > before.gnu << EOF" << endl;
         cout << GnuPlotDump::gp_vec_dump( vertices.begin(),
                                           vertices.end() ) << endl;
         cout << "EOF" << endl;
         cout << "gnuplot" << endl;
         cout << "plot \"before.gnu\" with vectors, "
              << "\"after.gnu\" with vectors";
         cout << endl << flush;
         // Make it possible to detect the next one, if any
         dump = false;

         cout << GnuPlotDump::octave_dump( polyToDraw.begin(),
                                           polyToDraw.end() );
      }
   }

   if ( failed ) {
      throw MC2Exception( "cleanSelfIntersection failed. runBin would have restarted me." );
   }
                          
   
   res.swap( polygons );
   
   csi_dbg << "[GfxUtility::cleanSelfintersecting]: --ENDS- " << endl;
   return res.size();
}

struct convertPointMC2 {
public:
   MC2Coordinate operator()( const POINT& p ) const {
      return MC2Coordinate( p.y, p.x );     
   }

   POINT operator()( const MC2Coordinate& c ) const {
      return makePoint( c.lon, c.lat );
   }
};

static void pointsToMC2( vector<vector<MC2Coordinate> > & coords,
                         const vector<vector<POINT> >& points )
{
   typedef vector<MC2Coordinate> coordVect_t;
   typedef vector<coordVect_t> coordVectVect_t;
   typedef vector<POINT> pointVect_t;
   typedef vector<pointVect_t> pointVectVect_t;
   
   coords.clear();
   coords.reserve( points.size() );

   for ( pointVectVect_t::const_iterator it = points.begin();
         it != points.end();
         ++it ) {
      coords.push_back( coordVect_t() );
      coordVect_t& back = coords.back();
      back.reserve( it->size() );
      std::transform( it->begin(), it->end(),
                      back_insert_iterator<coordVect_t>( back ),
                      convertPointMC2() );
   }

}

static void mc2ToPoints( vector<vector<POINT> >& points,
                         const vector<vector<MC2Coordinate> > & coords )
{

   typedef vector<MC2Coordinate> coordVect_t;
   typedef vector<coordVect_t> coordVectVect_t;
   typedef vector<POINT> pointVect_t;
   typedef vector<pointVect_t> pointVectVect_t;
   
   points.clear();
   points.reserve( coords.size() );
   // Move from MC2Coordinate to POINT
   for ( coordVectVect_t::const_iterator it = coords.begin();
         it != coords.end();
         ++it ) {
      points.push_back( pointVect_t() );
      pointVect_t& back = points.back();
      back.reserve( it->size() );
      std::transform( it->begin(), it->end(),
                      back_insert_iterator<pointVect_t>( back ),
                      convertPointMC2() );
   }
}


int
GfxUtility::cleanSelfintersecting( vector<vector<MC2Coordinate> >& coords )
                                   throw (MC2Exception)
{
   // Blargh! Could probably cast pointers...
   
   vector<vector<POINT> > points;

   // Copy to point vector
   mc2ToPoints( points, coords );

   int res = cleanSelfintersecting( points );

   // Copy to coords
   pointsToMC2( coords, points );

   return res;
}

/// Removes vertices from intersection clouds that are not contributing.
static void
cleanIntersections( VertexList& subjectPoly ) throw (MC2Exception)
{
   if ( subjectPoly.empty() ) {
      return;
   }
   int dir = 1;

   // Move back to the next real coordinate.
   VertexList::iterator begin = 
      moveToNextCoord( subjectPoly, subjectPoly.begin(), -dir );

   bool firstTime = true;
   VertexList::iterator stopIt = begin;
   
   // We break out of the loop.
   while ( true ) {
   
      // Set the begin and end iterators to the next chunk.
      begin = moveToNextCoord( subjectPoly, begin, dir ); 
      VertexList::iterator end = moveToNextCoord( subjectPoly, begin, dir );

      // Bad: Intersections that whose neighbor is in the same cloud as this one.
      list<VertexList::iterator> badIntersections;
      // Good: Intersections that whose neighbor is in other cloud.
      list<VertexList::iterator> goodIntersections;
      // These are not intersections.
      list<VertexList::iterator> nonIntersections;

      set<Vertex*> connections;
      
      for ( VertexList::iterator it = begin; it != end; 
            advanceCircular( subjectPoly, it, dir ) ) {
         if ( (*it)->m_intersect ) {
            // Check if the intersection leads to anything useful.
            VertexList::iterator nextIt = 
               moveToNextCoord( subjectPoly, (*it)->m_neighbor, dir );
            if ( nextIt == end ) {
               // Not a useful intersection since it is only connecting 
               // inside this cloud.
               badIntersections.push_back( it );
            } else {
               // Connects to something outside this cloud.
               // Check that it doesn't connect to something we already have a 
               // connection to.
               if ( connections.find( *nextIt ) == connections.end() ) {
                  // Leads to a new path. Keep it!
                  goodIntersections.push_back( it );
                  connections.insert( *nextIt );
               } else {
                  // Didn't lead to a new path. Mark as bad.
                  badIntersections.push_back( it );
               }
            }
         } else {
            // Store the non intersections.
            nonIntersections.push_back( it );
         }
      }


      // All bad neighbors must not intersect.
      for ( list<VertexList::iterator>::iterator it = badIntersections.begin();
             it != badIntersections.end(); ++it ) {
         (*(**it)->m_neighbor)->m_intersect = false;
      }
      
      // If no good intersections, keep one non-intersection, 
      // otherwise remove them.
      if ( goodIntersections.empty() ) {
         if ( ! nonIntersections.empty() ) {
            // Use one of the non intersections if present.
            begin = nonIntersections.front();
            nonIntersections.pop_front();
         } else {
            // Reuse one of the bad intersections, 
            // so that we have something at least.
            MC2_ASSERT( ! badIntersections.empty() );
            begin = badIntersections.front();
            // Make bad intersection non intersection.
            (*begin)->m_intersect = false;
            badIntersections.pop_front();
         }
      } else {
         begin = goodIntersections.front();   
      }

      // Remove the bad ones.
      for ( list<VertexList::iterator>::iterator it = badIntersections.begin();
             it != badIntersections.end(); ++it ) {
         delete **it;
         subjectPoly.erase( *it );
      }

      // Remove duplicate non intersections.
      for ( list<VertexList::iterator>::iterator it = nonIntersections.begin();
            it != nonIntersections.end(); ++it ) {
         delete **it;
         subjectPoly.erase( *it );
      }

      // Remember where we start, and if we get back again, break. 
      if ( firstTime ) {
         stopIt = begin;
         firstTime = false;
      } else if ( stopIt == begin ) {
         // End eternal loop.
         break;
      }
   } 
}

static void
getCloudRange( VertexList& subjectPoly, 
               VertexList::iterator curIt,
               VertexList::iterator& from,
               VertexList::iterator& to ) throw (MC2Exception)
{
   MC2_ASSERT( (*curIt)->m_intersect );
   
   // Backwards.
   from = moveToNextCoord( subjectPoly, curIt, -1 );
   advanceCircular( subjectPoly, from, 1 );

   // Forward.
   to = moveToNextCoord( subjectPoly, curIt, 1 );
}

/**
 *    Returns true if curIt and otherIt are intersections
 *    in two clouds of intersections that point to each other
 *    if so sets <code>from</code> and <code>to</code> to the
 *    two intersections connecting the clouds.
 *    @param subjectPoly The current polygon
 *    @param curIt       An iterator in subjectPoly
 *    @param otherIt     Another iterator in subjectPoly
 *    @param from        Output parameter that will contain the
 *                       iterator that point from one cloud to the other.
 *    @param to          The intersection corresponding to <code>from</code>.
 *    @return True if the two clouds point to eachother. <code>from</code> and
 *            <code>to</code> will be invalid if the return value is false.
 */
static bool
cloudContaining( VertexList& subjectPoly, 
                 VertexList::iterator curIt, 
                 VertexList::iterator otherIt, 
                 VertexList::iterator& from, 
                 VertexList::iterator& to ) throw (MC2Exception)
{  
   if ( ! (*curIt)->m_intersect || ! (*otherIt)->m_intersect ) {
      return false;
   }

   VertexList::iterator curFrom;
   VertexList::iterator curTo;
   getCloudRange( subjectPoly, curIt,
                  curFrom, curTo );
  
   VertexList::iterator otherFrom;
   VertexList::iterator otherTo;
   getCloudRange( subjectPoly, otherIt,
                  otherFrom, otherTo );
  
   // Check things.
   for ( VertexList::iterator outerIt = curFrom;
         outerIt != curTo; advanceCircular( subjectPoly, outerIt, 1 ) ) {

      for ( VertexList::iterator innerIt = otherFrom;
            innerIt != otherTo; advanceCircular( subjectPoly, innerIt, 1 ) ) {
         
         if ( (*outerIt)->m_intersect && (*outerIt)->m_neighbor == innerIt ) {
            from = outerIt;
            to = innerIt;
            MC2_ASSERT( (*innerIt)->m_intersect && (*innerIt)->m_neighbor == outerIt );
            return true;
         }
      }
   }
   return false;
}

bool
GfxUtility::splitSelfTouching( vector< vector<POINT> >& polygons )
                               throw (MC2Exception)
{
   csi_dbg << "[GfxUtility::splitSelfTouching]: ------- " << endl;
   // If failed - dump
   bool failed = false;
   
   vector< vector<POINT> > res;
   
   for ( vector<vector<POINT> >::const_iterator it = polygons.begin();
         it != polygons.end();
         ++it ) {
      VertexList subjectPoly;
      // Reference to the original polygons.
      const vector<POINT>& vertices1 = *it;
      vector<POINT> vertices( vertices1.begin(), vertices1.end() );
      // Removes duplicate coordinates.
      vertices.resize( std::distance( vertices.begin(),
                                      std::unique( vertices.begin(),
                                                   vertices.end() ) ) );
     
      // Create subject polygon from the vertices.
      ::addUniquePoints( vertices, subjectPoly );

      // Manually close polygon.
      if ( *subjectPoly.back() != *subjectPoly.front() ) {
         subjectPoly.push_back( new Vertex( POINT(*subjectPoly.front() ) ) );
      }

      csi_dbg << "[GU::SST]: Before inserting intersections." << endl;
      csi_dbg << subjectPoly << endl;
      
      // Insert the intersections
      int nbrIntersect = 0;
      nbrIntersect += insertIntersections( subjectPoly,
                                           subjectPoly );

      subjectPoly.renumber();

      csi_dbg << "[GU::SST]: After inserting intersections:" << endl;
      csi_cout << GnuPlotDump::octave_dump( subjectPoly.begin(), subjectPoly.end() );
      csi_dbg << subjectPoly << endl;

      // Get rid of confusing vertices.
      cleanIntersections( subjectPoly );
      
      csi_dbg << "[GU::SST]: After cleaning intersections:" << endl;
      csi_cout << GnuPlotDump::octave_dump( subjectPoly.begin(), subjectPoly.end() );
      csi_dbg << subjectPoly << endl;

      csi_dbg << "[GU::SST]: Number of intersections " << nbrIntersect
              << endl;
                 
      // Split polygons until nothing left to split.
      while ( ! subjectPoly.empty() ) {

         csi_dbg << "[GU::SST]: Start of snipping loop:" << endl;
         
         // Remember where we started
         VertexList::iterator startIt = subjectPoly.begin();
        
         // Iterators to vertices to be snipped
         vector<VertexList::iterator> snipped;
         
         // Iterator that we work with
         VertexList::iterator curIt = startIt;
         
         size_t maxSize = subjectPoly.size();
         
         // Nbr of rewinds since snipping.
         uint32 nbrRewindsSinceLastSnip = 0;
         
         while ( true ) {
            
            // Store all non intersecting iterators.
            // These are candidates to be snipped.
            // Intersections will be handled last.
            if ( ! (*curIt)->m_intersect ) {

               csi_dbg << **curIt;
               snipped.push_back( curIt );

               // Extra check, for a good nights sleep.
               if ( snipped.size() > maxSize ) {
                  mc2log << error
                         << "[GU::SST]: Failed since output is bigger than input!" 
                         << endl;
                  failed = true;
                  break;
               }
            }

            // More checks for good nights sleep.
            // If we have rewinded more than maxSize times since
            // last snip, then we will never be finished.
            if ( nbrRewindsSinceLastSnip > maxSize ) {
               mc2log << error
                      << "[GU::SST]: Failed since " << nbrRewindsSinceLastSnip
                      << " rewinds have occured when input is "
                      << maxSize << " in size." << endl;
               failed = true;
               break;
            }
         
            curIt = moveToNextCoord( subjectPoly, curIt, 1 );

            if ( curIt == startIt ) {
               // Finished.
               csi_dbg << "[GU::SST]: Finished since got back" 
                       << " to start iterator." << endl;
               break;
            }

            if ( (*curIt)->m_intersect ) {

               VertexList::iterator from;
               VertexList::iterator to;
               if ( cloudContaining( subjectPoly, curIt, startIt, from, to ) ) {
                  csi_dbg << "[GU::SST]: Found intersection leading back"
                             " to previous cloud.";
                  csi_dbg << "[GU::SST]: from = " << **from;
                  csi_dbg << "[GU::SST]: to = " << **to;
                  // Keep one of the points.
                  snipped.push_back( to );
                  // Reset intersect flag instead of removing it since:
                  // otherwise the polygon may collapse
                  // and it's neighbour has already been added anyway.
                  // cleanIntersections will make things right afterwards.
                  (*from)->m_intersect = false;
                  break;
               } else {
                  csi_dbg << "[GU::SST]: Found intersection that didn't"
                          << " make an ear. Rewind!" << endl;
                     
                  // Didn't find an ear. Restart here.
                  snipped.clear();
                  startIt = curIt;
                  // Count how many rewinds have occured.
                  ++nbrRewindsSinceLastSnip;
               }
            } 
         }
  
         // Failed.
         if ( failed ) {
            // Dump.
            mc2log << error 
                   << "[GU::SST]: Dumping remains of failed polygon." 
                   << endl;
            cout << GnuPlotDump::octave_dump( subjectPoly.begin(), 
                                              subjectPoly.end() );
            throw MC2Exception( "[GU::SST]: Splitting polygons failed. " 
                                "Would have resulted in infinite "
                                "loop that runBin would have restarted." );
         }
  
         // Snip the snipped ones.
         csi_dbg << "[GU::SST]: snipped:" << endl; 
         MC2_ASSERT( ! snipped.empty() );
         vector<POINT> polyToAdd;
         for ( uint32 i = 0; i < snipped.size(); ++i ) {
            VertexList::iterator& it = snipped[ i ];
            polyToAdd.push_back( **it );
            csi_dbg << **it;

            delete *it;
            subjectPoly.erase( it );
         }
       
         // Add the snipped poly.
         res.push_back( polyToAdd );
        
         csi_dbg << "[GU::SST]: snipped:" << endl;
         csi_cout << GnuPlotDump::octave_dump( polyToAdd.begin(), polyToAdd.end() );
         csi_dbg << "[GU::SST]: what's left:" << endl;
         csi_cout << GnuPlotDump::octave_dump( subjectPoly.begin(), subjectPoly.end() );
         csi_dbg << subjectPoly << endl;

         // Clean intersections.
         cleanIntersections( subjectPoly );
         
         csi_dbg << "[GU::SST]: After cleaning intersections (inside loop)." 
                 << endl;
         csi_dbg << subjectPoly << endl;
      }
   }

   res.swap( polygons );

   csi_dbg << "[GU::SST]: result:" << endl;
   for ( uint32 i = 0; i < res.size(); ++i ) {
      csi_dbg << "[GU::SST]: poly [" << i << "]" << endl;
      csi_cout << GnuPlotDump::octave_dump( 
            res[i].begin(), res[i].end() );
   }
   csi_dbg << "[GfxUtility::splitSelfTouching]: --ENDS- " << endl;
   
   // Return if this method did anything.
   return res.size() != polygons.size();
   
}

bool 
GfxUtility::splitSelfTouching( vector<vector<MC2Coordinate> >& coords )
                               throw (MC2Exception)
{

   // Blargh! Could probably cast pointers...

   vector<vector<POINT> > points;

   // Copy to point vector
   mc2ToPoints( points, coords );

   bool res = splitSelfTouching( points );

   // Copy to coords
   pointsToMC2( coords, points );

   return res;
}

int
GfxUtility::clipToPolygon( const vector<POINT>& clipPolygon, 
                           vector< vector<POINT> >& polygons )
{
      
   VertexList subjectPoly;
   VertexList clipPoly;

   // New scope.
   {
      if ( polygons.empty() ) {
         mc2dbg8 << "clipToPolygon: no polygon supplied" << endl;
         return 0;
      }
      vector<POINT>& vertices = polygons.front();
      // Need at least 4 coordinates, since the start and end coordinate
      // is same.
      if ( vertices.size() < 4 ) {
         mc2dbg8 << "clipToPolygon: too few coordinates" << endl;
         return 0;
      }

      // Create subject polygon from the vertices.
      for ( vector<POINT>::const_iterator it = vertices.begin();
            it != vertices.end(); ++it ) {
         Vertex* p = new Vertex( *it );
         subjectPoly.push_back( p );
      }
   }

   // Store the original subject polygon in case of emergency debug.
   vector<POINT> originalSubjPoly;
   polygons.front().swap( originalSubjPoly );
   // Clear polygons.
   polygons.clear();
   
   // Create the clip polygon from the vertices.
   for ( vector<POINT>::const_iterator it = clipPolygon.begin();
         it != clipPolygon.end(); ++it ) {
      Vertex* p = new Vertex( *it );
      clipPoly.push_back( p );
   }


   clipdbg << "---PHASE 0" << endl;

   clipdbg << "---CLIP POLY" << endl;
   clipdbg << clipPoly;

   clipdbg << "---SUBJ POLY" << endl;
   clipdbg << subjectPoly;
   
   // Phase I
   insertIntersections( subjectPoly,
                        clipPoly );

   clipdbg << "---PHASE I" << endl;
   
   clipdbg << "---CLIP POLY" << endl;
   clipdbg << clipPoly;
   
   clipdbg << "---SUBJ POLY" << endl;
   clipdbg << subjectPoly;

   // Phase II
   
   bool subjectInsideClip = false;
   bool clipInsideSubject = false;
 
   VertexList* curPoly = &subjectPoly;
   VertexList* otherPoly = &clipPoly;
   for ( int i = 0; i < 2; ++i ) {
   
      Vertex::XYHelper helper;
      Vertex* firstVertex = curPoly->front();
      bool enteringPoly = true; // Entering polygon
      if ( InsideUtil::inside( otherPoly->begin(), 
                               otherPoly->end(),
                               firstVertex, 
                               helper ) ) {
         enteringPoly = false; // Exiting polygon
      }

      if ( i == 0 ) {
         // Subject point inside clip polygon? 
         subjectInsideClip = ! enteringPoly;
      } else {
         // Clip point inside subject polygon?
         clipInsideSubject = ! enteringPoly;
      }

      for ( VertexList::iterator it = curPoly->begin(); 
            it != curPoly->end(); ++it ) {
         if ( (*it)->m_intersect ) {
            (*it)->m_enteringPoly = enteringPoly;
            enteringPoly = !enteringPoly; // Toggle status.
         }
      }
      
      // Now switch polygons.
      curPoly = &clipPoly;
      otherPoly = &subjectPoly;
   }
   clipdbg << "---PHASE II" << endl;
   
   clipdbg << "---CLIP POLY" << endl;
   clipdbg << clipPoly;
   
   clipdbg << "---SUBJ POLY" << endl;
   clipdbg << subjectPoly;
   
   // Phase III 
   VertexList::iterator subjIt = subjectPoly.begin();
   VertexList::iterator stopIt = subjIt;
   ++subjIt;
   
   int nbrIntersections = 0;
   
   curPoly = &subjectPoly;
   otherPoly = &clipPoly;

   while ( subjIt != stopIt ) {
      if ( (*subjIt)->m_intersect ) {
         ++nbrIntersections;
         // Intersection means new polygon.
         polygons.push_back( vector<POINT>() );
         // Add vertex
         polygons.back().push_back( POINT(**subjIt) );
         Vertex* startPoint = *subjIt;
         clipdbg << "New polygon. " << (*startPoint).x << ", "
                 << (*startPoint).y
            << endl;
         // Mark that the intersection is processed.
         (*subjIt)->m_intersect = false;

         VertexList::iterator curIt = subjIt;
         do {
            if ( (*curIt)->m_enteringPoly ) {
               clipdbg << "Entering polygon" << endl;
               // Entering polygon. Add vertices in forward direction.
               bool wrapped = false;
               do {
                  ++curIt;
                  if ( curIt == curPoly->end() ) {
                     clipdbg << "wrapping: end -> begin" << endl;
                     ASSERT_OR_DUMP( !wrapped );
                     wrapped = true;
                     curIt = curPoly->begin();
                  }
                  // Add vertex
                  polygons.back().push_back( **curIt );
                  clipdbg << "Adding coord " << (*curIt)->x 
                          << ", " << (*curIt)->y << endl;
               
               } while ( ! (*curIt)->m_intersect );
               clipdbg << "Intersection found." << endl;

            } else { 
               // Exiting polygon. Add vertices in reverse direction.
               clipdbg << "Exiting polygon" << endl;
               bool wrapped = false;
               do {
                  if ( curIt == curPoly->begin() ) {
                     curIt = curPoly->end();
                     clipdbg << "wrapping: begin -> end" << endl;
                     ASSERT_OR_DUMP( !wrapped );
                     wrapped = true;
                  }
                  --curIt;
                  // Add vertex
                  polygons.back().push_back( **curIt );
                  clipdbg << "Adding coord " << (*curIt)->x 
                          << ", " << (*curIt)->y << endl;
               
               } while ( ! (*curIt)->m_intersect );
               clipdbg << "Intersection found." << endl;

            }
            // Mark that the intersection is processed.
            (*curIt)->m_intersect = false;
            // Continue with other polygon.
            curIt = (*curIt)->m_neighbor;
            // Swap curPoly and otherPoly
            VertexList* tmpPtr = curPoly;
            curPoly = otherPoly;
            otherPoly = tmpPtr;
            // Mark that the intersection is processed.
            (*curIt)->m_intersect = false;
            clipdbg << "Continuing with other polygon: " 
                    << (*curIt)->x 
                    << ", " << (*curIt)->y << endl;
            // List must contain pointers for the while to work.
         } while ( *curIt != startPoint ); 
         clipdbg << "Polygon closed." << endl;
         stopIt = subjIt;
      }
      ++subjIt;
      if ( subjIt == subjectPoly.end() ) {
         subjIt = subjectPoly.begin();
      }
   }
  

   if ( nbrIntersections == 0 ) {
      // No intersections at all between the clip and subject polygon.
      // Three cases: 
      // 1. Subject completely inside clip.
      // 2. Clip completely inside subject.
      // 3. Subject and clip disjunct.
      ASSERT_OR_DUMP( !( subjectInsideClip && clipInsideSubject ) );
      if ( subjectInsideClip ) {
         // 1. Subject completely inside clip.
         // Return the subject polygon.
         polygons.push_back( vector<POINT>() ); 
         for ( VertexList::const_iterator it = subjectPoly.begin();
               it != subjectPoly.end(); ++it ) {
            polygons.back().push_back( **it );
         }
      } else if ( clipInsideSubject ) {
         // 2. Clip completely inside subject
         // Return the clip polygon.
         polygons.push_back( clipPolygon );
      } else {
         // 3. Subject and clip disjunct.
         // Return empty polygon.
      }
   }
   
   clipdbg << "---PHASE III" << endl;
   
   clipdbg << "---CLIP POLY" << endl;
   clipdbg << clipPoly;
   
   clipdbg << "---SUBJ POLY" << endl;
   clipdbg << subjectPoly;

#ifdef DEBUG_CLIPPING   
   // Dump the clipped polygons:
   for ( vector< vector<POINT> >::iterator it = polygons.begin();
         it != polygons.end(); ++it ) {
      mc2dbg << "Polygon: " << endl;
      for ( vector<POINT>::iterator jt = it->begin(); jt != it->end();
            ++jt ) {
         mc2dbg << jt->x << ", " << jt->y << endl;
      }
   }
   
   if ( polygons.size() > 0 ) {
      mc2dbg << "Clipping: " << polygons.size() << " polygons!!" << endl;
      mc2dbg << "First polygon contains " << polygons.front().size() 
             << " coordinates" << endl;
   }
#endif   
   return polygons.size();
}

int 
GfxUtility::clipPolylineToBBox( const MC2BoundingBox& bbox, 
                                vector< vector<POINT> >& polygons )
{
   uint32 initialSize = polygons.front().size();
   if ( initialSize == 0 ) {
      polygons.clear();
      return 0;
   }
   
   vector<POINT> origPoints;
   polygons.front().swap( origPoints );
   polygons.clear();
   
   vector<POINT>::const_iterator prevIt = origPoints.begin();
   bool prevInside = bbox.contains( prevIt->y, prevIt->x );
   
   // First. If prevVertex inside. Output it.
   if ( prevInside ) {
      polygons.push_back( vector<POINT>() );
      polygons.back().push_back( *prevIt );
   }
   
   // Special case for only one coordinate.
   if ( initialSize == 1 ) {
      return polygons.size();
   }
   
   vector<POINT>::const_iterator curIt = prevIt;
   ++curIt;

   // 1) prevVertex inside, currVertex inside: output currVertex
   // 2) prevVertex inside, currVertex outside: output intersection
   // 3) prevVertex outside, currVertex outside: Special trick.
   //       if crosses the bbox:
   //          new polygon and output start and end intersection.
   // 4) prevVertex outside, currVertex inside:
   //       new polygon and output intersection and currVertex
   //
   //
   // 3): 
      
   while ( curIt != origPoints.end() ) {
      
      bool curInside = bbox.contains( curIt->y, curIt->x );
     
      if ( prevInside && curInside ) {
         // 1) prevVertex inside, currVertex inside: output currVertex
         polygons.back().push_back( *curIt );
      }
      if ( prevInside && !curInside ) {
         // 2) prevVertex inside, currVertex outside: output intersection
         
         // Calculate intersection.
         POINT intersectPoint;
         bbox.clipToFirstIntersectionWithEdge( prevIt->y,
                                               prevIt->x,
                                               curIt->y,
                                               curIt->x,
                                               intersectPoint.y,
                                               intersectPoint.x );
         polygons.back().push_back( intersectPoint );
      }
      if ( !prevInside && !curInside ) {
         // 3) prevVertex outside, currVertex outside: 
         //       if crosses the bbox:
         //          new polygon and output start and end intersection.
         POINT startPoint, endPoint;
         
         bool intersects = false;
         
         // Check if the starting intersection point can be found.
         if ( bbox.clipToFirstIntersectionWithEdge( prevIt->y,
                                                    prevIt->x,
                                                    curIt->y,
                                                    curIt->x,
                                                    startPoint.y,
                                                    startPoint.x ) ) {

            // There must also be an ending intersection point.
            // Swap the coordinates to the intersect method so we 
            // find the ending intersection point.
            bbox.clipToFirstIntersectionWithEdge( curIt->y,
                                                  curIt->x,
                                                  prevIt->y,
                                                  prevIt->x,
                                                  endPoint.y,
                                                  endPoint.x );
            
            intersects = true;
         }

         if ( intersects ) {
            polygons.push_back( vector<POINT>() );
            polygons.back().push_back( startPoint ); 
            polygons.back().push_back( endPoint ); 
         }
      }
      if ( !prevInside && curInside ) {
         // 4) prevVertex outside, currVertex inside:
         //       new polygon and output intersection and currVertex
         polygons.push_back( vector<POINT>() );
         POINT intersectPoint;
         bbox.clipToFirstIntersectionWithEdge( curIt->y,
                                               curIt->x,
                                               prevIt->y,
                                               prevIt->x,
                                               intersectPoint.y,
                                               intersectPoint.x );
         polygons.back().push_back( intersectPoint );
         polygons.back().push_back( *curIt );
      }

      prevInside = curInside;
      prevIt = curIt;
      ++curIt;
   }
  
   return polygons.size();
}


int
GfxUtility::clipToPolygon( const MC2BoundingBox& bbox, 
                           vector< vector<POINT> >& polygons )
{
#if 0
   // Save the original polygon so that we can look at it.
   vector<POINT> origPoly;
   origPoly = polygons.front();
#endif
   
   //uint32 startTime = TimeUtility::getCurrentTime();
   
   MC2BoundingBox bboxToReduceBy( bbox );
   
   // No increase of the bboxToReduceBy
   // due to the earth being round and bboxes not.
   // bboxToReduceBy += 10;
   vector<POINT> toBeReduced;
   polygons.front().swap( toBeReduced );
   
   // Outcodes. Not currently used.
   vector<byte> outcodes;
   outcodes.reserve( toBeReduced.size() );
   polygons.front().reserve( toBeReduced.size() );
   reduceByBBox( &bboxToReduceBy, toBeReduced, polygons.front(), outcodes );
   
   // Create clip polygon from bbox.
   vector<POINT> clipPolygon;
   clipPolygon.resize(5);
   
   clipPolygon[0].x = bbox.getMinLon();
   clipPolygon[0].y = bbox.getMaxLat();

   clipPolygon[1].x = bbox.getMaxLon();
   clipPolygon[1].y = bbox.getMaxLat();

   clipPolygon[2].x = bbox.getMaxLon();
   clipPolygon[2].y = bbox.getMinLat();

   clipPolygon[3].x = bbox.getMinLon();
   clipPolygon[3].y = bbox.getMinLat();

   clipPolygon[4].x = bbox.getMinLon(); 
   clipPolygon[4].y = bbox.getMaxLat(); 

   // Perturb coordinates that are on the border of the clip boundingbox.
   for ( vector<POINT>::iterator it = polygons.front().begin();
         it != polygons.front().end();
         ++it ) {
      
      // Check top boundary.
      if ( it->y == bbox.getMaxLat() ) {
         mc2dbg << "GfxU: clipToPoly - Perturbing top" << endl;
         ++(it->y);
      }
      // Bottom.
      if ( it->y == bbox.getMinLat() ) {
         mc2dbg << "GfxU: clipToPoly - Perturbing bottom" << endl;
         --(it->y);
      }
      // Left.
      if ( it->x == bbox.getMinLon() ) {
         // Watch out for the end of the earth
         if ( it->x == MIN_INT32 ) {
            mc2dbg << "GfxU: clipToPoly - Perturbing right should be left "
                   << endl;
            ++(it->x);
         } else {
            mc2dbg << "GfxU: clipToPoly - Perturbing left" << endl;
            --(it->x);
         }
      }
      // Right.
      if ( it->x == bbox.getMaxLon() ) {
         // Watch out for the end of the earth
         if ( it->x == MAX_INT32 ) {
            mc2dbg << "GfxU: clipToPoly - Perturbing left should be right"
                   << endl;
            --(it->x);
         } else {
            mc2dbg << "GfxU: clipToPoly - Perturbing right" << endl;
            ++(it->x);
         }
      }
   }
   
   int retVal = clipToPolygon( clipPolygon, polygons );
//   mc2dbg << "clipClosedPoly took " << TimeUtility::getCurrentTime() - startTime
//          << " ms." << endl;
   return retVal;

}


inline int
GfxUtility::isLeft( const POINT& p0, const POINT& p1, const POINT& p2 )
{
   double res = double(p1.x - p0.x) * (p2.y - p0.y)
         - double(p2.x - p0.x) * (p1.y - p0.y);
 
   if ( res > 0.5 ) {
      return 1;
   } else if ( res < -0.5 ) {
      return -1;
   } else {
      return 0; 
   }
}


bool
GfxUtility::clipToBoundary( const byte boundaryOutcode,
                            const MC2BoundingBox* bbox,
                            vector<POINT>& vertices,
                            vector<byte>& outcodes,
                            vector<POINT>& resVertices,
                            vector<byte>& resOutcodes  ) 
{
   // This method is a block in the Sutherland-Hodgeman 
   // polygon clipping pipeline.

   // A polygon must consist of at least three vertices.
   if (vertices.size() < 3) {
      return (false);
   }

   resVertices.reserve(vertices.size());
   resOutcodes.reserve(outcodes.size());

   // Previous outcode
   vector<byte>::const_iterator prevOcIt = outcodes.begin();
   bool prevInside = (((*prevOcIt) & boundaryOutcode) == 0);

   // Current outcode
   vector<byte>::const_iterator currOcIt = outcodes.begin();
   ++currOcIt;

   // Previous vertex
   vector<POINT>::const_iterator prevVxIt = vertices.begin();
   // Current vertex
   vector<POINT>::const_iterator currVxIt = vertices.begin();
   ++currVxIt;
   for ( ; currVxIt != vertices.end(); ++currVxIt ) {
      bool currInside = (((*currOcIt) & boundaryOutcode) == 0);

      clipSegment( *prevVxIt, *currVxIt, prevInside, currInside,
                   *currOcIt, bbox, boundaryOutcode,
                   resOutcodes, resVertices );

      // Update prevVxIt
      ++prevVxIt;

      // Update prevInside
      prevInside = currInside;
      ++currOcIt;
   }

   // The same thing for the last edge:
   currOcIt = outcodes.begin();
   currVxIt = vertices.begin();
   bool currInside = (((*currOcIt) & boundaryOutcode) == 0);
   clipSegment( *prevVxIt, *currVxIt, prevInside, currInside,
                *currOcIt, bbox, boundaryOutcode,
                resOutcodes, resVertices );

   // Done with clipping boundary. Now reset the input vectors.
   vertices.clear();
   outcodes.clear();

   return (resVertices.size() > 2);
}

void
GfxUtility::clipSegment( const POINT& prevVertex,
                         const POINT& currVertex,
                         bool prevInside,
                         bool currInside,
                         byte currOutcode,
                         const MC2BoundingBox* bbox,
                         byte boundaryOutcode,
                         vector<byte>& resOutcodes,
                         vector<POINT>& resVertices )
{
   // 1) prevVertex inside, currVertex inside: output currVertex
   // 2) prevVertex inside, currVertex outside: output intersection
   // 3) prevVertex outside, currVertex outside: no output
   // 4) prevVertex outside, currVertex inside:
   //                        output currVertex and intersection

   if (prevInside && currInside) {
      // Case 1
      // 1) prevVertex inside, currVertex inside: output currVertex
      resVertices.push_back( currVertex );
      resOutcodes.push_back( currOutcode );
      mc2dbg8 << "Case 1: " << "(" << currVertex.y << "," << currVertex.x
              << ") " << endl;
   } else if (prevInside && !currInside) {
      // Case 2
      // 2) prevVertex inside, currVertex outside: output intersection
      POINT intersection;
      bbox->getIntersection( prevVertex.y, prevVertex.x,
                             currVertex.y, currVertex.x,
                             boundaryOutcode,
                             intersection.y, intersection.x );
      resVertices.push_back( intersection );
      resOutcodes.push_back(
            bbox->getCohenSutherlandOutcode( intersection.y,
                                             intersection.x ));
      mc2dbg8 << "Case 2: " << "(" << prevVertex.y << "," << prevVertex.x
              << ") " << " % " << "(" << currVertex.y << "," << currVertex.x
              << ") " << " -> " << "(" << intersection.y << ","
              << intersection.x << ") " << endl;
   } else if (!prevInside && currInside) {
      // Case 4
      // 4) prevVertex outside, currVertex inside:
      //                        output intersection and currVertex

      POINT intersection;
      bbox->getIntersection( prevVertex.y, prevVertex.x,
                             currVertex.y, currVertex.x,
                             boundaryOutcode,
                             intersection.y, intersection.x );
      resVertices.push_back( intersection );
      resOutcodes.push_back(
            bbox->getCohenSutherlandOutcode( intersection.y,
                                             intersection.x ));
      resVertices.push_back( currVertex );
      resOutcodes.push_back( currOutcode );
      mc2dbg8 << "Case 4: " << "(" << prevVertex.y << "," << prevVertex.x
              << ") " << " % " << "(" << currVertex.y << "," << currVertex.x
              << ") " << " -> " << "(" << intersection.y << ","
              << intersection.x << ") "<< "(" << currVertex.y << ","
              << currVertex.x << ") " << endl;
   }  // else (!prevInside && !currInside) 
      // Case 3
      // 3) prevVertex outside, currVertex outside: no output             
}

/**
 *   Needed for getting x and y inside reduceByBBox.
 */
struct PointXYHelper {
   inline int32 getX(const POINT& p) const {
      return p.x;
   }
   
   inline int32 getY(const POINT& p) const {
      return p.y;
   }
   
};

bool
GfxUtility::reduceByBBox( const MC2BoundingBox* bbox,
                          const vector<POINT>& vertices,
                          vector<POINT>& reducedVertices,
                          vector<byte>& outcodes )
{
   return ClipUtil::reduceByBBox( PointXYHelper(), *bbox, vertices,
                                  reducedVertices, &outcodes);
}

float64 
GfxUtility::getCoslat( int32 minLat, int32 maxLat ) {
     return cos( GfxConstants::invRadianFactor * float64( maxLat/2 + minLat/2 ) );
}


