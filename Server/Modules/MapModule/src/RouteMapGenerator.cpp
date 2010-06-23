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

#include "DatagramSocket.h"
#include "multicast.h"
#include "BusRouteItem.h"
#include "StreetSegmentItem.h"
#include "TCPSocket.h"
#include "IDTranslationTable.h"
#include "RouteMapGenerator.h"

#include "ExternalConnections.h"
#include "GenericMap.h"
#include "OverviewMap.h"
#include "MapModuleNoticeContainer.h"

#include "MapHandler.h"
#include "ByteBuffer.h"
#include "Node.h"
#include "MapBits.h"
#include "NodeBits.h"

#if defined (__GNUC__) && __GNUC__ > 2
#include <ext/algorithm>
using namespace __gnu_cxx;
#else
#include <algorithm>
using namespace std;
#endif
#include <stdlib.h>

#define FORWARD_CONNECTIONS_FIRST
#ifdef FORWARD_CONNECTIONS_FIRST
/// Direction for the connections sent first
#define first_conn_dir  true
/// Direction for the connections sent second
#define second_conn_dir false
#else
#define first_conn_dir  false
#define second_conn_dir true
#endif

class TmpRouteNode {
public:

   TmpRouteNode() {}
   
   TmpRouteNode( uint32 nodeID,
                 const MC2Coordinate& nodeCord,
                 uint32 restrictions) : m_nodeID( nodeID ),
                                        m_coord( nodeCord ),
                                        m_rest( restrictions ) {
   }

   /// ID of this node
   uint32 m_nodeID;
   /// Coordinate of this node.
   MC2Coordinate m_coord;
   ///  Restrictions of the node
   uint32 m_rest;
   /// Index of the node in the node vector
   uint32 m_idx;  
};

/**
 *   Container for the nodes that should be sent to the
 *   RouteModule. 
 */
class TmpRouteNodeVector : public vector<TmpRouteNode> {

   class NodeCmp {
   public:
      bool operator()(const TmpRouteNode& a, const TmpRouteNode& b ) {
         return a.m_nodeID < b.m_nodeID;
      }
      bool operator()(const TmpRouteNode& a, uint32 nodeID ) {
         return a.m_nodeID < nodeID;
      }
   };

public:
   /**
    *   Sorts the nodes. RouteModule must also know the sorting
    *   order in order to be able to find the nodes using ID.
    */
   void sort() {
      if ( !is_sorted( begin(), end(), NodeCmp() ) ) {
         std::sort( begin(), end(), NodeCmp() );
      } else {
         mc2dbg << "[TRNV]: Already sorted" << endl;
      }
      m_idToIdxVect.clear();
      m_idToIdxVect.reserve( size() );
      for ( uint32 i = 0; i < size(); ++i ) {
         (*this)[i].m_idx = i;
         m_idToIdxVect.push_back( make_pair( (*this)[i].m_nodeID,
                                             (*this)[i].m_idx ) );
      }
      // Now also sort the pairs. They have a nice operator<
      std::sort( m_idToIdxVect.begin(), m_idToIdxVect.end() );
   }
   

   /**
    *   Returns the index of a node in the vector using the
    *   node ID.
    */
   uint32 getIdxFromID( uint32 nodeID ) const {
      idVect_t::const_iterator findit =
         std::lower_bound( m_idToIdxVect.begin(),
                           m_idToIdxVect.end(),
                           make_pair( nodeID, -1 ) );
      
      MC2_ASSERT( findit != m_idToIdxVect.end() );
      MC2_ASSERT( findit->first == nodeID );
      return findit->second;
   }

private:
   /// Typedef for the id to index vector
   typedef vector<pair<uint32,int> > idVect_t;
   /// Maps id to index. Id in first, idx in second.
   idVect_t m_idToIdxVect;
   
};

class TmpRouteConnData {
public:

   TmpRouteConnData() {}

   TmpRouteConnData( DataBuffer& buf ) {
      m_costA = buf.readNextLong();
      m_costB = buf.readNextLong();
      m_costC = buf.readNextLong();
      m_costD = buf.readNextLong();
      m_vehicleRest = buf.readNextLong();
   }
   
   uint32 m_costA;
   uint32 m_costB;
   uint32 m_costC;
   uint32 m_costD;
   uint32 m_vehicleRest;
   uint32 m_index;
};

/**
 *   Connection as used in the RouteModule.
 *   Forward connections are created by sorted using from node
 *   and backward connections are created by sorting using tonode.
 */
class TmpRouteConnection {
public:

   /// Default constructor. NULLs m_connData to detect errors.
   TmpRouteConnection() : m_connData(NULL) {}

   /**
    *   @param fromNodeID ID of the from-node.
    *   @param toNodeID   ID of the to-node.
    *   @param connData   Pointer to the connectionData.
    */
   TmpRouteConnection( uint32 fromNodeID,
                       uint32 toNodeID,
                       TmpRouteConnData* connData )
         : m_fromNodeID( fromNodeID ),
           m_toNodeID( toNodeID ),
           m_connData( connData ) {
      MC2_ASSERT( m_fromNodeID != m_toNodeID );
   }

   uint32 getNodeIdx( bool from, const TmpRouteNodeVector& v ) const {
      if ( from ) {
         return v.getIdxFromID( m_fromNodeID );
      } else {
         return v.getIdxFromID( m_toNodeID );
      }
   }
      
