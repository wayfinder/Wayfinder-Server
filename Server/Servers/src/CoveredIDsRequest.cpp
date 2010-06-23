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
#include "CoveredIDsRequest.h"
#include "ServerProximityPackets.h"
#include "GfxConstants.h"
#include "TopRegionRequest.h"
#include "Properties.h"
#include "MapBits.h"

#define SETSTATE(x)(m_state = x)

CoveredIDsRequest::CoveredIDsRequest(const RequestData& parentOrID,
                                     const MC2Coordinate& center,
                                     uint32 radiusMeters,
                                     const set<ItemTypes::itemType>& itemTypes,
                                     const TopRegionRequest* topReq)
      : RequestWithStatus(parentOrID)
{
   init(center,
        IDPair_t(MAX_UINT32, MAX_UINT32),
        radiusMeters,
        itemTypes,
        topReq);
}


CoveredIDsRequest::CoveredIDsRequest(const RequestData& parentOrID,
                                     const IDPair_t& centerID,
                                     uint32 radiusMeters,
                                     const set<ItemTypes::itemType>& itemTypes,
                                     const TopRegionRequest* topReq)
      : RequestWithStatus(parentOrID)
{
   init(MC2Coordinate(),
        centerID,
        radiusMeters,
        itemTypes,
        topReq);
}


CoveredIDsRequest::CoveredIDsRequest(const RequestData& parentOrID,
   const MC2BoundingBox& bbox,
   const set<ItemTypes::itemType>& itemTypes,
   const TopRegionRequest* topReq)
      : RequestWithStatus(parentOrID)
{
   // Make center point and radius in meters
   MC2Coordinate center;
   uint32 radius = 0;
   uint32 radiusMeters = 0;
//   bbox.updateCosLat();
   bbox.getCenter( center.lat, center.lon );
   if ( bbox.getWidth() > bbox.getHeight() ) {
      radius = bbox.getWidth() / 2;
   } else {
      radius = bbox.getHeight() / 2;
   }
   radiusMeters = uint32( radius * GfxConstants::MC2SCALE_TO_METER );
   init( center,
         IDPair_t( MAX_UINT32, MAX_UINT32 ),
         radiusMeters,
         itemTypes,
         topReq);
   // Hidden feature that makes the center and radius be a bbox
   m_startAngle        = MAX_UINT16;
   m_stopAngle         = MAX_UINT16;
}

void
CoveredIDsRequest::init(const MC2Coordinate& center,
                        const IDPair_t& centerID,
                        uint32 radiusMeters,
                        const set<ItemTypes::itemType>& itemTypes,
                        const TopRegionRequest* topReq)
{
   m_topReq            = topReq;
   m_center            = center;
   m_centerID          = centerID;
   m_innerRadiusMeters = 0;
   m_outerRadiusMeters = radiusMeters;
   m_itemTypes         = itemTypes;
   m_state             = SENDING_PACKETS;
   m_nbrOutstanding    = 0;
   m_startAngle        = 0;
   m_stopAngle         = 360;

   
   if ( center.isValid() ) {
      // Add packet for sending ( we do not know the mapid yet )
      addPacketForSending(createCoveredIDsRequestPacket(MAX_UINT32),
                          MODULE_TYPE_MAP);
   } else {
      // Add packet for sending when we know the map id.
      addPacketForSending(createCoveredIDsRequestPacket(m_centerID.getMapID()),
                          MODULE_TYPE_MAP);
   }
}

int
CoveredIDsRequest::addPacketForSending(Packet* packet,
                                       moduletype_t moduleType)
{
   int packetID = getNextPacketID();
   packet->setRequestID(getID());
   packet->setPacketID(packetID);
   enqueuePacketContainer( new PacketContainer(packet,
                                               0,
                                               0,
                                               moduleType) );
   ++m_nbrOutstanding;
   return packetID;
}

