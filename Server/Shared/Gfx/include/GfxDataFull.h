/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFXDATAFULL_H
#define GFXDATAFULL_H

#include "config.h"
#include "GfxData.h"
#include "GfxConstants.h"

#include "TileMapCoord.h"

/**
 *    The original implementation of the GfxData. Store allmost all 
 *    information in stl-arrays. Can have 2^16 polygons with infinite
 *    number of coordinates in each. Have a precalculated BoundingBox
 *    and one length per polygon. But only one attribut for closed.
 *
 */   
class GfxDataFull : public GfxData {
   friend class GMSGfxData;
   
private:
   /**
    *    @name Types used in the data representation.
    */
   //@{
   /**
    *    A point is a MC2Coordinate
    */
   typedef TileMapCoord point_type;
         
   /**
    *    A set of points.
    */
   typedef vector<point_type> point_vector_type;

   /**
    *    Point set size type.
    */
   typedef point_vector_type::size_type point_vector_size_type;

   /**
    *    A polygon consists of a set of points and a length.
    *    The length is stored in a float in meters.
    */
   typedef pair<point_vector_type, float32> polygon_type;

   /**
    *    A set of polygons.
    */
   typedef vector<polygon_type> polygon_vector_type;

   /**
    *    Polygon set size type.
    */
   typedef polygon_vector_type::size_type polygon_vector_size_type;
   //@}

public:
   /**
    *    Create an empty object, inlined to optimize speed.
    *    @todo Remove initialization of m_closedFigure.
    */
   GfxDataFull() : m_closedFigure(false) { };

   /**
    *    Delete this object and all data that is created in it.
    */
   virtual ~GfxDataFull() { };

   /**
    *    Fills in the supplied bbox. Fast in this class (if p=MAX_UINT16)
    *    slow in the ones who do not have bounding boxes.
    */
   bool getMC2BoundingBox( MC2BoundingBox& bb, uint16 p = MAX_UINT16) const;

   /**
    *    @name Virtual methods.
    *    This is the implementation of the necessary, virtual methods
    *    in the superclass. See documentation in GfxData.h!
    *    @see GfxData
    */
   //@{
   /// Returns the begin iterator for polygon p
   const_iterator polyBegin( uint16 p ) const;
   /// Returns the end iterator for polygon p
   const_iterator polyEnd( uint16 p ) const;
   /// Returns the begin iterator for polygon p
   iterator polyBegin( uint16 p );
   /// Returns the end iterator for polygon p
   iterator polyEnd( uint16 p );
         
   coordinate_type getLat(uint16 p, uint32 i) const;

   coordinate_type getLon(uint16 p, uint32 i) const;

   uint16 getNbrPolygons() const;

   uint32 getNbrCoordinates(uint16 poly) const;

   bool sortPolygons();

   void setClosed(uint16 poly, bool closed);

   bool getClosed(uint16 poly) const;

   coordinate_type getMinLat() const;

   coordinate_type getMaxLat() const;

   coordinate_type getMinLon() const;

   coordinate_type getMaxLon() const;

   float64 getCosLat() const;

   float64 getLength(uint16 p) const;
        
   float64 getLengthAll() const;
 
   virtual bool updateLength(uint16 polindex = MAX_UINT16);

   void setLength(uint16 p, float64 length);

   bool updateBBox();

   bool updateBBox(const coordinate_type lat, 
                   const coordinate_type lon);

   /**
    *   Removes identical coordinates from the GfxData.
    */
   void removeIdenticalCoordinates();
   /**
    *   Removes dead-end coordinates from the gfxData.
    *   Dead-end coordinates are such, that are in the gfxData 
    *   of closed polygons but are not on the gfx-border.
    */
   void removeDeadEndCoordinates();

   bool add(const GfxData* otherData, bool backwards = false, 
            uint16 otherPolygon = 0);

   bool addPolygon(const GfxData* otherData, 
                   bool backwards = false, 
                   uint16 otherPolygon = 0);

