/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ITEM_COMBO_TABLE_H
#define ITEM_COMBO_TABLE_H

#include "ItemDataTable.h"
#include "DataTable.h"
#include "ItemComboTableFwd.h"
#include "AlignUtility.h"

#include <algorithm>

/**
 *    Table to use for storing a few combinations of large data.
 *    See UserRightsItemTable for an example of usage.
 *    @param VALUE_TYPE    The data to store.
 *    @param VALUE_SAVER   Something that has the functions
 *                         writeValue( DataBuffer*, const VALUE_TYPE& val )
 *                         readValue( DataBuffer*, VALUE_TYPE& val )
 *                         getValSizeInDataBuffer( const VALUE_TYPE& val )
 *    @param INTEGER_TYPE  Type of UNSIGNED integer to map the combinations
 *                         into. If less than 256 combos, use uint8 etc.
 */
template<class VALUE_TYPE, class INTEGER_TYPE, 
         class VALUE_SAVER_T 
         //The default value is actually set in ItemComboTableFwd.h
          /*= LoaderTypeSelector<VALUE_TYPE> */>
class ItemComboTable: public DataTable<VALUE_TYPE> {
private:
   typedef typename LoaderTypeSelector<VALUE_TYPE>::loaderType VALUE_SAVER;
   typedef ItemDataTable<INTEGER_TYPE,
                         ItemComboLoader<INTEGER_TYPE> > otherTable_t;
   typedef typename ItemDataTable<INTEGER_TYPE,
                         ItemComboLoader<INTEGER_TYPE> >::itemMap_t otherMap_t;
   friend class M3Creator;
public:
   /// Type of input for the table. Item id in first VALUE_TYPE in second
   typedef std::map<uint32, VALUE_TYPE> itemMap_t;

   /// helper class for max_element
   template <typename T>
   struct LessSecond:public std::less< T > {
      bool operator () (const T& a, const T& b) const {
         return a.second < b.second;
      }
   };

   ItemComboTable() {
      init();
   }

   /**
    *   Creates a new table with the supplied default value.
    *   @param defaultVal The value to return if an item isn't found
    *                     in the table. Items with exactly this value will not
    *                     be added to the table.
    *   @param itemsToAdd The items to add.
    */
   ItemComboTable( const VALUE_TYPE& defaultVal,
                   const itemMap_t& itemsToAdd ) {
      init();
      setValues( defaultVal, itemsToAdd );
   }

   explicit ItemComboTable( const itemMap_t& itemsToAdd ) {
      init();
      typename itemMap_t::value_type def = getBestDefaultValue( itemsToAdd );

      setValues( def.second, itemsToAdd ); 
   }

