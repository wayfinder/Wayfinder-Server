/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef VECTOR_INDEX_TABLE_H
#define VECTOR_INDEX_TABLE_H

#include "config.h"

#include "ItemComboTable.h"
#include "DataBuffer.h"

#include <vector>

/**
 *    Convert a map of uint32 -> vector<VALUE_TYPE> to a
 *    big array to be used with indeces.
 */
template<class VALUE_TYPE,
         class VALUE_RW = typename LoaderTypeSelector<VALUE_TYPE>::loaderType >
class VectorIndexTable {
public:

   typedef vector<VALUE_TYPE> valVect_t;
   typedef map<uint32, valVect_t> valMap_t;
   typedef map<valVect_t, uint32> addedMap_t;
   typedef map<uint32,uint32> indexMap_t;
   typedef const VALUE_TYPE* const_iterator;
   typedef pair<const_iterator, const_iterator> range_t;

   /// Subject to change.
   enum { offsetMask = 0xffffff };

   /**
    *   Creates a VectorIndexTable from a map<uint32,VALUE_TYPE>
    *   and fills the indeces map. Same key will be kept.
    */
   VectorIndexTable( indexMap_t& indeces,
                     const valMap_t& vals ) {
      m_array = NULL;
      setValues( indeces, vals );
   }
   
   VectorIndexTable() : m_array( NULL ),
                        m_arraySize( 0 ) {
   }

   void clear() {
      delete [] m_array;
      m_arraySize = 0;
      m_array = NULL;
   }
   
   ~VectorIndexTable() {
      clear();
   }

   
   uint32 getOneIndex( const valVect_t& val,
                       addedMap_t& added,
                       valVect_t& resVect ) {
      if ( val.empty() ) {
         // So that all empty ones will be the same.
         return 0;
      }

      typename addedMap_t::const_iterator findit = added.find( val );

      if ( findit == added.end() ) {
         // Add it
         uint32 pos = resVect.size();
         resVect.insert( resVect.end(), val.begin(), val.end() );
         added.insert( make_pair( val, pos ) );
         // Try again ( and succeed )
         return getOneIndex( val, added, resVect );
      } else {
         return findit->second;
      }
   }

   /// 24 bits of offset and 8 bits of nbr entries
   static uint32 makeIdx( uint32 offset,
                          uint8 nbrEntries ) {
      MC2_ASSERT( ( offset & offsetMask ) == offset );
      return offset | ( uint32(nbrEntries) << 24 );
   }
   
   void setValues( indexMap_t& indeces,
                   const valMap_t& vals ) {
      
      valVect_t resVect;
      addedMap_t added;
      for ( typename valMap_t::const_iterator it = vals.begin();
            it != vals.end();
            ++it ) {
         uint32 curIdx = makeIdx( getOneIndex( it->second, added, resVect ),
                                  it->second.size() );
         indeces.insert( make_pair( it->first, curIdx ) );
      }
      // Now make array.
      m_arraySize = resVect.size();
      delete [] m_array;
      m_array = new VALUE_TYPE[ m_arraySize ];
      for ( uint32 i = 0; i < m_arraySize; ++i ) {
         m_array[i] = resVect[i];
      }
      
      // Test that it's working
      for ( typename valMap_t::const_iterator it = vals.begin();
            it != vals.end();
            ++it ) {
         range_t r = range( indeces[it->first] );
         vector<VALUE_TYPE> test( r.first, r.second );
         MC2_ASSERT( test == it->second );
      }
      
   }

   uint32 getSizeInDataBuffer() const {
      uint32 size =  4 + 4;
      for ( uint32 i = 0; i < m_arraySize; ++i ) {
         size += VALUE_RW::getValSizeInDataBuffer( m_array[i] );
      }
      AlignUtility::alignLong( size );
      return size;
   }
   
   
   void save( DataBuffer& buf ) const {
      buf.writeNextLong( getSizeInDataBuffer() );
      buf.writeNextLong( m_arraySize );
      for ( uint32 i = 0; i < m_arraySize; ++i ) {
         VALUE_RW::writeValue( buf, m_array[i] );         
      }
      buf.alignToLongAndClear();
   }

   void load( DataBuffer& buf ) {
      clear();
      buf.readNextLong();
      buf.readNextLong( m_arraySize );
      m_array = new VALUE_TYPE[ m_arraySize ];
      for ( uint32 i = 0; i < m_arraySize; ++i ) {
         VALUE_RW::readValue( buf, m_array[i] );
      }
      buf.alignToLong();
   }
   
