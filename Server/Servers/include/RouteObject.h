/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTE_OBJECT_H
#define ROUTE_OBJECT_H

#include "config.h"

#include "Request.h"
#include "RoutePacket.h"
#include "CoordinateOnItemPacket.h"
#include "RouteExpandItemPacket.h"
#include "ExpandRoutePacket.h"
#include "CoordinatePacket.h"
#include "Types.h"
#include "StringTable.h"
#include "PacketContainer.h"
#include "PacketContainerTree.h"
#include "Vector.h"
#include "RouteExpander.h"
#include "ItemNamesPacket.h"
#include "CoordinateObject.h"
#include "DriverPref.h"
#include "OrigDestInfo.h"

class RouteSender;                   // forward decl
class DisturbanceList;              // forward decl
class OrigDestInfoList;            // forward decl
class ServerSubRouteVectorVector; // forward decl
class RoutingInfo;               // forward decl
class RouteAllowedMap;          // forward decl
class UnexpandedRoute;         // forward decl
class TopRegionRequest;
class RequestUserData;


/**
 *    Send route-request to the Route Module. Before that both 
 *    coordinates and IDs for the origins and destinations must be 
 *    looked up.
 *
 */
class RouteObject {
   public:

      /**
       *    Create a new Route object.
       *    @param user         The request user which will be used for
       *                        user rights.
       *    @param req          The request that this RouteObject belongs to.
       *    @param expandType   The type of expansion that should be performed.
       *                        Zero means "no expansion".
       *    @param language     The prefered language of items.
       *    @param topReq       Pointer to valid TopRegionRequest with data
       *    @param abbreviate   True if the street names should be abbreviated,
       *                        false otherwise.
       *    @param disturbances A list of disturbances to be added.
       *    @param nbrWantedRoutes The number of routes to destinations
       *                           that should be returned.
       */
      RouteObject( const RequestUserData& user,
                   Request* req,
                   uint32 expandType,
                   StringTable::languageCode language,
                   bool noConcatenate,
                   uint32 passedRoads,
                   const TopRegionRequest* topReq,
                   bool abbreviate = true,
                   const DisturbanceList* disturbances = NULL,
                   uint32 nbrWantedRoutes = MAX_UINT32,
                   bool removeAheadIfDiff = false,
                   bool nameChangeAsWP = false);

      /**
       *    Delete this route object, and release allocated memory.
       */
      virtual ~RouteObject();

      /**
       *    Checks if the other RouteObject has the same request data.
       *    @param other   The object to compare this to.
       *    @return  True if {\tt this} and {\tt other} are equal, false
       *             otherwise.
       */
      bool operator == ( const RouteObject &other ) const;

      /**
       *    Get the next packet from this object, returned in a packet
       *    container.
       *    @return A packet container that contains the next packet from
       *            this object.
       */
      PacketContainer* getNextPacket();

      /**
       *    Insert one new packet into this object.
       *    @param   pack  The packet received from the modules.
       */
      void processPacket( const PacketContainer* pack );

      /**
       *    Get the answer from this object.
       *    @return  A packet container that have the packet that is the
       *             result of the calculations done in this object.
       */
      PacketContainer* getAnswer();

      /**
       *    Get the route. If steal is true the route will be set to NULL
       *    in the RouteObject and not deleted by the RouteObject.
       *    The routes in the vector
       *    will be deleted when the vector is deleted in both cases.
       *    @param steal If true the route will <b>not</b> be deleted
       *                 by the RouteObject.
       *    @return The route as a ServerSubRouteVector.
       */
      ServerSubRouteVectorVector* getRoute(bool steal = false);

      /**
       *    Get the unexpanded route.
       *    @param steal If true the route will <b>not</b> be deleted
       *                 by the RouteObject.
       */
      UnexpandedRoute* getUnexpandedRoute(bool steal = false);

      /**
       *    Set if the streets should be abbreviated or not.
       *    @param abbreviate True if the street names should be abbreviated,
       *                      false otherwise.
       */
      void setAbbreviate(bool abbreviate);

      /**
       *    Set if there should be landmarks in the result or not.
       *    @param landmarks True if landmarks should be added to the
       *                     result, false if not.
       */
      void setLandmarks( bool landmarks );

      /**
       *    Set to true if namechanges att aheads should be removed from
       *    the expanded reply.
       *    @param landmarks True if namechanges att aheads should be removed.
       */
      void setRemoveAheadIfDiff( bool removeAheadIfDiff);

      /**
       *    Set to true if namechanges should be presented as waypoints
       *    @param landmarks True if namechanges as waypoints.
       */
      void setNameChangeAsWP( bool nameChangeAsWP);

      

      /**
       *    Set the paramters that should be used when calculating the
       *    route.
       *    @param pIsStartTime  True if the given time is start time, 
       *                         false otherwise.
       *    @param pRouteType    The type of route that should be 
       *                         calculated. DEPRECATED?!?!?
       *    @param pCostA        The amount of costA to use in calculation.
       *    @param pCostB        The amount of costB to use in calculation.
       *    @param pCostC        The amount of costC to use in calculation.
       *    @param pCostD        The amount of costD to use in calculation.
       *    @param pVehicleParam Parameters that describes the vehicle.
       *    @param pTime         The time. Start- or end time depending on
       *                         the pIsStartTime-parameter.
       *    @param pTurnCost     The cost to make a U-turn.
       *    @param avoidTollRoads If to avoid toll-roads.
       *    @param avoidHighways  If to avoid highways.
       */
      inline void setRouteParameters(bool   pIsStartTime,
                                     byte   pRouteType,
                                     byte   pCostA,
                                     byte   pCostB,
                                     byte   pCostC,
                                     byte   pCostD,
                                     uint32 pVehicleParam,
                                     uint32 pTime,
                                     uint32 pTurnCost,
                                     bool avoidTollRoads,
                                     bool avoidHighways);

      /**
       *    Returns the set routeparameters.
       *    @see setRouteParameters.
       */
      inline void getRouteParameters( bool&   pIsStartTime,
                                      byte&   pRouteType,
                                      byte&   pCostA,
                                      byte&   pCostB,
                                      byte&   pCostC,
                                      byte&   pCostD,
                                      uint32& pVehicleParam,
                                      uint32& pTime,
                                      uint32& pTurnCost ) const;

      
      /**
       *    @name Add origin.
       *    @memo Add one possible origin of the route.
       *    @doc  Add one possible origin of the route.
       */
      //@{
         /**
          *    Add one origin that is known by mapID, itemID and mabye
          *    offset. The offset is ignored if itemID not is a 
          *    StreetSegmentItem.
          *    @param mapID   The ID of the map where the item is located.
          *    @param itemID  The ID of item.
          *    @param offset  Optional parameter that is the offset on 
          *                   the street if itemID is a StreetSegmentItem. 
          *                   Otherwise ignored.
          *    @return index in the internal vector. 
          */
         inline int addOriginID(uint32  mapID,
                                uint32  itemID, 
                                uint16  offset = 0);

