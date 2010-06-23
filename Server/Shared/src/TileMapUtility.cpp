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

#include "TileMapUtility.h"
#include "TileMapProperties.h"
#include "MapUtility.h"
#include "GenericMap.h"
#include "MapSettings.h"
#include "GfxFeatureMap.h"
#include "GfxData.h"
#include "GfxConstants.h"
#include "GfxPolygon.h"
#include "Item.h"
#include "BuiltUpAreaItem.h"
#include "BusRouteItem.h"
#include "GroupItem.h"
#include "PointOfInterestItem.h"
#include "RouteableItem.h"
#include "StreetItem.h"
#include "StreetSegmentItem.h"
#include "ServerTileMapFormatDesc.h"

#include "FilterSettings.h"

#include "GfxBBox.h"

// Copied the m_scaleLevels from MapUtility, to change some levels to
// fit with the tile sizes.
const TileMapUtility::scalelevel_t
TileMapUtility::m_scaleLevels[] = 
{
   { "Continent",
      700000000   // E.g.: Europe in 200*200 pixels
   },
   { "Country",
      170000000   // 17000000    // E.g.: Sweden in 200*200 pixels
   },
   { "County",
      17000000    // E.g.: "Götaland" in 200*200 pixels
   },
   { "Small county",
      500000      // E.g.: "Skåne" in 200*200 pixels
   },
   { "Municipal",
      50000       // E.g.: Municipal Lund in 200*200 pixels
   },
   { "City",
      5000        // E.g.: Malmö in 200*200 pixels
   },
   { "Small city",
      500         // E.g.: Lund in 200*200 pixels
   },
   { "District",
      140          // E.g.: Norra Fäladen in 300*300 pixels
   },
   { "Block",
      70          // E.g.: Most zoomed in Jakarta 200x200 pixels (40)
   },
   { "Part of block",
      22          // 
   },
   { "Detailed street",
      3           // 
   }
};

const uint32 TileMapUtility::m_nbrScales = 
   sizeof(m_scaleLevels) / sizeof(scalelevel_t);


int
TileMapUtility::getScaleLevel(const MC2BoundingBox* bbox, 
                          int widthPixel, int heightPixel) 
{
   // Add one to make sure that curScale not become 0
   const uint32 curScale = 
      uint32( int64(bbox->getLonDiff())*int64(bbox->getHeight()) *
              GfxConstants::SQUARE_MC2SCALE_TO_SQUARE_METER / 
              (widthPixel * heightPixel))+1;

   int scaleLevel = 0;
   while ( (scaleLevel < int(m_nbrScales)-1) && 
           (m_scaleLevels[scaleLevel].maxScale > curScale)) {
      mc2dbg8 << scaleLevel << ":" << m_scaleLevels[scaleLevel].maxScale 
              << " > " << curScale << endl;
      scaleLevel++;
   }

   mc2dbg8 << " boxH:" << bbox->getHeight() 
           << " boxW:" << bbox->getLonDiff()
           << " hPix:" << heightPixel
           << " wPix:" << widthPixel
           << endl;
   mc2dbg8 << "[TMU] getScaleLevel curScale = " << curScale 
               << " <=> scaleLevel " 
           << scaleLevel 
           << " = \"" << m_scaleLevels[scaleLevel].name << "\"" << endl;
   return (scaleLevel);
}


int
TileMapUtility::getScaleLevel(const MC2BoundingBox* bbox,
                          int widthPixel, int heightPixel,
                          uint32 factor)
{
   // Add one to make sure that curScale not become 0
   const uint32 curScale = 
      uint32( bbox->getArea() * factor * 
              GfxConstants::SQUARE_MC2SCALE_TO_SQUARE_METER / 
              (widthPixel * heightPixel))+1;

   int scaleLevel = 0;
   while ( (scaleLevel < int(m_nbrScales)-1) && 
           (m_scaleLevels[scaleLevel].maxScale > curScale)) {
      scaleLevel++;
   }

   return (scaleLevel);
}

