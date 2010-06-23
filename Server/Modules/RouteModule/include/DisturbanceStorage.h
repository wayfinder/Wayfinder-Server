/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTINGMAPROLLBACKSTACK
#define ROUTINGMAPROLLBACKSTACK

#include "config.h"

#include <map>

class RoutingMap;
class RoutingNode;
class RoutingConnection;
class RoutingConnectionData;
class DisturbanceVector;


#include "RoutingConnection.h"
/**
 *   Contains changes that can be replayed when a multiconnection
 *   is changed and restored.
 *   The structure of all this should change.
 */
class RoutingMapConnChanges {
public:
   /**
    *   Creates a new diff.
    *   @param fromNode The from node.
    *   @param toNode   The to node.
    *   @param oldConnData Old connection data to save.
    *   @param newConnData The new connection data.
    *   @param forward     True if the connection is forward.
    */
   RoutingMapConnChanges(RoutingNode* fromNode,
                         RoutingNode* toNode,
                         const RoutingConnectionData& oldConnData,
                         const RoutingConnectionData& newConnData,
                         bool forward)
         : m_fromNode(fromNode),
           m_toNode(toNode),
           m_origData(oldConnData),
           m_newData(newConnData),
           m_forward(forward)
      {
      }

   /**
    *   Rolls back a diff.
    *   @param theMap The RoutingMap.
    */
   void rollBack(const RoutingMap* theMap);
   
   /**
    *   Applies the diff on the node.
    *   @param theMap      The RoutingMap.
    *   @param node        Node to change.
    *   @param conn        Connection to change.
    *   @param causingConn Causing connection for debug.
    */
   bool applyDiff(const RoutingMap* theMap,
                  RoutingNode* node,
                  RoutingConnection* conn,
                  fromToNode_t* causingConn = NULL) const;

   /**
    *   Applies the diff on the node.
    *   @param theMap      The RoutingMap.
    *   @param fromNode    From node of connection to change.
    *   @param toNode      Tonode of connection to change.
    *   @param causingConn Causing connection for debug.
    */
   bool applyDiff(const RoutingMap* theMap,
                  RoutingNode* fromNode,
                  RoutingNode* toNode,
                  fromToNode_t* causingConn = NULL) const;

   /**
    *   Returns the from-node id.
    */
   uint32 getFromNodeID() const {
      return m_fromNode->getItemID();
   }
   
   /**
    *   Returns the to-node id.
    */
   uint32 getToNodeID() const {
      return m_toNode->getItemID();
   }

   /**
    *   Returns the original connection data.
    */
   const RoutingConnectionData& getOldData() const {
      return m_origData;
   }
private:
   /// Node that the connection comes from
   RoutingNode* m_fromNode;
   /// Node that the connection leads to.
   RoutingNode* m_toNode;
   /// Original connection data
   RoutingConnectionData m_origData;
   /// New connection data
   RoutingConnectionData m_newData;
   /// True if the connection is in the forward direction. Not really used.
   bool m_forward;
};





//---------------------------------------------------------------------
// Stack begins here.
//---------------------------------------------------------------------

/**
 *   Class for keeping track of changes in the RoutingMap
 *   and restoring them when the changes no longer are valid.
 */
class DisturbanceStorage {
   
public:

   /**
    *   Creates a new RoutingMapRollBackstack.
    *   @param routingMap The routing map. Needed for node lookups.
    */
   DisturbanceStorage(RoutingMap* routingMap);
   
   /**
    *   Destructor.
    */
   virtual ~DisturbanceStorage() {}
   
   /**
    *   Should be called when the connections are removed
    *   from a node. The old connections are saved in firstConn.
    */
   void addConnectionChange(RoutingNode* fromNode,
                            RoutingNode* toNode,                    
                            const RoutingConnectionData& newData,
                            bool forward);

   /**
    *    Will set the connection datas of the node to very
    *    difficult ones to pass.
    */
   void avoidNode(RoutingNode* node);