         /**
          *    Add one origin that is known by coordinates.
          *    @param lat     The latitude part of the coordinate for
          *                   this origin.
          *    @param lon     The longitude part of the coordinate for
          *                   this origin.
          *    @param angle   Optional paramter that is the initial heading
          *                   of the vehicle. The angle is in degrees and
          *                   calculated clockwise from the north-direction.
          *                   To use the start-direction in the routing the
          *                   routing parameter for the U-turn cost must 
          *                   be set.
          *    @return Index that can be used in getOrigin.
          */
         inline int addOriginCoord(int32   lat,
                                   int32   lon, 
                                   uint16  angle = MAX_UINT16);

         /**
          *    Add one origin that is fully known. The offset is ignored 
          *    if itemID not is a StreetSegmentItem.
          *    @param mapID   The ID of the map where the item is located.
          *    @param itemID  The ID of item.
          *    @param offset  The offset on the street if itemID is a
          *                   StreetSegmentItem. Otherwise ignored.
          *    @param lat     The latitude part of the coordinate for this 
          *                   origin.
          *    @param lon     The longitude part of the coordinate for this 
          *                   origin.
          *    @return Index that can be used in getOrigin.
          */
         inline int addOrigin(uint32  mapID,
                              uint32  itemID,
                              uint16  offset,
                              int32   lat,
                              int32   lon);
         /**
          *    Add one origin that is known either by lat/lon or itemID.
          *    Note that the vehicle of the origdestinfo will not be used.
          *    @param orig Origin to add.
          */
         inline int addOrigin(const OrigDestInfo& orig);
         
       //@}

      /**
       *    Get the number of added origins.
       *    @return The number of origins that are added.
       */
      inline uint32 getNbrOrigin() const;


      /**
       *    Get one of the added origins. The origin is returned by
       *    using outparmaters.
       *    @param i       The index of the origin to return. Valid values
       *                   are 0 <= i < getNbrOrigin().
       *    @param mapID   Outparameter that is set to the ID of the map 
       *                   where the item is located.
       *    @param itemID  Outparameter that is set to the ID of item.
       *    @param offset  Outparameter that is set to the offset on the 
       *                   street if itemID is a StreetSegmentItem.
       *    @param lat     Outparameter that is set to the latitude part 
       *                   of the coordinate for this origin.
       *    @param lon     Outparameter that is set to the longitude part 
       *                   of the coordinate for this origin.
       */
      inline void getOrigin( uint32 i,
                             uint32& mapID, uint32& itemID, 
                             uint16& offset, int32& lat, 
                             int32& lon ) const;

      /**
       * Get the origin angle for the made route, if not set 
       * MAX_UINT16 is returned.
       */
      uint16 getOriginAngle() const;

      /**
       *    @name Add destination.
       *    @memo Add one possible destination of the route.
       *    @doc  Add one possible destination of the route.
       */
      //@{
         /**
          *    Add one destination that is known by mapID, itemID and mabye
          *    offset. The offset is ignored if itemID not is a 
          *    StreetSegmentItem.
          *    @param mapID   The ID of the map where the item is located.
          *    @param itemID  The ID of item.
          *    @param offset  Optional parameter that is the offset on 
          *                   the street if itemID is a StreetSegmentItem. 
          *                   Otherwise ignored.
          *    @return Index that can be used in getDestination.
          */
         inline int addDestinationID(uint32 mapID, uint32 itemID, 
                                     uint16 offset = 0);

         /**
          *    Add one origin that is known by coordinates.
          *    @param lat  The latitude part of the coordinate for this 
          *                destination.
          *    @param lon  The longitude part of the coordinate for this 
          *                destination.
          *    @return Index that can be used in getDestination.          
          */
         inline int addDestinationCoord(int32 lat, int32 lon);

         /**
          *    Add one destination that is fully known. The offset is ignored 
          *    if itemID not is a StreetSegmentItem.
          *    @param mapID   The ID of the map where the item is located.
          *    @param itemID  The ID of item.
          *    @param offset  The offset on the street if itemID is a
          *                   StreetSegmentItem. Otherwise ignored.
          *    @param lat     The latitude part of the coordinate for this 
          *                   origin.
          *    @param lon     The longitude part of the coordinate for this 
          *                   origin.
          *    @return Index that can be used in getDestination.          
          */
         inline int addDestination(uint32 mapID, uint32 itemID, 
                                   uint16 offset,
                                   int32 lat, int32 lon);
      //@}

      /**
       *    Get the number of added destinations.
       *    @return The number of possible destinations.
       */
      inline uint32 getNbrDestination() const;

      /**
       *    Get one of the added destinations. The destination is returned 
       *    by using outparmaters.
       *    @param i       The index of the destination to return. Valid 
       *                   values are 0 <= i < getNbrDestination().
       *    @param mapID   Outparameter that is set to the ID of the map 
       *                   where the item is located.
       *    @param itemID  Outparameter that is set to the ID of item.
       *    @param offset  Outparameter that is set to the offset on the 
       *                   street if itemID is a StreetSegmentItem.
       *    @param lat     Outparameter that is set to the latitude part 
       *                   of the coordinate for this destination.
       *    @param lon     Outparameter that is set to the longitude part 
       *                   of the coordinate for this destination.
       */
      inline void getDestination( uint32 i,
                                  uint32& mapID, uint32& itemID, 
                                  uint16& offset, int32& lat, 
                                  int32& lon ) const;

      /**
       *    Find out if the answer is calculated alright, false otherwise.
       *    @return  True if done, false otherwise.
       */
      inline bool getDone();

      /**
       *    Same as getDone but more like Request.
       */
      inline bool requestDone();

      /**
       *    Get the route as a route reply packet. {\it {\bf NB!} The
       *    packet that is returned belongs to this request and must
       *    not be deleted by the caller but will be deleted when this
       *    request is deleted}.
       *    @return The route reply packet that was sent by the Route 
       *            Module. NULL will be returned upon error.
       */
      inline const RouteReplyPacket* getRouteReplyPacket();
      
      /**
       *    @name Get nbr orig/dests.
       *    @memo Get the number of valid origins and destinations.
       *    @doc  Get the number of origins that are valid. That means, 
       *          included in the calculated route. Will normaly return 1, 
       *          but if e.g. several companies are located on the same 
       *          address a grater number might be returned. {\it {\bf NB!}
       *          Each call to this method is linear in the number of 
       *          origins/destinations that are send to the route module.}
       */
      //@{
         /**
          *    Get the number of valid origins.
          *    @return The number of valid origins in the route.
          */
         inline uint32 getNbrValidOrigins() const;

         /**
          *    Get the number of valid destinations.
          *    @return The number of valid destinations in the route.
          */
         inline uint32 getNbrValidDestinations() const;
      //@}

