/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExpandRoutePacket.h"
#include "StringTable.h"
#include "ExpandItemID.h"
#include "ExpandStringItem.h"
#include "RoutePacket.h"
#include "GenericMap.h"
#include "ExpandStringItem.h"
#include "LandmarkHead.h"
#include "PacketDump.h"
#include "ScopedArray.h"
#include "DisturbanceDescription.h"
#include "ExpandStringLane.h"
#include "ExpandStringSignPost.h"
#include "StringTableUtility.h"

#include "Math.h"

#ifndef _GNUC_
#include <stdio.h>
#define TRACE printf
#endif

// =======================================================================
//ExpandRouteRequestPacket =

ExpandRouteRequestPacket::ExpandRouteRequestPacket()
      : RequestPacket( MAX_PACKET_SIZE,
                       DEFAULT_PACKET_PRIO,
                       Packet::PACKETTYPE_EXPANDROUTEREQUEST,
                       0, // Must be set before using
                       0, // request ID
                       MAX_UINT32) 
{
   setLength( REQUEST_HEADER_SIZE + EXPAND_REQ_MAPID_TO_COUNTRY );
   // Initialize that not all names should be used in the turndescription.
   setPrintAllNames(false);
   // Initalize to not remove names at ahead "turns" if they differ
   setRemoveAheadIfNameDiffer(false);

   // Special debug for telenor. Use Coh option -C when starting MCS
   setNoConcatinate(false);
   
   // Do abbreviate the streets name per default.
   setAbbreviate(true);
   setIncludeLandmarks(false);
   
   setUseAddCost(false);
   
   // Use 3 passed streets as default for telenor, 0 for respons.
   setNbrPassStreets(3); 
   
   setNbrItems(0);
   setNbrMapIDToCountryID( 0 );
   setNbrTrafficInfo(0);
}

void  
ExpandRouteRequestPacket::setStartOffset( uint16 startOffset ){
    writeShort( REQUEST_HEADER_SIZE+EXPAND_REQ_START_OFFSET, 
                startOffset );
}

uint16
ExpandRouteRequestPacket::getStartOffset() const {
    return readShort( REQUEST_HEADER_SIZE+EXPAND_REQ_START_OFFSET );
}

void  
ExpandRouteRequestPacket::setEndOffset( uint16 endOffset ){
    writeShort( REQUEST_HEADER_SIZE+EXPAND_REQ_END_OFFSET, endOffset );
}

uint16  
ExpandRouteRequestPacket::getEndOffset() const {
   return readShort( REQUEST_HEADER_SIZE+EXPAND_REQ_END_OFFSET );
}

void  
ExpandRouteRequestPacket::setOrdinal(uint16 ordinal) {
   writeShort( REQUEST_HEADER_SIZE + EXPAND_REQ_ORDINAL, ordinal);
}

uint16  
ExpandRouteRequestPacket::getOrdinal() const {
   return readShort( REQUEST_HEADER_SIZE + EXPAND_REQ_ORDINAL );
}

void  
ExpandRouteRequestPacket::addNodeID( uint32 nodeID ){
   // Double the size if there isn't room for 50 more bytes.
   if ( updateSize( 50, getBufSize()) ) {
      mc2dbg << "[ERP]: Resized packet to " << getBufSize()
             << " bytes" << endl;
   }
   uint32 num = readShort( REQUEST_HEADER_SIZE+EXPAND_REQ_NUMITEMS);
   uint32 pos = REQUEST_HEADER_SIZE + EXPAND_REQ_MAPID_TO_COUNTRY + 
      getNbrMapIDToCountryID()*(4 + 4) + num*4;
   writeLong( pos, nodeID );
   writeShort(REQUEST_HEADER_SIZE+EXPAND_REQ_NUMITEMS,num+1);
   setLength(pos+4);
}

uint32 
ExpandRouteRequestPacket::getRouteItems( const GenericMap* theMap,
                                         list<uint32>& nodeIDs ) const
{
   uint32 nbrItems = getNbrItems();
   uint32 pos = REQUEST_HEADER_SIZE + EXPAND_REQ_MAPID_TO_COUNTRY+
      getNbrMapIDToCountryID()*(4 + 4);
   for ( uint32 i = 0; i < nbrItems; ++i ) {
      nodeIDs.push_back( readLong( pos ) ); 
      pos += 4;
   }
   // Expand the nodes.
   return theMap->expandNodeIDs( nodeIDs );
}

void
ExpandRouteRequestPacket::setLanguage(StringTable::languageCode language)
{
   writeLong(REQUEST_HEADER_SIZE+EXPAND_REQ_LANGUAGE, language);
}

uint32
ExpandRouteRequestPacket::getLanguage() const
{
   return readLong(REQUEST_HEADER_SIZE+EXPAND_REQ_LANGUAGE);
}


byte  
ExpandRouteRequestPacket::getType() const{
   return readByte(REQUEST_HEADER_SIZE+EXPAND_REQ_TYPE);
}

void  
ExpandRouteRequestPacket::setType(byte t) {
   writeByte(REQUEST_HEADER_SIZE+EXPAND_REQ_TYPE, t);
}

bool
ExpandRouteRequestPacket::includeLandmarks() const {
   return ((readByte(REQUEST_HEADER_SIZE+EXPAND_REQ_BOOL_BYTE)) & 0x1) != 0;
}

void
ExpandRouteRequestPacket::setIncludeLandmarks(bool includeLandmarks) {
   byte boolByte = readByte(REQUEST_HEADER_SIZE+EXPAND_REQ_BOOL_BYTE);
   if (includeLandmarks) {
      boolByte |= 0x1;
   } else {
      boolByte &= 0xe;
   }
   
   writeByte(REQUEST_HEADER_SIZE+EXPAND_REQ_BOOL_BYTE, boolByte);
}

bool
ExpandRouteRequestPacket::useAddCost() const {
   return ((readByte(REQUEST_HEADER_SIZE+EXPAND_REQ_BOOL_BYTE)) & 0x2) != 0;
}

