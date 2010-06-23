/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PUSHSERVICE_H
#define PUSHSERVICE_H

#include "config.h"
#include "SubscriptionResource.h"
#include "PushResourceData.h"
#include <list>
#include <map>
#include <set>
#include <vector>


// Forward
class PacketContainer;
class Packet;
class PushPacket;

// A list of PacketContainers
typedef list<PacketContainer*> PacketContainerList;

// A map of resource and PushResourceData*
typedef map<SubscriptionResourceNotice, 
   PushResourceData*> PushResourceDataMap;


/**
 * Class respesenting a push service.
 *
 */
class PushService {
   public:
      /**
       * Create a new PushService.
       * 
       * @param serviceID The unique ID of the service that the PushService
       *                  respesents.
       */
      PushService( uint32 serviceID );


      /**
       * Decontruct this PushService.
       */
      virtual ~PushService();

      /**
       *   Creates a copy with the right type.
       */
      virtual PushService* clone() const = 0;

      /**
       * Get the serviceID of this PushService.
       * 
       * @return The serviceID of this PushService.
       */
      inline uint32 getServiceID() const;


      /**
       * Get the timeout for receiving anything from push provider before
       * trying to subscribe again.
       *
       * @return The timeout for receiving anything from 
       *         push provider before trying to subscribe again.
       */
      inline uint32 getReceiveTimeout() const;


      /**
       * Set the timeout for receiving anything from push provider before
       * trying to subscribe again.
       *
       * @param receiveTimeout The timeout for receiving anything from 
       *                       push provider before trying to subscribe 
       *                       again.
       */
      inline void setReceiveTimeout( uint32 receiveTimeout );


      /**
       * Get the timeout for resending subscription.
       *
       * @return The timeout for resending subscription.
       */
      inline uint32 getResendSubscriptionTimeout() const;


      /**
       * Set the timeout for resending subscription.
       *
       * @param timeout The timeout for resending subscription.
       */
      inline void setResendSubscriptionTimeout( uint32 timeout );
                                       

      /**
       * Get the last access time for a resource.
       * 
       * @param  resource The resource to get last access time for.
       * @return The last access time for the resource or MAX_UINT32
       *         if the resource doesn't exist.
       */
      inline uint32 getLastAccessTimeForResource( SubscriptionResource& 
                                                  resource ) const;


      /**
       * Adds a resource that this service should have push for.
       *
       * @param resource The resource that this service should have push
       *                 for.
       * @param lastUpdateTime The time from which push data should be
       *        sent.
       * @return True if resource was added and false if resource
       *              already exists.
       */
      virtual bool addResource( SubscriptionResource& resource,
                                uint32 lastUpdateTime = 0 );


      /**
       * Get the number of resources.
       *
       * @return The number of resources.
       */
      inline uint32 getNumberResources() const;


      /**
       * Get all of the resources.
       *
       * @param resources All resources of the sevice is added to this 
       *                  vector.
       */
      void getAllResources( 
         vector<SubscriptionResourceNotice>& resources );


      /**
       * Remove all resources.
       *
       * @param packetList The list of packets to add packets that needs
       *                   to be sent to.
       */
      inline void removeAllResources( PacketContainerList& packetList );


      /**
       * Removess a resource that this service shouldn't have push for
       * anymore.
       *
       * @param resource The resource that this service should stop having
       *                 push for.
       * @param packetList The list of packets to add packets that needs
       *                   to be sent to.
       * @return True if resource did exist, false if it didn't.
       */
      inline bool removeResource( SubscriptionResource& resource, 
                                  PacketContainerList& packetList );


      /**
       * Handle a received push packet for this service.
       *
       * @param pushPacket The push packet to handle.
       * @param pushSocket The socket that the pushPacket was received on.
       * @param packetList The list of packets to add packets that needs
       *                   to be sent to.
       * @param isDataPacket Set to true if the pushPacket is a data 
       *                     packet.
       * @param resource Set to the resource of the pushPacket if it is a
       *                 data packet or NULL if not.
       * @return True if the packet was for this service, false if not.
       */
      bool handlePushPacket( PushPacket* pushPacket, TCPSocket* pushSocket,
                             PacketContainerList& packetList,
                             bool& isDataPacket,
                             SubscriptionResource*& resource );


