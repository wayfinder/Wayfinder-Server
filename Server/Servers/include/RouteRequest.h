/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTEREQUEST_H
#define ROUTEREQUEST_H

#include "config.h"

#include "StringTable.h"
#include "Request.h"
#include "RouteTypes.h"
#include "ItemTypes.h"
#include "RequestUserData.h"

class DisturbanceList;
class RouteObject;
class ExpandedRoute;
class RouteRequestPacket;
class ServerSubRouteVectorVector;
class RouteReplyPacket;
class PacketContainer;
class RouteRequestData;
class SearchMatch;
class RouteExpander;
class DisturbanceDescription;
class TopRegionRequest;
class RouteRequestParams;
//#ifdef DEBUG_LEVEL_1
//#define CALCULATE_REQUEST_STATISTICS
//#endif

/**
 *    Calculate a route between the origin and destinations. The
 *    route is also expanded into text and/or coordinate 
 *    representation.
 *    
 */
class RouteRequest : public RequestWithStatus {
public:
   RouteRequest( const RequestUserData& user,
                 uint16 reqID,
                 const RouteRequestParams& params,
                 const TopRegionRequest* topReq,
                 const DisturbanceList* disturbances,
                 Request* parentRequest );
   /** 
    *    Create a RouteRequest that can be filled with origdests.
    *    @param user       The request user which will be used for
    *                      user rights.
    *    @param reqID      The unique request ID for this request.
    *    @param expandType The type of expansion that should be done
    *                      by the map module on the caclulated route.
    *                      Valid values could be found in
    *                      ExpandRoutePacket.
    *    @param   language      The prefered language of items.
    *    @param   topReq        Pointer to valid TopRegionRequest with data
    *    @param   disturbances  A list of disturbances to be sent to the
    *                           RouteModule.
    *    @param   parentRequest If <code>parentRequest</code> != NULL
    *                           the requestID and packetid:s will come
    *                           from that Request instead of this and
    *                           the packetcontainers in getNextPacket
    *                           will not be deleted.
    *    @param nbrWantedRoutes The number of routes to destinations
    *                           that should be returned.
    */
   RouteRequest( const RequestUserData& user,
                 uint16 reqID,
                 uint32 expandType,
                 StringTable::languageCode language,
                 bool noConcatenate,
                 uint32 passedRoads,
                 const TopRegionRequest* topReq,
                 const DisturbanceList* disturbances = NULL,
                 Request* parentRequest = NULL,
                 uint32 nbrWantedRoutes = MAX_UINT32 );
   
   /**
    *    Delete this request, including allocated packets.
    */
   virtual ~RouteRequest();
   
   /**
    * Extract the necesary parameters into a RouteRequestParams object
    * that can be used to create other, similar RouteRequests.
    * @param params The RouteRequestParams object.
    */
   void extractParams(RouteRequestParams& params) const;

   /**
    *    Checks if the other RouteRequest has the same request data.
    */
   bool operator == ( const RouteRequest &other ) const;
   
   /**
    *    Return a packet container with the next packet that should 
    *    be processed by the modules.
    *    @return  A packet container with the next packet that should
    *             be processed by the modules.
    */
   PacketContainer* getNextPacket();
   
   /**
    *    Take care of one packet that is processed by the modules.
    *    @param pack A packet container with a packet that have
    *                been processed by the modules.
    */
   void processPacket(PacketContainer* pack);
   
   /**
    *    Return the found path between the origins and destinations.
    *    The answer is returned as a packet container with a 
    *    ExpandRouteReplyPacket.
    *    @return A packet container with a ExpandRouteReplyPacket.
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
    *    Set the route calculation parameters.
    *    @param pIsStartTime  True if the given time is start time.
    *    @param pCostA        The ratio for costA in calculations.
    *    @param pCostB        The ratio for costB in calculations.
    *    @param pCostC        The ratio for costC in calculations.
    *    @param pCostD        The ratio for costD in calculations.
    *    @param pVehicleParam The type of vehicle used.
    *    @param pTime         The time (start-time).
    *    @param pTurnCost     The cost to make a u-turn at the origin.
    *    @param pAbbreviate   Should the street names be abbreviated or
    *                         not. If true and swedish then "Baravägen"
    *                         will be written as "Barav.".
    *    @param landmarks     Should landmarks be added to the result.
    *    @param avoidTollRoads If toll roads should be avoided, 
    *                          default false.
    *    @param avoidHighways  If highway roads should be avoided,
    *                          default false.
    */
   void setRouteParameters(bool pIsStartTime,
                           byte pCostA,
                           byte pCostB,
                           byte pCostC,
                           byte pCostD,
                           uint32 pVehicleParam,
                           uint32 pTime,
                           uint32 pTurnCost,
                           bool pAbbreviate = true,
                           bool landmarks = false,
                           bool avoidTollRoads = false,
                           bool avoidHighways = false );
   
   
   /**
    *    Set the route calculation parameters.
    *    @param pIsStartTime  True if the given time is start time.
    *    @param pRouteCost    The route cost to use in calculations.
    *    @param pVehicleParam The type of vehicle used.
    *    @param pTime         The time (start-time).
    *    @param pTurnCost     The cost to make a u-turn at the origin.
    *    @param pAbbreviate   Should the street names be abbreviated or
    *                         not. If true and swedish then "Baravägen"
    *                         will be written as "Barav.".
    *    @param landmarks     Should landmarks be added to the result.
    *    @param avoidTollRoads If toll-roads should be avoided, default 
    *                          false.    
    *    @param avoidHighways  If highway roads should be avoided,
    *                          default false.
    **/
   void setRouteParameters(bool pIsStartTime,
                           RouteTypes::routeCostType pRouteCost,
                           uint32 pVehicleParam,
                           uint32 pTime,
                           uint32 pTurnCost,
                           bool pAbbreviate = true,
                           bool landmarks = false,
                           bool avoidTollRoads = false,
                           bool avoidHighways = false );   

