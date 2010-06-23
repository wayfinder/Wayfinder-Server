/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSItem.h"

#include "GMSBuildingItem.h"
#include "GMSBuiltUpAreaItem.h"
#include "GMSBusRouteItem.h"
#include "GMSCategoryItem.h"
#include "GMSCityPartItem.h"
#include "GMSFerryItem.h"
#include "GMSForestItem.h"
#include "GMSIslandItem.h"
#include "GMSMunicipalItem.h"
#include "GMSParkItem.h"
#include "GMSPointOfInterestItem.h"
#include "GMSRailwayItem.h"
#include "GMSStreetItem.h"
#include "GMSStreetSegmentItem.h"
#include "GMSWaterItem.h"
#include "GMSZipCodeItem.h"
#include "GMSAirportItem.h"
#include "GMSAircraftRoadItem.h"
#include "GMSPedestrianAreaItem.h"
#include "GMSMilitaryBaseItem.h"
#include "GMSIndividualBuildingItem.h"
#include "OldSubwayLineItem.h"
#include "OldZipAreaItem.h"
#include "OldNullItem.h"
#include "GMSCartographicItem.h"
#include "AllocatorTemplate.h"

#include "MidMifData.h"


GMSItem::GMSItem(map_t mapType)
{
   initMapType(mapType);
}

void
GMSItem::initMapType(map_t mapType)
{
   switch (mapType) {
      case (MIDMIF) :
         m_mapData = new MidMifData();
      break;
      default :
         m_mapData = NULL;
   }
}

GMSItem::~GMSItem()
{
   delete m_mapData;
}

