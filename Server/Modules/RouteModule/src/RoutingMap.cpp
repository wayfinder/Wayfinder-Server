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

#include "RoutingMap.h"

#include "MapPacket.h"
#include "DisturbanceSubscriptionPacket.h"
#include "DatagramSocket.h"
#include "TCPSocket.h"
#include "Properties.h"
#include "multicast.h"

#include "TimeTableContainer.h"
#include "ModuleMap.h"
#include "RoutingNode.h"
#include "RoutingConnection.h"

#include "DisturbanceStorage.h"

#include "OrigDestNodes.h"
#include "NetUtility.h"
#include "MapBits.h"

#include <algorithm>
#include <iterator>

// This define must be in synch with what the MM sends.
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

class RoutingConnectionAndDataPair {
public:
   RoutingConnection first;
   RoutingConnectionData second;   
};


#define UDP_TIME_OUT 40000000

RoutingMap::RoutingMap( uint32 mapID )
{
   m_timeTable = NULL;
   m_nodeVector         = NULL; // To be able to delete these

   // Future extensions - must fix the OrigDestNodes though
   m_estCosts           = NULL;   
   m_realCosts          = NULL;
   m_gradients          = NULL;
   m_isDests            = NULL;
   m_isVisiteds         = NULL;
   // End future
   m_externalNodeVector = NULL;
   
   m_mapID = mapID;
   
   m_boundingBox       = new MC2BoundingBox;
   m_tempRollBackStack = new DisturbanceStorage(this);
   m_mainRollBackStack = new DisturbanceStorage(this);

   m_expNodeIndexTable = NULL;
   m_fromToNodeTable = NULL; 
   m_nbrExpNodes = 0;
   m_nbrMultiConnections = 0;
   
   m_connections  = NULL;
   m_connDatas    = NULL;
   m_extConns     = NULL;
   m_extConnDatas = NULL;

   m_nbrExternalNodes = 0;
   
   m_curInf = 0;
}


void
RoutingMap::deleteAllMapData()
{
   delete [] m_extConns;
   delete [] m_extConnDatas;
   delete [] m_connDatas;
   delete [] m_connections;

   m_connections = NULL;
   m_connDatas = NULL;
   m_extConns = NULL;
   m_extConnDatas = NULL;

   // Clear the IDTranslationTable
   m_idTranslationTable = IDTranslationTable();
   
   delete [] m_nodeVector;
   m_nodeVector = NULL;
   delete [] m_externalNodeVector;
   m_externalNodeVector = NULL;
   
   delete m_boundingBox;
   m_boundingBox = new MC2BoundingBox;
   
   m_nbrNodes = 0;
   m_nbrExternalNodes = 0;
   
   delete [] m_expNodeIndexTable;
   m_expNodeIndexTable = NULL;
   delete [] m_fromToNodeTable; 
   m_fromToNodeTable = NULL;
   m_nbrExpNodes = 0;
   m_nbrMultiConnections = 0;
      
   delete [] m_estCosts;
   delete [] m_realCosts;
   delete [] m_gradients;
   delete [] m_isDests;
   delete [] m_isVisiteds;

}


RoutingMap::~RoutingMap()
{
   // Set back the old stuff
   mc2dbg << "Removing disturbances ..."  << flush;
   m_tempRollBackStack->rollBackAll();
   m_mainRollBackStack->rollBackAll();
   mc2dbg << " done " << endl;
   // Remove the stacks
   delete m_tempRollBackStack;
   delete m_mainRollBackStack;
   DEBUG4( cerr << "RoutingMap::~RoutingMap" << endl );

   delete [] m_extConns;
   delete [] m_extConnDatas;
   delete [] m_connDatas;
   delete [] m_connections;

   delete [] m_nodeVector;
   delete [] m_externalNodeVector;
   delete m_boundingBox;
   
   delete [] m_expNodeIndexTable;
   delete [] m_fromToNodeTable; 
}

RoutingNode*
RoutingMap::createDestNodes( int nbr )
{
   RoutingNode* res = new RoutingNode[nbr];
   for ( int i = 0; i < nbr; ++i ) {
      res[i].setMuch( MAX_UINT32, MAX_UINT32, MAX_UINT32,
                       MAX_UINT32, MAX_UINT32 );
      res[i].reset(this);
   }
   return res;
}

