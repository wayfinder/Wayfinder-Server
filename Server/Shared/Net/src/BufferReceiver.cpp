/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "BufferReceiver.h"
#include "TCPSocket.h"
#include <netinet/in.h>
#include "IPnPort.h"

namespace {
const char* getStateString( BufferReceiver::State state ) {
   switch ( state ) {
   case BufferReceiver::ERROR:
      return "ERROR";
   case BufferReceiver::IDLE:
      return "IDLE";
   case BufferReceiver::RECEIVING:
      return "RECEIVING";
   case BufferReceiver::DONE:
      return "DONE";
   }
   return "";
}

}

struct BufferReceiver::Impl {
   Impl( TCPSocket* socket ):
      curByte( 0 ),
      curLengthByte( 0 ),
      sock( socket ),
      state( BufferReceiver::IDLE ) {
      mc2dbg2 << "[BufferReceiver]: Connected to " << socket->getPeer() << endl;
   }

   void getBuffer( BufferReceiver::BufferType& buff ) {
      // 
      // Only swap the buffers if the buffer is complete
      // i.e state = IDLE. Although this should not happen unless
      // the user of BuffeReceiver does not check the state 
      // before getting buffer.
      //
      if ( state != BufferReceiver::IDLE ) {
         return;
      }

      buffer.swap( buff );
      // once we released the buffer, clear old buffer.
      // At this point we dont know if the buffer
      // in the argument was empty or not, and
      // we can not assume it is.
      BufferReceiver::BufferType empty;
      buffer.swap( empty );
      // 
      // Once the buffer is cleared the iterator needs to
      // be reset too.
      //
      curByte = buffer.begin();
      curLengthByte = 0;
      lengthBuffer = 0;
      // Set the state in receiving mode so we can get new buffers.
      state = BufferReceiver::RECEIVING;
   }

   uint32 receive() {
      if ( state != BufferReceiver::RECEIVING ) {
         return 0;
      }
      // whether we read the length in the loop
      bool readLength = false;

      BufferReceiver::BufferType::iterator prevByte = curByte;

      //
      // Read buffer size
      //
      int32 readSize = -1;
      errno = 0;
      if ( curLengthByte < 4 ) {
         readSize = sock->read( (byte*)(&lengthBuffer) + curLengthByte, 
                                4 - curLengthByte );
         if ( readSize > 0 ) {
            curLengthByte += readSize;
         }
         readLength = true;
      } else if ( curByte != buffer.end() ) {
         //
         // Read data 
         //
         readSize = sock->read( curByte,
                                buffer.end() - curByte );
         if ( readSize > 0 ) {
            curByte += readSize;
         }
      } else {
         mc2dbg << warn << "[BufferReceiver] something is wrong! " << endl;
         state = ERROR;
         return 0;
      }

      //
      // Check error from read
      //
      if ( readSize <= 0 ) {
         if ( readSize == 0 ) {
            state = BufferReceiver::DONE;
         } else if ( errno != EAGAIN &&
              errno != EINTR ) {

            state = BufferReceiver::ERROR;

            mc2dbg << warn << "[BufferReceiver]: Connection to "
                   << sock->getPeer() << " had an error. errno: "
                   << strerror( errno ) << endl;

            if ( readLength ) {
               mc2dbg2 << "[BufferReceiver] while reading length" << endl;
            }
         }
         
      }  else if ( readLength && curLengthByte == 4 ) {
         //
         // No error and we were reading the length bytes, lets
         // create a new buffer with the length we just read
         //
         BufferReceiver::BufferType newBuff( ntohl( lengthBuffer ) );
         buffer.swap( newBuff );
         curByte = buffer.begin();
         prevByte = curByte;
         readLength = false;

      } else if ( ! readLength && curByte == buffer.end() ) {
         //
         // All data was read?, change state to idle
         // so we dont read more data until the buffer is empty
         //
         state = BufferReceiver::IDLE;
      }

      return readSize;
   }


   BufferReceiver::BufferType buffer;
   BufferReceiver::BufferType::iterator curByte;
   uint32 lengthBuffer;
   uint32 curLengthByte;
   auto_ptr<TCPSocket> sock;
   BufferReceiver::State state;
};
namespace {
const uint32 RECV_WORKING_TIMEOUT = 1000; // milliseconds
const uint32 RECV_IDLE_TIMEOUT = 60000; // milliseconds
}

BufferReceiver::BufferReceiver( TCPSocket* receivingSocket ):
   TimedSelectable( ::RECV_WORKING_TIMEOUT, ::RECV_IDLE_TIMEOUT ),
   m_impl( new Impl( receivingSocket ) )  {
   setIdle();
}

BufferReceiver::~BufferReceiver()
{
}

void BufferReceiver::receive() 
{
   m_impl->receive();
}

Selectable::selectable BufferReceiver::getSelectable() const {
   return m_impl->sock->getSelectable();
}

void BufferReceiver::getBuffer( BufferType& buffer ) 
{
   m_impl->getBuffer( buffer );
}

BufferReceiver::State BufferReceiver::getState() const 
{
   return m_impl->state;
}

void BufferReceiver::handleIO( bool readReady, bool writeReady ) 
{
   mc2dbg4 << "[BufferReceiver] handleIO(" 
           << readReady << ", " << writeReady << ")"
           << " state: " << ::getStateString( getState() )
           <<"  selectable: " << getSelectable() << endl;

   MC2_ASSERT( ! writeReady );

   if ( readReady ) {
      if ( getState() == IDLE &&
           m_impl->curLengthByte == 0 ) { // = 0 which means we havent read anything
         m_impl->state = RECEIVING;
      }
      m_impl->receive();
      // only update timeout if we received anything
      if ( getState() == RECEIVING ) {
         setWorking();
      } else if ( getState() == IDLE ) {
         setIdle();
      }
   }

   //
   // In case of error, set timeout to smallest possible
   // and let the create of this receiver take care of timeout
   // once the selector returns this as a timed out selectable
   //
   if ( m_impl->state == ERROR ||
        m_impl->state == DONE ) {
      setTimeout( 0 );
   }
 
   mc2dbg4 << "[BufferReceiver] handleIO done." << endl;
}

bool BufferReceiver::wantRead() const
{
   return true;
}

bool BufferReceiver::wantWrite() const
{
   return false;
}

void BufferReceiver::handleTimeout()
{
   mc2dbg << "[BufferReceiver]: Connection closed to " 
          << m_impl->sock->getPeer() << " - ";
   if ( getState() == ERROR ) {
      mc2dbg << "[BufferReceiver] error: ";
      
      int socket_error;
      socklen_t valSize = sizeof ( int );    
      if ( getsockopt( m_impl->sock->getSelectable(), 
                       SOL_SOCKET, SO_ERROR, 
                       &socket_error, &valSize ) >= 0) {
         mc2dbg << strerror( socket_error ) << endl;
      } else {
         mc2dbg << "Error in getsockopt!. " 
                << strerror( errno ) << endl;
      }
   } else if ( getState() != DONE ) {
      mc2dbg << "timeout." << endl;
   } else {
      mc2dbg << "done." << endl;
   }

   // timed out, change state to error
   m_impl->state = ERROR;
}


