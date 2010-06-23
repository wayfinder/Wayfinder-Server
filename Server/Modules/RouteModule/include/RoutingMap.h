/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTINGMAP_H
#define ROUTINGMAP_H

#include "config.h"
#include <iostream>
#include<set>
#include<map>
#include<vector>
#if defined (__GNUC__) && __GNUC__ > 2
#include <ext/algorithm>
using namespace __gnu_cxx;
#else
#include <algorithm>
using namespace std;
#endif

// There are some more includes below the class.

#include "IDPairVector.h"
#include "IDTranslationTable.h"
#include "RouteConstants.h"
#include "RMTypes.h"

class MapSafeVector;
class DataBuffer;
class Readable;
class TimeTableContainer;
class RoutingNode;
class OrigDestNode;

class MC2BoundingBox;
class DisturbanceVector;
class RoutingConnection;
class RoutingConnectionData;
class RoutingConnectionAndDataPair;
class ExternalRoutingConnection;

class DisturbanceStorage;

/**
 *   Class containing the map used for routing.
 *   Contains node and connection data and external nodes and connections 
 *   (connections to another maps).
 *   <br>
 *   <b>NB!</b> If mc2 is not compiled with -D_REENTRANT=1, the 
 *   loading of maps will not be threadsafe and the constructor will 
 *   read outside the DataBuffer sent from MapModule.}
 *   <br>
 *   FIXME: Allocate all the connections at once when loading the map.
 *   The problem is that the connections and nodes etc. must know if they
 *   are created in the map or elsewhere (copies) and behave differently
 *   in their destructors in the different cases.
 *
 */
class RoutingMap 
{
public:
   
   /**
    *   Initialization where the map is read from the mapserver.
    *   NB: See class documentation about unsafe threads.
    */
   RoutingMap( uint32 mapID );
   
   /**
    *   Destructor for the routing map.
    */
   ~RoutingMap();

   /**
    *   Creates destnodes for use in calccostdijkstra.
    */
   RoutingNode* createDestNodes( int nbr );
   
   /**
    *    This mthod must be called once after the RoutingMap is created.
    *    Fills the map with data from the MapModule.
    *    {\it {\bf NB!} This was previously done in the constructor, but
    *    since the map loading might fail, it must be possible for the
    *    caller to know the status of the creation!}
    *
    *    @param mapSize A number that should reflect the size of the
    *                   map. Not necessarily the true map size.
    *                   Not valid if the function returns false.
    *
    *    @return True if this routing map is created alright, false 
    *            otherwise.
    */
   bool load( uint32& mapSize, MapSafeVector* loadedMaps);
   
   /**
    *   Resets all the nodes (cost and gradient) in the map.
    *   Used by the routing algorithm (@see CalcRoute).
    */
   void reset();
   
   /**
    *    Get the ID of this routing map.
    *    @returns The ID of the map. 
    */
   inline uint32 getMapID() const;
   
   /**
    *    Returns a pointer to the set of neighbour map ids.
    */
   inline const set<uint32>* getNeighbourMapIDs() const;

   /**
    *    Returns the map id of the map with the supplied level.
    *    Only works upwards, since there can be meny maps on the
    *    level below. Use the lookuptable for that.
    *    @return The map id of the map or MAX_UINT32 if the level
    *            does not exist.
    */
   inline uint32 getMapAtLevel(uint32 level) const;
   
   /**
    *    Gets the node with a certain index in the array. 
    *    @param index The index in the node vector.
    *    @return A pointer to a node with index.
    */
   inline RoutingNode* getNode(uint32 index);
   
   /**
    *   Uses binary search to lookup the node with nodeID.
    *   @param node is the nodeID i.e. the itemID with toggled MSB.
    *
    *   @return A pointer to the node with the nodeID.
    */
   RoutingNode* getNodeFromTrueNodeNumber(uint32 nodeID);
   
   /**
    * Get the number of ordinary nodes.
    *
    * @return The size of m_nodeVector.
    */
   inline uint32 getNbrNodes() const;
   
   /**
    *    Get a pointer to the external nodes.
    *    @return A pointer to vector containing external nodes.
    */
   inline RoutingNode* getExternalNodes();
   
