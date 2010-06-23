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

#include "MapUtility.h"
#include "GenericMap.h"
#include "MapSettings.h"
#include "GfxFeatureMap.h"
#include "GfxConstants.h"
#include "GfxPolygon.h"
#include "GfxRoadPolygon.h"
#include "GfxData.h"
#include "Item.h"
#include "BuildingItem.h"
#include "BuiltUpAreaItem.h"
#include "BusRouteItem.h"
#include "GroupItem.h"
#include "IslandItem.h"
#include "ParkItem.h"
#include "PointOfInterestItem.h"
#include "RouteableItem.h"
#include "StreetItem.h"
#include "StreetSegmentItem.h"
#include "WaterItem.h"
#include "IndividualBuildingItem.h"
#include "CartographicItem.h"
#include "ServerTileMapFormatDesc.h"
#include "POIImageIdentificationTable.h"
#include "FilterSettings.h"

const MapUtility::scalelevel_t
MapUtility::m_scaleLevels[] = 
{
   { "Continent",
      400000000   // E.g.: Europe in 200*200 pixels
   },
   { "Country",
      500000000   // E.g.: Sweden in 200*200 pixels
   },
   { "County",
      5000000     // E.g.: "Götaland" in 200*200 pixels
   },
   { "Small county",
      30000      // E.g.: "Skåne" in 200*200 pixels
   },
   { "Municipal",
      7000         // E.g.: Municipal Lund in 200*200 pixels
   },
   { "City",
      1000        // E.g.: Malmö in 200*200 pixels
   },
   { "Small city",
      300         // E.g.: Lund in 200*200 pixels
   },
   { "District",
      50          // E.g.: Norra Fäladen in 300*300 pixels
   },
   { "Block",
      24          // 
   },
   { "Part of block",
      10          // 
   },
   { "Detailed street",
      2           // 
   }
};


const uint32 MapUtility::m_nbrScales = 
   sizeof(m_scaleLevels) / sizeof(scalelevel_t);



int 
MapUtility::getScaleLevel( int64 area, 
                           int widthPixel, int heightPixel,
                           uint32 factor ) {
   // Add one to make sure that curScale not become 0
   const uint32 curScale = 
      uint32( area * factor * 5 *
              GfxConstants::SQUARE_MC2SCALE_TO_SQUARE_METER / 
              (widthPixel * heightPixel))+1;

   int scaleLevel = 0;
   while ( (scaleLevel < int(m_nbrScales)-1) && 
           (m_scaleLevels[scaleLevel].maxScale > curScale)) {
      scaleLevel++;
   }

   return scaleLevel;
}

int
MapUtility::getScaleLevel( const MC2BoundingBox& bbox, 
                           int widthPixel, int heightPixel )
{
   return getScaleLevel( bbox.getArea(),
                         widthPixel, heightPixel, 
                         1 ); // factor 1
}


int
MapUtility::getScaleLevel( const MC2BoundingBox& bbox,
                           int widthPixel, int heightPixel,
                           uint32 factor )
{
   return getScaleLevel( bbox.getArea(), 
                         widthPixel, heightPixel,
                         factor );
}


const char* 
MapUtility::getScaleLevelName(uint32 scaleLevel)
{
   if (scaleLevel < m_nbrScales) {
      return m_scaleLevels[scaleLevel].name;
   }

   return "Invalid scalelevel";
}


