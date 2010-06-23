/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XMLPARSERTHREADGROUP_H
#define XMLPARSERTHREADGROUP_H

#include "config.h"
#include "HttpParserThreadConfig.h"
#include "HttpParserThreadGroup.h"
#include "MC2String.h"


/**
 * Subclass to HttpParserThreadGroup that creates XMLParserThreads.
 *
 */
class XMLParserThreadGroup : public HttpParserThreadGroup {
   public:
      /** 
       * Creates a new XMLParserThreadGroup.
       *
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
      XMLParserThreadGroup( 
         bool hostnameLookups = false,
         const char* httplogFilename = NULL,
         const char* threadGroupName = "XMLInterfaceParserThreadGroup",
         uint32 minNbrThreads = 5, uint32 maxNbrThreads = 20,
         uint32 queueFullFactor = 3, uint32 queueOverFullFactor = 3 );

      
      /**
       * Destructor.
       */
      ~XMLParserThreadGroup();


   protected:
      /**
       * Create a new XMLPaserThread for processing.
       *
       * @return A new ParserThread.
       */
      virtual ParserThreadHandle spawnThread();

      
      /**
       * The default page for a request for "/".
       * @return A MC2String with the default page for a request for "/".
       */
      virtual MC2String getDefaultPage() const;
};


#endif // XMLPARSERTHREADGROUP_H