   uint32 m_fromNodeID;
   uint32 m_toNodeID;
   TmpRouteConnData* m_connData;   
};

RouteMapGenerator::RouteMapGenerator(MapHandler *mh,
                                     uint32 *mapIDs,
                                     uint32 nbrMaps,
                                     uint32 port,
                                     const vector<uint32>& overview)
      : MapGenerator( mh, 
                      mapIDs, 
                      nbrMaps, 
                      port)

{
   m_overviewMaps = overview;
   mc2dbg2 << "RouteMapGenerator created" << endl;
}


RouteMapGenerator::~RouteMapGenerator()
{
   mc2dbg2 << "RouteMapGenerator destroyed" << endl;
}

bool
RouteMapGenerator::getIndexDBNeeded() const
{
   return true;
}


void
RouteMapGenerator::writeConnectionData(DataBuffer* conBuffer,
                                       uint32 costA,
                                       uint32 costB,
                                       uint32 costC,
                                       uint32 costD,
                                       uint32 vehicleRest)
{
   // cost A
   conBuffer->writeNextLong(costA);

   // cost B
   conBuffer->writeNextLong(costB);

   // cost C
   conBuffer->writeNextLong(costC);

   // cost D
   conBuffer->writeNextLong(costD);
   
   // Vehicle restrict
   conBuffer->writeNextLong(vehicleRest);

}

inline void
RouteMapGenerator::writeConnectionData(DataBuffer* conBuffer, 
                                       Connection* conn,
                                       const Node* toNode,
                                       const GenericMap* theMap,
                                       bool externalConnection)
{
   
   uint32 costA, costB, costC, costD;
   
   theMap->getConnectionCost(conn, const_cast<Node*>(toNode),
                             costA, costB,
                             costC, costD, // outparams
                             externalConnection);   
   uint32 vehicleRestrictions = theMap->getVehicleRestrictions( *conn );
   
   // -->
   // Remove code when maps are regenerated.
   // Start temoporary code.
   // 
   // Remove avoid tollroad and avoid highway bits from the vehicle
   // restrictions. 
   vehicleRestrictions &= ~(uint32( ItemTypes::avoidTollRoad | 
                                    ItemTypes::avoidHighway ));
   Node* fromNode = theMap->nodeLookup( conn->getConnectFromNode() );
   if ( fromNode != NULL ) {

      // Add avoid toll road when going from non tollroad to tollroad.
      if ( ! fromNode->hasRoadToll() &&
           toNode->hasRoadToll() ) {
         // Changing from non toll road -> toll road.
         // Add the avoid toll road vehicle type to the vehicle restrictions.
         vehicleRestrictions |= ItemTypes::avoidTollRoad;
      }

      // Add avoid highway for all highways (controlled access).
      StreetSegmentItem* ssi = 
         item_cast<StreetSegmentItem*> (theMap->itemLookup( 
                  MapBits::nodeItemID( fromNode->getNodeID() ) ) );
      if ( ( ssi != NULL ) && 
           ( ssi->isControlledAccess() || ssi->getRoadClass() == 0 ) ) {
         vehicleRestrictions |= ItemTypes::avoidHighway;
      }
   }
   // End temporary code.
   // <-- 
        
   writeConnectionData(conBuffer, costA, costB, costC, costD,
                       vehicleRestrictions );
}

inline int
RouteMapGenerator::checkAndAddBoundrySegment(int nodeNbr,
                                             const GenericMap* theMap,
                                             BoundrySegment* curBS,
                                             Vector& sendNodes)
{
   const uint32 tmpID = curBS->getConnectRouteableItemID();

   // Calculate node to add to vector. The _other_ node will
   // be added  since all we have here is the backward connection
   // but the RouteModule needs the forward connection.
   const uint32 addID = ((nodeNbr == 0) ? (tmpID | 0x80000000) :
                                          (tmpID & 0x7fffffff));
   
   // The routeable item is here to check that only
   // the connection from the real external node is
   // added, i.e. the one without internal connections.
   // If it is enabled, the routing on overview maps
   // does not work.
   RouteableItem* ri =
      static_cast<RouteableItem*>(theMap->itemLookup(tmpID));

   const int nbrExtConns = curBS->getNbrConnectionsToNode(nodeNbr);
   const int nbrIntConns = ri->getNode(nodeNbr)->getNbrConnections();

   bool addOK = false;
   // The node should not have internal connections and external
   // connections at the same time.
   if ( nbrExtConns > 0 ) {
      if ( nbrIntConns == 0 ) {
         addOK = true;
      } else {
         mc2dbg << "[RMG]: " << IDPair_t(theMap->getMapID(), tmpID)
                << " has both connection to node "
                << nodeNbr << " and external connection"
                << endl;
         // Should not happen, but if we don't add it here
         // it will not work when routing on overview maps.
         if ( MapBits::isOverviewMap(theMap->getMapID() ) ) {
            // Testing not adding them on underview.
            //addOK = true;
         }
      }
   }
                         
   // Add to vector if all was OK.
   if ( addOK ) {
      sendNodes.push_back(addID);
      return curBS->getNbrConnectionsToNode(nodeNbr);
   } else {
      return 0;
   }      
}

