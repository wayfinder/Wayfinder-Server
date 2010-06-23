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

#include "SearchUnitBuilder.h"

#include "Item.h"
#include "PointOfInterestItem.h"
#include "StreetSegmentItem.h"
#include "StreetItem.h"

#include "MC2Coordinate.h"
#include "GenericMap.h"
#include "OverviewMap.h"

#include "SearchMapStringTable.h"
#include "SearchMapRegionTable.h"
#include "SearchMapItem.h"
#include "SearchMap2.h"
#include "SearchUnit.h"

#include "WriteableSearchUnit.h"
#include "StringSearchUtility.h"
#include "Properties.h"
#include "UserRightsItemTable.h"

#include "POIInfo.h"

#include "GfxFeatureMapUtility.h"

#include "TimeUtility.h"
#include "MapBits.h"
#include "NodeBits.h"

#if defined (__GNUC__) && __GNUC__ > 2
#include <ext/slist>
#include <ext/algorithm>
using namespace __gnu_cxx;
#else
#include <slist>
#include <algorithm>
using namespace std;
#endif

// -- The first lot of functions here should be used for adding
//    one GenericMap to the SearchMap.
//    Further down are the functions that create the SearchStructs.

// -- Functions that should be inlined in the TemporarySearchMapItems

inline uint32
SearchUnitBuilder::getTemporaryStringIndex(const char* aString)
{
   // Insert/find the string
   pair<stringMap_t::iterator, bool> res(
       m_strings.insert(make_pair(aString, m_strings.size() ) ));
   
   return res.first->second;
}

// -- TemporarySearchMapItem

/**
 *   SearchMapItem temporarily used when creating the
 *   SearchMap from a GenericMap.
 */
class TemporarySearchMapItem {
public:   
   /**
    *   Constructor for convenience.
    */
   inline TemporarySearchMapItem(const Item* item,
                                 const GenericMap& theMap,
                                 SearchUnitBuilder* builder,
                                 set<IDPair_t>& allRegions);

   /**
    *   Adds the groups and locations from the item item to
    *   this TemporarySearchMapItem.
    *   @param item   Item to get the groups from.
    *   @param theMap Map to use for itemLookup.
    */
   inline void addGroupsFromItem(const Item* item,
                                 const GenericMap& theMap);

   /**
    *    Index - set in the second step, after all maps are loaded.
    */
   uint32 m_idx;
   
   /**
    *   The id of the item.
    */
   IDPair_t m_id;

   /**
    *   One coordinate of the item.
    */
   MC2Coordinate m_coord;
   
   /**
    *   The item type of the item. Will be copied to a
    *   separate array by the SearchMap.
    */
   ItemTypes::itemType m_itemType;

   /**
    *   The names. Type in first and stringindex in second.
    */
   slist<pair<uint32, uint32> > m_names;

   typedef slist<IDPair_t> regionCont;
   /** 
    *   The regions. Must be ID:s since the indices have not
    *   been given to the items yet.
    */
   regionCont m_inRegions;

   /**
    *   The radius of the regions that include radii.
    *   Zero means that it will not be added.
    */
   uint32 m_radiusMeters;
   
};

inline void
TemporarySearchMapItem::addGroupsFromItem(const Item* item,
                                          const GenericMap& theMap)
{
   // Insert the regions using groups too. 
   uint32 nbrGroups = item->getNbrGroups();
   for ( uint32 i = 0; i < nbrGroups; ++i ) {
      const Item* region = theMap.itemLookup(item->getGroup(i));
      switch ( region->getItemType() ) {
         // ItemTypes to ignore for regions.
         case ItemTypes::streetItem:
         case ItemTypes::categoryItem: // Add again in the future.
            break;
         default:
            m_inRegions.push_front( IDPair_t(theMap.getMapID(), 
                                             item->getUnmaskedGroup(i)));
            break;
      }
   } 
}

