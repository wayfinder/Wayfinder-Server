/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ItemInfoPacket.h"
#include "StringUtility.h"
#include "UserRightsMapInfo.h"

#include "IDPairVector.h"
#include "SearchMatch.h"
#include "ItemInfoEntry.h"

#define ITEMINFO_REPLY_PRIO   0
#define ITEMINFO_REQUEST_PRIO 0


#define ITEMINFO_REQ_ITEMID_POS     (REQUEST_HEADER_SIZE)
#define ITEMINFO_REQ_POILAT_POS     (REQUEST_HEADER_SIZE+4)
#define ITEMINFO_REQ_POILON_POS     (REQUEST_HEADER_SIZE+8)
#define ITEMINFO_REQ_LANG_POS       (REQUEST_HEADER_SIZE+12)
#define ITEMINFO_REQ_ITEMTYPE_POS   (REQUEST_HEADER_SIZE+16)
#define ITEMINFO_REQ_OUTFORMAT_POS  (REQUEST_HEADER_SIZE+20)
#define ITEMINFO_REQ_PACKET_LENGTH  (REQUEST_HEADER_SIZE+24)

// The flags positions in ItemInfoData
#define MORE_INFO_BIT 0


uint32
ItemInfoRequestPacket::calcPackSize( const IDPair_t& id,
                                     const UserUser* user,
                                     int32 poiLat, int32 poiLon,
                                     const char* poiName )
{   
   UserRightsMapInfo rights( id.getMapID(), user );
   return ITEMINFO_REQ_PACKET_LENGTH + strlen( poiName ) + 1 +
      rights.getSizeInPacket();
}

ItemInfoRequestPacket::ItemInfoRequestPacket(const IDPair_t& id,
                                             const UserUser* user,
                                             int32 poiLat, int32 poiLon,
                                             const char* poiName )
   : RequestPacket( calcPackSize(id, user, poiLat, poiLon, poiName),
                    ITEMINFO_REQUEST_PRIO,
                    Packet::PACKETTYPE_ITEMINFO_REQUEST,
                    0, // packetID
                    0, // requestID
                    id.getMapID() )
{
   // Add itemID and (lat,lon) to the packet
   writeLong(ITEMINFO_REQ_ITEMID_POS, id.getItemID() );
   setPOICoordinate(poiLat, poiLon);
   setOutdataFormat(ItemInfoRequestPacket::text);
   int pos = ITEMINFO_REQ_PACKET_LENGTH;
   incWriteString( pos, poiName );
   UserRightsMapInfo rights( id.getMapID(), user );
   rights.save( this, pos );
   setLength( pos );
}

void 
ItemInfoRequestPacket::setPreferredLanguage(LangTypes::language_t lang)
{
   writeLong(ITEMINFO_REQ_LANG_POS, uint16(lang));
}

LangTypes::language_t
ItemInfoRequestPacket::getPreferredLanguage() const
{
   return LangTypes::language_t(readLong(ITEMINFO_REQ_LANG_POS));
}

void 
ItemInfoRequestPacket::setOutdataFormat(ItemInfoRequestPacket::outformat_t t)
{
   writeLong(ITEMINFO_REQ_OUTFORMAT_POS, uint32(t));
}

ItemInfoRequestPacket::outformat_t
ItemInfoRequestPacket::getOutdataFormat() const
{
   return ItemInfoRequestPacket::outformat_t(
               readLong(ITEMINFO_REQ_OUTFORMAT_POS));
}
   


void 
ItemInfoRequestPacket::setItemType(ItemTypes::itemType t)
{
   writeLong(ITEMINFO_REQ_ITEMTYPE_POS, uint16(t));

}

ItemTypes::itemType 
ItemInfoRequestPacket::getItemType() const
{
   return ItemTypes::itemType(readLong(ITEMINFO_REQ_ITEMTYPE_POS));

}

uint32 
ItemInfoRequestPacket::getItemID() const
{
   return ( readLong(ITEMINFO_REQ_ITEMID_POS));
}

void
ItemInfoRequestPacket::setPOICoordinate(int32 lat, int32 lon)
{
   writeLong(ITEMINFO_REQ_POILAT_POS, lat);
   writeLong(ITEMINFO_REQ_POILON_POS, lon);

}

int32 
ItemInfoRequestPacket::getPOILat() const
{
   return ( readLong(ITEMINFO_REQ_POILAT_POS));
}

