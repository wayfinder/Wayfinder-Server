/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ITEMADA_ATA_TABLE_H
#define ITEMADA_ATA_TABLE_H

#include "config.h"
#include "DataTable.h"
#include "ItemComboLoader.h"
#include "AlignUtility.h"
#include "MapBits.h"

class DataBuffer;
class DataBufferChecker;

#include <map>
#include <algorithm>

#include "NonStdStl.h"

#undef URIT_USES_ZOOM_LEVELS


/**
 *   A class that can store values for items in a compact way
 *   using a default value to remove storage for a common value.
 *   FIXME: Template the option to store the items in zoom levels.
 */
template<class VALUE_TYPE,
         class VALUE_SAVER =
                  typename LoaderTypeSelector<VALUE_TYPE>::loaderType >
class ItemDataTable: public DataTable<VALUE_TYPE> {
   friend class M3Creator;
private:
   /// helper class for max_element
   template <typename T>
   struct LessSecond:public std::less< T > {
      bool operator () (const T& a, const T& b) const {
         return a.second < b.second;
      }
   };

   static uint32 GET_ZOOM( uint32 itemID ) {
      return (((itemID) & 0x78000000) >> 27);
   }
   
   static uint32 GET_POS_IN_ZOOM( uint32 itemID ) {
      return ( (itemID) & 0x07ffffff);
   }
  public:
   /// Type of input for the table. Item id in first VALUE_TYPE in second
   typedef map<uint32, VALUE_TYPE> itemMap_t;
   
   /**
    *   Creates a new, empty table 
    */
   ItemDataTable( const VALUE_TYPE& defaultVal ) {
      m_defaultVal = defaultVal;
      init();
   }

   /**
    *   Creates a new table with the supplied default value.
    *   @param defaultVal The value to return if an item isn't found
    *                     in the table. Items with exactly this type will not
    *                     be added to the table.
    *   @param itemsToAdd The items to add.
    */
   ItemDataTable( const VALUE_TYPE& defaultVal,
                  const itemMap_t& itemsToAdd ) {
      m_defaultVal = defaultVal;
      init();
      setData( itemsToAdd );
   }
   
   /// Deallocates the memory previously allocated
   virtual ~ItemDataTable() {
      clear();
   }

   /// Returns the default value.
   const VALUE_TYPE& getDefault() const {
      return m_defaultVal;
   }

   void setValues( const VALUE_TYPE& defaultVal, 
                   const itemMap_t& items ) {
      m_defaultVal = defaultVal;
      init();
      setData( items );
   }

   const VALUE_TYPE& getData( uint32 itemID ) const {
#ifndef URIT_USES_ZOOM_LEVELS
      const uint32* begin = m_items;
      const uint32* end   = m_items + m_nbrItems;
      const uint32* item = std::lower_bound( begin,
                                             end,
                                             itemID );
      if ( item == end || *item != itemID ) {
         return m_defaultVal;
      } else {
         return m_itemDatas[ item - begin ];
      }
#else
      uint32 zoom = GET_ZOOM( itemID );
      
      if ( m_nbrItemsInZoom[zoom] == 0 ) {
         return m_defaultVal;
      }
   
      int pos  = GET_POS_IN_ZOOM( itemID ) - m_zoomOffsets[zoom];
      
      if ( pos < 0 || pos >= m_nbrItemsInZoom[zoom] ) {
         // It was before the first one.
         return m_defaultVal;
      } else {
         return m_dataPerZoom[zoom][pos];
      }
#endif
   }

   int getAllData( itemMap_t& data ) const {
#ifndef URIT_USES_ZOOM_LEVELS
      const uint32* begin = m_items;
      const uint32* end   = m_items + m_nbrItems;
      for ( const uint32* it = begin;
            it != end;
            ++it ) {
         data.insert( make_pair( *it, getData(*it) ) );
      }
#else
#error "Not implemented for zoom level storage"
#endif
      return data.size();
   }
   
