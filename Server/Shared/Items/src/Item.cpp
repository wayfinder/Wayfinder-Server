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

#include "DataBuffer.h"

#include "Item.h"
#include "BuildingItem.h"
#include "BuiltUpAreaItem.h"
#include "BusRouteItem.h"
#include "CategoryItem.h"
#include "CityPartItem.h"
#include "FerryItem.h"
#include "ForestItem.h"
#include "GroupItem.h"
#include "IslandItem.h"
#include "MunicipalItem.h"
#include "NullItem.h"
#include "ParkItem.h"
#include "PointOfInterestItem.h"
#include "RailwayItem.h"
#include "RouteableItem.h"
#include "StreetItem.h"
#include "StreetSegmentItem.h"
#include "WaterItem.h"
#include "ZipCodeItem.h"
#include "ZipAreaItem.h"
#include "AirportItem.h"
#include "AircraftRoadItem.h"
#include "PedestrianAreaItem.h"
#include "IndividualBuildingItem.h"
#include "SubwayLineItem.h"
#include "CartographicItem.h"
#include "OverviewMap.h"
#include "ItemAllocator.h"

#include "MC2String.h"

#include "GfxDataFull.h"
#include "GenericMap.h"
#include "ItemNames.h"

#include "AllocatorTemplate.h"

void 
Item::setGfxData( GfxData* gfx, bool deleteOld)
{
   if (deleteOld)
      delete m_gfxData;
   m_gfxData = gfx;
}

uint32 
Item::getLength() const
{
   if (m_gfxData != NULL)
      return ((uint32) m_gfxData->getLength(0));
   else
      return MAX_UINT32;
}

GroupItem*
Item::groupItem( Item* maybeGroup )
{
   if ( maybeGroup == NULL ) {
      return NULL;
   }
   switch ( maybeGroup->getItemType() ) {
      case ItemTypes::builtUpAreaItem:
      case ItemTypes::categoryItem:
      case ItemTypes::streetItem:
      case ItemTypes::zipAreaItem:
      case ItemTypes::zipCodeItem:
         return static_cast<GroupItem*>( maybeGroup );
      default:
         return NULL;
   }
   // Different compilers complain about different things.
   return NULL;
}

RouteableItem*
Item::routeableItem( Item* maybeRouteable )
{
   if ( maybeRouteable == NULL ) {
      return NULL;
   }
   switch ( maybeRouteable->getItemType() ) {
      case ItemTypes::streetSegmentItem :
      case ItemTypes::ferryItem :
      case ItemTypes::busRouteItem:
         // It is routeable
         return static_cast<RouteableItem*>( maybeRouteable );
      default:
         return NULL;
   }
   // Different compilers complain about different things.
   return NULL;
}

Item*
Item::itemOfType( Item* item,
                  ItemTypes::itemType wantedType )
{
   if ( item == NULL || item->getItemType() == wantedType ) {
      return item;
   } 
   return NULL;
}

