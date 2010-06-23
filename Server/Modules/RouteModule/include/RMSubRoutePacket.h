/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SUB_ROUTE_PACKET_RM_H
#define SUB_ROUTE_PACKET_RM_H

#include "config.h"
#include "DebugRM.h"

#include "Packet.h"


class OrigDestNode;
class SubRouteList;
class RMSubRoute;
class RMDriverPref;
class Head;
class DisturbanceVector;
class RoutingMap;

/**
 * Packets of this class are sent internally between the Route leader and 
 * the jobthreads of the availables (and the leader). 
 *
 * The name of the class has been changed to RMSubRoutePacket so that
 * we will be able to test the new Server-classes and then remove this
 * version of the packet.
 *
 * \begin{tabular}{lll}
 * Pos            & Size    & Description \\ \hline
 *                & 4 bytes & routeID \\
 *                & 4 bytes & cutOff  \\
 *                & 4 bytes & routingCosts(A -- D) \\
 *                & 4 bytes & vehicleRestrictions
 *                & 1 byte  & route to all destinations ( 1 = yes ) \\
 *                & 1 byte  & listType \\
 *                & 1 byte  & firsttime request \\
 *                & 1 byte  & use turncost ? \\
 *                & 4 byte  & time  in UMT time \\
 *                & 2 bytes & min waiting time in seconds\\
 *                & 2 bytes & the number of subroutes stored in this packet
 *                            (nbrSubRoutes) \\
 *                & 2 bytes & the number of origins in the packet \\
 *                & 2 bytes & the number of destinations in the packet \\
 * each origin    & 4 bytes & the ID of the map in which this node lies \\
 *                & 4 bytes & itemID \\
 *                & 2 bytes & offset \\
 *                & 1 byte  & isDirectionTowardsZero \\
 *                & 1 byte  & pad byte \\
 *                & 4 bytes & latitude \\
 *                & 4 bytes & longitude \\
 * each dest      & 4 bytes & the ID of the map in which this node lies \\
 *                & 4 bytes & itemID \\
 *                & 2 bytes & offset \\
 *                & 2 byte  & pad bytes \\
 *                & 4 bytes & latitude \\
 *                & 4 bytes & longitude \\
 * each subroute  & 4 bytes & the ID of the subroute (subRouteID) \\
 *                & 4 bytes & the ID of the subroute previous to this one
 *                            in the total route (prevSubRouteID) \\
 *                & 1 byte  & positive number larger than zero means that
 *                            the route is complete, zero means not
 *                            (routecomplete) \\
 *                & 1 byte  & positive number larger than zero means the
 *                            routingdirection is forward, zero means
 *                            backwards (forward) \\
 *                & 2 bytes & the number of connections in the subroute to
 *                            other maps (nbrConnections) \\
 * each connection& 4 bytes & the ID of the map that the connection leads
 * in subroute                to (mapID) \\
 *                & 4 bytes & the ID of the node in the other map
 *                            (nodeID) \\
 *                & 4 bytes & the cost to reach the node in the other map
 *                            (cost) \\
 *                & 4 bytes & the estimated cost to reach the node in the
 *                            other map (estimatedCost) \\
 * \end{tabular}
 *
 */
class RMSubRouteRequestPacket : public RequestPacket
{
   public:
   
      /**
       * Creates a RMSubRouteRequestPacket. 
       *
       * @param leaderIP is the IP of the leader.
       * @param leaderPort is the port that the leader listen to TCP.
       * @param subRouteList contains data for start routing on.
       * @param driverPref contains the driver preferences.
       * @param origin   A list with origins (only those valid to the map
       *                 are added).
       * @param dest     A list with destinations (all added for
       *                 estimation).
       * @param original If true this is the first SubRouteRequest for
       *                 this route.
       * @param mapID    The mapID to send the request to.
       */
      RMSubRouteRequestPacket(uint32 leaderIP,
                              uint16 leaderPort,
                              SubRouteList* subRouteList,
                              RMDriverPref* driverPref,
                              Head* origin,
                              Head* dest,
                              bool original,
                              uint32 mapID);
      
      /**
       * Fills the supplied SubRouteList with the data from the packet.
       *
       * @param subRouteList The SubRouteList to fill.
       * @param disturbances The DisturbanceVector to fill.
       */
      void getSubRouteList(SubRouteList* subRouteList,
                           DisturbanceVector* disturbances = NULL) const;
      
