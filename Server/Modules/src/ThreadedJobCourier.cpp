/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ThreadedJobCourier.h"

#include "ISABThread.h"
#include "Processor.h"
#include "Packet.h"

/**
 * Implementation details for ThreadedJobCourier.
 */
struct ThreadedJobCourier::Impl: public ISABThread {
   Impl( ISABThreadGroupHandle threadGroup,
         Processor* processor,
         ThreadedJobCourier& courier,
         ThreadedJobCourier::ReplyQueue& replyQueue ):
      ISABThread( threadGroup.get() ),
      m_courier( courier ),
      m_replyQueue( replyQueue ),
      m_processor( processor )
   { }

   ~Impl() {
      delete m_processor;
   }

   /**
    * Thread function, process job in original JobCourier.
    * @see ISABThread::run()
    */
   void run();

   // Terminate thread
   void setTerminated() {
      terminate();
      // notify the thread so it dies.
      m_jobs.push( JobInfo() );
      delete m_processor;
      m_processor = NULL;
   }

   /// Holds information about current job
   struct JobInfo {
      JobInfo():
         m_job( NULL ),
         m_urgent( false ) {
      }

      /// The job to send to processor.
      Packet* m_job;

      /**
       * Whether or not the job is urgent.
       * @see JobCourier::takeJob(Packet,Reply,bool)
       */
      bool m_urgent;
   };

   /// Courier to use for calling.
   ThreadedJobCourier& m_courier;
   /// Put finished job and the result in this queue.
   ThreadedJobCourier::ReplyQueue& m_replyQueue;
   /// The processor for courier, it is owned and deleted by this.
   Processor* m_processor;
   /// The jobs from the dispatcher, usually only one.
   ConcurrentQueue< JobInfo > m_jobs;
};

ThreadedJobCourier::
ThreadedJobCourier( ISABThreadGroupHandle threadGroup,
                    Processor* processor,
                    ReplyQueue& replyQueue, uint32 threadIndex,
                    const PacketQueue& receiveQueue ):
   JobCourier( *processor, receiveQueue, threadIndex ),
   m_impl( new Impl( threadGroup, processor, *this, replyQueue ) ),
   m_threadHandle( m_impl ) {
   m_impl->start();
}

ThreadedJobCourier::~ThreadedJobCourier() {
   m_impl->setTerminated();
   m_impl->join();
}

void ThreadedJobCourier::takeJob( Packet* job, JobReply& reply, bool urgent ) {
   Impl::JobInfo info;
   info.m_job = job;
   info.m_urgent = urgent;
   m_impl->m_jobs.push( info );
}

ThreadedJobCourier::ReplyQueue& ThreadedJobCourier::getReplyQueue() {
   return m_impl->m_replyQueue;
}

void ThreadedJobCourier::Impl::run() {
   while ( ! terminated ) {
      JobInfo job;
      m_jobs.waitAndPop( job );
      // Got an empty job?, this means that someone wants to shutdown
      if ( job.m_job == NULL ) {
         terminated = true;
         break;
      }

      auto_ptr<JobReply> reply( new JobReply() );

      // Send job to base class courier.
      m_courier.JobCourier::takeJob( job.m_job, *reply, job.m_urgent );

      // Add reply from courier to reply data queue
      m_replyQueue.push( new CourierData( &m_courier,
                                          reply.release(),
                                          job.m_urgent ) );
   }
}
