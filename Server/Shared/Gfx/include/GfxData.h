/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFXDATA_H
#define GFXDATA_H

#include "config.h"

#include <vector>
#include <set>
#include <utility>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <cmath>
#include <iterator>
#include <fstream>

#include "MC2BoundingBox.h"
#include "CoordinateTransformer.h"
#include "GfxDataTypes.h"
#include "FilteredCoord.h"

class GenericMap;
class ConvexHullNotice;
class DataBuffer;
class Stack;

class GfxData;
class GfxDataFull;

/**
 *   Iterator to use with filtered GfxData.
 *
 *   Can not be used in some stl-stuff e.g. vector<MC2Coordinate>::insert for
 *   some unknown reason. Might work better now since the coordinates
 *   are backed up be real coordinates.
 */
class GfxDataFilterIterator {
   
public:

   /// The value type of this kind of iterator.
   typedef MC2Coordinate value_type;
   
   /**
    *   Creates a new GfxDataAdaptor to use when iterating
    *   through coordinates in a GfxData.
    *   Should use GfxData::const_iterator, but there were
    *   problems with the order of definition.
    */
   GfxDataFilterIterator( const MC2Coordinate* curPos,
                          const MC2Coordinate* end,
                          uint8 filterLevel ) :
      m_filterLevel(filterLevel) {
      m_curPos = curPos;
      m_end    = end;
   }
   
   /**
    *   Returns true if the iterator is equal to another iterator.
   */
   bool operator==(const GfxDataFilterIterator& other) const {
      return ( ( other.m_curPos == m_curPos ) &&
               ( other.m_filterLevel == m_filterLevel) );
   }

   /**
    *   Returns false if the iterator is equal to another iterator.
    */
   bool operator!=(const GfxDataFilterIterator& other) const {
      return ! (other == *this);
   }
   
   /**
    *   Dereferences the iterator.
    */
   inline const value_type& operator*() const {
      MC2_ASSERT( m_curPos < m_end );
      return *m_curPos;
   }

   /**
    *   Prefix ++operator. Please use most often.
    */
   const GfxDataFilterIterator& operator++() {
      if ( m_curPos < m_end ) {
         while ( ++m_curPos != m_end &&
                 FilteredCoord::getFilterLevel(**this) < m_filterLevel ) {
            // Step
         }
      } else {
         // Will crash sooner or later.
         ++m_curPos;
      }
      return *this;
   }
   
private:
   /// The current position
   const MC2Coordinate* m_curPos;
   /// The forbidden position
   const MC2Coordinate* m_end;
   /// The zoom level
   uint8 m_filterLevel;
};

/**
  *   Contains all graphical information about one item. 
  *   All coordinates are stored as an int32 in parts of (2*pi/2^32)
  *   <it>Observe that this means that it <b>not</b> is possible to
  *   treat them as planear coordinates!</it>
  *
  *   It is important that alla comparisons being done with 
  *   longitudes consider that this variable is cyclic.
  *   If you want to do 
  *      <tt>lon1 < lon2</tt>
  *   You should write
  *      <tt>(int32)(lon1 - lon2) < 0</tt>
  *   With latitudes this problem is of no concern as latitudes doesn't 
  *   loop. In all code it is assumed that no polygon covers more than 
  *   180 longitudinal degrees.
  *
  */
class GfxData {
public:
   /// Numeric type of coordinates
   typedef int32 coordinate_type;
      
   /// Ordinary iterator for all coordinates in a polygon
   typedef MC2Coordinate* iterator;
   /// Ordinary const iterator for all coordinates in a polygon
   typedef const MC2Coordinate* const_iterator;

   static GfxDataFull* createNewGfxData(GenericMap* theMap, 
                                        bool createNewPolygon=false);
         
   static GfxData* createNewGfxData(GenericMap* theMap, 
                                    const MC2BoundingBox* bbox);

   static GfxData* createNewGfxData(GenericMap* theMap, 
                                    const coordinate_type* firstLat,
                                    const coordinate_type* firstLon, 
                                    uint16 nbrCoords, 
                                    bool closed);

   static GfxData* createNewGfxData(GenericMap* theMap, 
                                    const GfxData* src);


   /**
    *    Creates an empty GfxData.
    */
   GfxData() { };


   /**
    *    Since this class have to be abstract, we need a proper 
    *    destructor.
    */
   virtual ~GfxData() {};

   /// Returns the begin iterator for polygon p
   virtual const_iterator polyBegin( uint16 p ) const = 0;
   /// Returns the end iterator for polygon p
   virtual const_iterator polyEnd( uint16 p ) const = 0;
   /// Returns the begin iterator for polygon p
   virtual iterator polyBegin( uint16 p ) = 0;
   /// Returns the end iterator for polygon p
   virtual iterator polyEnd( uint16 p ) = 0;
      
   virtual coordinate_type getLat(uint16 p, uint32 i) const = 0;

   virtual coordinate_type getLon(uint16 p, uint32 i) const = 0;

   virtual uint16 getNbrPolygons() const = 0;

   virtual uint32 getNbrCoordinates(uint16 poly) const = 0;
      
   uint32 getTotalNbrCoordinates() const {
      uint32 n=0;
      for (uint32 p=0; p<getNbrPolygons(); ++p)
         n += getNbrCoordinates(p);
      return n;
   }

   /**
    * Gets coordinate for a specific polygon and segment
    * @param p polygon number
    * @param i segment number
    * @return coordinate in polygon p and segment i
    */
   inline MC2Coordinate getCoordinate( uint16 p, uint32 i ) const {
      return MC2Coordinate( getLat( p, i ), getLon( p, i ) );
   }

   /// The filter iterator. Operates on coordinates, not polygons.
   typedef GfxDataFilterIterator const_filter_iterator;
   
   /// Begin iterator for polygon polyIdx
   const_filter_iterator
   beginFilteredPoly(uint16 polyIdx, uint8 filterLevel ) const {
      return GfxDataFilterIterator(polyBegin(polyIdx),
                                   polyEnd(polyIdx),
                                   filterLevel);
   }

