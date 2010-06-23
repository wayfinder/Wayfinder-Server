/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "HttpInterfaceFactory.h"
#include "InterfaceParserThreadGroup.h"
#include "TCPSocket.h"
#include "SelectableSelector.h"
#include "HttpInterfaceRequest.h"
#include "HttpHeader.h"
#include "SSLSocket.h"


HttpInterfaceFactory::HttpInterfaceFactory( 
   InterfaceParserThreadGroup* group,
   const char* port )
      : InterfaceFactory( group )
{
   m_thread = new HttpInterfaceFactoryThread( port, this );
}


HttpInterfaceFactory::HttpInterfaceFactory( 
   InterfaceParserThreadGroup* group, TCPSocket* sock )
      : InterfaceFactory( group )
{
   m_thread = new HttpInterfaceFactoryThread( sock, this );
}


HttpInterfaceFactory::~HttpInterfaceFactory() {
   // Wait for m_thread to die... That should not be needed
}


void 
HttpInterfaceFactory::start() {
   m_thread->start();
}


void 
HttpInterfaceFactory::terminate() {
   m_thread->terminate();
   m_thread->notify();
}


void 
HttpInterfaceFactory::handleDoneInterfaceRequest( 
   InterfaceRequest* ireply )
{
   HttpInterfaceRequest* irep = 
      static_cast< HttpInterfaceRequest* > ( ireply );
   // Close the socket
   const MC2String USERAGENT = "User-Agent";
   
#ifdef USE_SSL
   if ( irep->isHttps() ) {
      if ( irep->m_requestHeader->getHeaderValue( &USERAGENT ) != NULL &&
           strstr( irep->m_requestHeader->getHeaderValue( &USERAGENT )
                   ->c_str(), "MSIE" ) != NULL  ) 
      {
         // Dont send anything on socket as that might be 
         // interpreted as the connection is alive and the 
         // browser sends a request and then reads the close
         // and the user gets an error. (IE browser)
         static_cast< SSLSocket* > ( 
            irep->m_reqSock )->nonWriteClose();
      }
   }
#endif
   delete irep->m_reqSock;
   // Ok it is done delete it
   delete ireply;
}


void
HttpInterfaceFactory::putNewRequest( InterfaceRequest* ireq ) {
   m_group->putInterfaceRequest( ireq );
   if ( ireq != NULL ) {
      // Overloaded
      handleDoneInterfaceRequest( ireq );
   }
}


HttpInterfaceFactory::
HttpInterfaceFactoryThread::
HttpInterfaceFactoryThread( const char* port,
                            HttpInterfaceFactory* factory ):
   ISABThread( NULL, "HttpInterfaceFactoryThread" )
{
   // Data port
   char* tmpPtr = NULL;
   int portNbr = strtol( port, &tmpPtr, 10 );
   if ( tmpPtr == NULL || (tmpPtr[ 0 ] != '\0' && tmpPtr[ 0 ] != ',') ) {
      mc2log << fatal << "HttpInterfaceFactoryThread bad port "
             << MC2CITE( port ) << " exiting." << endl;
      exit( 1 );
   }
   m_socket = new TCPSocket();

   if ( ! m_socket->open() ) {
      mc2log << fatal << "HttpInterfaceFactoryThread failed to open "
             << "socket exiting." << endl;
      exit( 1 );
   }
   uint16 foundPort = m_socket->listen( portNbr );
   if ( foundPort != portNbr ) {
      mc2log << fatal << "HttpInterfaceFactoryThread failed listen on "
             << "port " << portNbr << " exiting." << endl;
      exit( 1 );
   }
   // Set non blocking
   m_socket->setBlocking( false );

   m_selector = new SelectableSelector();
   m_selector->addSelectable( m_socket, true, false ); // Accept is read
   m_factory = factory;

   mc2dbg4 << "HttpInterfaceFactoryThread port " << portNbr << endl;
}


HttpInterfaceFactory::HttpInterfaceFactoryThread::
HttpInterfaceFactoryThread(
   TCPSocket* sock,
   HttpInterfaceFactory* factory )
      : ISABThread( NULL, "HttpInterfaceFactoryThread" )
{
   m_socket = sock;
   // Set non blocking
   m_socket->setBlocking( false );

   m_selector = new SelectableSelector();
   m_selector->addSelectable( m_socket, true, false ); // Accept is read
   m_factory = factory;
}


HttpInterfaceFactory::HttpInterfaceFactoryThread::
~HttpInterfaceFactoryThread()
{
   delete m_selector;
   delete m_socket;
}


void
HttpInterfaceFactory::HttpInterfaceFactoryThread::run() {
   const uint32 maxTimeout = MAX_UINT32;
   int res = 0;
   set<Selectable*> readReady;
   set<Selectable*> writeReady;

   IPnPort address( 0, 0 );
   if ( ! m_socket->getSockName( address.first, address.second ) ) {
      mc2dbg << "[HttpIF]: Failed to get socket name." << endl;
   }


   while ( !terminated ) {
      res = m_selector->select( maxTimeout, readReady, writeReady );
      if ( !readReady.empty() && *readReady.begin() == m_socket ) {
         // Accept all new sockets in queue
         TCPSocket* newSocket = NULL;
         do {
            newSocket = m_socket->accept();
            if ( newSocket != NULL ) {
               // Set non blocking
               newSocket->setBlocking( false );
               uint32 IP = 0;
               uint16 port = 0;
               if ( newSocket->getPeerName( IP, port ) ) {
                  mc2dbg2 << "Connection from " << prettyPrintIP( IP ) 
                          << " port " << port << endl;
                  m_factory->putNewRequest( new HttpInterfaceRequest( 
                                               newSocket, address ) );
               } else {
                  // No peer, then broken socket
                  mc2log << warn
                         << " Couldn't get peer, aborting request." <<endl;
                  delete newSocket;
               }
            }
         } while ( newSocket != NULL );
         readReady.clear();
         m_selector->addSelectable( m_socket, true, false );
      }
   } // End while !terminated

   mc2log << info << "HttpInterfaceFactoryThread ends" << endl;
}


void
HttpInterfaceFactory::HttpInterfaceFactoryThread::notify() {
   m_selector->notify();
}
