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

#include "DataBuffer.h"

#include "GfxRoadPolygon.h"


GfxRoadPolygon::GfxRoadPolygon( bool coordinates16Bits, uint32 startSize,
                                byte posSpeedLimit, byte negSpeedLimit,
                                bool multidigitialized, bool ramp, 
                                bool roundabout, int8 level0, int8 level1,
                                ItemTypes::entryrestriction_t entryrestr0,
                                ItemTypes::entryrestriction_t entryrestr1) 
      : GfxPolygon( coordinates16Bits, startSize ),
        m_posSpeedLimit( posSpeedLimit ),
        m_negSpeedLimit( negSpeedLimit ),
        m_level0( level0 ),
        m_level1( level1 ),
        m_entryrestrictions0( entryrestr0 ),
        m_entryrestrictions1( entryrestr1 ),
        m_multidigitialized( multidigitialized ),
        m_ramp( ramp ),
        m_roundabout( roundabout )
{

}


GfxRoadPolygon::GfxRoadPolygon() : GfxPolygon()
{
}


GfxRoadPolygon::~GfxRoadPolygon() 
{
   // I hope vector works
}

uint32 
GfxRoadPolygon::getSize() const 
{
   // Params = 4 bytes on disc
   return 4 + GfxPolygon::getSize();
}

void 
GfxRoadPolygon::dump( int verboseLevel ) const 
{
   // Write header
   GfxPolygon::dump(verboseLevel);

   if ( verboseLevel > 0 ) {
      cout << "      GfxRoadPolygon" << endl
           << "         Header:" << endl
           << "            posSpeedLimit: " << (int)m_posSpeedLimit << endl
           << "            negSpeedLimit: " << (int)m_negSpeedLimit << endl
           << "            multidigi. :   " << m_multidigitialized << endl
           << "            ramp:          " << m_ramp << endl
           << "            roundabout:    " << m_roundabout << endl
           << "            level0:        " << (int)m_level0 << endl
           << "            level1:        " << (int)m_level1 << endl
           << "            entryRestr.0:  " << (int)m_entryrestrictions0<< endl
           << "            entryRestr.1:  " << (int)m_entryrestrictions1<< endl;
   }
}


#define SHIFT_SIGNED(a, posMSB, n) int32( (int32(a)<<(31-(posMSB))) >> (32-(n))  )

void 
GfxRoadPolygon::readHeader( DataBuffer* data ) 
{
   GfxPolygon::readHeader( data );
   uint32 params = data->readNextLong();
   mc2dbg8 << "Parameters in GfxRoadPolygon read: 0x" << hex << params 
           << dec << endl;

   // Extract from params, see writeHeader
   m_posSpeedLimit = (params>>24)&0xFF;
   m_negSpeedLimit = (params>>16)&0xFF;
   m_multidigitialized = (params>>15)&0x01;
   m_ramp = (params>>14)&0x01;
   m_roundabout = (params>>13)&0x01;
   m_level0 = SHIFT_SIGNED(params, 12, 2);
   m_level1 = SHIFT_SIGNED(params, 10, 2); 
   m_entryrestrictions0 = ItemTypes::entryrestriction_t( (params >> 7) & 0x03);
   m_entryrestrictions1 = ItemTypes::entryrestriction_t( (params >> 5) & 0x03);

   DEBUG4(
      if ( (m_entryrestrictions0 != 0) || 
           (m_entryrestrictions1 != 0))
         mc2dbg << "Read: m_entryrestrictions0=" << int(m_entryrestrictions0) 
                << ", m_entryrestrictions1=" << int(m_entryrestrictions1) 
                << ", param=0x" << hex << params << dec << endl;
   );
   DEBUG4(
      if ((m_level0 != 0) || (m_level1 != 0))
         mc2dbg << "Read: level0=" << int(m_level0) << ", level1="
                << int(m_level1) << ", roundabout=" << m_roundabout 
                << ", param=0x" << hex << params << dec << endl;
   );
   
}


void 
GfxRoadPolygon::writeHeader( DataBuffer* data ) const {
   GfxPolygon::writeHeader( data );
   
   // Create params see, readHeader
   uint32 params = ( ((uint32(m_posSpeedLimit) & 0xff ) << 24) |     // 8 bits
                     ((uint32(m_negSpeedLimit) & 0xff ) << 16) |     // 8 bits
                     ( (uint32(m_multidigitialized) & 0x1 ) << 15) | // 1 bit
                     ( (uint32(m_ramp) & 0x1) << 14 ) |              // 1 bit
                     ( (uint32(m_roundabout) & 0x1 ) << 13) |        // 1 bit
                     ( (uint32(m_level0) & 0x03) << 11) |            // 2 bits
                     ( (uint32(m_level1) & 0x03) << 9) |             // 2 bits
                     ( (uint32(m_entryrestrictions0) & 0x03) << 7) | // 2 bits
                     ( (uint32(m_entryrestrictions1) & 0x03) << 5)); // 2 bits
   
   data->writeNextLong( params );

   DEBUG4(
      if ( (m_entryrestrictions0 != 0) || 
           (m_entryrestrictions1 != 0))
         mc2dbg << "Write: m_entryrestrictions0=" << int(m_entryrestrictions0) 
                << ", m_entryrestrictions1=" << int(m_entryrestrictions1) 
                << ", param=0x" << hex << params << dec << endl;
   );
   DEBUG4(
      if ((m_level0 != 0) || (m_level1 != 0))
         mc2dbg << "Write: level0=" << int(m_level0) << ", level1="
                << int(m_level1) << ", roundabout=" << m_roundabout 
                << ", param=0x" << hex << params << dec << endl;
   );

   DEBUG8(
      cerr << "Parameters in GfxRoadPolygon written: 0x"
           << hex << params << dec << endl;
      dump(2);
   );
}

