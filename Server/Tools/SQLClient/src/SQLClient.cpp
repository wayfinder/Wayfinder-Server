/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SQLClient.h"
#include "CommandlineOptionHandler.h"
#include "Readline.h"

#include "config.h"
#include "MySQLDriver.h"
#include "MySQLReplDriver.h"
#include "PostgreSQLDriver.h"
#include "OracleSQLDriver.h"

SQLClient::SQLClient(char* driver,
                     char* database,
                     char* host, 
                     uint16 port,
                     char* username,
                     char* password)
{
   m_drivername = driver;
   m_database = database;
   m_host = host;
   m_port = port;
   m_username = username;
   m_password = password;
   init();
}

SQLClient::~SQLClient()
{
   delete m_connection;
}

void SQLClient::init()
{ 

   mc2dbg4 << "SQLClient::init() top" << endl;
   if (strcmp(m_drivername, "mysql") == 0) {
      m_driver = new MySQLDriver(m_host, m_database, m_username, m_password);
   } else if (strcmp(m_drivername, "mysqlrepl") == 0) {
      m_driver = new MySQLReplDriver(m_host, m_database, m_username, m_password);
   } else if(strcmp(m_drivername, "postgresql") == 0) {
      m_driver = new PostgreSQLDriver(m_host, m_database, m_username, m_password);
#ifdef USE_ORACLE
   } else if(strcmp(m_drivername, "oracle") == 0) {
      m_driver = new OracleSQLDriver(m_host, m_database, m_username, m_password);
#endif
   } else {
      MC2ERROR("Unknown database driver!");
      return;
   }
   mc2dbg4 << "SQLClient::init() driver created" << endl;

   m_connection = new SQLConnection(m_driver);
   mc2dbg4 << "SQLClient::init() connection created" << endl;
   m_connected = m_connection->connect();
   mc2dbg4 << "SQLClient::init() connected" << endl;
}

void SQLClient::doSQL(const char* query)
{ 
   SQLQuery* theQuery;

   theQuery = m_connection->newQuery();
/*   theQuery->prepare(query);
   if (theQuery->execute()) { */
   if (theQuery->prepAndExec(query)) {
      while (theQuery->nextRow()) {
         for (uint32 i = 0; i<theQuery->getNumColumns(); i++) {
            cout << theQuery->getColumn(i) << "\t";
         }
         cout << endl;
      }
      if (theQuery->getError() > 0) {
         cout << "Query failed, error: " << theQuery->getErrorString() << endl;
      }
   } else {
      if (theQuery->getError() > 0)
         cout << "Query failed, error: " << theQuery->getErrorString() << endl;
      else
         cout << "Query OK (0 rows returned)" << endl; // ???
   }

   delete theQuery;
}

void SQLClient::doClient()
{
   const char* input;
   bool quit = false;
   char query[4096];

   mc2dbg4 << "SQLClient::doClient() top" << endl;
   if (!m_connected) {
      cout << "Connection failed, exiting." << endl;
      return;
   }
   query[0] = '\0';
   Readline* readl = new Readline("> ", true, NULL);
   while (!quit && ((input = readl->getInput()) != NULL)) {
      mc2dbg8 << "SQLClient::doClient() user input: " << input << endl;

      // is it a command for me?
      if ('\\' == input[0]) {
         if ('h' == input[1]) {
            cout << "SQLClient help" << endl << endl;
            cout << "\\h - This help" << endl;
            cout << "\\q - Quit" << endl;
            cout << "\\b - Begin transaction" << endl;
            cout << "\\c - Commit transaction" << endl;
            cout << "\\r - Rollback transaction" << endl;
            cout << "\\t - Check if table exists (eg '\\tfoo')" << endl;
         } else if ('q' == input[1]) {
            quit = true;
         } else if ('b' == input[1]) {
            cout << "Begin Transaction: ";
            m_connection->beginTransaction() ? cout << "OK" : cout << "err";
            cout << endl;
         } else if ('c' == input[1]) {
            cout << "Commit Transaction: ";
            m_connection->commitTransaction() ? cout << "OK" : cout << "err";
            cout << endl;
         } else if ('r' == input[1]) {
            cout << "Rollback Transaction: ";
            m_connection->rollbackTransaction() ? cout << "OK" : cout << "err";
            cout << endl;
         } else if ('t' == input[1]) {
            cout << "Table " << &(input[2]) << " ";
            m_connection->tableExists(&input[2]) ? cout << "exists" :
                                                   cout << "doesn't exist";
            cout << endl;
         }
      } else {
         // no, it's a SQL command
         // add to query buffer
         if (query[0] != '\0')
            strcat(query, " ");
         strcat(query, input);
         // command terminated?
         if (strchr(input, ';') != NULL) {
            *(strchr(query, ';')) = '\0';
            doSQL(query);
            query[0] = '\0';
         }
      }
   }
   delete readl;
}

// ====================================================================
//                                                             main() =
int main(int argc, char *argv[]) {
   uint32 port = 0;
   char* host = new char[512];
   char* username = new char[512];
   char* password = new char[512];
   char* driver = new char[512];
   char* database = new char[512];

   cout << "SQLClient starting up" << endl << endl;
   host[0] = '\0';
   CommandlineOptionHandler coh(argc, argv);
   coh.setTailHelp("");
   coh.setSummary("A few example commands:\nSQLClient -d mysql -u ISABUser -w secret -b isabdb -s localhost\nSQLClient -d oracle -u user1 -w pw1 -s server1");

   coh.addOption("-s", "--host",
              CommandlineOptionHandler::stringVal,
              1, &host, "",
              "Set the hostname to connect to");

   coh.addOption("-b", "--database",
              CommandlineOptionHandler::stringVal,
              1, &database, "",
              "Set the database (if applicable)");

   coh.addOption("-u", "--username",
              CommandlineOptionHandler::stringVal,
              1, &username, "",
              "Set the user name");

   coh.addOption("-w", "--password",
              CommandlineOptionHandler::stringVal,
              1, &password, "",
              "Set the password");

   coh.addOption("-d", "--driver",
              CommandlineOptionHandler::stringVal,
              1, &driver, "",
              "The SQL driver to use");

   if(!coh.parse()) {
      cout << "SQLClient: Error on commandline! (-h for help)" << endl;
      exit(1);
   }

   if ( host == NULL ) {
      cout << "You have to supply a host name!" << endl;
      exit(-1);
   }

   if ( username == NULL ) {
      cout << "You have to supply a user name!" << endl;
      exit(-1);
   }

   if ( password == NULL ) {
      cout << "You have to supply a password!" << endl;
      exit(-1);
   }

   if ( driver == NULL ) {
      cout << "You have to choose a db driver!" << endl;
      cout << "   mysql      MySQLDriver" << endl;
      cout << "   mysqlrepl  MySQLReplDriver" << endl;
      cout << "   postgresql PostgreSQLDriver" << endl;
#ifdef USE_ORACLE
      cout << "   oracle     OracleSQLDriver" << endl;
#endif
      exit(-1);
   }

   SQLClient* theClient = new SQLClient(driver, database, host, port, username, password);

   theClient->doClient();

   delete theClient;
   mc2log << info << "[SQLClient] shutting down, exit code 0" << endl;

   exit(0);
}



