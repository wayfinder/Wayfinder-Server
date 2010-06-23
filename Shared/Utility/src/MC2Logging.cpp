/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2Logging.h"
#include <time.h>
#include <stdio.h>
#include "ISABThread.h"
#include "DeleteHelpers.h"
#include "StreamLogHandler.h"
#include "LogBuffer.h"
#ifndef _MSC_VER
#include <unistd.h>
#endif

MC2Logging* MC2Logging::s_instance;

namespace {

/// Makes sure the MC2Logging singleton is deleted
std::auto_ptr<MC2Logging> singletonRemover( &MC2Logging::getInstance() );

void printStartInfo( ) {
   mc2log << info << "*** MC2 logging starting";
#ifdef HAVE_ARCH
   mc2log << " for pid: " << getpid();
#endif
   mc2log << endl;
}

/**
 * Unlike std::ostream this class deletes its streambuf
 * in the destructor.
 */
class MC2OStream : public std::ostream {
public:
   explicit MC2OStream( std::streambuf* sb ) 
         : std::ostream( sb ) {
   }

   virtual ~MC2OStream() {
      delete rdbuf();
   }
};

/**
 *   Creates a log ostream connected to log handlers
 *   via LogBuffer.
 */
ostream& createLogStream() {
   return *( new MC2OStream(new LogBuffer()) );
}
}

MC2Logging::~MC2Logging() {
   deleteHandlers();
}

MC2Logging&
MC2Logging::getInstance() {
   static ISABOnceFlag onceControl = ISAB_ONCE_INIT;
   ISABCallOnce( constructSingleton, onceControl );
   return *s_instance;
}

void
MC2Logging::constructSingleton() {
   s_instance = new MC2Logging();
}

MC2Logging::MC2Logging() 
      : m_debugOutput( true ),
        m_mutex( new ISABMutexBeforeInit() ) {

   LogHandler* defaultHandler = new StreamLogHandler( stderr );

   addHandler( ( LOGLEVEL_DEBUG | 
                 LOGLEVEL_INFO  | 
                 LOGLEVEL_WARN  |
                 LOGLEVEL_ERROR |
                 LOGLEVEL_FATAL ), defaultHandler );
}

void
MC2Logging::setDebugOutput(bool debug) {
   ISABSyncBeforeInit sync( *m_mutex );
   m_debugOutput = debug;
}

bool
MC2Logging::getDebugOutput() const {
   ISABSyncBeforeInit sync( *m_mutex );
   return m_debugOutput;
}

void
MC2Logging::deleteHandlers() {
   vector<LogHandler*> allOldHandlers;

   // only lock during simple operations where there is no risk of 
   // logging being done.
   {
      ISABSyncBeforeInit sync( *m_mutex );
      allOldHandlers.swap( m_allHandlers );
      
      m_handlersDebug.clear();
      m_handlersInfo.clear();
      m_handlersWarn.clear();
      m_handlersError.clear();
      m_handlersFatal.clear();
   }

   STLUtility::deleteValues( allOldHandlers );
}

vector<LogHandler*> 
MC2Logging::getDebugHandlers() const {
   ISABSyncBeforeInit sync( *m_mutex );
   if ( m_debugOutput ) {
      return m_handlersDebug;
   }
   else {
      return vector<LogHandler*>();
   }
}

vector<LogHandler*> 
MC2Logging::getInfoHandlers () const {
   ISABSyncBeforeInit sync( *m_mutex );
   return m_handlersInfo;
}

vector<LogHandler*> 
MC2Logging::getWarnHandlers () const {
   ISABSyncBeforeInit sync( *m_mutex );
   return m_handlersWarn;
}

vector<LogHandler*> 
MC2Logging::getErrorHandlers() const {
   ISABSyncBeforeInit sync( *m_mutex );
   return m_handlersError;
}

vector<LogHandler*> 
MC2Logging::getFatalHandlers() const {
   ISABSyncBeforeInit sync( *m_mutex );
   return m_handlersFatal;
}

void
MC2Logging::addHandler(int level, LogHandler* handler) {
   ISABSyncBeforeInit sync( *m_mutex );

   if ((level & LOGLEVEL_DEBUG) == LOGLEVEL_DEBUG)
      m_handlersDebug.push_back( handler );

   if ((level & LOGLEVEL_INFO) == LOGLEVEL_INFO)
      m_handlersInfo.push_back( handler );

   if ((level & LOGLEVEL_WARN) == LOGLEVEL_WARN)
      m_handlersWarn.push_back( handler );

   if ((level & LOGLEVEL_ERROR) == LOGLEVEL_ERROR)
      m_handlersError.push_back( handler );

   if ((level & LOGLEVEL_FATAL) == LOGLEVEL_FATAL)
      m_handlersFatal.push_back( handler );

   m_allHandlers.push_back( handler );
}

namespace {

/** 
 *  Destructor function which cleans up the thread specific
 *  stream for each thread when the thread terminates.
 */
void deleteOStream( void* ptr ) {
   ostream* stream = static_cast<ostream*>( ptr );
   delete stream;
}

/// Makes sure we only call initTSS once
ISABOnceFlag tssInited = ISAB_ONCE_INIT;

/// The key to the thread specific storage of log streams
ISABTSS::TSSKey logStreamTSSKey;

/// Initiates logStreamTSSKey
void initTSS() {
   logStreamTSSKey = ISABTSS::createKey( deleteOStream );
}

/// Makes sure printStartInfo is only called once
ISABOnceFlag startInfoWritten = ISAB_ONCE_INIT;

/**
 * The purpose of this auto_ptr is to clean up the log
 * stream for the main thread. For some reason the TSS
 * destructor is only called when child threads 
 * terminate, not when the main thread terminates.
 */
std::auto_ptr<ostream> mainThreadLogStream(&mc2log);

}

ostream& getThreadSpecificLogStream() {
   ISABCallOnce( &initTSS, tssInited );
  
   ostream* stream = static_cast<ostream*>( ISABTSS::get( logStreamTSSKey ) );
   
   if ( stream == NULL ) {
      stream = &createLogStream();
      ISABTSS::set( logStreamTSSKey, stream );

      ISABCallOnce( &printStartInfo, startInfoWritten );
   }
   return *stream;
}

// struct mallinfo mi;

ostream & info (ostream & os) {
       os << '\0';
       return (os << 'I');
}

ostream & warn (ostream & os) {
       os << '\0';
       return (os << 'W');
}

ostream & error (ostream & os) {
       os << '\0';
       return (os << 'E');
}

ostream & fatal (ostream & os) {
       os << '\0';
       return (os << 'F');
}