   /**
    *    Get the number of external nodes in this routing map.
    *    @return The number of nodes in external node vector.
    */
   inline uint32 getNumExternalNodes();
   
   /**
    *   Search the nodeVector to find the matching nodenumber.
    *   @param    itemID  item ID of the node to find the index for 
    *   @return   index of node in nodeVector. 
    */
   inline uint32 binarySearch( uint32 itemID );
   
   /**
    *    Get a pointer to the boundingbox for this map.
    *    @return The bounding box.
    */
   inline MC2BoundingBox* getMC2BoundingBox();

public:
   /**
    *    Translates the map and node id from the lower level
    *    to an itemID on this level. Node number will be preserved.
    *    @param mapID Lower level map id.
    *    @param node Lower level node id.
    *    @return The node id on this map or MAX_UINT32 if not found.
    */
   inline uint32 translateToHigher(uint32 mapID,
                                   uint32 nodeID);

   /**
    *    Translates the lowerPair to higher level.
    *    @param lowerPair The map and node id on lower level.
    *    @return A pair of map and node id on higher level or
    *            MAX_UINT32, MAX_UINT32 if not found.
    */
   inline IDPair_t translateToHigher(const IDPair_t& lowerPair);
   
   /**
    *    Translates the node id to a node id on a lower level
    *    map id. Node number will be preserved.
    *    @param nodeID Node Id on this level.
    *    @return An IDPair_t with the map and node id. Will return
    *            MAX_UINT32, MAX_UINT32 if not found.
    */
   inline IDPair_t translateToLower(uint32 nodeID);

   /**
    *    Translates the map and node id from higher to lower level.
    *    @param higherPair A pair of map and node id to be translated
    *                      to lower level.
    *    @return An IDPair_t containing the Map and node id on lower
    *            level or MAX_UINT32, MAX_UINT32 if the node could
    *            not be found or the map id is wrong.
    */
   inline IDPair_t translateToLower(const IDPair_t& higherPair);


   inline uint32 getEstCost(const RoutingNode* node) const;

   inline void setEstCost(RoutingNode* node, uint32 cost);
   
   inline uint32 getRealCost(const RoutingNode* node) const;

   inline void setRealCost(RoutingNode* node, uint32 cost);

   inline RoutingNode* getGradient(const RoutingNode* node) const;

   inline void setGradient(RoutingNode* node, RoutingNode* gradient);
   
   inline bool isDest(const RoutingNode* node) const;

   inline void setDest(RoutingNode* node, bool isDest);

   inline bool isVisited(const RoutingNode* node) const;

   inline void setVisited(RoutingNode* node, bool visited);

   /**
    *    Dump all nodes and connection to standard out.
    */
   void dump();
   
   /**
    *    Dumps all external nodes and connections to standard out.
    */
   void dumpExternal();
   
   /**
    * Dumps the route costs of the ordinary nodes to a file.
    *
    * @param outfile The file to dump the data to.
    */
   void dumpAllRouteCosts(ofstream& outfile);
   
   /**
    *    Get a random item on this map.
    *    @return A random ItemID that is valid in the map. Used for 
    *            profiling.
    */
   inline uint32 getRandomItemID();
   
   /**
    *   Returns the next time in the the timetable.
    *
    *   @param lineID is the id of the line.
    *   @param time is the current time to find the next entry in the
    *               timetable for.
    *  @return the next time.or MAX_UINT16 if not found.
    */
   uint32 getNextDepartureTime( uint16 lineID, uint32 nodeID, uint32 time );

   /**
    *   Returns the current infinity.
    */
   inline uint8 getInfinity() const;

   /**
    *   Creates a new OrigDestNode. Will be needed when the
    *   writeable part of the nodes is separated from the readable.
    */
   OrigDestNode* newOrigDestNode(uint32 index,
                                 uint32 mapID,
                                 uint16 offset,
                                 uint32 lat,
                                 uint32 lon,
                                 uint32 cost,
                                 uint32 estCost,
                                 uint32 costASum,
                                 uint32 costBSum,
                                 uint32 costCSum);

