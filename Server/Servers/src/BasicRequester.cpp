/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "BasicRequester.h"

#include "RequestHandler.h"
#include "PacketResendHandler.h"

#include "RequestContainer.h"
#include "ServerPort.h"
#include "ThreadRequestContainer.h"

#include "MultiRequest.h"
#include "MultiplePacketsRequest.h"

#include "SinglePacketRequest.h"
#include "ThreadRequestInfo.h"

#include "DebugClock.h"

struct BasicRequester::Impl {
   explicit Impl( ServerTypes::servertype_t type );
   ~Impl();

   void start();
   void stop();
   void putRequest( RequestContainer* rc );
   RequestContainer* getAnswer();

   ServerTypes::servertype_t m_type;
   uint32 m_currentRequestID; ///< current request ID.
   PacketResendHandler* m_resender; ///< Handles resends of packets.
   RequestHandler* m_requestor; ///< Send the requests.

};

BasicRequester::Impl::Impl( ServerTypes::servertype_t type ):
   m_type( type ),
   m_currentRequestID( 1 ),
   m_resender( new PacketResendHandler( ServerPort::getPort( type ) ) ),
   m_requestor( new RequestHandler( m_resender, type ) ) {
}

BasicRequester::Impl::~Impl() {
   delete m_requestor;
}

void BasicRequester::Impl::start() {
   m_requestor->start();
   m_resender->start();
}

void BasicRequester::Impl::stop() {
   m_resender = NULL;
   delete m_requestor;
   m_requestor = NULL;
}

void BasicRequester::Impl::putRequest( RequestContainer* rc ) {
   m_requestor->insert( rc );
}

RequestContainer* BasicRequester::Impl::getAnswer() {
   return m_requestor->getAnswer();
}

BasicRequester::BasicRequester( ServerTypes::servertype_t type ):
   m_impl( new Impl( type ) ) {

}

BasicRequester::~BasicRequester() {
   delete m_impl;
}

void BasicRequester::start() {
   m_impl->start();
}

void BasicRequester::stop() {
   m_impl->stop();
}

void BasicRequester::putRequest( ThreadRequestContainer* reqCont ) {
   reqCont->getRequest()->setOriginator( m_impl->m_type );

   mc2log << info << "[BasicRequester] Processing request ";
   ThreadRequestInfo::printBeginInfo( mc2log, m_impl->m_type, *reqCont );
   mc2log << endl;

   DebugClock clock;

   m_impl->putRequest( reqCont );
   // wait for answer, ignore the return value
   m_impl->getAnswer();

   mc2log << info << "[BasicRequester] Processed request ";
   ThreadRequestInfo::printEndInfo( mc2log, m_impl->m_type, *reqCont,
                                    clock.getTime() );
   mc2log << endl;

}

void BasicRequester::putRequest( Request* req ) {
   ThreadRequestContainer reqCont( req );
   putRequest( &reqCont );
}

void BasicRequester::
putRequests( const vector<RequestWithStatus*>& reqs ) {
   if ( ! reqs.empty() ) {
      MultiRequest req( getNextRequestID(), reqs );
      putRequest( &req );
   }

}

PacketContainer* BasicRequester::putRequest( PacketContainer* cont ) {
   SinglePacketRequest request( getNextRequestID(), cont );
   putRequest( &request );
   return request.getAnswer();
}

PacketContainer* BasicRequester::putRequest( Packet* pack,
                                             moduletype_t module ) {
   return putRequest( new PacketContainer( pack, 0, 0, module ) );
}

void BasicRequester::putRequest( vector< PacketContainer* >& reqs,
                                 vector< PacketContainer* >& reps ) {
   // NOTE! void ParserThread::putRequest( vector< PacketContainer* >&
   // packets ) calls this function with reps and reqs referring to the
   // same vector. If this function is changed in a way that makes this
   // unsafe, you must also change that function.
   if ( ! reqs.empty() ) {
      uint32 nbrPackets = reqs.size();
      auto_ptr<MultiplePacketsRequest>
         multiPacketReq( new MultiplePacketsRequest( getNextRequestID(),
                                                     nbrPackets ) );
      for ( uint32 i = 0 ; i < nbrPackets ; ++i ) {
         multiPacketReq->add( reqs[ i ] );
      }

      // The packets are now in multiPacketReq
      reqs.clear();

      putRequest( multiPacketReq.get() );

      for ( uint32 i = 0 ; i < nbrPackets ; ++i ) {
         reps.push_back( multiPacketReq->getAnswer( i ) );
      }
   }
}

void BasicRequester::putRequest( vector< PacketContainer* >& packets ) {
   putRequest( packets, packets );
}

RequestData BasicRequester::getNextRequestID() {
   return RequestData( m_impl->m_currentRequestID );
}

