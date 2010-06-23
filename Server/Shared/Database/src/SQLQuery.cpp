/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SQLQuery.h"
#include "StringUtility.h"

#include "MC2String.h"

SQLQuery::SQLQuery(SQLDriver* driver)
{
   m_driver = driver;   
   m_result = NULL;
   m_row = NULL;
   m_prepQuery = NULL;
   m_rawQuery = NULL;
}

SQLQuery::~SQLQuery()
{
   if (m_prepQuery != m_rawQuery)
      m_driver->freePrepared(m_prepQuery);

   delete[] m_rawQuery;

   if (m_result != NULL)
      m_driver->freeResult(m_result);
}

bool
SQLQuery::prepare(const char* query)
{
   if (m_prepQuery != m_rawQuery)
      m_driver->freePrepared(m_prepQuery);

   delete[] m_rawQuery;

   if (m_result != NULL)
      m_driver->freeResult(m_result);

   if (m_row != NULL)
      m_driver->freeRow(m_row);

   m_rawQuery = StringUtility::newStrDup(query);
   m_prepQuery = m_driver->prepare(m_rawQuery);
   m_result = NULL;
   return (m_prepQuery != NULL);
}

bool
SQLQuery::execute()
{
   m_row = NULL;
   m_result = m_driver->execute(m_prepQuery); 
   if (m_result != NULL) {
      if (m_driver->getError(m_result) == 0)
         return true;
   }

   return false;
}

bool
SQLQuery::prepAndExec(const char* query)
{
   if (prepare(query))
      return execute();
   else
      return false;
}

bool
SQLQuery::prepAndExec(const MC2String& query)
{
   return prepAndExec( query.c_str() );
}

/*uint32
SQLQuery::getNumRows()
{
   return m_driver->getNumRows(m_result); 
} */

uint32
SQLQuery::getNumColumns()
{
   return m_driver->getNumColumns(m_result); 
}

bool
SQLQuery::nextRow()
{
   if (m_row != NULL) {
      m_driver->freeRow(m_row);
   }
   return ((m_row = m_driver->nextRow(m_result)) != NULL);   
}

const char*
SQLQuery::getColumn(int colIndex)
{
   // The getColumn method is overridden in sub class CharEncSQLQuery.
   return getRawColumn(colIndex);
}

void
SQLQuery::getColumnNames( vector< MC2String >& colNames ) {
   m_driver->getColumnNames( m_result, colNames );
}

const char*
SQLQuery::getRawColumn(int colIndex)
{
   MC2_ASSERT((uint32)colIndex < m_driver->getNumColumns(m_result));
   return m_driver->getColumn(m_result, m_row, colIndex);   
}
      
int
SQLQuery::getError()
{
   return m_driver->getError(m_result); 
}

const char*
SQLQuery::getErrorString()
{
   return m_driver->getErrorString(m_result); 
}

