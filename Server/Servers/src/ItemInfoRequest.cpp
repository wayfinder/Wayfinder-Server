/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ItemInfoRequest.h"

#include "ItemInfoPacket.h"
#include "IDPairVector.h"
#include "CoordinatePacket.h"
#include "SearchMatch.h"
#include "ItemInfoEntry.h"
#include "SearchReplyData.h"
#include "ExternalItemInfoPacket.h"
#include "ExternalSearchPacket.h"
#include "DeleteHelpers.h"
#include "ExternalSearchConsts.h"

ItemInfoRequest::ItemInfoRequest( const RequestData& reqData,
                                  const TopRegionRequest* topReq ) 
   : Request(reqData), m_coordinateObject(this, topReq)
{
   m_state = AWAITING_INDATA;

   m_coordinateData.lat = MAX_INT32;
   m_coordinateData.lon = MAX_INT32;
   m_searchReplyData = new SearchReplyData;
   m_origMatch = NULL;
}


ItemInfoRequest::~ItemInfoRequest() 
{
   STLUtility::deleteValues( m_savedPackets );
   delete m_searchReplyData;
   delete m_origMatch;
}

const vector<VanillaMatch*>&
ItemInfoRequest::getMatches() const
{
   return m_searchReplyData->getMatchVector();
}

PacketContainer* 
ItemInfoRequest::getNextPacket() 
{
   // The packet to return
   PacketContainer* retPacketCont = m_packetsReadyToSend.getMin();
   if (retPacketCont != NULL) {
      m_packetsReadyToSend.remove(retPacketCont);
   }

   switch (m_state) {
      case SENDING_COORDINATE_REQUEST  :
      case AWAITING_COORDINATE_REPLY  :
         if (retPacketCont != NULL) {
            mc2dbg2 << "[IIR]:   Changing state to AWAITING_COORDINATE_REPLY"
                   << endl;
            m_state = AWAITING_COORDINATE_REPLY;
         }
      break;

      case SENDING_ITEMINFO :
      case AWAITING_ITEMINFO :
         if (retPacketCont != NULL) {
            mc2dbg2 << "[IIR]:   Changing state to AWAITING_ITEMINFO"
                   << endl;
            m_state = AWAITING_ITEMINFO;
         }
      break;

      case DONE :
      case ERROR :
         // Nothing to do
         break;

      default: 
         mc2log << error << "[IIR]: getNextPacket(), unhandled state ("
                << uint32(m_state) << ")"
                << endl;
      break;
   }

   // Return
   return retPacketCont;
}


void 
ItemInfoRequest::processPacket(PacketContainer* packCont) 
{
   mc2dbg2 << "ItemInfoRequest::processPacket, m_state=" 
        << uint32(m_state) << endl;
   
   if ( (packCont == NULL) ||
        (packCont->getPacket() == NULL)){
      mc2dbg2 << "ItemInfoRequest::processPacket(NULL)" << endl;
      return;
   }
   
   switch (m_state) {
      case SENDING_COORDINATE_REQUEST  :
      case AWAITING_COORDINATE_REPLY :
         if (packCont->getPacket()->getSubType() == 
             Packet::PACKETTYPE_COORDINATEREPLY) {
            mc2dbg2 << "[IIR]:   Got CoordinateReplyPacket" << endl;
            m_state = handleCoordinateReply(packCont);
            //delete packCont;
         } else {
            mc2log << error << "[IIR]:   Got strange packet != Coord: " 
                 << packCont->getPacket()->getSubTypeAsString() << endl;
         }
      break;

      case SENDING_ITEMINFO  :
      case AWAITING_ITEMINFO :
      {
         Packet* curPack = packCont->getPacket();
         if ( curPack->getSubType() == Packet::PACKETTYPE_ITEMINFO_REPLY) {
            mc2dbg2 << "[IIR]:   Got ItemInfoReplyPacket" << endl;
            m_state = handleItemInfoReply(
                           static_cast<ItemInfoReplyPacket*>
                                      (packCont->getPacket()));
         } else if ( curPack->getSubType() ==
                     Packet::PACKETTYPE_EXTERNALSEARCH_REPLY ) {
            m_state = handleExternalSearchReply( curPack );
         } else {
            cerr << "   Got strange packet != ItemInfo: " 
                 << packCont->getPacket()->getSubTypeAsString() << endl;
         }
         // Strings can point into the packet so keep it.
         m_savedPackets.push_back( packCont );
      }
      break;

      default: 
         mc2log << error
                << "[IIR]: processPacket(), unhandled state ("
                << uint32(m_state) 
                << endl;
      break;
   }

   mc2dbg2 << "[IIR]:   Leaving in state = " << uint32(m_state) << endl;
}

