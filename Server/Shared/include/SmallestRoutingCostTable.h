/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#ifndef SMALLESTROUTINGCOSTTABLE_H
#define SMALLESTROUTINGCOSTTABLE_H

#include "SmallestRoutingCostPacket.h"

class SmallestRoutingCostTable {
public:
   /**
    *   Returns a pointer to the routing costs from map
    *   <code>fromMapID</code> to the map <code>toMapID</code>.
    *   NULL if no cost could be found.
    */
   inline const RoutingCosts* getCosts(uint32 fromMapID, uint32 toMapID) const;

   /**
    *   Sets the costs from <code>fromMapID</code> to map <code>
    *   toMapID</code> to <code>costs</code>.
    */
   void setCosts(uint32 fromMapID, uint32 toMapID, const RoutingCosts& costs);

   /**
    *   Loads the table from a file.
    */
   bool loadFromFile(const char* filename);
   
private:

   /**
    *   Loads the table from a textfile.
    */
   bool loadFromTextFile(const char* filename);
   
   /** Typedef of the map to store the costs in */
   typedef map<pair<uint32,uint32>, RoutingCosts> costMap; 

   /** The storage */
   costMap m_costMap;
   
};

inline const RoutingCosts*
SmallestRoutingCostTable::getCosts(uint32 fromMapID, uint32 toMapID) const
{
   costMap::const_iterator it = m_costMap.find(make_pair(fromMapID, toMapID));
   if ( it == m_costMap.end() ) {
      return NULL;
   } else {
      return &(it->second);
   }
}

inline void
SmallestRoutingCostTable::setCosts(uint32 fromMapID,
                                   uint32 toMapID,
                                   const RoutingCosts& costs)
{
   m_costMap[make_pair(fromMapID, toMapID)] = costs;
}

#endif