CoveredIDsRequestPacket*
CoveredIDsRequest::createCoveredIDsRequestPacket(uint32 mapID) const
{
   CoveredIDsRequestPacket* covPacket =
      new CoveredIDsRequestPacket( getUser(), 0, 0, mapID,
                                   m_itemTypes);

   covPacket->setInnerRadius(0);
   covPacket->setOuterRadius(m_outerRadiusMeters);
   covPacket->setStartAngle(m_startAngle);
   covPacket->setStopAngle(m_stopAngle);
   covPacket->setMapID(mapID);
   
   if ( m_center.isValid() ) {
      // Use the coordinate
      // map set handling
      // NOTE this should be changed to send one request to each map set!
      // compare with topregionrequest.
      // See also GfxFeatureMapImageRequest, a genric boundingbox -> mapIDs
      // packet would be best!
      // uses just the center coordinate for now, should be fixed!
      uint32 mapSet = MAX_UINT32;
      if (Properties::getProperty("MAP_SET_COUNT") != NULL) {
         const TopRegionMatch* reg =
                            m_topReq->getTopRegionByCoordinate( m_center );
         if (reg != NULL) {
            // We need to get on of the region's map IDs to get the map set ID:
            set<uint32> mapIDs;
            mc2dbg4 << "[CIR] coord in top region: " << reg->getID() << endl;
            reg->getItemIDTree().getTopLevelMapIDs(mapIDs);
            mc2dbg4 << "[CIR] mapIDs.begin: " << prettyMapIDFill(*(mapIDs.begin())) << endl;
            mapSet = MapBits::getMapSetFromMapID(*(mapIDs.begin()));
            mc2dbg4 << "[CIR] mapSet value now: " << mapSet << endl;
         }
      }
      covPacket->setMapSet(mapSet);
      covPacket->setLat(m_center.lat);
      covPacket->setLon(m_center.lon);
      covPacket->setLatLonAsValid();
   } else {
      covPacket->setItemID( m_centerID.getItemID() );
      covPacket->setItemIDAsValid();
   }
   return covPacket;
}

inline void
CoveredIDsRequest::handleCoveredIDsPacket(const CoveredIDsReplyPacket* packet)
{
   if ( packet->getStatus() != StringTable::OK ) {
      mc2dbg2 << "[CovR]: Got packet with "
             << " type = " << packet->getSubTypeAsString() 
             << "status "
             << packet->getStatusAsString() << endl;         
      SETSTATE(ERROR);
      return;
   }
   
   // Check if more packets need sending.
   if ( packet->isMapIDs() ) {
      int nbrMaps = packet->getNumberIDs();
      mc2dbg2 << "[CovR]: " << nbrMaps << " maps covered - sending more"
             << endl;
      for ( int i = 0; i < nbrMaps; ++i ) {
         uint32 curMapID = packet->getID(i);
         addPacketForSending(createCoveredIDsRequestPacket(curMapID),
                             MODULE_TYPE_MAP);   
      }
   } else {
      int nbrItems = packet->getNumberIDs();
      uint32 mapID = packet->getMapID();
      mc2dbg2 << "[CovR]: Got " << nbrItems << " items from map "
             << mapID << endl;
      
      for ( int i = 0; i < nbrItems; ++i ) {
      // Get the items and put them in the result.
         m_resultIDs.insert(make_pair(packet->getType(i),
                                      IDPair_t(mapID, packet->getID(i)) ) );
         
         mc2dbg8 << "[CovR]: Adding item " << IDPair_t(mapID, packet->getID(i))
                 << " with type " 
                 << ItemTypes::getItemTypeAsString(packet->getType(i))
                 << endl;
      }
   }
   
   if ( m_nbrOutstanding == 0 ) {
      // No packets to receive - we're done now.
      mc2dbg2 << "[CovR]: Done: Nbr resulting items = "
             << m_resultIDs.size()
             << endl;
      SETSTATE(DONE);
   }
}

void
CoveredIDsRequest::processPacket(PacketContainer* pack)
{
   --m_nbrOutstanding;
   // Assume right packettype.
   bool wrongPacket = false;
   Packet* packet = pack->getPacket();
   switch ( m_state ) {
      case SENDING_PACKETS:
         switch ( packet->getSubType() ) {
            case Packet::PACKETTYPE_COVEREDIDSREPLYPACKET:
               handleCoveredIDsPacket(
                  static_cast<CoveredIDsReplyPacket*>(packet));
               break;
            default:
               wrongPacket = true;
               break;
         };
         break;
      case DONE:
      case ERROR:
         mc2dbg << "[CovR]; Recevied packet in DONE or ERROR state which is "
                << "wrong"
                << endl;
         break;
   };
   if ( wrongPacket ) {
      mc2log << warn << "[CovR]: Wrong type of packet received" << endl;
      SETSTATE(ERROR);
   }
   delete pack;
}
