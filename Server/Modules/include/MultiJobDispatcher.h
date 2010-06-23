/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MULTI_JOBDISPATCHER_H
#define MULTI_JOBDISPATCHER_H

#include "config.h"

#include "JobDispatcher.h"

class PacketSender;
class PacketCache;
class PacketQueue;
class ProcessorFactory;
class JobScheduler;

/**
 * Handles multiple JobCouriers in different threads.
 *
 * When a job is dispatched, the dispatcher asks the scheduler
 * (@see JobScheduler) for a free courier and sends the request to it.
 *
 * Once the work is done by the courier it puts itself in the reply set together
 * with the work result.
 *
 * The replies are dequeued in a reply thread and sent back to the origin, and
 * the courier that was associated with the reply is then put back in the
 * scheduler again.
 * \verbatim
 *
 *   Incoming Job
 *       o
 *       |
 *       |
 *   +---v------+              +-----------+
 *   | dispatch | get runnable | scheduler |
 *   |          <--------------<           |
 *   +---v------+              +----^------+
 *       |                          |
 *       | put it to work           | add runnable
 *       o                          |
 *                                  |
 *                                  |
 *          +------------+          |
 *          |  Reply     >----------+
 *          |  Thread    | work done
 *          +------------+
 * \endverbatim
 * @see SingleMultiJobDispatcher
 */
class MultiJobDispatcher: public JobDispatcher {
public:
   /**
    * Create dispatcher with a set of workers.
    * @param jobCache Cached replies.
    * @param sender Sends replies from workers.
    * @param procCreator Creator for processors ( each worker needs one ).
    * @param nbrThreads Number of workers.
    * @param scheduler Determines which courier to take for a specific package.
    * @param receiveQueue For fetching current status of the receive queue.
    */
   MultiJobDispatcher( PacketCache& jobCache,
                       PacketSender& sender,
                       ProcessorFactory& procCreator,
                       uint32 nbrThreads,
                       JobScheduler* scheduler,
                       const PacketQueue& receiveQueue );
   ~MultiJobDispatcher();

   /**
    * Dispatch the packet to an idle worker.
    * Will block until there is a free worker to take the
    * job.
    * @see JobDispatcher::dispatch(Packet)
    */
   void dispatch( Packet* packet );

   /**
    * Dispatch the packet to an idle worker.
    * Will block until there is a free worker to take the
    * job.
    * @see JobDispatcher::dispatchDirect(Packet)
    */
   void dispatchDirect( Packet* packet );

private:
   struct Impl;
   Impl* m_impl;
};

#endif // MULTI_JOBDISPATCHER_H

