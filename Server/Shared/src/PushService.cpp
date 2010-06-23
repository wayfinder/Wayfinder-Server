/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PushService.h"
#include "Packet.h"
#include "PushPacket.h"
#include "StringUtility.h"


PushService::PushService( uint32 serviceID ):
   m_serviceID( serviceID ),
   m_receiveTimeout( 5 ),
   m_resendSubscriptionTimeout( 5 )
{
}


PushService::~PushService() {
   PushResourceDataMap::iterator it = m_resourceData.begin();

   while ( it != m_resourceData.end() ) {
      delete it->second;
      it++;
   }
   m_resourceData.clear();
}


bool 
PushService::addResource( SubscriptionResource& resource, 
                          uint32 lastUpdateTime  ) 
{
   PushResourceDataMap::const_iterator it = 
      m_resourceData.find( resource );
   if ( it == m_resourceData.end() ) {
      // Add it
      PushResourceData* data = new PushResourceData();
      data->setTimeForLastUpdate( lastUpdateTime );
      m_resourceData.insert( make_pair( SubscriptionResourceNotice( 
         resource ), data ) );

      return true;
   } else {
      return false;
   }
}


void 
PushService::getAllResources( 
   vector<SubscriptionResourceNotice>& resources ) 
{
   for ( PushResourceDataMap::iterator it = m_resourceData.begin() ;
         it != m_resourceData.end() ; ++it )
   {
      resources.push_back( it->first );
   }
}


bool
PushService::handlePushPacket( PushPacket* pushPacket, 
                               TCPSocket* pushSocket,
                               PacketContainerList& packetList,
                               bool& isDataPacket,
                               SubscriptionResource*& resource )
{
   set<SubscriptionResourceNotice> subscribed;
   set<SubscriptionResourceNotice> unsubscribed;
   bool isHeartBeat = false;

   if ( getPushPacketMaps( pushPacket, subscribed, unsubscribed,
                           isHeartBeat ) ) 
   {
      uint32 now = TimeUtility::getRealTime();

      if ( !isHeartBeat && subscribed.begin() != subscribed.end() ) {
         isDataPacket = true;
         resource = (**subscribed.begin()).clone();
      }
      
      set<SubscriptionResourceNotice>::iterator it = subscribed.begin();
      while( it != subscribed.end() ) {
         PushResourceDataMap::iterator rit = m_resourceData.find( *it );
         if ( rit == m_resourceData.end() ) {
            // We don't want this resource to be subscribed
            mc2dbg2 << "PushService::handlePushPacket, unwanted service "
                   << "unsubscribe, serviceID "
                   << getServiceID() << " resource ";
#ifdef DEBUG_LEVEL_2
            **it << mc2log;
#endif
            mc2dbg2 << endl;
            packetList.push_back( makeUnSubscribePacket( **it ) );
         } else {
            // Else all ok subscribed resource found update times
            rit->second->setPushSocket( pushSocket );
            if ( isHeartBeat ) {
               rit->second->setTimeForLastHeartbeat( now );
            } else { // Data packet
               rit->second->setTimeForLastUpdate( now );
            }
         }
         ++it;
      }
      
   } else {
      mc2log << warn << "PushService::handlePushPacket getPushPacketMaps "
             << "falied, strange packet: " << endl;
      pushPacket->dump();
   }

   return true;
}


bool
PushService::handleBrokenSocket( TCPSocket* pushSocket, 
                                 PacketContainerList& packetList )
{
   PushResourceDataMap::iterator it = m_resourceData.begin();
   bool usedSocket = false;
   uint32 now = TimeUtility::getRealTime();

   while ( it != m_resourceData.end() ) {
      if ( it->second->getPushSocket() == pushSocket ) {
         it->second->setPushSocket( NULL );
         packetList.push_back( makeSubscribePacket( 
            *it->first, it->second->getTimeForLastUpdate() ) );
         it->second->setTimeForLastSentSubscription( now );
         usedSocket = true;
      }
      ++it;
   }

   return usedSocket;
}