namespace {
uint32
toDrawItem( Item& item, 
            GfxBBox& bbox,
            MC2BoundingBox* viewBBox,
            int widthPixel, int heightPixel, 
            bool countryMap, bool useStreets, 
            GenericMap* theMap,
            bool alwaysIncludedFromCountryMap,
            byte extraPOIInfo,
            const char* imageName )
{  
 
   
   // Check if we want to check items that always should be included
   // from the country map (ie. items that are removed from the 
   // underlying maps and moved to the country maps).
   if (countryMap && alwaysIncludedFromCountryMap) {
      if ( item.getItemType() == ItemTypes::waterItem &&
           MapUtility::getFeatureTypeForItem( &item, NULL ) != 
           GfxFeature::WATER_LINE ) {
         return CONTINENT_LEVEL;
      } else {
         return MAX_UINT32;
      }
   }   

   // Different case for different item-types
   switch (item.getItemType()) {
      case (ItemTypes::airportItem):
      case (ItemTypes::builtUpAreaItem) : {
         // BuiltUpAreas
         return MapUtility::getScaleLevel( 
                     bbox, widthPixel, heightPixel, 400 );
      }
      break;

      case (ItemTypes::streetSegmentItem) : {
         // Don't send the ssi if we are using streets and it is
         // already included in a street.
         if (!( useStreets && theMap->isPartOfStreet(&item) )) {
            // Main roadclass streetsegments
            StreetSegmentItem& ssi = static_cast<StreetSegmentItem&>(item);
            if ( viewBBox->getWidth() >
                 (400000 * GfxConstants::METER_TO_MC2SCALE) ) // hack
            {
               // INFO: Quickfix II
               if ( ssi.getRoadClass() != ItemTypes::mainRoad ) {
                  // Don't add
                  //return MAX_UINT32;
               }
            }

            switch (ssi.getRoadClass()) {
               case (ItemTypes::mainRoad) :
                  return SMALL_COUNTY_LEVEL;
                  break;
               case (ItemTypes::firstClassRoad) :
                  return CITY_LEVEL;
                  break;
               case (ItemTypes::secondClassRoad) :
                  return CITY_LEVEL;
                  break;
               case (ItemTypes::thirdClassRoad) :
                  return SMALL_CITY_LEVEL;
                  break;
               default :
                  return DISTRICT_LEVEL;
                  break;
            }
            // We don't get here
            return MAX_UINT32;
         } 
      }
      break;

      case (ItemTypes::streetItem) : {
         if (countryMap) {
            // No "childs" (street segment items), include!
            mc2dbg4 << "   including StreetItem!" << endl;
            return SMALL_COUNTY_LEVEL;
         } else if (useStreets) {
            // Use streets
            switch (GET_ZOOMLEVEL(item.getID())) {
               // Main road
               case (1) :
                  return COUNTY_LEVEL;
                  return SMALL_COUNTY_LEVEL;
                  break;
               // First class
               case (2) :
                  return CITY_LEVEL;
               // Second class
               case (3) :
                  return CITY_LEVEL;
                  break;
               // Third class
               case (4) :
                  return SMALL_CITY_LEVEL;
                  break;
               // Third class
               default :
                  return DISTRICT_LEVEL;
                  break;
            }
         }
      }
      break;
      
      case (ItemTypes::ferryItem) : {
         return (SMALL_COUNTY_LEVEL);
      } 
      break;
      
      case (ItemTypes::railwayItem) : {
         return COUNTRY_LEVEL;
      } 
      break;
      
      case (ItemTypes::buildingItem) : {
         return PART_OF_BLOCK_LEVEL;
      } 
      break;

      case (ItemTypes::individualBuildingItem) : {
         return PART_OF_BLOCK_LEVEL;
      } 
      break;

      case (ItemTypes::waterItem) : {
         if ( countryMap && !alwaysIncludedFromCountryMap ) {
            // The water items should already be added.
            return MAX_UINT32;
         }
         if ( MapUtility::getFeatureTypeForItem( &item, NULL ) != 
              GfxFeature::WATER_LINE ) {
            // Water that is not water line
            return CONTINENT_LEVEL;
         }
         else {
            // Water line
            return MapUtility::getScaleLevel( 
                     bbox, widthPixel, heightPixel, 300 );
         }
      }
      break;

      case (ItemTypes::islandItem) : {
         // Island item, they will be included early to make sure that we
         // don't draw street in the water.
         return MapUtility::getScaleLevel( 
                     bbox, widthPixel, heightPixel, 1000 );
      }
      break;

      case (ItemTypes::parkItem) : {
         // Park
         return MapUtility::getScaleLevel( 
                     bbox, widthPixel, heightPixel, 500 );
      }
      break;

      case (ItemTypes::forestItem) : {
        // Forest
         return MAX_UINT32; // not drawing forest for now.
         //return MapUtility::getScaleLevel( 
         //            bbox, widthPixel, heightPixel, 300 ); // please optimize
      }
      
      case (ItemTypes::pointOfInterestItem) : {
         PointOfInterestItem& poi = 
            static_cast<PointOfInterestItem&> (item);
         switch (poi.getPointOfInterestType()) {
            case ( ItemTypes::cityCentre ): 
               if ( extraPOIInfo <= 7 ) {
                  return CONTINENT_LEVEL;
               } else {
                  return COUNTY_LEVEL;
               }

            case ( ItemTypes::airport ):
               return COUNTRY_LEVEL;

// Features below this point are only included in tile level 3 maps.
            case ( ItemTypes::ferryTerminal ):
               return MUNICIPAL_LEVEL;

// Features below this point are only included in tile level 2 maps.
            case ( ItemTypes::publicSportAirport ):
            case ( ItemTypes::marina ):
            case ( ItemTypes::tollRoad ):
            case ( ItemTypes::hospital ):
               return CITY_LEVEL;

// Features below this point are only included in tile level 1 maps.
            //case ( ItemTypes::cityHall ):
            //case ( ItemTypes::courtHouse ):
            //case ( ItemTypes::school ):
            case ( ItemTypes::amusementPark ):
            case ( ItemTypes::skiResort ):
            case ( ItemTypes::golfCourse ):
            case ( ItemTypes::railwayStation ):
               return SMALL_CITY_LEVEL;
//               // Extra info contains GDF display class.
//               if ( extraPOIInfo <= 2 ) {        // Capital of country
//                  return CONTINENT_LEVEL;
//               } else if ( extraPOIInfo <= 4 ) { // >= 1,000,000
//                  return COUNTRY_LEVEL;
//               } else if ( extraPOIInfo <= 5 ) { // 500k - 1000k
//                  return COUNTY_LEVEL;
//               } else if ( extraPOIInfo <= 7 ) { // 100k - 500k
//                  return SMALL_COUNTY_LEVEL;
//               } else if ( extraPOIInfo <= 8 ) { // 50k - 100k
//                  return MUNICIPAL_LEVEL;
//               } else if ( extraPOIInfo <= 10 ) {// 10k - 50k
//                  return CITY_LEVEL;
//               } else {
//                  return DISTRICT_LEVEL;
//               }
//               case ( ItemTypes::hospital ):
            case ( ItemTypes::busStation ):
            case ( ItemTypes::library ):
               return DISTRICT_LEVEL;

// Features below this point are only included in tile level 0 maps.

            case ( ItemTypes::museum ):
            case ( ItemTypes::university ):
            case ( ItemTypes::theatre ):
            case ( ItemTypes::touristAttraction ):
            case ( ItemTypes::touristOffice ):
            case ( ItemTypes::tramStation ):
            //case ( ItemTypes::sportsActivity ):
               return BLOCK_LEVEL;
            
            case ( ItemTypes::postOffice ):
            //case ( ItemTypes::bank ):
            case ( ItemTypes::rentACarFacility ):
            //case ( ItemTypes::policeStation ):
            case ( ItemTypes::parkingGarage ):
            case ( ItemTypes::openParkingArea ):
            case ( ItemTypes::parkAndRide ):
            case ( ItemTypes::petrolStation ):
            case ( ItemTypes::wlan ):
            //case ( ItemTypes::atm ):
            //case ( ItemTypes::restArea ):
            //case ( ItemTypes::bowlingCentre ):
            case ( ItemTypes::placeOfWorship ): // remove later
            case ( ItemTypes::church ):
            case ( ItemTypes::mosque ):
            case ( ItemTypes::synagogue ):
            case ( ItemTypes::hinduTemple ):
            case ( ItemTypes::buddhistSite ):
            case ( ItemTypes::cafe ):
            case ( ItemTypes::subwayStation ):
            case ( ItemTypes::casino ):
            case ( ItemTypes::cinema ):
            case ( ItemTypes::historicalMonument ):
            //case ( ItemTypes::vehicleRepairFacility ):
            //case ( ItemTypes::iceSkatingRink ):
            //case ( ItemTypes::recreationFacility ):
            case ( ItemTypes::commuterRailStation ):
            case ( ItemTypes::hotel ):
            case ( ItemTypes::nightlife ):
            case ( ItemTypes::restaurant ):
               return PART_OF_BLOCK_LEVEL;

             case ( ItemTypes::shop ):
               // only add special custom pois for shops
               if ( extraPOIInfo != MAX_BYTE &&
                    extraPOIInfo &
                    ServerTileMapFormatDesc::SPECIAL_CUSTOM_POI_MASK ) {
                  return PART_OF_BLOCK_LEVEL;
               }
               break;            
            //case ( ItemTypes::winery ):
            //case ( ItemTypes::groceryStore ):
               return DETAILED_STREET_LEVEL;            
            case ( ItemTypes::company ): {
               // Only include company poi:s with custom images.
               if ( imageName != NULL ) {
                  return SMALL_CITY_LEVEL;     
               }
               break;
            }
            default:
               break;
         }
      }
      break;

      case (ItemTypes::cartographicItem) : {
         return (CITY_LEVEL);
      } 
      break;

      case (ItemTypes::aircraftRoadItem) : {
         return (SMALL_CITY_LEVEL);
      } 
      break;
      
      default :
         break;
   }

   return MAX_UINT32;
}

}

