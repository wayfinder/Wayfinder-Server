/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "InterfaceParserThread.h"
#include "InterfaceParserThreadGroup.h"
#include "StringUtility.h"
#include "NetUtility.h"

InterfaceParserThread::InterfaceParserThread( 
   InterfaceParserThreadGroupHandle group,
   const char* threadName )
      : ParserThread( group.get(), threadName )
{}


InterfaceParserThread::~InterfaceParserThread() {
   mc2dbg << "[InterfaceParserThread] dead." << endl;
}


void
InterfaceParserThread::run() {
   // Get a irequest and call handleInterfaceRequest
   InterfaceRequest* ireply = NULL;
   InterfaceRequest* ireq = NULL;
   InterfaceParserThreadGroupHandle group = 
      static_cast< InterfaceParserThreadGroup* > ( m_group.get() );
   while ( !terminated ) {
      if ( group->returnAndGetInterfaceRequest( ireply, ireq, this ) ) {
         ireply = NULL;
         if ( ireq != NULL ) {
            handleInterfaceRequest( ireq );
            ireply = ireq;
            ireq = NULL;
         } else {
            // Hmm, nothing to do taking a short nap.
            // This helps the busy-wait in shutdown
            sleep( 100 );
         }
      } else {
         terminated = true;
      }
   }
}