   /**
    *   End iterator for polygon polyIdx.
    *   @warning! Relative complicated to create so only do it once
    *             per loop.
    */
   const_filter_iterator
   endFilteredPoly(uint16 polyIdx, uint8 filterLevel ) const {
      return GfxDataFilterIterator(polyEnd(polyIdx),
                                   polyEnd(polyIdx),
                                   filterLevel );
   }

   /**
    *    Sort the polygons by the number of coordinates they
    *    have. The polygon with the most number coordinates are
    *    set to polygon with index 0.
    *    @return  True if the polygons are sorted, false otherwise.
    */
   virtual bool sortPolygons() = 0;

   virtual void setClosed(uint16 poly, bool closed) = 0;
      
   virtual bool getClosed(uint16 poly) const = 0;

   /**
    *   Get the minimum latitude.
    *   @return  The minimum latitude for the bounded box for this
    *            gfxData.
    */
   virtual coordinate_type getMinLat() const = 0;

   /**
    *   Get the maximum latitude. 
    *   @return  The maximum latitude for the bounded box for this
    *            gfxData.
    */
   virtual coordinate_type getMaxLat() const = 0;


   /**
    *   Get the minimum longitude. We have to remember that 
    *   minLon can be < maxLon if we e.g. are close to London or 
    *   Paris. So take care when using this function.
    *
    *   @return  The minimum longitude for the bounded box for this
    *            gfxData.
    */
   virtual coordinate_type getMinLon() const = 0;

   /**
    *   Get the maximum longitude. We have to remember that minLon 
    *   can be < maxLon if we are close to London or Paris. So take 
    *   care when using this function.
    *
    *   @return  The maximum longitude for the bounded box for this
    *            gfxData.
    */
   virtual coordinate_type getMaxLon() const = 0;

   virtual float64 getCosLat() const = 0;

   /// Returns the length in meters.
   virtual float64 getLength(uint16 p) const = 0;

   /**
    *   Calculates the length of polygon <code>polyIndex</code>.
    */
   float64 calcPolyLength( uint32 polyIdx ) const;

   
   /// Note! length is in mm.
   virtual void setLength(uint16 p, float64 length) = 0;

 
   /**
    *   Calculates the lengths of the polygons.
    *
    *   @param   polindex Index of the polygon to update,
    *                     default: update all polygons.
    */
   virtual bool updateLength(uint16 polindex = MAX_UINT16) = 0;

   virtual bool updateBBox() = 0;
      
   virtual bool updateBBox(const coordinate_type lat,
                           const coordinate_type lon) = 0;

   /**
    *    Get the memory usage of this object.
    *    @return  The number of bytes used by this object.
    */
   virtual uint32 getMemoryUsage() const;

   enum gfxdata_t {
      gfxDataFull = 0,
      gfxDataSingleSmallPoly,
      gfxDataSingleLine,
      gfxDataSinglePoint,
      gfxDataMultiplePoints
   };

   gfxdata_t getGfxDataType() const {
      if (getNbrPolygons() == 1) {
         if (getNbrCoordinates(0) == 1) {
            return gfxDataSinglePoint;
         } else if (getNbrCoordinates(0) == 2 ) {
            if ( ! closed() ) {
               return gfxDataSingleLine;
            } else {
               mc2dbg << "[GfxData]: A closed polygon with 2 coordinates "
                      << "is stupid" << endl;
               // This shold be impossible really.
               return gfxDataSingleSmallPoly;                  
            }
         } else {
            return gfxDataSingleSmallPoly;
         }
      } else {
         // More than one polygon, check if all polygons
         // only have one coordinate
         bool onlyPoints = true;
         uint32 p = 0;
         while ((onlyPoints) && (p < getNbrPolygons())) {
            onlyPoints = getNbrCoordinates(p) == 1;
            ++p;
         }
         if (onlyPoints) {
            return gfxDataMultiplePoints;
         }
      }
      // Default, return FULL
      return gfxDataFull;
   }

   void createFromDataBuffer( DataBuffer* dataBuffer );

      
   /**
    *    Create a GfxData that is an approximation of an arc or 
    *    circle sector. Angles are given in degrees and counted 
    *    from the north direction in clockwise direction.
    *
    *    @warning The GfxData returned by this method is created 
    *             in this method but <b>must</b> be deleted by the 
    *             caller!
    *    
    *    @param   lat         The latitude part of the coordinate
    *    @param   lon         The longitude part of the coordinate
    *                         describing the centre of the circle.
    *    @param   startAngle  The start angle in degrees.
    *    @param   stopAngle   The stop angle in degrees.
    *    @param   outerRaduis The outer radius <it>in mc2-units</it>.
    *                         use the constant METER_TO_MC2SCALE in
    *                         this class to convert from meters.
    *    @param   innerRaduis Optional parameter describing the inner
    *                         radius <it>in mc2-units</it>. Default value
    *                         is 0.
    *    @return  A GfxData approximating the area described by the
    *             parameters. NULL is returned upon error. Please
    *             note that the GfxData is created inside this method
    *             but must be deleted by the caller!
    */
   static GfxData* getArcApproximation(int32 lat, int32 lon, 
                                       int startAngle, int stopAngle, 
                                       int outerRadius, 
                                       int innerRadius = 0,
                                       float64 angleIncrement = 0.2);

   /**
    *    Method used for text labeling of features described with closed
    *    polygons.
    *    It calculates the centroid (the center of gravity) of a
    *    closed polygon.
    *
    *    @warning This function does not seem to work for all polygons.
    *             It has not been examined when it fails, but I suspect
    *             that selfintersecting polygons do not work.
    *
    *    @param polygon      The index for the polygon used
    *    @param centroidLat  Outparameter. The latitude of the centroid of
    *                        the polygon
    *    @param centroidLon  Outparameter. The longitude of the centroid of
    *                        the polygon
    *    @return             True if indata is closed and has at least
    *                        three nodes, false otherwise.
    */
   bool getPolygonCentroid(const uint32 polygon,
                           int32& centroidLat,
                           int32& centroidLon) const;