inline bool
RouteMapGenerator::sendMapHierarchy( const GenericMap* theMap,
                                     Writeable* socket )
{
   // Get the mapID
   uint32 mapID = theMap->getMapID();
   
   // Map to put levels and mapids in.
   map<uint32,uint32> levels;

   // Add the map itself.
   levels[MapBits::getMapLevel(mapID)] = mapID;

   // Add the overview maps
   for( uint32 i = 0; i < m_overviewMaps.size(); ++i ) {
      levels[MapBits::getMapLevel(m_overviewMaps[i])] = m_overviewMaps[i];
   }
   
   DataBuffer* buf = new DataBuffer(levels.size() * 8 + 4 + 4);

   buf->writeNextLong(0); // Nbr items. Will be written by sendBuffer
   buf->writeNextLong(0); // Number of bytes. Also sendBuffer
   
   // Write to databuffer.
   for( map<uint32,uint32>::const_iterator it = levels.begin();
        it != levels.end();
        ++it ) {
      buf->writeNextLong(it->first);  // Level nbr
      buf->writeNextLong(it->second); // Map id
   }
   // Send the stuff
   bool result = sendBuffer(buf, levels.size(), socket, "LevelInfo") > 0;
   
   delete buf;
   return result;
}

bool
RouteMapGenerator::sendNodeExpansionTable( const GenericMap* theMap,
                                           Writeable* socket )
{   
   list<GenericMap::multiNodes_t> firstLastNodeByIndex;
   multimap<uint32, uint32> indexByExpandedNode;
   uint32 index = 0;
   typedef map<GenericMap::multiNodes_t, 
               GenericMap::expandedNodes_t>::const_iterator 
                  tableIt_t;
   
   for ( tableIt_t it = theMap->m_nodeExpansionTable.begin(); 
         it != theMap->m_nodeExpansionTable.end(); ++it ) {   
      firstLastNodeByIndex.push_back( it->first );
      typedef GenericMap::expandedNodes_t::const_iterator enIt_t;
      for ( enIt_t jt = it->second.begin();
            jt != it->second.end(); ++jt ) {

         indexByExpandedNode.insert( make_pair( *jt, index ) );
      }
      ++index;
   }
 
   uint32 nbrExpandedNodes = indexByExpandedNode.size();
   
   uint32 size = 4 + 4 + nbrExpandedNodes * ( 4 + 4 ) + 
      4 + index * ( 4 + 4 );
   DataBuffer* buf = new DataBuffer( size, "sendNodeExpansionTable" );
   DataBufferChecker dbc( *buf, "sendNodeExpansionTable" );
   
   buf->writeNextLong(0); // Nbr items. Will be written by sendBuffer
   buf->writeNextLong(0); // Number of bytes. Also sendBuffer
   
   for ( multimap<uint32, uint32>::const_iterator it = 
            indexByExpandedNode.begin(); it != indexByExpandedNode.end();
         ++it ) {
      // Expanded node
      buf->writeNextLong( it->first );
      // Index
      buf->writeNextLong( it->second );
   }
   
   // Nbr index
   buf->writeNextLong( index );
   
   for ( list<GenericMap::multiNodes_t>::const_iterator it = 
            firstLastNodeByIndex.begin(); it != firstLastNodeByIndex.end();
         ++it ) {
      // From node
      buf->writeNextLong( it->first );
      // To node
      buf->writeNextLong( it->second );
   }
   
   // Check databuffer
   dbc.assertPosition( size );
   
   // Send the stuff
   bool result = sendBuffer(buf, indexByExpandedNode.size(), socket,
                            "Multiconnections") > 0;
   
   delete buf;
   return result;
}


void
RouteMapGenerator::run()
{
   /* For printing the handshake later */
   const int  maxHandshake = 1024;
   char handshake[maxHandshake];
   
   TCPSocket* socket = waitForConnection(handshake, maxHandshake);
   if ( socket == NULL ) {
      mc2log << error << "No connection for RouteMapGenerator" << endl;
      return;
   }
   
   uint32 startTime = TimeUtility::getCurrentTime();
   // We've got an request for a route-map...
   uint32 mapID = mapIDs[0];   

   mc2dbg2 << "RouteMapGenerator sending mapID = " << mapID << endl;

   const GenericMap* theMap = mapHandler->getMap(mapID);
   
   if (theMap != NULL) {
      const bool sendResult = sendMap(theMap, socket);      
      char logString[1024+maxHandshake];
      if ( sendResult ) {
         sprintf(logString,
                 "RouteMapGenerator sent all data for map %u (\"%s\"),"
                 " processing time %u ms\n", mapID, handshake,
                 TimeUtility::getCurrentTime() - startTime);
         mc2log << info << logString;
      } else {
         sprintf(logString,
                 "RouteMapGenerator FAILED to send all data for "
                 "map %u (\"%s\"),"
                 " processing time %u ms\n", mapID, handshake,
                 TimeUtility::getCurrentTime() - startTime);
         mc2log << warn << logString;
      }

   } else {
      // theMap == NULL
      char logString[1024+maxHandshake];
      sprintf(logString, " Map (%u) not present", mapID);
      mc2log << warn << here << logString << endl;
   }
   socket->close();
   delete socket;
   // Done.
}

