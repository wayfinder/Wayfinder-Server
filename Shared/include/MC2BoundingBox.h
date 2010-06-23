/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2BOUNDINGBOX_H
#define MC2BOUNDINGBOX_H

#include "config.h"

#include "MC2Coordinate.h"

#include <math.h>

#define IMPOSSIBLE_COSINE 2.0

#ifndef WIN32
struct POINT;
#endif

/**
  *   Describes a bounding box with latitude and longitude.
  *
  */
class MC2BoundingBox {
   public:
      /**
       *   Impossible latitude.
       */
      enum impossible_lat_enum {
        c_impossible_lat = MAX_INT32 - 1
      };
   
      /**
       *   Create one empty bounding box.
       */
      inline MC2BoundingBox();

      /**
       *    Creates a bounding box where the circle
       *    with the supplied center and radius
       *    will fit inside.
       *    @param center       The center of the bbox.
       *    @param radiusMeters Half the side of the bbox.
       */
      MC2BoundingBox( const MC2Coordinate& center,
                      uint32 radiusMeters );

      /**
       *    Creates a MC2BoundingBox between the two coordinates.
       *    @param coord1 One corner of the bbox.
       *    @param coord2 The other corner of the bbox.
       */
      inline MC2BoundingBox( const MC2Coordinate& coord1,
                             const MC2Coordinate& coord2 );

      /**
       * Creates a bbox initialized to be used with quickerUpdate.
       */
      inline explicit MC2BoundingBox(bool init);

      /**
       * Creates a bbox without initializing it.
       * @param dummy1 Ignored.
       * @param dummy2 Ignored.
       */
      inline MC2BoundingBox(bool dummy1, bool dummy2) {};

       /**
        * Equals operator.
        */
       bool operator==( const MC2BoundingBox& bbox ) const;

      /**
        *   Create a bounding box with given boundry.
        *   @param   maxLat   The maximum latitude for this bounding box.
        *   @param   minLong  The minimum longitude for this bounding box.
        *   @param   minLat   The minimum latitude for this bounding box.
        *   @param   maxLong  The maximum longiude for this bounding box.
        */
      MC2BoundingBox( int32 maxLat, int32 minLong, 
                      int32 minLat, int32 maxLong );

      /**
        *   @name Minimun square distance.
        *
        *   Get minimum square distance from the bounding box
        *   to the point (lat,lon).
        */
      //@{
         /**
           *   @return  The smallest squared distance in @b meters
           *            to the bounding box from the point specified by 
           *            the parameters.
           */
         int64 squareDistTo(int32 lat_32, int32 lon_32, 
                            float64 cosLat = IMPOSSIBLE_COSINE) const;
         
         /**
           *   @return  The smallest squared distance in @b MC2-scale
           *            to the bounding box from the point specified by 
           *            the parameters.
           */
         int64 squareMC2ScaleDistTo(int32 lat_32, int32 lon_32, 
                             float64 cosLat = IMPOSSIBLE_COSINE) const;
      //@}

       /**
        *   @name Maximum square distance.
        *
        *   Get maximum square distance from the bounding box
        *   to the point (lat,lon).
        */
      //@{
         /**
           *   @return  The largest squared distance in @b meters to the 
           *            bounding box from the point specified by the 
           *            parameters.
           */
         int64 maxSquareDistTo(int32 lat_32, int32 lon_32, 
                                float64 cosLat = IMPOSSIBLE_COSINE) const;

         /**
           *   @return  The largest squared distance in @b MC2-scale to the 
           *            bounding box from the point specified by the 
           *            parameters.
           */
         int64 maxMC2ScaleSquareDistTo(int32 lat_32, int32 lon_32, 
                                       float64 cosLat = IMPOSSIBLE_COSINE)
            const;
      //@}

      /**
       *   Find out if a given coordinate is inside this bounding box.
       *   @return  True if the point (lat,lon) is inside or on the
       *            the boundary of this boundingbox. 
       */
      inline bool contains(int32 lat, int32 lon) const;

      /**
       *   Find out if a given coordinate is inside this bounding box.
       *   @return  True if the point (lat,lon) is inside or on the
       *            the boundary of this boundingbox. 
       */
     inline bool contains(const MC2Coordinate& coord) const;
   
       /**
        *   Find out if a given coordinate is inside this bounding box.
        *   @return  True if the point (lat,lon) is inside this boundingbox,
        *            false otherwise.
        */
      inline bool inside(int32 lat, int32 lon) const;
      inline bool inside(const MC2Coordinate& coord) const;

