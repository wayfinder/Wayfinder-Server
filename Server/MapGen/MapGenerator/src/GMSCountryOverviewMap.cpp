/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSCountryOverviewMap.h"
#include "GMSGfxData.h"
#include "StringUtility.h"
#include "OldWaterItem.h"
#include "OldForestItem.h"
#include "OldZipCodeItem.h"
#include "GMSStreetSegmentItem.h"
#include "Stack.h"
#include "OldPointOfInterestItem.h"
#include "GMSUtility.h"
#include "OldMapFilter.h"
#include "OldStreetItem.h"
#include "MapFilterUtil.h"
#include "MapGenUtil.h"

// If change, change also in OldOverviewMap
#define MAX_ITEM_SIZE 7000000


const uint32 GMSCountryOverviewMap::MAX_STREET_ZOOMLEVEL = 2;
const uint32 GMSCountryOverviewMap::MAX_STREETSEGMENT_ZOOMLEVEL = 
   GMSCountryOverviewMap::MAX_STREET_ZOOMLEVEL + 1;


GMSCountryOverviewMap::GMSCountryOverviewMap(uint32 id)
   : OldCountryOverviewMap(id)
{
   reset();
}

GMSCountryOverviewMap::GMSCountryOverviewMap(uint32 id, const char* path)
   : OldCountryOverviewMap(id, path)
{
   reset();
}

void
GMSCountryOverviewMap::reset()
{
   DEBUG4(cerr << "GMSCountryOverviewMap::reset()" << endl);
   for (uint32 i=0; i<NUMBER_GFX_ZOOMLEVELS; i++) {
      m_itemsZoomSize[i] = 0;
      m_itemsZoomAllocated[i] = 0;
      m_itemsZoom[i] = NULL;
   }
   m_hashTable = NULL;
}


GMSCountryOverviewMap::~GMSCountryOverviewMap()
{
}


bool 
GMSCountryOverviewMap::internalLoad( DataBuffer& dataBuffer ) 
{
   bool retVal = OldCountryOverviewMap::internalLoad( dataBuffer );

   if ( retVal ) {
      // Create the table m_idsByOriginalIDs (which is a reverse map
      // of m_originalIDs).
      
      buildIDsByOriginalIDsTable();
   }

   return retVal;
}
   

bool 
GMSCountryOverviewMap::createCountryBorder()
{
   // The country polygon (the country border) should be loaded 
   // from the country polygon mif file.
   // Path hardcoded, the country mif file must be in the same directory
   // as the country overview map is created in
   const char* path = "./";      
   char* mifname = new char[256];
   MC2String tmp = getMapName();
   MC2String countryName = StringUtility::copyLower(tmp);
   
   sprintf(mifname, "%s%s.mif", path, countryName.c_str());
   cerr << "   Tryes to open file:" << mifname << endl;
   
   ifstream miffile(mifname, ios::in);

   

   GMSGfxData* gfx = NULL;
   if (miffile) {
      // Found a file, take the boundry from that
      cerr << "   Found miffile, creating from that!" << endl;
      gfx = new GMSGfxData();
      gfx->createFromMif(miffile);
      gfx->sortPolygons();
      setMapGfxDataFiltered(false);
   } else {
      mc2log << error << "GMSCountryOverviewMap::createCountryBorder: "
             << "Mif file not present in directory: " 
             << path << " Exits!" << endl;
      MC2_ASSERT(false);
   }

   // Set the m_gfxData-member
   m_gfxData = gfx;

   delete [] mifname;
   mifname = NULL;
   return (true);
}

bool
GMSCountryOverviewMap::addDataFromMap(OldGenericMap* otherMap)
{
   mc2dbg << "GMSCountryOverviewMap::addDataFromMap() ID=" 
          << otherMap->getMapID() << endl; 
   
   // Create a copy of the GfxData for otherMap
   GfxData* gfx = GMSGfxData::createNewGfxData(NULL, otherMap->getGfxData());

   // Add the ID, creation time and bounding box of the other map to this country
   struct mapnotice_t notice;
   notice.mapID = otherMap->getMapID();
   notice.creationTime = otherMap->getCreationTime();
   MC2BoundingBox otherBBox;
   otherMap->getMapBoundingBox(otherBBox);
   notice.maxLat = otherBBox.getMaxLat();
   notice.minLon = otherBBox.getMinLon();
   notice.minLat = otherBBox.getMinLat();
   notice.maxLon = otherBBox.getMaxLon();

   m_mapsInCountry.push_back(notice);

   // Add all items from the underview map to this co-map.
   const uint32 zoomStop = NUMBER_GFX_ZOOMLEVELS;
   for (uint32 z=0; z<zoomStop; z++) {
      for (uint32 i=0; i<otherMap->getNbrItemsWithZoom(z); i++) {
         OldItem* item = otherMap->getItem(z, i);
         uint32 curZoom = z;
         // check if we want the item in this co-map
         if ( toIncludeItem(item, otherMap, curZoom ) ) {
            addItemToCountryOverview(item, otherMap, curZoom, gfx);
         }
      }
   }

   mc2dbg1 << "Number m_waterCandidates = " << m_waterCandidates.size()
           << " m_forestCandidates = " << m_forestCandidates.size() << endl;

   return (true);
}