uint32
TileMapUtility::toDrawItem( Item& item,
                            uint32 minScale, uint32 maxScale,
                            MC2BoundingBox* viewBBox,
                            int widthPixel, int heightPixel, 
                            bool countryMap, bool useStreets, 
                            GenericMap* theMap,
                            const MC2Coordinate& itemCoord,
                            bool alwaysIncludedFromCountryMap,
                            byte extraPOIInfo,
                            const char* imageName ) 
{

   // if we do not have gfx data and are not a POI
   // OR if we useStreets and we dont have a map,
   // then return.

   if ( ( item.getGfxData() == NULL && 
          item.getItemType() != ItemTypes::pointOfInterestItem ) ||
        ( useStreets && theMap == NULL ) ) {
      return MAX_UINT32;
   }
   
   GfxBBox bbox( item.getGfxData() );
   uint32 scaleLevel =  ::toDrawItem( item, bbox,
                                      viewBBox,
                                      widthPixel, heightPixel,
                                      countryMap, useStreets,
                                      theMap,
                                      alwaysIncludedFromCountryMap,
                                      extraPOIInfo, imageName );

   if ( scaleLevel >= minScale && 
        scaleLevel <= maxScale ) {
      if (item.getItemType() != ItemTypes::pointOfInterestItem) {
         // Not a poi, gfxdata present
         if (!viewBBox->overlaps( bbox )) {
            return MAX_UINT32;
         }
      } else {
         // Poi, use the supplied coordinate.
         if (! viewBBox->contains( itemCoord )) {
            // Poi not inside view bbox.
            return MAX_UINT32;
         }
      }
   }

   return scaleLevel;
}