ItemInfoRequest::state_t
ItemInfoRequest::handleCoordinateReply(PacketContainer* pc)
{
   m_coordinateObject.processPacket(pc);
   
   if (m_coordinateObject.getDone()) {
      CoordinateReplyPacket* p = m_coordinateObject.getCoordinateReply();
      m_packetsReadyToSend.add(
               createItemInfoRequestPacket(p->getMapID(), 
                                           p->getItemID(), 
                                           m_language,
                                           m_coordinateData.itemType,
                                           m_coordinateData.lat,
                                           m_coordinateData.lon,
                                           m_coordinateData.poiName ) );
      return SENDING_ITEMINFO;
   } else {
      PacketContainer* packCont = m_coordinateObject.getNextPacket();
      while (packCont != NULL) {
         m_packetsReadyToSend.add(packCont);
         packCont = m_coordinateObject.getNextPacket();
      }
      return SENDING_COORDINATE_REQUEST;
   }

   return ERROR;
}

ItemInfoRequest::state_t
ItemInfoRequest::handleItemInfoReply(ItemInfoReplyPacket* p)
{
   // Now get the matches
   p->getAsMatches( m_searchReplyData->getMatchVector() );
   
   return DONE;
}

ItemInfoRequest::state_t
ItemInfoRequest::handleExternalSearchReply( const Packet* p ) 
{
   const ExternalSearchReplyPacket* reply =
      static_cast<const ExternalSearchReplyPacket*>(p);
   uint32 topHits = 0;
   reply->get( *m_searchReplyData, topHits );

   mc2dbg << "[IIR]: Size of vector is "
          << m_searchReplyData->getMatchVector().size() << endl;

   StringTable::stringCode status = StringTable::stringCode( 
      reply->getStatus() );
   if ( m_searchReplyData->getMatchVector().empty() ) {
      // Nothing in the packet. Might be external provider. Put back the
      // original match if there is one.      
      if ( m_origMatch ) {
         mc2dbg << "[IIR]: No info returned from module - "
                << " putting back original match " << endl;
         m_searchReplyData->getMatchVector().push_back(
            static_cast<VanillaMatch*>(m_origMatch->clone() ) );
         // Set status to OK, so we can use this match we just copied
         status = StringTable::OK;
      }
   }     
   
   if ( status == StringTable::OK ) {
      return DONE;
   } else {
      return ERROR;
   }
}


PacketContainer* 
ItemInfoRequest::getAnswer() 
{
   return NULL;
}


bool 
ItemInfoRequest::replyDataOk() const {
   return (m_state == DONE);
}


bool 
ItemInfoRequest::requestDone()
{
   return (m_state == DONE || m_state == ERROR);
}


void 
ItemInfoRequest::setItem(uint32 mapID, uint32 itemID,
                         LangTypes::language_t lang,
                         ItemTypes::itemType itemType )
{
   m_packetsReadyToSend.add(createItemInfoRequestPacket(mapID,
                                                        itemID,
                                                        lang,
                                                        itemType));
   m_language = lang;
   m_state = SENDING_ITEMINFO;
}

PacketContainer*
ItemInfoRequest::createItemInfoRequestPacket(uint32 mapID, uint32 itemID, 
                                             LangTypes::language_t lang,
                                             ItemTypes::itemType itemType,
                                             int32 lat,
                                             int32 lon,
                                             const char* poiName )
{
   ItemInfoRequestPacket* req = new ItemInfoRequestPacket(
      IDPair_t(mapID, itemID), getUser(), lat, lon, poiName );

   req->setRequestID(getID());
   req->setPacketID(getNextPacketID());

   if (lat != MAX_INT32) {
      req->setPOICoordinate(lat, lon);
   }

   // Set itemType, language and outdata-type
   mc2dbg2 << "ItemInfoRequest::createItemInfoRequestPacket, itemType="
          << StringTable::getString(
                            ItemTypes::getItemTypeSC(itemType), 
                            StringTable::ENGLISH) << endl;
   req->setItemType(itemType);
   req->setPreferredLanguage(lang);
   req->setOutdataFormat(ItemInfoRequestPacket::text);
   
   return (new PacketContainer(req, 0, 0, MODULE_TYPE_MAP));
}