   /// @see above
   inline bool getPolygonCentroid( uint32 polygon,
                                   MC2Coordinate& coord ) const {
      return getPolygonCentroid( polygon, coord.lat, coord.lon );
   }
   /**
    *    Finds out if a given simple polygon is convex, i.e all
    *    lines drawn between any pair of nodes are entirely inside
    *    of or on the boundary of the polygon. 
    *
    *    @param   polyIndex     The index for the polygon used.
    *                           Polygon is assumed to be CLOCKWISE
    *                           oriented!
    *
    *    @return                True if polygon is convex,
    *                           false otherwise
    *
    */
   bool convexPolygon(const uint32 polygon) const;

   /**
    *    Finds the cosine of the relative angle between two consecutive
    *    segments in a simple polygon. 
    *  
    *    @param  polyIndex    The index for the polygon used.
    *    @param  p            Vertex only in first segment
    *    @param  tip          Middle vertex, appear in both segments
    *    @param  q            Vertex only in second segment
    *    @return              Cosine of the angle
    */
   double getCosRelativeAngle(int32 polyIndex, uint32 p,
                              uint32 tip, uint32 q);
     
   /**
    *   Save the data in a data buffer.
    *
    *   @param   dataBuffer  The databuffer where the GfxData will be
    *                        stored. Must be at least 
    *                        getSizeInDataBuffer() bytes large.
    *                       
    */
   void save(DataBuffer& dataBuffer) const;

   /**
    *    Returns the size (in bytes) of the GfxData when saved as a 
    *    DataBuffer. Use this method in conjunction with save().
    *
    *    @return The size (in bytes) of the GfxData when saved as a 
    *            DataBuffer.
    */
   uint32 getSizeInDataBuffer() const;

   /**
    *   @name Inspect coordinates.
    *   Methods to get the coordinates of this GfxData.
    */
   //@{
   /**
    *    Get the coordinate at given offset. The coordinates are
    *    returned via outparameters.
    *    @param   offset   The offset of the polygon where the 
    *                      coordinate should be calculated.
    *    @param   lat      Outparameter that is set to the latitude
    *                      part of the coordinate at offset. This
    *                      is @b not set if returning false.
    *    @param   lon      Outparameter that is set to the longitude
    *                      part of the coordinate at offset. This
    *                      is @b not set if returning false.
    *    @return  0 if the outparameters were not set properly,
    *             otherwise the index of the next coordinate following
    *             this offset.
    */
   uint32 getCoordinate(uint16 offset, coordinate_type& lat,
                        coordinate_type& lon) const;

   /**
    *   Get the latitude-part of the last coordinate.
    *   @param   p  The polygon to get the last latitude for.
    *   @return  Latitude for the last coordinate.
    */
   inline coordinate_type getLastLat(uint16 p) const;

   /**
    *   Get the longitude-part of the last coordinate.
    *   @param   p  The polygon to get the last latitude for.
    *   @return  Longitude for the last coordinate.
    */
   inline coordinate_type getLastLon(uint16 p) const;

   /**
    *    Get the bounding box for this GfxData.
    *
    *    Note: This function isn't virtual. Strange since
    *          GfxDataFull has a bbox.
    *
    *    @param bb      Pointer to the bounding box of this GfxData.
    *    @param polygon Which polygon of the GfxData the bounding
    *                   box applies to. Note that when this parameter
    *                   is specified, the bounding box is calculated
    *                   on the fly, and it is therefore somewhat more
    *                   time consuming.
    *                   If set to MAX_UINT16 then
    *                   the bounding box applies to all polygons.
    *                   Defaults to MAX_UINT16 when not specified.
    *    @return True if the operation was succesful, false
    *            otherwise.
    */
   virtual bool getMC2BoundingBox(MC2BoundingBox& bb, 
                                  uint16 polygon = MAX_UINT16) const;

   /**
    *    Default implementation uses the getMC2BoundingBix-methods.
    *    Could be overrided for better performance in the subclasses,
    */
   virtual float64 getBBoxArea() const;
   virtual uint64 getBBoxArea_mc2() const;
   virtual bool insideBBox(int32 lat, int32 lon) const;

   /**
    *    Get a random coordinate that is inside any of the
    *    polygons in the GfxData.
    *
    *    @param   lat      Random lateral coordinate inside GfxData.
    *    @param   lon      Random longitudinal coordinate inside
    *                      GfxData.
    *    @param   maxTries Maximum number of attempts to find a
    *                      coordinate inside the GfxData.
    *    
    *    @return  True if a coordinate was found inside the GfxData,
    *             false otherwise.
    */
   bool getRandomCoordinateInside(coordinate_type& lat,
                                  coordinate_type& lon, uint32 maxTries = 1000) const;
   //@}

   /**
    *    @name Math utility.
    *    Inspect graphical properties of this GfxData.
    */
   //@{  
   /**
    *   This fucntion may be used as a a help in method 
    *   getRandomCoordinateInside.
    *   It determines the average number of tries needed when a random
    *   coordinate inside a given polygon is wanted.
    *   The complexity is linear.
    *
    *   @param     polygon      Index for the polygon used.
    *
    *   @return                 The average number of tries needed,   
    *                           casted to uint16.                
    */ 
   uint16 getAverageNbrMaxTries(const uint16 polygon) const;

   /**
    *   Determins where a specified point is located relative this
    *   polygon (inside, outside or on the boundry). If the optional
    *   parameter usePolygon not is set, the given coordinate is
    *   checked againas all polygons in this GfxData.
    *
    *   @param   lat          The latitude part of the position.
    *   @param   lon          The longitude part of the position.
    *   @param   usePolygon   Optional paramter, that if set describes
    *                         what polygon in ths GfxData that should
    *                         be used in the calculations.
    *   @return The returned integer has the following meaning:
    *           <DL>
    *               <DT>0</DT>
    *                  <DD>The position is @b outside the polygon.</DD>
    *               <DT>1</DT>
    *                  <DD>The position is located <b>on the 
    *                      boundry</b> of the polygon.</DD>
    *               <DT>2</DT>
    *                  <DD>The position is @b inside the polygon.</DD>
    *           </DL>
    */
   int insidePolygon(coordinate_type lat, coordinate_type lon,
                     uint16 usePolygon = MAX_UINT16) const;
         
