/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ServerProximityPackets.h"

#include "UserRightsMapInfo.h"
//////////////////////////////////////////////////////////////
// CoveredIDsRequestPacket
//////////////////////////////////////////////////////////////
CoveredIDsRequestPacket::
CoveredIDsRequestPacket( const UserUser* user,
                         uint16 packetID,
                         uint16 reqID,
                         uint32 mapID,
                         const set<ItemTypes::itemType>& itemTypes)

   // We guess a size. It might be resized later, but I doubt it.
      : RequestPacket( 1024 + REQUEST_HEADER_SIZE + 36 + itemTypes.size() * 4,
                       SERVER_PROXIMITY_REQUEST_PRIO,
                       PACKETTYPE_COVEREDIDSREQUESTPACKET,
                       packetID,
                       reqID,
                       mapID )
{
   writeShort(REQUEST_HEADER_SIZE + 18, COVEREDIDSREQUEST_CONTENT_INVALID);

   // Set the default values of the inner radius and the angles
   setInnerRadius(0);
   setStartAngle(0);
   setStopAngle(360);
   int pos = REQUEST_HEADER_SIZE + 32; // Itemtypes
   incWriteLong(pos, itemTypes.size());
   for( set<ItemTypes::itemType>::const_iterator it = itemTypes.begin();
        it != itemTypes.end();
        ++it ) {
      incWriteLong(pos, *it);
   }
   setLength(pos);
   
   // Check size of packet and then add User Rights
   UserRightsMapInfo rights( mapID, user );
   updateSize( rights.getSizeInPacket(), rights.getSizeInPacket() );
   rights.save( this, pos );
   setLength(pos);
}

int
CoveredIDsRequestPacket::
getItemTypesAndRights(set<ItemTypes::itemType>& types,
                      UserRightsMapInfo& rights) const
{
   int pos = REQUEST_HEADER_SIZE + 32;
   int nbr = incReadLong(pos);
   for ( int i=0; i < nbr; ++i ) {
      types.insert(ItemTypes::itemType(incReadLong(pos)));
   }
   if ( pos >= (int)getLength() ) {
      // All allowed
      UserRightsMapInfo tmp( getMapID(), NULL );
      rights.swap( tmp );
      return types.size();
   }
   rights.load( this, pos );
   return types.size();
}

void 
CoveredIDsRequestPacket::setLat(int32 lat) {
   writeLong(REQUEST_HEADER_SIZE, lat);
}


int32
CoveredIDsRequestPacket::getLat() const {
   return readLong(REQUEST_HEADER_SIZE);  
}


void
CoveredIDsRequestPacket::setLon(int32 lon) {
   writeLong(REQUEST_HEADER_SIZE + 4, lon);
}


int32
CoveredIDsRequestPacket::getLon() const {
   return readLong(REQUEST_HEADER_SIZE + 4);
}


bool
CoveredIDsRequestPacket::isItemID() const {
   return ( readShort(REQUEST_HEADER_SIZE + 18) == 
            COVEREDIDSREQUEST_CONTENT_ITEMID );
}


void
CoveredIDsRequestPacket::setItemIDAsValid() {
   writeShort(REQUEST_HEADER_SIZE + 18, COVEREDIDSREQUEST_CONTENT_ITEMID); 
}


bool
CoveredIDsRequestPacket::isLatLon() const {
 return ( readShort(REQUEST_HEADER_SIZE + 18) == 
          COVEREDIDSREQUEST_CONTENT_LAT_LON );
}


void
CoveredIDsRequestPacket::setLatLonAsValid() {
   writeShort(REQUEST_HEADER_SIZE + 18, COVEREDIDSREQUEST_CONTENT_LAT_LON);
}


void
CoveredIDsRequestPacket::setItemID(uint32 itemID) {
   writeLong(REQUEST_HEADER_SIZE + 12, itemID);
}