Item* Item::createItemFromType( ItemTypes::itemType type, GenericMap& theMap, 
                                uint32 itemID ) 
{
   // Create the new item, fill it with data and return it
   Item* item = NULL;
   switch (type) {
   case ItemTypes::streetSegmentItem :
      item = dynamic_cast< MC2Allocator<StreetSegmentItem> &>
         (*theMap.m_streetSegmentItemAllocator).getNextObject();
      break;

   case ItemTypes::streetItem:
      item = dynamic_cast< MC2Allocator<StreetItem> &>
         (*theMap.m_streetItemAllocator).getNextObject();
      break;

   case ItemTypes::municipalItem : 
      item = dynamic_cast< MC2Allocator<MunicipalItem> &>
         (*theMap.m_municipalItemAllocator).getNextObject();
      break;
      
   case ItemTypes::cityPartItem :
      item = dynamic_cast< MC2Allocator<CityPartItem> &>
         (*theMap.m_cityPartItemAllocator).getNextObject();
      break;

   case ItemTypes::waterItem :
      item = dynamic_cast< MC2Allocator<WaterItem> &>
         (*theMap.m_waterItemAllocator).getNextObject();
      break;

   case ItemTypes::parkItem :
      item = dynamic_cast< MC2Allocator<ParkItem> &>
         (*theMap.m_parkItemAllocator).getNextObject();
      break;

   case ItemTypes::forestItem :
      item = dynamic_cast< MC2Allocator<ForestItem> &>
         (*theMap.m_forestItemAllocator).getNextObject();
      break;

   case ItemTypes::buildingItem :
      item = dynamic_cast< MC2Allocator<BuildingItem> &>
         (*theMap.m_buildingItemAllocator).getNextObject();
      break;

   case ItemTypes::railwayItem : 
      item = dynamic_cast< MC2Allocator<RailwayItem> &>
         (*theMap.m_railwayItemAllocator).getNextObject();
      break;

   case ItemTypes::islandItem :
      item = dynamic_cast< MC2Allocator<IslandItem> &>
         (*theMap.m_islandItemAllocator).getNextObject();
      break;

   case ItemTypes::nullItem :
      item = new NullItem(itemID);
      break;

   case ItemTypes::zipCodeItem :
      item = dynamic_cast< MC2Allocator<ZipCodeItem> &>
         (*theMap.m_zipCodeItemAllocator).getNextObject();
      break;

   case ItemTypes::zipAreaItem :
      item = dynamic_cast< MC2Allocator<ZipAreaItem> &>
         (*theMap.m_zipAreaItemAllocator).getNextObject();
      break;

   case ItemTypes::pointOfInterestItem :
      item = dynamic_cast< MC2Allocator<PointOfInterestItem> &>
         (*theMap.m_pointOfInterestItemAllocator).getNextObject();
      break;

   case ItemTypes::categoryItem :
      item = dynamic_cast< MC2Allocator<CategoryItem> &>
         (*theMap.m_categoryItemAllocator).getNextObject();
      break;

   case ItemTypes::builtUpAreaItem :
      item = dynamic_cast< MC2Allocator<BuiltUpAreaItem> &>
         (*theMap.m_builtUpAreaItemAllocator).getNextObject();
      break;

   case ItemTypes::busRouteItem :
      item = dynamic_cast< MC2Allocator<BusRouteItem> &>
         (*theMap.m_busRouteItemAllocator).getNextObject();
      break;

   case ItemTypes::airportItem :
      item = dynamic_cast< MC2Allocator<AirportItem> &>
         (*theMap.m_airportItemAllocator).getNextObject();
      break;

   case ItemTypes::aircraftRoadItem : 
      item = dynamic_cast< MC2Allocator<AircraftRoadItem> &>
         (*theMap.m_aircraftRoadItemAllocator).getNextObject();
      break;

   case ItemTypes::pedestrianAreaItem :
      item = dynamic_cast< MC2Allocator<PedestrianAreaItem> &>
         (*theMap.m_pedestrianAreaItemAllocator).getNextObject();
      break;

   case ItemTypes::militaryBaseItem :
      item = dynamic_cast< MC2Allocator<Item> &>
         (*theMap.m_militaryBaseItemAllocator).getNextObject();
      break;

   case ItemTypes::individualBuildingItem :
      item = dynamic_cast< MC2Allocator<IndividualBuildingItem> &>
         (*theMap.m_individualBuildingItemAllocator).getNextObject();
      break;

   case ItemTypes::subwayLineItem :
      item = dynamic_cast< MC2Allocator<SubwayLineItem> &>
         (*theMap.m_subwayLineItemAllocator).getNextObject();
      break;

   case ItemTypes::ferryItem :
      item = dynamic_cast< MC2Allocator<FerryItem> &>
         (*theMap.m_ferryItemAllocator).getNextObject();
      break;
      
   case ItemTypes::borderItem :
      item = dynamic_cast< MC2Allocator<Item> &>
         (*theMap.m_simpleItemAllocator).getNextObject();
      break;

   case ItemTypes::routeableItem:
      return NULL;
      break;

   case ItemTypes::notUsedItemType:
      return NULL;
      break;

   case ItemTypes::cartographicItem:
      item = dynamic_cast< MC2Allocator<CartographicItem> &>
         (*theMap.m_cartographicItemAllocator).getNextObject();
   break;

   // This should not happen!
   case ItemTypes::numberOfItemTypes:
      mc2log << fatal << here 
             << "Could not create unknown item. ID=" << itemID
             << ", type=" << (uint32) type << endl;

      break;
      
   } // switch
   if ( item != NULL ) {
      item->m_localID = itemID;
      // item type not stored in simple allocator, please set here:
      item->m_type = type;
   }
   return item;
}

