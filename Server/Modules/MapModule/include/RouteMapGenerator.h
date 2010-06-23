/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTEMAPGENERATOR_H
#define ROUTEMAPGENERATOR_H

#include "config.h"

#include<vector>
#include "Vector.h"

#include "MapGenerator.h"

class Node;
class Connection;
class MapHandler;
class GenericMap;
class BoundrySegment;
class DataBuffer;

class TmpRouteNode;
class TmpRouteNodeVector;
class TmpRouteConnection;
class TmpRouteConnData;

/**
  *   Objects of this class extracts an route-map and sends it via
  *   socket to the route-module. After the map is send (or the time
  *   out time is reached) the process (thread) ends and the object 
  *   is deleted by the runtime JTC system.
  *
  */
class RouteMapGenerator : public MapGenerator {
   public:
      /**
        *   Creates one RouteMapGenerator.
        *
        *   @param   mh The MapHandler to use to retreiving the Map:s
        *   @param   mapID Pointer to the mapID that the searchmap 
        *            should be generated for.
        *   @param   port The port to listen to connection on.
        *   @param   overview The ID:s of the overview maps. (Should be in
        *            the maps in the future).
        */
      RouteMapGenerator(MapHandler *mh,
                        uint32 *mapIDs,
                        uint32 nbrMaps,
                        uint32 port,
                        const vector<uint32>& overview);

      /**
        *   Delete this RouteMapGenerator.
        */
      virtual ~RouteMapGenerator();

      /**
        *   Performes the actual generation of the map.
        *
        *   XXX:  Include the file with the external file-information
        *         (.../MC2/Server/docs/grammars.tex).
        */
      virtual void run();

      /**
       *    Returns true.
       */
      bool getIndexDBNeeded() const;
   
      /**
       *   Generates the DataBuffer that would be sent to the
       *   SearchModule.
       *   @param map The GenericMap to generate the SearchMap from.
       *   @return New DataBuffer containing the map that would be sent
       *           to the SearchModule.
       */
      DataBuffer* generateDataBuffer(const GenericMap* theMap,
                                     const MapModuleNoticeContainer* indexdb);
   
   private:

      /**
       *   Creates temporary route nodes for the id:s sent in.
       */
      static void createTempNodes( TmpRouteNodeVector& outNodes,
                                   const Vector& ids,
                                   const GenericMap& theMap );
                            
      /**
       *   Creates the temporary connections and conndatas.
       */
      static void createTmpConns( vector<TmpRouteConnection>& conns,
                                  vector<TmpRouteConnection>& backCons,
                                  vector<TmpRouteConnData>& connDatas,
                                  int totalNbrConnections,
                                  const TmpRouteNodeVector& nodes,
                                  const GenericMap& theMap );

      /**
       *   Sets the temporary conndata.
       */
      static TmpRouteConnData getTempConnData( const Connection& conn,
                                               const Node& tonode,
                                               const GenericMap& theMap );
      
      /**
       *   Sends the map on the socket.
       *   @return false if failure.
       */
      bool sendMap(const GenericMap* theMap, Writeable* sock);
      
      /**
       *    Writes the connection data to the databuffer.
       *    @param conBuffer   Buffer to write the data to.
       *    @param costA       Distance cost.
       *    @param costB       Time cost.
       *    @param costC       Time cost with disturbances.
       *    @param costD       Unused.
       *    @param vehicleRest Vehicle restrictions.
       */
      inline static void writeConnectionData(DataBuffer* conBuffer,
                                             uint32 costA,
                                             uint32 costB,
                                             uint32 costC,
                                             uint32 costD,
                                             uint32 vehicleRest);
      
      /**
        *   Writes the data needed for the routing to a databuffer.
        *
        *   @param   conBuffer   Pointer to the Databuffer where to store
        *                        the data.
        *   @param   con         The connection to save into conBuffer.
        *   @param   toNode      The node which the connection is leading
        *                        to.
        *   @param   theMap      The map where con is located.
        *   @param   externalConnection   If true that means that the
        *                                 specified connection is an
        *                                 external connection, otherwise
        *                                 it is assumed to be an ordinary
        *                                 connection. Defaults to false.
        */
      inline static void writeConnectionData(DataBuffer* conBuffer, 
                                             Connection* conn,
                                             const Node* toNode,
                                             const GenericMap* theMap,
                                             bool externalConnection = false);

      /**
       *   Checks if a boundry segment should be added to the
       *   RoutingMap as an external node.
       *   @param nodeNbr   Node number 0 or 1.
       *   @param theMap    The current map.
       *   @param curBS     The current boundry segment.
       *   @param sendNodes Vector to add nodeIDs to.
       *   @return The number of connections that will be added.
       */
      inline static int checkAndAddBoundrySegment(int nodeNbr,
                                                  const GenericMap* theMap,
                                                  BoundrySegment* curBS,
                                                  Vector& sendNodes);
                                       

      /**
        *   Write the data in the databuffer into the socket.
        *   Also writes the number of items and total size of data-8
        *   in the first 8 bytes.
        *   @param info Info to display or null.
        */
      int sendBuffer( DataBuffer* dataBuffer, 
                      uint32 nbrItems, 
                      Writeable* socket,
                      const char* infostr = NULL );

