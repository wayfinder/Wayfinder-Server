/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTESENDER_H
#define ROUTESENDER_H

#include "config.h"

#include <list>
#include <set>
#include <map>

#include "StringTable.h"
#include "PacketContainerTree.h"
#include "SubRouteContainer.h"
#include "IDPairVector.h"

class SubRouteVector;        // forward decl
class ServerSubRouteVector;  // forward decl
class ServerSubRouteVectorVector; // forward decl
class DriverPref;            // forward decl
class OrigDestInfoList;      // forward decl
class SubRouteRequestPacket; // forward decl
class SubRouteReplyPacket;   // forward decl
class PacketContainer;       // forward decl
class Request;               // forward decl
class LevelTransitObject;    // forward decl
class DisturbanceList;       // forward decl
class DisturbanceVector;     
class MapPairVector;         // forward decl
class SmallestRoutingCostTable; // forward decl.
class RoutingInfo;           // forward decl
class RequestUserData;


/**
 *    This class administrates direct routing (non-via) questions.
 *    Each such question is divided into request packets for a single map.
 *    The packet is sent to the RouteModule, and the reply packets are
 *    handled and administrated to create new request packets until
 *    the route is completed.
 *
 */
class RouteSender{
public:

   /**
    *    Standard constructor, taking origins and destinations
    *    to create a question. Furthermore, if the route is a via route,
    *    and this part of the route is not the last, 'allDestInfoList'
    *    is a list that contains the destinations in the total question
    *    from user. Finally 'pref' is the object containing information
    *    about the user preferences.
    *    @param user            The request user which will be used for
    *                           user rights.
    *    @param request         Request to get packetID:s from.
    *    @param origInfoList    A list of the origins.
    *    @param destInfoList    List of all destinations.
    *    @param allDestInfoList List of all final destinations. If NULL
    *                           destInfoList is used instead and only
    *                           one route to the cheapest destination is
    *                           calculated if not nbrBestDests is set.
    *    @param pref            The driver preferences.
    *    @param routingInfo     RoutingInfo about the map levels etc.
    *    @param disturbances    The disturbances that should be added to
    *                           the SubRoutePackets.
    *    @param nbrBestDests    The number of destinations to calculate
    *                           routes to. If MAX_UINT32 allDestInfoList
    *                           will decide.
    *    @param level           The current routing level. Should only
    *                           be used internally or be 0.
    *    @param externalCutOff  Used internally to avoid too much routing
    *                           on higher level.
    *    @param calcCostSums    Internally used. Will be true if nbrBestDests
    *                           != MAX_UINT32. Means that the sum of cost
    *                           a-c should be calculated and returned by the
    *                           RM. (for distsort).
    *    @param dontSendSubRoutes Means that the nodes in the subroutes should
    *                             not be sent to save network bandwith. Used
    *                             internally. Will be true if nbrBestDests ==
    *                             MAX_UINT32. (For distsort).
    *    @param lowerLevelDestMaps Destination maps on low level.
    *    @param allowedMaps The allowed maps to route on, NULL means all.
    */
   RouteSender( const RequestUserData& user,
                Request* request,
                const OrigDestInfoList* origInfoList,
                const OrigDestInfoList* destInfoList,
                const OrigDestInfoList* allDestInfoList,
                const DriverPref* pref,
                const RoutingInfo* routingInfo,
                uint32 distFromStartToFinish,
                const DisturbanceList*  disturbances,
                uint32 nbrBestDests = MAX_UINT32,
                int level = 0,
                uint32 externalCutOff = MAX_UINT32,
                bool calcCostSums = false,
                bool dontSendSubRoutes = false,
                set<uint32>* lowerLevelDestMaps = NULL,
                RouteAllowedMap* allowedMaps = NULL );

   /**
    *    Delete this RouteSender, and release allocated memory.
    */
   virtual ~RouteSender();
  
   /**
    *    Handles an incoming answer containing a Packet or a
    *    timeout and rebuilds the outgoing list.
    *    @param p The PacketContainer to handle.
    */
   void processPacket( const PacketContainer* p );

   /**
    *    Returns the top RequestPacket on the outgoing list
    *    (m_outGoingQueue).
    */
   PacketContainer* getNextPacket( );

   /**
    *    Goes through the m_finishedServerSubRouteVector and backtracks
    *    the finished routes. Each of these is a ServerSubRouteVector.
    *    The ServerSubRouteVectorVector returned should be deleted by
    *    the caller when finished. Use getStatus() to find out
    *    if the routing was succesful.
    *
    *    @return  A ServerSubRouteVectorVector containing all the
    *             ServerSubRouteVectors that represent finished routes
    *             to the requested destinations.
    */
   ServerSubRouteVectorVector* getRoute( );

