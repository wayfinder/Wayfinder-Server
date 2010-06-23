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

#include "DATEXRequest.h"

#include "Connection.h"
#include "RouteTypes.h"
#include "RoutePacket.h"
#include "ExpandRoutePacket.h"
#include "RequestUserData.h"
#include "GfxUtility.h"
#include "CoveredIDsRequest.h"
#include "TrafficPointRequest.h"
#include "AddDisturbanceData.h"
#include "STLUtility.h"
#include "DeleteHelpers.h"

#include <memory>

class UserUser; 

DATEXRequest::DATEXRequest(uint16 requestID,
                           TrafficSituation* trafficSituation,
                           const TopRegionRequest* topReq)
   : Request(requestID)
{
   if(trafficSituation != NULL)
      m_trafficSituations.push_back(trafficSituation);
   m_topReq = topReq;
   m_state = processSituation();
   m_tmcFailed = false;
}

DATEXRequest::DATEXRequest(uint16 requestID,
                           vector<TrafficSituation*> trafficVector,
                           const TopRegionRequest* topReq,
                           MC2String supplier,
                           vector<MC2String> toBeKept)
   : Request(requestID)
{
   m_trafficSituations = trafficVector;
   m_topReq = topReq;
   m_state = processSituation();
   m_tmcFailed = false;
}

DATEXRequest::~DATEXRequest()
{
   while (!m_routeRequests.empty()) {
      delete m_routeRequests.back()->getAnswer();
      delete m_routeRequests.back();
      m_routeRequests.pop_back();
   }
}

void
DATEXRequest::init()
{
   m_streetSegmentRequestPackets.clear();
   while ( ! m_idRequestPackets.empty() ) {
      //XXX Since this container contains pointers, shouldn't we delete them?
      m_idRequestPackets.pop();
   }
   m_addRequestPackets.clear();
   m_nbrRequestPackets = 0;
   m_routeRequests.clear();
   m_nbrRouteRequests = 0;
   m_mapID.clear();
   m_nodeID.clear();
   m_coords.clear();
   m_tmcFirst.clear();
   m_tmcSecond.clear();
   m_angle.clear();
   m_sent = 0;
   m_received = 0;
   m_lowLevelNodes.clear();
   m_lowLevelPairToHighLevel.clear();
   m_streetReplyPackets.clear();

   m_firstLocation = "";
   m_secondLocation = "";
   m_tmcFailed = false;

   m_coveredIDreq = NULL;
   
   m_status = StringTable::NOTOK;

   m_originsCovered = false;

   m_originIDs.clear();
   m_destinationIDs.clear();
   m_distance = 0;
   
}

DATEXRequest::datex_state
DATEXRequest::processSituation()
{
   mc2log << "[DATEX] processSituation, " 
          << m_trafficSituations.size() << " situations left"
          << endl;
   // Clear and init the member variables.
   init();

   // TODO: Here we need to redo how we process a situation. As it is right now
   // we only look at the last situation in the vector of TrafficSituations.
   if( m_trafficSituations.size() > 0 ) {
      mc2dbg8 << "Size = " << m_trafficSituations.size() << endl;
      TrafficSituation* trafficSituation = m_trafficSituations.back();
      m_situationReference = trafficSituation->getSituationReference();
      mc2dbg8 << "situation reference = " << m_situationReference << endl;
      m_locationTable = trafficSituation->getLocationTable();
      mc2dbg8 << "Location table = " << m_locationTable << endl;
      TrafficSituationElement* trafficElem =
         trafficSituation->getSituationElements()[0];
      m_elementReference = trafficElem->getElementReference();
      mc2dbg8 << "elementReference = " << m_elementReference << endl;
      m_startTime = trafficElem->getStartTime();
      mc2dbg8 <<  "Start time = " << m_startTime << endl;
      m_expiryTime = trafficElem->getExpiryTime();
      mc2dbg8 << "Expiry time = " << m_expiryTime << endl;
      m_creationTime = trafficElem->getCreationTime();
      mc2dbg8 << "Creation time = " << m_creationTime << endl;
      m_type = trafficElem->getType();
      mc2dbg8 << "type = " << (int) m_type << endl;
      m_phrase = trafficElem->getPhrase();
      mc2dbg8 << "phrase = " << (int) m_phrase << endl;
      m_eventCode = trafficElem->getEventCode();
      mc2dbg8 << "event code = " << m_eventCode << endl;
      m_severity = trafficElem->getSeverity();
      mc2dbg << "[DATEX]" << " severity = " << (int) m_severity << endl;
      m_severityFactor = trafficElem->getSeverityFactor();
      mc2dbg2 << "severity factor = " << (int) m_severityFactor << endl;
      m_costFactor = TrafficDataTypes::getCostFactorFromSeverity(m_severity);
      if( m_costFactor != MAX_UINT32) {
         m_costFactor = (uint32)( (float)m_costFactor * 
               TrafficDataTypes::
               getCostFactorFromSeverityFactor(m_severityFactor) );
      }
      mc2dbg << "[DATEX]" << " costFactor = " << m_costFactor << endl;

      m_text = trafficElem->getText();
      mc2dbg8 << "text = " << m_text << endl;
      m_queueLength = trafficElem->getQueueLength();
      mc2dbg8 << "queue length = " << m_queueLength << endl;

      m_firstLocation = trafficElem->getFirstLocationCode();
      m_secondLocation = trafficElem->getSecondLocationCode();

      m_extent = trafficElem->getExtent();
      mc2dbg8 << "extent = " << (int) m_extent << endl;
      m_direction = trafficElem->getDirection();
      mc2dbg8 << "direction = " << (int) m_direction << endl;
      m_tmcCoords = trafficElem->getCoordinates();
      m_trafficSituations.pop_back();
      if( m_tmcCoords.size() > 1 ) {
         m_tmcFirst.push_back( m_tmcCoords.front() );
         m_tmcSecond.push_back( m_tmcCoords.back() );
      } else if ( m_tmcCoords.size() == 1 ) {
         m_tmcFirst.push_back( m_tmcCoords.front() );
      } 

      if( ! m_firstLocation.empty() ){
         //Use TMC to find start and end point of disturbance
         mc2log << "[DATEX] creating GetTMCCoordinateRequestPacket" << endl;
         return createGetTMCCoordinateRequestPacket();
      } else {
         if( m_tmcCoords.size() > 1 ) {
            //We have coordinates already. Get covered segment ids.

            //m_tmcFirst.push_back(pair<int32, int32>
            //                     (m_tmcCoords[0]));
            //m_tmcSecond.push_back(pair<int32, int32>
            //                      (m_tmcCoords[m_tmcCoords.size()-1]));
            mc2log << "[DATEX] creating coveredids request"  << endl;
            //return createRouteRequest();
            return createCoveredIDsRequestRequest();
         } else if ( m_tmcCoords.size() == 1 ) {
            //We have a single coordinate. Find the segment.

            mc2log << "[DATEX] creating street segm requests"  << endl;
            //m_tmcFirst.push_back(pair<int32, int32>
            //                     (m_tmcCoords[0]));
            return createStreetSegmentItemRequestPackets();
         } else {
            //We can do nothing! Nothing I tell you!
            m_done = true;
            return STATE_ERROR;
         }
      }
   } else {
      m_done = true;
      return STATE_DONE;
   }
}

