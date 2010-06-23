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

#include "SearchMap2.h"

#include "SearchMapItem.h"

#include "SearchMapStringTable.h"
#include "SearchMapRegionTable.h"
#include "DataBuffer.h"
#include "IDTranslationTable.h"

#include "ItemTypes.h"

#include "UserRightsItemTable.h"
#include "UserRightsMapInfo.h"
#include "MapRights.h"

#include "Sort1st.h"

#include "DebugClock.h"
#include "MapBits.h"
#include "AlignUtility.h"

// -- Some utility classes

/**
 *   Comparator to use when searching for items using id:s.
 */
class IDComparator {
public:
   IDComparator(const SearchMap2* theMap) : m_map(theMap) {}
   
   bool operator()(const SearchMapItem& a, const IDPair_t& b) const {
      return a.getID(m_map) < b;
   }
private:
   const SearchMap2* m_map;
};

/**
 *   Comparator to use when searching for SearchMapPOIInfos
 *   using index.
 */
class POIIdxComparator {
public:
   POIIdxComparator(const SearchMap2* theMap) : m_map(theMap) {}

   bool operator()(const SearchMapPOIInfo& a, uint32 b) const {
      return a.getItemIdx(m_map) < b;
   }
private:
   const SearchMap2* m_map;
};


/**
 *   Comparator to use when searching for SearchMapRadiusInfos
 *   using index.
 */
class RadiusIdxComparator {
public:
   RadiusIdxComparator(const SearchMap2* theMap) : m_map(theMap) {}

   bool operator()(const SearchMapRadiusInfo& a, uint32 b) const {
      return a.getItemIdx(m_map) < b;
   }
private:
   const SearchMap2* m_map;
};

// -- SearchMap2

SearchMap2::SearchMap2()
{
   m_allItems           = NULL;
   m_allItemTypes       = NULL;
   m_nbrItems           = 0;
   m_POIInfos           = NULL;
   m_nbrPOIInfos        = 0;
   m_stringTable        = NULL;
   m_regionTable        = NULL;
   m_idTranslationTable = NULL;
   m_nbrRadiusInfos     = 0;
   m_radiusInfos        = NULL;
   m_nbrRightTables     = 0;
   m_rightTables        = NULL;
}

SearchMap2::SearchMap2(uint32 nbrAllItems,
                       SearchMapItem* allItems,
                       uint8* itemTypes,
                       uint32 nbrPOIInfos,
                       SearchMapPOIInfo* poiInfos,
                       SearchMapStringTable* stringTable,
                       SearchMapRegionTable* regionTable,
                       IDTranslationTable* idtranslationTable,
                       uint32 nbrRadiusInfos,
                       SearchMapRadiusInfo* radiusInfos,
                       StringTable::countryCode countryCode,
                       const set<LangTypes::language_t>& nativeLanguages,
                       map<uint32, UserRightsItemTable*>& rights )
      : m_allItems(allItems), m_allItemTypes(itemTypes),
        m_nbrItems(nbrAllItems), m_POIInfos(poiInfos),
        m_nbrPOIInfos(nbrPOIInfos), m_stringTable(stringTable),
        m_regionTable(regionTable),
        m_idTranslationTable(idtranslationTable),
        m_nbrRadiusInfos(nbrRadiusInfos),
        m_radiusInfos(radiusInfos),
        m_countryCode(countryCode),
        m_nativeLanguages(nativeLanguages)
{   
   m_nbrRightTables = rights.size(); // Probably mostly == 1
   m_rightTables = new pair<uint32, UserRightsItemTable*>[m_nbrRightTables];
   int idx = 0;
   for ( map<uint32, UserRightsItemTable*>::const_iterator it = rights.begin();
         it != rights.end();
         ++it ) {
      m_rightTables[idx++] = *it;
   }
   // Now we have stolen the stuff in rights
   rights.clear();
   // setup inside map
   createInsideMap();
}