   /**
    *    Calls getRoute and sorts the routes, cheapest first.
    *    @return A sorted ServerSubRouteVectorVector containing all the
    *             ServerSubRouteVectors that represent finished routes
    *             to the requested destinations.
    */
   ServerSubRouteVectorVector* getSortedRoute();
   
   /**
    *    Inserts an origin as destination in a SubRoute to
    *    initialize routing there. Then adds this SubRoute to
    *    m_requestSubRouteContainer.
    *
    *    @param  insertInfo  The origin to start routing from.
    */
   inline void insertOrigDestInfo( const OrigDestInfo& insertInfo );

   /**
    *    Returns true if the routing has finished. Use getStatus()
    *    to find out if the routing was succesful.
    */
   inline bool requestDone( );

   /**
    *    Returns StringTable::OK if routing was successful.
    *    @return StringTable::OK if routing was succesful.
    *            StringTable::ERROR_TIMEOUT if a timeout occured
    *            Status code sent from RouteModule if != OK
    *            LevelTransitObject::getStatus if != OK
    *            StringTable::INTERNAL_SERVER_ERROR on unexpected
    *            state or packet type.
    */
   inline StringTable::stringCode getStatus( ) const;

   inline uint32 getStartIndex() const;
   
private:

   /**
    *    Initialized the allowedMaps-stuff.
    */
   void initAllowedMaps();

   /**
    *    Returns the disturbance vector to use in packets.
    */
   const DisturbanceVector*
      getDistVectorForPackets(uint32 mapID = MAX_UINT32) const;
   
   /**
    *    Returns the current cutoff. This is used for an experiment
    *    where we try to find the real cutoff when routing on higher
    *    level by going to low level and route to the dest and then
    *    continue on higher level where there will probably be no
    *    more routes to route on.
    */
   uint32 getCutOff() const;
   
   /**
    *    Returns the number of routes to return in the answer.
    *    If allDestInfoList is NULL and nbrBestDests is MAX_UINT32
    *    one route will be calculated. If allDestInfoList is != NULL
    *    routes to all destinations will be calculated (for via-routes).
    *    If nbrBestDests is set, that number decides how many routes
    *    should be calced.
    */
   static int calcNbrDests(const OrigDestInfoList* allDestInfoList,
                           const OrigDestInfoList* destInfoList,
                           uint32 nbrBestDests);
   
   /**
    *    The possible states of this object.
    *
    */
   enum state_t { 
      /** 
       *    The state that is entered at the start.
       *    Routing on this level until we either reach dest or
       *    hit the roof for this level routing.
       */
      START_ROUTING,
      
      /**
       *    Before we send a request to m_higherLevelRouteSender,
       *    we need to find out what maps to look for destinations in and
       *    what nodes to use as destinations on the higher level.
       */
      TRANSIT_FINDING,       
      
      /**   
       *    Routing on higher level by using m_higherLevelRouteSender.
       *    While in this state, all packets are just passed on to or
       *    from the child m_higherLevelRouteSender.
       */
      HIGHER_ROUTING,

      /**
       *    Routing from higher level to lower level
       *    in order to get a cutoff or a real route.
       */
      CUTOFF_END_ROUTING,

      /**
       *    More routing on higher level. Hopefully
       *    CUTOFF_END_ROUTING will make all routes on
       *    higher level disappear.
       */
      CUTOFF_HIGHER_ROUTING,
      
      /**   
       *    Routing from higher level destinations down to
       *    detail destination on this level.
       */
      END_ROUTING,
      
      /**
       *    We have finished the routing.
       *    Either finished routing or something is wrong.
       */
      DONE
      
   };

   state_t m_state;

   /**
    *    Returns a static string describing the state.
    *    (For debugging).
    */
   static const char* stateToString( state_t state);
   
   /**
    *    Takes new SubRoutes from m_requestSubRouteContainer
    *    and creates an outgoing packet container from them.
    *    This is then enqueued in m_outgoingQueue, waiting for
    *    the next getNextPacket. Unless Traffic info is needed,
    *    then the packet is created and sent later.
    *    @return  true if a packet is created.
    */
   bool createAndEnqueueSubRouteRequest();

   /**
    *    Creates a SubRouteRequestPacket from m_nextMapAndSubRouteVector
    *    and enqueues it in the outgoing queue.
    */
   bool enqueueSubRouteRequest();
   
   /**
    *    Takes care of the routing to point on higher level,
    *    or to dest if higher level is not needed.
    *    @param srv A SubRouteVector from an earlier reply.
    */
   void processStartRouting( SubRouteVector* srv );

