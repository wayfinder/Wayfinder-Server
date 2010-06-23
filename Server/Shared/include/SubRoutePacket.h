/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NEW_SUB_ROUTE_PACKET_H
#define NEW_SUB_ROUTE_PACKET_H

#include "config.h"
#include "Types.h"
#include "Packet.h"
#include "SubRouteListTypes.h"
#include "DisturbanceList.h"

class OrigDestInfo;
class OrigDestInfoList;
class SubRoute;
class SubRouteVector;
class DriverPref;
class Vehicle;


/**
 *   Packets of this class are sent internally between the Route leader and 
 *   the jobthreads of the availables (and the leader).
 *   Modified to work with the new classes in the server. (Write only).
 *   Should be modified later to be the same in the RouteModule and the
 *   server. Currently called RMSubRoutePacket in the Module.
 *
 * \begin{tabular}{lll}
 * Pos            & Size    & Description \\ \hline
 *                & 4 bytes & routeID \\
 *                & 4 bytes & cutOff  \\
 *                & 4 bytes & routingCosts(A -- D) \\
 *                & 4 bytes & vehicleRestrictions
 *                & 1 byte  & route to all destinations ( 1 == yes ) \\
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
class SubRouteRequestPacket : public RequestPacket
{
public:
   
   /**
    * Creates a SubRouteRequestPacket. 
    *
    * @param driverPref Contains the driver preferences.
    * @param origs      A list with origins (only those valid to the map
    *                   are added).
    * @param dests      A list with destinations (all added for
    *                   estimation).
    * @param allDests   All destinations for the last via route.
    * @param routeToOneDest Stop after finding the first destination.
    * @param cutOff     The cutoff. If the cost is higher than this,
    *                   we stop routing.
    * @param levelDelta 0  -> Go up 0 levels,
    *                   1  -> Go up one level,
    *                   -1 -> Go up -1 level == go down one level.
    * @param disturbances Disturbances to add to the packet.
    * @param calcCostSums True if the separate sums of the costs should
    *                     be calculated and returned in the reply.
    * @param dontSendSubRoutes True if no SubRoutes should be sent in the
    *                          reply, only the costs. (For sortdist)
    * @param infoModuleDists Disturbances from InfoModule. Can only be
    *                        on this map and can only have extra cost,
    *                        never factor.
    */
   SubRouteRequestPacket(const DriverPref*       driverPref,
                         const SubRouteVector*   origs,
                         const OrigDestInfoList* dests,
                         const OrigDestInfoList* allDests,
                         bool                    routeToOneDest,
                         uint32                  cutOff,
                         int                     levelDelta = 0,
                         const DisturbanceVector* disturbances = NULL,
                         bool calcCostSums                     = false,
                         bool dontSendSubRoutes                = false,
                         const DisturbanceVector* infoModuleDists = NULL );
   
   /**
    * Sets the vehicle restrictions.
    *
    * @param vehicle is the vehicle restrictions.
    */
   inline void setVehicleRestrictions( uint32 vehicle );
   
   /**
    *   Set the listType.
    *
    *   @param listType is the.
    */
   inline void setListType( uint16 listType );
   
   /**
    *   Returns the list type of this subroutepacket.
    */
   inline SubRouteListTypes::list_types getListType() const;      
   
   /**
    *   Set to true if routes to all destinations should
    *   be returned.
    *   @param routeToAll True if all destinations should
    *                     return routes.
    */
   inline void setRouteToAll(bool routeToAll);
   
   /**
    *   Set to true if the sums of costA, the sums of costB and
    *   the sums of costC should be calculated for all routes.
    */
   inline void setCalcCostSums(bool calcCostSums);

   /**
    *   Set to true if only the costs should be sent and not the
    *   subroutes. Typically for sortdist.
    */
   inline void setDontSendSubRoutes(bool dont);
   
   /**
    * Tells if this is the first time a subrouteRequest
    * is sent for this route.
    *
    * @param original if true this is the first time a subrouteRequest.
    */
   inline void setIsOriginalRequest( bool original ); 
   
   /**
    * Tells if we should use a u-turn cost when routing.
    *
    * @param uturn if true we should use a uturn cost when routing.
    */
   inline void setUseTurnCost( bool uturn ); 
   
   /**
    * Setts the start or end time in UMT time.
    *
    * @param time is the start or end time.
    */
   inline void setTime( uint32 time ); 
   
   /**
    * Setts the minimum waiting time at a bus stop. Only used when 
    * routing on public transportations.In seconds.
    *
    * @param time is the minimum waiting time i.e. the time of
    *         arrival before
    *        the transportation leaves.
    */
   inline void setMinWaitTime( uint16 time ); 
   
   /**
    *   Sets all the routing costs at once.
    *   @param routingCost A,B,C,D in a longword where A is the
    *                      most significant byte.
    */
   inline void setRoutingCosts( uint32 routingCost );
      
   /**
    * Get the number of SubRoutes in the packet.
    *
    * @return The number of SubRoutes in the packet.
    */
   uint16 getNbrSubRoutes();
   
   /**
    * Sets the number of subroutes.
    *
    * @param nbrSubRoute is the number of subroutes.
    */
   inline void setNbrSubRoutes( uint16 nbrSubRoute );
   
   /**
    *   Sets the routeid of the pcaket.
    */
   inline void setRouteID( uint32 routeID );
   
   /**
    *   Sets the cutoff.
    */
   inline void setCutOff( uint32 cutOff );
   
   /**
    * Get the number of origins in the packet.
    *
    * @return The number of origins in the packet.
    */
   inline uint16 getNbrOrigins();
   
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
   inline uint16 getNbrDestinations();
   
   /**
    * Set the number of destimations in the packet.
    *
    * @param nbr is the number of destimations in the packet.
    */
   inline void setNbrDestinations( uint16 nbr );
   
   /**
    *   Adds the disturbancelist at position <code>pos</code>
    *   in the packet. <code>pos</code> will be incremented.
    *   @param pos  The current position before and after.
    *   @param vect The vector to add.
    */
   void addDisturbances(int& pos,
                        const DisturbanceVector* vect);

   /**
    *   Adds the disturbancelist at position <code>pos</code>
    *   in the packet. The disturbances must be on this map and
    *   can only have extra cost, not factors.
    */
   void addInfoModDisturbances( int& pos,
                                const DisturbanceVector* vect );

   /**
    * Add the specified origin to the packet. Increases the value of
    * number of origins in the packet.
    *
    * @param origin The node to write.
    * @param pos    The position tio write at.
    */
   void addOrigin( const OrigDestInfo& origin,
                   int32& pos );
   
   void addOrigin( uint32 mapID,
                   uint32 nodeID,
                   float  offset,
                   uint32 lat,
                   uint32 lon,
                   int32& pos);
   
   /**
    * Add the specified destination to the packet. Increases the value of
    * number of destinations in the packet.
    *
    * @param dest The node to write.
    * @param pos  The position tio write at.
    */
   void addDestination( const OrigDestInfo& dest,
                        int32& pos );
   
   /**
    * Add a subroute to the packet.
    *
    * @param subRoute is the subroute to add.
    * @param pos is the position to wtrite the subroute at.
    */
   void addSubRoute( const SubRoute* subRoute,
                     int32& pos,
                     bool forward = true);
   
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

