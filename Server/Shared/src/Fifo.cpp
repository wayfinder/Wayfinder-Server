/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Fifo.h"
#include "Packet.h"

Fifo::Fifo()
{
   ISABSync synchronized(m_monitor);
   m_terminated = false;
   m_wakeUp = false;
}

Fifo::~Fifo()
{
   ISABSync synchronized(m_monitor);
   
   while ( ! m_queue.empty() ) {
      delete m_queue.front();
      m_queue.pop();
   }
}

bool
Fifo::enqueue(Packet* p)
{
   // Make sure no one else is changing the list
   ISABSync synchronized(m_monitor);

   // Insert the packet into the queue if not NULL
   if (p != NULL) {
      mc2dbg8 << "Fifo::enqueue  = " << ", subType = " 
              << int(p->getSubType()) << endl;
      m_queue.push( p );
      m_monitor.notifyAll();
      return true;
   } else {
      // NULL means that we should not wait for long now.
      m_wakeUp = true;
      m_monitor.notifyAll();      
      return false;
   }
}

Packet*
Fifo::dequeue(uint32 maxWaitTime)
{
   // Make sure no one else is changing the list
   ISABSync synchronized(m_monitor);

   if (!m_wakeUp && m_queue.empty() && !m_terminated) {
      try {
         if ( maxWaitTime != MAX_UINT32 ) {
            m_monitor.wait(maxWaitTime);
         } else {
            m_monitor.wait();
         }
      } catch ( const JTCInterruptedException & ) {
         mc2log << warn << "Fifo::dequeue wait( int ) interrupted!" << endl;
      }
   }

   if ( m_wakeUp ) {
      m_wakeUp = false;
   }
   
   // If we have a packet to return, we return it here
   if (!m_queue.empty() && !m_terminated) {
      Packet *p = m_queue.front();
      m_queue.pop();
      mc2dbg8 << "Fifo::Dequeue  = " << ", subType = " 
             << int(p->getSubType()) << endl;
      return p;
   } else {
      return NULL;
   }
}


Packet*
Fifo::dequeue()
{
   return dequeue(MAX_UINT32);
}

void Fifo::terminate()
{
   ISABSync synchronized(m_monitor);
   m_terminated = true;
   m_monitor.notifyAll();
}