uint32
MapUtility::toDrawItem( Item* item, MC2BoundingBox* viewBBox,
                        int widthPixel, int heightPixel, 
                        bool countryMap, bool useStreets, 
                        GenericMap* theMap,
                        int32 lat, int32 lon,
                        bool alwaysIncludedFromCountryMap,
                        byte extraPOIInfo )
{  
   // Check indata
   if ( (item == NULL) || 
        ( (item->getGfxData() == NULL) && 
          (item->getItemType() != ItemTypes::pointOfInterestItem) ) ||
        (useStreets && (theMap == NULL) ) ) {
      return MAX_UINT32;
   }
 
   // Get boundingbox for item and make sure that it overlaps the map
   MC2BoundingBox bbox;
   if (item->getItemType() != ItemTypes::pointOfInterestItem) {
      // Not a poi, gfxdata present
      item->getGfxData()->getMC2BoundingBox(bbox);
      if (!viewBBox->overlaps(bbox)) {
         return MAX_UINT32;
      }
   } else {
      // Poi, use the supplied coordinate.
      if (! viewBBox->contains(lat, lon)) {
         // Poi not inside view bbox.
         return MAX_UINT32;
      }
   }
   
   // Check if we want to check items that always should be included
   // from the country map (ie. items that are removed from the 
   // underlying maps and moved to the country maps).
   if (countryMap && alwaysIncludedFromCountryMap) {
      // Water that is not water line should be included
      if ( item->getItemType() == ItemTypes::waterItem && 
           getFeatureTypeForItem( item, NULL ) != GfxFeature::WATER_LINE ) {
         return CONTINENT_LEVEL;
      } else if ( item->getItemType() == ItemTypes::forestItem ) {
         return COUNTY_LEVEL;
      }
      else {
         return MAX_UINT32;
      }
   }   

   // Different case for different item-types
   switch (item->getItemType()) {

      case (ItemTypes::builtUpAreaItem) : {
         // BuiltUpAreas
         return getScaleLevel( bbox, widthPixel, heightPixel, 400 );
      }
      break;

      case (ItemTypes::streetSegmentItem) : {
         // Don't send the ssi if we are using streets and it is
         // already included in a street.
         if (!( useStreets && theMap->isPartOfStreet(item) )) {
            // Main roadclass streetsegments
            StreetSegmentItem* ssi = static_cast<StreetSegmentItem*> (item);
            if ( viewBBox->getWidth() >
                 (400000 * GfxConstants::METER_TO_MC2SCALE) )
            {
               // FIXME: Quickfix II
               if ( ssi->getRoadClass() != ItemTypes::mainRoad ) {
                  // Don't add
                  return MAX_UINT32;
               }
            }
            
            switch (ssi->getRoadClass()) {
               case (ItemTypes::mainRoad) :
                  return COUNTY_LEVEL;
                  break;
               case (ItemTypes::firstClassRoad) :
                  return SMALL_COUNTY_LEVEL;
                  break;
               case (ItemTypes::secondClassRoad) :
                  return MUNICIPAL_LEVEL;
                  break;
               case (ItemTypes::thirdClassRoad) :
                  return CITY_LEVEL;
                  break;
               default :
                  return SMALL_CITY_LEVEL;
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
            return COUNTY_LEVEL;
         } else if (useStreets) {
            // Use streets
            switch (GET_ZOOMLEVEL(item->getID())) {
               // Main road
               case (1) :
                  return COUNTY_LEVEL;
                  break;
               // First class
               case (2) :
                  return SMALL_COUNTY_LEVEL;
                  break;
               // Second class
               case (3) :
                  return MUNICIPAL_LEVEL;
                  break;
               // Third class
               case (4) :
                  return CITY_LEVEL;
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
      
      case (ItemTypes::cartographicItem) : {
         return (CITY_LEVEL);
      }

      case (ItemTypes::buildingItem) : {
         return (DISTRICT_LEVEL);
      } 
      break;

      case (ItemTypes::individualBuildingItem) : {
         return (DISTRICT_LEVEL);
      } 
      break;

      case (ItemTypes::aircraftRoadItem) : {
         return getScaleLevel( bbox, widthPixel, heightPixel, 300 );
      } 
      break;

      case (ItemTypes::waterItem) : {
         if ( countryMap && !alwaysIncludedFromCountryMap ) {
            // The water items should already be added.
            return MAX_UINT32;
         }
         if ( getFeatureTypeForItem( item, NULL ) != GfxFeature::WATER_LINE ) {
            // Water that is not water line
            return CONTINENT_LEVEL;
         }
         else {
            // Water line
            return getScaleLevel( bbox, widthPixel, heightPixel, 300 ); 
         }
      }
      break;

      case (ItemTypes::islandItem) : {
         // Island item, they will be included early to make sure that we
         // don't draw street in the water.
         return getScaleLevel( bbox, widthPixel, heightPixel, 1000 );
      }
      break;
      case (ItemTypes::parkItem) : {
         // Park
         return getScaleLevel( bbox, widthPixel, heightPixel, 500 );
      }
      break;

      case (ItemTypes::forestItem) : {
         // Forest
         return COUNTY_LEVEL;
      }
      
      case (ItemTypes::pointOfInterestItem) : {

         PointOfInterestItem* poi = 
            static_cast<PointOfInterestItem*> (item);
         switch (poi->getPointOfInterestType()) {
            
            case ( ItemTypes::airport ):
               return getScaleLevel( bbox, widthPixel, heightPixel, 400 );
            
            case ( ItemTypes::ferryTerminal ):
               return MUNICIPAL_LEVEL;
            
            case ( ItemTypes::publicSportAirport ):
            case ( ItemTypes::marina ):
            case ( ItemTypes::tollRoad ):
               return CITY_LEVEL;

            case ( ItemTypes::railwayStation ):            
               return SMALL_COUNTY_LEVEL;

            //case ( ItemTypes::cityHall ):
            //case ( ItemTypes::courtHouse ):
            //case ( ItemTypes::school ):
            case ( ItemTypes::amusementPark ):
            case ( ItemTypes::skiResort ):
            case ( ItemTypes::golfCourse ):
            case ( ItemTypes::hospital ):
               return SMALL_CITY_LEVEL;

            case ( ItemTypes::cityCentre ):
               if ( extraPOIInfo <= 4 ) {
                  return COUNTRY_LEVEL;
               } else if( extraPOIInfo <= 6 ) {
                  return COUNTY_LEVEL;
               } else if ( extraPOIInfo <= 8 ) {
                  return SMALL_COUNTY_LEVEL;
               } else if( extraPOIInfo <= 11 ) {
                  return MUNICIPAL_LEVEL;
               } else {
                  return CITY_LEVEL;
               } 
            case ( ItemTypes::subwayStation ):
            case ( ItemTypes::busStation ):
            case ( ItemTypes::library ):
               return DISTRICT_LEVEL;

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
            case ( ItemTypes::casino ):
            case ( ItemTypes::cinema ):
            case ( ItemTypes::historicalMonument ):
            //case ( ItemTypes::vehicleRepairFacility ):
            //case ( ItemTypes::iceSkatingRink ):
            //case ( ItemTypes::recreationFacility ):
            case ( ItemTypes::commuterRailStation ):
            case ( ItemTypes::restaurant ):
            case ( ItemTypes::hotel ):
            case ( ItemTypes::cafe ):
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
            //case ( ItemTypes::nightlife ):
            
            //case ( ItemTypes::groceryStore ):
               return DETAILED_STREET_LEVEL;            
            default:
               break;
         }
      }
      break;

      case (ItemTypes::airportItem) : {
         return getScaleLevel( bbox, widthPixel, heightPixel, 400 );
      } 
      break;
      
      default :
         break;
   }
   
   return MAX_UINT32;
}


uint32
MapUtility::toDrawStreetPolygon( StreetItem* street, uint16 polygon )
{  
   // Check indata
   if (street == NULL) {
      return MAX_UINT32;
   }
   
   // Choose scalelevel depending on roadclass.
   switch (street->getRoadClassForPolygon(polygon)) {
      case (ItemTypes::mainRoad) :
         return COUNTRY_LEVEL;
         break;
      case (ItemTypes::firstClassRoad) :
         return COUNTY_LEVEL;
         break;
      case (ItemTypes::secondClassRoad) :
         return SMALL_COUNTY_LEVEL;
         break;
      case (ItemTypes::thirdClassRoad) :
         return CITY_LEVEL;
         break;
      default :
         return DISTRICT_LEVEL;
         break;
   }
}

bool 
MapUtility::getFilterSettings(FilterSettings* settings,
                              Item* item, int32 scale,
                              uint16 polygon)
{
   // Reset the filtersettings, this means not filtering...
   settings->reset();

   // Turn on filtering of water and forest at zoomed out levels, otherwise there
   // will be quite a lot of data for GfxModule since currently we don't
   // have pre-filtered water/forests.
   switch ( item->getItemType() ) {
      case ItemTypes::forestItem :
      case ItemTypes::waterItem :
         switch ( scale ) {
            case CONTINENT_LEVEL:
               settings->m_filterType = FilterSettings::DOUGLAS_PEUCKER_POLYGON_FILTER;
               settings->m_maxLatDist = 1500; // 1500 m
               return true;
               break;
            case COUNTRY_LEVEL:
               settings->m_filterType = FilterSettings::DOUGLAS_PEUCKER_POLYGON_FILTER;
               settings->m_maxLatDist = 750; // 750 m
               return true;
               break;
            case COUNTY_LEVEL:
               settings->m_filterType = FilterSettings::DOUGLAS_PEUCKER_POLYGON_FILTER;
               settings->m_maxLatDist = 375; // 375 m
               return true;
               break;
            default:
               break;
         }
         break;
      default:
         break;
   }
   
   return false;
}


int
MapUtility::getDrawOrder(const GfxFeature* feature, uint32 poly, bool border)
{
   switch (feature->getType()) {
      case GfxFeature::LAND :
         return 0;
      case GfxFeature::BUILTUP_AREA :
         return 1;
      case GfxFeature::BORDER :
         return 2;
      case GfxFeature::WATER :
         return 3;
      case GfxFeature::WATER_LINE :
         return 4;
      case GfxFeature::FERRY :
         return 5;
      case GfxFeature::ISLAND : 
      case GfxFeature::ISLAND_IN_BUA :
         return 6;
      case GfxFeature::BUA_ON_ISLAND:
      case GfxFeature::CARTOGRAPHIC_GROUND:
      case GfxFeature::CARTOGRAPHIC_GREEN_AREA:
      case GfxFeature::PARK :
         return 7;
      case GfxFeature::NATIONALPARK :
         return 8;
      case GfxFeature::FOREST :
         return 9;
      case GfxFeature::BUILDING :
         return 10;
      case GfxFeature::PEDESTRIANAREA :
      case GfxFeature::WATER_IN_PARK :
         return 11;
      case GfxFeature::AIRPORTGROUND :
      case GfxFeature::ISLAND_IN_WATER_IN_PARK :
      case GfxFeature::ISLAND_IN_WATER_IN_PARK_BUA :
      case GfxFeature::ISLAND_IN_WATER_IN_PARK_ISLAND :
         return 12;
      case GfxFeature::INDIVIDUALBUILDING :
      case GfxFeature::AIRCRAFTROAD :
         return 13;
      case GfxFeature::RAILWAY :
         return 14;
      case GfxFeature::WALKWAY:
         return 15 + ( border ? 0 : 1 );
      // The draw order for the streets depends on the attributes in the 
      // polygon and the border-parameter. The drawOrder-value for the 
      // streets will be in the intervall 100-200
      case GfxFeature::STREET_FOURTH :
      case GfxFeature::STREET_THIRD :
      case GfxFeature::STREET_SECOND :
      case GfxFeature::STREET_FIRST :
      case GfxFeature::STREET_MAIN : {
         int rcOffset = 0;
         switch (feature->getType()) {
            case GfxFeature::STREET_FOURTH :
               rcOffset = 1;
               break;
            case GfxFeature::STREET_THIRD :
               rcOffset = 3;
               break;
            case GfxFeature::STREET_SECOND :
               rcOffset = 5;
               break;
            case GfxFeature::STREET_FIRST :
               rcOffset = 7;
               break;
            case GfxFeature::STREET_MAIN :
               rcOffset = 9;
               break;
            default :
               break;
         }

         if ( (static_cast<GfxRoadPolygon*>(feature->getPolygon(poly))
               ->getEntryRestrictions(0) != ItemTypes::noRestrictions) ||
              (static_cast<GfxRoadPolygon*>(feature->getPolygon(poly))
               ->getEntryRestrictions(1) != ItemTypes::noRestrictions)) {
            --rcOffset;
         }
         

         const int level0 = static_cast<GfxRoadPolygon*>
                                 (feature->getPolygon(poly))->getLevel0();
         const int level1 = static_cast<GfxRoadPolygon*>
                                 (feature->getPolygon(poly))->getLevel1();

         if (abs(level0-level1) > 1) {
            mc2log << warn << here << " Level diff = " << int(level0-level1)
                   << ", l0=" << level0 << ", l1=" << level1 << endl;
         }

         uint32 levelAndBorderOffset = 0;
         switch (level0+level1) {
            case -4:
               if (border)
                  levelAndBorderOffset = 0;
               else
                  levelAndBorderOffset = 2;
               break;
            
            case -3:
               if (border)
                  levelAndBorderOffset = 1;
               else
                  levelAndBorderOffset = 4;
               break;
            
            case -2:
               mc2log << warn << "Levelsum == -2, l0=" << level0 
                      << ", l1=" << level1 << endl;
               if (border)
                  levelAndBorderOffset = 1;
               else
                  levelAndBorderOffset = 4;
               break;
            
            case -1:
               if (border)
                  levelAndBorderOffset = 3;
               else
                  levelAndBorderOffset = 6;
               break;
            
            case 0:
               if (border)
                  levelAndBorderOffset = 5;
               else
                  levelAndBorderOffset = 8;
               break;
            
            case 1:
               if (border)
                  levelAndBorderOffset = 7;
               else
                  levelAndBorderOffset = 10;
               break;
            
            case 2:
               mc2log << warn << "Levelsum == 2, l0=" << level0 
                      << ", l1=" << level1 << endl;
               if (border)
                  levelAndBorderOffset = 7;
               else
                  levelAndBorderOffset = 10;
               break;
            
            case 3:
               if (border)
                  levelAndBorderOffset = 9;
               else
                  levelAndBorderOffset = 12;
               break;

            case 4:
               if (border)
                  levelAndBorderOffset = 11;
               else
                  levelAndBorderOffset = 13;
               break;
         }
        
         mc2dbg4 << "Order for RoadPolygon: " 
                 << (100 + levelAndBorderOffset*10+rcOffset) << ", rcOffset="
                 << rcOffset << ", levelAndBorderOffset=" 
                 << levelAndBorderOffset << ", level0=" << level0 
                 << ", level1=" << level1 << endl;
         return (100 + levelAndBorderOffset*10+rcOffset);
      }

      // The features that will be drawn after all streets
      case GfxFeature::BUILTUP_AREA_SQUARE :
         return 241;
      case GfxFeature::BUILTUP_AREA_SMALL :
         return 242;
      case GfxFeature::ROUTE:
         return 243;
      case GfxFeature::ROUTE_ORIGIN:
         return 244;
      case GfxFeature::ROUTE_DESTINATION:
         return 245;
      case GfxFeature::PARK_CAR:
         return 246;
      case GfxFeature::TRAFFIC_INFO :
         return 252;
      case GfxFeature::POI : {
         const GfxPOIFeature* poi =
            static_cast<const GfxPOIFeature*>(feature);
         if( poi->getPOIType() == ItemTypes::cityCentre ) {
            switch( poi->getExtraInfo() ) {
               case 1: return 253;
               case 2: return 254;
               case 3: return 255;
               case 4: return 256;
               case 5: return 257;
               case 6: return 258;
               case 7: return 259;
               case 8: return 260;
               case 9: return 261;
               case 10: return 262;
               case 11: return 263;
               case 12: return 264;
                  
               default: return 265;
            }
         } else {
            return 266;
         }
      }
      case GfxFeature::SYMBOL :
         return 268;

      default :
         return -1;
   }
   return -1; // Will never get here...
}

bool
MapUtility::getDrawSettings(GfxFeature::gfxFeatureType type,
                            int scale,
                            MapSettings* mapSettings,
                            DrawSettings* settings,
                            const GfxFeature* feature,
                            bool border,
                            GfxPolygon* poly,
                            const GfxFeatureMap* featureMap,
                            DrawSettings::poiStatus_t poiStatus,
                            const POIImageIdentificationTable* imageTable)
{   
   MapSetting* mapSetting = mapSettings->getSettingFor( type, 
                                                        scale ); 

   if ( mapSetting != NULL ) {
      // Found setting use it
      bool onMap = false;
      settings->m_style = mapSetting->m_drawStyle;
      settings->m_symbol = mapSetting->m_drawSymbol;
      if ( border ) {
         settings->m_color = mapSetting->m_borderColor;
         settings->m_lineWidth = mapSetting->m_borderLineWidth;
         onMap = mapSetting->m_borderOnMap;
      } else {
         settings->m_color = mapSetting->m_drawColor;
         settings->m_lineWidth = mapSetting->m_lineWidth;
         onMap = mapSetting->m_onMap;
      }
      if ( type == GfxFeature::STREET_MAIN || 
           type == GfxFeature::STREET_FIRST ||
           type == GfxFeature::STREET_SECOND ||
           type == GfxFeature::STREET_THIRD ||
           type == GfxFeature::STREET_FOURTH )
      {
         // Adjust the width of ramp-, roundabout- and multidig.-segments.
         GfxRoadPolygon* roadPoly = dynamic_cast<GfxRoadPolygon*>(poly);
         if ( ( roadPoly != NULL) && 
              ( roadPoly->isRamp() || roadPoly->isRoundabout() || 
                roadPoly->isMultidigitialized() ) &&
              ( ( (settings->m_lineWidth > 4) && (border)) ||
                ( (settings->m_lineWidth > 2) && (!border)) ) )  
         {
            settings->m_lineWidth -= 2;
         }
         // Adjust the color where vehicles not are allowed
         mc2dbg4 << here << "roadPoly: entryRest0=" 
                 << int(roadPoly->getEntryRestrictions(0)) << ", entryRest1=" 
                 << int(roadPoly->getEntryRestrictions(1)) << endl;

         if ( ( roadPoly != NULL) && 
              ( roadPoly->getEntryRestrictions(0) != 
                  ItemTypes::noRestrictions) &&
              ( roadPoly->getEntryRestrictions(1) != 
                  ItemTypes::noRestrictions)) {
            mc2dbg1 << here << " Adjusting color due to entry restr." << endl;
            if (border)
               settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::SLATEGREY );
            else
               settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::LIGHTGREY );
         }

      } else if ( type == GfxFeature::POI ) {
         const GfxPOIFeature* feat = 
            static_cast<const GfxPOIFeature*> ( feature );
         settings->m_poiType = feat->getPOIType();
         settings->m_poiStatus = poiStatus;
         if ( feat->getCustomImageName() != "" ) {
            settings->m_symbolImage = feat->getCustomImageName().c_str();
            settings->m_symbol = DrawSettings::USER_DEFINED;
         }
      } else if ( type == GfxFeature::ROUTE_ORIGIN ) {
         if ( featureMap != NULL ) {
            settings->m_transportationType = 
               featureMap->getTransportationType();
            settings->m_startingAngle = featureMap->getStartingAngle();
            settings->m_drivingOnRightSide = 
               featureMap->getDrivingOnRightSide();
            settings->m_initialUTurn = featureMap->getInitialUTurn();
         } else {
            settings->m_transportationType = ItemTypes::drive;
            settings->m_startingAngle = 64; // Right
            settings->m_drivingOnRightSide = true;
            settings->m_initialUTurn = false; 
         }
      }
      if ((type == GfxFeature::BUILTUP_AREA_SQUARE) ||
          (type == GfxFeature::BUILTUP_AREA_SMALL))
         onMap = false;
      return onMap;
   } else {
   switch (type) {
      case (GfxFeature::ISLAND_IN_WATER_IN_PARK_ISLAND) :
      case (GfxFeature::ISLAND) :
      case (GfxFeature::LAND) : {
         // Land-area
         settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::LAND ); 
         settings->m_style = DrawSettings::FILLED;
         return (true);
      }

      case (GfxFeature::BORDER) : {
         // border item
         settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::BORDER );
         settings->m_style = DrawSettings::LINE;
         settings->m_lineWidth = 1;
         switch (scale) {
            case CONTINENT_LEVEL :
            case COUNTRY_LEVEL :
            case COUNTY_LEVEL :
               break;
            case SMALL_COUNTY_LEVEL :
            case MUNICIPAL_LEVEL :
            case CITY_LEVEL :
               break;
            case SMALL_CITY_LEVEL :
            case DISTRICT_LEVEL :
            case BLOCK_LEVEL :
            case PART_OF_BLOCK_LEVEL :
            case DETAILED_STREET_LEVEL :
               return (false);
         }
         return (true);
      }
      
      case GfxFeature::BUA_ON_ISLAND :
      case GfxFeature::ISLAND_IN_WATER_IN_PARK_BUA :
      case GfxFeature::ISLAND_IN_BUA :
      case (GfxFeature::BUILTUP_AREA) : {
         // BuiltUpAreas
         settings->m_style = DrawSettings::FILLED;
         if ( scale >= SMALL_CITY_LEVEL) {
            settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::BUA );
         } else {
             settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::BUA_OUT );
         }
         return (true);
      }

      case (GfxFeature::BUILTUP_AREA_SQUARE) : {
         // BuiltUpAreas
         settings->m_style = DrawSettings::SYMBOL;
         settings->m_symbol = DrawSettings::SQUARE_3D_SYMBOL;
         return (false);
      }

      case (GfxFeature::BUILTUP_AREA_SMALL) : {
         // BuiltUpAreas
         settings->m_style = DrawSettings::SYMBOL;
         settings->m_symbol = DrawSettings::SMALL_CITY_SYMBOL;
         return (false);
      }
         
      case GfxFeature::WATER_IN_PARK:
      case (GfxFeature::WATER) : {
         // Water
         settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::WATER );
         settings->m_style = DrawSettings::FILLED;
         return (true);
      }
      case (GfxFeature::WATER_LINE) : {
         // Water
         settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::WATER );

         switch (scale) { 
            case CONTINENT_LEVEL :
            case COUNTRY_LEVEL :
            case COUNTY_LEVEL :
            case SMALL_COUNTY_LEVEL :
            case MUNICIPAL_LEVEL :
            case CITY_LEVEL :
            case SMALL_CITY_LEVEL :
            case DISTRICT_LEVEL :
               settings->m_lineWidth = 1;
               break;
            case BLOCK_LEVEL :
               settings->m_lineWidth = 2;
               break;
            case PART_OF_BLOCK_LEVEL :
               settings->m_lineWidth = 5;
               break;
            case DETAILED_STREET_LEVEL :
               settings->m_lineWidth = 9;
               break;

         }
         settings->m_style = DrawSettings::LINE;
         return (true);
      }
       
      case (GfxFeature::FERRY) : {
         // Ferry item
         settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::WHITE );
         settings->m_style = DrawSettings::LINE;
         switch (scale) {
            case CONTINENT_LEVEL :
            case COUNTRY_LEVEL :
               return (false);
            case COUNTY_LEVEL :
            case SMALL_COUNTY_LEVEL :
            case MUNICIPAL_LEVEL :
            case CITY_LEVEL :
            case SMALL_CITY_LEVEL :
            case DISTRICT_LEVEL :
            case BLOCK_LEVEL :
               settings->m_lineWidth = 1;
               break;
            case PART_OF_BLOCK_LEVEL :
               settings->m_lineWidth = 2;
               break;
            case DETAILED_STREET_LEVEL :
               settings->m_lineWidth = 3;
               break;
         }
         return (true);
      }
      
      case (GfxFeature::RAILWAY) : {
         // Railway item
         settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::RAILWAY );
         settings->m_style = DrawSettings::LINE;
         switch (scale) {
            case CONTINENT_LEVEL :
               return (false);
            case COUNTRY_LEVEL :
            case COUNTY_LEVEL :
            case SMALL_COUNTY_LEVEL :
               settings->m_lineWidth = 1;
               break;
            case MUNICIPAL_LEVEL :
            case CITY_LEVEL :
            case SMALL_CITY_LEVEL :
            case DISTRICT_LEVEL :
            case BLOCK_LEVEL :
            case PART_OF_BLOCK_LEVEL :
            case DETAILED_STREET_LEVEL :
               settings->m_lineWidth = 2;
               break;
         }
         return (true);
      }
      
      case (GfxFeature::AIRPORTGROUND) : {
         // airport item
         settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::AIRPORT_GROUND );
         settings->m_style = DrawSettings::FILLED;
         return (true);
      }
     
      case (GfxFeature::AIRCRAFTROAD) : {
         // airport item
         settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::AIRCRAFT_ROAD );
         settings->m_style = DrawSettings::FILLED;
         settings->m_lineWidth = 1;
         return (true);
      }
 
      case (GfxFeature::BUILDING) : {
         // building item
         settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::BUILDING_AREA );
         settings->m_style = DrawSettings::FILLED;
         return (true);
      }
      
      case (GfxFeature::INDIVIDUALBUILDING) : {
         // Individual building item
         settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::BUILDING_OUTLINE );
         settings->m_style = DrawSettings::FILLED;
         return (true);
      }

      case GfxFeature::CARTOGRAPHIC_GREEN_AREA: {
         settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::NEW_FOREST );
         settings->m_style = DrawSettings::FILLED;
         return true;
      }
      case GfxFeature::CARTOGRAPHIC_GROUND: {
         settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::BUILDING_AREA );
         settings->m_style = DrawSettings::FILLED;
         return true;
      }

      case (GfxFeature::ISLAND_IN_WATER_IN_PARK) :
      case (GfxFeature::PARK) : {
         // Park
         settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::NEW_FOREST );
         settings->m_style = DrawSettings::FILLED;
         return (true);
      }
      case (GfxFeature::NATIONALPARK) : {
         // Park
         settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::NEW_FOREST );
         settings->m_style = DrawSettings::CLOSED;
         return (true);
      }
       
      case (GfxFeature::FOREST) : {
         // Forest
         settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::NEW_FOREST );
         settings->m_style = DrawSettings::FILLED;
         return (true);
      }

      case (GfxFeature::ROUTE) : {
         // Piece of route
         settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::RED );
         settings->m_style = DrawSettings::LINE;
         settings->m_lineWidth = 3;
         return (true);
      }

      case (GfxFeature::ROUTE_ORIGIN) : {
         // The route start
         mc2dbg8 << "MapUtility::getDrawSettings ROUTE_ORIGIN" 
                 << endl;
         settings->m_style = DrawSettings::SYMBOL;
         settings->m_symbol = DrawSettings::ROUTE_ORIGIN_SYMBOL;
         if ( featureMap != NULL ) {
            settings->m_transportationType = 
               featureMap->getTransportationType();
            settings->m_startingAngle = featureMap->getStartingAngle();
            settings->m_drivingOnRightSide = 
               featureMap->getDrivingOnRightSide();
            settings->m_initialUTurn = featureMap->getInitialUTurn();
         } else {
            settings->m_transportationType = ItemTypes::drive;
            settings->m_startingAngle = 64; // Right
            settings->m_drivingOnRightSide = true;
            settings->m_initialUTurn = false; 
         }
         return (true);
      }

      case (GfxFeature::ROUTE_DESTINATION) : {
         // The route end
         mc2dbg8 << "MapUtility::getDrawSettings ROUTE_DESTINATION" << endl;
         settings->m_style = DrawSettings::SYMBOL;
         settings->m_symbol = DrawSettings::ROUTE_DESTINATION_SYMBOL;
         return (true);
      }

      case (GfxFeature::PARK_CAR) : {
         // The route end
         mc2dbg8 << "MapUtility::getDrawSettings PARK_CAR" 
                 << endl;
         settings->m_style = DrawSettings::SYMBOL;
         settings->m_symbol = DrawSettings::ROUTE_PARK_SYMBOL;
         settings->m_featureType = type;
         return (true);
      }

      case (GfxFeature::SYMBOL) : {
         // A symbol
         mc2dbg8 << "MapUtility::getDrawSettings SYMBOL" 
                << endl;
         const GfxSymbolFeature* feat = 
            static_cast<const GfxSymbolFeature*> ( feature );
         settings->m_style = DrawSettings::SYMBOL;
         settings->m_symbol = DrawSettings::symbol_t( 
            DrawSettings::PIN + feat->getSymbol() );
         settings->m_symbolImage = feat->getSymbolImage();
         settings->m_featureType = type;
         return (true);
      }
      
      case (GfxFeature::STREET_MAIN) :
      case (GfxFeature::STREET_FIRST) :
      case (GfxFeature::STREET_SECOND) :
      case (GfxFeature::STREET_THIRD) :
      case (GfxFeature::STREET_FOURTH) :
      case (GfxFeature::WALKWAY) :
         return (getDrawSettingsSSI(type,
                                    scale,
                                    settings,
                                    border,
                                    dynamic_cast<GfxRoadPolygon*>(poly)));
      case GfxFeature::POI: {
         mc2dbg8 << "MapUtility::getDrawSettings POI" << endl;
         const GfxPOIFeature* feat = 
            static_cast<const GfxPOIFeature*> ( feature );

         // don't draw city centres  when more zoomed in than COUNTY_LEVEL
         if ( feat->getPOIType() == ItemTypes::cityCentre && 
              scale > COUNTY_LEVEL ) {
            return false;
         }

         settings->m_style = DrawSettings::SYMBOL;
         settings->m_symbol = DrawSettings::POI_SYMBOL;
         settings->m_featureType = type;
         settings->m_poiType = feat->getPOIType();
         settings->m_poiStatus = poiStatus;
         if ( ! feat->getCustomImageName().empty() ) {
            if ( imageTable ) {
               settings->m_symbolImage = 
                  imageTable->
                  getFullImageName( feat->getPOIType(),
                                    feat->getCustomImageName().c_str() );
            } else {
               settings->m_symbolImage = feat->getCustomImageName().c_str();
            }
            settings->m_symbol = DrawSettings::USER_DEFINED;
         }
         return (true); 
      }
      
      case GfxFeature::TRAFFIC_INFO: {
         mc2dbg8 << "MapUtility::getDrawSettings TRAFFIC_INFO" << endl;
         const GfxTrafficInfoFeature* feat = 
            static_cast<const GfxTrafficInfoFeature*> ( feature );
         settings->m_style = DrawSettings::SYMBOL;
         switch (feat->getTrafficInfoType()) {
            case (TrafficDataTypes::RoadWorks) :
               settings->m_symbol = DrawSettings::ROADWORK;
               break;
            case (TrafficDataTypes::MaintenanceWorks) :
               settings->m_symbol = DrawSettings::ROADWORK;
               break;
            case (TrafficDataTypes::ConstructionWorks) :
               settings->m_symbol = DrawSettings::ROADWORK;
               break;
            case (TrafficDataTypes::Camera) :
               settings->m_symbol = DrawSettings::SPEED_CAMERA;
               break;
            case (TrafficDataTypes::UserDefinedCamera) :
               settings->m_symbol = DrawSettings::USER_DEFINED_SPEED_CAMERA;
               break;
            default:
               settings->m_symbol = DrawSettings::DANGER;
               break;
         }
         return (true); 
      }
      default :
         break;
   } 
   }
   return (false);
}

