/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ISABBOXROUTEMESSAGE_H
#define ISABBOXROUTEMESSAGE_H

#include "config.h"
#include "isabBoxNavMessage.h"
#include "NavRouteMessage.h"

class IsabRouteList;


/**
 *   RouteReq that can be translated from isabBox-format.
 *   @see RouteReq
 */
class isabBoxRouteReq : public RouteReq {

public:
   /**
    *   Constructor.
    *   @param senderAddress The address which the message came from.
    */
   isabBoxRouteReq(const NavAddress& senderAddress, NavSession * session);


   /**
    *   Convert this message from bytes.
    *   @param buf       The buffer to read from.
    *   @param bufSize   The size of the buffer.
    *   @return          True if the conversion could be made.
    */
   bool convertFromBytes(const byte* buf,
                         int bufSize);
   
   /**
    *  Set route data explicitly.
    *
    *  @param origLat The latitude of the origin.
    *  @param origLon The longitude of the origin.
    *  @param destLat The latitude of the destination.
    *  @param destLon The longitude of the destination.
    *  @param startAngle The starting angle at the origin. Counting
    *                    clockwise and using 360 degrees per complete 
    *                    circle.
    */
   inline void setRouteData( int32 origLat,
                             int32 origLon,
                             int32 destLat,
                             int32 destLon,
                             uint16 startAngle );

   /**
    *  Set RouteReq header data.
    *
    *  @param protoVer The protocol version.
    *  @param navID Used if protoVer is greater than 0.
    *  @param userID Used if protoVer is 1.
    *  @param userName Used if protoVer is greater than 1. Must be a 
    *                  20 char long string, including terminating NULL 
    *                  byte.
    */
   inline void setRouteReqHeader( uint8 protoVer,
                                  uint32 navID,
                                  uint32 userID,
                                  const char* userName );


   /**
    * Set RouteReq route params.
    */
   void setRouteParams( StringTable::languageCode language,
                        ItemTypes::vehicle_t vehicle,
                        RouteTypes::routeCostType routeCost,
                        uint32 timeToTrunk, routeType routeType );
};

/**
 *
 */
class isabBoxRouteReply : public RouteReply {

public:

   /**
    *   Create a new answer.
    *   @see NavRouteReply.
    */
   isabBoxRouteReply(const NavAddress& recipientAddress,
                     PacketContainer* expansionAnswer,
                     uint32 routeID, 
                     uint32 routeCreateTime,
                     RouteReq* req,
                     NavSession* session,
                     RouteRequest* rr,
                     uint8 reqVer = 1 );


   /**
    * Destructor.
    */
   ~isabBoxRouteReply();


   /**
    *  Constructor for RouteReplies that are concatenated to
    *  search replies.
    *  @param recipientAddress  The recipient's address.
    *   @param expansionAnswer  An expandroutePacket that describes
    *                           the route.
    *  @param protoVer          The version of the RouteReply.
    */
   isabBoxRouteReply(const NavAddress& recipientAddress,
                     PacketContainer* expansionAnswer,
                     uint32 routeID, 
                     uint32 routeCreateTime,
                     int protoVer,
                     NavSession* session,
                     uint8 reqVer = 1 );
      
   /**
    *   Converts the buffer to a sendable message.
    */
   bool convertToBytes(byte* buf,
                       int bufSize);


private:
   /**
    * The reply data.
    */
   IsabRouteList* m_irl;
};


inline void 
isabBoxRouteReq::setRouteData( int32 origLat,
                               int32 origLon,
                               int32 destLat,
                               int32 destLon,
                               uint16 startAngle )
{
   m_origLat = origLat;
   m_origLon = origLon;
   m_destLat = destLat;
   m_destLon = destLon;
   m_startAngle = startAngle;
}


inline void 
isabBoxRouteReq::setRouteReqHeader( uint8 protoVer,
                                    uint32 navID,
                                    uint32 userID,
                                    const char* userName )
{
   m_protoVer = protoVer;
   if ( m_protoVer > 0 ) {
      setNavID( navID );
   }
   if ( m_protoVer == 1 ) {
      setUserID( userID );
   }
   if ( m_protoVer > 1 ) {
      setUserName( (byte*)userName );
   }
}


inline void
isabBoxRouteReq::setRouteParams( StringTable::languageCode language,
                                 ItemTypes::vehicle_t vehicle,
                                 RouteTypes::routeCostType routeCost,
                                 uint32 timeToTrunk, routeType routeType )
{
   m_language = language;
   m_vehicle = vehicle;
   m_routeCost = routeCost;
   m_timeToTrunk = timeToTrunk;
   m_routeType = routeType;
}


#endif // ISABBOXROUTEMESSAGE_H

