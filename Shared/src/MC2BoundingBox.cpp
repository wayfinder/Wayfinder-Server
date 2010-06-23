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

#include "MC2BoundingBox.h"
#ifdef MC2_SYSTEM
#include "GfxUtility.h"
#endif
#include "GfxConstants.h"

#undef SQUARE
#define SQUARE(a) ((a)*(a))

// We cannot use the GfxConstants::IMPOSSIBLE in Symbian.
//const int32 MC2BoundingBox::c_impossible_lat = MAX_INT32-1;

MC2BoundingBox::MC2BoundingBox( const MC2Coordinate& center,
                                uint32 radiusMeters )
{
   this->maxLat = center.lat;
   this->minLat = center.lat;   
   this->minLon = center.lon;
   this->maxLon = center.lon;
   updateCosLat();
   this->increaseMeters( radiusMeters );
}

bool MC2BoundingBox::operator==( const MC2BoundingBox& bbox ) const
{
   return ( (this->maxLat == bbox.maxLat ) &&
            (this->minLat == bbox.minLat ) &&
            (this->minLon == bbox.minLon ) &&
            (this->maxLon == bbox.maxLon ) );
}

MC2BoundingBox::MC2BoundingBox( int32 maxLat, int32 minLong, 
                                int32 minLat, int32 maxLong)
{
   this->maxLat = maxLat;
   this->minLat = minLat;

   this->minLon = minLong;
   this->maxLon = maxLong;

   updateCosLat();
}
 
/*
void MC2BoundingBox::reset()
{
   maxLat = minLat = GfxConstants::IMPOSSIBLE;
   maxLon = minLon = 0;
   cos_lat = 0; 
}
*/

int32 MC2BoundingBox::getMinLongOffset( MC2BoundingBox& b ) const
{
   return int32( double( minLon - b.minLon ) * b.cos_lat);
}


// Distance calculations from point to a box. The point of all the tests 
// is to determine to what corner or side the calculation should be done
// to. With the case of a distance the answer is squared the follow the
// declaration of the function!

inline int64
MC2BoundingBox::innerSquareMC2ScaleDistTo( int32 lat_32, 
                                           int32 lon_32, 
                                           double cosLat) const
{
   if( cosLat == IMPOSSIBLE_COSINE )
      cosLat = cos_lat;

   int32 tmpDiff;

   if ( (tmpDiff = minLon - lon_32) > 0) {
      if (lat_32 < minLat ) {
         int32 latDiff = minLat - lat_32;
         int32 lonDiff = int32(cosLat*tmpDiff);
         return SQUARE( int64(latDiff) )+ 
                   SQUARE( int64(lonDiff) );
      } else if (lat_32 > maxLat){
         int32 latDiff = lat_32 - maxLat;
         int32 lonDiff = int32(cosLat*tmpDiff);
         return SQUARE( int64(latDiff) ) + 
                   SQUARE( int64(lonDiff) );
      } else {
         int32 lonDiff = int32(cosLat*tmpDiff);
         return SQUARE( int64(lonDiff) );
      }

   } else if ( (tmpDiff = lon_32 - maxLon) > 0 ) {
      if (lat_32 < minLat) {
         int32 latDiff = minLat - lat_32;
         int32 lonDiff = int32(cosLat*tmpDiff);
         return SQUARE( int64(latDiff) ) + 
                   SQUARE( int64(lonDiff) );
      } else if (lat_32 > maxLat) {
         int32 latDiff = lat_32 - maxLat;
         int32 lonDiff = int32(cosLat*tmpDiff);
         return SQUARE( int64(latDiff) ) + 
                    SQUARE( int64(lonDiff) );
      } else {
         int32 lonDiff = int32(cosLat*tmpDiff);
         return SQUARE( int64(lonDiff) );
      }

   } else {
      if (lat_32 < minLat) {
         int32 latDiff = minLat - lat_32;
         return SQUARE( int64(latDiff) );
      } else if (lat_32 > maxLat) {
         int32 latDiff = lat_32 - maxLat;
         return SQUARE( int64(latDiff) );
      } else {
         return 0;  //Inside
      }
   }
   return 0;
}

int64
MC2BoundingBox::squareMC2ScaleDistTo( int32 lat_32, 
                                      int32 lon_32, 
                                      double cosLat) const
{
#ifdef MC2_SYSTEM
   int64 tmpRes = innerSquareMC2ScaleDistTo(lat_32, lon_32, cosLat);
   MC2_ASSERT( tmpRes >= 0);
   return tmpRes;
#else
   return innerSquareMC2ScaleDistTo(lat_32, lon_32, cosLat);
#endif
}

int64
MC2BoundingBox::squareDistTo( int32 lat_32, 
                              int32 lon_32, 
                              double cosLat) const
{
   return  int64 (( GfxConstants::SQUARE_MC2SCALE_TO_SQUARE_METER * 
                    squareMC2ScaleDistTo(lat_32, lon_32, cosLat)));
}