bool
MapUtility::getDrawSettingsSSI(GfxFeature::gfxFeatureType type,
                               int scaleLevel,
                               DrawSettings* settings,
                               bool border,
                               GfxRoadPolygon* poly)
{
   settings->m_style = DrawSettings::LINE;

   switch (type) {
   case (GfxFeature::STREET_MAIN):{
         if (border) {
            settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::XL_ROAD_BORDER );
            switch (scaleLevel) {
               case CONTINENT_LEVEL :
                  return (false);
               case COUNTRY_LEVEL :
               case COUNTY_LEVEL :
                  settings->m_lineWidth = 1;
                  // settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::WHITE ); 
                  break;
               case SMALL_COUNTY_LEVEL :
                  // settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::WHITE );
                  settings->m_lineWidth = 5;
                  break;
               case MUNICIPAL_LEVEL :
                  //  settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::WHITE );
                  settings->m_lineWidth = 6;
                  break;
               case CITY_LEVEL :
                  settings->m_lineWidth = 7;
                  break;
               case SMALL_CITY_LEVEL :
               case DISTRICT_LEVEL :
                  settings->m_lineWidth = 10;
                  break;
               case BLOCK_LEVEL :
               case PART_OF_BLOCK_LEVEL :
                  settings->m_lineWidth = 15;
                  break;
               case DETAILED_STREET_LEVEL :
                  settings->m_lineWidth = 19;
                  break;
            }
         } else {
            settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::XL_ROAD );
            switch (scaleLevel) {
               case CONTINENT_LEVEL :
               case COUNTRY_LEVEL :
               case COUNTY_LEVEL :
                  return (false);
               case SMALL_COUNTY_LEVEL :
                  settings->m_lineWidth = 3;
                  break;
               case MUNICIPAL_LEVEL :
                  settings->m_lineWidth = 4;
                  break;
               case CITY_LEVEL :
                  settings->m_lineWidth = 5;
                  break;
               case SMALL_CITY_LEVEL :
               case DISTRICT_LEVEL :
                  settings->m_lineWidth = 8;
                  break;
               case BLOCK_LEVEL :
               case PART_OF_BLOCK_LEVEL :
                  settings->m_lineWidth = 13;
                  break;
               case DETAILED_STREET_LEVEL :
                  settings->m_lineWidth = 15;
                  break;
            }
         }
     } break;

      case (GfxFeature::STREET_FIRST) :          // = = = = = = 1:st class
         if (border) {
            settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::L_ROAD_BORDER );
            switch (scaleLevel) {
               case CONTINENT_LEVEL :
               case COUNTRY_LEVEL :
               case COUNTY_LEVEL :
                  return (false);
               case SMALL_COUNTY_LEVEL :
                  settings->m_lineWidth = 2;
                  break;
               case MUNICIPAL_LEVEL :
               case CITY_LEVEL :
                  settings->m_lineWidth = 5;
                  break;
               case SMALL_CITY_LEVEL :
               case DISTRICT_LEVEL :
                  settings->m_lineWidth = 8;
                  break;
               case BLOCK_LEVEL :
                  settings->m_lineWidth = 12;
                  break;
               case PART_OF_BLOCK_LEVEL :
                  settings->m_lineWidth = 14;
                  break;
               case DETAILED_STREET_LEVEL :
                  settings->m_lineWidth = 16;
                  break;
            }
         } else {
            settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::L_ROAD );
            switch (scaleLevel) {
               case CONTINENT_LEVEL :
               case COUNTRY_LEVEL :
               case COUNTY_LEVEL :
               case SMALL_COUNTY_LEVEL :
                  return (false);
               case MUNICIPAL_LEVEL :
               case CITY_LEVEL :
                  settings->m_lineWidth = 3;
                  break;
               case SMALL_CITY_LEVEL :
               case DISTRICT_LEVEL :
                  settings->m_lineWidth = 6;
                  break;
               case BLOCK_LEVEL :
                  settings->m_lineWidth = 10;
                  break;
               case PART_OF_BLOCK_LEVEL :
                  settings->m_lineWidth = 12;
                  break;
               case DETAILED_STREET_LEVEL :
                  settings->m_lineWidth = 14;
                  break;
            }
         }
      break;

      case (GfxFeature::STREET_SECOND) :          // = = = = = = 2:nd class
         if (border) {
            settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::M_ROAD_BORDER );
            switch (scaleLevel) {
               case CONTINENT_LEVEL :
               case COUNTRY_LEVEL :
               case COUNTY_LEVEL :
               case SMALL_COUNTY_LEVEL :
               case MUNICIPAL_LEVEL :
                  return (false);
               case CITY_LEVEL :
                  settings->m_lineWidth = 2;
                  break;
               case SMALL_CITY_LEVEL :
                  settings->m_lineWidth = 4;
                  break;
               case DISTRICT_LEVEL :
                  settings->m_lineWidth = 8;
                  break;
               case BLOCK_LEVEL :
               case PART_OF_BLOCK_LEVEL :
                  settings->m_lineWidth = 13;
                  break;
               case DETAILED_STREET_LEVEL :
                  settings->m_lineWidth = 15;
                  break;
            }
         } else {
            settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::M_ROAD );
            switch (scaleLevel) {
               case CONTINENT_LEVEL :
               case COUNTRY_LEVEL :
               case COUNTY_LEVEL :
               case SMALL_COUNTY_LEVEL :
               case MUNICIPAL_LEVEL :
               case CITY_LEVEL :
                  return (false);
               case SMALL_CITY_LEVEL :
                  settings->m_lineWidth = 2;
                  break;
               case DISTRICT_LEVEL :
                  settings->m_lineWidth = 6;
                  break;
               case BLOCK_LEVEL :
               case PART_OF_BLOCK_LEVEL :
                  settings->m_lineWidth = 11;
                  break;
               case DETAILED_STREET_LEVEL :
                  settings->m_lineWidth = 13;
                  break;
            }
         }
      break;

      case (GfxFeature::STREET_THIRD) :          // = = = = = = 3:rd class
         if (border) {
            settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::S_ROAD_BORDER );
            switch (scaleLevel) {
               case CONTINENT_LEVEL :
               case COUNTRY_LEVEL :
               case COUNTY_LEVEL :
               case SMALL_COUNTY_LEVEL :
               case MUNICIPAL_LEVEL :
                  return (false);
               case CITY_LEVEL :
                  settings->m_lineWidth = 3;
                  break;
               case SMALL_CITY_LEVEL :
                  settings->m_lineWidth = 4;
                  break;
               case DISTRICT_LEVEL :
                  settings->m_lineWidth = 5;
                  break;
               case BLOCK_LEVEL :
                  settings->m_lineWidth = 10;
                  break;
               case PART_OF_BLOCK_LEVEL :
                  settings->m_lineWidth = 12;
                  break;
               case DETAILED_STREET_LEVEL :
                  settings->m_lineWidth = 13;
                  break;
            }
         } else {
            settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::S_ROAD );
            switch (scaleLevel) {
               case CONTINENT_LEVEL :
               case COUNTRY_LEVEL :
               case COUNTY_LEVEL :
               case SMALL_COUNTY_LEVEL :
               case MUNICIPAL_LEVEL :
                  return (false); 
               case CITY_LEVEL :
                  settings->m_lineWidth = 1;
                  break;
               case SMALL_CITY_LEVEL :
                  settings->m_lineWidth = 2;
                  break;
               case DISTRICT_LEVEL :
                  settings->m_lineWidth = 3;
                  break;
               case BLOCK_LEVEL :
                  settings->m_lineWidth = 8;
                  break;
               case PART_OF_BLOCK_LEVEL :
                  settings->m_lineWidth = 10;
                  break;
               case DETAILED_STREET_LEVEL :
                  settings->m_lineWidth = 11;
                  break;
            }
         }
      break;

      case (GfxFeature::STREET_FOURTH) :          // = = = = = = 4:th class
         if (border) {
            settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::XS_ROAD_BORDER );
            switch (scaleLevel) {
               case CONTINENT_LEVEL :
               case COUNTRY_LEVEL :
               case COUNTY_LEVEL :
               case SMALL_COUNTY_LEVEL :
               case MUNICIPAL_LEVEL :
               case CITY_LEVEL :
               case SMALL_CITY_LEVEL :
                  return (false);
               case DISTRICT_LEVEL :
                  settings->m_lineWidth = 4;
                  break;
               case BLOCK_LEVEL :
                  settings->m_lineWidth = 7; 
                  break;
               case PART_OF_BLOCK_LEVEL :
                  settings->m_lineWidth = 11;
                  break;
               case DETAILED_STREET_LEVEL :
                  settings->m_lineWidth = 13;
                  break;
            }
         } else {
            settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::XS_ROAD );
            switch (scaleLevel) {
               case CONTINENT_LEVEL :
               case COUNTRY_LEVEL :
               case COUNTY_LEVEL :
               case SMALL_COUNTY_LEVEL :
               case MUNICIPAL_LEVEL :
               case CITY_LEVEL :
               case SMALL_CITY_LEVEL:
                  return (false);
               case DISTRICT_LEVEL:
                  settings->m_lineWidth = 2;
                  break;
               case BLOCK_LEVEL:
                  settings->m_lineWidth = 5;
                  break;
               case PART_OF_BLOCK_LEVEL :
                  settings->m_lineWidth = 9;
                  break;
               case DETAILED_STREET_LEVEL :
                  settings->m_lineWidth = 11;
                  break;
            }
         }
      break;
      case (GfxFeature::WALKWAY) :          // = = = = = = Walkway
         if (border) {
            settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::WALKWAY_BORDER );
            switch (scaleLevel) {
               case CONTINENT_LEVEL :
               case COUNTRY_LEVEL :
               case COUNTY_LEVEL :
               case SMALL_COUNTY_LEVEL :
               case MUNICIPAL_LEVEL :
               case CITY_LEVEL :
               case SMALL_CITY_LEVEL :
               case DISTRICT_LEVEL :
                  return (false);
               case BLOCK_LEVEL :
                  settings->m_lineWidth = 2; // Only border
                  break;
               case PART_OF_BLOCK_LEVEL :
                  settings->m_lineWidth = 4;
                  break;
               case DETAILED_STREET_LEVEL :
                  settings->m_lineWidth = 5;
                  break;
            }
         } else {
            settings->m_color = GDUtils::Color::makeColor( GDUtils::Color::WALKWAY );
            switch (scaleLevel) {
               case CONTINENT_LEVEL :
               case COUNTRY_LEVEL :
               case COUNTY_LEVEL :
               case SMALL_COUNTY_LEVEL :
               case MUNICIPAL_LEVEL :
               case CITY_LEVEL :
               case SMALL_CITY_LEVEL:
               case DISTRICT_LEVEL:
               case BLOCK_LEVEL:
                  return (false);
               case PART_OF_BLOCK_LEVEL :
                  settings->m_lineWidth = 2;
                  break;
               case DETAILED_STREET_LEVEL :
                  settings->m_lineWidth = 3;
                  break;
            }
         }
      break;

      default :
         return (false);
   }

   if ( poly != NULL && type != GfxFeature::WALKWAY ) {
      // Make the ramps, roundabouts and multi-dig roads thinner.
      if ( ( poly->isRamp() || poly->isRoundabout() || 
             poly->isMultidigitialized()) &&
           ( ( (settings->m_lineWidth > 4) && (border)) ||
             ( (settings->m_lineWidth > 2) && (!border)) ) ) {
         mc2dbg4 << "Makes the line 2 pixels thinner" << endl;
         settings->m_lineWidth -= 2;
      }

   }
   return true;
}


