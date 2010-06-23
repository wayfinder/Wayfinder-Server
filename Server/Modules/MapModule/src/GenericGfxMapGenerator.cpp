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

#include "Stack.h"
#include "GenericGfxMapGenerator.h"


GenericGfxMapGenerator::GenericGfxMapGenerator( MapHandler *mh,
                                  uint32 *mapIDs,
                                  uint32 port )
      : MapGenerator( mh, 
                      mapIDs, 
                      1,
                      port) 
{
   #ifdef PRINT_GFX_STATISTICS
      // Reset the statistics-members
      resetStatistics();
   #endif
}


GenericGfxMapGenerator::~GenericGfxMapGenerator()
{

}


bool
GenericGfxMapGenerator::writeItem(Item* item, 
                           DataBuffer* itemBuffer, 
                           GenericMap* theMap,
                           uint16 polygon) 
{
   bool reduceGfxToPoint;
   // Check if we shall include the given item
   if (!includeItem(item, theMap, reduceGfxToPoint, polygon)) {
      // Nono, this item should not be included
      return (false);
   } else {

      // OK, the item is valid and should be included,
      // write it into the databuffer

      #ifdef PRINT_GFX_STATISTICS
         // Set the position to be able to calculate statistics
         uint32 firstPosition = itemBuffer->getCurrentOffset();
      #endif
  

      // Get the GfxData of the item
      GfxData* gfx = item->getGfxData();
     
      // Only send items with gfx-data and coordinate. 
      // If the gfx has only one coordinate 
      // (e.g. a water textpoint) the length is 0 which then is ok
      if ( (gfx != NULL) && 
           (((gfx->getNbrCoordinates(0) > 0) && (gfx->getLength(0) > 0)) ||
            (gfx->getNbrCoordinates(0) == 1) )  &&
           (item->getItemType() != ItemTypes::pointOfInterestItem)) {
         // Handle items with GfxData
         
         // Check if the item should be filtered
         uint32 maxFilterDistance = MAX_UINT32;
         uint32 minFilterDistance = 0;
         /*
         switch (item->getItemType()) {
            // 
            // Do not filter municipals since the filter-algorithm does not
            // work properly
            //case (ItemTypes::municipalItem) :
            //   maxFilterDistance = 500;
            //   minFilterDistance = 0;
            //   break;
            //
            case (ItemTypes::builtUpAreaItem) :
               maxFilterDistance = 300;
               minFilterDistance = 0;
               break;
            case (ItemTypes::cityPartItem) :
               maxFilterDistance = 300;
               minFilterDistance = 0;
               break;
            case (ItemTypes::buildingItem) :
               maxFilterDistance = 300;
               minFilterDistance = 0;
               break;
            case (ItemTypes::parkItem) :
               maxFilterDistance = 300;
               minFilterDistance = 0;
               break;
            
            // Do not filter water-items since they might have holes.
            //case (ItemTypes::waterItem) :
            //   maxFilterDistance = 500;
            //   minFilterDistance = 0;
            //   break;
            default :
               // Do nothing...
               break;
         }
      */
         
         // Start writing data to the DataBuffer
         itemBuffer->writeNextShort((uint16) item->getItemType());
         
         uint16 nbrPolygons = 0;

      
         // Make sure gfxdata is not null (some municipals may have
         // gfxdata == NULL).
         
         if (gfx != NULL) {
            if ( reduceGfxToPoint || ( polygon != MAX_UINT16 ) )
               nbrPolygons = 1; // Just send one polygon
            else
               nbrPolygons = gfx->getNbrPolygons(); // Should be 16 bits
         }
         
         itemBuffer->writeNextShort(nbrPolygons);
         
         // Nbr names.
         byte nbrNames = item->getNbrNames();
         itemBuffer->writeNextByte(nbrNames);

         
         // Write road class if streetsegment or street item.
         itemBuffer->alignToLong();
         if (item->getItemType() == ItemTypes::streetSegmentItem) {
            StreetSegmentItem* ssi = static_cast<StreetSegmentItem*> (item);
            itemBuffer->writeNextLong((uint32) ssi->getRoadClass());
            mc2dbg4 << "   RoadClass = " << (uint32) ssi->getRoadClass()
                    << endl;
         } else if (item->getItemType() == ItemTypes::streetItem) {
            StreetItem* street = static_cast<StreetItem*> (item);
            itemBuffer->writeNextLong(street->getRoadClassForPolygon(polygon));

         // In case of a buildingItem, send the building pointOfInterest_t
         } else if (item->getItemType() == ItemTypes::buildingItem) {
            BuildingItem* bItem = static_cast<BuildingItem*> (item);
            itemBuffer->writeNextLong(uint32(bItem->getBuildingType()));

         // In case of an individualBuildingItem send the 
         // building pointOfInterest_t
         } else if (item->getItemType() == ItemTypes::individualBuildingItem) {
            IndividualBuildingItem* ibItem = 
                        static_cast<IndividualBuildingItem*> (item);
            itemBuffer->writeNextLong(uint32(ibItem->getBuildingType()));
         }

         
         mc2dbg8 << "Itemtype = " << (uint32) item->getItemType()
              << ", nbrNames = " << (uint32) nbrNames
              << ", nbrPolygones = " << (uint32) nbrPolygons << endl;

         itemBuffer->writeNextLong(mapIDs[0]);
         itemBuffer->writeNextLong( item->getID() );
         mc2dbg8 << "   MapID = 0x" << hex << mapIDs[0]
                 << ", ItemID = 0x" << item->getID() 
                 << dec << endl;

         for (byte i=0; i<nbrNames; i++) { 
            itemBuffer->writeNextLong(item->getRawStringIndex(i));
            mc2dbg8 << "   StringIndex = " 
                    << item->getRawStringIndex(i) << endl;
         }
         
         // Write GfxData
         // Check if to only write middle of bbox...
         if ( reduceGfxToPoint ) {
            // Only write single point
            int32 lat, lon;
            MC2BoundingBox bbox;
            gfx->getMC2BoundingBox(bbox);
            bbox.getCenter( lat, lon );
            itemBuffer->writeNextLong( 1 ); // Only one point
            itemBuffer->writeNextLong( lat ); // Lat
            itemBuffer->writeNextLong( lon ); // Lon
            

         } else {
            // Write the whole gfxdata
            uint16 startPolygon;
            uint16 endPolygon;
            if (polygon == MAX_UINT16) {
               // Send all polygons, just as usual.
               startPolygon = 0;
               endPolygon = nbrPolygons;
            } else {
               // Just send the specified polygon.
               startPolygon = polygon;
               endPolygon = polygon + 1;
            }

            if (maxFilterDistance != MAX_UINT32) {
               for (uint16 poly = startPolygon; 
                    poly < endPolygon; poly++) {
                  // Filter this polygon
                  
                  Stack indices(1024);
                  if ((gfx->getSimplifiedPolygon(&indices, 
                                                poly,
                                                maxFilterDistance, 
                                                minFilterDistance)) &&
                           (indices.getStackSize() > 0)) {
                     uint32 curNbrCoord = indices.getStackSize();
                     mc2dbg4 << "Saved " 
                             << gfx->getNbrCoordinates(poly)-curNbrCoord
                             << " coordinates by filtering ="
                             << double(curNbrCoord)/ 
                                    gfx->getNbrCoordinates(poly) 
                             << "% ("
                             << gfx->getNbrCoordinates(poly) 
                             << "), type=" << (int) item->getItemType()
                             << theMap->getFirstItemName(item) << endl;
                     
                     // Save the number of coordinates
                     itemBuffer->writeNextLong(curNbrCoord);
                     
                     // Write the coordinates
                     uint32 curIndex = indices.pop();
                     while (curIndex != MAX_UINT32) {
                        itemBuffer->writeNextLong(gfx->getLat(poly,curIndex));
                        itemBuffer->writeNextLong(gfx->getLon(poly,curIndex));
                        curIndex = indices.pop();
                     }
                  } else {
                     // Failed to simplify the Gfx
                     uint32 curNbrCoord = gfx->getNbrCoordinates(poly);
                     // Save the number of coordinates
                     itemBuffer->writeNextLong(curNbrCoord);
                     
                     // Write the coordinates
                     for (uint32 i=0; i<curNbrCoord; i++) {            
                        itemBuffer->writeNextLong(gfx->getLat(poly, i) );
                        itemBuffer->writeNextLong(gfx->getLon(poly, i) );
                     }
                  }

               } // next polygon

            } else {
               for (uint16 poly = startPolygon; 
                    poly < endPolygon; poly++) {
                  
                  itemBuffer->writeNextLong( gfx->getNbrCoordinates(poly) );
                  for (uint32 i = 0; 
                       i < gfx->getNbrCoordinates(poly); i++) {
                     
                     itemBuffer->writeNextLong( gfx->getLat(poly,i) );
                     itemBuffer->writeNextLong( gfx->getLon(poly,i) );
                  }
               }
            }
         } // reduceGfxToPoint (else)

         #ifdef PRINT_GFX_STATISTICS
            // Calculate statistics
            calculateStatistics(itemBuffer->getCurrentOffset() - 
                                firstPosition,
                                item);
         #endif

         // In case of a waterItem, send whether it's a closed or
         // open polygon.
         if (item->getItemType() == ItemTypes::waterItem) {
            bool closed = false;
            if ((gfx != NULL) && (gfx->closed())) {
               closed = true;
            }
            itemBuffer->writeNextBool(closed);
            itemBuffer->writeNextByte(
                  (static_cast<WaterItem*> (item))->getWaterType());
         }
      
         return (true);
      } else if ((item->getItemType() == ItemTypes::pointOfInterestItem)) {
         
         
         // Fill in the common stuff.
         itemBuffer->writeNextShort((uint16) item->getItemType());
         
         itemBuffer->writeNextShort(0); // Nbr polygons
         byte nbrNames = item->getNbrNames();
         itemBuffer->writeNextByte(nbrNames);

         itemBuffer->writeNextLong(mapIDs[0]);
         itemBuffer->writeNextLong( item->getID() );

         for (byte i=0; i<nbrNames; i++) { 
            itemBuffer->writeNextLong(item->getRawStringIndex(i));
         }
         
         // Instead of ordinary gfxData, send poi_type and lat/lon for
         // the streetsegment item the poi is located on.
         PointOfInterestItem* poii = static_cast<PointOfInterestItem*> (item);
         mc2dbg4 << "Checking POI " << theMap->getFirstItemName(poii) 
                 << endl;

         // The GfxClient does not handle the new church sub types,
         // so map (at least) church back to placeOfWorship
         // (drawn with a church symbol in GfxClient)
         ItemTypes::pointOfInterest_t type = poii->getPointOfInterestType();
         if ( type == ItemTypes::church ) {
            type = ItemTypes::placeOfWorship;
         }
         itemBuffer->writeNextLong(type);
            
         int32 lat, lon;
         if ( ( gfx != NULL ) && 
              ( gfx->getNbrCoordinates( 0 ) > 0 ) )  {
            lat = gfx->getLat( 0, 0 );
            lon = gfx->getLon( 0, 0 );
         } else {
            theMap->getItemCoordinates(poii->getID(), 
                                       poii->getOffsetOnStreet(),
                                       lat, lon);
         }
         
         itemBuffer->writeNextLong(lat);
         itemBuffer->writeNextLong(lon);
         #ifdef PRINT_GFX_STATISTICS
         // Calculate statistics
         calculateStatistics(
            itemBuffer->getCurrentOffset() - firstPosition, item);
         #endif
         return (true);
      } else {
         return (false);
      }
   }
}