inline
int
RouteMapGenerator::addNodesAndCountConnections(Vector& nodes,
                                               const GenericMap* theMap)
{
   // Vector to add the "node 1":s in
   // The contents of this vector is then added to the end of the other
   // vector which will retain the sorting.
   vector<uint32> otherNodes;
   otherNodes.reserve(nodes.getAllocSize());

   // Counter for connections.
   uint32 totNbrConnections = 0;

   // No nodes if country map.
   if ( MapBits::isCountryMap(theMap->getMapID() ) ) {
      return 0;
   }
  
   for ( uint32 currentZoom=0; 
         currentZoom < NUMBER_GFX_ZOOMLEVELS; 
         currentZoom++) {
      
      const uint32 zoomMask = (currentZoom << 27) & 0x78000000;
      const uint32 nbrItemsWithZoom =
         theMap->getNbrItemsWithZoom(currentZoom);
      
      for (uint32 i=0; i<nbrItemsWithZoom; i++) {
         Item* item = theMap->itemLookup(zoomMask | i);
         if ( item != NULL) {
            switch (item->getItemType() ) {
               case (ItemTypes::streetSegmentItem) :
               case (ItemTypes::ferryItem) : 
               case (ItemTypes::busRouteItem) : {
                  RouteableItem* ri = static_cast<RouteableItem*> (item);
                  for (int nodeNbr = 0; nodeNbr < 2; ++nodeNbr) {
                     mc2dbg8 << "[RMG]: Insert RouteableItem (0x" 
                             << hex << item->getID()  << ")" 
                             << dec << endl;
                     Node* currentNode = ri->getNode(nodeNbr);
                     if ( nodeNbr == 0 ) {
                        // Node zeros in original vector.
                        nodes.addLast(currentNode->getNodeID() );
                     } else {
                        // Add node ones to other vector.
                        otherNodes.push_back(currentNode->getNodeID() );
                     }
                     totNbrConnections += currentNode->
                        getNbrConnections();
                  }
               }
               break;
               default : {
                  mc2dbg4<< "Skipped itemType = " 
                         << uint32(item->getItemType()) << endl;
                  
               }
               break;
            }
         }
      }
   }

   // These nodes will be sorted.   
   for( vector<uint32>::const_iterator it = otherNodes.begin();
        it != otherNodes.end();
        ++it ) {
      nodes.push_back(*it);
   }

   MC2_ASSERT( is_sorted(nodes.begin(), nodes.end() ) );
               
   // Sort the nodes
   // std::sort(nodes.begin(), nodes.end());
   //nodes.sort();
   mc2dbg << "[RMG]: Nodes added" << endl;
   return totNbrConnections;
}

void
RouteMapGenerator::createTempNodes( TmpRouteNodeVector& outNodes,
                                    const Vector& ids,
                                    const GenericMap& theMap )
{
   mc2dbg << "[RMG]: Creating temp nodes" << endl;
   outNodes.resize( ids.size() );
   MC2Coordinate coord;
   for ( int i = 0, n = ids.size(); i < n; ++i ) {
      // The coordinates (lat, lon)
      theMap.getNodeCoordinates(ids[i], coord.lat, coord.lon );
      const Node* curNode = theMap.nodeLookup( ids[i] );
      uint32 rest = curNode->getEntryRestrictions();
      // Now set the node.
      outNodes[i] = TmpRouteNode( ids[i],
                                  coord,
                                  rest );
   }
   outNodes.sort();
   mc2dbg << "[RMG]: Temp nodes done" << endl;   
}

inline TmpRouteConnData
RouteMapGenerator::getTempConnData( const Connection& conn,
                                    const Node& node,
                                    const GenericMap& theMap )
{
   uint8 stackBuf[64];
   DataBuffer tmpConnDataBuf(stackBuf, sizeof(stackBuf) );
   writeConnectionData( &tmpConnDataBuf,
                        const_cast<Connection*>(&conn),
                        &node,
                        &theMap );
   tmpConnDataBuf.reset();
   return TmpRouteConnData( tmpConnDataBuf );
}

/**
 *   Connection sorter that can sort using from node index
 *   or to-node index.
 */
template<bool FROM> class TmpConnSorter {
public:
   // Needed for the find_if in getConnectionDataBuffer
   typedef bool result_type;
   typedef TmpRouteConnection second_argument_type;
   typedef uint32 first_argument_type;

   TmpConnSorter( const TmpRouteNodeVector& rv ) : m_rv(rv) {}

   /**
    *   Returns from node for forward conns, to node for backward
    *   conns.
    */
   uint32 getNodeIdx( const TmpRouteConnection& conn ) const {
      return conn.getNodeIdx( FROM, m_rv );
   }

   /**
    *   Returns to node for forward conns, from node for backward conns.
    */
   uint32 getOtherNodeIdx( const TmpRouteConnection& conn ) const {
      return conn.getNodeIdx( ! FROM, m_rv );
   }
   
   bool operator()(const TmpRouteConnection& a,
                   const TmpRouteConnection& b ) const {
      return a.getNodeIdx(FROM, m_rv) < b.getNodeIdx(FROM, m_rv);
   }
   
   bool operator()( uint32 a,
                    const TmpRouteConnection& b ) const {
      return m_rv.getIdxFromID( a ) < b.getNodeIdx(FROM, m_rv);
   }
   
   bool operator()( const TmpRouteConnection& a,
                    uint32 b ) const {
      return a.getNodeIdx(FROM, m_rv) < m_rv.getIdxFromID( b );
   }
   
   
private:
   const TmpRouteNodeVector& m_rv;
};

