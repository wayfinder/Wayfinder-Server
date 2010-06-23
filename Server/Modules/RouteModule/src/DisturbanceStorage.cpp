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
#include "Types.h"
#include "RoutingNode.h"
#include "RoutingConnection.h"
#include "RoutingMap.h"

#include "DisturbanceStorage.h"
#include "DisturbanceList.h"
#include "MapBits.h"

void
RoutingMapConnChanges::rollBack(const RoutingMap* theMap)
{
   RoutingConnection* conn = m_fromNode->getConnection(m_toNode, m_forward);
   conn->setData(m_origData);
}

bool
RoutingMapConnChanges::applyDiff(const RoutingMap* theMap,
                                 RoutingNode* node,
                                 RoutingConnection* conn,
                                 fromToNode_t* causingConn) const
{
   mc2dbg8 << "[RMCC]: applyDiff for node "
           << IDPair_t(theMap->getMapID(),
                       node->getItemID());
   if ( causingConn != NULL ) {
      mc2dbg8 << " caused by " << *causingConn;
   }
   mc2dbg8 << endl;
      
   static const uint32 maxCost = MAX_UINT32 / 256;
   // Get the data so that it is possible to keep track of what is what
   RoutingConnectionData& nodeData = *conn->getData();
   if ( nodeData.getCostC(0) == MAX_UINT32 ) {
      // Nothing more to do. Already very difficult to pass.
      return false;
   }

   // Check for vehicle restrictions
   nodeData.setVehicleRestriction( nodeData.getVehicleRestriction(false) &
                                   m_newData.getVehicleRestriction(false) );
   
   // Check for too large costs
   // Use all vehicles so that the time for avoidRoadTolls will be addded.
   if ( nodeData.getCostC(MAX_UINT32) >= maxCost ||
        m_newData.getCostC(MAX_UINT32) >= maxCost ) {
      nodeData.setCostC(MAX_UINT32);
      return true;
   }

   // Else apply the diff
   nodeData.setCostC( m_origData.getCostB(0) +
                      m_newData.getCostC(0) - m_origData.getCostC(0) );

   return true;
}

bool
RoutingMapConnChanges::applyDiff(const RoutingMap* theMap,
                                 RoutingNode* fromNode,
                                 RoutingNode* toNode,
                                 fromToNode_t* causingNode) const
{
   return applyDiff(theMap,
                    fromNode,
                    fromNode->getConnection(toNode, true),
                    causingNode);
}


//---------------------------------------------------------------------
// Stack begins here.
//---------------------------------------------------------------------

DisturbanceStorage::DisturbanceStorage(RoutingMap* routingMap)
{
   m_map = routingMap;
}

void
DisturbanceStorage::avoidNode(RoutingNode* node)
{
   mc2dbg8 << "[DS]: avoidNode" << endl;
   // Set all connection from the node to allow no vehicles
   for( RoutingConnection* conn = node->getFirstConnection(true);
        conn != NULL;
        conn = conn->getNext() ) {
      RoutingConnectionData newData(*conn->getData());
      newData.setVehicleRestriction(0);
      addConnectionChange(node, conn->getNode(), newData, true);
   }
}

void
DisturbanceStorage::
addConnectionChange(RoutingNode* fromNode,
                    RoutingNode* toNode,                    
                    const RoutingConnectionData& newData,
                    bool forward)
{
   mc2dbg8 << "[DS]: addConnectionChange" << endl;
   rollBackOne(fromNode->getItemID(), toNode->getItemID());
   RoutingConnection* conn = fromNode->getConnection(toNode, forward);
   // Copy connection data.
   RoutingConnectionData oldData(*conn->getData());

   // Create a diff
   RoutingMapConnChanges diff(fromNode, toNode, oldData, newData, forward);
   // Apply the diff on the node.
   conn->setData(newData);
   
   // Mmmm. STL at its best.
   m_changedNodes.insert(
      make_pair(
         fromToNode_t(fromNode->getItemID(),
                      toNode->getItemID()),
         diff
         ));
   
   // Then update the multinodes
   updateMulti(&diff, false);
}

bool
DisturbanceStorage::removeNodeFromMultiConnection(const fromToNode_t& multi,
                                                  const fromToNode_t& notmulti)
{
   pair<nodesInMultiMap_t::iterator, nodesInMultiMap_t::iterator>
      range = m_nodesInMulti.equal_range(multi);

   // Remove the node from the range.
   for( nodesInMultiMap_t::iterator it = range.first;
        it != range.second;
        ++it ) {
      if ( it->second == notmulti ) {
         // There it is! Remove it.
         m_nodesInMulti.erase(it);
         return true;
      }
   }
   return false;
}