void
DATEXRequest::processPacket(PacketContainer* packet)
{
   ReplyPacket* reply = 
      packet ? static_cast<ReplyPacket*>(packet->getPacket()) : NULL;
   if( packet != NULL && reply != NULL ) {
         switch(m_state) {
            case STATE_TMC:
               m_state = processGetTMCCoordinateReplyPacket(packet);
               break;
            case STATE_STREET_SEGMENT_ITEM:
               m_state = processStreetSegmentItemReplyPacket(packet);
               break;
            case STATE_COVERED_IDS:
               if ( processSubRequestPacket(m_trafficPointReq.get(),
                                            //m_coveredIDreq,
                                            packet,
                                            m_status) ) {
                  // Other request done
                  if ( m_status == StringTable::OK ) {
                     m_state = handleCoveredIDsDone();
                  } else {
                     m_state = STATE_ERROR;
                  }
               } else {
                  // Keep on truckin' 
               }
               break;   
            case STATE_ROUTE_REQUEST:
               m_state = processRouteReply(packet);
               break;
            case STATE_TOP_REGION_REQUEST:
               m_state = processTopRegionReplyPacket(packet);
               break;
            case STATE_ID_TRANSLATION_REQUEST:
               m_state = processIDTranslationReplyPacket(packet);
               break;           
            case STATE_ADD_DISTURBANCE:
               m_state = processAddDisturbanceReplyPacket(packet);
               break;
            default:
               mc2dbg4 << error << "[DATEXRequest]: processPacket() in "
                      << "invalid state!" << endl;
               delete packet;
               setDone(true);
               m_state = STATE_ERROR;
               break;
      }
   }
}

const char* toCString(TrafficDataTypes::direction direction)
{
   switch(direction){
   case TrafficDataTypes::Positive:       return "Positive Direction";
   case TrafficDataTypes::Negative:       return "Negative Direction";
   case TrafficDataTypes::BothDirections: return "BothDirections";
   case TrafficDataTypes::NoDirection:    return "NoDirection";
   }
   return "Invalid";
}

DATEXRequest::datex_state
DATEXRequest::createGetTMCCoordinateRequestPacket()
{
   //convert tmc ids to ???
   mc2dbg2 << "[DATEX]: Creating GetTMCCoordinateRequestPacket." << endl;
   GetTMCCoordinateRequestPacket* packet =
      new GetTMCCoordinateRequestPacket(getNextPacketID(),
                                        getID() );
   
   mc2dbg8 << "[DATEX]: Requesting " << m_firstLocation << ", " 
           << m_secondLocation << ", "  << m_extent << ", " 
           << toCString(m_direction) << endl;

   packet->addPoints(m_firstLocation,
                     m_secondLocation,
                     m_extent,
                     m_direction);   
   
   enqueuePacket(packet, MODULE_TYPE_TRAFFIC);
   
   return STATE_TMC;
}

DATEXRequest::datex_state
DATEXRequest::processGetTMCCoordinateReplyPacket(PacketContainer* packet)
{
   using namespace STLUtility;
   //this reply contains the 
   mc2dbg2 << "[DATEX]: Processing a GetTMCCoordinateReplyPacket." << endl;
   GetTMCCoordinateReplyPacket* replyPacket =
      static_cast<GetTMCCoordinateReplyPacket*>(packet->getPacket());
   if((replyPacket != NULL) && (replyPacket->getStatus() == StringTable::OK)){
         replyPacket->getCoordinates(m_tmcFirst, m_tmcSecond);
         mc2dbg8 << "[DATEX]: m_tmcFirst: " << co_dump(m_tmcFirst, "\n") 
                 << "\n[DATEX]: m_tmcSecond: " << co_dump(m_tmcSecond, "\n")
                 << endl;
         delete packet;
         mc2dbg8 << "[DATEX]: m_extent: " << m_extent 
                 << ", m_firstLocation:" << m_firstLocation
                 << ", m_tmcCoords,size(): " << m_tmcCoords.size() << endl; 
         if( ! m_tmcCoords.empty() ){
            mc2dbg8 << co_dump(m_tmcCoords, "\n") << endl;
         }

         // If we only have an origin point (FirstLocation) in Alert-C 
         // or if the point isnt supplied check how many coordniate positions
         // we have. If there is only one again, then create a street segment
         // else we are going for a route between origin and destination points.
         if( ( m_extent == 0 && 
               !m_firstLocation.empty() && 
               m_secondLocation.empty() ) ||
             ( m_firstLocation.empty()  && m_tmcCoords.size() == 1 ) ) {
            //we have a single coordinate
            return createStreetSegmentItemRequestPackets();
         } else {
            return createRouteRequest();
         }
   } else {

      mc2log << "[DATEX]:: GetTMCCoordinateReplyPacket " 
             << (replyPacket != NULL ? "NOT OK" : "(null)") << endl
             << ", m_firstLocation " << m_firstLocation
             << ", secondLocation: "  << m_secondLocation
             << ", situationReference " << m_situationReference << endl
             << ", text " << m_text 
             << endl;
      
      delete packet;
      // Lets see if we can use coords instead
      mc2dbg2 << "[DATEX] tmcCoords size: " << m_tmcCoords.size() << endl;
      if( m_tmcCoords.size() > 1 ) {
         mc2dbg2 << "[DATEX] more than 1 tmcCoord. Use " << m_tmcCoords[0] 
                 << " and " << m_tmcCoords.back() << endl;
         m_tmcFailed = true;
         m_tmcFirst.push_back( m_tmcCoords.front() );
         m_tmcSecond.push_back( m_tmcCoords.back() );
         if( replyPacket != NULL ){
            return createRouteRequest();
         } else {
            return createCoveredIDsRequestRequest();
         }               
      } else if ( m_tmcCoords.size() == 1 ) {
         mc2dbg2 << "[DATEX] exactly 1 tmcCoord: " 
                 << m_tmcCoords.front() << endl;
         m_tmcFailed = true;
         m_tmcFirst.push_back( m_tmcCoords.front() );
         return createStreetSegmentItemRequestPackets();
      }
      if( replyPacket != NULL){
         //do the next situation
         return processSituation();         
      } else {
         //all is lost
         setDone(true);
         return STATE_ERROR;
      }
      
   }
}

