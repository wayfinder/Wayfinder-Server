/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PacketQueue.h"
#include "Packet.h"

PacketQueue::PacketQueue():
   m_queues( Packet::m_nbrPriorities ),
   m_terminated( false )
{

}

PacketQueue::~PacketQueue()
{
   // Delete the queues and the array for the queues
   
   for (uint32 i=0; i< m_queues.size(); i++) {
      while ( ! m_queues[ i ].empty() ) {
         delete m_queues[ i ].front();
         m_queues[ i ].pop();
      }
   }
}

bool
PacketQueue::enqueue(Packet* p)
{
   if ( p == NULL ) {
      return false;
   }

   ISABSync synchronized(m_monitor);

   // Get the priority of p and insert into correct queue
   uint32 curPrio = p->getPriority();

   mc2dbg4 << "[PacketQueue] Enqueue curPrio: "
           << curPrio << ", subType: "
           << (int)p->getSubType() << endl;

   MC2_ASSERT( curPrio < m_queues.size() );

   m_queues[ curPrio ].push( p );

   // Wake up all others and return
   m_monitor.notifyAll();

   return true;
}

Packet*
PacketQueue::dequeue()
{
   ISABSync synchronized(m_monitor);

   uint32 queueIndex = 0;
   while ( ((queueIndex = getFirstNonEmptyIndex() ) 
            >= m_queues.size()) && !m_terminated ) {
      try {
         m_monitor.wait();
      } catch ( JTCInterruptedException e) {
         mc2log << warn << "[PacketQueue] dequeue() interrupted!" <<
            endl;
      }
   }

   Packet* p = NULL;
   if ( !m_terminated ) {
      p = m_queues[ queueIndex ].front();
      m_queues[ queueIndex ].pop();
      mc2dbg4 << "[PacketQueue] Dequeue curPrio: "
              << queueIndex << ", subType: " 
              << (int)p->getSubType() << ", packetID: " << p->getPacketID() 
              << ", reqID: " << p->getRequestID() << endl;
   } else {
      mc2dbg4 << "[PacketQueue] Dequeue NULL" << endl;;
      p = NULL;
   }
   return p;
}


Packet*
PacketQueue::dequeue(uint32 maxWaitTime) {
   ISABSync synchronized(m_monitor);
   
   uint32 queueIndex = getFirstNonEmptyIndex();
   if ( queueIndex >= m_queues.size() ) {
      try {
         m_monitor.wait( maxWaitTime );
      }
      catch(const JTCInterruptedException &) {
         mc2log << warn << "[PacketQueue] dequeue(uint32 maxWaitTime) "
                           "interrupted!" << endl;
      }

      queueIndex = getFirstNonEmptyIndex();   
   }

   if (!m_terminated && queueIndex  < m_queues.size() ) {
      Packet* p = m_queues[ queueIndex ].front();
      m_queues[ queueIndex ].pop();
      return p;
   } 

   return NULL;
}

uint32
PacketQueue::getStatistics()
{
   uint32 retVal = 0;
   for (uint32 i=0; i < m_queues.size(); i++) {
      retVal += m_queues[ i ].size();
   }

   return retVal;
}

uint32
PacketQueue::getFirstNonEmptyIndex()
{
   uint32 retIndex = 0;
   while ( retIndex < m_queues.size() &&
           m_queues[ retIndex ].empty() ) {
      retIndex++;
   }

   return retIndex;
}

void
PacketQueue::terminate()
{
   ISABSync synchronized(m_monitor);
   m_terminated = true;
   m_monitor.notifyAll();
}
