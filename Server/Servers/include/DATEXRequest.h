/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DATEXREQUEST_H
#define DATEXREQUEST_H

#include "config.h"
#include "Request.h"
#include "StreetSegmentItemPacket.h"
#include "GetTMCCoordinatePacket.h"
#include "AddDisturbancePacket.h"
#include "RemoveDisturbancePacket.h"
#include <time.h>
#include <vector>
#include <queue>
#include <memory>
#include "TopRegionPacket.h"
#include "TopRegionMatch.h"
#include "IDTranslationPacket.h"
#include "TrafficSituation.h"
#include "TrafficDataTypes.h"
#include "MC2String.h"
#include "ExpandItemID.h"
#include "RouteRequest.h"
#include "GenericServer.h"

class CoveredIDsRequest;

class TopRegionRequest;
class TrafficPointRequest;

/**
 *  Request the processing of a DATEX message in a TRISS server.
 *  The result of the processing of this request is the addition,
 *  modification or removal of one or more Disturbances in the InfoModule
 */

class DATEXRequest : public Request
{
  public:

   /**
    *   Constructor
    *   @param topReq    Pointer to valid TopRegionRequest with data
    */
   DATEXRequest(uint16 requestID,
                TrafficSituation* trafficSituation,
                const TopRegionRequest* topReq);

   /**
    *   Constructor
    *   @param topReq    Pointer to valid TopRegionRequest with data
    */
   DATEXRequest(uint16 requestID,
                vector<TrafficSituation*> trafficVector,
                const TopRegionRequest* topReq,
                MC2String supplier,
                vector<MC2String> toBeKept);
   
   /**
    *  Delete this Request and release allocated memory.
    */
   virtual ~DATEXRequest();

   /// init the member variables
   void init();
   
   /** 
    *    The state variable.
    */
   enum datex_state {
      STATE_TMC = 0,
      STATE_STREET_SEGMENT_ITEM,
      STATE_COVERED_IDS,
      STATE_ROUTE_REQUEST,
      STATE_TOP_REGION_REQUEST,
      STATE_ID_TRANSLATION_REQUEST,
      STATE_ADD_DISTURBANCE,
      STATE_DONE,
      STATE_ERROR
   } m_state;
   
   
   /// Processes a situation
   datex_state processSituation();
   
   /**
    *  Process a ReplyPacket received from a Module.
    *  @param packetCont PacketContainer with the reply from a Module.
    */
   void processPacket(PacketContainer* packetCont);
   
   /**
    *  Method for recieving the result of the Request. This is always
    *  NULL for a DATEXRequest.
    *  @return Always NULL for a DATEXRequest.
    */
   PacketContainer* getAnswer() {
      return NULL;
   }
      
   /**
    *  Get the current state of the DATEXRequest.
    *  @return The current state of the DATEXRequest.
    */
   datex_state getState() {
      return m_state;
   }
   
  protected:
   /**
    *  Creates the GetTMCCoordinateRequestPacket(s).
    *  @return The PacketContainer to send to the InfoModule.
    */
   datex_state createGetTMCCoordinateRequestPacket();

   /**
    *  Handles the information of the GetTMCCoordinateReplyPacket from the 
    *  InfoModule and returns the next Request state.
    *  @param pack The reply packet from the InfoModule.
    *  @return The next state of the request.
    */
   datex_state processGetTMCCoordinateReplyPacket(PacketContainer* packet);

   /**
    *  Processes all the StreetSegmentItemReplyPackets.
    */
   void processStreetReplyPackets();
   
   /**
    *  Creates the StreetSegmentItemRequestPacket(s).
    *  @return The next state of the request.
    */
   datex_state createStreetSegmentItemRequestPackets();
   
   /**
    *  Handles the information of the StreetSegmentItemReplyPacket from
    *  the MapModule and returns the next Request state.
    *  @param packet The reply packet from the MapModule.
    *  @return The next state of the request.
    */
   datex_state processStreetSegmentItemReplyPacket(PacketContainer* packet);
   
   /**
    *  Returns two arrays, one with the angles and one with the extra
    *  costC for the routeModule.
    *  @param expItemID  The ExpandItemID from the route
    *  @param stringItems The ExpandStringItem array from the route
    *                     module.
    *  @param angle  A reference to the angle array, where the angles are
    *                put.
    *  @param costC  A reference to the costC array, where the costC:s are
    *                put.
    */
   void getAngle(ExpandItemID* expItemID,
                 uint32* &angle);
   
   /**
    *  Creates a CoveredIDsRequestRequest to get multiple start and origins.
    *  @return The next state of the request.
    */
   datex_state createCoveredIDsRequestRequest();

   /**
    *  Handle when a covered ID is finnished.
    */
   datex_state handleCoveredIDsDone();

   
   /**
    *  Handles the next replypacket from the CoveredIDsRequest.
    *  @param pack The reply packet from the CoveredIDsRequest.
    *  @return The next state of the request.
    */
   datex_state processCoveredIDsReply(PacketContainer* pack);

   /**
    *  Creates the RouteRequest if we have a two-point disturbance.
    *  @return The next state of the request.
    */
   datex_state createRouteRequest();

   /**
    *  Handles the next replypacket from the RouteRequest.
    *  @param pack The reply packet from the RouteRequest.
    *  @return The next state of the request.
    */
   datex_state processRouteReply(PacketContainer* pack);

   /**
    *  Handles the information in the ExpandRouteReplyPacket from the
    *  RouteRequest.
    *  @return The next state of the request.
    */
   datex_state handleRouteReply();
   
