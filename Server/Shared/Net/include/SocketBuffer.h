/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SOCKETBUFFER_H
#define SOCKETBUFFER_H

#include "config.h"
#include "NotCopyable.h"

class TCPSocket;

/**
 * Makes long reads from socket and stores the extra data until its read.
 * 
 */
class SocketBuffer: private NotCopyable {
   public:
      /**
       * Makes a new socket buffer using the sock.
       *
       * @param sock The socket to read from, isn't closed not deleted.
       * @param readSize The amount of data read by each read on socket.
       */
      SocketBuffer( TCPSocket* sock, 
                    uint32 readSize = 16384 );
      
      
      /**
       * Deallocates all buffer.
       */
      ~SocketBuffer();

      
      /**
       * Read data uses cached data if possible.
       * @param buffer The buffer where the result will be stored.
       *               This buffer must be at least length bytes big.
       * @param length The requested length.
       * @paarm micros Read timeout in micro seconds, if 
       *               TCPSOCKET_INFINITE_TIMEOUT there is no timeout.
       * @return The number of successully read bytes or -1
       *         upon failure.
       */
      ssize_t read( byte *buffer,
                    size_t length,
                    long micros );
      
      
      /**
       * Reads a line.
       *
       * @param target The string to wtite line into.
       * @param maxlinelength The maximum length of the line.
       * @param micros Read timeout in micro seconds, if 
       *               TCPSOCKET_INFINITE_TIMEOUT there is no timeout.
       * @return The number of bytes read.
       */
      int readLine( char* target, uint32 maxlinelength,
                    long micros = 10 * 60 * 1000000 );


      /**
       * Read data uses cached data if possible.
       *
       * @param buffer The buffer where the result will be stored.
       *               This buffer must be at least length bytes big.
       * @param length The requested length.
       * @return The number of successully read bytes or error from
       *         socket upon failure.
       */
      ssize_t read( byte *buffer, size_t length );


      /**
       * Check if there is length bytes in cache.
       */
      bool hasBytes( uint32 length = 1 ) const;


   private:
      /**
       * The socket to read from.
       */
      TCPSocket* m_sock;

      
      /**
       * The size of the chunks of data in bytes to read from socket.
       */
      uint32 m_readSize;

      
      /**
       * The buffer.
       */
      byte* m_buffer;

      
      /**
       * Position in buffer.
       */
      uint32 m_pos;


      /**
       * Length of buffer.
       */
      uint32 m_length;
};


#endif // SOCKETBUFFER_H