int
DisturbanceStorage::
applyAllConnectionChangesToMultiConnection(const fromToNode_t& multi)
{
   int nbrChanges = 0;
   
   pair<nodesInMultiMap_t::iterator, nodesInMultiMap_t::iterator>
      range = m_nodesInMulti.equal_range(multi);

   // Copy the old connectiondata.
   RoutingNode* fromNode   = m_map->getNodeFromTrueNodeNumber(multi.first);
   RoutingNode* toNode     = m_map->getNodeFromTrueNodeNumber(multi.second);
   RoutingConnection* conn = fromNode->getConnection(toNode, true);
   
   RoutingConnectionData oldData( *conn->getData() );
   
   for( nodesInMultiMap_t::iterator it = range.first;
        it != range.second;
        ++it ) {
      // Find change
      changedNodes_t::const_iterator change = m_changedNodes.find(it->second);
      
      // Apply change
      change->second.applyDiff(m_map, fromNode, conn, &(it->second));
      
      ++nbrChanges;
   }

   if ( nbrChanges != 0 ) {
      // Save the old connection data
      RoutingMapConnChanges diff(fromNode, toNode, oldData,
                                 *conn->getData(), true);
      m_changedNodes.insert(
         make_pair(
            fromToNode_t(fromNode->getItemID(),
                         toNode->getItemID()),
            diff
            ));
   }

   mc2dbg << "[DS]: " << nbrChanges << " applied" << endl;
   
   return nbrChanges;
}

int
DisturbanceStorage::updateMulti(RoutingMapConnChanges* change,
                                bool removing)
{
   // Then update the multinodes
   set<fromToNode_t> expNodes;
   if ( !m_map->lookupExpandedNodes(expNodes, change->getFromNodeID())) {
      // Done - not part of multi
      mc2dbg2 << "[DS]: No multis to update" << endl;
      return 0;
   } else {
      mc2dbg2 << "[DS]: "
             << expNodes.size()
             << " multis to update" << endl;
   }

   // Reset the multiconnections back to normales.
   for( set<fromToNode_t>::iterator it = expNodes.begin();
        it != expNodes.end();
        ++it ) {
      // I really think we should have the multiconnected node here too
      fromToNode_t notmulti(change->getFromNodeID(), change->getToNodeID());
      changedNodes_t::iterator found = m_changedNodes.find(notmulti);

      if ( found != m_changedNodes.end() ) {
         found->second.rollBack(m_map);
      }
      // Remove the connection from the multimap containing the
      // multiconnections (fromNode)
      if ( removing ) {
         const bool removed = removeNodeFromMultiConnection(*it, notmulti);
         if ( ! removed ) {
            mc2dbg << "[DS]: Could not remove " << notmulti << endl;
         }
      } else {
         m_nodesInMulti.insert(make_pair(*it, notmulti) );
      }

      // Finally apply all the old connection-changes to the
      // multiconnection
      applyAllConnectionChangesToMultiConnection(*it);
      
   }
   return 1;
}

int
DisturbanceStorage::rollBackOne(uint32 fromNodeID,
                                uint32 toNodeID)
{
   changedNodes_t::iterator it = m_changedNodes.find(fromToNode_t(fromNodeID,
                                                                  toNodeID));
   if ( it != m_changedNodes.end() ) {
      it->second.rollBack( m_map );
      // it is erased here... keep a local copy of it's values and use that
      changedNodes_t::value_type itsData = *it;
      m_changedNodes.erase( it );

      // We must rollback the multiconnections and then apply all the
      // other disturbances again.
      updateMulti( /*&it->second*/ &itsData.second, true );
      return 1;
   }
   return 0;
}

int
DisturbanceStorage::rollBackAll()
{
   int nbrRolls = 0;
   for( changedNodes_t::iterator it = m_changedNodes.begin();
        it != m_changedNodes.end();
        ++it ) {
      ++nbrRolls;
      it->second.rollBack(m_map);
   }
   m_changedNodes.clear();
   m_nodesInMulti.clear();
   return nbrRolls;
}

