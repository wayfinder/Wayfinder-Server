/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GenericServer.h"
#include "PushServiceHandler.h"
#include "PacketResendHandler.h"
#include "MonitorCache.h"

#include "TopRegionRequest.h"
#include "NetUtility.h"
#include "ServerPort.h"

GenericServer::GenericServer( const MC2String& name,
                              CommandlineOptionHandler* handler,
                              ServerTypes::servertype_t type, 
                              bool pushService ):
   Component( name.c_str(), handler )
{
   s_port = ServerPort::getPort( type );
   s_type = type;
   s_ip = NetUtility::getLocalIP();
   m_reqID = 1;
   m_cache = new MonitorCache(CacheElement::SERVER_TYPE);
   m_pushService = pushService;

   // init to null to avoid heap corruption on immediate destruction.
   m_handler = NULL;
   m_requestHandler = NULL;
   m_pushServiceHandler = NULL;
   m_topRegionRequest = NULL;
}


GenericServer::~GenericServer() {
   delete m_requestHandler; 
   delete m_cache;
   delete m_topRegionRequest;
}

void GenericServer::shutdown() {
   delete m_requestHandler;
   m_requestHandler = NULL;
  
   delete m_cache;
   m_cache = NULL;

   delete m_topRegionRequest;
   m_topRegionRequest = NULL;
}

void
GenericServer::start() {
#ifdef SINGLE_VERSION
   mc2log << info << "[GenericServer] Server running in SINGLE mode" << endl;
#endif
   
   m_handler = new PacketResendHandler( s_port );
   m_requestHandler = new RequestHandler( m_handler, s_type );
   if ( m_pushService ) {
      m_pushServiceHandler = new PushServiceHandler( 
         m_handler->getPort() + 10000 );
   }

   s_port = m_handler->getPort();
   mc2dbg1 << "[GenericServer] IP: " << prettyPrintIP(s_ip) << ", port: " 
           << s_port << endl;

   m_requestHandler->start();
   m_handler->start();
   if ( m_pushService ) {
      m_pushServiceHandler->start();
   }

   init();
   //gotoWork( m_threadInit );

}

void
GenericServer::callGotoWork() {
   gotoWork( m_threadInit );
}

const TopRegionRequest*
GenericServer::getTopRegionRequest()
{
   TopRegionRequest* topRequest = NULL;
   m_topRegionsMutex.lock();
   if ( m_topRegionRequest == NULL ) {
      // Get a new one
      topRequest = new TopRegionRequest( getNextRequestID() );
      RequestContainer* rc = new RequestContainer(topRequest);
      insertRequest( rc );
      RequestContainer* ans = getAnswer();
      topRequest = dynamic_cast<TopRegionRequest*>(ans->getRequest());
      if ( topRequest->getStatus() == StringTable::OK) {
         if (topRequest->isCacheable()) {
            m_topRegionRequest = topRequest;
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
   } else {
      topRequest = m_topRegionRequest;
   }
   m_topRegionsMutex.unlock();
   return topRequest;
}
