/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SQLQUERYHANDLER_H 
#define SQLQUERYHANDLER_H

#include "config.h"
#include "SQLDataContainer.h"
#include "SQLTableData.h"

// Forward declaration
class CharEncSQLQuery;
class RequestPacket; 
class ReplyPacket; 

// Using namespace so we dont brake old code that dont use this since we moved
// the function from UserProcessor to here.
namespace DoQuerySpace {
   /**
   * Do a SQL query
   *
   * @param sqlQuery Pointer to the CharEncSQLQuery to use
   * @param query The query string
   * @param whereTag Set this to a constant string which identifies
   *        the callee
   * @return True if successful, false otherwise
   */
   bool doQuery(SQLQuery* sqlQuery, const char* query, const char* whereTag);
}

/**
* A class that constructs the sql querys needed for inserting and getting data
* from a specific sql data table. And sets the correct reply to the reply
* packet.
*
* Example:
*
* First have the SQLDataTable created, here m_exampleTableData 
* is a auto_ptr. The sql table has userUIN as a primary key and is
* numeric, a name value which is not a primary key, and a string.
* And last a phoneNbr which is not a primary key, and numericly stored
* in the database.
* \code
* m_exampleTableData.reset( new SQLDataTable("ExampleData") );
* m_exampleTableData->setColumnName(
*      SQLDataTable::TableDataColumn("userUIN", true, true ) );
* m_exampleTableData->setColumnName(
*      SQLDataTable::TableDataColumn("name", false, false ) );
* m_exampleTableData->setColumnName(
*      SQLDataTable::TableDataColumn("phoneNbr", true, false ) );
* \endcode
* This data will be used to determine how to build up the sql query's 
* in the handler.
* 
* To get stored data example, where the request packet might contain
* a request for a specific phoneNbr from a specific user.
* \code
*  // p is the request packet
*  int reqPos = ... the request header size of the packet ...;
*  int replyPos = .. the reply header size of the packet ...;
*  auto_ptr< CharEncSQLQuery >  sqlQuery( m_sqlConnection->newQuery() );
*  auto_ptr< ... packet reply type ... > 
*     reply( new ... packet reply type ...( &p ) );
*  const MC2String whereTag( "UP::... the handle function name ..." );
*  SQLDataContainer cont;
*  SQLQueryHandler queryHandler;
*  queryHandler.fetchData( sqlQuery.get(), whereTag, p, *reply, reqPos,
*                           replyPos, *m_exampleTableData, cont );
*   return reply.release();
* \endcode
* fetchData will store the requested data in the reply packet at replyPos.
*
* To insert/update data in a table:
* \code
*  // p is the request packet
*  int reqPos = ... the request header size of the packet ...;
*  auto_ptr< CharEncSQLQuery >  sqlQuery( m_sqlConnection->newQuery() );
*  auto_ptr< ... packet reply type ... > 
*     reply( new ... packet reply type ...( &p ) );
*  const MC2String whereTag( "UP::... the handle function name ..." );
*  SQLDataContainer cont;
*  SQLQueryHandler queryHandler;
*  queryHandler.insertData( sqlQuery.get(), whereTag, p, *reply, reqPos,
*                           *m_exampleTableData, cont );
*   return reply.release();
* \endcode
* insertData will first try to insert the data suplied by the request
* packet, if it fails it will then try and update the table instead.
* If that also fails it will set the status in the reply packet to
* StringTable::NOTOK, else it will be set to StringTable::OK.
*/
class SQLQueryHandler {

public:

   /** Creates a new SQLQueryHandler */
   SQLQueryHandler();

   ~SQLQueryHandler();

   /**
    * Insert data into the database.
    *
    * @param sqlQuery The sql query connection
    * @param whereTag Debug message
    * @param reqPacket The request packet
    * @param replyPacket The reply packet
    * @param reqPos The request header size of the request packet
    * @param table The table data of the databse
    * @param cont The sql data container
    */
   static void insertData( CharEncSQLQuery* sqlQuery, const MC2String& whereTag,
                           const RequestPacket& reqPacket, ReplyPacket& replyPacket,
                           int reqPos, SQLTableData& table,
                           SQLDataContainer& cont );

   /**
    * Fetches data from the database.
    *
    * @param sqlQuery The sql query connection
    * @param whereTag Debug message
    * @param reqPacket The request packet
    * @param replyPacket the reply packet
    * @param reqPos The request header size of the request packet
    * @param replyPos The reply header size of the reply packet
    * @param table The table data of the databse
    * @param cont The sql data container
    */
   static void fetchData( CharEncSQLQuery* sqlQuery, const MC2String& whereTag,
                          const RequestPacket& reqPacket, ReplyPacket& replyPacket, 
                          int reqPos, int replyPos, SQLTableData& table, 
                          SQLDataContainer& cont );

   /**
    * Fetches data from the database.
    *
    * @param sqlQuery The sql query connection.
    * @param whereTag Debug message.
    * @param reqCont The container with the column(s) to select with.
    * @param table The table data of the databse
    * @param cont The sql data container, is filled with the result.
    * @return True if select went ok, false if not.
    */
   static bool fetchData( CharEncSQLQuery* sqlQuery, const MC2String& whereTag,
                          const SQLDataContainer& reqCont, SQLTableData& table,
                          SQLDataContainer& cont );

private:


};

#endif // SQLQUERYHANDLER_H