void
ExpandRouteRequestPacket::setUseAddCost(bool ignoreAddCost) {
   byte boolByte = readByte(REQUEST_HEADER_SIZE+EXPAND_REQ_BOOL_BYTE);
   if (ignoreAddCost) {
      boolByte |= 0x2;
   } else {
      boolByte &= 0xd;
   }
   
   writeByte(REQUEST_HEADER_SIZE+EXPAND_REQ_BOOL_BYTE, boolByte);
}


byte
ExpandRouteRequestPacket::getNbrPassStreets() const
{
   return readByte(REQUEST_HEADER_SIZE+EXPAND_REQ_PASSEDSTREET);
}

void
ExpandRouteRequestPacket::setNbrPassStreets(byte nbrOfStreets)
{
   mc2dbg8 << "[ExpandRouteRequestPacket] setting " << (int)nbrOfStreets
           << " passed streets." << endl;
   writeByte(REQUEST_HEADER_SIZE+EXPAND_REQ_PASSEDSTREET, nbrOfStreets);
}

void  
ExpandRouteRequestPacket::setLastID(uint32 ID) {
   writeLong( REQUEST_HEADER_SIZE + EXPAND_REQ_LAST_ID, ID );
}

uint32  
ExpandRouteRequestPacket::getLastID(void) const {
   return readLong( REQUEST_HEADER_SIZE + EXPAND_REQ_LAST_ID);
}

void  
ExpandRouteRequestPacket::setLastMapID(uint32 mapID) {
   writeLong( REQUEST_HEADER_SIZE + EXPAND_REQ_LAST_MAP, mapID);
}

uint32  
ExpandRouteRequestPacket::getLastMapID(void) const {
   return readLong( REQUEST_HEADER_SIZE + EXPAND_REQ_LAST_MAP );
}


void 
ExpandRouteRequestPacket::setLastCountry( uint32 countryID ) {
   writeLong( (REQUEST_HEADER_SIZE + EXPAND_REQ_LAST_COUNTRY), countryID );
}


uint32 
ExpandRouteRequestPacket::getLastCountry() const {
   return readLong( REQUEST_HEADER_SIZE + EXPAND_REQ_LAST_COUNTRY );
}


bool 
ExpandRouteRequestPacket::setMapIDToCountryID( 
   map<uint32,uint32>& mapID2CountryID ) 
{
   if ( getNbrItems() == 0 ) {
      setNbrMapIDToCountryID( mapID2CountryID.size() );
      int pos = REQUEST_HEADER_SIZE + EXPAND_REQ_MAPID_TO_COUNTRY;
      for ( map<uint32,uint32>::const_iterator it = mapID2CountryID.begin()
               ; it != mapID2CountryID.end() ; it++ )
      {
         incWriteLong( pos, it->first );
         incWriteLong( pos, it->second );
      }

      setLength( pos );
      
      return true;
   } else {
      mc2log << error << "ExpandRouteRequestPacket::setMapIDToCountryID "
             << "Called when having route items!" << endl;

      return false;
   }
}


void 
ExpandRouteRequestPacket::getMapIDToCountryID( 
   map<uint32,uint32>& mapID2CountryID ) const 
{
   int pos = REQUEST_HEADER_SIZE + EXPAND_REQ_MAPID_TO_COUNTRY;
   for ( uint32 i = 0 ; i < getNbrMapIDToCountryID() ; ++i ) {
      uint32 mapID = incReadLong( pos );
      uint32 countryID = incReadLong( pos );
      mapID2CountryID.insert( make_pair( mapID, countryID ) );
   }
}


uint32 
ExpandRouteRequestPacket::getNbrMapIDToCountryID() const {
   return readLong( REQUEST_HEADER_SIZE + EXPAND_REQ_NBR_MAPID_TO_COUNTRY );
}


void 
ExpandRouteRequestPacket::setNbrMapIDToCountryID( uint32 nbr ) {
   writeLong( REQUEST_HEADER_SIZE + EXPAND_REQ_NBR_MAPID_TO_COUNTRY, nbr );
}

void
ExpandRouteRequestPacket::setNoConcatinate(bool noConcatinate)
{
   writeByte(REQUEST_HEADER_SIZE + EXPAND_REQ_NO_CONCATINATE, noConcatinate);
}

bool
ExpandRouteRequestPacket::getNoConcatinate() const
{
   return (readByte(REQUEST_HEADER_SIZE + EXPAND_REQ_NO_CONCATINATE) != 0);
}
void
ExpandRouteRequestPacket::nameChangeAsTurn(bool nameChange)
{
   writeByte(REQUEST_HEADER_SIZE + EXPAND_REQ_NAME_CHANGE_AS, nameChange);
}

bool
ExpandRouteRequestPacket::getNameChangeAsTurn() const
{
   return (readByte(REQUEST_HEADER_SIZE + EXPAND_REQ_NAME_CHANGE_AS) != 0);
}

uint16
ExpandRouteRequestPacket::getNbrTrafficInfo() const
{
   return (readShort(REQUEST_HEADER_SIZE + EXPAND_REQ_TRAF_INFO_NBR));
}
void
ExpandRouteRequestPacket::setNbrTrafficInfo(uint16 nbr)
{
   writeShort(REQUEST_HEADER_SIZE + EXPAND_REQ_TRAF_INFO_NBR, nbr);
}


   
void
ExpandRouteRequestPacket::addTrafficInfo(uint32 nodeID, bool split,
                             uint32 distNodeID, bool merge, uint32 distID,
                             TrafficDataTypes::disturbanceType type,
                             const char* text)
{
   if(getNbrTrafficInfo() == 0){
      writeShort(REQUEST_HEADER_SIZE +EXPAND_REQ_TRAF_INFO_POS, getLength());
   }
   int pos = getLength();
   uint32 textSize = 0;
   if(text != NULL){
      textSize = strlen(text);
   }
   
   // Add 3 longs, 3 short string and some padding to be sure.
   updateSize( 3*4+3*2+textSize+6, getBufSize() );
   incWriteLong(pos, nodeID);
   incWriteLong(pos, distID);
   incWriteLong(pos, distNodeID);
   incWriteShort(pos, int(type));
   incWriteShort(pos, int(split));
   incWriteShort(pos, int(merge));
   incWriteString(pos, text);
   setLength(pos);
   setNbrTrafficInfo(getNbrTrafficInfo()+1);
}

