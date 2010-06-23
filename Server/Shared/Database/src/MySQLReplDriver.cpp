/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MySQLReplDriver.h"
#include "StringUtility.h"

MySQLReplDriver::MySQLReplDriver(const char* host,
                  const char* database,
                  const char* user,
                  const char* password) :
   MySQLDriver(host, database, user, password)
{
   // split m_host into m_hosts
   char* hostp;
   char* hostCopy = StringUtility::newStrDup(m_host); // is const char
   char* hostList = hostCopy;
   while ((hostp = strchr(hostList, ','))) {
      mc2dbg8 << "[MySQLReplDriver] found ',' points at: " << hostList << endl;
      hostp[0] = '\0';
      mc2dbg8 << "[MySQLReplDriver] fixup, points at: " << hostList << endl;
      m_hosts.push_back(StringUtility::newStrDup(hostList));
      hostList = hostp + 1;
   }
   m_hosts.push_back(StringUtility::newStrDup(hostList));
   mc2dbg4 << "[MySQLReplDriver] " << m_hosts.size() << " DB hosts" << endl;
   if (m_hosts.size() == 1) {
      mc2log << warn << "[MySQLReplDriver] Only one DB host specified! "
                "Continuing anyway." << endl;
   }
   delete [] hostCopy;
   m_hostIter = m_hosts.begin();
   m_host = *m_hostIter;
}

MySQLReplDriver::~MySQLReplDriver()
{
   mc2dbg8 << "[MySQLReplDriver] Shutting down" << endl;
   for (m_hostIter = m_hosts.begin(); m_hostIter != m_hosts.end(); m_hostIter++)
      delete *m_hostIter;

   m_hosts.erase(m_hosts.begin(), m_hosts.end());
}

bool
MySQLReplDriver::connect()
{
   // try to connect to the current host first, then try the others,
   // do any wrap-around
   vector<const char*>::iterator start = m_hostIter;
   bool allFailed = false;
   mc2dbg8 << "[MySQLReplDriver] connecting, starting with: " << m_host << endl;
   while (!MySQLDriver::connect() && !allFailed) {
      ++m_hostIter;
      if (m_hostIter == m_hosts.end())
         m_hostIter = m_hosts.begin();
      if (m_hostIter == start) {
         allFailed = true;
      } else {
         m_host = *m_hostIter;
         mc2dbg8 << "[MySQLReplDriver] connect failed, trying next: "
                 << m_host << endl;
      }
   }
   if (allFailed) {
      mc2log << error << "[MySQLReplDriver] Couldn't connect to any of the hosts! "
             << endl;
      return false;
   }
   return true;
}

bool
MySQLReplDriver::ping()
{
   if(MySQLDriver::ping()) {
      return true;
   } else {
      // for now: if ping() fails, do a panic!!!
      mc2log << fatal << "[MySQLReplDriver] Server gone!" << endl;
      PANIC("[MySQLReplDriver] Emergency abort!", "");
      return false;
   }
}

int
MySQLReplDriver::getError(const void* result)
{
   // we probably want to do driver specific checks here later
   return MySQLDriver::getError(result);
}

const char*
MySQLReplDriver::getErrorString(const void* result)
{
   // we probably want to do driver specific checks here later
   return MySQLDriver::getErrorString(result);
}