   int insidePolygon(MC2Coordinate coord, 
                     uint16 usePolygon = MAX_UINT16) const;

   /**
    *    Calculates how many of the corners of the bounding box
    *    that are inside or at the boundry of the polygon.
    *    This function may be a bad idea.
    *    @param bbox The bounding box to check.
    *    @param maxCorners Exit when the number of corners reach this
    *                      value.
    *    @param usePolygon Optional parameter. If set only
    *                      the supplied polygon in the GfxData
    *                      will be checked.
    *    @return Nbr of corners inside or at the boundry of the
    *            polygons.
    *            
    */
   int nbrCornersInsidePolygon(const MC2BoundingBox& bbox,
                               int maxCorners = 4,
                               uint16 usePolygon = MAX_UINT16) const;
         

   /**
    *   A better name would be squaredDistToBoundry.
    *   This funtion takes the distance from
    *   the boundry of our current object to a point lat/lon.
    *
    *   @param   lat      The latitude (in "our" 32bit measure 
    *                     system).
    *   @param   lon      The longitude (in "our" 32bit measure 
    *                     system).
    *   @param   closestI Outparameter that is set to the coordinate
    *                     in the polygon that is closest to
    *                     (lat, lon). If equal to MAX_UINT16
    *                     when calling, this calculation is not done.
    *   @param   closestT Outparameter that is set to the part of the
    *                     distance from closestI (E.g. 0 <=> coordinate 
    *                     closetI is closest, 0.5 <=> the closest
    *                     point to (lat, lon) on the polygon
    *                     is located between coordinate closestI and
    *                     closestI+1%nbrCoordinates and 1 <=> coordinate 
    *                     closestI+1%nbrCoordinates is closest.
    *   @return  The distance in mc2-units, squared.
    */
   uint64 squareDistWithOffsetToLine_mc2(coordinate_type lat,
                                         coordinate_type lon, uint16& closestPolygon, uint32& closestI,
                                         float64& closestT, uint16 usePolygon = MAX_UINT16) const;

   /**
    *   A better name would be squaredDistToBoundry.
    *   This funtion takes the distance from
    *   the boundry of our current object to a point lat/lon.
    *
    *   @param   lat      The latitude (in "our" 32bit measure 
    *                     system).
    *   @param   lon      The longitude (in "our" 32bit measure 
    *                     system).
    *   @param   closestI Outparameter that is set to the coordinate
    *                     in the polygon that is closest to
    *                     (lat, lon). If equal to MAX_UINT16
    *                     when calling, this calculation is not done.
    *   @param   closestT Outparameter that is set to the part of the
    *                     distance from closestI (E.g. 0 <=> coordinate 
    *                     closetI is closest, 0.5 <=> the closest
    *                     point to (lat, lon) on the polygon
    *                     is located between coordinate closestI and
    *                     closestI+1%nbrCoordinates and 1 <=> coordinate 
    *                     closestI+1%nbrCoordinates is closest.
    *   @return  The distance in meters squared.
    */
   uint64 squareDistWithOffsetToLine(coordinate_type lat,
                                     coordinate_type lon, uint16& cloestPoly, uint32& closestI,
                                     float64& closestT, uint16 usePolygon = MAX_UINT16) const;

   /**
    *   A better name would be squaredDistToBoundry.
    *   This funtion takes the distance from
    *   the boundry of our current object to a point lat/lon.
    *
    *   @param   lat      The latitude (in "our" 32bit measure 
    *                     system).
    *   @param   lon      The longitude (in "our" 32bit measure 
    *                     system).
    *   @return  The distance in mc2-units, squared.
    */
   uint64 squareDistToLine_mc2(coordinate_type lat,
                               coordinate_type lon, uint16 usePolygon = MAX_UINT16) const;

   /**
    *   A better name would be squaredDistToBoundry.
    *   This funtion takes the distance from
    *   the boundry of our current object to a point lat/lon.
    *
    *   @param   lat      The latitude (in "our" 32bit measure 
    *                     system).
    *   @param   lon      The longitude (in "our" 32bit measure 
    *                     system).
    *   @return  The distance in meters squared.
    */
   uint64 squareDistToLine(coordinate_type lat,
                           coordinate_type lon, uint16 usePolygon = MAX_UINT16) const;

   /**
    *   This function is the same as the above with the differance 
    *   that points inside (if the polygon is closed) have their
    *   distance set to 0.
    *   This function is not to be used on distances where earth 
    *   no longer can be linearised locally!
    *
    *   @param   lat   The latitude (in "our" measure system)
    *   @param   lon   The longitude (in "our" measure system)
    *   @return  The minimum square distance in meters to this 
    *            object. 
    */
   uint64 squareDistTo(coordinate_type lat, coordinate_type lon,
                       uint16 usePolygon = MAX_UINT16) const;

   /**
    *   This function is the same as the above with the differance 
    *   that points inside (if the polygon is closed) have their 
    *   distance set to 0.
    *   This function is not to be used on distances where earth 
    *   no longer can be linearised locally!
    *
    *   @param   otherGfx    The gfxData to check distance to.
    *   @return  The minimum square distance in meters between
    *            otherGfx and this object. 
    */
   uint64 minSquareDistTo(const GfxData* otherGfx) const;

   /**
    *   As squareDistToLine with the differance that points on the inside
    *   of a closed polygon will have their distance counted as negative.
    *   This function is not to be used on distances where earth
    *
    *   no longer can be linearised locally!
    *
    *   @param   lat   The latitude (in "our" measure system)
    *   @param   lon   The longitude (in "our" measure system)
    *   @return  The minimum square distance in meters to this 
    *            object. If (lat, lon) is inside the polygon
    *            the returned distance is negative.
    */
   int64 signedSquareDistTo(coordinate_type lat, coordinate_type lon,
                            uint16 usePolygon = MAX_UINT16) const;

