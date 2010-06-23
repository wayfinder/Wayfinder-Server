/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "LoggingHttpInterfaceRequest.h"

#include "TimedOutSocketInfo.h"
#include "SocketBuffer.h"

LoggingHttpInterfaceRequest::LoggingHttpInterfaceRequest( TimedOutSocketInfo* socketInfo ) :
      HttpInterfaceRequest( socketInfo->detachSocket(), IPnPort() /* Not needed */),
      m_socketInfo( socketInfo ) {
   setState( Ready_To_IO_Request );
   // m_timeout is 60000ms at the moment. compares 
   // m_totalTimeout ? compares with m_ioStartTime
}

LoggingHttpInterfaceRequest::~LoggingHttpInterfaceRequest() {
   if (m_socketInfo != NULL)
   {
      delete m_socketInfo;
   }
}

void 
LoggingHttpInterfaceRequest::handleIO( bool readyRead, bool readyWrite) {
   HttpInterfaceRequest::handleIO(readyRead, readyWrite);

   if (getState() != Ready_To_IO_Request) {
      dumpState( mc2log );
   }
}

void 
LoggingHttpInterfaceRequest::timedout() {
   HttpInterfaceRequest::timedout();
   
   dumpState( mc2log );
}

ostream& 
LoggingHttpInterfaceRequest::dumpState( ostream& out ) const {

   out << "[LoggingHttpInterfaceRequest]: Connection handledIO/timedout"
       << ", Source message: " << m_socketInfo->getLoggedString()
       << ", Total time: " << (getUsedIOTime() + m_socketInfo->getTimeOutMs()) << "ms"
       << ", Timeout: " << m_socketInfo->getTimeOutMs() << "ms"
       << ", State: " << getStateAsString( getState() ) << endl;
         
   HttpInterfaceRequest::dumpState( out );

   return out;
}