// Distance calculations from point to a box. The point of all the tests 
// is to determine to what corner or side the calculation should be done
// to. With the case of a distance the answer is squared the follow the
// declaration of the function!
inline int64
MC2BoundingBox::innerMaxMC2ScaleSquareDistTo( int32 lat_32, 
                                         int32 lon_32, 
                                         double cosLat) const
{
   if (cosLat == IMPOSSIBLE_COSINE)
      cosLat = cos_lat;

   int32 midLon = ((maxLon - minLon) / 2) + minLon;
   int32 midLat = (maxLat + minLat) / 2;

   if (lon_32 - midLon < 0) {
      if( midLat > lat_32 ) { // 4
         int32 lonDiff = int32(cosLat*(maxLon - lon_32));
         int32 latDiff = maxLat - lat_32;
         return SQUARE( int64(lonDiff) ) +
                  SQUARE( int64(latDiff) );
      } else {              // 1
         int32 lonDiff = int32(cosLat*(maxLon - lon_32));
         int32 latDiff = minLat - lat_32;
         return SQUARE( int64(lonDiff) ) +
                   SQUARE( int64(latDiff) );
      }
   } else {
      if( midLat > lat_32 ) { // 3
         int32 lonDiff = int32(cosLat*(lon_32 - minLon));
         int32 latDiff = maxLat - lat_32;
         return SQUARE( int64(lonDiff) ) +
                   SQUARE( int64(latDiff) );
      } else {             // 2
         int32 lonDiff = int32(cosLat*(lon_32 - minLon));
         int32 latDiff = minLat - lat_32;
         return SQUARE( int64(lonDiff) ) +
                    SQUARE( int64(latDiff) );
      }
   }
}

int64
MC2BoundingBox::maxMC2ScaleSquareDistTo( int32 lat_32, 
                                         int32 lon_32, 
                                         double cosLat) const
{
#ifdef MC2_SYSTEM
   int64 tmpRes = innerMaxMC2ScaleSquareDistTo(lat_32, lon_32, cosLat);
   // Check that 63 bits is enough (should be since there are 31 in lat)
   MC2_ASSERT( tmpRes >= 0 );
   return tmpRes;
#else
   return innerMaxMC2ScaleSquareDistTo(lat_32, lon_32, cosLat);
#endif
}

int64 MC2BoundingBox::maxSquareDistTo( int32 lat_32, 
                                     int32 lon_32, 
                                     double cosLat) const
{
   return  int64( ( GfxConstants::SQUARE_MC2SCALE_TO_SQUARE_METER * 
                    maxMC2ScaleSquareDistTo(lat_32, lon_32, cosLat)));

}

void 
MC2BoundingBox::update( const MC2BoundingBox& b, bool alsoUpdateCosLat )
{
   update(b.getMinLat(),b.getMinLon(), alsoUpdateCosLat );
   update(b.getMaxLat(),b.getMaxLon(), alsoUpdateCosLat );
}

#ifdef MC2_SYSTEM
void
MC2BoundingBox::update(POINT* points, int nbrPoints)
{
   // calculate new bbox.
   almostAssert(nbrPoints > 0);
   if (minLat == GfxConstants::IMPOSSIBLE)// ||
//      (maxLat == GfxConstants::IMPOSSIBLE) ||
//      (minLon == 0) || (maxLon == 0))
   {
      minLon = points[0].x;
      maxLon = points[0].x;
      minLat = points[0].y;
      maxLat = points[0].y;
   }
   for( int i=0; i < nbrPoints; i++) {
      if (int32(points[i].x - maxLon) > 0) {
         maxLon = points[i].x;
      } else if (int32(points[i].x - minLon) < 0) {
         minLon = points[i].x;
      }
      if (points[i].y > maxLat) {
         maxLat = points[i].y;
      } else if (points[i].y < minLat) {
         minLat = points[i].y;
      }
   }
   updateCosLat();
}
#endif


void
MC2BoundingBox::dump(bool compact) const
{
   if (!compact) {
      mc2dbg << "MC2BoundingBox->dump()" << endl;
      mc2dbg << "Max lat = " << getMaxLat() << ", Min lon = " << getMinLon() << endl;
      mc2dbg << "Min lat = " << getMinLat() << ", Max lon " << getMaxLon()
           << ", Cos lat = " << getCosLat() << endl;
   } else {
      mc2dbg << "MaxLat=" << getMaxLat() << ", MinLon="<< getMinLon()
           << ", MinLat=" << getMinLat() << ", MaxLon=" << getMaxLon() << endl;
   }
}

