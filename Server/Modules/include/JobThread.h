/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef JOBTHREAD_H
#define JOBTHREAD_H

#include "config.h"
#include "ISABThread.h"
#include "Vector.h"

#include "Fifo.h"
#include "PacketQueue.h"
#include "QueuedPacketSender.h"

#include <memory>

class Processor;
class PacketPool;
class PacketCache;
class PacketQueue;
class DatagramSender;
class Packet;
class PacketSender;
class RequestPacket;
class FixedSizeString;
class JobDispatcher;
class JobScheduler;
class ProcessorFactory;

/**
 * It is one object of this class that have the thread that do
 * the real work in the modules. It waits for packets from the
 * reader (via the Queue::dequeueu()-method) and gives them to
 * the dispatcher. The reply is handled by the dispatcher.
 *
 * For a single processor setup:
 * \verbatim
 *    +--------------+
 *    |    Reader    |
 *    +------+-------+
 *PacketQueue|
 *    +------v-------+
 *    |  JobThread   |
 *    +------+-------+
 *  Packet   |
 *    +------v-------+   Reply +--------------+
 *    | JobDispatcher+-------->|  SendQueue   |
 *    +----+----^----+         +--------------+
 *  Packet |    | Reply
 *    +----v----+----+
 *    | JobCourier   |
 *    +----+----^----+
 *  Packet |    | Reply
 *    +----v----+----+
 *    |  Processor   |
 *    +--------------+
 * \endverbatim
 *
 * Example of a multi processor setup:
 *
 * \verbatim
 *    +--------------+
 *    |    Reader    |
 *    +------+-------+
 *PacketQueue|
 *    +------v-------+
 *    |  JobThread   |
 *    +------+-------+
 *  Packet   |
 *    +------v----------------------------+ Reply +--------------+
 *    |        JobDispatcher              +------>|  SendQueue   |
 *    +----+----^---------------+----^----+       +--------------+
 *  Packet |    | Reply  Packet |    | Reply
 *    +----v----+----+     +----v----+----+
 *    | JobCourier   |     | JobCourier   |
 *    +----+----^----+     +----+----^----+
 *  Packet |    | Reply  Packet |    | Reply
 *    +----v----+----+     +----v----+----+
 *    |  Processor   |     |  Processor   |
 *    +--------------+     +--------------+
 * \endverbatim
 *
 *
 */
class JobThread : public ISABThread {
public:

   /**
    * Creates a JobThread with multiple processors.
    *
    * @param factory Creates processors.
    * @param nbrProcessors Maximum number of processors.
    * @param scheduler Schedules couriers, if NULL a FIFO scheduler will be
    *                  used.
    * @param q Incoming packets.
    * @param sendQueue Outgoing reply packets.
    * @param verbose Whether to give more debug output.
    * @param tcpPacketTypes Contains packet types that should always
    *                       be sent via TCP.
    */
   JobThread( ProcessorFactory& factory, uint32 nbrProcessors,
              JobScheduler* scheduler,
              PacketQueue *q,
              QueuedPacketSender::SendQueue& sendQueue,
              bool verbose = false,
              const Vector* tcpPacketTypes = NULL );

   /**
    *    Create a new job thread that uses a given processor to
    *    do the actual caluculations.
    *    @param   processor   The processor that is used to
    *                         calculate the reply.
    *    @param   q           Queue used to get the packets. The
    *                         dequeue method in the Queue must be
    *                         hanging!
    *    @param   pp          Optional parameter that if set is a
    *                         packetpool that is used to store the
    *                         packets (instead of "new" and "delete").
    *    @param   verbose     Optional parameter that, if set to true,
    *                         give a more verbose output.
    *    @param   tcpPacketTypes
    *                         Optional parameter containing packettypes
    *                         that should always be sent via TCP.
    */
   JobThread( Processor *processor,
              PacketQueue *q,
              QueuedPacketSender::SendQueue& sendQueue,
              bool verbose = false,
              const Vector* tcpPacketTypes = NULL);

   /**
    *    Delete this job thread and release the allocated memory.
    */
   virtual ~JobThread();

   /**
    *    The method that is executed when this method is started.
    *    This must not be called explicit (used internally by the
    *    JTC-packet).
    */
   virtual void run();

   /**
    * Send the packet to a courier to be processed. 
    * Caution: Use this only before the thread is running, if its used outside
    * this instance.
    * @param packet Will be deleted by this.
    */
   void processPacket( Packet* packet );

protected:
   /**
    *    An array with subTypes of the packets that should be send
    *    via tcp to the destination.
    */
   Vector m_tcpPacketTypes;

private:


   /**
    *  The incoming job queue.
    */
   PacketQueue* m_jobQueue;

   /**
    *    The packet cache.
    */
   auto_ptr<PacketCache> m_packetCache;

   /// Enqueues replies and adds them to the real sender.
   std::auto_ptr<PacketSender> m_packetSender;
   /// Dispatches packets.
   std::auto_ptr< JobDispatcher > m_dispatcher;
};

#endif