      /**
       * Gets all origins from the packet.
       *
       * @param  origins is the list to be filled with the origins.
       */
      void getOrigins(RoutingMap* theMap, Head* origins) const;
      
      /**
       * Gets all destinations from the packet containing the mapID.
       *
       * @param destinations is the list to be filled with the destinations.
       */
      void getDestinations( RoutingMap* theMap, Head* destinations ) const;
      
      /**
       * Gets all destinations from the packet.
       *
       * @param  destinations is the list to be filled with the destinations.
       */
      void getAllDestinations( RoutingMap* theMap, Head* destinations ) const;
      
      /**
       * Gets the driver preferences (Routing parameters).
       *
       * @param driverPref id the driver preferences.
       */
      void getDriverPreferences( RMDriverPref* driverPref ) const;
      
      /**
       * Returns true if this is the first subroute request for the route.
       *
       * @return True if this is the first subroute request, false otherwise.
       */
      inline bool isOriginalRequest() const;
      
      /**
       * Dumps the packet data to cout.
       */
      void dumpRoutePacket();
      
//   private:
      
      /**
       * Get the routeID.
       *
       * @return The routeID.
       */
      inline uint32 getRouteID() const;
      
      /**
       * Set the routeID 
       *
       * @param routeID is the routeID to set.
       */
      inline void setRouteID( uint32 routeID );
      
      /**
       * Get the cutOff.
       *
       * @return The cutOff.
       */
      inline uint32 getCutOff() const;
      
      /**
       * Set the cutOff.
       *
       * @param cutOff The cutOff to set.
       */
      inline void setCutOff( uint32 cutOff );
      
      /**
       * Get the routingCosts.
       *
       * @return The routingCosts.
       */
      inline uint32 getRoutingCosts() const;
      
      /**
       * Set the routing costs.
       *
       * @return cost is the routing costs.
       */
      inline void setRoutingCosts( uint32 cost );
      
      /**
       * Get the vehicleRestrictions.
       *
       * @return The vehicleRestrictions.
       */
      inline uint32 getVehicleRestrictions() const;
      
      /**
       * Sets the vehicle restrictions.
       *
       * @param vehicle is the vehicle restrictions.
       */
      inline void setVehicleRestrictions( uint32 vehicle );
      
      /**
       * Get the listType.
       *
       * @return The listType.
       */
      inline uint16 getListType() const;
      
      /**
       * Set the listType.
       *
       * @param listType is the.
       */
      inline void setListType( uint16 listType );

      /**
       *   Get the parameter which tells us that we calculate
       *   routes to <b>all</b> destinations.
       */
      inline bool getRouteToAll() const;

      /**
       *   Set the parameter which tells us that we should
       *   calc routes to <b>all</b> destinations.
       */
      inline void setRouteToAll(bool routeToAll);

      /**
       *   If this parameter is set to true the sums of costA-C
       *   should be returned in the SubRoutes.
       *   @return True if the sums of costA-C should be returned.
       */
      inline bool getCalcSums() const;


      /**
       *   If this one returns true if the SubRoutes should not be
       *   sent in the replypacket.
       */
      inline bool getDontSendSubRoutes() const;
   
      /**
       * Tells if this is the first time a subrouteRequest
       * is sent for this route.
       *
       * @param original if true this is the first time a subrouteRequest.
       */
      inline void setIsOriginalRequest( bool original ); 
      
      /**
       * Returns true if we should use a u-turn cost.
       *
       * @return true if we should use a u-turn cost.
       */
      inline bool getUseTurnCost() const; 
      
      /**
       * Tells if we should use a u-turn cost when routing.
       *
       * @param uturn if true we should use a uturn cost when routing.
       */
      inline void setUseTurnCost( bool uturn ); 
      
      /**
       * Returns the start or end time in UMT time.
       *
       * @return the start or end time.
       */
      inline uint32 getTime() const; 
      
      /**
       * Setts the start or end time in UMT time.
       *
       * @param time is the start or end time.
       */
      inline void setTime( uint32 time ); 
      
      /**
       * Returns the minimum waiting time at a bus stop. Only used when 
       * routing on public transportations. In seconds.
       *
       * @return the minimum waiting time i.e. the time of arrival before
       *         the transportation leaves.
       */
      inline uint16 getMinWaitTime() const; 
      