OldItem*
GMSCountryOverviewMap::addItemToCountryOverview(OldItem* item, 
                                                OldGenericMap* otherMap,
                                                uint32 requestedZoomLevel,
                                                GfxData* mapGfx )
{
   // First copy the item (naive way)
   DataBuffer dataBuffer( MAX_ITEM_SIZE );
   otherMap->saveOneItem(&dataBuffer, item);
   dataBuffer.reset();
   OldItem* newItem = createNewItem(&dataBuffer);

   // Create a GfxData for the boundingBox if municipal or BUA
   GfxDataFull* newGfx = NULL;
   
   // Skip geometry of index areas, they are only important for searching
   if ( otherMap->isIndexArea(item->getID()) ) {
      newItem->setGfxData( NULL );
   }
   else if ( ( item->getGfxData() != NULL ) && 
             ( item->getItemType() == ItemTypes::municipalItem ) ) {
      // Set GfxData of municipal to the bbox of its GfxData.
      MC2BoundingBox bbox;
      item->getGfxData()->getMC2BoundingBox(bbox);
      newGfx = GMSGfxData::createNewGfxData(NULL, &bbox);
   } else if ( ( item->getGfxData() == NULL ) && 
               ( ( item->getItemType() == ItemTypes::municipalItem ) ||
                 ( item->getItemType() == ItemTypes::builtUpAreaItem ) ||
                 ( item->getItemType() == ItemTypes::zipCodeItem ) )
             ) {
      // Either municipal or bua and their GfxData is NULL.
      mc2log << error << here 
             << "gfxData for municipal/bua/zip set to NULL"
             << endl;
   } else if ( ( item->getGfxData() == NULL ) &&
               ( item->getItemType() == ItemTypes::pointOfInterestItem ) ){
      // Since the POI doesnt have a gfxdata, create one from the 
      // ssi.
      OldPointOfInterestItem* poi = 
         static_cast<OldPointOfInterestItem*> ( item ); 
      OldItem* ssi = otherMap->itemLookup( poi->getStreetSegmentItemID() );
      int32 lat, lon;
      ssi->getGfxData()->getCoordinate( 
            poi->getOffsetOnStreet(), lat, lon );
      
      newGfx = GMSGfxData::createNewGfxData(NULL, true);
      newGfx->addCoordinate( lat, lon );
          
   } else {
      uint32 maxDist = 10;
      switch (item->getItemType()) {
         case (ItemTypes::streetItem) :
         case (ItemTypes::streetSegmentItem) :
            mc2dbg4 << "To get GfxData for OldStreetItem" << endl;
            maxDist = MAX_UINT32;
            break;
         case (ItemTypes::waterItem) :
         case (ItemTypes::forestItem) :
            maxDist = MAX_UINT32;
            break;
         case (ItemTypes::builtUpAreaItem) :
            mc2dbg4 << "To get GfxData for OldBuiltUpAreaItem" << endl;
            maxDist = 2000;
            break;
         default :
            mc2dbg4 << "To get GfxData for unknown OldItem" << endl;
      }

      // Note that this method only adds the appropriate
      // polygons. The skipped polygons are stored in 
      // skippedPolygons.
      vector<uint16> skippedPolygons;
      newGfx = createNewGfxData(item, maxDist, &skippedPolygons);
      
      if (item->getItemType() == ItemTypes::streetItem) {

         // We need to update the itemOffsetForPolygon vector
         // and indexInGroup vector. At least if we did not
         // add all the polygons that existed in 
         // the original item.
         OldStreetItem* streetItem = 
            static_cast<OldStreetItem*> (newItem);
         vector<uint32> itemsInGroupToRemove;
         for ( vector<uint16>::const_iterator it = 
               skippedPolygons.begin();
               it != skippedPolygons.end(); 
               ++it ) {
            itemsInGroupToRemove.push_back( 
                  streetItem->m_itemOffsetForPolygon[*it] );
            // Mark that this element should be removed.
            streetItem->m_itemOffsetForPolygon[*it] = MAX_UINT16;
         }
         
         if (newGfx->getNbrPolygons() > 0) {
            uint16* newItemOffsetForPolygon = 
               new uint16[newGfx->getNbrPolygons()];
            uint16 idx = 0;
            uint16 counter = 0;
            while (idx < newGfx->getNbrPolygons()) {
               // Copy the elements that are not marked to
               // be removed. 
               if (streetItem->m_itemOffsetForPolygon[counter] !=
                   MAX_UINT16) {
                  newItemOffsetForPolygon[idx] = 
                     streetItem->m_itemOffsetForPolygon[counter];
                  idx++;
               }
               counter++;
            }
            
            // Delete and replace the old array with the new.
            delete streetItem->m_itemOffsetForPolygon;
            streetItem->m_itemOffsetForPolygon = 
               newItemOffsetForPolygon;

         } else {
            delete streetItem->m_itemOffsetForPolygon;
            streetItem->m_itemOffsetForPolygon = NULL; 
         }
          
         // Sort itemsInGroupToRemove
         sort(itemsInGroupToRemove.begin(), 
              itemsInGroupToRemove.end());
         // Also update the group vector:
         // Remove backwards (so we don't mess up the indices).
         for ( vector<uint32>::reverse_iterator it =  
                  itemsInGroupToRemove.rbegin();
               it != itemsInGroupToRemove.rend();
               ++it ) {
            streetItem->removeItemNumber(*it);
         }
      }
   }
   
   // Give the new GfxData to the new OldItem
   newItem->setGfxData(newGfx);

   // Update the name
   updateNames(newItem, item, otherMap);

   // Remove all groups except municipal or bua groups.
   removeUnwantedGroups(newItem, otherMap);
   
   // Add groups to pois to enable unique names (underview ids)
   addGroupsToPoi(newItem, item, otherMap);
   
   // Add the new item to this map, 
   // or to a temporary array if water-item or forest-items
   // since only the largest waters& forests will be added to the map
   if (item->getItemType() == ItemTypes::waterItem) {
      waterNotice_t notice;
      notice.waterItems.push_back(
            static_cast<OldWaterItem*>(newItem));
      struct originalIDs_t origIDs;
      origIDs.origItemID = item->getID();
      origIDs.origMapID = otherMap->getMapID();
      notice.origIDs.push_back( origIDs );
      mc2dbg1 << "Adding water candidate (" << origIDs.origMapID << ", "
              << origIDs.origItemID << ") " 
              << otherMap->getFirstItemName(item) << endl;
      float64 area = 0;
      const GfxData* gfx = newItem->getGfxData();
      MC2BoundingBox tmpBBox;
      gfx->getMC2BoundingBox(tmpBBox);

      if (otherMap->getGfxData() != NULL) {
         MC2BoundingBox otherTmpBBox;
         otherMap->getGfxData()->getMC2BoundingBox(otherTmpBBox);

         if (! tmpBBox.inside(otherTmpBBox)) {
            // Make sure this water items will be included since
            // it is not completely inside map boundingbox.
            // Set area to max, will that the item will be included.
            mc2dbg1 << " adding water " 
                    << otherMap->getFirstItemName(item)
                    << " since it is not completely inside map bbox."
                    << endl;
            area = 1000000.0*1000000.0; // ca 10*10 km^2
         }
      } 
      // Calculate real area if the area is not already set.
      // Use less than operator instead of equal since we're dealing
      // with floating numbers.
      float64 eps = 0.001;
      if (area < eps) {
         for (uint32 p=0; p<gfx->getNbrPolygons(); p++) {
            float64 curArea = fabs(gfx->polygonArea(p));
            area += curArea;
            mc2dbg8 << " area=" << area << ", curArea=" 
                    << curArea << endl;
         }
      }
      notice.area = uint32(rint(area * 
               GfxConstants::SQUARE_MC2SCALE_TO_SQUARE_METER));
      notice.name = StringUtility::newStrDup(
         otherMap->getFirstItemName(item));
      m_waterCandidates.push_back(notice);
      mc2dbg8 << "   adding water candiate, area=" 
              << notice.area << " (" << area << ")" << endl;
   }
   else if (item->getItemType() == ItemTypes::forestItem) {
      forestNotice_t notice;
      notice.forestItems.push_back( static_cast<OldForestItem*>(newItem));
      struct originalIDs_t origIDs;
      origIDs.origItemID = item->getID();
      origIDs.origMapID = otherMap->getMapID();
      notice.origIDs.push_back( origIDs );
      mc2dbg1 << "Adding forest candidate (" << origIDs.origMapID << ", "
              << origIDs.origItemID << ") " 
              << otherMap->getFirstItemName(item) << endl;
      float64 area = 0;
      const GfxData* gfx = newItem->getGfxData();

      // Calculate real area if the area is not already set.
      // Use less than operator instead of equal since we're dealing
      // with floating numbers.
      float64 eps = 0.001;
      if (area < eps) {
         for (uint32 p=0; p<gfx->getNbrPolygons(); p++) {
            float64 curArea = fabs(gfx->polygonArea(p));
            area += curArea;
            mc2dbg8 << " area=" << area << ", curArea=" 
                    << curArea << endl;
         }
      }
      notice.area = uint32(rint(area * 
               GfxConstants::SQUARE_MC2SCALE_TO_SQUARE_METER));
      notice.name = StringUtility::newStrDup(
         otherMap->getFirstItemName(item));
      m_forestCandidates.push_back(notice);
      mc2dbg8 << "   adding forest candiate, area=" 
              << notice.area << " (" << area << ")" << endl;
   
   } else {
      uint32 zoomLevel = 0;
      switch (item->getItemType()) {
         case (ItemTypes::municipalItem) :
            zoomLevel = 0;
            break;
         case (ItemTypes::builtUpAreaItem) :
            zoomLevel = 1;
            break;
         default :
            zoomLevel = requestedZoomLevel;
      }
      uint32 newID = addItem(newItem, zoomLevel);
      mc2dbg1 << "   New item added on zoomlevel=" << zoomLevel
              << " with ID=" << newID << " type=" 
              << uint32(item->getItemType()) << endl;

      // Save the original OldItemID and OldMapID of this item.
      struct originalIDs_t origIDs;
      origIDs.origItemID = item->getID();
      origIDs.origMapID = otherMap->getMapID();
      m_originalIDs.insert(make_pair(newID, origIDs));
   }

   return newItem;
}


void 
GMSCountryOverviewMap::makeUniqueNames()
{
   // Builtup areas 
   makeUniqueNames( ItemTypes::builtUpAreaItem );
   // City centres
   makeUniqueNames( ItemTypes::pointOfInterestItem,
                    ItemTypes::cityCentre );
}



