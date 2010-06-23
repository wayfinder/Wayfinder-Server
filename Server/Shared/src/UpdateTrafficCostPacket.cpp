/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "UpdateTrafficCostPacket.h"
#include "StringTable.h"

#define UPDATETRAFFICCOSTREQUEST_PRIO 0


const uint32 
UpdateTrafficCostRequestPacket::m_allcostspos = REQUEST_HEADER_SIZE;
         
const uint32 
UpdateTrafficCostRequestPacket::m_nbrcostspos = REQUEST_HEADER_SIZE + 4;

const uint32 
UpdateTrafficCostRequestPacket::m_firstcostpos = REQUEST_HEADER_SIZE + 8;



UpdateTrafficCostRequestPacket::UpdateTrafficCostRequestPacket()
   : RequestPacket(MAX_PACKET_SIZE, 
                   UPDATETRAFFICCOSTREQUEST_PRIO,
                   Packet::PACKETTYPE_UPDATETRAFFICCOSTREQUEST,
                   0,   // packet ID
                   0,   // request ID
                   0)   // mapID
{
   init();
}

UpdateTrafficCostRequestPacket::UpdateTrafficCostRequestPacket(
   uint32 mapID,
   uint32 allCost)
   : RequestPacket(MAX_PACKET_SIZE, 
                   UPDATETRAFFICCOSTREQUEST_PRIO,
                   Packet::PACKETTYPE_UPDATETRAFFICCOSTREQUEST,
                   0,   // packet ID
                   0,   // request ID
                   mapID)
{
   init();
   writeLong(m_allcostspos, allCost);
}

UpdateTrafficCostRequestPacket::UpdateTrafficCostRequestPacket(
                                                uint16 packID,
                                                uint16 reqID,
                                                uint32 mapID,
                                                uint32 allCost)
   : RequestPacket(MAX_PACKET_SIZE, 
                   UPDATETRAFFICCOSTREQUEST_PRIO,
                   Packet::PACKETTYPE_UPDATETRAFFICCOSTREQUEST,
                   packID,   // packet ID
                   reqID,   // request ID
                   mapID)
{
   init();
   writeLong(m_allcostspos, allCost);
}


UpdateTrafficCostRequestPacket::~UpdateTrafficCostRequestPacket()
{
   // Nothing to do, really
}

bool
UpdateTrafficCostRequestPacket::allCosts() const
{
   bool allCosts = (bool) readLong(m_allcostspos);
   return allCosts;
}



bool
UpdateTrafficCostRequestPacket::addCost(uint32 itemID, uint32 cost)


{
   if (getLength() + 8 < MAX_PACKET_SIZE ) {
      uint32 nbrCosts = getNbrCosts();
      writeLong(m_firstcostpos + 8*nbrCosts, itemID);
      writeLong(m_firstcostpos + 8*nbrCosts + 4, cost);
      incNbrCosts();
      setLength(m_firstcostpos + 8*nbrCosts + 8 );
      return (true);







   }
     
   return (false);
}

uint32
UpdateTrafficCostRequestPacket::getNbrCosts() const
{
   return (readLong(m_nbrcostspos));
}

bool
UpdateTrafficCostRequestPacket::getCost(uint32 &itemID, 
                                        uint32 &cost, 
                                        uint32 i) const
{
   if (i<getNbrCosts()) {
      itemID = readLong(m_firstcostpos+8*i);
      cost = readLong(m_firstcostpos+8*i + 4);


      return (true);


   }
     
   return (false);
}

#ifndef _MSC_VER
void
UpdateTrafficCostRequestPacket::dump(bool headerOnly, bool lookupIP) const
{
   cout << "Dumping UpdateTrafficCostRequestPacket :" << endl;
   Packet::dump();
   cout << "Dumping data specific to packet type: " << endl;
   
   cout << "mapID    " << getMapID() << endl;
   cout << "allCosts " << allCosts() << endl;
   uint32 nbrCosts = getNbrCosts();
   cout << "nbrCosts " << nbrCosts << endl;

   for (uint32 i = 0;i < nbrCosts; i++) {
      uint32 nodeID    = MAX_UINT32;
      uint32 addedCost = MAX_UINT32;  // different name to avoid conflict
      if (getCost(nodeID, addedCost, i)) {
         cout << "node " << i << ":" << endl;
         cout << "nodeID  " << nodeID << endl;
         cout << "addCost " << addedCost << endl;


      }
      else {
         mc2log << warn << "Node unexistent" << endl;
      }
   }
}
#endif

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
//                                                     Private functions =

void 
UpdateTrafficCostRequestPacket::incNbrCosts()
{
   writeLong(m_nbrcostspos, getNbrCosts()+1);
}

void 
UpdateTrafficCostRequestPacket::init()
{
   writeLong(m_allcostspos, 0);
   writeLong(m_nbrcostspos, 0);
   setLength(m_firstcostpos);
}

// =======================================================================
//                                          UpdateTrafficCostReplyPacket =

UpdateTrafficCostReplyPacket::UpdateTrafficCostReplyPacket(
const UpdateTrafficCostRequestPacket* p)
      : ReplyPacket( REPLY_HEADER_SIZE,
                     Packet::PACKETTYPE_UPDATETRAFFICCOSTREPLY,
                     p,
                     StringTable::OK)
{
}

UpdateTrafficCostReplyPacket::~UpdateTrafficCostReplyPacket()
{

}






