/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SQLQueryHandler.h"
#include "Packet.h"
#include "CharEncSQLConn.h"
#include "StringUtility.h"
#include "StringTable.h"

using namespace DoQuerySpace;

SQLQueryHandler::SQLQueryHandler()
{
}

SQLQueryHandler::~SQLQueryHandler()
{
}

void
SQLQueryHandler::insertData( CharEncSQLQuery* sqlQuery, 
                             const MC2String& whereTag, 
                             const RequestPacket& reqPacket, 
                             ReplyPacket& replyPacket, int reqPos,
                             SQLTableData& table,
                             SQLDataContainer& cont )
{
   MC2String query( "INSERT INTO " );
   query += StringUtility::SQLEscapeSecure( table.getTableName() );
   query += " ";
   cont.load( &reqPacket, reqPos );
   cont.appendValues( query );

   if ( !doQuery( sqlQuery, query.c_str(), whereTag.c_str() ) ) {
      // Insert failed, will do an update instead..
      query = "UPDATE ";
      query += StringUtility::SQLEscapeSecure( table.getTableName() );
      query += " SET ";
      MC2String setPart;
      MC2String wherePart;
      const SQLTableData::tableData_t& tableData = table.getTableData();
      SQLTableData::tableData_t::const_iterator cit = tableData.begin(); 
      for ( ; cit != tableData.end(); ++cit ) {
         if ( !cit->m_isKey ) {
            MC2String s;
            // Check if its in the container
            if ( cont.getCol( 0, cit->m_name, s ) ) {
               if ( !setPart.empty() ) {
                  setPart += ", ";
               }
               setPart += StringUtility::SQLEscapeSecure( cit->m_name );
               setPart += " = ";
               if ( cit->m_isNumeric ) {
                  setPart += StringUtility::SQLEscapeSecure( s );
               } else {
                  setPart += "'";
                  setPart += StringUtility::SQLEscapeSecure( s );
                  setPart += "'";
               }
            }
         } else {
            MC2String s;
            // Check if its in the container
            if ( cont.getCol( 0, cit->m_name, s ) ) {
               if ( !wherePart.empty() ) {
                  wherePart += " AND ";
               }
               wherePart += StringUtility::SQLEscapeSecure( cit->m_name );
               wherePart += " = ";
               if ( cit->m_isNumeric ) {
                  wherePart += StringUtility::SQLEscapeSecure( s );
               } else {
                  wherePart += "'";
                  wherePart += StringUtility::SQLEscapeSecure( s );
                  wherePart += "'";
               }
            }
         }
      }
      query += setPart;
      query += " WHERE ";
      query += wherePart;

      if ( !doQuery( sqlQuery, query.c_str(), whereTag.c_str() ) ) {
         // Update failed as well
         mc2log << error << whereTag
            << "Failed to set value, returning error msg in packet." << endl;
         replyPacket.setStatus( StringTable::NOTOK );
      } else {
         // we succeded with the update!
         replyPacket.setStatus( StringTable::OK );
      }
   } else {
      // we succeded with the insert!
      replyPacket.setStatus( StringTable::OK );
   }
}

void
SQLQueryHandler::fetchData( CharEncSQLQuery* sqlQuery, 
                            const MC2String& whereTag, 
                            const RequestPacket& reqPacket,
                            ReplyPacket& replyPacket, int reqPos, int replyPos,
                            SQLTableData& table,
                            SQLDataContainer& cont )
{
   cont.load( &reqPacket, reqPos );

   if ( !fetchData( sqlQuery, whereTag, cont, table, cont ) ) {
      // Select failed..
      mc2log << error << whereTag 
             << "Failed to get value, returning error msg in packet." << endl;
      replyPacket.setStatus( StringTable::NOTOK );
   } else {
      cont.save( &replyPacket, replyPos );
   }
}

bool
SQLQueryHandler::fetchData(
   CharEncSQLQuery* sqlQuery, const MC2String& whereTag,
   const SQLDataContainer& reqCont, SQLTableData& table,
   SQLDataContainer& cont )
{
   MC2String query( "SELECT " );
   const SQLTableData::tableData_t& tableData = table.getTableData();

   // For the future change so we dont ask for unneded data from the database
   SQLTableData::tableData_t::const_iterator cit = tableData.begin(); 
   for ( ; cit != tableData.end(); ++cit ) {
      if ( !( cit == tableData.begin() ) ) {
         query += ", ";
      }
      query += StringUtility::SQLEscapeSecure( cit->m_name );
   }
   query += " FROM ";
   query += StringUtility::SQLEscapeSecure( table.getTableName() );
   query += " WHERE ";
   reqCont.appendRowCol( query );

   bool ok = doQuery( sqlQuery, query.c_str(), whereTag.c_str() );
   if ( ok ) {
      cont.getSQLData( sqlQuery );
   }

   return ok;
}

namespace DoQuerySpace {
bool
doQuery(SQLQuery* sqlQuery, const char* query, const char* whereTag)
{
   mc2dbg8 << "UP::doQuery(), query: " << query << ", tag: "
           << whereTag << endl;
   if (query != NULL) {
      if (! sqlQuery->prepare(query) ) {
         mc2log  << error << "Problem preparing query at " << whereTag << ": "
                 << sqlQuery->getErrorString() << endl;
         mc2dbg4 << "Failed query: " << query << endl;
         return false;
      }
   }

   if (! sqlQuery->execute() && sqlQuery->getError() > 0) {
      mc2log << error << "Problem executing query at " << whereTag << ": "
             << sqlQuery->getErrorString() << endl;
      mc2log << error << "Failed query: " << query << endl;
      return false;
   }

   return true;
}

}