   const_iterator begin( uint32 idx ) const {
      return & ( m_array[ idx & offsetMask] );
   }

   const_iterator end( uint32 idx ) const {
      return begin( idx ) + (idx >> 24);
   }

   pair<const_iterator, const_iterator> range( uint32 idx ) const {
      return make_pair( begin(idx), end(idx) );
   }

   static map<uint32, vector<uint32> > test() {
      int nbrVects = 10;   
      typename VectorIndexTable<uint32>::valMap_t vects;
      for ( int xox = 0; xox < 3; ++xox ) {
         for ( int i = 0; i < nbrVects; ++i ) {
            for ( int j = 0; j < i; ++j ) {
               vects[xox*nbrVects + i].push_back( j );
            }
         }
      }
      nbrVects *= 3;
      // Add 100 empty vectors
      for ( int i = 0; i < 100; ++i ) {
         vects[nbrVects] = vector<uint32>();
         ++nbrVects;
      }

      typedef typename VectorIndexTable<uint32>::indexMap_t tindexMap_t;
      tindexMap_t indeces;
      VectorIndexTable<uint32> vt( indeces, vects );
      for ( typename tindexMap_t::const_iterator it =
               indeces.begin();
            it != indeces.end();
            ++it ) {
         mc2dbg8 << "[VTAB]: Index = " << it->first << ", "
                 << it->second << endl;
      }
      DataBuffer buf( vt.getSizeInDataBuffer() );
      vt.save( buf );
      buf.reset();
      vt.load( buf );
      mc2dbg << "[VTAB]::test returning" << endl;
      return vects;
   }

private:
   VALUE_TYPE* m_array;
   uint32      m_arraySize;
   
};

template<class VALUE_TYPE, class ITEM_TABLE = ItemDataTable<uint32> >
class VectorDataTable {
public:

   typedef typename VectorIndexTable<VALUE_TYPE>::valMap_t valMap_t;
   typedef typename VectorIndexTable<VALUE_TYPE>::range_t range_t;
   
   VectorDataTable( const valMap_t& vals ) : m_itemTable( 0 ) {
      setValues( vals );
   }

   void setValues( const valMap_t& vals ) {
      typename VectorIndexTable<VALUE_TYPE>::indexMap_t indeces;
      m_vectorTable.setValues( indeces, vals );
      m_itemTable.setValues( m_itemTable.getBestDefaultValue(indeces).second,
                             indeces );

      mc2dbg << "[VDT]: Default value is "
             << m_itemTable.getBestDefaultValue(indeces).second
             << endl;
      
      // Check for correctness.
      for ( typename valMap_t::const_iterator it = vals.begin();
            it != vals.end();
            ++it ) {
         range_t r = range( it->first );
         MC2_ASSERT( it->second ==
                     typename valMap_t::value_type::second_type(r.first,
                                                                r.second ) );
      }
   }

   uint32 getSizeInDataBuffer() const {
      return m_vectorTable.getSizeInDataBuffer() +
             m_itemTable.getSizeInDataBuffer();
   }

   void save( DataBuffer& buf ) const {
      m_vectorTable.save(buf);
      m_itemTable.save(buf);
   }

   void load( DataBuffer& buf ) {
      m_vectorTable.load( buf );
      m_itemTable.load( buf );
   }

   range_t range( uint32 id ) const {
      return m_vectorTable.range( m_itemTable.getData( id ) );
   }

   static void test() {
      valMap_t vals = VectorIndexTable<uint32>::test();
      VectorDataTable<uint32> table( vals );
      DataBuffer buf( table.getSizeInDataBuffer() );
      table.save( buf );
      buf.reset();
      table.load( buf );
      // Check for correctness.
      for ( typename valMap_t::const_iterator it = vals.begin();
            it != vals.end();
            ++it ) {
         range_t r = table.range( it->first );
         MC2_ASSERT( it->second ==
                     typename valMap_t::value_type::second_type(r.first,
                                                                r.second ) );
      }
      mc2dbg << "[VectorDataTable]::test returns" << endl;      
   }
   
private:
   ITEM_TABLE              m_itemTable;
   VectorIndexTable<VALUE_TYPE> m_vectorTable;
   
};

#endif