void MC2BoundingBox::increaseFactor( double percent )
{
   int lat_inc = int(getHeight() * percent/2);
   int lon_inc = int(getLonDiff() * percent/2);
   maxLat += lat_inc;
   minLat -= lat_inc;
   maxLon += lon_inc;
   minLon -= lon_inc;
   updateCosLat();
}


const MC2BoundingBox&
MC2BoundingBox::operator*=(double factor)
{
   if ( factor >= 1.0 ) {
      increaseFactor( factor-1.0 );
   } else {
      increaseFactor( -1.0+factor );
   }
   return *this;
}

const MC2BoundingBox
MC2BoundingBox::operator*(double factor) const
{
   MC2BoundingBox retVal(*this);
   retVal *= factor;
   return retVal;
}

int64
MC2BoundingBox::getArea() const
{
   return (int64(getWidth()) * int64(getHeight()));
}

#ifdef MC2_SYSTEM


bool MC2BoundingBox::intersects( int32 startLat, int32 startLon,
                                int32 endLat, int32 endLon ) const
{
   bool result = false;
  
   if ( this->contains(startLat, startLon ) ||
        this->contains(endLat, endLon) )
   {
      result = true;
      mc2dbg8 << here
              << "One of the points were inside the bounding box." 
              << endl;
   }
   else {
      byte startCode = getCohenSutherlandOutcode(startLat, startLon);
      byte endCode = getCohenSutherlandOutcode(endLat, endLon);
      mc2dbg8 << here
              << "startCode = " << (int)startCode 
              << " endCode = " << (int)endCode << endl;

      if ( (startCode & endCode) != 0){
         //Both points are on the same outer side of one the 
         //extended bounding box edges and can therefore not
         //intersect with it.
         result = false;
         mc2dbg8 << here
                 << "Both points were on the same side of "
                 << "the bounding box, "
                 << "no intersection" 
                 << endl;
      }
      else{
         if( ( ( startCode | endCode ) == 0x0C) ||
             ( ( startCode | endCode ) == 0x03) )
         {
            //One point is on each side of the bounding box 
            //and must therefore intersect.
            result = true;
            mc2dbg8 << here
                    << "The poins are on opposite sides "
                    << "of the bounding box. "
                    << "Intersection must therefore occur." 
                    << endl;
         }
         else{
            //Since the points weren't on opposite sides of the 
            //bounding box, it must intersect with one of the 
            //vertical edges if it intersects with the bounding
            //box.

            //Don't care where the lines intersects.
            int32 dummy1 = MAX_INT32;
            int32 dummy2 = MAX_INT32;
            result =
               GfxUtility::getIntersection(startLat, startLon,
                                           endLat, endLon,
                                           maxLat, minLon,
                                           minLat, minLon,
                                           dummy1, dummy2);
            if (!result){
               result = 
                  GfxUtility::getIntersection(startLat, startLon,
                                              endLat, endLon,
                                              maxLat, maxLon,
                                              minLat, maxLon,
                                              dummy1, dummy2);
               mc2dbg8 << here
                       << "Needed test for intersectoin with "
                       << "one of the vertical edges." 
                       << endl;
            }
         }
      }
   }
   
   

   return result;
}

bool
MC2BoundingBox::clipToFirstIntersectionWithEdge(int32 startLat,
                                                int32 startLon,
                                                int32 endLat,
                                                int32 endLon,
                                                int32& clippedStartLat,
                                                int32& clippedStartLon) const
{
   bool result = false;
   bool intersects = false;
   int32 clippedLat = MAX_INT32;
   int32 clippedLon = MAX_INT32;

   // Parametric line equation. Distance from the start point as a value
   // between 0 and 1. A low value means that the point is close to the 
   // start point.
   float64 t = MAX_FLOAT64;
   float64 old_t = MAX_FLOAT64;

   // Clip to the left vertical edge of the bounding box.
   intersects  = GfxUtility::getIntersection(startLat, startLon,
                                             endLat, endLon,
                                             maxLat, minLon,
                                             minLat, minLon,
                                             clippedLat, clippedLon);
   if (intersects){
      result = true;
      t = float64(clippedLon - startLon) / (endLon - startLon);
      old_t = t;
      clippedStartLat = clippedLat;
      clippedStartLon = clippedLon;
   }

   // Clip to the right vertical edge of the bounding box.
   intersects  = GfxUtility::getIntersection(startLat, startLon,
                                             endLat, endLon,
                                             maxLat, maxLon, 
                                             minLat, maxLon,
                                             clippedLat, clippedLon);
   if (intersects){
      result = true;
      t = float64(clippedLon - startLon) / (endLon - startLon);
      if (t < old_t){
         old_t = t;
         clippedStartLat = clippedLat;
         clippedStartLon = clippedLon;
      }
   }
   
   // Clip to the top horizontal edge of the bounding box.
   intersects  = GfxUtility::getIntersection(startLat, startLon,
                                             endLat, endLon,
                                             maxLat, minLon,
                                             maxLat, maxLon,
                                             clippedLat, clippedLon);
   if (intersects){
      result = true;
      // Use lat instead of lon here becase start and end lat must differ
      // if the line intersected with a horizontal line.
      t = float64(clippedLat - startLat) / (endLat - startLat);
      if (t < old_t){
         old_t = t;
         clippedStartLat = clippedLat;
         clippedStartLon = clippedLon;
      }
   }
    
   // Clip to the bottom horizontal edge of the bounding box.
   intersects  = GfxUtility::getIntersection(startLat, startLon,
                                             endLat, endLon,
                                             minLat, minLon,
                                             minLat, maxLon,
                                             clippedLat, clippedLon);
   if (intersects){
      result = true;
      t = float64(clippedLat - startLat) / (endLat - startLat);
      if (t < old_t){
         old_t = t;
         clippedStartLat = clippedLat;
         clippedStartLon = clippedLon;
      }
   }
   if (!result){
      clippedStartLat = startLat;
      clippedStartLon = startLon;
      mc2dbg8 << "MC2BoundingBox::clipToFirstIntersectionWithEdge. "
              << "No clipping was done." << endl;
   }
   return result;
}
#endif

