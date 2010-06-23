/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PROVIDER_PRIO_SCHEDULER_H
#define PROVIDER_PRIO_SCHEDULER_H

#include "JobScheduler.h"
#include "DebugClock.h"
#include "PointerFifoTemplate.h"
#include "ISABThread.h"

#include <map>
#include <queue>
#include <set>

/**
 * This scheduler priorities external service providers
 * to specific couriers and blacklists providers that takes
 * too long to search in.
 */
class ProviderPrioScheduler: public JobScheduler {
public:
   ProviderPrioScheduler();
   ~ProviderPrioScheduler();

   /**
    * @see JobScheduler::addInitialRunnable(Runnable)
    */
   virtual void addInitialRunnable( Runnable newRunnable );

   /**
    * Returns a courier that should handle the packet.
    * @see JobScheduler::getRunnable(Packet)
    */
   virtual Runnable getRunnable( const Packet& job );

   /**
    * @see JobScheduler::addRunnable(Runnable)
    */
   virtual void addRunnable( Runnable finished );

private:
   /// External service id
   typedef uint32 ServiceID;
   typedef uint32 Milliseconds;

   /// Sends a test packet to be checked
   void sendTestPacket( ServiceID serviceID,
                        const Packet& job );

   /**
    * Information about service and the time it took
    * to process it.
    */
   struct ServiceTime {
      explicit ServiceTime( ServiceID id ):
         m_serviceID( id ) {
      }
      /// The service id.
      ServiceID m_serviceID;
      /// Total processing time in milliseconds for this service.
      DebugClock m_time;
   };


   typedef std::set< ServiceID > ServiceIDSet;

   /// Maps runnable to the current running service time
   typedef std::map< Runnable, std::queue< ServiceTime > > RunnableQueue;

   /// Current runnables with their service id
   RunnableQueue m_runnableQueue;
   /// Sync for m_runnableQueue
   ISABMutex m_runnableMutex;

   /// Runnables that are free and ready to be handled.
   PointerFifoTemplate< RunnableType > m_freeRunnables;
   /// Services that are blacklisted due to various reasons.
   ServiceIDSet m_blacklist;
   /// Mutex for m_blacklist
   ISABMutex m_blacklistMutex;

   /// Total number of runnables that were created at startup.
   uint32 m_initialNumberOfRunnables;

   /// A runnable that fakes events
   Runnable m_dummyRunnable;
   /// maximum service reply time in milliseconds
   const uint32 MAX_SERVICE_TIME_MS;
};

#endif // PROVIDER_PRIO_SCHEDULER_H
