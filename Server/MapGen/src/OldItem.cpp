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

#include "OldItem.h"
#include "OldBuildingItem.h"
#include "OldBuiltUpAreaItem.h"
#include "OldBusRouteItem.h"
#include "OldCategoryItem.h"
#include "OldCityPartItem.h"
#include "OldFerryItem.h"
#include "OldForestItem.h"
#include "OldGroupItem.h"
#include "OldIslandItem.h"
#include "OldMunicipalItem.h"
#include "OldNullItem.h"
#include "OldParkItem.h"
#include "OldPointOfInterestItem.h"
#include "OldRailwayItem.h"
#include "OldRouteableItem.h"
#include "OldStreetItem.h"
#include "OldStreetSegmentItem.h"
#include "OldWaterItem.h"
#include "OldZipCodeItem.h"
#include "OldZipAreaItem.h"
#include "OldAirportItem.h"
#include "OldAircraftRoadItem.h"
#include "OldPedestrianAreaItem.h"
#include "OldMilitaryBaseItem.h"
#include "OldIndividualBuildingItem.h"
#include "OldSubwayLineItem.h"
#include "OldOverviewMap.h"
#include "OldCartographicItem.h"

#include "MC2String.h"
#include "StringUtility.h"
#include "CoordinateTransformer.h"

#include "OldGenericMap.h"
#include "OldItemNames.h"
#include "GMSGfxData.h"

#include "AllocatorTemplate.h"
#include "MapBits.h"

#include "Utility.h"

void 
OldItem::setGfxData( GfxDataFull* gfx, bool deleteOld)
{
   if (deleteOld)
      delete m_gfxData;
   m_gfxData = gfx;
}

uint32 
OldItem::getLength() const
{
   if (m_gfxData != NULL)
      return ((uint32) m_gfxData->getLength(0));
   else
      return MAX_UINT32;
}