   /**
    *    Checks if there are costs in the table from all the maps in
    *    the reply packet to all the destination maps.
    *    @param  packet     A SubRouteReplyPacket sent from the RM.
    *    @param  neededMaps A vector of maps which need precalculated
    *                       mincosts.
    *    @return A SubRouteVector that can be used in processStartRouting
    *            immediately or later. (No need to decode the packet twice).
    */
   SubRouteVector* checkCostsAvailable( SubRouteReplyPacket* packet,
                                        MapPairVector& neededMaps);

   /**
    *    Processes a SmallestRoutingCostReplyPacket. Probably in
    *    the START_ROUTING state.
    *    @param packet The SmallestRoutingCostReplyPacket.
    *    @return The SubRouteVector from the previously received
    *            SubRouteReplyPacket that caused the request to
    *            send for the SmallestRoutingCostReplyPacket.
    */
   SubRouteVector* processSmallestRoutingCostReply( Packet* packet );

   /**
    *    Returns true if traffic info is needed on the map.
    */
   bool trafficNeeded( uint32 mapID ) const;
   
   /**
    *    Processes a RouteTrafficCostRequestPacket in the
    *    START_ROUTING and END_ROUTING states.
    *    Will enqueue the delayed SubRouteRequestPacket afterwards.
    *    @param packet The RouteTrafficCostReplyPacket.
    */
   void processTrafficReplyPacket( const Packet* packet );

   /**
    *    Sends RouteTrafficCostRequestPacket for the map in question.
    */
   void prepareGettingTraffic( uint32 mapID );
   
   /**
    *    Creates a SmallestRoutingCostRequestPacket and registers
    *    the srv in m_subRoutesWaitingForSmallestCost.
    *    Adds the packet to m_outgoingQueue.
    */
   bool prepareGettingRoutingCosts( MapPairVector& neededMaps,
                                    SubRouteVector* srv);
   
   /**
    *    Communicates the packet to LevelTransitObject.
    *    @param p  A PacketContainer to send to LevelTransitObject
    */
   void processTransitFinding( const PacketContainer* p );

   /**
    *    Communicates the packet to child RouteSender.
    *    @param p  A PacketContainer to send to child RouteSender
    */
   void processHigherRouting( const PacketContainer* p );

   /**
    *    Takes care of the routing from destination on higher level
    *    to dest on this level.
    *    @param packet A SubRouteReplyPacket to rebuild the list
    *                  from.
    */
   void processEndRouting( SubRouteReplyPacket* packet );

   /**
    *    Uses m_routingCostTable to calculate the smallest cost
    *    from the destination of the SubRoute to the nearest of
    *    the destMaps.
    *    @param originMap  The map to use as origin.
    *    @param destMaps   The destination maps to use.
    *    @param driverPref The current driver preferences.
    *    @return The smallest cost for routing from originMap
    *            to one of the destMaps.
    */
   uint32 calcSmallestCost( uint32 originMap,
                            const set<uint32>& destMaps,
                            const DriverPref& driverPref) const;
   
   /**
    *    Uses calcSmallestCost to calculate the smallest cost
    *    from the destination of the SubRoute to the nearest of
    *    the destMaps.
    *    @param subRoute   The subroute to get the origin from.
    *                      (I.e. the dest in the SubRoute).
    *    @param destMaps   The destination maps.
    *    @param driverPref Must multiply with the costs of driverpref.
    *    @return The smallest cost to get from the subRoute to the
    *            cheapest of the destination maps.
    */
   uint32 calcSmallestCost( const SubRoute* subRoute,
                            const set<uint32>& destMaps,
                            const DriverPref& driverPref) const;

   /**
    *    Returns true if the SubRoute leads to a destination.
    */
   bool isDestRoute(const SubRoute* subRoute) const;
   
   /**
    *    Container with SubRoutes yet to keep routing on
    */
   SubRouteContainer m_requestSubRouteContainer;

   /**
    *    Container with SubRoutes yet to keep routing on
    */
   SubRouteContainer m_tooFarSubRouteContainer;   

   /**
    *    Vector with SubRoutes that have been continued on,
    *    or currently in RouteModule.
    */
   ServerSubRouteVector m_finishedServerSubRouteVector;
   
   /**
    *    Preferences from user.
    */
   const DriverPref* m_driverPref;

   /**
    *    List of the nodes the current routing is going to.
    *    Can be via-nodes, separated by RouteObject.
    */
   const OrigDestInfoList* m_destInfoList;

   /**
    *    List of nodes that the total route is going to, past all
    *    via routes. This pointer is equal to m_destInfoList
    *    if the current routing is to the final destinations.
    */
   const OrigDestInfoList* m_allDestInfoList;

   /**
    *    Counter keeping track of how many packets are in RouteModule.
    *    This is to determine when we are finished.
    */
   uint32 m_nbrOutstanding;