void
SearchMap2::deleteAllData()
{
   delete [] m_allItems;
   delete [] m_allItemTypes;
   delete [] m_POIInfos;
   delete [] m_radiusInfos;
   for ( int i = 0; i < m_nbrRightTables; ++i ) {
      delete m_rightTables[i].second;
   }
   delete [] m_rightTables;

   delete m_stringTable;
   delete m_regionTable;
   delete m_idTranslationTable;

   // Set these too.
   m_nbrItems       = 0;
   m_nbrPOIInfos    = 0;
   m_nbrRadiusInfos = 0;
   m_nbrRightTables = 0;
   // clear inside map info
   m_insideMap.clear();
}

SearchMap2::~SearchMap2()
{
   deleteAllData();
}

int
SearchMap2::getSizeInDataBuffer() const
{
   uint32 categoriesSize = 0;
   for ( uint32 i = 0; i < m_nbrPOIInfos; ++i ) {
      categoriesSize += 4 + // nbr pois
         m_POIInfos[ i ].getCategories().size() * 2 +
         m_POIInfos[ i ].getSpecialImage().size() + 1; // +1 for \0
      AlignUtility::alignLong( categoriesSize );
   }

   int itemsAndSize = 4 + m_nbrItems * (8+8+4+4);
   int itemTypesSize =  AlignUtility::alignToLong(m_nbrItems);
   int poisAndSize  = 4 + m_nbrPOIInfos * (4+4+4);

   int radiusSize = 4 + m_nbrRadiusInfos * (4+4);
   int languageSize = 4 + 4 * m_nativeLanguages.size();

   int userRightsSize = 4;
   for ( int i = 0; i < m_nbrRightTables; ++i ) {
      userRightsSize += 4 + m_rightTables[i].second->getSizeInDataBuffer();
   }
   return 4 + itemsAndSize + itemTypesSize + categoriesSize +
      + poisAndSize
      + radiusSize
      + m_stringTable->getSizeInDataBuffer()
      + m_regionTable->getSizeInDataBuffer()
      + m_idTranslationTable->getSizeInDataBuffer()
      + 4 // country code
      + languageSize 
      + userRightsSize
      + getInsideMapSizeInDataBuffer();
      
}

