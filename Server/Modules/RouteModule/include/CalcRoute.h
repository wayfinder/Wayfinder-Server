/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CALCROUTE_H
#define CALCROUTE_H

#define USE_RESET_THREAD_IN_CALCROUTE

// Define if the main priority queue should be the
// bucket heap else RedBlackTree (which is stl multimap now).
#define MAIN_PRIO_QUEUE_IN_RM_IS_BUCKET_HEAP

#include "config.h"

#include "RMDriverPref.h"
#include "CCSet.h"
#include "OrigDestNodes.h"
#include "GfxConstants.h"
#include "IDPairVector.h"
#include "Math.h"

class OrigDestInfoList;

#ifdef USE_RESET_THREAD_IN_CALCROUTE
#include "ISABThread.h"
#endif

#include <set>

class MC2BoundingBox;
class BucketHeap;
class RedBlackTree;
class RoutingNode;
class SubRoute;
class SubRouteList;
class RMSubRoute;
class CompleteRMSubRoute;
class RoutingNode;
class RoutingConnection;
class RoutingMap;


/** The type of the cost parameters derived from the DriverPrefs */
typedef uint32 dpCost_t;

/**
 * The brain of the routemodule. Calculates the (sub)route on 1 map:
 * <ul>
 * <li>
 * Between origin/dest points on the map.
 * <li>
 * From origin to all external connections.
 * <li>
 * From all external connections to destinations.
 * </ul>
 *
 */
class CalcRoute
{
   public:
   
      /**
       * Constructs a CalcRoute-object containing the routing map.
       *
       * @param map The map that this CalcRoute object uses.
       */
      CalcRoute(RoutingMap* map);

      /**
       * Destructor that frees the routingmap.
       */
      ~CalcRoute();
   
      /**
       * Routes between all origins with the correct mapID to all
       * destinations with correct mapID.
       * 
       * @param origin          A list of the origin nodes in this map.
       * @param destination     A list of the destination nodes in this
       *                        map.
       * @param allDestinations A list of all destinations (used to
       *                        calculate the estimated distance).
       * @param incomingList    A list of the previous subroutes.
       * @param resultList      A list of all the generated subroutes
       *                        (outparameter).
       * @param driverParam     The driver's preferences.
       * @param originalRequest True if this is the first subroute request
       *                        for a route forward or backward.
       * @param routeToAll      True if SubRoutes to all destinations should
       *                        be calculated.
       * @param disturbances    A list of disturbances to be added to the
       *                        map before calculating the route.
       * @param calcCostSums    Calculate the sums of costs a-c.
       * @param sendSubRoutes   True if the answer should contain nodes.
       *                        If the results should only be sorted by cost
       *                        the subroutes does not have to contain nodes.
       * @return                A status code to tell if everything went
       *                        OK.
       */
      uint32 route(Head* origin, 
                   Head* destination,
                   Head* allDestinations,
                   SubRouteList* incomingList,
                   SubRouteList* resultList,
                   const RMDriverPref* driverParam,
                   bool originalRequest,
                   bool routeToAll,
                   const DisturbanceVector* disturbances,
                   bool calcCostSums,
                   bool sendSubRoutes);

      /**
       *   Returns the external nodes that exist on the specified
       *   level in the supplied vector. The vector should be 
       *   empty when calling the function.
       *   @param level The level to check for 0 is lower level 1 is
       *                the overview map. Later levels 2 and more will
       *                be introduced.
       *   @param externalNodes The result is put in this vector.
       *   @param lats          Vector to fill with latitudes.
       *   @param lons          Vector to fill with longitudes.
       *   @param distances     Smallest distance to border maps.
       *   @param dests         The destinations.
       *   @param driverPrefs   The driver preferences. Should be used
       *                        to remove the unreachable nodes.
       */
      void getExternalNodesToLevel(int level,
                                   vector<uint32> &externalNodes,
                                   vector<int32>& lats,
                                   vector<int32>& lons,
                                   map<uint32,uint32>& distances,
                                   const OrigDestInfoList& dests,
                                   const RMDriverPref& driverPrefs);

      /**
       *   Return the ID of the higher level map at level
       *   <code>higherlevel</code>.
       *   @param higherlevel The higher level.
       */
      uint32 getHigherLevelMapID(int higherlevel) const;

      /**
       *   Fill the set with maps neighbouring this one
       *   on the same level as this map.
       */
      void getNeighbourMapIDs(set<uint32> &neigbourMapIDs);

      /**
       *   On a higher level map this function translates the
       *   nodes from lower level <code>lower</code> the vector
       *   in to nodes on higher level in <code>higher</code>.
       *   @param higher The higher level mapIDs/nodeIDs will be
       *                 put here. If a node isn't found on higher
       *                 level <code>MAX_UINT32</code>,
       *                 <code>MAX_UINT32</code> will be put in the
       *                 vector.
       *   @param lower  The lower level nodeid:s to translate.
       *   @return The size of lower.
       */
      int lookupNodesLowerToHigher( IDPairVector_t& higher,
                                    const IDPairVector_t& lower);


