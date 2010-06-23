/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TRAFFIC_MAP_INFO_H
#define TRAFFIC_MAP_INFO_H

#include "config.h"

#include <set>
#include <map>
#include <vector>
#include <utility>

class DisturbanceElement;
class UserRightsMapInfo;
class InfoSQL;
class MC2BoundingBox;


/*
 * Takes cares of disturbances within a map area. Works like a cache for the
 * database.
 */
class TrafficMapInfo {
public:

   typedef uint32 DisturbanceID;
   typedef uint32 NodeID;
   typedef uint32 CostFactor;
   typedef uint32 MapID;
   typedef std::vector<DisturbanceElement*> Disturbances;
   typedef std::set< NodeID > NodeIDs;
   typedef std::pair< NodeID, CostFactor > NodeIDCostFactor;
   typedef std::vector< NodeIDCostFactor > NodesWithDelays;
   typedef std::map< NodeID, const DisturbanceElement* > IDToDisturbanceMap;
   typedef std::vector< pair< NodeID, const DisturbanceElement* > > NodeIDsToDisturbances;

   /**
    * @param id Map ID.
    */
   explicit TrafficMapInfo( MapID id );
   ~TrafficMapInfo();

   /**
    * Load disturbances from SQL database.
    * @param database SQL database.
    * @return True on success. False if database failed or if there were no
    *         disturbances for this map.
    */
   bool load( InfoSQL& database );

   /**
    * Add, updated, and remove disturbances in this unit.
    * @param updated Disturbances to be added to this unit.
    * @param removed Disturbances to be removed from this unit.
    */
   void updateDisturbances( Disturbances& updated, const Disturbances& removed );

   /// @return active disturbances
   const Disturbances& getDisturbances() const;

   /**
    * Find out if a set of nodes have disturbances and map them to disturbance
    * elements.
    * @param ids The nodes to test.
    * @param userRights User rights on map.
    * @return mapped node IDs to disturbances elements.
    */ 
   NodeIDsToDisturbances
   findNodeID( const NodeIDs& ids,
               const UserRightsMapInfo& userRights ) const;

   int getNodesWithDelays( const UserRightsMapInfo& userRights,
                           NodesWithDelays& delays ) const;
   // remove when we are done
   const IDToDisturbanceMap& getDisturbanceList() const;

   /// @return map size in bytes
   uint32 mapSize() const;

   /// @return bounding box of the current map.
   const MC2BoundingBox& getMapBox() const;
   /// @return map id
   MapID getMapID() const;

private:
   struct Impl;
   Impl *m_impl;
};

#endif // TRAFFIC_MAP_INFO_H