DATEXRequest::datex_state
DATEXRequest::createStreetSegmentItemRequestPackets()
{
   mc2dbg2 <<"[DATEX]: Creating StreetSegmentItemRequest packet(s)." << endl;
   
   int index = 0;
   vector<pair<int32, int32> >::iterator it;
   for(it = m_tmcFirst.begin(); it != m_tmcFirst.end(); it++) {
      const pair<int32, int32>& coordPair = *it;
      const int32& lat = coordPair.first;
      const int32& lon = coordPair.second;
      StreetSegmentItemRequestPacket* packet =
         new StreetSegmentItemRequestPacket(getNextPacketID(),
                                            getID(),
                                            index,
                                            lat,
                                            lon,
                                            TrafficDataTypes::NoDirection);
      mc2dbg8 << "[DATEX]: Request segment on " << coordPair << endl;
      m_streetSegmentRequestPackets.push_back(packet);
      index++;
   }   
   m_nbrRequestPackets = m_streetSegmentRequestPackets.size();
   enqueuePacket( m_streetSegmentRequestPackets.front(), MODULE_TYPE_MAP );
   m_streetSegmentRequestPackets.pop_front();
   m_sent = 1;
   m_received = 0;
   
   return STATE_STREET_SEGMENT_ITEM;
}


void
DATEXRequest::processStreetReplyPackets()
{
   mc2log << "[DATEX]: processStreetReplyPackets" << endl;
   uint32 minDistance = MAX_UINT32;
   uint32 index = 0;
   
   uint32 mapID = 0;
   int32 lat = 0;
   int32 lon = 0;
   uint32 firstAngle = 0;
   uint32 secondAngle = 0;
   uint32 firstNodeID = 0;
   uint32 secondNodeID = 0;
   typedef multimap<uint32, PacketContainer*>::iterator mm_iterator_t;
   while( m_streetReplyPackets.count(index) > 0 ) {
      pair<mm_iterator_t, mm_iterator_t> range = 
         m_streetReplyPackets.equal_range(index);
      for( mm_iterator_t it = range.first; it != range.second; ++it ) {
         PacketContainer* packet = it->second;
         StreetSegmentItemReplyPacket* currPacket =
            static_cast<StreetSegmentItemReplyPacket*>(packet->getPacket());
         uint32 distance = currPacket->getDistance();
         if( distance < minDistance) {
            mapID = currPacket->getMapID();
            lat = currPacket->getLatitude();
            lon = currPacket->getLongitude();
            firstAngle = currPacket->getFirstAngle();
            secondAngle = currPacket->getSecondAngle();
            firstNodeID = currPacket->getFirstNodeID();
            secondNodeID = currPacket->getSecondNodeID();
            minDistance = distance;
         }
      }
      uint32 nbrCoords = m_mapID.size();
      bool found = false;
      for(uint32 i = 0; i < nbrCoords; i++) {
         uint32 currMapID = m_mapID[i];
         int32 currLat = m_coords[i].first;
         int32 currLon = m_coords[i].second;
         if( (currMapID == mapID) && (currLat == lat) && (currLon == lon) ) {
            found = true;
         }
      }
      if( !found ) {
         if( firstNodeID != MAX_UINT32 ) {
            mc2dbg4 << "[DATEX]: pushing firstnode " << MC2HEX(mapID)
                    << " : " << make_pair(lat, lon) << " : " 
                    << firstAngle << " : " << firstNodeID << endl;
            m_mapID.push_back(mapID);
            m_coords.push_back(pair<int32, int32>(lat, lon));
            m_angle.push_back(firstAngle);
            m_nodeID.push_back(firstNodeID);
         }
         if( secondNodeID != MAX_UINT32 ) {
            mc2dbg4 << "[DATEX]: pushing secondNode " << MC2HEX(mapID)
                    << " : " << make_pair(lat, lon) << " : " 
                    << secondAngle << " : " << secondNodeID << endl;
            m_mapID.push_back(mapID);
            m_coords.push_back(pair<int32, int32>(lat, lon));
            m_angle.push_back(secondAngle);
            m_nodeID.push_back(secondNodeID);
         } 
      }
      index++;
      minDistance = MAX_UINT32;
   }
   for(mm_iterator_t it = m_streetReplyPackets.begin(); 
       it != m_streetReplyPackets.end(); ++it ){
      delete it->second;
   }
   m_streetReplyPackets.clear();
} 


DATEXRequest::datex_state
DATEXRequest::processStreetSegmentItemReplyPacket(PacketContainer* packet)
{
   mc2log << "[DATEX]: Processing an StreetSegmentItemReplyPacket" << endl;
   //so we do not have to delete
   m_received++;
   StreetSegmentItemReplyPacket* replyPacket =
      static_cast<StreetSegmentItemReplyPacket*>(packet->getPacket());
   if( (replyPacket != NULL) &&
       (replyPacket->getStatus() == StringTable::OK) ) {
      m_streetReplyPackets.insert(pair<uint32, PacketContainer*>
                                  (replyPacket->getIndex(), packet));
      
      if( m_received < m_nbrRequestPackets ) {
         mc2dbg2 << "[DATEX]: StreetSegmentItemRequestPacket received" << endl;
         if( m_sent < m_nbrRequestPackets ) {
            mc2dbg2 << "[DATEX]: Sending next StreetSegmentRequest" << endl;
            enqueuePacket( m_streetSegmentRequestPackets.front(), 
                           MODULE_TYPE_MAP );
            m_streetSegmentRequestPackets.pop_front();
            m_sent++;
         }
         return STATE_STREET_SEGMENT_ITEM;
      } else {
         mc2dbg2 << "[DATEX]: All StreetSegmentItemReplyPackets received"
                 << endl;
         
         processStreetReplyPackets();
         return createTopRegionRequestPacket();
      }
   } else if( (replyPacket != NULL) &&
              (replyPacket->getStatus() == StringTable::NOT_UNIQUE) ) {

      mc2dbg2 << "[DATEXRequest]: processStreetSegmentItemReplyPacket() "
              << "mapID not unique, creating new packets to send."
              << endl;
      
      uint32 index = replyPacket->getIndex();
      uint32 nbrMaps = replyPacket->getNbrMapIDs();
      for(uint32 i = 0; i < nbrMaps; i++) {
         uint32 currentMapID = replyPacket->getMapID(i);
         mc2dbg4 << "[DATEX]: map[" << index <<  "] " << MC2HEX(currentMapID)
                 << " " << m_tmcFirst[index] << endl;
         StreetSegmentItemRequestPacket* newPacket =
            new StreetSegmentItemRequestPacket(getNextPacketID(),
                                               getID(),
                                               index,
                                               m_tmcFirst[index].first,
                                               m_tmcFirst[index].second,
                                               TrafficDataTypes::NoDirection);
         
         newPacket->setMapID(currentMapID);
         m_nbrRequestPackets++;
         m_streetSegmentRequestPackets.push_back(newPacket);
      }
      m_sent++;
      enqueuePacket( m_streetSegmentRequestPackets.front(), MODULE_TYPE_MAP );
      m_streetSegmentRequestPackets.pop_front();
      delete packet;
      return STATE_STREET_SEGMENT_ITEM;
   }  else {
      mc2dbg2 << "[DATEXRequest]: StreetSegmentItemReplyPacket NOTOK!" << endl;
      setDone(true);
      delete packet;
      return STATE_ERROR;
   }
}