bool
RoutingMap::load(uint32& mapSize, MapSafeVector* loadedMaps)
{

   char handshake[1024];
   uint32 IP = NetUtility::getLocalIP();
   sprintf(handshake, "RouteModule at %u.%u.%u.%u reporting in", 
           uint32((IP & 0xff000000) >> 24),
           uint32((IP & 0x00ff0000) >> 16), 
           uint32((IP & 0x0000ff00) >>  8), 
           uint32(IP & 0x000000ff));
   
   bool mapLoadedOK = false;
   const uint32 MAX_NBR_TRIES = 2;
   uint32 nbrTries = 0;

   while ( (nbrTries++ < MAX_NBR_TRIES) && (mapLoadedOK == false) ) {
      Readable* tcpSocket =
         ModuleMap::getMapLoadingReadable(m_mapID,
                                          MapRequestPacket::MAPREQUEST_ROUTE,
                                          handshake,
                                          0,
                                          loadedMaps);

      if ( tcpSocket == NULL ) {
         mc2log << warn
                << here << " could't get socket for maploading" << endl;
         return false; // Won't retry - already done.
      }
   
      mapLoadedOK = loadMapFromSocket( tcpSocket, mapSize );
      
      if (!mapLoadedOK) {
         mc2log << warn << here << " error while loading map number "
                << m_mapID << " - retrying " << endl;
         delete tcpSocket;
         continue;
      } else if( mapLoadedOK &&
                 MapBits::isOverviewMap(m_mapID) ) {
         mc2dbg << "[RM]: Will read IDTranslationTable" << endl;
         DataBuffer buffSize(4);
         if(4 ==
            tcpSocket->read( buffSize.getBufferAddress(), 4) ) {
            int32 nbrBytes = buffSize.readNextLong();
            
            DataBuffer* buff = new DataBuffer( nbrBytes );
            
            if(nbrBytes == tcpSocket->
               read(buff->getBufferAddress(), nbrBytes)) {
               createHigherLevelTable( buff );
               mapSize += nbrBytes;
            } else {
               mapLoadedOK = false;
               delete buff;
               delete tcpSocket;
               mc2log << warn << "tcpSocket->read failed (1)" << endl;
               continue;
            }
            delete buff;
         } else {
            mapLoadedOK = false;
            delete tcpSocket;
            mc2log << warn << "tcpSocket->read failed (2)" << endl;
            continue;
         }
      }
      delete tcpSocket;
   }

#if 0
#ifdef DEBUG_LEVEL_1
   if ( mapLoadedOK ) {
      int nbrNoForwConn = 0;
      int nbrNoBackConn = 0;
      int nbrNoThroughFare = 0;
      mc2dbg << "[RMap]: Checking map" << endl;
      // Sanity check
      for( uint32 i = 0; i < m_nbrNodes; ++i ) {
         MC2_ASSERT( i == m_nodeVector[i].getIndex() );
         if ( m_nodeVector[i].getFirstConnection(true) == NULL ) {
            nbrNoForwConn++;
         }
         if ( m_nodeVector[i].getFirstConnection(false) == NULL ) {
            nbrNoBackConn++;
         }
         if ( HAS_NO_THROUGHFARE(m_nodeVector[i].getRestriction()) ) {
            ++nbrNoThroughFare;
         }
      }
      // Sanity check for sorting and dups
      for ( uint32 i = 1; i < m_nbrNodes; ++i ) {
         // Sorting
         MC2_ASSERT( m_nodeVector[i].getItemID() >=
                     m_nodeVector[i-1].getItemID());
         // Dups
         MC2_ASSERT( m_nodeVector[i].getItemID() !=
                     m_nodeVector[i-1].getItemID());
         // Search comp
         MC2_ASSERT( SEARCH_COMP( m_nodeVector[i-1].getItemID(), i) < 0 );
      }

      //      MC2_ASSERT( m_idTranslationTable.sanityCheck() );
      
      mc2dbg << "[RMap]: " << nbrNoForwConn << " nodes w/o forw conn" << endl;
      mc2dbg << "[RMap]: " << nbrNoBackConn << " nodes w/o back conn" << endl;
      mc2dbg << "[RMap]: " << nbrNoThroughFare
             << " nodes with no throughfare" << endl;

      // Count connections
      map<int,int> nbrConnections;
      int nbrOneOne = 0;
      for( uint32 i = 0; i < m_nbrNodes; ++i ) {
         int nbrConn = 0;
         for( RoutingConnection* conn =
                 m_nodeVector[i].getFirstConnection(true);
              conn != NULL;
              conn = conn->getNext()) {
            ++nbrConn;
         }
         ++nbrConnections[nbrConn];
         if ( nbrConn == 1 ) {
            RoutingConnection* firstConn =
               m_nodeVector[i].getFirstConnection(true);
            RoutingNode* nextNode = firstConn->getNode();
            int nbrConnStep2 = 0;
            for( RoutingConnection* conn =
                 nextNode->getFirstConnection(true);
              conn != NULL;
              conn = conn->getNext()) {
               ++nbrConnStep2;
            }
            if ( nbrConnStep2 == 1 ) {
               ++nbrOneOne;
            }
         }
      }      

      int totalNbrConnections = 0;
      for( map<int,int>::const_iterator it = nbrConnections.begin();
           it != nbrConnections.end();
           ++it ) {
         mc2dbg << "[RMap]: Number of nodes with " << it->first
                << " connections: " << it->second << endl;
         totalNbrConnections += it->second;
      }
      mc2dbg << "[RMap]: Total number of nodes " << totalNbrConnections
             << endl;
      mc2dbg << "[RMap]: Number of nodes with one conn to one conn node "
             << nbrOneOne << endl;
   }
#endif
#endif
   return mapLoadedOK;
}


void
RoutingMap::createHigherLevelTable(DataBuffer* buff)
{
   DEBUG4( cerr << "RoutingMap::createHigherLevelTable" << endl);
   mc2dbg << "Reading higher level table" << endl;
   uint32 startTime = TimeUtility::getCurrentTime();

   m_idTranslationTable.load( *buff, MAX_UINT32 );
   
   uint32 stopTime = TimeUtility::getCurrentTime();
   mc2dbg << "Done in " << (stopTime - startTime) <<endl;   
}

