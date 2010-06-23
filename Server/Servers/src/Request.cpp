/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Request.h"
#include "Packet.h"

#include <typeinfo>

// Request ------------------------

Request::
Request( const RequestPtrOrRequestID& reqOrID ) :
      m_requestData( reqOrID )
{
   m_done               = false;
   m_packetID           = 1;
   m_parent             = reqOrID.getRequest();
   m_nbrSentPackets     = 0;
   m_nbrReceivedPackets = 0;
   m_nbrReceivedBytes   = 0;
   m_nbrResentPackets   = 0;
   m_totalResendNbr     = 0;
   if (m_parent != NULL) {
      m_timestamp = m_parent->getTimestamp();
      m_originator =  m_parent->getOriginator();
   } else 
      m_timestamp = ::time(NULL);
}

Request::
Request( const RequestData& reqOrID ) :
      m_requestData( reqOrID )
{
   m_done               = false;
   m_packetID           = 1;
   m_parent             = reqOrID.getRequest();
   m_nbrSentPackets     = 0;
   m_nbrReceivedPackets = 0;
   m_nbrReceivedBytes   = 0;
   m_nbrResentPackets   = 0;
   m_totalResendNbr     = 0;

   if (m_parent != NULL) {
      m_timestamp = m_parent->getTimestamp();
      m_originator =  m_parent->getOriginator();
   } else {
      m_timestamp = ::time(NULL);
   }
}

Request::~Request() 
{
   // All PacketContainers in m_packetsReadyToSend are deleted in
   // PacketContainerTree(BinarySearchTree) destructor.
}

const char*
Request::getName() const
{
   return typeid(*this).name();
}

bool 
Request::requestDone() 
{
   return m_done;
}

const UserUser*
Request::getUser() const
{   
   return m_requestData.getUser();
}


PacketContainer*
Request::getNextPacket() {
   // The packet to return
   return m_packetsReadyToSend.extractMin();
}

PacketContainer* 
Request::getAnswer() {
   return NULL;
}

int
Request::enqueuePacketsFromRequest(Request* req)
{
   // Cannot dequeue packets from ourselves
   MC2_ASSERT( req != this );
   int nbrPacks = 0;
   for ( PacketContainer* pack = req->getNextPacket();
         pack != NULL;
         pack = req->getNextPacket() ) {
      enqueuePacketContainer(pack);
      ++nbrPacks;
   }
   return nbrPacks;
}

bool
Request::processSubRequestPacket(RequestWithStatus* req,
                                 PacketContainer* pack,
                                 StringTable::stringCode& status)
{   
   if ( pack) {
      req->processPacket(pack);
   }

   // Check if request is done
   if ( !req->requestDone() ) {
      enqueuePacketsFromRequest(req);
   }

   // Just to be sure, check again.
   if ( req->requestDone() ) {
      status = req->getStatus();
      if ( status == StringTable::OK ) {
         mc2dbg2 << "[Request]: Other request done OK" << endl;         
         return true;
      } else {
         mc2dbg2 << "[Request]: Other request done NOT OK" << endl;
         return true;
      }
   } else {
      return false;
   }
}


Packet*
Request::updateIDs( Packet* pack ) {
   pack->setRequestID(getID());
   pack->setPacketID(getNextPacketID());
   pack->setRequestTimestamp(getTimestamp());
   pack->setRequestOriginator(getOriginator());
   // Return the same packet
   return pack;
}

RequestPacket*
Request::updateIDs( RequestPacket* pack )
{
   // Hope it will use the other one...
   return static_cast<RequestPacket*>(
      updateIDs( static_cast<Packet*>(pack) ) );
}

PacketContainer*
Request::updateIDs( PacketContainer* pc )
{
   updateIDs( pc->getPacket() );
   return pc;
}

bool
RequestWithStatus::isCacheable() const
{
   return m_isCacheable;
}