inline void
SubRouteRequestPacket::setRouteID( uint32 routeID ) 
{
   writeLong( ROUTE_ID_POS, routeID );
}

inline void
SubRouteRequestPacket::setCutOff( uint32 cutOff )
{
   writeLong( CUT_OFF_POS, cutOff );
}

inline void 
SubRouteRequestPacket::setRoutingCosts( uint32 routingCost )
{
   writeLong( ROUTING_COSTS_POS, routingCost );
}


inline void 
SubRouteRequestPacket::setVehicleRestrictions( uint32 vehicle )
{
   writeLong( VEHICLE_RESTRICTION_POS, vehicle );
}

inline void 
SubRouteRequestPacket::setListType( uint16 listType )
{
   writeByte( LIST_TYPE_POS, listType );
}

inline SubRouteListTypes::list_types
SubRouteRequestPacket::getListType() const
{
   return SubRouteListTypes::list_types(readByte( LIST_TYPE_POS));
}

inline void
SubRouteRequestPacket::setRouteToAll(bool routeToAll)
{
   writeBit( ROUTE_TO_ALL_POS, ROUTE_TO_ALL_BIT_POS,
             routeToAll);
}


inline void
SubRouteRequestPacket::setCalcCostSums(bool calcCostSums)
{
   writeBit( CALC_SUM_OF_COSTS_POS, CALC_SUM_OF_COSTS_BIT_POS,
             calcCostSums);
}

