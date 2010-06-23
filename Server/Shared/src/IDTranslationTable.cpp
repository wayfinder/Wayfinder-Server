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

#include "IDTranslationTable.h"

#include "DataBuffer.h"
#include "MapBits.h"

void
IDTranslationTable::save(DataBuffer& dbuf) const
{
   MC2_ASSERT(is_sorted(m_lookupTable.begin(),
                        m_lookupTable.end(),
                        SortLookup()));
   MC2_ASSERT(is_sorted(m_reverseLookupTable.begin(),
                        m_reverseLookupTable.end(),
                        SortLessThanIndex( m_lookupTable )));

   DataBufferChecker dbc(dbuf, "IDTranslationTable::save");
   dbc.assertRoom(getSizeInDataBuffer());

   if ( m_version != 0 ) {
      saveOld( dbuf );
   } else {
      saveNew( dbuf );
   }


   dbc.assertPosition(getSizeInDataBuffer());

}


void 
IDTranslationTable::load(DataBuffer& dbuf)
{
   DataBufferChecker dbc(dbuf, "IDTranslationTable::load");
   if ( m_version != 0) {
      loadOld( dbuf, 0 ); // ok to use 0 here? when is load() used?
   } else {
      loadNew( dbuf, 0 );
   }
   dbc.assertPosition(getSizeInDataBuffer());

}

void
IDTranslationTable::load(DataBuffer& dataBuffer, uint32 mapSet)
{
   if ( m_version != 0 ) {
      loadOld( dataBuffer, mapSet );
   } else {
      loadNew( dataBuffer, mapSet );
   }
}


uint32
IDTranslationTable::getSizeInDataBuffer() const
{
   if ( m_version != 0 ) {

      if ( m_version == 1 ) {
         uint32 size = 4 + 4; // nbrElements and two table size
      
         // table sizes
         size += m_lookupTable.size() * 4 * 3 * 2;
         
         return size;
      } else {
         // version 2
         return m_lookupTable.size() * 4 * 4 + 8;
      }

   } 

   return m_lookupTable.size() * 4 * 4 + 4;
}

void 
IDTranslationTable::saveNew( DataBuffer& dbuf ) const {
   mc2dbg << "saving New ID translationTable with size " << m_lookupTable.size() << endl;
   // write size
   dbuf.writeNextLong( m_lookupTable.size() );

   // write the data
   for ( uint32 i = 0;  i < m_lookupTable.size(); ++i ) {
      dbuf.writeNextLong( m_lookupTable[i].first );
      dbuf.writeNextLong( m_lookupTable[i].second.first );
      dbuf.writeNextLong( m_lookupTable[i].second.second );
      dbuf.writeNextLong( m_reverseLookupTable[i] );
   }

}

void 
IDTranslationTable::loadNew(DataBuffer& dataBuffer, uint32 mapSet) {

   uint32 size = dataBuffer.readNextLong();

   mc2dbg << "Loading New IDTranslationTable with size " << size << endl;

   m_lookupTable.clear();
   m_lookupTable.reserve( size );
   m_reverseLookupTable.resize( size );

   for ( uint32 i = 0; i < size; ++i ) {

      uint32 first = dataBuffer.readNextLong();
      uint32 second_first = dataBuffer.readNextLong();
      uint32 second_second = dataBuffer.readNextLong();
      m_reverseLookupTable[ i ] = dataBuffer.readNextLong();

      if ( mapSet != MAX_UINT32 ) {
         second_first = MapBits::getMapIDWithMapSet( second_first, mapSet );
      }

      m_lookupTable.push_back( lookupElement_t( first, 
                               IDPair_t( second_first, second_second ) ) );
   }

}