bool MC2BoundingBox::getInterSection( const MC2BoundingBox& bbox, MC2BoundingBox& interSection ) const
{
   if( overlaps( bbox ) ){
      // The bboxes contains an intersection.
      if( minLat < bbox.minLat )
         interSection.setMinLat( bbox.minLat );
      else
         interSection.setMinLat( minLat );

      if( maxLat > bbox.maxLat )
         interSection.setMaxLat( bbox.maxLat );
      else
         interSection.setMaxLat( maxLat );

      if( minLon-bbox.minLon < 0 )
         interSection.setMinLon( bbox.minLon );
      else
         interSection.setMinLon( minLon );

      if( maxLon - bbox.maxLon > 0 )
         interSection.setMaxLon( bbox.maxLon );
      else
         interSection.setMaxLon( maxLon );

      return true;
   }
   else
      return false;
}

void MC2BoundingBox::increaseMeters( uint32 meters )
{
   int32 lat_inc = int32( meters * GfxConstants::METER_TO_MC2SCALE );
   int32 lon_inc = int32( meters * GfxConstants::METER_TO_MC2SCALE / cos_lat );
   maxLat += lat_inc;
   minLat -= lat_inc;
   maxLon += lon_inc;
   minLon -= lon_inc;
   updateCosLat();
}

bool
MC2BoundingBox::getCenter( int32& lat, int32& lon ) const
{
   if (!isValid()) 
      return (false);
   lon = minLon + int32(rint(float64((maxLon - minLon)) / 2));
   lat = minLat + int32(rint(float64((maxLat - minLat)) / 2));
   return (true);
}

MC2Coordinate
MC2BoundingBox::getCenter() const
{
   // Should be invalid when created.
   MC2Coordinate coord;
   getCenter(coord.lat, coord.lon);
   return coord;
}

bool 
MC2BoundingBox::getIntersection( int32 lat1, int32 lon1,
                                 int32 lat2, int32 lon2,
                                 byte boundaryOutcode,
                                 int32& intersectLat,
                                 int32& intersectLon ) const
{
   // Solved using uniformity of triangles.
   switch (boundaryOutcode) {
      case (LEFT) :
         if (lon1 == lon2) {
            // parallell to boundary
            return (false);
         }
         intersectLon = minLon;
         intersectLat = lat1 + int32((lat2 - lat1) / 
                                     float64(lon2 - lon1) * (intersectLon - lon1) 
                                     + 0.5);
      break;
         
      case (RIGHT) :
         if (lon1 == lon2) {
            // parallell to boundary
            return (false);
         }
         intersectLon = maxLon;
         intersectLat = lat1 + int32((lat2 - lat1) / 
                                     float64(lon2 - lon1) * (intersectLon - lon1) 
                                     + 0.5);
      break;
         
      case (TOP) :
         if (lat1 == lat2) {
            // parallell to boundary
            return (false);
         }
         intersectLat = maxLat;
         intersectLon = lon1 + int32((lon2 - lon1) / 
                                     float64(lat2 - lat1) * (intersectLat - lat1)
                                     + 0.5);
         
      break;
      
      case (BOTTOM) :
         if (lat1 == lat2) {
            // parallell to boundary
            return (false);
         }
         intersectLat = minLat;
         intersectLon = lon1 + int32((lon2 - lon1) / 
                                     float64(lat2 - lat1) * (intersectLat - lat1)
                                     + 0.5);
      break;

      default:
         // No boundary specified.
         return (false);
   }
         
   // Everything is ok.
   return (true);
}

