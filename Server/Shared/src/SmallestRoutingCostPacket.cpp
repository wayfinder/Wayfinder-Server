/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SmallestRoutingCostPacket.h"
#include "StringTable.h"

RoutingCosts::RoutingCosts(uint32 costA, uint32 costB)
{
   m_costA = costA;
   m_costB = costB;
}

MapPair::MapPair(uint32 fromMap, uint32 toMap) :
      pair<uint32, uint32>(fromMap, toMap)
{
}

//-----------------------------------------------------------------
// SmallestRoutingCostRequestPacket
//-----------------------------------------------------------------
SmallestRoutingCostRequestPacket::
SmallestRoutingCostRequestPacket(const MapPairVector& maps,
                                 uint32 userDefinedData
                                 ) :
      RequestPacket( SMALLESTROUTINGCOST_REQUEST_MAX_LENGTH,
                     SMALLESTROUTINGCOST_REQUEST_PRIO,
                     Packet::PACKETTYPE_SMALLESTROUTINGCOSTREQUEST,
                     0,
                     0,
                     MAX_UINT32)
{
   int pos = USER_DATA_POS;
   incWriteLong(pos, userDefinedData);
   incWriteLong(pos, maps.size());
   for (uint32 i = 0; i < maps.size(); i++) {
      incWriteLong(pos, maps[i].getFromMap());
      incWriteLong(pos, maps[i].getToMap());
   }
   setLength(pos);
}

void
SmallestRoutingCostRequestPacket::getMapPairVector(MapPairVector& maps) const
{
   int pos = SIZE_POS;
   uint32 size = incReadLong(pos);
   maps.reserve(size);
   for (uint32 i = 0; i < size; i++) {
      uint32 fromMap = incReadLong(pos);
      uint32 toMap = incReadLong(pos);
      MapPair mappair = MapPair(fromMap, toMap);
      maps.push_back(mappair);
   }
}


//-----------------------------------------------------------------
// SmallestRoutingCostReplyPacket
//-----------------------------------------------------------------

SmallestRoutingCostReplyPacket::
SmallestRoutingCostReplyPacket(const SmallestRoutingCostRequestPacket* req,
                               StringCode status) :
      ReplyPacket( SMALLESTROUTINGCOST_REPLY_MAX_LENGTH,
                   Packet::PACKETTYPE_SMALLESTROUTINGCOSTREPLY,
                   req,
                   status)
{
   int pos = USER_DATA_POS;
   incWriteLong(pos, req->getUserDefinedData());
   incWriteLong(pos, 0); // Size is noll. Just in case.
}

SmallestRoutingCostReplyPacket::
SmallestRoutingCostReplyPacket(const SmallestRoutingCostRequestPacket* req,
                               const costMap_t& result) :
      ReplyPacket( SMALLESTROUTINGCOST_REPLY_MAX_LENGTH,
                   Packet::PACKETTYPE_SMALLESTROUTINGCOSTREPLY,
                   req,
                   StringTable::OK)
{
   int pos = USER_DATA_POS;
   incWriteLong(pos, req->getUserDefinedData());
   incWriteLong(pos, result.size());
   for (costMap_t::const_iterator it = result.begin(); 
        it != result.end(); 
        it++) {
      incWriteLong(pos, (*it).first.getFromMap());
      incWriteLong(pos, (*it).first.getToMap());
      incWriteLong(pos, (*it).second.getCostA());
      incWriteLong(pos, (*it).second.getCostB());
   }
   setLength(pos);
}

void
SmallestRoutingCostReplyPacket:: getRoutingCosts(costMap_t& result) const
{
   int32 pos = SIZE_POS;
   uint32 size = incReadLong(pos);
   for (uint32 i = 0; i < size; i++) {
      uint32 fromMap = incReadLong(pos);
      uint32 toMap = incReadLong(pos);
      uint32 costA = incReadLong(pos);
      uint32 costB = incReadLong(pos);
      MapPair mappair = MapPair(fromMap, toMap);
      RoutingCosts costs = RoutingCosts(costA, costB);
      result.insert( pair<MapPair, RoutingCosts>(mappair, costs) );
   }
}