uint32
CoveredIDsRequestPacket::getItemID() const {
   return readLong(REQUEST_HEADER_SIZE + 12);
}


void
CoveredIDsRequestPacket::setOffset(uint16 offset) {
   writeShort(REQUEST_HEADER_SIZE + 16, offset);
}


uint16
CoveredIDsRequestPacket::getOffset() const {
   return readShort(REQUEST_HEADER_SIZE + 16);
}

void
CoveredIDsRequestPacket::setOuterRadius(uint32 dist) {
   DEBUG8(cout << "setOuterRadius = " << dist << endl);
   writeLong(REQUEST_HEADER_SIZE + 8, dist);
}


uint32
CoveredIDsRequestPacket::getOuterRadius() const {
   DEBUG8(cout << "getOuterRadius = " 
               << readLong(REQUEST_HEADER_SIZE + 8) << endl);
   return readLong(REQUEST_HEADER_SIZE + 8); 
}


void
CoveredIDsRequestPacket::setInnerRadius(uint32 dist) {
   DEBUG8(cout << "setInnerRadius = " << dist << endl);
   writeLong(REQUEST_HEADER_SIZE + 20, dist);
}

uint32
CoveredIDsRequestPacket::getInnerRadius() const {
   DEBUG8(cout << "getInnerRadius = " 
               << readLong(REQUEST_HEADER_SIZE + 20) << endl);
   return readLong(REQUEST_HEADER_SIZE + 20); 
}


void
CoveredIDsRequestPacket::setStartAngle(uint32 dist) {
   writeLong(REQUEST_HEADER_SIZE + 24, dist);
}

uint32
CoveredIDsRequestPacket::getStartAngle() const {
   return readLong(REQUEST_HEADER_SIZE + 24); 
}


void
CoveredIDsRequestPacket::setStopAngle(uint32 dist) {
   writeLong(REQUEST_HEADER_SIZE + 28, dist);
}

uint32
CoveredIDsRequestPacket::getStopAngle() const {
   return readLong(REQUEST_HEADER_SIZE + 28); 
}

//////////////////////////////////////////////////////////////
// CoveredIDsReplyPacket
//////////////////////////////////////////////////////////////

const uint32 
CoveredIDsReplyPacket::m_mapIDPos = REPLY_HEADER_SIZE;

const uint32 
CoveredIDsReplyPacket::m_nbrIDsPos = REPLY_HEADER_SIZE + 4;

const uint32 
CoveredIDsReplyPacket::m_firstIDPos = REPLY_HEADER_SIZE + 8;


CoveredIDsReplyPacket::CoveredIDsReplyPacket(const CoveredIDsRequestPacket* 
                                             inPacket,
                                             int packetSize) 
      : ReplyPacket(packetSize,
                    PACKETTYPE_COVEREDIDSREPLYPACKET,
                    inPacket,
                    StringTable::OK ) // Status
{
   setMapID(inPacket->getMapID());
   setNumberIDs(0);
   setLength(m_firstIDPos);
}

int
CoveredIDsReplyPacket::calcPacketSize(int nbrNodes)
{       
   int32 size =
      // 8 bytes/node (id+type)  // nbrnodes // map id // >
      nbrNodes * 8  +    4         + 4        + 1
      + REPLY_HEADER_SIZE;
   mc2dbg4 << here << " size = " << size << endl;
   return size;

}

void
CoveredIDsReplyPacket::setMapID(uint32 mapID) {
   writeLong(m_mapIDPos, mapID);
}

uint32
CoveredIDsReplyPacket::getMapID() const {
   return readLong(m_mapIDPos);
}

bool
CoveredIDsReplyPacket::isMapIDs() const {
   return (getMapID() == MAX_UINT32);
}

uint32
CoveredIDsReplyPacket::getNumberIDs() const {
   return (readLong(m_nbrIDsPos));
}

void
CoveredIDsReplyPacket::setNumberIDs(uint32 newNbrIDs) {
   writeLong(m_nbrIDsPos, newNbrIDs);
}

