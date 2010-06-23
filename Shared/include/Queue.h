/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef QUEUE_H
#define QUEUE_H

#include "config.h"

class Packet;

/**
 *    Abstract superclass that describes a packet queue. Contains two
 *    virtual methods to enqueue and dequeue packets from the queue.
 *
 */
class Queue {
   public:
      /**
       *    Destructor.
       */
      virtual ~Queue() {;}

      /**
       *    Insert a new packet into this queue.
       *    @param   p  The packet to enqueue.
       *    @return  True on success, false otherwise.
       */
      virtual bool enqueue(Packet *p) = 0;

      /**
       *    Get a packet from the queue. This method should never return
       *    NULL. If the queue is empty it should hang until any packet
       *    is available.
       *    
       *    @return  A packet from the queue.
       */
      virtual Packet* dequeue() = 0;
      
      /**
       *    Get a packet from the queue, but maximum a given time. This
       *    means that this method might return NULL if the time-out is
       *    reached before any packet is available in the queue.
       *    
       *    @return  A packet from the queue, NULL is returned if timed 
       *             out.
       */
      virtual Packet* dequeue(uint32 maxWaitTime) = 0;

      /**
       *    Get the statistics of this queue. This is intened to be a
       *    measurement of the time to process the packtes in this
       *    queue.
       *
       *    @return  A value of the statistics for this queue.
       */
      virtual uint32 getStatistics() = 0;
      
   protected:
      /**
       *    This class is abstract and should not be instantiated.
       */
      Queue() {;}
};

#endif // QUEUE_H

