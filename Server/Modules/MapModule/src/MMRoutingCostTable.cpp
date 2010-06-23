/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MMRoutingCostTable.h"

const RoutingCosts*
MMRoutingCostTable::getMinCost(const set<uint32>& fromMapIDs,
                               const set<uint32>& toMapIDs,
                               uint32 realFromMap,
                               uint32 realToMap )
{
   uint32 minCostA = MAX_UINT32;
   uint32 minCostB = MAX_UINT32;
   if ( fromMapIDs.empty() || toMapIDs.empty() ) {
      mc2log << warn << "[MMRCT]: No costs from "
             << realFromMap << " to " << realToMap << endl;
      setCosts( realFromMap, realToMap, RoutingCosts(0,0));
      return getCosts( realFromMap, realToMap );
   }
   
   for( set<uint32>::const_iterator from( fromMapIDs.begin() );
        from != fromMapIDs.end();
        ++from ) {
      for( set<uint32>::const_iterator to( toMapIDs.begin() );
           to != toMapIDs.end();
           ++to ) {
         const RoutingCosts* costs = getCosts(*from, *to);
         if ( costs == NULL ) {
            // Blargh. Could not find all needed maps.
            // Insert zero cost
            mc2dbg << "[MMRCT]: Cost is missing from "
                   << *from << " to " << *to << " - no cost from "
                   << realFromMap << " to " << realToMap << endl;
            setCosts( realFromMap, realToMap, RoutingCosts(0,0));
            return getCosts( realFromMap, realToMap );
         } else {
            minCostA = MIN( minCostA, costs->getCostA());
            minCostB = MIN( minCostB, costs->getCostB());
         }
      }
   }
   if ( (minCostA != MAX_UINT32) && (minCostB != MAX_UINT32 ) ) {
      mc2dbg << "[MMRCT]: Found all needed costs from "
             << realFromMap << " to " << realToMap 
             << endl;
      setCosts(realFromMap, realToMap, RoutingCosts(minCostA, minCostB));
      return getCosts( realFromMap, realToMap );
   } else {
      mc2dbg << "[MMRCT]: One of the costs is MAX_UINT32" << endl;
      setCosts(realFromMap, realToMap, RoutingCosts(0,0));
      return getCosts( realFromMap, realToMap );
   }
}
