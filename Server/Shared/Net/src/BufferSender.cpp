/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "BufferSender.h"

#include "TCPSocket.h"
#include "IPnPort.h"
#include "DebugClock.h"
#include "Packet.h"

namespace {
// return state as string
MC2String getStateString( BufferSender::State state ) {
   switch ( state ) {
   case BufferSender::CONNECTING:
      return "CONNECTING";
   case BufferSender::READY:
      return "READY";
   case BufferSender::ERROR:
      return "ERROR";
   case BufferSender::DONE:
      return "DONE";
   case BufferSender::SENDING:
      return "SENDING";
   }
   return "";
}
}

struct BufferSender::Impl {
   typedef BufferSender::BufferType BufferType;
   Impl( const IPnPort& _destination )
      throw (SocketException) :
      bytesLeft( 0 ),
      curByte( 0 ),
      destination( _destination ),
      state( BufferSender::ERROR ){
      connect();
   }

   Impl( BufferType& buffer, 
         const IPnPort& _destination )
      throw (SocketException) :
      bytesLeft( buffer.size() ),
      curByte( buffer.begin() ),
      destination( _destination ),
      state( BufferSender::ERROR )  {
      // take control of the data
      setBuffer( buff );
      connect();

   }
   void reconnect() throw (SocketException) {
      sock.close();
      connect();
   }
   void connect() throw (SocketException) {
      state = CONNECTING;
      //
      // open, connect and set socket in non-blocking mode
      //
      sock.open();
      if ( ! sock.setBlocking( false ) ) {
         mc2log << fatal << "[BufferSender]: "
                << "blocking state was not changed!" << endl;
         MC2_ASSERT( false );
      }

      // Try to connect
      errno = 0;
      bool ret = sock.connect( destination.getIP(), destination.getPort() );
      if ( errno == EINPROGRESS || errno == EALREADY ) {
         mc2dbg2 << "[BufferSender] Waiting for connect to finish." << endl;
         m_bufferSendTime = DebugClock(); // take connection time
         return;
      } else if ( !ret ) {
         throw SocketException( strerror( errno ) );
      }

      // we are ready to send
      state = BufferSender::READY;



      mc2dbg2 << "[BufferSender]: connected to: " << destination << endl;

   }

   void setBuffer( BufferType& newBuff ) {
      buff.swap( newBuff );
      curByte = buff.begin();
      bytesLeft = buff.size();
   }

   void send() {

      // dont do anything unless we are in sending state
      if ( state != BufferSender::SENDING ) {
         return;
      }

      int32 written = sock.write( curByte, bytesLeft );

      if ( written < 0 ) {
         if ( errno != EAGAIN ) {

            state = BufferSender::ERROR;

            mc2log << warn << "[BufferSender]: Connection to "  
                   << destination << " had an error: "
                   << strerror( errno ) << endl;
         } // else write again
         
      } else {
         bytesLeft -= written;
         curByte += written;

         // make sure write size is ok
         MC2_ASSERT( curByte <= buff.end() );

         if ( bytesLeft == 0 ) {
            state = BufferSender::DONE;
         }
      }


      if ( state == BufferSender::DONE ) {
         mc2dbg2 << "[BufferSender] Buffer to destination: " 
                 << destination << " sent." << endl;
            
      }
   }


   BufferType buff;
   uint32 bytesLeft;
   byte* curByte;
   IPnPort destination;
   TCPSocket sock;
   BufferSender::State state;

   DebugClock m_bufferSendTime; //< time to send a packet
};

namespace {
const uint32 SEND_WORKING_TIMEOUT = 1000; // milliseconds
const uint32 SEND_IDLE_TIMEOUT = 60000; // milliseconds
}

BufferSender::BufferSender( const IPnPort& destination )
   throw (SocketException) :
   TimedSelectable( ::SEND_WORKING_TIMEOUT, ::SEND_IDLE_TIMEOUT ),
   m_impl( new Impl( destination ) ) {
}

BufferSender::~BufferSender() 
{
}

void BufferSender::setBuffer( BufferType& buffer ) 
{
   MC2_ASSERT( m_impl->state == READY ||
               m_impl->state == DONE );
   MC2_ASSERT( buffer.size() != 0 );

   m_impl->setBuffer( buffer );
   m_impl->state = SENDING;
   resetTimeout();
}

