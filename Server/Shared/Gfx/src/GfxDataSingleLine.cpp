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
#include "GfxDataSingleLine.h"
#include "GfxUtility.h"
#include "DataBuffer.h"
#include "Math.h"

GfxDataSingleLine::GfxDataSingleLine( const GfxDataSingleLine& gfx )
{
   
}

      
GfxDataSingleLine::GfxDataSingleLine(const GfxData& gfx)
{
   
}

GfxDataSingleLine::~GfxDataSingleLine()
{
   
}

GfxData::const_iterator
GfxDataSingleLine::polyBegin( uint16 p ) const
{
   MC2_ASSERT( p == 0 );
   return m_coordinates;
}

GfxData::const_iterator
GfxDataSingleLine::polyEnd( uint16 p ) const
{
   MC2_ASSERT( p == 0 );
   return m_coordinates + 2;
}

GfxData::iterator
GfxDataSingleLine::polyBegin( uint16 p )
{
   MC2_ASSERT( p == 0 );
   return m_coordinates;
}

GfxData::iterator
GfxDataSingleLine::polyEnd( uint16 p )
{
   MC2_ASSERT( p == 0 );
   return m_coordinates + 2;
}

GfxData::coordinate_type
GfxDataSingleLine::getLat(uint16 p, uint32 i) const
{
   MC2_ASSERT(p == 0);
   //MC2_ASSERT(i < 2);
   MC2_ASSERT(m_coordinates[i].lat != GfxConstants::IMPOSSIBLE);
   if (i < 2)
      return m_coordinates[i].lat;
   return GfxConstants::IMPOSSIBLE;
}

GfxData::coordinate_type
GfxDataSingleLine::getLon(uint16 p, uint32 i) const
{
   MC2_ASSERT(p == 0);
   //MC2_ASSERT(i < 2);
   MC2_ASSERT(m_coordinates[i].lat != GfxConstants::IMPOSSIBLE);
   if (i < 2)
      return m_coordinates[i].lon;
   return GfxConstants::IMPOSSIBLE;
}

uint16
GfxDataSingleLine::getNbrPolygons() const
{
   return 1;
}

uint32
GfxDataSingleLine::getNbrCoordinates(uint16 poly) const
{
   MC2_ASSERT(m_coordinates[0].lat != GfxConstants::IMPOSSIBLE);
   MC2_ASSERT(m_coordinates[1].lat != GfxConstants::IMPOSSIBLE);
   return 2;
}

bool
GfxDataSingleLine::sortPolygons()
{
   return true; // One polygon is always sorted...
}

void
GfxDataSingleLine::setClosed(uint16 poly, bool closed)
{
   
}

bool
GfxDataSingleLine::getClosed(uint16 poly) const
{
   return false;
}

GfxData::coordinate_type
GfxDataSingleLine::getMinLat() const
{
   if (m_coordinates[0].lat < m_coordinates[1].lat)
      return m_coordinates[0].lat;
   else 
      return m_coordinates[1].lat;
}

GfxData::coordinate_type
GfxDataSingleLine::getMaxLat() const
{
   if (m_coordinates[0].lat > m_coordinates[1].lat)
      return m_coordinates[0].lat;
   else 
      return m_coordinates[1].lat;
}

GfxData::coordinate_type
GfxDataSingleLine::getMinLon() const
{
   if ( ((int32) (m_coordinates[0].lon - m_coordinates[1].lon) ) < 0 )
      return m_coordinates[0].lon;
   else 
      return m_coordinates[1].lon;
}

GfxData::coordinate_type
GfxDataSingleLine::getMaxLon() const
{
   if ( ((int32) (m_coordinates[0].lon - m_coordinates[1].lon) ) > 0 )
      return m_coordinates[0].lon;
   else 
      return m_coordinates[1].lon;
}

float64
GfxDataSingleLine::getCosLat() const
{
   return cos( (2*M_PI/ POW2_32 * 
               ((m_coordinates[0].lat/2)+(m_coordinates[1].lat/2))));
}

float64
GfxDataSingleLine::getLength(uint16 p) const
{
   MC2_ASSERT(p == 0);
   MC2_ASSERT(m_coordinates[0].lat != GfxConstants::IMPOSSIBLE);
   MC2_ASSERT(m_coordinates[1].lat != GfxConstants::IMPOSSIBLE);
   return calcPolyLength( p );
}

         
bool
GfxDataSingleLine::updateLength(uint16 p)
{
   MC2_ASSERT( (p == 0) || (p == MAX_UINT16));
   return true;
}

void
GfxDataSingleLine::setLength(uint16 p, float64 length)
{
   MC2_ASSERT(p == 0);
}

bool
GfxDataSingleLine::updateBBox()
{
   return true;
}

bool
GfxDataSingleLine::updateBBox(const GfxData::coordinate_type lat, 
                                 const GfxData::coordinate_type lon)
{
   return true;
}

void 
GfxDataSingleLine::readPolygons( uint16 nbrPolygons,
                                 DataBuffer& dataBuffer, 
                                 GenericMap* map )
{

   uint32 nbrCoords = dataBuffer.readNextLong();

   for (uint32 i = 0; i < nbrCoords; ++i ) {
      m_coordinates[i].lat = dataBuffer.readNextLong();
      m_coordinates[i].lon = dataBuffer.readNextLong();
   }

   setLength(0, dataBuffer.readNextFloat() );

   MC2_ASSERT( nbrPolygons == 1 );
   MC2_ASSERT( nbrCoords == 2 );

}