      /**
       *    @return true if this boundingbox is totally inside b.
       */
      inline bool inside( const MC2BoundingBox& b ) const;

      /**
        *   Check if any part of b overlaps this boundingbox.
        *
        *   @return True if b overlaps.
        */
      inline bool overlaps( const MC2BoundingBox& b ) const;

      /**
        *   Check if any part of b overlaps this boundingbox.
        *
        *   @return True if b overlaps.
        */
      inline bool overlaps( const MC2BoundingBox* b ) const;
      
      /**
       *    Corner types.
       */
      enum corner_t {
         no_corner = -1,
         top_left = 0,
         top_right = 1,
         bottom_left = 2,
         bottom_right = 3
      };
      
      /**
       *    Checks if the coordinate is one of the corners of the
       *    boundingbox, and returns the result.
       *    
       *    @param   coord    The coordinate to identify as corner.
       *    @return  The approriate corner_t.
       */
      inline corner_t getCorner( const MC2Coordinate& coord ) const;
     
      /**
       *    Get the coordinate of a specific corner.
       *    @param   corner   The corner type.
       *    @return  MC2Coordinate of the corner. If no_corner is supplied
       *             then an invalid MC2Coordinate is returned.
       */
      inline MC2Coordinate getCorner( const corner_t& corner ) const;
      
      /**
       *    Checks if the coordinate is lying on the border of the
       *    bbox.
       */
      inline bool onBorder( const MC2Coordinate& coord ) const;
      
      /**
        *   Get the distance between two boxes' minLong. Uses the 
        *   cosinefactor from the boundingbox b.
        *   @param   b  Another boundingbox.
        *   @return  The distance in radians
        */
      int32 getMinLongOffset( MC2BoundingBox& b ) const;

      /**
        *   Get the width of the boundingbox. Uses the cosinefactor member.
        *   @return The width of the boundingbox. 
        */
      inline int32 getWidth() const;

      /**
        *   Get the width of the boundingbox. Doesn't uses the
        *   cosinefactor member.
        *   @return The "width" of the boundingbox withoout coseine factor. 
        */
      inline int32 getLonDiff() const;

      /**
        *   Get the height of the boundingbox.
        *   @return The height of the boundingbox.
        */
      inline int32 getHeight() const;
      
      /**
        *   Updates the bounding box if necessery. Don't forget to 
        *   update the cosinefactor if needed since it won't be updated 
        *   by this method.
        *   @param alsoUpdateCosLat Optional parameter that should be set
        *               to false if the cos-lat-factor not should be
        *               updated.
        */
      void update( const MC2BoundingBox& b, bool alsoUpdateCosLat = true );

      /**
        *   Updates the bounding box if necessery.
        *   @param la   The latitude to check if to be included in this
        *               bounding box.
        *   @param lo   The longitude to check if to be included in this
        *               bounding box.
        *   @param alsoUpdateCosLat Optional parameter that should be set
        *               to false if the cos-lat-factor not should be
        *               updated.
        */
      inline void update(int32 la, int32 lo, bool alsoUpdateCosLat = true);

      inline void update(const MC2Coordinate& coord,
                         bool alsoUpdateCosLat = true);

      inline void quickerUpdate( const MC2BoundingBox& b );

      /**
        *   Updates the cosinefactor.
        */
      inline void updateCosLat();

      /**
        *   This method is inlined.
        *   @return  The cosinefactor for this bounded box.
        */
      inline float64 getCosLat() const;

      /**
        *   Resets the bounded box.
        */
      inline void reset();

      /**
        *   Sets the coslat factor. This is useful when calculating the 
        *   offset for the individual maps in the user's boundingbox.
        *   This method is inlined.
        *
        *   @param cosLat  The coslat factor.
        */
      inline void setCosLat(float32 cosLat);

      /**
        *   @name Get coordinates.
        *   Methods returning the coordinates in the boundingbox.
        */
      //@{
         /** 
           *   @return Maximum latitude.
           */
         inline const int32 getMaxLat() const;

         /** 
           *   @return Minimum latitude.
           */
         inline const int32 getMinLat() const;

         /** 
           *   @return Maximum longitude. 
           */
         inline const int32 getMaxLon() const;