Item*
Item::createNewItem(DataBuffer& dataBuffer, GenericMap& theMap)
{
   
   // Read the type, length and ID of the new item
   uint32 startIndex = dataBuffer.getCurrentOffset();
   uint32 typeAndLength = dataBuffer.readNextLong();
   ItemTypes::itemType type = ItemTypes::itemType( 
                                    (typeAndLength >> 24) & 0xff);
   uint32 itemLength = typeAndLength & 0xffffff;
   uint32 itemID = dataBuffer.readNextLong();

   DEBUG_DB(mc2dbg << "Creating Item, type=" << int(type) << ", id=" 
            << itemID << ", length=" << itemLength << endl);


   Item* item = createItemFromType( type, theMap, itemID );
   
   if ( item != NULL ) {
      // Make sure the ID is set and create the item
      item->setID( itemID );
      item->load( dataBuffer, theMap );

      MC2_ASSERT(item->getID() == itemID);
      MC2_ASSERT(item->getItemType() == type);

      // Skip possible extra bytes at the end of the item
      int skipBytes = itemLength - 
                      (dataBuffer.getCurrentOffset() - startIndex);

      mc2dbg4 << "itemLength=" << itemLength << ", skipBytes=" << skipBytes
              << ", startIdx=" << startIndex << ", curOffset=" 
              << dataBuffer.getCurrentOffset() << endl;

      MC2_ASSERT(skipBytes >= 0);
      dataBuffer.readPastBytes( skipBytes );
      DEBUG1(
         if (skipBytes > 0) 
            mc2dbg2 << "Skipping " << skipBytes << " bytes data for item, ID="
                    << itemID << ", type=" << int(type) << endl;
      );

      // Make sure we don't return any NullItems
      if ( (item != NULL) && (item->getItemType() == ItemTypes::nullItem) ) {
         delete item;
         item = NULL;
      }
   }
   // item is NULL, create a null item and read past rest of item
   else {
      item = new NullItem( itemID );
      item->setID( itemID );
      item->load( dataBuffer, theMap );
      int skipBytes = itemLength - 
                      (dataBuffer.getCurrentOffset() - startIndex);
      dataBuffer.readPastBytes( skipBytes );
      mc2dbg << "Created NULL item for unknown item type, id=" << itemID
             << " and read past " << skipBytes << " bytes" << endl;
   }

   // Return the "new" item
   return item;
}


Item::Item(ItemTypes::itemType type, uint32 id)
{
   init(type);

   m_localID = id;

   mc2dbg8 << "New item with type=" << (int)m_type << " created, id=" 
           << id << endl;
}

void 
Item::init(ItemTypes::itemType type)
{
   m_type = type;
   m_localID = MAX_UINT32;
   m_gfxData = NULL;
   m_source = SearchTypes::NONE;

   m_names = NULL;
   m_nbrNames = 0;
   m_groups = NULL;
   m_nbrGroups = 0;
}

void 
Item::save( DataBuffer& dataBuffer, const GenericMap& map ) const
{

   dataBuffer.alignToLong();

   dataBuffer.writeNextLong( m_type ); 
   dataBuffer.writeNextLong( m_localID );

   dataBuffer.writeNextLong( m_nbrGroups );
   dataBuffer.writeNextByte( m_nbrNames );
   dataBuffer.writeNextBool( m_gfxData != NULL );
   dataBuffer.writeNextByte( m_source );

   if ( m_nbrNames != 0 ) {
      dataBuffer.writeNextLong( m_names - map.getItemNameIdx() );
   }

   if ( m_nbrGroups != 0 ) {
      dataBuffer.writeNextLong( m_groups - map.getItemGroups() );
   }

   if (m_gfxData != NULL) {
      m_gfxData->save( dataBuffer );
      DEBUG1(
         if (m_gfxData->getNbrCoordinates( 0 ) < 1)
            mc2dbg << here << "SAVE: gfxData with < 1 coordinate" << endl;
      );
   }
}