   int64 signedSquareDistTo_mc2(coordinate_type lat, coordinate_type lon,
                            uint16 usePolygon = MAX_UINT16) const;


   /**
    *    Get the angle at given offset. The angle is returned in
    *    degrees from the north direction.
    *
    *    @param   offset   The offset of the polygon where the
    *                      angle should be calculated.
    *    @return  The angle (in deg) of the segment in the
    *             polygon where the offset point is.
    */
   float64 getAngle(uint16 offset) const;

   /**
    *
    *
    */
   uint16 getOffset(int32 lat, int32 lon, 
                    int32& latOnPoly, int32 &lonOnPoly) const;
   //@}

   /**
    *    Find out if this GfxData is equal to another one.
    */
   bool equals(uint16 thisPolygon, uint32 thisCoord, 
               const GfxData* otherGfx,
               uint16 otherPolygon, uint32 otherCoord,
               uint16 factor=1) const;
      
   /**
    *    Find out if this GfxData is equal to another GfxData.
    *    NOTE: Both GfxData must have the same orientation.
    *
    *    @param  otherData   The other GfxData to be compared to
    *                        this GfxData.
    *
    *    @return             True if GfxData and otherData are
    *                        equivalent, false otherwise.
    */
   bool equals(const GfxData* otherData) const;



   /**
    *   Calculates the convex hull of a polygon using Grahams scan.
    *   The indices of the coordinates that belongs to the convex hull
    *   is inserted into the preallocated stack that is given as
    *   parameter.
    *   The method does not use coslat when calculating the to the
    *   points of the polygon. The coslat differs for each coord-pair,
    *   which may give an incorrect result, including points in the convex
    *   hull although they should not be there.
    *   @param stack    The preallocated stack which will be filled with
    *                   the indices of the coordiantes that makes
    *                   the convex hull.
    *   @param polygon  The number of the polygon within this GfxData.
    *   @return   True if the the operation was succesful, 
    *             false otherwise.
    */
   bool getConvexHull(Stack* stack, uint16 polygon);

   /**
    *   Calculates the convex hull for all the polygons of the
    *   GfxData and returns a new GfxData created by
    *   GfxData::createGfxData(NULL). Uses the function
    *   getConvexHull(Stack* stack, uint16 polygon) internally
    *   which seems to work.
    *   @return New GfxData or NULL if failure.
    */
   GfxData* createNewConvexHull() const;
      
   /**
    *    Calculates the convex hull of a set of points using Jarvi's March
    *   method. 
    *   
    *   @param pointSet  A GfxData containing the given points
    *   @return          A GfxData with the convex hull of the set of
    *                    points. It is a clockwise oriented polygon.
    *
    */
   GfxData* getConvexHull(const GfxData* pointSet);
      
   /**
    *    Get the indices of a simplified CLOSED polygon.
    *    These indices are inserted into the preallocated
    *    stack that are given as  parameter. 
    *   
    *
    *    @param stack   This stack will be filled with the indices
    *                   of the coordinates to include in the 
    *                   simplified polygon.
    *
    *    @param polygon The number of the polygon within this GfxData
    *                   to simplify.
    *
    *    @param maxDist The maximum distance between two coordinates.
    *    @param minDist The minimum distance between two coordinates.
    *    @return        True if the stack is filled with indices,
    *                   false otherwise.
    */
   bool getSimplifiedPolygon(Stack* stack, uint16 polygon,
                             uint32 maxDist, uint32 minDist) const;

   /**
    *    Get the indices of a simplified OPEN polygon.
    *    These indices are inserted into the preallocated
    *    stack that are given as parameter.
    *
    *
    *    @param stack         This stack will be filled with the indices
    *                         of the coordinates to include in the 
    *                         simplified polygon.
    *    @param polyIndex     The index of the polygon within this 
    *                         GfxData to simplify.
    *
    *    @param maxLatDist    The maximum deviation (meters) 
    *                         of the given points
    *                         from a new line. Example: If we want to 
    *                         make a new line from point 1 to point 4 
    *                         and therefore skip points 2 and 3, the 
    *                         perpendicular distance to points 2 and 3 
    *                         from the wanted line must be
    *                         smaller than maxLatDist.
    *    @param maxWayDist    The maximum distance (meters)
    *                         between two coordinates.
    *    @param minimizeError [Optional] Default set to false.
    *                         If true, the maxLatDist will be used
    *                         as a hint to where it is needed to insert
    *                         points. The position of the points will
    *                         be selected so that the crossDistance
    *                         error is minimized. Will result in
    *                         a more sophisticated filtering at the
    *                         cost of a longer processing time.
    *                         If this parameter is set, the filtering
    *                         will also work for closed polygons.
    *    @param startIndex    [Default 0] Start index in polygon
    *                         for filtering.
    *    @param endIndex      [Default last index in polygon] Last
    *                         index in polygon for filtering.
    *    @return              True if the stack is filled with indices,
    *                         false otherwise.
    */
   bool openPolygonFilter( Stack* newPoly, uint32 polyIndex,
                           uint32 maxLatDist, uint32 maxWayDist,
                           bool minimizeError = false,
                           uint32 startIndex = 0,
                           uint32 endIndex = MAX_UINT32 );

   /**
    * A new polygon filtering algorithm, uses Douglas-Peucker.
    * Should be faster than openPolygonFilter when heavy filtering
    * is done (lots of coords removed).
    *
    * @param newPoly   Will be filled with the indices of the coordinates
    *                  of the simplified polygon.
    * @param polyIndex The index of the polygon to filter.
    * @param epsilon   The maximum deviation that the simplified polygon
    *                  may have from the original one (in meters).
    * @return          True if the resulting poly is more than just a point.
    */
   bool douglasPeuckerPolygonFilter( Stack& newPoly, 
                                     uint32 polyIndex,
                                     uint32 epsilon ) const;

   /*
    * This method returns the coordinate indexes of all coordinates that exists
    * more than one time in this GfxData.
    *
    * @param Outparameter A multimap with polygon index as key and coordinate 
    *        index as value. It should always be empty when calling the method,
    *        and it is filled in by the call.
    * 
    * @return Returns true if any coordinates were stored in selfTouchCoords.
    *
    */
   bool getMultiCoords(multimap< uint32,uint32 >& selfTouchCoords) const;


