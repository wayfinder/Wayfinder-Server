/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GfxPolygon.h"
#include "DataBuffer.h"
#include <math.h>

GfxPolygon::GfxPolygon( bool coordinates16Bits, uint32 startSize ) 
{
   if (coordinates16Bits) {
      mc2dbg8 << "16 bit coordinates" << endl;
      m_coords = new GfxCoordinates16( startSize ); 
   } else {
      mc2dbg8 << "32 bit coordinates" << endl;
      m_coords = new GfxCoordinates32( startSize ); 
   }
   m_area = 0.0;
}


GfxPolygon::GfxPolygon() 
{
   m_coords = NULL;
   m_area = 0.0;
}


GfxPolygon::~GfxPolygon() {
   // I hope vector works
   delete m_coords;
}

void
GfxPolygon::setCoords( const vector<MC2Coordinate>& coords )
{
   delete m_coords;
   m_coords = new GfxCoordinates32( coords );
}

uint32 
GfxPolygon::getSize() const {
   uint32 size = 32; // The area. 
   size += m_coords->getSize();

   return size;
}


void
GfxPolygon::dump(int verboseLevel) const {   
   if (m_coords != NULL) {
      m_coords->dump(verboseLevel);
   } else {
      cout << "            No coordinates" << endl;
   }
}


bool
GfxPolygon::createFromDataBuffer( DataBuffer* data ) {
   readHeader( data );
   readCoords( data );
   
   DEBUG8(dump(2));
   return true;
}


bool
GfxPolygon::save( DataBuffer* data ) const {
   writeHeader( data );
   writeCoords( data );

   DEBUG8(dump(2));
   return true;
}


void
GfxPolygon::readHeader( DataBuffer* data ) {
   mc2dbg8 << "GfxPolygon::readHeader" << endl;
   m_area = (float64) data->readNextLong();
   m_area = m_area * m_area;
}

void
GfxPolygon::readCoords( DataBuffer* data ) {
   delete m_coords;
   m_coords = GfxCoordinates::createNewGfxCoordinates( data );
}


void
GfxPolygon::writeHeader( DataBuffer* data ) const {
   uint32 sqrtArea = 0;
   if ( m_area > 1.0 ) {
      sqrtArea = (uint32) sqrt( m_area );
   }
   data->writeNextLong( sqrtArea );
}


void
GfxPolygon::writeCoords( DataBuffer* data ) const {
   m_coords->save( data );
}

