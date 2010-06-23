/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MySQLDriver.h"
#include "config.h"
#include "StringUtility.h"
#include "SysUtility.h"
#include "Properties.h"
#include <mysql/mysql.h>

template <>
void AutoPtr<MYSQL>::destroy() {
   if ( get() ) {
      mysql_close( get() );
      release();
   }
}

MySQLDriver::MySQLDriver(const char* host,
                         const char* database,
                         const char* user,
                         const char* password)
{
   m_host = host;
   m_database = database;
   m_user = user;
   m_password = password;
   m_masterState = false;
   m_dbState = MYSQL_NOT_CONNECTED;
   m_mysql.reset( mysql_init(NULL) );
   if ( m_mysql.get() == NULL ) {
      mc2log << error << "[MySQLDriver] Internal error, mysql_init() failed!" 
             << endl;
   } else {
      // setup options

      // timeout in seconds
      uint32 timeout = Properties::
         getUint32Property( "SQL_CONNECT_TIMEOUT", 10 );
      
      if ( mysql_options( m_mysql.get(), MYSQL_OPT_CONNECT_TIMEOUT, 
                          (const char*)&timeout ) != 0 ) {
         mc2log << error 
                << "[MySQLDriver] Failed to set connect timeout option."
                << endl;
      }
#if MYSQL_VERSION_ID > 40101
      // set read/write timeout,
      // if the MySQL versino supports it.
      timeout = Properties::getUint32Property( "SQL_READWRITE_TIMEOUT", 5 );
      if ( mysql_options( m_mysql.get(), MYSQL_OPT_WRITE_TIMEOUT, 
                          (const char*)&timeout ) != 0 ) {
         mc2log << error 
                << "[MySQLDriver] Failed to set write timeout option."
                << endl;
      }

      if ( mysql_options( m_mysql.get(), MYSQL_OPT_READ_TIMEOUT, 
                          (const char*)&timeout ) != 0 ) {
         mc2log << error 
                << "[MySQLDriver] Failed to set read timeout option."
                << endl;
      }
#else
      mc2log << warn 
             << "[MySQLDriver] No read and write timeout in this mysql version." 
             << "Version: " << MYSQL_SERVER_VERSION << endl;

#endif

   }
}

bool
MySQLDriver::connect()
{
   if ( mysql_real_connect( m_mysql.get(), 
                            m_host, m_user, m_password, 
                            NULL,            // database
                            0,               // port
                            NULL,            // unix socket
                            CLIENT_IGNORE_SIGPIPE    // client flags
                            ) == NULL) {
      mc2log << error << "[MySQLDriver] Error connecting to host: " 
             << m_host << " Error: " << mysql_error( m_mysql.get() ) << endl;
      m_dbState = MYSQL_NOT_CONNECTED;
      return false;
   }
   if (mysql_select_db(m_mysql.get(),m_database)) {
      mc2log << error << "[MySQLDriver] Error selecting database: "
             << mysql_error( m_mysql.get() ) << " On host: " << m_host << endl;
      m_dbState = MYSQL_CONNECTED_NO_DB;
      return false;
   }

   m_dbState = MYSQL_CONNECTED;
   return true;
}   

bool
MySQLDriver::ping()
{
   SysUtility::IgnorePipe ignorePipe;

   if (mysql_ping( m_mysql.get() ) != 0)
      return connect();
   else
      return true;
   
}

MySQLDriver::~MySQLDriver()
{
}

bool
MySQLDriver::setMaster(bool master)
{
   m_masterState = master; 
   return true;
}

const char*
MySQLDriver::prepare(const char* query)
{
   bool create = strncasecmp( "create table", query, 12 ) == 0;
   if ( create || strncasecmp( "alter table", query, 11 ) == 0 )
   {
      mc2dbg8 << "MySQLDriver::prepare(): this is a create/alter "
              << "table stmt!" << endl;
      char* replaced = StringUtility::replaceString(query, "MC2BLOB", "MEDIUMBLOB");
      char* newquery;
      if (replaced != NULL) {
         newquery = new char[strlen(replaced) + 20];
         strcpy(newquery, replaced);
         delete [] replaced;
      } else {
         newquery = new char[strlen(query) + 20];
         strcpy(newquery, query);
      }

      if ( create ) {
         strcat( newquery, " TYPE=InnoDB" );
      }

      mc2dbg4 << "MySQLDriver::prepare(): newquery: " << newquery << endl;
      return(newquery);
   } else
   return query; 
}

void
MySQLDriver::freePrepared(const char* prepQuery)
{
   delete [] prepQuery;
}

const void*
MySQLDriver::execute(const char* query)
{
   int error;

   if ((error = mysql_query(m_mysql.get(), query))) {
      return NULL;
   }
  
   return mysql_use_result(m_mysql.get());
}

void
MySQLDriver::freeResult(const void* result)
{
   mysql_free_result((MYSQL_RES*)result);
}

uint32
MySQLDriver::getNumColumns(const void* result)
{
   MC2_ASSERT( result != NULL );
   return mysql_num_fields((MYSQL_RES*)result);
   
}

const void*
MySQLDriver::nextRow(const void* result)
{
   return mysql_fetch_row((MYSQL_RES*)result);
}

void
MySQLDriver::freeRow(const void* row)
{
   // NOP for MySQL
}

void
MySQLDriver::getColumnNames( const void* result,
                             vector< MC2String >& colNames )
{
   MYSQL_FIELD* field;
   while ( (field = mysql_fetch_field( ( MYSQL_RES* )result )) ) {
         colNames.push_back( field->name );
   }
   // Seek to the beginning of the row to reset the field cursor
   mysql_field_seek( ( MYSQL_RES* )result, 0 );
}


const char*
MySQLDriver::getColumn(const void* result, const void* row, int colIndex)
{
   const char* col;
   if (NULL == (col = ((MYSQL_ROW)row)[colIndex]))
      col =  m_nullString;
   return col;
}
      
int
MySQLDriver::getError(const void* result)
{
   // return the connection's error status...
   return mysql_errno(m_mysql.get());
}

const char*
MySQLDriver::getErrorString(const void* result)
{
   // return the connection's error string...
   return mysql_error(m_mysql.get());
}

bool
MySQLDriver::tableExists(const char* tableName)
{
   char query[1024];
   MYSQL_RES* res;

   // \todo
   // to gain speed; read table list when you connect and put
   // them in a hash_map (problems when creating new ones though)
   sprintf(query, "SHOW TABLES FROM %s LIKE '%s'", m_database, tableName);
   if (mysql_query(m_mysql.get(), query) == 0) {
      res = mysql_store_result(m_mysql.get());
      int i = mysql_num_rows(res);
      mysql_free_result(res);
      return (i > 0);
   } else
      return false;
}

bool
MySQLDriver::beginTransaction()
{
   return (mysql_query(m_mysql.get(), "BEGIN") == 0);
}

bool
MySQLDriver::commitTransaction()
{
   return (mysql_query(m_mysql.get(), "COMMIT") == 0);
}

bool
MySQLDriver::rollbackTransaction()
{
   return (mysql_query(m_mysql.get(), "ROLLBACK") == 0);
}

const char*
MySQLDriver::m_nullString = "";