int32 
ItemInfoRequestPacket::getPOILon() const
{
   return ( readLong(ITEMINFO_REQ_POILON_POS));
}


const char* 
ItemInfoRequestPacket::getPOINameAndRights( UserRightsMapInfo& rights ) const {
   int pos = ITEMINFO_REQ_PACKET_LENGTH;
   const char* poiName = incReadString( pos );
   if ( pos < (int)getLength() ) {
      rights.load( this, pos );
   } else {
      mc2dbg << "[IIRP]: Sender didn't include UserRightsMapInfo - "
             << " all is allowed" << endl;
      UserRightsMapInfo all( getMapID(), ~MapRights() );
      rights.swap( all );
   }
   return poiName;
}

// ======================================================================

uint32 sizeForGetItemInfoRequestPacket( 
   uint32 mapID, const UserUser* user,
   const vector< VanillaMatch* >& items ) {
   return REQUEST_HEADER_SIZE + 
      4 + // language 
      4 + // info type filter
      4 + // items size
      4 + // map info size
      UserRightsMapInfo( mapID, user ).getSizeInPacket() +
      4*items.size() + 3;
}

GetItemInfoRequestPacket::GetItemInfoRequestPacket( 
   uint32 mapID,
   LangTypes::language_t language,
   ItemInfoEnums::InfoTypeFilter infoTypeFilter,
   const UserUser* user,
   const vector< VanillaMatch* >& items )
      :  RequestPacket( sizeForGetItemInfoRequestPacket( mapID, user, items ),
                        ITEMINFO_REQUEST_PRIO,
                        Packet::PACKETTYPE_GET_ITEM_INFO_REQUEST,
                        0, // packetID
                        0, // requestID
                        mapID )
{
   int pos = REQUEST_HEADER_SIZE;
   UserRightsMapInfo mapInfo( mapID, user );
   
   incWriteLong( pos, language );
   incWriteLong( pos, infoTypeFilter );
   incWriteLong( pos, items.size() );
   incWriteLong( pos, mapInfo.getSizeInPacket() );
   for ( size_t i = 0, end = getNbrItems() ; i < end ; ++i ) {
      incWriteLong( pos, items[ i ]->getItemID() );
   }
   mapInfo.save( this, pos );
   setLength( pos );
}

LangTypes::language_t 
GetItemInfoRequestPacket::getLanguage() const {
   return LangTypes::language_t( readLong( language_POS ) );
}

ItemInfoEnums::InfoTypeFilter
GetItemInfoRequestPacket::getItemInfoFilter() const {
   return ItemInfoEnums::InfoTypeFilter( readLong( infotypefilter_POS ) );
}

void
GetItemInfoRequestPacket::getUserRightsMapInfo( 
   UserRightsMapInfo& rights ) const {
   int pos = items_start_POS + getNbrItems()*4;

   pos = rights.load( this, pos );
}

void
GetItemInfoRequestPacket::getItemIDs( itemIDVector& itemIDs ) const {
   int pos = items_start_POS;

   for ( size_t i = 0, end = getNbrItems() ; i < end ; ++i ) {
      itemIDs.push_back( incReadLong( pos ) );
   }
}

uint32
GetItemInfoRequestPacket::getNbrItems() const {
   return readLong( nbr_items_POS );
}


// ======================================================================


ItemInfoData::ItemInfoData() {
}

ItemInfoData::ItemInfoData( const MC2String& type,
                            const MC2String& itemName,
                            ItemTypes::itemType itemType,
                            uint32 subType,
                            const MC2Coordinate& coord,
                            const Categories& categories,
                            const ItemInfoEntryCont& fields )
      : m_type( type ), m_itemName( itemName ), m_itemType( itemType ),
        m_subType( subType ), m_coord( coord ), m_categories( categories ),
        m_fields( fields ), m_moreInfo( false )
{
}

uint32
ItemInfoData::getSizeInPacket() const {
   uint32 size = m_type.size() + 1 + 
      m_itemName.size() + 1 +
      sizeof( m_itemType ) +
      sizeof( m_subType ) +
      sizeof( m_coord ) +
      sizeof( size_t ) +
      sizeof( size_t ) +
      sizeof( uint8 ) + // flags
      m_categories.size()*2;
   for ( size_t i = 0, end = m_fields.size() ; i < end ; ++i ) {
      size += m_fields[ i ].getSizeInPacket() + 3/*Might be padding here*/;
   }
   return size + 6/*Possible padding*/;
}