   int getAllData( itemMap_t& data, const set<uint32>& allowedItems ) const {
#ifndef URIT_USES_ZOOM_LEVELS
      const uint32* begin = m_items;
      const uint32* end   = m_items + m_nbrItems;
      for ( const uint32* it = begin;
            it != end;
            ++it ) {
         if ( allowedItems.find( *it ) != allowedItems.end() ) {
            data.insert( make_pair( *it, getData(*it) ) );
         }
      }
#else
#error "Not implemented for zoom level storage"
#endif
      return data.size();
   }
   
   /**
    * @param items the map to count combinations from
    * @param combos adds all combinations to this one with
    *               counter in the second argument
    */
   template <typename TYPE_ONE, typename TYPE_TWO>
   static void getCombinations(const std::map<TYPE_ONE, TYPE_TWO> &items,
                               std::map<TYPE_TWO, uint32> &combos ) {

      typename std::map<TYPE_ONE, TYPE_TWO>::const_iterator it = items.begin();
      typename std::map<TYPE_ONE, TYPE_TWO>::const_iterator it_end =
         items.end();
      for ( ; it != it_end; ++it ) {
         combos[ it->second ]++;
      }
   }

   static typename itemMap_t::value_type
   getBestDefaultValue(const itemMap_t &items) {
      //
      // Calculate default value
      // by counting number of different combinations
      // and selecting the combination that has the most 
      // counts to be the default value
      //

      typedef std::map<VALUE_TYPE, uint32> reverseItemMap_t;
      reverseItemMap_t counterMap;

      getCombinations( items, counterMap );

      typename reverseItemMap_t::iterator max_it = 
         std::max_element( counterMap.begin(),
                           counterMap.end(),
                         LessSecond<typename reverseItemMap_t::value_type>() );

      return make_pair( max_it->second, max_it->first );
   }
   