      /**
       *   On a higher level map this function translates 
       *   the nodes from higher level in the vector
       *   <code>higher</code> to nodes on lower level in the vector
       *   <code>lower</code>.
       *   @param lower  The lower level map/node-ids will be put here.
       *   @param higher The higher level map/node-ids to translate.
       *   @return size of higher.
       */
      int lookupNodesHigherToLower( IDPairVector_t& lower,
                                    const IDPairVector_t& higher);
      
      /**
       * Gets the mapID of the map in this CalcRoute-object.
       *
       * @return The mapID.
       */
      uint32 getMapID();

      /**
       * Gets the map itself.
       *
       * @return The RoutingMap.
       */      
      RoutingMap* getMap();
   
   private:

      /**
       *   Function that does noting but forces the expression to
       *   be calculated now.
       */
      void trickTheOptimizer(uint32& inParam);
      
      /**
       *   Removes duplicate nodes in the supplied list.
       *   Keeps the ones with the smallest offsets.
       *   @param theList The list to remove the dups from.
       *   @param isDest  True if the list is a destination list.
       */
      int removeDups(Head* theList, bool isDest);
      
      /**
       * @name Route initialization functions.
       */
      //@{

        /**
         *   Clean ups old garbage and resets the map.
         */
        void resetToStartState();
 
        /**
         *   The real route function. Can return early so
         *   things that should be done before and after
         *   routing should be done in route.
         */      
         uint32 realRoute(Head* origin, 
                          Head* destination,
                          Head* allDestinations,
                          SubRouteList* incomingList,
                          SubRouteList* resultList,
                          const RMDriverPref* driverParam,
                          bool originalRequest,
                          bool routeToAll,
                          bool calcCostSums,
                          bool sendSubRoutes);
   
         /**
          * Initializes the route. If original is true this method calls 
          * initRouteFirstTime otherwise initRoute below. 
          *
          * @param  original true if this is the first initRoute called 
          *         for this route.
          * @param  origin A list with the origins in this map.
          * @param  destination A list with the destinations in this map.
          * @param  driverParam The driver's parameters.
          * @param  routeToHigherLevel Is true if we are trying to route up
          *         a level.
          * @param  forward Tells if we are supposed to route forward.
          * @return An error code if something goes wrong.
          */
         uint32 initRoute( bool original, 
                           Head* origin,
                           Head* destination,
                           const RMDriverPref* driverParam,
                           bool routeToHigherLevel,
                           bool forward);

         /**
          * Initializes the route. The function updates the BucketHeap's
          * costs to those in the startRoutingNodeList, and puts the nodes
          * (and their opposites) of the list into the priority queue. The
          * function marks the destination nodes in the map (from the
          * endRoutingNodeList) also.
          *
          * @param  startRoutingNodeList A list with the origins in this
          *                              map.
          * @param  endRoutingNodeList A list with the destinations in this
          *         map.
          * @param  driverParam The driver's parameters.
          * @param  routeToHigherLevel Is true if we are trying to route up
          *         a level.
          * @param  forward Tells if we are supposed to route forward.
          * @return An error code if something goes wrong.
          */
         uint32 initRoute( Head* startRoutingNodeList,
                           Head* endRoutingNodeList,
                           const RMDriverPref* driverParam,
                           bool routeToHigherLevel,
                           bool forward );

         /**
          * Initializes the route the first time. The function
          * checks whether or not it is necessary to walk nodes, and checks
          * if they lie within no throughfare areas. The nodes of the
          * startRoutingNodeList (and the opposite nodes on the same
          * segments) will be enqueued into the correct priority queue
          * (m_priorityQueue for no restrictions,
          * m_throughfarePriorityQueue for no throghfare nodes and
          * m_notValidPriorityQueue for not vaild nodes). The turning
          * costs of the segments are also added (but not for not valid
          * nodes, as there is no turning cost for walking). The function
          * also marks the destination nodes from endRoutingNodeList.
          *
          * @param  startRoutingNodeList A list with the origins in this
          *                              map.
          * @param  endRoutingNodeList A list with the destinations in this
          *         map.
          * @param  driverParam The driver parameters.
          * @param  routeToHigherLevel This is true if we are trying to
          *         route up a level.
          * @param  forward Tells if we are supposed to route forward.
          * @param  secondTIme If secondTime is true only the origins
          *                    are updated.
          
          * @return Error code if something goes wrong.
          */
         uint32 initRouteFirstTime( Head* startRoutingNodeList,
                                    Head* endRoutingNodeList,
                                    const RMDriverPref* driverParam,
                                    bool routeToHigherLevel,
                                    bool forward,
                                    bool secondTime = false);