bool
RoutingMap::readMapHierarchy(Readable* socket)
{
   uint32 nbrDataItems = 0;
   uint32 dataSize = 0;
   if( !setBuffertData( socket, nbrDataItems, dataSize ) ) {
      mc2dbg << "[RMap]: No map hier" << endl;
      return false;
   }

   // Create buffer and fill it with data.
   DataBuffer buf(dataSize);
   uint32 size = socket->read( buf.getBufferAddress(), dataSize);
   if ( size != dataSize ) {
      mc2dbg << "[RMap]: Could not read buffer data for map hier" << endl;
      return false;
   }
   
   for( uint32 i = 0; i < nbrDataItems; ++i ) {
      uint32 level = buf.readNextLong();
      uint32 mapID = buf.readNextLong();
      mc2dbg << "[RM]: Level " << level << ", mapID " << prettyMapIDFill(mapID)
             << endl;
      // Add the levels to our internal storage.
      m_levels[level] = mapID;
   }
   return true;   
}
   
bool
RoutingMap::readNodeExpansionTable(Readable* socket)
{
   uint32 dataSize = 0;
   if( !setBuffertData( socket, m_nbrExpNodes, dataSize ) ) {
      mc2dbg << "[RMap]: No node expansion table" << endl;
      return false;
   }

   // Create buffer and fill it with data.
   DataBuffer buf(dataSize, "readNodeExpansionTable");
   // Checker
   DataBufferChecker dbc( buf, "readNodeExpansionTable" );
   uint32 size = socket->read( buf.getBufferAddress(), dataSize);
   if ( size != dataSize ) {
      mc2dbg << "[RMap]: Could not read buffer data for "
             << "node expansion table" << endl;
      return false;
   }

  
   
   // Allocate the expanded node table.
   m_expNodeIndexTable = new nodeIndex_t[ m_nbrExpNodes ];
   
   // Note that the expanded node ids are already sorted.
   for( uint32 i = 0; i < m_nbrExpNodes; ++i ) {
      // Expanded node
      m_expNodeIndexTable[ i ].first = buf.readNextLong();
      // Associated index
      m_expNodeIndexTable[ i ].second = buf.readNextLong();      
      // Add to exp node -> index table.
      mc2dbg8 << "[RM]: m_exp[" << i << "] = 0x"
              << hex << m_expNodeIndexTable[ i ].first << ", " << dec
              << m_expNodeIndexTable[ i ].second  << endl;
   }

   MC2_ASSERT( is_sorted( m_expNodeIndexTable,             
                          m_expNodeIndexTable + m_nbrExpNodes ) );
   
   m_nbrMultiConnections = buf.readNextLong();
   m_fromToNodeTable = new fromToNode_t[ m_nbrMultiConnections ];
   for ( uint32 i = 0; i < m_nbrMultiConnections; ++i ) {
      // From node
      m_fromToNodeTable[ i ].first = buf.readNextLong();
      // To node
      m_fromToNodeTable[ i ].second = buf.readNextLong();
      // Add to index -> to node, from node table
      mc2dbg8 << "[RM]: m_fromto[" << i << "] = 0x"
              << hex << m_fromToNodeTable[ i ].first << ", 0x"
              << m_fromToNodeTable[ i ].second << dec << endl;
   }
   
   mc2dbg << "[RM]:  m_nbrExpNodes = " << m_nbrExpNodes
          << ", m_nbrMultiConnections" << m_nbrMultiConnections
          << endl;
   
   // Check buffer
   dbc.assertPosition( dataSize );
  
   return true;   
}