      /**
       * Setts the minimum waiting time at a bus stop. Only used when 
       * routing on public transportations.In seconds.
       *
       * @param time is the minimum waiting time i.e. the time of arrival before
       *        the transportation leaves.
       */
      inline void setMinWaitTime( uint16 time ); 
      
      /**
       * Get the number of SubRoutes in the packet.
       *
       * @return The number of SubRoutes in the packet.
       */
      inline uint16 getNbrSubRoutes() const;
      
      /**
       * Sets the number of subroutes.
       *
       * @param nbrSubRoute is the number of subroutes.
       */
      inline void setNbrSubRoutes( uint16 nbrSubRoute );
      
      /**
       * Get the number of origins in the packet.
       *
       * @return The number of origins in the packet.
       */
      inline uint16 getNbrOrigins() const;
      
      /**
       * Set the number of origins in the packet.
       *
       * @param nbr is the number of origins in the packet.
       */
      inline void setNbrOrigins( uint16 nbr );
      
      /**
       * Get the number of destinations in the packet.
       *
       * @return The number of destinations in the packet.
       */
      inline uint16 getNbrDestinations() const;
      
      /**
       * Set the number of destimations in the packet.
       *
       * @param nbr is the number of destimations in the packet.
       */
      inline void setNbrDestinations( uint16 nbr );
      
      /**
       * Add the specified origin to the packet. Increases the value of
       * number of origins in the packet.
       *
       * @param origin The node to write.
       * @param pos    The position tio write at.
       */
      void addOrigin( OrigDestNode* origin,
                      int32& pos );
      
      /**
       * Add the specified destination to the packet. Increases the value of
       * number of destinations in the packet.
       *
       * @param dest The node to write.
       * @param pos  The position tio write at.
       */
      void addDestination( OrigDestNode* dest,
                           int32& pos );
      
      /**
       * Add a subroute to the packet.
       *
       * @param subRoute is the subroute to add.
       * @param pos is the position to wtrite the subroute at.
       */
      void addSubRoute( RMSubRoute* subRoute,
                        int32& pos );
      
      /**
       * Increases the variable nbrSubRoutes in the packet (one step).
       */
      inline void incNbrSubRoutes();
      
// ========================================================================
//                                                 Constants in the class =
      /**
       * @name Packet positions.
       * @memo The positions in the packet of the variables.
       * @doc  The positions in the packet of the variables.
       */
      //@{
         /**
          * The position in the packet of routeID.
          */
         static const int ROUTE_ID_POS = REQUEST_HEADER_SIZE;
         
         /**
          * The position in the packet of cutOff.
          */
         static const int CUT_OFF_POS = ROUTE_ID_POS + 4;
         
         /**
          * The position in the packet of routingCosts.
          */
         static const int ROUTING_COSTS_POS = CUT_OFF_POS + 4;
         
         /**
          * The position in the packet of vehicleRestrictions.
          */
         static const int VEHICLE_RESTRICTION_POS = ROUTING_COSTS_POS + 4;

         /**
          *   The position of the value that says if we should
          *   route to all destinations or not.
          */
         static const int ROUTE_TO_ALL_POS = VEHICLE_RESTRICTION_POS +4;

         /**
          *   The bitposition of ROUTE_TO_ALL.
          */
         static const int ROUTE_TO_ALL_BIT_POS = 0;
         
         /**
          *   The position for the bit that tells us if we should
          *   calc cost A - C for the route. ( For sortdist).
          *   Currently the same as ROUTE_TO_ALL_POS.
          */
         static const int CALC_SUM_OF_COSTS_POS = ROUTE_TO_ALL_POS;

         /**
          *   The bit-position of the CALC_SUM_OF_COSTS flag.
          */
         static const int CALC_SUM_OF_COSTS_BIT_POS = 1;

         /**
          *   Set this flag to true if you don't want SubRoutes
          *   to be sent, only the costs. Should be used for sortdist.
          */
         static const int DONT_SEND_SUBROUTES_POS = ROUTE_TO_ALL_POS;

         /**
          *   The bit-position of the DONT_SEND_SUBROUTES flag.
          */
         static const int DONT_SEND_SUBROUTES_BIT_POS = 2;
   
         /**
          * The position in the packet of listType.
          */
         static const int LIST_TYPE_POS = ROUTE_TO_ALL_POS + 1;
         
