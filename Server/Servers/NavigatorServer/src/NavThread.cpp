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

#include "NavThread.h"

#include "isabBoxSession.h"
#include "LogBuffer.h"

#include "NavInterfaceRequest.h"
#include "NavParserThreadGroup.h"
#include "NavRequestHandler.h"
#include "STLStringUtility.h"
// Http stuff
#include "HttpHeader.h"
#include "HttpBody.h"
#include "HttpInterfaceRequest.h"
#include "HttpCodes.h"

#include "isabBoxInterfaceRequest.h"

#include "NavMessage.h"
#include "NavMessageFactory.h"



NavThread::NavThread( uint32 id,
                      NavParserThreadGroup* group )
      : InterfaceParserThread( group, "NavThread" ),
        m_navHandler( new NavRequestHandler( this, group ) )
{
   m_id = id;
   m_server = group;
   m_session = NULL;
}


NavThread::~NavThread() {
   delete m_navHandler;
}


int
NavThread::getID() const {
   return m_id;
}


void 
NavThread::handleInterfaceRequest( InterfaceRequest* ireq ) {
//   mc2dbg << "NavThread::handleInterfaceRequest " << ireq << endl;
   // If HttpInterfaceRequest then use if to construct a 
   // NavInterfaceRequest
   IsabBoxInterfaceRequest* delreq = NULL;
   HttpInterfaceRequest* hreq = NULL;
   HttpHeader* outHead = NULL;
   HttpBody* outBody = NULL;

   if ( dynamic_cast< HttpInterfaceRequest* > ( ireq ) ) {
      hreq = static_cast< HttpInterfaceRequest* > ( ireq );
      outHead = new HttpHeader();
      outBody = new HttpBody();
      delreq = new IsabBoxInterfaceRequest( hreq, outHead, outBody );
      if ( hreq->getState() == InterfaceRequest::Ready_To_IO_Reply ) {
         mc2log << warn << delreq->getSession()->m_logPrefix
                << "NT::handleInterfaceRequest HTTP error, "
                << "not processing request" << endl;
         delete delreq;
         delete outBody;
         delete outHead;
         return;
      }
      ireq = delreq;
   }
   
   MC2_ASSERT( dynamic_cast< NavInterfaceRequest* > ( ireq ) );
   m_ireq = static_cast< NavInterfaceRequest* > ( ireq );
   if ( m_ireq->getProtoVer() >= 0xa ) {
      // handle request
      m_navHandler->handleRequest( m_ireq );

      // setup reply
      m_ireq = NULL;
   } else {
      // Too old, no longer supported
      NavMessage* mess = m_ireq->getRequestMessage();
      if ( mess != NULL ) {
         NavMessageFactory* factory = NavMessageFactory::instance(
            *mess->getAddress() );
         OutGoingNavMessage* reply = factory->createStatusReply(  
            *mess->getAddress(),
            NavMessage::replyType( mess->getType() ),
            NavMessage::NAV_STATUS_PROTOVER_NOT_SUPPORTED,
            mess->getSession(),
            mess->getProtoVer() );
            
         mc2log << error << "[NavThread " << m_id << "] too "
                << "low protover, sending error."
                << endl;
         m_ireq->setReplyMessage( reply );
      } else {
         mc2log << error << "[NavThread " << m_id << "] too "
                << "low protover and not know request, dropping connection."
                << endl;
      }
      mess->getSession()->setCallDone( 1 ); // No more on this
   }

   if ( hreq != NULL ) {
      setHttpReply( delreq, outHead, outBody, hreq );
      delete delreq;
   }
   m_peerIP = 0;
   m_ireq = NULL;
   m_session = NULL;
   static_cast<LogBuffer*>( mc2log.rdbuf() )->setPrefix( "" );
}


void
NavThread::setHttpReply( IsabBoxInterfaceRequest* ireq,
                         HttpHeader* outHead, HttpBody* outBody,
                         HttpInterfaceRequest* hreq )
{
   // Set reply in hreq
   const MC2String CONTENTLENGTH = "Content-Length";
   MC2String* length = new MC2String();
   STLStringUtility::uint2str( ireq->getReplySize(), *length );
   outHead->addHeaderLine( &CONTENTLENGTH, length );
   outBody->setBody( ireq->getReplyData(), 
                     ireq->getReplySize()  );
   if ( ireq->getReplySize() != 0 ) {
      hreq->setReply( outHead, outBody );
   } else {
      mc2log << warn << ireq->getSession()->m_logPrefix
             << "setHttpReply setting 404 reply" << endl;
      hreq->setStatusReply( HttpCode::NOT_FOUND );
      delete outHead;
      delete outBody;
   }
   if ( ireq->getSession() && ireq->getSession()->getCallDone() ) {
      hreq->terminate();
   }
}


const char* 
NavThread::getServerType() {
   return "NS";
}