bool
RoutingMap::loadMapFromSocket( Readable* tcpSocket,
                               uint32& mapSize)
{
   mapSize = 1; // At least
   uint32 calcMapSize = 0; // We will add to this one
   DEBUG4( cerr << "RoutingMap::loadMapFromSocket" << endl );

   // Variables used to store the data (nbr items and size)
   // about the current buffer 
   uint32 nbrDataItems;
   uint32 dataSize;

   // Start by reading the map hierarchy.
   if ( ! readMapHierarchy(tcpSocket) ) {
      mc2log << warn << "[RMap]: Could not read map hierarchy" << endl;
      return false;
   }
   
   // Read node expansion table.
   if ( ! readNodeExpansionTable(tcpSocket) ) {
      mc2log << warn << "[RMap]: Could not read node expansion table" 
             << endl;
      return false;
   }
   
   // First read the nodes
   if( !setBuffertData( tcpSocket, nbrDataItems, dataSize ) ) {
      MC2WARNING("nbrDataItems and dataSize is zero!!");
      return false;
   }

   DataBuffer* buff = new DataBuffer(dataSize);
   uint32 size = 0;
   if((size=tcpSocket->read(buff->getBufferAddress(),dataSize) ) != dataSize) {
      MC2WARNING2("RoutingMap:: Error reading nodes",
                  cerr << "   dataSize=" << dataSize << ", size=" 
                       << size << endl; );
      delete buff;
      return false;
   }
   mc2dbg << "   " << size << " bytes read for nodes" << endl;

   calcMapSize += size;
   
   bool nodeBufferOK = processNodeData(buff, nbrDataItems);
   delete buff;
   if( ! nodeBufferOK ){
      deleteAllMapData();
      mc2log << error << "Map not loaded OK, nodeBuffer not OK" << endl;
      return false;
   }
   
   // Then read all the connections
   if ( ! setBuffertData( tcpSocket, nbrDataItems, dataSize ) ) {
      MC2WARNING("nbrDataItems and dataSize is zero");
   }
   buff = new DataBuffer(dataSize);
   if ( (size=tcpSocket->read(buff->getBufferAddress(), dataSize)) 
        != dataSize) {
      MC2WARNING2("RoutingMap:: Error reading connections",
                  cerr << "   dataSize=" << dataSize << ", size=" 
                       << size << endl;);
      delete buff;
      deleteAllMapData();
      return (false);
   }
   mc2dbg << "   " << size << " bytes read for connections" << endl;

   calcMapSize += size;
   
   bool connectionBufferOK = processConnectionData(buff, nbrDataItems);
   delete buff;
   if (!connectionBufferOK) {
      MC2ERROR("Map not loaded OK, connectionBuffer not OK");
      deleteAllMapData();
      return false;
   }

   {
      // Process other connections
      if (!setBuffertData(tcpSocket, nbrDataItems, dataSize)) {
         mc2dbg << warn << "[RM]: Error when reading other conns" << endl;
         deleteAllMapData();
         return false;
      }
      DataBuffer buf( dataSize );
      if ( (size=tcpSocket->read(buf.getBufferAddress(), dataSize)) 
           != dataSize) {
         mc2log << error << "[RM]: Error reading opposit connections"
                << endl;
         deleteAllMapData();
         return false;
      }
      // OK - now we can start reading
      processOppositConnectionData( buf, nbrDataItems );
   }
      // Read external nodes
   if (!setBuffertData(tcpSocket, nbrDataItems, dataSize)) {
      MC2WARNING("nbrDataItems and dataSize is zero");
   }
   if (dataSize > 0) {
      buff = new DataBuffer(dataSize);
      if ( (size=tcpSocket->read(buff->getBufferAddress(), dataSize)) 
           != dataSize) {
         MC2WARNING2("RoutingMap:: Error reading external nodes",
                     cerr << "   dataSize=" << dataSize << ", size=" 
                          << size << endl;);
         delete buff;
         deleteAllMapData();
         return (false);
      }
      mc2dbg << "   " << size
             << " bytes read for external nodes" << endl;

      calcMapSize += size;
      
      bool externalNodeBufferOK = processExternalNodeData( buff,
                                                           nbrDataItems );
      delete buff;
      if (!externalNodeBufferOK) {
         MC2ERROR("Map not loaded OK, externalNodeBuffer not OK");
         deleteAllMapData();
         return false;
      }
   }
   else {
      MC2INFO("No external nodes in this map");
      m_externalNodeVector = NULL;
      m_nbrExternalNodes = 0;
   }
   
   // Read external connection
   if (!setBuffertData(tcpSocket, nbrDataItems, dataSize)) {
      MC2WARNING("nbrDataItems and dataSize is zero");
   }
   if( dataSize > 0 ) {
      buff = new DataBuffer(dataSize);
      if((size=tcpSocket->read(buff->getBufferAddress(), dataSize)) 
           != dataSize) {
         MC2WARNING2("RoutingMap:: Error reading external connections",
                     cerr << "   dataSize=" << dataSize << ", size=" 
                          << size << endl;);
         delete buff;
         deleteAllMapData();
         return (false);
      }
      mc2dbg << "   " << size << " bytes read for external connections"
                     << endl;

      calcMapSize += size;
      
      bool externalConnectionBufferOK =
         processExternalConnectionData(buff,
                                       nbrDataItems);      delete buff;
      if (!externalConnectionBufferOK) {
         MC2ERROR("Map not loaded OK, externalConnectionBuffer not OK");
         deleteAllMapData();
         return false;
      }

      // OK. Update the latitudes and longitudes for the external
      // nodes. We need them now when overviewmaps can have external
      // connections and the LevelTransitObject makes edgenode requests.
      const int num = getNumExternalNodes();
      mc2dbg << "[RMap]: Updating latlons for " << num
             << " external nodes" << endl;

      RoutingNode* externalNode = getExternalNodes();
      for(int i=0; i < num; ++i ) {
         RoutingNode* realNode =
            getNodeFromTrueNodeNumber(externalNode[i].getItemID() );

         MC2_ASSERT(realNode != NULL);
         
         externalNode[i].setLat(realNode->getLat());
         externalNode[i].setLong(realNode->getLong());
      }
      mc2dbg << "[RMap]: Extlatlons DONE" << endl;
   } else {
      mc2dbg << "No external connections in this map" << endl;
   }
#ifdef DEBUG_LEVEL_1
   mc2dbg << "This map has connections to the following maps: {" << hex;
   copy(m_neighbours.begin(),
        m_neighbours.end(),
        ostream_iterator<uint32>(mc2dbg, " "));
   mc2dbg << "}" << dec << endl;
#endif
   
   mapSize = calcMapSize;
   
   return true;
} // loadMapFromSocket

bool
RoutingMap::processNodeData( DataBuffer* buff, uint32 nbrNodes )
{
   mc2dbg4 << "RoutingMap::processNodeData" << endl;

   m_nbrNodes = nbrNodes;
   mc2dbg << "Number of nodes: " << m_nbrNodes << endl;
   m_nodeVector = new RoutingNode[m_nbrNodes];
//     m_estCosts   = new uint32[m_nbrNodes];
//     m_realCosts  = new uint32[m_nbrNodes];
//     m_gradients =  new RoutingNode*[m_nbrNodes];
//     m_isDests    = new uint8[m_nbrNodes];
//     m_isVisiteds = new uint8[m_nbrNodes];

   
   if (buff->getBufferSize() != nbrNodes * 16) {
      MC2WARNING("Nodesbuffer is corrupt!");
      return false;
   }
      
   mc2dbg8 << "Reading nodes...." << flush;
   // First read the real node numbers

   for (uint32 i=0; i<m_nbrNodes; i++) {
      const uint32 itemID = buff->readNextLongAligned();
      const uint32 rest   = buff->readNextLongAligned();
      const int32 lat = buff->readNextLongAligned();      
      const int32 lon = buff->readNextLongAligned();
      m_nodeVector[i].setMuch( itemID, rest, i, lat, lon );
      m_boundingBox->update( lat, lon, false );      
   }  
   mc2dbg8 << "done" << endl;

   m_boundingBox->updateCosLat();
   return true;
}

