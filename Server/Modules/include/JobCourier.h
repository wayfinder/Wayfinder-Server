/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef JOBCOURIER_H
#define JOBCOURIER_H

#include "config.h"

#include "MC2String.h"
#include "PacketQueue.h"

class Packet;
class Processor;
class JobLogger;
struct JobReply;

/**
 * Takes packages and delivers them to the processor.
 * Does the pre- and postprocess of timers and logging.
 * This should be used in colaboration with JobDispatcher.
 * @see JobDispatcher
 */
class JobCourier {
public:
   /**
    * Setup a courier to handle packets.
    * @param processor
    * @param recieveQueue Incoming packets queue, used for logging.
    * @param id The id of this courier, used in log prefix.
    */
   explicit JobCourier( Processor& processor,
                        const PacketQueue& receiveQueue = PacketQueue(),
                        uint32 id = MAX_UINT32 );
   virtual ~JobCourier();

  /**
   * Accept the job and process it.
   * @param pack The job.
   * @param urgent Whether or not the packet is urgent and thus should skip
   *               any logging and go directly to the processor.
   */
   virtual void takeJob( Packet* pack, JobReply& reply, bool urgent = false );

private:

   /**
    * Set the log prefix to correspond to the passed packet
    * @param  packet Pointer to packet to base the prefix on, NULL
    *                to reset prefix to default.
    */
   void setLogPrefix( const Packet* packet );

   /// The real work horse, process the packages.
   Processor& m_processor;

   auto_ptr<JobLogger> m_logger;
};

#endif // JOBCOURIER_H
