/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapPushService.h"
#include "MapSubscriptionPacket.h"
#include<typeinfo>

MapPushService::MapPushService( uint32 serviceID, 
                                moduletype_t subscriptionModule ) 
      : PushService( serviceID ),
        m_subscriptionModule( subscriptionModule )
{
   setReceiveTimeout( 
      MapSubscriptionHeartbeatRequestPacket::HEARTBEAT_TIME_MS  / 1000 +
      5 );
}


MapPushService::~MapPushService() {
}


PushService*
MapPushService::clone() const
{
   return new MapPushService(getServiceID(), m_subscriptionModule);
                             
}

bool 
MapPushService::addResource( uint32 mapID, uint32 lastUpdateTime ) {
   MapSubscriptionResource resource( mapID );

   return PushService::addResource( resource, lastUpdateTime );
}


bool
MapPushService::addResource( SubscriptionResource& resource, 
                             uint32 lastUpdateTime  ) 
{
   if ( typeid(resource) == typeid( MapSubscriptionResource ) ) {
      return PushService::addResource( resource, lastUpdateTime );
   } else {
      // This is a MapPushService. We handle MapSubscriptionResources
      return false;
   }
}


PacketContainer* 
MapPushService::makeUnSubscribePacket( 
   const SubscriptionResource& resource ) 
{
   const MapSubscriptionResource& mapID = 
      static_cast< const MapSubscriptionResource& >( resource );
   map<uint32,uint32> wantedMaps;
   vector<uint32> unwantedMaps;
   unwantedMaps.push_back( *mapID );
   return new PacketContainer( new MapSubscriptionRequestPacket( 
      *mapID, getServiceID(), wantedMaps, unwantedMaps ), 
                               0, 0, m_subscriptionModule );
}


PacketContainer*
MapPushService::makeSubscribePacket( 
   const SubscriptionResource& resource, uint32 lastUpdateTime  ) 
{
   const MapSubscriptionResource& mapID = 
      static_cast< const MapSubscriptionResource& >( resource );
   map<uint32,uint32> wantedMaps;
   vector<uint32> unwantedMaps;
   wantedMaps.insert( make_pair( *mapID, lastUpdateTime ) );
   return new PacketContainer( new MapSubscriptionRequestPacket( 
      *mapID, getServiceID(), wantedMaps, unwantedMaps ), 
                               0, 0, m_subscriptionModule );
}


bool
MapPushService::getPushPacketMaps( 
   PushPacket* pushPacket,
   set<SubscriptionResourceNotice>& subscribed,
   set<SubscriptionResourceNotice>& unsubscribed,
   bool& isHeartBeat )
{
   if ( pushPacket->getServiceID() == getServiceID() ) {
      if ( pushPacket->getSubType() == 
           Packet::PACKETTYPE_MAPSUBSCRIPTIONHEARTBEATREQUEST )
      {
         set<uint32> subscribedMaps;
         set<uint32> unsubscribedMaps;
         static_cast< MapSubscriptionHeartbeatRequestPacket* > ( 
            pushPacket )->getData( subscribedMaps, unsubscribedMaps );
         isHeartBeat = true;
         set<uint32>::iterator it = subscribedMaps.begin();
         while ( it != subscribedMaps.end() ) {
            MapSubscriptionResource res( *it );
            subscribed.insert( res );
            ++it;
         }
         it = unsubscribedMaps.begin();
         while ( it != unsubscribedMaps.end() ) {
            MapSubscriptionResource res( *it );
            unsubscribed.insert( res );
            ++it;
         }
      } else {
         MapSubscriptionResource res( pushPacket->getMapID() );
         subscribed.insert( res );
         isHeartBeat = false;
      }

      return true;
   } else {
      return false;
   }
}