void
GMSCountryOverviewMap::makeUniqueNames( 
                               ItemTypes::itemType itemType,
                               ItemTypes::pointOfInterest_t poiType
                                  /* = ItemTypes::nbr_pointOfInterest*/,
                               OldItem* checkItem /* = NULL */)
{
   // Check all BUA or CityCentrePoi-names (official names) and add 
   // unique names (item-name + municipal name) to the ones with 
   // duplicated names.

   mc2log << info << "GMSCountryOverviewMap::makeUniqueNames itemType="
          << StringTable::getString(
               ItemTypes::getItemTypeSC(itemType), StringTable::ENGLISH)
          << endl;

   // If checkItem is not NULL, means that we are applying changes to 
   // co-map and need to check if a unique name should be added to 
   // checkItem and any other items with the same official name.
   vector<uint32> checkStringIdxs;
   vector<LangTypes::language_t> checkLanguages;
   if (checkItem != NULL) {
      for (byte k=0; k <checkItem->getNbrNames(); ++k) {
         if (checkItem->getNameType(k) == ItemTypes::officialName) {
            checkStringIdxs.push_back(checkItem->getStringIndex(k));
            checkLanguages.push_back(checkItem->getNameLanguage(k));
         }
      }
      mc2dbg1 << "Make uniqe name for check item " << checkItem->getID();
      for (uint32 i = 0; i < checkStringIdxs.size(); i++) {
         mc2dbg1 << ", strIdx=" << checkStringIdxs[i] << " name="
                 << getName(checkStringIdxs[i]) << " lang="
                 << LangTypes::getLanguageAsString(checkLanguages[i]);
      }
      mc2dbg1 << endl;

      // Does the checkItem already have a unique name in any of the
      // languages?
      for (uint32 i = 0; i < checkLanguages.size(); i++) {
         LangTypes::language_t checkLang = checkLanguages[i];
         uint32 checkStringIdx = checkStringIdxs[i];
         uint32 checkUniqueIndex = 
            checkItem->getNameWithType(ItemTypes::uniqueName, checkLang);
         if (checkUniqueIndex != MAX_UINT32) {
            // If the unique name already includes the official name, there 
            // is no need to continue, else remove it to set a new un below.
            mc2dbg2 << "Check item already had unique name "
                 << getName(checkUniqueIndex) << " for lang="
                 << LangTypes::getLanguageAsString(checkLang) << endl;
            char checkONStr[128];
            sprintf(checkONStr, "%s,", getName(checkStringIdx));
            if ( StringUtility::strncasecmp( 
                     getName(checkUniqueIndex), checkONStr, 
                     strlen(getName(checkStringIdx))+1 ) == 0) {
               // The officialName is in the unique name, don't continue
               mc2dbg2 << " correct uniqueName (" << getName(checkUniqueIndex)
                       << " - " << checkONStr << endl;
               return;
            } else {
               // Remove the incorrect unique name
               mc2dbg2 << " incorrect uniqueName (" << getName(checkUniqueIndex)
                       << " - " << checkONStr << " << - remove it" << endl;
               byte offset = 0;
               while ( (offset < checkItem->getNbrNames()) && 
                       (checkItem->getStringIndex(offset) != 
                                             checkUniqueIndex)) {
                  ++offset;
               }
               if (offset < checkItem->getNbrNames())
                  checkItem->removeNameWithOffset(offset);
            }
         }
      }
   }
   
   // Add all official names to a multi-map < <stringIndex,language>, itemID>
   typedef pair<uint32, LangTypes::language_t> strIdxLangPair_t;
   multimap<strIdxLangPair_t, uint32> names;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; ++z) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); ++i) {
         OldItem* item = getItem(z,i);
         if ( (item != NULL) && 
              (item->getItemType() == itemType)) {
            // Check that it is correct poi-type if poi.
            if ( ( itemType != ItemTypes::pointOfInterestItem ) ||
                 ( (static_cast<OldPointOfInterestItem*> (item))
                        ->isPointOfInterestType( poiType ) ) ) {
               // Fill the multimap.
               for (byte k=0; k<item->getNbrNames(); ++k) {

                  if ((item->getNameType(k) == ItemTypes::officialName) &&
                      ( (checkItem == NULL) ||
                        ( (checkItem != NULL) &&
                          hasWantedName(item, k, 
                                 checkStringIdxs, checkLanguages)) )) {
                     
                     names.insert(pair<strIdxLangPair_t,uint32>(
                         pair<uint32, LangTypes::language_t>(
                          item->getStringIndex(k), item->getNameLanguage(k)),
                         item->getID() ));
                  }
               }
            }
         }
      }
   }

   if (checkItem != NULL) {
      mc2dbg1 << "GMSCOMap makeUniqueNames() collected " << names.size() 
              << " official names for check item " << checkItem->getID() 
              << endl;
      // To pass through the loops alright insert an invalid item last
      // in the names vector.
      names.insert(pair<strIdxLangPair_t, uint32>(
            pair<uint32,LangTypes::language_t>(
               MAX_UINT32, LangTypes::invalidLanguage),
            MAX_UINT32));
   }

   // Print all names (debug)
   typedef multimap<strIdxLangPair_t,uint32>::const_iterator nameIterator;
   DEBUG4(
      for (nameIterator it = names.begin(); it != names.end(); ++it)
         mc2dbg << getName((*it).first.first) << ", " << (*it).second << " "
                << LangTypes::getLanguageAsString((*it).first.second) << endl;
   );
   
   // Print all multiple names (debug)
   DEBUG8(
      for (nameIterator it = names.begin(); it != names.end(); ++it) {
         if (names.count((*it).first) > 1) {
            mc2dbg << names.count((*it).first) << " items with name \""
                   << getName((*it).first.first) << "\" and lang="
                   << LangTypes::getLanguageAsString((*it).first.second)
                   << ", ID: ";
            nameIterator upperIt = names.upper_bound((*it).first);
            for (nameIterator lowerIt = names.lower_bound((*it).first);
                 lowerIt != upperIt; ++lowerIt) {
               mc2dbg << (*lowerIt).second << " ";
               ++it; // to not print the ids count nbr times..
            }
            mc2dbg << endl;
         }
      }
   );
   
   // Create unique names
   for (nameIterator it = names.begin(); it != names.end(); ++it) {
      if (names.count((*it).first) > 1) {
         nameIterator upperIt = names.upper_bound((*it).first);
         for (nameIterator lowerIt = names.lower_bound((*it).first);
              lowerIt != upperIt; ++lowerIt) {
            OldItem* curItem = itemLookup((*lowerIt).second);
            LangTypes::language_t lang = (*lowerIt).first.second;

            uint32 munID = 
               this->getRegionID( curItem, ItemTypes::municipalItem );
            if (munID != MAX_UINT32) {
               const char* munName = getName( itemLookup(munID)
                        ->getNameWithType(ItemTypes::officialName, 
                                          lang));

               // If checkItem != NULL we are applying changes to the co-map
               // perhaps this item already has a unique name, then don't
               // add one again, just skip to next item
               bool hasUniqueName = false;
               if ( (checkItem != NULL) &&
                    (curItem->getNameWithType(
                           ItemTypes::uniqueName, lang) != MAX_UINT32)) {
                  hasUniqueName = true;
               }

               if (!hasUniqueName) {
                  char uniqueName[128];
                  sprintf(uniqueName, "%s, %s", 
                          getName((*lowerIt).first.first), munName);
                  if (addNameToItem(curItem, uniqueName, lang, 
                                    ItemTypes::uniqueName) != MAX_UINT32) {
                     mc2dbg1 << "Unique name \"" << uniqueName << "\" lang=" 
                             << LangTypes::getLanguageAsString(lang) 
                             << " added to item " << curItem->getID()
                             << endl;
                  } else {
                     mc2log << error << "Failed to add unique name ("
                            << uniqueName << ") to curItem with ID=" 
                            << (*it).second << endl;
                  }
               } else {
                  mc2dbg1 << "OldItem " << curItem->getID() << " had un \""
                          << getName(curItem->getNameWithType(
                                ItemTypes::uniqueName, lang)) << "\" lang="
                          <<  LangTypes::getLanguageAsString(lang) << endl;
               }
            }
            ++it;
            if (it == names.end()){
               --it; // Or else we go out of bounds.
            }
         }
      }
   }
   
   if (checkItem == NULL)
      mc2dbg1 << "Leaving GMSCountryOverviewMap::makeUniqueNames()" << endl;
}

bool
GMSCountryOverviewMap::hasWantedName(
      OldItem* item, byte nameNbr, 
      const vector<uint32>& checkStringIdxs,
      const vector<LangTypes::language_t>& checkLanguages)
{
   bool hasName = false;
   uint32 itemStrIdx = item->getStringIndex(nameNbr);
   LangTypes::language_t itemLang = item->getNameLanguage(nameNbr);

   for (uint32 i = 0; i < checkStringIdxs.size(); i++) {
      if ( (itemStrIdx == checkStringIdxs[i]) &&
           (itemLang == checkLanguages[i]) ) {
         hasName = true;
      }
   }
   
   return hasName;
}

uint32 
GMSCountryOverviewMap::addZipCodeItems(OldOverviewMap* overview)
{
   uint32 nbrAddedZips = 0;
   const uint32 z = 10;
   for (uint32 i=0; i<overview->getNbrItemsWithZoom(z); ++i) {
      OldZipCodeItem* zip = dynamic_cast<OldZipCodeItem*>(overview->getItem(z, i));
      if (zip != NULL) { 
         // Include if not represented in underview OR 
         //            no parent with exact same name
         bool includeItem = true;
         cerr << overview->getName(zip->getStringIndex(0)) << " has " 
              << uint32(zip->getNbrGroups()) << " group(s):";
         for (uint32 x=0; x<zip->getNbrGroups(); ++x)
            cerr << overview->getName(overview->itemLookup(zip->getGroup(x))->getStringIndex(0)) << " ";
         cerr << endl;
         if (zip->getNbrGroups() > 0) {
            for (uint32 g=0; g<zip->getNbrGroups(); ++g) {
               OldGroupItem* group = dynamic_cast<OldGroupItem*>
                  (overview->itemLookup(zip->getGroup(g)));
               mc2dbg << "Checking items with names "
                      << overview->getName(group->getStringIndex(0)) << " and " 
                      << overview->getName(zip->getStringIndex(0)) << endl;
               if ( (group != NULL) &&
                    (group->getItemType() == zip->getItemType()) &&
                    group->hasSameNames(zip)) {
                  includeItem = false;
               }
            }
         }
         
         if (includeItem) {
            OldZipCodeItem* countryZip = dynamic_cast<OldZipCodeItem*>
                                      (addItemToCountryOverview(zip, overview, z));
            if (countryZip != NULL) {
               // Remove all groups, this is not necessary!
               while (countryZip->getNbrGroups() > 0) {
                  countryZip->removeGroup(countryZip->getNbrGroups()-1);
               }
               while (countryZip->getNbrItemsInGroup() > 0) {
                  countryZip->removeItemNumber(countryZip->getNbrItemsInGroup()-1);
               }

            }
            ++nbrAddedZips;
         } else {
            mc2dbg << "Excluding zip with overviewID " << zip->getID() << " ("
                   << overview->getName(zip->getStringIndex(0)) << ")" << endl;
         }
      }
   }
   return nbrAddedZips;
}
      
void 
GMSCountryOverviewMap::updateGroupIDs()
{
   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; ++z ) {
      for ( uint32 i = 0; i < getNbrItemsWithZoom( z ); ++i) {
         OldItem* item = getItem( z, i );
         if ( item != NULL ) {
            updateGroupIDs(item);
         }
      }
   }
}

void
GMSCountryOverviewMap::updateGroupIDs(OldItem* item)
{
   DEBUG8(
   uint32 munID = 
      this->getRegionID( item, ItemTypes::municipalItem ); 
   cout << " for item " << item->getID() << " nbr groups="
        << int(item->getNbrGroups()) << " munid=" << munID;
   if (munID != MAX_UINT32) {
      const char* munName = getName( itemLookup(munID)
               ->getNameWithType(ItemTypes::officialName, 
                                 LangTypes::invalidLanguage));
      cout << " mun=" << munName;
   }
   cout << endl;
   );
   
   for ( uint32 g = 0; g < item->getNbrGroups(); ++g ) {
      uint32 origMapID, foo;
      getOriginalIDs( item->getID(), origMapID, foo );
      uint32 groupID = getOverviewID( origMapID, item->getGroup( g ) );
      mc2dbg8 << "for item " << item->getID() << " to set group "
              << g << " to " << groupID << " (was " << item->getGroup( g )
              << ":" << getFirstItemName(item->getGroup(g)) << ")" << endl;
      if ( groupID != MAX_UINT32 ) {
         mc2dbg4 << "for item " << item->getID() << " group " << g 
                 << " updated to " << groupID << ":"
                 << getFirstItemName(groupID)
                 << " (was " << item->getGroup( g ) << ":" 
                 << getFirstItemName(item->getGroup(g)) << ")" << endl;
         item->setGroup( g, groupID );
      } else {
         mc2dbg8 << "Could not succesfully lookup group "
                 << g << " = " << item->getGroup( g )
                 << " for " << item->getID() << "." 
                 << " Removing the group." << endl;
         mc2dbg8 << "Nbr groups for item: " 
                 << item->getNbrGroups() << endl;
         item->removeGroup( g );
         g--; // Because size of the groups vector decreased.
      }
   }
}

