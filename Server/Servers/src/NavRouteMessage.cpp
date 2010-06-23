/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "NavRouteMessage.h"
#include "ExpandRoutePacket.h"
#include "PacketContainer.h"

/*------------------------------------------------------------------*/

RouteReq::RouteReq(const NavAddress& senderAddress, NavSession * session)
      : IncomingNavMessage(senderAddress, NavMessage::ROUTE_REQ, true, session)
{
   m_avoidTollRoads = false;
   m_avoidHighway = false;
   m_routeID = 0;
   m_routeCreateTime = 0;
   m_landmarks = false;
   m_abbreviate = true;
}

RouteReq::~RouteReq()
{
   /* Nothing is newed */
}

int32
RouteReq::getOrigLat() const
{
   return m_origLat;
}


int32
RouteReq::getOrigLon() const
{
   return m_origLon;
}

int32
RouteReq::getDestLat() const
{
   return m_destLat;
}

void
RouteReq::setDestLat(int32 lat)
{
   m_destLat = lat;
}


int32
RouteReq::getDestLon() const
{
   return m_destLon;
}


void
RouteReq::setDestLon(int32 lon)
{
   m_destLon = lon;
}

uint16
RouteReq::getStartAngle() const
{
   return m_startAngle;
}

StringTable::languageCode 
RouteReq::getLanguage() const {
   return m_language;
}


ItemTypes::vehicle_t 
RouteReq::getVehicle() const {
   return m_vehicle;
}


RouteTypes::routeCostType 
RouteReq::getRouteCost() const {
   return m_routeCost;
}


uint32 
RouteReq::getTimeBeforeTrunk() const {
   return m_timeToTrunk;
}


RouteReq::routeType 
RouteReq::getRouteType() const {
   return m_routeType;
}


uint8 
RouteReq::getCurrentSpeed() const {
   return m_speed;
}


bool 
RouteReq::avoidTollRoads() const {
   return m_avoidTollRoads;
}


bool 
RouteReq::avoidHighway() const {
   return m_avoidHighway;
}


uint32 
RouteReq::getRouteID() const {
   return m_routeID;
}


uint32 
RouteReq::getRouteCreateTime() const {
   return m_routeCreateTime;
}


bool 
RouteReq::addLandmarks() const {
   return m_landmarks;
}


bool 
RouteReq::abbreviate() const {
   return m_abbreviate;
}


/*------------------------------------------------------------------*/

RouteReply::RouteReply(const NavAddress& recipientAddress,
                       PacketContainer* expansionAnswer,
                       uint32 routeID, 
                       uint32 routeCreateTime,
                       RouteReq* req,
                       NavSession* session,
                       RouteRequest* rr )
      : OutGoingNavMessage(recipientAddress, NavMessage::ROUTE_REPLY, false, session),
        m_answer(expansionAnswer), m_routeList(req, expansionAnswer, rr ),
        m_routeID( routeID ), m_routeCreateTime( routeCreateTime )
{
   // Set the protoVer
   if ( req != NULL ) {
      m_protoVer = req->getProtoVer();
      m_navID = req->getNavID();
      m_userID = req->getUserID();
   } else {
      m_protoVer = 0;
   }
   /* Do NULL-checks */
   if ( m_answer == NULL ) {
      setStatus( NAV_STATUS_NOT_OK );
   } else {
      Packet* p = m_answer->getPacket();
      ExpandRouteReplyPacket* erp =
         dynamic_cast<ExpandRouteReplyPacket*>(p);
      if ( erp != NULL && erp->getStatus() == StringTable::OK ) {
         setStatus( NAV_STATUS_OK );
      } else {
         setStatus( NAV_STATUS_NOT_OK );
      }
   }
}

RouteReply::RouteReply(const NavAddress& recipientAddress,
                       PacketContainer* expansionAnswer,
                       uint32 routeID, 
                       uint32 routeCreateTime,
                       int protoVer,
                       NavSession* session,
                       RouteRequest* rr )
      : OutGoingNavMessage(recipientAddress, NavMessage::ROUTE_REPLY, false, session),
        m_answer(expansionAnswer), m_routeList(NULL, expansionAnswer, rr ),
        m_routeID( routeID ), m_routeCreateTime( routeCreateTime )
{
   m_protoVer = protoVer;
   /* Do NULL-checks */
   if ( m_answer == NULL ) {
      setStatus( NAV_STATUS_NOT_OK );
   } else {
      Packet* p = m_answer->getPacket();
      ExpandRouteReplyPacket* erp =
         dynamic_cast<ExpandRouteReplyPacket*>(p);
      if ( erp != NULL && erp->getStatus() == StringTable::OK ) {
         setStatus( NAV_STATUS_OK );
      } else {
         setStatus( NAV_STATUS_NOT_OK );
      }
   }
}

RouteReply::~RouteReply()
{
   delete m_answer;
}


uint32 
RouteReply::getRouteID() const {
   return m_routeID;
}


uint32 
RouteReply::getRouteCreateTime() const {
   return m_routeCreateTime;
}


const RouteList& 
RouteReply::getRouteList() const {
   return m_routeList;
}


/*------------------------------------------------------------------*/