OldItem*
OldItem::createNewItem(DataBuffer* dataBuffer, OldGenericMap* theMap)
{

   // Read the type, length and ID of the new item
   uint32 startIndex = dataBuffer->getCurrentOffset();
   uint32 typeAndLength = dataBuffer->readNextLong();
   ItemTypes::itemType type = ItemTypes::itemType( 
                                    (typeAndLength >> 24) & 0xff);
   uint32 itemLength = typeAndLength & 0xffffff;
   uint32 itemID = dataBuffer->readNextLong();

   DEBUG_DB(mc2dbg << "Creating OldItem, type=" << int(type) << ", id=" 
            << itemID << ", length=" << itemLength << endl);

   // Create the new item, fill it with data and return it
   OldItem* item = NULL;
   switch (type) {
      case ItemTypes::streetSegmentItem :
         mc2dbg8 << "new streetOldSegmentItem, id=" << itemID << endl;
         if (theMap == NULL)
            item = new OldStreetSegmentItem(itemID);
         else
            item = static_cast< MC2Allocator<OldStreetSegmentItem> *>
               (theMap->m_streetSegmentItemAllocator)->getNextObject();
         break;

      case ItemTypes::streetItem:
         mc2dbg8 << "new streetItem, id = " << itemID << endl;
         if (theMap == NULL)
            item = new OldStreetItem(itemID);
         else
            item = static_cast< MC2Allocator<OldStreetItem> *>
               (theMap->m_streetItemAllocator)->getNextObject();
         break;

      case ItemTypes::municipalItem : 
         mc2dbg8 << "new municipal, id=" << itemID << endl;
         if (theMap == NULL) 
            item = new OldMunicipalItem(itemID);
         else
            item = static_cast< MC2Allocator<OldMunicipalItem> *>
               (theMap->m_municipalItemAllocator)->getNextObject();
         break;
      
      case ItemTypes::cityPartItem :
         mc2dbg8 << "new citypart, id=" << itemID << endl;
         if (theMap == NULL) 
            item = new OldCityPartItem(itemID);
         else
            item = static_cast< MC2Allocator<OldCityPartItem> *>
               (theMap->m_cityPartItemAllocator)->getNextObject();
         break;

      case ItemTypes::waterItem :
         mc2dbg8 << "new water, id = " << itemID << endl;
         if (theMap == NULL)
            item = new OldWaterItem(itemID);
         else 
            item = static_cast< MC2Allocator<OldWaterItem> *>
               (theMap->m_waterItemAllocator)->getNextObject();
         break;

      case ItemTypes::parkItem :
         mc2dbg8 << "new park, id = " << itemID << endl;
         if (theMap == NULL) 
            item = new OldParkItem(itemID);
         else
            item = static_cast< MC2Allocator<OldParkItem> *>
               (theMap->m_parkItemAllocator)->getNextObject();
         break;

      case ItemTypes::forestItem :
         mc2dbg8 << "new forest, id = " << itemID << endl;
         if (theMap == NULL) 
            item = new OldForestItem(itemID);
         else
            item = static_cast< MC2Allocator<OldForestItem> *>
               (theMap->m_forestItemAllocator)->getNextObject();
         break;

      case ItemTypes::buildingItem :
         mc2dbg8 << "new building, id = " << itemID << endl;
         if (theMap == NULL)
            item = new OldBuildingItem(itemID);
         else
            item = static_cast< MC2Allocator<OldBuildingItem> *>
               (theMap->m_buildingItemAllocator)->getNextObject();
         break;

      case ItemTypes::railwayItem : 
         mc2dbg8 << "new railway, id=" << itemID << endl;
         if (theMap == NULL)
            item = new OldRailwayItem(itemID);
         else
            item = static_cast< MC2Allocator<OldRailwayItem> *>
               (theMap->m_railwayItemAllocator)->getNextObject();
         break;

      case ItemTypes::islandItem :
         mc2dbg8 << "new island, id=" << itemID << endl;
         if (theMap == NULL)
            item = new OldIslandItem(itemID);
         else
            item = static_cast< MC2Allocator<OldIslandItem> *>
               (theMap->m_islandItemAllocator)->getNextObject();
         break;

      case ItemTypes::nullItem :
         mc2dbg8 << "new nullItem, id = " << itemID << endl;
         item = new OldNullItem(itemID);
         break;

      case ItemTypes::zipCodeItem :
         mc2dbg8 << "new OldZipCodeItem, id = " << itemID << endl;
         if (theMap == NULL)
            item = new OldZipCodeItem(itemID);
         else
            item = static_cast< MC2Allocator<OldZipCodeItem> *>
               (theMap->m_zipCodeItemAllocator)->getNextObject();
         break;

      case ItemTypes::zipAreaItem :
         mc2dbg8 << "new OldZipAreaItem, id = " << itemID << endl;
         if (theMap == NULL)
            item = new OldZipAreaItem(itemID);
         else
            item = static_cast< MC2Allocator<OldZipAreaItem> *>
               (theMap->m_zipAreaItemAllocator)->getNextObject();
         break;

      case ItemTypes::pointOfInterestItem :
         mc2dbg8 << "new POI, id = " << itemID << endl;
         if (theMap == NULL)
            item = new OldPointOfInterestItem(itemID);
         else
            item = static_cast< MC2Allocator<OldPointOfInterestItem> *>
               (theMap->m_pointOfInterestItemAllocator)->getNextObject();
         break;

      case ItemTypes::categoryItem :
         mc2dbg8 << "new Category, id = " << itemID << endl;
         if (theMap == NULL) 
            item = new OldCategoryItem(itemID);
         else
            item = static_cast< MC2Allocator<OldCategoryItem> *>
               (theMap->m_categoryItemAllocator)->getNextObject();
         break;

      case ItemTypes::builtUpAreaItem :
         mc2dbg8 << "new builtUpArea, id = " << itemID << endl;
         if (theMap == NULL)
            item = new OldBuiltUpAreaItem(itemID);
         else
            item = static_cast< MC2Allocator<OldBuiltUpAreaItem> *>
               (theMap->m_builtUpAreaItemAllocator)->getNextObject();
         break;

      case ItemTypes::busRouteItem :
         mc2dbg8 << "new busOldRouteItem, id = " << itemID << endl;
         if (theMap == NULL) 
            item = new OldBusRouteItem(itemID);
         else
            item = static_cast< MC2Allocator<OldBusRouteItem> *>
               (theMap->m_busRouteItemAllocator)->getNextObject();
         break;

      case ItemTypes::airportItem :
         mc2dbg8 << "new airportItem, id = " << itemID << endl;
         if (theMap == NULL) 
            item = new OldAirportItem(itemID);
         else
            item = static_cast< MC2Allocator<OldAirportItem> *>
               (theMap->m_airportItemAllocator)->getNextObject();
         break;

      case ItemTypes::aircraftRoadItem : 
         mc2dbg8 << "new aircraftOldRoadItem, id = " << itemID << endl;
         if (theMap == NULL) 
            item = new OldAircraftRoadItem(itemID);
         else
            item = static_cast< MC2Allocator<OldAircraftRoadItem> *>
               (theMap->m_aircraftRoadItemAllocator)->getNextObject();
         break;

      case ItemTypes::pedestrianAreaItem :
         mc2dbg8 << "new pedestrianOldAreaItem, id = " << itemID << endl;
         if (theMap == NULL)
            item = new OldPedestrianAreaItem(itemID);
         else
            item = static_cast< MC2Allocator<OldPedestrianAreaItem> *>
               (theMap->m_pedestrianAreaItemAllocator)->getNextObject();
         break;

      case ItemTypes::militaryBaseItem :
         mc2dbg8 << "new militaryOldBaseItem, id = " << itemID << endl;
         if (theMap == NULL)
            item = new OldMilitaryBaseItem(itemID);
         else
            item = static_cast< MC2Allocator<OldMilitaryBaseItem> *>
               (theMap->m_militaryBaseItemAllocator)->getNextObject();
         break;

      case ItemTypes::individualBuildingItem :
         mc2dbg8 << "new IndividualBuilding, id = " << itemID << endl;
         if (theMap == NULL) 
            item = new OldIndividualBuildingItem(itemID);
         else
            item = static_cast< MC2Allocator<OldIndividualBuildingItem> *>
               (theMap->m_individualBuildingItemAllocator)->getNextObject();
         break;

      case ItemTypes::subwayLineItem :
         mc2dbg8 << "new subwayOldLineItem, id = " << itemID << endl;
         if (theMap == NULL) 
            item = new OldSubwayLineItem(itemID);
         else
            item = static_cast< MC2Allocator<OldSubwayLineItem> *>
               (theMap->m_subwayLineItemAllocator)->getNextObject();
         break;

      case ItemTypes::ferryItem :
         DEBUG_DB(mc2dbg << "Creating ferryItem with id = " 
                  << itemID << endl);
         if (theMap == NULL)
            item = new OldFerryItem(itemID);
         else
            item = static_cast< MC2Allocator<OldFerryItem> *>
               (theMap->m_ferryItemAllocator)->getNextObject();
         break;
      
      case ItemTypes::borderItem :
         DEBUG_DB(mc2dbg << "Creating OldItem with id = " << itemID << endl);
         if (theMap == NULL) {
            item = new OldItem(type, itemID);
         } else {
            item = static_cast< MC2Allocator<OldItem> *>
               (theMap->m_simpleItemAllocator)->getNextObject();
            // item type not stored in simple allocator, please set here:
            item->m_type = type;
         }
         break;
      
      case ItemTypes::cartographicItem :
         DEBUG_DB(mc2dbg << "Creating OldCatorgaphicItem with id = " 
                  << itemID << endl);
         if (theMap == NULL) {
            item = new OldCartographicItem(itemID);
         } else {
            item = static_cast< MC2Allocator<OldCartographicItem> *>
               (theMap->m_cartographicItemAllocator)->getNextObject();
         }
         break;

      default :
         mc2log << fatal << here 
                << "Could not create unknown item. ID=" << itemID
                << ", type=" << (uint32) type << endl;
         break;
   } // switch
   
   if ( item != NULL ) {
      // Make sure the ID is set and create the item
      item->setID(itemID);
      item->createFromDataBuffer(dataBuffer, theMap);
      MC2_ASSERT(item->getID() == itemID);
      if (item->getItemType() != type){
         mc2log << error << "item->getItemType()=" << item->getItemType()
                << " type=" << type << endl;
         mc2log << error << "Should have been equal." << endl;
         MC2_ASSERT(false);
      }

      // Skip possible extra bytes at the end of the item
      int skipBytes = itemLength - 
                      (dataBuffer->getCurrentOffset() - startIndex);
      mc2dbg4 << "itemLength=" << itemLength << ", skipBytes=" << skipBytes
              << ", startIdx=" << startIndex << ", curOffset=" 
              << dataBuffer->getCurrentOffset() << endl;
      MC2_ASSERT(skipBytes >= 0);
      dataBuffer->readPastBytes(skipBytes);
      DEBUG8(
         if (skipBytes > 0) 
            mc2dbg2 << "Skipping " << skipBytes << " bytes data for item, ID="
                    << itemID << ", type=" << int(type) << endl;
      );
      // Make sure we don't return any OldNullItems
      if ( (item != NULL) && (item->getItemType() == ItemTypes::nullItem) ) {
         delete item;
         item = NULL;
      }
   }
   // item is NULL, create a null item and read past rest of item
   else {
      item = new OldNullItem(itemID);
      item->setID(itemID);
      item->createFromDataBuffer(dataBuffer, theMap);
      int skipBytes = itemLength - 
                      (dataBuffer->getCurrentOffset() - startIndex);
      dataBuffer->readPastBytes(skipBytes);
      mc2dbg << "Created NULL item for unknown item type, id=" << itemID
             << " and read past " << skipBytes << " bytes" << endl;
   }
   

   // Return the "new" item
   return item;
}


