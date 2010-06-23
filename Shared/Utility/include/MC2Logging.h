/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2LOGGING_H
#define MC2LOGGING_H

#include "config.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
using namespace std;

class LogHandler;
class ISABMutexBeforeInit;

/**
 *   MC2 logging helper class
 *
 */
class MC2Logging {
public:

   ~MC2Logging();

   /**
    *   The log levels
    */
   enum LOGLEVELS {
      LOGLEVEL_DEBUG = 1,
      LOGLEVEL_INFO  = 2,
      LOGLEVEL_WARN  = 4,
      LOGLEVEL_ERROR = 8,
      LOGLEVEL_FATAL = 16
   };
   
   /**
    *  Gets the only instance of MC2Logging.
    */
   static MC2Logging& getInstance();

   /**
    *   Turn debugging on/off
    *   @param debug If true debugging is turned on, if false all debugging
    *                output is sent to /dev/null
    */
   void setDebugOutput(bool debug);

   /**
    *    Returns whether debugging is turned on/off.
    */
   bool getDebugOutput() const;
   
   /**
    *   Add a handler.
    *   @param level The log levels we want to be notified at
    *   @param handler Pointer to a LogHandler instance
    */
   void addHandler( int level, LogHandler* handler );
   
   /**
    *   Deletes all LogHandlers.
    */
   void deleteHandlers();

   /* 
    * The getXXXHandlers() functions deliberately returns a copy for
    * thread safety.
    */
   vector<LogHandler*> getDebugHandlers() const;
   vector<LogHandler*> getInfoHandlers () const;
   vector<LogHandler*> getWarnHandlers () const;
   vector<LogHandler*> getErrorHandlers() const;
   vector<LogHandler*> getFatalHandlers() const;

private:

   /// Hidden constructor since this is a singleton.
   MC2Logging();

   /// Creates the singleton
   static void constructSingleton();

   /// The single instance
   static MC2Logging* s_instance;

   /// Whether debugging is turned on/off.
   bool m_debugOutput;

   /// The LogHandlers for level LOGLEVEL_DEBUG
   vector<LogHandler*> m_handlersDebug;

   /// The LogHandlers for level LOGLEVEL_INFO
   vector<LogHandler*> m_handlersInfo;

   /// The LogHandlers for level LOGLEVEL_WARN
   vector<LogHandler*> m_handlersWarn;

   /// The LogHandlers for level LOGLEVEL_ERROR
   vector<LogHandler*> m_handlersError;

   /// The LogHandlers for level LOGLEVEL_FATAL
   vector<LogHandler*> m_handlersFatal;

   /// All added handlers for easy deletion
   vector<LogHandler*> m_allHandlers;

   std::auto_ptr<ISABMutexBeforeInit> m_mutex;
};

/**
 *   Abstract superclass for a LogHandler that's derived to build
 *   external logging features (eg syslog, SNMP traps).
 *
 */
class LogHandler {
public:
   virtual ~LogHandler() {
   }
   /**
    *   handleMessage is called with a new log entry (level and text)
    *
    *   This method should be thread safe if the LogHandler is used
    *   in a multi threaded program.
    *
    *   @param level The loglevel, see MC2Logging
    *   @param msg   Pointer to the log message
    *   @param msgLen Length of the log message
    *   @param levelStr String representation of the log level
    *   @param timeStamp String representation of the time stamp
    */
   virtual void handleMessage(int level, const char* msg, int msgLen,
                              const char* levelStr, 
                              const char* timeStamp) = 0;
};

/**
 * This function gets the log stream for the currently executing thread.
 * Do not use this function directly, instead use mc2log.
 */
ostream& getThreadSpecificLogStream();

// the logging stream
#define mc2log getThreadSpecificLogStream()

/*
extern struct mallinfo mi;

#define MINFO(a) mi = mallinfo(); mc2dbg << a << " minfo: " << mi.uordblks << endl;
*/

#define MINFO(a) 

// the debug stream
#ifdef DEBUG_LEVEL_1
#   define mc2dbg mc2log
#   define mc2dbg1 mc2log
#else
#   define mc2dbg if (false) mc2log
#   define mc2dbg1 if (false) mc2log
#endif
#ifdef DEBUG_LEVEL_2
#   define mc2dbg2 mc2log
#else
#   define mc2dbg2 if (false) mc2log
#endif
#ifdef DEBUG_LEVEL_4
#   define mc2dbg4 mc2log
#else
#   define mc2dbg4 if (false) mc2log
#endif
#ifdef DEBUG_LEVEL_8
#   define mc2dbg8 mc2log
#else
#   define mc2dbg8 if (false) mc2log
#endif

// manipulator that chooses info level logging
ostream & info (ostream & os);

// manipulator that chooses warning level logging
ostream & warn (ostream & os);

// manipulator that chooses error level logging
ostream & error (ostream & os);

// manipulator that chooses fatal level logging
ostream & fatal (ostream & os);

// small macro that outputs the filename and line of the current line of code
// when used on an ostream.
#define here __FILE__ ":" << __LINE__

// macro to output an IP numerically
#define prettyPrintIP(a) int((a & 0xff000000) >> 24) << "." \
                      << int((a & 0x00ff0000) >> 16) << "." \
                      << int((a & 0x0000ff00) >>  8) << "." \
                      << int (a & 0x000000ff)

// macro to output an map id in hex
#define prettyMapIDFill(a) "0x" << setw(8) << setfill('0') << hex << a << dec
#define prettyMapID(a) "0x" << hex << a << dec

// macro to print citation marks before and after a
#define MC2CITE(a) '"' << (a) << '"'
/// macro to print a hex number prefixed by 0x and then switch to dec again
#define MC2HEX(a) "0x" << hex << (a) << dec

#endif // MC2LOGGING_H
