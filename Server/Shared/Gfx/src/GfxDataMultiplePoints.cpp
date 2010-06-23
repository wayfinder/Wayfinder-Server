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

#include "GfxConstants.h"
#include "GfxDataMultiplePoints.h"
#include "Math.h"
#include "DataBuffer.h"

GfxDataMultiplePoints::GfxDataMultiplePoints(const GfxDataMultiplePoints& gfx)
{
   
}

      
GfxDataMultiplePoints::GfxDataMultiplePoints(const GfxData& gfx)
{
   
}

GfxDataMultiplePoints::~GfxDataMultiplePoints()
{
   
}

GfxData::const_iterator
GfxDataMultiplePoints::polyBegin( uint16 p ) const
{
   return &(m_coordinates[p]);
}

GfxData::const_iterator
GfxDataMultiplePoints::polyEnd( uint16 p ) const
{
   return &(m_coordinates[p+1]);
}

GfxData::iterator
GfxDataMultiplePoints::polyBegin( uint16 p )
{
   return &(m_coordinates[p]);
}

GfxData::iterator
GfxDataMultiplePoints::polyEnd( uint16 p )
{
   return &(m_coordinates[p+1]);
}

GfxData::coordinate_type
GfxDataMultiplePoints::getLat(uint16 p, uint32 i) const
{
   MC2_ASSERT(i == 0);
   if (p < getNbrPolygons())
      return m_coordinates[p].lat;
   return GfxConstants::IMPOSSIBLE;
}

GfxData::coordinate_type
GfxDataMultiplePoints::getLon(uint16 p, uint32 i) const
{
   MC2_ASSERT(i == 0);
   if (p < getNbrPolygons())
      return m_coordinates[p].lon;
   return GfxConstants::IMPOSSIBLE;
}

uint16
GfxDataMultiplePoints::getNbrPolygons() const
{
   return m_coordinates.size();
}

uint32
GfxDataMultiplePoints::getNbrCoordinates(uint16 poly) const
{
   MC2_ASSERT(poly < getNbrPolygons());
   return 1;
}

bool
GfxDataMultiplePoints::sortPolygons()
{
   return true; // One coordinate in one polygon is always sorted...
}

void
GfxDataMultiplePoints::setClosed(uint16 poly, bool closed)
{
   
}

bool
GfxDataMultiplePoints::getClosed(uint16 poly) const
{
   return false;
}


GfxData::coordinate_type
GfxDataMultiplePoints::getMinLat() const
{
   coordinate_type minLat = MAX_INT32;
   for (uint32 p=0; p<getNbrPolygons(); ++p) {
      if (minLat > m_coordinates[p].lat)
         minLat = m_coordinates[p].lat;
   }
   mc2dbg8 << "GfxDataMultiplePoints::getMinLat() returning " 
           << minLat << endl;
   return minLat;
}

GfxData::coordinate_type
GfxDataMultiplePoints::getMaxLat() const
{
   GfxData::coordinate_type maxLat = MIN_INT32;
   for (uint32 p=0; p<getNbrPolygons(); ++p) {
      if (maxLat < m_coordinates[p].lat)
         maxLat = m_coordinates[p].lat;
   }
   mc2dbg8 << "GfxDataMultiplePoints::getMaxLat() returning " 
           << maxLat << endl;
   return maxLat;
}

GfxData::coordinate_type
GfxDataMultiplePoints::getMinLon() const
{  
   uint32 nbrPoly = getNbrPolygons();
   if (nbrPoly > 0) {
      GfxData::coordinate_type minLon = m_coordinates[0].lon;
      for (uint32 p=1; p<nbrPoly; ++p) {
         if ( ((int32) (m_coordinates[p].lon - minLon) ) < 0 )
            minLon = m_coordinates[p].lon;
      }
      mc2dbg8 << "GfxDataMultiplePOints::getMinLon() returning "
              << minLon << endl;
      return minLon;
   }
   mc2log << error << here << " No coordinates!" <<endl;
   return GfxConstants::IMPOSSIBLE;
}

GfxData::coordinate_type
GfxDataMultiplePoints::getMaxLon() const
{  
   uint32 nbrPoly = getNbrPolygons();
   if (nbrPoly > 0) {
      GfxData::coordinate_type maxLon = m_coordinates[0].lon;
      for (uint32 p=1; p<nbrPoly; ++p) {
         if ( ((int32) (m_coordinates[p].lon - maxLon) ) > 0 )
            maxLon = m_coordinates[p].lon;
      }
      mc2dbg8 << "GfxDataMultiplePoints::getMaxLon() returning " 
              << maxLon << endl;
      return maxLon;
   }
   mc2log << error << here << " No coordinates!" <<endl;
   return GfxConstants::IMPOSSIBLE;
   
}

float64
GfxDataMultiplePoints::getCosLat() const
{
   return cos( (2*M_PI/ POW2_32 * ((getMaxLat()/2)+(getMinLat()/2))));
}


float64
GfxDataMultiplePoints::getLength(uint16 p) const
{
   MC2_ASSERT(p < getNbrPolygons());
   return 0;
}

         
bool
GfxDataMultiplePoints::updateLength(uint16 p)
{
   MC2_ASSERT( (p < getNbrPolygons()) || (p == MAX_UINT16));
   return true;
}

void
GfxDataMultiplePoints::setLength(uint16 p, float64 length)
{
   MC2_ASSERT(p < getNbrPolygons());
}

bool
GfxDataMultiplePoints::updateBBox()
{
   return true;
}

bool
GfxDataMultiplePoints::updateBBox(const GfxData::coordinate_type lat, 
                                 const GfxData::coordinate_type lon)
{
   return true;
}

void
GfxDataMultiplePoints::readPolygons( uint16 nbrPolygons,
                                     DataBuffer& dataBuffer, 
                                     GenericMap* map )
{
   m_coordinates.clear();
   coord_t coord;
   for ( uint32 i = 0; i < nbrPolygons; ++i ) {
      uint32 nbrCoords = dataBuffer.readNextLong();      
      for ( uint32 p = 0; p < nbrCoords; ++p ) {
         coord.lat = dataBuffer.readNextLong();
         coord.lon = dataBuffer.readNextLong();
         m_coordinates.push_back( coord );
      }
      setLength( i, dataBuffer.readNextFloat() );
   }
}