   /**
    *    Container for keeping packets on the way to the RouteModule
    */
   PacketContainerTree m_outgoingQueue;

   /**
    *    The current status of the routing
    */
   StringTable::stringCode m_status;

   /**
    *    True if this routing is to the last section of a via route
    *    or if the route is not a via route.
    */
   bool m_isEnd;

   /**
    *    If RouteSebder needs to route on several different levels,
    *    this variable keeps track of what level we are routing
    *    at the moment.
    */
   int m_level;

   /**
    *    The highest possible level.
    */
   int m_maxLevel;
   
   /**
    *    Maps that we need to route on at the end of the route,
    *    for this level
    */
   LevelTransitObject* m_levelTransitObject;
   
   /**
    *    When we go to higher level, we use a child RouteSender
    */
   RouteSender* m_higherLevelRouteSender;
   
   /**
    *    Parameter to constructor of m_higherLevelRouteSender.
    *    Has to be member variable in order to be removed later.
    */
   OrigDestInfoList* m_higherOrigList;
   
   /**
    *    The mother ship of the sender. To get packetid:s from.
    */
   Request* m_request;

   /**
    *    The disturbances sent in from the outside.
    */
   const DisturbanceList* m_disturbances;

   /**
    *    List of disturbances to add to outgoing packets,
    *    i.e. including AURA-disturbances.
    */
   DisturbanceList* m_distListForPackets;

   /**
    *    Eh. Another list of disturbances. This one is the ones
    *    from the InfoModule.
    */
   DisturbanceList* m_infoModuleDists;
   
   /**
    *    True if the SubRoutes returned from the RM should
    *    contain cost sums for costA, costB and costC.
    */
   bool m_calcCostSums;
   
   /**
    *    True if the SubRoutes returned from the RM should not
    *    contain the nodes, just the costs.
    */
   bool m_dontSendSubRoutes;

   /**
    *    Set of IDPair_t:s containing the destinations.
    */
   set<IDPair_t> m_destItems;

   /**
    *    Set of destination maps on this level.
    */
   set<uint32> m_destMaps;

   /**
    *    Set of destination maps on lowest level.
    */
   set<uint32> m_lowerLevelDestMaps;

   /**
    *    Table of smallest routing costs.
    */
   SmallestRoutingCostTable* m_routingCostTable;

   /**
    *    Counter to use when keeping track of packets.
    *    (Currently SmallestRoutingCostPackets).
    */
   uint32 m_packetCounter;

   /**
    *   Map containing the subRoutes that are waiting for 
    *   answers to smallestcost packets.
    *   The key is m_packetCounter that is sent as userDefinedData
    *   in the smallestcost packets and then returned by the module
    *   so that it is possible to find the SubRouteVectors.
    */
   map<uint32, SubRouteVector*> m_subRoutesWaitingForSmallestCost;
      
   /**
    *   Set of map ids of the maps that have we have received
    *   traffic information for.
    */
   set<uint32> m_mapsWithTrafficInfo;

   /**
    *   The next subroutevector to send when traffic info
    *   has arrived
    */
   pair<uint32, SubRouteVector> m_nextMapAndSubRouteVector;
   
   /**
    *   Map containing the number of requests sent to each map.
    *   For debugging and to choose higher level routing if the
    *   destination map has been visited to many times.
    */
   map<uint32, int> m_usedMaps;

   /**
    *   Information about the number of overviewmap levels etc.
    *   Belongs to the RouteObject.
    */
   const RoutingInfo* m_routingInfo;

   /**
    *   The minimum distance from origins to destinations.
    */
   int m_minDistFromOrigToDest;
   
   /**
    *   The minimum number of highlevel nodes that must be in
    *   the m_tooFarSubRouteContainer before going to the higher level.
    */
   int m_minNbrHighLevelNodes;

   /**
    * The allowed maps, may be NULL.
    */
   RouteAllowedMap* m_allowedMaps;

   /// The user
   const RequestUserData& m_user;

   /// True if traffic should be used if costC is set
   bool m_useTrafficIfC;
};

// -----------------------------------------------------------------------
//                                     Implementation of inlined methods -

inline void
RouteSender::insertOrigDestInfo(const OrigDestInfo& insertInfo) {
   m_requestSubRouteContainer.insertOrigDestInfo(insertInfo); 
}

inline bool
RouteSender::requestDone()
{
   if ( ( m_requestSubRouteContainer.getSize() == 0 &&
          m_nbrOutstanding == 0 &&
          m_outgoingQueue.getCardinal() == 0 )
        || m_status != StringTable::OK )
      return true;
   else
      return false;
}

inline StringTable::stringCode
RouteSender::getStatus( ) const{
   return m_status;
}
   
#endif
