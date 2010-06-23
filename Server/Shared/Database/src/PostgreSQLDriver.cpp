/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PostgreSQLDriver.h"
#include "config.h"
#include "StringUtility.h"

PostgreSQLDriver::PostgreSQLDriver(const char* host,
                         const char* database,
                         const char* user,
                         const char* password)
{
   m_host = host;
   m_database = database;
   m_user = user;
   m_password = password;
   m_dbState = POSTGRESQL_NOT_CONNECTED;
}

bool
PostgreSQLDriver::connect()
{
   char* connInfo = new char[strlen(m_host) + strlen(m_database) +
                             strlen(m_user) + strlen(m_password) + 50];

   sprintf(connInfo, "host='%s' dbname='%s' user='%s' password='%s'",
           m_host, m_database, m_user, m_password);

   if ((m_pgconn = PQconnectdb(connInfo)) == NULL) {
      mc2log << warn << "[PostgreSQLDriver] Internal error, PQconnectdb "
                        "returned NULL";
      m_dbState = POSTGRESQL_NOT_CONNECTED;
   } else {
      if (PQstatus(m_pgconn) != CONNECTION_OK) {
         mc2log << warn << "[PostgreSQLDriver] Error connecting to database: "
                << PQerrorMessage(m_pgconn) << endl;;
         m_dbState = POSTGRESQL_CONNECTED_NO_DB;
         PQfinish(m_pgconn);
      } else {
         m_dbState = POSTGRESQL_CONNECTED;
      }
   }

   delete [] connInfo;
   return (POSTGRESQL_CONNECTED == m_dbState);
}   

bool
PostgreSQLDriver::ping()
{
   if (PQstatus(m_pgconn) != CONNECTION_OK) {
      PQreset(m_pgconn);
      return (PQstatus(m_pgconn) == CONNECTION_OK);
   } else
      return true;
}

PostgreSQLDriver::~PostgreSQLDriver()
{
   PQfinish(m_pgconn); 
}

bool
PostgreSQLDriver::setMaster(bool master)
{
   // not supported
   return false;
}

const char*
PostgreSQLDriver::prepare(const char* query)
{
   if ( strncasecmp( "create table", query, 12 ) == 0 ||
        strncasecmp( "alter table", query, 11 ) == 0 )
   {
      mc2dbg8 << "PostgreSQLDriver::prepare(): this is a create/alter "
              << "table stmt!" << endl;
      char* replaced = StringUtility::replaceString(query, "MC2BLOB", "TEXT");
      if (replaced != NULL)
         return replaced;
   }

   return query;
}

void
PostgreSQLDriver::freePrepared(const char* prepQuery)
{
   mc2log << warn << "PostgreSQLDriver::freePrepared() call me should you not!";
}

const void*
PostgreSQLDriver::execute(const char* query)
{
   PostgreSQLResult* result;

   result = new PostgreSQLResult;
   if ((result->res = PQexec(m_pgconn, query)) == NULL) {
      mc2log << warn << "[PostgreSQLDriver] Internal error, PQexec returned "
                        "NULL";
      return NULL;
   } else {
      result->curRow = -1;
      result->maxRow = PQntuples((PGresult*)result->res) - 1;
      return result;
   }
}

void
PostgreSQLDriver::freeResult(const void* result)
{
   PQclear(((PostgreSQLResult*)result)->res);
   delete (PostgreSQLResult*)result;
}

/*uint32
PostgreSQLDriver::getNumRows(const void* result)
{
   return ((PostgreSQLResult*)res)->maxRow;
} */

uint32
PostgreSQLDriver::getNumColumns(const void* result)
{
   return PQnfields(((PostgreSQLResult*)result)->res);
   
}

const void*
PostgreSQLDriver::nextRow(const void* result)
{
   if (((PostgreSQLResult*)result)->curRow <
       ((PostgreSQLResult*)result)->maxRow) {
      ((PostgreSQLResult*)result)->curRow++;
      return (void*)&((PostgreSQLResult*)result)->curRow;
   } else
      return NULL;
}

void
PostgreSQLDriver::freeRow(const void* row)
{
   // NOP for PostgreSQL
}

void 
PostgreSQLDriver::getColumnNames( const void* result, vector< MC2String >& colNames ) 
{
   uint32 nbrColumns = getNumColumns( result );
   for ( uint32 i = 0; i < nbrColumns; ++i ) {
      colNames.push_back( PQfname( ((PostgreSQLResult*)result)->res, i ) );
   }
}

const char*
PostgreSQLDriver::getColumn(const void* result, const void* row, int colIndex)
{
   return PQgetvalue(((PostgreSQLResult*)result)->res,
                     ((PostgreSQLResult*)result)->curRow, colIndex);
}
      
int
PostgreSQLDriver::getError(const void* result)
{
   ExecStatusType status;
   if (result == NULL)
      return 0;  // connection errors not supported for now
   status = PQresultStatus(((PostgreSQLResult*)result)->res);
   if ((status != PGRES_COMMAND_OK) && (status != PGRES_TUPLES_OK)) {
      return 1;
   }
   return 0;
}

const char*
PostgreSQLDriver::getErrorString(const void* result)
{
   if (result == NULL)
      return "";  // connection errors not supported for now
   return PQresultErrorMessage(((PostgreSQLResult*)result)->res);
}

bool
PostgreSQLDriver::tableExists(const char* tableName)
{
   char query[1024];
   PGresult* res;

   // \todo
   // to gain speed; read table list when you connect and put
   // them in a hash_map (problems when creating new ones though)
   sprintf(query, "SELECT * FROM pg_class WHERE relname LIKE lower('%s')",
           tableName);
   if ((res = PQexec(m_pgconn, query)) != NULL) {
      ExecStatusType status;
      status = PQresultStatus(res);
      if ((status == PGRES_COMMAND_OK) || (status == PGRES_TUPLES_OK)) {
         if (PQntuples(res) > 0)
            return true;
      }
   }

   return false;
}

bool
PostgreSQLDriver::beginTransaction()
{
   PGresult* res;
   if ((res = PQexec(m_pgconn, "BEGIN")) != NULL) {
      ExecStatusType status;
      status = PQresultStatus(res);
      if ((status == PGRES_COMMAND_OK) || (status == PGRES_TUPLES_OK))
         return true;
   }

   return false;
}

bool
PostgreSQLDriver::commitTransaction()
{
   PGresult* res;
   if ((res = PQexec(m_pgconn, "COMMIT")) != NULL) {
      ExecStatusType status;
      status = PQresultStatus(res);
      if ((status == PGRES_COMMAND_OK) || (status == PGRES_TUPLES_OK))
         return true;
   }

   return false;
}

bool
PostgreSQLDriver::rollbackTransaction()
{
   PGresult* res;
   if ((res = PQexec(m_pgconn, "ROLLBACK")) != NULL) {
      ExecStatusType status;
      status = PQresultStatus(res);
      if ((status == PGRES_COMMAND_OK) || (status == PGRES_TUPLES_OK))
         return true;
   }

   return false;
}