      /**
       *    Sends information about the map hierarchy to the RouteModule.
       *    @packetdesc
       *    @row 0 @sep 4 bytes @sep The number of levels from this level
       *                             and up = <i>n</i> @endrow.
       *    @row 8*[0..n[ + 4 @sep 4 bytes @sep level number @endrow
       *    @row 8*[0..n[ + 8 @sep 4 bytes @sep Map id for level @endrow
       *    @endpacketdesc
       *    @param theMap The current map.
       *    @param socket The socket to send the data on.
       *    @return True if the data could be sent.
       */
      bool sendMapHierarchy( const GenericMap* theMap,
                             Writeable* socket);

      /**
       *    Sends information about the map hierarchy to the RouteModule.
       *    @packetdesc
       *    @row 0 @sep 4 bytes @sep The number of expanded nodes 
       *                             = <i>n</i> @endrow.
       *    @row 8*[0..n[ + 4 @sep 4 bytes @sep Expanded node id @endrow
       *    @row 8*[0..n[ + 8 @sep 4 bytes @sep Multi conn index @endrow
       *    @row 8*n + 4 @sep 4 bytes @sep The number of multi connections 
       *                             = <i>m</i> @endrow.
       *    @row 8*[n + 1] + 8*[0..m[     @sep 4 bytes @sep From node id 
       *                                  @endrow
       *    @row 8*[n + 1] + 8*[0..m[ + 4 @sep 4 bytes @sep To node id
       *                                  @endrow
       *    @endpacketdesc
       *
       *    @param theMap  The current map.
       *    @param socket  The socket to send the data on.
       *    @return True if the data could be sent.
       */
      bool sendNodeExpansionTable( const GenericMap* theMap,
                                   Writeable* socket );

      /**
       *    Goes through the map and puts the id:s of the
       *    nodes that should be added to the map in the Vector
       *    called nodes. Also sorts the vector.
       *    @param nodes  Vector to put nodeids into.
       *    @param theMap Generic map to get nodes from.
       *    @return The number of connections to allocate.
       */
      static int addNodesAndCountConnections(Vector& nodes,
                                             const GenericMap* theMap);


      /**
       *    Creates a DataBuffer containing the nodes.
       *    @param nodes Vector of nodeID:s to send to the RM.
       *    @param theMap GenericMap for lookups etc.
       */
      static DataBuffer* getNodeDataBuffer(const TmpRouteNodeVector& nodes,
                                           const GenericMap* theMap);


      /**
       *    Creates connections and costs from the nodes in the nodeVector.
       *    @param nodes The vector of nodes.
       *    @param conns     Forward connections.
       *    @param backConns Backward connections.
       *    @param connDatas Connection datas. The indeces will be updated.
       *    @param send_forward If true, the forward connections will be
       *                        send and the indeces of the connectiondatas
       *                        will reflect the forward connections.
       */
      static DataBuffer*
         getConnectionDataBuffer(const TmpRouteNodeVector& nodes,
                                 const vector<TmpRouteConnection>& conns,
                                 const vector<TmpRouteConnection>& backConns,
                                 vector<TmpRouteConnData>& connDatas,
                                 bool send_forward);
      
      /**
       *    Creates a DataBuffer containing connections and connection
       *    datas represented by only the index.
       *    @param nodeVector The vector of nodeID:s to send to the RM.
       *    @param calcNbrConnections The calculated number of connections.
       *    @param theMap The GenericMap that the nodes come from.
       */
      static DataBuffer*
         getRestOfConnectionsBuf(const TmpRouteNodeVector& nodes,
                                 const vector<TmpRouteConnection>& conns,
                                 const vector<TmpRouteConnection>& backConns,
                                 const vector<TmpRouteConnData>& connDatas,
                                 bool send_forward);

      /**
       *    Creates a DataBuffer containing the nodes with external
       *    connections. The vector of nodes will be sorted.
       *    @param nodes Vector of nodeID:s to send to the RM.
       *    @param nbrConnections Number of connections to allocate room
       *                          for.
       *    @param theMap GenericMap for lookups etc.
       *    @return DataBuffer containing the node-id:s. To be deleted
       *            by the caller.
       */
      static DataBuffer* getExternalNodeBuffer(Vector& sendNodes,
                                               int& nbrConnections,
                                               const GenericMap* theMap);
      
      /**
       *    Creates the databuffer containing the external connections.
       *    @param sendNodes Sorted vector of id:s of the external nodes.
       *    @param totNbrExtConns The number of connections to allocate room
       *                          for in the DataBuffer.
       *    @param theMap         GenericMap containing the nodes.
       *    @return DataBuffer containing the external connections to be
       *            deleted by the caller.
       */
      static DataBuffer* getExternalConnectionBuffer(const Vector& sendNodes,
                                                     int& totNbrExtConns,
                                                     const GenericMap* theMap);
                                                 
    
      /** Vector of overview maps */
      vector<uint32> m_overviewMaps;
};

#endif