   /**
    *   Creates a new OrigDestNode. Will be needed when the
    *   writeable part of the nodes is separated from the readonly.
    */
   OrigDestNode* newOrigDestNode( const IDPair_t& id,
                                  uint16 offset );

   
//-----------------------------------------------------------------------
// "Map editing"-functions start here
//-----------------------------------------------------------------------   


   /**
    *    Rolls back all changes to the connection costs
    *    from the stack. If <code>temporary</code> is true
    *    the temporary stack will be used and if it's false
    *    the main stack will be used.
    */
   int rollBack(bool temporary);

   /**
    *    Changes the connection costs to new values and adds the
    *    change to the <code>rollBackStack</code>.
    *    @param node The node that has the connection.
    *    @param conn The connection that is affected.
    *    @param rollBackStack The stack to store the old data in.
    *    @param costA The new costA.
    *    @param costB The new costB.
    *    @param costC The new costC.
    *    @param vehicleRestriction The new vehicle restrictions.
    */
   void changeConnectionCosts( RoutingNode* node,
                               RoutingConnection* conn,
                               DisturbanceStorage* rollBackStack,
                               uint32 costA,
                               uint32 costB,
                               uint32 costC,
                               uint32 vehicleRestrictions);

   /**
    *    Changes the connection costs to new values and adds the
    *    change to the <code>rollBackStack</code>.
    *    @param node The node that has the connection.
    *    @param conn The connection that is affected.
    *    @param temporary True if the disturbance is temporary.
    *                     False if the disturbance is from InfoModule.
    *    @param costA The new costA.
    *    @param costB The new costB.
    *    @param costC The new costC.
    *    @param vehicleRestriction The new vehicle restrictions.
    */
   void changeConnectionCosts( RoutingNode* node,
                               RoutingConnection* conn,
                               bool temporary,
                               uint32 costA,
                               uint32 costB,
                               uint32 costC,
                               uint32 vehicleRestrictions);
   
   /**
    *   Changes the cost to traverse a node.
    *   @param node   The node to change the cost for.
    *   @param factor The factor to multiply the old costs with.
    *   @param rollBackStack The rollbackstack to save the change in.
    */
   uint32 changeNodeCost( RoutingNode* node,
                          float factor,
                          DisturbanceStorage* rollBackStack);

   /**
    *   Changes the cost to traverse a node.
    *   @param nodeID    The id of the node to change the cost for.
    *   @param factor    The factor to multiply the old costs with.
    *   @param temporary True if the temporary rollbackstack should be used.
    */
   uint32 changeNodeCost( uint32 nodeID,
                          float factor,
                          bool temporary);

   /**
    *   Sets a new list of connections in a node. The new list can
    *   be NULL.
    *   @param node The node to change.
    *   @param firstNewConnection The new list of connections.
    *   @param forward True if the forwardconnections should be replaced.
    *   @param rollBackStack The rollbackstack to save the changes in.
    */ 
   uint32 setConnections(RoutingNode* node,
                         RoutingConnection* firstNewConnection,
                         bool forward,
                         DisturbanceStorage* rollBackStack);
                         
   
   /**
    *   Changes the cost of a segment to infinity, i.e. removes
    *   all connections to/from the node.
    *   @param nodeID    The segment to change.
    *   @param temporary True if the cost should be changed for one route only.
    */
   uint32 avoidNode( uint32 nodeID,
                     bool temporary );

   /**
    *   Multiplies the costC of the node with the infoModuleFactor
    *   divided by 1000.
    *   If <code>inCostC</code> is <code><b>MAX_UINT32</b></code> the
    *   vehicle restrictions will be set to zero instead.
    *   Operates on the permanent disturbances.
    *   @param node             The node to change.
    *   @param infoModuleFactor Will be divided by 1000 and then costC is
    *                           multiplied with it.
    */
   uint32 multiplyNodeCost(RoutingNode* node,
                           uint32 infoModuleFactor );

   /**
    *   Restores the original connections from node node.
    *   @param node The node to restore.
    */
   uint32 restoreConnections(RoutingNode* mode);
   
   /**
    *   Tries to add all the disturbances in <code>distVect</code> to 
    *   the map.
    *   @return Number of disturbances added succesfully.
    */
   int addDisturbances(const DisturbanceVector* distVect,
                       bool temporary = true);