bool
CoveredIDsReplyPacket::addID(uint32 ID, ItemTypes::itemType type) {
   if ( (getLength() + 2*sizeof(uint32)) < getBufSize() ) {
      uint32 nbrIDs = getNumberIDs();
      int pos = m_firstIDPos + nbrIDs*2*sizeof(uint32);
      incWriteLong(pos, ID);
      incWriteLong(pos, type); // Put type here.
      setNumberIDs(nbrIDs+1);
      setLength(pos);
      return true;
   } else {
      mc2log << error << "CoveredIDsReplyPacket::addID() - ID won't fit!!"
             << endl;
      return false;
   }
}

uint32
CoveredIDsReplyPacket::getID(uint32 index) const {
   return (readLong(m_firstIDPos + index*8));
}

ItemTypes::itemType
CoveredIDsReplyPacket::getType(uint32 idx) const {
   return ItemTypes::itemType(readLong( m_firstIDPos + idx*8 + 4));
}

//////////////////////////////////////////////////////////////
// ProximitySearchRequest
//////////////////////////////////////////////////////////////


ProximitySearchRequestPacket::ProximitySearchRequestPacket(uint16 packetID,
                                                           uint16 reqID ,
                                                           uint32 mapID )
      : OldSearchRequestPacket( MAX_PACKET_SIZE * 2,
                             SERVER_PROXIMITY_REQUEST_PRIO,
                             PACKETTYPE_PROXIMITYSEARCHREQUEST,
                             packetID,
                             reqID,
                             mapID )
{
   setLength(REQUEST_HEADER_SIZE + 52);  
}


void
ProximitySearchRequestPacket::encodeRequest(
   uint8 dbMask,
   proximitySearchItemsType 
   proximityItems,    
   uint16 maximumNumberHits,
   bool useString,
   const char* searchString,
   uint16 categoryType,
   SearchTypes::StringMatching matchType,
   SearchTypes::StringPart stringPart,
   SearchTypes::SearchSorting sortingType,
   uint8 nbrSortedHits,
   LangTypes::language_t requestedLanguage,
   uint32 regionsInMatches ) 
{
   int i = REQUEST_HEADER_SIZE;
   
   uint32 mapID = RequestPacket::getMapID();
   i = writeHeader( sortingType, 1, &mapID, "", 0, NULL, 0,
                    0, NULL, 0, NULL, NULL,
                    NULL, dbMask, 0, 
                    StringTable::SWEDEN_CC ); // TODO: Dummy value 

   AlignUtility::alignLong(i);
   mc2dbg2 << "encodeRequest, i == " << i << endl;
   incWriteLong(i, 0); // No items
   incWriteByte(i, useString);
   incWriteByte(i, nbrSortedHits);
   incWriteShort(i, categoryType);
   incWriteLong(i, matchType);
   incWriteLong(i, stringPart);
   incWriteLong(i, sortingType);
   incWriteLong(i, strlen(searchString) + 1);
   incWriteLong(i, proximityItems);
   incWriteLong( i, requestedLanguage );
   incWriteLong( i, regionsInMatches );
   incWriteShort(i, maximumNumberHits);
   incWriteString(i, searchString);
   setLength(i);
}


