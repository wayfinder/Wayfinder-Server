/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MMSMALLESTROUTINGCOSTTABLE_H
#define MMSMALLESTROUTINGCOSTTABLE_H

#include "config.h"

#include "ItemIDTree.h"
#include "SmallestRoutingCostTable.h"
#include "MapBits.h"

#include <set>


class MMRoutingCostTable : public SmallestRoutingCostTable {
public:

   /**
    *    Returns the smallest routing cost from <code>fromMapID</code>
    *    to <code>toMapID</code>. If one of the maps is an overview
    *    map and the costs are not in the table the function tries
    *    to calculate the cost using the sub-maps of the overview maps
    *    instead. It then inserts the cost in the table. That is why
    *    the function is not const.
    */
   inline const RoutingCosts* getCostsUsingTree(const ItemIDTree& tree,
                                                uint32 fromMapID,
                                                uint32 toMapID);

private:

   /**
    *    Uses the itemids to calculate the minimum cost 
    *    between (overview) maps.
    */
   const RoutingCosts* getMinCost(const set<uint32>& fromMapIDs,
                                  const set<uint32>& toMapIDs,
                                  uint32 realFromMap,
                                  uint32 realToMap);
   
};


inline const RoutingCosts*
MMRoutingCostTable::getCostsUsingTree(const ItemIDTree& tree,
                                      uint32 fromMapID,
                                      uint32 toMapID )
{
   const RoutingCosts* costs = getCosts(fromMapID, toMapID);
   if ( costs != NULL ) {
      return costs;
   }

   // Blagh. Find the costs using the tree.
   set<uint32> fromMaps;
   if ( MapBits::isOverviewMap( fromMapID ) ) {
      ItemIDTree newTree;
      tree.getContents( fromMapID, newTree );
      newTree.getLowestLevelMapIDs( fromMaps );
   } else {
      fromMaps.insert(fromMapID);
   }
   
   set<uint32> toMaps;
   if ( MapBits::isOverviewMap( toMapID ) ) {
      ItemIDTree newTree;
      tree.getContents( toMapID, newTree );
      newTree.getLowestLevelMapIDs( toMaps );
   } else {
      toMaps.insert(toMapID);
   }

   return getMinCost(fromMaps, toMaps, fromMapID, toMapID);
   
}

#endif