DATEXRequest::datex_state
DATEXRequest::createCoveredIDsRequestRequest()
{
   mc2log << "[DATEX]: Creating " 
          << (m_originsCovered ? "destination" : "origin" )
          << " CoveredIDRequest(s)." << endl;
   
   set<ItemTypes::itemType> itemTypes;
   itemTypes.insert(ItemTypes::streetSegmentItem);
   
   MC2Coordinate  firstCoord(m_tmcCoords[m_tmcCoords.size()-1].first,
                             m_tmcCoords[m_tmcCoords.size()-1].second);
   MC2Coordinate  lastCoord(m_tmcCoords[0].first, 
                            m_tmcCoords[0].second);
            
   MC2Coordinate  theCoord;
   m_distance = sqrt(
      GfxUtility::squareP2Pdistance_linear(firstCoord, lastCoord));
   mc2log << "float64 distance = "  << m_distance << endl;
   
   float64 angle = GfxUtility::getAngleFromNorth(firstCoord.lat,
                                                 firstCoord.lon,
                                                 lastCoord.lat,
                                                 lastCoord.lon);
   uint16 degAngle = uint16(angle*180.0 / 3.14159265);
   int maxRC = 4;
   if(m_distance > 10000)
      maxRC = 1;
   else if (m_distance > 3000)
      maxRC = 2;
   else if (m_distance > 1000)
      maxRC = 3;
   
   uint32 radiusMeters;
   if(m_distance > 2000)
      radiusMeters = 200;
   else if(m_distance < 500)
      radiusMeters = 50;
   else
      radiusMeters = static_cast<uint32>(m_distance/10);

   
   if(!m_originsCovered){
      mc2log << endl << " Setting covered ID radius to " << radiusMeters
             << " meters" <<  endl <<  endl;
      mc2log << "Origin coord (" << firstCoord.lat << ","
             << firstCoord.lon << ")" << endl;
      theCoord = firstCoord;
      //if((m_direction == TrafficDataTypes::BothDirections)&&
      //   (radiusMeters > 50))
      //   radiusMeters = radiusMeters / 2;
   } else {
      mc2dbg2 << "Destination coord (" << lastCoord.lat << ","
              << lastCoord.lon << ")" << endl;
      theCoord = lastCoord;
      // if(radiusMeters > 50)
      //    radiusMeters = radiusMeters / 2;
      if(degAngle < 180){
         degAngle += 180;
      } else {
         degAngle -= 180;
      }
   }
   
   TrafficDataTypes::direction dir;
   if(m_direction == TrafficDataTypes::BothDirections){
      dir = TrafficDataTypes::BothDirections;
   } else if (!m_originsCovered){
      dir = TrafficDataTypes::Positive;
   } else {
      dir = TrafficDataTypes::Negative;
   }
   
   //m_coveredIDreq = new CoveredIDsRequest(this, theCoord, radiusMeters, 
   //                                       itemTypes, m_topReq);
   mc2log << "[DATEX]::creating new Traffic PointRequest " << endl
          << " center " << theCoord << endl
          << " radius " << radiusMeters << endl
          << " angle  " << degAngle << endl 
          << " " << toCString(dir) << endl;

   m_trafficPointReq.reset( new TrafficPointRequest(this,
                                               theCoord,
                                               radiusMeters, 
                                               2,
                                               degAngle,
                                               dir,
                                               maxRC,
                                               m_topReq) );
   
   if ( enqueuePacketsFromRequest(m_trafficPointReq.get()) != 0 ) {
      return STATE_COVERED_IDS;
   } else {
      m_status = m_trafficPointReq->getStatus();
      return STATE_ERROR;
   }
}

DATEXRequest::datex_state
DATEXRequest::handleCoveredIDsDone()
{
   mc2dbg2 << "[DATEX] handleCoveredIDsDone" << endl;
   //const CoveredIDsRequest::result_t& coveredIDs =
   //   m_coveredIDreq->getCoveredIDs();
   
   const TrafficPointRequest::result_t& coveredIDs =
      m_trafficPointReq->getTrafficPoint();


//   for ( CoveredIDsRequest::result_t::const_iterator it = coveredIDs.begin();
//         it != coveredIDs.end(); ++it ) {
   for(TrafficPointRequest::result_t::const_iterator it = coveredIDs.begin();
       it != coveredIDs.end(); ++it ) {
      if(!m_originsCovered){
        // m_originIDs.insert(IDPair_t(it->second.getMapID(),
         //                            it->second.getItemID()));
         mc2dbg4 << "[DATEX] Adding origin ID " << *it << endl;
         m_originIDs.insert(IDPair_t(it->getMapID(),
                                     it->getItemID()));
      } else {
//         m_destinationIDs.insert(IDPair_t(it->second.getMapID(),
//                                          it->second.getItemID()));
         mc2dbg4 << "[DATEX] Adding destination ID " << *it << endl;
         m_destinationIDs.insert(IDPair_t(it->getMapID(),
                                          it->getItemID()));
      }
      
   }
   if(!m_originsCovered){
      mc2log << "Found " << m_originIDs.size() << " origin IDs " << endl;
      m_originsCovered = true;
      return createCoveredIDsRequestRequest();
   }

   mc2log << "Found " << m_destinationIDs.size() << " destination IDs "
          << endl;
   
   mc2log << "[DATEXRequest]: Creating IDRouteRequest(s)." << endl;
   
   const uint32 expandType = ( ROUTE_TYPE_STRING | 
                               ROUTE_TYPE_GFX |
                               ROUTE_TYPE_GFX_TRISS |
                               ROUTE_TYPE_ITEM_STRING );

   ItemTypes::vehicle_t routingVehicle = ItemTypes::passengerCar;
   
   if(m_distance < 1000){
      //   cerr << "Setting ItemTypes::pedestrian." << endl;
      //routingVehicle = ItemTypes::pedestrian;
   }
   
   RouteRequest* req = new RouteRequest( (UserUser*)NULL,  
                                         getID(),
                                         expandType,
                                         StringTable::ENGLISH,
                                         false,
                                         0,
                                         m_topReq,
                                         NULL,
                                         this);
   
   req->setRouteParameters(false,
                           0, 1, 0, 0,
                           routingVehicle,
                           0, 0);
   mc2dbg8 << "[DATEX] route origins: " 
           << STLUtility::co_dump(m_originIDs, "\n") << endl;
   for(set<IDPair_t>::iterator it = m_originIDs.begin(); 
       it != m_originIDs.end(); it++){
      req->addOriginID((*it).getMapID(), (*it).getItemID());
   }
   
   mc2dbg8 << "[DATEX] route destinations: " 
           << STLUtility::co_dump(m_destinationIDs, "\n") << endl;
   for(set<IDPair_t>::iterator it = m_destinationIDs.begin(); 
       it != m_destinationIDs.end(); it++){
      req->addDestinationID((*it).getMapID(), (*it).getItemID());
   }
   m_routeRequests.push_back(req);
   mc2log << "m_direction " << toCString(m_direction) << endl;
   if( m_direction ==  TrafficDataTypes::BothDirections ) {
      mc2log << "[DATEX] Request route for the other direction." << endl;
      req = new RouteRequest( (UserUser*)NULL,
                              getID(),
                              expandType,
                              StringTable::ENGLISH,
                              false,
                              0,
                              m_topReq,
                              NULL,
                              this);
      
      req->setRouteParameters(false,
                              0, 1, 0, 0,
                              routingVehicle,
                              0, 0);
      for(set<IDPair_t>::iterator it = m_originIDs.begin(); 
          it != m_originIDs.end(); it++){
         req->addDestinationID((*it).getMapID(), (*it).getItemID());
      }
      
      for(set<IDPair_t>::iterator it = m_destinationIDs.begin(); 
          it != m_destinationIDs.end(); it++){
         req->addOriginID((*it).getMapID(), (*it).getItemID());
      }
      
      m_routeRequests.push_back(req);
   }
   m_nbrRouteRequests = m_routeRequests.size();
   m_packetsReadyToSend.add(m_routeRequests.back()->getNextPacket());
   return STATE_ROUTE_REQUEST;
}