void
Item::load( DataBuffer& dataBuffer, GenericMap& theMap )
{
   dataBuffer.alignToLong();

   m_type = dataBuffer.readNextLong(); // type 
   m_localID = dataBuffer.readNextLong();

   m_nbrGroups = dataBuffer.readNextLong();
   m_nbrNames = dataBuffer.readNextByte();
   bool hasGfxData = dataBuffer.readNextBool();
   m_source = dataBuffer.readNextByte();         
   
   if ( m_nbrNames != 0 ) {
      m_names = const_cast<uint32*>(theMap.getItemNameIdx()) +
         dataBuffer.readNextLong();
   }

   if ( m_nbrGroups != 0 ) {
      m_groups = const_cast<uint32*>(theMap.getItemGroups()) +
         dataBuffer.readNextLong();
   }

   if (hasGfxData) {
      m_gfxData = theMap.createNewGfxData( &dataBuffer );
   } else {
      m_gfxData = NULL;
   }

}


uint32 
Item::getMemoryUsage() const 
{
   uint32 gfxSize = 0;
   if (m_gfxData != NULL )
      gfxSize = m_gfxData->getMemoryUsage();
   return sizeof(Item) + 
          m_nbrNames * sizeof(uint32) + 
          m_nbrGroups* sizeof(uint32) +
          gfxSize;
}

bool 
Item::hasSameNames(const Item* item) const
{
   // Whether the names differ
   bool namesDiffer = false;
   uint32 i = 0;

   const Item* mostNamesItem;
   const Item* leastNamesItem;

   if (this->getNbrNames() > item->getNbrNames()) {
      mostNamesItem = this;
      leastNamesItem = item;
   } else {
      mostNamesItem = item;
      leastNamesItem = this;
   }
   
   while ((i < mostNamesItem->getNbrNames()) && (! namesDiffer)) {
      uint32 pos = 0;
      while ( (pos < leastNamesItem->getNbrNames()) &&
              ( leastNamesItem->getRawStringIndex(pos) !=
                mostNamesItem->getRawStringIndex(i))) {
         ++pos;
      }
      if (pos < leastNamesItem->getNbrNames()) {
         ++i;
      } else {
         // Exit loop
         namesDiffer = true;
      }
   }

   return (!namesDiffer);
}

bool 
Item::hasCommonName(const Item* item) const
{
   for (uint32 i=0; i<getNbrNames(); ++i) {
      for (uint32 j=0; j<item->getNbrNames(); ++j) {
         if (getStringIndex(i) == item->getStringIndex(j))
            return true;
      }
   }
   return false;
}


bool 
Item::hasCommonLangName(const Item* item) const
{
   for (uint32 i=0; i<getNbrNames(); ++i) {
      for (uint32 j=0; j<item->getNbrNames(); ++j) {
         if (getStringIndex(i) == item->getStringIndex(j)){
            if ( getNameLanguage(i) == item->getNameLanguage(j) ){
               // Has a common name in same language.
               return true;
            }
         }
      }
   }
   return false;
}


bool
Item::hasAllLangNames(const Item* item) const
{
   bool differ = false;
   uint32 i=0;
   while ( (!differ) && (i<item->getNbrNames()) ){
      uint32 itemStrIdx = item->getStringIndex(i);
      LangTypes::language_t itemLang = item->getNameLanguage(i);

      uint32 j=0;
      bool found = false;
      while ( (!found) && (j<getNbrNames()) ){
         found = ( ( itemStrIdx == getStringIndex(j) ) &&
                   ( itemLang   == getNameLanguage(j) )  );
         j++;
      }
      if (!found){
         // This name could not be found in this item.
         differ=true;
      }
      i++;
   }
   return !differ;
}

vector<uint32>
Item::getCommonStrIdxes(const Item* item) const
{
   vector<uint32> result;

   for (uint32 i=0; i<getNbrNames(); ++i) {
      for (uint32 j=0; j<item->getNbrNames(); ++j) {
         if (getStringIndex(i) == item->getStringIndex(j)){
            result.push_back( getStringIndex(i) );
         }
      }
   }
   return result;
}


bool
Item::setNameType(byte offset, ItemTypes::name_t type)
{
   if (offset < getNbrNames()) {
      uint32 lang = getNameLanguage(offset);
      uint32 idx = getStringIndex(offset);
      m_names[offset] = CREATE_NEW_NAME(lang, type, idx);
      return true;
   }
   return false;
}

bool 
Item::setNameLanguage(byte offset, LangTypes::language_t lang)
{
   if (offset < getNbrNames()) {
      uint32 type = getNameType(offset);
      uint32 idx = getStringIndex(offset);
      m_names[offset] = CREATE_NEW_NAME(lang, type, idx);
      return true;
   }
   return false;

}

