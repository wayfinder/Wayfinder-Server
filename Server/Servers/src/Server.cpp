/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Server.h"


Server::Server( const MC2String& name,
                CommandlineOptionHandler* coh,
                ServerTypes::servertype_t type ):
   GenericServer( name, coh, type )
{
}


Server::~Server() {
   m_running = false;
   m_putThread->terminate();

   DEBUG8(cerr << "Server deleted" << endl;);
}

void Server::init() {
   m_putThread = new PutThread( this );
   m_getThread = new GetThread( this );

   m_running = true;

   m_putThread->start();
   m_getThread->start();

   GenericServer::init();
}

void Server::start() {

   // Start Request and Packet handlers
   GenericServer::start();
   GenericServer::callGotoWork();
}


//**********************************************************************
// PutThread
Server::PutThread::PutThread(Server *server ) {
   m_server = server;
}


Server::PutThread::~PutThread() {
}


void Server::PutThread::run() {
   RequestContainer* rc = NULL;
   
   while( !terminated ) {
      rc = m_server->getAnswer();
      if ( rc != NULL ) {
         m_server->putAnswer( rc );
      }
   }
   DEBUG1(cerr << "Server::PutThread::run ends" << endl;);
}
				

//**********************************************************************
// GetThread
Server::GetThread::GetThread(Server *server) {
   m_server = server;
}


Server::GetThread::~GetThread() {
}


void Server::GetThread::run() {
   RequestContainer *req;
   while(!terminated) {
      req = m_server->getRequest();
      
      if (req != NULL) {
         DEBUG8(cerr << "GetThread::Inserting request in list, ReqID: "
                << req->getRequest()->getID() << endl;);
         m_server->insertRequest(req);
      } else {
         DEBUG8(cerr << "GetThread:: got request == NULL. Does nothing"
                << endl;);
      }
   }
   DEBUG1(cerr << "Server::GetThread::run ends" << endl;);
}