         /**
          * Initialize the route when routing with public transportations.
          *
          * @param origin is the list of with origin(s).
          * @param dest is the list with the destinations.
          * @param driverParam is the list with driver parameters.
          * @param forward if true we should route in the forward
          *                direction.
          * @return Error code if something goes wrong. 
          */
         uint32 initRouteBus( Head* origin,
                              Head* dest,
                              const RMDriverPref* driverParam,
                              bool forward );

         /**
          * Initializes a walking route. 
          *
          * @param  origin A list with the origins in this map.
          * @param  dest A list with the destinations in this map.
          * @param  driverParam The driver parameters.
          * @param  forward Tells if we are supposed to route forward.
          * @return Error code if something goes wrong. 
          */
         uint32 initRouteWalking( Head* origin,
                                  Head* dest,
                                  const RMDriverPref* driverParam,
                                  bool forward );
            
         /**
          * Initializes the route on higher level. Enqueues the nodes into
          * the priority queue, as a preparation for the Dijkstra
          * algorithm.
          * 
          * @param  startRoutingNodeList A list with the origins in the
          *                              higher level map.
          * @param  endRoutingNodeList A list with the destinations in the
                    higher level map.
          * @param  driverParam The driver parameters.
          * @return Error code if something goes wrong. 
          */
         uint32 initRouteOnHigherLevel( Head* startRoutingNodeList,
                                        Head* endRoutingNodeList,
                                        const RMDriverPref* driverParam );
      //@}

      /**
       * Method that checks if it is possible to drive from a 
       * node to at least one connecting node with the 
       * current vehicle.
       *
       * @param curNode is the start node.
       * @param driverParam contains the vehicle type.
       * @param forward if true we are routing in the forward 
       *        direction.
       * @return true if we could drive to at least one connecting node.
       */
      bool checkVehicleRestrictions( RoutingNode* curNode,
                                     const RMDriverPref* driverParam,
                                     bool forward );

      /**
       * Method for expanding the no way streets.
       * Expands the m_notValidPriorityQueue heap
       * that where initialized in initRouteFirstTime.
       *
       * @param driverParam is the driver parameters.
       * @param forward if true we are routing in the forward 
       *        direction.
       */
      void expandNonValidNodes( const RMDriverPref* driverParam,
                                bool forward );

      /**
       * Method for expanding the no throughfare streets.
       * Expands the m_throughfarePriorityQueue heap
       * that where initialized in initRouteFirstTime.
       *
       * @param driverParam is the driver parameters.
       * @param forward if true we are routing in the forward 
       *        direction.
       */
      void expandThroughfareNodes( const RMDriverPref* driverParam,
                                   bool forward );

      /**
       * @name Cost calculating funtions (the implementations of the
       *       Dijkstra algorithm).
       */
      //@{
         /**
          * Calculates the cost between the origins and the destinations
          * that are set by the route initialization functions. This
          * variant uses the normal Dijkstra algorithm to calculate the
          * routes.
          * 
          * @param driverParam The driver preferences.
          * @param destination The list of destination nodes (used to add
                               the offset to the destination cost).
          * @param forward     If true then the routing will be done from
          *                    origin to destination, otherwise in the
          *                    opposite direction.
          * @param routeToAll  True if routes to all destinations should
          *                    be calculated.
          * @param estimate    True if the distance to the destination
          *                    should be added to the cost.
          * @param walking     True if the route is a walking route.
          * @param throughfareOK True if no-throughfare is ok to drive on.
          */
         inline void calcCostDijkstra(const RMDriverPref* driverParam,
                                      dpCost_t costA,
                                      dpCost_t costB,
                                      dpCost_t costC,
                                      Head* destination,
                                      bool forward,
                                      bool routeToAll,
                                      bool estimate,
                                      bool walking,
                                      bool throughfareOK = false);
         /**
          *   Does some checks on the parameters before calling 
          *   calcCostDijkstra. If all goes well there should 
          *   be inlined versions of calcCostDijkstra for different
          *   combinations of constant indata.
          */
         inline void calcCostDijkstra(const RMDriverPref* driverParam,
                                      dpCost_t costA,
                                      dpCost_t costB,
                                      dpCost_t costC,
                                      Head* destination,
                                      bool forward,
                                      bool routeToAll);
         /**
          *   Does some checks on the parameters before calling 
          *   calcCostDijkstra. If all goes well there should 
          *   be inlined versions of calcCostDijkstra for different
          *   combinations of costs.
          *
          * @param driverParam The driver preferences.
          * @param destination The list of destination nodes (used to add
          the offset to the destination cost).
          * @param forward     If true then the routing will be done from
          *                    origin to destination, otherwise in the
          *                    opposite direction.
          * @param routeToAll  True if routes to all destinations should
          *                    be calculated.          
          */
         inline void calcCostDijkstra(const RMDriverPref* driverParam,
                                      Head* destination,
                                      bool forward,
                                      bool routeToAll = false);