/**
 *    Connection sorter that sorts using the from-node index.
 */
struct TmpForwardConnSorter : public TmpConnSorter<true> {

   TmpForwardConnSorter( const TmpRouteNodeVector& rv ) :
         TmpConnSorter<true>( rv ) {}
   
};

/**
 *    Connection sorter that sorts using the to-node index.
 */
struct TmpBackwardConnSorter : public TmpConnSorter<false> {

   TmpBackwardConnSorter( const TmpRouteNodeVector& rv ) :
         TmpConnSorter<false>( rv ) {}   
};

void
RouteMapGenerator::createTmpConns( vector<TmpRouteConnection>& conns,
                                   vector<TmpRouteConnection>& backConns,
                                   vector<TmpRouteConnData>& connDatas,
                                   int totNbrConnections,
                                   const TmpRouteNodeVector& nodes,
                                   const GenericMap& theMap )
{
   mc2dbg << "[RMG]: Start createTmpConns" << endl;

   // Set the sizes
   conns.resize( totNbrConnections );
   connDatas.resize( totNbrConnections );
   int nbrAdded = 0;
   
   for ( vector<TmpRouteNode>::const_iterator it = nodes.begin();
         it != nodes.end();
         ++it ) {
      const Node* curNode = theMap.nodeLookup( it->m_nodeID );
      int nbrConnections = curNode->getNbrConnections();
      for ( int i = 0; i < nbrConnections; ++i ) {
         Connection* curConn = curNode->getEntryConnection( i );

         // Connection data
         connDatas[nbrAdded] = getTempConnData( *curConn, *curNode, theMap );
         // And the connection.
         conns[nbrAdded] = TmpRouteConnection( curConn->getConnectFromNode(),
                                               it->m_nodeID,
                                               &connDatas[nbrAdded] );
         ++nbrAdded;
      }
   }
   MC2_ASSERT( uint32(nbrAdded) == connDatas.size() );
   MC2_ASSERT( connDatas.size() == conns.size() );

   mc2dbg << "[RMG]: Copying connections" << endl;
   backConns = conns;
   mc2dbg << "[RMG]: Done copying forw -> back conns" << endl;
   mc2dbg << "[RMG]: Done createTmpConns" << endl;

   mc2dbg << "[RMG]: Sorting connections " << endl;

   // Forward connections need sorting.
   std::sort( conns.begin(), conns.end(), TmpForwardConnSorter( nodes ) );

   if ( is_sorted( backConns.begin(),
                   backConns.end(),
                   TmpBackwardConnSorter( nodes ) ) ) {
      mc2dbg << "[RMG]: Backward conns already sorted" << endl;
   } else {
      std::sort( backConns.begin(), backConns.end(),
                 TmpBackwardConnSorter( nodes ) );
   }
   mc2dbg << "[RMG]: Done sorting connections " << endl;
}


DataBuffer*
RouteMapGenerator::getNodeDataBuffer(const TmpRouteNodeVector& nodes,
                                     const GenericMap* theMap)
{   
   // Create a DataBuffer for the nodes and fill it with data.
   uint32 nbrNodes = nodes.size();
   DataBuffer* nodeBuffer = new DataBuffer(8 + 16 * nbrNodes); 
   nodeBuffer->writeNextLong(0);    // Nbr nodes
   nodeBuffer->writeNextLong(0);    // Total length of data in bytes

   for (uint32 i=0; i<nbrNodes; i++) {
      nodeBuffer->writeNextLong( nodes[i].m_nodeID );
      nodeBuffer->writeNextLong( nodes[i].m_rest );
      nodeBuffer->writeNextLong( nodes[i].m_coord.lat );
      nodeBuffer->writeNextLong( nodes[i].m_coord.lon );
   }
   
   // Now I think the nodebuffer is done. We should send it here then.
   // Send the nodes
   mc2dbg << "[RMG]: " << nodes.size()
          << " nodes written to DataBuffer" << endl;
   
   return nodeBuffer;
}

/**
 *   Template functio that can write forward or backward connections
 *   and write complete connectiondata or only index.
 */
