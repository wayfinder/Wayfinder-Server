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

#include "SearchMapRegionTable.h"

#include "ArrayTools.h"
#include "DataBuffer.h"

#include <algorithm>

SearchMapRegionTable::SearchMapRegionTable()
{
   m_allRegions      = NULL;
   m_nbrAllRegions   = 0;
   m_nbrCombos       = 0;
   m_overflowVector  = NULL;
   m_nbrOverflow     = 0;
}

SearchMapRegionTable::~SearchMapRegionTable()
{
   delete [] m_allRegions;
   delete [] m_overflowVector;
}

int
SearchMapRegionTable::getNbrCombos() const
{
   return m_nbrCombos;
}

int
SearchMapRegionTable::getSizeInDataBuffer() const
{
   // Version + nbrcombos + nbrallregions + the all regions.
   return 4 + 4 + 4 + m_nbrAllRegions*4 + 4 + 
      m_nbrOverflow*(4 + 2) + (m_nbrOverflow&0x1 ? 2 : 0/*Align*/);
}

int
SearchMapRegionTable::save(DataBuffer& buff) const
{
   DataBufferChecker dbc(buff, "SearchMapRegionTable::save");
   dbc.assertRoom(getSizeInDataBuffer());
   
   buff.writeNextLong(0); // Version - can be nice to have
   buff.writeNextLong(m_nbrCombos);
   buff.writeNextLong(m_nbrAllRegions);
   buff.writeNextLong( m_overflowTable.size() );
   for( uint32 i = 0; i < m_nbrAllRegions; ++i ) {
      buff.writeNextLong( m_allRegions[i] );
   }
   
   // First all uint32 in m_overflowTable
   for ( uint32 i = 0 ; i < m_nbrOverflow ; ++i ) {
       buff.writeNextLong( m_overflowVector[ i ].key );
   }
   // Then all uint16 in m_overflowTable
   for ( uint32 i = 0 ; i < m_nbrOverflow ; ++i ) {
      buff.writeNextShort( m_overflowVector[ i ].value );
   }

   // Make sure it is aligned to long
   buff.alignToLongAndClear();
   dbc.assertPosition(getSizeInDataBuffer());

   return getSizeInDataBuffer();
}

int
SearchMapRegionTable::load(DataBuffer& buff)
{
   DataBufferChecker dbc(buff, "SearchMapRegionTable::load");   
   
   int version = buff.readNextLong();
   MC2_ASSERT(version == 0 ); version *= 1; // To be used in release
   m_nbrCombos     = buff.readNextLong();
   m_nbrAllRegions = buff.readNextLong();
   m_nbrOverflow = buff.readNextLong();
   delete [] m_allRegions;
   m_allRegions = new uint32[m_nbrAllRegions];
   delete [] m_overflowVector;
   m_overflowVector = new overflowVector_t[ m_nbrOverflow ];
   for ( uint32 i = 0; i < m_nbrAllRegions; ++i ) {
      m_allRegions[i] = buff.readNextLong();
   }

   // First all uint32 to m_overflowTable
   for ( uint32 i = 0 ; i < m_nbrOverflow ; ++i ) {
      m_overflowVector[ i ].key = buff.readNextLong();
   }
   // Then all uint16 to m_overflowTable
   for ( uint32 i = 0 ; i < m_nbrOverflow ; ++i ) {
      m_overflowVector[ i ].value = buff.readNextShort();
   }

   // Make sure it is aligned to long
   buff.alignToLong();
   dbc.assertPosition(getSizeInDataBuffer());

   return getSizeInDataBuffer();
}

uint32
SearchMapRegionTable::makeIndex(uint32 posInArray, uint32 nbrRegions)
{
   // Make sure that there posInArray isn't too big.
   MC2_ASSERT( posInArray == (posInArray & 0x00ffffff ) );
   MC2_ASSERT( nbrRegions <= 0xff );
   return (posInArray & 0x00ffffff ) | 
      (std::min( nbrRegions, uint32(0xff) ) & 0xff ) << 24;
}

int
SearchMapRegionTable::getNbrRegions( uint32 idx ) const {
   uint32 nbrRegions = (idx >> 24) & 0xff;
   if ( nbrRegions == 0xff ) {
      // look in overflow vector for the real number
      nbrRegions = (*std::lower_bound( 
                       m_overflowVector, 
                       m_overflowVector + m_nbrOverflow, idx ) ).value;
   }
   return nbrRegions;
}

uint32
SearchMapRegionTable::getOffset(uint32 idx)
{
   return idx & 0x00ffffff;
}

void 
SearchMapRegionTable::fixupRegionCombo() {
   // FIXME: Might not be the right place for this...
   delete [] m_overflowVector;
   m_nbrOverflow = m_overflowTable.size();
   m_overflowVector = new overflowVector_t[ m_nbrOverflow ];
   uint32 i = 0;
   for ( overflowTable_t::const_iterator it = m_overflowTable.begin() ; 
         it != m_overflowTable.end() ; ++i, ++it ) {
      m_overflowVector[ i ].key = it->first;
      m_overflowVector[ i ].value = it->second;
   }
}

// -- WriteableSearchMapRegionTable

WriteableSearchMapRegionTable::WriteableSearchMapRegionTable() :
      SearchMapRegionTable()
{
   // Pre-allocate some room.
   m_nbrAllocatedRegions = 200000;
   m_nbrAllRegions       = 0;
   m_allRegions          = new uint32[m_nbrAllocatedRegions];
}

WriteableSearchMapRegionTable::~WriteableSearchMapRegionTable()
{
   // Nothing to be done here yet
}

uint32
WriteableSearchMapRegionTable::addRegionCombo(const set<uint32>& combo)
{
   lookupTable_t::const_iterator it = m_lookupTable.find(combo);
   if ( it != m_lookupTable.end() ) {      
      // Already there. Nice.
      return it->second;
   }

   // Now we will have to add it.
   // Save the startpos
   uint32 retVal = makeIndex( m_nbrAllRegions, 
                              std::min( combo.size(), size_t( 0xff ) ) );
   if ( combo.size() >= 0xff ) {
      // Add to overflow table for the real number
      MC2_ASSERT( combo.size() <= MAX_UINT16 );
      m_overflowTable.insert( make_pair( retVal, combo.size() ) );
   }

   for ( set<uint32>::const_iterator it = combo.begin();
         it != combo.end();
         ++it ) {
      // Add the regions to the combination list.
      m_allRegions = ArrayTool::addElement(m_allRegions, *it, m_nbrAllRegions,
                                           m_nbrAllocatedRegions);
   }

   // Check that regionIdx_t is large enough...
   MC2_ASSERT( m_nbrAllRegions < maxNbrRegionsInTable );
   
   // Also add the set to our table.
   m_lookupTable.insert( make_pair(combo, retVal) );
   ++m_nbrCombos;

   // FIXME: Make combos of the parts of the added combo.

   MC2_ASSERT( m_nbrCombos == m_lookupTable.size() );
   
   return retVal;   
}