void
GenericGfxMapGenerator::writeItemsInZoomLevel(GenericMap* theMap,
                                       DataBuffer* itemBuffer,
                                       uint32 zoomLevel, 
                                       uint32& nbrItems)
{
   #ifdef PRINT_GFX_STATISTICS
      // Reset the statistics-members
      resetStatistics();
   #endif
   uint32 nbrItemsWithZoom = theMap->getNbrItemsWithZoom(zoomLevel);
   mc2dbg8 << "Nbr items in zoom " << nbrItemsWithZoom << endl;
   for (uint32 i=0; i<nbrItemsWithZoom; i++) {
      
      Item* item = theMap->getItem(zoomLevel, i);
      if (item != NULL) {
         // Check if we're about to send a streetitem
         if (item->getItemType() == ItemTypes::streetItem) {
            // We need check if we are to send each polygon for the
            // streetitems.
            GfxData* gfx = item->getGfxData();
            if (gfx != NULL) {
               for (uint16 poly = 0; poly < gfx->getNbrPolygons(); poly++) {
                  if (writeItem(item, itemBuffer, theMap, poly)) {
                     nbrItems++;
                     m_nbrItemsPerItemType[item->getItemType()]++;
                  }
               }
            }
         } 
         else if (writeItem(theMap->getItem(zoomLevel, i), itemBuffer, 
                  theMap)) {
            nbrItems++;
            m_nbrItemsPerItemType[item->getItemType()]++;
            DEBUG4(
               if (item->getItemType() == ItemTypes::municipalItem) {
                  mc2dbg << "=== Added municipal: ";
                  theMap->printItemNames(item->getID());
                  mc2dbg << " to gfx-map. Contains ";
                  if (item->getGfxData() == NULL)
                     mc2dbgt << "(null)";
                  else
                     mc2dbg << item->getGfxData()->getNbrCoordinates();
                  
                  mc2dbg << " coordinates" << endl;
               }
            );
         }
      }
   }
   #ifdef PRINT_GFX_STATISTICS
      // Print the statistics
      printStatistics(zoomLevel, theMap);
   #endif
}