template<class SORTER, class CONNDATAWRITER> DataBuffer*
getConnectionsBuf( const TmpRouteNodeVector& nodes,
                   const vector<TmpRouteConnection>& conns,
                   const vector<TmpRouteConnData>& connDatas,
                   const SORTER& sorter,
                   const CONNDATAWRITER& connDataWriter )
{
   MC2_ASSERT( is_sorted( conns.begin(), conns.end(), sorter ) );
   mc2dbg << "[RMG]: Creating connection buffer" << endl;
   
   int nbrWrittenConnections = 0;
   DataBuffer* buf = new DataBuffer(8 +
                                    // Nbr connections for each node.
                                    4 * nodes.size() +
                                    // From or tonode.
                                    4 * conns.size() +
                                    connDataWriter.entrySize()
                                    * connDatas.size());

   // To be filled in by the sender function.
   buf->writeNextLong(0);    // Nbr connections
   buf->writeNextLong(0);    // Total length of connections
   
   typedef vector<TmpRouteConnection>::const_iterator conn_it_t;
   
   conn_it_t last_end = conns.begin();
   
   for( TmpRouteNodeVector::const_iterator it = nodes.begin();
        it != nodes.end();
        ++it ) {
      
      // Get the next connection with a different id.
      conn_it_t new_end = find_if( last_end, conns.end(),
                                   bind1st( sorter,
                                            it->m_nodeID ) );
      // This is the range to iterate over.
      pair<conn_it_t,conn_it_t> bend ( last_end, new_end );
      // Save the new end.
      last_end = new_end;

      // Number of connections
      buf->writeNextLong( distance( bend.first, bend.second ) );

#if 0
      MC2_ASSERT( bend == std::equal_range( backConns.begin(),
                                            backConns.end(),
                                            it->m_nodeID,
                                            sorter ) );
#endif
      // For all connections ( for this node )
      for ( conn_it_t ct = bend.first;
            ct != bend.second;
            ++ct ) {
         // Node index - to or from depending on sorter.
         uint32 nodeIdx = sorter.getOtherNodeIdx( *ct );
         buf->writeNextLong( nodeIdx );
         ++nbrWrittenConnections;

         // Connection data
         TmpRouteConnData* connData = ct->m_connData;

         connDataWriter.writeConnData( *buf, *connData );         
      }            
   }
   
   MC2_ASSERT( uint32(nbrWrittenConnections) == conns.size() );
   
   mc2dbg << "[RMG]: " << nbrWrittenConnections << " conns written" << endl;
   
   return buf;
}

/**
 *   Writes only the connection data index.
 *   Used when sending the other set of connections.
 */
class ConnDataIdxWriter {
public:
   static uint32 entrySize() {
      return 4;
   }

   static void writeConnData( DataBuffer& buf,
                              TmpRouteConnData& connData ) {
      buf.writeNextLong( connData.m_index );
   }
   
};


DataBuffer*
RouteMapGenerator::
getRestOfConnectionsBuf( const TmpRouteNodeVector& nodes,
                         const vector<TmpRouteConnection>& conns,
                         const vector<TmpRouteConnection>& backConns,
                         const vector<TmpRouteConnData>& connDatas,
                         bool send_forward )
{
   if ( send_forward ) {
      return getConnectionsBuf( nodes, conns, connDatas,
                                TmpForwardConnSorter(nodes),
                                ConnDataIdxWriter() );
   } else {
      return getConnectionsBuf( nodes, backConns, connDatas,
                                TmpBackwardConnSorter(nodes),
                                ConnDataIdxWriter() );
   }
}

/**
 *   Writes the complete connection data.
 *   Used when sending the first connections, the ones
 *   with connection data.
 */
class ConnDataWriter {
public:
   static uint32 entrySize() {
      return 5*4;
   }

   static void writeConnData( DataBuffer& buf,
                              TmpRouteConnData& connData ) {
      buf.writeNextLong(connData.m_costA);
      buf.writeNextLong(connData.m_costB);
      buf.writeNextLong(connData.m_costC);
      buf.writeNextLong(connData.m_costD);
      buf.writeNextLong(connData.m_vehicleRest );
   }
   
};

DataBuffer*
RouteMapGenerator::
getConnectionDataBuffer(const TmpRouteNodeVector& nodes,
                        const vector<TmpRouteConnection>& conns,
                        const vector<TmpRouteConnection>& backConns,
                        vector<TmpRouteConnData>& connDatas,
                        bool send_forward )
{
   if ( send_forward ) {
      // Update indeces
      for ( uint32 i = 0; i < conns.size(); ++i ) {
         conns[i].m_connData->m_index = i;
      }
      return getConnectionsBuf( nodes, conns, connDatas,
                                TmpForwardConnSorter(nodes),
                                ConnDataWriter() );
   } else {
      // Update indices
      for ( uint32 i = 0; i < backConns.size(); ++i ) {
         backConns[i].m_connData->m_index = i;
      }
      return getConnectionsBuf( nodes, backConns, connDatas,
                                TmpBackwardConnSorter(nodes),
                                ConnDataWriter() );
   }   
}

DataBuffer*
RouteMapGenerator::getExternalNodeBuffer(Vector& sendNodes,
                                         int& nbrConnections,
                                         const GenericMap* theMap)
{
   BoundrySegmentsVector* boundryVect = theMap->getBoundrySegments();
   // Don't add stuff if it is an country map.
   if ( boundryVect != NULL && !MapBits::isCountryMap(theMap->getMapID()) ) {
      // Guess a max-size for the vector.
      sendNodes.setAllocSize( boundryVect->getSize() * 2 );
      
      // This is where the external nodes are added.
      const uint32 nbrExtNodes = boundryVect->getSize();
      mc2dbg << "[RMG]: Number of boundrysegments = " << nbrExtNodes
             << endl;
      for (uint32 i=0; i < nbrExtNodes; ++i ) {
         BoundrySegment* curBS =
            (BoundrySegment*) boundryVect->getElementAt(i);            
         if (curBS != NULL) {
            // Maybe add connections for node 0
            nbrConnections += checkAndAddBoundrySegment(0, theMap, curBS,
                                                        sendNodes);
            // Maybe add connections for node 1
            nbrConnections += checkAndAddBoundrySegment(1, theMap, curBS,
                                                        sendNodes);
         }
      }
      
   }

   mc2dbg << "[RMG]: Number of external nodes = " << sendNodes.size() << endl;
   mc2dbg << "[RMG]: Sorting external nodes" << endl;
   sendNodes.sort();
   mc2dbg << "[RMG]: Vector with nodes with external connections "
          << "sorted, nbr=" << sendNodes.size() << endl;

   // Create the buffer.
   DataBuffer* externalNodesBuffer =
      new DataBuffer(8 + sendNodes.size() * 8);
   externalNodesBuffer->writeNextLong(0);
   externalNodesBuffer->writeNextLong(0);
   uint32 totNbrExternalNodes = sendNodes.size();
   for (uint32 i=0; i < totNbrExternalNodes; i++) {
      externalNodesBuffer->writeNextLong(sendNodes[i]);
      // Send the vehicle restrictions for the external connections
      // They are always NO_RESTRICTIONS
      externalNodesBuffer->writeNextLong(0);
   }
   mc2dbg4 << "All nodes with external connections saved in buffer" 
           << endl;
   
   return externalNodesBuffer;
}

