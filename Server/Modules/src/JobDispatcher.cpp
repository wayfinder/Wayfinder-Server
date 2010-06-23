/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "JobDispatcher.h"

#include "PacketQueue.h"
#include "PacketSender.h"
#include "PacketCache.h"
#include "Packet.h"
#include "IPnPort.h"
#include "JobReply.h"

JobDispatcher::JobDispatcher( PacketCache& cache,
                              PacketSender& sender ):
   m_cache( cache ),
   m_sender( sender ) {

}

bool JobDispatcher::checkCache( Packet* packet, JobReply& reply ) {

   char packetInfo[ 1024 ];

   Packet* cachedPacket =
      m_cache.getCachedReply( static_cast< const RequestPacket* >( packet ),
                              packetInfo );
   if ( cachedPacket ) {
      // Found cache
      reply.m_request = packet;
      reply.m_reply = cachedPacket;
      reply.m_infoString = packetInfo;
      reply.m_cached = true;
      return true; // we had cached reply
   }

   return false;
}

void JobDispatcher::sendReply( JobReply& reply, bool cache ) {
   if ( reply.m_reply ) {
      // Add to cache before sending, because the send might fail and
      // that should not stop us from caching ( and the reply is killed
      // by the sender ) .
      // Only add to cache if the reply was not already in cache.
      if ( cache && ! reply.m_cached ) {
         m_cache.putPacket( static_cast<const RequestPacket*>
                            ( reply.m_request ),
                            reply.m_reply,
                            reply.m_infoString.c_str() );
      }

      m_sender.sendPacket( reply.m_reply, reply.m_reply->getOriginAddr() );
      // Sender takes care of the package now.
      reply.m_reply = NULL;
   }
}