void
ProximitySearchRequestPacket::decodeRequest(
   uint8& dbMask,
   proximitySearchItemsType& proximityItems,
   uint32& nbrItems,
   uint16& maximumNumberHits,
   bool& useString,
   char*& searchString,
   uint16& categoryType,
   SearchTypes::StringMatching& matchType,
   SearchTypes::StringPart& stringPart,
   SearchTypes::SearchSorting& sortingType,
   uint8& nbrSortedHits,
   uint32& requestedLanguage,
   uint32& regionsInMatches )
{
   vector<MC2BoundingBox> bboxes;
   int i = REQUEST_HEADER_SIZE;
   
   char* tmp = NULL;
   char** tmpv = NULL;
   uint32* tmpu = NULL;
   uint32 u;
   StringTable::countryCode dummy;
   i = readHeader( sortingType, tmp, u, tmpv, u, u, tmpu, u, tmpu, tmpu, tmpv,
                   dbMask, u, dummy, bboxes ); // STORKA

   AlignUtility::alignLong(i);
   
   mc2dbg4 << "decodeRequest, i == " << i << endl; 
   nbrItems = incReadLong(i);
   useString = incReadByte(i);
   nbrSortedHits = incReadByte(i);
   categoryType = incReadShort(i);
   matchType = static_cast<SearchTypes::StringMatching>(incReadLong(i));
   stringPart = static_cast<SearchTypes::StringPart>(incReadLong(i));
   sortingType = static_cast<SearchTypes::SearchSorting>(incReadLong(i));
   u = incReadLong(i); // StringLength
   proximityItems = proximitySearchItemsType(incReadLong(i));
   requestedLanguage = incReadLong( i );
   regionsInMatches = incReadLong( i );
   maximumNumberHits = incReadShort(i);
   incReadString(i, searchString);
}

uint32
ProximitySearchRequestPacket::getNbrItems()
{
   // Find out the position of the nbrItems (first after the header
   // in the SearchRequestPacket).
   int nbrItemsPos = getHeaderEndPosition();

   return (readLong(nbrItemsPos));
}


bool 
ProximitySearchRequestPacket::addItem(uint32 itemID, int& position) {
   if ( (position+sizeof(uint32)) < getBufSize() ) {
      // Find out the position of the nbrItems (first after the header
      // in the SearchRequestPacket).
      int nbrItemsPos = getHeaderEndPosition();

      // Increas the number of items 
      int n = readLong(nbrItemsPos);
      n++;
      writeLong(nbrItemsPos, n);

      // Add the ID last at this packet
      incWriteLong( position, itemID );
      setLength(position);
      return true;
   } else {
      return false;
   }
}


int
ProximitySearchRequestPacket::getFirstItemPosition()
{
   int pos = getHeaderEndPosition();
   AlignUtility::alignLong( pos );
   pos += 4 +  //    nbr items                  +4
          1 +  //    useString                  +1
          1 +  //    nbrSortedHits              +1
          2 +  //    categoryType               +2
          4 +  //    matchType                  +4
          4 +  //    stringPart                 +4
          4 +  //    sortingType                +4
          4 +  //    length of searchString     +4
          4 +  //    proximityItems             +4
          4 +  //    requestedLanguage          +4
          4 +  //    regionsInMatches           +4
          2;   //    maxNbrHits                 +2
   // Also add the length of the search string
   char* foo;
   incReadString(pos, foo);
   pos++; // Skipp strings null-terminator
   return (pos);
}


uint32
ProximitySearchRequestPacket::getItem(int& position) {
   return incReadLong(position);
}


//////////////////////////////////////////////////////////////
// ProximitySearchReplyPacket
//////////////////////////////////////////////////////////////
//
const uint32 
ProximitySearchReplyPacket::m_mapIDPos = REPLY_HEADER_SIZE ;

const uint32 
ProximitySearchReplyPacket::m_nbrItemsPos = REPLY_HEADER_SIZE + 4;

const uint32 
ProximitySearchReplyPacket::m_moreItemsPos = REPLY_HEADER_SIZE + 8;

const uint32 
ProximitySearchReplyPacket::m_firstItemPos = REPLY_HEADER_SIZE + 12;


ProximitySearchReplyPacket::ProximitySearchReplyPacket( 
   ProximitySearchRequestPacket* inPacket )
      : SearchReplyPacket( PACKETTYPE_PROXIMITYSEARCHREPLY,
                           inPacket)
{
   writeLong(REPLY_HEADER_SIZE, 0);       // 
   writeLong(REPLY_HEADER_SIZE + 4, 0);   // map ID items
   setStatus(StringTable::OK);
   setLength(REPLY_HEADER_SIZE + 8);
}

