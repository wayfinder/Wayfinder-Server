/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SyslogLogHandler.h"
#include "StringUtility.h"

SyslogLogHandler::SyslogLogHandler(const char* ident, int facility)
{
   m_ident = StringUtility::newStrDup(ident);
   m_facility = facility;

   openlog(m_ident, LOG_NDELAY && LOG_PID, m_facility);
}

SyslogLogHandler::~SyslogLogHandler()
{
   closelog();
   delete [] m_ident;
}

void
SyslogLogHandler::handleMessage(int level, const char* msg, int msgLen,
                                const char* levelStr, const char* timeStamp)
{
   int prio = 0;
   char *syslogMsg;

   syslogMsg = new char[msgLen + 64];
   syslogMsg[0] = '\0';
   strcat(syslogMsg, levelStr);
   strcat(syslogMsg, msg);

   syslog(prio, syslogMsg);

   delete[] syslogMsg;
}
