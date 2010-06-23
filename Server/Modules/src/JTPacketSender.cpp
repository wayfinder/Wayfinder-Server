/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "JTPacketSender.h"
#include "IPnPort.h"
#include "Vector.h"
#include "Packet.h"
#include "PacketQueue.h"

JTPacketSender::
JTPacketSender( QueuedPacketSender::SendQueue& sendQueue,
                PacketQueue* jobQueue,
                uint32 tcpLimit,
                uint32 excludePort,
                const Vector& tcpTypes,
                bool verbose ):
   ModulePacketSender( sendQueue, tcpLimit ),
   m_jobQueue( jobQueue ),
   m_excludePort( excludePort ),
   m_tcpTypes( tcpTypes ),
   m_sum( 0 ),
   m_cnt( 0 ),
   m_verbose( verbose )
{

}

bool
JTPacketSender::sendPacket( Packet* packet,
                            const IPnPort& destination ) {

   const int printEveryNpacket = 1;

   if ( packet == NULL ) {
      mc2dbg << warn << "[JobThread] Reply packet is NULL!" << endl;
      return false;
   }

   if ( destination == IPnPort( 0, 0 ) ) {
      mc2log << warn << "[JobThread] Reply packet has invalid IP/port"    
             << "(0:0) " << endl;
      delete packet;
      return false;
   }
      
   
   if (m_verbose && packet->getLength() < 0) {
      packet->dump(true);
      m_sum += TimeUtility::getCurrentMicroTime() - packet->getDebInfo();
      if (m_cnt % printEveryNpacket == 0) {

         mc2log << info << "[JobThread] Time to process packet: " 
                << m_sum/printEveryNpacket << endl;

         if (dynamic_cast<PacketQueue*>(m_jobQueue) != NULL)
            mc2log << info  << "[JobThread] "
                   << ((PacketQueue*)m_jobQueue)->getStatistics() 
                   << endl;
         m_sum = 0;
      }
      m_cnt++;
   }
   // exclude port from tcp send
   if ( m_excludePort == destination.getPort() ) {
      sendUDP( packet, destination );
      return true;
   }

   // some packets wants to be sent via tcp
   if ( m_tcpTypes.
        binarySearch( packet->getSubType() ) < MAX_UINT32 ) {
      sendTCP( packet, destination );
      return true;
   }

   return ModulePacketSender::sendPacket( packet, destination );
}
