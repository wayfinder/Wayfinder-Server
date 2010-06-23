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

#include "CoordinateObject.h"
#include "CoordinatePacket.h"
#include "OrigDestInfo.h"
#include "TopRegionRequest.h"
#include "Properties.h"
#include "MapBits.h"
#include "STLUtility.h"

CoordinateObject::CoordinateObject(const RequestData& parentOrID,
                                   const TopRegionRequest* topReq,
                                   StringTable::languageCode language)
      : RequestWithStatus(parentOrID),
        m_language( language )
{
   m_coordNotices.reserve(4);
   m_status = StringTable::TIMEOUT_ERROR;
   m_topReq = topReq;
}


CoordinateObject::~CoordinateObject()
{
   STLUtility::deleteValues(m_coordNotices);
   // TODO: Clear m_packetsReadyToSend

}


PacketContainer* 
CoordinateObject::getNextPacket()
{
   // The packet to return
   PacketContainer* retPacketCont = m_packetsReadyToSend.getMin();
   if (retPacketCont != NULL) {
      m_packetsReadyToSend.remove(retPacketCont);
   }

   return (retPacketCont);
}


void 
CoordinateObject::processPacket(PacketContainer* packetCont)
{
   if ( (packetCont != NULL) && 
        (packetCont->getPacket() != NULL) &&
        (packetCont->getPacket()->getSubType() == 
            Packet::PACKETTYPE_COORDINATEREPLY)) {
      CoordinateReplyPacket* coordReplyPacket = 
         static_cast<CoordinateReplyPacket*>(packetCont->getPacket());
      uint16 packetID = coordReplyPacket->getPacketID();
      CoordNotice* notice = getNoticeWithPacketID(packetID);
      if (notice != NULL) {
         // Got the notice that created this packet
         // Remove the packetID from this notice
         notice->m_packetIDs.remove(packetID);
         
         if (coordReplyPacket->getStatus() == StringTable::NOT_UNIQUE) {
            // The reply contatins mapIDs, not item IDs. 
            
            // Set nbr outstanding packets
            const uint32 nbrMaps = coordReplyPacket->getNbrPossibleMaps();
            mc2dbg4 << "nbrMaps = " << nbrMaps << endl;

            // Add one packet to the tree for each mapID
            for (uint32 i=0; i<nbrMaps; i++) {
               uint16 pID = getNextPacketID();
               notice->m_packetIDs.addLast(pID);
               CoordinateRequestPacket* p = 
                  new CoordinateRequestPacket(
                     pID,
                     getID(),
                     notice->m_lat,
                     notice->m_lon,
                     0, 0,          // orig IP + port
                     notice->m_angle,
                     ItemTypes::getLanguageCodeAsLanguageType( 
                        m_language ) );
               p->addOutDataType(notice->m_outDataTypes);
               p->setMapID(coordReplyPacket->getPossibleMap(i));
               for (uint32 j=0; j<notice->m_nbrItemTypes; j++) {
                  p->addItemType(notice->m_itemTypes[j]);
               }

               mc2dbg4 << "Inserting new CoordinateRequestPacket for map "
                       << coordReplyPacket->getPossibleMap(i) << endl;

               // Insert into ready-queue and add packetID to notice
               m_packetsReadyToSend.add(
                  new PacketContainer(p, 0, 0, MODULE_TYPE_MAP));
            }
            mc2dbg4 << nbrMaps << " packets inserted into tree" << endl;

         } else if ( coordReplyPacket->getStatus() == 
                     StringTable::MAPNOTFOUND ) 
         {
            // There is no map at the coordinate, out of map.
            mc2dbg2 << "CoordObj::processPacket(): "
                   << "Updating notice with packetID = " << packetID 
                   << " lat " << notice->m_lat << " lon " << notice->m_lon 
                   << " to be MAPNOTFOUND!" << endl;
            delete notice->m_replyPacket;
            notice->m_replyPacket = static_cast<CoordinateReplyPacket*> (
               coordReplyPacket->getClone() );
            notice->m_minDist = MAX_UINT32;
         } else {
            // The reply contains an item ID
            
            if (coordReplyPacket->getDistance() < notice->m_minDist) {
               mc2dbg4 << "CoordObj::processPacket(): Updating notice with "
                          "packetID = " << packetID << endl;
               delete notice->m_replyPacket;
               notice->m_replyPacket = 
                  static_cast<CoordinateReplyPacket*>
                  (coordReplyPacket->getClone());
               notice->m_minDist = coordReplyPacket->getDistance();
            }
         }
      } else {
         mc2log << warn << "CoordObj::processPacket(): Got reply with unknown "
                   "packet ID: " << packetID << endl;
      }
      if( packetCont != NULL )
         delete packetCont;
   } else {
      mc2log << error << "CoordObj::processPacket(): Got reply with unknown "
                "type or with NULL-packet" << endl;
   }
}