uint32 
ProximitySearchReplyPacket::getMapID() 
{
   return (readLong(m_mapIDPos));
}

void 
ProximitySearchReplyPacket::setMapID(uint32 newID)
{
   writeLong(m_mapIDPos, newID);
}


uint32
ProximitySearchReplyPacket::getNumberItems() {
   return readLong(m_nbrItemsPos);
}

void
ProximitySearchReplyPacket::setMoreHits(uint8 moreHits) {
   writeByte(m_moreItemsPos, moreHits);
}

uint8
ProximitySearchReplyPacket::getMoreHits() {
   return readByte(m_moreItemsPos);
}

bool
ProximitySearchReplyPacket::addItem(uint32 itemID, int& position) {
   if ( (position+sizeof(uint32)) < MAX_PACKET_SIZE ) {
      incWriteLong(position, itemID);
      uint32 nbr = getNumberItems();
      nbr++;
      writeLong(m_nbrItemsPos, nbr);
      setLength(position);
      return true;
   } else {
      return false;
   }
}

int 
ProximitySearchReplyPacket::getFirstItemPosition() {
   return (m_firstItemPos);
}

uint32
ProximitySearchReplyPacket::getItem(int& position) {
   return (incReadLong(position));
}


//////////////////////////////////////////////////////////////
// TrafficPointRequestPacket
//////////////////////////////////////////////////////////////

#define TRAFFIC_LATPOS     0
#define TRAFFIC_LONPOS     4
#define TRAFFIC_RADPOS     8
#define TRAFFIC_ANGLEPOS  12
#define TRAFFIC_DIRPOS    14
#define TRAFFIC_NBRPOS    15
#define TRAFFIC_RCPOS     16
#define TRAFFIC_STRINGPOS 17


TrafficPointRequestPacket::
TrafficPointRequestPacket( const UserUser* user,
                           uint16 packetID,
                           uint16 reqID,
                           uint32 mapID)

   // We guess a size. It might be resized later, but I doubt it.
      : RequestPacket( 1024 + REQUEST_HEADER_SIZE * 2,
                       SERVER_PROXIMITY_REQUEST_PRIO,
                       PACKETTYPE_TRAFFICPOINTREQUESTPACKET,
                       packetID,
                       reqID,
                       mapID )
{
   // Check size of packet and then add User Rights
   int pos = REQUEST_HEADER_SIZE+4;
   UserRightsMapInfo rights( mapID, user );
   updateSize( rights.getSizeInPacket(), rights.getSizeInPacket() );
   rights.save( this, pos );
   writeLong(REQUEST_HEADER_SIZE, pos);
   setLength(pos);
}

bool 
TrafficPointRequestPacket::
getUserRights(UserRightsMapInfo& rights) const
{
   
   int pos = REQUEST_HEADER_SIZE +4;
   rights.load( this, pos );
   return true;
}

void
TrafficPointRequestPacket::
addValues(const MC2Coordinate& center,
          uint32 maxRadius, int nbrOfHits,
          uint16 angle, TrafficDataTypes::direction direction,
          uint8 maxRoadClass){

   uint32 startPosition = readLong(REQUEST_HEADER_SIZE);
   writeLong(startPosition+TRAFFIC_LATPOS, center.lat);
   writeLong(startPosition+TRAFFIC_LONPOS, center.lon);
   writeLong(startPosition+TRAFFIC_RADPOS, maxRadius);
   writeShort(startPosition+TRAFFIC_ANGLEPOS, angle);
   writeByte(startPosition+TRAFFIC_DIRPOS, (byte)direction);
   writeByte(startPosition+TRAFFIC_NBRPOS, nbrOfHits);
   writeByte(startPosition+TRAFFIC_RCPOS, maxRoadClass);
   setLength(startPosition+TRAFFIC_STRINGPOS);
   
}
void
TrafficPointRequestPacket::addRoadName(const MC2String roadName){
   int pos = readLong(REQUEST_HEADER_SIZE)+TRAFFIC_STRINGPOS;
   incWriteString(pos, roadName.c_str());
   setLength(pos); 
}

