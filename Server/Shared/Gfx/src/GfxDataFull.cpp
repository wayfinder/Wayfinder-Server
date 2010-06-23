/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GfxDataFull.h"
#include "GfxUtility.h"
#include "DataBuffer.h"

GfxDataFull::GfxDataFull(const GfxDataFull& gfx) 
{
   mc2log << fatal 
          << "GfxDataFull::GfxDataFull(const GfxDataFull& gfx)" 
          << " not implemented" << endl;
   MC2_ASSERT(false);
}

GfxDataFull::GfxDataFull(const GfxData& gfx) 
{
   mc2log << fatal 
          << "GfxDataFull::GfxDataFull(const GfxData& gfx)" 
          << " not implemented" << endl;
   MC2_ASSERT(false);
}

GfxData::const_iterator
GfxDataFull::polyBegin( uint16 p ) const
{
   return &(*m_polygons[p].first.begin());
}

GfxData::const_iterator
GfxDataFull::polyEnd( uint16 p ) const
{
   return &(*m_polygons[p].first.end());
}

GfxData::iterator
GfxDataFull::polyBegin( uint16 p )
{
   return &(*m_polygons[p].first.begin());
}

GfxData::iterator
GfxDataFull::polyEnd( uint16 p )
{
   return &(*m_polygons[p].first.end());
}

inline uint16
GfxDataFull::getNbrPolygons_inline() const
{
   return uint16(m_polygons.size());
}

inline uint32
GfxDataFull::getNbrCoordinates_inline(uint32 p) const
{
   if (p < getNbrPolygons_inline())
      return m_polygons[p].first.size();
   mc2log << warn << "Tries to get nbr coordinats for invalid polygon:"
          << p << " of " << getNbrPolygons() << endl;
   return 0;
}

inline GfxData::coordinate_type
GfxDataFull::getLon_inline(uint16 p, uint32 i) const
{
   if ( (p < getNbrPolygons_inline()) && 
        (i < getNbrCoordinates_inline(p))) {
      return (m_polygons[p].first[i].lon);
   }
   mc2log << warn << "Tries to get invalid lonitude ("
          << p << ", " << i << ")" << endl;
   return (GfxConstants::IMPOSSIBLE);
}

GfxData::coordinate_type
GfxDataFull::getLat_inline(uint16 p, uint32 i) const
{
   if ( (p < getNbrPolygons_inline()) && 
        (i < getNbrCoordinates_inline(p))) {
      return (m_polygons[p].first[i].lat);
   }
   mc2log << warn << "Tries to get invalid latitude ("
          << p << ", " << i << ")" << endl;
   return (GfxConstants::IMPOSSIBLE);
}


GfxData::coordinate_type 
GfxDataFull::getLat(uint16 p, uint32 i) const
{
   return getLat_inline(p, i);
}

GfxData::coordinate_type 
GfxDataFull::getLon(uint16 p, uint32 i) const
{
   return getLon_inline(p, i);
}

uint16 
GfxDataFull::getNbrPolygons() const
{
   return getNbrPolygons_inline();
}

uint32 
GfxDataFull::getNbrCoordinates(uint16 poly) const
{
   return getNbrCoordinates_inline(poly);
}

bool 
GfxDataFull::sortPolygons() {
   sort(m_polygons.begin(), m_polygons.end(), LessNbrCoord());
   return true;
}

void 
GfxDataFull::setClosed(uint16 poly, bool closed) {
   m_closedFigure = closed;
};

bool 
GfxDataFull::getClosed(uint16 poly) const {
   return m_closedFigure;
};

GfxData::coordinate_type 
GfxDataFull::getMinLat() const {
   return m_bbox.getMinLat();
};

GfxData::coordinate_type 
GfxDataFull::getMaxLat() const {
   return m_bbox.getMaxLat();
}

GfxData::coordinate_type 
GfxDataFull::getMinLon() const {
   return m_bbox.getMinLon();
}

GfxData::coordinate_type 
GfxDataFull::getMaxLon() const {
   return m_bbox.getMaxLon();
}

float64 
GfxDataFull::getCosLat() const {
   return m_bbox.getCosLat();
}

float64 
GfxDataFull::getLength(uint16 p) const
{
   MC2_ASSERT( p < getNbrPolygons_inline() );

   if ( m_polygons[p].second < 0 ) {
      const_cast<GfxDataFull*>(this)->updateLength( p );
   }
   return m_polygons[p].second;   
}

float64 
GfxDataFull::getLengthAll() const
{
   uint32 nbrOfPolygons = getNbrPolygons();
   float64 sum = 0;

   for( uint32 i=0; i < nbrOfPolygons; i++) {
      sum += getLength( i );
   }

   return sum;
}