uint32 
GMSCountryOverviewMap::addWaterItems( 
      multimap<uint32, uint32>& addedWaterOriginalIDs )
{
   typedef vector<waterNotice_t>::iterator waterIt;
   typedef list<OldWaterItem*>::iterator waterItemIt;

   mc2dbg << "GMSCountryOverviewMap::addWaterItems Got "
          << m_waterCandidates.size() << " wateritems" << endl;

   // Sort the water-items according to the size
   sort(m_waterCandidates.begin(), m_waterCandidates.end(),
        LessWaterAreaOrder());

   // Print all the water-items
   uint32 k=0;
   DEBUG4(
      for (waterIt w=m_waterCandidates.begin(); 
           w<m_waterCandidates.end(); w++){
         cerr << "   " << k++ << " " << w->name << " " 
              << w->area << " m^2" << endl;
      }
   );

   // Remove all really small water-items
   const uint32 FIRST_FILTER_MIN_SQ_AREA = 1000*1000;
   uint32 nbrWaterBefore = m_waterCandidates.size();
   waterIt w = m_waterCandidates.begin();
   while ( (w != m_waterCandidates.end()) && 
           (w->area > FIRST_FILTER_MIN_SQ_AREA)) {
      w++;
   }
   m_waterCandidates.erase(w, m_waterCandidates.end());
   mc2dbg << "GMSCountryOverviewMap::addWaterItems:"
          << "Not adding: " << nbrWaterBefore-m_waterCandidates.size() 
          << " water items of " << nbrWaterBefore
          << " because thay are too small"
          << " -> still has " << m_waterCandidates.size()
          << " candidates." <<endl;

   // Sort again, since the area might have changed
   sort(m_waterCandidates.begin(), m_waterCandidates.end(),
        LessWaterAreaOrder());

   // Remove all water-items that are to small
   const uint32 MIN_SQ_AREA = 5000*5000;
   w = m_waterCandidates.begin();
   nbrWaterBefore = m_waterCandidates.size();
   while ( (w != m_waterCandidates.end()) && 
           (w->area > MIN_SQ_AREA)) {
      w++;
   }
   m_waterCandidates.erase(w, m_waterCandidates.end());
   mc2dbg1 << "Size (2): Removed " << nbrWaterBefore-m_waterCandidates.size()
           << " water items of " << nbrWaterBefore
           << " -> still has " << m_waterCandidates.size()
           << " candidates." <<endl;


   // Print all the water-items
   mc2dbg1 << "Remains with " << m_waterCandidates.size() << " candidates"
           << endl;
   k=0;
   for (waterIt w=m_waterCandidates.begin(); w<m_waterCandidates.end(); w++){
      mc2dbg8 << "   " << k++ << " " << w->name << " "
              << w->origIDs.begin()->origMapID << ":"
              << w->origIDs.begin()->origItemID << " "
              << w->area << " m^2" << endl;
   }

   // Add the items to this map.
   uint32 nbrAdded = 0;
   for (w=m_waterCandidates.begin(); w<m_waterCandidates.end(); w++){
      uint32 zoomLevel = 1;
      // Sort polygons of the water items.
      w->waterItems.front()->getGfxData()->sortPolygons();
      uint32 newID = addItem(w->waterItems.front(), zoomLevel);
      mc2dbg1 << "Added water item to co-map "
              << newID << " " << w->name << endl;

      // Save the original OldItemID and OldMapID of this item.
      nbrAdded++;
      for ( vector<originalIDs_t>::const_iterator it = w->origIDs.begin();
            it != w->origIDs.end();
            ++it ) {
         addedWaterOriginalIDs.insert( 
            pair<uint32, uint32> (it->origMapID, it->origItemID));
         mc2dbg8 << "  (mapID, itemID) = (" << it->origMapID << "," 
                 << it->origItemID << ")" << endl;
         
         // water items in co-map, do not exist in lower level map
         //         struct originalIDs_t origIDs;
         //         origIDs.origItemID = it->origItemID;
         //         origIDs.origMapID = it->origMapID;
         //         m_originalIDs.insert(make_pair(newID, origIDs));
      }
   }

   return (nbrAdded);
} // addWaterItems

uint32 
GMSCountryOverviewMap::addForestItems(
      multimap<uint32, uint32>& addedForestOriginalIDs )
{
   typedef vector<forestNotice_t>::iterator forestIt;
   typedef list<OldForestItem*>::iterator forestItemIt;

   mc2dbg << "GMSCountryOverviewMap::addForestItems Got "
          << m_forestCandidates.size() << " forestitems" << endl;

   // Sort the forest-items according to the size
   sort(m_forestCandidates.begin(), m_forestCandidates.end(),
        LessForestAreaOrder());

   // Remove all really small forest-items
   const uint32 FIRST_FILTER_MIN_SQ_AREA = 1000*1000;
   uint32 nbrBefore = m_forestCandidates.size();
   forestIt cand = m_forestCandidates.begin();
   while ( (cand != m_forestCandidates.end()) && 
           (cand->area > FIRST_FILTER_MIN_SQ_AREA)) {
      cand++;
   }
   m_forestCandidates.erase(cand, m_forestCandidates.end());
   mc2dbg1 << "GMSCountryOverviewMap::addForestItems:"
           << "Not adding: " << nbrBefore-m_forestCandidates.size() 
           << " items of " << nbrBefore
           << " because thay are too small"
           << " -> still has " << m_forestCandidates.size()
           << " candidates." <<endl;

   // Sort again, since the area might have changed ?
   sort(m_forestCandidates.begin(), m_forestCandidates.end(),
        LessForestAreaOrder());

   // Remove all forest-items that are too small again
   const uint32 MIN_SQ_AREA = 5000*5000;
   cand = m_forestCandidates.begin();
   nbrBefore = m_forestCandidates.size();
   while ( (cand != m_forestCandidates.end()) && 
           (cand->area > MIN_SQ_AREA)) {
      cand++;
   }
   m_forestCandidates.erase(cand, m_forestCandidates.end());
   mc2dbg1 << "Size (2): Removed " << nbrBefore-m_forestCandidates.size()
           << " items of " << nbrBefore
           << " -> still has " << m_forestCandidates.size()
           << " candidates." <<endl;



   // Print all the forest-items
   mc2dbg1 << "Remains with " << m_forestCandidates.size() << " candidates"
           << endl;
   uint32 k=0;
   for (forestIt cand=m_forestCandidates.begin(); 
        cand < m_forestCandidates.end(); cand++){
      mc2dbg8 << "   " << k++ << " " << cand->name << " "
              << cand->origIDs.begin()->origMapID << ":"
              << cand->origIDs.begin()->origItemID << " "
              << cand->area << " m^2" << endl;
   }

   // Add the items to this map.
   uint32 nbrAdded = 0;
   for (cand=m_forestCandidates.begin(); cand<m_forestCandidates.end(); cand++){
       uint32 zoomLevel = 1;
      // Sort polygons of the forest items.
      cand->forestItems.front()->getGfxData()->sortPolygons();
      uint32 newID = addItem(cand->forestItems.front(), zoomLevel);
      mc2dbg1 << "Added forest item to co-map "
              << newID << " " << cand->name << endl;

      // Save the original OldItemID and OldMapID of this item.
      nbrAdded++;
      for ( vector<originalIDs_t>::const_iterator it = cand->origIDs.begin();
            it != cand->origIDs.end();
            ++it ) {
         addedForestOriginalIDs.insert( 
            pair<uint32, uint32> (it->origMapID, it->origItemID));
         mc2dbg8 << "  (mapID, itemID) = (" << it->origMapID << "," 
                 << it->origItemID << ")" << endl;
         
         // forest items in co-map, do not exist in lower level map
         //         struct originalIDs_t origIDs;
         //         origIDs.origItemID = it->origItemID;
         //         origIDs.origMapID = it->origMapID;
         //         m_originalIDs.insert(make_pair(newID, origIDs));
      }
   }

   return (nbrAdded);
} // addForestItems

bool
GMSCountryOverviewMap::toIncludeItem(OldItem* origItem, 
                                     OldGenericMap* origMap,
                                     uint32& zoomLevel )
{
   // Make sure that origItem is ok
   if (origItem == NULL) {
      return false;
   }

   // Check the item
   switch (origItem->getItemType()) {
      case ItemTypes::municipalItem :
         // Municipals should always be included
         return true;

      case ItemTypes::builtUpAreaItem : {
         // Include all BUA's to make sure they are in the list in GfxClient?
         return true;
      }

      case ItemTypes::streetItem : {
         if (origItem->getGfxData() == NULL) {
            return false;
         }
         // Include if zoomlevel <= MAX_STREET_ZOOMLEVEL
         uint32 z = GET_ZOOMLEVEL(origItem->getID());
         return (z <= MAX_STREET_ZOOMLEVEL);
      }

      case ItemTypes::streetSegmentItem : {
         // Include if zoomlevel <= MAX_STREET_ZOOMLEVEL
         uint32 z = GET_ZOOMLEVEL(origItem->getID());
         // If the zoomlevel is high enough, and this ssi
         // is NOT a part of a street (ie. not included in any
         // streetitems), then we should include the ssi.
         return ((z <= (MAX_STREETSEGMENT_ZOOMLEVEL)) && 
                 (! origMap->isPartOfGfxStreet(origItem)));
      }

      case ItemTypes::waterItem :
         // Always include in temporary array
         return true;

      case ItemTypes::forestItem :
         // Always include in temporary array
         return true;

      case ItemTypes::pointOfInterestItem : {
         OldPointOfInterestItem* poi = 
            static_cast<OldPointOfInterestItem*> ( origItem );
         if ( poi->isPointOfInterestType( ItemTypes::cityCentre ) ) {
            if ( origMap->isUniqueCityCentre( origItem ) ) {
               zoomLevel = 1;
            }
            return true;
         } else if ( isMountainPassToAdd( origItem ) ) {
            zoomLevel = 1;
            return true;
         } else {
            return false;
         }
     }

      default:
         return false;

   } // end switch
   return false;
}

