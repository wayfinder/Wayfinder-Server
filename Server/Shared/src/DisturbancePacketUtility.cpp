/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DisturbancePacketUtility.h"

namespace DisturbancePacketUtility {

int calcPacketSize( const DisturbanceElement& disturbance ) {
   static const int disturbanceSize =
      4    // disturbanceID
      + 4  // type
      + 4  // phrase
      + 4  // eventCode
      + 4  // startTime
      + 4  // endTime
      + 4  // creation time
      + 4  // severity
      + 4  // direction
      + 4  // extent
      + 4  // extra Cost
      + 4  // mapID
      + 4; // queue length
 
   static const int coordSize =
      4    // routeIndex
      + 4  // lat
      + 4  // lon
      + 4  // nodeID
      + 4; // angle

   int size = disturbanceSize;
   size += disturbance.getFirstLocation().size() + 1;
   size += disturbance.getSecondLocation().size() + 1;
   size += disturbance.getText().size() + 1;
   size += disturbance.getSituationReference().size() + 1;
   AlignUtility::alignLong( size );
   size += coordSize * disturbance.getNbrCoordinates();
   size += 4; // nbr coordinates

   return size;
}

void writeToPacket( Packet& packet, int& packetPosition,
                    const DisturbanceElement& currentDist ) {

   packet.incWriteLong( packetPosition, currentDist.getDisturbanceID() );
   packet.incWriteLong( packetPosition, int(currentDist.getType() ) );
   packet.incWriteLong( packetPosition, int(currentDist.getPhrase() ) );
   packet.incWriteLong( packetPosition, int(currentDist.getEventCode() ) );
   packet.incWriteLong( packetPosition, currentDist.getStartTime() );
   packet.incWriteLong( packetPosition, currentDist.getEndTime() );
   packet.incWriteLong( packetPosition, currentDist.getCreationTime() );
   packet.incWriteLong( packetPosition, int(currentDist.getSeverity() ) );
   packet.incWriteLong( packetPosition, int(currentDist.getDirection() ) );
   packet.incWriteLong( packetPosition, currentDist.getExtent() );
   packet.incWriteLong( packetPosition, currentDist.getCostFactor() );
   packet.incWriteLong( packetPosition, currentDist.getMapID() );
   packet.incWriteLong( packetPosition, currentDist.getQueueLength() );
      
   packet.incWriteString( packetPosition,
                          currentDist.getFirstLocation().c_str() );
   packet.incWriteString( packetPosition,
                          currentDist.getSecondLocation().c_str() );
   packet.incWriteString( packetPosition,
                          currentDist.getText().c_str() );
   packet.incWriteString( packetPosition,
                          currentDist.getSituationReference().c_str() );

   AlignUtility::alignLong( packetPosition );
      
   map<uint32, int32> latVector = currentDist.getLatMap();
   map<uint32, int32> lonVector = currentDist.getLonMap();
   map<uint32, uint32> nodeVector = currentDist.getNodeID();
   map<uint32, uint32> angleVector = currentDist.getAngle();
   vector<uint32> indexVector = currentDist.getRouteIndex();

   vector<uint32>::iterator indexIt;
   map<uint32, int32>::iterator latlonIt;
   map<uint32, uint32>::iterator it;
      
   packet.incWriteLong( packetPosition, currentDist.getNbrCoordinates() );
   for ( indexIt = indexVector.begin(); indexIt != indexVector.end();
         indexIt++) {
      uint32 routeIndex = *indexIt;
      packet.incWriteLong( packetPosition, routeIndex );
         
      latlonIt = latVector.find( routeIndex );
      int32 lat = latlonIt->second;
      packet.incWriteLong( packetPosition, lat );

      latlonIt = lonVector.find( routeIndex );
      int32 lon = latlonIt->second;
      packet.incWriteLong( packetPosition, lon );
         
      it = nodeVector.find( routeIndex );
      uint32 nodeID = it->second;
      packet.incWriteLong( packetPosition, nodeID );
         
      it = angleVector.find(routeIndex);
      uint32 angle = it->second;
      packet.incWriteLong( packetPosition, angle );
   }

}

DisturbanceElement*
readDistFromPacket( const Packet& packet, int& packetPosition ) {

   uint32 disturbanceID = uint32(packet.incReadLong(packetPosition));
   TrafficDataTypes::disturbanceType type =
      TrafficDataTypes::disturbanceType(packet.incReadLong(packetPosition));
   TrafficDataTypes::phrase phrase =
      TrafficDataTypes::phrase(packet.incReadLong(packetPosition));
   uint32 eventCode = uint32(packet.incReadLong(packetPosition));
   uint32 startTime = uint32(packet.incReadLong(packetPosition));
   uint32 endTime = uint32(packet.incReadLong(packetPosition));
   uint32 creationTime = uint32(packet.incReadLong(packetPosition));
   TrafficDataTypes::severity severity =
      TrafficDataTypes::severity(packet.incReadLong(packetPosition));
   TrafficDataTypes::direction direction =
      TrafficDataTypes::direction(packet.incReadLong(packetPosition));
   uint32 extent = uint32(packet.incReadLong(packetPosition));
   uint32 costFactor = uint32(packet.incReadLong(packetPosition));
   uint32 mapID = uint32(packet.incReadLong(packetPosition));
   uint32 queueLength = uint32(packet.incReadLong(packetPosition));
      
   char* temp;
   MC2String text, situationReference;
   MC2String firstLocation, secondLocation;
   packet.incReadString(packetPosition, temp);
   firstLocation = temp;
   packet.incReadString(packetPosition, temp);
   secondLocation = temp;
   packet.incReadString(packetPosition, temp);
   text = temp;
   packet.incReadString(packetPosition, temp);
   situationReference = temp;
      
   int n = packetPosition % 4;
   if( n != 0 ) {
      packetPosition = packetPosition + (4 - n);
   }
   DisturbanceElement* distElem =
      new DisturbanceElement( disturbanceID,
                              situationReference,
                              type,
                              phrase,
                              eventCode,
                              startTime,
                              endTime,
                              creationTime,
                              severity,
                              direction,
                              firstLocation,
                              secondLocation,
                              extent,
                              costFactor,
                              text,
                              queueLength );
   distElem->setMapID(mapID);
      
   uint32 size = uint32(packet.incReadLong(packetPosition));
   for(uint32 i = 0; i < size; i++) {
      uint32 routeIndex = uint32(packet.incReadLong(packetPosition));
      int32 lat = int32(packet.incReadLong(packetPosition));
      int32 lon = int32(packet.incReadLong(packetPosition));
      uint32 nodeID = uint32(packet.incReadLong(packetPosition));
      uint32 angle = uint32(packet.incReadLong(packetPosition));
         
      distElem->addCoordinate(nodeID,
                              lat,
                              lon,
                              angle,
                              routeIndex);
   }

   return distElem;
}

void getDisturbances( vector< DisturbanceElement* > &dists,
                      const Packet* packet,
                      int& packetPosition,
                      bool& removeDisturbances,
                      bool& removeAll ) {
   int nbrOfDisturbances = (int)packet->incReadLong(packetPosition);
   dists.resize( nbrOfDisturbances );
   for(int i = 0; i < nbrOfDisturbances; i++) {
      DisturbanceElement* distElem = readDistFromPacket( *packet,
                                                         packetPosition );
      
      dists[ i ] = distElem;
   }
   byte b = packet->incReadByte(packetPosition);
   if( (b & 0x01) == 0x01 ) {
      removeDisturbances = true;
   } else {
      removeDisturbances = false;
   }
   if( (b & 0x02) == 0x02 ) {
      removeAll = true;
   } else {
      removeAll = false;
   }
}

void getDisturbances( map<uint32,DisturbanceElement*> &distMap,
                      const Packet* packet,
                      int& packetPosition,
                      bool& removeDisturbances,
                      bool& removeAll ) {
   int nbrOfDisturbances = (int)packet->incReadLong(packetPosition);
   for(int i = 0; i < nbrOfDisturbances; i++) {
      DisturbanceElement* distElem = readDistFromPacket( *packet,
                                                         packetPosition );
      distMap.insert( make_pair( distElem->getDisturbanceID(), distElem ) );
   }
   byte b = packet->incReadByte(packetPosition);
   if( (b & 0x01) == 0x01 ) {
      removeDisturbances = true;
   } else {
      removeDisturbances = false;
   }
   if( (b & 0x02) == 0x02 ) {
      removeAll = true;
   } else {
      removeAll = false;
   }
}

} // DisturbancePacketUtility
