/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTE_PROCESSOR_H
#define ROUTE_PROCESSOR_H

#include "config.h"

#include "MapHandlingProcessor.h"

class CalcRoute;
class DisturbancePushPacket;
class EdgeNodesReplyPacket;
class EdgeNodesRequestPacket;
class Head;
class IDTranslationReplyPacket;
class IDTranslationRequestPacket;
class RMDriverPref;
class RMSubRouteReplyPacket;
class RMSubRouteRequestPacket;
class RoutingMap;
class SubRouteList;
class UpdateTrafficCostReplyPacket;
class UpdateTrafficCostRequestPacket;

/**
 * The RouteProcessor handles routing, maploading and disturbance cost
 * updating requests.
 *
 *
 */
class RouteProcessor : public MapHandlingProcessor 
{
  public:
  
      /**
       *   Constructor creates a new RouteProcessor
       *
       *   @param   SafeVector   Array containing the ID of the maps
       *                         loaded by this processor. To make it
       *                         possible for the RouteReader to know
       *                         what maps are loaded.
       *   @param   packetFile   File to save all incoming packets to
       *                         for reading when profiling or something.
       *                         If <code>NULL</code> no file will be opened.
       */
      RouteProcessor(MapSafeVector* loadedMaps,
                      const char* packetFile = NULL);

      /**
       *   Destructor for RouteProcessor.
       */
      virtual ~RouteProcessor();


      /**
       *   Get the status of this processor. 
       *   This is currently not used.
       *
       *   @return 0 for now.
       */
      int getCurrentStatus();

      /**
       *    Virtual function for loading a map. Should be called
       *    by handleRequest when that function gets a loadMapRequest
       *    and isn't virtual anymore.
       *    @param mapID The map to load.
       *    @param mapSize Outparameter describing the size of the
       *                   map.
       *    @return StringTable::OK if ok.
       */
      StringTable::stringCode loadMap(uint32 mapID,
                                      uint32& mapSize);

      /**
       *    Virtual function to be called when a map should
       *    be deleted.
       *    @param mapID The map to be deleted.
       *    @return StringTable::OK if ok.
       */
      StringTable::stringCode deleteMap(uint32 mapID);

protected:
      /**
       *    Handles all request for a new routing.
       *
       *    @param   p  A request packet that contains a "question"
       *                to this module.
       *    @return  A Packet containg the answer of the request.
       */
      Packet* handleRequestPacket( const RequestPacket& p,
                                   char* packetInfo);
   
private:

      /**
       * Updates a map with traffic information.
       *
       * @param updateCostsPacket is a packet containing data to update.
       * @return a UpdateTrafficCostReplyPacket containing the status of
       *         the updated data.
       */
      inline UpdateTrafficCostReplyPacket*
         processUpdateTrafficCostRequestPacket(
            const UpdateTrafficCostRequestPacket* updateCostsPacket );

      /**
       * Handles a route request from the leader.
       *
       * @param subRouteRequestPacket is the incoming packet to be handled.
       * @return a SubRouteReplyPacket contining route data.
       */
      inline RMSubRouteReplyPacket*
         processSubRouteRequestPacket( const RMSubRouteRequestPacket*
                                       subRouteRequestPacket );

      /**
       *   Handles an EdgeNodesRequest.
       *   @param reqPacket The request to handle.
       *   @return The answer to the request.
       */
      inline EdgeNodesReplyPacket*
         processEdgeNodesRequestPacket( const EdgeNodesRequestPacket* reqPacket);

      /**
       *   Handles a IDTranslationRequestPacket.
       *   @param p The packet to handle.
       *   @return The answer to the request.
       */
      inline IDTranslationReplyPacket*
       processIDTranslationRequestPacket( const IDTranslationRequestPacket* p);

      /**
       *   Adds disturbances from InfoModule to the RoutingMap.
       *   Does not return a reply, because this is push.
       *   @param dp The DisturbancePushPacket from the InfoModule.
       *   @return Number of disturbances in packet.
       */
      inline int processDisturbancePushPacket( const DisturbancePushPacket* dp);
      