bool
GMSCountryOverviewMap::isMountainPassToAdd( OldItem* item )
{
   OldPointOfInterestItem* mntPass =
      dynamic_cast<OldPointOfInterestItem*>(item); 
   if ( ( mntPass != NULL ) && 
        ( mntPass->isPointOfInterestType( ItemTypes::mountainPass ) ) &&
        ( mntPass->getNbrNamesWithType( ItemTypes::officialName ) > 0 ) )
   {
      // Only checks that this is a mountain pass with a name for now.
      // Some other checks could be added here as well.
      return true;
   } else {
      return false;
   }
}

bool
GMSCountryOverviewMap::toIncludePolygon(OldItem* item, 
                                        uint16 poly)
{
   // Note that this method assumes that toIncludeItem() has already
   // returned true for this item.
   
   switch (item->getItemType()) {
      case (ItemTypes::streetItem) : {
         OldStreetItem* si = static_cast<OldStreetItem*> (item);
         // Check the ssi that represents this polygon.
         uint32 z = GET_ZOOMLEVEL(si->getItemInPolygon(poly));
         // The zoomlevel of streetsegmentitems are one higher than
         // the zoomlevel for streetitems...
         return (z <= MAX_STREETSEGMENT_ZOOMLEVEL );
      }
      default:
         return (true);
   }
   
   // Shouldn't reach this statement.
   return (true);
}


GfxDataFull*
GMSCountryOverviewMap::createNewGfxData(OldItem* origItem, 
                                        uint32 maxDist,
                                        vector<uint16>* skippedPolygons)
{

   GfxData* origGfx = origItem->getGfxData();
   // Create the gfxdata.
   GfxDataFull* newGfx = new GMSGfxData();
   
   // Filter all the polygons in the oigiGfx
   Stack filterIndex(origGfx->getNbrCoordinates(0));
   
   for (uint16 poly = 0; poly < origGfx->getNbrPolygons(); poly++) { 
      // Make additional check to see if this polygon should be included.
      // For now it's just street items that may have polygons with
      // "wrong" roadclass.

      if (toIncludePolygon(origItem, poly)) {

         // Get indices for the coordinates in the simplified polygon
         bool ok = false;
         if (maxDist != MAX_UINT32) {
            if (origGfx->closed()) {
               DEBUG4(cerr << "To call closedPolygonFilter" << endl);
               ok = origGfx->closedPolygonFilter(&filterIndex, poly, 
                                                 maxDist, MAX_UINT32);
            } else {
               DEBUG4(cerr << "To call openPolygonFilter" << endl);
               ok = origGfx->openPolygonFilter(&filterIndex, 
                                               poly, 
                                               maxDist, 
                                               MAX_UINT32);
            }
         }
         // Add a new polygon to the newGfx
         if (ok) {
            bool newPolygon = true;
            uint32 i = filterIndex.pop();
            uint32 n = 0;
            while (i < MAX_UINT32) {
               DEBUG4(cerr << "i = " << i << endl);
               newGfx->addCoordinate(origGfx->getLat(poly, i), 
                                     origGfx->getLon(poly, i), 
                                     newPolygon);
               newPolygon = false;
               i = filterIndex.pop();
               n++;
            }
            DEBUG4(cerr << "   Polygon " << poly << ", added " << n 
                        << " coordinates of " 
                        << origGfx->getNbrCoordinates(poly) 
                        << endl);
         } else {
            // Add the polygon unfiltered.
            bool newPolygon = true;
            for (uint32 i = 0; i < origGfx->getNbrCoordinates(poly); i++) {
               DEBUG4(cerr << "i = " << i << endl);
               newGfx->addCoordinate(origGfx->getLat(poly, i), 
                                     origGfx->getLon(poly, i), 
                                     newPolygon);
               newPolygon = false;
            }
         }
      } else {
         if (skippedPolygons != NULL) {
            skippedPolygons->push_back( poly );
         }
      }
   }
  
   // Update length.
   newGfx->updateLength();
   // Update the closed-member and return.
   newGfx->setClosed(0, origGfx->closed());
   DEBUG4(
      if (newGfx->closed())
         mc2dbg << "New polygon is closed!" << endl;
      else
         mc2dbg << "New polygon is NOT closed!" << endl;
   );

   return (newGfx);

}

bool 
GMSCountryOverviewMap::setSimplifiedGfxFromMap(
                                    GMSCountryOverviewMap* otherMap)
{
   if (otherMap->m_simplifiedGfxStack != NULL) {
      // Delete the current array.
      deleteSimplifiedGfxStack();
      
      m_nbrGfxPolygons = otherMap->m_nbrGfxPolygons;
      // Allocate array
      m_simplifiedGfxStack = new Stack**[
         OldCountryOverviewMap::NBR_SIMPLIFIED_COUNTRY_GFX];
      for (uint32 level = 0; 
           level < OldCountryOverviewMap::NBR_SIMPLIFIED_COUNTRY_GFX; 
           level++) {
         m_simplifiedGfxStack[level] = new Stack*[m_nbrGfxPolygons];
         for (uint32 poly = 0; poly < m_nbrGfxPolygons; poly++) {
            // Copy the stack
            m_simplifiedGfxStack[level][poly] = 
               new Stack(*otherMap->getFilterStack(level, poly));
         }
      }

      return (true);
   } else {
      return (false);
   }
}

bool
GMSCountryOverviewMap::copyCoPolFilteringFromMap(
         GMSCountryOverviewMap* otherMap, const char* breakPointsFileName)
{
   if ( ! otherMap->mapGfxDataIsFiltered() ) {
      mc2log << error << "copyCoPolFilteringFromMap: "
             << "Will not copy copol filtering from unfiltered co map"
             << endl;
      return false;
   }
   mc2log << info << "copyCoPolFilteringFromMap: Copy from other map "
          << otherMap->getMapID() << endl;
   
   // 1. Copy co pol (map gfx data) from otherMap
   // 2. Set map gfx data filtered
   // 3. copy gfx filter levels from otherMap, or create from scratch
   //    (equally fast)
   // 4. If break points file provided, create/add to countryBorders.txt
   //    if the parts of this co map that was not already in the border file.
   //  a. Read break points
   //  b. Find break points in co pol, loop for each part
   //    i.   Build borderGfx for the border part
   //    ii.  Build firstBreakPoint, lastBreakPoint, onTheWayPoint
   //    iii. Look for these in countryBorders.txt
   //         (use read-part of OldMapFilter::oldBorderFiltering)
   //    iv.  If not found, write to countryBorders.txt
   //

   // 1. & 2.
   const GfxData* otherGfx = otherMap->getGfxData();
   GfxDataFull* newGfx = GMSGfxData::createNewGfxData( static_cast<OldGenericMap *>( NULL ), otherGfx );
   setGfxData( newGfx );
   setMapGfxDataFiltered( true );
   mc2log << info << "copyCoPolFilteringFromMap: Country polygon copied"
          << endl;
   
   // 3.
   makeSimplifiedCountryGfx();
   mc2log << info << "copyCoPolFilteringFromMap: "
          << "Country polygon gfx filtering done" << endl;

   // 4.
   // Find country border parts (4 a. & 4 b i.)
   vector<GfxDataFull*> borderGfxs = 
      createBorderGfxs(breakPointsFileName, "copyCoPolFilteringFromMap");
   
   // Loop the vector and find out for each border part: if its is 
   // already present in the country borders file or if it should be added
   const char* countryMapName = this->getMapName();
   vector<GfxDataFull*>::iterator bit;
   for ( bit= borderGfxs.begin(); bit != borderGfxs.end(); bit++ ) {

      GfxData* borderGfx = *bit;
      
      // 4 b ii. identification coordinates (blanked)
      MC2Coordinate firstBreakPoint = MC2Coordinate(
            (borderGfx->getLat(0,0) & OldMapFilter::coordBlankMask),
            (borderGfx->getLon(0,0) & OldMapFilter::coordBlankMask) );
      uint32 lastIdx = borderGfx->getNbrCoordinates(0) -1;
      MC2Coordinate lastBreakPoint = MC2Coordinate( 
         (borderGfx->getLat(0,lastIdx) & OldMapFilter::coordBlankMask),
         (borderGfx->getLon(0,lastIdx) & OldMapFilter::coordBlankMask) );
      int32 lat, lon;
      uint16 offset = uint16( 0.5 * MAX_UINT16 );
      MC2Coordinate onTheWayPoint;
      if ( borderGfx->getCoordinate( offset, lat, lon ) ) {
         onTheWayPoint = MC2Coordinate( lat, lon );
      } else {
         onTheWayPoint = MC2Coordinate( 0, 0 );
      }
      mc2dbg2 << "Ident points: first=" << firstBreakPoint << " last="
              << lastBreakPoint << ", onTheWay=" << onTheWayPoint << endl;
      
      // 4 b iii.
      bool oldBorder = OldMapFilter::oldBorderFiltering(
         borderGfx, countryMapName, 
         firstBreakPoint, lastBreakPoint, onTheWayPoint,
         false, NULL, 0 );
         // gfx=NULL -> use the read-part of the method
         //  - check if the border part was already written to 
         //    countryBorders.txt
         //  - write the borderItems.txt file

      // 4 b iv.
      if ( !oldBorder ) {
         OldMapFilter::printCountryBorderToFile(
            firstBreakPoint, lastBreakPoint, onTheWayPoint,
            countryMapName, borderGfx,
            0, 0, borderGfx->getNbrCoordinates(0));
      }
   }
   
   return true;
}

