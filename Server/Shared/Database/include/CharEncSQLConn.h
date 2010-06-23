/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CHARENCSQLCONN_H
#define CHARENCSQLCONN_H


#include "SQLConnection.h"
#include "CharEncSQLQuery.h"
#include "CharEncodingType.h"

class CharEncoding;

/**
 *  Encapsulates a connection to a POI SQL database backend. Creates 
 *  CharEncSQLQuery, which converts query and result texts between the
 *  character encoding systems supplied in the constructor.
 *
 */
class CharEncSQLConn : public SQLConnection
{
   public:

   /**
    * @param driver        The dirver to use whne connecing to the 
    *                      database. 
    * @param dbCharEnc     The character encoding system of the database.
    * @param resultCharEnc The character encodign system the texts in the 
    *                      database are wanted in.
    * @param dieOnError    Makes the code exit when there is an error in
    *                      character conversion.
    */
   CharEncSQLConn( SQLDriver* driver,
                   CharEncodingType::charEncodingType dbCharEnc,
                   CharEncodingType::charEncodingType resultCharEnc,
                   bool dieOnError = false );

   /**
    * Virtual destructor.
    */
   virtual ~CharEncSQLConn();


   /**
    *  Create a new query with right character encoding handling.
    *  @return Pointer to a new SQLQuery
    */
   virtual CharEncSQLQuery* newQuery();



   protected:
   
   /**
    * The CharEncoding object used for converting strings from the database
    * to the wanted character encodng.
    */
   CharEncoding* m_charEncoder;

   /**
    * The CharEncoding object used for converting queries before sending 
    * them to the database.
    */
   CharEncoding* m_queryCharEncoder;
   
};

#endif // CHARENCSQLCONN_H