int
ExpandRouteRequestPacket::
addTrafficInfo(const vector<DisturbanceDescription>& dVect)
{
   int nbrOfDists = 0;
   uint32 myMapID = getMapID(); // RequestPacket::getMapID()
   

   
   for( vector<DisturbanceDescription>::const_iterator vi = dVect.begin() ;
        vi != dVect.end();
        ++vi ) {
      // Check all those disturbances where description is  set
      if((*vi).isDescriptionSet() || (*vi).isDetour()){
         // No detour
         if(!(*vi).isDetour()){
            // Add it if it is on the map.
            if((*vi).getMapID() == myMapID){
               addTrafficInfo((*vi).getNodeID(), false,(*vi).getNodeID() ,
                              false, (*vi).m_distID, (*vi).m_type,
                              (*vi).m_comment.data());
               nbrOfDists++;
               mc2dbg4 << "Add disturbance to expandPacket." << endl
                       << " node :" << (*vi).getNodeID() << ", distID:"
                       << (*vi).m_distID << " , type:" << int((*vi).m_type)
                       << ",text: " << (*vi).m_comment.data() << endl;
               
            }
         } else {
            // Detour. All points might be on this map.
            uint32 distPoint = MAX_UINT32;
            if((*vi).getMapID() == myMapID)
               distPoint = (*vi).getNodeID();
            
            // Check split
            if((*vi).m_splitPoint.getMapID() == myMapID){
               addTrafficInfo((*vi).m_splitPoint.getItemID(), true, distPoint,
                              false,(*vi).m_distID, (*vi).m_type,
                              (*vi).m_comment.data());
               nbrOfDists++;
               mc2dbg2 << "Add detour split to expandPacket." << endl
                       << "  - disturbance at node : " << distPoint << endl;
            }

            // Check merge
            if((*vi).m_joinPoint.getMapID() == myMapID){
               addTrafficInfo((*vi).m_joinPoint.getItemID(), false, distPoint,
                              true, (*vi).m_distID, (*vi).m_type,
                              (*vi).m_comment.data());  
               nbrOfDists++;
               mc2dbg2 << "Add detour merge to expandPacket." << endl
                    << "  - disturbance at node : " << distPoint << endl;
            }
         }
         
         
      } else {
         mc2log << warn << "[ERP] Description was not set" << endl;
      }
      
   }
   return nbrOfDists;
}


int
ExpandRouteRequestPacket::getTrafficInfo( int &pos, uint32 &nodeID,
                             bool &split, uint32 &distNodeID, bool &merge,
                             uint32 &distID,
                             TrafficDataTypes::disturbanceType &type,
                             char* &text) const
{
   uint16 firstPos =
      readShort(REQUEST_HEADER_SIZE +EXPAND_REQ_TRAF_INFO_POS);
   if(pos < int(firstPos))
      pos = int(firstPos);
   nodeID = incReadLong(pos);
   distID = incReadLong(pos);
   distNodeID = incReadLong(pos);
   type   = (TrafficDataTypes::disturbanceType)incReadShort(pos);
   split  = (incReadShort(pos) != 0);
   merge  = (incReadShort(pos) != 0);
   incReadString(pos, text);
   mc2dbg4 << "Got disturbance from expandPacket." << endl
           << " node :" << nodeID << ", distID:" << distID
           << " , type:" << int(type) << ",text: " << text << endl;
   return pos;
}



// =======================================================================
//                                                ExpandRouteReplyPacket =

#define EXPAND_ROUTE_REPLY_MAX_FILL_LEVEL (int(getBufSize() - 512))

ExpandRouteReplyPacket::ExpandRouteReplyPacket(int size)
      : ReplyPacket( size,                    
                     Packet::PACKETTYPE_EXPANDROUTEREPLY) {
   setNumStringData(0);
   setNumItemData(0);
   setSizeStringData(0);
   setSizeItemsData( 0 );
   setNbrItemsPerString(0);

   // This is not necessery, but good when debugging...
   setTotalDist(0);
   setTotalTime(0);
   setStandStillTime(0);

   setStartDirectionHousenumber(ItemTypes::unknown_nbr_t);
   setStartDirectionOddEven(ItemTypes::unknown_oddeven_t);

   setStatus(StringTable::OK);
   setLength(REPLY_HEADER_SIZE+EXPAND_REPLY_SUB_HEADER_SIZE);
}
    
ExpandRouteReplyPacket::ExpandRouteReplyPacket( const ExpandRouteRequestPacket* p )
      : ReplyPacket( MAX_PACKET_SIZE,
                     PACKETTYPE_EXPANDROUTEREPLY,
                     p,
                     StringTable::OK) {
   setNumStringData(0);
   setNumItemData(0);
   setSizeStringData(0);
   setSizeItemsData( 0 );
   setNbrItemsPerString(0);

   // This is not necessery, but good when debugging...
   setTotalDist(0);
   setTotalTime(0);
   setStandStillTime(0);

   // Transfer the information from p to this
   setRouteType( p->getType() );
   setOrdinal( p->getOrdinal());   
   setStartOffset( p->getStartOffset() );
   setEndOffset( p->getEndOffset() );
   setLength(REPLY_HEADER_SIZE + EXPAND_REPLY_SUB_HEADER_SIZE);
}