   /**
    *  Creates the TopRegionRequestPacket.
    *  @return The next state of the request.
    */
   datex_state createTopRegionRequestPacket();

   /**
    *  Processes the TopRegionReplyPacket from the MapModule.
    *  @param packet The TopRegionReplyPacket.
    *  @return The next state of the request.
    */
   datex_state processTopRegionReplyPacket(PacketContainer* packet);
   
   /**
    *  Creates the IDTranslationRequestPacket(s).
    *  @return The next state of the request.
    */
   datex_state createIDTranslationRequestPacket();  
   
   /**
    *  Handles the reply from the IDTranslationRequesPacket and
    *  creates the map, which transforms low level nodes to nodes on the
    *  overview map.
    *  @param pack The IDTranslationReplyPacket.
    *  @return The next state of the request.
    */
   datex_state processIDTranslationReplyPacket(PacketContainer* packet);
   
   /**
    *  Creates the AddDisturbanceRequestPackets.
    *  @return The next state of the request.
    */  
   datex_state createAddDisturbanceRequestPacket();
   
   /**
    *  Handles the information of the AddDisturbanceReplyPacket from the
    *  InfoModule and returns the next state of the Request.
    *  @param pack The reply packet from the InfoModule.
    *  @return The next state of the request.
    */
   datex_state processAddDisturbanceReplyPacket(PacketContainer* packet);

  private:

   // Member variables
   /// The message ID from the file
   MC2String m_situationReference;
   /// The location table for the situation
   MC2String m_locationTable;
   /// The element reference
   MC2String m_elementReference;
   /// The start time
   uint32 m_startTime;
   /// The expiry time
   uint32 m_expiryTime;
   /// The creation time
   uint32 m_creationTime;
   /// The disturbance type
   TrafficDataTypes::disturbanceType m_type;
   /// The phrase
   TrafficDataTypes::phrase m_phrase;
   /// The eventcode
   uint32 m_eventCode;
   /// The text
   MC2String m_text;
   /// The severity
   TrafficDataTypes::severity m_severity;
   /// The severity factor
   TrafficDataTypes::severity_factor m_severityFactor;
   /// The first location code as a string 
   MC2String m_firstLocation;
   /// The direction
   TrafficDataTypes::direction m_direction;

   /// The second location code as a string
   MC2String m_secondLocation;
   /// The extent
   int16 m_extent;
   /// The extra cost
   uint32 m_costFactor;
   /// Vector with coordinates from Triss server
   vector<pair<int32, int32> > m_tmcCoords;
   /// The queue length
   uint32 m_queueLength;
   
   /// Request packets that is going to be sent
   list<StreetSegmentItemRequestPacket*> m_streetSegmentRequestPackets;
   queue<IDTranslationRequestPacket*> m_idRequestPackets;
   list<AddDisturbanceRequestPacket*> m_addRequestPackets;
   
   /// The number of request packets that is going to be sent
   uint32 m_nbrRequestPackets;
   
   /// A vector with all the RouteRequests.
   vector<RouteRequest*> m_routeRequests;
   
   /// The number of route requests.
   uint32 m_nbrRouteRequests;

   /// A vector with all the mapIDs.
   vector<uint32> m_mapID;
   
   /// The nodeID's for affected StreetSegmentItems.
   vector<uint32> m_nodeID;
   
   /**
    * The latitudes and longitudes from the MapModule, the coordinates
    * in the middle of the StreetSegmentItem
    */
   vector<pair<int32, int32> > m_coords;
   
   /// The latitudes and longitudes from the TMC-database
   vector<pair<int32, int32> > m_tmcFirst;
   vector<pair<int32, int32> > m_tmcSecond;
    
   /**
    * The angles from the MapModule, which decribes the direction of
    * the Disturbance on the StreetSegmentItem.
    */
   vector<uint32> m_angle;
   
   /// The number of packets sent so far.
   uint32 m_sent;
   
   /// The number of packets recieved in the current state so far.
   uint32 m_received;
   
   /** The total map ID tree structure. */
   auto_ptr<ItemIDTree> m_mapIDTree;

   /// A vector that contains all the nodes on the low level map.
   vector<IDPair_t> m_lowLevelNodes;

   /**
    * A vector with nodeVectors, used when creating the
    * IDTranslationRequestPacket. You put all the pairs with nodeIDs and
    * mapIDs that is going to translated in the vector.
    */
   IDPairVector_t* m_nodeVectors[1];
   
   /// A map that transforms a pair of mapID and nodeID, to a pair of
   // mapID and nodeID on the overview map.
   map<IDPair_t, IDPair_t> m_lowLevelPairToHighLevel;
   
   /// A multimap with the StreetSegmentItemReplyPackets.
   multimap<uint32, PacketContainer*> m_streetReplyPackets;

   /// A vector with all the traffic situations
   vector<TrafficSituation*> m_trafficSituations;

   /// pointer to valid topregionrequest *with* data
   const TopRegionRequest* m_topReq;

   /**
    *  Flag showing that we had TMC location but failed to use it
    *  The severity of the traffic situation should be minimal.
    */
   bool m_tmcFailed;

   /// Coverd id reqesuts
   CoveredIDsRequest* m_coveredIDreq;
   
   auto_ptr<TrafficPointRequest> m_trafficPointReq;
   
   /// The status of the request. Returned if m_state == ERROR
   StringTable::stringCode m_status;

   /// if the origin is covered or not
   bool m_originsCovered;

   /// The origin ids
   set<IDPair_t> m_originIDs;
   /// the destination ids
   set<IDPair_t> m_destinationIDs;

   /// The disturbance length.
   float64 m_distance;
   
};


#endif 