      /**
       *    @name Get orig/dests.
       *    @memo Get the valid origins and destinations.
       *    @doc  Get the origins and destination that are valid. That 
       *          means, included in the calculated route.
       */
      //@{
         /**
          *    Get the i:th valid origin.
          *    @param i       The index of the origin to return. Valid
          *                   values are 0 <= i < getNbeValidOrigins().
          *    @param mapID   Outparameter that is set to the ID of the
          *                   map for origin number i.
          *    @param mapID   Outparameter that is set to the ID of the
          *                   item that is origin number i.
          *    @param lat     The latitude of the coordinate for this origin.
          *    @param lat     The longitude of the coordinate for this 
          *                   origin.
          *    @param name    Outparameter that is setto point to the name
          *                   of destination number i. This must not be
          *                   deleted by the caller and must not be used
          *                   after the request is deleted.
          *    @param type    Outparameter that is set to the type for
          *                   origin number i.
          *    @return  True if the outparameters are set, false 
          *             otherwise.
          */
         inline bool getValidOrigin(uint32 i, 
                                    uint32& mapID, 
                                    uint32& itemID,
                                    int32& lat,
                                    int32& lon,
                                    const char*& name,
                                    ItemTypes::itemType& type);
         
         /**
          *    Get the i:th valid destination.
          *    @param i       The index of the destination to return. 
          *                   Valid values are 
          *                   0 <= i < getNbrValidDestinations().
          *    @param mapID   Outparameter that is set to the ID of the
          *                   map for destination i.
          *    @param mapID   Outparameter that is set to the ID of the
          *                   item that is destination number i.
          *    @param lat     The latitude of the coordinate for this 
          *                   destination.
          *    @param lat     The longitude of the coordinate for this 
          *                   destination.
          *    @param name    Outparameter that is setto point to the name
          *                   of destination number i. This must not be
          *                   deleted by the caller and must not be used
          *                   after the request is deleted.
          *    @param type    Outparameter that is set to the type for
          *                   destination number i.
          *    @return  True if the outparameters are set, false 
          *             otherwise.
          */
         inline bool getValidDestination(uint32 i, 
                                         uint32& mapID, 
                                         uint32& itemID,
                                         int32& lat,
                                         int32& lon,
                                         const char*& name,
                                         ItemTypes::itemType& type);
      //@}


   

   /**
    *    Returns the original index into the m_allDest or m_allOrig-
    *    vector for the originally added orig or dest that the route
    *    ended in.
    */
   int getOriginalOrigOrDestIndex(bool dest) const;
   
   
      /**
       * @name Get names
       * @memo Get the routes origin and destination.
       * @doc  Get the origin and destination of the route. That 
       *       means, the routes starting point and destination point.
       */
      //@{
         /**
          *    Get the name of the origin point in the route. If the
          *    route calculation failed then the data is not valid.
          *    @return A pointer to the name of the origin point in the
          *            route. This must not be deleted by the caller
          *            nor be used after the request is deleted.
          */
         inline const char* getRouteOriginName() const;

         /**
          *    Get the coordinates for the origin point in the route.
          *    If the routing failed then the data is not valid.
          *    @param lat  Outparameter that is set to the latitude
          *                part of the origin point in the route.
          *    @param lon  Outparameter that is set to the longitude
          *                part of the origin point in the route.
          *    @return True if the outparamters are set, false
          *            otherwise.
          */
         inline bool getRouteOriginCoord(int32& lat, int32& lon) const;
    
         /**
          *    Get the routes origin point, if the routing failed then
          *    the data is not valid.
          *    @param mapID Outparameter that is set to the ID of the
          *                 map for the routes origin point.
          *    @param itemID Outparameter that is set to the ID of the
          *                  item that is the routes origin point.
          *    @param offset Outparameter that is set to the offset of the
          *                  item that is the routes origin point.   
          *    @param name Outparameter that is set to point to the name
          *                of the routes origin point. This must not be
          *                deleted by the caller nor be used after the 
          *                request is deleted.
          */
         inline void getRouteOrigin( uint32& mapID, 
                                     uint32& itemID,
                                     uint16& offset,
                                     const char*& name ) const;

         /**
          *    Get the name of the destination point in the route. If
          *    the route calculation failed then the data is not valid.
          *    @return A pointer to the name of the destination point
          *            in the route. This must not be deleted by the
          *            caller nor be used after the request is deleted.
          *                                                             */
         inline const char* getRouteDestinationName() const;

         /**
          *    Get the coordinates for the destination point in the
          *    route. If the routing failed then the data is not valid.
          *    @param lat  Outparameter that is set to the latitude
          *                part of the destination point in the route.
          *    @param lon  Outparameter that is set to the longitude
          *                part of the destination point in the route.
          *    @return True if the outparamters are set, false
          *            otherwise.
          */
         inline bool getRouteDestinationCoord(int32 &lat, int32 &lon) const;
         
         /**
          *    Get the routes destination point, if the routing failed 
          *    then the data is not valid.
          *    @param mapID Outparameter that is set to the ID of the
          *                 map for the routes destination point.
          *    @param itemID Outparameter that is set to the ID of the
          *                  item that is the routes destination point.
          *    @param offset Outparameter that is set to the offset of the
          *                  item that is the routes destination point.   
          *    @param name  Outparameter that is set to point to the name
          *                 of the routes destination point. 
                            This must not be
          *                 deleted by the caller nor be used after the 
          *                 request is deleted.
          */
          inline void getRouteDestination(uint32& mapID, 
                                          uint32& itemID,
                                          uint16& offset,
                                          const char*& name ) const;
      //@}
      
      /**
       *    @name Get start direction of route.
       *    Get the start-direction of the route in terms of house numbers
       *    on the street where the route start.
       */
      //@{
         /**
          *    Get the start direction in terms of driving towards higher
          *    or lower street numbers.
          *    @return  The start direction of the route in terms of    
          *             increasing or decreasing house numbers.
          */
          inline ItemTypes::routedir_nbr_t getStartDirectionHousenumber();

         /**
          *    Get the start direction in terms of driving with the odd/even
          *    numbers to the left or right.
          *    @return  The start direction of the route in terms of    
          *             odd/even street numbers to the left/right.
          */
          inline ItemTypes::routedir_oddeven_t getStartDirectionOddEven();
      //@}

      /**
       *    Removes all non input or output data from the object.
       *    Used when caching a route.
       */
      void clearTemporaryRouteData();
      
      /**
       *    The aproximate size of the object. Used when caching a route.
       *    @return The aproximate size of this object.
       */
      uint32 getSize();

      /**
       *    The routes ID. Currently returns the RequestID.
       *    @return The ID of the route.
       */
      inline uint32 getRouteID() const;

      /**
       *    The routes creationTime.
       *    @return The creation time of the route.
       */
      inline uint32 getRouteCreateTime() const;

      /**
       *    Debug method that prints data about all the members in this
       *    object to standard out.
       */
      void dumpState();

      /**
       *    Debug method that prints data 
       */
      void dumpData();

