/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SINGLE_JOBDISPATCHER_H
#define SINGLE_JOBDISPATCHER_H

#include "JobDispatcher.h"

#include <memory>

class Processor;
class JobCourier;
class PacketCache;
class PacketSender;
class PacketQueue;
class Packet;
class JobLogger;

/**
 * A dispatcher that only process one job at the time.
 */
class SingleJobDispatcher: public JobDispatcher {
public:
   /**
    * @param jobCache Holds cached packages.
    * @param sender Send from courier reply.
    * @param proc Handles the packages and creates reply.
    * @param receiveQueue Input queue, only for logging.
    */
   explicit SingleJobDispatcher( PacketCache& jobCache,
                                 PacketSender& sender,
                                 Processor& proc,
                                 const PacketQueue& receiveQueue );
   virtual ~SingleJobDispatcher();

   /**
    * Process one job and wait until it is finished.
    * @see JobDispatcher::dispatch(Packet)
    */
   void dispatch( Packet* packet );

   /**
    * Process one job and wait until it is finished.
    * @see JobDispatcher::dispatchDirect(Packet)
    */
   void dispatchDirect( Packet* packet );

private:
   /// Sends packet to processor
   std::auto_ptr<JobCourier> m_courier;
   /// For logging when we have cache.
   std::auto_ptr<JobLogger> m_cacheLog;
};

#endif //  SINGLE_JOBDISPATCHER_H
