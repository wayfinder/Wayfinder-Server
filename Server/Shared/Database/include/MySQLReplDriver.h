/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MYSQLREPLDRIVER_H
#define MYSQLREPLDRIVER_H

#include "MySQLDriver.h"
#include <vector>

/**
 *  A SQLDriver for MySQL in a replicating setup *
 */
class MySQLReplDriver : public MySQLDriver
{
   public:

      /**
        *  Creates a driver
        *  @param host The hosts to connect to
        *  @param database The database to use
        *  @param user The user to connect as
        *  @param password The password to use
        */
      MySQLReplDriver(const char* host,
                  const char* database,
                  const char* user,
                  const char* password);

      /**
        *  Disposes of the MySQLDriver
        */
      virtual ~MySQLReplDriver();

      /**
        *  Do the actual connect to the database
        *  @return true if successfull
        */
      virtual bool connect();

      /**
        *  Check if the connection to the database is up, should do an
        *  automatic reconnect if its down. Only call this if an 
        *  earlier call to connect() has succeeded.
        *  @return true if its up or the reconnection succeeded, 
        *          false if there's a problem
        */
      virtual bool ping();

      /**
        *  Get the error status
        *  @param result Result handle
        *  @return 0 if OK
        */
      virtual int getError(const void* result);

      /**
        *  Get current error status as a string
        *  @param result Result handle
        *  @return A string describing the current status
        */
      virtual const char* getErrorString(const void* result);

   private:    

      /**
        *  The hosts
        */
      vector<const char*> m_hosts;

      /**
       *   Iterator for the host list
       */
      vector<const char*>::iterator m_hostIter;

};

#endif // MYSQLREPLDRIVER_H
