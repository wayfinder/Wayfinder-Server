/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "JobCourier.h"

#include "Processor.h"
#include "Packet.h"
#include "JobReply.h"
#include "JobLogger.h"

JobCourier::JobCourier( Processor& processor,
                        const PacketQueue& receiveQueue,
                        uint32 id ):
   m_processor( processor ),
   m_logger( new JobLogger( id, receiveQueue ) ) {
   m_processor.endWork();
}

JobCourier::~JobCourier() {

}

void JobCourier::takeJob( Packet* packet, JobReply& reply, bool urgent ) {
   // The packet will die in the processor so we need to clone it here 
   // so we can fetch info about it later.
   reply.m_request = packet->getClone();

   // Urgent package must skip logging and go directly to processor
   if ( urgent ) {
      reply.m_reply =
         m_processor.handleRequest( static_cast< RequestPacket* >( packet ),
                                    &m_logger->getPacketInfoString()[ 0 ] );
      return;
   }

   // Start logging this scope
   JobLogger::LogScope logScope( *m_logger, packet, reply );

   // Start handle the request.

   m_processor.beginWork();

   // Let the processor handle the request now.
   reply.m_reply =
      m_processor.handleRequest( (RequestPacket*)packet,
                                 &m_logger->getPacketInfoString()[ 0 ] );
   // Update various timers
   logScope.updateLogProcess();
   reply.m_infoString = m_logger->getPacketInfoString().c_str();

   // The processor should finalize the request if the packet was not a
   // LoadMap or a DeleteMap.
   if ( ( reply.m_reply != NULL )&&
        ( reply.m_reply->getSubType() != Packet::PACKETTYPE_LOADMAPREPLY) &&
        ( reply.m_reply->getSubType() != Packet::PACKETTYPE_DELETEMAPREPLY)) {

      m_processor.finishRequest( m_logger->getProcessingTime() );

   }

   m_processor.endWork();

}