inline
TemporarySearchMapItem::TemporarySearchMapItem(const Item* item,
                                               const GenericMap& theMap,
                                               SearchUnitBuilder* builder,
                                               set<IDPair_t>& allRegions) :
      m_id(theMap.getMapID(), item->getID()),
      m_coord(theMap.getOneGoodCoordinate(item)), // Best coordinate
      m_itemType(item->getItemType()),
      m_radiusMeters(0) // Assume zero
{  
   
   // Insert the names
   int nbrNames = item->getNbrNames();
   for ( int i = 0; i < nbrNames; ++i ) {
      
      uint32 rawIndex = item->getRawStringIndex(i);

      const char* name = theMap.getRawName(item->getRawStringIndex(i));

      mc2dbg8 << "[TSMI]: Item " << m_id
              << " has name = \"" << name
              << "\""
              << endl;
      
      // Insert the string into the table and the index into the set.
      // rawIndex will be used for preserving the language and type
      // of the string. The string will be taken from second in the pair.
      m_names.push_front(make_pair(rawIndex,
                                   builder->getTemporaryStringIndex(name)));
   }


   // Add the groups from the item.
   
   const PointOfInterestItem* poi = NULL; // Will be set if it is a poi.
   
   // If it is a street, then also add the groups of the ssis.
   if ( item->getItemType() == ItemTypes::streetItem ) {
      addGroupsFromItem( item, theMap );
      const StreetItem* street = static_cast<const StreetItem*>(item);
      for ( uint32 i = 0; i < street->getNbrItemsInGroup(); ++i ) {
         const Item* ssi = theMap.itemLookup(street->getItemNumber(i));
         if ( ssi != NULL ) {
            addGroupsFromItem(ssi, theMap);
         }
      }
   } else if ( item->getItemType() == ItemTypes::pointOfInterestItem ) {
      // For POI: Add all groups of it's streetsegment item.
      poi = static_cast<const PointOfInterestItem*> ( item );
      // Special for airports. They will be found in whole map.
      if ( ! poi->isPointOfInterestType(ItemTypes::airport) ) {

         addGroupsFromItem( item, theMap );
         const Item* ssi = theMap.itemLookup( poi->getStreetSegmentItemID() );
         if ( ssi != NULL ) {
            addGroupsFromItem( ssi, theMap );
         }
      }
   } else {
      addGroupsFromItem( item, theMap );
   }
      
   for( TemporarySearchMapItem::regionCont::const_iterator it = 
           m_inRegions.begin() ; it != m_inRegions.end(); ++it ) {
      // High bit of id should be cleared.
      allRegions.insert(IDPair_t(it->getMapID(),
                                 MapBits::nodeItemID(it->getItemID()) ) );
   }
   
   
   // FIXME: Add radius for radius-buas here
   //        Some BUAs are defined by a radius around a point it can be used
   //        here.
   m_radiusMeters = 0;
   
   if ( poi != NULL ) { // This is set above.
      if ( MapBits::isOverviewMap( theMap.getMapID() ) ) {
         // POI on the overview map.
         if ( ( poi->isPointOfInterestType( ItemTypes::cityCentre ) ) ||
              ( poi->isPointOfInterestType( ItemTypes::mountainPass ) ) ) 
         {
            // City centre on overview map.
            // Change item type to bua.
            m_itemType = ItemTypes::builtUpAreaItem;
         }
      } else {
         // POI on the underview map.
         // Used to take the offset on street, but that didn't work
         // due to the WASP.
      }
      if ( ( poi->isPointOfInterestType( ItemTypes::cityCentre ) ) ||
           ( poi->isPointOfInterestType( ItemTypes::mountainPass )  ) ) 
      {
         // Add a radius for all citycenters!
         m_radiusMeters = 10000; // 10 km
      }
   }
}

// -- TemporarySearchMapPOIInfo

class TemporarySearchMapPOIInfo {
public:
   
   typedef SearchUnitBuilder::waspMap_t waspMap_t;
   
   /**
    *   Constructor.
    *   @param item    The original item in the GenericMap.
    *   @param theMap  The map where the item is located.
    *   @param builder The SearchUnitBuilder used for the temporary
    *                  string indeces.
    *   @param poiData Data previously read from WASP.
    */
   inline TemporarySearchMapPOIInfo(const PointOfInterestItem* item,
                                    const GenericMap& theMap,
                                    SearchUnitBuilder* builder,
                                    const waspMap_t& poiData);

   /**
    *   Adds a string to m_addresses by getting a temporary
    *   string index from the builder.
    *
    *   @param address  The address to add
    *   @param language The language of the string
    *   @param builder  The builder to use to get the string index
    */
   void addToAddresses( const char* address,
                        uint32 language,
                        SearchUnitBuilder* builder ) {
      uint32 tempIdx = builder->getTemporaryStringIndex( address );
      uint32 type = ItemTypes::officialName;
      m_addresses.push_back( make_pair( CREATE_NEW_NAME( language, type, 0 ),
                                        tempIdx ) );
   }

