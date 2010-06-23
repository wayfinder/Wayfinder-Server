/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SQLConnection.h"

SQLConnection::SQLConnection(SQLDriver* driver)
{
   m_driver = driver;   
}

SQLConnection::~SQLConnection()
{
   mc2dbg8 << "[SQLConnection] Shutting down" << endl;
   delete m_driver;
}

bool
SQLConnection::connect()
{
   return m_driver->connect();
}

bool
SQLConnection::ping()
{
   return m_driver->ping();
}

bool
SQLConnection::tableExists(const char* tableName)
{
   return m_driver->tableExists(tableName);
}

bool
SQLConnection::tableExists(const MC2String& tableName)
{
   return m_driver->tableExists(tableName.c_str());
}

bool
SQLConnection::beginTransaction()
{
   return m_driver->beginTransaction();
}

bool
SQLConnection::commitTransaction()
{
   return m_driver->commitTransaction();
}

bool
SQLConnection::rollbackTransaction()
{
   return m_driver->rollbackTransaction();
}

SQLQuery*
SQLConnection::newQuery()
{
   return new SQLQuery(m_driver); 
}

int
SQLConnection::getError()
{
   return m_driver->getError(NULL); 
}

const char*
SQLConnection::getErrorString()
{
   return m_driver->getErrorString(NULL); 
}


