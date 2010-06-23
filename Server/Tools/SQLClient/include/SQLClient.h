/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SQLClient_H
#define SQLClient_H

#include "config.h"
#include "SQLDriver.h"
#include "SQLConnection.h"

/** 
 *    Small SQL client that mimics 'mysql', 'psql' for testing
 *    of the SQL interface in mc2.
 *
 */
class SQLClient {
   public:
      /**
       *    Constructor that sets up the initial state
       *    @param drivername  The SQL driver to use, one of
       *                   {mysql,postgresql,oracle}
       *    @param database  The database to use
       *    @param host  Host where MapCentServer runs
       *    @param port  The port number on the host
       *    @param username  The username
       *    @param password  The password
       *          
       */
      SQLClient(char* drivername,
                char* database,
                char* host, 
                uint16 port,
                char* username,
                char* password);

      /**
       *    Frees allocated members.
       */
      virtual ~SQLClient();

      /**
       *    Creates the driver, connection, etc
       */
      void init();

      /**
       *    Process a SQL command, with error handling and display of
       *    result
       *    @param query The query string
       */
      void doSQL(const char* query);

      /**
       *    Main method of the class, does the user interaction
       *    and processing
       */
      void doClient();


   private:

      /**
       *    The driver name
       */
      const char* m_drivername;

      /**
       *    The database
       */
      const char* m_database;

      /**
       *    The server host name
       */
      const char* m_host;

      /**   The server port
       *
       */
      uint16 m_port;

      /**
       *    The username
       */
      const char* m_username;

      /**
       *    The password
       */
      const char* m_password;

      /**
       *    Are we connected?
       */
      bool m_connected;

      SQLDriver *m_driver;

      SQLConnection *m_connection;

};

#endif