OldItem::OldItem(ItemTypes::itemType type, uint32 id)
{
   init(type);

   m_localID = id;

   mc2dbg8 << "New item with type=" << (int)m_type << " created, id=" 
           << id << endl;
}

void 
OldItem::init(ItemTypes::itemType type)
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


OldItem::~OldItem()
{
   MINFO("~OldItem()");
   delete [] m_names;
   delete [] m_groups;
   // Do not delete gfxData since it is allocated in an vector in the map
   // But you do not allways have a map so it is WRONG to assume you
   // have one!!!!
   // Delete the gfxdata manually to remove memoryleak.
}

bool 
OldItem::addGroup(uint32 groupID)
{
   const uint32 MAX_NBR_GROUPS = ~0;
   if ( getNbrGroups() < MAX_NBR_GROUPS ) {
      uint32 i = 0;
      bool found = false;
      while ( !found &&  i<getNbrGroups() ) {
         if ( ( m_groups[i] & 0x7fffffff ) == (groupID & 0x7fffffff ) ) {
            // Found group to add.
            found = true;

            if ( ( (m_groups[i] & 0x80000000) != 0 ) && 
                 ( (groupID & 0x80000000) == 0 ) ){
               // Removing high bit because the new group has it not set.
               m_groups[i] = groupID;
            }

         }
         ++i;
      }
      
      if (!found) {
         // Did not find the group, add it.
         m_groups = ArrayTool::addElement(m_groups, groupID, m_nbrGroups);
         return true;
      }
   } else {
      mc2log << error << "OldItem::addGroup(" << groupID 
             << ")  to item " << getID() << " ("
             << getItemTypeAsString() << ")"
             << " failed, getNbrGroups()=" << ((int)getNbrGroups()) 
             << endl;
      MC2_ASSERT(false);
   }

   return false;
}

