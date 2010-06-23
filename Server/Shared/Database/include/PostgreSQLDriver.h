/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef POSTGRESQLDRIVER_H
#define POSTGRESQLDRIVER_H

#include "libpq-fe.h"
#include "SQLDriver.h"

/**
 *  A SQLDriver for PostgreSQL
 *
 */
class PostgreSQLDriver : public SQLDriver
{
   public:

      /**
        *  Creates a driver
        *  @param host The host to connect to
        *  @param database The database to use
        *  @param user The user to connect as
        *  @param password The password to use
        */
      PostgreSQLDriver(const char* host,
                       const char* database,
                       const char* user,
                       const char* password);

      /**
        *  Disposes of the PostgreSQLDriver
        */
      virtual ~PostgreSQLDriver();

      /**
        *  Do the actual connect to the database
        *  @return true if successfull
        */
      virtual bool connect();

      /**
        *  Check if the connection to the database is up, should do an
        *  automatic reconnect if its down. Only call this if an 
        *  earlier call to connect() has succeeded.
        *  @return true if its up or the reconnection succeeded, 
        *          false if there's a problem
        */
      virtual bool ping();

      /**
        *  Set "master" status of this driver, a master
        *  is allowed to perform changes in the backend
        *  such as changing master/slave replication states.
        *  This is set to true for modules acting as leaders.
        *  @param master The new master status
        *  @return true if successfull
        */
      virtual bool setMaster(bool master);

      /**
        *  Prepare the query for use
        *  @param query The query string to prepare
        *  @return Pointer to prepared query or the pointer that was passed,
        *  if a new pointer was passed, you have to call freePrepared()
        *  later! This function returns NULL if something goes wrong.
        */
      virtual const char* prepare(const char* query);

      /**
        *  Free a prepared query
        *  @param prepQuery Pointer that was returned by an earlier call to
        *                   prepare()
        */
      virtual void freePrepared(const char* prepQuery);

      /**
        *  Sends the query to the server and get the result
        *  @param query Pointer to the (prepared) query string
        *  @return Pointer to a handle for the result or NULL if something
        *          went (really) wrong.
        */
      virtual const void* execute(const char* query);

      /**
        *  Free a result
        *  @param result Result handle
        */
      virtual void freeResult(const void* result);

#if 0
      /**
        *  Get the number of rows that a query result contains
        *  @param result Result handle
        *  @return The number of rows
        */
      virtual uint32 getNumRows(const void* result);
#endif

      /**
        *  Get the number of columns that a query result contains
        *  @param result Result handle
        *  @return The number of columns
        */
      virtual uint32 getNumColumns(const void* result);

      /**
        *  Get the next row for a result
        *  @param result Result handle
        *  @return NUll if error, otherwise pointer to row handle, you have to
        *  call freeRow() for the returned pointer when you're done!
        */
      virtual const void* nextRow(const void* result);

      /**
        *  Free a row
        *  @param row Pointer that was returned by an earlier call to
        *             nextRow()
        */
      virtual void freeRow(const void* row);
      
      /**
        * Get the column names
        * @param result Result handle
        * @param colNames vector to hold the names
        */
      virtual void getColumnNames( const void* result, 
                                   vector< MC2String >& colNames );

      /**
        *  Get a column value
        *  @param result Result handle
        *  @param row Row handle
        *  @return Pointer to column value
        */
      virtual const char* getColumn(const void* result,
                                    const void* row,
                                    int colIndex);
      
      /**
        *  Get the error status
        *  @param result Result handle
        *  @return 0 if OK
        */
      virtual int getError(const void* result);

      /**
        *  Get current error status as a string
        *  @param result Result handle
        *  @return A string describing the current status
        */
      virtual const char* getErrorString(const void* result);

      /**
        *  Check if a certain table exists
        *  @param tableName Pointer to the table name
        *  @return true if it exists
        */
      virtual bool tableExists(const char* tableName);

      /**
        *  Start a transaction
        *  @return true if successfull
        */
      virtual bool beginTransaction();

      /**
        *  Commit a transaction
        *  @return true if successfull
        */
      virtual bool commitTransaction();

      /**
        *  Rollback a transaction
        *  @return true if successfull
        */
      virtual bool rollbackTransaction();

   private:    

      /**
       *    This internal struct keeps of track of a result
       */

      struct PostgreSQLResult
      {
         PGresult* res;
         int    curRow;
         int    maxRow;
      };

      /**
       *    PostgreSQL database states
       */ 
      enum dbStates {
         POSTGRESQL_CONNECTED,
         POSTGRESQL_NOT_CONNECTED,
         POSTGRESQL_CONNECTED_NO_DB
      };

      /**
       *   The database connection handle
       */
      PGconn* m_pgconn;

      /**
       *   The current state for the connection
       */
      int m_dbState;

      /**
        *  Pointer to the hostname
        */
      const char* m_host;

      /**
        *  Pointer to the database name
        */
      const char* m_database;

      /**
        *  Pointer to the username
        */
      const char* m_user;

      /**
        *  Pointer to the user's password
        */
      const char* m_password;
};

#endif // POSTGRESQLDRIVER_H