   /**
    *   Called when all changed should be rolled back.
    *   @return The number of backrolled changes.
    */
   int rollBackAll();

   /**
    *   Restores the connection from fromnode to tonode to
    *   original state.
    *   @param fromNodeID Id of the node that the connection leads from.
    *   @param toNodeID   Id of the node that the connection leads to.
    *   @return The number of backrolled changes (not counting
    *           multiconnections).
    */
   int rollBackOne(uint32 fromNodeID, uint32 toNodeID);

   /**
    *   Called to check if the costs are changed from node
    *   <code>fromNodeID</code> to <code>toNodeID</code>.
    *   @return True if there is a cost-change and costs are valid.
    */
   inline bool getOldCosts(uint32 fromNodeID,
                           uint32 toNodeID,
                           uint32& costA,
                           uint32& costB,
                           uint32& costC,
                           uint32& costD);

   /**
    *    Change the connection costs for the node and connection
    *    to the new costs given.
    */
   void changeConnectionCosts( RoutingNode* node,
                               RoutingConnection* conn,
                               uint32 costA,
                               uint32 costB,
                               uint32 costC,
                               uint32 vehicleRestrictions);
   /**
    *   Adds all disturbances from the distVector distVect.
    */
   int addDisturbances( const DisturbanceVector* distVect );
   
protected:

   /**
    *   Updates the multiconnections affected by the change
    *   at rollback or an addition of changes.
    *   @param change The previously made change.
    *   @param removing True if removing, false if adding.
    */
   int updateMulti(RoutingMapConnChanges* change,
                   bool removing);

   /**
    *   Removes the fromNodeIDNotMulti from the map of nodes
    *   that affect a certain multi-connection node.
    *   @param multi The multiconnection.
    *   @param notmulti The connection that no longer affects the
    *                                   multiconnection.
    */
   bool removeNodeFromMultiConnection(const fromToNode_t& multi,
                                      const fromToNode_t& notmulti);

   /**
    *   Loops over the nodes that affect the multiconnection and applies
    *   the changes to the multiconnection.
    *   @param multi Multiconnection to look for.
    *   @return Number of changes.
    */
   int applyAllConnectionChangesToMultiConnection(const fromToNode_t& multi);
   

   /**
    *   The changed nodes are stored in a map like this.
    */
   typedef map<fromToNode_t, RoutingMapConnChanges > changedNodes_t;
   
   /**
    *   Map containing the changed nodes and costs. To be used when
    *   expanding the route and adding costs.
    *   The first in the pair is the nodeid of the from-node.
    *   The second in the pair is the nodeid of the to node.
    */
   changedNodes_t m_changedNodes;

   /**
    *   The map is needed to lookup nodes using the connections.
    */
   RoutingMap* m_map;

   /**
    *   Type of map to save multiconnection data in.
    */
   typedef multimap<fromToNode_t, fromToNode_t> nodesInMultiMap_t;

   /**
    *   Multimap containing the from-node id in a multiconnection
    *   in first and the connection that affects the multiconnection
    *   in second (the ones containing disturbances).
    */
   nodesInMultiMap_t m_nodesInMulti;
};

#include "RoutingNode.h"
#include "RoutingConnection.h"


inline bool
DisturbanceStorage::getOldCosts(uint32 fromNodeID,
                                uint32 toNodeID,
                                uint32& costA,
                                uint32& costB,
                                uint32& costC,
                                uint32& costD)
{
   if ( MC2_LIKELY(m_changedNodes.empty()) )
      return false;
   
   changedNodes_t::const_iterator it =
      m_changedNodes.find(fromToNode_t(fromNodeID, toNodeID));
   
   if ( MC2_LIKELY(it == m_changedNodes.end()) )
      return false;

   costA = it->second.getOldData().getCostA(0);
   costB = it->second.getOldData().getCostB(0);
   costC = it->second.getOldData().getCostC(0);
   costD = it->second.getOldData().getCostD();
   
   return true;
}


#endif
