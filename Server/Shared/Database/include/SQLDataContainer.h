/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SQLDATACONTAINER_H 
#define SQLDATACONTAINER_H

#include "config.h"
#include "SQLQuery.h"
#include "MC2String.h"

// Forward declaration
class Packet;


class SQLDataContainer {
public:

   /**
    * A class to keep track if the value is numeric or not.
    */
   class NameAndIsNum : public pair< MC2String, bool > {
      public:
         /**
          * Creates a NameAndIsNum with the value name and if its numeric.
          *
          * @param value The name of value.
          * @param isNumeric Set to true if it value is a numeric type, if not
          *                  set it to false.
          */
         NameAndIsNum( MC2String value, bool isNumeric )
            : pair< MC2String, bool >( value, isNumeric ) {}
   };
   
   typedef vector< NameAndIsNum > rowName_t;
   typedef vector< vector< MC2String > >  rowValues_t;

   /** Default constructor */
   SQLDataContainer();

   /** 
    * Creates a SQLDataContainer with the column names and a row of values.
    *
    * @param rowNames The column names.
    * @param rowValue The values of a row.
    */
   SQLDataContainer( const rowName_t& rowNames, 
                     const rowValues_t::value_type& rowValue );

   ~SQLDataContainer();

   /**
    * Get the number of rows.
    *
    * @return The number of value rows.
    */
   inline uint32 getNbrRows() const;

   /**
    * Append the key and value to the SQL query statement.
    * To be used when asking for data from the database.
    *
    * @param s The query string that we wish to append the key and value to.
    */
   void appendRowCol( MC2String& s ) const;
   
   /**
    * Append the key and value to the SQL query statement.
    * To be used when inserting data to the database.
    *
    * @param s The query string we wish to append the values to.
    */
   void appendValues( MC2String& s ) const;

   /**
    * Get the data from p and set into this.
    *
    * @param p The SQL query.
    *
    * @return Returns true if succeded false otherwise.
    */
   bool getSQLData( SQLQuery* p );

   /**
    * Get the value from a certain row and column.
    *
    * @param row The row number.
    * @param col The column name in the database.
    * @param value The value we are looking for will be set to this.
    *
    * @return Returns true if succeded false otherwise.
    */
   bool getCol( uint32 row, const MC2String& col, MC2String& value ) const;

   /**
    * Load from packet.
    *
    * @param p The packet we want to load from.
    * @param pos The header size of the packet type.
    *
    * @return Returns true if succeded false otherwise.
    */
   void load( const Packet* p, int& pos );

   /**
    * Save to packet.
    *
    * @param p The packet we want to save to.
    * @param pos The header size of the packet type.
    *
    * @return Returns true if succeded false otherwise.
    */
   void save( Packet* p, int& pos ) const;

   /**
    * Returns the size of the packet.
    *
    * @return Size in packet.
    */
   uint32 getSizeInPacket() const;

private:
   
   /// Holds the names and if the type is numeric of the columns in the table.
   rowName_t m_rowNames;

   /// Holds the data from the table.
   rowValues_t m_rowValues;
};

inline uint32 
SQLDataContainer::getNbrRows() const
{
   return m_rowValues.size();
}
#endif //SQLDATACONTAINER_H 
