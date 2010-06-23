/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GfxDataSinglePoint.h"
#include "Math.h"
#include "DataBuffer.h"

GfxDataSinglePoint::GfxDataSinglePoint( const GfxDataSinglePoint& gfx )
{
   
}

      
GfxDataSinglePoint::GfxDataSinglePoint(const GfxData& gfx)
{
   
}

GfxDataSinglePoint::~GfxDataSinglePoint()
{
   
}

GfxData::const_iterator
GfxDataSinglePoint::polyBegin( uint16 p ) const
{
   return &m_coord;
}

GfxData::const_iterator
GfxDataSinglePoint::polyEnd( uint16 p ) const
{
   return polyBegin(p) + 1;
}

GfxData::iterator
GfxDataSinglePoint::polyBegin( uint16 p )
{
   return &m_coord;
}

GfxData::iterator
GfxDataSinglePoint::polyEnd( uint16 p )
{
   return polyBegin(p) + 1;
}


GfxData::coordinate_type
GfxDataSinglePoint::getLat(uint16 p, uint32 i) const
{
   MC2_ASSERT(p == 0);
   MC2_ASSERT(i < 1);
   if (i < 1)
      return m_coord.lat;
   return GfxConstants::IMPOSSIBLE;
}

GfxData::coordinate_type
GfxDataSinglePoint::getLon(uint16 p, uint32 i) const
{
   MC2_ASSERT(p == 0);
   MC2_ASSERT(i < 1);
   if (i < 1)
      return m_coord.lon;
   return GfxConstants::IMPOSSIBLE;
}

uint16
GfxDataSinglePoint::getNbrPolygons() const
{
   return 1;
}

uint32
GfxDataSinglePoint::getNbrCoordinates(uint16 poly) const
{
   MC2_ASSERT( poly == 0 );
   if ( ! m_coord.isValid() ){
      mc2log << error << here << " Invalid coord: " << m_coord << endl;
      MC2_ASSERT(false);
   }
   return 1;
}

bool
GfxDataSinglePoint::sortPolygons()
{
   return true; // One coordinate in one polygon is always sorted...
}

void
GfxDataSinglePoint::setClosed(uint16 poly, bool closed)
{
   
}

bool
GfxDataSinglePoint::getClosed(uint16 poly) const
{
   return false;
}

GfxData::coordinate_type
GfxDataSinglePoint::getMinLat() const
{
   return m_coord.lat;
}

GfxData::coordinate_type
GfxDataSinglePoint::getMaxLat() const
{
   return m_coord.lat;
}

GfxData::coordinate_type
GfxDataSinglePoint::getMinLon() const
{
   return m_coord.lon;
}

GfxData::coordinate_type
GfxDataSinglePoint::getMaxLon() const
{
   return m_coord.lon;
}

float64
GfxDataSinglePoint::getCosLat() const
{
   return cos( (2*M_PI/ POW2_32 * m_coord.lat));
}

float64
GfxDataSinglePoint::getLength(uint16 p) const
{
   MC2_ASSERT(p == 0);
   MC2_ASSERT( m_coord.isValid() );
   return 0;
}

         
bool
GfxDataSinglePoint::updateLength(uint16 p)
{
   MC2_ASSERT( (p == 0) || (p == MAX_UINT16));
   return true;
}

void
GfxDataSinglePoint::setLength(uint16 p, float64 length)
{
   MC2_ASSERT(p == 0);
}

bool
GfxDataSinglePoint::updateBBox()
{
   return true;
}

bool
GfxDataSinglePoint::updateBBox(const GfxData::coordinate_type lat, 
                                 const GfxData::coordinate_type lon)
{
   return true;
}

void 
GfxDataSinglePoint::readPolygons( uint16 nbrPolys, DataBuffer& buff, 
                                  GenericMap* map ) 
{
   uint32 nbrCoord = buff.readNextLong();
   m_coord.lat = buff.readNextLong();
   m_coord.lon = buff.readNextLong();

   setLength(0, buff.readNextFloat() );

   // we can not have more coordinates or polys
   MC2_ASSERT( nbrCoord == 1 );
   MC2_ASSERT( nbrPolys == 1 );
}