inline RoutingConnection*
RoutingMap::getNextConnection()
{
   MC2_ASSERT(m_nextFreeConnIndex < m_nbrAllocatedConnections);

   const int maxIdx = m_nbrAllocatedConnections / 2;
   
   if ( m_nextFreeConnIndex < maxIdx ) {
      // A bit ugly. We know that the backward connections are
      // read first so we return one of the lonely connections
#ifdef FORWARD_CONNECTIONS_FIRST
      RoutingConnection* conn =
         &(m_connDatas[m_nextFreeConnIndex].first);      
#else
      RoutingConnection* conn = &m_connections[m_nextFreeConnIndex];
#endif
      ++m_nextFreeConnIndex;
      return conn;
   } else {
      // Here we return the connections that are paired with the
      // connection datas.
      uint32 idx = m_nextFreeConnIndex - maxIdx;
      ++m_nextFreeConnIndex;
#ifdef FORWARD_CONNECTIONS_FIRST
      RoutingConnection* conn = &m_connections[idx];
#else
      RoutingConnection* conn =
         &(m_connDatas[idx].first);
#endif
      return conn;
   }
}

inline RoutingConnectionData*
RoutingMap::getNextConnectionData()
{
   MC2_ASSERT(m_nextFreeConnDataIndex < m_nbrAllocatedConnDatas);
   return &(m_connDatas[m_nextFreeConnDataIndex++].second);
}


void
RoutingMap::allocateConnections(int nbrConnections)
{
   // Double the amount of connections are needed since
   // there are connections in the backward and forward
   // directions.
   m_nbrAllocatedConnections = nbrConnections << 1;
   m_nextFreeConnIndex = 0;
   m_connections = new RoutingConnection[m_nbrAllocatedConnections >> 1];
   // But two connections share the same data.
   m_nbrAllocatedConnDatas = nbrConnections;
   m_nextFreeConnDataIndex = 0;
   m_connDatas = new RoutingConnectionAndDataPair[m_nbrAllocatedConnDatas];
   mc2dbg << "m_connDatas = " << hex << m_connDatas << dec << endl;
   mc2dbg << "RM:RMap allocated " << nbrConnections << " connections" << endl;
}

bool
RoutingMap::processOppositConnectionData( DataBuffer& buf,
                                          int nbrConnections)
{
#if 1
   int nbrAdded = 0;
   for (uint32 i=0; i < m_nbrNodes; ++i) {
      
      RoutingNode& addToNode = m_nodeVector[i];
      
      const uint32 numConns = buf.readNextLong();
      for ( uint32 j = 0; j < numConns; ++j ) {
         // Read the node index.
         const uint32 connNodeIndex = buf.readNextLongAligned();

         // Get node and connection
         RoutingNode* connNode = getNode( connNodeIndex );         
         RoutingConnection* tmpRC = getNextConnection();
         
         // Read the conndata index and get the conndata
         const uint32 connDataIdx = buf.readNextLongAligned();
         RoutingConnectionData* tmpRCD = &m_connDatas[ connDataIdx ].second;
         
         tmpRC->initConnection( connNode, tmpRCD );
         // second_conn_dir depends on FORWARD_CONNECTIONS_FIRST
         addToNode.addConnection( tmpRC, second_conn_dir );
         ++nbrAdded;
      }
   }
   
   MC2_ASSERT( nbrAdded == nbrConnections );
      
#else
   updateConnections();   
#endif

#if 0
   mc2dbg << "[RMap]: Asserting connections ok" << endl;
   // Check the connections
   for( uint32 i = 0; i < m_nbrNodes; ++i ) {
      RoutingConnection* tempConn =
         m_nodeVector[i].getFirstConnection( false ); // backward
      // For all connections of this node
      while ( tempConn ) {
         bool found = false;
         // For all connections of the other node.
         for ( RoutingConnection* otherConn =
                  tempConn->getNode()->getFirstConnection( true ); // forw
               otherConn != NULL;
               otherConn = otherConn->getNext() ) {
            // Make sure that there is a connection the other way
            // and with the same conndata
            if ( otherConn->getNode() == getNode(i) ) {
               mc2dbg8 << "[RMap]: diff "
                       << labs( uintptr_t(otherConn)  -
                                uintptr_t(otherConn->getData() ) ) << endl;
               MC2_ASSERT( otherConn->getData() == tempConn->getData() );
               found = true;
            }            
         }
         MC2_ASSERT( found );
         tempConn = tempConn->getNext();
      }
   }
   mc2dbg << "[RMap]: Survived connection check" << endl;
#endif
   return true;
}