   /**
    *    Get the indices of a simplified CLOSED polygon.
    *    These indices are inserted into the preallocated
    *    stack that are given as parameter. All convex vertices
    *    are kept ( vertices which make the polygon smaller if
    *    they are removed ) so that no polygon part is "cut"
    *    Loops may be created in the simplified polygon, so that
    *    triangulation is impossible.
    *
    *    @param stack         This stack will be filled with the indices
    *                         of the coordinates to include in the 
    *                         simplified polygon.
    *    @param polyIndex     The index of the polygon within this GfxData
    *                         to simplify.
    *
    *    @param pMaxLatDist   The maximum deviation in meters of the given
    *                         points from a new line. Example: If we want
    *                         to make a new line from point 1 to point 4
    *                         and therefore skip points 2 and 3, the
    *                         perpendicular distance to points 2 and 3
    *                         from the wanted line must be smaller than
    *                         maxLatDist.
    *
    *    @param pMaxWayDist   The maximum distance in meters between
    *                         two coordinates.
    *    @return              True if the stack is filled with indices,
    *                         false otherwise.        
    */
   bool closedPolygonFilter(Stack* newPoly, uint32 polyIndex,
                            uint32 pMaxLatDist, uint32 pMaxWayDist);

   /**
    *   Calculates the area of a given polygon. Polygon is assumed 
    *   to be simple (i.e not self intersecting) and small enough for
    *   2D-calculations. 
    *
    *   @param    polyIndex  Index of the polygon used.
    *
    *   @return   area of polygon in MC2 units. If GfxData has less than
    *             three nodes, -1 is returned. The area is positive if
    *             the polygon is clockwise oriented, negative otherwise.
    *
    */
   double polygonArea(uint32 polyIndex) const;

   /**
    * 
    *    This method triangulates a given, simple polygon with N
    *    vertices into (N-2) triangles. All the triangles must have
    *    all three corners on the boundary of the polygon, i.e no
    *    new vertices are created to form a triangle. The expected
    *    complexity of the algortithm is N*log(N), worst case 
    *    complexity is O(N²).
    *
    *    @param stack     The stack originally contains the indices
    *                     of a simple, closed polygon. It will be
    *                     cleared and filled with the indices of
    *                     all the triangles. The first three indices
    *                     corrspond to the first triangle, etc.
    *   
    *    @param polyIndex The index of the polygon used to triangulate.
    *    @return          True if the stack is filled with indices,
    *                     false otherwise.
    */
   bool triangulatePolygon(Stack* stack, uint32 polyIndex);

   /**
    *    Function returns 0 for counterclockwise, 1 for clockwise 
    *    and a negative number if there was a problem. Problems 
    *    could be a polygon with no area, or a polygon with less 
    *    than 3 points. Indata is the index of the polygon to be 
    *    investigated and one point on the convex hull. This can be 
    *    found with GfxData::pointOnConvexHull.
    *
    *    Polygon must be checked first so that not two sides of 
    *    the polygon intersect. Nor can two points be equal, and 
    *    no sequence of ABC where A is on the line BC. On that 
    *    kind of behaviour, function might(!) also return negative 
    *    value.
    *
    *    @param   polyIndex   The index of the polygon to find 
    *                         the direction for.
    *    @param   startIndex  The index to start looking at.
    *                         This parameter is DEPRECATED, is not
    *                         used inside the method!
    *    @return  1 for clockwise and 0 for counter clockwise.
    *            -1 is returned upon error.
    */
   int clockWise(int polyIndex, int startIndex = 0) const;

      
   /**
    *   Another filtering algorithm, does not keep the convexity of
    *   the polygon. This means that capes may be cut off.
    *   If the distance between two vertices A and B is too close,
    *   B is removed and A will be compared to the vertex after B.
    *
    *   @param   polygon       The index of the polygon used.
    *
    *   @param   minSqDist     The minimum SQUARED distance allowed
    *                          between two consecutive vertices.
    *                          Distance is in meters.
    */
   
   GfxDataFull* removeTooCloseCoordinates( const uint32 polygon,
                                           const int64 minSqDist );


   /*
    *    
    *   @param  objectBBoxes  MC2BoundingBoxes to avoid collision with
    *   @param  polygon       Index of the polygon used.   
    *   @param  tmpGfx2       Outparameter. A subset of the original GfxData
    *   @param  gfxTextArray  vector with GfxData for GfxFeatures previously
    *                         placed out. 
    *   @return               True if the outdata has more than one coordinate, false otherwise
    */  
   bool getTextPosition( const vector<MC2BoundingBox>& objectBBoxes, 
                         uint16 polygon,  
                         GfxDataFull* tmpGfx2,
                         const vector<GfxData*>& gfxTextArray ) const;
      
   bool closed() const { 
      return getClosed(0); 
   };

   /**
    *   Reads the polygons from the dataBuffer.
    */
   virtual void readPolygons( uint16 nbrPolygons,
                              DataBuffer& dataBuffer, GenericMap* map) = 0;
      
protected:

   /**
    *   Method used in triangulatePolygon method. The triangle T
    *   formed by vertices prevIndex, currIndex and nextIndex in the
    *   polygon P (with number polyIndex) is considered. IF vertex
    *   testIndex in P is inside T, a measure of the perpendicular
    *   distance from testIndex to the line between prevIndex
    *   and nextIndex is determined. 
    *   
    *   @param polyIndex The index of the polygon used to triangulate.
    *   @param testIndex Vertex to test if it is inside a triangle.
    *   @param prevIndex One vertex forming triangle P.
    *   @param currIndex Second vertex forming triangle P.
    *   @param nextIndex Third vertex forming triangle P.
    *
    *   @return          IF testIndex is in P:
    *                    Area of P, which is proportional to the
    *                    perpendicular distance searched for.
    *                    -1 otherwise.
    */
   double distToTriangleSide(uint32 polyIndex, uint32 testIndex,
                             uint32 prevIndex, uint32 currIndex,
                             uint32 nextIndex);
      