      /**
       *    Returns the index of the calcRoute containing the map with
       *    ID mapID. 
       *    If no CalcRoute object with correct map is found, MAX_UINT32 is 
       *    returned.
       * 
       *    @param  mapID  The ID of the map the wanted CalcRoute should
       *                   contain.
       *    @return The index in the calcRouteVector if the wanted
       *            calcroute or MAX_UINT32.
       */
      uint32 getCalcRouteIndex( uint32 mapID ) const;
      
      /**
       *    Returns the CalcRoute object containing the map with ID mapID. 
       *    If no CalcRoute object with correct map is found, NULL is 
       *    returned.
       * 
       *    @param  mapID  The ID of the map the wanted CalcRoute
       *                   should contain.
       *    @return The wanted CalcRoute object if it exists, otherwise NULL.
       */
      CalcRoute* getCalcRoute( uint32 mapID ) const;
      
      /**
       *    Get a packet with the subroutes that is the answer to the
       *    requestPacket. {\it {\ bf NB!} The packet returned is
       *    created here, but {\bf must} be deleted elsewhere!}
       *    @param   requestPacket  The packet containing the "question"
       *                            (the origins and destinations).
       *    @param   resultList     A list with the possible sub routes
       *                            that is the answer to the 
       *                            requestPacket.
       *    @param   status         is the status of the reply.
       *    @return  A SubRouteReplyPacket containing the sub routes in 
       *             the result list.
       */
      RMSubRouteReplyPacket*
      getSubRouteReplyPacket( const RMSubRouteRequestPacket* requestPacket,
                              SubRouteList* resultList,
                              uint32 status );

      /**
       * Setts the uturn cost in the origin list.
       * 
       * @param origin is the list with origins.
       */
      void updateUturns( const RMDriverPref* driverParam,
                         Head* origin );

      /**
       * Update the indices of the origins. For origins this is only
       * needed the first time as the subroutes to the external maps
       * already contain the indices.
       *
       * @param calc     The CalcRoute object containing a RoutingMap.
       * @param nodeList The list to update the indices for.
       */
      void updateIndices(CalcRoute* calc, Head* nodeList);
      
      /**
       * Copies the valid origins (i.e. the ones lying in routingMap) in
       * the subRouteList to the origin list.
       *
       * @param subList The SubRoute-list to copy from.
       * @oaram origin  The CCSet-list to copy to.
       * @param map     The map to be routed in. 
       */
      void copySubroutesToOrigin(SubRouteList* subList, 
                                 Head* origin, 
                                 RoutingMap* routingMap);
      
      /**
       * Copies the valid origins and destinations in the subRouteList 
       * to the lists. If subroute is forward it is copied to the origin list
       * and if backward to the destination list.
       *
       * @param subList is the list to copy from.
       * @oaram origin is the list to copy to.
       * @param map is the map to be routed on. 
       */
      void copySubroutes( SubRouteList* subList, 
                          Head* origin, 
                          Head* dest, 
                          RoutingMap* map );


      typedef std::vector < CalcRoute* > CalcRouteVector;

      /**
       *   Used to do the actual routing.
       *
       *   @see CalcRoute  
       */
      CalcRouteVector* m_calcRouteVector;

      /**
       * Print debug data for SubRouteRequest. (Should only be done for
       * original SubRouteRequests.)
       *
       * @param original Only print data when this is true.
       * @param origin   The list with origins.
       * @param dest     The list with destinations.
       */ 
      void printDebugData(bool original, Head* origin, Head* dest);

      /**
       * Prints some information when no map was found.
       * @param mapID The requested mapID.
       * @param p The request packet.
       */
      void handleMapNotFound(uint32 mapID, const RequestPacket* p);

      /**
       *   Filename of file to save packets to or <code>NULL</code>.
       */
      char* m_packetFileName;

}; // RouteProcessor

// =======================================================================
//                                    Implementation of inline functions =

#endif // ROUTE_PROCESSOR_H