DataBuffer*
RouteMapGenerator::getExternalConnectionBuffer(const Vector& sendNodes,
                                               int&totNbrExtConns,
                                               const GenericMap* theMap)
{
   BoundrySegmentsVector* boundryVect = theMap->getBoundrySegments();   
   // The connections to the external nodes
   DataBuffer* externalConBuffer = new DataBuffer(8 +
                                                  totNbrExtConns*28 + 
                                                  sendNodes.getSize()*4);
   
   externalConBuffer->writeNextLong(0);
   externalConBuffer->writeNextLong(0);

   int totNbrSentConns = 0;
   const int nbrNodes = sendNodes.size();
   for( int i=0; i < nbrNodes; i++) {
      const uint32 tmpID = sendNodes[i];
      BoundrySegment* curBS =
         boundryVect->getBoundrySegment(tmpID & 0x7fffffff);
      
      if (curBS != NULL) {
         
         uint32 nodeNbr = MAX_UINT32;
         if ( (tmpID & 0x80000000) != 0) {
            // Node 1, but forward connections so add conn from 0,
            // i.e. turn the connection around.
            nodeNbr = 0;
         } else {
            nodeNbr = 1;
         }
         const uint32 nbrCon = curBS->getNbrConnectionsToNode(nodeNbr);
         MC2_ASSERT( nbrCon > 0 ); // Should not be added otherwise
         externalConBuffer->writeNextLong(nbrCon);
         
         totNbrSentConns += nbrCon;
         for (uint32 j=0; j<nbrCon; j++) {
            const uint32 otherMapID =
               curBS->getFromMapIDToNode(nodeNbr, j);
            
            if ( otherMapID == theMap->getMapID() ) {
               mc2dbg << "[RMG]: Connection to same map" << endl;
               abort();
            }
            
            externalConBuffer->writeNextLong( otherMapID );
            
            externalConBuffer->writeNextLong(
               curBS->getConnectToNode(nodeNbr, j)->getConnectFromNode()
               ^ 0x80000000 );
            // Write the fake connection data.
            // Will probably not be used in the route module
            // (except for reading).
            // Cost A-D
            writeConnectionData(externalConBuffer, 0, 0, 0, 0,
                                // Vehiclerest
                                0xffffffff);
         } 
      } else {
         mc2dbg4 << "curBS == NULL, tmpID == " << tmpID << endl;
      }
   }
   // Just check
   MC2_ASSERT ( totNbrSentConns == totNbrExtConns );

   totNbrExtConns = totNbrSentConns;
   
   return externalConBuffer;
}