bool
GfxDataFull::updateLength(uint16 polindex)
{

   uint32 start_idx = polindex;
   uint32 end_idx   = polindex + 1;
   if ( polindex == MAX_UINT16 ) {
      // All polygons
      start_idx = 0;
      end_idx   = m_polygons.size();      
   }

   for ( uint32 i = start_idx; i != end_idx; ++i ) {
      m_polygons[i].second = calcPolyLength( i );
   }
   
   return true;
}

void 
GfxDataFull::setLength(uint16 p, float64 length) 
{
   m_polygons[p].second = length;
}

bool
GfxDataFull::getMC2BoundingBox( MC2BoundingBox& bb, uint16 p) const
{
   if ( p == MAX_UINT16 ) {
      // Complete bbox requested - I have that.
      bb = m_bbox;
      return true;
   } else {
      return GfxData::getMC2BoundingBox( bb, p );
   }   
}


bool
GfxDataFull::updateBBox()
{
   m_bbox.reset();
   const uint32 nbrPoly = getNbrPolygons_inline();
   // For all polygons
   for( uint32 p = 0; p < nbrPoly; ++p ) {
      GfxData::const_iterator begin = polyBegin(p);
      GfxData::const_iterator end   = polyEnd(p);
      // For all coords in polygon.
      for ( GfxData::const_iterator it = begin; it != end; ++it ) {
         m_bbox.update(*it, false);
      }
   }
   m_bbox.updateCosLat();
   return true;
}

bool
GfxDataFull::updateBBox(const coordinate_type lat, const coordinate_type lon)
{
   m_bbox.update(lat, lon);
   return true;
}

void
GfxDataFull::removeIdenticalCoordinates()
{
   typedef polygon_vector_type::iterator polygon_iter;
   typedef point_vector_type::iterator point_iter;

   // Remove the duplicates.
   for(polygon_iter i = m_polygons.begin(); i != m_polygons.end(); i++) {
      point_vector_type& polygon = (*i).first;

      // If not-closed polygon with > 1 coordinate (e.g. street segment)
      // - we must always keep at least 2 coordinates! ( = the nodes)
      if ( !closed() && (polygon.size() == 2) ) {
         continue;
      }
      
      point_iter new_end = unique(polygon.begin(), polygon.end());
      polygon.erase(new_end, polygon.end());

      // Check if first and last is the same if closed
      if ( (closed()) && (polygon.size() > 1) &&
           (polygon.front().lat  == polygon.back().lat) &&
           (polygon.front().lon == polygon.back().lon)) {
           //(polygon.begin()->first  == (polygon.end()-1)->first) &&
           //(polygon.begin()->second == (polygon.end()-1)->second)) {
         polygon.erase(polygon.end()-1);
      }
   }
}


void
GfxDataFull::removeDeadEndCoordinates()
{
   mc2dbg8 << "GfxDataFull removeDeadEndCoordinates" << endl;
   typedef polygon_vector_type::iterator polygon_iter;
   typedef point_vector_type::iterator point_iter;

   uint32 nbrCoordsBefore = getTotalNbrCoordinates();

   uint32 poly = 0;
   // Remove the duplicates.
   for(polygon_iter i = m_polygons.begin(); i != m_polygons.end(); i++) {

      point_vector_type& polygon = (*i).first;
      mc2dbg4 << " new poly " << poly << ":" << polygon.size() 
              << " closed=" << closed() << " " << getClosed(0) << endl;
      
      // Remove dead-ends
      if (closed() && (polygon.size() > 3)) {
         point_iter prev = polygon.begin();
         point_iter cur = prev;
         cur++;
         point_iter next = cur;
         next++;
         
         while (cur != polygon.end()) {
          
            // to cover the case that the end of the polygon is a dead end
            if (next == polygon.end()) {
               next = polygon.begin();
               mc2dbg8 << " updating next to begin." << endl;
            }
            
            mc2dbg8 << *prev << " " << *cur << " " << *next << endl;
            
            if ( *prev == *cur ) {
               // two identical coords in a row, caused by previous removal

               if (prev != polygon.begin()) {
                  //remove prev
                  prev--;
                  cur--;
                  if (next != polygon.begin())
                     next--;
                  else 
                     next = polygon.end()-1;
                  mc2dbg4 << "removing prev (identical): " 
                          << *prev << " " << *cur << " " << *next << endl;
                  polygon.erase(cur);
               } else {
                  mc2dbg4 << "removing cur (identical): "
                         << *prev << " " << *cur << " " << *next << endl;
                  polygon.erase(cur);
               }
            
            } else if ( (next != polygon.end()) && 
                        (*prev == *next) )  {
               // cur is inbetween two identical coords (a dead end), remove
               mc2dbg4 << "removing cur (between identical): "
                       << *prev << " " << *cur << " " << *next << endl;
               polygon.erase(cur);
               
            } else {
               // update
               prev++;
               cur++;
               next++;
            }
         }

         mc2dbg8 << "Checking begin-end" << endl;
         // if the polygon begins/ends in the dead end...
         bool oneRemoved = true;
         while (oneRemoved) {
            oneRemoved = false;
            if ((polygon.size() > 3)) {
               // use the last and the two first coords in the polygon
               // the last have already been removed above
               prev = polygon.end()-1;
               cur = polygon.begin();
               next = cur;
               next++;
               mc2dbg8 << *prev << " " << *cur << " " << *next << endl;
               
               if ( *prev == *cur ) {
                  mc2dbg4 << " removing prev (identical _end): "
                          << *prev << " " << *cur << " " << *next << endl;
                  polygon.erase(prev);
                  oneRemoved = true;
               } else if ( *prev == *next ) {
                  mc2dbg4 << " removing cur (between identical _end): "
                          << *prev << " " << *cur << " " << *next << endl;
                  polygon.erase(cur);
                  oneRemoved = true;
               }
            }   
         }

      }
      poly++;
   }
   updateLength();
   updateBBox();
   
   uint32 nbrCoordsAfter = getTotalNbrCoordinates();
   mc2dbg4 << "GfxData::removeDeadEndCoordinates, removed "
           << ( nbrCoordsBefore-nbrCoordsAfter ) << " coordinates" << endl;
}