      /**
       *    Returns StringTable::OK if routing was successful.
       *    @return StringTable::OK if routing was succesful.
       *            StringTable::ERROR_TIMEOUT if a timeout occured
       *            Status code sent from RouteSender if != OK
       *            StringTable::INTERNAL_SERVER_ERROR on unexpected
       *            state or packet type.
       *            StringTable::ERROR_NO_VALID_START_ROUTING_NODE
       *            StringTable::ERROR_NO_VALID_END_ROUTING_NODE 
       *            StringTable::MAPNOTFOUND if coordinate outside map
       *            coverage.
       *            StringTable::ORIGIN_OUTSIDE_ALLOWED_AREA or
       *            StringTable::DESTINATION_OUTSIDE_ALLOWED_AREA if origin
       *            or destination is outside the user's allowed areas.
       */
      inline StringTable::stringCode getStatus() const;

      /**
       * Set the allowed maps.
       *
       * @param maps The allowed maps. NULL means all maps are allowed.
       */
      inline void setAllowedMaps( RouteAllowedMap* maps );


      /**
       * Get the prefered language.
       */
      inline StringTable::languageCode getLanguage() const;


      
      /**
       * Create a route expander for this route object.
       */
      RouteExpander* getRouteExpander(uint32 expandType);
      
      /**
       *  Restart the object at expansion.
       */
      bool readyToExpand(RouteExpander* expander);

      void useCostC()
      { 
         m_routeData.costC = 1;
         m_routeData.costB = 0; 
      }

   private:
      /**
       *    The possible states of this object.
       *
       */
      enum state_t { 
         /** 
          *    Awaiting entering of origins and destintions. The request
          *    is not yet handled to the server. 
          */
         AWAITING_ORIGDEST,


         /**
          * RoutingInfo is retreived.
          */
         ROUTINGINFO,


         /**   
          *    Sending RouteItemExpandRequestPackets or 
          *    CoordinateRequestPackets to the map module.
          */
         SENDING_ITEM_EXPAND,

         /**
          *    Waiting for RouteItemExpandReplyPackets or 
          *    CoordinateReplyPackets from the map module.
          */
         RECEIVING_ITEM_EXPAND,

         /// Sending the route-request packet to the route module.
         SENDING_ROUTE,

         /// Waiting for the route-reply packet from the route module.
         RECEIVING_ROUTE,

         /**
          *    Sending the ExpandRouteRequestPackets to the map module.
          *    Let the ExpandRouteObject do the actual work.
          */
         SENDING_EXPAND_ROUTE,

         /**
          *    Wait for expand route reply packet. The packets are handled
          *    direct to the RouteExpander.
          */
         RECEIVING_EXPAND_ROUTE,

         ///   Send ItemNamesRequestPackets to the map module.
         SENDING_NAME_REQUEST,

         ///   Wait for ItemNamesReplyPackets from the map module.
         RECEIVING_NAME_REQUEST,

         ///   All processing completed.
         DONE,

         /**   
          *    Something whent wrong when processing any part of the 
          *    task for this object.
          */
         ERROR};


      /**
       *    This struct describes the origin or destination in the
       *    route that is returned from the route module.
       */
      struct origDestFinalPoint_t {
         uint32 mapID;
         uint32 itemID;
         uint16 offset;
         char* itemName;
      };

      /**
       *    Objects of this class describes one origin or destination that
       *    is inserted by the creator of this object.
       */
      class OrigDestPoint {
         public: 
            /**
             *    Create an empty objects. All members are set to default 
             *    values.
             */
            OrigDestPoint() {
               lat = lon = MAX_INT32;
               mapID = itemID = MAX_UINT32;
               offset = 0;
               startAngle = packetID = MAX_UINT16;
               itemName = NULL;
               coordinatePos = MAX_UINT16;
               processed = false;
            };

            OrigDestPoint(int32 pLat, int32 pLon) 
            {
               lat = pLat;
               lon = pLon;
               startAngle = MAX_UINT16;
               mapID = itemID = MAX_UINT32;
               offset = 0;
               routingType = MAX_BYTE;
               packetID = MAX_UINT16;
               itemName = NULL;
               coordinatePos = MAX_UINT16;
               processed = false;
            };

            OrigDestPoint(uint32 pMapID, uint32 pItemID, uint16 pOffset) 
            {
               lat = lon = MAX_INT32;
               mapID = pMapID;
               itemID = pItemID;
               offset = pOffset;
               routingType = MAX_BYTE;
               startAngle = packetID = MAX_UINT16;
               itemName = NULL;
               coordinatePos = MAX_UINT16;
               processed = false;
            };

            OrigDestPoint(int32 pLat, int32 pLon,
                          uint32 pMapID, uint32 pItemID, uint16 pOffset)
            {
               lat = pLat;
               lon = pLon;
               mapID = pMapID;
               itemID = pItemID;
               offset = pOffset;
               routingType = MAX_BYTE;
               startAngle = packetID = MAX_UINT16;
               itemName = NULL;
               coordinatePos = MAX_UINT16;
               processed = true;
            };

            virtual ~OrigDestPoint() { delete [] itemName;};

            ///   equal
            bool virtual operator == (const OrigDestPoint& elm) const {
               return (mapID == elm.mapID );
            }

            ///   not equal
            bool virtual operator != (const OrigDestPoint& elm) const {
               return (mapID != elm.mapID);
            }
            
            ///   greater
            bool virtual operator > (const OrigDestPoint& elm) const {
               return (mapID > elm.mapID);
            }
            
            ///   less
            bool virtual operator < (const OrigDestPoint& elm) const {
               return (mapID < elm.mapID);
            }
            
            /**
             *    Find out if it is necessary to exapand the coordinate
             *    to ID for this OrigDestPoint.
             */
            bool expandCoordinate() {
               return (lat != MAX_INT32) && (mapID == MAX_UINT32);
            }

            /**
             *    Find out if it is necessary to exapand the ID to
             *    coordinate for this OrigDestPoint.
             */
            bool expandID() {
               return (mapID != MAX_UINT32) && (lat == MAX_INT32);
            }

            int32 lat;
            int32 lon;
            uint32 mapID;
            uint32 itemID;
            uint16 offset;
            uint16 startAngle;
            uint16 coordinatePos;
            uint16 packetID;

            byte routingType;
            char* itemName;
            bool processed;

            void dump() {
               cout << "(" << lat << "," << lon << "), ID=" << mapID 
                    << "." << itemID << "." << offset 
                    << ", packetID=" << packetID << ", coordPos=" 
                    << coordinatePos << "processed=" << processed;
               if (itemName != NULL) {
                  cout << "m itemName=" << itemName;
               }
               cout << endl;
            }
      };

   
   class OrigDestDescription;
   typedef std::vector < OrigDestPoint* > OrigDestPointVector;
   typedef std::vector < OrigDestDescription* > OrigDestDescVector;

      /**
       *    Objects of this class describes one of the possible 
       *    routeable origins or destinations that will be used in route
       *    calculation.
       *
       */
      class OrigDestDescription {
         public: 
            OrigDestDescription(int32 pLat, int32 pLon) 
            {
               lat = pLat;
               lon = pLon;
               mapID = parentItemID = itemID = MAX_UINT32;
               offset = origDestIndex = MAX_UINT16;
               bDirectionFromZero = false;
               bDirectionFromZeroSet = false;
               validOrigDest = false;
               routingType = MAX_BYTE;
               itemType = ItemTypes::nullItem;
               itemName = NULL;
            };