inline void
SubRouteRequestPacket::setDontSendSubRoutes( bool dont )
{
   writeBit( DONT_SEND_SUBROUTES_POS, DONT_SEND_SUBROUTES_BIT_POS,
             dont);
}

inline void
SubRouteRequestPacket::setIsOriginalRequest( bool original ) 
{
   writeByte( IS_ORIGINAL_POS, original );
}

inline void
SubRouteRequestPacket::setUseTurnCost( bool uturn ) 
{
   writeByte( USE_UTURN, uturn );
}

inline void 
SubRouteRequestPacket::setTime( uint32 time )
{
   writeLong( TIME_POS, time );
}

inline void 
SubRouteRequestPacket::setMinWaitTime( uint16 time )
{
   writeShort( MIN_WAITING_TIME_POS, time );
}

inline void 
SubRouteRequestPacket::setNbrSubRoutes( uint16 nbrSubRoute )
{
   writeShort( NBR_SUB_ROUTES_POS, nbrSubRoute );
}

inline void
SubRouteRequestPacket::incNbrSubRoutes()
{
   writeShort( NBR_SUB_ROUTES_POS, getNbrSubRoutes() + 1 );
}


inline void 
SubRouteRequestPacket::setNbrOrigins( uint16 nbr )
{
   writeShort( NBR_ORIGINS_POS, nbr );
}

inline uint16
SubRouteRequestPacket::getNbrOrigins( )
{
   return readShort( NBR_ORIGINS_POS );
}


inline void 
SubRouteRequestPacket::setNbrDestinations( uint16 nbr )
{
   writeShort( NBR_DESTINATIONS_POS, nbr );
}

inline uint16
SubRouteRequestPacket::getNbrDestinations()
{
   return readShort( NBR_DESTINATIONS_POS);
}


/**
 * Class that describes packets that are sent from the RM availables to
 * RM leader. Contains information about costs and IDs of the nodes that
 * could/should be passed in the route. Packets of this type might be very
 * large, so they must be sent via TCP.
 *
 * Modyfied by pi to work with the new server classes. (Only receiving).
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
 * each node       & 2 bytes & the number of cost sums (typically 3).
 * each cost sum   & 4 bytes & the sum of cost [i] where i can be a,b,c.
 * each node       & 4 bytes & the cost to reach the node (only available
 *                             for proximity requests)
 * \end{tabular}
 *
 * @see RouteReplyPacket
 * @see SubRouteRequestPacket
 */
class SubRouteReplyPacket : public ReplyPacket
{
public:

   /**
    * Get the list of subroutes and store them in the list sent as
    * inparameter.
    *
    * @param driverPref   The driverpreferences. Needed to find the
    *                     vehicles for the <code>SubRoutes</code>.
    * @param subRouteList The <code>SubRouteVector</code>
    *                     to fill with the data from the packet. Should
    *                     be empty.
    */
   void getSubRouteVector( const DriverPref* driverPref,
                           SubRouteVector& subRouteVector );
      
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

private:

   /**
    *   Get a Vehicle from a state element.
    *   @param driverPref The driver prefs to get the element from.
    *   @param stateElement The state element to use.
    */
   static const Vehicle* stateElementToVehicle(const DriverPref* driverPref,
                                               uint32 stateElement);
      
   /**
    * Get the listType.
    *
    * @return The listType of the SubRouteList in this packet.
    */
   inline uint16 getListType() const;
    
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
   uint32 getNbrSubRoutes() const;

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
   static const int SUBROUTE_REPLY_PACKET_SIZE = NBR_SUB_ROUTES_POS + 4;
   //@}

}; // SubRouteReplyPacket

// =======================================================================
//                                    Implementation of inline functions =

inline uint16 
SubRouteReplyPacket::getListType() const
{
   return readShort(LIST_TYPE_POS);
}


inline uint32 
SubRouteReplyPacket::getRouteID() const
{
   return readLong(ROUTE_ID_POS);
}


inline uint32
SubRouteReplyPacket::getCutOff() const
{
   return readLong(CUT_OFF_POS);
}

inline uint32
SubRouteReplyPacket::getNbrSubRoutes() const 
{
   return readLong(NBR_SUB_ROUTES_POS);
}

#endif // SUB_ROUTE_REPLY_PACKET_H