#ifdef PRINT_GFX_STATISTICS
   void
   GenericGfxMapGenerator::resetStatistics()
   {
      m_nbrMunicipals = 0;
      m_munizipalSize = 0;
      m_nbrStreetSegments = 0;
      m_streetSegmentSize = 0;
      m_nbrStreets = 0;
      m_streetSize = 0;
      m_nbrCityParts = 0;
      m_cityPartSize = 0;
      m_nbrWaters = 0;
      m_waterSize = 0;
      m_nbrPark = 0;
      m_parkSize = 0;
      m_nbrBuildings = 0;
      m_buildingSize = 0;
      m_nbrIndividualBuildings = 0;
      m_individualBuildingSize = 0;
      m_nbrBuiltUpAreas = 0;
      m_builtUpAreaSize = 0;
      m_nbrIslands = 0;
      m_islandSize = 0;
      m_nbrRailways = 0;
      m_railwaySize = 0;
      m_nbrFerries = 0;
      m_ferrySize = 0;
      m_nbrPointOfInterest = 0;
      m_pointOfInterestSize = 0;
      m_nbrDefault = 0;
      m_defaultSize = 0;
   }

   void
   GenericGfxMapGenerator::calculateStatistics(uint32 itemLength,
                                        Item* item)
   {
      // Log the number and size of the different item-types
      switch (item->getItemType()) {
         case (ItemTypes::municipalItem) :
            m_nbrMunicipals++;
            m_munizipalSize += itemLength;
            break;
         case (ItemTypes::streetSegmentItem) :
            m_nbrStreetSegments++;
            m_streetSegmentSize += itemLength;
            break;
         case (ItemTypes::streetItem) :
            m_nbrStreets++;
            m_streetSize += itemLength;
            break;
         case (ItemTypes::cityPartItem) :
            m_nbrCityParts++;
            m_cityPartSize += itemLength;
            break;
         case (ItemTypes::waterItem) :
            m_nbrWaters++;
            m_waterSize += itemLength;
            break;
         case (ItemTypes::parkItem) :
            m_nbrPark++;
            m_parkSize += itemLength;
            break;
         case (ItemTypes::buildingItem) :
            m_nbrBuildings++;
            m_buildingSize += itemLength;
            break;
         case (ItemTypes::individualBuildingItem) :
            m_nbrIndividualBuildings++;
            m_individualBuildingSize += itemLength;
            break;
         case (ItemTypes::builtUpAreaItem) :
            m_nbrBuiltUpAreas++;
            m_builtUpAreaSize += itemLength;
            break;
         case (ItemTypes::islandItem) :
            m_nbrIslands++;
            m_islandSize += itemLength;
            break;
         case (ItemTypes::railwayItem) :
            m_nbrRailways++;
            m_railwaySize += itemLength;
            break;
         case (ItemTypes::ferryItem) :
            m_nbrFerries++;
            m_ferrySize += itemLength;
            break;
         case (ItemTypes::pointOfInterestItem) :
            m_nbrPointOfInterest++;
            m_pointOfInterestSize += itemLength;
            break;
         default :
            m_nbrDefault++;
            m_defaultSize += itemLength;
            break;
      }
   }

   void
   GenericGfxMapGenerator::printStatistics(uint32 zoomLevel, GenericMap* theMap)
   {
      mc2dbg << "Sent " 
           << (m_nbrMunicipals + m_nbrStreetSegments + m_nbrStreets + 
               m_nbrCityParts + m_nbrWaters + m_nbrPark + 
               m_nbrBuildings +  m_nbrIndividualBuildings + 
               m_nbrBuiltUpAreas +
               m_nbrIslands + m_nbrRailways + m_nbrPointOfInterest +
               m_nbrDefault)
           << " items in zoomlevel " << zoomLevel 
           << " in map " << theMap->getMapID() << ", size=" 
           << (m_munizipalSize + m_streetSegmentSize +
               m_streetSize + m_cityPartSize +
               m_waterSize + m_parkSize + m_buildingSize + 
               m_individualBuildingSize +
               m_builtUpAreaSize + m_islandSize + m_railwaySize +
               m_ferrySize +
               m_pointOfInterestSize + 
               m_defaultSize)
           << endl;

      mc2dbg << "   Municipals: " << m_nbrMunicipals 
           << ", size=" << m_munizipalSize << endl;
      mc2dbg << "   StreetSegments: " << m_nbrStreetSegments
           << ", size=" << m_streetSegmentSize << endl;
      mc2dbg << "   Streets: " << m_nbrStreets
           << ", size=" << m_streetSize << endl;
      mc2dbg << "   CityParts: " << m_nbrCityParts
           << ", size=" << m_cityPartSize << endl;
      mc2dbg << "   Waters: " << m_nbrWaters
           << ", size=" << m_waterSize << endl;
      mc2dbg << "   Parks: " << m_nbrPark
           << ", size=" << m_parkSize << endl;
      mc2dbg << "   Buildings: " << m_nbrBuildings
           << ", size=" << m_buildingSize << endl;
      mc2dbg << "   IndividualBuildings: " << m_nbrIndividualBuildings
           << ", size=" << m_individualBuildingSize << endl;
      mc2dbg << "   BuiltUpAreas: " << m_nbrBuiltUpAreas
           << ", size=" << m_builtUpAreaSize << endl;
      mc2dbg << "   Islands: " << m_nbrIslands
           << ", size=" << m_islandSize << endl;
      mc2dbg << "   Railways: " << m_nbrRailways
           << ", size=" << m_railwaySize << endl;
      mc2dbg << "   Ferries: " << m_nbrFerries
           << ", size=" << m_ferrySize << endl;
      mc2dbg << "   POIs: " << m_nbrPointOfInterest
           << ", size=" << m_pointOfInterestSize << endl;
      mc2dbg << "   Other items: " << m_nbrDefault
           << ", size=" << m_defaultSize << endl;
   }

   

#endif

