/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HTTPPARSERTHREADGROUP_H
#define HTTPPARSERTHREADGROUP_H

#include "config.h"
#include "InterfaceParserThreadGroup.h"
#include "HttpParserThreadConfig.h"
#include "ServerTypes.h"

#include <map>

class HttpSocketQueue;
class HttpSocketListener;
class ThreadRequestContainer;
class HttpFileHandler;
class TCPSocket;

namespace ActivityLog {
struct Activity;
};


/**
 * Superclass for classes that handles the ParserThreads. Acts as an 
 * interface to these Threads for other classes.
 *
 */
class HttpParserThreadGroup : public InterfaceParserThreadGroup {
   public:
      /** 
       * Creates a new HttpParserThreadGroup.
       *
       * @param serverType The type of server
       * @param hostnameLookups Log the names of client hosts or just 
       *        their IP addresses e.g., myhost.com (true) or 
       *        213.141.145.151 (false).
       * @param httplogFilename The name of the http logfile, if NULL
       *        then the Property HTTP_LOG is used if not such property
       *        then the default "http.log" is used.
       * @param threadGroupName The name of the group.
       * @param minNbrThreads The lowest number of parserthreads.
       * @param maxNbrThreads The highest number of parserthreads.
       * @param queueFactor See InterfaceParserThreadGroup.
       * @param queueOverFullFactor See InterfaceParserThreadGroup.
       */
      HttpParserThreadGroup( ServerTypes::servertype_t serverType,
                             bool hostnameLookups = false,
                             const char* httplogFilename = NULL,
                             const char* threadGroupName =
                             "HttpInterfaceParserThreadGroup",
                             uint32 minNbrThreads = 5,
                             uint32 maxNbrThreads = 20,
                             uint32 queueFullFactor = 3,
                             uint32 queueOverFullFactor = 3 );

      
      /**
       * Destructor.
       */
      virtual ~HttpParserThreadGroup();


      /**
       * Handle a done InterfaceRequest.
       *
       * @param ireply The InterfaceRequest that is done.
       */
      virtual void handleDoneInterfaceRequest( InterfaceRequest* ireply );

      
   protected:
      /**
       * Create a new thread for processing, subclasses should override
       * this with their own implementation that creates parserthreads
       * of the desired type.
       *
       * @return A new ParserThread.
       */
      virtual ParserThreadHandle spawnThread();


      /**
       * The default page for a request for "/".
       * @return A string with the default page for a request for "/".
       */
      virtual MC2String getDefaultPage() const;


   private:
      /**
       * Reads the file with the path fileString.
       * @param fileString the path and filename. 
       *        The path is relative the HTML_ROOT.
       * @param length is set to the length of the file.
       *        Only valid if the file excists.
       * @param fStat struct is set to contain the status of the file.
       *        Only valid if the file excists.
       */
      byte* getFile(const MC2String* fileString, 
                    int& length, 
                    struct stat *fStat);


      /**
       * Puts an entry in the httplog.
       *
       * @param sock The socket to get hostname from.
       * @param peerIP The peers IP-address, 0 if not known.
       * @param time The time of the request, if 0 current time
       *             is used.
       * @param startLine The request "GET / HTTP/1.0" line without 
       *                  linebreak.
       * @param code The result code of the request, 200, 404 etc.
       * @param size The number of bytes sent in reply, if not known use 0.
       * @param userAgent The user agent.
       * @param rfcname If you enable identd (see Web Developer® Spring 
       *                1996 issue, p.23), you can retrieve a name from 
       *                the remote server for the user. If no value is 
       *                present, a "-" is substituted.
       * @param logname If you're using local authentication and 
       *                registration, the user's log name will appear; 
       *                likewise, if no value is present, a "-" is
       *                substituted.
       */
      void httpLog( TCPSocket* sock, uint32 peerIP, uint32 time,
                    const char* startLine, uint32 code, uint32 size,
                    const char* userAgent = "-",
                    const char* rfcname = NULL, 
                    const char* logname = NULL );


      /**
       * If client hosts names should be looked up.
       */
      bool m_hostnameLookups;


      /**
       * The monitor protecting files.
       */
      ISABMonitor m_fileMonitor;


      /** 
       * The only filehandler.
       */
      HttpFileHandler* m_files;


      /**
       * The Httplog file.
       */
      FILE* m_httpLogfile;

      
      /**
       * The name of the Httplog file.
       */
      MC2String m_httpLogFilename; 


      /**
       * HttpParserThread is a friend, it calls getFile and httpLog.
       */
      friend class HttpParserThread;

};

typedef JTCHandleT<HttpParserThreadGroup> HttpParserThreadGroupHandle;


#endif // HTTPPARSERTHREADGROUP_H