   /**
    *   Checks if there is a disturnbance between fromID and toID.
    *   Puts the old costs in costA--costD.
    *   @param fromID The node the connection leads from.
    *   @param toID   The node the connection leads to.
    *   @param costA Old cost a.
    *   @param costB Old cost b.
    *   @param costC Old cost c.
    *   @param costD Old cost d.
    *   @return True if there is a temporary disturbance and the costs are
    *           valid.
    */
   inline bool checkTempDisturbances(uint32 fromID,
                                     uint32 toID,
                                     uint32& costA,
                                     uint32& costB,
                                     uint32& costC,
                                     uint32& costD) const;

   /**
    *    Lookup the start and end node of an expanded node.
    *    If the node is not part of a multi connection then 
    *    method returns false.
    *
    *    @param nodes [Out]   Set of from- and to-nodes, i.e.
    *                         multiconnections that this nodes is part of.
    *    @param nodeID        The expanded (skipped) node.
    *    @return  True if node is part of a multiconnection and the
    *             outparameters are set correctly. False otherwise.
    */
   inline bool lookupExpandedNodes( set<fromToNode_t>& nodes,
                                    uint32 nodeID) const;
   
private:

   /**
    *   Used by binary search algorithm in binarysearch.src.
    *   @param  key  The itemId to be compared to.
    *   @param  val  The index of the node to compare with in the
    *                vector.
    *   @return 0 if the values are equal, 1 if node[val].itemID > key 
    *           and -1 otherwise.
    */
   inline int SEARCH_COMP( uint32 key, uint32 val );
   
   /**
    *    Loads the map from the MapModule, when given a socket.
    *
    *    @param  TCPSock The socket to get data from.
    *    @param  mapSize An estimate of how much data that was read.
    *    @return True if all went ok, false if received map was corrupt.
    */
   bool loadMapFromSocket(Readable* TCPSock,
                          uint32& mapSize);

   /**
    *    Reads the map hierarchy for this map from the supplied socket.
    *    @param socket Socket to read from.
    *    @return <code>true</code> if successful.
    */
   bool readMapHierarchy(Readable* socket);
   
   /**
    *    Reads the node expansion table for this map from the 
    *    supplied socket.
    *    @param socket Socket to read from.
    *    @return <code>true</code> if successful.
    */
   bool readNodeExpansionTable(Readable* socket);
   
   /**
    *    Create the nodes from the databuffer send from the MapModule.
    *
    *    @param buff A pointer to the DataBuffer containing the map.
    *    @return True if all went OK, false otherwise.
    */
   bool processNodeData( DataBuffer* buff, uint32 nbrNodes);
   
   /**
    *    Create the connections from the databuffer send from
    *    the MapModule.
    *
    *    @param buff A pointer to the DataBuffer containing the map.
    *    @param nbrConnections The number of connections that will be
    *                          sent. 
    *    @return True if all went OK, false otherwise.
    */
   bool processConnectionData( DataBuffer* buff,
                               int nbrConnections);
   
   /**
    *    Create (currently) forward connections from the data
    *    sent from the MM.
    *
    *    @param buff A pointer to the DataBuffer containing the map.
    *    @param nbrConnections The number of connections that will be
    *                          sent. 
    *    @return True if all went OK, false otherwise.
    */
   bool processOppositConnectionData( DataBuffer& buf,
                                      int nbrConnections);
   
   /**
    *    Create the external nodes from the databuffer send from
    *    the MapModule.
    *
    *    @param buff A pointer to the DataBuffer containing the map.
    *    @return True if all went OK, false otherwise.
    */
   bool processExternalNodeData( DataBuffer* buff, uint32 nbrNodes);
   
   /**
    *    Create the external connections from the databuffer send from
    *    the MapModule.
    *
    *    @param buff A pointer to the DataBuffer containing the map.
    *    @param nbrConnections The total number of external connections.
    *    @return True if all went OK, false otherwise.
    */
   bool processExternalConnectionData( DataBuffer* buff,
                                       int nbrConnections);
   