   /**
    *   Adds an origin or destination depending on the value of
    *   <code>isdest</code>.
    *   @param orignOrDest Origin or destination to add.
    *   @param isdest      True if it is a destination.
    *   @return -1 if error.
    */
   int addOrigOrDest( const SearchMatch& orignOrDest, bool isdest );
   
   /**
    *    @name Add origin.
    *    @memo Add one possible origin in the route.
    *    @doc  Add one possible origin in the route.
    */
   //@{

      /**
       *    Adds a search match as origin.
       */
      int addOrigin( const SearchMatch& origin );
    
      /**
       *    Add one origin that is known by ID.
       *    @param mapID   ID of the map where the origin is located.
       *    @param itemID  ID of the item where the origin is located.
       *                   Might be a StreetSegmentItem, StreetItem,
       *                   CompanyItem or CategoryItem.
       *    @param offset  Optional parameter that is the offset at 
       *                   item with ID = itemID. This parameter is 
       *                   only valid if itemID is a StreetSegmentItem.
       */
      int addOriginID(uint32 mapID, uint32 itemID, 
                      uint16 offset = 0);

         /**
          *    Add one origin that is known by coordinates.
          *    @param lat  The latitude part of the coordinate for the
          *                new origin.
          *    @param lon  The longitude part of the coordinate for the
          *                new origin.
          */
          int addOriginCoord(int32 lat, int32 lon, uint16 angle = 0);

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
          int addOrigin(uint32  mapID,
                              uint32  itemID,
                              uint16  offset,
                              int32   lat,
                              int32   lon);
      //@}

      /**
       *    @name Add destination.
       *    @memo Add one possible destination in the route.
       *    @doc  Add one possible destination in the route.
       */
      //@{
          
         /** 
          *   Adds a SearchMatch as destination.
          */
         int addDestination( const SearchMatch& dest );
          
         /**
          *    Add one destination.
          *    @param mapID   ID of the map where the destination is located.
          *    @param itemID  ID of the item where the destination is located.
          *                   Might be a StreetSegmentItem, StreetItem,
          *                   CompanyItem or CategoryItem.
          *    @param offset  Optional parameter that is the offset at 
          *                   item with ID = itemID. This parameter is 
          *                   only valid if itemID is a StreetSegmentItem.
          */
          int addDestinationID(uint32 mapID, uint32 itemID, 
                                     uint16 offset = 0);

         /**
          *    Add one destination that is known by coordinates.
          *    @param lat  The latitude part of the coordinate for the
          *                new destination.
          *    @param lon  The longitude part of the coordinate for the
          *                new destination.
          */
          int addDestinationCoord(int32 lat, int32 lon, uint16 = MAX_UINT16);

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
          int addDestination(uint32 mapID, uint32 itemID, 
                                   uint16 offset,
                                   int32 lat, int32 lon);
      //@}

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
          uint32 getNbrValidOrigins() const;

