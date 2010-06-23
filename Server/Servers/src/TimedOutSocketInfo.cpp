/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TimedOutSocketInfo.h"
#include "TCPSocket.h"

TimedOutSocketInfo::TimedOutSocketInfo( TCPSocket* socket, 
                                        int timeOutMs,
                                        const MC2String& logStr ) :
   m_timedOutSocket ( socket ),
   m_loggedStr ( logStr ),
   m_timeOutMs ( timeOutMs ) {
}

TimedOutSocketInfo::~TimedOutSocketInfo() {
   if (m_timedOutSocket != NULL) {
      delete m_timedOutSocket;
   }
}

TCPSocket* 
TimedOutSocketInfo::detachSocket() {
   TCPSocket* socket = m_timedOutSocket;
   m_timedOutSocket = NULL;
   return socket;
}

const MC2String& 
TimedOutSocketInfo::getLoggedString() {
   return m_loggedStr;
}

int 
TimedOutSocketInfo::getTimeOutMs() {
   return m_timeOutMs;
}

bool
TimedOutSocketInfo::statusOk() {
   return ( m_timedOutSocket != NULL );
}
