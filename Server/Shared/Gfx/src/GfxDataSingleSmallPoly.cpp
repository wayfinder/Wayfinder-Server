/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GfxDataSingleSmallPoly.h"


#include "config.h"
#include "Math.h"

#include "GfxUtility.h"
#include "GfxConstants.h"
#include "DataBuffer.h"
#include "TileMapCoord.h"
#include "GenericMap.h"
#include "AllocatorTemplate.h"

GfxDataSingleSmallPoly::GfxDataSingleSmallPoly(
                                    const GfxDataSingleSmallPoly& gfx)
      : GfxData(gfx), m_coordinates(NULL), m_nbrCoordinates(0), m_closed(false) 
{
   
}

      
GfxDataSingleSmallPoly::GfxDataSingleSmallPoly(const GfxData& gfx)
      : GfxData(gfx), m_coordinates(NULL), m_nbrCoordinates(0), m_closed(false) 
{
   
}

GfxDataSingleSmallPoly::~GfxDataSingleSmallPoly()
{

}

GfxData::const_iterator
GfxDataSingleSmallPoly::polyBegin( uint16 p ) const
{
   return m_coordinates;
}

GfxData::const_iterator
GfxDataSingleSmallPoly::polyEnd( uint16 p ) const
{
   return m_coordinates + m_nbrCoordinates;
}

GfxData::iterator
GfxDataSingleSmallPoly::polyBegin( uint16 p )
{
   return m_coordinates;
}

GfxData::iterator
GfxDataSingleSmallPoly::polyEnd( uint16 p )
{
   return m_coordinates + m_nbrCoordinates;
}


GfxData::coordinate_type
GfxDataSingleSmallPoly::getLat(uint16 p, uint32 i) const
{
   MC2_ASSERT(p == 0);
   //MC2_ASSERT(i < getNbrCoordinates(0));
   if (i < m_nbrCoordinates)
      return m_coordinates[i].lat;
   return GfxConstants::IMPOSSIBLE;
}

GfxData::coordinate_type
GfxDataSingleSmallPoly::getLon(uint16 p, uint32 i) const
{
   if (i < m_nbrCoordinates)
      return m_coordinates[i].lon;
   return GfxConstants::IMPOSSIBLE;
}

uint16
GfxDataSingleSmallPoly::getNbrPolygons() const
{
   return 1;
}

uint32
GfxDataSingleSmallPoly::getNbrCoordinates(uint16 poly) const
{
   return m_nbrCoordinates;
}

bool
GfxDataSingleSmallPoly::sortPolygons()
{
   return true; // One polygon is always sorted...
}

void
GfxDataSingleSmallPoly::setClosed(uint16 poly, bool closed)
{
   m_closed = closed;
}

bool
GfxDataSingleSmallPoly::getClosed(uint16 poly) const
{
   return m_closed;
}

GfxData::coordinate_type
GfxDataSingleSmallPoly::getMinLat() const
{
   coordinate_type minLat = MAX_INT32;
   for (uint32 i=0; i<m_nbrCoordinates; ++i) {
      if (minLat > m_coordinates[i].lat)
         minLat = m_coordinates[i].lat;
   }
   mc2dbg8 << "GfxDataSingleSmallPoly::getMinLat() returning " 
           << minLat << endl;
   return minLat;
}

GfxData::coordinate_type
GfxDataSingleSmallPoly::getMaxLat() const
{
   GfxData::coordinate_type maxLat = MIN_INT32;
   for (uint32 i=0; i<m_nbrCoordinates; ++i) {
      if (maxLat < m_coordinates[i].lat)
         maxLat = m_coordinates[i].lat;
   }
   mc2dbg8 << "GfxDataSingleSmallPoly::getMaxLat() returning " 
           << maxLat << endl;
   return maxLat;
}

GfxData::coordinate_type
GfxDataSingleSmallPoly::getMinLon() const
{
   if (m_nbrCoordinates > 0) {
      GfxData::coordinate_type minLon = m_coordinates[0].lon;
      for (uint32 i=1; i<m_nbrCoordinates; ++i) {
         if ( ((int32) (m_coordinates[i].lon - minLon) ) < 0 )
            minLon = m_coordinates[i].lon;
      }
      mc2dbg8 << "GfxDataSingleSmallPoly::getMinLon() returning " 
              << minLon << endl;
      return minLon;
   }
   mc2log << error << here << " No coordinates!" <<endl;
   return GfxConstants::IMPOSSIBLE;
}

GfxData::coordinate_type
GfxDataSingleSmallPoly::getMaxLon() const
{
   if (m_nbrCoordinates > 0) {
      GfxData::coordinate_type maxLon = m_coordinates[0].lon;
      for (uint32 i=1; i<m_nbrCoordinates; ++i) {
         if ( ((int32) (m_coordinates[i].lon - maxLon) ) > 0 )
            maxLon = m_coordinates[i].lon;
      }
      mc2dbg8 << "GfxDataSingleSmallPoly::getMaxLon() returning " 
              << maxLon << endl;
      return maxLon;
   }
   mc2log << error << here << " No coordinates!" <<endl;
   return GfxConstants::IMPOSSIBLE;
   
}

float64
GfxDataSingleSmallPoly::getCosLat() const
{
   return cos( (2*M_PI/ POW2_32 * ((getMaxLat()/2)+(getMinLat()/2))));
}

float64
GfxDataSingleSmallPoly::getLength(uint16 p) const
{
   MC2_ASSERT(p == 0);
   return calcPolyLength( p );
}

         
bool
GfxDataSingleSmallPoly::updateLength(uint16 p)
{
   MC2_ASSERT( (p == 0) || (p == MAX_UINT16));
   return true;
}

void
GfxDataSingleSmallPoly::setLength(uint16 p, float64 length)
{
   MC2_ASSERT(p == 0);
}

bool
GfxDataSingleSmallPoly::updateBBox()
{
   return true;
}

bool
GfxDataSingleSmallPoly::updateBBox(const GfxData::coordinate_type lat, 
                                 const GfxData::coordinate_type lon)
{
   return true;
}

void
GfxDataSingleSmallPoly::readPolygons( uint16 nbrPolygons,
                                      DataBuffer& dataBuffer,
                                      GenericMap* theMap )
{

   MC2_ASSERT( theMap );
   MC2_ASSERT( nbrPolygons == 1 );
   
   uint32 nbrCoordinates = dataBuffer.readNextLong();

   MC2_ASSERT( nbrCoordinates < (uint32)(1<<31) ); // must be less than 32 bits.

   m_nbrCoordinates = nbrCoordinates;

   m_coordinates = theMap->getCoordinateAllocator().getNextObject();

   for ( uint32 i = 0; i < nbrCoordinates; ++i ) {
      if ( i != 0 ) {
         theMap->getCoordinateAllocator().getNextObject();
      }      
      m_coordinates[ i ].lat = dataBuffer.readNextLong();
      m_coordinates[ i ].lon = dataBuffer.readNextLong();
   }

   // Set the length
   setLength( 0, dataBuffer.readNextFloat() );

}


