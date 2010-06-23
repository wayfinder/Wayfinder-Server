/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UPDATETRAFFICCOSTPACKET_H
#define UPDATETRAFFICCOSTPACKET_H

#include "config.h"
#include "Packet.h"

/**
 * This packet is sent to the modules and/or servers that are 
 * interested in changes in traffic. It is mainly the RouteModule:s
 * that will get information like this!
 *
 * Together with the costs in this packet also the corresponding node 
 * ID are supplied. So the costs should be treated as additional delays
 * to the normal time-cost {\it for all connections that leads out from 
 * the given node!} In terms of the costs in the RouteModule the new
 * cost for the connections will be #costC = costB + addCost#, where
 * #addCost# is the additional cost from this packet.
 * TODO: Perhaps this packet should not have MAX_PACKET_SIZE allocated but 
 *       instead reallocate upon necessity.
 *       Also rename the static constants so they look like constants.
 * 
 *
 * After the normal header this packet contains
 * \begin{tabular}{lll}
 *   Pos & Size    & \\ \hline
 *    0         & 4 bytes & allCost, if this is larger than zero, the packet 
 *                          contains all the changes in this map, otherwise 
 *                          only the new changes \\
 *    4         & 4 bytes & nbrCosts, the number of changes in the packet \\
 *    each cost & 4 bytes & the nodeID of the node to change \\
 *    each cost & 4 bytes & the cost to add (cost + costB = costC) \\
 *  \end{tabular}
 */
class UpdateTrafficCostRequestPacket : public RequestPacket
{
   public:
      /**
       *    Create a new packet with new costs for some of the 
       *    StreetSegments.
       */
      UpdateTrafficCostRequestPacket();

      /**
       *    Create a new packet with new costs for some of the 
       *    StreetSegments. Set the mapID to the given value.
       *    @param   mapID The mapID of all the items in this packet.
       *    @param   allCosts If > 0 the packet contains all existing extra
       *                      costs, not just the new changes.
       */
      UpdateTrafficCostRequestPacket(uint32 mapID,
                                     uint32 allCost = 0);

      /**
       *    Create a new packet with new costs for some of the 
       *    StreetSegments. Set the mapID, packetID and requestID
       *    to the given values.
       *    
       *    @param   packID   The id of this packet.
       *    @param   reqID    The id of the request that will have this 
       *                      packet.
       *    @param   mapID    The mapID of all the items in this packet.
       *    @param   allCosts If > 0 the packet contains all existing extra
       *                      costs, not just the new changes.
       */
      UpdateTrafficCostRequestPacket(uint16 packID,
                                     uint16 reqID,
                                     uint32 mapID,
                                     uint32 allCost = 0);

      /**
       *    Delete this packet.
       */
      virtual ~UpdateTrafficCostRequestPacket();

      /**
       * Returns True if the packet contains all known disturbance
       * costs. If the packet contains updates the return value is false.
       * @return True if the packet contain all costs on the map.
       */
      bool allCosts() const;
         
      /**
       *    Add one new cost to this packet.
       *    @param   nodeID         The ID of the node that have a new 
       *                            cost.
       *    @param   additionalCost The additional cost for node with '
       *                            ID == nodeID.
       *    @return  True if the cost is added to the packet, false 
       *             otherwise. False will for example be returned if the
       *             packet is full.
       */
      bool addCost(uint32 nodeID, uint32 additionalCost);


      /**
       *    Get the number of additional costs in this packet.
       *    @return  The number of costs in this packet.
       */
      uint32 getNbrCosts() const;
      
      /**
       *    Get one cost, incluing the ID of the item with that cost.
       *    @param   nodeID   Outparameter that is set to the ID of the
       *                      node with cost number i.
       *    @param   addCost  Outparameter that is set to the additional
       *                      cost number i. This additional cost should
       *                      be applied on all the connections that leads
       *                      out from nodeID.
       *    @param   i        The index of the cost to return. Valid 
       *                      values are 0 < i <= getNbrCosts().
       *    @return  True if the outparameters are set, false otherwise.
       */
      bool getCost(uint32 &nodeID, uint32 &addCost, uint32 i) const;




#ifndef _MSC_VER
      /**
       *    Dump the packet data to cout.
       *
       * @param   headerOnly  If true dump only writes the packet 
       *          header
       * @param   do a dns lookup of the IP. Can be very time
       *          consuming if the IP is invalid.
       */
      virtual void dump(bool headerOnly = false, 
                        bool lookupIP = false ) const;
#endif

   private:
      /**
       *    @name Positions
       *    The positions of the fields in this packet.
       */
      //@{
         /**
          *  The position of the allcost flag. This field shows if the
          *  packet contains all trafficcosts on the map or if it only
          *  contains the most recent updates.
          */
         static const uint32 m_allcostspos;
         
         /**
          *    The position of the number of costs and itemIDs in this
          *    packet.
          */
         static const uint32 m_nbrcostspos;

         /**
          *    The position of the first itemID+cost in this packet.
          */
         static const uint32 m_firstcostpos;








      //@}
      
      /**
       *    Increase the number of costs in this packet with one.
       */
      void incNbrCosts();

      /**
       *    Initiate the buffer in this packet.
       */
      void init();

};


/**
 *    This packet is a reply to the update traffic cost request packet.
 *    Does not contain anything except for the status (in ReplyPacket).
 *
 *    A RouteModule sending this packet to TM-leader will set status
 *    OK if everything went ok, NOTFOUND if one or more nodes were not
 *    found in the map and MAPNOTFOUND if the map was not found in the
 *    RouteModule at all.
 *    @see StringTable
 * 
 */
class UpdateTrafficCostReplyPacket : public ReplyPacket
{
   public:
      /**
       * Constructor.
       */
      UpdateTrafficCostReplyPacket( const UpdateTrafficCostRequestPacket* p );

      /**
       * Destructor.
       */
      virtual ~UpdateTrafficCostReplyPacket();
};


#endif


