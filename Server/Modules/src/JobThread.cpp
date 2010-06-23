/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <time.h>

#include <iomanip>

#include "DatagramSocket.h"
#include "Processor.h"
#include "PacketQueue.h"
#include "PacketPool.h"
#include "JTPacketSender.h"
#include "IPnPort.h"
#include "LogBuffer.h"

#include "Fifo.h"

#include "JobThread.h"

#include "PacketQueue.h"
#include "PacketCache.h"
#include "DebugClock.h"
#include "SimpleArray.h"
#include "TimeUtility.h"
#include "FixedSizeString.h"

#include "MultiJobDispatcher.h"
#include "SingleJobDispatcher.h"
#include "FIFOJobScheduler.h"

JobThread::JobThread( ProcessorFactory &procFactory, uint32 nbrThreads,
                      JobScheduler* scheduler,
                      PacketQueue* q,
                      QueuedPacketSender::SendQueue& sendQueue,
                      bool verbose,
                      const Vector* tcpPacketTypes )
   : m_tcpPacketTypes(8, 8),
     m_jobQueue( q ),
     m_packetCache( new PacketCache() ),
     m_packetSender( new JTPacketSender( sendQueue,
                                         q,
                                         Packet::getTCPLimitSize(),
                                         8000,// exclude port 8000 from tcp send
                                         m_tcpPacketTypes,
                                         verbose ) ),
     m_dispatcher( new MultiJobDispatcher( *m_packetCache,
                                           *m_packetSender,
                                           procFactory,
                                           nbrThreads,
                                           // use default scheduler if none was
                                           // provided
                                           ( scheduler ? scheduler : new
                                             FIFOJobScheduler( nbrThreads ) ),
                                           *q ) ) {
   if ( tcpPacketTypes != NULL ) {
      for (uint32 i=0; i < tcpPacketTypes->getSize(); ++i ) {
         m_tcpPacketTypes.addLast((*tcpPacketTypes)[i]);
      }
   }
}

JobThread::JobThread( Processor *processor,
                      PacketQueue* q,
                      QueuedPacketSender::SendQueue& sendQueue,
                      bool verbose,
                      const Vector* tcpPacketTypes )
   : m_tcpPacketTypes(8, 8), 
     m_jobQueue( q ),
     m_packetCache( new PacketCache() ),
     m_packetSender( new JTPacketSender( sendQueue,
                                         q,
                                         Packet::getTCPLimitSize(),
                                         8000,// exclude port 8000 from tcp send
                                         m_tcpPacketTypes,
                                         verbose ) ),
     m_dispatcher( new SingleJobDispatcher( *m_packetCache,
                                            *m_packetSender,
                                            *processor,
                                            *q ) )
{
   if ( tcpPacketTypes != NULL ) {
      for (uint32 i=0; i < tcpPacketTypes->getSize(); ++i ) {
         m_tcpPacketTypes.addLast((*tcpPacketTypes)[i]);
      }
   }
}

JobThread::~JobThread()
{
}

void JobThread::run()
{
   Packet* packet = NULL;
   while ( !terminated ) {
      
      packet = m_jobQueue->dequeue();

      // loop until we get a packet
      if ( packet == NULL ) {
         continue;
      }

      if ( packet->timedOut() ) {
         mc2dbg << "[JobThread]: Packet timed out, type="
                << packet->getSubTypeAsString()
                << " age "
                << float(packet->getTimeSinceArrival() / 1000.0 )
                << " max age " << packet->getTimeout() << endl;
         delete packet;
         continue;
      }

      processPacket( packet );

   }
   // make sure the dispatcher dies quickly 
   m_dispatcher.reset( NULL );
}

void JobThread::processPacket( Packet* packet ) {
   // Periodic packets shouldn't be cached and logged so handle them here
   if ( packet->getSubType() == Packet::PACKETTYPE_PERIODIC_REQUEST ) {
      m_dispatcher->dispatchDirect( packet );
      return;
   }

   mc2log << info << "[JobThread] Received "
          << "'" << packet->getSubTypeAsString() << "'"
          << " packet from " << packet->getOriginAddr()
          << endl;

   if (packet->getSubType() == Packet::PACKETTYPE_SHUTDOWN) {
      mc2dbg << "JT::run(): ShutdownPacket received, starting to"
         " terminate" << endl;
      terminated = true;
      delete packet;
      return;
   }

   m_dispatcher->dispatch( packet );

}