bool 
OldItem::save(DataBuffer* dataBuffer) const
{

   dataBuffer->alignToLong();
   DEBUG_DB(mc2dbg << "   OldItem::save()" << endl;)

   DEBUG_DB(mc2dbg << "   Type=" << (uint32) m_type << endl);
   //dataBuffer->writeNextByte(m_type);
   //dataBuffer->writeNextShort(0);// Length, filled in by caller later.
   dataBuffer->writeNextLong(0);   // Type and length, filled in by caller!

   DEBUG_DB(mc2dbg << "   m_localID = " << m_localID << endl);
   dataBuffer->writeNextLong(m_localID);

   dataBuffer->writeNextLong(MAX_UINT32);

   DEBUG_DB(mc2dbg << "   nbrGroups = " << (int) m_nbrGroups << endl);
   dataBuffer->writeNextLong(m_nbrGroups);

   DEBUG_DB(mc2dbg << "   nbrNames = " << (int) m_nbrNames << endl);
   dataBuffer->writeNextByte(m_nbrNames);

   DEBUG_DB(mc2dbg << "   gfxData = " << (m_gfxData != NULL) << endl);
   dataBuffer->writeNextBool(m_gfxData != NULL);

   DEBUG_DB(mc2dbg << "   source = " << int(m_source) << endl);
   dataBuffer->writeNextByte(m_source);    

   // no pad
   for (uint32 i=0; i<m_nbrNames; ++i) {
      // Note that the full information in names[i] must be saved, and
      // _not_ only the string index (in the names is also the language 
      // and type stored)
      DEBUG_DB(mc2dbg << "   m_names[" << (int) i << "] = " << m_names[i] 
                    << endl);
      dataBuffer->writeNextLong(m_names[i]);
   }

   for (uint32 i=0; i<m_nbrGroups; ++i){
      DEBUG_DB(mc2dbg << "   m_groups[" << (int) i << "] = " 
                   << m_groups[i] << endl);
      dataBuffer->writeNextLong(m_groups[i]);
   }

   if (m_gfxData != NULL) {
      (m_gfxData)->save( *dataBuffer );
      DEBUG1(
         if (m_gfxData->getNbrCoordinates(0) < 1)
            mc2dbg << here << "SAVE: gfxData with < 1 coordinate" << endl;
      );
   }
   return true;
}

