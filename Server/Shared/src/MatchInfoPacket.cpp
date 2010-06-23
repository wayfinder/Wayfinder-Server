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

#include <map>

#include "MatchInfoPacket.h"
#include "SearchMatch.h"

int
MatchInfoRequestPacket::
calcPacketSize(const vector<VanillaMatch*>& matches)
{
   // This is often too big, but since it is only used
   // for a short while it is probably ok. (Not all matches
   // are streetmatches).
   return ITEMS_START_POS + matches.size() * 8 + 4;
}

MatchInfoRequestPacket::
MatchInfoRequestPacket(const vector<VanillaMatch*>& matches,
                       uint32 regionTypes,
                       LangTypes::language_t reqLang,
                       uint32 mapID,
                       uint32 packetID,
                       uint32 reqID,
                       uint32 userData)
      : RequestPacket(  calcPacketSize(matches),
                        DEFAULT_PACKET_PRIO,
                        Packet::PACKETTYPE_MATCHINFOREQUEST,
                        packetID,
                        reqID,
                        mapID)
                       
{
   int pos = USER_DEF_POS;
   incWriteLong( pos, userData    );
   incWriteLong( pos, reqLang     ); // Language
   incWriteLong( pos, regionTypes );
   incWriteLong( pos, 0 ); // Don't know yet. (NBR_ITEMS)
   int nbrAdded = 0;

   pos = ITEMS_START_POS;
   for ( uint32 i = 0; i < matches.size(); ++i ) {
      if ( matches[i]->getMapID() == mapID ) {
         VanillaStreetMatch* curMatch =
            dynamic_cast<VanillaStreetMatch*>(matches[i]);         
         if ( curMatch && curMatch->getStreetNbr() != 0 ) {
            incWriteLong(pos, curMatch->getItemID());
            incWriteLong(pos, curMatch->getStreetNbr());
            nbrAdded++;
         }
      }
   }
   writeLong(NBR_ITEMS_POS, nbrAdded);
   setLength(pos);
}

int
MatchInfoRequestPacket::
getStreetMatches(vector<VanillaStreetMatch*>& streets) const
{
   int origSize = streets.size();
   
   uint32 mapID = getMapID();
   int nbrMatches = getNbrItems();
   
   int pos = ITEMS_START_POS;
   
   for ( int i = 0; i < nbrMatches; ++i ) {
      uint32 itemID = incReadLong(pos);
      uint32 streetNbr = incReadLong(pos);
      
      VanillaStreetMatch* match =
         new VanillaStreetMatch(IDPair_t( mapID, itemID ), "", "", 0,
                                streetNbr, false, false );
      streets.push_back(match);
   }
   return streets.size() - origSize;
}

uint32
MatchInfoRequestPacket::getUserData() const
{
   return readLong(USER_DEF_POS);
}

uint32
MatchInfoRequestPacket::getNbrItems() const
{
   return readLong(NBR_ITEMS_POS);
}

LangTypes::language_t
MatchInfoRequestPacket::getRequestedLanguage() const
{
   return LangTypes::language_t(readLong(LANGUAGE_POS));
}

uint32
MatchInfoRequestPacket::getRegionSearchTypes() const
{
   return readLong(REGION_TYPES_POS);
}

//-----------------------------------------------------------------
// MatchInfoReplyPacket
//-----------------------------------------------------------------

int
MatchInfoReplyPacket::
calcPacketSize(const vector<VanillaStreetMatch*>& infos)
{
   // Only estimate. The add-functions will alloc more if
   // needed.
   return ITEMS_START_POS + infos.size() * (6 * 4 + 40);
}


uint32
MatchInfoReplyPacket::getNbrItems() const
{
   return readLong(NBR_ITEMS_POS);
}

uint32
MatchInfoReplyPacket::getMapID() const
{
   return readLong(MAP_ID_POS);
}

