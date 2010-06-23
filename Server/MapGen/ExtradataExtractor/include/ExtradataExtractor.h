/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXTRADATAEXTRACTOR_H
#define EXTRADATAEXTRACTOR_H

#include "config.h"
#include "SQLDriver.h"
#include "SQLConnection.h"
#include "CommandlineOptionHandler.h"

/**
 */
class ExtradataExtractor {
   public:
      
          /**
       *    Create a new ExtradataExtractor that will connect to the database 
       *    that is specified as parameters. The constructor will make
       *    the necessary connections to the database, these are used
       *    by extractExtradata and extractLatestExtradata for extracting 
       *    extradatarecords within given
       *    criterias and print them to output.
       *    @param database      The name of the database to use.
       *    @param host          The host where to connect to MySQL.
       *    @param username      Username when connecting to the database.
       *    @param password      Password when connection to the database.
       *    
       */
      ExtradataExtractor(const char* database,
                    const char* host,
                    const char* username,
                    const char* password);

      /**
       *    Delete the ExtradataExtractor and close the connections to the database.
       */
      virtual ~ExtradataExtractor();
      
      /**
       * Extract extradatarecords from the database and print them to a file.
       */
       bool extractExtradata(char* insertType, char* country, 
                             uint32 countryID, char* version,
                             const char* insertDate);
      
        private:
      /**
       *    Copyed from UserProcessor!
       */
      bool doQuery(SQLQuery* sqlQuery, const char* query, const char* whereTag);
     
      /**
       *    Are we connected?
       */
      bool m_connected;

      /**
       *    The main connection to the database.
       */
      SQLConnection* m_sqlConnection;
      /**
       *    An extra connection to be able to execute sub-selects when
       *    the m_sqlConnection still is used.
       */
      SQLConnection* m_subSqlConnection;

};

#endif
