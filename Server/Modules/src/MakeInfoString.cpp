/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MakeInfoString.h"

#include "FixedSizeString.h"
#include "PacketQueue.h"
#include "Packet.h"
#include "StringTable.h"
#include "JobReply.h"
#include "IPnPort.h"

#include <boost/lexical_cast.hpp>
#include <stdio.h>

namespace JobThreadString {

void makeInfoString( FixedSizeString& infoString,
                     const JobReply& reply ) {

   const char* packetInfo = reply.m_infoString.c_str();
   const char* requestType = reply.m_request->getSubTypeAsString();
   uint32 processingTime = reply.m_processingTimeMillis;
   float cpuTimeMillis = reply.m_processorTimeMillis;
   const ReplyPacket* replyPacket = (const ReplyPacket*)( reply.m_reply );
   uint32 mapID = static_cast<RequestPacket*>
      (reply.m_request)->getMapID();
   uint32 inPacketSize = reply.m_request->getLength();
   int timeSinceArrival = reply.m_timeSinceArrival;

   // Printing starts here.
   const char* lineStart = "JT: ";

   FixedSizeString queueInfo( 40 );
   sprintf( queueInfo, "%s", reply.m_receiveQueueInfo.c_str() );

   FixedSizeString mapInfo( 40 );
   if ( mapID != MAX_UINT32 ) {
      sprintf( mapInfo, ", m=0x%08x", mapID );
   } else {
      mapInfo[0] = '\0';
   }

   float cpuPercent = float(cpuTimeMillis) / float(processingTime);

   if ( replyPacket != NULL ) {
      StringTable::stringCode status = StringTable::UNKNOWN;
      // It may not be a replypacket...
      if ( replyPacket->getLength() >=
           ReplyPacket::REPLY_PACKET_STATUS_POS + 4) {
         status =
            StringTable::stringCode(replyPacket->getStatus());
      }
      const char* replyType = replyPacket->getSubTypeAsString();
      const char* statusString =
         StringTable::getString(status, StringTable::ENGLISH);

      // Now put together a string
      sprintf( infoString,
               "%s%s->%s(%s), l=%u->%u, t = %u (%.2f) ms%s%s (%s), ta=%u "
               "pid=%u r=%u",
               lineStart, requestType, replyType, statusString,
               inPacketSize,
               replyPacket->getLength(), processingTime,
               cpuPercent,
               mapInfo.c_str(), queueInfo.c_str(), packetInfo,
               timeSinceArrival,
               replyPacket->getPacketID(),
               replyPacket->getResendNbr() );
   } else {
      // Replypacket was null - print something anyway
      sprintf( infoString,
               "%s%s->NULL, t = %u (%.2f) ms%s%s (%s)",
               lineStart, requestType, processingTime, cpuPercent,
               mapInfo.c_str(), queueInfo.c_str(), packetInfo );
   }
}

void addReceiveQueueInfo( JobReply& reply,
                          const PacketQueue& receiveQueue ) {
   reply.m_receiveQueueInfo += ", q=";
   reply.m_receiveQueueInfo +=
      boost::lexical_cast<MC2String>( const_cast<PacketQueue&>
                                      ( receiveQueue ).getStatistics() );
}
} // JobThreadString