      /**
       * Handle a broken socket perhaps for this service.
       *
       * @param pushSocket The socket that is broken.
       * @param packetList The list of packets to add packets that needs
       *                   to be sent to.
       * @return True if the socket was used by this service, false if not.
       */
      bool handleBrokenSocket( TCPSocket* pushSocket, 
                               PacketContainerList& packetList );


      /**
       * Merge in resources from another PushService.
       *
       * @param other The other PushService to merge resources from.
       * @param packetList The list of packets to add packets that needs
       *                   to be sent to.
       * @param lastUpdateTime For all resources the lastUpdateTime is 
       *                       added to this vector.
       */
      void merge( PushService* other, PacketContainerList& packetList,
                  vector<uint32>& lastUpdateTime );


      /**
       * Make all packets that has to be sent and produce a new timeout.
       *
       * @param packetList The list of packets to add packets that needs
       *                   to be sent to.
       * @return Time for next timeout.
       */
      uint32 checkAndCalculateTimeout( PacketContainerList& packetList );


      /**
       * Get the lastUpdateTime for all resources.
       *
       * @param lastUpdateTime For all resources the lastUpdateTime is 
       *                       added to this vector.
       */
      void getLastUpdateTime( vector<uint32>& lastUpdateTime ) const;


      /**
       * Prints all data on stream.
       *
       * @param out The stream to print onto.
       */
      void dump( ostream& out ) const;


   protected:
      /**
       * Make an unsubscribe packet for a certain resource.
       * 
       * @param resource The resource to make unsubscribe packet for.
       * @return New PacketContainer with the unsubscribe packet.
       */
      virtual PacketContainer* makeUnSubscribePacket( 
         const SubscriptionResource& resource ) = 0;


      /**
       * Make an subscribe packet for a certain resource.
       * 
       * @param resource The resource to make subscribe packet for.
       * @param lastUpdateTime Time from which to send data.
       * @return New PacketContainer with the subscribe packet.
       */
      virtual PacketContainer* makeSubscribePacket( 
         const SubscriptionResource& resource, uint32 lastUpdateTime ) = 0;


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
         bool& isHeartBeat ) = 0;


   private:
      /**
       * The PushResourceData.
       */
      PushResourceDataMap m_resourceData;

      
      /**
       * The serviceID.
       */
      uint32 m_serviceID;


      /**
       * The receiveTimeout.
       */
      uint32 m_receiveTimeout;


      /**
       * The resendSubscriptionTimeout.
       */
      uint32 m_resendSubscriptionTimeout;
};


// =======================================================================
//                                 Implementation of the inlined methods =


inline uint32 
PushService::getServiceID() const {
   return m_serviceID;
}


inline uint32 
PushService::getReceiveTimeout() const {
   return m_receiveTimeout;
}


inline void 
PushService::setReceiveTimeout( uint32 receiveTimeout ) {
   m_receiveTimeout = receiveTimeout;
}


inline uint32 
PushService::getResendSubscriptionTimeout() const {
   return m_resendSubscriptionTimeout;
}


inline void 
PushService::setResendSubscriptionTimeout( uint32 timeout ) {
   m_resendSubscriptionTimeout = timeout;
}


inline uint32 
PushService::getLastAccessTimeForResource( 
   SubscriptionResource& resource ) const 
{
   PushResourceDataMap::const_iterator it = 
      m_resourceData.find( SubscriptionResourceNotice( resource ) );
   if ( it != m_resourceData.end() ) {
      return it->second->getTimeForLastProviderContact();
   } else {
      return MAX_UINT32;
   }
}


inline uint32 
PushService::getNumberResources() const {
   return m_resourceData.size();
}


inline void
PushService::removeAllResources( PacketContainerList& packetList ) {
   PushResourceDataMap::iterator it = m_resourceData.begin();

   while ( it != m_resourceData.end() ) {
      packetList.push_back( makeUnSubscribePacket( *(it->first) ) );
      delete it->second;
      ++it;
   }
   m_resourceData.clear();
}


inline bool 
PushService::removeResource( SubscriptionResource& resource, 
                             PacketContainerList& packetList ) 
{
   PushResourceDataMap::iterator it = 
      m_resourceData.find( SubscriptionResourceNotice( resource ) );
   if ( it != m_resourceData.end() ) {
      packetList.push_back( makeUnSubscribePacket( resource ) );
      delete it->second;
      m_resourceData.erase( it );
      return true;
   } else {
      return false;
   }
}


#endif // PUSHSERVICE_H