int 
OldItem::addName(LangTypes::language_t lang, ItemTypes::name_t type, 
              uint32 nameID)
{
   uint32 combinedData = CREATE_NEW_NAME((uint32) lang, (uint32) type, nameID);
   uint32 i = 0;
   while ( (i<getNbrNames()) && (m_names[i] != combinedData)) {
      ++i;
   }
   if (i < getNbrNames()) {
      // Ouups, combindedData already present
      return MAX_UINT32;
   }
   m_names = ArrayTool::addElement(m_names, combinedData, m_nbrNames);
   return m_nbrNames - 1;
}

bool
OldItem::removeNameWithOffset(uint32 i)
{
   ArrayTool::removeElement(m_names, i, m_nbrNames);
   return true;
}

bool
OldItem::removeAllNames()
{
   delete [] m_names;
   m_names = NULL;
   m_nbrNames = 0;
   return true;
}

char OldItem::itemAsString[ITEM_AS_STRING_LENGTH] = "Empty";

char* 
OldItem::toString()
{
   uint32 i;
   char charHasGfxData;
   if (m_gfxData == NULL)
      charHasGfxData = 'F';
   else
      charHasGfxData = 'T';
   sprintf(itemAsString,
           "   localID=%u\n"
           "   type=%u\n"
           "   nbrNames=%u\n"
           "   nbrGroups=%u\n"
           "   hasGfx=%c\n"
           "   source=%u\n"
           "   names=",
           m_localID,
           m_type,
           m_nbrNames,
           m_nbrGroups,
           charHasGfxData,
           m_source);

   for (i = 0; i < getNbrNames(); i++)
      sprintf(itemAsString+strlen(itemAsString), "%u ", m_names[i]);
   strcat(itemAsString, "\n   groups=");
   for (i = 0; i < getNbrGroups(); i++)
      sprintf(itemAsString+strlen(itemAsString), "%u ", m_groups[i]);
   strcat(itemAsString, "\n");
   return itemAsString; 
}

uint32 
OldItem::getMemoryUsage() const 
{
   uint32 gfxSize = 0;
   if (m_gfxData != NULL )
      gfxSize = m_gfxData->getMemoryUsage();
   return sizeof(OldItem) + 
          m_nbrNames * sizeof(uint32) + 
          m_nbrGroups* sizeof(uint32) +
          gfxSize;
}


