/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DUMMY_COURIER_H
#define DUMMY_COURIER_H

#include "ThreadedJobCourier.h"
#include "ISABThread.h"

/**
 * This courier creates a FakeExtServiceProcessor which respons
 * with empty search results for any provider.
 */
class DummyCourier: public JobCourier {
public:
   ~DummyCourier();

   /**
    * @see JobCourier::takeJob(Packet,JobReply,bool)
    */
   void takeJob( Packet* pack, JobReply& reply, bool urgent = false );

   /**
    * Creates a dummy courier with a fake ext service processor.
    * @param queue The reply queue to put fake replies in.
    * @return a newly created courier
    */
   static DummyCourier* createCourier( ThreadedJobCourier::ReplyQueue& queue );
private:
   /**
    * @param proc The fake processor.
    * @param replyQueue Will put replies from the fake processor here.
    */
   DummyCourier( Processor* proc,
                 ThreadedJobCourier::ReplyQueue& replyQueue );

   /// Will put replies in this.
   ThreadedJobCourier::ReplyQueue& m_reply;
   /// lock during takeJob processing.
   ISABMutex m_jobLock;
   /// The processor that fakes replies.
   Processor* m_processor;
};

#endif // DUMMY_COURIER_H