            OrigDestDescription(uint32 pMapID,
                                uint32 pItemID,
                                uint16 pOffset,
                                ItemTypes::itemType type) 
            {
               lat = lon = MAX_INT32;
               mapID = pMapID;
               itemID = pItemID;
               offset = pOffset;
               parentItemID = MAX_UINT32;
               origDestIndex = MAX_UINT16;
               bDirectionFromZero = false;
               bDirectionFromZeroSet = false;
               validOrigDest = false;
               routingType = MAX_BYTE;
               itemType = type;
               itemName = NULL;
            };

            OrigDestDescription(int32 pLat,
                                int32 pLon,
                                uint32 pMapID,
                                uint32 pItemID,
                                uint16 pOffset,
                                ItemTypes::itemType type)
            {
               lat = pLat;
               lon = pLon;
               mapID = pMapID;
               itemID = pItemID;
               offset = pOffset;
               parentItemID = MAX_UINT32;
               origDestIndex = MAX_UINT16;
               bDirectionFromZero = false;
               bDirectionFromZeroSet = false;
               validOrigDest = false;
               routingType = MAX_BYTE;
               itemType = type;
               itemName = NULL;
            };

            virtual ~OrigDestDescription() { delete [] itemName;};

            int32 lat;
            int32 lon;
            uint32 mapID;
            uint32 parentItemID;
            uint32 itemID;
            uint16 offset;
            uint16 origDestIndex;
            bool bDirectionFromZeroSet;
            bool bDirectionFromZero;
            bool validOrigDest;
            byte routingType;
            ItemTypes::itemType itemType;
            char* itemName;
            void dump() {
            }
            
            /*   cout << "(" << lat << "," << lon << ") ID=" << mapID << "."
                    << itemID << "." << offset << ", parentID=" 
                    << parentItemID << " origDestIndex=" << origDestIndex
                    << " validOrigDest=" << validOrigDest
                    << " itemType="  << (int)itemType;
               if (itemName != NULL) {
                  cout << " itemName=" << itemName;
               }
               cout << endl;
               }*/
      };

      /**
       *    Create the packets that should be send to the route module
       *    to get the coordinates and routeable itemIDs for the entered
       *    items.
       *    @return The next state of this object.
       */
      state_t createAndPrepareExpandItemPackets();

      /**
       *    Create the packet that should be send to the route module.
       *    @return The next state of this object.
       *    @param checkProcessed Optional parameter, that if set to true
       *                          will make this method check if all 
       *                          OrigDestPoints are processed.
       */
      state_t createAndPrepareRoutePacket(bool checkProcessed = false);

      /**
       *    Create the packets that should be send to the map module to
       *    get the names of the valid origins and destinations.
       *    @return The next state of this object.
       */
      state_t createAndPrepareNamePackets();
      
      /**
       *    Handle one incomming route expand item reply packet. This packet
       *    is used to fill the m_*Descriptions-arrays.
       *    @param p The packet that is processed by the map module.
       *    @return The next state of this object.
       */
      state_t handleRouteExpandItemReplyPacket(RouteExpandItemReplyPacket* p);

      /**
       *    Handle one incomming CoordinateReplyPacket. The mapID, itemID
       *    andoffset are inserted into the m_origDescription- and 
       *    m_destDescription arrays.
       *    @param p The packet that is processed by the map module.
       *    @return The next state of this object.
       */
      state_t handleCoordinateReplyPacket(const PacketContainer* pc);

      /**
       *    Handle one incomming route reply packet. This packet is 
       *    passed to the expander.
       *    @param p The packet that is processed by the route module.
       *    @return The next state of this object.
       */
      state_t handleRouteReplyPacket(RouteReplyPacket* p);
      
      /**
       *    Handle in incomming ItemNamesReplyPacket. The names are 
       *    inserted into the m_origDescription and m_destDescription
       *    arrays.
       *    @param p The packet that is processed by the map module.
       *    @return The next state of this object.
       */
      state_t handleNameReplyPacket(ItemNamesReplyPacket* p);

      /**
       *    Set the state of this object to ERROR and create a dummy
       *    ExpandRouteReplyPacket that will be returned as answer.
       *    @param stringCode Optional parameter with the string-code 
       *                      that describes the reason for trouble.
       */
      state_t setErrorState( StringTable::stringCode stringCode );

      /**
       *    Calculates the number of routes to tell the RouteSender
       *    to calculate to be able to return the specified number
       *    of routes. The number is calculated by taking the
       *    <code>nbrWantedResults</code> destinations which expand to
       *    the largest number of SSI:s and sum the number of SSI:s
       *    for them. The value is then multiplied with 2, since we
       *    use both node0 and node1.
       *    @param nbrWantedResults The final wanted number of results.
       *    @return The number of routes to calculate.
       */
      uint32 calcNbrWantedRoutes(uint32 nbrWantedResults);

      /**
       *    Removes routes to destinations that are from the same
       *    original destinations.
       *    @param srvv The route result.
       *    @param nbrWanted The number of wanted results.
       */
       void fixupRouteResult(ServerSubRouteVectorVector* srvv,
                             uint32 nbrWanted);
   
      /**
       *    Calculates an initial turncost for nodes to be used
       *    when the start direction is important.
       *    @param nodeID The node id of the node to add cost to.
       *    @return The turncost.
       */
      static uint32 getTurnCost(uint32 nodeID);
      
      /**
       *    Compares <code>origDest</code> to <code>finalOrigDest</code>
       *    and returns true if they refer to the same point.
       *
       */
      static bool checkSame(OrigDestDescription* origDest,
                            origDestFinalPoint_t& finalOrigDest);

      /**
       *    Checks if it is allowed to use the current vehicle 
       *    to route from the origins to the destinations, i.e.
       *    checks if it is too far to walk.
       *    @param dp The driver preferences.
       *    @param origList List of origins.
       *    @param destList List of destinations.
       *    @return True if the vehicle is allowed to be 
       *            used for the shortest distance between the
       *            origins and destinations.
       */
      static bool checkDistOKForVehicle(const DriverPref& dp,
                                        const OrigDestInfoList& origList,
                                        const OrigDestInfoList& destList);

      /**
       *   Returns the minimum distance from the origins to the dests.
       *   @param origs The origin list.
       *   @param dests The destination list.
       *   @return The minimum distance from the origs to the dests.
       */
      static int calcMinDistFromOrigsToDest(const OrigDestInfoList& origs,
                                            const OrigDestInfoList& dests);

      /**
       *   Returns the index in the origDestPoints vector of the 
       *   originally added origin or dest that is in the route.
       */
      static int getOriginalIndex(const OrigDestDescVector& origDestDescriptions,
                                  const OrigDestPointVector& origDestPoints,
                                  const origDestFinalPoint_t&
                                  finalOrigOrDest );
   