   /**
    *   Method used in triangulatePolygon method. The indices
    *   of the polygon are store in polyVec. The index at position
    *   VertexPos in polyVec is deleted by shifting all the
    *   positions after VertexPos one step back. The number of
    *   coordinates in the polygon, nbrCoordInVec, should decrease
    *   by 1 which is done by inserting -1 at position
    *   (nbrCoordInVec - 1).
    *
    *   @param polyVec          The indices of the polygon.
    *   @param nbrCoordInVec    Number of coordinates of the
    *                           polygon, determined by the number
    *                           nonnegative indices in polyVec.
    *                           These are stored at the beginning
    *                           of polyVec.
    *   @param VertexPos        The position in polyVec corresponding
    *                           to the index to be deleted.
    */
   void removePolygonVertex(int32 polyVec[], uint32 nbrCoordInVec,
                            uint32 VertexPos);
      
      
   /**
    *   Method used in triangulatePolygon method. Determines
    *   the number of coordinates of the polygon stored in
    *   polyVec.
    * 
    *   @param polyVec         The indices of the polygon.  
    *   
    *   @return                Number of coordinates in polygon.
    */
   uint32 nbrCoordInSplittedPolygon(int32 polyVec[]);
      
   /**
    *    
    *   Method used in triangulatePolygon method. Splits a 
    *   polygon in two parts. One part is stored in polyVec,
    *   the other part is obtained by swapping parameters
    *   startIndexPos and stopIndexPos whwn calling this function.
    *   
    *   @param                  The indices of the polygon.
    *  
    *   @param nbrCoordInVec    Number of coordinates of the
    *                           polygon, determined by the number
    *                           nonnegative indices in polyVec.
    *                           These are stored at the beginning
    *                           of polyVec.
    *   @param startIndexPos    One vertex in polygon which determines
    *                           where to split polygon.
    *   @param stopIndexPos     The other vertex in polygon which
    *                           determines where to split polygon.
    */                      
   void splitPolygon(int32 polyVec[], uint32 nbrCoordInPoly,
                     uint32 startIndexPos, uint32 stopIndexPos);


   
   typedef set< pair<uint32,uint32> > idxSet_t;
   typedef map< MC2Coordinate, idxSet_t > coordMap_t;
   
   /**
    *    Help method to i.e. mergeTwoPolygons. Fill a coordinate map<> with
    *    coordinates info from a gfx data. The map is filled with
    *    MC2Coordinate as key and a set of <poyIdx, coordIdx> as value.
    *
    *    @param   gfx         The gfx to fill into gfxCoords.
    *    @param   poly        Which polygon of gfx is wanted.
    *    @param   gfxCoords   The coordinate map<> to fill with info.
    *    @param   polyIdxToStore Which polyIdx to use for info from
    *                            this gfx.
    */
   static void fillCoordMap( const GfxData* gfx, uint32 poly,
                             coordMap_t& gfxCoords, uint32 polyIdxToStore );



            
private:
   /**
    *    @name Simplify polygon.
    *    These methods are used by the getSimplifiedPolygon-method.
    */
   //@{
               
   /**
    *    Calculates a simplified hull.
    *    @param stack      Outparameter that is filled with the
    *                      indices of the resulting hull.
    *    @param polyIndex  The index of the polygon within 
    *                      this GfxData.
    *    @param maxSqDist  The squared maximum distance between 
    *                      two coordinates in the resulting hull.
    *    @param minSqDist  The squared minimum distance between 
    *                      two coordinates in the resulting hull.
    *    @return  True if the newPoly-parameter is filled with
    *             indices, false otherwise.
    */
   bool simplifyPoly(Stack* newPoly, uint32 polyIndex, 
                     int64 maxSqist, int64 minSqDist) const;


   /**
    *     Finds and removes ALL the loops on the boundary of a 
    *     filtered polygon. These loops cause the triangulation 
    *     process to terminate. EXPENSIVE method since it
    *     compares all pairs of segments in the polygon.
    *     This means a Nï¿½-complexity.
    *
    *     @param stack     Outparameter that is filled with the
    *                      indices of the resulting hull.
    *     @param polyIndex The index of the polygon within this 
    *                      GfxData.
    */
   void findBoundaryLoops2(Stack* newPoly, uint32 polyIndex) const;
         
         
   /**
    *    Calculates the next index in the polygon vector.
    *    @param step       The step size.
    *    @param index      The startindex.
    *    @param nbrCoord   The total number of coordinates.
    */
   uint32 stepIndex(int step, int32 index, int32 nbrCoord) const;

   /**
    *    Finding the point in the polygon with the highest 
    *    y-coordinate (latitude). If two points have the same
    *    y-coordinate, the one with the highest x-value will be 
    *    chosen. This point will be a point on the convex hull.
    *    @param   polyIndex   Index of the polygon to use.
    *    @return  The index of the coordinate with the 
    *             specifications above.
    */
   uint32 getPointOnConvexHull(int polyIndex) const;

   /**
    *    Does the same as the function GfxData::clockWise,
    *    without using parameter startIndex.
    *    The area is calculated with GfxData::polygonArea, 
    *    and the area will be positive if the orientation
    *    is clockwise, negative otherwise.
    * 
    *    @param   polyIndex   The index of the polygon to find 
    *                         the direction for.
    *    @return  1 for clockwise and 0 for counter clockwise.
    *             -1 is returned upon error.
    */
   int clockWiseByArea(uint32 polyIndex) const;
         

   /**
    *    Get the distance between two points
    *
    *    @param polyIndex  The index of the polygon to look at.
    *    @param first      The index of the first coordinate.
    *    @param secind     The index of the second coordinate.
    *    @return The distance between first and second.
    */
   int64 dist(int32 polyIndex, int32 first, int32 second) const;
         
   /**
    *    Get the distance between two points
    *
    *    @param polyIter   The iterator of the polygon to look at.
    *    @param first      The index of the first coordinate.
    *    @param secind     The index of the second coordinate.
    *    @return The distance between first and second.
    */
   int64 dist(const_iterator polyIter, int32 first, int32 second) const;


         
   //@}
      
