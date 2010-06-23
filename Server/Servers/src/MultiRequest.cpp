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

#include "MultiRequest.h"
#include "Packet.h"

// -------------------------- MultiRequest::RequestEntry -------------

MultiRequest::RequestEntry::RequestEntry( PacketContainer* origCont,
                                          RequestWithStatus* originalReq )
{
   Packet* pack = origCont->getPacket();
   m_origReqID = pack->getRequestID();
   m_origPackID = pack->getPacketID();
   m_origRequest = originalReq;
   mc2dbg8 << "[MR::RE]: Saved reqID = " << m_origReqID
           << ", pakID = " << m_origPackID
           << ", req = " << MC2HEX( uintptr_t( m_origRequest ) ) << endl;
}

PacketContainer*
MultiRequest::RequestEntry::changeBackIDs( PacketContainer* pc )
{
   Packet* pack = pc->getPacket();
   pack->setRequestID( m_origReqID );
   pack->setPacketID( m_origPackID );
   mc2dbg8 << "[MR::RE]: Restored reqID = " << m_origReqID
           << ", pakID = " << m_origPackID
           << ", req = " << MC2HEX( uintptr_t( m_origRequest ) ) << endl;
   return pc;
}

RequestWithStatus*
MultiRequest::RequestEntry::getRequest() const
{
   return m_origRequest;
}

// -------------------------- MultiRequest ---------------------------

static inline MC2String makeName( const vector<RequestWithStatus*>& children )
{
   typedef map<MC2String, unsigned int> countMap_t;
   countMap_t counts;
   for ( vector<RequestWithStatus*>::const_iterator it = children.begin();
         it != children.end();
         ++it ) {
      counts[(*it)->getName()]++;
   }
   MC2String res;
   const char* comma = "";
   for ( countMap_t::const_iterator it = counts.begin();
         it != counts.end();
         ++it ) {
      res += comma;
      comma = " + ";
      char tmp[1024];
      sprintf(tmp, "%u * %s", it->second, it->first.c_str() );
      res += tmp;
   }
   return res;
}

MultiRequest::MultiRequest( const RequestData& data,
                            const vector<RequestWithStatus*>& children )
      : RequestWithStatus( data ),
        m_not_done_children( children.begin(), children.end() )
{
   m_name   = makeName( children );
   m_status = StringTable::NOTOK;
   sendSubPackets();
}

MultiRequest::~MultiRequest()
{
}

void
MultiRequest::registerAndSend( RequestWithStatus* req,
                               PacketContainer* cont )
{
   // Save old ids.
   RequestEntry entry( cont, req );
   // Set new id
   updateIDs( cont );
   // Remember
   m_packetIDMap.insert( make_pair(cont->getPacket()->getPacketID(),
                                   entry ) );
   // Enqueue
   enqueuePacketContainer( cont );
}

void MultiRequest::sendSubPackets( RequestWithStatus* curReq ) {
   for ( PacketContainer* pc = curReq->getNextPacket();
         pc != NULL;
         pc = curReq->getNextPacket() ) {
      registerAndSend( curReq, pc );
      // Increase the number of sent packets to mimic the RequestHandler
      curReq->incNbrSentPackets();
   }
}

void
MultiRequest::sendSubPackets()
{
   removeDone();
   for ( child_list_t::iterator it = m_not_done_children.begin();
         it != m_not_done_children.end();
         ++it ) {
      sendSubPackets( *it );
   }
}

void
MultiRequest::processPacket( PacketContainer* pack )
{
   packetIdMap_t::iterator it = m_packetIDMap.find(
      pack->getPacket()->getPacketID() );

   if ( it == m_packetIDMap.end() ) {
      mc2dbg << "[MultiRequest]: Unknown packet received" << endl;
      pack->getPacket()->dump( true );
      return;
   }

   // Increase the number of received packets to mimic the RequestHandler
   it->second.getRequest()->incNbrReceivedPackets();
   // Process the request.
   it->second.getRequest()->processPacket( it->second.changeBackIDs( pack ) );
   
   m_packetIDMap.erase( it );
   sendSubPackets();
}

bool
MultiRequest::requestDone()
{
   return m_not_done_children.empty();
}

int
MultiRequest::removeDone()
{
   int nbr_removed = 0;
   for ( child_list_t::iterator it = m_not_done_children.begin();
         it != m_not_done_children.end(); ) {
      if ( (*it)->requestDone() ) {
         if ( (*it)->getStatus() == StringTable::OK ) {
            m_status = StringTable::OK;
         }
         m_done_requests.push_back( *it );
         it = m_not_done_children.erase(it);
         ++nbr_removed;
      } else {
         ++it;
      }
   }
   if ( nbr_removed ) {
      mc2dbg8 << "[MultiPass]: "
             << nbr_removed << " new sub-request done" << endl;
      mc2dbg8 << "[MultiPass]: Done: " << m_done_requests.size()
             << " Not Done yet: " << m_not_done_children.size() << endl;
   }
   return nbr_removed;
}