bool
GfxDataFull::add(const GfxData* otherData, bool backwards, uint16 otherP)
{
   // Check myself
   if(getNbrPolygons_inline() < 1) {
      mc2log << error << "GfxData::add Has no polygons to add to" << endl;
      return false;
   }

   // Check the otherPolygon-parameter
   if (otherData == NULL ||
       otherData->getNbrPolygons() < 1 || 
       otherP >= otherData->getNbrPolygons()) {
      mc2log << error
             << "GfxData::add otherP >= otherData->getNbrPolygons()"
             << endl;
      return false;
   }

   if ( ! backwards ) {
      // Prolly quick.
      addCoordinates( otherData->polyBegin( otherP ),
                      otherData->polyEnd( otherP ) );
      // Length will be updated if needed
   } else {
      for (int32 i=otherData->getNbrCoordinates(otherP)-1; i>=0; --i) {
         if (!addCoordinate(otherData->getLat(otherP,i), 
                            otherData->getLon(otherP, i))) 
            return false;
      }
      updateLength(getNbrPolygons_inline() - 1); // update last polygon
   }

   return true;
}

bool
GfxDataFull::addPolygon( const GfxData* otherData, bool backwards,
                         uint16 otherPolygon )
{
   if(otherData == NULL
         || otherData->getNbrPolygons() < 1
         || otherPolygon >= otherData->getNbrPolygons()) {
      mc2log << error << "GfxData::addPolygon otherPolygon doesn't exist"
             << endl;
      return false;
   }
   
   // Only MAX_UINT16 nbr polygons are allowed.
   if (getNbrPolygons_inline() >= MAX_UINT16) {
      mc2log << error << "GfxData::addPolygon Polygon not added since there "
             <<"already exists 65536 polygons for this GfxData" << endl;
      return (false);
   }

   point_vector_type newPolygon;
   m_polygons.push_back( make_pair(newPolygon, -1) );

   return add(otherData, backwards, otherPolygon);
}

void
GfxDataFull::addPolygon(uint32 preAllocatedSize)
{
   point_vector_type newPolygon;
   m_polygons.push_back(make_pair(newPolygon, -1));

   // Reserve space for the coordinate-pairs
   if (preAllocatedSize > 0) {
      point_vector_type& cv = m_polygons.back().first;
      cv.reserve(preAllocatedSize);
   }
}

bool
GfxDataFull::addCoordinate( coordinate_type lat, coordinate_type lon,
                            bool createNewPolygon, bool front )
{
  if((m_polygons.size() < 1) && (! createNewPolygon)){
      mc2log << error << "GfxDataFull::addCoordinate No polygons to add to"
             << endl;
      return false;
   }

   if (createNewPolygon) {
      addPolygon();
   }

   if (front) {
      // Add coordinate to the beginning of the last polygon.
      m_polygons.back().first.insert(
            m_polygons.back().first.begin(), point_type( lat, lon ) );
   } else {
      m_polygons.back().first.push_back( point_type( lat, lon ) );
   }

   updateBBox( lat, lon );
   return true;
}

bool
GfxDataFull::addCoordinateBeforeSpecificCoordIdx( 
      coordinate_type lat, coordinate_type lon,
      uint16 polygon, uint32 coordIndex, bool updateBBoxAndLength )
{
   // Lookup polygon containing point index
   MC2_ASSERT(polygon < getNbrPolygons());
   MC2_ASSERT(coordIndex < getNbrCoordinates(polygon));

   point_vector_type::iterator p = m_polygons[polygon].first.begin();
   p += coordIndex;
   m_polygons[polygon].first.insert( p, point_type( lat, lon ) );

   if ( updateBBoxAndLength ) {
      updateLength(polygon);
      updateBBox();
   }
   
   return true;
}

