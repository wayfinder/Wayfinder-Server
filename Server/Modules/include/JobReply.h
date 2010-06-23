/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef JOB_REPLY_H
#define JOB_REPLY_H

#include "config.h"
#include "MC2String.h"
#include "NotCopyable.h"

class Packet;

/**
 * The reply data from JobCourier::takeJob
 * @see JobCourier::takeJob(Packet,JobReply,bool)
 */
struct JobReply: private NotCopyable {
   JobReply();
   ~JobReply();

   /// Reply packet to send back.
   Packet* m_reply;
   /// The request for this reply.
   Packet* m_request;
   /// Input queue info
   MC2String m_receiveQueueInfo;
   /// various information
   MC2String m_infoString;
   /// Processing time in milliseconds.
   uint32 m_processingTimeMillis;
   /// CPU time in milliseconds.
   float m_processorTimeMillis;
   /// Time since package arrival.
   uint32 m_timeSinceArrival;
   /// Whether the reply was created from a cached packet.
   bool m_cached;
};

#endif // JOB_REPLY_H