   /**
    *    Replace one specific polygon in my gfxData with a polygon
    *    from another gfxData.
    */
   bool replacePolygon( uint16 polygon, 
            const GfxData* otherData, uint16 otherPolygon );


   void quickAddCoordinate(coordinate_type lat, 
                           coordinate_type lon,
                           uint32 nbrCoordsInCurPoly = 0) {
      m_polygons.back().first.push_back(MC2Coordinate(lat, lon));
   }

   template <typename Iterator>
   inline void addCoordinates( const Iterator& begin, 
                               const Iterator& end ) ;

   bool addCoordinate( coordinate_type lat, 
                       coordinate_type lon, 
                       bool createNewPolygon = false, 
                       bool front = false );


   inline bool addCoordinate( const MC2Coordinate& coord ) {
      return addCoordinate( coord.lat, coord.lon );
   }

   /**
    *    Insert a coordinate into a certain poly of this
    *    gfx data. The coord is inserted into the poly
    *    before the given coordIdx.
    */
   bool addCoordinateBeforeSpecificCoordIdx( 
         coordinate_type lat, coordinate_type lon,
         uint16 polygon, uint32 coordIndex,
         bool updateBBoxAndLength = false );

   bool deleteCoordinate(uint16 polygon, uint32 index);

   bool setCoordinate(uint16 polygon, uint32 index, 
                      coordinate_type lat, coordinate_type lon,             
                      bool updateBBoxAndLength = false );         
         
   void addPolygon(uint32 preAllocatedSize = 0);
   //@}         

   /// Removes everything but one coordinate.
   void setToSinglePoint( const MC2Coordinate& coord );
         
   struct LessNbrCoord :
      public binary_function<polygon_type, polygon_type, bool> {
      bool operator()(polygon_type x, polygon_type y) {
         return (y.first.size() < x.first.size());
      }
   };

   void reserveCoordinates( uint32 poly, uint32 nbrCoordinates ) {
      m_polygons[ poly ].first.reserve( nbrCoordinates );
   }
protected:
   /**
    *   Reads the polygons from the dataBuffer.
    */
   void readPolygons( uint16 nbrPolygons,
                      DataBuffer& dataBuffer,
                      GenericMap* theMap );

   void createFromDataBuffer( DataBuffer& buff );
private:

   uint16 getNbrPolygons_inline() const;
   uint32 getNbrCoordinates_inline( uint32 poly ) const;
   GfxData::coordinate_type getLat_inline(uint16 p, uint32 i) const;
   GfxData::coordinate_type getLon_inline(uint16 p, uint32 i) const;
   
   /**
    *    Create an object that is a copy of an other GfxData.
    *    Private to avoid usage! 
    *    Use GfxData::createNewGfxData(GenericMap*, GfxData*)
    */
   GfxDataFull(const GfxDataFull& );
      
   /**
    *    Create an object that is a copy of an other GfxData.
    *    Private to avoid usage! 
    *    Use GfxData::createNewGfxData(GenericMap*, GfxData*)
    */
   GfxDataFull(const GfxData& );

   /**
    *    The polygons of this GfxData.
    */
   polygon_vector_type m_polygons;


   /**
    *    The bounding box for this item.
    */
   MC2BoundingBox m_bbox;

   /**
    *   Are the polygons closed or not? Observe that this member
    *   variable applies to <b>all</b> polygons.
    */
   bool m_closedFigure;
};

template <typename Iterator>
inline void GfxDataFull::
addCoordinates( const Iterator& begin,
                const Iterator& end )
{
   MC2_ASSERT( ! m_polygons.empty() );

   Iterator it( begin );

   for (; it != end; ++it ) {
      m_polygons.back().first.push_back( *it );
      m_bbox.update( *it, false ); // dont update cosLat
   }
   m_bbox.updateCosLat();
   // Set invalid length
   m_polygons.back().second = -1;
}

#endif