int
SearchMap2::save(DataBuffer& buf) const
{
   mc2log << "SearchMap2 save." << endl;
   DataBufferChecker dbc(buf, "SearchMap2::save");
    
   // Write the version
   buf.writeNextLong(0);
   
   // Write the strings
   m_stringTable->save(buf);
   // Write the regions
   m_regionTable->save(buf);
   // Write the idtranslation
   m_idTranslationTable->save(buf);
   
   // Write the items - puha!
   buf.writeNextLong(m_nbrItems);
   mc2dbg << "[SMap]: nbritems = " << m_nbrItems << endl;
   for ( uint32 i = 0; i < m_nbrItems; ++i ) {
      buf.writeNextLong(m_allItems[i].m_id.getMapID());
      buf.writeNextLong(m_allItems[i].m_id.getItemID());
      buf.writeNextLong(m_allItems[i].m_coord.lat);
      buf.writeNextLong(m_allItems[i].m_coord.lon);
      buf.writeNextLong(m_allItems[i].m_strIdx);
      buf.writeNextLong(m_allItems[i].m_regionIdx);
   }

   // Write the one byte types.
   buf.writeNextByteArray(m_allItemTypes, m_nbrItems);
   
   // And align
   buf.alignToLongAndClear();

   // Write the number of pois and the poi infos.
   buf.writeNextLong(m_nbrPOIInfos);
   mc2dbg << "[SMap]: nbrPOI = " << m_nbrPOIInfos << endl;
   // Write the SearchMapPOIInfos
   for ( uint32 i = 0; i < m_nbrPOIInfos; ++i ) {
      buf.writeNextLong(m_POIInfos[i].m_itemIdx);
      buf.writeNextLong(m_POIInfos[i].m_addressIdx);
      buf.writeNextShort(m_POIInfos[i].m_trueNumberOnStreet);
      buf.writeNextShort(m_POIInfos[i].m_subType);

      // write categories
      buf.writeNextLong( m_POIInfos[ i ].m_categories.size() );

      typedef SearchMapPOIInfo::Categories::const_iterator CategoryIt;
      CategoryIt catIt = m_POIInfos[ i ].m_categories.begin();
      CategoryIt catItEnd = m_POIInfos[ i ].m_categories.end();
      for ( ; catIt != catItEnd; ++catIt ) {
         buf.writeNextShort( *catIt );
      }
      buf.writeNextString( m_POIInfos[ i ].m_specialImage.c_str() );
      buf.alignToLong();
   }

   // Write the radius infos
   buf.writeNextLong(m_nbrRadiusInfos);
   for ( uint32 i = 0; i < m_nbrRadiusInfos; ++i ) {
      buf.writeNextLong(m_radiusInfos[i].getItemIdx(this));
      buf.writeNextLong(m_radiusInfos[i].getRadiusMeters(this));
   }

   // Country code
   buf.writeNextLong(m_countryCode);

   // Native languages
   uint32 nbrLangs = 0;
   if (m_nativeLanguages.empty()) {
      buf.writeNextLong(nbrLangs);
   } else {
      buf.writeNextLong(m_nativeLanguages.size());
      set<LangTypes::language_t>::iterator it;
      for ( it = m_nativeLanguages.begin();
            it != m_nativeLanguages.end(); it++ ) {
         buf.writeNextLong(*it);
      }
   }

   // User rights
   buf.writeNextLong( m_nbrRightTables );      
   for ( int i = 0; i < m_nbrRightTables; ++i ) {
      buf.writeNextLong( m_rightTables[i].first ); // MapID
      m_rightTables[i].second->save( buf );
   }

   // Inside map
   mc2dbg << "Writing inside map. " << endl;
   DebugClock insideMapClock;

   insideMap_t::const_iterator mIt = m_insideMap.begin();
   insideMap_t::const_iterator mItEnd = m_insideMap.end();
   // first write size so we know how much to expect at ::load
   buf.writeNextLong( m_insideMap.size() );
   // Make lookup map to avoid too much scaning of all items
   typedef map< const SearchMapItem*, uint32 > insideLookup_t;
   insideLookup_t insideLookup;
   if ( mIt != mItEnd ) {
      for ( uint32 itemIndex = 0 ; itemIndex < m_nbrItems; ++itemIndex ) {
         insideLookup.insert( make_pair( &m_allItems[ itemIndex ] , 
                                         itemIndex ) );
      }
   }
   for ( ; mIt != mItEnd; ++mIt ) {
      // find item index in m_allItems by comparing pointer
      uint32 itemIndex = insideLookup.find( (*mIt).second )->second;
      // must be valid, else something is very wrong.
      MC2_ASSERT( itemIndex < m_nbrItems );

      // save as itemID + m_allItems index
      buf.writeNextLong( (*mIt).first );
      buf.writeNextLong( itemIndex );
   }
   mc2dbg << "Done Writing inside map in " << insideMapClock << endl;

   // Done   
   dbc.assertPosition(getSizeInDataBuffer());
   return getSizeInDataBuffer();
}


