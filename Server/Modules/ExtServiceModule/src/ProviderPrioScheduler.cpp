/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ProviderPrioScheduler.h"

#include "ThreadedJobCourier.h"

#include "ExtServices.h"
#include "ExternalSearchRequestData.h"
#include "ExternalSearchPacket.h"
#include "ExternalItemInfoPacket.h"
#include "UserRightsMapInfo.h"
#include "DummyCourier.h"
#include "Properties.h"

namespace {

/**
 * Get service id from packet.
 * Assumes the packet is either external search or external item info.
 * @param job
 * @return service ID from packet.
 */
ExtService getServiceID( const Packet& job ) {

   // TODO: The packets should have a service ID more "visible"
   //       in the data so it does not have to go through
   //       ExternalSearchRequestData and USerRightsMapInfo each time.

   uint32 subtype = job.getSubType();
   if ( subtype == Packet::PACKETTYPE_EXTERNALSEARCH_REQUEST ) {

      const ExternalSearchRequestPacket* extReq =
         static_cast<const ExternalSearchRequestPacket*>( &job );
      ExternalSearchRequestData searchData;
      UserRightsMapInfo rights;
      extReq->get( searchData, rights );

      return searchData.getService();

   } else if ( subtype == Packet::PACKETTYPE_EXTERNALITEMINFO_REQUEST ) {

      const ExternalItemInfoRequestPacket* extReq =
         static_cast<const ExternalItemInfoRequestPacket*>( &job );

      uint32 service;
      MC2String externalID;
      UserRightsMapInfo rights;
      LangType lang;
      MC2Coordinate coord;
      MC2String name;

      extReq->get( service, externalID, lang, rights, coord, name );

      return service;
   }

   return ExtService::nbr_services;
}

}

ProviderPrioScheduler::ProviderPrioScheduler():
   m_initialNumberOfRunnables( 0 ),
   m_dummyRunnable( NULL ),
   MAX_SERVICE_TIME_MS( Properties::
                        getUint32Property("EXT_SERVICE_MODULE_MAX_SERVICE_TIME",
                                          4000 ) ) {

}

ProviderPrioScheduler::~ProviderPrioScheduler() {
   delete m_dummyRunnable;
}

void ProviderPrioScheduler::addInitialRunnable( Runnable newRunnable ) {

   // setup the dummy runnable first
   if ( m_dummyRunnable == NULL ) {
      ThreadedJobCourier* threadCourier =
         dynamic_cast< ThreadedJobCourier* >( newRunnable );
      if ( threadCourier ) {
         // hook up the dummy on the reply queue
         m_dummyRunnable = DummyCourier::
            createCourier( threadCourier->getReplyQueue() );
      }
   }

   m_freeRunnables.enqueue( newRunnable );
   m_initialNumberOfRunnables++;
}

void ProviderPrioScheduler::sendTestPacket( ServiceID serviceID,
                                            const Packet& job ) {
   // need more than one free runnable to send a test packet
   // so we keep at least one runnable ready for other providers.
   if ( m_freeRunnables.getSize() > 1 ) {
      Runnable testRunnable = m_freeRunnables.dequeue();
      if ( testRunnable == NULL ) {
         return;
      }

      Packet* clonedJob = job.getClone();
      // set invalid address and port so we do not reply anything
      clonedJob->setOriginIP( 0 );
      clonedJob->setOriginPort( 0 );
      // the reply data is not important here
      JobReply reply;
      testRunnable->takeJob( clonedJob,
                             reply,
                             // no urgent packets for ExtServiceModule
                             false );
      ISABSync queueSync( m_runnableMutex );
      m_runnableQueue[ testRunnable ].push( ServiceTime( serviceID ) );
   }
}

JobScheduler::Runnable ProviderPrioScheduler::getRunnable( const Packet& job ) {

   ServiceID serviceID = ExtService::nbr_services;

   // see if we have any blacklisted provider
   if ( m_dummyRunnable &&
        (job.getSubType() == Packet::PACKETTYPE_EXTERNALSEARCH_REQUEST ||
         job.getSubType() == Packet::PACKETTYPE_EXTERNALITEMINFO_REQUEST) ) {

      serviceID = ::getServiceID( job );

      ISABSync blacklistSync( m_blacklistMutex );
      if ( m_blacklist.find( serviceID ) != m_blacklist.end() ) {
         mc2dbg << "[ProviderPrioScheduler] service ID: " << serviceID
                << " is blacklisted, will not do any real search." << endl;
         sendTestPacket( serviceID, job );
         return m_dummyRunnable;
      }

   }

   Runnable runnable = m_freeRunnables.dequeue();

   ISABSync queueSync( m_runnableMutex );
   m_runnableQueue[ runnable ].push( ServiceTime( serviceID ) );

   return runnable;
}

void ProviderPrioScheduler::addRunnable( Runnable finished ) {
   //
   // Check average time
   //
   auto_ptr<ISABSync> queueSync( new ISABSync( m_runnableMutex ));
   RunnableQueue::iterator runnable = m_runnableQueue.find( finished );

   // If we have a runnable in queue and the service stack is not empty
   // then check the service time
   if ( runnable != m_runnableQueue.end() &&
        ! runnable->second.empty() ) {
      ServiceTime service = runnable->second.front();
      runnable->second.pop();
      // no more runnable queue sync needed
      queueSync.reset( 0 );

      // valid service?
      if ( service.m_serviceID != ExtService::nbr_services ) {

         //
         // Whitelist or blacklist the provider?
         //

         // black list the service if its outside the time range
         if ( service.m_time.getTime() > MAX_SERVICE_TIME_MS ) {
            ISABSync blacklistSync( m_blacklistMutex );
            m_blacklist.insert( service.m_serviceID );

            mc2dbg << "[ProviderPrioScheduler]"
                   << " Max service time(" << MAX_SERVICE_TIME_MS
                   << " ms) reached, blacklisting provider: "
                   << service.m_serviceID << endl;
         } else {

            // If the service id was blacklisted before then
            // white list it.

            ISABSync blacklistSync( m_blacklistMutex );
            ServiceIDSet::iterator blackIt =
               m_blacklist.find( service.m_serviceID );
            if ( blackIt != m_blacklist.end() ) {
               m_blacklist.erase( blackIt );
               mc2dbg << "[ProviderPrioScheduler]"
                      << " whitelisting provider: "
                      << service.m_serviceID
                      << endl;
            }

         }
      }
   }
   queueSync.reset( 0 );

   // Readd runnable to free runnables
   if ( finished != m_dummyRunnable ) {
      m_freeRunnables.enqueue( finished );
   }
}
