/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ThreadRequestHandler.h"
#include "ParserThreadGroup.h"
#include "PushPacket.h"


ThreadRequestHandler::ThreadRequestHandler(
                        GenericServer* server, 
                        ParserThreadGroupHandle threadGroup )
      : m_server( server ),
        m_group( threadGroup ),
        m_running( true )
{
   m_requestThread = new RequestThread( threadGroup.get(), this );
   m_pushReceiveThread = NULL;
   if ( m_server->getPushService() ) {
      m_pushReceiveThread = new PushReceiveThread( threadGroup.get(), this ); 
   }
}


ThreadRequestHandler::~ThreadRequestHandler() 
{
   ISABThreadHandle rt = m_requestThread;
   ISABThreadHandle pt = m_pushReceiveThread;

   rt->terminate();
   if ( m_pushReceiveThread != NULL ) {
      pt->terminate();
   }

   m_server->notifyAnswer();

   // Wait for m_requestThread to terminate
   mc2dbg << "[ThreadRequestHandler] Wait for m_requestThread to terminate" << endl;
   while ( rt->isAlive() || (pt.get() != NULL && pt->isAlive()) ) {
      ISABThread::yield();
   }

   mc2dbg << "[ThreadRequestHandler] dead." << endl;
}

void
ThreadRequestHandler::start() 
{
   m_requestThread->start();
   if ( m_pushReceiveThread != NULL ) {
      m_pushReceiveThread->start();
   }
}


void 
ThreadRequestHandler::terminate() {
   m_running = false;
}


void 
ThreadRequestHandler::putPushPacket( PushPacket* packet, uint32 serviceID, 
                                     SubscriptionResource* resource )
{
   if ( m_running ) {
      m_group->putPushPacket( packet, serviceID, resource );
   } else {
      delete packet;
      delete resource;
   }
}


ThreadRequestHandler::
RequestThread::RequestThread( ISABThreadGroup* group,
                              ThreadRequestHandler* handler ):
   ISABThread( group, "ThreadRequestHandler::RequestThread" ),
   m_handler( handler )
{
}


ThreadRequestHandler::RequestThread::~RequestThread() 
{

}


void
ThreadRequestHandler::RequestThread::run() {
   RequestContainer* rc = NULL;

   while( !terminated ) {
      rc = m_handler->getAnswer();
      if ( rc != NULL ) {
         if ( dynamic_cast< ThreadRequestContainer* > ( rc ) != NULL ) {
            m_handler->putAnswer(static_cast<ThreadRequestContainer*>(rc));
         } else {
            MC2ERROR( "ThreadRequestHandler::RequestThread::run not "
                      "ThreadRequestContainer answer!" );
            delete rc->getRequest();
            delete rc;
         }
      }
   }
   mc2dbg << "[ThreadRequestHandler::RequestThread] end." << endl;
}


void 
ThreadRequestHandler::putAnswer( RequestContainer* rc ) 
{
   m_group->putAnswer( dynamic_cast<ThreadRequestContainer*>(rc) );
}


ThreadRequestHandler::
PushReceiveThread::PushReceiveThread( ISABThreadGroup* group,
                                      ThreadRequestHandler* handler ):
   ISABThread( group, "ThreadRequestHandler::PushReceiveThread" ),
   m_handler( handler )
{
}


ThreadRequestHandler::PushReceiveThread::~PushReceiveThread() {
}


void
ThreadRequestHandler::PushReceiveThread::run() {
   uint32 serviceID = 0;
   SubscriptionResource* resource = NULL;
   PushPacket* packet = NULL;

   while ( !terminated ) {
      packet = m_handler->getPushPacket( serviceID, resource );
      if ( packet != NULL ) {
         m_handler->putPushPacket( packet, serviceID, resource );
      }
   }  
}