   /**
    *    Read buffer data from the socket. Read 8 bytes from the
    *    given socket and sets the outparameters to the correct
    *    values (zero upon error).
    *
    *    @param   socket         The socket from where the data 
    *                            should be read.
    *    @param   nbrDataItems   Outparameter that is set to the
    *                            number of items in the databuffer.
    *    @param   dataSize       Outparameter that is set to the
    *                            size of the data in the databuffer.
    *    @return  True if the outparameters is set correctly, false
    *             otherwise.
    */
   bool setBuffertData(Readable* socket, uint32 &nbrDataItems, 
                       uint32 &dataSize);
   
   /**
    *    Create the lookup tables for mapping a mapID and a nodeID 
    *    on a lower hierarchical level to a nodeID on a higher level.
    *    Only used if map is on a higher level.
    *    @param buff      Buffer to read from.
    */
   void createHigherLevelTable( DataBuffer* buff);
   
   /**
    *    Update all the connections in the forward directions.
    */
   void updateConnections();
   
   /**
    *    Delete all the nodes and their connections. Used if map 
    *    loading went wrong.
    */
   void deleteAllMapData();
  
   
   // ==================================================================
   //                                                 Member variables =
   // ==================================================================
   
   /**
    *    Array containg all the edge (external) nodes on the map,
    *    i.e. the nodes that are connected to other maps. 
    */
   RoutingNode* m_externalNodeVector;
     
   /**
    *    The size (nbr of nodes) in externalNodeVector.
    */
   uint32 m_nbrExternalNodes;
   
   /**
    *    Array containg all the internal nodes on the map.
    */      
   RoutingNode* m_nodeVector;

   /**
    *    Estimated costs for the nodes. Not used yet.
    */
   uint32* m_estCosts;

   /**
    *    Real costs for the nodes. Not used yet.
    */
   uint32* m_realCosts;

   /**
    *    Gradients for the nodes. Not used yet.
    */
   RoutingNode** m_gradients;

   /**
    *    Place to store if the nodes are destinations. Not used yet.
    */
   byte* m_isDests;
   
   /**
    *    Place to store if the nodes have been visited.
    */
   byte* m_isVisiteds;
   
   /**
    *    The size (nbr of nodes) in nodeVector.
    */
   uint32 m_nbrNodes;
   
   /**
    *    The ID of this map.
    */
   uint32 m_mapID;
   
   /**
    *    The bounding box of the map.
    */
   MC2BoundingBox* m_boundingBox;

   /**
    *    IDTranslationTable to translate to and from node id:s on
    *    higher level maps.
    */
   IDTranslationTable m_idTranslationTable;
   
   /** 
    *    The timetable object.
    */
   TimeTableContainer* m_timeTable;
   
   /**
    *   The rollback stack for updates from InfoModule.
    */
   DisturbanceStorage* m_mainRollBackStack;

   /**
    *   The rollback stack for updates from the user.
    */
   DisturbanceStorage* m_tempRollBackStack;
      
   /**
    *   Set containing the neigbour maps.
    */
   set<uint32> m_neighbours;

   /**
    *    Number of expanded node, which is also the
    *    size of m_expNodeIndexTable.
    */
   uint32 m_nbrExpNodes;
   
   /**
    *    Number of multiconnections, which is also the
    *    size of m_fromToNodeTable.
    */
   uint32 m_nbrMultiConnections;

   /**
    *    Sorted table containing expanded nodes and their index in
    *    m_fromToNodeTable.
    */
   nodeIndex_t* m_expNodeIndexTable;

   /**
    *    Table containing from and to nodes of multiconnections.
    *    Ordered according to the indices in m_expNodeIndexTable;
    */
   fromToNode_t* m_fromToNodeTable;

   /**
    *   Current infinity.
    */
   uint8 m_curInf;
   
   /**
    *   Allocates enough connections and connectiondatas
    *   to read the map.
    *   @param nbrConnections The number of connections that the
    *                         MapModule wants to send.
    */
   void allocateConnections(int nbrConnections);

   /**
    *   Returns a pointer to the next not used connection in the array
    */
   RoutingConnection* getNextConnection();

   /** 
    *   Returns a pointer to the next not used connectiondata.
    */
   RoutingConnectionData* getNextConnectionData();

   /**
    *   Allocates enough external connections and connectiondatas
    *   to read the map.
    *   @param nbrConnections The number of connections that the
    *                         MapModule wants to send.
    */   
   void allocateExternalConnections(int nbrConnections);