   /// Sets the table to the items in itemsToAdd. @return nbr items added
   int setData( const itemMap_t& itemsToAdd ) {
      clear();

      // Count the items
      m_nbrItems = 0;
      for( typename itemMap_t::const_iterator it = itemsToAdd.begin();
           it != itemsToAdd.end();
           ++it ) {
         if ( it->second != m_defaultVal ) {
            ++m_nbrItems;
         }
      }
      
      // Fill in the items
      uint32* items  = new uint32[ m_nbrItems ];
      VALUE_TYPE* tmpItemDatas = new VALUE_TYPE[ m_nbrItems ];
      int curNbr = 0;
      for( typename itemMap_t::const_iterator it = itemsToAdd.begin();
           it != itemsToAdd.end();
           ++it ) {
         if ( it->second != m_defaultVal ) {
            items[ curNbr ]   = it->first;
            tmpItemDatas[ curNbr ] = it->second;
            ++curNbr;
         }
      }
      

      // Count items per zoom
      uint32 lowestItemPerZoom[NUMBER_GFX_ZOOMLEVELS];
      uint32 highestItemPerZoom[NUMBER_GFX_ZOOMLEVELS];
      for ( int i = 0; i < NUMBER_GFX_ZOOMLEVELS; ++i ) {
         lowestItemPerZoom[i] = MAX_UINT32;
         highestItemPerZoom[i] = 0;
      }
      // Make the items per zoom
      for ( typename itemMap_t::const_iterator it = itemsToAdd.begin();
            it != itemsToAdd.end();
            ++it ) {
         if ( it->second != m_defaultVal ) {
            uint8 zoom = GET_ZOOM( it->first );         
            lowestItemPerZoom[zoom] = MIN( lowestItemPerZoom[zoom],
                                           GET_POS_IN_ZOOM(it->first) );
            highestItemPerZoom[zoom] = MAX( highestItemPerZoom[zoom],
                                            GET_POS_IN_ZOOM( it->first ) );
         }
      }

      // Find out the number of items actually used per zoom.
      for ( uint32 zoom = 0; zoom < NUMBER_GFX_ZOOMLEVELS; ++zoom ) {
         if ( lowestItemPerZoom[zoom] != MAX_UINT32 ) {
            m_nbrItemsInZoom[zoom] =
               highestItemPerZoom[zoom] - lowestItemPerZoom[zoom];
            ++m_nbrItemsInZoom[zoom];
         } else {
            m_nbrItemsInZoom[zoom] = 0;
         }
      }
      
      VALUE_TYPE* dataPerZoom[NUMBER_GFX_ZOOMLEVELS];
      for ( uint32 zoom = 0; zoom < NUMBER_GFX_ZOOMLEVELS; ++zoom ) {
         int nbrForCurZoom = m_nbrItemsInZoom[zoom];
         m_zoomOffsets[zoom] = lowestItemPerZoom[zoom];
         if ( 0 == nbrForCurZoom) {
            dataPerZoom[zoom] = NULL;
         } else {
            mc2dbg << "[IDT]: zoom[" << zoom << "] contains "
                   << m_zoomOffsets[zoom] << "-"
                   << (m_zoomOffsets[zoom] + m_nbrItemsInZoom[zoom]) << endl;
            dataPerZoom[zoom] = new VALUE_TYPE[ nbrForCurZoom ];
            // Reset to the default
            for ( int j = 0; j < nbrForCurZoom; ++j ) {
               dataPerZoom[zoom][j] = m_defaultVal;
            }
            for( typename itemMap_t::const_iterator it = itemsToAdd.begin();
                 it != itemsToAdd.end();
                 ++it ) {
               if ( GET_ZOOM( it->first ) != zoom ) continue;
               int pos = GET_POS_IN_ZOOM(it->first) - m_zoomOffsets[zoom];
               if ( pos < 0 || pos >= m_nbrItemsInZoom[zoom] ) {
                  // Outside - should be default
                  MC2_ASSERT( it->second == m_defaultVal );
                  continue;
               }
               // Fill in the ones inside - even if default.
               dataPerZoom[zoom][pos] = it->second;
            }
         }
      }
      
      
      
      MC2_ASSERT( curNbr == m_nbrItems );
      MC2_ASSERT( is_sorted( items, items + m_nbrItems ) );
      
#ifndef URIT_USES_ZOOM_LEVELS
      // Keep the table
      m_items = items;
      m_itemDatas = tmpItemDatas;
      for ( int i = 0; i < NUMBER_GFX_ZOOMLEVELS; ++i ) {
         delete [] dataPerZoom[i];
      }
#else
      for ( int i = 0; i < NUMBER_GFX_ZOOMLEVELS; ++i ) {
         m_dataPerZoom[i] = dataPerZoom[i];
      }
      delete [] items;
      delete [] tmpItemDatas;
#endif
      
      // Sanity check.
      for ( typename itemMap_t::const_iterator it = itemsToAdd.begin();
            it != itemsToAdd.end();
            ++it ) {
         MC2_ASSERT( it->second == getData( it->first ) );
      }
      return m_nbrItems;
   }

   /// Returns true if the table is empty.
   bool empty() const {
      return ! m_nbrItems;
   }

   /// Clears the table
   void clear() {
#ifndef URIT_USES_ZOOM_LEVELS
      delete [] m_items;
      delete [] m_itemDatas;
#else
      for ( int i = 0; i < NUMBER_GFX_ZOOMLEVELS; ++i ) {
         delete [] m_dataPerZoom[i];
      }
#endif
      init();
   }

   uint32 getSizeInDataBuffer() const {
      uint32 size = 4 + 4 + 4;
      size += NUMBER_GFX_ZOOMLEVELS * 8;
      size += m_nbrItems * 4;
      size += VALUE_SAVER::getValSizeInDataBuffer( m_defaultVal );
      AlignUtility::alignLong( size );
#ifndef URIT_USES_ZOOM_LEVELS
      for ( int i = 0; i < m_nbrItems; ++i ) {
         size += VALUE_SAVER::getValSizeInDataBuffer( m_itemDatas[i] );
      }
#else
#endif
      AlignUtility::alignLong( size );
      return size;
   }
   