DATEXRequest::datex_state
DATEXRequest::createRouteRequest()
{
   mc2log << "[DATEX]: Creating RouteRequest(s)." << endl;
   
   const uint32 expandType = ( ROUTE_TYPE_STRING |
                               ROUTE_TYPE_GFX |
                               ROUTE_TYPE_GFX_TRISS |
                               ROUTE_TYPE_ITEM_STRING );
   
   RouteRequest* req = new RouteRequest( (UserUser*)NULL, /* User */
                                         getID(),
                                         expandType,
                                         StringTable::ENGLISH,
                                         false,
                                         0,
                                         m_topReq,
                                         NULL,
                                         this);
      
   req->setRouteParameters(false,
                           0, 1, 0, 0,
                           ItemTypes::passengerCar,
                           0, 0);

   mc2log << "[DATEX] m_tmcFirst " << m_tmcFirst.size() << endl;
   mc2log << "[DATEX] m_tmcSecond " << m_tmcSecond.size() << endl;
   
   vector<pair<int32, int32> >::const_iterator it;
   for(it = m_tmcSecond.begin(); it != m_tmcSecond.end(); it++) {
      pair<int32, int32> coordPair = *it;
      uint16 degAngle = 0;
      if(m_secondLocation.empty() && m_tmcCoords.size() > 1){
         pair<int32, int32> coordNext = m_tmcCoords[m_tmcCoords.size()-2];
         float64 angle = GfxUtility::getAngleFromNorth(coordNext.first,
                                                       coordNext.second,
                                                       coordPair.first,
                                                       coordPair.second);
         degAngle = uint16(angle*180.0 / 3.14159265);

         mc2log << "*************************************************" << endl
                << " SitRef :" << m_situationReference << endl
                << coordPair << endl
                << "Setting origin angle " << degAngle << endl
                << "*************************************************" << endl;
      }

      mc2dbg8 << "[DATEX] addOrigin " << coordPair << " angle: " 
              << degAngle << endl;

      // Check the direction. 
      // Since the primary location indicates the origin of the situation 
      // and the second location is at the other end of the affected length 
      // or area. 
      // So if the direction is positive we are going to route from the
      // second location to the first.
      // Or if its both directions we will make the positive route first
      // and then a route in the opposite direction.
      // If it is negative we are going to route from the first to the second.
      if( m_direction == TrafficDataTypes::Positive ||
          m_direction == TrafficDataTypes::BothDirections ) {
         req->addOriginCoord(coordPair.first, coordPair.second, degAngle);
      } else if ( m_direction ==  TrafficDataTypes::Negative ) {
         req->addDestinationCoord(coordPair.first, coordPair.second, degAngle);
      }
   }

   for(it = m_tmcFirst.begin(); it != m_tmcFirst.end(); it++) {
      pair<int32, int32> coordPair = *it;
      uint16 degAngle = 0;
      if(m_firstLocation.empty() == 0 && m_tmcCoords.size() > 1){
         pair<int32, int32> coordNext = m_tmcCoords[1];
         float64 angle = GfxUtility::getAngleFromNorth(coordPair.first,
                                                       coordPair.second,
                                                       coordNext.first,
                                                       coordNext.second);
         degAngle = uint16(angle*180.0 / 3.14159265);
         mc2log << "*************************************************" << endl
                << " SitRef :" << m_situationReference << endl
                << coordPair << endl
                << "Setting dest angle " << degAngle << endl
                << "*************************************************" << endl;
      }      
      mc2dbg8 << "[DATEX] addDestination " << coordPair 
              << " angle: " << degAngle << endl;

      // Check the direction. 
      // Since the primary location indicates the origin of the situation 
      // and the second location is at the other end of the affected length 
      // or area. 
      // So if the direction is positive we are going to route from the
      // second location to the first.
      // Or if its both directions we will make the positive route first
      // and then a route in the opposite direction.
      // If it is negative we are going to route from the first to the second.
      if( m_direction == TrafficDataTypes::Positive ||
          m_direction == TrafficDataTypes::BothDirections ) {
         req->addDestinationCoord(coordPair.first, coordPair.second, degAngle);
      } else if ( m_direction ==  TrafficDataTypes::Negative ) {
         req->addOriginCoord(coordPair.first, coordPair.second, degAngle);
      }
   }  
   m_routeRequests.push_back(req);

   // Add the other route request if we have both directions.
   if( m_direction ==  TrafficDataTypes::BothDirections ) {
      mc2log << "[DATEX] Request route in other direction." << endl;
      req = new RouteRequest( (UserUser*)NULL,
                              getID(),
                              expandType,
                              StringTable::ENGLISH,
                              false,
                              0,
                              m_topReq,
                              NULL,
                              this);
      
      req->setRouteParameters(false,
                              0, 1, 0, 0,
                              ItemTypes::passengerCar,
                              0, 0);
      for(it = m_tmcFirst.begin(); it != m_tmcFirst.end(); it++) {
         pair<int32, int32> coordPair = *it;
         req->addOriginCoord(coordPair.first,
                             coordPair.second);
      }
      for(it = m_tmcSecond.begin(); it != m_tmcSecond.end(); it++) {
         pair<int32, int32> coordPair = *it;
         req->addDestinationCoord(coordPair.first,
                                  coordPair.second);
      }  
      m_routeRequests.push_back(req);
   }
   m_nbrRouteRequests = m_routeRequests.size();
   m_packetsReadyToSend.add(m_routeRequests.back()->getNextPacket());
   return STATE_ROUTE_REQUEST;
}


