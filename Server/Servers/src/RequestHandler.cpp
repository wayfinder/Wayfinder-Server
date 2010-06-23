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

#include "RequestHandler.h"
#include "PacketResendHandler.h"
#include "Packet.h"
#include "RequestTime.h"
#include "DebugClock.h"

PacketContainer* 
RequestHandler::getPacketAnswer() {
   return m_handler->getAnswer();
}


RequestHandler::RequestHandler( PacketResendHandler* handler, 
                                ServerTypes::servertype_t serverType ):
   m_requestThread( NULL ),
   m_running( false ),
   m_handler( handler ),
   m_requestList( new RequestList() ),
   m_answerList( new RequestList() ),
   m_serverType( serverType ) {

}


RequestHandler::~RequestHandler() {
   if ( m_running ) {
      m_running = false;

      ISABThreadHandle getHandle = m_requestThread;
      m_requestThread->terminate();

      notifyAnswer();
      m_handler->notifyAnswer();

      // Wait for m_requestThread to exit m_handler so it can be deleted
      while ( getHandle->isAlive() ) {
         ISABThread::yield();
      }
   }

   delete m_handler;
   delete m_requestList;
   delete m_answerList;
}


void RequestHandler::start() {
   m_running = true;
   m_requestThread = new RequestThread( this );
   m_requestThread->start();
}

RequestContainer* 
RequestHandler::getAnswer() {
   ISABSync sync( m_answerMonitor );
   
   if ( m_answerList->getCardinal() == 0 ) {
      try {
         m_answerMonitor.wait();
      }
      catch(const JTCInterruptedException &) {
      }
   }

   if ( m_answerList->getCardinal() > 0 ) {
      return m_answerList->removeFirst();
   } else {
      return NULL;
   }
}


void 
RequestHandler::notifyAnswer() {
  ISABSync sync( m_answerMonitor );

  m_answerMonitor.notifyAll();
}


void
RequestHandler::insert( RequestContainer* rc ) {
   ISABSync sync( m_monitor );

   // Zero the processingtime please.
   rc->getRequest()->setProcessingTime(0);
   
   if ( rc->getRequest()->requestDone() ) {
      // not inserted return to user, please
      addAnswer( rc );
   } else {
      m_requestList->insert( rc );
      // Send all initiall packets
      PacketContainer* cont = NULL;

      {
         DebugClock clock;
         cont = rc->getRequest()->getNextPacket();
         rc->getRequest()->setTime( RequestTime::getNextPacket, 
                                    clock.getTime() );
      }

      while ( cont != NULL ) {
         // set identification fields in the packet
         cont->getPacket()->setRequestOriginator(m_serverType);
         cont->getPacket()->setRequestTimestamp(rc->getRequest()->getTimestamp());
         rc->getRequest()->incNbrSentPackets();
         m_handler->addAndSend( cont );


         DebugClock clock;
         cont = rc->getRequest()->getNextPacket();
         // add time for getNextPacket
         rc->getRequest()->addTime( RequestTime::getNextPacket, 
                                    clock.getTime() );

      }
      bool noPackets = (rc->getRequest()->getNbrSentPackets() - // No packets
                        rc->getRequest()->getNbrReceivedPackets()) <= 0 ;
      if ( rc->getRequest()->requestDone() || noPackets ) {
         if ( noPackets ) {
            mc2dbg << "[RequestHandler] No packets" << endl;
         }
         // We can actually be done already.
         // For example an error condition before
         // any packets were sent.
         m_requestList->remove( rc );
         addAnswer( rc );
      }
   }
}


void 
RequestHandler::addAnswer( RequestContainer* rc ) {
   ISABSync sync( m_answerMonitor );

   m_answerList->insert( rc );
   m_answerMonitor.notifyAll();
}


