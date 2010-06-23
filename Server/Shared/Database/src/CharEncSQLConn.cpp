/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CharEncSQLConn.h"
#include "config.h"
#include "CharEncoding.h"


CharEncSQLConn::
CharEncSQLConn( SQLDriver* driver,
                CharEncodingType::charEncodingType dbCharEnc,
                CharEncodingType::charEncodingType resultCharEnc,
                bool dieOnError )
   : SQLConnection( driver )
{
   if ( dbCharEnc != resultCharEnc ){
      m_charEncoder = new CharEncoding( dbCharEnc, 
                                        resultCharEnc, 
                                        dieOnError );

      if ( ( dbCharEnc == CharEncodingType::UTF8_or_Iso8859_1 ) &&
           ( resultCharEnc == CharEncodingType::UTF8 ) ){

         // In this case, always try to insert UTF-8 in the database,
         // therfore no conversion of queries are needed.
         m_queryCharEncoder = NULL;
      }
      else {
         m_queryCharEncoder = new CharEncoding( resultCharEnc,
                                                dbCharEnc, 
                                                dieOnError );
      }
   }
   else {
      // No need to convert between equal character types.
      m_charEncoder = NULL;
      m_queryCharEncoder = NULL;
   }

} // CharEncSQLConn

CharEncSQLConn::~CharEncSQLConn(){
   delete m_charEncoder;
   delete m_queryCharEncoder;
}


CharEncSQLQuery*
CharEncSQLConn::newQuery()
{
   return new CharEncSQLQuery(m_driver, m_charEncoder, m_queryCharEncoder);

} // newQuery