vector<GfxDataFull*>
GMSCountryOverviewMap::createBorderGfxs(
      const char* breakPointsFileName, const char* caller)
{
   // Vector with gfxdatas to return
   vector<GfxDataFull*> borderGfxs;
   
   if ( ! mapGfxDataIsFiltered() ) {
      mc2log << error << "createBorderGfxs: Can not create border gfxs"
             << " - map gfx data is not filtered in coord levels" << endl;
      exit(1);
   }
   
   // extract break points
   if ( breakPointsFileName == NULL ) {
      mc2log << error << "createBorderGfxs: No break points file "
             << "given - can't create border gfxs for "
             << caller << endl;
      exit(1);
   }
   set<MC2Coordinate> breakPoints;
   GMSUtility::extractCoPolBreakPoints( breakPointsFileName, breakPoints );
   mc2dbg1 << "createBorderGfxs: Extracted " << breakPoints.size()
           << " country polygon break points from file." << endl;
   // blank out last bits
   set<MC2Coordinate> blankedBreakPoints;
   typedef set<MC2Coordinate>::const_iterator setIt;
   for ( setIt it = breakPoints.begin(); it != breakPoints.end(); it++ ) {
      MC2Coordinate coord( it->lat & OldMapFilter::coordBlankMask,
                           it->lon & OldMapFilter::coordBlankMask );
      blankedBreakPoints.insert( coord );
      mc2dbg4 << " blanked break point " << coord << endl;
   }
   
   // Loop my map gfx and find break points to create border gfxs.
   const GfxData* myGfx = getGfxData();
   for ( uint32 p = 0; p < myGfx->getNbrPolygons(); p++ ) {
      
      // Look for one break point in this co pol polygon
      GfxData::const_iterator end = myGfx->polyEnd( p );
      GfxData::const_iterator it = myGfx->polyBegin( p );
      uint32 startIdx = 0;
      bool found = false;
      while ( !found && ( it != end) ) {
         MC2Coordinate gfxCoord( it->lat & OldMapFilter::coordBlankMask,
                                 it->lon & OldMapFilter::coordBlankMask );
         setIt bpCoord = blankedBreakPoints.find( gfxCoord );
         if ( bpCoord != blankedBreakPoints.end() ) {
            found = true;
         } else {
            it++;
            startIdx++;
         }
      }
      
      if ( found ) { // There was a break point in map gfx data
         mc2dbg2 << "Found first break point poly=" << p << " startIdx="
                 << startIdx << " coord=" << *it << endl;
      
         // Identify country border parts starting from startIdx,
         // ending at startIdx
         uint32 myNbrCoords = myGfx->getNbrCoordinates(p);
         bool moreBorders = true;
         uint32 nbrBorders = 0;
         uint32 coordIdx = startIdx;
         while ( moreBorders ) {  // for each border part

            // Build borderGfx (true filtered coords)
            GfxDataFull* borderGfx = GMSGfxData::createNewGfxData( static_cast<OldGenericMap *>( NULL ) );
            MC2Coordinate gfxCoord(
                  myGfx->getLat(p, coordIdx), myGfx->getLon(p, coordIdx) );
            MC2Coordinate blankedCoord(
                  (gfxCoord.lat & OldMapFilter::coordBlankMask),
                  (gfxCoord.lon & OldMapFilter::coordBlankMask) );
            if ( blankedBreakPoints.find(blankedCoord) == 
                     blankedBreakPoints.end() ) {
               // startIdx (or later break points) is not among break points!
               cout << " something is wrong.." << endl;
            }
            mc2dbg4 << "begin of border part " << coordIdx << " "
                    << gfxCoord << endl;
            borderGfx->addCoordinate(gfxCoord.lat, gfxCoord.lon, true);
            bool borderPartDone = false;
            while ( !borderPartDone ) {
               coordIdx = (coordIdx+1) % (myNbrCoords);
               MC2Coordinate gfxCoord(myGfx->getLat(p, coordIdx),
                                      myGfx->getLon(p, coordIdx) );
               borderGfx->addCoordinate(gfxCoord.lat, gfxCoord.lon);
               
               // check if we have reached next breakpoint, thus this
               // border part is completed.
               MC2Coordinate blankedCoord(
                     (gfxCoord.lat & OldMapFilter::coordBlankMask),
                     (gfxCoord.lon & OldMapFilter::coordBlankMask) );
               if ( blankedBreakPoints.find(blankedCoord) != 
                        blankedBreakPoints.end() ) {
                  mc2dbg4 << "end of border part " << coordIdx << " "
                          << gfxCoord << endl;
                  borderPartDone = true;
               }
            }
            borderGfx->updateLength();
            borderGfx->setClosed(0, false);
            mc2dbg2 << "Built borderGfx with "
                    << borderGfx->getNbrCoordinates(0) << " coords" << endl;
            nbrBorders++;
            borderGfxs.push_back( borderGfx );
            
            if ( coordIdx == startIdx ) {
               moreBorders = false;
            }
         }
         mc2log << info << "createBorderGfxs: Country polygon "
                << this->getMapName() << " poly " << p << " has "
                << nbrBorders << " border parts" << endl;
      
      } // if break point in poly
      
   } // for polygons


   return borderGfxs;
}

uint32
GMSCountryOverviewMap::createBorderItems(const char* breakPointsFileName)
{
   mc2dbg1 << "GMSCountryOverviewMap::createBorderItems map " 
           << getMapName() << endl;
   
   uint32 nbrCreated = 0;

   if ( ! this->mapGfxDataIsFiltered() ) {
      mc2log << warn << "This co map has unfiltered country polygon"
             << " - can not create border items" << endl;
      return nbrCreated;
   }

   // Read the borderItems.txt file to collect which border parts
   // that should be added as border items
   vector< MC2Coordinate > firstCoords;
   vector< MC2Coordinate > lastCoords;
   vector< MC2Coordinate > middleCoords;
   ifstream biFile( "borderItems.txt" );
   if ( ! biFile ) {
      mc2log << warn << "No borderItems.txt file to read" << endl;
      return nbrCreated;
   }
   else {
      const int maxLineLength = 200;
      char buf[maxLineLength];
      buf[0] = '\0';
      
      biFile >> buf;
      
      while ( !biFile.eof() && (strlen(buf) > 0) ) {
         if ( strstr(buf, "BORDERITEM") != NULL ) {
            MC2Coordinate coord;
            // read the border part id
            biFile >> buf;
            // read the first coord
            biFile >> buf; // "from"
            biFile >> buf;
            MapFilterUtil::coordFromString( buf, coord );
            firstCoords.push_back( coord );
            // read the last coord
            biFile >> buf; // "to"
            biFile >> buf;
            MapFilterUtil::coordFromString( buf, coord );
            lastCoords.push_back( coord );
            // read the middle coord
            biFile >> buf; // "via"
            biFile >> buf;
            MapFilterUtil::coordFromString( buf, coord );
            middleCoords.push_back( coord );
         }
         biFile >> buf;
      }
   }
   mc2dbg1 << "Read " << firstCoords.size() 
           << " border items from file" << endl;

   // Find country border parts in this co map using the break points file
   vector<GfxDataFull*> borderGfxs = 
         createBorderGfxs( breakPointsFileName, "createBorderItems" );
   mc2dbg1 << "Created " << borderGfxs.size() << " border gfxs" << endl;

   // Loop the border part gfx vector and find out for each border part:
   //    if it is marked in the borderItems.txt file create a border item.
   vector<GfxDataFull*>::iterator bit;
   for ( bit= borderGfxs.begin(); bit != borderGfxs.end(); bit++ ) {
      GfxDataFull* borderGfx = *bit;

      // The coordinates in the borderItem file are blanked...
      // Blank the coords in this border part to be able to match them
      GfxData::const_iterator it = borderGfx->polyBegin(0);
      MC2Coordinate checkFirst( it->lat & OldMapFilter::coordBlankMask,
                                it->lon & OldMapFilter::coordBlankMask );
      it = borderGfx->polyEnd(0); it--;
      MC2Coordinate checkLast( it->lat & OldMapFilter::coordBlankMask,
                               it->lon & OldMapFilter::coordBlankMask );
      mc2dbg8 << "borderGfx " << checkFirst << " " << checkLast << endl;
      bool addBorder = false;
      uint32 c = 0;
      while ( !addBorder && ( c < firstCoords.size()) ) {
         mc2dbg8 << "borderItem " << c << " " << firstCoords[c] << " " 
                 << lastCoords[c] << " " << middleCoords[c] << endl;
         if ( ((checkFirst == firstCoords[c]) && 
                        (checkLast == lastCoords[c])) ||
              ((checkFirst == lastCoords[c]) &&
                        (checkLast == firstCoords[c])) ) {
            // The first/last matches, check if middleCoord is close enough
            float64 sqDist;
            bool ok = OldMapFilter::coordCloseToBorderPart(
                              borderGfx, middleCoords[c], sqDist);
            if ( ok ) {
               addBorder = true;
            }
         }
         c++;
      }

      if ( addBorder ) {
         OldItem* borderItem = new OldItem( ItemTypes::borderItem, MAX_UINT32 );
         borderItem->setGfxData( borderGfx );
      
         uint32 zoomLevel = 4;
         uint32 newID = addItem( borderItem, zoomLevel);
         if ( newID != MAX_UINT32 ) {

            mc2log << info << "Added borderItem " << newID << endl;
            nbrCreated++;
         }
      }
   }
   
   return nbrCreated;

}