RequestContainer* 
RequestHandler::processPacket( PacketContainer* cont ) {
   ISABSync sync( m_monitor );
   RequestContainer* rc = NULL;

   uint16 reqID = cont->getPacket()->getRequestID();

   rc = m_requestList->find( reqID );

   if ( rc != NULL ) {
      Request& request = *rc->getRequest();
      // Increase the number of resent packets if any.
      request.incNbrResentPackets( cont->getServerResend() );
      // Increase processing time for the request.
      request.setProcessingTime( request.getProcessingTime() +
                                 cont->getPacket()->getDebInfo() );
      // Increase the number of received packets
      request.incNbrReceivedPackets();
      // Increase the number of received bytes
      request.addNbrReceivedBytes( cont->getPacket()->getLength() );

      // This may be removed in release versions if too slow
      request.addProcessingTime( cont->getModuleType(),
                                 cont->getPacket()->getDebInfo() );

      request.addTotalResentNbr( cont->getPacket()->getResendNbr() );

      mc2dbg8 << "Module type in pc "
              << uint32(cont->getModuleType()) << endl;
      
      request.addTime( RequestTime::totalCPU,
                       cont->getPacket()->getCPUTime() );
                       
      // Process the packet.
      {
         DebugClock clock;
         request.processPacket( cont );
         request.addTime( RequestTime::processPacket, 
                          clock.getTime() );
      }

      PacketContainer* c = NULL;
      { 
         DebugClock clock;
         c = rc->getRequest()->getNextPacket();
         rc->getRequest()->addTime( RequestTime::getNextPacket,
                                    clock.getTime() );
      }

      while ( c != NULL ) {
         // set identification fields in the packet
         c->getPacket()->setRequestOriginator(m_serverType);
         c->getPacket()->setRequestTimestamp(rc->getRequest()->getTimestamp());
         rc->getRequest()->incNbrSentPackets();
         m_handler->addAndSend( c );
         // time getNextPacket
         DebugClock clock;
         c = rc->getRequest()->getNextPacket();
         rc->getRequest()->addTime( RequestTime::getNextPacket, clock.getTime() );
      }  
      if ( rc->getRequest()->requestDone() ||
           (rc->getRequest()->getNbrSentPackets() - // No packets
            rc->getRequest()->getNbrReceivedPackets()) <= 0 ) 
      {
         m_requestList->remove( rc );
         return rc;
      } else {
         return NULL;
      }
   } else {
      DEBUG2( cerr << "RequestHandler::processPacket( " 
              << cont->getPacket()->getRequestID() << ", "
              << cont->getPacket()->getPacketID() 
              << " not found" << endl; );
      delete cont;
      return NULL;
   }
}


RequestContainer* 
RequestHandler::removeRequest( uint16 reqID ) {
   ISABSync sync( m_monitor );
   
   return m_requestList->findAndRemove( reqID );
}


RequestContainer* 
RequestHandler::checkDoneRequest() {
   ISABSync sync( m_monitor );

   return m_requestList->findAndRemoveDone();
}


//**********************************************************************
// RequestThread
RequestHandler::
RequestThread::RequestThread( RequestHandler* handler ):
   ISABThread( NULL, "RequestHandler::RequestThread" )
{
	m_handler = handler;
}


RequestHandler::RequestThread::~RequestThread() {
}


void RequestHandler::RequestThread::run() {
   RequestContainer* rc = NULL;
   PacketContainer* pc = NULL;

   while( !terminated ) {
      pc = m_handler->getPacketAnswer();
      if ( pc != NULL ) {

         PacketContainer* cont = pc->getAnswer();
         if ( cont == NULL ) {
            // Take timeout (mostly InfoModule) packet instead.
            cont = pc->newTimeoutContainer();
         }
         if ( cont != NULL ) {

            cont->setModuleType( pc->getModuleType() );
            rc = m_handler->processPacket( cont );
            if ( rc != NULL ) {
               // Return done request
               m_handler->addAnswer( rc );
              
            } // else not done
         } else { // No answer for packet -> remove and return request
            rc = m_handler->removeRequest( 
               pc->getPacket()->getRequestID() );
            if ( rc != NULL ) {
               // Increase the number of resent packets if any.
               rc->getRequest()->incNbrResentPackets( 
                  pc->getServerResend() );
               // Return request with failed packet
               m_handler->addAnswer( rc );
            }

         }
         delete pc;
      } else { // Perhaps done requests in list?
         rc = m_handler->checkDoneRequest();
         while ( rc != NULL ) {
            m_handler->addAnswer( rc );
            rc = m_handler->checkDoneRequest(); 
         }
      }
   }
   mc2dbg << "[RequestHandler]::RequestThread::run ends" << endl;
}