ExpandItemID* 
ExpandRouteReplyPacket::getItemID() const {
   uint32 size = getNumItemData();
   ExpandItemID* items = new ExpandItemID;
   int position = REPLY_HEADER_SIZE + 
                  EXPAND_REPLY_SUB_HEADER_SIZE + 
                  getSizeStringData();
   mc2dbg8 << "Size " << size << " pos " << position << endl;
   
   for(uint32 i=0; i<size; i++)  {
      uint32 mapID  = incReadLong(position);
      uint32 itemID = incReadLong(position);
      int32 lat=0,lon=0, groupID=0;
      uint16 nbrCoords=0;
      if( getRouteType() & ROUTE_TYPE_ITEM_STRING ){         
         groupID = getGroupID(i);
         mc2dbg8 << " GroupID " << i << ":" << groupID << endl;
         if ( getRouteType() & ROUTE_TYPE_GFX_COORD ) {
            lat = incReadLong(position);
            lon = incReadLong(position);           
            items->addItem( mapID, groupID, itemID, lat, lon);
         }
         else if ( (getRouteType() & ROUTE_TYPE_NAVIGATOR) ||
                   (getRouteType() & ROUTE_TYPE_ALL_COORDINATES) ||
                   (getRouteType() & ROUTE_TYPE_GFX_TRISS) ) 
         {
            items->addItem( mapID, groupID, itemID);
            items->addSpeedLimit(incReadByte(position));
            items->addAttributes(incReadByte(position));
            nbrCoords = incReadShort(position);
            for (uint16 j=0; j < nbrCoords; j++) {
               lat = incReadLong(position);
               lon = incReadLong(position);
               items->addCoordinate(lat, lon);
            }
         }
         else{
            items->addItem( mapID, groupID, itemID );
         }
      }
      else if ( getRouteType() & ROUTE_TYPE_GFX_COORD ) {
         lat = incReadLong(position);
         lon = incReadLong(position);
         items->addItem( mapID, itemID, lat, lon);      
      } else if ( getRouteType() & ROUTE_TYPE_NAVIGATOR ) {
         items->addItem( mapID, itemID );
         items->addSpeedLimit(incReadByte(position));
         items->addAttributes(incReadByte(position));
         nbrCoords = incReadShort(position);
         for (uint16 j=0; j < nbrCoords; j++) {
            lat = incReadLong(position);
            lon = incReadLong(position);
            items->addCoordinate(lat, lon);
         }
      }
   }
   // Add last coordinate
   int32 lat = getLastItemLat();
   int32 lon = getLastItemLon();
   items->addCoordinate(lat, lon);

   return items;
}

bool
ExpandRouteReplyPacket::setStringData( 
               const char* str,
               uint32 stringCode,
               uint32 dist,
               uint32 time,
               ItemTypes::transportation_t transport,
               int32 lat,
               int32 lon,
               byte nameType,
               byte turnNumber,
               ItemTypes::crossingkind_t crossingKind,
               uint32 nbrPossTurns,
               uint32* possTurns,
               const ExpandStringLanesCont& lanes,
               const ExpandStringSignPosts& signPosts,
               LandmarkHead* landmarks)
{
   uint32 strSize = 4; // 4 *  '\0'
   if ( str != NULL ) {
      strSize += strlen( str ) + 1;
   }
   uint32 objectSize = 0;
   objectSize += lanes.getSizeInPacket();
   objectSize += signPosts.getSizeInPacket();

   // Update size to include strings, longs, bytes and then nbrLMs
   updateSize(
      4*5 + 4*1 + strSize + 4 + 4*nbrPossTurns + objectSize, 
      (getBufSize() + MAX_PACKET_SIZE/2 ) * 2);
   
   int position = getLength();
   //if (position < EXPAND_ROUTE_REPLY_MAX_FILL_LEVEL) {
      mc2dbg8 << "********** EXITCOUNT: " << (int)turnNumber << endl;
      mc2dbg8 << "******************** CROSSINGKIND: " << 
      StringTable::getString(ItemTypes::getCrossingKindSC(crossingKind),
                             StringTable::ENGLISH) << endl;
   
      incWriteLong(position, stringCode);
      incWriteLong(position, dist);
      incWriteLong(position, time);
      incWriteLong(position, lat); // Lat 
      incWriteLong(position, lon); // Lon, before string to avoid dble padding
      if(nbrPossTurns < 7) {
         incWriteLong(position, nbrPossTurns);
         for(uint32 i = 0 ; i < nbrPossTurns ; i++){
            incWriteLong(position, possTurns[i]);
         }
      }else{
         incWriteLong(position, 0);
      }
      incWriteByte(position, byte(transport));
      incWriteByte(position, nameType);
      incWriteByte(position, turnNumber);
      incWriteByte(position, crossingKind);
      incWriteString(position, str);
      // Lanes
      lanes.save( this, position );
      // Signposts
      signPosts.save( this, position );
      // writeNbrLandmarks, then loop and write the lms
      if (landmarks == NULL) {
         mc2dbg8 << " for str=\"" << str << "\", setting no landmarks" << endl;
         incWriteLong(position, 0);
      } else {
         uint32 nbrLMs = landmarks->cardinal();
         incWriteLong(position, nbrLMs);
         mc2dbg4 << " for str=\"" << str << "\" setting " 
                 << nbrLMs << " LMs" << endl;
         LandmarkLink* lmLink = static_cast<LandmarkLink*>(landmarks->first());
         while (lmLink != NULL) {
            // update packet size for each landmark
            updateSize(3*4 + 4*1 + strlen(lmLink->getLMName())+1,
                       MAX_PACKET_SIZE / 2);
            byte boolByte = 0;
            if(lmLink->isEndLM())
               boolByte |= 0x1;
            if(lmLink->isDetour())
               boolByte |= 0x2;
            if(lmLink->isStart())
               boolByte |= 0x4;
            if(lmLink->isTraffic())
               boolByte |= 0x8;
            if(lmLink->isStop())
               boolByte |= 0x10;
            
            //mc2dbg4 << endl << hex << "Boolbyte : " << int(boolByte) << dec
            //     << endl << endl;
            //lmLink->dump();
            incWriteLong(position, lmLink->m_lmdesc.itemID);
            incWriteLong(position, lmLink->m_lmdesc.importance);
            incWriteLong(position, lmLink->m_lmDist);
            incWriteByte(position, byte(lmLink->m_lmdesc.side));
            incWriteByte(position, byte(lmLink->m_lmdesc.location));
            incWriteByte(position, byte(lmLink->m_lmdesc.type));
            incWriteByte(position, boolByte);
            incWriteString(position, lmLink->getLMName());
            incWriteString(position, lmLink->getStreetName());
            DEBUG4(
            uint32 id = lmLink->m_lmdesc.itemID;
            uint32 imp = lmLink->m_lmdesc.importance; 
            SearchTypes::side_t side = lmLink->m_lmdesc.side;
            ItemTypes::landmarklocation_t loc = lmLink->m_lmdesc.location;
            ItemTypes::landmark_t type = lmLink->m_lmdesc.type;
            int32 dist = lmLink->m_lmDist;
            const char* name = lmLink->getLMName();
            mc2dbg2 << "LM: id=" << id << " imp=" << imp 
                   << " side=" << int(side) 
                   << " loc=" << int(loc) << " type=" << int(type)
                   << " endLM=" << lmLink->isEndLM() << " dist=" << dist 
                   << " name=" << name << endl;

            DescProps descProps = ExpandStringItem::createRouteDescProps(
               StringTable::SWEDISH, false, 9,true, true, true, 
               StringTableUtility::NORMAL, false, ItemTypes::leftOddRightEven,
               ItemTypes::increasing);
            char* buf = new char[257];
            uint32 nbrBytesWritten = 0;
            lmLink->getRouteDescription(descProps, buf,
               256, nbrBytesWritten, StringTable::stringCode(stringCode));
            mc2dbg2 << "DESC: \"" << buf << "\"" << endl;
            delete [] buf;
            );

            lmLink = static_cast<LandmarkLink*>(lmLink->suc());
         }
      }
         
      incNumStringData();
      setSizeStringData(position - 
                        REPLY_HEADER_SIZE - 
                        EXPAND_REPLY_SUB_HEADER_SIZE);
      setLength(position);
      return (true);
      // } else {
      //return (false);
      //}
}


