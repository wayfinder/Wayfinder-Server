/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "BasicMapRequester.h"
#include "TopRegionRequest.h"
#include "RequestContainer.h"

#include "BasicRequester.h"

BasicMapRequester::BasicMapRequester( ServerTypes::servertype_t type ):
   m_requester( new BasicRequester( type ) ) {
}

BasicMapRequester::~BasicMapRequester() {

}

const TopRegionRequest* BasicMapRequester::getTopRegionRequest() {
   // have old top region request?
   if ( m_topRegionRequest.get() != NULL ) {
      return m_topRegionRequest.get();
   }

   // No old top region request, request a new one
   TopRegionRequest* topRequest = new TopRegionRequest( getNextRequestID() );
   putRequest( topRequest );
   if ( topRequest->getStatus() == StringTable::OK ) {
      if ( topRequest->isCacheable() ) {
         m_topRegionRequest.reset( topRequest );
      } else {
         // this will leak memory, but it's better than returning NULL
         // FIXME somehow.
         /* delete topRequest;
            topRequest = NULL; */
         mc2log << warn << "[PTG] TopRegionRequest not cacheable" << endl;
      }
   } else {
      delete topRequest;
      topRequest = NULL;
      mc2log << warn << "[PTG] TopRegionRequest not OK, returning NULL"
             << endl;
   }

   return topRequest;
}

void BasicMapRequester::start() {
   static_cast<BasicRequester*>( m_requester.get() )->start();
}

void BasicMapRequester::stop() {
   static_cast<BasicRequester*>( m_requester.get() )->stop();
}

void BasicMapRequester::putRequest( ThreadRequestContainer* reqCont ) {
   m_requester->putRequest( reqCont );
}

void BasicMapRequester::putRequest( Request* req ) {
   m_requester->putRequest( req );
}

void BasicMapRequester::putRequests( const vector<RequestWithStatus*>& reqs ) {
   m_requester->putRequests( reqs );
}

PacketContainer* BasicMapRequester::putRequest( PacketContainer* cont ) {
   return m_requester->putRequest( cont );
}

PacketContainer* BasicMapRequester::putRequest( Packet* pack,
                                                moduletype_t module ) {
   return m_requester->putRequest( pack, module );
}

void BasicMapRequester::putRequest( vector< PacketContainer* >& reqs,
                                    vector< PacketContainer* >& reps ) {
   m_requester->putRequest( reqs, reps );
}

void BasicMapRequester::putRequest( vector< PacketContainer* >& packets ) {
   m_requester->putRequest( packets );
}

RequestData BasicMapRequester::getNextRequestID() {
   return m_requester->getNextRequestID();
}