void
IDTranslationTable::loadOld(DataBuffer& dataBuffer, uint32 mapSet) {

   // Nbr elements in the table or tables
   uint32 size = dataBuffer.readNextLong();

   mc2dbg << "Loading old id translation table size = " << size << endl;

   m_lookupTable.clear();
   m_lookupTable.reserve( size );
   uint32 version = MAX_UINT32 - dataBuffer.readLong(dataBuffer.getCurrentOffset());

   // if we have version above 100....this should be enough to indicate
   // that the buffer value is MapID
   if ( version < MAX_UINT32 - 100 )
      version++;
   else
      version = 0;

   mc2dbg << "Loading old id table version = " << version << endl;

   // Chck if we have one or several tables in the databuffer
   if ( version == 1 ) {
      // TWO tables
      mc2dbg << "[IDT] To create IDTranslationTable from "
             << "databuffer with TWO tables, size: " << size << endl;
      dataBuffer.readNextLong();   // Skip flag
      
      // Read and ignore elements for reverseTable
      for (uint32 i=0; i < size; i++) {
         dataBuffer.readNextLong(); // mapID
         dataBuffer.readNextLong(); // itemID
         dataBuffer.readNextLong(); // overviewItemID
      }

      // Read and insert the elements into the lookup table
      for (uint32 i=0; i < size; i++) {

         uint32 tmpMapID = dataBuffer.readNextLong();

         if (mapSet != MAX_UINT32)
            tmpMapID = MapBits::getMapIDWithMapSet(tmpMapID, mapSet);

         const uint32 tmpItemID = dataBuffer.readNextLong();
         const uint32 tmpOverviewItemID = dataBuffer.readNextLong();

         m_lookupTable.push_back(
            lookupElement_t(tmpOverviewItemID, 
                            IDPair_t(tmpMapID, tmpItemID) ) );
      }
      // setup and sort reverse table
      sortReverseTable();   
  
   } else if ( version == 2 ) {
      dataBuffer.readNextLong();   // Skip flag

      m_reverseLookupTable.resize( size );

      // Read and insert the elements into the lookup tables
      for ( uint32 i = 0; i < size; ++i ) {
         uint32 tmpMapID = dataBuffer.readNextLong();
         const uint32 tmpItemID = dataBuffer.readNextLong();
         const uint32 tmpOverviewItemID = dataBuffer.readNextLong();
         m_reverseLookupTable[ i ] = dataBuffer.readNextLong();

         if ( mapSet != MAX_UINT32 )
            tmpMapID = MapBits::getMapIDWithMapSet(tmpMapID, mapSet);


         addElement(tmpOverviewItemID, tmpMapID, tmpItemID);


      }
      
   } else if ( version == 0 ) {
      // "Normal", one table..
      
      mc2log << "[IDTranslationTable]: Reading ONE table, size = "
             << size << endl;
      
      // Read and insert the elements into the lookup tables
      // Must be sorted as in m_reverseLookupTable
      for (uint32 i=0; i < size; ++i) {
         uint32 tmpMapID = dataBuffer.readNextLong();
         if (mapSet != MAX_UINT32)
            tmpMapID = MapBits::getMapIDWithMapSet(tmpMapID, mapSet);
         const uint32 tmpItemID = dataBuffer.readNextLong();
         const uint32 tmpOverviewItemID = dataBuffer.readNextLong();
         addElement(tmpOverviewItemID, tmpMapID, tmpItemID);
      }

      // setup and sort reverse table
      sortReverseTable();
   }
   

}



void
IDTranslationTable::saveOld(DataBuffer& dataBuffer) const
{
   
   mc2dbg << "saving old id translation table size = " << 
      m_lookupTable.size() << endl;

   MC2_ASSERT(m_reverseLookupTable.size() == m_lookupTable.size());
   MC2_ASSERT(is_sorted(m_lookupTable.begin(),
                        m_lookupTable.end(),
                        SortLookup()));
   MC2_ASSERT(is_sorted(m_reverseLookupTable.begin(),
                        m_reverseLookupTable.end(),
                        SortLessThanIndex( m_lookupTable ) ) );

   dataBuffer.writeNextLong(m_lookupTable.size());

   // write version, MAX_UINT32 on version 1 (thus the +1)
   dataBuffer.writeNextLong( MAX_UINT32 - m_version + 1);

   mc2dbg << "IDTranslationTable::save; version = " << m_version << endl;

   if ( m_version == 1 ) {
      uint32 i;
      for (i = 0 ; i < getNbrElements(); ++i ) {
         uint32 newIdx = m_reverseLookupTable[ i ];
         dataBuffer.writeNextLong( m_lookupTable[ newIdx ].second.getMapID() );
         dataBuffer.writeNextLong( m_lookupTable[ newIdx ].second.getItemID() );
         dataBuffer.writeNextLong( m_lookupTable[ newIdx ].first );
      }
      
      for (i = 0; i < getNbrElements(); ++i ) {
         dataBuffer.writeNextLong( m_lookupTable[ i ].second.getMapID() );
         dataBuffer.writeNextLong( m_lookupTable[ i ].second.getItemID() );
         dataBuffer.writeNextLong( m_lookupTable[ i ].first );
      } 

   } else if ( m_version == 2 ) {
      // save reverse lookup table too
      for (uint32 i = 0; i < getNbrElements(); ++i ) {
         dataBuffer.writeNextLong( m_lookupTable[ i ].second.getMapID() );
         dataBuffer.writeNextLong( m_lookupTable[ i ].second.getItemID() );
         dataBuffer.writeNextLong( m_lookupTable[ i ].first );
         dataBuffer.writeNextLong( m_reverseLookupTable[ i ] );
      } 
   } else {
      mc2dbg << "Unknown translation version" << endl;
      MC2_ASSERT( false );
   }
}