ExpandStringItem** 
ExpandRouteReplyPacket::getStringDataItem() const {
   uint32 numOfItems = getNumStringData();
   ExpandStringItem** vect = new ExpandStringItem*[numOfItems];
   int position = REPLY_HEADER_SIZE + EXPAND_REPLY_SUB_HEADER_SIZE;
   uint32 dist, time, stringCode;
   int32 lat, lon;
   uint32 nbrOfPossTurns;
   uint32 possTurns[7];
   byte nameType;
   ItemTypes::transportation_t transport;
   byte turnNumber;
   ItemTypes::crossingkind_t crossingKind;
   char* str;
   uint32 nbrLMs;
   char* lmName;
   char* lmStreetName;
   for( uint32 i=0; i<numOfItems; i++){
      stringCode = incReadLong(position);
      dist       = incReadLong(position);
      time       = incReadLong(position);
      lat        = incReadLong(position);
      lon        = incReadLong(position);
      nbrOfPossTurns = incReadLong(position);
      MC2_ASSERT(nbrOfPossTurns < 7);
      for(uint32 j = 0 ; j < nbrOfPossTurns ; j++){
         possTurns[j] = incReadLong(position);
      }
      transport  = ItemTypes::transportation_t(incReadByte(position));
      nameType   = incReadByte(position); // Add reading of nametype here
      turnNumber = incReadByte(position);  // Read the number of the turn
      crossingKind = ItemTypes::crossingkind_t(incReadByte(position));
      
      incReadString(position, str);
      // Lanes container
      ExpandStringLanesCont lanes;
      lanes.load( this, position );
      // Signposts
      ExpandStringSignPosts signPosts;
      signPosts.load( this, position );
      // read number of LMs, create a landmarkHead,
      // loop the lms and create & insert lmLinks into the head.
      nbrLMs = incReadLong(position);
      mc2dbg8 << " to read " << nbrLMs << " LMs from the packet" << endl;
      LandmarkHead* landmarks = NULL;
      if (nbrLMs > 0) {
         landmarks = new LandmarkHead();
         mc2dbg4 << "  creating LMs";
         for (uint32 j = 0; j < nbrLMs; j++) {
            
            ItemTypes::lmdescription_t lmdesc;
            lmdesc.itemID     = incReadLong(position);
            lmdesc.importance = incReadLong(position);
            uint32 dist       = incReadLong(position);
            lmdesc.side       = SearchTypes::side_t(incReadByte(position));
            lmdesc.location   = ItemTypes::landmarklocation_t(
                                             incReadByte(position));
            lmdesc.type       = ItemTypes::landmark_t(incReadByte(position));
            byte boolByte     = incReadByte(position);
            incReadString(position, lmName);
            incReadString(position, lmStreetName);
            
            LandmarkLink* newLM = new LandmarkLink(lmdesc, dist, false,
                                                   lmName);
            newLM->setEndLM((boolByte & 0x1) != 0);
            
            if((boolByte & 0x2) != 0)
               newLM->setDetour();
            newLM->setIsStart((boolByte & 0x4) != 0);
            if((boolByte & 0x8) != 0)
               newLM->setIsTraffic();
            newLM->setIsStop((boolByte & 0x10) != 0);
            newLM->setStreetName(lmStreetName);
            
            //newLM->dump();
            
            
            
            
            newLM->into(landmarks);
            mc2dbg4 << " (" << landmarks->cardinal() << ")" << lmName;

         }
         mc2dbg4 << endl;
      }
      vect[i]= new ExpandStringItem( dist, time, stringCode, str, transport,
                                     lat, lon, nameType, turnNumber,
                                     lanes, signPosts,
                                     crossingKind,
                                     landmarks,
                                     nbrOfPossTurns,
                                     possTurns);
      delete landmarks;
   }
   return vect;
}

void
ExpandRouteReplyPacket::setItemData( uint32 mapID,
                                     uint32 itemID )
{
#ifdef DEBUG1
   if ( getRouteType() & ROUTE_TYPE_GFX_COORD )
      mc2log << error
             << "ERROR: setItemData(uint, uint) called when routeType & "
             << "ROUTE_TYPE_GFX_COORD != 0" << endl;
   
#endif
   updateSize(2*4, MAX_PACKET_SIZE / 2);
   int position = getLength();
   incWriteLong( position, mapID );
   incWriteLong( position, itemID );
   setSizeItemsData( getSizeItemsData() + 8 );
   setLength( position );
   incNumItemData();
}