         /**
          *    Get the number of valid destinations.
          *    @return The number of valid destinations in the route.
          */
          uint32 getNbrValidDestinations() const;
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
          *    @param name    Outparameter that is setto point to the name
          *                   of destination number i. This must not be
          *                   deleted by the caller and must not be used
          *                   after the request is deleted.
          *    @param lat     Outparameter that is set to the latitude-part
          *                   or the origin-coordinate.
          *    @param lon     Outparameter that is set to the longiude-part
          *                   or the origin-coordinate.
          *    @param type    Outparameter that is set to the type for
          *                   origin number i.
          *    @return  True if the outparameters are set, false 
          *             otherwise.
          */
          bool getValidOrigin(uint32 i, 
                                    uint32& mapID, 
                                    uint32& itemID,
                                    const char*& name,
                                    int32& lat,
                                    int32& lon,
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
          *    @param name    Outparameter that is setto point to the name
          *                   of destination number i. This must not be
          *                   deleted by the caller and must not be used
          *                   after the request is deleted.
          *    @param lat     Outparameter that is set to the latitude-part
          *                   or the origin-coordinate.
          *    @param lon     Outparameter that is set to the longiude-part
          *                   or the origin-coordinate.
          *    @param type    Outparameter that is set to the type for
          *                   destination number i.
          *    @return  True if the outparameters are set, false 
          *             otherwise.
          */
          bool getValidDestination(uint32 i, 
                                         uint32& mapID, 
                                         uint32& itemID,
                                         const char*& name,
                                         int32& lat,
                                         int32& lon,
                                         ItemTypes::itemType& type);
      //@}


      /**
       * @name Get names.
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
          const char* getRouteOriginName() const;

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
          bool getRouteOriginCoord(int32 &lat, int32 &lon) const;

        /**
         * Get the routes origin point, if the routing failed then
         * the data is not valid.
         * @param mapID Outparameter that is set to the ID of the
         *              map for the routes origin point.
         * @param itemID Outparameter that is set to the ID of the
         *               item that is the routes origin point.
         * @param offset Outparameter that is set to the offset of the
         *               item that is the routes origin point.   
         * @param name Outparameter that is set to point to the name
         *             of the routes origin point. This must not be
         *             deleted by the caller nor be used after the 
         *             request is deleted.
         */
          void getRouteOrigin( uint32& mapID, 
                               uint32& itemID,
                               uint16& offset,
                               const char*& name ) const;


         /**
          *    Get the name of the destination point in the route. If 
          *    the route calculation failed then the data is not valid.
          *    @return A pointer to the name of the destination point 
          *            in the route. This must not be deleted by the 
          *            caller nor be used after the request is deleted.
          */
          const char* getRouteDestinationName() const;

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
          bool getRouteDestinationCoord(int32 &lat, int32 &lon) const;

        /**
         * Get the routes destination point, if the routing failed then
         * the data is not valid.
         * @param mapID Outparameter that is set to the ID of the
         *              map for the routes destination point.
         * @param itemID Outparameter that is set to the ID of the
         *               item that is the routes destination point.
         * @param offset Outparameter that is set to the offset of the
         *               item that is the routes destination point.   
         * @param name Outparameter that is set to point to the name
         *             of the routes destination point. This must not be
         *             deleted by the caller nor be used after the 
         *             request is deleted.
         */
          void getRouteDestination( uint32& mapID, 
                                    uint32& itemID,
                                    uint16& offset,
                                    const char*& name ) const; 
      //@}


      /**
       *    Get the route as a route reply packet. {\it {\bf NB!} The
       *    packet that is returned belongs to this request and must
       *    not be deleted by the caller but will be deleted when this
       *    request is deleted}.
       *    @return The route reply packet that was sent by the Route 
       *            Module. NULL will be returned upon error.
       */
       const RouteReplyPacket* getRouteReplyPacket();

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
           ItemTypes::routedir_nbr_t getStartDirectionHousenumber();

         /**
          *    Get the start direction in terms of driving with the 
          *    odd/even numbers to the left or right.
          *    @return  The start direction of the route in terms of    
          *             odd/even street numbers to the left/right.
          */
           ItemTypes::routedir_oddeven_t getStartDirectionOddEven();
      //@}

      /**
       * Removes all non input or output data from the request.
       * Used when caching a route.
       */
       void clearTemporaryRouteData();

      /**
       * The aproximate size of the request.
       * Used when caching a route.
       */
       uint32 getSize();

      /**
       * The routes ID. Currently returns the RequestID.
       */
       uint32 getRouteID() const;

      /**
       * The routes creationTime.
       */
       uint32 getRouteCreateTime() const;


      /**
       * Debug method that prints data to standard out.
       */
       void dumpState();

      /**
       *    Returns StringTable::OK if routing was successful.
       *    @return StringTable::OK if routing was succesful.
       *            RouteObject::getStatus() otherwise
       */
      StringTable::stringCode getStatus() const;

      /**
       * Get the route as an ExpandedRoute object.
       *
       * @return The ExpandedRoute object for this route or NULL if
       *         getStatus() isn't OK.
       */
      ExpandedRoute* getExpandedRoute();

      /**
       * Set the allowed maps.
       *
       * @param maps The allowed maps. NULL means all maps are allowed.
       */
      void setAllowedMaps( RouteAllowedMap* maps );
      
      /**
       * Set if aheads should be removed from the expanded route even if
       * roadname changes..
       *
       * @param removeAheadIfDiff If true aheads are removed at namechanges..
       */
      void setRemoveAheadIfDiff( bool removeAheadIfDiff);
      
      
      /**
       * Set if name changes should be included as waypoints.
       *
       * @param removeAheadIfDiff If true aheads are removed at namechanges..
       */
      void setNameChangeAsWP( bool nameChangeAsWP );


      /**
       * Set if to route with and without disturbances.
       */
      void setCompareDisturbanceRoute( bool val );


      /**
       *   Compares unexpanded routes looking for disturbances.
       *   If setCompareDisturbanceRoute is not true then this always
       *   returns false. If setCompareDisturbanceRoute is set then
       *   this method checks if there are any differences between the
       *   route with disturbances and the route without disturbances.
       */
      bool compareRoutes();


      /**
       * Get the prefered language.
       */
      StringTable::languageCode getLanguage() const;

      /**
       *   Returns a new SearchMatch containing the origin
       *   routed to, but in the original item and map id.
       *   Only lat and lon have been checked for validity.
       */
      const SearchMatch* getOriginalRouteOrigin() const;
      
      /**
       *   Returns a new SearchMatch containing the destination
       *   routed to, but with the original item and map id.
       *   Only lat and lon have been checked for validity.
       */
      const SearchMatch* getOriginalRouteDest() const;

      /**
       * Get the origin angle, if not set MAX_UINT16 is returned.
       */
      uint16 getOriginAngle() const;

      /**
       * Get the disturbances for the route. 
       *
       * @return Vector with information of the disturbances affecting
       *         the route.
       */
      const vector<DisturbanceDescription>& getDisturbanceInfo() const;

      /**
       * Get nbrRouteObjectsUsed.
       */
      uint32 getNbrRouteObjectsUsed() const;

private:

   /**
    *   Sets original orig and dest search matches.
    */
   void setOriginalOrigAndDest();

   /**
    *   Creates a SearchMatch representing the original orig or dest
    *   depending on the value of <code>dest</code>.
    */
   static SearchMatch* createOriginalOrigOrDest(RouteObject* routeObject,
                                                bool dest);
   
   /**
    *   Creates a RouteObject using the data in m_data.
    *   @param expand True if the RouteObject should expand the route.
    *   @param costC If true traffic information affects the route.
    */
   RouteObject* prepareRouteObject(bool expand, bool costC);

   /**
    *   Handles the RouteObject(s).
    *   @param pack PacketContainer from processPacket.
    *   @return PacketContainer to delete.
    */
   inline PacketContainer* handleRouteObjectState(PacketContainer* pack);
   
   /**
    *   Handles the expansion state.
    *   @param pack PacketContainer from processPacket.
    *   @return PacketContainer to delete.
    */
   inline PacketContainer* handleExpansionState(PacketContainer* pack);

   /**
    *   Handles the Request for disturbances state.
    *   @param pack PacketContainer from processPacket.
    *   @return PacketContainer to delete.
    */
   inline PacketContainer* handleDistState(PacketContainer* pack);
   
   
   /**
    *   Prepares the route expansion state.
    */
   inline void prepareExpansion();


   
   
   /**
    *   Enqueues packets from the RouteObject until it 
    *   returns NULL.
    */
   int enqueuePacketsFromRouteObject(RouteObject* robj);
   
   /**
    *   Enqueues packets from the RouteExpander until it 
    *   returns NULL.
    */
   int enqueuePacketsFromRouteExpander(RouteExpander* rexp);

   
   
   /**
    *    Data used by the RouteRequest.
    */
   RouteRequestData* m_data;

   /**
    *    The expander that sends expand route request packets to the
    *    map module and concatenates the replies.
    */
   RouteExpander* m_routeExpander;

   /**
    *    A packet that contains the expanded route.
    */
   PacketContainer* m_answer;

   
   /**
    *    Vector with information of the disturbances affecting the route.
    */
   vector<DisturbanceDescription> m_disturbanceInfo;

   /**
    * The number of expand route request packets sent and how many
    * we expect back before starting to concatenate them.
    */
   uint32 m_nbrDisturbanceInfoPackets;

   
   /// pointer to valid topregionrequest *with* data
   const TopRegionRequest* m_topReq;

   /// User data for the request
   RequestUserData m_user;
   
#ifdef CALCULATE_REQUEST_STATISTICS
   uint32 m_startTime;
   uint32 m_nbrProcessedPacket;
#endif
   
};

// ========================================================================
//                                  Implementation of the inlined methods =

inline const vector<DisturbanceDescription>& 
RouteRequest::getDisturbanceInfo() const {
   return m_disturbanceInfo;
}

#endif