OldItem*
GMSItem::createNewItem(DataBuffer* dataBuffer, GMSMap* theMap)
{
   mc2dbg8 << "GMSItem::createNewItem()" << endl;
   // Read the type, length and ID of the new item
   uint32 startIndex = dataBuffer->getCurrentOffset();
   uint32 typeAndLength = dataBuffer->readNextLong();
   ItemTypes::itemType type = ItemTypes::itemType(
                                    (typeAndLength >> 24) & 0xff);
   uint32 itemLength = typeAndLength & 0xffffff;
   uint32 itemID = dataBuffer->readNextLong();

   DEBUG_DB(cerr << "Creating OldItem, type=" << int(type) << ", id="
                 << itemID << ", length=" << itemLength << endl);

   // Create the new item, fill it with data and return it
   OldItem* item = NULL;
   switch (type) {
      case ItemTypes::streetSegmentItem : {
         mc2dbg8 << "new streetSegmentItem, id=" << itemID << endl;
         if (theMap == NULL)
            item = new GMSStreetSegmentItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSStreetSegmentItem> *>
               (theMap->m_streetSegmentItemAllocator)->getNextObject();
         GMSStreetSegmentItem* ssi = static_cast<GMSStreetSegmentItem*>(item);
         ssi->m_map = theMap;
         } break;

      case ItemTypes::streetItem:
         mc2dbg8 << "new streetItem, id = " << itemID << endl;
         if (theMap == NULL)
            item = new GMSStreetItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSStreetItem> *>
               (theMap->m_streetItemAllocator)->getNextObject();
         break;

      case ItemTypes::municipalItem :
         mc2dbg8 << "new municipal, id=" << itemID << endl;
         if (theMap == NULL)
            item = new GMSMunicipalItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSMunicipalItem> *>
               (theMap->m_municipalItemAllocator)->getNextObject();
         break;

      case ItemTypes::cityPartItem :
         mc2dbg8 << "new citypart, id=" << itemID << endl;
         if (theMap == NULL)
            item = new GMSCityPartItem();
         else
            item = static_cast< MC2Allocator<GMSCityPartItem> *>
               (theMap->m_cityPartItemAllocator)->getNextObject();
         break;

      case ItemTypes::waterItem :
         mc2dbg8 << "new water, id = " << itemID << endl;
         if (theMap == NULL)
            item = new GMSWaterItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSWaterItem> *>
               (theMap->m_waterItemAllocator)->getNextObject();
         break;

      case ItemTypes::parkItem :
         mc2dbg8 << "new park, id = " << itemID << endl;
         if (theMap == NULL)
            item = new GMSParkItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSParkItem> *>
               (theMap->m_parkItemAllocator)->getNextObject();
         break;

      case ItemTypes::forestItem :
         mc2dbg8 << "new forest, id = " << itemID << endl;
         if (theMap == NULL)
            item = new GMSForestItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSForestItem> *>
               (theMap->m_forestItemAllocator)->getNextObject();
         break;

      case ItemTypes::buildingItem :
         mc2dbg8 << "new building, id = " << itemID << endl;
         if (theMap == NULL)
            item = new GMSBuildingItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSBuildingItem> *>
               (theMap->m_buildingItemAllocator)->getNextObject();
         break;

      case ItemTypes::railwayItem :
         mc2dbg8 << "new railway, id=" << itemID << endl;
         if (theMap == NULL)
            item = new GMSRailwayItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSRailwayItem> *>
               (theMap->m_railwayItemAllocator)->getNextObject();
         break;

      case ItemTypes::islandItem :
         mc2dbg8 << "new island, id=" << itemID << endl;
         if (theMap == NULL)
            item = new GMSIslandItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSIslandItem> *>
               (theMap->m_islandItemAllocator)->getNextObject();
         break;

      case ItemTypes::nullItem :
         mc2dbg8 << "new nullItem, id = " << itemID << endl;
         if (theMap == NULL)
            item = new OldNullItem(itemID);
         else
            item = static_cast< MC2Allocator<OldNullItem> *>
               (theMap->m_nullItemAllocator)->getNextObject();
         break;

      case ItemTypes::zipCodeItem :
         mc2dbg8 << "new OldZipCodeItem, id = " << itemID << endl;
         if (theMap == NULL)
            item = new GMSZipCodeItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSZipCodeItem> *>
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
            item = new GMSPointOfInterestItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSPointOfInterestItem> *>
               (theMap->m_pointOfInterestItemAllocator)->getNextObject();
         break;

      case ItemTypes::categoryItem :
         mc2dbg8 << "new Category, id = " << itemID << endl;
         if (theMap == NULL)
            item = new GMSCategoryItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSCategoryItem> *>
               (theMap->m_categoryItemAllocator)->getNextObject();
         break;

      case ItemTypes::builtUpAreaItem :
         mc2dbg8 << "new builtUpArea, id = " << itemID << endl;
         if (theMap == NULL)
            item = new GMSBuiltUpAreaItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSBuiltUpAreaItem> *>
               (theMap->m_builtUpAreaItemAllocator)->getNextObject();
         break;

      case ItemTypes::busRouteItem :
         mc2dbg8 << "new busRouteItem, id = " << itemID << endl;
         if (theMap == NULL)
            item = new GMSBusRouteItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSBusRouteItem> *>
               (theMap->m_busRouteItemAllocator)->getNextObject();
         break;

      case ItemTypes::airportItem :
         mc2dbg8 << "new airportItem, id = " << itemID << endl;
         if (theMap == NULL)
            item = new GMSAirportItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSAirportItem> *>
               (theMap->m_airportItemAllocator)->getNextObject();
         break;

      case ItemTypes::aircraftRoadItem :
         mc2dbg8 << "new aircraftRoadItem, id = " << itemID << endl;
         if (theMap == NULL)
            item = new GMSAircraftRoadItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSAircraftRoadItem> *>
               (theMap->m_aircraftRoadItemAllocator)->getNextObject();
         break;

      case ItemTypes::pedestrianAreaItem :
         mc2dbg8 << "new pedestrianAreaItem, id = " << itemID << endl;
         if (theMap == NULL)
            item = new GMSPedestrianAreaItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSPedestrianAreaItem> *>
               (theMap->m_pedestrianAreaItemAllocator)->getNextObject();
         break;

      case ItemTypes::militaryBaseItem :
         mc2dbg8 << "new militaryBaseItem, id = " << itemID << endl;
         if (theMap == NULL)
            item = new GMSMilitaryBaseItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSMilitaryBaseItem> *>
               (theMap->m_militaryBaseItemAllocator)->getNextObject();
         break;

      case ItemTypes::individualBuildingItem :
         mc2dbg8 << "new IndividualBuilding, id = " << itemID << endl;
         if (theMap == NULL)
            item = new GMSIndividualBuildingItem();
         else
            item = static_cast< MC2Allocator<GMSIndividualBuildingItem> *>
               (theMap->m_individualBuildingItemAllocator)->getNextObject();
         break;

      case ItemTypes::subwayLineItem :
         mc2dbg8 << "new subwayLineItem, id = " << itemID << endl;
         if (theMap == NULL)
            item = new OldSubwayLineItem(itemID);
         else
            item = static_cast< MC2Allocator<OldSubwayLineItem> *>
               (theMap->m_subwayLineItemAllocator)->getNextObject();
         break;

      case ItemTypes::ferryItem :
         DEBUG_DB(cerr << "Creating ferryItem with id = "
                  << itemID << endl);
         if (theMap == NULL)
            item = new GMSFerryItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSFerryItem> *>
               (theMap->m_ferryItemAllocator)->getNextObject();
         break;

      case ItemTypes::cartographicItem :
         DEBUG_DB(cerr << "Creating cartographic item  with id = "
                  << itemID << endl);
         if (theMap == NULL)
            item = new GMSCartographicItem(itemID);
         else
            item = static_cast< MC2Allocator<GMSCartographicItem> *>
               (theMap->m_cartographicItemAllocator)->getNextObject();
         break;

      default :
         mc2log << fatal << "Could not create unknown item. ID=" << itemID
                << ", type=" << (uint32) type << endl;
         break;
   } // switch

   // Make sure the ID is set and create the item
   MC2_ASSERT(item != NULL);
   item->setID(itemID);
   item->createFromDataBuffer(dataBuffer, theMap);
   MC2_ASSERT(item->getID() == itemID);
   MC2_ASSERT(item->getItemType() == type);

   // Skip possible extra bytes at the end of the item
   int skipBytes = itemLength -
                   (dataBuffer->getCurrentOffset() - startIndex);
   mc2dbg4 << "itemLength=" << itemLength << ", skipBytes=" << skipBytes
           << ", startIdx=" << startIndex << ", curOffset="
           << dataBuffer->getCurrentOffset() << endl;
   MC2_ASSERT(skipBytes >= 0);
   dataBuffer->readPastBytes(skipBytes);
   
   // Return the "new" item
   return item;
}