inline void
MatchInfoReplyPacket::incWriteRegion(int& pos,
                                     const SearchMatch* region)
{
   mc2dbg8 << "[MIP]: addRegion - pos = " << pos << endl;

   // Double the size if there isn't room for 1024
   setLength(pos);
   if ( updateSize( 1024, 1024+getBufSize()) ) {
      mc2dbg8 << "[MIP]: Resized packet to " << getBufSize()
              << " bytes" << endl;
   }
   
   incWriteLong(pos, region->getItemID());
   incWriteLong(pos, region->getType());
   incWriteLong(pos, region->getItemType());
   incWriteString(pos, region->getName());
   incWriteString(pos, region->getAlphaSortingName());
   
   // Write the number of regions and the regions
   incWriteLong( pos, region->getNbrRegions());
   for ( int i = 0; i < int(region->getNbrRegions()); ++i ) {
      incWriteRegion(pos, region->getRegion(i));
   }
}


inline VanillaRegionMatch*
MatchInfoReplyPacket::incReadRegion(int& pos) const
{
   uint32 itemID                = incReadLong(pos);
   uint32 searchType            = incReadLong(pos);
   ItemTypes::itemType itemType = ItemTypes::itemType(incReadLong(pos));
   char* name;
   incReadString(pos, name);
   char* alphaSortingName;
   incReadString(pos, alphaSortingName);
   
   VanillaRegionMatch* regionMatch =
      new VanillaRegionMatch(name,
                             0, // nameinfo
                             getMapID(),
                             itemID,
                             searchType,
                             SearchMatchPoints(),
                             0, // source
                             alphaSortingName,
                             0,
                             0,
                             itemType, 0);
   // Read the regions
   int nbrRegions = incReadLong(pos);
   for ( int i = 0; i < nbrRegions; ++i ) {
      regionMatch->addRegion(incReadRegion(pos), true);
   }   
   return regionMatch;
}


inline void
MatchInfoReplyPacket::incWriteStreet(int& pos,
                                     const VanillaStreetMatch* street)
{
   // Double the size if there isn't room for 1024 more bytes.
   setLength(pos);
   if ( updateSize( 1024, 1024+getBufSize()) ) {
      mc2dbg2 << "[MIP]: Resized packet to " << getBufSize()
              << " bytes" << endl;
   }
   
   incWriteLong( pos, street->getItemID());
   incWriteLong( pos, street->getStreetSegmentID());
   incWriteLong( pos, street->getCoords().lat);
   incWriteLong( pos, street->getCoords().lon);
   incWriteLong( pos, street->getStreetNbr());
   incWriteLong( pos, street->getOffset());

   if( street->getStreetNumberFirst() )
      incWriteByte(pos, 1);
   else
      incWriteByte(pos, 0);
   if( street->getStreetNumberComma() )
      incWriteByte(pos, 1);
   else
      incWriteByte(pos, 0);

   // Write the number of regions and the regions
   incWriteLong( pos, street->getNbrRegions());
   for ( int i = 0; i < int(street->getNbrRegions()); ++i ) {
      incWriteRegion(pos, street->getRegion(i));
   }
   incWriteString(pos, street->getLocationName());
}
   
inline VanillaStreetMatch*
MatchInfoReplyPacket::incReadStreet(int& pos) const
{
   uint32 itemID   = incReadLong(pos);
   uint32 ssiID    = incReadLong(pos);
   MC2Coordinate coord;
   coord.lat       = incReadLong(pos);
   coord.lon       = incReadLong(pos);
   uint32 housenbr = incReadLong(pos);
   uint32 offset   = incReadLong(pos);

   byte n;
   bool streetNumberFirst, streetNumberComma;
   n = incReadByte(pos);
   if( n == 0 )
      streetNumberFirst = false;
   else
      streetNumberFirst = true;
   n = incReadByte(pos);
   if (n == 0 )
      streetNumberComma = false;
   else
      streetNumberComma = true;
   
   VanillaStreetMatch* street =
      new VanillaStreetMatch(IDPair_t( getMapID(), itemID ), "", "", offset,
                             housenbr, streetNumberFirst, streetNumberComma );
   street->setStreetSegmentID(ssiID);
   street->setCoords(coord);
   
   // Read the regions
   int nbrRegions = incReadLong(pos);
   for ( int i = 0; i < nbrRegions; ++i ) {
      street->addRegion(incReadRegion(pos), true);
   }
   char* locatioName;
   incReadString(pos, locatioName);
   mc2dbg8 << "[MIRP]: Set location name to " << locatioName << endl;
   street->setLocationName(locatioName);

   return street;
}

