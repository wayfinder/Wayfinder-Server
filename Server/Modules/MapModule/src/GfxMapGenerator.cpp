/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// No improvement in speed when sending nbr items, so lets 
// not do that then.
//#define SEND_NBR_ITEMS

#include "config.h"
#include "GfxData.h"
#include "GenericMap.h"
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
#include "AirportItem.h"
#include "AircraftRoadItem.h"
#include "PedestrianAreaItem.h"
#include "IndividualBuildingItem.h"
#include "SubwayLineItem.h"
#include "TCPSocket.h"
#include "MapHandler.h"
#include "GfxMapGenerator.h"
#include "GfxConstants.h"
#include "TimeUtility.h"

const uint32 GfxMapGenerator::m_POIGfxZoomLevel = 1;

GfxMapGenerator::GfxMapGenerator( MapHandler *mh,
                                  uint32 *mapIDs,
                                  uint32 port,
                                  byte zoomlevel )
      : GenericGfxMapGenerator( mh, 
                                mapIDs, 
                                port) 
{
   mc2dbg2 << "GfxMapGenerator created" << endl;

   // Set the zoomlevel of the gfx-map
   m_gfxZoomlevel = zoomlevel;
   mc2dbg4 << "Zoomlevel " << uint32(m_gfxZoomlevel) << " requested " 
           << endl; 
   
}


GfxMapGenerator::~GfxMapGenerator()
{
   mc2dbg4 << "GfxMapGenerator destroyed" << endl;
}


void
GfxMapGenerator::run()
{

   const int maxHandshake = 1024;
   char handshake[maxHandshake];
   
   TCPSocket* socket = waitForConnection(handshake, maxHandshake);

   
   if (socket == NULL) {
      char debugString[1024];
      sprintf(debugString, "GfxMapGenerator()->generateMap(): "
              "No connection for map with ID %d on port %d", mapIDs[0],
              this->port);
      mc2log << error << debugString << endl;
      delete socket;
      return;
   }
   
   uint32 startTime = TimeUtility::getCurrentTime();
   // We've got an request for a map...
   mc2dbg4 << "GfxMapGenerator, sending map with ID " << mapIDs[0] 
               << endl;

   GenericMap *theMap = mapHandler->getMap(mapIDs[0]);
   mc2dbg8 << "MAPID " << mapIDs[0] << endl;

   DataBuffer* itemBuffer = NULL;

   if (theMap != NULL) {
      itemBuffer = new DataBuffer(80000000); // XXX: Not hardcoded size here

      // First write the version of this map
      mc2dbg4 << "sending version for map " << mapIDs[0] << " v=" 
                  << theMap->getCreationTime() << endl;
      itemBuffer->writeNextLong(theMap->getCreationTime());

      itemBuffer->writeNextLong(0);       // Length of items, written later
      itemBuffer->writeNextLong(0);       // Nbr items, written later

#ifdef SEND_NBR_ITEMS      
      // Number itemtypes.
      itemBuffer->writeNextLong(ItemTypes::numberOfItemTypes);
      // Write number items per itemtype (this is filled in later).
      // Store this offset in the itemBuffer
      uint32 nbrItemPerItemTypeOffset = itemBuffer->getCurrentOffset();
      for (uint32 i = 0; i < ItemTypes::numberOfItemTypes; i++) {
         itemBuffer->writeNextLong(0); // fill in later.
         m_nbrItemsPerItemType[i] = 0; // reset
      }
#endif      

      // Send the items
      uint32 nbrItems = 0;
      mc2dbg8 << "Zoom level " << uint32(m_gfxZoomlevel) << endl;

      for ( uint32 z=0; z<=ItemTypes::zoomlevelVect[m_gfxZoomlevel]; z++) {
         uint32 stTime = TimeUtility::getCurrentTime();
         writeItemsInZoomLevel(theMap, itemBuffer, z, nbrItems);
         uint32 endTime = TimeUtility::getCurrentTime();
         mc2dbg4 << "Zoomlevel " << z << " in map generated in " 
                 << endTime-stTime << "ms" << endl;
      }
      
      if (m_gfxZoomlevel == m_POIGfxZoomLevel) {
         // Since poii zoomlevel is 14 and therefore not written above.
         // Go through that zoomlevel.
         uint32 stTime = TimeUtility::getCurrentTime();
         writeItemsInZoomLevel(theMap, itemBuffer,
                               ItemTypes::poiiZoomLevel, nbrItems);
         uint32 endTime = TimeUtility::getCurrentTime();
         mc2dbg4 << "m_POIGfxZoomLevel in map generated in " 
                 << endTime-stTime << "ms" << endl;
      }
      
      mc2dbg8 << "Wrote data to buffe " << endl;

      mc2dbg4 << "nbrItems = " << dec << nbrItems << ", itemLength = " 
              << itemBuffer->getCurrentOffset()-8 << endl;

      itemBuffer->writeLong(itemBuffer->getCurrentOffset()-8, 4);
      itemBuffer->writeLong(nbrItems, 8);

#ifdef SEND_NBR_ITEMS
      // Fill in number items per itemtype as well.
      for (uint32 i = 0; i < ItemTypes::numberOfItemTypes; i++) {
         itemBuffer->writeLong( m_nbrItemsPerItemType[i], 
                                nbrItemPerItemTypeOffset + 
                                i * sizeof(uint32) );
         cout << "itemtype = " << i << ", nbr items " 
              << m_nbrItemsPerItemType[i] << endl;
      }
#endif      
 
      
   } else {
      // theMap == NULL
   }

   int nbrBytes = socket->writeAll( itemBuffer->getBufferAddress(),
                                    itemBuffer->getCurrentOffset() );
   mc2dbg2 << nbrBytes << " bytes with map " <<  theMap->getMapID()
           <<" sent on socket." << endl ;


   delete itemBuffer;

   socket->close();
   delete socket;

   mc2dbg2 << "GfxMapGenerator sent all data for map " << mapIDs[0]
              << ", processing time " << TimeUtility::getCurrentTime()-startTime 
              << " ms" << endl;
}

