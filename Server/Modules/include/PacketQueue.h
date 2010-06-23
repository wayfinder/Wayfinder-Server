/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PACKETQUEUE_H
#define PACKETQUEUE_H

#include "config.h"
#include "Queue.h"
#include "ISABThread.h"
#include <vector>
#include <queue>

class Packet;
/**
 *    Describes a queue of packets. The packets are inserted into the
 *    queue with the enqueue method and retreived with dequeue. The 
 *    packets are returned in priority order. {\it {\bf NB!} Internally 
 *    an array that is Packet::m_nbrPriorities elements big is used, so 
 *    if this becomes too large the implementation here might need to be
 *    changed.
 *
 */
class PacketQueue : public Queue {
   public:
      /**
       *    Create a new PacketQueue.
       */
      PacketQueue();

      /**
       *    Delete this object.
       */
      virtual ~PacketQueue();

      /**
       *    Insert one new packet into this queue.
       *    @param   p  The new packet to insert into this queue.
       *    @return  True if the packet is inserted correctly, false 
       *             otherwise.
       */
      virtual bool enqueue(Packet *p);

      /**
       *    Get the next packet from this packet queue. This method will
       *    hang if no packet available. The packets are returned in 
       *    priotity order (fifo within the same priority).
       *    @return  A packet from the queue.
       */
      virtual Packet* dequeue();

      /**
       *    Get a packet from the queue, but don't wait more than 
       *    maxWaitTime ms.
       *    @param   maxWaitTime The maximum time to wait for packets
       *                         in the queue. In milliseconds.
       *    @return  A packet from the queue or NULL if timeout.
       */
      virtual Packet *dequeue(uint32 maxWaitTime);

      /**
       *    Get a value of the statistics for this queue. Currently the
       *    length is used as statistics...
       *    @return  A value that represents the load of this module.
       */
      virtual uint32 getStatistics();

      /**
       *    Releases the monitor by notifying all. NOTE! This will make 
       *    dequeue() return NULL.
       */
      void terminate();
      
   private:
      /**
       *    Array with the queues.
       */
      std::vector<std::queue<Packet*> > m_queues;
      /**
       *    The monitor used to protect the enqueue- and dequeuemethods.
       */
      ISABMonitor m_monitor;

      /**
       *    Get the index if the first nonempty queue. A value that is
       *    >= Packet::m_nbrPriorities is returned if no such queue.
       */
      uint32 getFirstNonEmptyIndex();

      /**
       *    True if the queue is terminated, false otherwise.
       */
      bool m_terminated;
      
};

#endif
