/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REAL_TRAFFIC_IPC_H
#define REAL_TRAFFIC_IPC_H

#include "config.h"
#include "TrafficIPC.h"

#include <vector>

class TrafficSituation;
class TrafficSituationElement;
class MapRequester;

/**
 * A class for communicating with other modules and processes.
 */
class RealTrafficIPC: public TrafficIPC {
public:
   /// Vector of MC2Coordinate
   typedef std::vector< MC2Coordinate > MC2Coordinates;

   /// @param moduleCom The module communicator
   explicit RealTrafficIPC( MapRequester& moduleCom );

   virtual ~RealTrafficIPC();

   /// @copydoc TrafficIPC::sendChangeset( newElements, removedElements )
   virtual bool sendChangeset( const Disturbances& newElements,
                               const Disturbances& removedElements );

   /// @copydoc TrafficIPC::sendPacketContainers( pcs )
   virtual bool sendPacketContainers( PacketContainers& pcs );

   /// @copydoc TrafficIPC::getMapIDsNodeIDsCoords( traffSit, idPairsToCoords )
   virtual void getMapIDsNodeIDsCoords( const TrafficSituation& traffSit,
                                        IDPairsToCoords& idPairsToCoords );

   /// @copydoc TrafficIPC::getAllDisturbances(provider, disturbances)
   virtual bool getAllDisturbances( const MC2String& provider,
                                    Disturbances& disturbances );

private:

   /// Vector containing route request pointer.
   typedef std::vector< RouteRequest* > RouteRequests;
   /// Index.
   typedef uint32 Index;
   /// A multi map containing the index for the packet and the packet.
   typedef std::multimap< Index, PacketContainer* > StreetReplies;
   
   typedef std::vector< IDPair_t > IDPairVector_t;
   typedef std::vector< IDPairVector_t > IDPairVectors;
   
   /**
    * Gets the coordinates from a given TrafficSituation.
    * It will first look for TMC locations, if that isn't available it will get
    * the coordniates that the supplier supplied.
    *
    * @param traffSit The traffic situation.
    * @param firstCoords The coordinates for the first location.
    * @param secondCoords The coordinates for the second loaction.
    */
   bool getCoordinates( const TrafficSituation& traffSit, 
                        MC2Coordinates& firstCoords, 
                        MC2Coordinates& secondCoords );

   /**
    * Gets the coordinates from the TMC locations that are in set in the
    * TrafficSituationElement.
    *
    * @param tse The traffic situation element.
    * @param firstCoords The coordinates for the first location.
    * @param secondCoords The coordinates for the second loaction.
    */
   bool getTMCCoordinates( const TrafficSituationElement& tse,
                           MC2Coordinates& firstCoords,
                           MC2Coordinates& secondCoords );

   /**
    * Creates a route request.
    *
    * @param startCoords The start coordinates for the route.
    * @param endCoords The end coordinates for the route.
    * @param rrs The vector to store the route request in.
    */
   void createRouteRequest( const MC2Coordinates& startCoords,
                            const MC2Coordinates& endCoords,
                            RouteRequests& rrs );

   /**
    * Handles the route requests by storing the map ids, street segment ids in
    * a IDPair_t as a key and the coordinates and routeIndex as a value.
    *
    * @param rrs The vector with route request.
    * @param idPairsToCoords Contains the IDPair_t as a key and and the
    *                        coordinate and routeIndex as a value.
    */
   void handleRouteRequests( RouteRequests& rrs,
                             IDPairsToCoords& idPairsToCoords );

   /**
    * Creates street segment request packets for the given coordinates.
    *
    * @param coord The coordinates we want SSI for.
    * @param pcs The packet container vector for the packets.
    */
   virtual void createSSIRequestPackets( const MC2Coordinates& coord,
                                         PacketContainers& pcs );

   /**
    * Handles the street segment reply packets. It checks if the packet is ok
    * or if its not unique. If its not unique it will create additional request
    * packets with same index and the map id that is set for that packet in the
    * reply. And then send those packets and call it self recursevlie.
    * The index relates to the index number for the coordinates that we used to
    * build up the packets.
    *
    * @param ssiPC The packet containers with the reply packets in.
    * @param firstCoords The coordinates that we use for creating the packets.
    * @param StreetReplies The multi map with the creation index as key and the 
    *                      reply packet as a value.
    *                   
    */
   void handleSSIReplies( const PacketContainers& ssiPC,
                          const MC2Coordinates& firstCoords,
                          StreetReplies& streetReplies );

   /**
    * This will get nodeID's on a higer level map and store them if they are
    * valid nodes.
    *
    * @param idPairsToCoords The id pair to coords map for this situation.
    */
   void getHigerLevelNodes( IDPairsToCoords& idPairsToCoords );

   /**
    * Creates a IDTranslationReplyPacket for each unique mapID with all the
    * nodes in that map asking for the nodes for the map above.
    *
    * @param idPairsToCoords The map ids and node ids.
    * @param idTransReqPackets The request packets.
    * @param lowLevelIDPairs The lower level nodes that we want the higher
    *                        level nodes for.
    */
   void createIDTranslationPackets( const IDPairsToCoords& idPairsToCoords,
                                    PacketContainers& idTransReqPackets,
                                    IDPairVectors& lowLevelIDPairs );

   /**
    * Process the the information we get back from the map module.
    * By geting the higher level nodes from the reply packet and checking if
    * they are valid nodes. Id they are use the corresponding low level id
    * pairs to get the coordinates and route index for the new id pair.
    * If we have new id pairs call getHigerLevelNodes again and check for valid
    * nodes on one map level up.
    *
    * @param idPairsToCoords The id pair to coords map for the current
    *                        situation.
    * @param idTransReqPackets The packet containers with the replies in them.
    * @param lowLevelIDPairs The low level id pair vector.
    */
   void getIDTranslationData( IDPairsToCoords& idPairsToCoords,
                              const PacketContainers& idTransReqPackets,
                              const IDPairVectors& lowLevelIDPairs );

   /// The requester for handling communication and such with other processes and
   /// modules.
   MapRequester& m_moduleCom;
};

#endif // REAL_TRAFFIC_IPC_H