   /**
    *   The full item id.
    */
   IDPair_t m_id;
   
   /**
    *   The adresses of the company. (Names of streets).
    *   Type and language in first and temporary string index
    *   in second. Only one street is supported.
    */
   vector<pair<uint32, uint32> > m_addresses;
   
   /**
    *   The house number on the street.
    */
   uint16 m_trueNumberOnStreet;

   /**
    *   Subtype for the poi.
    */
   uint16 m_poiType;
   /// categories for this poi
   SearchMapPOIInfo::Categories m_categories;
   /// Special image data, from the map data
   MC2String m_specialImage;
};

inline
TemporarySearchMapPOIInfo::
TemporarySearchMapPOIInfo(const PointOfInterestItem* item,
                          const GenericMap& theMap,
                          SearchUnitBuilder* builder, 
                          const waspMap_t& allPoiData )
      : m_id(theMap.getMapID(),item->getID()),
        m_trueNumberOnStreet(0)
{

   // Fetch special imagename from map poi database
   byte ignoreExtraByte = 0;
   GfxFeatureMapUtility::getExtraInfoForPOI( *item, theMap,
                                             ignoreExtraByte,
                                             m_specialImage );

   // fetch categories
   vector<uint16> categories;
   theMap.getCategories( *item, categories );
   m_categories.insert( categories.begin(), categories.end() );

   m_poiType = item->getPointOfInterestType();

   // Find the data for the current poi.
   const GetAdditionalPOIInfo::resultMap_t emptyMap;
   const GetAdditionalPOIInfo::resultMap_t* poiData = &emptyMap;
   waspMap_t::const_iterator curit = allPoiData.find( item->getWASPID() );
   if ( curit != allPoiData.end() ) {
      poiData = &curit->second;
   }
   
   // Get street name and number, try POI info first

   pair<GetAdditionalPOIInfo::resultMap_t::const_iterator,
        GetAdditionalPOIInfo::resultMap_t::const_iterator> range =
      poiData->equal_range( GetAdditionalPOIInfo::VIS_ADDRESS );

   if ( range.first != range.second ) {
      // Add the street name strings to m_addresses
      for ( GetAdditionalPOIInfo::resultMap_t::const_iterator it = range.first;
            it != range.second; ++it ) {
         addToAddresses( it->second.first.c_str(), // the string
                         it->second.second,        // the language
                         builder );
      }      

      // Try to get the streetnumber from the POI.
      GetAdditionalPOIInfo::resultMap_t::const_iterator it = 
         poiData->find( GetAdditionalPOIInfo::VIS_HOUSE_NBR );
      if ( it != poiData->end() ) {
         m_trueNumberOnStreet = 
            strtol( it->second.first.c_str(), (char**) NULL, 10 );
      }
   } else {
      // Fall back on VIS_FULL_ADDRESS
      range = poiData->equal_range( GetAdditionalPOIInfo::VIS_FULL_ADDRESS );

      // Add the street name strings to m_addresses
      for ( GetAdditionalPOIInfo::resultMap_t::const_iterator it = range.first;
            it != range.second; ++it ) {
         MC2String addressWithoutHouseNumber;
         int tmpHouseNbr = 0;
         StringSearchUtility::simpleGetStreetNumberAndName(
            it->second.first,
            tmpHouseNbr,
            addressWithoutHouseNumber );

         if ( tmpHouseNbr != 0 ) {
            m_trueNumberOnStreet = tmpHouseNbr;
         }
         addToAddresses( addressWithoutHouseNumber.c_str(), // the string
                         it->second.second,                 // the language
                         builder );
      }      
   }

   if ( range.first == range.second ) {
      // No POI info about street name, get the street name from the SSI
      uint32 ssiID = item->getStreetSegmentItemID();
      StreetSegmentItem* ssi =
         item_cast<StreetSegmentItem*>(theMap.itemLookup(ssiID));

      if ( ssi == NULL ) {
         // No more data to add here.
         return;
      }

      // Add the names of the streetsegment
      int nbrStreetNames = ssi->getNbrNames();
      if ( nbrStreetNames == 0 ) {
         // Nothing to do
         return;
      }
   
      // Now insert the names
      for ( int i = 0; i < nbrStreetNames; ++i ) {
         uint32 rawIndex = ssi->getRawStringIndex(i);
         // Avoid roadNumbers. Maybe some more checks should be
         // added here.
         if ( GET_STRING_TYPE(rawIndex) != ItemTypes::roadNumber ) {
            // Get a temporary string index.
            const char* name = theMap.getRawName(rawIndex);
         
            uint32 tempIdx = builder->getTemporaryStringIndex(name);
            // Add the string to our vector.
            // The rawIndex is used for preserving the language and type
            // of the name. tempIdx will be used for the string.
            m_addresses.push_back(make_pair(rawIndex, tempIdx));
         }
      }
   }
}

