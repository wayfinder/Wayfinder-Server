/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPPUSHSERVICE_H
#define MAPPUSHSERVICE_H

#include "config.h"
#include "PushService.h"
#include "PacketContainer.h"


/**
 * Class respesenting a push service for maps.
 *
 */
class MapPushService : public PushService {
public:
   /**
    *   Create a new MapPushService.
    * 
    *   @param serviceID The unique ID of the service that the 
    *                    MapPushService respesents.
    */
   MapPushService( uint32 serviceID, moduletype_t subscriptionModule );
   

   /**
    *   Decontruct this MapPushService.
    */
   virtual ~MapPushService();

   /**
    *   Creates a copy with the right type.
    */
   virtual PushService* clone() const;

   
   /**
    * Adds a resource that this service should have push for.
    *
    * @param resource The resource that this service should have push
    *                 for.
    * @param lastUpdateTime The time from which push data should be
    *                       sent.
    * @return True if resource was added and false if resource
    *              already exists or was not MapSubscriptionResource.
    */
   virtual bool addResource( SubscriptionResource& resource,
                             uint32 lastUpdateTime = 0 );
   
   
   /**
    * Adds a mapID that this map service should have push for.
    *
    * @param mapID The mapID that this service should have push for.
    * @param lastUpdateTime The time from which push data should be
    *        sent.
    * @return True if mapID was added and false if mapID
    *              already exists.
    */
   bool addResource( uint32 mapID, uint32 lastUpdateTime = 0 );
   
protected:
   /**
    * Make an unsubscribe packet for a certain resource ID.
    * 
    * @param ID The resource ID to make unsubscribe packet for.
    * @return New PacketContainer with the unsubscribe packet.
    */
   virtual PacketContainer* makeUnSubscribePacket( 
      const SubscriptionResource& resource );
   
   
   /**
    * Make an subscribe packet for a certain resource ID.
    * 
    * @param ID The resource ID to make subscribe packet for.
    * @return New PacketContainer with the subscribe packet.
    */
   virtual PacketContainer* makeSubscribePacket( 
      const SubscriptionResource& resource, uint32 lastUpdateTime  );
   
   
   /**
    * Read the wanted and unwanted maps from pushpacket.
    *
    * @param pushPacket The pushPacket to read.
    * @param subscribed subscribed Set of successfully subscribed
    *                   resources.
    * @param unsubscribed Set of recently removed resources.
    * @param isHeartBeat Set to true if pushPacket is a heartbreat and
    *                    false if pushPacket is a push data packet.
    * @return True if packet was decoded ok, false if strange packet.
    */
   virtual bool getPushPacketMaps(
      PushPacket* pushPacket,
      set<SubscriptionResourceNotice>& subscribed,
      set<SubscriptionResourceNotice>& unsubscribed,
      bool& isHeartBeat );


private:
      /**
       * The module type to send subscriptions to.
       */
      moduletype_t m_subscriptionModule;
};


#endif // MAPPUSHSERVICE_H

