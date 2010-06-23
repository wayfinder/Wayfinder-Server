/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "SocketBuffer.h"
#include "TCPSocket.h"

SocketBuffer::SocketBuffer( TCPSocket* sock, uint32 readSize ) 
      : m_sock( sock ),
        m_readSize( readSize ),
        m_buffer( NULL ),
        m_pos( 0 ),
        m_length( 0 )
{
   if ( m_readSize == 0 ) {
      // Sanity check
      m_readSize = 1;
   }
}


SocketBuffer::~SocketBuffer() {
   delete [] m_buffer;
}


ssize_t
SocketBuffer::read( byte *buffer,
                    size_t length,
                    long micros )
{
   uint32 totLength = length;
   if ( m_length - m_pos < length ) {
      // Read from socket and add unread from buffer
      ssize_t nbr = 1;

      totLength = m_length - m_pos; // We have this much data buffered
      while ( nbr > 0 && totLength < length ) {
         ssize_t readSize = max( m_readSize, static_cast<uint32>(length - totLength) );
         byte* readBuff = new byte[ readSize ];

         nbr = m_sock->readMaxBytes( readBuff, readSize, micros );

         mc2dbg8 << "SocketBuffer::read( " << length << ", " << micros
                 << ") " << max( m_readSize, static_cast<uint32>(length-totLength)) 
                 << " totlength " << totLength << "nbr read " << nbr << endl; 


         if ( nbr > 0 ) {
            // Ok read some data
            if ( m_pos < m_length ) {
               // Concatenate buffers
               uint32 buffSize = (m_length - m_pos);
               byte* newBuff = new byte[ nbr + buffSize ];
            
               memcpy( newBuff, m_buffer + m_pos, buffSize );
               memcpy( newBuff + buffSize, readBuff, nbr );
               m_pos = 0;
               m_length = buffSize + nbr;
               delete [] readBuff;
               delete [] m_buffer;
               m_buffer = newBuff;
            } else {
               // Use new buffer
               delete [] m_buffer;
               m_buffer = readBuff;
               m_pos = 0;
               m_length = nbr;
            }
            totLength += nbr;
         } else {
            // Socket has not enough data return errornbr
            delete [] readBuff;
            return nbr;
         }
      }
      // Only return as much as requested even if we have read more
      totLength = min( length, static_cast<size_t>(m_length) );
   }

   // If we are here then we have data in buffer return it
   // Read from buffer
//   mc2dbg8 << "Returning pos " << m_pos << " of " << m_length 
//          << " for " << length << " bytes " << endl;
   memcpy( buffer, m_buffer + m_pos, totLength );
   m_pos += totLength;
   return totLength;
}


int 
SocketBuffer::readLine( char* target, uint32 maxlinelength,
                        long micros ) {
   int nbr;
   uint32 nbrRead = 0;
   char c;

   nbr = read((byte*)&c, 1, micros);
   mc2dbg8 << "read #" << c << "#" << endl;
   while ( (nbr == 1) && (c != '\r') && (c != '\n') &&
           (nbrRead < maxlinelength) ) { 
      // More to read and not end of line
      target[nbrRead++] = c;
      nbr = read((byte*)&c, 1, micros);
      mc2dbg8 << "read #" << c << "#" << endl;
   }
   
   // Skip linebreak
   if ( (nbr == 1) && (c == '\r') ) { // Has read CR
      nbr = read((byte*)&c, 1, micros); // Then read the LF
      mc2dbg8 << "read #" << c << "#" << endl;
   }
   target[nbrRead] = '\0';
   mc2dbg8 << "Line: " << target << endl;
   return nbrRead;
}


ssize_t 
SocketBuffer::read( byte *buffer, size_t length ) {
   uint32 totLength = length;
   if ( m_length - m_pos < length ) {
      // Read from socket and add unread from buffer
      ssize_t nbr = 1;

      totLength = m_length - m_pos; // We have this much data buffered
      while ( nbr > 0 && totLength < length ) {
         ssize_t readSize = max( m_readSize, static_cast<uint32>(length - totLength) );
         byte* readBuff = new byte[ readSize ];

         nbr = m_sock->read( readBuff, readSize );

         mc2dbg8 << "SocketBuffer::read( " << length << ") " 
                 << max( m_readSize, static_cast<uint32>(length-totLength)) 
                 << " totlength " << totLength << "nbr read " << nbr 
                 << endl; 

         if ( nbr > 0 ) {
            // Ok read some data
            if ( m_pos < m_length ) {
               // Concatenate buffers
               uint32 buffSize = (m_length - m_pos);
               byte* newBuff = new byte[ nbr + buffSize ];
            
               memcpy( newBuff, m_buffer + m_pos, buffSize );
               memcpy( newBuff + buffSize, readBuff, nbr );
               m_pos = 0;
               m_length = buffSize + nbr;
               delete [] readBuff;
               delete [] m_buffer;
               m_buffer = newBuff;
            } else {
               // Use new buffer
               delete [] m_buffer;
               m_buffer = readBuff;
               m_pos = 0;
               m_length = nbr;
            }
            totLength += nbr;
         } else {
            // Socket has not enough data return errornbr
            delete [] readBuff;
            return nbr;
         }
      }
      // Only return as much as requested even if we have read more
      totLength = min( length, static_cast<size_t>(m_length) );
   }

   // If we are here then we have data in buffer return it
   // Read from buffer
   memcpy( buffer, m_buffer + m_pos, totLength );
   m_pos += totLength;
   return totLength;
   
}


bool 
SocketBuffer::hasBytes( uint32 length ) const {
   return length <= (m_length - m_pos);
}