bool
GMSCountryOverviewMap::mergeStreetSegments()
{
   // Init the IDsByOriginalIDsTable in case it's empty.
   if ( m_idsByOriginalIDs.empty() ) {
      buildIDsByOriginalIDsTable();
   }
   
   // Set containing the processed SSI.
   set<uint32>processedSSI;

   // One set of merged node id:s.
   typedef list<uint32> mergedNodes_t;
   
   // Vector with all the merged SSI
   vector<mergedNodes_t> allMergedNodes( getNbrItems() );
   
   // Check which items that can be merged. 
   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; ++z ) {
      for (uint32 i = 0; i < getNbrItemsWithZoom( z ); ++i ) {
         OldStreetSegmentItem* ssi = 
            dynamic_cast<OldStreetSegmentItem*> ( getItem( z, i ) );
         if ( ( ssi != NULL ) && 
              ( processedSSI.find( ssi->getID() ) == 
                processedSSI.end() ) ) {
            // The SSI is not yet processed.

            mergedNodes_t mergedNodes;
           
            mergedNodes.push_back( ssi->getID() );
            processedSSI.insert( ssi->getID() );

            // Merge 
            mc2dbg4 << "--------------------------" << endl;
            mc2dbg4 << "--- SSI to merge (node 0) " << ssi->getID() << endl;
            mergeSSI( mergedNodes, processedSSI, 
                      ssi, 0, false );
            mc2dbg4 << "--- SSI to merge (node 1) " << ssi->getID() << endl;
            mergeSSI( mergedNodes, processedSSI, 
                      ssi, 1, true );

            // Add to allMergedNodes.
            allMergedNodes.push_back( mergedNodes );
         }
      }
   }

   mc2dbg1 << "--- Ready to do actual merging ---" << endl;
   
   // Merge items.
   for ( vector<mergedNodes_t>::iterator it = allMergedNodes.begin();
         it != allMergedNodes.end(); ++it ) {
      if ( it->size() > 1 ) {
         mc2dbg4 << "--------------" << endl;
         mc2dbg4 << "Original item " << it->front() << endl;
         // Original item.
         OldStreetSegmentItem* origItem = static_cast<OldStreetSegmentItem*>
            ( itemLookup( it->front() ) );
         // New merged item.
         OldStreetSegmentItem* mergedItem = 
            new GMSStreetSegmentItem( this, origItem->getID() );
         // Copy essential attributes.
         mergedItem->setRoadClass( origItem->getRoadClass() );
         for ( uint32 i = 0; i < origItem->getNbrNames(); ++i ) {
            mergedItem->addName( origItem->getNameLanguage( i ),
                                 origItem->getNameType( i ),
                                 origItem->getStringIndex( i ) );
         }
         
         // Create gfxdata with first polygon created.
         GfxDataFull* gfx = GMSGfxData::createNewGfxData( this, true );
         
         for ( list<uint32>::iterator jt = it->begin(); jt != it->end();
               ++jt ) {
            uint32 nodeID = *jt;
            OldStreetSegmentItem* ssi = static_cast<OldStreetSegmentItem*>
               ( itemLookup( nodeID & 0x7fffffff ) );
               
            if ( ( nodeID & 0x7fffffff ) == nodeID ) {
               // OldNode 0, add gfxdata forward.
               gfx->add( ssi->getGfxData() );
               mc2dbg4 << "Adding item " << ssi->getID() 
                      << " in normal order." 
                      << endl; 
            } else {
               // OldNode 1, add gfxdata backwards
               gfx->add( ssi->getGfxData(), true );
               mc2dbg4 << "Adding item " << ssi->getID() 
                      << " in reversed order." 
                      << endl; 
            }
            // Remove the old item. Don't use removeItem() since the
            // connections are corrupt.
            m_itemsZoom[ GET_ZOOMLEVEL( ssi->getID() ) ]
                       [ GET_INDEXINZOOM( ssi->getID() ) ] = NULL;
         }
         
         // Remove the duplicate coordinates.
         gfx->removeIdenticalCoordinates();

         // Set gfxdata to merged item.
         mergedItem->setGfxData( gfx );

         // Add item to the map.
         addItem( mergedItem, GET_ZOOMLEVEL( mergedItem->getID() ) );
         mc2dbg4 << "Adding merged item as id " << mergedItem->getID() 
                << endl;
      }
   }
   
   return true;
}

void
GMSCountryOverviewMap::mergeSSI( list<uint32>& mergedNodes,
                                 set<uint32>& processedSSI,
                                 OldStreetSegmentItem* ssi,
                                 byte nodeNbr,
                                 bool forward ) 
{
   
   OldNode* node = ssi->getNode( nodeNbr );

   uint32 nbrRealConnections = 0;
   uint32 otherNodeID = MAX_UINT32;
   
   // Note that the connections are referring to ids in the original maps.
   for ( uint32 i = 0; i < node->getNbrConnections(); ++i ) {
      OldConnection* conn = node->getEntryConnection( i );
      uint32 originalNodeID = conn->getConnectFromNode();
      
      uint32 origMapID, foo;
      getOriginalIDs( ssi->getID(), origMapID, foo );
      uint32 itemID = getOverviewIDFast( origMapID, 
                                     originalNodeID & 0x7fffffff );
      if ( itemLookup( itemID ) != NULL ) {
         // The item really existed, so the connection is real.
         ++nbrRealConnections;
         otherNodeID = itemID;
         if ( ( originalNodeID & 0x7fffffff ) != originalNodeID ) {
            // OldNode 1.
            otherNodeID = otherNodeID ^ 0x80000000;
         }
      }
   }
   
   if ( ( nbrRealConnections == 1 ) && 
        ( processedSSI.find( otherNodeID & 0x7fffffff ) == 
          processedSSI.end() ) ) {
      
      OldStreetSegmentItem* otherSSI = dynamic_cast<OldStreetSegmentItem*> 
         ( itemLookup( otherNodeID & 0x7fffffff ) );
      if ( otherSSI != NULL ) {
         // Check that the attributes and names are the same.
         if ( ( ssi->getRoadClass() == otherSSI->getRoadClass() ) &&
              ( ssi->hasCommonName( otherSSI ) ) ) {
            // OK. Same attributes.
            // Add to list.
            if ( forward ) {
               mc2dbg4 << "Merging " << ( otherNodeID ^ 0x80000000 )
                      << " to back." << endl;
               mergedNodes.push_back( otherNodeID ^ 0x80000000 );
            } else {
               mergedNodes.push_front( otherNodeID );
               mc2dbg4 << "Merging " << otherNodeID << " to front." << endl;
            }
            // Add as being processed.
            processedSSI.insert( otherNodeID & 0x7fffffff );
            byte otherNodeNbr = 1;
            if ( ( otherNodeID & 0x7fffffff ) == otherNodeID ) {
               otherNodeNbr = 0;
            }
            // Continue merging.
            mergeSSI( mergedNodes, processedSSI, 
                      otherSSI, otherNodeNbr, forward );
         }
      }
   } else {
      mc2dbg4 << "Not merging " << (otherNodeID & 0x7fffffff) << " since ";
      if ( nbrRealConnections != 1 ) {
         mc2dbg4 << nbrRealConnections 
                << " connections where found." << endl;
      } else {
         mc2dbg4 << "item already present in processed list." << endl;
      }
   }
}

void
GMSCountryOverviewMap::buildIDsByOriginalIDsTable()
{
   m_idsByOriginalIDs.clear();
   for ( map<uint32, struct originalIDs_t>::iterator p = 
            m_originalIDs.begin(); p != m_originalIDs.end(); ++p ) {
      m_idsByOriginalIDs.insert( make_pair( p->second, p->first ) );
   }
}


void
GMSCountryOverviewMap::updateNames(OldItem* newItem,
                                   OldItem* otherItem, OldGenericMap* otherMap )
{
   // Update the name
   mc2dbg8 << "updating names for item " << newItem->getID()
           << " " << getFirstItemName(newItem) << ", has " 
           << int(newItem->getNbrNames()) << " names" << endl;
   
   newItem->removeAllNames();
   for (uint32 j=0; j<otherItem->getNbrNames(); j++) {
      const char* name = otherMap->getName(otherItem->getStringIndex(j));      
      addNameToItem(newItem,
                    name,
                    otherItem->getNameLanguage(j),
                    otherItem->getNameType(j));
   }
   mc2dbg8 << "updated names for item " << newItem->getID() << " to " 
           << getFirstItemName(newItem) << ", has " <<
           int(newItem->getNbrNames())
           << " names" << endl;
}

void
GMSCountryOverviewMap::removeUnwantedGroups(
                  OldItem* newItem, OldGenericMap* otherMap)
{
   // Remove all groups except for buas and municipals
   // The groupids are underview ids why otherMap is needed.
   int nbrGroups = newItem->getNbrGroups() - 1;
   for ( int i = nbrGroups; i >= 0; --i ) {
      OldItem* group = otherMap->itemLookup( newItem->getGroup( i ) );
      if ( ( group != NULL ) && 
           ( ( group->getItemType() == ItemTypes::municipalItem ) || 
             ( group->getItemType() == ItemTypes::builtUpAreaItem ) ) ) {
         // Keep group.
      } else {
         newItem->removeGroup( i );
      }
   }
}