bool
RoutingMap::processConnectionData(DataBuffer* buff,
                                  int nbrConnections)
{

   allocateConnections(nbrConnections);

   int totNbrConnections = 0;
   mc2dbg4 << "RoutingMap::processConnectionData" << endl;

   // Now read the connections
   mc2dbg << "Reading connections...." << flush;
   for (uint32 i=0; i < m_nbrNodes; ++i) {
      
      const uint32 numConnections = buff->readNextLong();
      if (numConnections > 45) { // XXX Something less hardcoded perhaps?
         MC2WARNING("Node has more than 45 connections");
         DEBUG1(
            mc2dbg1 << "   m_nbrNodes=" << m_nbrNodes << ", i=" << i << endl;
            uint32 pos = (buff->getCurrentOffset()) / 4;
            mc2dbg1 << "   pos = " << pos << endl;
            uint32* buffP = (uint32*) (buff->getBufferAddress());
            uint32 k = 0;
            while ((pos >= 0) && (k < 45)) {
               mc2dbg1 << "dataBuffer->buff[" << pos << "] = " 
                       << buffP[pos] << endl;
               k++;
               pos--;
           }
         );
         
         return false;
      }
      
      for( uint32 j=0; j < numConnections; j++){
         ++totNbrConnections;
         const uint32 connectionIndex = buff->readNextLongAligned();

         // Get the connections from the already allocated vector.
         RoutingConnectionData* tmpRCD = getNextConnectionData();
         tmpRCD->readFromBuffer( buff );
         RoutingConnection* tmpRC = getNextConnection();

         RoutingNode* connNode = getNode(connectionIndex);
         MC2_ASSERT( connNode != NULL );
         
         tmpRC->initConnection( connNode, tmpRCD);
         // first_conn_dir depends on FORWARD_CONNECTIONS_FIRST
         m_nodeVector[i].addConnection( tmpRC, first_conn_dir );
      }
   }
   mc2dbg << " connections done" << endl;
   mc2dbg << "Total nbr connections " << totNbrConnections << endl;
   // updateConnections();

   return true;
}


bool
RoutingMap::processExternalNodeData(DataBuffer* buff, uint32 nbrNodes)
{
   mc2dbg4 << "RoutingMap::processExternalNodeData" << endl;

   // nbr of external nodes
   m_nbrExternalNodes = nbrNodes;
   
   if (buff->getBufferSize() != (8 * nbrNodes)) {
      MC2WARNING("Externalnodesbuffer corrupt!");
      return false;
   }

   m_externalNodeVector = new RoutingNode[m_nbrExternalNodes];
    
   mc2dbg << "Reading "
          << m_nbrExternalNodes << "external nodes....";
   
   for (uint32 i=0; i < m_nbrExternalNodes; i++) {
      uint32 itemID = buff->readNextLong();
      uint32 rest   = buff->readNextLong();
      // Lat/lon set later
      m_externalNodeVector[i].setMuch( itemID, rest, i, 0, 0 );
   }
   mc2dbg << "done" << endl;
   
   return true;
}

void
RoutingMap::allocateExternalConnections(int nbrConnections)
{
   // Don't Double the amount of connections are needed since
   // there are connections in the backward and forward
   // directions.
   // Seems that we allocate double the amount needed anyway.
   m_nbrAllocatedExtConns = nbrConnections;
   m_nextFreeExtConnIndex = 0;
   m_extConns = new ExternalRoutingConnection[m_nbrAllocatedExtConns];
   // But two connections share the same data.
   m_nbrAllocatedExtConnDatas = nbrConnections << 1;
   m_nextFreeExtConnDataIndex = 0;
   m_extConnDatas = new RoutingConnectionData[m_nbrAllocatedExtConnDatas];
   mc2dbg << "RM:RMap allocated " << m_nbrAllocatedExtConnDatas
          << " extconnections" << endl;
}

ExternalRoutingConnection*
RoutingMap::getNextExtConnection()
{
   MC2_ASSERT(m_nextFreeExtConnIndex < m_nbrAllocatedExtConns);
   return &m_extConns[m_nextFreeExtConnIndex++];
}

RoutingConnectionData*
RoutingMap::getNextExtConnectionData()
{
   MC2_ASSERT(m_nextFreeExtConnDataIndex < m_nbrAllocatedExtConnDatas);
   return &m_extConnDatas[m_nextFreeExtConnDataIndex++];
}

bool
RoutingMap::processExternalConnectionData(DataBuffer* buff,
                                          int nbrConnections)
{   
   mc2dbg << "RoutingMap::processExternalConnectionData" << endl;
   mc2dbg << "Number of external connections : " << nbrConnections << endl;

   allocateExternalConnections(nbrConnections);

   uint32 externalMapID;
   uint32 externalItemID;
   uint32 numConnections;
   RoutingConnectionData* tmpRCD;

   for (uint32 i=0; i < m_nbrExternalNodes; i++) {
      
      numConnections = buff->readNextLong();
      if (numConnections > 45) { // XXX Perhaps not hardcoding later
         MC2WARNING("External node has more than 45 connections!");
         return false;
      }
      
      for( uint32 j=0; j<numConnections; j++){
         externalMapID  = buff->readNextLong();
         // Add the mapid to the set of neighbours
         m_neighbours.insert(externalMapID);
         externalItemID = buff->readNextLong();

         tmpRCD = getNextExtConnectionData();
         tmpRCD->readFromBuffer( buff );

         ExternalRoutingConnection* erc = getNextExtConnection();
         erc->initConnection( externalMapID, externalItemID, tmpRCD);
         mc2dbg4 << "[RMap]: Connection from "
                 << hex << m_externalNodeVector[i].getItemID() << dec
                 << " to " << externalMapID << ":" << hex
                 << externalItemID << dec << " will be added now" << endl;
         m_externalNodeVector[i].addConnection(erc, true);
      }
   }
   DEBUG8(cerr << "done" << endl);

   mc2dbg << "Number of external connections read "
          << m_nextFreeExtConnIndex << endl;

   return true;
}