bool
OldItem::createFromDataBuffer(DataBuffer* dataBuffer, 
                           OldGenericMap* theMap)
{
   dataBuffer->alignToLong();
   DEBUG_DB(mc2dbg << "   OldItem::OldItem() loading common data" << endl;)

   uint32 location = dataBuffer->readNextLong();
   
   m_nbrGroups = dataBuffer->readNextLong();
   DEBUG_DB(mc2dbg << "   nbrGroups = " << (int) m_nbrGroups << endl);

   m_nbrNames = dataBuffer->readNextByte();
   DEBUG_DB(mc2dbg << "   nbrNames = " << (int) m_nbrNames << endl);

   bool hasGfxData = dataBuffer->readNextBool();
   DEBUG_DB(mc2dbg << "   hasGfxData = " << hasGfxData << endl);

   m_source = dataBuffer->readNextByte();         
   DEBUG_DB(mc2dbg << "   source = " << (int) m_source << endl;);
   
   if (m_nbrNames > 0) {
      m_names = new uint32[m_nbrNames];
      for (byte i=0; i<m_nbrNames; i++) {
         m_names[i] = dataBuffer->readNextLong();
         DEBUG_DB(mc2dbg << "   m_names[" << (int) i << "] = " << m_names[i] 
                       << endl);
      }
   } else {
      m_names = NULL;
   }

   if (m_nbrGroups > 0) {
      m_groups = new uint32[m_nbrGroups];
      for (uint32 i=0; i<m_nbrGroups; i++) {
         m_groups[i] = dataBuffer->readNextLong();
         DEBUG_DB(mc2dbg << "   groups[" << (int) i << "] = " << m_groups[i] 
                       << endl);
      }
   } else {
      m_groups = NULL;
   }

   // Only present for location backwards compability.
   // Remove from here -->
   // Backwards compatible. Add location as groups.
   if ( ( location != MAX_UINT32 ) && 
        ( MapBits::getMapLevel( theMap->getMapID() ) < 2 ) ) {
      OldOverviewMap* overview = dynamic_cast<OldOverviewMap*> ( theMap );
      if ( overview != NULL ) {
         // Store in ugly temporary table in overview map 
         // since we need the id lookup table.
         overview->m_tempLocationTable.push_back( 
               make_pair( this->getID(), location ) );
      } else {
         byte municipal = Utility::getMunicipal( location );
         if ( ( municipal != MAX_BYTE ) && ( municipal != getID() ) ) {
            theMap->addRegionToItem(this, municipal );
         }
         uint16 bua = Utility::getBuiltUpArea( location );
         if ( ( bua != MAX_UINT16 ) && ( bua != getID() ) ) {
            theMap->addRegionToItem(this, bua );
         }

         // Special trick for old citypart location.
         // The citypart refers to index in itemsInGroup for the bua.
         byte cpIdx = Utility::getCityPart( location );
         if ( cpIdx != MAX_BYTE ) {
            OldGroupItem* group = 
               dynamic_cast<OldGroupItem*> ( theMap->itemLookup( bua ) );
            if ( group != NULL ) {
               MC2_ASSERT( cpIdx < group->getNbrItemsInGroup() );
               uint32 cp = group->getItemNumber( cpIdx );
               theMap->addRegionToItem(this, cp );
            }
         }
      }
   }
   // <-- To here
   
   if (hasGfxData) {
      DEBUG_DB(mc2dbg << "   loading gfxData" << endl);
      m_gfxData = theMap->createNewGfxData(dataBuffer);
   }
   else {
      DEBUG_DB(mc2dbg << "   No gfxData" << endl;)
      m_gfxData = NULL;
   }

   return (true);
}