void
ExpandRouteReplyPacket::setItemData( uint32 mapID,
                                     uint32 itemID,
                                     int32 lat,
                                     int32 lon)
{
#ifdef DEBUG1
   if ( ( getRouteType() & ROUTE_TYPE_GFX_COORD ) == 0 ) {
      mc2log << error
             << "ERROR: setItemData(uint32 mapID, uint32 itemID, int32 lat, "
             <<"int32 lon) called when routeType != ROUTE_TYPE_GFX_COORD" 
             << endl;
   }
#endif
   updateSize(4*4, MAX_PACKET_SIZE / 2);
   int position = getLength();
   incWriteLong( position, mapID);
   incWriteLong( position, itemID);
   incWriteLong( position, lat);
   incWriteLong( position, lon);
   mc2dbg4<< "EXP_ROUTE_REP::Setting coord : (" << lat << "," << lon << ")"
          << endl;
   setSizeItemsData( getSizeItemsData() + 16 );
   setLength( position );
   incNumItemData();
}

uint32 
ExpandRouteReplyPacket::setNavItemData( uint32 mapID, 
                                        uint32 itemID, 
                                        byte speedLimit,
                                        byte attributes )
{
#ifdef DEBUG1
   if ( (getRouteType() & ROUTE_TYPE_NAVIGATOR) == 0 &&
        (getRouteType() & ROUTE_TYPE_ALL_COORDINATES) == 0 &&
        (getRouteType() & ROUTE_TYPE_GFX_TRISS) == 0 ) {
      mc2log << error
             << "ERROR: setNavItemData(uint32 mapID, uint32 itemID, "
             << "byte speedLimit) "
             << "called when routeType != ROUTE_TYPE_NAVIGATOR" << endl;
   }
#endif
   updateSize(2*4+ 4, MAX_PACKET_SIZE / 2);
   int position = getLength();
   uint32 retVal;
   
   incWriteLong(position, mapID);
   incWriteLong(position, itemID);
   
   incWriteByte(position, speedLimit);
   incWriteByte(position, attributes);
   retVal = position; // Store the position for nbrCoords
   incWriteShort(position, 0); // Nbr coordinates. Filled in later.
   
   setSizeItemsData( getSizeItemsData() + 12 );
   setLength(position);
   incNumItemData();
   
   return (retVal);

}

void 
ExpandRouteReplyPacket::addCoordToNavItemData( int32 lat, int32 lon, 
                                               uint32 nbrCoordPos)
{
#ifdef DEBUG1
   if ( (getRouteType() & ROUTE_TYPE_NAVIGATOR) == 0 &&
        (getRouteType() & ROUTE_TYPE_ALL_COORDINATES) == 0 &&
        (getRouteType() & ROUTE_TYPE_GFX_TRISS) == 0 ) {
      mc2log << error
             << "ERROR: addCoordToNavItemData(int32 lat, int32 lon, "
             << "uint32 nbrCoordPos "
             << "called when routeType != ROUTE_TYPE_NAVIGATOR" << endl;
   }
#endif
   updateSize( 2*4 , MAX_PACKET_SIZE / 2);
   // Update the nbr of coordinates
   uint16 nbrCoords = readShort(nbrCoordPos);
   writeShort(nbrCoordPos, ++nbrCoords);

   // Add the new coordinate last
   int position = getLength();
   incWriteLong(position, lat);
   incWriteLong(position, lon);
   mc2dbg4 << "EXP_ROUTE_REP::Setting Coord : (" << lat << "," << lon << ")"
          << endl;
   setSizeItemsData( getSizeItemsData() + 8 );
   setLength(position);
}

void ExpandRouteReplyPacket::dump2( bool headerOnly ) const {
   //Packet::dump(false);
   TRACE("Status code: %d\n", getStatus() );
   TRACE("SUBHEADER:\n");
   TRACE("RouteType: %d\n", getRouteType() );
   TRACE("StartDir: %d\n", getStartDir() );
   TRACE("EndDir: %d\n", getEndDir() );
   TRACE("NumStringData: %d\n", getNumStringData() );
   TRACE("Stringsize: %d\n", getSizeStringData() );
   TRACE("NumItemData: %d\n", getNumItemData() );
   TRACE("Itemsize: %d\n", getSizeItemsData() );
   TRACE("NumItemsPerString: %d\n", getNbrItemsPerString() );
   TRACE("Ordinal: %d\n", getOrdinal() );
   TRACE("Total dist: %d\n", getTotalDist() );
   TRACE("Total time: %d\n", getTotalTime() );
   TRACE("Total standstilltime: %d\n", getStandStillTime() );
   TRACE("LastLeftCount: %d\n", getLastSegmentsLeftStreetCount() );
   TRACE("LastRightCount: %d\n", getLastSegmentsRightStreetCount() );
   TRACE("StartOffset: %d\n", getStartOffset() );
   TRACE("EndOffset: %d\n", getEndOffset() );
   TRACE("LastLat: %d\n", getLastItemLat() );
   TRACE("LastLon: %d\n", getLastItemLon() );

   if( !headerOnly ) {
      PacketUtils::
         dumpDataToFile( stdout, *this,
                         REPLY_HEADER_SIZE+EXPAND_REPLY_SUB_HEADER_SIZE );
   }
}

