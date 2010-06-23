/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DATATABLESELEC_H
#define DATATABLESELEC_H

#include "DataTable.h"
#include "config.h"

#include "ItemComboTable.h"

#include <memory>
#include <typeinfo>

/**
 * Selects among different DataTables
 * A bridge for real data tables
 */
template <typename DataType, typename TableType = std::map<uint32, DataType> >
class SelectableDataTable: public DataTable<DataType, TableType> 
{
public:
   typedef TableType table_t;
   /// @param bits the number of bits for index in data table
   explicit SelectableDataTable( uint32 bits = 0);
   virtual ~SelectableDataTable() { }


   /**
    * Sets the values to data table
    * Calculates best default value and the best size of index 
    * and then creates an appropiate data table and inserts the values
    * @param values the values to copy
    */ 
   void setValues( const table_t& values );

   /// @see DataTable
   void setValues( const DataType &defaultVal, 
                   const table_t& values );

   /// @see DataTable
   void load( DataBuffer& data );
   /// @see DataTable
   void save( DataBuffer& data ) const;
   /// @see DataTable
   const DataType& getData( uint32 index ) const;
   /// @see DataTable
   uint32 getSizeInDataBuffer() const;

   /// select a data table type from number of bits
   /// @param bits the number of bits for index
   void select( uint32 bits );
   /// @return number of bits that this table uses for index
   uint32 getNbrOfBitsInIndex() const;

private:
   /// not copyable
   SelectableDataTable( const SelectableDataTable<DataType>& );
   /// not assignable
   SelectableDataTable<DataType> &operator = 
                                     ( const SelectableDataTable<DataType>& );

   std::auto_ptr< DataTable<DataType> > m_dataTable; /// internal data base
};



template <typename DataType, typename TableType>
SelectableDataTable<DataType, TableType>::SelectableDataTable( uint32 bits ) 
{
   select( bits );
}

template <typename DataType, typename TableType>
void
SelectableDataTable<DataType, TableType>::
setValues( const TableType& values )
{
   std::map<DataType, uint32> combos;
   ItemComboTable< DataType, uint8 >::getCombinations( values, combos );
   // determine how many bits we need for index
   uint8 bits = 0;
   if ( combos.size() > 0xFFFF ) {
      bits = 32;
   } else if ( combos.size() < 0xFF ) {
      bits = 8;
   } else {
      bits = 16;
   }

   // calculate default value
   typename TableType::value_type def =
      ItemComboTable< DataType, uint8 >::getBestDefaultValue( values );

   // determine if we should use ItemDataTable instead of ItemComboTable
   uint32 nbrDefaults = def.first;
   uint32 nbrNonDefaults = values.size() - nbrDefaults;
   uint32 sizeOfDataTable = nbrNonDefaults * ( sizeof( DataType ) +  
                                               sizeof( uint32 ) );
   uint32 sizeOfComboTable = 
      nbrNonDefaults * (  bits / 8 + sizeof( uint32 ) ) +
      combos.size() * sizeof( DataType );
      

   if ( sizeOfDataTable < sizeOfComboTable ) {
      bits = 0; // use ItemDataTable
   }

   select( bits );

   setValues( def.second, values );
}

template <typename DataType, typename TableType>
void
SelectableDataTable<DataType, TableType>::
setValues( const DataType &defaultVal, 
           const table_t& values ) 
{
   m_dataTable->setValues( defaultVal, values );
}

template <typename DataType, typename TableType>
void 
SelectableDataTable<DataType, TableType>::
load( DataBuffer& data ) 
{
   uint32 bits = data.readNextLong();
   mc2dbg << "[SelectableDT] Loading index with " << bits << " bits " << endl;
   select( bits );
   m_dataTable->load( data );
}

template <typename DataType, typename TableType>
void 
SelectableDataTable<DataType, TableType>::
save( DataBuffer& data )  const
{
   uint32 bits = getNbrOfBitsInIndex();
   mc2dbg << "[SelectableDT] saving index with " << bits << " bits " << endl;
   data.writeNextLong( bits );
   m_dataTable->save( data );
}

template <typename DataType, typename TableType>
const DataType& 
SelectableDataTable<DataType, TableType>::
getData( uint32 index ) const
{
   return m_dataTable->getData( index );
}

template <typename DataType, typename TableType>
uint32 SelectableDataTable<DataType, TableType>::
getSizeInDataBuffer() const 
{
   // four extra for bits in index 
   return m_dataTable->getSizeInDataBuffer() + 4;
}

template <typename DataType, typename TableType>
void SelectableDataTable<DataType, TableType>::
select( uint32 bits ) 
{
   switch ( bits ) {
   case 0:
      m_dataTable.reset( new ItemDataTable<DataType>( DataType() ) );
      break;
   case 8:
      m_dataTable.reset( new ItemComboTable<DataType, uint8>() );
      break;
   case 16:
      m_dataTable.reset( new ItemComboTable<DataType, uint16>() );
      break;
   case 32:
      m_dataTable.reset( new ItemComboTable<DataType, uint32>() );
      break;
   default:
      // should throw something, but the rest of the code is not ready....
      mc2dbg << "Can not create DataTable<DataType> with " << bits 
             << " bits" << endl;
      break;
   }
}

template <typename DataType, typename TableType>
uint32 
SelectableDataTable<DataType, TableType>::
getNbrOfBitsInIndex() const 
{
   if ( typeid( *m_dataTable.get() ) == 
        typeid( ItemDataTable<DataType> ) ) {
      return 0;
   } else if ( typeid( *m_dataTable.get() ) ==
               typeid( ItemComboTable< DataType, uint8 > ) ) {
      return 8;
   } else if ( typeid( *m_dataTable.get() ) ==
               typeid( ItemComboTable< DataType, uint16 > ) ) {
      return 16;
   } else if ( typeid( *m_dataTable.get() ) ==
               typeid( ItemComboTable< DataType, uint32 > ) ) {
      return 32;
   }
   // should throw...
   return -1;
}

#endif 