// -- SearchUnitBuilder.

SearchUnitBuilder::SearchUnitBuilder() 
      : m_getPOIInfo(Properties::getProperty("POI_SQL_DATABASE"),
                     Properties::getProperty("POI_SQL_HOST"),
                     Properties::getProperty("POI_SQL_USER"),
                     Properties::getProperty("POI_SQL_PASSWORD"))
{
   
}

void
SearchUnitBuilder::deleteTempItems()
{
   for ( map<IDPair_t, TemporarySearchMapItem*>::iterator it =
            m_tempItems.begin();
         it != m_tempItems.end();
         ++it ) {
      delete it->second;
   }
   m_tempItems.clear();
   
   for ( map<IDPair_t, TemporarySearchMapPOIInfo*>::iterator it =
            m_tempPOIs.begin();
         it != m_tempPOIs.end();
         ++it ) {
      delete it->second;
   }
   m_tempPOIs.clear();

   for ( userRightsPerMap_t::iterator it = m_userRightsPerMap.begin();
         it != m_userRightsPerMap.end();
         ++it ) {
      delete it->second;
   }
   m_userRightsPerMap.clear();
   
}

SearchUnitBuilder::~SearchUnitBuilder()
{
   deleteTempItems();
}


inline bool
SearchUnitBuilder::addPOIInfo(const Item* item,
                              const GenericMap& theMap,
                              const waspMap_t& allPoiData)
{
   if ( item->getItemType() != ItemTypes::pointOfInterestItem ) {
      mc2log << warn << "[SUB]: addPoiInfo run for non-poi "
             << MC2HEX( item->getID() )  << endl;
      return false;
   }
   
   const PointOfInterestItem* poi =
      static_cast<const PointOfInterestItem*>(item);
   
   if ( poi != NULL ) {
      TemporarySearchMapPOIInfo* tempPoi =
         new TemporarySearchMapPOIInfo(poi, 
                                       theMap, 
                                       this, 
                                       allPoiData);
      
      m_tempPOIs.insert(make_pair(tempPoi->m_id, tempPoi) );

      return true;
   } else {
      return false;
   }
   
}

inline bool
SearchUnitBuilder::itemShouldBeAdded(const Item* item,
                                     const GenericMap& theMap) const
{
   // We must check some things
   
   if ( item->getNbrNames() == 0 ) {
      // No names - do not add.
      return false;
   }
   
   if ( item->getItemType() == ItemTypes::streetSegmentItem ) {
      // Check if the ssi isn't part of any street.      
      for (uint32 i=0; i < item->getNbrGroups(); ++i) {
         const Item* tmpItem = theMap.itemLookup(item->getGroup(i));
         if ((tmpItem != NULL) && 
             ( ItemTypes::itemType(tmpItem->getItemType()) == 
               ItemTypes::streetItem) ) {
            // Part of a street! Do not add.
            return false;
         }
      }
   }
   
   if ( ItemTypes::itemTypeToSearchType( item->getItemType() ) == 0 ) {
      // Will not be found anyway
      return false;
   }
   
   if ( MapBits::isOverviewMap( theMap.getMapID() ) ) {
      if ( ( ItemTypes::itemTypeToSearchType( item->getItemType() ) &
             SEARCH_ALL_REGION_TYPES ) ||
           // Also add if city centre poi
           ( ( item->getItemType() == ItemTypes::pointOfInterestItem ) &&
             ( ( static_cast<const PointOfInterestItem*> (item)
                 ->isPointOfInterestType( ItemTypes::cityCentre ) ) ||
               ( static_cast<const PointOfInterestItem*> (item)
                 ->isPointOfInterestType( ItemTypes::mountainPass ) )
             ) 
             )
           )
      {
         // ok - it is a region
      } else {
         return false;
      }         
   }
   // All checks are OK - add the item.
   return true;
}