   /**
    *   Returns the next unused ExternalRoutingConnection.
    *   @return A unused ExternalRoutingConnection.
    */
   ExternalRoutingConnection* getNextExtConnection();

   /**
    *   Returns the next unused RoutingConnectionData for
    *   use for external connections.
    */
   RoutingConnectionData* getNextExtConnectionData();
   
   /** Array of preallocated connections */
   RoutingConnection* m_connections;
   /** Array of preallocated connection datas */
   //RoutingConnectionData* m_connDatas;
   RoutingConnectionAndDataPair* m_connDatas;
   
   /** Array of preallocated external connections */
   ExternalRoutingConnection* m_extConns;
   /** Array of preallocated external connection datas */
   RoutingConnectionData* m_extConnDatas;
   /** Number of connections */
   int m_nbrAllocatedConnections;
   /** Number of conndatas */
   int m_nbrAllocatedConnDatas;
   /** Index of next not used connection */
   int m_nextFreeConnIndex;
   /** Index of next not used connectiond data */
   int m_nextFreeConnDataIndex;
   /** Number of external connections */
   int m_nbrAllocatedExtConns;
   /** Number of allocated external conndatas */
   int m_nbrAllocatedExtConnDatas;
   /** Index of next not used external connexion */
   int m_nextFreeExtConnIndex;
   /** Index of next not used external conndata */
   int m_nextFreeExtConnDataIndex;

   /** The type of map used for storing level information */
   typedef map<uint32, uint32> levelMap_t;

   /** Information about the map id:s on different levels */
   levelMap_t m_levels;
};

#include "RoutingNode.h"
#include "DisturbanceStorage.h"

// ========================================================================
//                                      Implementation of inlined methods =
// ========================================================================

// -- Methods to be used later. Problem with the OrigDestNode.

inline uint32
RoutingMap::getEstCost(const RoutingNode* node) const
{
   return m_estCosts[node->getIndex()];
}

inline void
RoutingMap::setEstCost(RoutingNode* node, uint32 cost)
{
   m_estCosts[node->getIndex()] = cost;
}

inline uint32
RoutingMap::getRealCost(const RoutingNode* node) const
{
   return m_realCosts[node->getIndex()];
}

inline void
RoutingMap::setRealCost(RoutingNode* node, uint32 cost)
{
   m_realCosts[node->getIndex()] = cost;
}

inline RoutingNode*
RoutingMap::getGradient(const RoutingNode* node) const
{
   return m_gradients[node->getIndex()];
}

inline void
RoutingMap::setGradient(RoutingNode* node, RoutingNode* gradient)
{
   m_gradients[node->getIndex()] = gradient;
}

inline bool
RoutingMap::isDest(const RoutingNode* node) const
{
   return bool(m_isDests[node->getIndex()]);
}

inline void
RoutingMap::setDest(RoutingNode* node, bool isDest)
{
   m_isDests[node->getIndex()] = isDest;
}

inline bool
RoutingMap::isVisited(const RoutingNode* node) const
{
   return bool(m_isVisiteds[node->getIndex()]);
}

inline void
RoutingMap::setVisited(RoutingNode* node, bool visited)
{
   m_isVisiteds[node->getIndex()] = visited;
}

// -- End


inline uint32
RoutingMap::getMapID() const
{
   return m_mapID;
}

inline const set<uint32>*
RoutingMap::getNeighbourMapIDs() const
{
   return &m_neighbours;
}

inline RoutingNode*
RoutingMap::getNode( uint32 index )
{
   MC2_ASSERT( index < m_nbrNodes );
   
   return &m_nodeVector[index];
}


inline uint32
RoutingMap::getNbrNodes() const
{
   return m_nbrNodes;
}


inline RoutingNode*
RoutingMap::getExternalNodes()
{
   return m_externalNodeVector;
}

inline uint32
RoutingMap::getNumExternalNodes()
{
   return m_nbrExternalNodes;
}

inline MC2BoundingBox*
RoutingMap::getMC2BoundingBox()
{
   return m_boundingBox;
}
      