int 
CoordinateObject::addCoordinate(int32 lat,
                                int32 lon,
                                uint32 outDataTypes,
                                byte nbrAllowedItemTypes,
                                const byte* itemTypes,
                                uint16 angle)
{
   // Create the CoordNotice and insert that into the array 
   CoordNotice* n = new CoordNotice(lat, lon, outDataTypes,
                                    nbrAllowedItemTypes, itemTypes,
                                    angle);
   uint16 packetID = getNextPacketID();
   n->m_packetIDs.addLast(packetID);
   m_coordNotices.push_back(n);
   int retVal = m_coordNotices.size() - 1;
   DEBUG4( mc2dbg4 << "Adding packet with ID=" << packetID 
           << " to new Notice at position " << retVal << endl );

   // Create the packet-contatiner and insert that into the ready queue.
   CoordinateRequestPacket* p = 
      new CoordinateRequestPacket(packetID,
                                  getID(),
                                  lat, lon,
                                  0, 0,         // orig IP + port
                                  angle,
                                  ItemTypes::getLanguageCodeAsLanguageType(
                                     m_language ) );
   p->addOutDataType(outDataTypes);
   for (uint32 j=0; j<nbrAllowedItemTypes; j++) {
      p->addItemType(itemTypes[j]);
   }

   // map set handling
   uint32 mapSet = MAX_UINT32;
   if (Properties::getProperty("MAP_SET_COUNT") != NULL) {
      const TopRegionMatch* reg =
                     m_topReq->getTopRegionByCoordinate(MC2Coordinate(lat, lon));
      if (reg != NULL) {
         // We need to get on of the region's map IDs to get the map set ID:
         set<uint32> mapIDs;
         mc2dbg4 << "[CoordObj] coord in top region: " << reg->getID() << endl;
         reg->getItemIDTree().getTopLevelMapIDs(mapIDs);
         mc2dbg4 << "[CoordObj] mapIDs.begin: " 
                 << prettyMapIDFill(*(mapIDs.begin())) << endl;
         mapSet = MapBits::getMapSetFromMapID(*(mapIDs.begin()));
         mc2dbg4 << "[CoordObj] mapSet value now: " << mapSet << endl;
      }
   }

   m_packetsReadyToSend.add(new PacketContainer(p, 0, 0, MODULE_TYPE_MAP,
                                       PacketContainer::defaultResendTimeoutTime,
                                       PacketContainer::defaultResends, 
                                       mapSet));
   
   return (retVal);
}

int
CoordinateObject::addOrigDestInfo(OrigDestInfo& info)
{
   byte allowed = ItemTypes::streetSegmentItem;
   
   return addCoordinate(info.getLat(),
                        info.getLon(),
                        0, // All outdatatypes
                        1, // One allowed item type
                        &allowed,
                        info.getAngle());
                        
}

bool
CoordinateObject::getDone() const
{
   // Done if all the packetIDs in the notices are empty
   bool retVal = true;
   uint32 pos = 0;
   while ( (retVal) && (pos < m_coordNotices.size())) {
      CoordNotice* tmp = static_cast<CoordNotice*>(m_coordNotices[pos]);
      if (tmp != NULL) {
         retVal = (tmp->m_packetIDs.getSize() == 0);
      }
      pos++;
   }

   return (retVal);
}

bool
CoordinateObject::requestDone()
{
   return getDone();
}

StringTable::stringCode
CoordinateObject::getStatus() const
{
   if ( getDone() ) {
      return StringTable::OK;
   } else {
      return m_status;
   }
}

CoordinateReplyPacket*
CoordinateObject::getCoordinateReply(uint32 i) 
{
   if (i < m_coordNotices.size()) {
      mc2dbg4 << "CoordObj::getCoordinateReply(): Returning packet from "
                 "position " << i << endl;
      return (m_coordNotices[i]->m_replyPacket);
   }

   return (NULL);

}


int
CoordinateObject::fillOrigDestInfo(uint32 index,
                                   OrigDestInfo& info)
{
   CoordinateReplyPacket* p = getCoordinateReply(index);
  
   if ( p == NULL )
      return -1;

   // OK - set mapID and nodeID
   int dirFromZero = p->getDirFromZero(info.getAngle());

   // If we don't know we use node 1. Probably bad when not routing
   uint32 itemID = p->getItemID();
   if ( dirFromZero == 0 ) { // It is node 1
      itemID = itemID | 0x80000000;
   }
      
   // Set stuff.
   info.setMapID(p->getMapID());
   info.setNodeID(itemID);
   return index;
}

CoordinateObject::CoordNotice* 
CoordinateObject::getNoticeWithPacketID(uint16 pID)
{
   CoordNotice* retVal = NULL;
   uint32 pos = 0;
   while ( (retVal == NULL) && (pos < m_coordNotices.size())) {
      CoordNotice* tmp = m_coordNotices[pos];
      if ( (tmp != NULL) && 
           (tmp->m_packetIDs.linearSearch(pID) < MAX_UINT32)) {
         retVal = tmp;
      }
      pos++;
   }
   return (retVal);
}

