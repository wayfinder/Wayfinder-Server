/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CharEncSQLQuery.h"

CharEncSQLQuery::
CharEncSQLQuery( SQLDriver* driver,
                 const CharEncoding* charEncoder,
                 const CharEncoding* queryCharEncoder ) : SQLQuery(driver)
{
   m_charEncoder = charEncoder;
   m_queryCharEncoder = queryCharEncoder;

} // CharEncSQLQuery


MC2String
CharEncSQLQuery::getEncColumn(int colIndex)
{
   MC2String rawValue( m_driver->getColumn(m_result, m_row, colIndex) );
   if ( m_charEncoder != NULL ){
      MC2String result;
      m_charEncoder->convert( rawValue, result );
      return result;
   }
   else {
      // When there are no char encoding object we should not need to 
      // convert
      return rawValue;
   }

} // getEncColumn

const char*
CharEncSQLQuery::getColumn(int colIndex)
{
   if ( m_charEncoder != NULL ){
      MC2String rawValue( m_driver->getColumn(m_result, m_row, colIndex) );
      MC2String converted;
      m_charEncoder->convert( rawValue, converted );
      
      // Debug code
      if ( m_charEncoder->getFromType() == 
           CharEncodingType::UTF8_or_Iso8859_1 ){
         if ( rawValue != converted ){
            mc2dbg8 << "QQQ: " << m_prepQuery << endl;
         }
      }

      pair< set<MC2String>::const_iterator, bool> result = 
         m_resultStrings.insert( converted );
      return (result.first)->c_str();
   }
   else {
      return  m_driver->getColumn(m_result, m_row, colIndex);
   }

} // getColumn


bool
CharEncSQLQuery::prepare(const char* query)
{
   // The result is no longer valid.
   m_resultStrings.clear();

   MC2String encodedQuery;
   if ( m_queryCharEncoder != NULL ){
      m_queryCharEncoder->convert(MC2String(query), encodedQuery);
   }
   else {
      encodedQuery = query;
   }

   return SQLQuery::prepare(encodedQuery.c_str());

} // execute
