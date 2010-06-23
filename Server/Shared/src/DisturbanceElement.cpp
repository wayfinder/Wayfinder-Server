/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DisturbanceElement.h"
#include "UserEnums.h"
#include "UTF8Util.h"
#include "STLUtility.h"

DisturbanceElement::DisturbanceElement():
   m_disturbanceID( INVALID_DISTURBANCE_ID ),
   m_situationReference(),
   m_type( TrafficDataTypes::Accident ),
   m_phrase( TrafficDataTypes::ABL ),
   m_eventCode( 0 ),
   m_startTime( 0 ),
   m_endTime( 0 ),
   m_creationTime( 0 ),
   m_severity( TrafficDataTypes::Blocked ),
   m_direction( TrafficDataTypes::Positive ),
   m_latMap(),
   m_lonMap(),
   m_mapID( MAX_UINT32 ),
   m_nodeIDSet(),
   m_angle(),
   m_routeIndex(),
   m_firstLocation(),
   m_secondLocation(),
   m_extent( 0 ),
   m_costFactor( 0 ),
   m_text(),
   m_queueLength( 0 ),
   m_deleted( false ),
   m_nbrCoordinates( 0 ),
   m_isActive( false ),
   m_neededRights(),
   m_startOffset( MAX_UINT32 )
{

}

DisturbanceElement::DisturbanceElement(uint32 disturbanceID,
                                       const SituationReference& situationReference,
                                       TrafficDataTypes::disturbanceType type,
                                       TrafficDataTypes::phrase phrase,
                                       uint32 eventCode,
                                       uint32 startTime,
                                       uint32 endTime,
                                       uint32 creationTime,
                                       TrafficDataTypes::severity severity,
                                       TrafficDataTypes::direction direction,
                                       const MC2String& firstLocation,
                                       const MC2String& secondLocation,
                                       uint8 extent,
                                       uint32 costFactor,
                                       const MC2String& text,
                                       uint32 queueLength) : 
   m_disturbanceID(disturbanceID),
   m_situationReference(situationReference),
   m_type(type),
   m_phrase(phrase),
   m_eventCode(eventCode),
   m_startTime(startTime),
   m_endTime(endTime),
   m_creationTime(creationTime),
   m_severity(severity),
   m_direction(direction),
   m_mapID(MAX_UINT32),
   m_firstLocation(firstLocation),
   m_secondLocation(secondLocation),
   m_extent(extent),
   m_costFactor(costFactor),
   m_text(text),
   m_queueLength(queueLength),
   m_deleted(false),
   m_nbrCoordinates(0),
   m_isActive(false),
   m_neededRights(),
   m_startOffset(MAX_UINT32)

{
   if ( ! UTF8Util::isValidUTF8( m_text ) ) {
      mc2dbg << "[DisturbanceElement]: Text " << MC2CITE( m_text )
             << " is not valid utf-8. Fix" << endl;
   }
   
   // Check situation reference to decide the provider.
   if((uint32)situationReference.size() >= 6 &&
      (strncasecmp(situationReference.c_str(), "WFTIT", 5) == 0)){
      m_neededRights |= MapRights::WF_TRAFFIC_INFO_TEST;
   } else if((uint32)situationReference.size() >= 4 &&
             (strncasecmp(situationReference.c_str(), "SCDB", 4) == 0)){ 
      m_neededRights |= MapRights::SCDB;
   } else {
      if (type == TrafficDataTypes::Camera){
         m_neededRights |= MapRights::SPEEDCAM;
      } else {
         m_neededRights |= MapRights::TRAFFIC;
      }
   }
}


DisturbanceElement::~DisturbanceElement()
{
  
}

void
DisturbanceElement::addCoordinate(uint32 nodeID,
                                  int32 latitude,
                                  int32 longitude,
                                  uint32 angle,
                                  uint32 routeIndex)
{
   m_nodeID.insert(pair<uint32, uint32>(routeIndex, nodeID));
   m_nodeIDSet.insert(nodeID);
   m_latMap.insert(pair<uint32, int32>(routeIndex, latitude));
   m_lonMap.insert(pair<uint32, int32>(routeIndex, longitude));
   m_angle.insert(pair<uint32, uint32>(routeIndex, angle));
   m_routeIndex.push_back(routeIndex);
   m_nbrCoordinates++;
}