void 
ItemInfoRequest::setItem(int32 lat, int32 lon,
                         ItemTypes::itemType itemType,
                         LangTypes::language_t lang,
                         const char* poiName )
{
   m_language = lang;
   m_coordinateData.itemType = itemType;
   m_coordinateData.lat = lat;
   m_coordinateData.lon = lon;
   m_coordinateData.poiName = poiName;
   
   byte coordItemType = byte(itemType);
   if (itemType == ItemTypes::pointOfInterestItem) {
      // Can not find POI:s by using the CoordinateRequestPacket
      coordItemType = byte(ItemTypes::streetSegmentItem);
   }

   m_coordinateObject.addCoordinate(lat, lon,
                                    0, // Indata type
                                    1,
                                    &coordItemType);
   
   PacketContainer* pc = m_coordinateObject.getNextPacket();
   while (pc != NULL) {
      m_packetsReadyToSend.add(pc);
      pc = m_coordinateObject.getNextPacket();
   }

   m_state = SENDING_COORDINATE_REQUEST;
}

bool
ItemInfoRequest::setItem( const SearchMatch& match,
                          LangTypes::language_t lang )
{
   delete m_origMatch;
   m_origMatch = static_cast<const VanillaMatch*>(&match)->clone();
   if ( match.getExtSource() == ExternalSearchConsts::not_external ) { // MC2
      if ( match.getID().isValid() ) {
         setItem( match.getMapID(), match.getItemID(),
                  lang, match.getItemType() );
         mc2log << info << "[IIR]: Item " << match.getID() << " type " 
                << int(match.getItemType()) << endl;
         return true;
      } else if ( match.getCoords().isValid() ) {
         // Using coord and name (and type)
         setItem( match.getCoords().lat, match.getCoords().lon,
                  match.getItemType(), lang, match.getName() );
         mc2log << info << "[IIR]: Item " << match.getCoords() << " type " 
                << int(match.getItemType()) << " Name " 
                << match.getName() << endl;
         return true;
      } else {
         mc2log << warn 
                << "[IIR]: Not id nor coords valid for match m_state = ERRROR"
                << endl;
         m_state = ERROR;
         return false;
      }
   } else {
      // External source -> send to extservicemodule
      // FIXME: Add language.
      m_state = SENDING_ITEMINFO;
      ExternalItemInfoRequestPacket* req =
         new ExternalItemInfoRequestPacket( match,
                                            lang,
                                            getUser() );
      updateIDs( req );
      m_packetsReadyToSend.add(
         new PacketContainer( req, 0, 0,
                              MODULE_TYPE_EXTSERVICE ) );
      return true;
   }
}


uint32 
ItemInfoRequest::getReplyNbrFields(uint32 i) const
{
   return getMatches()[i]->getItemInfos().size();
}

uint32 
ItemInfoRequest::getNbrReplyItems() const
{
   return getMatches().size();
}

uint32 
ItemInfoRequest::getReplySubType(uint32 i) const
{
   return getMatches()[i]->getItemSubtype();
}

int32 
ItemInfoRequest::getReplyLatitude(uint32 i) const
{
   return getMatches()[i]->getCoords().lat;
}
        
int32 
ItemInfoRequest::getReplyLongitude(uint32 i) const
{
   return getMatches()[i]->getCoords().lon;
}


const char* 
ItemInfoRequest::getReplyType(uint32 i) const
{
   return getMatches()[i]->getSynonymName();
}

const char* 
ItemInfoRequest::getReplyName(uint32 i) const
{
   return getMatches()[i]->getName();
}

const char* 
ItemInfoRequest::getReplyItemFieldname(uint32 itemNbr, 
                                       uint32 fieldNbr) const
{
   return getMatches()[itemNbr]->getItemInfos()[fieldNbr].getKey();
}

const char* 
ItemInfoRequest::getReplyItemValue(uint32 itemNbr, uint32 fieldNbr) const
{
   return getMatches()[itemNbr]->getItemInfos()[fieldNbr].getVal();
}


ItemInfoEnums::InfoType 
ItemInfoRequest::getReplyItemType( uint32 itemNbr, 
                                   uint32 fieldNbr ) const
{
   return getMatches()[itemNbr]->getItemInfos()[fieldNbr].getInfoType();
}