bool
GfxDataFull::deleteCoordinate(uint16 polygon, uint32 coordIndex)
{
   // Lookup polygon containing point index
   MC2_ASSERT(polygon < getNbrPolygons());
   MC2_ASSERT(coordIndex < getNbrCoordinates(polygon));

   point_vector_type::iterator p = m_polygons[polygon].first.begin();
   p += coordIndex;

   m_polygons[polygon].first.erase(p);

   updateLength(polygon);
   updateBBox();

   return true;
}

bool 
GfxDataFull::setCoordinate(uint16 polygon, uint32 index, 
                           coordinate_type lat, coordinate_type lon,
                           bool updateBBoxAndLength )
{
   if ( ( polygon < getNbrPolygons() ) && 
        ( index < getNbrCoordinates( polygon ) ) ) {

      polygon_type& thePolygon = m_polygons[ polygon ];
      thePolygon.first[ index ].lat = lat;
      thePolygon.first[ index ].lon = lon;
      
      if ( updateBBoxAndLength ) {
         updateBBox();
         updateLength();
      }
      return true;

   } else {
      return false;
   }
}

void
GfxDataFull::setToSinglePoint( const MC2Coordinate& coord )
{
   // Check if the GfxDataFull contains more than one polygon
   // or a polygon has more than one coordinate.
   if ( m_polygons.size() != 1 ||
        ( ( !m_polygons.empty() ) && ( m_polygons[0].first.size() != 1 ) ) ) {
      // Make one polygon with one point
      m_polygons.clear();
      m_polygons.push_back( polygon_type() );
      m_polygons.front().first.push_back( coord );
   }
   // Set the coordinate and update the bbox etc.
   setCoordinate( 0, 0, coord.lat, coord.lon, true );
   m_closedFigure = false;
}



void
GfxDataFull::readPolygons( uint16 nbrPolygons,
                           DataBuffer& dataBuffer,
                           GenericMap* theMap )
{
   m_bbox.reset();
   m_polygons.resize( nbrPolygons );
   
   for ( uint32 poly = 0; poly < nbrPolygons; ++poly ) {
      uint32 nbrCoords = dataBuffer.readNextLong();  // nbrCoords (uint32)
      m_polygons[ poly ].first.resize( nbrCoords );

      // Add the points
      point_vector_type& vect = m_polygons[ poly ].first;
      for ( point_vector_type::iterator it = vect.begin();
            it != vect.end();
            ++it ) {
         it->lat = dataBuffer.readNextLong();
         it->lon = dataBuffer.readNextLong();
         m_bbox.update( *it, false );
      }

      // Set the length
      setLength( poly, dataBuffer.readNextFloat() );
   }

   m_bbox.updateCosLat();
}

void
GfxDataFull::createFromDataBuffer( DataBuffer& dataBuffer )
{
   dataBuffer.alignToLong();

   setClosed( 0, dataBuffer.readNextBool() ); 
   dataBuffer.readNextByte(); // Pad 1 byte(type)
   uint16 nbrPolygons = dataBuffer.readNextShort();

   for ( uint16 i = 0; i < nbrPolygons; i++ ) { 
      uint32 nbrCoords = dataBuffer.readNextLong();
      // Will reserve space
      addPolygon( nbrCoords );

      // Add the points
      for ( uint32 j = 0; j < nbrCoords; j++ ) {
         // Must read into temporaries to get the order right
         uint32 tempLat = dataBuffer.readNextLong();
         uint32 tempLon = dataBuffer.readNextLong();
         addCoordinate( tempLat, tempLon );
      }
      // Set the length
      setLength( i, dataBuffer.readNextFloat() );
   }

   updateBBox();
}


// Inspired from GfxDataFull::addCoordinates
bool
GfxDataFull::replacePolygon( 
   uint16 polygon, const GfxData* otherData, uint16 otherPolygon )
{
   // Lookup polygon containing point index
   MC2_ASSERT(polygon < getNbrPolygons());
   polygon_type& thePolygon = m_polygons[ polygon ];

   // clear it
   thePolygon.first.clear();

   // add the coordinated from the other gfxdata poly
   GfxData::const_iterator end = otherData->polyEnd( otherPolygon );
   for ( GfxData::const_iterator it = otherData->polyBegin( otherPolygon );
         it != end; it++ ) {
      thePolygon.first.push_back( *it );
      m_bbox.update( *it, false ); // dont update cosLat
   }
   m_bbox.updateCosLat();
    
   updateLength(polygon);
   updateBBox();

   return true;
}