DATEXRequest::datex_state
DATEXRequest::processRouteReply(PacketContainer* pack)
{
   mc2log << "[DATEX] processRouteReply" << endl;
   m_routeRequests.back()->processPacket(pack);
   delete pack;
   if( m_routeRequests.back()->requestDone() ) {
      return handleRouteReply();
   }   
   else {
      mc2log << "[DATEX] sending next routerequest packet" << endl;
      PacketContainer* packet = m_routeRequests.back()->getNextPacket();
      if( packet != NULL ) {
         m_packetsReadyToSend.add(packet);
         return STATE_ROUTE_REQUEST;
      } else {
         return handleRouteReply();
      }
   }
}


void
DATEXRequest::getAngle(ExpandItemID* expItemID,
                       uint32* &angle)
{
   IntVector& latVect = expItemID->getLat();
   IntVector& lonVect = expItemID->getLon();
   Vector& offsetVect = expItemID->getCoordinateOffset();
   
   uint32 nbrOfItems = expItemID->getNbrItems();
      
   int32 lat1, lat2;
   int32 lon1, lon2;
   
   for(uint16 i = 0; i < nbrOfItems; i++) {
      uint32 offset = offsetVect.getElementAt(i); 
      lat1 = latVect.getElementAt(offset);
      lat2 = latVect.getElementAt(offset+1);
      lon1 = lonVect.getElementAt(offset);
      lon2 = lonVect.getElementAt(offset+1);
      
      float64 deltaY = float64( lat2 - lat1 );
      float64 deltaX = float64( lon2 - lon1 );
      uint32 q;
      
      // Find the correct quadrant, q
      if( deltaX > 0 ) {
         if( deltaY >= 0 )
            q = 1;
         else
            q = 4;
      }
      else if( deltaX < 0 ) {
         if ( deltaY >= 0 )
            q = 2;
         else
            q = 3;
      }
      else {
         if( deltaY >= 0 )
            q = 1;
         else
            q = 3;
      }
      
      float64 alfa = 0;
      
      // Calculate the angle between the two points
      switch (q) {
         // First quadrant
         case 1: {
            alfa = float64( atan( float64(deltaY / deltaX)) );
            alfa = float64( alfa / (2 * M_PI) * 360 );
         }
         break;
         // Second quadrant
         case 2: {
            alfa =
               float64( atan( float64( deltaY / fabs(deltaX) ) ) );
            alfa = float64( alfa / (2 * M_PI) * 360 );
            alfa = float64( alfa + 90 );
            if( (alfa == 90) && (deltaX < 0) )
               alfa = float64( alfa + 90 );
         }
         break;
         // Third quadrant
         case 3: {
            alfa =
               float64( atan( float64( fabs(deltaY) / fabs(deltaX) ) ) );
            alfa = float64( alfa / (2 * M_PI) * 360 );
            alfa = float64( alfa + 180 );
         }
         break;
         // Fourth quadrant 
         case 4: {
            alfa = float64( atan( float64( (fabs(deltaY) / deltaX) ) ) );
            alfa = float64( alfa / (2 * M_PI) * 360 );
            alfa = float64( alfa + 270 );
         }
         break;
      }
      // Convering to integer
      angle[i] = uint32(alfa + 0.5);
   }   
}


DATEXRequest::datex_state
DATEXRequest::handleRouteReply()
{
   mc2dbg2 << "[DATEXRequest]: Handling RouteReply" << endl;
   PacketContainer* packet =
      m_routeRequests.back()->getAnswer();
   if( packet != NULL ) {
      ExpandRouteReplyPacket* replyPacket =
         static_cast<ExpandRouteReplyPacket*>(packet->getPacket());
      if( replyPacket->getStatus() == StringTable::OK ) {
         auto_ptr<ExpandItemID> expandItemID( replyPacket->getItemID() );
         uint32 nbrOfItems = expandItemID->getNbrItems();
         vector<uint32> angle(nbrOfItems);
         Vector& itemID = expandItemID->getItemID();
         Vector& mapID = expandItemID->getMapID();            
         uint32* angle_p = &(angle.front());
         getAngle( expandItemID.get(), angle_p );
         for(uint32 i = 0; i < nbrOfItems; i++) {
            m_mapID.push_back(mapID.getElementAt(i));
            m_nodeID.push_back(itemID.getElementAt(i));
            m_coords.push_back(pair<int32, int32>
                               (expandItemID->getLatValue(i, 0),
                                expandItemID->getLonValue(i, 0)));
            m_angle.push_back(angle[i]);
            mc2dbg4 << "[DATEX] item[" << i << "]: " << MC2HEX(m_mapID.back())
                    << " : " << MC2HEX(m_nodeID.back()) << " : " 
                    << m_coords.back() << " : " << m_angle.back() << endl;
         }
      } else {
         mc2dbg << "[DATEX]: Route reply not ok!" << endl;
         setDone(true);
         return STATE_ERROR;
      }
      if( !m_routeRequests.empty() ) {
         delete m_routeRequests.back()->getAnswer();
         delete m_routeRequests.back();
         m_routeRequests.pop_back();
      }
      if( m_routeRequests.size() > 0 ) {
         mc2dbg2 << "[DATEX] starting next routerequest." << endl;
         m_packetsReadyToSend.add(m_routeRequests.back()->getNextPacket());
         return STATE_ROUTE_REQUEST;
      } else {
         return createTopRegionRequestPacket();
      }
   } else {
      mc2dbg << "[DATEX] Routepacket = NULL" << endl;
      setDone(true);
      return STATE_ERROR;
   }
}

DATEXRequest::datex_state
DATEXRequest::createTopRegionRequestPacket()
{
   mc2dbg2 << "[DATEX]: Creating TopRegionRequestPacket" << endl;
   TopRegionRequestPacket* packet = 
      new TopRegionRequestPacket( getID(), getNextPacketID() );
   enqueuePacket(packet, MODULE_TYPE_MAP);
   return STATE_TOP_REGION_REQUEST;
}

DATEXRequest::datex_state
DATEXRequest::processTopRegionReplyPacket(PacketContainer* packet_p)
{
   auto_ptr<PacketContainer> packet( packet_p );
   mc2dbg2 << "[DATEX]: Processing TopRegetionReplyPacket" << endl;
   TopRegionReplyPacket* replyPacket =
      static_cast<TopRegionReplyPacket*>(packet->getPacket());
   if( replyPacket != NULL ) {
      {
         //delete m_mapIDTree;
         //m_mapIDTree = NULL;
         m_mapIDTree.reset( new ItemIDTree );
         TopRegionMatchesVector topRegions;
         replyPacket->getTopRegionsAndIDTree( topRegions, *m_mapIDTree );
         STLUtility::deleteValues( topRegions );
      }

      //      delete packet;
      return createIDTranslationRequestPacket();
   } else {
      mc2dbg << "[DATEX]: TopRegionReplyPacket NOTOK!" << endl;
      //      delete packet;
      setDone(true);
      return STATE_ERROR;
   } 
}


