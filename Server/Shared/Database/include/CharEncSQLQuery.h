/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CHARENCSQLQUERY_H
#define CHARENCSQLQUERY_H

#include "SQLQuery.h"

#include "CharEncoding.h"
#include <set>


/**
 *  Class used for executing queries to a database. This class is only
 *  instansiated with CharEncSQLConn::newQuery.
 *
 *  Converts query and result texts between the character encoding systems
 *  supplied converted between by the CharEncoding objects supplied in the
 *  constructor.
 *
 */
class CharEncSQLQuery : public SQLQuery
{
   public:
   // Gives access to the constructor.
   friend class CharEncSQLConn;

   /**
    *  Get a column from the current row. The result is converted using the
    *  charEncoder set in the constructor.
    *
    *  @param colIndex The column to get the value for.
    *  @return Pointer to column value
    */
   MC2String getEncColumn(int colIndex);

   /**
    *  Get a column from the current row
    *  @param colIndex Zero based index of the column to get the value of.
    *
    *  @return Pointer to column value
    */
   const char* getColumn(int colIndex);

   /**
    * Initiate the query with the query text and prepare it for use. Call
    * execute to make it possible to get the result.
    *
    * When this method is called, any old result strings are deallocated.
    *
    * @param query The query text.
    */
   bool prepare(const char* query);
   

   protected:
  
   /**
    * @param driver      The dirver to use whne connecing to the database.
    * @param charEncoder The char encoder is used for converting texts
    *                    from the database before they are returned.
    * @param queryCharEncoder The encoder to use when converting queries
    *                         before sending them to the database.
    */
   CharEncSQLQuery( SQLDriver* driver,
                    const CharEncoding* charEncoder,
                    const CharEncoding* queryCharEncoder  );


   /**
    * Char encoding object used for converting text from the database.
    *
    * Can be NULL if the constructing connection did not find any reason
    * for conversion.
    */
   const CharEncoding* m_charEncoder;

   /**
    * Char encoding object used for converting queries before sending them
    * to the database.
    *
    * Can be NULL if the constructing connection did not find any reason
    * for conversion.
    */
   const CharEncoding* m_queryCharEncoder;

   /**
    * Allocates result stings so they can be used as long as this query
    * is up to date.
    */
   set<MC2String> m_resultStrings;

};

#endif  // CHARENCSQLQUERY_H