         /**
          * Calculates the cost between an origin and all external
          * connections. This funtion uses the normal Dijkstra algorithm.
          * 
          * @param driverParam        The driver preferences.
          * @param routeToHigherLevel Is true if we are trying to route up
          *                           to a higher level.
          * @param forward            If true then the routing will be
          *                           done from origin to destination,
          *                           otherwise in the opposite direction.
          * @param estimate           True if estimation should be used.
          */
         inline void calcCostExternalDijkstra(const RMDriverPref* driverParam,
                                              dpCost_t costA,
                                              dpCost_t costB,
                                              dpCost_t costC,
                                              bool routeToHigherLevel,
                                              bool forward,
                                              bool estimate,
                                              Head* allDestinations);
         
         /**
          *   Function that tries to inline the other
          *   calcCostExternalDijkstra.
          */
         inline void calcCostExternalDijkstra(const RMDriverPref* driverParam,
                                              dpCost_t costA,
                                              dpCost_t costB,
                                              dpCost_t costC,
                                              bool routeToHigherLevel,
                                              bool forward,
                                              Head* allDestinations);
         
         /**
          *    Function that tries to inline the other
          *    calcCostExternalDijkstra.
          */
         inline void calcCostExternalDijkstra(const RMDriverPref* driverParam,
                                              bool routeToHigherLevel,
                                              bool forward,
                                              Head* allDestinations);
      //@}

         
      /**
       * @name Result reading function that extracts the calculated routes.
       */
      //@{         
         /**
          * Fills the result list with the path and distance/time params.
          * 
          * @param origin      The list with the origins in this map.
          * @param destination The list with the destinations in this map.
          * @param result      The list with the resulting subroutes
          *                    (outparam).
          * @param forward     If true then the routing will be done from
          *                    origin to destination, otherwise in the
          *                    opposite direction.
          * @param calcCostSums True if the sums of costs a-c should
          *                     be calculated too.
          * @param readToExternal True (default) if the result should
          *                       be read to external nodes also.
          *                       Should be false when reading result
          *                       to all.
          */
         uint32 readResult(Head* origin,
                           Head* destination,
                           SubRouteList* incomingList,
                           SubRouteList* resultList,
                           const RMDriverPref* driverParam,
                           bool forward,
                           bool calcCostSums,
                           bool readToExternal = true);

         /**
          * Fills the result list with the path and distance/time params.
          * Gives the SubRoutes to all destinations.
          * @param origin      The list with the origins in this map.
          * @param destination The list with the destinations in this map.
          * @param result      The list with the resulting subroutes
          *                    (outparam).
          * @param forward     If true then the routing will be done from
          *                    origin to destination, otherwise in the
          *                    opposite direction.
          */
         uint32 readResultToAll(Head* origin,
                                Head* destination,
                                SubRouteList* incomingList,
                                SubRouteList* resultList,
                                const RMDriverPref* driverParam,
                                bool forward,
                                bool calcCostSums);
         
         /**
          * Fills the result list with the path and distance params, when
          * a walking route was calculated.
          *
          * @param origin      The list with the origins in this map.
          * @param destination The list with the destinations in this map.
          * @param result      The list with the resultint subroutes
          *                    (outparam).
          * @param forward     If true the routing will be done from origin
          *                    to destination, otherwise in the opposite
          *                    direction.
          */
         uint32 readResultWalk(Head* origin,
                               Head* destination,
                               SubRouteList* incomingList,
                               SubRouteList* resultList,
                               const RMDriverPref* driverParam,
                               bool forward,
                               bool calcCostSums);

         /**
          * Used to read the result when routing on 
          * public transportations.
          */
         uint32 readResultBus( Head* origin,
                               Head* destination,
                               SubRouteList* incomingList,
                               SubRouteList* resultList,
                               const RMDriverPref* driverParam,
                               bool forward );

      
         /**
          * Fills the result list with the external nodes that could be
          * routed to and their cost.
          * 
          * @param incoming The list with all incoming subroutes.
          * @param result The list to be filled with the result (outparam).
          * @param driverParam The driver preferences.
          * @param dest The allDestination list.
          * @param orig The origins. Used when routing over segments
          *             with only external connections.
          * @param forward True if routing forward,
          *                false if routing backwards.
          * @param calcCostSums True if the sum of costs a-c should be
          *        calculated.
          */
         uint32
            readResultToExternalConnection( SubRouteList* incoming,
                                            SubRouteList* result,
                                            const RMDriverPref* driverParam,
                                            Head* dest,
                                            Head* origin,
                                            bool forward,
                                            bool calcCostSums);

