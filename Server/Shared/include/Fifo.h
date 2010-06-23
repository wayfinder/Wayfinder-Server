/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FIFO_H
#define FIFO_H

#include "config.h"
#include "Queue.h"
#include "ISABThread.h"
#include <queue>

/**
 *    An simple (and fast!) implementation of a queue that uses the
 *    first-in-first-out algorithm. The methods are protected with
 *    a monitor to make them safe.
 *
 */
class Fifo : protected Queue {
   public:
      /**
       *    Create an empty fifo-queue.
       */
      Fifo();
   
      /**
       *    Delete the fifo-queue.
       */
      virtual ~Fifo();

      /**
       *    Insert one packet into the queue. The packet is not copied,
       *    so it must not be deleted by the caller.
       *    If NULL is enqued, the queue will be woken up once.
       *    @param   p  The packet to insert into the queue.
       *    @return  True if the packet is inserted, false otherwise.
       */
      virtual bool enqueue(Packet* p);

      /**
       *    Get one packet from the queue. If the queue is empty this
       *    method will hang until there is a packet to return. So NULL
       *    will not be returned, unless terminate() is called!
       *    @return  One packet in the queue.
       */
      virtual Packet* dequeue();

      /**
       *    Get one packet from the queue. If the queue is empty this
       *    method will hang until there is a packet to return, but
       *    maximum maxWaitTime. If the timeout is reached or if 
       *    terminate() is called, NULL is returned.
       *    
       *    @param   maxWaitTime The maximum time to wait for packets
       *                         in the queue. In milliseconds.
       *    @return  One packet in the queue.
       */
      virtual Packet* dequeue(uint32 maxWaitTime);

      /**
       *    Get some statistics about the packets in this queue. This will
       *    be implemented in a better way (currently the number of elements
       *    in the queue is returned).
       *    @return  The statistics for the packets in this queue.
       */
      virtual uint32 getStatistics();

      /**
       *    Returns the number of Packets in the Fifo.
       */ 
      inline int cardinal() const;
      
      /**
       *    Releases the monitor by notifying all. NOTE! This will make 
       *    dequeue() return NULL.
       */
      void terminate();

   private:
      /**
       *    The monitor used to protect the enqueue- and dequeue-methods.
       */
      ISABMonitor m_monitor;

      /**
       *    The list to store the packets.
       */
      queue<Packet*> m_queue;

      /**
       *    True if the queue is terminated, false otherwise.
       */
      bool m_terminated;

      /**
       *    True if a NULL packet was enqueued and the fifo should
       *    wake up even if it is empty.
       */
      bool m_wakeUp;
      
      /**
       * Private copy constructor.
       */
      Fifo(const Fifo& rhs) {
         // don't use this one
      }
};

// ------------- Inlined implementations.

inline int
Fifo::cardinal() const
{
   return m_queue.size();
}

inline uint32
Fifo::getStatistics() 
{
   return cardinal();
}

#endif