void BufferSender::send() {
   m_impl->send();
}

void BufferSender::reconnect() throw (SocketException) 
{
   //
   // try to reconnect and if it succeeds
   // restart the buffer send from the beginning
   //
   MC2_ASSERT( false );
   m_impl->reconnect();   
   m_impl->setBuffer( m_impl->buff );
}

Selectable::selectable 
BufferSender::getSelectable() const {
   return m_impl->sock.getSelectable();
}

BufferSender::State 
BufferSender::getState() const {
   return m_impl->state;
}

const IPnPort& 
BufferSender::getDestination() const {
   return m_impl->destination;
}


void BufferSender::handleIO( bool readyRead, bool readyWrite ) 
{
   mc2dbg2 << "[BufferSender]: handleIO(" 
           << readyRead << ", " << readyWrite << ")" 
           << " state: " << ::getStateString( getState() )
           << " selectable: " << getSelectable() << endl;
   //
   // if we got read signal, which indicates error
   //
   if ( readyRead ) {
      m_impl->state = ERROR;
   } else if ( readyWrite ) {
      if ( getState() == CONNECTING ) {
         if ( m_impl->m_bufferSendTime.getTime() >= 1000 ) {
            mc2dbg << warn << "[BufferSender] Took " 
                   << m_impl->m_bufferSendTime << " to connect to "
                   << " destination " << getDestination() << endl;
         }
         m_impl->state = READY;
         mc2dbg2 << "[BufferSender]: connected to: " << getDestination() 
                 << endl;
      }

      // start send clock if we start writting
      if ( m_impl->bytesLeft == m_impl->buff.size() ) {
         m_impl->m_bufferSendTime = DebugClock();
      }
      //
      // send the buffer and reset timeout 
      //
      setWorking();
      send();


      if ( m_impl->state == DONE ) {
         // if send time is larger than 1 second print info
         if ( m_impl->m_bufferSendTime.getTime() >= 1000 ) {

            // note: should probably use timer stuff in MultiPacketSender or
            // something else, since this class is not specific to Packets 
            // (thus the extra size check here)
            if ( m_impl->buff.size() > 4 ) {
               Packet packet( m_impl->buff.data() + 4, m_impl->buff.size() - 4,
                              true );
               mc2dbg << warn << "[BufferSender] It took " 
                      << m_impl->m_bufferSendTime << " to send buffer."
                      <<  " Packet type: " 
                      << packet.getSubTypeAsString() << endl;
            }
         }
         // start idle timer now when we got nothing to do
         setIdle();
      }

   } else {
      // woohaa, would be very bad....and strange.
      MC2_ASSERT( readyRead && readyWrite );
   }

   // if state is error
   // set timers to 0 and let the "creator" 
   // of this selectable take care of it.
   if ( m_impl->state == ERROR ) {
      mc2dbg2 << "[BufferSender] socket error. socket:" << getSelectable() << endl;
      setTimeout( 0 );
   }

}

void BufferSender::handleTimeout()
{

   mc2dbg2 << "[BufferSender] Connection closed to " 
          << getDestination()
          << " - ";
   if ( getState() == ERROR ) {
      mc2dbg << "[BufferSender] error: ";
      //
      // Get socket error
      // 
      int socket_error;
      socklen_t valSize = sizeof ( int );    
      if ( getsockopt( m_impl->sock.getSelectable(), 
                       SOL_SOCKET, SO_ERROR, 
                       &socket_error, &valSize ) >= 0) {
         mc2dbg << strerror( socket_error ) << endl;
      } else {
         mc2dbg << "Error in getsockopt!. " 
                << strerror( errno ) << endl;
      }
   } else {
      mc2dbg2 << "timeout." << endl;
   }
   

   // shutdown socket
   m_impl->state = ERROR;
}

bool BufferSender::wantRead() const 
{
   // we want to read so we can close the socket
   // when the remote socket closed it.
   // unless we have an error, then we just want
   // to use the timeout to delete this socket.
   return m_impl->state != ERROR;
}

bool BufferSender::wantWrite() const
{
   // ...unless we had an error
   return m_impl->state != ERROR;
}