void
OldItem::writeGenericMifHeader(uint32 nbrCol, ofstream& mifFile,
                        CoordinateTransformer::format_t coordsys,
                        bool coordOrderLatLon)
{
   MC2String coordSysStr = "mc2";
   if ( coordsys == CoordinateTransformer::mc2 ) {
      if ( coordOrderLatLon ) { coordSysStr = "mc2"; }
      else { coordSysStr = "mc2_lonlat"; }
   }
   else if ( coordsys == CoordinateTransformer::wgs84deg ) {
      if ( coordOrderLatLon ) { coordSysStr = "wgs84_latlon_deg"; }
      else { coordSysStr = "wgs84_lonlat_deg"; }
   }
   else if ( coordsys == CoordinateTransformer::rt90_2_5gonV_rh70_XYH ) {
      if ( coordOrderLatLon ) { coordSysStr = "wgs84_latlon_deg"; }
      else { coordSysStr = "wgs84_lonlat_deg"; }
   }
   else {
      mc2log << warn << here << "Can't handle this coordsys" 
             << int(coordsys) << endl;
   }
   
   
   mifFile << "VERSION 300" << endl
           << "Charset \"WindowsLatin1\"" << endl
	        << "DELIMITER \",\"" << endl
           << "COORDSYS " << coordSysStr << endl
           << "COLUMNS " << nbrCol+3 << endl
           << "  LOCAL_ID integer(16,0)" << endl
           << "  NAME char(50)" << endl
           << "  ALL_NAMES char(256)" << endl;
   
}

void
OldItem::printMidMif(ofstream& midFile, ofstream& mifFile, OldItemNames* namePointer)
{

   // Print the local item ID
   midFile << m_localID << ",";
   
   // Print the official name in a single column
   uint32 officialNameIndex = getNameWithType(ItemTypes::officialName,
                                              LangTypes::swedish);
   if (officialNameIndex < MAX_UINT32) {
      midFile << "\"" << namePointer->getString(officialNameIndex) << "\"";
   } else {
      midFile << "\"" << "XXX" << "\"";
   }
   
   midFile << ",\"";   
   // Print all the names in the same column
   for (byte offset = 0; offset < getNbrNames(); offset++){
      LangTypes::language_t lang_ref;
      ItemTypes::name_t type;
      uint32 index;
      if (getNameAndType(offset, lang_ref, type, index)){
         
         // Print the name
         midFile << namePointer->getString(index)
                 << ":";

         // Print the type
         midFile << ItemTypes::getNameTypeAsString(type)
                 << ":";
         
         // Print the language
         midFile << LangTypes::getLanguageAsString(lang_ref);
         
         if (offset != (getNbrNames() - 1))
            midFile << " ";
         
      }
   }
   midFile << "\"";

   //print all gfx-data for the item
   getGfxData()->printMif(mifFile);
}

bool
OldItem::createFromMidMif(ifstream& midFile, bool readRestOfLine)
{
   // Empty
   // Everything is handled in the GMSItems

   return true;
}

