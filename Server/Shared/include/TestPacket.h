/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TESTPACKET_H
#define TESTPACKET_H

#include "config.h"
#include "PushPacket.h"

#define TEST_REQUEST_PRIO DEFAULT_PACKET_PRIO
#define TEST_REPLY_PRIO   DEFAULT_PACKET_PRIO

/**
 *    TestRequestPacket.
 *
 *   After the general RequestPacket-header the format is 
 *   \begin{tabular}{rll}
 *      Position &  Size     & Description \\ \hline
 *      REQUEST_HEADER_SIZE   & 4 bytes
 *       +4    & 4 bytes   & Module nbr.
 *       +8    & 4 bytes   & Server send time
 *       +12   & 4 bytes   & Leader recieve time
 *       +16   & 4 bytes   & Leader send/enqueue time
 *       +20   & 4 bytes   & avialable reader recieve time
 *       +24   & 4 bytes   & avialable reader enqueue time
 *       +28   & 4 bytes   & avialable process dequeue time
 *   \end{tabular}
 *
 */
class TestRequestPacket : public RequestPacket {
   public:
      /**
       *    Constructor.
       */
      TestRequestPacket(uint32 mapID, uint32 sendTime,
                        uint32 ip, uint16 port);

      /**
       *    Destructor.
       */
      virtual ~TestRequestPacket();

      /// Types of time.
      enum timetype_t
      {
         // Set when the packet is sent from the server.
         TIMETYPE_SERVERSEND = 0,
         
         // Set when the leader reader recieves the packet.
         TIMETYPE_LEADERRECIEVE,
         
         // Set when the leader distributes the packet.
         TIMETYPE_LEADERSEND,
         
         // Set when the available reader recieves the packet.
         TIMETYPE_READRECIEVE,
         
         // Set when the reader enqueues the packet.
         TIMETYPE_READSEND,
         
         // Set when the processor dequeues the packet.
         TIMETYPE_PROCRECIEVE,
         
         // Set when the processor creates the replypacket
         TIMETYPE_PROCESSDONE  // =6
      };
      

      /**
       *  Sets the next time in the TestPacket.
       */
      void setTime(uint32 time);

      /**
       *
       */
      uint32 getTime(timetype_t type) const;

      void setModuleNbr(uint32 moduleNbr);

      uint32 getModuleNbr() const;
      
      
};


/**
 *    TestReplyPacket.
 *
 *   \begin{tabular}{rll}
 *      Position &  Size     & Description \\ \hline
 *       +4    & 4 bytes   & Module nbr
 *       +8    & 4 bytes   & mapID 
 *       +12   & 4 bytes   & Server send time 
 *       +16   & 4 bytes   & Leader recieve time
 *       +20   & 4 bytes   & Leader send time
 *       +24   & 4 bytes   & avialable recieve time
 *       +28   & 4 bytes   & avialable reader enqueue time
 *       +32   & 4 bytes   & avialable process done time
 *       +36   & 4 bytes   & avialable process start time
 *       +40   & 4 bytes   & avialable process done time
 *   \end{tabular}
 *
 */
class TestReplyPacket : public ReplyPacket {
   public:
      /**
       *    Constructor.
       */
      TestReplyPacket( const TestRequestPacket& p, uint32 status, uint32 replyTime);

      /**
       *    Destructor.
       */
      virtual ~TestReplyPacket();


      /**
       *  Return the time it took from sending the request to now.
       *  @param currentTime Current time in us.
       *  @return The time it took from sending the request to now.
       */
      uint32 getTotalTime(uint32 currentTime);

      /**
       *  Get the time it took for the processor to handle the dequeued packet.
       */
      uint32 getProcessTime();

      /**
       *  The time from when the available modules PacketReaderThread reads
       *  the packet to the time it enqueues it.
       */
      uint32 getReaderTime();
      
      /**
       *  The time from when the available modules PacketReaderThread reads
       *  the packet to the time the processor has handled the dequeued
       *  packet.
       *  If the packet is handled by the leader the start time is when the
       *  packet is enqueued.
       */
      uint32 getModuleTime();
      
      /**
       *  The time from when the leader modules PacketReaderThread reads
       *  the packet to the time it sends it or enqueues it.
       */
      uint32 getLeaderTime();

      /**
       *  The time the packet isn't in a module or module queue.
       *  @param currentTime Current time in us.
       */
      uint32 getNetworkTime(uint32 currentTime);

      /**
       *  Returns the number of the module that served the packet.
       */
      uint32 getModuleNbr();

      /**
       *  Returns the mapID of the request.
       */
      uint32 getMapID();
      

      /**
       *  Prints all the time fields in the packet.
       */
      void dumpPacket();
      
};

class TestPushPacket : public PushPacket {
public:
   /**
    *   Creates a testPacket for push.
    *   Contains nothing.
    *
    */
   TestPushPacket( uint32 mapID,
                   uint32 serviceID,
                   uint32 timeStamp);
   
};



#endif

