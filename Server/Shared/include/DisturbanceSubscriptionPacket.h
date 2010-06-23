/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DISTURBANCESUBSCRIPTIONPACKET_H
#define DISTURBANCESUBSCRIPTIONPACKET_H

#include "Types.h"
#include "Packet.h"

/**
  *   Packet used for requesting the beginning or end of a subscription
  *   of a maps disturbances.
  *
  *   After the normal header this packet contains
  *   @packetdesc
  *      @row 16  @sep 4 bytes @sep map ID                        @endrow
  *      @row 20  @sep 1 byte  @sep subscribe y/n                 @endrow
  *      @row 21  @sep 1 byte  @sep lowest road size of intrest   @endrow
  *   @endpacketdesc
  *
  */
class DisturbanceSubscriptionRequestPacket : public RequestPacket{
  public:
   
   /**
    * Constructor.
    * @param packetId the packetId of the packet
    * @param reqId the request id of the packet
    */
   DisturbanceSubscriptionRequestPacket(uint16 packetId,
                                        uint16 reqId);

   /**
    * Destructor.
    */
   virtual ~DisturbanceSubscriptionRequestPacket(){};

   /**
    * Sets the parameters of the Subscription request
    * @param mapID The Id of the map to subscribe/unsubscribe to.
    * @param subscribe True to subscribe. False to unsubscribe.
    * @param roadMinSize The minimum size of the roads of intrest.
    */
   void encode(uint32 mapID,
               bool subscribe,
               byte roadMinSize = MAX_BYTE);

   
   /**
    * Extracts subscription info from the package.
    * @param mapID The Id of the map to subscribe/unsubscribe to.
    * @param subscribe True if module wants to subscribe, false
    *                  if unsubscribe.
    * @param roadMinSize The minimum size of the roads of intrest.
    */
   void decode(uint32& mapID,
               bool& subscribe,
               byte& roadMinSize);
   

   
  protected:
   
   /*
    * Default constructor. Protected to avoid use.
    */
   DisturbanceSubscriptionRequestPacket();

};


/**
  *   Packet used for replying to a subscription request.
  *
  *   After the normal header this packet contains
  *   @packetdesc
  *      @row 16  @sep 4 bytes @sep status @endrow
  *   @endpacketdesc
  *
  */
class DisturbanceSubscriptionReplyPacket : public ReplyPacket
{
   
   public:
   /**
    * Constructor.
    * @param p The request packet.
    * @param status The result of the addition.
    */
   DisturbanceSubscriptionReplyPacket(const RequestPacket* p,
                                      uint32 status);

   /**
    * Destructor.
    */
   virtual ~DisturbanceSubscriptionReplyPacket();

  protected:
   
   /*
    * Default constructor. Protected to avoid use.
    */
   DisturbanceSubscriptionReplyPacket();
   
};

    
#endif   // DISTURBANCESUBSCRIPTIONPACKET_H