DATEXRequest::datex_state
DATEXRequest::createIDTranslationRequestPacket()
{
   mc2dbg2 << "[DATEX]: Creating IDTranslationRequestPacket." << endl;
   // Puts all the mapIDs in a set. the m_mapID vector contains each
   // map id sevreal times so that m_mapID and m_nodeID makes pairs of
   // map id and node id on the same index. The set will only contain
   // each map id once.
   set<uint32> mapIDs(m_mapID.begin(), m_mapID.end());

   // Puts all the nodeIDs in the request packet, one packet for each
   // mapID

   mc2dbg4 << "[DATEX] Number of maps = " << mapIDs.size() << endl;
   mc2dbg4 << "[DATEX] Number of nodes = " << m_nodeID.size() << endl;
   //foreach of our map ids...
   for(set<uint32>::iterator setIt = mapIDs.begin(); 
       setIt != mapIDs.end(); setIt++) {
      uint32 mapID = *setIt;
      mc2dbg8 << "[DATEX] map " << MC2HEX(mapID) << endl;
      IDPairVector_t nodeVector;
      int index = 0;
      //...find the items in the m_mapID that match...
      for(vector<uint32>::iterator it = m_mapID.begin(); 
          it != m_mapID.end(); it++) {
         uint32 currMapID = *it;
         if( currMapID == mapID ) {
            //...and add it to the translation vector. 
            uint32 nodeID = m_nodeID[index];
            nodeVector.push_back(IDPair_t(currMapID, nodeID));
         }
         index++;
      }
      mc2dbg8 << "[DATEX] nodes to translate\n" << hex 
              << STLUtility::co_dump(nodeVector, "\n") << dec << endl;

      //then find the id of the map one layer up
      uint32 overviewMapID = m_mapIDTree->getHigherLevelMap( mapID );
      if ( overviewMapID != MAX_UINT32 ) {
         mc2dbg4 << "[DATEX] higher map " << MC2HEX(overviewMapID) << endl;
         IDPairVector_t* redir = &nodeVector;
         //create the packet that gets the node ids on the map one level up
         IDTranslationRequestPacket* IDPacket =
            new IDTranslationRequestPacket(getNextPacketID(),
                                           getID(),
                                           overviewMapID,
                                           false,
                                           &redir,
                                           1);
         //add the packet to the m_idRequestPackets vector. Packets from
         //this vector will be sent one at at time
         m_idRequestPackets.push(IDPacket);
      }
   }
   m_received = 0; //reset the received counter. What does it count. 
   
   if( !m_idRequestPackets.empty() ) {
      //send the first packet. received packets handled in
      //processIDTranslationReplyPacket.
      IDTranslationRequestPacket* packet = m_idRequestPackets.front();
      if( packet != NULL ) {
         mc2dbg << "[DATEX] Sending IDTranslationRequestPacket." << endl;
         IDPairVector_t().swap( m_lowLevelNodes ); //clear m_lowLevelNodes
         packet->getAllNodes( m_lowLevelNodes );
         enqueuePacket( packet, MODULE_TYPE_MAP );
         m_idRequestPackets.pop();
         return STATE_ID_TRANSLATION_REQUEST;
      } else {
         mc2log << "[DATEX]::IDTranslationRequestPacket == NULL!!!" << endl;
         setDone(true);
         return STATE_ERROR;
      }
   } else {
      mc2log << "[DATEX]: No IDTranslationRequestsPackets!!!" << endl;
      setDone(true);
      return STATE_ERROR;
   }
}
   
DATEXRequest::datex_state
DATEXRequest::processIDTranslationReplyPacket(PacketContainer* packet)
{
   mc2dbg2 << "[DATEX]: Processing a IDTranslationReplyPacket " << endl;   
   IDTranslationReplyPacket* replyPacket =
      static_cast<IDTranslationReplyPacket*>(packet->getPacket());
   if( replyPacket != NULL ) {
      //fetch the translated nodes into highVector
      IDPairVector_t highVector;
      replyPacket->getTranslatedNodes(0, highVector);
      const uint32 size = highVector.size();
      //m_lowLevelPairToHighLevel contains map and node of lower
      //mapped to map and node of higher map.
      mc2log << "[DATEX] " << m_lowLevelNodes.size() << " lowlevelnodes, "
             << highVector.size() << " highlevelnodes" << endl;
      MC2_ASSERT( m_lowLevelNodes.size() == highVector.size() );
      for( uint32 i = 0; i < size; ++i ) {
         m_lowLevelPairToHighLevel[ m_lowLevelNodes[i] ] = highVector[i];
      }

      //debugging
      vector<pair<IDPair_t,IDPair_t> > ovector(highVector.size());
      transform(m_lowLevelNodes.begin(), m_lowLevelNodes.end(), 
                highVector.begin(),
                ovector.begin(),
                &make_pair<IDPair_t,IDPair_t>);
      mc2dbg8 << "[DATEX] translation \n" 
              << STLUtility::co_dump(ovector, "\n") << endl;

      
      //remove all (MAX_UINT32, MAX_UINT32) pairs 
      highVector.erase( remove( highVector.begin(), 
                                highVector.end(),  
                                IDPair_t() ),
                        highVector.end() );

      //find the first mapid in highVector...
      uint32 highLevelMap = 
         highVector.empty() ? MAX_UINT32 : highVector.front().getMapID();
      //...and the map one level beyond that
      uint32 evenHigherMapID =  m_mapIDTree->getHigherLevelMap( highLevelMap );
            
      if(evenHigherMapID != MAX_UINT32){

         mc2dbg8 << "[DATEX] translate from map " << MC2HEX(highLevelMap) 
                 << " to " << MC2HEX(evenHigherMapID) << endl;
         //there might be another map. translate to it.
         IDPairVector_t* redir = &highVector;
         IDTranslationRequestPacket* IDPacket =
            new IDTranslationRequestPacket(getNextPacketID(),
                                           getID(),
                                           evenHigherMapID,
                                           false,
                                           &redir,
                                           1);
         //add the packet to the m_idRequestPackets vector. Packets from
         //this vector will be sent one at at time
         m_idRequestPackets.push(IDPacket);
         
      } else {
         mc2dbg4 << "[DATEX] No further translation of theses nodes necessary."
                 << endl;
      }

      if( ! m_idRequestPackets.empty() ) {
         //send next id translation packet
         mc2dbg << "[DATEX] send next id translation packet." << endl;
         IDPairVector_t().swap(m_lowLevelNodes); //clear m_lowLevelNodes
         m_idRequestPackets.front()->getAllNodes(m_lowLevelNodes);
         enqueuePacket(m_idRequestPackets.front(), MODULE_TYPE_MAP);
         m_idRequestPackets.pop();
         delete packet;
         return STATE_ID_TRANSLATION_REQUEST;
      }
      delete packet;
     
      return createAddDisturbanceRequestPacket();
   } else {
      mc2dbg << "[DATEXRequest]: IDTranslationReplyPacket NOTOK!" << endl;
      delete packet;
      setDone(true);
      return STATE_ERROR;
   }
}
   