int
SearchMap2::load(DataBuffer& buf)
{
   mc2dbg << "SearchMap2 load." << endl;
   DataBufferChecker dbc(buf, "SearchMap2::load");
   
   // Read the version
   int version = buf.readNextLong();
   MC2_ASSERT( version == 0 ); version |= 0;

   // Remove old data, if any.
   deleteAllData();

   // The stringTable
   m_stringTable = new SearchMapStringTable;   
   m_stringTable->load(buf);
     
   m_regionTable = new SearchMapRegionTable;
   m_regionTable->load(buf);
   // The region table
   mc2dbg << "[SMap]::load: Size of regiontable in buffer : "
          << m_regionTable->getSizeInDataBuffer() << endl;

   m_idTranslationTable = new IDTranslationTable;
   m_idTranslationTable->load(buf);
   
   // The items
   m_nbrItems = buf.readNextLong();
   mc2dbg << "[SMap]: nbritems = " << m_nbrItems << endl;
   m_allItems = new SearchMapItem[m_nbrItems];
   // Read the items - puha!
   for ( uint32 i = 0; i < m_nbrItems; ++i ) {
      m_allItems[i].m_id.first   = buf.readNextLong();
      m_allItems[i].m_id.second  = buf.readNextLong();
      m_allItems[i].m_coord.lat  = buf.readNextLong();
      m_allItems[i].m_coord.lon  = buf.readNextLong();
      m_allItems[i].m_strIdx     = buf.readNextLong();
      m_allItems[i].m_regionIdx  = buf.readNextLong();
   }
   
   // Read the itemtypes
   m_allItemTypes = new byte[m_nbrItems];
   buf.readNextByteArray(m_allItemTypes, m_nbrItems);
   buf.alignToLong();

   // The point of interests
   // Read the number of pois and the poi infos.
   m_nbrPOIInfos = buf.readNextLong();
   m_POIInfos = new SearchMapPOIInfo[m_nbrPOIInfos];
   
   // Read the poi infos.
   for ( uint32 i = 0; i < m_nbrPOIInfos; ++i ) {
      m_POIInfos[i].m_itemIdx            = buf.readNextLong();
      m_POIInfos[i].m_addressIdx         = buf.readNextLong();
      m_POIInfos[i].m_trueNumberOnStreet = buf.readNextShort();
      m_POIInfos[i].m_subType            = buf.readNextShort();
      // load categories and create ( category id -> index ) map
      uint16 nbrCategories = buf.readNextLong();
      for ( uint32 cat = 0; cat < nbrCategories; ++cat ) {
         SearchMapPOIInfo::CategoryID category = buf.readNextShort();
         m_POIInfos[ i ].m_categories.insert( category );
         m_poiInfoMap.push_back( make_pair( category, i ) );
      }
      m_POIInfos[ i ].m_specialImage = buf.readNextString();
      buf.alignToLong();
   }
   // create category id to poi info map
   std::sort( m_poiInfoMap.begin(), m_poiInfoMap.end(),
              STLUtility::makeSort1st( m_poiInfoMap ) );

   // Read the number of radius infos
   m_nbrRadiusInfos = buf.readNextLong();
   mc2dbg << "[SMap]: nbrRadii = " << m_nbrRadiusInfos << endl;
   m_radiusInfos = new SearchMapRadiusInfo[m_nbrRadiusInfos];
   for ( uint32 i = 0; i < m_nbrRadiusInfos; ++i ) {
      const uint32 itemIdx = buf.readNextLong();
      const uint32 radius  = buf.readNextLong();
      m_radiusInfos[i].setAll(itemIdx, radius);
   }

   // Country code
   m_countryCode = StringTable::countryCode(buf.readNextLong());
   
   // Native languages
   uint32 nbrLangs = buf.readNextLong();
   for ( uint32 i = 0; i < nbrLangs; i++ ) {
      //LangTypes::language_t lang = buf.readNextLong();
      //m_nativeLanguages.insert(lang);
      m_nativeLanguages.insert( LangTypes::language_t(buf.readNextLong()) );
   }

   // The user rights
   buf.readNextLong( m_nbrRightTables );      
   m_rightTables = new rightTablesEntry_t[ m_nbrRightTables ];
   for ( int i = 0; i < m_nbrRightTables; ++i ) {
      buf.readNextLong( m_rightTables[i].first ); // MapID
      m_rightTables[i].second = new UserRightsItemTable( ~MapRights() );
      m_rightTables[i].second->load( buf );
   }
   
   // Inside map

   mc2log << "[SearchMap2] Reading inside map. " << endl;
   m_insideMap.clear();
   const uint32 insideMapSize = buf.readNextLong();
   m_insideMap.reserve( insideMapSize );
   for ( uint32 i = 0;  i < insideMapSize; ++i ) {
      uint32 id = buf.readNextLong();
      uint32 itemIndex = buf.readNextLong();
      // must be in range at save!
      MC2_ASSERT( itemIndex < m_nbrItems );
      m_insideMap.push_back( make_pair( id, &m_allItems[ itemIndex ] ) );
   }
   const uint32 dataReadSize = getSizeInDataBuffer();
   dbc.assertPosition( dataReadSize );
   
   return dataReadSize;
}

