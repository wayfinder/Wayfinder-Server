/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SMALLESTROUTINGCOSTPACKET_H
#define SMALLESTROUTINGCOSTPACKET_H

#define SMALLESTROUTINGCOST_REQUEST_PRIO DEFAULT_PACKET_PRIO
#define SMALLESTROUTINGCOST_REPLY_PRIO   DEFAULT_PACKET_PRIO

#define SMALLESTROUTINGCOST_REQUEST_MAX_LENGTH 65536
#define SMALLESTROUTINGCOST_REPLY_MAX_LENGTH   65536

#include "config.h"

#include "Packet.h"

#include <set>
#include <vector>
#include <map>

/**
 *    Class for holding the costs for routing from one map
 *    to another.
 */
class RoutingCosts {

public:
   RoutingCosts(uint32 costA = 0, uint32 costB = 0);

   inline uint32 getCostA() const;
   inline uint32 getCostB() const;

private:
   uint32 m_costA;
   uint32 m_costB;

};

inline uint32
RoutingCosts::getCostA() const
{
   return m_costA;
}

inline uint32
RoutingCosts::getCostB() const
{
   return m_costB;
}


/**
 *    Class for holding the pair of maps, when checking costs 
 *    for routing from one map to the other.
 */
class MapPair : public pair<uint32, uint32> {

public:

   MapPair(uint32 fromMap, uint32 toMap);
   
   /**
    *    Returns the id of the "from" map.
    */
   inline uint32 getFromMap() const;
   
   /**
    *    Returns the id of the "to" map.
    */
   inline uint32 getToMap() const;
};

inline uint32
MapPair::getFromMap() const
{
   return first;
}

inline uint32
MapPair::getToMap() const
{
   return second;
}

class MapPairVector : public vector<MapPair> {
};


/**
 *   Packet used to get the smallest cost for routing from
 *   one map to another.
 *   After the normal RequestPacket header this packet
 *   contains (X is REQUEST_HEADER_SIZE):
 *   @packetdesc
 *      @row X     @sep 4 bytes @sep Number of map pairs in the request.
 *      @endrow
 *      @row X + 4 @sep 4 bytes @sep The user defined data @endrow       
 *      @row X + 8 @sep 4 bytes @sep The map pairs, for which to get the
 *                                   smallest routing cost. @endrow
 *   @endpacketdesc
 */
class SmallestRoutingCostRequestPacket : public RequestPacket {

public:
   /**
    *    Creates a new packet using the supplied map pairs.
    */
   SmallestRoutingCostRequestPacket(const MapPairVector& maps,
                                    uint32 userDefinedData = MAX_UINT32);

   /**
    *    Returns the mapid pairs in <code>maps</code>.
    */
   void getMapPairVector(MapPairVector& maps) const;

   /**
    *    Returns the user defined data.
    */
   inline uint32 getUserDefinedData() const;
   
private:
   
   /**   The position of the level variable */   
   static const int USER_DATA_POS = REQUEST_HEADER_SIZE;

   /**   The position of the size of the vector */
   static const int SIZE_POS      = USER_DATA_POS + 4;

};


class StringCode;

/**
 *   Packet containing information about the smallest routing costs
 *   from one map to another.
 *   Packet contains (X is REPLY_HEADER_SIZE):
 *   @packetdesc
 *      @row X     @sep 4 bytes @sep   Number of map pairs with 
 *                                     routing costs. @endrow
 *      @row X + 4 @sep 4 bytes @sep   The user defined data. @endrow
 *      @row X + 8 @sep 4 bytes @sep   The map pairs with routing costs.
 *                                     @endrow
 *   @endpacketdesc
 */
class SmallestRoutingCostReplyPacket : public ReplyPacket {

public:

   typedef map<MapPair, RoutingCosts> costMap_t;
   
   /**
    *   Creates a reply to the request. Problably used when 
    *   something went wrong.
    *   @param req      The corresponding SmallestRoutingCostRequestPacket.
    *   @param status   The status of the reply.
    */
   SmallestRoutingCostReplyPacket(const SmallestRoutingCostRequestPacket* req,
                                  StringCode status);

   /**
    *   Creates a reply to the request with the supplied result and
    *   status == StringTable::OK
    *   @param req      The corresponding SmallestRoutingCostRequestPacket.
    *   @param status   The map pairs and their routing costs.
    */
   
   SmallestRoutingCostReplyPacket(const SmallestRoutingCostRequestPacket* req,
                                  const costMap_t& result);
   
   /**
    *    Get the result (= the smallest routing costs) from the packet.
    */
   void getRoutingCosts(costMap_t& result) const;

   /**
    *    Get the user defined data.
    */
   inline uint32 getUserDefinedData() const;

private:

   static const int USER_DATA_POS = REPLY_HEADER_SIZE;
   /// The position of the size of the vector
   static const int SIZE_POS = USER_DATA_POS + 4;
   
};

// --- Implementation of inlined methods

inline uint32
SmallestRoutingCostRequestPacket::getUserDefinedData() const
{
   return readLong(USER_DATA_POS);
}

inline uint32
SmallestRoutingCostReplyPacket::getUserDefinedData() const
{
   return readLong(USER_DATA_POS);
}

#endif