void
DisturbanceElement::addCoordinate(int32 latitude,
                                  int32 longitude,
                                  uint32 angle,
                                  uint32 routeIndex)
{
   m_latMap.insert(pair<uint32, int32>(routeIndex, latitude));
   m_lonMap.insert(pair<uint32, int32>(routeIndex, longitude));
   m_angle.insert(pair<uint32, uint32>(routeIndex, angle));
   m_routeIndex.push_back(routeIndex);
   m_nbrCoordinates++;
}

void
DisturbanceElement::stripData() {
   using namespace STLUtility;
   // clear the text
   swapClear( m_text );
   // clear the lat map
   swapClear( m_latMap );
   // clear the lon map
   swapClear( m_lonMap );
   // clear the route index
   swapClear( m_routeIndex );
   // clear node ids
   swapClear( m_nodeID );
   swapClear( m_nodeIDSet );
   // clear angles
   swapClear( m_angle );
   // clear nbr of coordinates :(
   m_nbrCoordinates = 0;
}

void
DisturbanceElement::removeIndex(uint32 routeIndex)
{
   vector<uint32> newRouteIndex;
   vector<uint32>::iterator it;
   int n = 0;
   for(it = m_routeIndex.begin(); it != m_routeIndex.end(); it++) {
      uint32 currentRouteIndex = *it;
      if( currentRouteIndex != routeIndex ) {
         newRouteIndex.push_back(currentRouteIndex);
         n++;
      }
   }
   m_nbrCoordinates = n;
   
   m_routeIndex.clear();
   m_routeIndex = newRouteIndex;

   m_latMap.erase(routeIndex);
   m_lonMap.erase(routeIndex);
   m_angle.erase(routeIndex);
   m_nodeID.erase(routeIndex);
}

void
DisturbanceElement::addNodeID(uint32 nodeID,
                              uint32 routeIndex,
                              uint32 offset)
{
   if(m_nodeID.empty()){
      m_startOffset = offset;
   }
   
   m_nodeID.insert(pair<uint32, uint32>(routeIndex, nodeID));
   m_nodeIDSet.insert(nodeID);
}
                                  
void
DisturbanceElement::setDisturbanceID(uint32 disturbanceID)
{
   m_disturbanceID = disturbanceID;
}

void
DisturbanceElement::getCoordinates( Coordinates& coords ) const {
   coords.reserve( getLatMap().size() );
   RouteIndex2CoordMap::const_iterator latIt = getLatMap().begin();
   RouteIndex2CoordMap::const_iterator lonIt = getLonMap().begin();
   // assuming latmap.size() == lonmap.size()
   for (; latIt != getLatMap().end(); ++latIt, ++lonIt ) {
      coords.push_back( MC2Coordinate( latIt->second, lonIt->second ) );
   }
}

void
DisturbanceElement::setMapID(uint32 mapID)
{
   m_mapID = mapID;
}

void
DisturbanceElement::setDeleted(bool deleted)
{
   m_deleted = deleted;
}

void
DisturbanceElement::setIsActive(bool isActive)
{
   m_isActive = isActive;
}

bool DisturbanceElement::
operator == ( const DisturbanceElement& other ) const {

#define DIST_COMP( x ) ( x == other.x )

   return 
      DIST_COMP( m_disturbanceID ) &&
      DIST_COMP( m_situationReference ) &&
      DIST_COMP( m_type ) &&
      DIST_COMP( m_phrase ) &&
      DIST_COMP( m_eventCode ) &&
      DIST_COMP( m_startTime ) &&
      DIST_COMP( m_endTime ) &&
      DIST_COMP( m_creationTime ) &&
      DIST_COMP( m_severity ) &&
      DIST_COMP( m_direction ) &&
      DIST_COMP( m_latMap ) &&
      DIST_COMP( m_lonMap ) &&
      DIST_COMP( m_mapID ) &&
      DIST_COMP( m_nodeID ) &&
      DIST_COMP( m_nodeIDSet ) &&
      DIST_COMP( m_angle ) &&
      DIST_COMP( m_routeIndex ) &&
      DIST_COMP( m_firstLocation ) &&
      DIST_COMP( m_secondLocation ) &&
      DIST_COMP( m_extent ) &&
      DIST_COMP( m_costFactor ) &&
      DIST_COMP( m_text ) &&
      DIST_COMP( m_queueLength ) &&
      DIST_COMP( m_deleted ) &&
      DIST_COMP( m_nbrCoordinates ) &&
      DIST_COMP( m_isActive ) &&
      DIST_COMP( m_neededRights ) &&
      DIST_COMP( m_startOffset );

#undef DIST_COMP

}