   /// Saves the table in the buffer
   void save( DataBuffer& buf ) const {
      DataBufferChecker dbc(buf, "ItemDataTable::save");
      buf.writeNextLong( getSizeInDataBuffer() );
      buf.writeNextLong( 0 ); // Version
      VALUE_SAVER::writeValue ( buf, m_defaultVal );
      buf.alignToLongAndClear();
      
      buf.writeNextLong( m_nbrItems );
      
      for ( int i = 0; i < NUMBER_GFX_ZOOMLEVELS; ++i ) {
         buf.writeNextLong( m_nbrItemsInZoom[i] );
         buf.writeNextLong( m_zoomOffsets[i] );      
      }
#ifndef URIT_USES_ZOOM_LEVELS
      for ( int i = 0; i < m_nbrItems; ++i ) {
         buf.writeNextLong( m_items[i] );
      }
      for ( int i = 0; i < m_nbrItems; ++i ) {
         VALUE_SAVER::writeValue( buf, m_itemDatas[i] );
      }
      buf.alignToLongAndClear();
#else   
#endif
      dbc.assertPosition( getSizeInDataBuffer() );
      mc2dbg << "[IDT]: Saved " << endl;
   }

   void load( DataBuffer& buf ) {
      load(buf, (VALUE_SAVER*)NULL );
   }
   
   /// Loads the table from the buffer
   template<class VALUE_LOADER>
   void load( DataBuffer& buf, VALUE_LOADER* unused ) {
      clear();
      DataBuffer readBuf( buf.getCurrentOffsetAddress(),
                          buf.getNbrBytesLeft() );
      uint32 length = readBuf.readNextLong();
      readBuf.readNextLong(); // Version - should be 0
      VALUE_LOADER::readValue( readBuf, m_defaultVal );
      readBuf.alignToLong();
      
      readBuf.readNextLong( m_nbrItems );
      
      for ( int i = 0; i < NUMBER_GFX_ZOOMLEVELS; ++i ) {
         readBuf.readNextLong( m_nbrItemsInZoom[i] );
         readBuf.readNextLong( m_zoomOffsets[i] );      
      }
#ifndef URIT_USES_ZOOM_LEVELS
      m_items = new uint32[ m_nbrItems ];
      m_itemDatas = new VALUE_TYPE[ m_nbrItems ];
      for ( int i = 0; i < m_nbrItems; ++i ) {
         readBuf.readNextLong( m_items[i] );
      }
      for ( int i = 0; i < m_nbrItems; ++i ) {
         VALUE_LOADER::readValue( readBuf, m_itemDatas[i] );
      }
      readBuf.alignToLong();
#else   
#endif
      // Skip unknown data.
      buf.readPastBytes( length );
   }
  private:
   
   void init() {
#ifndef URIT_USES_ZOOM_LEVELS
      m_items     = NULL;
      m_itemDatas = NULL;
#else
      for ( int i = 0; i < NUMBER_GFX_ZOOMLEVELS; ++i ) {
         m_dataPerZoom[i] = NULL;
      }
#endif
      m_nbrItems  = 0;
      for ( int i = 0; i < NUMBER_GFX_ZOOMLEVELS; ++i ) {
         m_nbrItemsInZoom[i] = 0;
         m_zoomOffsets[i] = 0;
      }
   }
   
   /// Number of items in each zoom level
   int m_nbrItemsInZoom[NUMBER_GFX_ZOOMLEVELS];
   /// Offset for first item in each zoom level
   int m_zoomOffsets[NUMBER_GFX_ZOOMLEVELS];
   /// The number of items
   int m_nbrItems;

#ifndef URIT_USES_ZOOM_LEVELS
   /// The items
   uint32* m_items;
   /// The UR-combination numbers
   VALUE_TYPE* m_itemDatas;
#else
   /// Storage of the URTypes
   VALUE_TYPE* m_dataPerZoom[NUMBER_GFX_ZOOMLEVELS];
#endif
   /// The default value to return.
   VALUE_TYPE m_defaultVal;
};

#endif