MatchInfoReplyPacket::
MatchInfoReplyPacket(const MatchInfoRequestPacket* req,
                     const vector<VanillaStreetMatch*>& infos)
      : ReplyPacket( calcPacketSize(infos),
                     Packet::PACKETTYPE_MATCHINFOREPLY,
                     req,
                     StringTable::OK)
{
   int pos = MAP_ID_POS;
   incWriteLong(pos, req->getMapID());
   incWriteLong(pos, req->getUserData());
   incWriteLong(pos, infos.size()); // NbrItems

   mc2dbg2 << "[MIRP]: NbrMatches = " << getNbrItems() << endl;
   
   int nbrItems = infos.size();
   pos = ITEMS_START_POS;
   for ( int i = 0; i < nbrItems; ++i ) {
      incWriteStreet(pos, infos[i]);
   }
   
   setLength(pos);
}

int
MatchInfoReplyPacket::fixupMatches(vector<VanillaMatch*>& oldMatches) const
{
   // Start by reading all the matches in the packet
   map<IDPair_t, VanillaStreetMatch*> packetStreets;
   
   int nbrMatches = getNbrItems();
   int pos = ITEMS_START_POS;
   for ( int i = 0; i < nbrMatches; ++i ) {
      VanillaStreetMatch* street = incReadStreet(pos);
      packetStreets.insert(make_pair(street->getID(), street));
   }
   
   if ( packetStreets.empty() ) {
      return 0;
   }
   
   int nbrFixes = 0;
   int nbrOldMatches = oldMatches.size();
   for ( int i = 0; i < nbrOldMatches; ++i ) {
      VanillaStreetMatch* curMatch =
         dynamic_cast<VanillaStreetMatch*>(oldMatches[i]);
      if ( curMatch == NULL ) {
         // Not a street
         continue;
      }
      
      map<IDPair_t, VanillaStreetMatch*>::const_iterator it =
         packetStreets.find(curMatch->getID());
      if ( it == packetStreets.end() ) {
         // Not found
         continue;
      }

      mc2dbg2 << "[MIRP]: Changing match " << curMatch->getName() << endl;
      // Move some of the information in the packet-match to the
      // old match.
      const VanillaStreetMatch* packetMatch = it->second;
      
      curMatch->setStreetSegmentID(packetMatch->getStreetSegmentID());
      uint32 searchedStreetNbr = curMatch->getStreetNbr();
      curMatch->setStreetNbr(packetMatch->getStreetNbr());
      curMatch->setOffset(packetMatch->getOffset());
      // Set HouseNbrDiff in SearchMatchPoints
      curMatch->getPointsForWriting().setHouseNbrDiff( 
         searchedStreetNbr - packetMatch->getStreetNbr() );

      // Do tricks
      if ( curMatch->getStreetSegmentID() != MAX_UINT32 ) {
         curMatch->setItemID( curMatch->getStreetSegmentID() );
         curMatch->setItemType( ItemTypes::streetSegmentItem );
      }

      if( packetMatch->getStreetNbr() != 0 ) {
         char str[1024];
         if( curMatch->getStreetNumberFirst() ) {
            if( curMatch->getStreetNumberComma() ) {
               sprintf(str, "%d, %s",
                       curMatch->getStreetNbr(),
                       curMatch->getName());            
            } else {
               sprintf(str, "%d %s",
                       curMatch->getStreetNbr(),
                       curMatch->getName());
            }
         } else {
            if( curMatch->getStreetNumberComma() ) {
               sprintf(str, "%s, %d",
                       curMatch->getName(),
                       curMatch->getStreetNbr());
            } else {
               sprintf(str, "%s %d",
                       curMatch->getName(),
                       curMatch->getStreetNbr());
            }
         }
         curMatch->setName(str);
      }
      
      // Also set the regions....
      if ( packetMatch->getStreetSegmentID() != MAX_UINT32 ) {
         curMatch->setRegions(*packetMatch);
         curMatch->setLocationName(packetMatch->getLocationName());
         curMatch->setCoords(packetMatch->getCoords());
      }
   }
   
   // Clean up the temporary data.
   for ( map<IDPair_t,VanillaStreetMatch*>::iterator it(packetStreets.begin());
         it != packetStreets.end();
         ++it) {
      delete it->second;
   }
   return nbrFixes;
}