void RoutingMap::dumpExternal()
{
   cout << "===========EXTERNAL NODES============"<< endl;
   for(uint32 i=0;i<m_nbrExternalNodes;i++){
      m_externalNodeVector[i].dump();
      cout << "------------" << endl;
   }
}


void RoutingMap::dump(){
   cout << "===========NODES============"<< endl;
   for( uint32 i = 0; i<m_nbrNodes;i++ ){
      m_nodeVector[i].dump();
   }
}


void
RoutingMap::dumpAllRouteCosts(ofstream& outfile)
{
   for (uint32 i = 0; i < m_nbrNodes; i++) {
      RoutingNode* node = getNode(i);
      outfile << "Node " << node->getItemID() << "  cost " <<
         node->getRealCost(this) << endl;
   }
}


void RoutingMap::updateConnections()
{
   DEBUG4( cerr << "RoutingMap::updateConnections" << endl );

   // Updating connection
   for( uint32 i=0; i<m_nbrNodes; i++){
      RoutingConnection* tempConn =
         m_nodeVector[i].getFirstConnection(false); // backward 

//       xox : Loop over the backward connections directly instead
//             The nodes are only used since there was no connection
//             array before....
      
      while( tempConn != NULL ) {
         // FIXME: Won't allocate the connections and data
         // at the right positions, because the nodes are in
         // backward connection order.
         RoutingConnection* tmpRC = getNextConnection();
         
         tmpRC->initConnection( getNode(i), tempConn->getData() );
         tempConn->getNode()->addConnection( tmpRC,
                                             second_conn_dir );
         mc2dbg8 << "Second - first "
                 << (uintptr_t(tmpRC) - uintptr_t(tmpRC->getData()))
                 << endl;
         tempConn = tempConn->getNext();
      }
   }
}


void RoutingMap::reset()
{
   ++m_curInf;
   if ( m_curInf == 1 ) {
      for( uint32 i = 0; i < m_nbrNodes; ++i) {
         m_nodeVector[i].reset(this);
      }
   }
}


bool
RoutingMap::setBuffertData(Readable* socket, 
                           uint32 &nbrDataItems, 
                           uint32 &dataSize)
{
   DataBuffer buffData(8);
   if( 8 == socket->read( (byte*)buffData.getBufferAddress(), 8 ) ){
      nbrDataItems = buffData.readNextLong();
      dataSize     = buffData.readNextLong();

      DEBUG1( mc2dbg << "   "
              << "nbrDataItems = " << nbrDataItems << ", dataSize = "
              << dataSize << endl );

      return true;
   } 
   else {
      nbrDataItems = 0;
      dataSize = 0;
      MC2WARNING( "RoutingMap::setBuffertData socket->read failed" );
      return false;
   }
}

uint32
RoutingMap::getNextDepartureTime( uint16 lineID,
                                  uint32 nodeID,
                                  uint32 time )
{
   /// XXX Doesn't work yet..
   MC2WARNING("RoutingMap::getNextDepartureTime doesn't work");
   TimeTable* timeTable = m_timeTable->getTimeTable( lineID );
   if( timeTable != NULL ){
      return timeTable->getNextTime( time );
   }
   return MAX_UINT16;
}

void
RoutingMap::changeConnectionCosts( RoutingNode* node,
                                   RoutingConnection* conn,
                                   DisturbanceStorage* rollBackStack,
                                   uint32 costA,
                                   uint32 costB,
                                   uint32 costC,
                                   uint32 vehicleRestrictions)
{
   rollBackStack->changeConnectionCosts( node,
                                         conn,
                                         costA,
                                         costB,
                                         costC,
                                         vehicleRestrictions );
}

void
RoutingMap::changeConnectionCosts( RoutingNode* node,
                                   RoutingConnection* conn,
                                   bool temporary,
                                   uint32 costA,
                                   uint32 costB,
                                   uint32 costC,
                                   uint32 vehicleRestrictions)
{
   DisturbanceStorage* rollBackStack = m_tempRollBackStack;
   if ( ! temporary ) {
      // Traffic cost module?
      rollBackStack = m_mainRollBackStack;
   }
   changeConnectionCosts(node, conn, rollBackStack, costA,
                         costB, costC, vehicleRestrictions);
}

uint32
RoutingMap::changeNodeCost( RoutingNode* node,
                            float factor,
                            DisturbanceStorage* rollBackStack)
{
   // This should not be here when testing is done.
   if ( rollBackStack == NULL )
      rollBackStack = m_tempRollBackStack;
   
   // Just guessing here...
   const float maxAllowedCost = float(MAX_UINT32 / 256);
   // Now we know what nodes to change.
   // We only have to change the RoutingConnectionData in one direction,
   // e.g. forward, because that will change the data in the other
   // direction too.
   
   // Loop over all connections from this node.
   RoutingConnection* curConn = node->getFirstConnection( true );
   mc2dbg << "curConn = " << hex << curConn << dec << endl;
   while ( curConn != NULL ) {
      // Get the connection data
      RoutingConnectionData* connData = curConn->getData();
      float costA = float(connData->getCostA(0)) * factor;
      float costB = float(connData->getCostB(0)) * factor;
      float costC = float(connData->getCostC(0)) * factor;
      costA = MIN(costA, maxAllowedCost);
      costB = MIN(costB, maxAllowedCost);
      costC = MIN(costC, maxAllowedCost);

      changeConnectionCosts(node,
                            curConn,
                            rollBackStack,
                            uint32(costA),
                            uint32(costB),
                            uint32(costC),
                            connData->getVehicleRestriction(false));
      
      curConn = curConn->getNext();
   }
   return StringTable::OK;   
}