   virtual ~ItemComboTable() {
      clear();
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
         if ( combos.find( it->second ) == combos.end() ) {
            combos[ it->second ] = 1;
         } else {
            combos[ it->second ] = combos[ it->second ] + 1;
         }
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


   /// Return the default value
   const VALUE_TYPE& getDefault() const {
      return m_combos[ m_dataTable->getDefault() ];
   }

   /// Inefficient copy constructor.
   ItemComboTable( const ItemComboTable& other ) {
      init();
      itemMap_t itemsToAdd;
      other.getAllData( itemsToAdd );
      setValues( other.getDefault(), itemsToAdd );
   }

   /// Inefficient constructor that removes all items not in the set
   ItemComboTable( const ItemComboTable& other,
                   const set<uint32>& allowedItems ) {
      VALUE_TYPE defVal = other.getDefault();
      init();
      itemMap_t itemsToAdd;
      other.getAllData( itemsToAdd, allowedItems );
      setValues( defVal, itemsToAdd );
   }
   
   /// Inefficient operator=
   ItemComboTable& operator=( const ItemComboTable& other ) {
      if ( this != & other ) {
         VALUE_TYPE defVal = other.getDefault();
         clear();
         itemMap_t itemsToAdd;
         other.getAllData( itemsToAdd );
         setValues( defVal, itemsToAdd );
      }
      return *this;
   }

   /// A non-efficient way to get all the data in the table
   int getAllData( itemMap_t& res ) const {
      otherMap_t tmpMap;
      m_dataTable->getAllData( tmpMap );
      for( typename otherMap_t::const_iterator it = tmpMap.begin();
           it != tmpMap.end();
           ++it ) {
         res.insert( make_pair( it->first, m_combos[ it->second ] ) );
      }
      return res.size();
   }
   
   /// A non-efficient way to get all the data in the table
   int getAllData( itemMap_t& res, const set<uint32>& allowedItems ) const {
      otherMap_t tmpMap;
      m_dataTable->getAllData( tmpMap, allowedItems );
      for( typename otherMap_t::const_iterator it = tmpMap.begin();
           it != tmpMap.end();
           ++it ) {
         res.insert( make_pair( it->first, m_combos[ it->second ] ) );
      }
      return res.size();
   }
   
   
   /// Returns the data for the item. If not found, the default is returned.
   const VALUE_TYPE& getData( uint32 itemID ) const {
      return m_combos[ m_dataTable->getData( itemID ) ];
   }
   
   void setValues( const VALUE_TYPE& defaultVal,
                   const itemMap_t& itemsToAdd ) {
      clear();
      // Get the combos
      typedef map<VALUE_TYPE, uint32> comboMap_t;
      comboMap_t comboMap;
      for( typename itemMap_t::const_iterator it = itemsToAdd.begin();
           it != itemsToAdd.end();
           ++it ) {
         // Always try to insert the URType, even if it is the default.
         comboMap.insert( make_pair( it->second, comboMap.size() ) );
      }
      // Also insert the default, if no-one has it.
      comboMap.insert( make_pair( defaultVal, comboMap.size() ) );
      // This is here because numeric_limits doesn't seem to exist.
      uint32 maxNbr = INTEGER_TYPE(-1) + 1;
      if ( maxNbr == 0 ) {
         // uint32 overflows, we'll miss one
         maxNbr = MAX_UINT32;
      }
      
      if ( ! itemsToAdd.empty() ) {
         // Don't print this if not adding stuff.
         mc2dbg << "[ICT]: Nbr ur-type combinations: "
                << comboMap.size() << " of max " << maxNbr << endl;
      }
      
      // This is here because numeric_limits doesn't seem to exist.
      MC2_ASSERT( comboMap.size() <= maxNbr );
      m_nbrCombos = comboMap.size();
      m_combos = new VALUE_TYPE[ m_nbrCombos ];
      
      // Fill in the combos
      for ( typename comboMap_t::const_iterator it = comboMap.begin();
            it != comboMap.end();
            ++it ) {
         m_combos[ it->second ] = it->first;
      }

      // Make the map for the other table.
      otherMap_t dataMap;
      for ( typename itemMap_t::const_iterator it = itemsToAdd.begin();
            it != itemsToAdd.end();
            ++it ) {
         if ( it->second != defaultVal ) {
            dataMap.insert( make_pair( it->first, comboMap[it->second] ) );
         }
      }
      m_dataTable =
         new otherTable_t(comboMap[defaultVal], dataMap );
      for ( typename itemMap_t::const_iterator it = itemsToAdd.begin();
            it != itemsToAdd.end();
            ++it ) {
         MC2_ASSERT( getData( it->first ) == it->second );
      }
   }

   uint32 getSizeInDataBuffer() const {
      uint32 size = 4 + 4 + 4 + 4;
      for ( uint32 i = 0; i < m_nbrCombos; ++i ) {
         size += VALUE_SAVER::getValSizeInDataBuffer( m_combos[i] );
      }
      // Align to long
      AlignUtility::alignLong( size );
      size = size + m_dataTable->getSizeInDataBuffer();
      return size;
   }
   
   void save( DataBuffer& buf ) const {
      DataBufferChecker dbc( buf, "ItemComboTable::save" );
      buf.writeNextLong( getSizeInDataBuffer() );
      buf.writeNextLong( 0 ); // Version
      buf.writeNextLong( m_nbrCombos );
      buf.writeNextLong( sizeof( INTEGER_TYPE ) );
      for ( uint32 i = 0; i < m_nbrCombos; ++i ) {
         VALUE_SAVER::writeValue( buf, m_combos[i] );
      }
      buf.alignToLongAndClear();
      m_dataTable->save( buf );
      dbc.assertPosition( getSizeInDataBuffer() );
      mc2dbg << "[ICT]: Saved" << endl;
   }

   void load( DataBuffer& inbuf ) {
      clear();
      // Some tricks to make it easier
      // Read from a temporary buffer.
      DataBuffer buf( inbuf.getCurrentOffsetAddress(),
                      inbuf.getNbrBytesLeft() );
      uint32 length = int_load( buf );
      // Then skip unknown data
      inbuf.readPastBytes( length );
   }
   
   /// Clears all
   void clear() {
      delete [] m_combos;
      delete m_dataTable;
      init();
   }

private:
   uint32 int_load( DataBuffer& buf ) {
      
      const uint32 length = buf.readNextLong();
      buf.readNextLong(); // Version
      buf.readNextLong( m_nbrCombos );

      mc2dbg << "[ICT]: load: Nbr combos = " << m_nbrCombos << endl;

      const uint32 size_of_int = buf.readNextLong();
      m_combos = new VALUE_TYPE[ m_nbrCombos ];
      for ( uint32 i = 0; i < m_nbrCombos; ++i ) {
         VALUE_SAVER::readValue( buf, m_combos[i] );
      }
      
      buf.alignToLong();
      // Create new datatable with the wrong default value.
      // Will be correct when loaded.
      m_dataTable = new otherTable_t(INTEGER_TYPE());
      switch ( size_of_int ) {
         case 1:
            mc2dbg << "[ICT]: load: Loading uint8:s" << endl;
            m_dataTable->load( buf, (ItemComboLoader<uint8>*)NULL );
            break;
         case 2:
            mc2dbg << "[ICT]: load: Loading uint16:s" << endl;
            m_dataTable->load( buf, (ItemComboLoader<uint16>*)NULL );
            break;
         default:
            mc2dbg << "[ICT]: load: Loading uint32:s" << endl;
            m_dataTable->load( buf, (ItemComboLoader<uint32>*)NULL );
            break;
      }

      mc2dbg << "[ICT]: load: Read " << length << " bytes" << endl;

      return length;
   }

   /// Inits an already empty table
   void init() {
      m_dataTable = NULL;
      m_combos = NULL;
      m_nbrCombos = 0;
   }

   /// Array of combinations
   VALUE_TYPE* m_combos;
   /// Number of combinations
   uint32 m_nbrCombos;
   /// The table of ints.
   otherTable_t* m_dataTable;
   
};

#endif
