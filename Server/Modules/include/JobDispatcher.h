/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef JOBDISPATCHER_H
#define JOBDISPATCHER_H

#include "config.h"

class Packet;
class PacketCache;
class PacketSender;
struct JobReply;

/**
 * Dispatch packet jobs to processor(s), takes care
 * of packet cache, and send the reply back to the originator.
 */
class JobDispatcher {
public:
   JobDispatcher( PacketCache& cache,
                  PacketSender& sender );

   virtual ~JobDispatcher() {
   }

   /**
    * Dispatch a packet to processor(s)
    */
   virtual void dispatch( Packet* packet ) = 0;

   /**
    * Dispatch a packet directly to processor(s) without
    * handling various pre and post processing.
    * This should only be used by very special packages,
    * such as the "periodic" package. The reply packet for these
    * will not be sent.
    */
   virtual void dispatchDirect( Packet* packet ) = 0;

protected:
   /**
    * Check in packet cache for cached packages.
    * @param packet The packet to find in cache.
    * @param reply Will be filled in with the cached reply.
    * @return true if the package was cached and the reply is filled
    *         with the cached answer.
    */
   bool checkCache( Packet* packet, JobReply& reply );

   /**
    * Send the reply to the packet originator and add it to cache.
    * @param reply The reply to send.
    * @param cache Whether or not to add it to cache ( although the cache
    *              might not accept it.) .
    */
   void sendReply( JobReply& reply, bool cache = true );

private:
   PacketCache& m_cache;
   PacketSender& m_sender;
};

#endif // JOBDISPATCHER_H