bool 
OldItem::hasSameNames(const OldItem* item) const
{
   // Whether the names differ
   bool namesDiffer = false;
   uint32 i = 0;

   const OldItem* mostNamesItem;
   const OldItem* leastNamesItem;

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
OldItem::hasCommonName(const OldItem* item) const
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
OldItem::hasCommonLangName(const OldItem* item) const
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
OldItem::hasAllLangNames(const OldItem* item) const
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
OldItem::getCommonStrIdxes(const OldItem* item) const
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
OldItem::setNameType(byte offset, ItemTypes::name_t type)
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
OldItem::setNameLanguage(byte offset, LangTypes::language_t lang)
{
   if (offset < getNbrNames()) {
      uint32 type = getNameType(offset);
      uint32 idx = getStringIndex(offset);
      m_names[offset] = CREATE_NEW_NAME(lang, type, idx);
      return true;
   }
   return false;

}


bool
OldItem::updateAttributesFromItem(OldItem* otherItem, bool sameMap)
{
   mc2dbg8 << "OldItem::updateAttributesFromItem" << endl;
   bool retVal = false;

   if (otherItem == NULL) {
      return retVal;
   }

   // The itemId, type and source are not to be changed
   
   // Groups
   // Simple way: remove all groups from this item and add from otherItem
   // If this item is overview item and the other is not, the ids can be 
   // updated in o- and co-map...
   uint32 nbrGroups = getNbrGroups();
   for (int g = nbrGroups-1; g >= 0; g--) {
      removeGroup(g);
   }
   for (uint32 g = 0; g < otherItem->getNbrGroups(); g++) {
      addGroup(otherItem->getGroup(g)); // This call to addGroup is OK,
      // because the group must initially have been added to the otherItem
      // with addRegionToItem.
   }
   retVal = true;

   if ( sameMap ) {
      // Names, str ids only vaild if the items originates from the same map
      // Simple way: remove all names from this item and add from otherItem
      removeAllNames();
      for (uint32 n = 0; n < otherItem->getNbrNames(); n++) {
         uint32 strIdx = otherItem->getStringIndex(n);
         if (strIdx != MAX_UINT32) {
            addName(otherItem->getNameLanguage(n),
                    otherItem->getNameType(n),
                    strIdx);
         }
      }
      retVal = true;
   }
   
   // GfxData. 
   if ( !sameMap ) {
      // handled in the overview- and country overview map
      return retVal;
   } else {
      // to be implemented.
   }

   return retVal;
}

bool 
OldItem::removeAllGroups()
{
   bool result = true;

   uint32 nbrGroups = getNbrGroups();
   for ( uint32 g = 0; g < nbrGroups; g++){
      result = ( (result) && (removeGroup(0)) );
   }
   return result;
} // removeAllGroups.


uint32
OldItem::getOfficialCode(const OldGenericMap& theMap) const {
   uint32 result = MAX_UINT32;
   ItemMap<uint32>::const_iterator it = 
      theMap.m_itemOfficialCodes.find(this->getID());
   if ( it != theMap.m_itemOfficialCodes.end() ){
      result = it->second;
   }
   return result;
} // getOfficialCode

void
OldItem::setOfficialCode(OldGenericMap& theMap, uint32 officialCode){
   theMap.m_itemOfficialCodes.insert(make_pair(this->getID(), officialCode));
} // setOfficialCode



set<uint16>
OldItem::getCategories(const OldGenericMap& theMap) const {
   set<uint16> result;
   ItemMap<set<uint16> >::const_iterator it = 
      theMap.m_itemCategories.find(this->getID());
   if ( it != theMap.m_itemCategories.end() ){
      result = it->second;
   }
   return result;
} // getCategory

void
OldItem::addCategories(OldGenericMap& theMap, 
                       const set<uint16>& poiCategories ){
   for ( set<uint16>::const_iterator catIt = poiCategories.begin(); 
         catIt != poiCategories.end(); ++catIt){
      this->addCategory(theMap, *catIt);
   }
} // addCategories

void
OldItem::addCategory(OldGenericMap& theMap, uint16 category){
   // Check status of this item.
   if ( this->getID() == MAX_UINT32 ){
      mc2log << error << "Trying to add category to an item with no item ID."
             << " exits." << endl;
      MC2_ASSERT(false);
   }

   // Add the category.
   ItemMap<set<uint16> >::iterator it = 
      theMap.m_itemCategories.find(this->getID());
   if ( it == theMap.m_itemCategories.end() ){
      // Add a set to store categories in of this item.
      set<uint16> dummySet;
      it = theMap.m_itemCategories.insert(make_pair(this->getID(), 
                                                    dummySet) ).first;
   }
   it->second.insert(category);
} // setCategory

void
OldItem::removeCategories(OldGenericMap& theMap){
   // Check status of this item.
   if ( this->getID() == MAX_UINT32 ){
      mc2log << error 
             << "Trying to remove categories from an item with no item ID."
             << " exits." << endl;
      MC2_ASSERT(false);
   }

   // Find the categories for this item
   ItemMap<set<uint16> >::iterator it = 
      theMap.m_itemCategories.find(this->getID());
   if ( it != theMap.m_itemCategories.end() ){
      // Remove
      theMap.m_itemCategories.erase(it);
   }
   // else no poi categories to remove
   
} // removeCategories

