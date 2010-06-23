/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVROUTEMESSAGE_H
#define NAVROUTEMESSAGE_H

#include "config.h"
#include "NavMessage.h"
#include "RouteList.h"


/**
 *    Message representing a route request from Navigator to Server.
 */
class RouteReq : public IncomingNavMessage {

public:
   enum routeType { 
      ///Preferred by the GPS-less mode. Sends enough 
      ///information that an itinerary can be generated by 
      ///the client.
      slim = 1,
      ///Preferred by the GPS mode. Plenty of coordinates
      ///ensures good route following, distance and time to 
      ///goal calculation, and off track detection.
      full = 2,
   };

   /**
    *   Destructor.
    */
   virtual ~RouteReq();

   /**
    *   @return The latitude of the origin.
    */
   int32 getOrigLat() const;

   /**
    *   @return The longitude of the origin.
    */
   int32 getOrigLon() const;

   /**
    *   @return The latitude of the destination.
    */
   int32 getDestLat() const;

   /**
    *   Set the latitude of the destination.
    */
   void setDestLat(int32 lat);
   
   /**
    *   @return The longitude of the destination.
    */
   int32 getDestLon() const;

   /**
    *   Set the longitude of the destination.
    */
   void setDestLon(int32 lon);
   
   /**
    *    The start angle of the car.
    *    @return The start angle of the car.
    */
   uint16 getStartAngle() const;

   
   /**
    * Get the prefered language.
    * 
    * @return The prefered language.
    */
   StringTable::languageCode getLanguage() const;
   

   /**
    * Get the route vehicle.
    *
    * @return The route vehicle.
    */
   ItemTypes::vehicle_t getVehicle() const;


   /**
    * Get the route cost.
    *
    * @return The route cost.
    */
   RouteTypes::routeCostType getRouteCost() const;


   /**
    * Estimated driving time before truncation. 
    * What this means is that when time_to_trunc seconds of route
    * is done trunkate.
    *
    * @return The estimated time before trunk.
    */
   uint32 getTimeBeforeTrunk() const;


   /**
    * The degree of data wanted for the route. 
    *
    * @return The route data content type.
    */
   routeType getRouteType() const;


   /**
    * The current speed of the client.
    *
    * @return The current speed of the client.
    */
   uint8 getCurrentSpeed() const;


   /**
    * Returns if to avoid toll-roads.
    */
   bool avoidTollRoads() const;


   /**
    * Returns if to avoid highway.
    */
   bool avoidHighway() const;


   /**
    * Returns the re-route ID, 0 if none.
    */
   uint32 getRouteID() const;


   /**
    * Returns the re-route create tme, 0 if none.
    */
   uint32 getRouteCreateTime() const;


   /**
    * Returns if add landmarks.
    */
   bool addLandmarks() const;


   /**
    * Returns if to abbreviate.
    */
   bool abbreviate() const;


protected:

   /**
    *   Constructor.
    *   @param NavAddress The sender of the message.
    */
   RouteReq(const NavAddress& senderAddress, NavSession * session);

   /**
    *   The latitude of the origin.
    */    
   int32 m_origLat;

   /**
    *   The longitude of the origin.
    */
   int32 m_origLon;

   /**
    *   The latitude of the destination.
    */
   int32 m_destLat;

   /**
    *   The longitude of the destination.
    */
   int32 m_destLon;

   /**
    *   The start angle of the car.
    */
   uint16 m_startAngle;


   /**
    * The prefered language.
    */
   StringTable::languageCode m_language;


   /**
    * The route vehicle.
    */
   ItemTypes::vehicle_t m_vehicle;


   /**
    * The route cost.
    */
   RouteTypes::routeCostType m_routeCost;


   /**
    * The estimated time to trunk.
    */
   uint32 m_timeToTrunk;


   /**
    * The route type.
    */
   routeType m_routeType;


   /**
    * The speed.
    */
   uint16 m_speed;


   /**
    * If to avoid toll-roads.
    */
   bool m_avoidTollRoads;


   /**
    * If to avoid highway.
    */
   bool m_avoidHighway;


   /**
    * The re-routeID.
    */
   uint32 m_routeID;


   /**
    * The re-routeCreateTime.
    */ 
   uint32 m_routeCreateTime;


   /**
    * If to add landmarks.
    */
   bool m_landmarks;


   /**
    * If to abbreviate.
    */
   bool m_abbreviate;
};

/**
 *   Message representing a route reply to the Navigator
 *   from the server.
 */
class RouteReply : public OutGoingNavMessage {
   
public:
   /**
    * Route reply specific status codes.
    */
   enum RouteReplyStatus {
      ROUTE_REPLY_NO_ROUTE_FOUND        
      = (0x01|NAV_STATUS_REQUEST_SPECIFIC_MASK),
      ROUTE_REPLY_TOO_FAR_FOR_VEHICLE   
      = (0x02|NAV_STATUS_REQUEST_SPECIFIC_MASK),
      ROUTE_REPLY_PROBLEM_WITH_ORIGIN   
      = (0x03|NAV_STATUS_REQUEST_SPECIFIC_MASK),
      ROUTE_REPLY_PROBLEM_WITH_DEST     
      = (0x04|NAV_STATUS_REQUEST_SPECIFIC_MASK),
      ROUTE_REPLY_NO_AUTO_DEST          
      = (0x05|NAV_STATUS_REQUEST_SPECIFIC_MASK)
   };

   /**
    *   Destructor.
    */
   virtual ~RouteReply();


   /**
    * Get the route ID.
    */
   uint32 getRouteID() const;


   /**
    * Get the route createTime.
    */
   uint32 getRouteCreateTime() const;


   /**
    * Get the route list.
    */
   const RouteList& getRouteList() const;


protected:
   
   /**
    *   Constructor.
    *   @param recipientAddress The recipient's address.
    *   @param expansionAnswer  An expandroutePacket that describes
    *                           the route.
    *   @param req              The RouteReq that was sent to create
    *                           the reply.
    */
   RouteReply(const NavAddress& recipientAddress,
              PacketContainer* expansionAnswer,
              uint32 routeID, 
              uint32 routeCreateTime,
              RouteReq* req,
              NavSession* session,
              RouteRequest* rr );

   /**
    *  Constructor for RouteReplies that are concatenated to
    *  search replies.
    *  @param recipientAddress  The recipient's address.
    *   @param expansionAnswer  An expandroutePacket that describes
    *                           the route.
    *  @param protoVer          The version of the RouteReply.
    */
   RouteReply(const NavAddress& recipientAddress,
              PacketContainer* expansionAnswer,
              uint32 routeID, 
              uint32 routeCreateTime,
              int protoVer,
              NavSession * session,
              RouteRequest* rr );
   
   /**
    *   The packet that contains the answer to the routing request.
    */
   PacketContainer* m_answer;

   /**
    *   The routelist, a middle stage for route creation to the
    *   clients.
    */
   RouteList m_routeList;

   /**
    * The route id.
    */
   uint32 m_routeID;

   /**
    * The route create time.
    */
   uint32 m_routeCreateTime;
};


#endif // NAVROUTEMESSAGE_H

