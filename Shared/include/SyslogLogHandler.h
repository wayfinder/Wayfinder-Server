/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SYSLOGLOGHANDLER_H
#define SYSLOGLOGHANDLER_H

#include "MC2Logging.h"
#include "NotCopyable.h"

#include <syslog.h>

/**
 *   LogHandler that logs to syslog
 *
 */
class SyslogLogHandler : public LogHandler, private NotCopyable
{
   public:

      /**
        *
        *   Initializes the instance and sets up the syslogging
        *   @param ident The identifier to use (eg the program name)
        *   @param facility The syslog facility to use
        */
      SyslogLogHandler(const char* ident, int facility);

      /**
        *   Cleans up
        */
      virtual ~SyslogLogHandler();

      /**
        *   handleMessage is called with a new log entry (level and text)
	     *   @param level The loglevel, see LogBuffer
        *   @param msg   Pointer to the log message
        *   @param msgLen Length of the log message
        *   @param levelStr String representation of the log level
        *   @param timeStamp String representation of the time stamp
	     */
      virtual void handleMessage(int level, const char* msg, int msgLen,
                                 const char* levelStr, 
                                 const char* timeStamp);

      /**
       *    Enum of the facilities we might be interested in using
       */
      enum SYSLOGFACILITIES {
         SYSLOG_DAEMON     = LOG_DAEMON,
         SYSLOG_USER       = LOG_USER,
         SYSLOG_LOCAL0     = LOG_LOCAL0,
         SYSLOG_LOCAL1     = LOG_LOCAL1,
         SYSLOG_LOCAL2     = LOG_LOCAL2,
         SYSLOG_LOCAL3     = LOG_LOCAL3,
         SYSLOG_LOCAL4     = LOG_LOCAL4,
         SYSLOG_LOCAL5     = LOG_LOCAL5,
         SYSLOG_LOCAL6     = LOG_LOCAL6,
         SYSLOG_LOCAL7     = LOG_LOCAL7
      };

      /**
        *   The syslog ident string
        */
      const char* m_ident;

      /**
        *   The syslog facility to use
        */
      int m_facility;

};

#endif // SYSLOGLOGHANDLER_H