         /**
          * Reads the result after a higher level routing.
          *
          * @param origin The list of origin nodes.
          * @param destination The list of destination nodes.
          * @param incomingList A list of incoming subroutes.
          * @param resultList A list of the result (outparameter).
          * @param driverParam The driver preferences.
          * @param calcCostSums True if the sum of all costs a-c should
          *                     be calced.
          */
         uint32 readResultOnHigherLevel( Head* origin,
                                         Head* destination,
                                         SubRouteList* incomingList,
                                         SubRouteList* resultList,
                                         const RMDriverPref* driverParam,
                                         bool calcCostSums);

         /**
          * Reads the result to the best node after higher level routing.
          * 
          * @param origin The list of origin nodes.
          * @param destination The list of destination nodes.
          * @param incomingList A list of incoming subroutes.
          * @param resultList A list of the result (outparameter).
          * @param driverParam The driver preferences.
          */
         uint32 readResultToBestOnHigherLevel( Head* origin,
                                               Head* destination,
                                               SubRouteList* incomingList,
                                               SubRouteList* resultList,
                                            const RMDriverPref* driverParam,
                                               bool calcCostSums);

         
      
         /**
          * This function is browses the route and puts the nodes into a
          * SubRoute. The function is called by the other result reading
          * functions.
          * 
          * @param incoming The incoming list of subroutes.
          * @param result The result list of subroutes (outparam).
          * @param dest The destination to be parsed from.
          * @param external The external node leading to another map.
          * @param driverParam The driver's parameters.
          * @param forward If true read result as we routed from
          *        origin to dest, otherwise in the opposite direction.
          * @param offset The offset of the subroute.
          */
         CompleteRMSubRoute*
            readResultFromDestination( SubRouteList* incoming,
                                       SubRouteList* result,
                                       RoutingNode* dest,
                                       RoutingNode* external,
                                       const RMDriverPref* driverParam,
                                       bool forward,
                                       uint16 offset,
                                       bool calcCostSums);      
      //@}

      /**
       *   "Routes" backwards from the destinations and tries to
       *   find the valid nodes on the edges of a non-valid area.
       *   The valid nodes are put in m_normalPriorityQueue and
       *   expanded in expandNodesResult.
       *   Note that the estimated cost is used in a special way
       *   in this function. Since
       *   this functions does not set real cost it can tell if
       *   a node was visited by calcCostDijkstra by looking at 
       *   the real cost and see if it is not MAX_UINT32. All nodes
       *   visited by this function will have realCost == MAX_UINT32
       *   inside the function.
       */
      void expandNonValidNodesResult( RedBlackTree* resetHeapNonValid,
                                      RedBlackTree* resetHeapThroughfare,
                                      const RMDriverPref* driverParam,
                                      bool forward  );

      /**
       */
      void expandThrougfareNodesResult( RedBlackTree* resetHeapThroughfare,
                                        const RMDriverPref* driverParam,
                                        bool forward  );

      /**
       *   Routes from the nodes in m_normalPriorityQueue to the destinations
       */
      void expandNodesResult( const RMDriverPref* driverParam,
                              bool forward );
      
      /**
       *    Finds the smallest connection cost from the supplied node.
       *    Can be used as the cost for traversing the segment.
       *    @param curNode The node to use.
       *    @param driverParam The driver parameters.
       *    @param forward Not used, was here because of an error.
       *    @return The smallest connection cost.
       */
      uint32 getMinCost( RoutingNode* curNode,
                         const RMDriverPref* driverParam,
                         bool forward = true);
                         

      /**
       *    Finds the smallest connection cost from the supplied node.
       *    Can be used as the cost for traversing the segment.
       *    @param curNode The node to use.
       *    @param costA   The priver parameter cost A.
       *    @param costB   The driver parameter cost B.
       *    @return The smallest connection cost.
       */
      uint32 getMinCost( RoutingNode* curNode,
                         uint32 vehRes,
                         dpCost_t costA,
                         dpCost_t costB,
                         dpCost_t costC);

      /**
       *    Does the same as getMinCost but for walking when
       *    we would want to drive.
       */
      uint32 getMinCostWalk( RoutingNode* curNode,
                             uint32 vehRes,
                             dpCost_t costA,
                             dpCost_t costB,
                             dpCost_t costC );

      /**
       *    Does the same as getMinCost but for walking when
       *    we would want to drive.
       */
      uint32 getMinCostWalk( RoutingNode* curNode,
                             const RMDriverPref* driverParam);

      /**
       */
      void insertStateElement( CompleteRMSubRoute* completeSubRoute,
                               const RMDriverPref* driverParam,
                               bool forward );

      /**
       * Calculates the cost corresponding to the estimated distance or the
       * time to the destination from a node.
       * 
       * @param  node The node for which to calculate the
       *         minimum distance from all destinations.
       * @param  dest is the list of destinations to calculate
       *         the distance to.
       * @param  driverPref The driver parameters.
       * @return The minimum estimated cost from node to all destinations.
       */
      uint32 estimateDistToDest(RoutingNode* node,
                                Head* dest,
                                const RMDriverPref* driverPref);