DATEXRequest::datex_state
DATEXRequest::createAddDisturbanceRequestPacket()
{
   mc2dbg2 << "[DATEX]: Creating AddDisturbanceRequestPacket." << endl;  
   //create the set of unique lowlevel map ids.
   set<uint32> mapIDs( m_mapID.begin(), m_mapID.end() );
   
   if(m_tmcFailed){ //????
      mc2log << "TMC location has failed, setting no severity" << endl;
      m_severity   = TrafficDataTypes::NotProvided;
      m_costFactor = 1010;
   }
   
   
   int n = 0; //what an aptly named variable! WTF does it do!!!!
              //Apparently it counts the disturbances added to any
              //AddDisturbanceRequestPacket. It is used to enumerate
              //the routes associated with each disturbance. Should be
              //the same for AddDisturbanceRequestPackets for
              //different map levels?.
   //for each map id...
   typedef map<uint32, AddDisturbanceRequestData> DisturbanceMap;
   //this map will keep the data before we create the packets
   DisturbanceMap adPrepare;
   //create a AddDisturbanceRequestData object for each map in m_mapID
   //and all of the upper maps of those.
   {
      //Template AddDisturbanceRequestData object. 
      AddDisturbanceRequestData data( m_type, m_phrase, m_eventCode, 
                                      m_startTime, m_expiryTime, 
                                      m_creationTime, m_severity,
                                      m_direction, m_extent, 
                                      m_costFactor, m_queueLength );
      data.addStrings(m_firstLocation, m_secondLocation,
                      m_situationReference, m_text);
      mc2log << "[DATEX] Creating AddDisturbanceRequests for situation " 
             << m_situationReference << endl;
      for(set<uint32>::iterator it = mapIDs.begin(); it != mapIDs.end(); ++it){
         //iterate upwards over maps 
         for ( uint32 mapID = *it; mapID != MAX_UINT32; 
               mapID = m_mapIDTree->getHigherLevelMap( mapID ) ) {
            adPrepare[mapID] = data;
         }
      }
   }
   //for each disturbed node in the bottom layer, add it to the
   //AddDisturbanceRequestData object for the right map. Also find
   //corresponding nodes upwards in the map tree and add the nodes to
   //the AddDisturbanceRequestData for that map.
   mc2dbg8 << "[DATEX] Adding disturbed coords" << endl;
   for ( vector<uint32>::size_type i = 0; i < m_mapID.size(); ++i){
      IDPair_t map_node(m_mapID[i], m_nodeID[i]);
      while ( map_node.isValid() ) {
         //find the AddDisturbanceRequestData object for this map...
         DisturbanceMap::iterator q = adPrepare.find( map_node.getMapID() );
         if(q != adPrepare.end()){
            //... and add the disturbed node to it
            mc2dbg8 << "[DATEX] " << prettyMapIDFill(map_node.getMapID()) 
                    << " " << m_coords[i] << " " 
                    << setw(3) << m_angle[i] << " " << n << endl;
            q->second.addDisturbance( map_node.getItemID(), m_coords[i],
                                      m_angle[i], n);
         } else {
            mc2dbg2 << "[DATEX] couldn't find data for map " 
                    << MC2HEX(map_node.getMapID()) << endl;
         }
         //find the node one map level up 
         map<IDPair_t, IDPair_t>::iterator upOne = 
            m_lowLevelPairToHighLevel.find( map_node );
         if( upOne != m_lowLevelPairToHighLevel.end() ){
            map_node = upOne->second;
         } else {
            map_node = IDPair_t();
         }
      }
      ++n; //I don't think this makes sense.
   }
   //Create an AddDisturbanceRequestPacket for each of the
   //AddDisturbanceRequestData objects in the map. Collect the packets
   //in m_addRequestPackets.
   for( DisturbanceMap::iterator it = adPrepare.begin(); 
        it != adPrepare.end(); ++it ){
      AddDisturbanceRequestPacket* packet = 
         new AddDisturbanceRequestPacket( getNextPacketID(), getID(), 
                                          it->second );
      packet->setMapID(it->first);
      mc2dbg2 << "[DATEX] creating AddDisturbanceRequestPacket for map " 
              << MC2HEX(it->first) << endl;
      m_addRequestPackets.push_back(packet);      
   }

   m_nbrRequestPackets = m_addRequestPackets.size();
   m_received = 0;
   mc2dbg << "[DATEX] Sending AddDisturbanceRequestPacket "
          << MC2HEX(m_addRequestPackets.front()->getMapID()) << endl;
   enqueuePacket( m_addRequestPackets.front(), MODULE_TYPE_TRAFFIC );
   m_addRequestPackets.pop_front();
   
   return STATE_ADD_DISTURBANCE;   
}


DATEXRequest::datex_state
DATEXRequest::processAddDisturbanceReplyPacket(PacketContainer* packet_p)
{
   auto_ptr<PacketContainer> packet( packet_p );
   mc2dbg2 << "[DATEXRequest]: Processing AddDisturbanceReplyPacket " << endl;
   AddDisturbanceReplyPacket* replyPacket =
      static_cast<AddDisturbanceReplyPacket*>(packet->getPacket());
   
   if( (replyPacket != NULL) &&
       (replyPacket->getStatus() == StringTable::OK) ) {
      m_received++;
      mc2dbg2 << "[DATEX] m_received: " << m_received << endl;
      if( m_received < m_nbrRequestPackets ) {
         uint32 disturbanceID = replyPacket->getDisturbanceID();
         mc2dbg2 << "[DATEX]: AddDisturbanceReply received, id: " 
                 << disturbanceID << endl;
         AddDisturbanceRequestPacket* addPacket =
            m_addRequestPackets.front();
         addPacket->setDisturbanceID(disturbanceID);

         enqueuePacket(addPacket, MODULE_TYPE_TRAFFIC);
         m_addRequestPackets.pop_front();
         
         return STATE_ADD_DISTURBANCE;
      }
      else {
         mc2dbg2 << "[DATEXRequest]: All AddDisturbanceReplyPackets received! "
                << endl;
         return processSituation();
      }
   }
   else {
      mc2log << error 
             << "[DATEXRequest]: AddDisturbanceReplyPacket NOT OK!"
             << endl;
      m_done = true;
      return STATE_ERROR;      
   }
}