byte
MapUtility::getCountryFilterLevel(int scaleLevel) 
{
   byte countryFilterLevel = 0;
   
   switch (scaleLevel) {
      case CONTINENT_LEVEL :
         countryFilterLevel = 0;
         break;
      case COUNTRY_LEVEL :
         countryFilterLevel = 1;
         break;
      case COUNTY_LEVEL :
         countryFilterLevel = 2;
         break;
      case SMALL_COUNTY_LEVEL :
         countryFilterLevel = 3;
         break;
      case MUNICIPAL_LEVEL :
         countryFilterLevel = 4;
         break;
      case CITY_LEVEL :
         countryFilterLevel = 5;
         break;
      case SMALL_CITY_LEVEL :
         countryFilterLevel = 6;
         break;
      case DISTRICT_LEVEL :
         countryFilterLevel = 7;
         break;
      case BLOCK_LEVEL :
         countryFilterLevel = 8;
         break;
      case PART_OF_BLOCK_LEVEL :
         countryFilterLevel = 9;
         break;
      case DETAILED_STREET_LEVEL :
         countryFilterLevel = 10;
         break;
   }
   
   return (countryFilterLevel);  
}


void 
MapUtility::makeDrawItemParameters( uint16 width, uint16 height,
                                    MC2BoundingBox& bbox,
                                    float64& xFactor, 
                                    float64& yFactor )
{
   bbox.updateCosLat();
   xFactor = float64(width - 1) / bbox.getLonDiff();
   yFactor = float64(height - 1) / bbox.getHeight();
   
   // Make sure that the image will not have strange proportions.
   float64 factor = float64(bbox.getHeight()) / bbox.getWidth()
      * (width - 1) / (height - 1);
   if (factor < 1) {
      // Compensate for that the image is wider than it is high
      yFactor *= factor;
   } else {
      // Compensate for that the image is higher than it is wide
      xFactor /= factor;
   }
}


