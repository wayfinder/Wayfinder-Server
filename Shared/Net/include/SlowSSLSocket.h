/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SLOWSSLSOCKET_H
#define SLOWSSLSOCKET_H

#include "config.h"
#include "SSLSocket.h"

#ifdef USE_SSL

/**
 *    This a debug-version of the SSLSocket that simulates a slow 
 *    connection. This is siumlated by sleeping a random time in
 *    the read-, write- and connect-methods.
 *
 */
class SlowSSLSocket : public SSLSocket {
   public:
      /**
       *    Create a new SlowSSLSocket.
       *    @param ctx      The SSL-contex to use.
       *    @param maxDelay The maximum delay in the read-, write- and 
       *                    connect-methods. The delay in milliseconds.
       */
      SlowSSLSocket(SSL_CONTEXT* ctx, uint32 maxDelay);

      /**
       *    Delete this socket. If the socket is open, it is also 
       *    closed.
       */
      virtual ~SlowSSLSocket();

   protected:
      /**
       *    Reads from the socket.
       *    @param buffer The buffer to put the result into.
       *    @param size   The maximum size to read.
       *    @return The number of bytes read or -1 if errno != EINTR.
       */
      virtual ssize_t protectedRead( byte* buffer, size_t size );
      
      /**
       *    Writes to the socket.
       *    @param buffer The buffer to write.
       *    @param size   The maximum size to write.
       *    @return The number of bytes written or -1 if errno != EINTR.
       */
      virtual ssize_t protectedWrite( const byte* buffer, size_t size );

   private:
      /**
       *    The maximum delaytime in milliseconds.
       */
      uint32 m_maxDelay;

      /**
       *
       */
      void simulateDelay(uint32 startTime);
};

#endif // USE_SSL

#endif