         /**
          * The position in the packet of isOriginalRequest.
          */
         static const int IS_ORIGINAL_POS = LIST_TYPE_POS + 1;
         
         /**
          * The position in the packet of isOriginalRequest.
          */
         static const int USE_UTURN = IS_ORIGINAL_POS + 1;
         
         /**
          * The position in the packet of isOriginalRequest.
          */
         static const int TIME_POS = USE_UTURN + 1;
         
         /**
          * The position in the packet of isOriginalRequest.
          */
         static const int MIN_WAITING_TIME_POS = TIME_POS + 4;
         
         /**
          * The position in the packet of nbrOriginNodes.
          */
         static const int NBR_SUB_ROUTES_POS = MIN_WAITING_TIME_POS + 2;
         
         /**
          * The position in the packet of nbr of origins..
          */
         static const int NBR_ORIGINS_POS = NBR_SUB_ROUTES_POS + 2;
         
         /**
          * The position in the packet of nbr of destinatoins.
          */
         static const int NBR_DESTINATIONS_POS = NBR_ORIGINS_POS + 2;
         
         /**
          * The position where the OrigDests start. This constant does not
          * hold the position of a variable, it is the actual position in
          * the packet where the OrigDests start.
          */
         static const int SUBROUTE_REQUEST_HEADER_SIZE =
            NBR_DESTINATIONS_POS + 2;
      //@}
         
      /**
       * The size of an origin or a destination in the packet (in bytes).
       */
      static const int ORIG_DEST_SIZE = 20;
      
};

// ======================================================================
//                                   Implementation of inline functions =

inline uint32
RMSubRouteRequestPacket::getRouteID() const
{
   return readLong( ROUTE_ID_POS );
}

inline void
RMSubRouteRequestPacket::setRouteID( uint32 routeID ) 
{
   writeLong( ROUTE_ID_POS, routeID );
}

inline uint32
RMSubRouteRequestPacket::getCutOff() const
{
   return readLong( CUT_OFF_POS );
}

inline void
RMSubRouteRequestPacket::setCutOff( uint32 cutOff )
{
   writeLong( CUT_OFF_POS, cutOff );
}

inline uint32
RMSubRouteRequestPacket::getRoutingCosts() const
{
   return readLong( ROUTING_COSTS_POS );
}

inline void 
RMSubRouteRequestPacket::setRoutingCosts( uint32 routingCost )
{
   writeLong( ROUTING_COSTS_POS, routingCost );
}


inline uint32
RMSubRouteRequestPacket::getVehicleRestrictions() const
{
   return readLong( VEHICLE_RESTRICTION_POS );
}

inline void 
RMSubRouteRequestPacket::setVehicleRestrictions( uint32 vehicle )
{
   writeLong( VEHICLE_RESTRICTION_POS, vehicle );
}

inline uint16
RMSubRouteRequestPacket::getListType() const
{
   return readByte( LIST_TYPE_POS );
}

inline void 
RMSubRouteRequestPacket::setListType( uint16 listType )
{
   writeByte( LIST_TYPE_POS, listType );
}

inline void
RMSubRouteRequestPacket::setRouteToAll(bool routeToAll)
{
   writeByte( ROUTE_TO_ALL_POS, routeToAll ? 1 : 0 );
}

inline bool
RMSubRouteRequestPacket::getRouteToAll() const
{
   return bool( readBit( ROUTE_TO_ALL_POS, ROUTE_TO_ALL_BIT_POS) );
}

inline bool
RMSubRouteRequestPacket::getCalcSums() const
{
   return bool ( readBit( CALC_SUM_OF_COSTS_POS,
                          CALC_SUM_OF_COSTS_BIT_POS ));
}


inline bool
RMSubRouteRequestPacket::getDontSendSubRoutes() const
{
   return bool( readBit( DONT_SEND_SUBROUTES_POS,
                         DONT_SEND_SUBROUTES_BIT_POS) );
}

inline bool
RMSubRouteRequestPacket::isOriginalRequest() const
{
   return bool( readByte( IS_ORIGINAL_POS ) );
}

inline void
RMSubRouteRequestPacket::setIsOriginalRequest( bool original ) 
{
   writeByte( IS_ORIGINAL_POS, original );
}

