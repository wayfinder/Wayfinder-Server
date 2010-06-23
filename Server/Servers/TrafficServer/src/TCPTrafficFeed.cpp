/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TCPTrafficFeed.h"

#include "TCPSocket.h"
#include "SelectableSelector.h"

struct TCPTrafficFeed::Impl: public TrafficFeed {
   explicit Impl( uint32 port,
                  Selectable& shutdownNotifier );
   ~Impl();

   bool eos() const;
   bool getData( Data& data );

   /**
    * Opens the socket in listening mode. The socket will be listening
    * on the port specified in the portNumber argument of the
    * constructor.
    * @param port 
    * @return True if successful.
    */
   bool openListeningSocket( uint32 port );

   /// Socket for receiving data
   TCPSocket* m_socket;

   /// The selectable that will be notified when the thread work should
   /// shutdown.
   Selectable& m_shutdownNotifier;
   bool m_eos; ///< End of stream.
};

TCPTrafficFeed::TCPTrafficFeed( uint32 port,
                                Selectable& shutdownNotifier ):
   m_impl( new Impl( port, shutdownNotifier ) ) {
}

TCPTrafficFeed::~TCPTrafficFeed() {
   delete m_impl;
}

bool TCPTrafficFeed::eos() const {
   return m_impl->eos();
}

bool TCPTrafficFeed::getData( Data& data ) {
   return m_impl->getData( data );
}


//
// Implementation details
//

TCPTrafficFeed::Impl::Impl( uint32 port,
                            Selectable& shutdownNotifier  ):
   m_socket( new TCPSocket() ),
   m_shutdownNotifier( shutdownNotifier ),
   m_eos( false ) {

   if ( ! openListeningSocket( port ) ) {
      //      throw SocketException();
      MC2_ASSERT( false );
   }

}

TCPTrafficFeed::Impl::~Impl() {
   delete m_socket;
}


bool TCPTrafficFeed::Impl::eos() const {
   return m_eos;
}


bool TCPTrafficFeed::Impl::openListeningSocket( uint32 port ) {

   if ( ! m_socket->open() ) {
      mc2log << error << "[TCPTrafficFeed] Failed to open socket!" << endl;
      return false;
   }

   if ( ! m_socket->listen( port, TCPSocket::GENERIC ) ){
      mc2log << error << "[TCPTrafficFeed] to listen on " << port << endl;
      return false;
   }

   m_socket->setBlocking( false );

   return true;
}

namespace {

/**
 * Checks if there is something on the socket or if we get a shutdown from
 * the server.
 *
 * @param notify A notify selectable that indicates shutdown or some other kind
 *               of stop procedure.
 * @param socket The socket.
 * @return Return false if we get a shutdown, true else. Which means there
 *         something on the socket.
 */
bool checkSelectable( Selectable& notify, Selectable& socket ) {
   SelectableSelector selector;
   // set up the selector
   selector.addSelectable( &notify,
                           true, false, // read, not write
                           false );     // no notify
   selector.addSelectable( &socket,
                           true, false, // read, not write
                           false );     // no notify

   // wait for read to be ready 
   SelectableSelector::selSet readReady, writeReady;
   selector.select( -1, readReady, writeReady );

   // if we find shutdown...then ..shutdown...
   if ( readReady.find( &notify ) != readReady.end() ) {
      return false;
   } 
   // else there is something on the socket
   return true;
}

bool acceptAndReadData( Selectable& notify, TCPSocket& socket,
                        MC2String& data ) {
   // accept the connection
   auto_ptr<TCPSocket> dataSocket( socket.accept() );

   if ( ! dataSocket.get() ){
      mc2log << error << "Accept failed!" << endl;
      return false;
   }

   char readBuffer[100000];
   const size_t bufferSize = sizeof( readBuffer ) - 1;
   size_t readBytes;
   do {
      // see if there is anything on the socket.
      if( !checkSelectable( notify, *dataSocket ) ) {
         return false;
      }
      // read from the socket
      readBytes = dataSocket->readMaxBytes( (byte*)readBuffer, bufferSize );
      if ( readBytes > 0 ) {
         readBuffer[ readBytes ] = 0;
         data += readBuffer;
      }
   } while( ( readBytes != MAX_UINT32 ) && ( readBytes > 0 ) );

   return true;
}

} // anonymous

bool TCPTrafficFeed::Impl::getData( Data& data ) {
   // get from the server
   mc2log << " Waiting for new file" << endl;

   // see if there is anything on the socket.
   if( ! ::checkSelectable( m_shutdownNotifier, *m_socket ) ) {
      m_eos = true;
      return false;
   }

   if ( ! ::acceptAndReadData( m_shutdownNotifier, *m_socket,
                               data ) ) {
      // Failed to read data here is "fatal".
      m_eos = true;
      return 0;
   }
   return true;
}