         /** 
           *   @return Minimum longitude.
           */
         inline const int32 getMinLon() const;
      //@}

      /**
        *   @name Set coordinates.
        *   Methods for setting the coordinates in the boundingbox.
        */
      //@{
         /** 
           *   Set maxumim latitude.
           *   @param  k   Maximum latitude.
           */
         inline void setMaxLat(int32 k);

         /** 
           *   Set minumim latitude.
           *   @param  k   Minimum latitude.
           */
         inline void setMinLat(int32 k);

         /** 
           *   Set maxumim longitude.
           *   @param  k   Maximum longitude. 
           */
         inline void setMaxLon(int32 k);

         /** 
           *   Set minumim longitude.
           *   @param  k   Minimum longitude.
           */
         inline void setMinLon(int32 k);
      //@}
      
      /**
        *   Dumps information about the bounding box to stdout.
        *   @param   compact  If set to true, the output will be on a 
        *                     single line and more compact. Default true.
        */
      void dump(bool compact = true) const;

      /**
       *    Increases the bounding box with a factor.
       * 
       *    @param percent The percent to increase the boundingbox
       *                   with, e.g. 1 gives a twice as big 
       *                   boundingbox.
       */
      void increaseFactor( double percent );

      /**
       *    Multiply the bounding box with the factor, i.e.
       *    scale it.
       */
      const MC2BoundingBox operator*(double factor) const;

      /**
       *    Multiply the bounding box with the factor, i.e.
       *    scale it.
       */
      const MC2BoundingBox& operator*=(double factor);

      /**
       *    Add the specified number of units to each side of the
       *    boundingbox.
       */
      inline const MC2BoundingBox& operator+=(int units);
      
      /**
       *    Subtract the specified number of units from each side of the
       *    boundingbox.
       */
      inline const MC2BoundingBox& operator-=(int units);

      /**
       *    Adds a distance to each side of the bounding box.
       * 
       *    @param meters The distance to add to each side of the 
       *                   bounding box in meters.
       */
       void increaseMeters( uint32 meters );

      /**
        *   Gets the intersecting area between two bounding boxes.
        *
        *   @param bbox          The bounding box to retrieve the 
        *                        intersection with.
        *   @param interSection  The bbox containing the intersection 
        *                        if successful.
        *   @return True if there was an intersection, false otherwise.
        */
       bool getInterSection(const MC2BoundingBox& bbox, 
                            MC2BoundingBox& interSection ) const;

       /** 
        *   Returns true if coordinates is valid.
        *
        *   @return True if coordinates is valid, false otherwise.
        */
       inline bool isValid() const;

       /**
        *   Returns the coordinate of the center of the boundingbox.
        *   @param   lat   Output param. The latitude of the center.
        *   @param   lon   Output param. The longitude of the center.
        *   @return  True if the center could be calculated, 
        *            false otherwise.
        */
       bool getCenter( int32& lat, int32& lon ) const;

       /**
        *   Returns the center coordinate of the bounding box.
        *   If invalid bounding box then invalid coordinate.
        */
       MC2Coordinate getCenter() const;

       /**
        *   The method will check were the coordinate is located 
        *   related to the bounding box and return the outcode according
        *   to the following table.
        * 
        *   @verbatim
        *   1001 | 1000 | 1010
        *   ------------------ maxLat
        *   0001 | 0000 | 0010
        *   ------------------ minLat
        *   0101 | 0100 | 0110
        *     minLon  maxLon
        * 
        *   @endverbatim
        *   See computer-graphics literature for more details.
        *   
        *   @param   lat   Latitude
        *   @param   lon   Longitude
        *   @return  The Cohen-Sutherland outcode.
        */
       inline uint32 getCohenSutherlandOutcode( int32 lat, int32 lon ) const;
       
       /**
        *   @name Constants for Cohen-Sutherland outcodes.
        */
       //@{
       enum cohen_sutherland_outcodes {
          /// Inside.
          INSIDE = 0,
       
          /// Left.
          LEFT   = 1,
          
          /// Right.
          RIGHT  = 2,
          
          /// Bottom.
          BOTTOM = 4,
          
          /// Top.
          TOP    = 8,
       };
      //@}

       /**
        *   Get the area of the boundingbox.
        *   Uses uint64 which is unsuitable for e.g. Symbian.
        *   @return The area of the boundingbox, in square-mc2-units.
        */
      int64 getArea() const;
#ifdef MC2_SYSTEM