uint32 
SearchMap2::getPOIInfoForCategories( const set<uint16>& categories,
                                     vector<const SearchMapPOIInfo*>& 
                                     poiInfos ) const {
   uint16 nbrPois = 0;
   // find range of matching pois
   set<uint16>::const_iterator catIt = categories.begin();
   set<uint16>::const_iterator catItEnd = categories.end();
   for ( ; catIt != catItEnd; ++catIt ) {
      typedef const poiInfoMap_t::value_type* RangeType;
      std::pair< RangeType, RangeType > 
         range = std::equal_range( &m_poiInfoMap[0],
                                   &m_poiInfoMap[ m_poiInfoMap.size() ],
                                   *catIt,
                                   STLUtility::makeSort1st( m_poiInfoMap ) );
      // reserve space
      nbrPois += std::distance( range.first, range.second );
      poiInfos.reserve( nbrPois + poiInfos.size() );
      // add all matching poi info pointers
      for ( const poiInfoMap_t::value_type* it = range.first; 
            it != range.second; ++it ) {
         poiInfos.push_back( &m_POIInfos[ it->second ] );
      }
   }

   return nbrPois;
}

IDPair_t
SearchMap2::translateToHigher(const IDPair_t& lower) const
{
   // WARNING! The mapID of the overview map is
   //          lost, really.
   
   uint32 itemID = m_idTranslationTable->translateToHigher(lower);
   if ( itemID == MAX_UINT32 ) {
      return IDPair_t(MAX_UINT32, MAX_UINT32);
   } else {
      uint32 mapID = m_allItems[0].m_id.getMapID();
      return IDPair_t(mapID, itemID);
   }

}

IDPair_t
SearchMap2::translateToLower(const IDPair_t& higher) const
{
   // WARNING! The map id is lost here
   return m_idTranslationTable->translateToLower(higher.getItemID());
}

const SearchMapItem*
SearchMap2::getItemByItemID(const IDPair_t& id) const
{
   const SearchMapItem* foundItem = std::lower_bound(begin(),
                                                     end(),
                                                     id,
                                                     IDComparator(this));
   
   if ( foundItem != end() && foundItem->getID(this) == id ) {
      return foundItem;
   } else {
      return NULL;
   }         
}

const SearchMapPOIInfo*
SearchMap2::getPOIInfoFor(const SearchMapItem* item) const
{
   const SearchMapPOIInfo* begin = &m_POIInfos[0];
   const SearchMapPOIInfo* end   = &m_POIInfos[m_nbrPOIInfos];
   
   const SearchMapPOIInfo* foundInfo =
      std::lower_bound(begin,
                       end,
                       item->getIndex(this),
                       POIIdxComparator(this));
   if ( foundInfo != end &&
        foundInfo->getItemIdx(this) == item->getIndex(this) ) {
      return foundInfo;
   } else {
      return NULL;
   }
}

const SearchMapRadiusInfo*
SearchMap2::getRadiusInfo(const SearchMapItem* item) const
{
   const SearchMapRadiusInfo* begin = &m_radiusInfos[0];
   const SearchMapRadiusInfo* end   = &m_radiusInfos[m_nbrRadiusInfos];

   const SearchMapRadiusInfo* foundInfo =
      std::lower_bound(begin,
                       end,
                       item->getIndex(this),
                       RadiusIdxComparator(this));
   if ( foundInfo != end &&
        foundInfo->getItemIdx(this) == item->getIndex(this) ) {
      return foundInfo;
   } else {
      return NULL;
   }
}

uint32
SearchMap2::getMapID() const
{
   return m_allItems[0].m_id.getMapID();
}

bool
SearchMap2::itemShouldBeFound(uint32 idx) const
{
   const SearchMapItem* curItem = &m_allItems[idx];
#if 0
   // No-one can remember why it had to be exactly one region.
   // Since the maps on the production cluster did not meet
   // that criterion it had to be removed here.
   // It must be part of exactly one region to be a multimap item.   
   if ( curItem->getNbrRegions(this) != 1 ) {
      return true;
   }
#endif

   // Here it used to be: If the only region in the item has
   // the same name and the same type -> should not be displayed

   // Changed to: If one of the regions has the same type and
   // name -> should not be displayed.
   
   for( SearchMapRegionIterator it = curItem->getRegionBegin(this);
        it != curItem->getRegionEnd(this);
        ++it ) {
      // Check if the searchable is in a region with the same type
      // as the searchable.
      const SearchMapItem* region = getItemByIndex(*it);
      if ( curItem->getItemType(this) == region->getItemType(this) ) {
         // It may be part of a multimap item
         if ( curItem->sameNames(this, region) ) {
            // It has the same names to. We do not want to find it.
            return false;
         }
      }
   }
   return true;
}