      /**
       *    Returns the index in the m_allDest vector for the originally
       *    added destination that the route ended with.
       */
      int getOriginalDestIndex() const;
   
      /**
       *    Returns the index in the m_allOrigs vector for the originally
       *    added origin that the route ended with.
       */
      int getOriginalOrigIndex() const;

      /**
       *    @name Inserted endpoints.
       *    @memo The origins and destinations that are inserted into 
       *          this object.
       *    @doc  Arrays with the data that is entered by the user and
       *          inserted into this object. The IDs might refer to
       *          streets, street segment items, companies or categories.
       */
      //@{
         /// The inserted origins.
         OrigDestPointVector m_allOrigs;
      
         /// The inserted origins, with maintained order.
         OrigDestPointVector m_allOrigsSafe;

         /// The inserted destinations.
         OrigDestPointVector m_allDests;

         /// The inserted destinations, with maintained order.
         OrigDestPointVector m_allDestsSafe;
      //@}
      
      /**
       *    @name Routeable endpoints.
       *    @memo The routeable origins and destinations.
       *    @doc  Arrays with the expanded endpoints (from m_allOrigs and
       *          m_allDests). This must be routeable items, that means
       *          that the IDs might only refer to street segment items.
       */
      //@{
         /// The routeable origins.
         OrigDestDescVector m_origDescriptions;

         /// The routeable destinations.
         OrigDestDescVector m_destDescriptions;
      //@}

      /**
       *    @name Endpoints
       *    @memo The origin and destination in the calculated route.
       *    @doc  One meber for the origin in the calculated route and
       *          one for the destination. These two might also be found
       *          in the m_origDescriptions and m_destDescriptions as the
       *          elements that have the validOrigDest-variable set to
       *          true.
       */
      //@{
         /// The origin in the caluclated route.
         origDestFinalPoint_t m_origPoint;

         /// The destination in the caluclated route.
         origDestFinalPoint_t m_destPoint;
      //@}
         
      /**
       *    Containers with the packets that are ready to be processed
       *    by the modules.
       */
      PacketContainerTree m_packetsReadyToSend;

      /**
       *    The parameters that describes how to calculate the route. 
       *    These are inserted into the route request packet that is
       *    sent to the Route Module.
       */
      struct { 
         bool isStartTime;
         byte routeType;
         byte costA;
         byte costB;
         byte costC;
         byte costD;
         uint32 vehicleParam;
         uint32 time;
         uint32 turnCost;
      } m_routeData;
      
      /**
       *    The current state of this object.
       */
      state_t m_state;

      /**
       *    The type of route expansion that should be performed by the 
       *    map module.
       */
      uint32 m_expandType;

      /**
       *    A packet that contains the expanded route.
       */
      PacketContainer* m_answer;

      /**
       *    The calculated route. Stored to be able to insert it into the
       *    route-cache. Might be NULL if no route is calculated.
       */
      RouteReplyPacket* m_routeReplyPacket;

      /**
       *    The request that this object belongs to. This is used to get
       *    the request ID and to get the next valid packet ID.
       */
      Request* m_request;

      /**
       *    The object that does routing between a group of origins and
       *    a group of destinations.
       */
      RouteSender* m_routeSender;

      /**
       *    The expander that sends expand route request packets to the
       *    map module and concatenates the replies.
       */
      RouteExpander* m_routeExpander;

      CoordinateObject m_coordinateObject;

      /**
       *    The time of the created route. Used to identify routes together 
       *    with routeID.
       */
      uint32 m_createTime;
      
      /**
       *    The prefered language of items.
       */
      StringTable::languageCode m_language;

      /**
       *    True if the street names should be abbreviated, false otherwise.
       */
      bool m_abbreviate;

      /**
       * If landmarks should be in the result.
       */
      bool m_landmarks;

      /**
       *   Object to keep track of the customer parameters.
       *   Not fully used yet, but will be used when getting
       *   the SubRouteList from the RoutePacket for now.
       */
      DriverPref* m_driverPref;

      /**
       *   Pointer to the driverpref if driverpref
       *   was created by RouteObject. NULL otherwise.
       */
      DriverPref* m_driverPrefToDelete;

      /**
       *    The current status of the routing
       */
      StringTable::stringCode m_status;

      /**
       *    The disturbances that should be added to the sent packets.
       */
      const DisturbanceList* m_disturbances;
      
      /**
       *    True if no concatenations should be used (Debug-method)
       */
      bool m_noConcatenate;
      
      /**
       *    The number of passed roads to include as landmarks.
       */
      uint32 m_passedRoads;

      
      /**
       *    True if no Aheads should be removed at name changes.
       */
      bool m_removeAheadIfDiff;


      /**
       *  If road namechanges should be included as waypoints(for Wayfinder).
       */
      bool m_nameChangeAsWP;
      
      
      // Containers that holds the information about nodes.
      // These are used to send to RouteSender.
      OrigDestInfoList* m_origList;
      OrigDestInfoList* m_destList;

      /**
       *  The number of wanted destinations if more than one
       *  destination is given.
       */
      uint32 m_nbrWantedRoutes;

      /**
       *   The non-expanded route result is here.
       */
      ServerSubRouteVectorVector* m_routeResultVector;

      /**
       *   One unexpanded route is here.
       */
      UnexpandedRoute* m_unexpandedRoute;

      /**
       *   True if ItemNamesrequest should be sent.
       */
      bool m_sendItemNames;

      /**
       *   The number of outstanding itemname packets.
       */
      int m_nbrItemNamesToReceive;

      /**
       *   Pointer to structure containing information about the
       *   number of map levels etc. Used by the RouteSender.
       */
      RoutingInfo* m_routingInfo;

      /**
       *   Start time for the routing step.
       */
      uint32 m_routingStartTime;

      /**
       * The allowed maps.
       */
      RouteAllowedMap* m_allowedMaps;

      /**
       *   User - owned by the RouteRequest.
       */
      const RequestUserData& m_user;
};


// ========================================================================
//                                  Implementation of the inlined methods =

inline const RouteReplyPacket* 
RouteObject::getRouteReplyPacket() {
   return (m_routeReplyPacket);
}

inline uint32 
RouteObject::getNbrValidOrigins() const
{
   uint32 retVal = 0;
   for (uint32 j=0; j<m_origDescriptions.size(); j++) {
      if (m_origDescriptions[j]->validOrigDest) {
         retVal++;
      }
   }
   return (retVal);
}

inline uint32 
RouteObject::getNbrValidDestinations() const
{
   uint32 retVal = 0;
   for (uint32 j=0; j<m_destDescriptions.size(); j++) {
      if (m_destDescriptions[j]->validOrigDest) {
         retVal++;
      }
   }
   return (retVal);
}

