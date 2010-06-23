/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MultiJobDispatcher.h"

#include "ProcessorFactory.h"
#include "DeleteHelpers.h"
#include "ISABThread.h"
#include "PointerFifoTemplate.h"
#include "ThreadedJobCourier.h"

#include "JobLogger.h"
#include "JobReply.h"
#include "JobScheduler.h"

/**
 * Implementation details for MultiJobDispatcher.
 */
struct MultiJobDispatcher::Impl: public JobDispatcher,
                                 public ISABThread {
   /// @copydoc MultiJobDispatcher::MultiJobDispatcher(PacketCache,PacketSender,ProcessorFactory,uint32,PacketQueue)
   Impl( PacketCache& jobCache,
         PacketSender& sender,
         ProcessorFactory& procCreator,
         uint32 nbrThreads,
         JobScheduler* scheduler,
         const PacketQueue& receiveQueue );

   ~Impl();

   /// @copydoc JobDispatcher::dispatch(Packet)
   void dispatch( Packet* packet );

   /// @copydoc JobDispatcher::dispatchDirect(Packet)
   void dispatchDirect( Packet* packet );

   /// Same as the functions above.
   void dispatch( Packet* packet, bool urgent );

   /**
    * Thread function that waits for replies from courier,
    * sends replies and puts them back into the scheduler.
    * @see ISABThread::run()
    */
   void run();

   /**
    * Stop work threads. This will wait until all threads are finished.
    * This means that any work that are in progress will be finalized and
    * the reply will be sent.
    */
   void stop();

   /// A cache for jobs.
   PacketCache& m_jobCache;
   /// The reply sender.
   PacketSender& m_sender;

   typedef ThreadedJobCourier::ReplyQueue ReplyQueue;

   /// Reply data waiting to be sent away.
   ReplyQueue m_replies;

   /// Lock for sending replies.
   ISABMutex m_replyLock;

   /// Group for Threaded dispatchers.
   ISABThreadGroupHandle m_threadGroup;

   /// Log info while handling cached packets.
   JobLogger m_cacheLogger;

   auto_ptr< JobScheduler > m_jobScheduler;

   /**
    * Statistic about packets during the MultiJobDispatchers lifetime.
    */
   struct Statistics {
      Statistics():
         m_dispatched( 0 ),
         m_replies( 0 ),
         m_cached( 0 ) {
      }
      /// Number of dispatched packets.
      uint64 m_dispatched;
      /// Number of replies. (Should match dispatched)
      uint64 m_replies;
      /// Number of cache hits.
      uint64 m_cached;
   } m_statistics;
};

MultiJobDispatcher::
MultiJobDispatcher( PacketCache& jobCache,
                    PacketSender& sender,
                    ProcessorFactory& procCreator,
                    uint32 nbrThreads,
                    JobScheduler* scheduler,
                    const PacketQueue& receiveQueue ):
   JobDispatcher( jobCache, sender ),
   m_impl( new Impl( jobCache, sender, procCreator, nbrThreads,
                     scheduler, receiveQueue ) ) {
   m_impl->start();
}

MultiJobDispatcher::~MultiJobDispatcher() {
   m_impl->stop();
}

void MultiJobDispatcher::dispatch( Packet* packet ) {
   m_impl->dispatch( packet );
}

void MultiJobDispatcher::dispatchDirect( Packet* packet ) {
   m_impl->dispatchDirect( packet );
}

MultiJobDispatcher::
Impl::Impl( PacketCache& jobCache,
            PacketSender& sender,
            ProcessorFactory& procCreator,
            uint32 nbrThreads,
            JobScheduler* scheduler,
            const PacketQueue& receiveQueue ):
   JobDispatcher( jobCache, sender ),
   m_jobCache( jobCache ),
   m_sender( sender ),
   m_threadGroup( new ISABThreadGroup( "JobDispatcherGroup" ) ),
   m_cacheLogger( MAX_UINT32, receiveQueue ),
   m_jobScheduler( scheduler ),
   m_statistics() {
   // Add slackers
   for ( uint32 i = 0; i < nbrThreads; ++i ) {
      m_jobScheduler->
         addInitialRunnable( new ThreadedJobCourier( m_threadGroup,
                                                     procCreator.create(),
                                                     m_replies,
                                                     i,
                                                     receiveQueue ) );
   }

}

MultiJobDispatcher::Impl::~Impl() {
   // Print some nice statistics
   mc2dbg << "[MultiJobDispatcher] Number of jobs dispatched: "
          << m_statistics.m_dispatched << endl;
   mc2dbg << "[MultiJobDispatcher] Number of replies: "
          << m_statistics.m_replies << endl;
   mc2dbg << "[MultiJobDispatcher] Number of cache hits: "
          << m_statistics.m_cached << endl;
}

void MultiJobDispatcher::Impl::stop() {
   mc2dbg << "[MultiJobDispatcher] Thread is stopping." << endl;
   // make sure all runnables dies.
   m_jobScheduler.reset( 0 );

   // stop reply thread
   terminate();
   // wake reply thread
   m_replies.push( NULL );

}

void MultiJobDispatcher::Impl::dispatch( Packet* packet ) {
   dispatch( packet, false );
}

void MultiJobDispatcher::Impl::dispatchDirect( Packet* packet ) {
   dispatch( packet, true );
}

void MultiJobDispatcher::Impl::dispatch( Packet* packet, bool urgent ) {
   JobReply reply;
   // check cache
   {
      ISABSync replySync( m_replyLock );
      // start cache logging
      auto_ptr<JobLogger::LogScope>
         cacheLogScope( new
                        JobLogger::LogScope( m_cacheLogger, packet, reply ) );
      // urgent packages should not be in cache.
      if ( ! urgent && checkCache( packet, reply ) ) {
         // force output of cache log
         cacheLogScope.reset( 0 );

         sendReply( reply );
         // update statistics
         m_statistics.m_replies++;
         m_statistics.m_dispatched++;
         m_statistics.m_cached++;
         return;
      }
      // Disable cachelog since there was no cache and
      // let the courier take care of the logging.
      cacheLogScope->disable();
   }

   // No cache, get a slacker and put it to work!
   JobScheduler::Runnable slacker = m_jobScheduler->getRunnable( *packet );

   // Reply data is not important here, the courier will create new one
   slacker->takeJob( packet, reply, urgent );
   // update statistics
   m_statistics.m_dispatched++;
}

void MultiJobDispatcher::Impl::run() {
   while ( ! terminated ) {
      // Wait for a reply from a worker
      auto_ptr< ThreadedJobCourier::CourierData > replyData;
      {
         ThreadedJobCourier::CourierData* data;
         m_replies.waitAndPop( data );
         replyData.reset( data );
      }
      if ( replyData.get() == NULL ) {
         mc2dbg << "[MultiJobDispatcher] Got empty reply." << endl;
         continue;
      }

      m_jobScheduler->addRunnable( replyData->m_courier );

      { // Send reply
         ISABSync replySync( m_replyLock );
         // dont send replies that came from urgent requests.
         // See JobDispatcher::dispatchDirect
         if ( ! replyData->m_urgent ) {
            sendReply( *replyData->m_reply, true );
            m_statistics.m_replies++;
         }
      }
   }
}