typedef pair<uint32, const SearchMapItem* > InsideMapValueType;
struct InsideMapComp: 
   public binary_function< InsideMapValueType, InsideMapValueType, bool > {
   
   bool operator ()( const InsideMapValueType& a, uint32 b ) {
      return a.first < b;
   }
};

int
SearchMap2::expandIDs( vector<pair<uint32, IDPair_t> >& expandedIDs,
                       const vector<pair<uint32, IDPair_t> >&
                       idsToExpand,
                       bool expand) const
{
   if ( expand ) {
      // EXPAND
      for( uint32 i = 0; i < idsToExpand.size(); ++i ) {
         IDPair_t curItemToExpand(idsToExpand[i].second);

         // If the item is already on low level we don't have
         // to expand it.
         if ( MapBits::isUnderviewMap(curItemToExpand.getMapID()) ) {
            // Just put it back
            expandedIDs.push_back(make_pair(idsToExpand[i].first,
                                            curItemToExpand));
            // Next, please
            continue;
         }
         // If it is possible to translate the id into a lower map id
         // we do not have to expand it either
         IDPair_t curItemOnLowLevel( translateToLower(curItemToExpand));
         if ( curItemOnLowLevel.isValid() ) {
            // Add it to the packet.
            expandedIDs.push_back( make_pair( idsToExpand[i].first, // idx
                                              curItemOnLowLevel ) );
            continue;
         } 
         
         // All the items should have the same id as the map.
         // (packet fixes this).
         // uint32 mapID = curItemToExpand.getMapID();
        

          // Insert the items on low level
         // that are inside our region to expand.
         uint32 curItemID = curItemToExpand.getItemID();
         insideMap_t::const_iterator it = lower_bound( m_insideMap.begin(),
                                                       m_insideMap.end(),
                                                       curItemID,
                                                       InsideMapComp() );
         insideMap_t::const_iterator itEnd = m_insideMap.end();
         for (; it != itEnd && it->first == curItemID; ++it ) {
            IDPair_t id( curItemToExpand.getMapID(),
                         it->second->getID(this).getItemID() );
            // only add those that are translatable to lower level
            if ( translateToLower( id ).isValid() ) { 
               expandedIDs.
                  push_back( make_pair( idsToExpand[i].first, // idx
                                        translateToLower( id ) ) );
            }
            
         } 
         
      }
   } else {
      // UNEXPAND
      // Unexpand. This differs from expand because it only unexpands
      // the regions that exists both in the underview map and the
      // overview map.
      for( uint32 i = 0; i < idsToExpand.size(); ++i ) {
         IDPair_t curID( translateToHigher(idsToExpand[i].second) );
         if ( !curID.isValid() ) {
            // Not found. Cannot be expanded.
            continue;
         }
         
         const SearchMapItem* curItem =
            getItemByItemID(curID);
         if ( curItem->shouldBeFound(this) ) {
            // Should not be expanded.
            continue;
         } else {
            // Get the group for the item.
            // Should be there if the item should not be found.
            IDPair_t groupID =
               getItemByIndex(*curItem->getRegionBegin(this))->getID(this);
            // Put the group id in the result.
            expandedIDs.push_back(
               make_pair(idsToExpand[i].first, // idx
                         groupID));
         }         
      }      
   }
   return expandedIDs.size();
}

bool 
SearchMap2::hasRights( const SearchMapItem* item,
                       const MapRights& right) const {

   // get the rights for the item
   MapRights itemRights = getRights( item );

   // only use the first 32 bits;
   // the rights in the item uses 32 bits of 1's ( 0xFF...)
   // to show it has no rights. or all 64 bits are 1's,
   // ( not sure why some items have only 32bits set to 1's,
   //   but it will be fixed in the map data for next release,
   //  2007-05-something. )
   // So we do this ugly hack for now.
   MapRights noRights( static_cast<MapRights::Masks>( MAX_UINT32) ); 
   MapRights fullNoRights = ~MapRights();

   // check for no rights
   if ( itemRights == fullNoRights ||
        itemRights == noRights ) {
      return false;
   }

   // do bitwise AND to see if "itemRights" has "right"
   itemRights &= right;
   if ( itemRights ) {
      return true;
   }

   return false;
}


