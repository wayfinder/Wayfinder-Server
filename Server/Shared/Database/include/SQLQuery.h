/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SQLQUERY_H
#define SQLQUERY_H

#include "SQLDriver.h"
#include "MC2String.h"
/**
 *  Encapsulates a SQL query and its result
 *
 */
class SQLQuery
{
   public:
      // Gives access to the constructor.
      friend class SQLConnection;

      /**
        *  Disposes of the SQLQuery
        */
      virtual ~SQLQuery();

      /**
        *  Prepare a query for use
        *  @param query The query string to prepare
        */
      virtual bool prepare(const char* query);

      /**
        *  Get a column from the current row
        *  @param result Result handle
        *  @return Pointer to column value
        */
      virtual const char* getColumn(int colIndex);


      /**
        * Get the column names
        * @param colNames vector to hold the names
        */
      void getColumnNames( vector<MC2String>& colNames );

      /**
       *   Returns a raw column, i.e. no character
       *   conversion is done.
       */
      const char* getRawColumn(int colIndex);
      
      /**
        *  Sends the query to the server and get the result
        *  @param query Pointer to the (prepared) query string
        *  @return Pointer to a handle for the result or NULL if something
        *          went (really) wrong.
        */
      bool execute();

      /**
        *  Prepare and execute a query at the same time
        *  @param query The query string to use
        */
      bool prepAndExec(const char* query);
      bool prepAndExec(const MC2String& query);

#if 0
      /**
        *  Get the number of rows that the query result contains
        *  @return The number of rows
        */
      uint32 getNumRows();
#endif
      /**
        *  Get the number of columns that the query result contains
        *  @return The number of columns
        */
      uint32 getNumColumns();

      /**
        *  Get the next row for a result
        *  @return true if successfull, false if no more rows or if an error
        *  ocurred
        */
      bool nextRow();

      /**
        *  Get the error status
        *  @param result Result handle
        *  @return 0 if OK
        */
      int getError();

      /**
        *  Get current error status as a string
        *  @param result Result handle
        *  @return A string describing the current status
        */
      const char* getErrorString();

   protected:

      /**
        *  Creates a query.
        *  @param driver The driver to use.
        */
      SQLQuery(SQLDriver* driver);


      /**
        *   Pointer to the driver to use
        */
      SQLDriver* m_driver;

      /**
        *   Pointer to my result
        */
      const void* m_result;

      /**
        *   Pointer to the current row
        */
      const void* m_row;

      /**
        *   Pointer to the prepared query
        */
      const char* m_prepQuery;

   private:

      /**
        *   Pointer to the "raw" query
        */
      const char* m_rawQuery;

};

#endif // SQLQUERY_H
