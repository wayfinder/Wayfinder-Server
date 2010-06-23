/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "InterfaceRequest.h"
#include "TimeUtility.h"


InterfaceRequest::InterfaceRequest() 
      : m_startQueueTime(0), m_state( Uninitalized ), m_startTime( 
         TimeUtility::getCurrentTime() )
        
        
{}


InterfaceRequest::~InterfaceRequest() {
}


const char* 
InterfaceRequest::getStateAsString( state_t state ) {
   switch( state ) {
      case Ready_To_IO_Request:
         return "Ready_To_IO_Request";
      case Ready_To_IO_Reply:
         return "Ready_To_IO_Reply";
      case Ready_To_Process:
         return "Ready_To_Process";
      case Done:
         return "Done";
      case Error:
         return "Error";
      case Timeout_request:
         return "Timeout_request";
      case Timeout_reply:
         return "Timeout_reply";
      case Uninitalized:
         return "Uninitalized";
   }

   // Should not reach this
   return "Error";
}


void 
InterfaceRequest::terminate() {
   // Nothing on this level.
}


uint32
InterfaceRequest::getTimeFromPutInQueueMs() const
{
   return TimeUtility::getCurrentTime() - m_startQueueTime;
}

uint32
InterfaceRequest::getStartQueueTime() const {
   return m_startQueueTime;
}


void
InterfaceRequest::startQueueTime()
{
   m_startQueueTime = TimeUtility::getCurrentTime();
}

int
InterfaceRequest::getPriority( const InterfaceHandleIO* g ) const {
   return 0;
}