MapRights
SearchMap2::getRights( const SearchMapItem* item ) const {
   // Find the correct rights table
   // TODO: do search in m_rightTables for map id
   // ( the map ids are ordered. )
   UserRightsItemTable* rights = m_rightTables[ 0 ].second;

   if ( rights == NULL ) {
      return ~MapRights();
   }

   const IDPair_t& id = item->getID( this );   
   MapRights itemRights = rights->getRights( id.getItemID() );

   return itemRights;
}


bool
SearchMap2::itemAllowedByUserRights( const SearchMapItem* item,
                                     const UserRightsMapInfo& rights ) const
{
   const IDPair_t& id = item->getID(this);
   // FIXME: Not number 0
   return rights.itemAllowed( m_rightTables[0].second->getRights(id.second),
                              id,
                              *m_idTranslationTable );
}

void
SearchMap2::getAllRegionsFor(const SearchMapItem* item,
                             set<const SearchMapItem*>& regions) const
{
   for( SearchMapRegionIterator it = item->getRegionBegin(this);
        it != item->getRegionEnd(this);
        ++it ) {
      // Add our region
      regions.insert( &m_allItems[*it] );
      // Also get the regions for the region
      getAllRegionsFor( &m_allItems[*it], regions );
   }
}

void SearchMap2::createInsideMap() {
   typedef multimap< uint32, const SearchMapItem*> InsideMap;
   InsideMap insideMap;

   mc2dbg << "[SMap]: Scanning whole map" << endl;
   // Create the insideMap
   // Go through the map and check the items that exist on 
   // the underview map.

   for( const_iterator it = begin(); it != end(); ++it ) {
      const SearchMapItem* curItem = it;
      // Add the items that exist on the underview maps
      // but are inside items that are not on the underview
      // map.
      IDPair_t lowerID( translateToLower( curItem->getID( this ) ) );
      
      if ( lowerID.isValid() ) {
         // Low id valid. We must check.
         set<const SearchMapItem*> allRegions;
         getAllRegionsFor( curItem, allRegions );
         for( set<const SearchMapItem*>::const_iterator it =
                 allRegions.begin();
              it != allRegions.end();
              ++it ) {
            // If this item is inside a region that is not
            // possible to translate to lower level, insert
            // the id of the region together with the item.
            const IDPair_t regID ( (*it)->getID( this ) );
            if ( ! translateToLower( regID ).isValid() ) {
               insideMap.insert( make_pair( regID.getItemID(),
                                            curItem ) );
            }
         }
      }

      /* Note: 
         This is actually the correct way, 
         but it is terribly slow.
      */
      /*
      const SearchMapItem* curItem = it;
      // Add the items that exist on the underview maps
      // but are inside items that are not on the underview
      // map.
      IDPair_t lowerID(translateToLower(curItem->getID(this)));

      if ( lowerID.isValid() ) {
         continue;  
      }
      
      for ( const_iterator nIt = begin() ; nIt != end(); ++nIt ) {
         const SearchMapItem* nItem = nIt;
         if ( nIt == it ||
              nItem->getItemType( this ) != curItem->getItemType( this ) ) {
            continue;
         }

         SearchMapRegionIterator rIt = nItem->getRegionBegin( this );
         SearchMapRegionIterator rItEnd = nItem->getRegionEnd( this );
         for (; rIt != rItEnd; ++rIt ) {
            if ( (*rIt) == curItem->getIndex( this ) ){
               insideMap.insert( make_pair( curItem->getID( this ).getItemID(),
                                            nItem ) );
               break;
            }
         }
      }
      */            
   }
   m_insideMap.clear();
   m_insideMap.reserve( insideMap.size() );

   InsideMap::const_iterator it = insideMap.begin();
   InsideMap::const_iterator itEnd = insideMap.end();
   for ( uint32 i = 0; it != itEnd; ++it, ++i ) {
      m_insideMap.push_back( make_pair( (*it).first, (*it).second ) );
   }

   mc2dbg << "[SMap]: Whole map scanned" << endl;
}

uint32
SearchMap2::getInsideMapSizeInDataBuffer() const { 
   // length + data size for inside map
   return sizeof(uint32) + m_insideMap.size() * 2 * sizeof(uint32);
}