inline bool 
RouteObject::getValidOrigin(uint32 i, 
                            uint32& mapID, 
                            uint32& itemID,
                            int32& lat,
                            int32& lon,
                            const char*& name,
                            ItemTypes::itemType& type)
{
   // Read past the i first valid origins
   uint32 curValid = 0;
   uint32 pos = 0;
   while ( (curValid < i) && 
           (pos < m_origDescriptions.size()) ) {
      if (static_cast<OrigDestDescription*>
                     (m_origDescriptions[pos])->validOrigDest) {
         curValid++;
      }
      pos++;
   }

   if (pos < m_origDescriptions.size()) {
      OrigDestDescription* odd = static_cast<OrigDestDescription*>
                                            (m_origDescriptions[pos]);
      mapID = odd->mapID;
      if (odd->parentItemID != MAX_UINT32) {
         itemID = odd->parentItemID;
      } else {
         itemID = m_origPoint.itemID;
      }
      if ( (odd->itemName != NULL) &&
           (strlen(odd->itemName) > 0)) {
         name = odd->itemName;
      } else {
         name = m_origPoint.itemName;
      }
      // Check name for NULL
      if ( name == NULL ) {
         name = "";
      }
      lat = odd->lat;
      lon = odd->lon;
      type = odd->itemType;
      return (true);
   } else {
      mapID = MAX_UINT32;
      itemID = MAX_UINT32;
      name = NULL;
      return (false);
   }
}

inline bool 
RouteObject::getValidDestination(uint32 i, 
                                 uint32& mapID, 
                                 uint32& itemID,
                                 int32& lat,
                                 int32& lon,
                                 const char*& name,
                                 ItemTypes::itemType& type)
{
   // Read past the i first valid destinations
   int curValid = -1;
   uint32 pos = 0;
   bool found = false;
   while ( (!found) && 
           (pos < m_destDescriptions.size()) ) {
      if (static_cast<OrigDestDescription*>
                     (m_destDescriptions[pos])->validOrigDest) {
         curValid++;
      }
      if (curValid == int(i)) {
         found = true;
      } else {
         pos++;
      }
   }

   if (found) {
      OrigDestDescription* odd = static_cast<OrigDestDescription*>
                                            (m_destDescriptions[pos]);
      mapID = odd->mapID;
      if (odd->parentItemID != MAX_UINT32) {
         itemID = odd->parentItemID;
      } else {
         itemID = m_destPoint.itemID;
      }
      if ( (odd->itemName != NULL) &&
           (strlen(odd->itemName) > 0)) {
         name = odd->itemName;
      } else {
         name = m_destPoint.itemName;
      }
      // Check name for NULL
      if ( name == NULL ) {
         name = "";
      }
      lat = odd->lat;
      lon = odd->lon;
      type = odd->itemType;
      return (true);
   } else {
      mapID = MAX_UINT32;
      itemID = MAX_UINT32;
      name = NULL;
      return (false);
   }
}


inline int
RouteObject::addOriginID(uint32 mapID, uint32 itemID, uint16 offset) 
{
   OrigDestPoint* odp = new OrigDestPoint(mapID, itemID, offset);
   m_allOrigs.push_back(odp);
   m_allOrigsSafe.push_back(odp);
   return m_allOrigs.size() - 1;
}

inline int
RouteObject::addOriginCoord(int32 lat, int32 lon, uint16 angle)
{
   OrigDestPoint* odp = new OrigDestPoint(lat, lon);
   odp->startAngle = angle;
   m_allOrigs.push_back(odp);
   m_allOrigsSafe.push_back(odp);
   mc2dbg8 << "RO:Added to m_allOrigs with id=" << odp->mapID 
          << '.' << odp->itemID << '.' << odp->offset << " (" << odp->lat
          << ',' << odp->lon << ")" << " angle = "
          << uint32(angle) << endl;
   return m_allOrigs.size() - 1;
}

inline int
RouteObject::addOrigin(uint32 mapID, uint32 itemID, uint16 offset,
                       int32 lat, int32 lon)
{
   OrigDestPoint* odp = new OrigDestPoint(lat, lon, mapID, itemID, offset);
   m_allOrigs.push_back(odp);
   m_allOrigsSafe.push_back(odp);
   return m_allOrigs.size() - 1;
}

inline int
RouteObject::addOrigin(const OrigDestInfo& orig)
{
   if ( orig.getMapID() == MAX_UINT32 ) {
      return addOriginCoord(orig.getLat(), orig.getLon(), orig.getAngle());
   } else {
      return addOriginID(orig.getMapID(),
                         orig.getNodeID(),
                         uint16(orig.getOffset()*MAX_UINT16));
   }
}

inline uint32 
RouteObject::getNbrOrigin() const {
   return m_allOrigs.size();
}


inline void 
RouteObject::getOrigin( uint32 i,
                        uint32& mapID, uint32& itemID, 
                        uint16& offset, int32& lat, int32& lon ) const
{
   if ( i < m_allOrigs.size() ) {
      const OrigDestPoint* odp = m_allOrigs[ i ];
      mapID = odp->mapID;
      itemID = odp->itemID;
      offset = odp->offset;
      lat = odp->lat;
      lon = odp->lon;      
   } else {
      MC2ERROR2( "RouteObject::getOrigin index out of bounds!",
                 cerr << "i " << i << " size " << m_allOrigs.size() 
                 << endl;);
   }
}

inline uint16
RouteObject::getOriginAngle() const {
   uint16 angle = MAX_UINT16;
   int i = getOriginalOrigIndex();
   if ( i >= 0 && i < int(m_allOrigs.size()) ) {
      angle = m_allOrigs[ i ]->startAngle;
   }
   return angle;
}

inline int
RouteObject::addDestinationID(uint32 mapID, uint32 itemID, uint16 offset) 
{
   OrigDestPoint* odp = new OrigDestPoint(mapID, itemID, offset);
   m_allDests.push_back(odp);
   m_allDestsSafe.push_back(odp);
   mc2dbg8 << "RO:Added to m_allDests with id=" << odp->mapID 
          << '.' << odp->itemID << '.' << odp->offset << " ("
          << odp->lat << ',' << odp->lon << ')' << endl;
   return m_allDests.size() - 1;
}

inline int
RouteObject::addDestinationCoord(int32 lat, int32 lon)
{
   OrigDestPoint* odp = new OrigDestPoint(lat, lon);
   m_allDests.push_back(odp);
   m_allDestsSafe.push_back(odp);
   mc2dbg8 << "RO:Added to m_allDests with id=" << odp->mapID 
          << '.' << odp->itemID << '.' << odp->offset << " (" << odp->lat
          << ',' << odp->lon << ")" << endl;
   return m_allDests.size() - 1;
}

inline int
RouteObject::addDestination(uint32 mapID, uint32 itemID, uint16 offset,
                            int32 lat, int32 lon)
{
   OrigDestPoint* odp = new OrigDestPoint(lat, lon, mapID, itemID, offset);
   m_allDests.push_back(odp);
   m_allDestsSafe.push_back(odp);
   mc2dbg8 << "RO:Added to m_allDests with id=" << odp->mapID 
          << "." << odp->itemID << '.' << odp->offset << " ("
          << odp->lat << ',' << odp->lon << ')' << endl;
   return m_allDests.size() - 1;
}