void
DisturbanceStorage::changeConnectionCosts( RoutingNode* node,
                                           RoutingConnection* conn,
                                           uint32 costA,
                                           uint32 costB,
                                           uint32 costC,
                                           uint32 vehicleRestrictions)
{
   // Change the costs for the connectiondata. Will change the
   // other direction too, since the data is shared in the forward
   // and backward directions.
   RoutingConnectionData connDataCopy(*conn->getData());
   
   mc2dbg8 << "Old costs from " << hex << node->getItemID() << dec
           << " are "
           << connDataCopy.getCostA(0) << ":"
           << connDataCopy.getCostB(0) << ":"
           << connDataCopy.getCostC(0) << endl;
   
   connDataCopy.setCostA(costA);
   connDataCopy.setCostB(costB);
   connDataCopy.setCostC(costC);
   connDataCopy.setVehicleRestriction(vehicleRestrictions);

   mc2dbg8 << "New costs are "
           << connDataCopy.getCostA(0) << ":"
           << connDataCopy.getCostB(0) << ":"
           << connDataCopy.getCostC(0) << endl;
   
   
   // Add the copy of the connection to the storage.
   addConnectionChange(node,
                       conn->getNode(),
                       connDataCopy,
                       true);
}


int
DisturbanceStorage::addDisturbances( const DisturbanceVector* distVect )
{
   int nbrDistAdded = 0;
   int nbrDist = distVect->size();
   const bool isOverviewMap = MapBits::isOverviewMap( m_map->getMapID() );
   for(int i=0; i < nbrDist; ++i ) {
      DisturbanceListElement* el = (*distVect)[i];
      uint32 mapID = el->getMapID();
      uint32 nodeID = el->getNodeID();

      mc2dbg4 << "[DStor]: Element has ID " << IDPair_t(mapID, nodeID)
              << endl;
      
      if ( isOverviewMap &&
           !MapBits::isOverviewMap( mapID ) ) {
         // We will have to translate the id

         uint32 higherID = m_map->translateToHigher( mapID,
                                                     nodeID );

         if ( higherID != MAX_UINT32 ) {
            mapID  = m_map->getMapID();
            nodeID = higherID;
         } else {
            // Not found - go another round
            continue;
         }
      }

      mc2dbg4 << "[DStor]: Dist-node has id = " << IDPair_t(mapID, nodeID)
              << endl;
      
      if ( mapID == m_map->getMapID() ) {
         RoutingNode* node = m_map->getNodeFromTrueNodeNumber(nodeID);
         if ( node == NULL ) {
            mc2dbg << "[DStor]: Coulnd't find node "
                   << hex << nodeID << dec << endl;
            continue;
         }
         if ( el->getUseFactor() || el->getRawFactor() == MAX_UINT32 ) {
            mc2dbg8 << "[DStor]: Using factor when adding disturbance on node "
                   << MC2HEX( nodeID )
                   << endl;
            mc2dbg8 << "RAW factor is " << el->getRawFactor() << endl;
            
            if ( el->getRawFactor() == MAX_UINT32 ) {
               mc2dbg8 << "[DStor]: Node " << MC2HEX( nodeID )
                       << " is blocked " << endl;
               avoidNode( node );
               ++nbrDistAdded;
            } else {
               const float factor = el->getFactor();
               mc2dbg8 << "[DStor]: Factor is " << factor << endl;
               bool added = false;
               for ( RoutingConnection* curConn = node->getFirstConnection();
                     curConn != NULL;
                     curConn = curConn->getNext() ) {
                  added = true;
                  const RoutingConnectionData* connData = curConn->getData();
                  uint32 newCostC = uint32(connData->getCostB(0) * factor );
                  if(newCostC == connData->getCostB(0)){
                     mc2dbg8 << "newCostC same adding 1 **" << endl;
                     newCostC++;
                  }
                  
                  changeConnectionCosts(
                     node,
                     curConn,
                     connData->getCostA(0),
                     connData->getCostB(0),
                     newCostC,
                     connData->getVehicleRestriction(false));
               }
               nbrDistAdded += added;
            }
         } else {
            mc2dbg4 << "Using time instead of factor" << endl;

            ++nbrDistAdded;
            uint32 cost = uint32(SEC_TO_TIME_COST(el->getTime()));
            
            RoutingConnection* curConn = node->getFirstConnection();
            while ( curConn != NULL ) {
               RoutingConnectionData* connData =
                  curConn->getData();

               uint32 costC = connData->getCostB(0) + cost;
               changeConnectionCosts( node,
                                      curConn,
                                      connData->getCostA(0),
                                      connData->getCostB(0),
                                      costC,
                                      connData->getVehicleRestriction(false));
               curConn = curConn->getNext();
            }
         }
      }
   }
   return nbrDistAdded;
}
