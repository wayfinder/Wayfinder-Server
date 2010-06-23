/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MultiplePacketsRequest.h"
#include "Packet.h"

MultiplePacketsRequest::MultiplePacketsRequest(uint16 reqID,
                                               int nbrPackets)
      : Request(reqID)
{
   m_requestPackets = new PacketContainer*[nbrPackets];
   m_replyPackets   = new PacketContainer*[nbrPackets];
   
   m_nbrAllocPackets        = nbrPackets;
   m_curAddRequestPacketPos = 0;
   m_curGetRequestPacketPos = 0;
   m_nbrProcessedPackets    = 0;

   // Reset the array of replyPackets
   for (int i = 0; i < nbrPackets; i++) {
      m_replyPackets[i] = NULL;
   }
}


MultiplePacketsRequest::~MultiplePacketsRequest()
{
   for (int i = 0; i < m_curAddRequestPacketPos; i++) {
      delete m_requestPackets[i];
   }
   for (int j = 0; j < m_nbrAllocPackets; j++) {
      delete m_replyPackets[j];
   }
   
   delete[] m_requestPackets;
   delete[] m_replyPackets;
}


bool
MultiplePacketsRequest::add(PacketContainer* container)
{
   bool status = true;
   
   if (m_curAddRequestPacketPos < m_nbrAllocPackets) {
      // Space left in the array, add packet...
      m_requestPackets[m_curAddRequestPacketPos] = container;
      // ...and set packetID to the pos in the array
      container->getPacket()->setPacketID(m_curAddRequestPacketPos++);
      // Also set requestID
      container->getPacket()->setRequestID( getID() );
   }
   else {
      MC2WARNING("Trying to add more packets than allocation permits");
      status = false;
   }

   return status;
}


PacketContainer*
MultiplePacketsRequest::getNextPacket()
{
   PacketContainer* returnContainer = NULL;

   // Can only get packet if one exists
   if (m_curGetRequestPacketPos < m_curAddRequestPacketPos) {
      returnContainer = m_requestPackets[m_curGetRequestPacketPos];
      // Thus only packets not sent further will be deleted
      m_requestPackets[m_curGetRequestPacketPos++] = NULL;
   }

   return returnContainer;
}


void
MultiplePacketsRequest::processPacket(PacketContainer* container)
{
   if (container != NULL) {
      Packet* packet = container->getPacket();
      if (packet != NULL) {
         int pos = packet->getPacketID();
         if ((pos < m_nbrAllocPackets) && (m_replyPackets[pos] == NULL)) {
            m_replyPackets[pos] = container;
            m_nbrProcessedPackets++;
         }
         else {
            MC2WARNING2("Could not use packet position",
                        cerr << pos << endl;);
         }
      }
      else {
         MC2WARNING("PacketContainer contains no Packet");
      }
   }
   else {
      MC2WARNING("PacketContainer is NULL, cannot be added to array");
   }
   
   if (m_nbrProcessedPackets == m_curAddRequestPacketPos) {
      // All packets have been processed and received
      setDone(true);
   }
} // processPacket


PacketContainer*
MultiplePacketsRequest::getAnswer()
{
   return NULL;
}


PacketContainer*
MultiplePacketsRequest::getAnswer(int i)
{
   PacketContainer* returnContainer = NULL;

   if (i < m_nbrAllocPackets) {
      returnContainer = m_replyPackets[i];
      // Answers that are not fetched will be deleted
      m_replyPackets[i] = NULL;
   }

   return returnContainer;
}
