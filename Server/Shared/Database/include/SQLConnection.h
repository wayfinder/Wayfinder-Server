/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SQLCONNECTION_H
#define SQLCONNECTION_H

#include "SQLDriver.h"
#include "SQLQuery.h"

/**
 *  Encapsulates a connection to a SQL database backend
 *
 */
class SQLConnection
{
   public:

      /**
        *  Creates a connection
        *  @param driver An instance of the driver to use
        *  @param database The database to use
        *  @param user The user to connect as
        *  @param password The password to use
        */
      SQLConnection(SQLDriver* driver);

      /**
        *  Disposes of the SQLConnection
        */
      virtual ~SQLConnection();

      /**
        *  Do the connect to the database
        *  @return true if successfull
        */
      bool connect();

      /**
        *  Check if the connection to the database is up, should do an
        *  automatic reconnect if its down. Only call this if an 
        *  earlier call to connect() has succeeded.
        *  @return true if its up or the reconnection succeeded, 
        *          false if there's a problem
        */
      bool ping();

      /**
        *  Check if a certain table exists
        *  @param tableName Pointer to the table name
        *  @return true if it exists
        */
      bool tableExists(const char* tableName);
      bool tableExists(const MC2String& tableName);

      /**
        *  Start a transaction
        *  @return true if successfull
        */
      bool beginTransaction();

      /**
        *  Commit a transaction
        *  @return true if successfull
        */
      bool commitTransaction();

      /**
        *  Rollback a transaction
        *  @return true if successfull
        */
      bool rollbackTransaction();

      /**
        *  Create a new query
        *  @return Pointer to a new SQLQuery
        */
      virtual SQLQuery* newQuery();

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

      SQLDriver* m_driver;
};

#endif // SQLCONNECTION_H