inline bool
SearchUnitBuilder::addItem(const Item* item,
                           const GenericMap& theMap,
                           set<IDPair_t>& allRegionIDs,
                           const waspMap_t& waspInfo )
{
   IDPair_t curID ( theMap.getMapID(), item->getID() );
   
   if ( m_tempItems.find(curID) != m_tempItems.end() ) {
      // Already added - return false.
      allRegionIDs.erase(curID);
      return false;
   }
   
   // Skip other checks - we must add it since it is used
   // as an added region.
   if ( ! itemShouldBeAdded(item, theMap) ) {
      const bool usedAsRegionInOtherItem =
         ( allRegionIDs.find(curID) != allRegionIDs.end() );
      if ( ! usedAsRegionInOtherItem ) {
         // Don't add
         return false;
      } else {
         mc2dbg << "[SUB]: Item " << curID << " is region to other item"
                << endl;
      }
   }
   
   // Add the temporary item
   TemporarySearchMapItem* searchItem =
      new TemporarySearchMapItem(item, theMap, this, allRegionIDs);
   
   m_tempItems.insert(make_pair(searchItem->m_id, searchItem));
   
   // If the item is a poi, we will have to add some more info.
   if ( searchItem->m_itemType == ItemTypes::pointOfInterestItem ) {
      return addPOIInfo(item, theMap, waspInfo);
   } else {
      return true;
   }
   
//   return true;
}

inline uint32
SearchUnitBuilder::
getAllPOIInfo( waspMap_t& poiData,
               const GenericMap& theMap,
               GetAdditionalPOIInfo& poiInfoGetter )
{
   mc2dbg << "[SUB]: Reading WASP data from database" << endl;

   vector<Item*> allPois;
   for (uint32 i=0; i < NUMBER_GFX_ZOOMLEVELS; i++) {
      for (uint32 j=0; j < theMap.getNbrItemsWithZoom(i); j++) {
         Item* item = theMap.getItem(i, j);
         if ( item == NULL) {
            continue;
         } else if ( item->getItemType() ==
                     ItemTypes::pointOfInterestItem ) {
            allPois.push_back( item );
         }
      }
   }

   set<uint32> keyIDs;
   keyIDs.insert( GetAdditionalPOIInfo::VIS_HOUSE_NBR );
   keyIDs.insert( GetAdditionalPOIInfo::VIS_FULL_ADDRESS );
   keyIDs.insert( GetAdditionalPOIInfo::VIS_ADDRESS );
   keyIDs.insert( GetAdditionalPOIInfo::LONG_DESCRIPTION );

   map<uint32, POIInfo*> infos( theMap.getPOIInfos( allPois.begin(),
                                                    allPois.end(),
                                                    keyIDs ) );
   uint32 infosInserted = 0;
   // now for each info;
   // decompose the POIInfo into a GetAdditionalPOIInfo::poiResultMap_t
   map<uint32, POIInfo*>::const_iterator poiInfoIt = infos.begin();
   map<uint32, POIInfo*>::const_iterator poiInfoItEnd = infos.end();
   for ( ; poiInfoIt != poiInfoItEnd; ++poiInfoIt ) {
      GetAdditionalPOIInfo::resultMap_t resultValue;

      // decompose each POIInfoData into a GetAdditionalPOIInfo::resultMap_t
      for ( uint32 dataIt = 0; dataIt < (*poiInfoIt).second->getInfos().size();
            ++dataIt ) {
         ++infosInserted;
         POIInfoData* data = (*poiInfoIt).second->getInfos()[ dataIt ];
         resultValue.insert( make_pair( static_cast<GetAdditionalPOIInfo::keyID_t>
                                        ( data->getType() ),
                                        pair<MC2String, uint32>
                                        ( data->getInfo(), data->getLang() ) ) );
      }
      // now lets insert the decomposed data
      poiData.insert( make_pair( (*poiInfoIt).first, resultValue ) );
      // we dont need it anymore
      delete (*poiInfoIt).second;
   }
   // all pointers in infos are already dead at this point
   // so its safe to clear
   infos.clear();


   mc2dbg << "[SUB]: " << infosInserted
          << " infos read from database " << endl;

   return infosInserted;
}