GfxFeature::gfxFeatureType
MapUtility::getFeatureTypeForItem( const Item* item, 
                                   const FilterSettings* filterSettings )
{
   switch (item->getItemType()) {
      case ItemTypes::streetSegmentItem : {
         const StreetSegmentItem* ssi = 
            static_cast<const StreetSegmentItem*>(item);
         ItemTypes::roadDisplayClass_t roadDisplayClass =
            ssi->getDisplayClass();
         if ( roadDisplayClass == ItemTypes::partOfWalkway ||
              roadDisplayClass == ItemTypes::roadForAuthorities ||
              roadDisplayClass == ItemTypes::etaParkingGarage ||
              roadDisplayClass == ItemTypes::etaParkingPlace ||
              roadDisplayClass == ItemTypes::etaUnstructTrafficSquare ) {
            return GfxFeature::WALKWAY;
         }
         switch (ssi->getRoadClass()) {
            case 0 :
               return (GfxFeature::STREET_MAIN);
            case 1 :
               return (GfxFeature::STREET_FIRST);
            case 2 :
               return (GfxFeature::STREET_SECOND);
            case 3 :
               return (GfxFeature::STREET_THIRD);
            default :
               return (GfxFeature::STREET_FOURTH);
         }
         } break;

      case ItemTypes::streetItem : {
         uint32 zoom = (item->getID()>>27) & 0x1f;
         switch (zoom) {
            case 1 :
               return (GfxFeature::STREET_MAIN);
            case 2 :
               return (GfxFeature::STREET_FIRST);
            case 3 :
               return (GfxFeature::STREET_SECOND);
            case 4 :
               return (GfxFeature::STREET_THIRD);
            default :
               return (GfxFeature::STREET_FOURTH);
            }
         } break;

      case ItemTypes::builtUpAreaItem : {
         const BuiltUpAreaItem* bua = static_cast<const BuiltUpAreaItem*>( item );
         ItemTypes::areaFeatureDrawDisplayClass_t displayClass =
            bua->getDisplayClass();
         if (displayClass == ItemTypes::buaOnIsland ) {
            return GfxFeature::BUA_ON_ISLAND;
         }
         if ( (filterSettings != NULL) && 
              (filterSettings->m_filterType == 
               FilterSettings::SYMBOL_FILTER)) {
            if (filterSettings->m_symbol == DrawSettings::SQUARE_3D_SYMBOL){
               return (GfxFeature::BUILTUP_AREA_SQUARE);
            } else {
               return (GfxFeature::BUILTUP_AREA_SMALL); 
            }
         } else {
            return (GfxFeature::BUILTUP_AREA);
         }
         } break;

      case ItemTypes::waterItem : {
         if ( item->getGfxData() != NULL ) {
            // Check if the water is closed or not.
            if ( item->getGfxData()->closed() ) {
               const WaterItem* water = static_cast<const WaterItem*>( item );
               ItemTypes::areaFeatureDrawDisplayClass_t displayClass =
                  water->getDisplayClass();
               if ( displayClass == ItemTypes::waterInCityPark ||
                    displayClass == ItemTypes::waterInCartographic ||
                    displayClass == ItemTypes::waterInBuilding ||
                    displayClass == ItemTypes::waterOnIsland ) {
                  return GfxFeature::WATER_IN_PARK;
               }
               else {
                  return GfxFeature::WATER;
               }
            } else {
               // Water line (open polygon, water represented as a line)
               return (GfxFeature::WATER_LINE);
            }
         }
         } break;

      case ItemTypes::parkItem : 
         switch ( static_cast<const ParkItem*>
                  (item)->getParkType()) {
            case 0 :
               return (GfxFeature::PARK);
            case 1 :
               return (GfxFeature::NATIONALPARK);
         } break;

      case ItemTypes::forestItem : 
         return (GfxFeature::FOREST);

      case ItemTypes::buildingItem : 
         return (GfxFeature::BUILDING);

      case ItemTypes::individualBuildingItem : 
         // Decide which draw type the ibi should have
         // Parking garage may be under ground so display as industrial area
         // not to mix up with individual buildings that are on the ground
         switch ( static_cast<const IndividualBuildingItem*>
                  (item)->getBuildingType()) {
            case ItemSubTypes::parkingGarage :
               return (GfxFeature::BUILDING);
            default:
               return (GfxFeature::INDIVIDUALBUILDING);
         } break;

      case ItemTypes::islandItem : {
         const IslandItem* island = static_cast<const IslandItem*>( item );
         ItemTypes::areaFeatureDrawDisplayClass_t displayClass =
            island->getDisplayClass();
         if ( displayClass == ItemTypes::islandInBua ) {
            return GfxFeature::ISLAND_IN_BUA;
         } else if ( displayClass == ItemTypes::IIWIPOutsideParkOutsideBua ) {
            return GfxFeature::ISLAND_IN_WATER_IN_PARK_ISLAND;
         } else if ( displayClass == ItemTypes::IIWIPOutsideParkInsideBua ) {
            return GfxFeature::ISLAND_IN_WATER_IN_PARK_BUA;
         } else if ( displayClass == ItemTypes::IIWIPInsidePark ) {
            return GfxFeature::ISLAND_IN_WATER_IN_PARK;
         } else {
            return GfxFeature::ISLAND;
         }
      } break;
      case ItemTypes::airportItem :
         return (GfxFeature::AIRPORTGROUND);
         
      case ItemTypes::aircraftRoadItem :
         return (GfxFeature::AIRCRAFTROAD);

      case ItemTypes::cartographicItem :
         // decide which draw type the carto item should have
         // items with certain carto sub types should not be included at all
         switch ( static_cast<const CartographicItem*>
                  (item)->getCartographicType()) {
            case ItemSubTypes::golfGround :
            case ItemSubTypes::recreationalAreaGround :
            case ItemSubTypes::restAreaGround :
            case ItemSubTypes::zooGround :
            case ItemSubTypes::cemetaryGround :
            case ItemSubTypes::amusementParkGround :
               return (GfxFeature::CARTOGRAPHIC_GREEN_AREA);
            case ItemSubTypes::militaryServiceBranch :
               return (GfxFeature::NBR_GFXFEATURES);
            default:
               return (GfxFeature::CARTOGRAPHIC_GROUND);
         } break;

      case ItemTypes::pedestrianAreaItem :
         return (GfxFeature::PEDESTRIANAREA);

      case ItemTypes::pointOfInterestItem :
         return (GfxFeature::POI);
      case ItemTypes::ferryItem :
         return (GfxFeature::FERRY);
      case ItemTypes::railwayItem :
         return (GfxFeature::RAILWAY);
      //case ItemTypes::streetItem : 
      case ItemTypes::categoryItem : 
      case ItemTypes::nullItem :
      case ItemTypes::zipCodeItem :
      case ItemTypes::cityPartItem:
      case ItemTypes::routeableItem :
      case ItemTypes::busRouteItem :
      case ItemTypes::militaryBaseItem :
      case ItemTypes::subwayLineItem :
      case ItemTypes::municipalItem : 
      case ItemTypes::numberOfItemTypes: 
      default:
         return (GfxFeature::NBR_GFXFEATURES);
   }
   return (GfxFeature::NBR_GFXFEATURES);
}


bool MapUtility::isLargeAreaType( GfxFeature::gfxFeatureType type ) {
   return 
      type == GfxFeature::INDIVIDUALBUILDING ||
      type == GfxFeature::AIRCRAFTROAD ||
      type == GfxFeature::CARTOGRAPHIC_GROUND ||
      type == GfxFeature::CARTOGRAPHIC_GREEN_AREA ||
      type == GfxFeature::PARK ||
      type == GfxFeature::NATIONALPARK ||
      type == GfxFeature::BUILDING ||
      type == GfxFeature::LAND || 
      type == GfxFeature::WATER ||
      type == GfxFeature::ISLAND ||
      type == GfxFeature::WATER_IN_PARK ||
      type == GfxFeature::ISLAND_IN_BUA;
}
