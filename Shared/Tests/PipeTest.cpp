/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Thread.h"
#include "Pipe.h"

/**
 * Produces a set of bytes read by the Consumer.
 */
class Producer: private NotCopyable {
public:
   Producer( Pipe& pipe, uint32 nbrBytes ):
      m_pipe( pipe ),
      m_nbrBytes( nbrBytes ),
      m_nbrBytesWritten( 0 ) {
   }

   void produce() {
      // write a bunch of strings to pipe and count the bytes written
      while ( m_nbrBytesWritten < m_nbrBytes ) { 
         int written = m_pipe.write( "A", 1 );
         if ( written != -1 ) {
            m_nbrBytesWritten += written;
         } else if ( errno != EAGAIN ) {
            break;
         }
      }
   }
   int getNbrBytesWritten() const {
      return m_nbrBytesWritten;
   }
private:
   Pipe& m_pipe;
   const uint32 m_nbrBytes;
   uint32 m_nbrBytesWritten;
};

/**
 * Consumes a set of bytes written by the Producers.
 */
class Consumer: private NotCopyable {
public:
   explicit Consumer( Pipe& pipe, uint32 maxBytesToRead ):
      m_pipe( pipe ),
      m_nbrBytesRead( 0 ),
      m_maxBytesToRead( maxBytesToRead ) {
   }
      
   void consume() {
      char buff[256];
      
      int nbrBytes = 0;
      while ( 1 ) {
         nbrBytes = m_pipe.read( buff, 256 );
         if ( nbrBytes == -1 ) {
            if ( errno == EAGAIN ) {
               continue;
            }
            break;
         }
         buff[ nbrBytes ] = 0;
         m_nbrBytesRead += nbrBytes;
         if ( m_nbrBytesRead == m_maxBytesToRead ) {
            break;
         }
      }

      if ( nbrBytes == -1 ) {
         cerr << errno << endl;
         cerr << "failed: " << strerror( errno ) << endl;
         cerr << "Failed to read bytes!" << endl;
      }
   }
   int getNbrBytesRead() const {
      return m_nbrBytesRead;
   }

private:
   Pipe& m_pipe;
   uint32 m_nbrBytesRead;
   const uint32 m_maxBytesToRead;
};

int main ( int argc, char** argv ) {

   Pipe pipe;
   const uint32 NBR_BYTES = 1200;
   Producer producer( pipe, NBR_BYTES );
   Consumer consumer( pipe, NBR_BYTES );

   MC2::Thread producerThread( MC2::threadCall( producer, 
                                                &Producer::produce ) );
   MC2::Thread consumerThread( MC2::threadCall( consumer, 
                                                &Consumer::consume ) );

   consumerThread.join();
   producerThread.join();

   if ( consumer.getNbrBytesRead() != producer.getNbrBytesWritten() ) {
      cout << "bytes written not the same as bytes read. "
           << endl
           << "written: " << producer.getNbrBytesWritten()
           << " read: " << consumer.getNbrBytesRead() << endl;
      MC2_ASSERT( consumer.getNbrBytesRead() == producer.getNbrBytesWritten() );
   }

   return 0;
}