bool
SearchUnitBuilder::addMap(const GenericMap& theMap)
{
   // Set containing all the regions used in the items
   set<IDPair_t> allRegions;
   
   mc2dbg << "[SUB]: addMap starts" << endl;
   const OverviewMap* ovMap = dynamic_cast<const OverviewMap*>(&theMap);
   if ( ovMap ) {
      if ( ! m_ovToUndTable.empty() ) {
         mc2dbg << "[SUB]: The idtranslation table sent to the SearchModule"
                << " does not support multiple overview maps" << endl;
         MC2_ASSERT( m_ovToUndTable.empty() );
      }
   }
   int nbrAdded = 0;
   int roundNbr = 0;

   // First get the poi info from the database, then
   // add all the items as usual.
   waspMap_t waspData;
   /* uint32 nbrWasp = */ getAllPOIInfo( waspData, theMap, m_getPOIInfo );
   
   // Loop until no items are added. This is so that the regions
   // of regions can be added even if they have the wrong type.
   do {
      nbrAdded = 0;
      // Loop over all the items (as usual)
      for (uint32 i=0; i < NUMBER_GFX_ZOOMLEVELS; i++) {
         for (uint32 j=0; j < theMap.getNbrItemsWithZoom(i); j++) {
            Item* item = theMap.getItem(i, j);
            if ( item == NULL) {
               continue;
            } else {
               const bool addRes = addItem(item, theMap, allRegions,
                                           waspData );
               if ( addRes ) {
                  ++nbrAdded;                  
               }
               if ( addRes && ovMap ) {
                  // Also add the stuff to the id-translation,
                  // but only for items that were added to the map.
                  m_ovToUndTable.insert(
                     make_pair(IDPair_t(theMap.getMapID(),
                                        item->getID()),
                               ovMap->lookup(item->getID())));
               }
            }
         }
      }
      
      mc2dbg << "[SUB]: " << nbrAdded << " items added in round "
             << roundNbr << endl;
      mc2dbg << "[SUB]: " << m_tempPOIs.size() << " of them are pois" << endl;
      for ( set<IDPair_t>::const_iterator it = allRegions.begin();
            it != allRegions.end();
            /**/ ) {
         if ( m_tempItems.find(*it) != m_tempItems.end() ) {
            allRegions.erase(it++);
         } else if ( theMap.itemLookup( (*it).second ) == NULL ) {
            mc2dbg << "[SUB]: Region was null" << endl;
            allRegions.erase(it++);
         } else {
            ++it;
         }
      }
      ++roundNbr;
   } while ( ! allRegions.empty() );
   
   // Save the country code for the map
   m_countryCode = theMap.getCountryCode();

   // Add the native languages from the map
   LangTypes::language_t lang = LangTypes::invalidLanguage;
   for (uint32 i = 0; i < theMap.getNbrNativeLanguages(); i++) {
      lang = theMap.getNativeLanguage(i);
      if (m_nativeLanguages.find(lang) == m_nativeLanguages.end()) {
         m_nativeLanguages.insert(lang);
      }
   }

   // Add user rights for the map
   // Oh. I have forgotten where the itemids are.
   // The are probably in m_tempItems
   mc2dbg << "[SUB]: START Re-building rights " << endl;
   {
      // Remember that the SearchUnit supports more than one map.
      set<uint32> localIDs;
      for ( itemMap_t::const_iterator it = m_tempItems.begin();
            it != m_tempItems.end();
            ++it ) {
         if ( it->first.getMapID() == theMap.getMapID() ) {
            localIDs.insert( it->first.getItemID() );
         }
      }
      // Create the new table containing only the wanted ids
      // and add it to the map.
      UserRightsItemTable* newRights =
         new UserRightsItemTable( theMap.getRightsTable(),
                                  localIDs );
      m_userRightsPerMap.insert( make_pair( theMap.getMapID(),
                                            newRights ) );
      
      for ( set<uint32>::const_iterator it = localIDs.begin();
            it != localIDs.end();
            ++it ) {
         MC2_ASSERT (
            m_userRightsPerMap[ theMap.getMapID() ]->getRights(*it) ==
            theMap.getRights( *it )
            );
      }
   }
   mc2dbg << "[SUB]: DONE Re-building rights " << endl;
   
   // FIXME: Check that all regions have been added to the
   //        map so it is possible to get info about them.

   mc2dbg << "[SUB]: addMap ends" << endl;
   return true;
}

// ----------------------------------------------------------
// -- Here are the functions that build the SearchMap and --
// -- SearchStructs                                       --
// -- GenericMaps are not needed here and no items must   --
// -- be added after buildItemIdx                         --
// ----------------------------------------------------------