      uint32 estimateDistToDest(RoutingNode* node,
                                Head* dest,
                                dpCost_t costA,
                                dpCost_t costB,
                                dpCost_t costC,
                                dpCost_t costD);


      /**
       *   Searches the origDests for a node with the supplied id.
       *   @param origDests List of origins and destinations to look in.
       *   @param nodeID    Node id to look for.
       *   @return OrigDestNode with the right node id or NULL if not found.
       */
      static OrigDestNode* getOrigDestNode( Head* origDests,
                                            uint32 nodeID );
      
      /**
       *   Searches the origDests for a node with the same id as the
       *   supplied one.
       *   @param origDests List of origins and destinations to look in.
       *   @param realNode  A node with the id to look for.
       *   @return OrigDestNode with the right node id or NULL if not found.
       */
      static OrigDestNode* getOrigDestNode( Head* origDests,
                                            const RoutingNode* realNode );

      
      /**
       * Calculates the end offset cost of the destination node.
       * 
       * @param  destination A list of the destination nodes (to get the
       *                     offset).
       * @param  node        The destination node.
       * @param  driverParam The customer parameters.
       * @param  forward     Tells if routing is carried out forwards or
       *                     backwards.
       * @return             The cost between the node and the destination
       *                     point.
       */
      uint32 calcOffsetCost(Head* destination,
                            RoutingNode* node,
                            const RMDriverPref* driverParam,
                            bool forward = true);

      /**
       *
       *
       */
      uint32 calcOffsetCostWalk(  Head* destinations,
                                  RoutingNode* node,
                                  const RMDriverPref* driverParam,
                                  bool forward );
      
      /**
       * Traces the path from the input node back to the origin. The
       * nodeIDs are dumped to cout as hex and the node costs as dec.
       * 
       * @param node        The node to trace the path from.
       * @param driverParam The driver parameters of the routing.
       */
      void tracePath(RoutingNode* node,
                     const RMDriverPref* driverParam);

      /**
       * Prints the number of external nodes visited during routing to
       * cout. This is a function used for debugging only.
       */
      void printNbrExternalNodesVisited();

      /**
       *   Checks the origins and destinations to see if they are
       *   valid ( exist in the map).
       *   @param origins The origin list.
       *   @param dests   The destination list.
       *   @return StringTable::OK if all dests and origs are OK.
       */
      inline uint32 checkOrigDests(Head* origins,
                                   Head* dests);
      
      /**
       *   Calculates the cost to be added to the cost of a node.
       *   This is so that we may optimize the calculation.
       *   @param costA The costA of the driverPrefs.
       *   @param costB The costB of the driverPrefs.
       *   @param costC The costC of the driverPrefs.
       *   @param costD The costD of the driverPrefs.
       *   @param data  The RoutingConnectionData for the current connection.
       *   @return <code>costA * data->getCostA() +
       *           costB * data->getCostB() + costC * data->getCostC() +
       *           costD * data->getCostD() </code>
       */
      static
      inline uint32 calcConnectionCost(dpCost_t costA,
                                       dpCost_t costB,
                                       dpCost_t costC,
                                       dpCost_t costD,
                                       uint32 vehicleRest,
                                       const RoutingConnectionData* data);

      /**
       *   Calculates the walking costB using the costA and walking speed.
       *   @param costA Distance cost.
       *   @return Time cost.
       */
      static inline uint32 getWalkingCostBFromA( uint32 costA );
      
      /**
       *   Calculates the connection cost to be added to the cost of a 
       *   node when the user has to walk instead of driving a car.
       *   @param costA The costA of the driverPrefs.
       *   @param costB The costB of the driverPrefs.
       *   @param costC The costC of the driverPrefs.
       *   @param costD The costD of the driverPrefs.
       *   @param data  The RoutingConnectionData for the current connection.
       */
      static inline
         uint32 calcConnectionCostWalk(dpCost_t costA,
                                       dpCost_t costB,
                                       dpCost_t costC,
                                       dpCost_t costD,
                                       uint32 vehicleRest,
                                       const RoutingConnectionData* data);

      /**
       *    Returns true if it is allowed to drive on the segment.
       *    @param node The node to drive from.
       *    @param pref The driver preferences.
       *    @param forward True if we are driving forward.
       *    @param onlyFrom True if we should only check the direction
       *                    from the segment.
       */  
      static inline bool canDriveOnSegment(const RoutingNode* node,
                                           const RMDriverPref* pref,
                                           bool forward,
                                           bool onlyFrom = false);

      /**
       *    Returns true if we think that the supplied node was
       *    ok to drive into. Used to check if non-valid routing is
       *    needed when starting the routing.
       *    @param node The node to check.
       *    @param pref The priver parameters.
       *    @param forward True if we are routing in the forward dir.
       */
      inline bool couldDriveIntoSegment(const RoutingNode* node,
                                        const RMDriverPref* pref,
                                        bool forward);
      