uint32
RoutingMap::multiplyNodeCost( RoutingNode* node,
                              uint32 infoModuleFactor )

{
   DisturbanceStorage* rollBackMap = m_mainRollBackStack;
   
   // We know which nodes to change.
   // We only have to change the RoutingConnectionData in one direction,
   // e.g. forward, because that will change the data in the other
   // direction too.
   
   // Loop over all connections from this node.
   RoutingConnection* curConn = node->getFirstConnection( true );
   mc2dbg << "[RM]: cnc - curConn = " << hex << curConn << dec << endl;
   while ( curConn != NULL ) {
      // Get the connection data
      RoutingConnectionData* connData = curConn->getData();

      // Restore the old connection
      rollBackMap->rollBackOne(node->getItemID(),
                               curConn->getNode()->getItemID());

      if ( infoModuleFactor != MAX_UINT32 ) {
         double factor = double(infoModuleFactor) / 1000.0;
         changeConnectionCosts(node,
                               curConn,
                               rollBackMap,
                               connData->getCostA(0),
                               connData->getCostB(0),
                               uint32(connData->getCostC(0) * factor),
                               connData->getVehicleRestriction(false));
      } else {
         changeConnectionCosts(node,
                               curConn,
                               rollBackMap,
                               connData->getCostA(0),
                               connData->getCostB(0),
                               MAX_UINT32,
                               connData->getVehicleRestriction(false));
      }
      
      curConn = curConn->getNext();
   }
   return StringTable::OK;
}

uint32
RoutingMap::restoreConnections( RoutingNode* node )
{
   DisturbanceStorage* rollBackMap = m_mainRollBackStack;
   
   // Loop over all connections from the node.
   RoutingConnection* curConn = node->getFirstConnection( true );
   mc2dbg << "[RM]: res - curConn = "
          << hex << curConn << dec << endl;
   while ( curConn != NULL ) {
      // Restore the old connection
      rollBackMap->rollBackOne(node->getItemID(),
                               curConn->getNode()->getItemID());
                               
      curConn = curConn->getNext();
   }

   return StringTable::OK;
}

uint32
RoutingMap::changeNodeCost(uint32 nodeID,
                           float factor,
                           bool temporary)
{
   RoutingNode* node = getNodeFromTrueNodeNumber(nodeID);
   if ( node == NULL ) {
      mc2dbg << "Disturbed node not found in map" << endl;
      return StringTable::NOT;
   }

   if ( temporary ) {
      return changeNodeCost(node, factor, m_tempRollBackStack);
   } else {
      return changeNodeCost(node, factor, m_mainRollBackStack);
   }
}


uint32
RoutingMap::avoidNode( uint32 nodeID,
                       bool temporary )
{
   mc2dbg8 << here << " avoiding segment " << hex << nodeID << dec << endl;
   DisturbanceStorage* rollBackStack = m_tempRollBackStack;
   if ( ! temporary ) {
      // Traffic cost module?
      rollBackStack = m_mainRollBackStack;
   }
      
   // First, get the nodes
   RoutingNode* node = getNodeFromTrueNodeNumber( nodeID );
   
   if ( node == NULL ) {
      // Couldn't find node
      mc2log << warn << " Node not found when adding disturbance" << endl;
      return StringTable::NOT;
   }

   rollBackStack->avoidNode( node );
   
   return StringTable::OK;
}


int
RoutingMap::rollBack(bool temporary)
{
   mc2dbg << here << " - ready to roll out - temp = " <<
      temporary << endl;
   if ( temporary ) {
      return m_tempRollBackStack->rollBackAll();
   } else {
      return m_mainRollBackStack->rollBackAll();
   }
}

int
RoutingMap::addDisturbances(const DisturbanceVector* distVect,
                            bool temporary)
{
   if ( temporary ) {
      return m_tempRollBackStack->addDisturbances( distVect );
   } else {
      return m_mainRollBackStack->addDisturbances( distVect );
   }
}


RoutingNode* RoutingMap::getNodeFromTrueNodeNumber( uint32 node )
{
   uint32 index = binarySearch(node);

   if( index != MAX_UINT32 ) {
      return getNode(index);
   } else {
      // Not found.
      return NULL;
   }
}


OrigDestNode*
RoutingMap::newOrigDestNode(uint32 index,
                            uint32 mapID,
                            uint16 offset,
                            uint32 lat,
                            uint32 lon,
                            uint32 cost,
                            uint32 estCost,
                            uint32 costASum,
                            uint32 costBSum,
                            uint32 costCSum)
{
   OrigDestNode* node = new OrigDestNode(index, mapID, offset, lat, lon,
                                         cost, estCost, costASum, costBSum,
                                         costCSum);
   node->setInfinity(getInfinity());
   return node;
}

OrigDestNode*
RoutingMap::newOrigDestNode( const IDPair_t& id,
                             uint16 offset )
{
   // Note the strange order of item and mapid
   OrigDestNode* node = new OrigDestNode(id.getItemID(),
                                         id.getMapID(),
                                         offset);
   node->setInfinity(getInfinity());
   return node;
}