bool
RouteMapGenerator::sendMap( const GenericMap* theMap,
                            Writeable* socket )
{
   // New! 2002-05-30
   // Start by sending the ID:s of the maps on different
   // levels
   
   if ( ! sendMapHierarchy(theMap, socket) ) {
      // How do I stop this thing?
      mc2log << error << "[RMG]: Failed to send map hierarch info" << endl;
      return false;
   }
   
   if ( ! sendNodeExpansionTable(theMap, socket) ) {
      // How do I stop this thing?
      mc2log << error 
             << "[RMG]: Failed to send node expansion table" << endl;
      return false;
   }

   {
      // Create a vector with all the RouteableItemID:s
      Vector nodeVector(theMap->getNbrItems());
      
      // Get the nodes and put them in the vector ( also counts the number of
      // connections).
      uint32 totNbrConnections = addNodesAndCountConnections(nodeVector,
                                                             theMap);

      TmpRouteNodeVector tmpNodes;
      
      createTempNodes( tmpNodes,
                       nodeVector,
                       *theMap );
                             
      
      {
         // Write the nodes to a DataBuffer.
         DataBuffer* nodeBuffer = getNodeDataBuffer(tmpNodes, theMap);
         
         const uint32 nbrNodes = nodeVector.size();
         if (sendBuffer(nodeBuffer, nbrNodes, socket, "Nodes") < 0) {
            mc2log << error
                   << "RouteMapGenerator:: Error sending nodes" << endl;
            delete nodeBuffer;
            return false;
         }
         
         delete nodeBuffer;
         nodeBuffer = NULL;
      }
      {

         vector<TmpRouteConnection> tmpConns;
         vector<TmpRouteConnection> tmpBackConns;
         vector<TmpRouteConnData> tmpConnDatas;
         createTmpConns( tmpConns, tmpBackConns,
                         tmpConnDatas, totNbrConnections,
                         tmpNodes, *theMap );
         
         DataBuffer* connectionBuffer =
            getConnectionDataBuffer(tmpNodes,
                                    tmpConns,
                                    tmpBackConns,
                                    tmpConnDatas,
                                    first_conn_dir );

         
         // Send the connections        
         if (sendBuffer(connectionBuffer, totNbrConnections, socket,
                        "Connections") < 0 ) {
            mc2log << error << "RouteMapGenerator:: Error sending connections"
                   << endl;
            delete connectionBuffer;
            return false;
         }

         delete connectionBuffer;
         connectionBuffer = NULL;

         // Now create the connections using the index of the conndatas.
         DataBuffer* otherConnBuffer =
            getRestOfConnectionsBuf(tmpNodes,
                                    tmpConns,
                                    tmpBackConns,
                                    tmpConnDatas,
                                    second_conn_dir );
         if ( sendBuffer( otherConnBuffer, totNbrConnections, socket,
                          "Other connections") < 0 ) {
            mc2log << error << "RouteMapGenerator:: Error sending opposit"
                   << endl;
            delete connectionBuffer;
            return false;
         }
         
         delete otherConnBuffer;
         
      }
   }
   // "External nodes" (nodes on this map with external connection).     
   Vector extNodes;   
   int nbrExtConns = 0;
   {
      DataBuffer* extNodeBuffer = getExternalNodeBuffer(extNodes,
                                                        nbrExtConns,
                                                        theMap);
      
      
      // Send the external nodes, sorted by nodeID   
      if (sendBuffer(extNodeBuffer,
                     extNodes.size(), socket, "External Nodes") < 0 ) {
         mc2log << error << "[RMG]: Error sending external nodes"
                << endl;
         delete extNodeBuffer;      
         return false;
      }
      
      delete extNodeBuffer;
      extNodeBuffer = NULL;
   }
   {
      // Get the external connections too.
      DataBuffer* extConnBuffer = getExternalConnectionBuffer(extNodes,
                                                              nbrExtConns,
                                                              theMap);
      
      if (sendBuffer(extConnBuffer, 
                     nbrExtConns, socket, "External Connections") < 0 ) {
         mc2log << error << "[RMG]: Error sending external nodes"
                << endl;
         delete extConnBuffer;
         return false;
      }
      delete extConnBuffer;
      extConnBuffer = NULL;
   }
   
   // Send the translation-table if this is an overviewmap!
   const uint32 mapID = theMap->getMapID();
   if (MapBits::isOverviewMap(mapID)) {
      const OverviewMap* overview = dynamic_cast<const OverviewMap*>(theMap);
      if (overview != NULL) {
         
         IDTranslationTable transTable;
         
         // We only want to send the routeable items.
         uint32 overviewID, trueMapID, trueItemID;
         uint32 totNbrOverviewItems = overview->getNbrOverviewItems();
         for (uint32 i=0; i<totNbrOverviewItems; i++) {
            Item* curOverviewItem = overview->getOverviewItem(i, 
                                                              overviewID, 
                                                              trueMapID, 
                                                              trueItemID);
            switch ( curOverviewItem->getItemType() ) {
               case ItemTypes::streetSegmentItem:
               case ItemTypes::ferryItem:
                  transTable.addElement(overviewID, trueMapID, trueItemID);
                  break;
               default:
                  break;
            }
         }
         

         transTable.sortElements();
         
         DataBuffer translationBuffer( transTable.getSizeInDataBuffer() + 4 );
         translationBuffer.writeNextLong( transTable.getSizeInDataBuffer() );

         transTable.save( translationBuffer );

         if ( socket->write( translationBuffer.getBufferAddress(), 
                             translationBuffer.getCurrentOffset() ) < 0)
         {
            mc2log << error << here 
                   << "RouteMapGenerator:: Error sending translationbuffer" 
                   << endl;
         }
      }
   }
   
   return true;
}

int
RouteMapGenerator::sendBuffer( DataBuffer* dataBuffer,
                               uint32 nbrItems,
                               Writeable* socket,
                               const char* infostr )
{
   mc2dbg << "[RMG]: Writing buffer "
          << MC2CITE( infostr ) << endl;
   dataBuffer->writeLong(nbrItems, 0);
   dataBuffer->writeLong(dataBuffer->getCurrentOffset()-8, 4);
   int nbrBytes = socket->write( dataBuffer->getBufferAddress(),
                                 dataBuffer->getCurrentOffset() );

   mc2dbg << "[RMG]: Done writing buffer " << MC2CITE( infostr ) << endl;

   return (nbrBytes);
}

DataBuffer*
RouteMapGenerator::generateDataBuffer(const GenericMap* theMap,
                                      const MapModuleNoticeContainer* indexdb)
{
   indexdb->getMapsAbove( theMap->getMapID(), m_overviewMaps );
   ByteBuffer buf;
   sendMap(theMap, &buf);

   // I know that this is not efficient cpu-wise, but it
   // was very efficient implementationwise and it is no
   // use optimizing for this case.
   // You don't have to visit me and tell me.
   DataBuffer* dataBuffer = new DataBuffer(buf.getNbrBytes());
   buf.read(dataBuffer->getBufferAddress(), buf.getNbrBytes());
   
   return dataBuffer;
}