bool
ExpandRouteReplyPacket::getTotalDistStr(DescProps descProps,
                                        char* buf,
                                        uint32 maxLength,
                                        uint32 nbrBytesWritten) const
{
   // Extract information from the description properties bitfield.
   StringTable::languageCode lang = descProps.getLanguage();
   StringTableUtility::distanceFormat distFormat = descProps.getDistFormat() ;
   StringTableUtility::distanceUnit distUnit = descProps.getDistUnit() ;
   bool wapFormat = descProps.getWapFormat();

   buf[0] = '\0';

   // Add "Total distance"
   if (! ExpandStringItem::addString(  
         StringTable::getString(
            StringTable::TOTAL_DISTANCE, lang),
         buf, maxLength, nbrBytesWritten, wapFormat)  ) { 
      return (false);
   } 

   // Add space
   if (! ExpandStringItem::addString(
         " ", buf, maxLength, nbrBytesWritten, wapFormat)) {
      return (false);
   }

   // Create a temporary buffer
   ScopedArray<char> distBuf( new char[1024] );
   StringTableUtility::printDistance( distBuf.get(), getTotalDist(), 
                                 lang, distFormat, distUnit );
   
   // Add the "5 km"
   return ExpandStringItem::addString( distBuf.get(), buf, maxLength,
                                       nbrBytesWritten, wapFormat  );
}

bool
ExpandRouteReplyPacket::getTimeStr(DescProps descProps,
                                   char* buf,
                                   uint32 maxLength,
                                   uint32 nbrBytesWritten,
                                   bool standStillTime) const
{
   // Extract information from the description properties bitfield.
   StringTable::languageCode lang = descProps.getLanguage();
   bool wapFormat = descProps.getWapFormat();

   buf[0] = '\0';

   // Set the variables depending on if it's standstill time or not.
   const char* timeString;
   uint32 time;
   if (standStillTime) {
      // Total standstill time
      timeString = StringTable::getString(
         StringTable::TOTAL_STANDSTILL_TIME, lang);
      time = getStandStillTime();
   } else {
      // Total time
      timeString = StringTable::getString(
         StringTable::TOTAL_TIME, lang);
      time = getTotalTime();
   }
   
   // Add "Total time"
   if (! ExpandStringItem::addString(
         timeString, buf, maxLength, nbrBytesWritten, wapFormat)) {
      return (false);
   } 

   // Add space
   if (! ExpandStringItem::addString(
         " ", buf, maxLength, nbrBytesWritten, wapFormat)) {
      return (false);
   }

   // Create a temporary buffer
   ScopedArray<char> timeBuf( new char[16] );
   StringUtility::splitSeconds( time, timeBuf.get() );
   
   // Add time "14:12"
   return ExpandStringItem::addString( timeBuf.get(), buf, maxLength, 
                                       nbrBytesWritten, wapFormat  );


}

bool
ExpandRouteReplyPacket::getAverageSpeedStr(DescProps descProps,
                                           char* buf,
                                           uint32 maxLength,
                                           uint32 nbrBytesWritten) const
{
   // Extract information from the description properties bitfield.
   StringTable::languageCode lang = descProps.getLanguage();
   bool wapFormat = descProps.getWapFormat();

   buf[0] = '\0';

   if (getTotalTime() > 0) { // Avoid divide by zero
      // Add "Average speed"
      if (! ExpandStringItem::addString(  
            StringTable::getString(StringTable::AVERAGE_SPEED, lang),
            buf, maxLength, nbrBytesWritten, wapFormat)  ) { 
         return (false);
      } 

      // Create a temporary buffer
      ScopedArray<char> speedBuf( new char[16] );
      
      // Compute average speed
      uint16 avgSpeed = (uint16)( Math::MS_TO_KMH * (
         ((float64) getTotalDist()) /
         getTotalTime() ) );
      
      sprintf(speedBuf.get(), " %d", avgSpeed);
      
      // Add the speed "70"
      if (! ExpandStringItem::addString(
         speedBuf.get(), buf, maxLength, nbrBytesWritten, wapFormat  )  ) {
         return (false);
      }

      // Add space
      if (! ExpandStringItem::addString(
            " ", buf, maxLength, nbrBytesWritten, wapFormat)) {
         return (false);
      }

      // Add "km/h"
      if (! ExpandStringItem::addString(
            StringTable::getString(StringTable::KM_PER_HOUR, lang),
            buf, maxLength, nbrBytesWritten, wapFormat  )  ) {
         return (false);
      }
   }

   return (true);   
   
}

uint32 
ExpandRouteReplyPacket::getGroupID( uint32 index ) const
{
   uint32 nbr = getNbrItemsPerString();
   uint32 totalNbr = 0;
   mc2dbg8 << "GetGroupID " << index
           << " nbr of items " << nbr << endl;
   
   for( uint32 i=0; i<nbr; i++){
      totalNbr +=  getItemsPerString(i);
      mc2dbg8 << i << ":" << totalNbr << " " << endl;
      if(index < totalNbr ){
         mc2dbg8 << "GroupID =" << i << endl;
         return i;
      }
   }
   return MAX_UINT32;
}


bool  
ExpandRouteReplyPacket::getStartDir() const {
   byte b = readByte(REPLY_HEADER_SIZE + EXPAND_REPLY_HEADING );
   if ( (b & 0x80) == 0x80) {
      // The first bit is set
      return (true);
   } else {
      return (false);
   }
}

void  
ExpandRouteReplyPacket::setStartDir(bool towards0) {
   byte b = readByte(REPLY_HEADER_SIZE+ EXPAND_REPLY_HEADING);
   if (towards0) {
      b |= 0x80;
   } else {
      b &= 0x7f;
   }
   writeByte(REPLY_HEADER_SIZE+EXPAND_REPLY_HEADING, b);
}

bool  
ExpandRouteReplyPacket::getEndDir() const {
   byte b = readByte(REPLY_HEADER_SIZE + EXPAND_REPLY_HEADING);
   if ( (b & 0x40) == 0x40) {
      // The first bit is set
      return (true);
   } else {
      return (false);
   }
}

void  
ExpandRouteReplyPacket::setEndDir(bool towards0) {
   byte b = readByte(REPLY_HEADER_SIZE+EXPAND_REPLY_HEADING);
   if (towards0) {
      b |= 0x40;
   } else {
      b &= 0xbf;
   }
   writeByte(REPLY_HEADER_SIZE+EXPAND_REPLY_HEADING, b);
}   