void
GMSCountryOverviewMap::addGroupsToPoi(
            OldItem* newItem, OldItem* otherItem, OldGenericMap* otherMap)
{
   // Add bua and mun groups to pois
   
   OldPointOfInterestItem* poi = 
      dynamic_cast<OldPointOfInterestItem*> ( otherItem );
   if ( poi != NULL ) {
      // Get the streetsegment for the poi.
      OldItem* ssi = otherMap->itemLookup( poi->getStreetSegmentItemID() );
      if ( ssi != NULL ) {

         // The region types to add.
         set<ItemTypes::itemType> regionTypes;
         regionTypes.insert( ItemTypes::municipalItem );
         regionTypes.insert( ItemTypes::builtUpAreaItem );
         // Add the regions.
         for ( set<ItemTypes::itemType>::const_iterator it =
               regionTypes.begin(); it != regionTypes.end(); ++it ) {
            for ( uint32 i = 0; 
                  i < otherMap->getNbrRegions( ssi, *it );
                  ++i ) {
               addRegionToItem( newItem, 
                     otherMap->getRegionIDIndex( ssi, i, *it ) );
            }
         }
      }
   } 
}

bool
GMSCountryOverviewMap::applyChangesFromOtherMap(
      OldGenericMap* otherMap, vector<uint32> itemsAffectedInOtherMap,
      bool& coMapChanged)
{
   mc2dbg1 << "GMSCOMap applyChangesFromOtherMap, nbr affected items " 
           << itemsAffectedInOtherMap.size() << endl;
   coMapChanged = false;
   if (itemsAffectedInOtherMap.size() == 0) {
      return true;
   }
   
   if ( m_idsByOriginalIDs.empty() ) {
      buildIDsByOriginalIDsTable();
   }
   
   // Check if otherid exists in the ovmap
   //    Look up otheritem in other map
   //    If otheritem is not present in other map it was removed (e.g. poi)
   //       Remove also in co map
   //       If bua or city centre
   //          updateUniqueNames
   //    Else if not ssi or street
   //       Update "attributes" (name, gfxdata, conn, ...)
   //       If bua and name updated
   //          updateUniqueNames
   // If not use existing functions to determine if it should be (new poi)
   //    Use existing functions to add it to comap inlcuding groups,
   //    updating lookuptables etc.
   //    Update groups etc
   //    If citycentre or bua
   //       updateUniqueNames
  
   
   uint32 otherItemNbr = 0;
   uint32 otherMapId = otherMap->getMapID();
   vector<uint32>::const_iterator it;
   for (it = itemsAffectedInOtherMap.begin();
        it != itemsAffectedInOtherMap.end(); it++) {
      
      uint32 otherItemId = (*it);
      OldItem* otherItem = otherMap->itemLookup(otherItemId);
      mc2dbg1 << "aCFOM, trying otherItem nbr " << otherItemNbr << ":" 
              << otherMapId << "." << otherItemId << endl;
      uint32 overviewId = getOverviewIDFast(otherMapId, otherItemId);
      if (overviewId != MAX_UINT32) {
         // otherItem exists in co map
         if ((otherItem == NULL) ||
             (otherItem->getItemType() == ItemTypes::nullItem)) {
            // otherItem has been removed from otherMap
            mc2dbg1 << "aCFOM, otherItem was removed from otherMap, " 
                    << "removing co item " << overviewId << endl;
            removeItem(overviewId);
            coMapChanged = true;

            // If this was a bua or a city centre, check if unique names 
            // should be removed from any items with the same official name
            
         }
         else if (otherItem->getItemType() != ItemTypes::streetSegmentItem) {
            // Streets ands ferries do not exist in co-map
            // Ssis are merged, therefore we can't update them correctly
            
            // Update the item in the co map
            OldItem* overviewItem = itemLookup(overviewId);
            if (overviewItem != NULL) {
               mc2dbg1 << "aCFOM, updating attributes for co item " 
                       << overviewId << endl;

               // update attributes of the country overview item
               // including names, gfxdata, remove unwanted groups and
               // make sure groupids are overview ids.
               updateAttributes(overviewItem, otherItem, otherMap);

               // If bua or citycentre and name changed update unique names
               // Municipal group ids must be overview ids first!
               updateUniqueNames(overviewItem);
               
               coMapChanged = true;
            }
         }
         else {
            mc2dbg8 << "aCFOM, do not update ssi" << endl;
         }

      } else {
         // find zoom from otherMap
         uint32 otherZoomLevel = (otherItemId & 0x78000000) >> 27;
         // The otherItem does not exist in co map, check if it is 
         // a new item that should be included in co map (poi)
         mc2dbg1 << "aCFOM, otherItem does NOT exist in co map " << endl;
         if ((otherItem == NULL) ||
             (otherItem->getItemType() == ItemTypes::nullItem)) {
            // The other item was perhaps removed from the other map
            // The item could else be a street item that was re-generated
            // (in EDR: removed all si and created new ones)
            //
            // In any other case ending up here means that something is wrong
            // and that the provided item id is invalid.
            mc2dbg1 << "The otherItem " << otherItemId
                    << " exists neither in co map nor in other map" << endl;
         } 
         else if ((otherItem != NULL) &&
                  toIncludeItem(otherItem, otherMap, otherZoomLevel)) {
            mc2dbg2 << "aCFOM, otherItem should be added to co map" << endl;
            
            // gfx data from otherMap might be needed if not poi or water
            GfxData* otherMapGfx = NULL;
            ItemTypes::itemType otherItemType = otherItem->getItemType();
            if ((otherItemType != ItemTypes::pointOfInterestItem) &&
                (otherItemType != ItemTypes::waterItem) &&
                (otherItemType != ItemTypes::forestItem))
               otherMapGfx = GMSGfxData::createNewGfxData(
                                 NULL, otherMap->getGfxData());
            OldItem* newItem = addItemToCountryOverview(
                        otherItem, otherMap, otherZoomLevel, otherMapGfx);
            // a star '*' was added to city centre name (zoomlevel 1)
            if (newItem != NULL) {
               mc2dbg1 << "aCFOM, otherItem added to co map, newId "
                       << newItem->getID() << endl;
               coMapChanged = true;
               
               // Lookuptable m_originalIDs is ok
               // buildIDsByOriginalIDsTable?? don't know if needed
               // Groups are underview ids, update to overview ids
               updateGroupIDs(newItem);
               
               // Update unique names if added bua or poi city centre
               // Municipal group ids must be overview ids first!
               updateUniqueNames(newItem);
            }
         }
      }
      
      otherItemNbr++;
   }

   
   return true;
}

void
GMSCountryOverviewMap::updateUniqueNames(OldItem* checkItem)
{
   // Update unique names if checkItem is 
   // a bua or poi with poi type cityCentre
   
   ItemTypes::itemType itType = checkItem->getItemType();
   
   if ((itType == ItemTypes::builtUpAreaItem) ||
       ((itType == ItemTypes::pointOfInterestItem) &&
        ( static_cast<OldPointOfInterestItem*>(checkItem)->
            isPointOfInterestType(ItemTypes::cityCentre))) ) {

      ItemTypes::pointOfInterest_t poiType = ItemTypes::nbr_pointOfInterest;
      if (itType == ItemTypes::pointOfInterestItem) {
         poiType = static_cast<OldPointOfInterestItem*>
                   (checkItem)->getPointOfInterestType();
      }

      makeUniqueNames(itType, poiType, checkItem);
   }
}

void
GMSCountryOverviewMap::updateAttributes(OldItem* overviewItem,
      OldItem* otherItem, OldGenericMap* otherMap)
{
   mc2dbg4 << "GMSCOMap updateAttributes" << endl;

   // Let the item update itself for most attributes
   overviewItem->updateAttributesFromItem(otherItem, false); // not same map

   // GfxData not handled in item for overviews.
   // At least handle pois since they could have moved in WASP
   if ((overviewItem->getItemType() == ItemTypes::pointOfInterestItem) &&
       (otherItem->getItemType() == ItemTypes::pointOfInterestItem)) {
      GfxData* myGfx = overviewItem->getGfxData();
      GfxData* otherGfx = otherItem->getGfxData();
      GfxDataFull* newGfx = 
         GMSGfxData::createNewGfxData(NULL, true); // new polygon
      if (otherGfx == NULL) {
         // The other item had no gfxData, create it in the same 
         // way as in addItemToCountryOverview-method.
         OldPointOfInterestItem* poi = 
            static_cast<OldPointOfInterestItem*> ( overviewItem ); 
         OldItem* ssi = otherMap->itemLookup( poi->getStreetSegmentItemID() );
         int32 lat, lon;
         ssi->getGfxData()->getCoordinate( 
               poi->getOffsetOnStreet(), lat, lon );
         newGfx->addCoordinate( lat, lon );
      }
      else {
         int32 lat = otherGfx->getLat(0,0);
         int32 lon = otherGfx->getLon(0,0);
         newGfx->addCoordinate( lat, lon );
      }

      if ( (myGfx == NULL) ||
           ((myGfx != NULL) && !myGfx->equals(newGfx)) ) {
         overviewItem->setGfxData( newGfx );
      }
   }
   
   // update names
   updateNames(overviewItem, otherItem, otherMap);
   
   // Groups were updated in item. NB underview ids.
   // Remove groups that should not be present in co-map
   // (keep bua and mun groups)
   removeUnwantedGroups(overviewItem, otherMap);
   // Add groups to pois to enable unique names (underview ids)
   addGroupsToPoi(overviewItem, otherItem, otherMap);
   // Update ids to overview ids for this item since ids
   // were set to uv-ids in OldItem::updateAttributesFromItem
   updateGroupIDs(overviewItem);

   // connfromids + signpost names (strIndex)
   // NO CONNS IN COMAP
}