inline uint32
RoutingMap::translateToHigher(uint32 mapID,
                              uint32 nodeID)
{
   // Save the node number 
   const uint32 nodePattern = GET_UINT32_MSB( nodeID );

   const uint32 itemID =
      m_idTranslationTable.translateToHigher(mapID,
                                             REMOVE_UINT32_MSB(nodeID));

   return nodePattern | itemID;

}

inline IDPair_t
RoutingMap::translateToHigher(const IDPair_t& lowerPair)
{
   const uint32 higherNode = translateToHigher(lowerPair.first,
                                               lowerPair.second);
   
   if ( higherNode != MAX_UINT32 ) {
      return IDPair_t( getMapID(), higherNode );      
   } else {
      return IDPair_t( MAX_UINT32, MAX_UINT32 );
   }
}

inline IDPair_t
RoutingMap::translateToLower(uint32 nodeID)
{
   // Save the node number
   const uint32 nodePattern = GET_UINT32_MSB( nodeID );

   IDPair_t
      id(m_idTranslationTable.translateToLower(REMOVE_UINT32_MSB(nodeID)));

   id.second |= nodePattern;
   return id;
  
}

inline IDPair_t
RoutingMap::translateToLower(const IDPair_t& higherPair)
{
   // Check if it's this map
   if ( higherPair.first != getMapID() ) {
      mc2log << warn << here << " wrong map id - should be "
                << getMapID() << " is " << higherPair.first << endl;
      return IDPair_t( MAX_UINT32, MAX_UINT32 );
   }

   // Translate it to lower level.
   return translateToLower( higherPair.second );
   
}

inline uint32
RoutingMap::getRandomItemID()
{
   uint32 index = uint32((double(rand()) / RAND_MAX) * m_nbrNodes);
   return REMOVE_UINT32_MSB(m_nodeVector[index].getItemID());
}

int RoutingMap::SEARCH_COMP( uint32 key, uint32 val ) {
   if (m_nodeVector[val].getItemID() == key)
      return 0;
   else
      return (m_nodeVector[val].getItemID() < key ) ? 1 : -1;

}


inline bool
RoutingMap::checkTempDisturbances(uint32 fromID,
                                  uint32 toID,
                                  uint32& costA,
                                  uint32& costB,
                                  uint32& costC,
                                  uint32& costD) const
{
   return m_tempRollBackStack->getOldCosts(fromID,
                                           toID,
                                           costA,
                                           costB,
                                           costC,
                                           costD);
}

inline uint32
RoutingMap::getMapAtLevel(uint32 level) const
{
   levelMap_t::const_iterator it = m_levels.find(level);
   if ( it == m_levels.end() ) {
      return MAX_UINT32;
   } else {
      return it->second;
   }
}

inline bool
RoutingMap::lookupExpandedNodes( set<fromToNode_t>& nodes,
                                 uint32 nodeID ) const
{
   MC2_ASSERT( m_expNodeIndexTable != NULL );
   MC2_ASSERT( m_fromToNodeTable != NULL );

   nodeIndex_t searchKey;
   searchKey.first = nodeID;
  
   nodeIndex_t* endOfTable = m_expNodeIndexTable + m_nbrExpNodes;   

   //Slow MC2_ASSERT( is_sorted( m_expNodeIndexTable, endOfTable ) );
   
   // Binary search
   pair<nodeIndex_t*, nodeIndex_t*> range = 
       equal_range( m_expNodeIndexTable,             
                    endOfTable,
                    searchKey );

   for( nodeIndex_t* it = range.first;
        it != range.second;
        ++it ) {
      // Found
      const uint32 index = it->second;
      MC2_ASSERT( index < m_nbrMultiConnections );
      nodes.insert(m_fromToNodeTable[ index ]);
   }
   
   return !nodes.empty();
   
}

inline uint32
RoutingMap::binarySearch( uint32 itemID ) 
{
   uint32 KEY = itemID;
   uint32 const SEARCH_START_INDEX = 0;
   uint32 const SEARCH_STOP_INDEX  = m_nbrNodes - 1;
#undef SEARCH_COMP   
#include "binarysearch.src"

   return RESULT_INDEX;
}

inline
uint8
RoutingMap::getInfinity() const
{
   return m_curInf;
}


#endif