bool 
ExpandRouteReplyPacket::getUturn() const
{
   byte b = readByte(REPLY_HEADER_SIZE+EXPAND_REPLY_UTURN);
   b &= 0x80;
   if(b == 0x80)
      return true;
   else
      return false;
}
         
void 
ExpandRouteReplyPacket::setUturn(bool uTurn) 
{
   byte b = readByte(REPLY_HEADER_SIZE+EXPAND_REPLY_UTURN);
   if(uTurn)
      b |= 0x80;
   else 
      b &= 0x7f;
   writeByte(REPLY_HEADER_SIZE+EXPAND_REPLY_UTURN, b);
}

void  
ExpandRouteReplyPacket::setOrdinal(uint16 ordinal) {
   writeShort( REPLY_HEADER_SIZE + EXPAND_REPLY_ORDINAL, ordinal );
}

uint16  
ExpandRouteReplyPacket::getOrdinal() const {
   return readShort( REPLY_HEADER_SIZE + EXPAND_REPLY_ORDINAL);
}

uint32  
ExpandRouteReplyPacket::getTotalDist() const {
   return readLong(REPLY_HEADER_SIZE+EXPAND_REPLY_TOT_DIST);
}

void  
ExpandRouteReplyPacket::setTotalDist( uint32 dist ){
   writeLong(REPLY_HEADER_SIZE+EXPAND_REPLY_TOT_DIST, dist);
}

uint32  
ExpandRouteReplyPacket::getTotalTime() const {
   return readLong(REPLY_HEADER_SIZE+EXPAND_REPLY_TOT_TIME);
}

void  
ExpandRouteReplyPacket::setTotalTime(uint32 time ){
   writeLong(REPLY_HEADER_SIZE+EXPAND_REPLY_TOT_TIME, time);
}

uint32  
ExpandRouteReplyPacket::getStandStillTime() const {
   return readLong(REPLY_HEADER_SIZE+EXPAND_REPLY_TOT_STANDSTILL);
}

void  
ExpandRouteReplyPacket::setStandStillTime( uint32 time ){
   writeLong(REPLY_HEADER_SIZE+EXPAND_REPLY_TOT_STANDSTILL, time);
}

byte  
ExpandRouteReplyPacket::getLastSegmentsLeftStreetCount() const {
   return readByte( REPLY_HEADER_SIZE + 
      EXPAND_REPLY_LAST_LEFT_STREETCOUNT );
}

void  
ExpandRouteReplyPacket::setLastSegmentsLeftStreetCount(byte count) {
   writeByte( REPLY_HEADER_SIZE + 
      EXPAND_REPLY_LAST_LEFT_STREETCOUNT, count );
}

byte  
ExpandRouteReplyPacket::getLastSegmentsRightStreetCount() const {
   return readByte( REPLY_HEADER_SIZE + 
      EXPAND_REPLY_LAST_RIGHT_STREETCOUNT );
}

void  
ExpandRouteReplyPacket::setLastSegmentsRightStreetCount(byte count) {
   writeByte( REPLY_HEADER_SIZE + 
      EXPAND_REPLY_LAST_RIGHT_STREETCOUNT, count );
}

void  
ExpandRouteReplyPacket::setStartOffset( uint16 offset ){
   writeShort(REPLY_HEADER_SIZE+EXPAND_REPLY_START_OFFSET, offset );
}

uint16  
ExpandRouteReplyPacket::getStartOffset() const {
   return readShort(REPLY_HEADER_SIZE+EXPAND_REPLY_START_OFFSET);
}

void  
ExpandRouteReplyPacket::setEndOffset( uint16 offset ){
    writeShort(REPLY_HEADER_SIZE+EXPAND_REPLY_END_OFFSET, offset );
}

uint16  
ExpandRouteReplyPacket::getEndOffset() const {
    return readShort(REPLY_HEADER_SIZE+EXPAND_REPLY_END_OFFSET);
}

void 
ExpandRouteReplyPacket::setStartDirectionHousenumber(
                              ItemTypes::routedir_nbr_t x)
{
   writeByte(REPLY_HEADER_SIZE+EXPAND_REPLY_START_DIR_NBR, byte(x));
   MC2_ASSERT(getStartDirectionHousenumber() == x);
   mc2dbg2 << "ExpandRouteRepPack: wrote start-dir nbr: " << int(x) << endl;
}

ItemTypes::routedir_nbr_t 
ExpandRouteReplyPacket::getStartDirectionHousenumber() const
{
   return ItemTypes::routedir_nbr_t(
               readByte(REPLY_HEADER_SIZE+EXPAND_REPLY_START_DIR_NBR));
}

void 
ExpandRouteReplyPacket::setStartDirectionOddEven(
                              ItemTypes::routedir_oddeven_t x)
{
   writeByte(REPLY_HEADER_SIZE+EXPAND_REPLY_START_DIR_ODDEVEN, byte(x));
   MC2_ASSERT(getStartDirectionOddEven() == x);
   mc2dbg2 << "ExpandRouteRepPack: wrote odd/even: " << int(x) << endl;
}

   
ItemTypes::routedir_oddeven_t 
ExpandRouteReplyPacket::getStartDirectionOddEven() const
{
   return ItemTypes::routedir_oddeven_t(
               readByte(REPLY_HEADER_SIZE+EXPAND_REPLY_START_DIR_ODDEVEN));
}

void  
ExpandRouteReplyPacket::addItemsPerString(uint32 nbr){
   int32 pos = getLength();
   updateSize( 4 , MAX_PACKET_SIZE / 2);
    mc2dbg8 << "addItemsPerString " << nbr << ":" << pos << endl;
   writeLong( pos, nbr );
   setNbrItemsPerString( getNbrItemsPerString()+1 );
   setLength(pos + 4 );
}

uint32  
ExpandRouteReplyPacket::getItemsPerString( uint32 index ) const {
   return readLong( REPLY_HEADER_SIZE + EXPAND_REPLY_SUB_HEADER_SIZE +
                    getSizeStringData() + getSizeItemsData() + index*4);
}
















