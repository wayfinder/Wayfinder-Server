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

#include<set>

#include "Packet.h"
#include "PacketContainer.h"
#include "TrafficPointRequest.h"
#include "ServerProximityPackets.h"
#include "TopRegionRequest.h"
#include "Properties.h"
#include "BBoxPacket.h"

#define SETSTATE(x)(m_state = x)

TrafficPointRequest::TrafficPointRequest(const RequestData& parentOrID,
                                         const MC2Coordinate& center,
                                         uint32 maxRadius,
                                         int nbrOfHits,
                                         uint16 angle,
                                         TrafficDataTypes::direction
                                         direction,
                                         uint8 maxRoadClass,
                                         const TopRegionRequest* topReq)
      : RequestWithStatus(parentOrID)
{
   
   init(center, maxRadius, nbrOfHits, angle, direction, maxRoadClass, topReq);
}



void
TrafficPointRequest::init(const MC2Coordinate& center,
                          uint32 maxRadius,
                          int nbrOfHits,
                          uint16 angle,
                          TrafficDataTypes::direction direction,
                          uint8 maxRoadClass,
                          const TopRegionRequest* topReq)
{
   
   m_topReq            = topReq;
   m_center            = center;
   m_maxRadius         = maxRadius;
   m_maxRoadClass      = maxRoadClass;
   m_nbrOfHits         = nbrOfHits;
   m_state             = SENDING_PACKETS;
   m_nbrOutstanding    = 0;
   m_angle             = angle;
   m_direction         = direction;
   mc2log << "[TPR] init TPR c:" << center << ", r: " << maxRadius 
          << ", n: " << nbrOfHits << ", a: " << angle << ", d: " << direction
          << endl;

   sendBoundingBoxRequests();
} 

void 
TrafficPointRequest::sendBoundingBoxRequests()
{
   MC2BoundingBox bbox(m_center, m_maxRadius);
   

   uint32 mapSetCount = Properties::getUint32Property("MAP_SET_COUNT", 
                                                      MAX_UINT32);
   if (mapSetCount != MAX_UINT32){
      mapSetCount--;
   }
   while (mapSetCount != (MAX_UINT32 - 1)) {
      bool underview = true;
      bool country = false;
      BBoxReqPacketData reqData( bbox, underview, country );
      mc2dbg8 << reqData << endl;
      BBoxRequestPacket* req = new BBoxRequestPacket( reqData );
      enqueuePacketContainer(
         new PacketContainer( 
            updateIDs(req), 0, 0, MODULE_TYPE_MAP,
            PacketContainer::defaultResendTimeoutTime,
            PacketContainer::defaultResends, 
            mapSetCount ));
      ++m_nbrOutstanding;

      if (mapSetCount == 0 || mapSetCount == MAX_UINT32) {
         mapSetCount = MAX_UINT32 - 1;
      } else {
         --mapSetCount;
      }
   }
}

inline void 
TrafficPointRequest::handleBBoxPacket(BBoxReplyPacket* packet)
{
   // Get the packet data.
   MC2BoundingBox bbox;
   vector<uint32> mapIDs;
   packet->get( bbox, mapIDs );
   uint32 count = 0;
   // Send packets
   for ( vector<uint32>::const_iterator it = mapIDs.begin();
         it != mapIDs.end();
         ++it, ++count ) {
      uint32 mapID = *it;
      addPacketForSending( createTrafficPointRequestPacket(mapID), 
                           MODULE_TYPE_MAP );
      mc2dbg2 << "[TPR] Sending tpr packet for map " << prettyMapIDFill(mapID)
              << endl;
   }
}

int
TrafficPointRequest::addPacketForSending(Packet* packet,
                                         moduletype_t moduleType)
{
   int packetID = getNextPacketID();
   packet->setRequestID(getID());
   packet->setPacketID(packetID);
   enqueuePacket(packet, moduleType);
   ++m_nbrOutstanding;
   mc2log << "[TPR] sent a packet, " <<  m_nbrOutstanding << " outstanding" 
          << endl;
   return packetID;
}

TrafficPointRequestPacket*
TrafficPointRequest::createTrafficPointRequestPacket(uint32 mapID) const
{
   mc2log << "[TPR] Creating TP request packets" << endl;
   TrafficPointRequestPacket* covPacket =
      new TrafficPointRequestPacket( getUser(), 0, 0, mapID);

   covPacket->addValues(m_center, m_maxRadius, m_nbrOfHits,
                        m_angle, m_direction, m_maxRoadClass);

   return covPacket;
}

inline void
TrafficPointRequest::
handleTrafficPointPacket(const TrafficPointReplyPacket* packet)
{
   if ( packet->getStatus() != StringTable::OK ) {
      mc2dbg2 << "[TPR]: Got packet with "
              << " type = " << packet->getSubTypeAsString() 
              << "status "
             << packet->getStatusAsString() << endl;         
      SETSTATE(ERROR);
      return;
   }
   
   // Check if more packets need sending.
   if ( packet->isMapIDs() ) {
      int nbrMaps = packet->getNumberIDs();
      mc2log << "[TPR]: " << nbrMaps << " maps covered - sending more" << endl;
      for ( int i = 0; i < nbrMaps; ++i ) {
         uint32 curMapID = packet->getID(i);
         addPacketForSending(createTrafficPointRequestPacket(curMapID),
                             MODULE_TYPE_MAP);   
      }
   } else {
      int nbrItems = packet->getNumberIDs();
      uint32 mapID = packet->getMapID();
      mc2log << "[TPR]: Got " << nbrItems << " items from map "
             << mapID << endl;
      
      for ( int i = 0; i < nbrItems; ++i ) {
      // Get the items and put them in the result.
         m_resultIDs.push_back(IDPair_t(mapID, packet->getID(i)) );
      
         mc2dbg8 << "[TPR]: Adding item "
                 << IDPair_t(mapID, packet->getID(i))
                 << endl;
      }
   }
    
   if ( m_nbrOutstanding == 0 ) {
      // No packets to receive - we're done now.
      //mc2dbg2
         mc2log << "[TPR]: Done: Nbr resulting items = "
             << m_resultIDs.size()
             << endl;
      SETSTATE(DONE);
   }
}

void
TrafficPointRequest::processPacket(PacketContainer* pack)
{
   --m_nbrOutstanding;
   // Assume right packettype.
   bool wrongPacket = false;
   Packet* packet = pack->getPacket();
   mc2log << " Receiving packet of type " << (int)packet->getSubType() << endl;
   switch ( m_state ) {
      case SENDING_PACKETS:
         switch ( packet->getSubType() ) {
            case Packet::PACKETTYPE_TRAFFICPOINTREPLYPACKET:
               handleTrafficPointPacket(
                  static_cast<TrafficPointReplyPacket*>(packet));
               break;
            case Packet::PACKETTYPE_BBOXREPLY:
               handleBBoxPacket(static_cast<BBoxReplyPacket*>(packet));
               break;
            default:
               wrongPacket = true;
               break;
         };
         break;
      case DONE:
      case ERROR:
         mc2dbg << "[TPR]; Recevied packet in DONE or ERROR state which is "
                << "wrong"
                << endl;
         break;
   };
   if ( wrongPacket ) {
      mc2log << warn << "[TPR]: Wrong type of packet received" << endl;
      SETSTATE(ERROR);
   }
   delete pack;
}
