/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SQLTABLEDATA_H
#define SQLTABLEDATA_H

#include "config.h"
#include "SQLDataContainer.h"
#include "MC2String.h"

/**
 * A class for holding table data for a sql database table
 */
class SQLTableData {

public:

   /**
    * A struct holding information about a column in the database table
    */
   struct TableDataColumn {
      /**
       * Creates a new TableDataColumn with the information about it.
       *
       * @param name The column value name
       * @param isNumeric Set to true if the column value is a numeric type,
       *                  if not set it to false
       * @param isKey Set to true if the column value is a primary key in the
       *              database table, if not set it to false.
       */
      TableDataColumn( const MC2String& name, bool isNumeric, bool isKey )
         : m_name( name), m_isNumeric( isNumeric ), m_isKey( isKey )
      {}

      /** The colum value name. */
      MC2String m_name;

      /** True if the value is a numeric type, false otherwise. */
      bool m_isNumeric;

      /** True if the value is a primary key in the table, false otherwise. */
      bool m_isKey;
   };

   typedef vector< TableDataColumn > tableData_t;
   
   /**
    * Creates a new TableDataColumn with the information about it.
    *
    * @param tableName The name of the databse table
    */
   SQLTableData( const MC2String tableName );

   ~SQLTableData();

   /**
    * Set the information about a column in a table.
    *
    * @param column The data for a column.
    */
   void setColumnName( const TableDataColumn& column );
   
   /**
    * Returns the name of the table
    */
   const MC2String& getTableName();

   /**
    * Returns the table data
    */
   const tableData_t& getTableData();

private:
   
   /** The string holding the name of the database table */
   MC2String m_tableName;

   /** The table data */
   tableData_t m_tableData;

};

#endif // SQLDATACONTAINER_H