void
PushService::merge( PushService* other, PacketContainerList& packetList,
                    vector<uint32>& lastUpdateTime )
{
   if ( other->getServiceID() == getServiceID() ) {
      // Merge resource IDs
      PushResourceDataMap::iterator it = other->m_resourceData.begin();
      
      while ( it != other->m_resourceData.end() ) {
         // Find resouceID
         PushResourceDataMap::iterator fit = m_resourceData.find( 
            it->first );
         if ( fit == m_resourceData.end() ) { // Not found
            packetList.push_back( makeSubscribePacket( 
               *it->first, it->second->getTimeForLastUpdate() ) );
            addResource( *it->first, it->second->getTimeForLastUpdate() );
            lastUpdateTime.push_back( it->second->getTimeForLastUpdate() );
         } else {
            lastUpdateTime.push_back( 
               fit->second->getTimeForLastUpdate() );
         }

         ++it;
      }
      
   } else {
      mc2log << warn << "PushService::merge serviceID missmatch! other "
             << other->getServiceID() << " != " << getServiceID() << endl;
   }
}


uint32 
PushService::checkAndCalculateTimeout( PacketContainerList& packetList ) {
   uint32 now = TimeUtility::getRealTime();
   PushResourceDataMap::iterator it = m_resourceData.begin();
   uint32 timeNextTimeOut = now + getResendSubscriptionTimeout() +
      getReceiveTimeout(); // Larger than all timeouts
   uint32 timeOutTime = 0;

   while ( it != m_resourceData.end() ) {
      if ( it->second->getPushSocket() != NULL ) { // Connected
         timeOutTime = it->second->getTimeForLastProviderContact() + 
            getReceiveTimeout();
         if ( timeOutTime <= now ) {
            mc2dbg2 << "PushService::checkAndCalculateTimeout "
                   << "Connected Resource time out, resubscribe serviceID "
                   << getServiceID() << " resource ";
#ifdef DEBUG_LEVEL_2
            *it->first << mc2log;
#endif
            mc2dbg2 << " lastUpdateTime " 
                   << it->second->getTimeForLastUpdate() << endl;
            // Timeout, resubscribe
            it->second->setPushSocket( NULL ); 
            packetList.push_back( makeSubscribePacket( 
               *it->first, it->second->getTimeForLastUpdate() ) );
            it->second->setTimeForLastSentSubscription( now );
            timeOutTime = it->second->getTimeForLastSentSubscription() + 
               getResendSubscriptionTimeout();
         }
      } else { // Subscribing
         timeOutTime = it->second->getTimeForLastSentSubscription() + 
            getResendSubscriptionTimeout();
         if ( timeOutTime <= now ) {
            mc2dbg2 << "PushService::checkAndCalculateTimeout "
                   << "Subscribing Resource time out, subscribe serviceID "
                   << getServiceID() << " resource ";
#ifdef DEBUG_LEVEL_2
            *it->first << mc2log;
#endif
            mc2dbg2 << " lastUpdateTime " 
                   << it->second->getTimeForLastUpdate() << endl;
            // (Re)send subscription
            it->second->setPushSocket( NULL ); 
            packetList.push_back( makeSubscribePacket( 
               *it->first, it->second->getTimeForLastUpdate() ) );
            it->second->setTimeForLastSentSubscription( now );
            timeOutTime = it->second->getTimeForLastSentSubscription() + 
               getResendSubscriptionTimeout();
         }
      }
      if ( timeOutTime < timeNextTimeOut ) {
         timeNextTimeOut = timeOutTime;
      }

      ++it;
   }

   return timeNextTimeOut;
}


void 
PushService::getLastUpdateTime( vector<uint32>& lastUpdateTime ) const {
   for ( PushResourceDataMap::const_iterator it = m_resourceData.begin() ;
         it != m_resourceData.end() ; ++it )
   {
      lastUpdateTime.push_back( it->second->getTimeForLastUpdate() );
   }
}


void 
PushService::dump( ostream& out ) const {
   out << "   PushService " << endl;
   out << "      serviceID " << m_serviceID << endl;
   out << "      receiveTimeout " << m_receiveTimeout << endl;
   out << "      resendSubscriptionTimeout " << m_resendSubscriptionTimeout
       << endl;
   out << "      nbr resourceData " << m_resourceData.size() << endl;


   PushResourceDataMap::const_iterator it = m_resourceData.begin();
   while ( it != m_resourceData.end() ) {
      out << "         SubscriptionResourceNotice " << *(it->first) 
          << endl;
      it->second->dump( out );
      ++it;
   }
}