uint32
TileMapUtility::toDrawStreetPolygon( StreetItem* street, uint16 polygon )
{  
   // Check indata
   if (street == NULL) {
      return MAX_UINT32;
   }
   
   // Choose scalelevel depending on roadclass.
   switch (street->getRoadClassForPolygon(polygon)) {
      case (ItemTypes::mainRoad) :
         return SMALL_COUNTY_LEVEL;
         break;
      case (ItemTypes::firstClassRoad) :
         return CITY_LEVEL;
         break;
      case (ItemTypes::secondClassRoad) :
         return CITY_LEVEL;
         break;
      case (ItemTypes::thirdClassRoad) :
         return SMALL_CITY_LEVEL;
         break;
      default :
         return DISTRICT_LEVEL;
         break;
   }

}

uint32 TileMapUtility::filterFactor(uint32 meterValue){
   uint32 result = static_cast<uint32>
      (floor((meterValue*TileMapProperties::areaFilterFactor)+0.5));
   mc2dbg8 << "filterFactor: " << meterValue << " m -> " << result << " factor"
          << endl;
   return result;
}


bool 
TileMapUtility::getFilterSettings(FilterSettings* settings,
                              Item* item, int32 scale,
                              uint16 polygon)
{
            
   // Reset the filtersettings, this means not filtering...
   settings->reset();
   settings->m_maxWayDist = MAX_UINT32;

   switch (item->getItemType()) {
      case ItemTypes::builtUpAreaItem :
      case ItemTypes::parkItem :
      case ItemTypes::individualBuildingItem :
      case ItemTypes::buildingItem :
      case ItemTypes::forestItem :
      case ItemTypes::waterItem :
      case ItemTypes::islandItem :
      case ItemTypes::airportItem :
      case ItemTypes::cartographicItem :
      case ItemTypes::aircraftRoadItem :
         settings->m_filterType = FilterSettings::CLOSED_POLYGON_FILTER;
         switch ( scale ) {
            case ( CONTINENT_LEVEL ): 
               if ( item->getItemType() == ItemTypes::waterItem ) {
                  settings->m_filterType = FilterSettings::DOUGLAS_PEUCKER_POLYGON_FILTER;
               }
               settings->m_maxLatDist = filterFactor(50000); // 50km
               return true;
               break;

            case ( COUNTRY_LEVEL ): 
               if ( item->getItemType() == ItemTypes::waterItem ) {
                  settings->m_filterType = FilterSettings::DOUGLAS_PEUCKER_POLYGON_FILTER;
               }
               settings->m_maxLatDist = filterFactor(25000); // 25km
               return true;
               break;

            case ( COUNTY_LEVEL ): 
               if ( item->getItemType() == ItemTypes::waterItem ) {
                  settings->m_filterType = FilterSettings::DOUGLAS_PEUCKER_POLYGON_FILTER;
               }
               settings->m_maxLatDist = filterFactor(6000); // 6km
               return true;
               break;

            case ( SMALL_COUNTY_LEVEL ): 
               if ( item->getItemType() == ItemTypes::waterItem ) {
                  settings->m_filterType = FilterSettings::DOUGLAS_PEUCKER_POLYGON_FILTER;
               }
               settings->m_maxLatDist = filterFactor(3000); // 3km
               return true;
               break;

            case ( MUNICIPAL_LEVEL ): 
               if ( item->getItemType() == ItemTypes::waterItem ) {
                  settings->m_filterType = FilterSettings::DOUGLAS_PEUCKER_POLYGON_FILTER;
               }
               settings->m_maxLatDist = filterFactor(1500); // 1.5km
               return true;
               break;

            case ( CITY_LEVEL ): 
               settings->m_maxLatDist = filterFactor(300); // 300m 
               return true;
               break;

            case ( SMALL_CITY_LEVEL ): 
               settings->m_maxLatDist = filterFactor(100); // 100m  
               return true;
               break;

            case ( DISTRICT_LEVEL ): 
               settings->m_maxLatDist = filterFactor(20); // 20m    
               return true;
               break;

            case ( BLOCK_LEVEL ): 
            case ( PART_OF_BLOCK_LEVEL ): 
            case ( DETAILED_STREET_LEVEL ): 
               settings->m_maxLatDist = filterFactor(3); // 3m    
               return true;
               break;

            default:
               break;
         }               
      
      case ItemTypes::streetSegmentItem :
      case ItemTypes::railwayItem:
         settings->m_filterType = FilterSettings::OPEN_POLYGON_FILTER;

         switch ( scale ) {
            case ( CONTINENT_LEVEL ): 
                  settings->m_maxLatDist = 50000; // 50km
                  return true;
               break;

            case ( COUNTRY_LEVEL ): 
                  settings->m_maxLatDist = 25000; // 25km
                  return true;
               break;

            case ( COUNTY_LEVEL ): 
                  settings->m_maxLatDist = 12000; // 12km
                  return true;
               break;

            case ( SMALL_COUNTY_LEVEL ): 
                  settings->m_maxLatDist = 1250; // 1250m // Changed from 3km
                  return true;
               break;

            case ( MUNICIPAL_LEVEL ): 
                  settings->m_maxLatDist = 600; // 600m // Changed from 1.5km
                  return true;
               break;

            case ( CITY_LEVEL ): 
                  settings->m_maxLatDist = 175; // 175m // Changed from 700m
                  return true;
               break;

            case ( SMALL_CITY_LEVEL ): 
                  settings->m_maxLatDist = 50; // 50m  // Changed from 300
                  return true;
               break;

            case ( DISTRICT_LEVEL ): 
                  settings->m_maxLatDist = 15; // 15m // Changed from 300
                  return true;
               break;

            case ( BLOCK_LEVEL ): 
            case ( PART_OF_BLOCK_LEVEL ): 
            case ( DETAILED_STREET_LEVEL ): 
                  settings->m_maxLatDist = 3; // 3m   // Changed from 1
                  return true;
               break;

            default:
               break;
         }         

      default:
         return false;

   }

   return false;
}