int
ItemInfoData::save( Packet* p, int& pos ) const {
   int orig_pos = pos;
   p->incWriteLong( pos, m_itemType );
   p->incWriteLong( pos, m_subType );
   p->incWriteLong( pos, m_coord.lat );
   p->incWriteLong( pos, m_coord.lon );
   p->incWriteLong( pos, m_categories.size() );
   p->incWriteLong( pos, m_fields.size() );
   p->writeBit( pos, MORE_INFO_BIT, m_moreInfo );
   pos++;

   p->incWriteString( pos, m_type );
   p->incWriteString( pos, m_itemName );

   // Categories
   for ( Categories::const_iterator it = m_categories.begin(), endIt =
            m_categories.end() ; it != endIt ; ++it ) {
      p->incWriteLong( pos, *it );
   }
   // Item Info entries
   for ( ItemInfoEntryCont::const_iterator it = m_fields.begin(), endIt =
            m_fields.end() ; it != endIt ; ++it ) {
      it->save( p, pos );
   }

   return pos - orig_pos;
}

int
ItemInfoData::load( const Packet* p, int& pos ) {
   int orig_pos = pos;

   p->incReadLong( pos, m_itemType );
   p->incReadLong( pos, m_subType );
   p->incReadLong( pos, m_coord.lat );
   p->incReadLong( pos, m_coord.lon );
   uint32 nbrCategories = 0;
   p->incReadLong( pos, nbrCategories );
   uint32 nbrFields = 0;
   p->incReadLong( pos, nbrFields );
   m_moreInfo = p->readBit( pos, MORE_INFO_BIT );
   pos++;

   p->incReadString( pos, m_type );
   p->incReadString( pos, m_itemName );

   // Categories
   m_categories.resize( nbrCategories );
   for ( size_t i = 0, end = nbrCategories ; i != end ; ++i ) {
      m_categories[ i ] = p->incReadLong( pos );
   }

   // Item Info entries
   m_fields.resize( nbrFields );
   for ( size_t i = 0, end = nbrFields ; i != end ; ++i ) {
      m_fields[ i ].load( p, pos );
   }

   return pos - orig_pos;
}

void
ItemInfoData::addInfoEntry( const ItemInfoEntry& entry ) {
   m_fields.push_back( entry );
}

void
ItemInfoData::addInfoEntry( const MC2String& key,
                            const MC2String& val,
                            ItemInfoEnums::InfoType infoType ) {
   addInfoEntry( ItemInfoEntry( key, val, infoType ) );
}


const MC2String&
ItemInfoData::getType() const {
   return m_type;
}

const MC2String&
ItemInfoData::getItemName() const {
   return m_itemName;
}

ItemTypes::itemType
ItemInfoData::getItemType() const {
   return m_itemType;
}

uint32
ItemInfoData::getSubType() const {
   return m_subType;
}

const MC2Coordinate&
ItemInfoData::getCoord() const {
   return m_coord;
}

const ItemInfoData::Categories&
ItemInfoData::getCategories() const {
   return m_categories;
}

const ItemInfoData::ItemInfoEntryCont&
ItemInfoData::getFields() const {
   return m_fields;
}

void
ItemInfoData::setSubType( uint32 type ) {
   m_subType = type;
}

void
ItemInfoData::setCategories( const Categories& categories ) {
   m_categories = categories;
}

void
ItemInfoData::setFields( const ItemInfoEntryCont& fields ) {
   m_fields = fields;
}

void
ItemInfoData::setItemName( const MC2String& name ) {
   m_itemName = name;
}

void
ItemInfoData::setMoreInfoAvailable( bool moreInfo ) {
   m_moreInfo = moreInfo;
}

bool
ItemInfoData::getMoreInfoAvailable() const {
   return m_moreInfo;
}

// ======================================================================


#define ITEMINFO_REP_NBR_ITEMS            (REPLY_HEADER_SIZE)
#define ITEMINFO_REP_FIRST_ITEMSTART_POS  (REPLY_HEADER_SIZE + 4)

ItemInfoReplyPacket::ItemInfoReplyPacket()
      : ReplyPacket( MAX_PACKET_SIZE, 
                     Packet::PACKETTYPE_ITEMINFO_REQUEST )
{
   writeLong( ITEMINFO_REP_NBR_ITEMS, 0 );
   setLength( ITEMINFO_REP_FIRST_ITEMSTART_POS );
}