void
SearchUnitBuilder::buildSearchMap()
{
   mc2dbg << "[SUB]: buildSearchMap starts" << endl;
   // Set index for the items. They are needed when setting regions
   {
      uint32 curIdx = 0;
      for( itemMap_t::const_iterator it = m_tempItems.begin();
           it != m_tempItems.end();
           ++it ) {
         it->second->m_idx = curIdx++;
      }
   }
   mc2dbg << "[SUB]: Indeces are set" << endl;
   
   // Create stringtable and regiontable
   m_stringTable = new WriteableSearchMapStringTable();
   WriteableSearchMapRegionTable* regionTable
      = new WriteableSearchMapRegionTable();
   
   // Map mapping old stringindeces to new ones
   map<uint32, uint32> revStringTable;
   
   // Add all the names to the stringtable so we will have them
   // in alphaorder (easier to see).
   for( stringMap_t::const_iterator it = m_strings.begin();
        it != m_strings.end();
        ++it ) {
      uint32 offset = m_stringTable->addItemName(it->first.c_str());
      revStringTable.insert(make_pair(it->second, offset));
   }

   mc2dbg << "[SUB]: Reverse stringtable built" << endl;
   
   // Release the memory for the strings.
   m_strings.clear();

   mc2dbg << "[SUB]: Oldstrings cleared" << endl;
   
   // Create the items - this should maybe be moved to the SearchMap?
   SearchMapItem* searchMapItems = new SearchMapItem[m_tempItems.size()];
   byte* itemTypes               = new byte[m_tempItems.size()];

   // Item index in first and radius in second.
   vector<pair<uint32, uint32> > tempRadii;
   
   mc2dbg << "[SUB]: Items allocated " << endl;
   
   uint32 curItemIdx = 0;
   
   // Keep this to avoid constructor/destructor inside.
   set<uint32> idxCombo;
   for( itemMap_t::const_iterator it = m_tempItems.begin();
        it != m_tempItems.end();
        ++it ) {
      TemporarySearchMapItem* curItem = it->second;
      // Get the stringcomboindex.
      uint32 stringComboIdx =
         m_stringTable->addStringCombo(curItem->m_names,
                                       &revStringTable);

      // We must translate the itemIDs to indeces
      idxCombo.clear();
      for ( TemporarySearchMapItem::regionCont::const_iterator it = 
               curItem->m_inRegions.begin() ; 
            it != curItem->m_inRegions.end() ; ++it ) {
         uint32 highBit = (*it).getItemID() & 0x80000000;

         IDPair_t idToFind = (*it);
         if ( highBit ) {
            // Trick needed
            idToFind.first = it->getMapID();
            idToFind.second = ( it->getItemID() & 0x7fffffff );
         }
         bool found = m_tempItems.find(idToFind) != m_tempItems.end();

         if ( found ) {
            // Add the combination including the high-bit.
            idxCombo.insert(m_tempItems[idToFind]->m_idx | highBit );
         } else {
            mc2dbg << "[SUB]: Region " << *it << " of "
                   << curItem->m_id << " has not been added" << endl;
         }
      }

      // Remove the item itself (if it is in there)
      idxCombo.erase(curItem->m_idx);
      idxCombo.erase(curItem->m_idx | 0x80000000);
      
      // Insert the combo in the table
      if ( idxCombo.size() >= 0xff ) {
         mc2dbg/*8*/ << "[WSMRT]: Item with more than 0xff ("
                     << idxCombo.size() << ") regions: " << curItem->m_id 
                     << endl;
      }
      uint32 regionComboIdx = regionTable->addRegionCombo(idxCombo);

      searchMapItems[curItemIdx] = SearchMapItem(curItem->m_id,
                                                 curItem->m_coord,
                                                 stringComboIdx,
                                                 regionComboIdx);
      
      itemTypes[curItemIdx]      = curItem->m_itemType;
      // Insert the radius if there is one
      if ( curItem->m_radiusMeters != 0 ) {
         tempRadii.push_back(make_pair(curItemIdx, curItem->m_radiusMeters) );
      }
      ++curItemIdx;
   }
   // Make the overflow lookup table for the region table
   regionTable->fixupRegionCombo();
   
   mc2dbg << "[SUB]: " << curItemIdx << " items built" << endl;
   // Check that the radii are sorted. They should be if there is
   // only one radius per item.
   MC2_ASSERT( is_sorted(tempRadii.begin(), tempRadii.end()));

   // Fixup the point of interest items.
   uint32 curPoiIdx = 0;
   SearchMapPOIInfo* searchMapPOIInfos =
      new SearchMapPOIInfo[m_tempPOIs.size()];

   for ( poiMap_t::const_iterator it = m_tempPOIs.begin();
         it != m_tempPOIs.end();
         ++it ) {
      TemporarySearchMapPOIInfo* curPoi = it->second;
      uint32 addressIdx = 0;
      if ( !curPoi->m_addresses.empty() ) {
         addressIdx = m_stringTable->addStringCombo( curPoi->m_addresses,
                                                     &revStringTable );
      }

      // Set all variables in the poiinfo.
      
      searchMapPOIInfos[curPoiIdx] =
         SearchMapPOIInfo(m_tempItems[curPoi->m_id]->m_idx,
                          addressIdx, curPoi->m_trueNumberOnStreet,
                          curPoi->m_poiType,
                          curPoi->m_categories,
                          curPoi->m_specialImage );
      curPoiIdx++;
   }

   mc2dbg << "[SUB]: " << curPoiIdx << " POIS built" << endl;
   mc2dbg << "[SUB]: " << m_stringTable->getTotalStringSize()
          << " chars in stringtable" << endl;

   IDTranslationTable* translationTable = new IDTranslationTable;
   
   for( map<IDPair_t,IDPair_t>::const_iterator it = m_ovToUndTable.begin();
        it != m_ovToUndTable.end();
        ++it ) {
      // Note that the translation table only works for one ov-map.
      translationTable->addElement(it->first.getItemID(),
                                   it->second.getMapID(),
                                   it->second.getItemID());
   }
   translationTable->sortElements();
   mc2dbg << "[SUB]: " << translationTable->getNbrElements() << " elements in "
          << "translation table" << endl;
   
   MC2_ASSERT( translationTable->getNbrElements() == m_ovToUndTable.size() );
   m_ovToUndTable.clear();

   // Create the searchMap. Note that the stringtable is owned by
   // the SearchMap now, but we will continue using it for the
   // SearchStructs built inside the SearchUnit.
   
   uint32 nbrRadiusInfos            = tempRadii.size();
   SearchMapRadiusInfo* radiusInfos = new SearchMapRadiusInfo[nbrRadiusInfos];
   for( uint32 i = 0; i < nbrRadiusInfos; ++i ) {
      radiusInfos[i].setAll(tempRadii[i].first, tempRadii[i].second);
   }
   
   m_searchMap = new SearchMap2(curItemIdx, searchMapItems, itemTypes,
                                curPoiIdx, searchMapPOIInfos,
                                m_stringTable,
                                regionTable,
                                translationTable,
                                nbrRadiusInfos,
                                radiusInfos,
                                m_countryCode,
                                m_nativeLanguages,
                                m_userRightsPerMap );

   mc2dbg << "[SUB]: So far the map is "
          << m_searchMap->getSizeInDataBuffer() << " bytes " << endl;

   
   if ( false ) {
      mc2dbg << "[SUB]: Testing if I can save the map in a DataBuffer"
          << endl;
      DataBuffer buf(m_searchMap->getSizeInDataBuffer());
      m_searchMap->save(buf);
      SearchMap2 newMap;
      uint32 startTime = TimeUtility::getCurrentTime();
      buf.reset();
      newMap.load(buf);
      mc2dbg << "[SUB]: Map loaded in "
             << (TimeUtility::getCurrentTime()-startTime) <<  " millis " << endl;
   }
   
   // Don't return some memory
   // deleteTempItems();
   
   mc2dbg << "[SUB]: buildSearchMap ends" << endl;   
}

SearchUnit*
SearchUnitBuilder::createSearchUnit()
{
   // After this no items can be added.
   buildSearchMap();

   // Now the SearchMap should be in m_searchMap and
   // all the temporary items unusable.

   // Create the SearchUnit.
   mc2dbg << "[SUB]: Creating SearchUnit" << endl;
   WriteableSearchUnit* searchUnit = new WriteableSearchUnit(m_searchMap,
                                                             m_stringTable);
   mc2dbg << "[SUB]: SearchUnit created" << endl;

   mc2dbg << "[SUB]: SearchUnit->getSizeInDataBuffer() = "
          << searchUnit->getSizeInDataBuffer() << endl;
   
   if ( false ) {
      DataBuffer db(searchUnit->getSizeInDataBuffer());
      searchUnit->save(db);
      uint32 startTime = TimeUtility::getCurrentTime();
      db.reset();
      SearchUnit unit;
      unit.load(db);
      mc2dbg << "[SUB]: SearchUnit loaded in "
             << (TimeUtility::getCurrentTime()-startTime) <<  " millis " << endl;
   }
   
   return searchUnit;
}

