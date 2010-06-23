/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SlowSSLSocket.h"
#include "TimeUtility.h"

#ifdef USE_SSL

SlowSSLSocket::SlowSSLSocket(SSL_CONTEXT* ctx, uint32 maxDelay) 
   : SSLSocket(ctx) 
{
   m_maxDelay = maxDelay;
}


SlowSSLSocket::~SlowSSLSocket()
{
   // Nothing to do...
}

ssize_t 
SlowSSLSocket::protectedRead(byte* buffer, size_t size)
{
   // Read and calculate the consumed time
   uint32 startTime = TimeUtility::getCurrentTime();
   ssize_t returnValue = SSLSocket::protectedRead(buffer, size);

   // Simulate the delay
   simulateDelay(startTime);

   return (returnValue);
}

ssize_t 
SlowSSLSocket::protectedWrite(const byte* buffer, size_t size)
{
   // Read and calculate the consumed time
   uint32 startTime = TimeUtility::getCurrentTime();
   ssize_t returnValue = SSLSocket::protectedWrite(buffer, size);

   // Simulate the delay
   simulateDelay(startTime);

   return (returnValue);
}


void 
SlowSSLSocket::simulateDelay(uint32 startTime) {
   uint32 trueTime = TimeUtility::getCurrentTime() - startTime;

   // Wait so simulate delay
   uint32 simTime = uint32( float64(m_maxDelay) * float64(random()) / 
                            float64(RAND_MAX));
#ifndef _WIN32
   cerr << "      RandomValue simTime = " << simTime << endl;
   if (simTime > trueTime) {
      cerr << "SlowSSLSocket sleeping in " << (simTime-trueTime) << "ms" 
           << endl;
      TimeUtility::microSleep( (simTime-trueTime) * 1000);
   }
#else
   if (simTime > trueTime) {
      Sleep( simTime-trueTime );
   }
#endif

}

#endif // USE_SSL