   /**
    *    Help method for closedPolygonFilter.
    *    Will check if the corner on the polygon is concave.
    *    Needs at least three coordinates in the polygon.
    *    @param   polyIndex      The polygon index.
    *    @param   currentIndex   The coordinate index for the corner.
    *    @return  If the corner is concave or not.
    */
   bool checkConcaveCorner( uint16 polyIndex, int32 currentIndex );
         
   /**
    *    Help method for getConvexHull.
    *    Gives the angle from north counted clockwise using
    *    the two coordinate pairs that
    *    the supplied ConvexHullNotices correspond to.
    *    Not using coslat since that leads to errors.
    *    @param   notice1  The first ConvexHullNotice, containing
    *                      the startpoints for the line.
    *    @param   notice2  The second ConvexHullNotice, containing
    *                      the end points for the line.
    *    @param   polygon  The polygon of this GfxData that the
    *                      coordinate indices apply to.
    *    @return  The angle in radians from north counted clockwise
    *             @f$ [0 , 2\pi[ @f$.
    */
   float64 getAngleNoCoslat(ConvexHullNotice* notice1, 
                    ConvexHullNotice* notice2,
                    uint16 polygon);

   /**
    *    Internal method for getting boundingbox.
    */
   bool getBBox(coordinate_type& minLat, coordinate_type& maxLat,
                coordinate_type& minLon, coordinate_type& maxLon) const;

   /**   Returns the number of the polygon where the offset point is
    *   only supports the use of one polygon
    *   @param   offset   The offset for the segment
    *   @return  The number of the segment at offset.
    */
   int getSegment(uint16 offset) const;

   /**   Get the orientation of three vertices x, y and z i a polygon.
    *   x,y and z are given as indices in the interval [0...n-1],
    *   where n is number of vertices. The vectors xy and xz
    *   are formed and the z-component of the cross product between
    *   them determines the orientation.
    *  
    *   @param  polyIndex The index for the polygon used
    *   @param  x         First vertex in the given polygon 
    *   @param  y         Second vertex in the given polygon
    *   @param  z         Third vertex in the given polygon
    *   @return The orientation in terms of:
    *           <DL>
    *               <DT>0</DT>
    *                  <DD>If x,y anz are on a straight line.</DD>
    *               <DT>-1</DT>
    *                  <DD>If orientation is counter clockwise.</DD>
    *               <DT>1</DT>
    *                  <DD>If orientation is clockwise.</DD>
    *           </DL>
    */
   int32 orientation(uint32 polyIndex,
                     uint32 x, uint32 y, uint32 z) const;

   /**
    * @see orientation
    */
   int32 orientation(int32 lat1, int32 lon1, int32 lat2, int32 lon2, 
                     int32 lat3, int32 lon3) const;

   /**
    *   Determines if two segments (p1,p2) and (q1,q2) intersect 
    *   at any point.
    *
    *   @param  p1        First vertex in first segment
    *   @param  p2        Second vertex in first segment
    *   @param  q1        First vertex in second segment
    *   @param  q2        Second vertex in second segment
    *   @param  polyIndex Index of the polygon used.
    *   @return True if there is an intersection, false otherwise.
    */ 
   bool segmentIntersect(uint32 polyIndex, uint32 p1,
                         uint32 p2, uint32 q1, uint32 q2) const;

   /**
    *    Function used in method openPolygonFilter. Determines
    *    the perpendicular distance from point k to the line
    *    containing points a and b.
    *
    *    @param  polyIndex Index of the polygon used.
    *    @param  k         Vertex in the open polygon
    *    @param  a         Vertex in the open polygon. 
    *                      Located before k.
    *    @param  b         Vertex in the open polygon. 
    *                      Located after k.
    *    @return           The distance.
    */
   double crossDistance(uint32 polyIndex, uint32 k, uint32 a, uint32 b );

   /**
    *    Function used in method openPolygonFilter. The line L
    *    between points a and b is considered. The distance to L
    *    from all the points between a and b is calculated and
    *    the maximum distance is determined.
    * 
    *    @param  polyIndex Index of the polygon used.
    *    @param  k         Vertex in the open polygon
    *    @param  a         Vertex in the open polygon. Usually
    *                      located before k.
    *    @param  b         Vertex in the open polygon. Usually
    *                      located after k.
    *    @return           The maximum distance.
    */
   double maxCrossDistance(uint32 polyIndex, uint32 a,  uint32 b);
      
   /**
    *    Help method to the filtering methods.
    *    @param   polyIndex   The polygon index.
    *    @param   a           The first coord index.
    *    @param   b           The last coord index.
    *    @return  The index between a and b that minimizes the
    *             crossDistance error.
    */
   uint32 getBestIndexBetween( uint16 polyIndex, uint32 a, uint32 b );
      
   /**
    *        Calculates the are of a triangle with given sides,
    *        using Heron's formula.
    *        The sum of two sides must always be greater than
    *        or equal to the third side. ( triangle inequality)
    *
    *        @param    d1  First sidelength in triangle.
    *        @param    d2  Second sidelength in triangle.
    *        @param    d3  Third  sidelength in triangle.
    *
    *        @return   Area of triangle if closed,
    *                  -1 if figure is unbounded.
    */
   double triangleArea(double d1, double d2, double  d3) const;

   friend class SimulateRouteWindow;
   friend class MESimulateRouteWindow;
   friend class GMSPolyUtility;
};

//-------------------------------------------------------------------------
//                                       Implementation of inlined methods
//-------------------------------------------------------------------------


inline GfxData::coordinate_type
GfxData::getLastLat(uint16 poly) const
{
   MC2_ASSERT(getNbrCoordinates(poly) > 0);
   return getLat(poly, getNbrCoordinates(poly) - 1);
}

inline GfxData::coordinate_type
GfxData::getLastLon(uint16 poly) const
{
   MC2_ASSERT(getNbrCoordinates(poly) > 0);
   return getLon(poly, getNbrCoordinates(poly) - 1);
}


#endif // GFXDATA_H


