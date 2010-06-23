/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PacketPool.h"


PacketPool::PacketPool() 
{
   firstPacket = NULL;
}


PacketPool::PacketPool(uint32 nbrPackets) 
{
   Packet* p;
   firstPacket = NULL;
   this->packetSize = MAX_UINT16;
   for (uint32 i=0; i<nbrPackets; i++) {
      p = new Packet(packetSize);
      p->next = firstPacket;
      firstPacket = p;
   }
   mc2dbg2 << "PacketPool created, "<< nbrPackets << " with size = "
        << packetSize << ", allocated." << endl;
}


PacketPool::PacketPool(uint32 nbrPackets, uint16 packetSize) 
{
   Packet* p;
   firstPacket = NULL;
   this->packetSize = packetSize;
   for (uint32 i=0; i<nbrPackets; i++) {
      p = new Packet(packetSize);
      p->next = firstPacket;
      firstPacket = p;
   }
   mc2dbg2 << "PacketPool created, "<< nbrPackets << " with size = "
        << packetSize << ", allocated." << endl;
}

Packet*
PacketPool::getPacket()
{
   JTCSynchronized synchronized(*this);
   if (firstPacket != NULL) {
      Packet* p = firstPacket;
      firstPacket = p->next;
      return (p);
   } else {
      return (new Packet(packetSize));
   }
}

void
PacketPool::returnPacket(Packet* p)
{
   JTCSynchronized synchronized(*this);
   p->next = firstPacket;
   firstPacket = p;
}

uint32
PacketPool::getCurNbrPackets()
{
   uint32 cnt=0;
   Packet* p = firstPacket;
   while (p != NULL) {
      cnt++;
      p = p->next;
   }
   return (cnt);
}