      /**
       *   Tells whether a line segment specified by its end coordinates
       *   intersects with the bounding box.
       *
       *   @param startLat Latitude for the start point of the line to 
       *                   check for intersection with.
       *   @param startLon Longitude for the start point of the line to 
       *                   check for intersection with.
       *   @param endLat   Latitude for the end point of the line to 
       *                   check for intersection with.
       *   @param endLon   Longitude for the end point of the line to 
       *                   check for intersection with.
       */
      bool intersects(int32 startLat, int32 startLon, 
                      int32 endLat, int32 endLon ) const;

      /**
       *   Clips a line segment specified by its end coordinates to its 
       *   first intersection with the edge of the bounding box.
       *
       *   @param startLat Latitude for the start point of the line to 
       *                   clip.
       *   @param startLon Longitude for the start point of the line to 
       *                   clip.
       *   @param endLat   Latitude for the end point of the line to 
       *                   clip.
       *   @param endLon   Longitude for the end point of the line to 
       *                   clip.
       *   @param clippedStartLat Outparameter The latitude value of the 
       *                          point where the line was clipped.
       *   @param clippedStartLon Outparameter The longitude value of the 
       *                          point where the line was clipped.
       */
      bool clipToFirstIntersectionWithEdge(int32 startLat, int32 startLon,
                                           int32 endLat, int32 endLon,
                                           int32& clippedStartLat,
                                           int32& clippedStartLon) const;

      /// @see the other clipToFirstIntersectionWithEdge function.
      inline bool 
      clipToFirstIntersectionWithEdge( const MC2Coordinate& start,
                                       const MC2Coordinate& end,
                                       MC2Coordinate& clipped ) const;

#endif

      /**
       *    Calculate the intersection between a boundary of the 
       *    boundingbox and a line between two coordinates.
       *
       *    @param lat1             Latitude part of the first point 
       *                            on the line.
       *    @param lon1             Longitude part of the first point 
       *                            on the line.
       *    @param lat2             Latitude part of the second point 
       *                            on the line.
       *    @param lon2             Longitude part of the second point 
       *                            on the line.
       *    @param boundaryOutcode  Boundary outcode for the boundary to
       *                            intersect with.
       *    @param intersectLat     Latitude part of the intersection. 
       *    @param intersectLon     Longitude part of the intersection. 
       *    @return  True if the intersection could be calculated, false
       *             if the boundaryOutcode was not valid or if no
       *             intersection could be found (parallell lines).
       */
      bool getIntersection( int32 lat1, int32 lon1,
                            int32 lat2, int32 lon2,
                            byte boundaryOutcode,
                            int32& intersectLat,
                            int32& intersectLon ) const;

#ifdef MC2_SYSTEM      
      /**
       * Updates the bbox with each of the points in turn.
       * @param point The points.
       * @param nbrPoints The number of points. You had better not set this to 0.
       */
      void update(POINT* points, int nbrPoints);
#endif
      
#ifndef _MSC_VER
      /**
       * Print on ostream.
       *
       * @param stream The stream to print on.
       * @param bbox   The bbox to print.
       * @return The stream.
       */
      friend ostream& operator<< ( ostream& stream,
                                   const MC2BoundingBox& bbox ) {
         return stream << "[(" << bbox.maxLat << ',' << bbox.minLon << "),("
                       << bbox.minLat << ',' << bbox.maxLon << ")]";
      }
#endif
                                   
protected:

      /**
        *   The coordinates describing the bounding box.
        */
      int32 maxLat, maxLon, minLat, minLon;

      /**
        *   Cosine at the latitude of the object
        */
      float32 cos_lat;
      
private:

   /**
    *   @return  The largest squared distance in @b MC2-scale to the 
    *            bounding box from the point specified by the 
    *            parameters.
    */
   inline int64 innerMaxMC2ScaleSquareDistTo(int32 lat_32, int32 lon_32,
                                             double cosLat) const;
   
   /**
    *   @return  The smallest squared distance in @b MC2-scale
    *            to the bounding box from the point specified by 
    *            the parameters.
    */
   inline int64 innerSquareMC2ScaleDistTo(int32 lat_32, int32 lon_32,
                                          double cosLat) const;
};

// -----------------------------------------------------------------------
//                                       Implementation of inlined methods