bool
GfxMapGenerator::includeItem(const Item* item, 
                             GenericMap* theMap,
                             bool& reduceGfxToPoint,
                             uint16 polygon)
{
   reduceGfxToPoint = false;
   // Limits used if to use area as limit. Might be changed in the
   // switch-statement (areas in square-mc2-units).
   const uint64 minAreaZoom0 = (uint64)
      ((10000*10000)*GfxConstants::SQUARE_METER_TO_SQUARE_MC2SCALE);
   const uint64 minAreaZoom1 = (uint64)
      ((6000*6000)*GfxConstants::SQUARE_METER_TO_SQUARE_MC2SCALE);
   const uint64 minAreaZoom2 = (uint64)
      ((800*800)*GfxConstants::SQUARE_METER_TO_SQUARE_MC2SCALE);
   
   // Verify the inparameters
   if ( (item == NULL) || 
        (m_gfxZoomlevel > 3) ) {
      mc2log << warn << "GfxMapGenerator::includeItem invalid parameters"
             << "m_gfxZoomlevel=" << uint32(m_gfxZoomlevel) << endl;
      return (false);
   }

   switch (item->getItemType()) {
      // - - - - - - - - - - - - - - - - - - - - - - Street segment item
      case (ItemTypes::streetSegmentItem) : {
         const StreetSegmentItem* ssi = 
            static_cast<const StreetSegmentItem*>(item);
         if (! theMap->isPartOfGfxStreet(ssi) ) {
            switch (m_gfxZoomlevel) {
               case (0) :
                  return (ssi->getRoadClass() == 0);
               case (1) :
                  return (ssi->getRoadClass() == 1);
               case (2) :
                  return (ssi->getRoadClass() == 2);
               default:
                  return (ssi->getRoadClass() > 2);
            }   
         } else {
            return (false);
         }
     } break;

      // - - - - - - - - - - - - - - - - - - - - - - Municipal item
      case (ItemTypes::municipalItem) : {
         // No need for municipals once we are using the gfxdata of the country.
         return (false);     

      }

      // - - - - - - - - - - - - - - - - - - - - - - Water item
      case (ItemTypes::waterItem) : {
         if ((static_cast<const WaterItem*>(item))->getWaterType() ==
               ItemTypes::ocean) {
            return (false);
         }
         // Otherwise; Use area 
      } break;

      // - - - - - - - - - - - - - - - - - - - - - - Park item
      case (ItemTypes::parkItem) : {
         return (m_gfxZoomlevel == 3);
      } break;

      // - - - - - - - - - - - - - - - - - - - - - - Forest item
      case (ItemTypes::forestItem) :
         return (false);

      // - - - - - - - - - - - - - - - - - - - - - - Building item
      case (ItemTypes::buildingItem) : {
         return (m_gfxZoomlevel == 3);
      } break;

      // - - - - - - - - - - - - - - - - - - - - - - Railway item
      case (ItemTypes::railwayItem) : {
         // Send all railways in m_gfxZoomlevellevel 2
         return (m_gfxZoomlevel == 2);
      } break;

      // - - - - - - - - - - - - - - - - - - - - - - Island item
      case (ItemTypes::islandItem) : {
         // Use area 
      } break;

      // - - - - - - - - - - - - - - - - - - - - - - Street item
      case (ItemTypes::streetItem) : {
         const StreetItem* si = static_cast<const StreetItem*>(item);
         if (si->getGfxData() == NULL) {
            return false;
         }
         switch (m_gfxZoomlevel) {
            case (0) :
               return (si->getRoadClassForPolygon(polygon) == 0);
            case (1) :
               return (si->getRoadClassForPolygon(polygon) == 1);
            case (2) :
               return (si->getRoadClassForPolygon(polygon) == 2);
            default:
               return (si->getRoadClassForPolygon(polygon) > 2);
         }   
      } break;

      // - - - - - - - - - - - - - - - - - - - - - - Null item
      case (ItemTypes::nullItem) :
         return (false);

      // - - - - - - - - - - - - - - - - - - - - - - Zip code item
      case (ItemTypes::zipCodeItem) :
         return (false);

      // - - - - - - - - - - - - - - - - - - - - - - Built up area item
      case (ItemTypes::builtUpAreaItem) : {
         // Use area
      } break;

      // - - - - - - - - - - - - - - - - - - - - - - City part item
      case (ItemTypes::cityPartItem) : {
         return (m_gfxZoomlevel == 2);
      } break;

      // - - - - - - - - - - - - - - - - - - - - - - Point of interest item
      case (ItemTypes::pointOfInterestItem) : {
         const PointOfInterestItem* poii = 
            static_cast<const PointOfInterestItem*>(item);
         mc2dbg4 << "Checking POI " << theMap->getFirstItemName(poii) 
                 << endl;
      
         // Include if "real poi" and not an ordinary company.
         return ( (m_gfxZoomlevel == m_POIGfxZoomLevel) &&
                  (!poii->isPointOfInterestType(ItemTypes::unknownType) ) &&
                  (!poii->isPointOfInterestType(ItemTypes::company)));

      } break;

      // - - - - - - - - - - - - - - - - - - - - - - Category item
      case (ItemTypes::categoryItem) :
         return (false);

      // - - - - - - - - - - - - - - - - - - - - - - Routable item
      case (ItemTypes::routeableItem) :
         return (false);

      // - - - - - - - - - - - - - - - - - - - - - - Bus route item
      case (ItemTypes::busRouteItem) :
         return (false);

      // - - - - - - - - - - - - - - - - - - - - - - Ferry item
      case (ItemTypes::ferryItem) :
         // Send all ferries in m_gfxZoomlevellevel 2
         return (m_gfxZoomlevel == 2);
      
      // - - - - - - - - - - - - - - - - - - - - - - Airport item
      case (ItemTypes::airportItem) :
         return (m_gfxZoomlevel == 3);
         
      // - - - - - - - - - - - - - - - - - - - - - - AircraftRoad item
      case (ItemTypes::aircraftRoadItem) :
         return (m_gfxZoomlevel == 3);
         
      // - - - - - - - - - - - - - - - - - - - - - - Pedestrianarea item
      case (ItemTypes::pedestrianAreaItem) :
         return (m_gfxZoomlevel == 3);
         
      // - - - - - - - - - - - - - - - - - - - - - - MilitaryBase item
      case (ItemTypes::militaryBaseItem) :
         return (m_gfxZoomlevel == 3);
	 
      // - - - - - - - - - - - - - - - - - - - - - - IndividualBuilding item
      case (ItemTypes::individualBuildingItem) :
         return (m_gfxZoomlevel == 3);

      // - - - - - - - - - - - - - - - - - - - - - - SubwayLine item
      case (ItemTypes::subwayLineItem) :
         return (m_gfxZoomlevel == 3);

      // - - - - - - - - - - - - - - - - - - - - - - NbrItemTypes
      case (ItemTypes::numberOfItemTypes) :
      default:
         return (false);


   } // switch


   // If not handled in the switch-statement above, use the area-limits 
   // to determine if the item should be included or not.

   if (item->getGfxData() != NULL) {
      // Get the GfxData and the approximative area of the item
      MC2BoundingBox bbox;
      item->getGfxData()->getMC2BoundingBox(bbox);
      uint64 area = bbox.getArea();

      // Compare the area with the area that should be seen at current 
      // m_gfxZoomlevel level
      switch (m_gfxZoomlevel) {
         case (0):
            return (area > minAreaZoom0);
         case (1):
            return ( (area <= minAreaZoom0) && (area > minAreaZoom1));
         case (2):
            return ( (area <= minAreaZoom1) && (area > minAreaZoom2));
         case (3):
            return (area <= minAreaZoom2);
      }
   }

   // This source should never be reached, returning false for the compiler...
   mc2log << warn << "   Reached \"impossible\" source-code!!!" << endl;
   return (false);
}


