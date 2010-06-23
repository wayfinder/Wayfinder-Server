/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FIFO_JOB_SCHEDULER_H
#define FIFO_JOB_SCHEDULER_H

#include "config.h"
#include "JobScheduler.h"
#include "ConcurrentQueue.h"

/**
 * Each runnable gets a job in a first in first out way.
 * TODO: Make exception safe initialization of the scheduler.
 *       ( @see MultiJobDispatcher )
 */
class FIFOJobScheduler: public JobScheduler {
public:
   /// @param maximum number of runnables that this will handle
   explicit FIFOJobScheduler( uint32 maxRunnables );
   ~FIFOJobScheduler();

   /**
    * Enqueues a new runnable.
    * @param newRunnable A new runnable to be scheduled.
    * @see JobScheduler::addInitialRunnable(Runnable)
    */
   virtual void addInitialRunnable( Runnable newRunnable );

   /**
    * Dequeues the next runnable.
    * @return next runnable in queue
    * @see JobScheduler::getRunnable(Packet)
    */
   virtual Runnable getRunnable( const Packet& packet );

   /**
    * Enqueues the runnable.
    * @param runnable Will be added back in to the queue.
    * @see JobScheduler::addRunnable(Runnable)
    */
   virtual void addRunnable( Runnable runnable );

private:
   /// Initial number of runnables
   const uint32 m_initialNumberOfRunnables;
   /// Runnables that are ready to be used.
   ConcurrentQueue< Runnable > m_runnables;
};

#endif // FIFO_JOB_SCHEDULER_H