inline bool
RMSubRouteRequestPacket::getUseTurnCost() const
{
   return bool( readByte( USE_UTURN ) );
}

inline void
RMSubRouteRequestPacket::setUseTurnCost( bool uturn ) 
{
   writeByte( USE_UTURN, uturn );
}

inline uint32 
RMSubRouteRequestPacket::getTime() const
{
   return readLong( TIME_POS );
}

inline void 
RMSubRouteRequestPacket::setTime( uint32 time )
{
   writeLong( TIME_POS, time );
}

inline uint16 
RMSubRouteRequestPacket::getMinWaitTime() const
{
   return readShort( MIN_WAITING_TIME_POS );
}

inline void 
RMSubRouteRequestPacket::setMinWaitTime( uint16 time )
{
   writeShort( MIN_WAITING_TIME_POS, time );
}

inline uint16
RMSubRouteRequestPacket::getNbrSubRoutes() const
{
   return readShort( NBR_SUB_ROUTES_POS );
}

inline void 
RMSubRouteRequestPacket::setNbrSubRoutes( uint16 nbrSubRoute )
{
   writeShort( NBR_SUB_ROUTES_POS, nbrSubRoute );
}

inline void
RMSubRouteRequestPacket::incNbrSubRoutes()
{
   writeShort( NBR_SUB_ROUTES_POS, getNbrSubRoutes() + 1 );
}

inline uint16
RMSubRouteRequestPacket::getNbrOrigins() const
{
   return readShort( NBR_ORIGINS_POS );
}

inline void 
RMSubRouteRequestPacket::setNbrOrigins( uint16 nbr )
{
   writeShort( NBR_ORIGINS_POS, nbr );
}

inline uint16 
RMSubRouteRequestPacket::getNbrDestinations() const
{
   return readShort( NBR_DESTINATIONS_POS );
}

inline void 
RMSubRouteRequestPacket::setNbrDestinations( uint16 nbr )
{
   writeShort( NBR_DESTINATIONS_POS, nbr );
}

/**
 * Class that describes packets that are sent from the RM availables to
 * RM leader. Contains information about costs and IDs of the nodes that
 * could/should be passed in the route. Packets of this type might be very
 * large, so they must be sent via TCP.
 *
 * The format of this packettype is as follows:
 * \begin{tabular}{lll}
 * Pos             & Size    & Description
 *                 & 2 bytes & the type of routes in this packet (listType)
 *                 & 2 bytes & (padbytes, not used)
 *                 & 4 bytes & the ID of this route (routeID)
 *                 & 4 bytes & the cutOff cost for this routing (cutOff)
 *                 & 4 bytes & the number of stored subroutes in this
 *                             packet (nbrSubRoutes)
 * each subroute   & 4 bytes & the ID of the SubRoute (subRouteID)
 *                 & 4 bytes & the ID of the SubRoute previous to this one
 *                             (prevSubRouteID)
 *                 & 1 byte  & if this value is greater than zero, this
 *                             route is complete (routeComplete)
 *                 & 1 byte  & if this value is greater than zero, the
 *                             routing was carried out forwards, otherwise
 *                             backwards (forward)
 *                 & 2 bytes & (padbytes, no meaning)
 *                 & 4 bytes & the ID of the map that this subroute comes
 *                             from (comesFromMapID)
 *                 & 4 bytes & the number of connections to other maps
 *                             (nbrConnections)
 *                 & 4 bytes & number of nodes in the subroute (nbrNodes)
 * each connection & 4 bytes & the ID of the map that this connection leads
 *                             to (mapID)
 * each connection & 4 bytes & the ID of the item that this connection
 *                             leads to (nodeID)
 * each connection & 4 bytes & the cost to reach the segment in the other
 *                             map (cost)
 * each connection & 4 bytes & the estimated cost to reach the closest
 *                             destination (estimatedCost)
 * each node       & 4 bytes & the node ID
 * each node       & 2 bytes & the offset of the node (only available for
 *                             proximity requests)
 * each node       & 2 bytes & (padbytes, only for proximity requests)
 * each node       & 4 bytes & the cost to reach the node (only available
 *                             for proximity requests)
 * \end{tabular}
 *
 * @see RouteReplyPacket
 * @see RMSubRouteRequestPacket
 */
class RMSubRouteReplyPacket : public ReplyPacket
{
   public:

