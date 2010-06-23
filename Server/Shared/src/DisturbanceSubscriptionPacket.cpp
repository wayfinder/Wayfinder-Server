/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DisturbanceSubscriptionPacket.h"

#define DISTURBANCE_SUBSCRIPTION_REPLY_SIZE REPLY_HEADER_SIZE

// Request packet

#define DISTURBANCE_SUBSCRIPTION_REQUEST_SIZE (REQUEST_HEADER_SIZE +2)
#define DISTURBANCE_SUBSCRIPTION_REQUEST_PRIO  1

DisturbanceSubscriptionRequestPacket::DisturbanceSubscriptionRequestPacket()
      :RequestPacket( DISTURBANCE_SUBSCRIPTION_REQUEST_SIZE,
                      DISTURBANCE_SUBSCRIPTION_REQUEST_PRIO,
                      0,
                      0,
                      0,
                      MAX_UINT32 ){
}


   
DisturbanceSubscriptionRequestPacket::DisturbanceSubscriptionRequestPacket(
   uint16 packetID,
   uint16 reqID)
      :RequestPacket(DISTURBANCE_SUBSCRIPTION_REQUEST_SIZE,
                     DISTURBANCE_SUBSCRIPTION_REQUEST_PRIO,
                     PACKETTYPE_DISTURBANCESUBSCRIPTIONREQUEST,
                     packetID,
                     reqID,
                     MAX_UINT32)
{
   
}

void
DisturbanceSubscriptionRequestPacket::encode(uint32 mapID,
                                             bool subscribe,
                                             byte roadMinSize)
{
   int position = REQUEST_HEADER_SIZE;
   setMapID(mapID);
   incWriteByte(position, (byte) subscribe);
   incWriteByte(position, roadMinSize);
   setLength(position);
}

void
DisturbanceSubscriptionRequestPacket::decode(uint32& mapID,
                                             bool& subscribe,
                                             byte& roadMinSize)
{
   int position = REQUEST_HEADER_SIZE;
   mapID        = getMapID();
   subscribe    = (bool) incReadByte(position);
   roadMinSize  = incReadByte(position);

}
// Reply packet


DisturbanceSubscriptionReplyPacket::DisturbanceSubscriptionReplyPacket()
      :ReplyPacket()
{
      setLength( REPLY_HEADER_SIZE );
}

DisturbanceSubscriptionReplyPacket::DisturbanceSubscriptionReplyPacket(
   const RequestPacket* p,
   uint32 status)
      : ReplyPacket(DISTURBANCE_SUBSCRIPTION_REPLY_SIZE,
                    PACKETTYPE_DISTURBANCESUBSCRIPTIONREPLY,
                    p,
                    status)
{
   setLength(REPLY_HEADER_SIZE);
}

DisturbanceSubscriptionReplyPacket::~DisturbanceSubscriptionReplyPacket()
{
     
}