inline bool 
MC2BoundingBox::
clipToFirstIntersectionWithEdge( const MC2Coordinate& start,
                                 const MC2Coordinate& end,
                                 MC2Coordinate& clipped ) const {
   return clipToFirstIntersectionWithEdge( start.lat, start.lon,
                                           end.lat, end.lon,
                                           clipped.lat, clipped.lon );
}

void 
MC2BoundingBox::reset()
{
   maxLat = minLat = c_impossible_lat;
   maxLon = minLon = 0;
   cos_lat = 0;
}


inline int32 
MC2BoundingBox::getWidth() const { 
   return int32(cos_lat*(maxLon-minLon)); 
}

inline int32 
MC2BoundingBox::getLonDiff() const { 
   return maxLon-minLon; 
}

inline int32 
MC2BoundingBox::getHeight() const {
   return maxLat-minLat;
}

inline float64 
MC2BoundingBox::getCosLat() const { 
   return (cos_lat); 
}

inline void 
MC2BoundingBox::setCosLat(float32 cosLat) { 
   cos_lat = cosLat; 
}

inline const int32 
MC2BoundingBox::getMaxLat() const {
   return (maxLat);
}

inline const int32 
MC2BoundingBox::getMinLat() const {
   return (minLat);
}

inline const int32
MC2BoundingBox::getMaxLon() const {
   return (maxLon);
}

inline const int32 
MC2BoundingBox::getMinLon() const {
   return (minLon);
}

inline void 
MC2BoundingBox::setMaxLat(int32 k) {
   maxLat = k;
}

inline void 
MC2BoundingBox::setMinLat(int32 k) {
   minLat = k;
}

inline void 
MC2BoundingBox::setMaxLon(int32 k) {
   maxLon = k;
}

inline void 
MC2BoundingBox::setMinLon(int32 k) {
   minLon = k;
}

inline bool
MC2BoundingBox::isValid() const {
   return (maxLat != c_impossible_lat) &&
      ( minLat != c_impossible_lat );
}

inline const MC2BoundingBox& 
MC2BoundingBox::operator+=(int units)
{
   setMaxLat( getMaxLat() + units );   
   setMinLat( getMinLat() - units );   
   setMaxLon( getMaxLon() + units );
   setMinLon( getMinLon() - units );
   return *this;
}

inline const MC2BoundingBox& 
MC2BoundingBox::operator-=(int units)
{
   return (*this)+= (-units);
}
      
inline uint32
MC2BoundingBox::getCohenSutherlandOutcode( int32 lat, int32 lon ) const
{  
   uint32 outcode = 0;
   if (lon - minLon < 0) {
      outcode = 0x1;
   } else if (lon - maxLon > 0) {
      outcode = 0x2;

   } 
   
   if (lat < minLat) {
      outcode |= 0x4;

   } else if (lat > maxLat) {
      outcode |= 0x8;

   }
   return (outcode);
}

#define LESS_THAN( a,b ) ( ( ( (a) - (b) ) <= 0 ) )
#define IN_RANGE(a,b,c) ( LESS_THAN(a,b) && LESS_THAN(b,c) )

inline bool 
MC2BoundingBox::overlaps( const MC2BoundingBox& b ) const
{
   return (b.maxLat >= minLat) && (maxLat >= b.minLat) &&
//            ((b.maxLon - minLon) >= 0) && ((maxLon - b.minLon) >= 0) );
            ( IN_RANGE( minLon, b.minLon, maxLon ) ||
              IN_RANGE( minLon, b.maxLon, maxLon ) ||
              IN_RANGE( b.minLon, minLon, b.maxLon ) ||
              IN_RANGE( b.minLon, maxLon, b.maxLon ) );
}

#undef IN_RANGE
#undef LESS_THAN

inline bool 
MC2BoundingBox::overlaps( const MC2BoundingBox* b ) const
{
   return overlaps( *b );
}

inline bool
MC2BoundingBox::contains(int32 lat, int32 lon) const
{
   return ( lat >= minLat &&
            lat <= maxLat && 
            (lon - maxLon)<=0 && 
            (lon - minLon)>=0 );
}

inline bool
MC2BoundingBox::contains(const MC2Coordinate& coord) const
{
   return contains(coord.lat, coord.lon);
}

inline bool
MC2BoundingBox::inside(int32 lat, int32 lon) const
{
   return ( lat > minLat &&
            lat < maxLat && 
            (lon - maxLon)<0 && 
            (lon - minLon)>0 );
}

