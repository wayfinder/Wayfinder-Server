/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PushServices.h"
#include "PushPacket.h"

PushServices::PushServices() 
{
}


PushServices::~PushServices() {
   PushServiceMap::iterator it = m_services.begin();

   while ( it != m_services.end() ) {
      delete it->second;
      ++it;
   }
   m_services.clear();   
}


void
PushServices::addPushService( PushService* service,
                              PacketContainerList& packetList,
                              vector<uint32>& lastUpdateTime )
{
   PushServiceMap::iterator it = m_services.begin();

   if ( it != m_services.end() ) { 
      it->second->merge( service, packetList, lastUpdateTime );
      delete service;
   } else {
      m_services.insert( make_pair( service->getServiceID(), service ) );
      service->getLastUpdateTime( lastUpdateTime );
      checkAndCalculateTimeout( packetList );
   }
}


bool
PushServices::removePushService( uint32 serviceID,
                                 PacketContainerList& packetList )
{
   PushServiceMap::iterator it = m_services.find( serviceID );

   if ( it != m_services.end() ) {
      it->second->removeAllResources( packetList );

      return true;
   } else {
      return false;
   }
}


bool
PushServices::removePushServiceResource( uint32 serviceID, 
                                         SubscriptionResource& resource,
                                         PacketContainerList& packetList )
{
   PushServiceMap::iterator it = m_services.find( serviceID );

   if ( it != m_services.end() ) {
      return it->second->removeResource( resource, packetList );
   } else {
      return false;
   }
}


void
PushServices::handlePushPacket( PushPacket* pushPacket, 
                                TCPSocket* pushSocket,
                                PacketContainerList& packetList,
                                bool& isDataPacket,
                                uint32& serviceID, 
                                SubscriptionResource*& resource )
{
   uint32 pushServiceID = pushPacket->getServiceID();
   resource = NULL;
   isDataPacket = false;

   PushServiceMap::iterator it = m_services.find( pushServiceID );

   if ( it != m_services.end() ) {
      it->second->handlePushPacket( pushPacket, pushSocket, packetList,
                                    isDataPacket, resource );
      serviceID = pushServiceID;
   } else {
      // TODO: Send general unsubscribe for service
   }
}


void
PushServices::handleBrokenSocket( TCPSocket* pushSocket, 
                                  PacketContainerList& packetList )
{
   PushServiceMap::iterator it = m_services.begin();

   while ( it != m_services.end() ) {
      it->second->handleBrokenSocket( pushSocket, packetList );
      ++it;
   }
}


uint32
PushServices::checkAndCalculateTimeout( PacketContainerList& packetList ) {
   uint32 timeNextTimeOut = MAX_UINT32;
   uint32 timeOutTime = 0;
   PushServiceMap::iterator it = m_services.begin();

   while ( it != m_services.end() ) {
      timeOutTime = it->second->checkAndCalculateTimeout( packetList );
      if ( timeOutTime < timeNextTimeOut ) {
         timeNextTimeOut = timeOutTime;
      }
      ++it;
   }

   return timeNextTimeOut;
}


void 
PushServices::dump( ostream& out ) const {
   PushServiceMap::const_iterator it = m_services.begin();

   out << "Nbr servises: " << m_services.size() << endl;
   while ( it != m_services.end() ) {
      it->second->dump( out );
      ++it;
   }
}