       /**
        *   Calculates the connection cost to be added to the cost of a 
        *   node when the user has to walk instead of driving a car.
        *   @param driverParam The driver preferences.
        *   @param data        Connection data for the current connection.
        *   @return costA converted into time if necessary.
        */
      static inline uint32
         calcConnectionCostWalk(const RMDriverPref* driverParam,
                                const RoutingConnectionData* data);
      
      /**
       *   Uses the <code>DriverPref*</code> and
       *   <code>RoutingConnectionData</code> to calc the cost to add to
       *   a node.
       */
      static inline uint32
         calcConnectionCost(const RMDriverPref* driverParam,
                            const RoutingConnectionData* data);

      /**
       *   Checks the connection between <code>fromNode</code> and 
       *   <code>toNode</code> and returns the difference between
       *   <code>costC</code> (time incl disturbances) and
       *   <code>costB</code> (time excl disturbances) in seconds.
       *   @param fromNode The node which the connection leads from.
       *   @param toNode   The node to which the connection leads.
       *   @return Extra cost in seconds.
       */ 
      inline uint32 checkAdditionalCosts(RoutingNode* fromNode,
                                         RoutingNode* toNode);


      /**
       *   Reads the result from the supplied destination and fills 
       *   a CompleteRMSubRoute with the result. If an error occurs
       *   NULL is returned. The CompleteRMSubRoute should be deleted
       *   by the caller.
       *   @param dest The destination in the route.
       *   @param forward If the route is forward, the read subroute is
       *                  reversed (so it will be in the right direction)
       *   @param extID   The ID of the node that can come from an external
                          route. Used to pair the route with
       *                  previous subroutes. Should not be needed since
       *                  this info should be in the OrigDestNode.
       *   @param calcCostSums True if the sums of the costs a-c should
       *                       be calced. If == false 0 will be returned
       *                       in costASum-costCSum
       *   @return A CompleteRMSubRoute from the origin to the destination.       
       */
      inline CompleteRMSubRoute* fillSubRouteFromDestination(RoutingNode* dest,
                                                             bool forward,
                                                             uint32& extID,
                                                             bool calcCostSums,
                                                             uint32& costASum,
                                                             uint32& costBSum,
                                                             uint32& costCSum
                                                             );
      /**
       *   Adds the connection costs a-c (from node fromNode to node
       *   toNode) to the supplied costs a-c.
       */
      void sumCostsForConnection(RoutingNode* fromNode,
                                 RoutingNode* toNode,
                                 uint32& costASum,
                                 uint32& costBSum,
                                 uint32& costCSum);
            
      /**
       *   Returns true if the node is inside <code>m_lowerLevelBBox</code>
       *   or if we are not routing to higher level. Used when routing to
       *   higher level.
       *   @param nextNode      The node that the current connection leads to.
       *   @param curNodeOnLowerLevel True if the node that the
       *                              current connection leads to is on lower.
       *   @param routeToHigher True if we are routing to higher level.
       *   @return True if the node should be included in the calculations.
       */
      inline bool shouldIncludeNode(const RoutingNode* nextNode,
                                    bool curNodeOnLowerLevel,
                                    bool routeToHigher);

      /**
       *    Removes node to which it is impossible to drive.
       *    @param origDest The nodes to check.
       *    @param prefs    The current driverprefs.
       *    @param forward  True if forwardconnections should be checked.
       */
      inline Head* removeUnreachable(Head* origDest,
                                     const RMDriverPref* prefs,
                                     bool forward);

      inline int routeOnOneSegment(Head* origs,
                                   Head* dests,
                                   const RMDriverPref* pref,
                                   bool forward,
                                   SubRouteList* incoming,
                                   SubRouteList* result,
                                   bool routeToAll,
                                   bool calcCostSums);
      
      /**
       *   Updates the lower level bounding. Inside the bounding box
       *   it is allowed to calculate lower level nodes, but outside
       *   it is forbidden. Some distance is added to the bounding box.
       *   @param origins The origins are used to update the bbox.
       */
      inline void updateLowerLevelBBox(Head* origins);
      
      /**
       *   Prints the CVS version of the .cpp file.
       */
      static bool printVersion();

      /**
       *   Dummy variable used when printing the version.
       */
      static bool versionPrinted;

      /**
       *   Checks if a node is on the specified level and above.
       *   @param level   The level, where 0 is lowest and 1 is the
       *                  country overview map and so on. NB! Level
       *                  is almost ignored until we know more about the
       *                  new map hierarchy. Level 1 is the overviewmap
       *                  for now.
       *   @param nodeToCheck The node to check the level for.
       *   @return True if the node exists on the specified level.
       */
      bool isOnLevel(uint32 level,
                     const RoutingNode& nodeToCheck);

      

// ========================================================================
//                                                       Member variables =
      