inline bool
MC2BoundingBox::inside(const MC2Coordinate& coord) const
{
   return inside( coord.lat, coord.lon );
}

inline bool 
MC2BoundingBox::inside( const MC2BoundingBox& b ) const
{
  return b.contains( minLat, minLon) && b.contains( maxLat, maxLon);
}

inline
MC2BoundingBox::MC2BoundingBox(bool init) {
   // init ignored. just makes it different from default constr.
   maxLat = -MAX_INT32/*-1*/; // something like that.
   minLat = MAX_INT32;
   maxLon = -MAX_INT32/2/*-1*/;
   minLon = MAX_INT32/2;
   // coslat not initiated
}

inline void
MC2BoundingBox::quickerUpdate( const MC2BoundingBox& b )
{
   // doesn't update coslat.
   // assumes values are initiated.
   if (b.getMinLat() < minLat) {
      minLat = b.getMinLat();
   }
   if (b.getMaxLat() > maxLat) {
      maxLat = b.getMaxLat();
   }
   if (int(b.getMinLon() - minLon) < 0) {
      minLon = b.getMinLon();
   }
   if (int(b.getMaxLon() - maxLon) > 0) {
      maxLon = b.getMaxLon();
   }
}

void
MC2BoundingBox::update(int32 la, int32 lo, bool alsoUpdateCosLat)
{
   DEBUG4(  if (la == c_impossible_lat)
            cerr << "  MC2BoundingBox::update() la == IMPOSSIBLE";
            );

   // This assumes that all initializations are being done to new zero.
   if (  (minLat == c_impossible_lat)/* &&
         (maxLat == c_impossible_lat) &&
         (minLon == 0) && (maxLon == 0)*/) {
      maxLat = la;
      minLat = la;
      maxLon = lo;
      minLon = lo;
      if(alsoUpdateCosLat)
         updateCosLat();
   } else {
      bool changed = false;
      // Test on latitude !      
      if (la > maxLat) {
         maxLat = la;
         changed = true;
      }
      if (la < minLat) {
         minLat = la;
         changed = true;
      }

      // Test on longitude !      
      if ( ((int32) (lo - maxLon) ) > 0 ) {
         maxLon = lo;
         changed = true;
      }
      if ( ((int32) (lo - minLon) ) < 0 ) {  // Test on longitude !
         minLon = lo;
         changed = true;
      }
      if ((changed) && (alsoUpdateCosLat))
         updateCosLat();
   }
}

inline
MC2BoundingBox::MC2BoundingBox()
{
   reset();
}

inline void
MC2BoundingBox::update(const MC2Coordinate& coord, bool alsoUpdateCosLat)
{
   update(coord.lat, coord.lon, alsoUpdateCosLat);
}

inline
MC2BoundingBox::MC2BoundingBox( const MC2Coordinate& coord1,
                                const MC2Coordinate& coord2 )
{
   reset();
   update(coord1, false);
   update(coord2, true);
}

inline void
MC2BoundingBox::updateCosLat()
{
   cos_lat = float32(cos( (2*M_PI/ (4294967296.0) *
                           ((maxLat/2)+(minLat/2)))));
}

inline MC2BoundingBox::corner_t 
MC2BoundingBox::getCorner( const MC2Coordinate& coord ) const
{
   if ( coord.lat == minLat ) {
      if ( coord.lon == minLon ) {
         return bottom_left;
      } else if ( coord.lon == maxLon ) {
         return bottom_right;
      }
   } else if ( coord.lat == maxLat ) {
      if ( coord.lon == minLon ) {
         return top_left;
      } else if ( coord.lon == maxLon ) {
         return top_right;
      }
   }
   return no_corner;
}

inline MC2Coordinate 
MC2BoundingBox::getCorner( const MC2BoundingBox::corner_t& corner ) const
{
   switch ( corner ) {
      case ( bottom_left ):
         return MC2Coordinate( minLat, minLon );
      case ( bottom_right ):
         return MC2Coordinate( minLat, maxLon );
      case ( top_left ):
         return MC2Coordinate( maxLat, minLon );
      case ( top_right ):
         return MC2Coordinate( maxLat, maxLon );
      default:
         return MC2Coordinate(); // Invalid coordinate.
   }
}



inline bool 
MC2BoundingBox::onBorder( const MC2Coordinate& coord ) const
{
   // Not optimized.
   return contains( coord ) && !inside( coord );
}

#endif
