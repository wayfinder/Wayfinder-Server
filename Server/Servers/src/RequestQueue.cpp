/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RequestQueue.h"


RequestQueue::RequestQueue() : ISABMonitor()
{
   terminated = false;
}


RequestQueue::~RequestQueue()
{
   DEBUG4( cerr << "~RequestQueue()" << endl );
}


RequestContainer*
RequestQueue::dequeue()
{
   ISABSync sync(*this);

   while ((empty()) && (!terminated)) {
      try {
         wait();
      }
      catch(const JTCInterruptedException &) {}
   }

   if (! terminated) {
      RequestContainer *req = static_cast<RequestContainer *>( first() );
      req->out();
      return req;
   } else {
      cerr << "RequestQueue::dequeue() terminated!!!" << endl;
      return (NULL);
   }
}


bool 
RequestQueue::enqueue(RequestContainer* req)
{
  ISABSync sync(*this);

   if (req != NULL) {
      req->into(this);
      notifyAll();
      return (true);
   } else {
      return (false);
   }
}


void
RequestQueue::terminate() {
   ISABSync sync(*this);
   terminated = true;
   notifyAll();
}