ItemInfoReplyPacket::ItemInfoReplyPacket( const ItemInfoRequestPacket* req )
      : ReplyPacket( MAX_PACKET_SIZE, 
                     Packet::PACKETTYPE_ITEMINFO_REPLY,
                     req, 
                     StringTable::OK )
{
   writeLong( ITEMINFO_REP_NBR_ITEMS, 0 );
   setLength( ITEMINFO_REP_FIRST_ITEMSTART_POS );
}

ItemInfoReplyPacket::ItemInfoReplyPacket( const GetItemInfoRequestPacket* req )
      : ReplyPacket( MAX_PACKET_SIZE, 
                     Packet::PACKETTYPE_ITEMINFO_REPLY,
                     req, 
                     StringTable::OK )
{
   writeLong( ITEMINFO_REP_NBR_ITEMS, 0 );
   setLength( ITEMINFO_REP_FIRST_ITEMSTART_POS );
}

void
ItemInfoReplyPacket::setInfo( const ItemInfoDataCont& info ) {
   int pos = ITEMINFO_REP_NBR_ITEMS;

   // Nbr items
   incWriteLong( pos, info.size() );
   for ( ItemInfoDataCont::const_iterator it = info.begin(), endIt = 
            info.end() ; it != endIt ; ++it ) {
      if ( pos + it->getSizeInPacket() >= getBufSize() ) {
         setLength( pos ); // So the existing data is copied
         resize( getBufSize() * 2 );
      }
      it->save( this, pos );
   }
   setLength( pos );
}

void
ItemInfoReplyPacket::getInfo( ItemInfoDataCont& info ) const {
   int pos = ITEMINFO_REP_NBR_ITEMS;
   const uint32 nbrItems = incReadLong( pos );
   info.resize( nbrItems );
   for ( uint32 i = 0 ; i < nbrItems ; ++i ) {
      info[ i ].load( this, pos );
   }
}

void transferInfo( const ItemInfoData& data, VanillaMatch* curMatch ) {
   if ( curMatch->getType() == SEARCH_COMPANIES ) {
      VanillaCompanyMatch& poi =
         static_cast< VanillaCompanyMatch& >( *curMatch );
      poi.setCategories( data.getCategories() );
   }
   
   curMatch->setSynonymName( data.getType().c_str() );
   
   // This cast is slightly ugly but as info is local it works
   // All item infos are given to the match.
   curMatch->swapItemInfos( const_cast<ItemInfoData::ItemInfoEntryCont&> ( 
                               data.getFields() ) );

   curMatch->setAdditionalInfoExists( data.getMoreInfoAvailable() );
}

bool
ItemInfoReplyPacket::getInfo( vector<VanillaMatch*>& matches ) const {
   ItemInfoDataCont info;
   getInfo( info );

   if ( info.size() == matches.size() ) {
      size_t i = 0;
      for ( ItemInfoDataCont::const_iterator it = info.begin(), endIt = 
            info.end() ; it != endIt ; ++it, ++i ) {
         VanillaMatch* match = matches[ i ];
         if ( match->getName()[ 0 ] == '\0' ) {
            match->setName( it->getItemName().c_str() );
         }
         mc2dbg4 << "Name " << it->getItemName() << " matchName " 
                << match->getName();
         match->setCoords( it->getCoord() );
         mc2dbg4 << " coord " << it->getCoord();
         match->setItemType( it->getItemType() );
         match->setItemSubType( it->getSubType() );
         transferInfo( *it, match );
         mc2dbg4 << endl;
      }
      return true;
   } else {
      mc2dbg << "[IIRP]:getInfo nbr matches differ " << info.size() << " != "
             << matches.size() << endl;
      return false;
   }
}

void
ItemInfoReplyPacket::getAsMatches( vector<VanillaMatch*>& matches ) const
{
   ItemInfoDataCont info;
   getInfo( info );
   for ( ItemInfoDataCont::const_iterator it = info.begin(), endIt = 
            info.end() ; it != endIt ; ++it ) {
      VanillaMatch* curMatch = static_cast<VanillaMatch*> ( 
         SearchMatch::createMatch( it->getItemName().c_str(),
                                   "",
                                   IDPair_t(),
                                   it->getCoord(),
                                   it->getItemType(),
                                   it->getSubType() ) );
      matches.push_back ( curMatch );
      if ( curMatch == NULL ) {
         matches.pop_back();
         mc2log << warn << "[ItemInfoRequest]: Could not create match"
                << endl;
         continue;
      }

      transferInfo( *it, curMatch );
   }
}
