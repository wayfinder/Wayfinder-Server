/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "HttpParserThreadGroup.h"
#include "HttpParserThread.h"
#include "GDImageDraw.h"
#include "ThreadRequestHandler.h"
#include "ThreadRequestContainer.h"
#include "HttpFileHandler.h"
#include "HttpInterfaceFactory.h"
#include "Properties.h"
#include "TCPSocket.h"
#include "NetUtility.h"

HttpParserThreadGroup::
HttpParserThreadGroup( ServerTypes::servertype_t serverType,
                       bool hostnameLookups,
                       const char* httplogFilename,
                       const char* threadGroupName,
                       uint32 minNbrThreads, uint32 maxNbrThreads,
                       uint32 queueFullFactor, uint32 queueOverFullFactor )
      : InterfaceParserThreadGroup( serverType, threadGroupName, minNbrThreads,
                                    maxNbrThreads, queueFullFactor,
                                    queueOverFullFactor ),
        m_hostnameLookups( hostnameLookups )
{
   m_files = new HttpFileHandler();
   m_httpLogfile = NULL;

   if ( httplogFilename != NULL ) {
      m_httpLogFilename = httplogFilename;
   } else {
      m_httpLogFilename = Properties::getProperty( "HTTP_LOG_FILE", "" );
   }

   m_httpLogFilename = StringUtility::trimStartEnd( m_httpLogFilename );

   GDImageDraw::initializeDrawImages(); 
}


HttpParserThreadGroup::~HttpParserThreadGroup() {
   delete m_files;
   if ( m_httpLogfile != NULL ) {
      fclose( m_httpLogfile );
   }

   GDImageDraw::removeDrawImages();

}


void 
HttpParserThreadGroup::handleDoneInterfaceRequest( 
   InterfaceRequest* ireply )
{
   static_cast< HttpInterfaceFactory* > ( getFirstInterfaceFactory() )
      ->handleDoneInterfaceRequest( ireply );
}


ParserThreadHandle
HttpParserThreadGroup::spawnThread() {
   return NULL;
//   return ( new HttpParserThread( this ) );
}


MC2String 
HttpParserThreadGroup::getDefaultPage() const {
   return "index.html";
}


byte*
HttpParserThreadGroup::getFile(const MC2String* fileString, 
                               int& length, 
                               struct stat *fStat) 
{
   ISABSync sync( m_fileMonitor );
   return m_files->getFile( *fileString, length, fStat );
}


void 
HttpParserThreadGroup::httpLog( TCPSocket* sock, uint32 peerIP, 
                                uint32 time,
                                const char* startLine, uint32 code, 
                                uint32 size,
                                const char* userAgent,
                                const char* rfcname,
                                const char* logname )
{
   if ( m_httpLogFilename.empty() ) {
      return;
   }

   if ( userAgent[0] == '\0' ) {
      userAgent = "-";
   }
   // Check time first to get time as close to the request as possible
   if ( time == 0 ) {
      time = TimeUtility::getRealTime();
   }


   if ( peerIP == 0 ) {
      // Get hostname
      uint16 port = 0;
      sock->getPeerName( peerIP, port );
   }
   MC2String hostName = NetUtility::getHostName( peerIP, m_hostnameLookups );

   // Time format
   char date[30];
   struct tm* tmStruct = NULL;
   struct tm timeresult;
   time_t aTime = time;
   tmStruct = gmtime_r( &aTime, &timeresult );
   strftime( date, 29, "[%d/%b/%Y:%H:%M:%S %z]" , tmStruct );
   
   const char* referer = "-";
   // Put log entry together
   uint32 length = 0;
   length = hostName.size() + 1 + 
      ( rfcname ? strlen( rfcname ) : 1 ) + 1 +
      ( logname ? strlen( logname ) : 1 ) + 1 +
      29 + 1 + // strlen( date ) + 1 +
      strlen( startLine ) + 1 +
      3 + // code
      strlen(referer) + 6 + // "referer" + spaces
      strlen(userAgent) + 6 + // "user agent" + spaces
      10; // size (nbr bytes sent)

   char log[ length + 1 ];
   uint32 logLength = 0;
   logLength = sprintf( log, "%s %s %s %s \"%s\" %d %d \"%s\" \"%s\"\n",
                        hostName.c_str(),
                        rfcname ? rfcname : "-",
                        logname ? logname : "-",
                        date,
                        startLine, 
                        code,
                        size,
                        referer,
                        userAgent);

   if ( m_httpLogfile == NULL ) { // Open
      m_httpLogfile = fopen( m_httpLogFilename.c_str(), "a" ); // Append mode
      if ( m_httpLogfile == NULL ) {
         MC2ERROR2( "HttpProcess:: Couldn't open logfile",
                    cerr << log << endl; );
         return;
      }
   }
   
   int32 retries = 3;
   while ( fwrite( log, sizeof( char ), logLength, m_httpLogfile ) 
           != logLength && retries > 0 )
   {
      retries--;
      // Reopen file
      fclose( m_httpLogfile );
      m_httpLogfile = fopen( m_httpLogFilename.c_str(), "a" ); // Append mode
      if ( m_httpLogfile == NULL ) {
         MC2ERROR2( "HttpProcess:: Couldn't open logfile",
                    cerr << log << endl; );
         return;
      }
   }
   fflush( m_httpLogfile );
}
