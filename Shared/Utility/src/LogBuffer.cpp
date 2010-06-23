/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "LogBuffer.h"
#include <sys/time.h>

LogBuffer::LogBuffer() {
     // we want no buffering here:
     // put area set to zero
     setp(0, 0);
     // get area set to zero
     setg(0, 0, 0);

     m_newline = true;
     m_level = MC2Logging::LOGLEVEL_DEBUG;
     m_buffer = new char[BUFFERSIZE];
     m_bufPos = 0;
     m_prefix = NULL;
     m_magic = false;

     memset( m_timeBuf, 0, TIME_BUFF_SIZE );
}

LogBuffer::~LogBuffer() {
   delete[] m_buffer;
}

void
LogBuffer::setPrefix(const char* prefix) {
   if (m_prefix != NULL && (m_bufPos == (int)strlen(m_prefix))) {
      m_bufPos = 0;
   } 
   if (m_bufPos == 0 && prefix != NULL) {
      strcpy(m_buffer, prefix);
      m_bufPos = strlen(prefix);
   }

   m_prefix = prefix;
}

void
LogBuffer::notify() {
   m_buffer[m_bufPos] = '\0';

   vector<LogHandler*> list;
      
   if (MC2Logging::LOGLEVEL_DEBUG == m_level)
      list = MC2Logging::getInstance().getDebugHandlers();
   else if (MC2Logging::LOGLEVEL_INFO == m_level)
      list = MC2Logging::getInstance().getInfoHandlers();
   else if (MC2Logging::LOGLEVEL_WARN == m_level)
      list = MC2Logging::getInstance().getWarnHandlers();
   else if (MC2Logging::LOGLEVEL_ERROR == m_level)
      list = MC2Logging::getInstance().getErrorHandlers();
   else if (MC2Logging::LOGLEVEL_FATAL == m_level)
      list = MC2Logging::getInstance().getFatalHandlers();
   
   for ( size_t i = 0; i < list.size(); ++i ) {
      list[i]->handleMessage(m_level, m_buffer, m_bufPos,
                             m_levelBuf, m_timeBuf + 2);
   }
}

int      
LogBuffer::putTime() { 
   // Get time in seconds and microseconds and then
   // break it down to local time, but keep the microtime
   // so we can add milliseconds to the log
   struct timeval tv;
   gettimeofday(&tv, NULL);
   time_t tt = tv.tv_sec;
   struct tm result;
   struct tm* tm = localtime_r(&tt, &result);

   snprintf( m_timeBuf, TIME_BUFF_SIZE, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
             tm->tm_hour, tm->tm_min, tm->tm_sec,
             (int) ( tv.tv_usec / 1000 ));

   return 0;
}

int      
LogBuffer::putLevel() { 
   m_levelBuf[0] = '\0';
   switch (m_level) {
      case MC2Logging::LOGLEVEL_DEBUG:
         strcpy(m_levelBuf, "DEBUG: ");
      break;
      case MC2Logging::LOGLEVEL_INFO:
         strcpy(m_levelBuf, "INFO : ");
      break;
      case MC2Logging::LOGLEVEL_WARN:
         strcpy(m_levelBuf, "WARN : ");
      break;
      case MC2Logging::LOGLEVEL_ERROR:
         strcpy(m_levelBuf, "ERROR: ");
      break;
      case MC2Logging::LOGLEVEL_FATAL:
         strcpy(m_levelBuf, "FATAL: ");
      break;
      default:
         strcpy(m_levelBuf, "FOO!?: ");
      break;
   }

   return 0;
}


int      
LogBuffer::overflow(int c) {
   switch(c) {
      case EOF:
         // NOP
         break;
      case '\n': {
         if (m_bufPos > 0)
            notify();
         // set the flag
         m_newline = true;
         m_level = MC2Logging::LOGLEVEL_DEBUG;
         m_bufPos = 0;
         const char* prefix = m_prefix;
         if ( prefix != NULL ) {
            strcpy( m_buffer, prefix );
            m_bufPos = strlen( m_buffer );
         }
      } break;
      case '\0':
         // handle the next character as a magic one ?
         if (m_newline) {
            m_magic = true;
         }

         break;
      default:
         if (m_magic) {
            m_magic = false;
            if ('I' == c)
               m_level = MC2Logging::LOGLEVEL_INFO;
            else if ('W' == c)
               m_level = MC2Logging::LOGLEVEL_WARN;
            else if ('E' == c)
               m_level = MC2Logging::LOGLEVEL_ERROR;
            else if ('F' == c)
               m_level = MC2Logging::LOGLEVEL_FATAL;
         } else {
            // should we output the time and log level now?
            if (m_newline) {
               m_newline = false;
               if (putTime() == EOF)
                  return EOF;
               if (putLevel() == EOF)
                  return EOF;
            }
            // put it in our buffer
            if ( m_bufPos < BUFFERSIZE-2 ) 
               m_buffer[m_bufPos++] = (char)c;
         }
         break;
   }

   return 0;
}

int      
LogBuffer::underflow() {
   // we don't handle input
   return EOF;
}

int      
LogBuffer::uflow() {
   // we don't handle input
   return EOF;
}

int      
LogBuffer::sync() {
   // The bad bit will be set if we do mc2dbg << (char*)NULL;
   // so we need to clear it here.

   // clear bad bit (check ptr of mc2dbg it might not be initialized)
   if ( &mc2log != 0 && mc2log.bad() ) {
      mc2log.clear();
   }
   return 0;
}