      /**
       * Constructor that copies the parameters of a RMSubRouteRequestPacket.
       *
       * @param requestPacket The RMSubRouteRequestPacket to get parameters
       *                      from.
       * @param listsize      the total size the list to add to the packet.
       */
      RMSubRouteReplyPacket( const RMSubRouteRequestPacket* requestPacket, 
                             uint32 listsize,
                             uint32 status );
      
      /**
       * Adds a SubRouteList to the packet.
       *
       * @param  subRouteList The SubRouteList to add.
       * @return              The sum of number of subroutes and startNbr.
       */
      void addSubRouteList( SubRouteList* subList );

      /**
       * Get the list of subroutes and store them in the list sent as
       * inparameter.
       *
       * @param subRouteList The SubRouteList to fill with the data from
       *                     the packet.
       */
      void getSubRouteList( SubRouteList* subRouteList );
      
      /**
       * Dump the packet contents to cout.
       */
      void dumpRoutePacket();
      
      /**
       * Get the routeID.
       *
       * @return The routeID of the SubRouteList in this packet.
       */
      inline uint32 getRouteID() const;

      /**
       * Set the routeID.
       *
       * @param routeID The routeID to set.
       */
      inline void setRouteID( uint32 routeID );
      
   private:

      /**
       * Set the listType.
       *
       * @param listType The type of list to set.
       */
      inline void setListType( uint16 listType );

      /**
       * Get the listType.
       *
       * @return The listType of the SubRouteList in this packet.
       */
      inline uint16 getListType() const;

    
      /**
       * Set the cutOff.
       *
       * @param cutOff The cutOff to set.
       */
      inline void setCutOff( uint32 cutOff );

      /**
       * Get the cutOff.
       *
       * @return The cutOff of the SubRouteList in this packet.
       */
      inline uint32 getCutOff() const;

      /**
       * Set the number of SubRoutes in the packet.
       *
       * @param nbrSubRoutes The new value of the number of SubRoutes in
       *                     the packet.
       */
      inline void setNbrSubRoutes( uint32 nbrSubRoutes );
      
      /**
       * Get the number of SubRoutes in the packet.
       *
       * @return The number of SubRoutes in the packet.
       */
      inline uint32 getNbrSubRoutes() const;

      /**
       * @name Positions in the packet.
       */
      //@{
         /**
          * The position of listType.
          */
         static const int LIST_TYPE_POS = REPLY_HEADER_SIZE;
         
         /**
          * The position of routeID.
          */
         static const int ROUTE_ID_POS = LIST_TYPE_POS + 4;
         
         /**
          * The position of cutOff.
          */
         static const int CUT_OFF_POS = ROUTE_ID_POS + 4;

         /**
          * The position of nbrSubRoutes.
          */
         static const int NBR_SUB_ROUTES_POS = CUT_OFF_POS + 4;

         /**
          * The position where the subRoutes start.
          */
         static const int SUBROUTE_REPLY_PACKET_SIZE = NBR_SUB_ROUTES_POS
            + 4;
      //@}

}; // RMSubRouteReplyPacket

// =======================================================================
//                                    Implementation of inline functions =

inline void
RMSubRouteReplyPacket::setListType(uint16 listType)
{
   writeShort(LIST_TYPE_POS, listType);
}


inline uint16 
RMSubRouteReplyPacket::getListType() const
{
   return readShort(LIST_TYPE_POS);
}


inline void 
RMSubRouteReplyPacket::setRouteID(uint32 routeID)
{
   writeLong(ROUTE_ID_POS, routeID);
}


inline uint32 
RMSubRouteReplyPacket::getRouteID() const
{
   return readLong(ROUTE_ID_POS);
}


inline void
RMSubRouteReplyPacket::setCutOff(uint32 cutOff)
{
   writeLong(CUT_OFF_POS, cutOff);
}


inline uint32
RMSubRouteReplyPacket::getCutOff() const
{
   return readLong(CUT_OFF_POS);
}


inline void
RMSubRouteReplyPacket::setNbrSubRoutes(uint32 nbrSubRoutes)
{
   writeLong(NBR_SUB_ROUTES_POS, nbrSubRoutes);
}


inline uint32
RMSubRouteReplyPacket::getNbrSubRoutes() const 
{
   return readLong(NBR_SUB_ROUTES_POS);
}

#endif // SUB_ROUTE_PACKET_RM_H