uint32
GMSItem::getLeftSettlement()
{
   MidMifData* midmifData = dynamic_cast<MidMifData*>(m_mapData);
   if ( midmifData != NULL ) {
      return (midmifData->getLeftSettlement());
   }
   
   return MAX_UINT32;
}

void
GMSItem::setLeftSettlement(uint32 settlement)
{
   MidMifData* midmifData = dynamic_cast<MidMifData*>(m_mapData);
   if ( midmifData != NULL ) {
      midmifData->setLeftSettlement(settlement);
   }
}

uint32
GMSItem::getRightSettlement()
{
   MidMifData* midmifData = dynamic_cast<MidMifData*>(m_mapData);
   if ( midmifData != NULL ) {
      return (midmifData->getRightSettlement());
   }
   
   return MAX_UINT32;
}

void
GMSItem::setRightSettlement(uint32 settlement)
{
   MidMifData* midmifData = dynamic_cast<MidMifData*>(m_mapData);
   if ( midmifData != NULL ) {
      midmifData->setRightSettlement(settlement);
   }
}

uint32
GMSItem::getSettlementOrder()
{
   MidMifData* midmifData = dynamic_cast<MidMifData*>(m_mapData);
   if ( midmifData != NULL ) {
      return (midmifData->getSettlementOrder());
   }
   
   return MAX_UINT32;
}

void
GMSItem::setSettlementOrder(uint32 settlement)
{
   MidMifData* midmifData = dynamic_cast<MidMifData*>(m_mapData);
   if ( midmifData != NULL ) {
      midmifData->setSettlementOrder(settlement);
   }
}

uint32
GMSItem::getTempIndexAreaOrder()
{
   MidMifData* midmifData = dynamic_cast<MidMifData*>(m_mapData);
   if ( midmifData != NULL ) {
      return (midmifData->getTempIndexAreaOrder());
   }
   
   return MAX_UINT32;
}

void
GMSItem::setTempIndexAreaOrder(uint32 order)
{
   MidMifData* midmifData = dynamic_cast<MidMifData*>(m_mapData);
   if ( midmifData != NULL ) {
      midmifData->setTempIndexAreaOrder(order);
   }
}

uint32
GMSItem::getTempRoadDisplayClass()
{
   MidMifData* midmifData = dynamic_cast<MidMifData*>(m_mapData);
   if ( midmifData != NULL ) {
      return (midmifData->getTempRoadDisplayClass());
   }
   
   return MAX_UINT32;
}


