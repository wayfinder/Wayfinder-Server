/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TcpInterfaceFactory.h"
#include "InterfaceParserThreadGroup.h"
#include "DataSocket.h"
#include "isabBoxSession.h"
#include "SelectableSelector.h"
#include "isabBoxInterfaceRequest.h"
#include "HttpInterfaceRequest.h"


TcpInterfaceFactory::TcpInterfaceFactory( 
   InterfaceParserThreadGroup* group,
   const char* dataPort,
   bool httpReq )
      : NavInterfaceFactory( group )
{
   m_thread = new TcpInterfaceFactoryThread( dataPort, this, httpReq );
}


TcpInterfaceFactory::TcpInterfaceFactory( 
   InterfaceParserThreadGroup* group,
   int fd, bool httpReq )
      : NavInterfaceFactory( group )
{
   m_thread = new TcpInterfaceFactoryThread( fd, this, httpReq );
}


TcpInterfaceFactory::~TcpInterfaceFactory() {
   // Wait for m_thread to die... That should not be needed
}


void 
TcpInterfaceFactory::start() {
   m_thread->start();
}


void 
TcpInterfaceFactory::terminate() {
   mc2log << info << "TcpInterfaceFactory terminate" << endl;
   m_thread->terminate();
   m_thread->notify();
}


void 
TcpInterfaceFactory::handleDoneInterfaceRequest( InterfaceRequest* ireply )
{
   SelectableInterfaceRequest* req = 
      static_cast<SelectableInterfaceRequest*>(ireply);
   DataSocket* sock = dynamic_cast<DataSocket*> ( req->getSelectable() );
   if (sock != NULL) {
      // Delete the session
      delete sock->getSession();
   }
   // Close the socket
   delete req->getSelectable();
   // Ok it is done delete it
   delete ireply;
}


void
TcpInterfaceFactory::putNewRequest( InterfaceRequest* ireq ) {
   m_group->putInterfaceRequest( ireq );
   if ( ireq != NULL ) {
      // Overloaded
      handleDoneInterfaceRequest( ireq );
   }
}


TcpInterfaceFactory::TcpInterfaceFactoryThread::TcpInterfaceFactoryThread(
   const char* dataPort,
   TcpInterfaceFactory* factory, 
   bool httpReq )
      : ISABThread( NULL, "TcpInterfaceFactoryThread" )
{
   m_httpReq = httpReq;
   // Data port
   char* tmpPtr = NULL;
   int port = strtol( dataPort, &tmpPtr, 10 );
   if ( tmpPtr == NULL || (tmpPtr[ 0 ] != '\0' && tmpPtr[ 0 ] != ',') ) {
      mc2log << fatal << "TcpInterfaceFactoryThread bad port "
             << MC2CITE( dataPort ) << " exiting." << endl;
      exit( 1 );
   }
   m_socket = new DataSocket( new isabBoxSession() );

   mc2log << info << "Trying to get port " << port << "..." << endl;
   bool opened = m_socket->listenDuration( port, 10/*10s*/ );

   if ( !opened ) {
      mc2log << fatal << "TcpInterfaceFactoryThread failed listen on port "
             << port << " exiting." << endl;
      exit( 1 );
   }

   // Set non blocking
   m_socket->setBlocking( false );
      
   m_selector = new SelectableSelector();
   m_selector->addSelectable( m_socket, true, false ); // Accept is read
   m_factory = factory;

   mc2log << info << "TcpInterfaceFactoryThread listening on " 
          << (httpReq ? "HTTP " : "") << "port " << port << endl;
}


TcpInterfaceFactory::TcpInterfaceFactoryThread::TcpInterfaceFactoryThread( 
   int fd,
   TcpInterfaceFactory* factory,
   bool httpReq )
{
   m_httpReq = httpReq;

   m_socket = new DataSocket( new isabBoxSession(), fd, DEFAULT_BACKLOG );
   m_selector = new SelectableSelector();
   m_selector->addSelectable( m_socket, true, false ); // Accept is read
   m_factory = factory;

   if ( !m_socket->listen( 80/*Not used*/, TCPSocket::GENERIC, false ) ) {
      mc2log << fatal << "TcpInterfaceFactoryThread failed to listen "
             << "on supplied socket, exiting." << endl;
      exit( 1 );
   }
   // Set non blocking
   m_socket->setBlocking( false );

   uint32 sIP = 0;
   uint16 sPort = 0;
   m_socket->getSockName( sIP, sPort );
   mc2log << info << "TcpInterfaceFactoryThread listening on supplied "
          << (httpReq ? "HTTP " : "") << "socket. Port " << sPort << endl;
}


TcpInterfaceFactory::TcpInterfaceFactoryThread::
~TcpInterfaceFactoryThread()
{
   delete m_selector;
   delete m_socket->getSession();
   delete m_socket;
}


void
TcpInterfaceFactory::TcpInterfaceFactoryThread::run() {
   const uint32 maxTimeout = MAX_UINT32;
   int res = 0;
   set<Selectable*> readReady;
   set<Selectable*> writeReady;

   while ( !terminated ) {

      res = m_selector->select( maxTimeout, readReady, writeReady );
      if ( !readReady.empty() && *readReady.begin() == m_socket ) {
         // Accept all new sockets in queue
         DataSocket* newSocket = NULL;
         do {
            newSocket = static_cast<DataSocket*> ( m_socket->accept() );
            if ( newSocket != NULL ) {
               // Set non blocking
               newSocket->setBlocking( false );
               uint32 IP = 0;
               uint16 port = 0;

               if ( newSocket->getPeerName( IP, port ) ) {
                  mc2dbg2 << "Connection from " << prettyPrintIP( IP ) 
                          << " port " << port << endl;
                  // SET new socket TO HAVE OWN SESSION!!!!
                  newSocket->setSession( new isabBoxSession() );
                  isabBoxSession *session = newSocket->getSession();
                  session->beginSession();
                  session->setSocketBusy();
                  session->setPeerIP( IP, port );
                  uint32 sIP = 0;
                  uint16 sPort = 0;
                  newSocket->getSockName( sIP, sPort);
                  mc2log << info << "Local endpoint is " 
                         << prettyPrintIP(sIP) << ":" << sPort 
                         << " Peer is " << prettyPrintIP(IP) << ":" << port
                         << endl;
                  InterfaceRequest* ireq = NULL;
                  if ( m_httpReq ) {
                     ireq = new HttpInterfaceRequest( 
                        newSocket, IPnPort( sIP, sPort ) );
                  } else {
                     ireq = new IsabBoxInterfaceRequest( newSocket );
                  }
                  m_factory->putNewRequest( ireq );
               } else {
                  // No peer, then broken socket
                  mc2log << warn
                         << " Couldn't get peer, aborting request." <<endl;
                  // No, no, not delete newSocket->getSession(); As it is 
                  // m_socket's session...
                  delete newSocket;
               }
            }
         } while ( newSocket != NULL );
         readReady.clear();
         writeReady.clear();
         m_selector->addSelectable( m_socket, true, false );
      }
   } // End while !terminated

   mc2log << info << "TcpInterfaceFactoryThread ends" << endl;
}


void
TcpInterfaceFactory::TcpInterfaceFactoryThread::notify() {
   m_selector->notify();
}