uint32
TrafficPointRequestPacket::getLat() const
{
   
   return readLong(readLong(REQUEST_HEADER_SIZE)+TRAFFIC_LATPOS);
}

uint32
TrafficPointRequestPacket::getLon() const
{
   
   return readLong(readLong(REQUEST_HEADER_SIZE)+TRAFFIC_LONPOS);
}



uint32
TrafficPointRequestPacket::getMaxRadius() const
{
   return readLong(readLong(REQUEST_HEADER_SIZE)+TRAFFIC_RADPOS);
}


int
TrafficPointRequestPacket::getNbrOfHits() const
{
   return readByte(readLong(REQUEST_HEADER_SIZE)+TRAFFIC_NBRPOS);
}


uint16
TrafficPointRequestPacket::getAngle() const
{
   return readShort(readLong(REQUEST_HEADER_SIZE)+TRAFFIC_ANGLEPOS);
}

TrafficDataTypes::direction
TrafficPointRequestPacket::getTrafficDirection() const
{
   return static_cast<TrafficDataTypes::direction>
      (readByte(readLong(REQUEST_HEADER_SIZE)+TRAFFIC_DIRPOS));
}


uint8
TrafficPointRequestPacket::getMaxRoadClass() const
{
   return readByte(readLong(REQUEST_HEADER_SIZE)+TRAFFIC_RCPOS);
}


//////////////////////////////////////////////////////////////
// TrafficPointReplyPacket
//////////////////////////////////////////////////////////////
const uint32 
TrafficPointReplyPacket::m_mapIDPos = REPLY_HEADER_SIZE;

const uint32 
TrafficPointReplyPacket::m_nbrIDsPos = REPLY_HEADER_SIZE + 4;

const uint32 
TrafficPointReplyPacket::m_firstIDPos = REPLY_HEADER_SIZE + 8;


TrafficPointReplyPacket::
TrafficPointReplyPacket(const TrafficPointRequestPacket* inPacket,
                        int packetSize ) 
      : ReplyPacket(packetSize,
                    PACKETTYPE_TRAFFICPOINTREPLYPACKET,
                    inPacket,
                    StringTable::OK ) {
      setMapID(inPacket->getMapID());
      setNumberIDs(0);
      setLength(m_firstIDPos);
}

void
TrafficPointReplyPacket::setMapID(uint32 mapID) {
   writeLong(m_mapIDPos, mapID);
}

uint32
TrafficPointReplyPacket::getMapID() const {
   return readLong(m_mapIDPos);
}

bool
TrafficPointReplyPacket::isMapIDs() const {
   return (getMapID() == MAX_UINT32);
}

uint32
TrafficPointReplyPacket::getNumberIDs() const {
   return (readLong(m_nbrIDsPos));
}

void
TrafficPointReplyPacket::setNumberIDs(uint32 newNbrIDs) {
   writeLong(m_nbrIDsPos, newNbrIDs);
}

bool
TrafficPointReplyPacket::addID(uint32 ID) {
   if ( (getLength() + sizeof(uint32)) < getBufSize() ) {
      uint32 nbrIDs = getNumberIDs();
      int pos = m_firstIDPos + nbrIDs*sizeof(uint32);
      incWriteLong(pos, ID);
      setNumberIDs(nbrIDs+1);
      setLength(pos);
      return true;
   } else {
      mc2log << error << "TrafficPointReplyPacket::addID() - ID won't fit!!"
             << endl;
      return false;
   }
}

uint32
TrafficPointReplyPacket::getID(uint32 index) const {
   return (readLong(m_firstIDPos + index*4));
}

