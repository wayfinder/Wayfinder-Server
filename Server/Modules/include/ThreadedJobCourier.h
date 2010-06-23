/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef THREADED_JOBCOURIER_H
#define THREADED_JOBCOURIER_H

#include "JobCourier.h"
#include "ISABThread.h"
#include "ConcurrentQueue.h"
#include "JobReply.h"

class Processor;
class SelectableSelector;
class PacketQueue;

/**
 * This courier takes a job and process it in the background, in a thread.
 */
class ThreadedJobCourier: public JobCourier {
public:
   /**
    * Reply data from a work.
    */
   struct CourierData {
      CourierData( JobCourier* courier,
                   JobReply* reply,
                   bool urgent ):
         m_courier( courier ),
         m_reply( reply ),
         m_urgent( urgent ) {
      }

      ~CourierData() {
         delete m_reply;
      }

      /// The job courier that had a reply.
      JobCourier* m_courier;
      /// Reply from a courier
      JobReply* m_reply;
      /// Whether or not the request was urgent.
      bool m_urgent;
   };

   /// The reply queue type.
   typedef ConcurrentQueue< CourierData* > ReplyQueue;

   /**
    * Setup job courier work.
    * @param threadGroup The group that the work thread should belong to.
    * @param proc The one that does the real work. It will be deleted by this
    *             instance.
    * @param replyQueue The works progress will be put in this queue.
    */
   ThreadedJobCourier( ISABThreadGroupHandle threadGroup,
                       Processor* proc,
                       ReplyQueue& replyQueue,
                       uint32 threadIndex,
                       const PacketQueue& receiveQueue );
   ~ThreadedJobCourier();

   /**
    * Take a job and return directly.
    * If you add a job without waiting for the current job to finish, you will
    * get a memory leak.
    *
    * @see JobCourier::takeJob(Packet,bool)
    */
   void takeJob( Packet* job, JobReply& reply, bool urget = false );

   /// @return reply queue
   ReplyQueue& getReplyQueue();

private:
   struct Impl;
   /// Hidden implementation.
   Impl* m_impl;
   ISABThreadHandle m_threadHandle;
};

#endif // THREADED_JOBCOURIER_H
