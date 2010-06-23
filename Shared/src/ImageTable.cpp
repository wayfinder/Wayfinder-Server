/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ImageTable.h"

namespace ImageTable {

const char* strings[NBR_IMAGES][NBR_IMAGE_SETS] = {
   { "", NULL },
   { "route_orig", NULL },
   { "route_dest", NULL },
   { "parkAndWalk", NULL },
   { "cityCentre_square", NULL },
   { "cityCentre", NULL },
   { "cityCentre_small_red", NULL },
   { "cityCentre_small_pink", NULL },
   { "cityCentre_point", NULL },
   { "cityCentre_point_small", NULL },
   { "tat_atm", NULL },
   { "tat_golfcourse", NULL },
   { "iceSkatingRink", NULL },
   { "marina", NULL },
   { "tat_carrepair", NULL },
   { "bowlingCentre", NULL },
   { "tat_bank", NULL },
   { "tat_casino", NULL },
   { "cityHall", NULL },
   { "commuter_rail_station", NULL },
   { "subway_station", NULL },
   { "courtHouse", NULL },
   { "tat_historicalmonument", NULL },
   { "museum", NULL },
   { "tat_nightlife", NULL },
   { "tat_churchchristian", NULL },
   { "tat_churchmoske", NULL },
   { "tat_churchsynagogue", NULL },
   { "hindu_temple", NULL },
   { "buddhist_temple", NULL },
   { "tat_postoffice", NULL },
   { "recreationFacility", NULL },
   { "rentAcarFacility", NULL },
   { "restArea", NULL },
   { "tat_skiresort", NULL },
   { "sportsActivity", NULL },
   { "tat_shop", NULL },
   { "tat_theatre", NULL },
   { "tat_touristattraction", NULL },
   { "university", NULL },
   { "winery", NULL },
   { "tat_parkinggarage", NULL },
   { "tat_parkandride", NULL },
   { "tat_openparkingarea", NULL },
   { "tat_amusementpark", NULL },
   { "library", NULL },
   { "school", NULL },
   { "tat_grocerystore", NULL },
   { "tat_petrolstation", NULL },
   { "tat_tramway", NULL },
   { "tat_ferryterminal", NULL },
   { "tat_cinema", NULL },
   { "tat_busstation", NULL },
   { "tat_railwaystation", NULL },
   { "tat_airport", NULL },
   { "tat_restaurant", NULL },
   { "cafe", NULL },
   { "tat_hotel", NULL },
   { "tat_touristoffice", NULL },
   { "policeStation", NULL },
   { "tat_hospital", NULL },
   { "tollRoad", NULL },
   { "tat_wifihotspot", NULL },
   { "tat_cheappetrolstation", NULL },
   { "tur_hospital", NULL },
   { "tat_trafficinformation", NULL },
   { "tat_roadwork", NULL },
   { "tat_speedcamera", NULL },
   { "user_defined_speedcamera", NULL },
   { "tat_speedtrap", NULL },
   { "tat_commutertrain", NULL },
   { "unknown", NULL },
   { "carRightU", NULL },
   { "carRight", NULL },
   { "carLeftU", NULL },
   { "carLeft", NULL },
   { "man", NULL },
   { "bikeRight", NULL },
   { "bikeLeft", NULL },
   { "mappin_centered", NULL },

   { "tat_airport", "cat_iphone_airport" },
   { "tat_atm", "cat_iphone_atm" },
   { "tat_bank", "cat_iphone_bank" },
   { "poi_cardealer", "cat_iphone_cardealer" },
   { "tat_cinema", "cat_iphone_cinema" },
   { "citycentre_dart", "cat_iphone_citycentre" },
   { "tat_hospital", "cat_iphone_hospital" },
   { "tat_ferryterminal",  "cat_iphone_ferryterminal" },
   { "tat_golfcourse", "cat_iphone_golfcourse" },
   { "tat_hotel", "cat_iphone_hotel" },
   { "commuter_rail_station", "cat_iphone_commuter_rail_station" },
   { "poi_historicalmonument_building", "cat_iphone_historicalmonument_building" },
   { "tat_historicalmonument", "cat_iphone_museum" },
   { "poi_musicstore", NULL },
   { "tat_nightlife", "cat_iphone_nightlife" },
   { "tat_openparkingarea", "cat_iphone_openparkingarea" },
   { "tat_parkinggarage", "cat_iphone_parkinggarage" },
   { "tat_petrolstation", "cat_iphone_petrolstation" },
   { "tat_pharmacy", "cat_iphone_pharmacy" },
   { "policeStation", "cat_iphone_policestation" },
   { "tat_postoffice", "cat_iphone_postoffice" },
   { "rentAcarFacility", "cat_iphone_rentacarfacility" },
   { "tat_restaurant", "cat_iphone_restaurant" },
   { "tat_grocerystore", "cat_iphone_grocerystore" },
   { "tat_skiresort", "cat_iphone_skiresort" },
   { "tat_theatre", "cat_iphone_theatre" },
   { "tat_touristoffice", "cat_iphone_touristoffice" },
   { "tat_railwaystation", "cat_iphone_railwaystation" },
   { "tat_carrepair", "cat_iphone_carrepair" },
   { "tat_nightlife", "cat_iphone_nightlife" },
   { "cafe", "cat_iphone_cafe" },
   { "tat_hospital", "cat_iphone_hospital" },
   { "tat_nightlife", "cat_iphone_nightlife" },
   { "tat_openparkingarea", "cat_iphone_openparkingarea" },
   { "subway_station", "cat_iphone_subway" },
   { "tat_grocerystore", "cat_iphone_shopping" },
   { "rentAcarFacility", "cat_iphone_rentacarfacility" },
   { "tat_wifihotspot", "cat_iphone_wifi" },
   { "tat_busstation", "cat_iphone_busstation" },
   { "tat_casino", "cat_iphone_casino" },
   { "tat_amusementpark", "cat_iphone_amusementpark" },
   { "tat_churchmoske", "cat_iphone_mosque" },
   { "tat_churchchristian", "cat_iphone_church" },
   { "tat_churchsynagogue", "cat_iphone_synagogue" },
   { "tat_touristattraction", "cat_iphone_touristattraction" },
   { "tat_tramway", "cat_iphone_tramstation" },
   { "tat_shop", "cat_iphone_generic" },
   { "tat_shop", "cat_iphone_generic" },

   { "search_heading_places", "search_heading_iphone_places" },
   { "search_heading_addresses", "search_heading_iphone_addresses" },
};

const char* getImage( ImageCode imageCode, 
                      ImageSet imageSet /*= DEFAULT */ ) {
   const char* result = strings[ imageCode ][ imageSet ];

   if ( result == NULL && imageSet != DEFAULT ) {
      result = strings[ imageCode ][ DEFAULT ];
   }

   if ( result == NULL ) {
      result = "";
   }

   return result;
}

ImageSet getImageSetFromString( const MC2String& imageSetStr ) {
   if ( imageSetStr == "iphone" ) {
      return IPHONE;
   } else if ( imageSetStr == "default" ||
               imageSetStr == "" ) {
      return DEFAULT;
   } else {
      mc2log << warn << "Invalid image set: " << imageSetStr << endl;
      return DEFAULT;
   }
}

MC2String imageSetToString( ImageSet imageSet ) {
   switch ( imageSet ) {
      case DEFAULT:
         return "";
      case IPHONE:
         return "iphone";
      case NBR_IMAGE_SETS:
         MC2_ASSERT( false );
   }

   return "";
}


} // namespace ImageTable
