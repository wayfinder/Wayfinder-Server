/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileFeatureStrings.h"


// All tile feature primitives
#define TCS_PRIMS \
   TF2STR(circle); \
   TF2STR(bitmap); \
   TF2STR(polygon); \
   TF2STR(line);

// All tile feature types
#define TCS_FEATURETYPES \
      TF2STR(circle); \
      TF2STR(bitmap); \
      TF2STR(polygon); \
      TF2STR(line); \
      TF2STR(street_class_0); \
      TF2STR(street_class_1); \
      TF2STR(street_class_2); \
      TF2STR(street_class_3); \
      TF2STR(street_class_4); \
      TF2STR(street_class_0_level_0); \
      TF2STR(street_class_1_level_0); \
      TF2STR(street_class_2_level_0); \
      TF2STR(street_class_3_level_0); \
      TF2STR(street_class_4_level_0); \
      TF2STR(poi); \
      TF2STR(water); \
      TF2STR(park); \
      TF2STR(land); \
      TF2STR(building_area); \
      TF2STR(building_outline); \
      TF2STR(bua); \
      TF2STR(railway); \
      TF2STR(ocean); \
      TF2STR(route); \
      TF2STR(route_origin); \
      TF2STR(route_destination); \
      TF2STR(route_park_car); \
      TF2STR(city_centre_2); \
      TF2STR(city_centre_4); \
      TF2STR(city_centre_5); \
      TF2STR(city_centre_7); \
      TF2STR(city_centre_8); \
      TF2STR(city_centre_10); \
      TF2STR(city_centre_11); \
      TF2STR(city_centre_12); \
      TF2STR(atm); \
      TF2STR(golf_course); \
      TF2STR(ice_skating_rink); \
      TF2STR(marina); \
      TF2STR(vehicle_repair_facility); \
      TF2STR(bowling_centre); \
      TF2STR(bank); \
      TF2STR(casino); \
      TF2STR(city_hall); \
      TF2STR(commuter_railstation); \
      TF2STR(courthouse); \
      TF2STR(historical_monument); \
      TF2STR(museum); \
      TF2STR(nightlife); \
      TF2STR(post_office); \
      TF2STR(recreation_facility); \
      TF2STR(rent_a_car_facility); \
      TF2STR(rest_area); \
      TF2STR(ski_resort); \
      TF2STR(sports_activity); \
      TF2STR(theatre); \
      TF2STR(tourist_attraction); \
      TF2STR(church); \
      TF2STR(winery); \
      TF2STR(parking_garage); \
      TF2STR(park_and_ride); \
      TF2STR(open_parking_area); \
      TF2STR(amusement_park); \
      TF2STR(library); \
      TF2STR(school); \
      TF2STR(grocery_store); \
      TF2STR(petrol_station); \
      TF2STR(tram_station); \
      TF2STR(ferry_terminal); \
      TF2STR(cinema); \
      TF2STR(bus_station); \
      TF2STR(railway_station); \
      TF2STR(airport); \
      TF2STR(restaurant); \
      TF2STR(hotel); \
      TF2STR(tourist_office); \
      TF2STR(police_station); \
      TF2STR(hospital); \
      TF2STR(toll_road); \
      TF2STR(island); \
      TF2STR(university); \
      TF2STR(border); \
      TF2STR(wlan); \
      TF2STR(custom_poi_2); \
      TF2STR(custom_poi_4); \
      TF2STR(custom_poi_7); \
      TF2STR(custom_poi_10); \
      TF2STR(custom_poi_15); \
      TF2STR(custom_poi_20); \
      TF2STR(custom_poi_30); \
      TF2STR(petrol_station_with_price); \
      TF2STR(traffic_default); \
      TF2STR(roadwork); \
      TF2STR(speed_camera); \
      TF2STR(speed_trap); \
      TF2STR(airport_ground); \
      TF2STR(mosque); \
      TF2STR(synagogue); \
      TF2STR(turkish_hospital); \
      TF2STR(subway_station); \
      TF2STR(cafe); \
      TF2STR(hindu_temple); \
      TF2STR(buddhist_site); \
      TF2STR(user_defined_speed_camera); \
      TF2STR(street_class_0_ramp); \
      TF2STR(street_class_1_ramp); \
      TF2STR(street_class_2_ramp); \
      TF2STR(cartographic_green_area); \
      TF2STR(cartographic_ground); \
      TF2STR(forest); \
      TF2STR(aircraftroad); \
      TF2STR(special_custom_poi_2); \
      TF2STR(special_custom_poi_4); \
      TF2STR(special_custom_poi_7); \
      TF2STR(special_custom_poi_10); \
      TF2STR(special_custom_poi_15); \
      TF2STR(special_custom_poi_20); \
      TF2STR(special_custom_poi_30); \
      TF2STR(shop); \
      TF2STR(event); \
      TF2STR(walkway); \
      TF2STR(water_in_park); \
      TF2STR(island_in_bua); \
      TF2STR(island_in_water_in_park); \
      TF2STR(bua_on_island);

namespace TileFeatureStrings {

const char* feat2str( uint32 type ) {
#define TF2STR(x) case TileFeature::x: return #x
   switch ( type ) {
      TCS_FEATURETYPES;      
   }
#undef TF2STR

   return "unknown";
}


TilePrimitiveFeature::tileFeature_t str2feat( const char* str ) {
#define TF2STR(x) if ( strcasecmp( str, #x ) == 0 ) return TileFeature::x;
   TCS_FEATURETYPES;
#undef TF2STR

   return TilePrimitiveFeature::nbr_tile_features;
}

TilePrimitiveFeature::tileFeature_t str2prim( const char* str ) {
#define TF2STR(x) if ( strcasecmp( str, #x ) == 0 ) return TileFeature::x;
   TCS_PRIMS;
#undef TF2STR
   
   return TilePrimitiveFeature::nbr_tile_features;
}

const char* prim2str( int prim ) {
#define TF2STR(x) case TileFeature::x: return #x
   switch ( prim ) {
      TCS_PRIMS;
   default:
      break;
   }
#undef TF2STR

   return "";
}

} // TileFeatureStrings