   /**
    * Object containing the map.
    */
   RoutingMap* m_map;
   
   /**
    * The cosine latitude factor for this map. See some references on
    * Vector analysis.
    */
   float32 m_cosLat;

   /**
    * The priority queue used for the normal routing.
    * @see PriorityQueue.h
    */
#ifdef  MAIN_PRIO_QUEUE_IN_RM_IS_BUCKET_HEAP
   BucketHeap* m_priorityQueue;
#else
   RedBlackTree* m_priorityQueue;
#endif
   /**
    * The priority queue used for the routing within no-throughfare areas.
    * @see PriorityQueue.h
    */
   RedBlackTree* m_throughfarePriorityQueue;

   /**
    *   Priority queue for nodes that are not allowed inside the
    *   bounding boxes for higher level.
    */
   RedBlackTree* m_outsideHeap;
   
   /**
    * The priority queue used for routing not valid areas (like in the
    * opposite direction of travel in a one-way street).
    * @see PriorityQueue.h
    */
   RedBlackTree* m_notValidPriorityQueue;

   /**
    */
   RedBlackTree* m_normalPriorityQueue;

   /**
    * The cutoff value.
    */
   uint32 m_cutOff;

   /**
    *   Bounding box that contains the area in which lower level
    *   routing is allowed when going to higher level.
    */
   MC2BoundingBox* m_lowerLevelBBox;

   /**
    *   Bounding box for the area where it is allowed to continue
    *   on from lower level nodes to lower level nodes.
    */
   MC2BoundingBox* m_outerLowerLevelBBox;
   
#ifdef USE_RESET_THREAD_IN_CALCROUTE

   /** Starts the reset thread */
   void startResetThread();

   /** Waits for the reset thread to finish */
   void waitForResetThread();

   /** Called by the reset thread when it's done */
   void resetThreadHasFinished();
   
   /**
    *   Monitor that keeps the map from beeing reset
    *   and used simultaneously
    */
   ISABMonitor m_resetMonitor;

   /**
    *   True if reset thread is running for this map.
    */
   bool m_resetThreadRunning;

   /**
    *   Vector of nodes visited by CalcCostDijkstra but rejected.
    */
   vector<pair<const RoutingConnection*,
               RoutingNode*> > m_invalidNodeVector;
   
   /** Testing a reset thread */
   friend class ResetThread;
#endif
   
}; // CalcRoute

// ========================================================================
//                                                       Inline functions =

inline uint32 
CalcRoute::estimateDistToDest( RoutingNode* node,
                               Head* dest,
                               dpCost_t incostA,
                               dpCost_t incostB,
                               dpCost_t incostC,
                               dpCost_t incostD)
                               
{
   uint32 driverA = incostA;
   uint32 driverB = incostB;
   uint32 driverC = incostC;
   //int32 tmpCost;
   //int32 cost = MAX_INT32;
   //int32 deltaX, deltaY;
   double cost = MAX_INT64;
   //uint32 z = 0;   

   OrigDestNode* tempNode = static_cast<OrigDestNode*>(dest->first());
   
   
   while( tempNode != NULL ){
      int32 meanLat = ((node->getLat() >> 1) + (tempNode->getLat() >> 1));
      double deltaX = ((double)node->getLong() - (double)tempNode->getLong());
      
      double cosLat = cos( meanLat * GfxConstants::invRadianFactor );
      double deltaY = (double)node->getLat()  - (double)tempNode->getLat();
      deltaX *= cosLat;
      // <= sqrt(deltaX²+deltaY²), at most 2% error
      // int32 z = deltaX + deltaY;
      // tmpCost = z - 6*deltaX*deltaY/(5*z);
      // Testing old version again.
      double tmpCost = deltaX*deltaX + deltaY * deltaY;
      // Testing manhattan distance / sqrt(2)
      //double tmpCost = (fabs(deltaX) + fabs(deltaY)) / 1.4;
      // Testing the longest of x and y
      // double tmpCost = MAX(deltaX, deltaY);
      if(tmpCost < cost )
         cost = tmpCost;
      tempNode = static_cast<OrigDestNode*>(tempNode->suc());
   }
   if( cost >= (MAX_INT64 * 0.9) ) {
      return 0;
   } else {
      // Calculate the distance in meters.
      double distance = sqrt(cost) * GfxConstants::MC2SCALE_TO_METER;

      // Trying with 120 km/h for now.
      static const double invSpeed = 1.0 / (Math::KMH_TO_MS * 120.0);

      double time = distance * invSpeed;
      // CostB is calculated as centimeters when driving
      // in "normal speed".
      double costB = Connection::secToTimeCost(time);
      
      // Cost A is in centimetres.
      double costA = Connection::metersToDistCost(distance);
      
      return uint32(costA * driverA) + uint32(costB * driverB)
         + uint32(costB * driverC);
      
   }
}


#endif // CALCROUTE_H