inline uint32 
RouteObject::getNbrDestination() const {
   return m_allDests.size();
}


inline void 
RouteObject::getDestination( uint32 i,
                             uint32& mapID, uint32& itemID, 
                             uint16& offset, int32& lat, int32& lon ) const
{
   if ( i < m_allDests.size() ) {
      const OrigDestPoint* ddp = m_allDests[ i ];
      mapID = ddp->mapID;
      itemID = ddp->itemID;
      offset = ddp->offset;
      lat = ddp->lat;
      lon = ddp->lon;      
   } else {
      MC2ERROR2( "RouteObject::getDestination index out of bounds!",
                 cerr << "i " << i << " size " << m_allDests.size() 
                 << endl;);
   }
}


inline void
RouteObject::setRouteParameters(bool pIsStartTime,
                                byte pRouteType,
                                byte pCostA,
                                byte pCostB,
                                byte pCostC,
                                byte pCostD,
                                uint32 pVehicleParam,
                                uint32 pTime,
                                uint32 pTurnCost,
                                bool avoidTollRoads,
                                bool avoidHighways)
{
   m_routeData.isStartTime = pIsStartTime;
   m_routeData.routeType = pRouteType;
   m_routeData.costA = pCostA;
   m_routeData.costB = pCostB;
   m_routeData.costC = pCostC;
   m_routeData.costD = pCostD;
   m_routeData.vehicleParam = pVehicleParam;
   m_routeData.time = pTime;
   m_routeData.turnCost = pTurnCost;
   // DriverPref should hold all the preferences in the future.
   delete m_driverPrefToDelete;
   m_driverPref = new DriverPref();
   m_driverPrefToDelete = m_driverPref;
   m_driverPref->setVehicleRestriction(pVehicleParam);
   m_driverPref->setRoutingCosts(pCostA, pCostB, pCostC, pCostD);   
   m_driverPref->setUturn( pTurnCost != 0 );
   if ( pTurnCost != 0 && pVehicleParam == ItemTypes::pedestrian ) {
      mc2dbg << "[RO]: Will not use turncost for pedestrian" << endl;
      m_driverPref->setUturn( 0 );
   }
   m_driverPref->setAvoidTollRoads( avoidTollRoads );
   m_driverPref->setAvoidHighways( avoidHighways );
   mc2dbg8 << "pTurnCost = " << pTurnCost << ", driverpref->useuturn = "
          << m_driverPref->useUturn() << endl;
}


inline void 
RouteObject::getRouteParameters( bool& pIsStartTime,
                                 byte& pRouteType,
                                 byte& pCostA,
                                 byte& pCostB,
                                 byte& pCostC,
                                 byte& pCostD,
                                 uint32& pVehicleParam,
                                 uint32& pTime,
                                 uint32& pTurnCost ) const
{
   pIsStartTime = m_routeData.isStartTime;
   pRouteType = m_routeData.routeType;
   pCostA = m_routeData.costA;
   pCostB = m_routeData.costB;
   pCostC = m_routeData.costC;
   pCostD = m_routeData.costD;
   pVehicleParam = m_routeData.vehicleParam;
   pTime = m_routeData.time;
   pTurnCost = m_routeData.turnCost;
}


inline bool 
RouteObject::getDone()
{
   return ( (m_state == DONE) || (m_state == ERROR));
}

inline bool
RouteObject::requestDone()
{
   return getDone();
}

inline const char* 
RouteObject::getRouteOriginName() const
{
   return (m_origPoint.itemName);
}

inline bool 
RouteObject::getRouteOriginCoord(int32& lat, int32& lon) const
{
    if (m_origDescriptions.size() > 0) {
      OrigDestDescription* desc = m_origDescriptions[0];
      // Find any of the descritioins that have the correct ID's
      uint32 pos = 0;
      while ( (desc != NULL) &&
              (desc->mapID != m_origPoint.mapID) &&
              (desc->itemID != m_origPoint.itemID)) {
         pos++;
         if( pos < m_origDescriptions.size() ) {
            desc = m_origDescriptions[pos];
         } else {
            desc = NULL;
         }
      }
      
      // update and return
      if (desc != NULL) {
         lat = desc->lat;
         lon = desc->lon;
         return (true);
      }
   }

   // Something is wrong, no descriptions
   return (false);
}


inline void
RouteObject::getRouteOrigin( uint32& mapID, 
                             uint32& itemID,
                             uint16& offset,
                             const char*& name ) const
{
   mapID = m_origPoint.mapID;
   itemID = m_origPoint.itemID;
   offset = m_origPoint.offset;
   name = m_origPoint.itemName;

}


inline const char* 
RouteObject::getRouteDestinationName() const
{
   return (m_destPoint.itemName);
}


inline bool 
RouteObject::getRouteDestinationCoord(int32& lat, int32& lon) const
{
   if (m_destDescriptions.size() > 0) {
      OrigDestDescription* desc = m_destDescriptions[0];
      // Find any of the descritioins that have the correct ID's
      uint32 pos = 0;
      while ( (desc != NULL) &&
              (desc->mapID != m_destPoint.mapID) &&
              (desc->itemID != m_destPoint.itemID)) {
         pos++;
         if( pos < m_destDescriptions.size() ) {
            desc = m_destDescriptions[pos];
         } else {
            desc = NULL;
         }
      }
      
      // update and return
      if (desc != NULL) {
         lat = desc->lat;
         lon = desc->lon;
         return (true);
      }
   }

   // Something is wrong, no descriptions
   return (false);
}

inline void
RouteObject::getRouteDestination( uint32& mapID, 
                                  uint32& itemID,
                                  uint16& offset,
                                  const char*& name ) const
{
   mapID = m_destPoint.mapID;
   itemID = m_destPoint.itemID;
   offset = m_destPoint.offset;
   name = m_destPoint.itemName;   
}


inline ItemTypes::routedir_nbr_t 
RouteObject::getStartDirectionHousenumber()
{
   if (m_routeExpander != NULL) {
      return m_routeExpander->getStartDirectionHousenumber();
   }
   return ItemTypes::unknown_nbr_t;
}

inline ItemTypes::routedir_oddeven_t 
RouteObject::getStartDirectionOddEven()
{
   if (m_routeExpander != NULL) {
      return m_routeExpander->getStartDirectionOddEven();
   }
   return ItemTypes::unknown_oddeven_t;
}

inline uint32 
RouteObject::getRouteID() const {
   return m_request->getID();
}


inline uint32 
RouteObject::getRouteCreateTime() const {
   return m_createTime;
}

inline StringTable::stringCode
RouteObject::getStatus() const
{
   return m_status;
}

inline void 
RouteObject::setAllowedMaps( RouteAllowedMap* maps ) {
   m_allowedMaps = maps;
}


inline StringTable::languageCode
RouteObject::getLanguage() const {
   return m_language;
}


#endif





