/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "InfoTypeConverter.h"

const struct InfoTypeConverter::infoTypeString_t 
InfoTypeConverter::m_infoTypes[] = {
   { ItemInfoEnums::dont_show, "dont_show", 0x00 },
   { ItemInfoEnums::text, "text", 0x01 },
   { ItemInfoEnums::url, "url", 0x02 },
   { ItemInfoEnums::wap_url, "wap_url", 0x03 },
   { ItemInfoEnums::email, "email", 0x04 },
   { ItemInfoEnums::phone_number, "phone_number", 0x05 },
   { ItemInfoEnums::mobile_phone, "mobile_phone", 0x06 },
   { ItemInfoEnums::fax_number, "fax_number", 0x07 },
   { ItemInfoEnums::contact_info, "contact_info", 0x08 },
   { ItemInfoEnums::short_info, "short_info", 0x09 },
   { ItemInfoEnums::vis_address, "vis_address", 0x0a },
   { ItemInfoEnums::vis_house_nbr, "vis_house_nbr", 0x0b },
   { ItemInfoEnums::vis_zip_code, "vis_zip_code", 0x0c },
   { ItemInfoEnums::vis_complete_zip, "vis_complete_zip", 0x0d },
   { ItemInfoEnums::Vis_zip_area, "vis_zip_area", 0x0e },
   { ItemInfoEnums::vis_full_address, "vis_full_address", 0x0f },
   { ItemInfoEnums::brandname, "brandname", 0x10 },
   { ItemInfoEnums::short_description, "short_description", 0x11 },
   { ItemInfoEnums::long_description, "long_description", 0x12 },
   { ItemInfoEnums::citypart, "citypart", 0x13 },
   { ItemInfoEnums::state, "state", 0x14 },
   { ItemInfoEnums::neighborhood, "neighborhood", 0x15 },
   { ItemInfoEnums::open_hours, "open_hours", 0x16 },
   { ItemInfoEnums::nearest_train, "nearest_train", 0x17 },
   { ItemInfoEnums::start_date, "start_date", 0x18 },
   { ItemInfoEnums::end_date, "end_date", 0x19 },
   { ItemInfoEnums::start_time, "start_time", 0x1a },
   { ItemInfoEnums::end_time, "end_time", 0x1b },
   { ItemInfoEnums::accommodation_type, "accommodation_type", 0x1c },
   { ItemInfoEnums::check_in, "check_in", 0x1d },
   { ItemInfoEnums::check_out, "check_out", 0x1e },
   { ItemInfoEnums::nbr_of_rooms, "nbr_of_rooms", 0x1f },
   { ItemInfoEnums::single_room_from, "single_room_from", 0x20 },
   { ItemInfoEnums::double_room_from, "double_room_from", 0x21 },
   { ItemInfoEnums::triple_room_from, "triple_room_from", 0x22 },
   { ItemInfoEnums::suite_from, "suite_from" , 0x23},
   { ItemInfoEnums::extra_bed_from, "extra_bed_from", 0x24 },
   { ItemInfoEnums::weekend_rate, "weekend_rate", 0x25 },
   { ItemInfoEnums::nonhotel_cost, "nonhotel_cost", 0x26 },
   { ItemInfoEnums::breakfast, "breakfast", 0x27 },
   { ItemInfoEnums::hotel_services, "hotel_services", 0x28 },
   { ItemInfoEnums::credit_card, "credit_card", 0x29 },
   { ItemInfoEnums::special_feature, "special_feature", 0x2a },
   { ItemInfoEnums::conferences, "conferences", 0x2b },
   { ItemInfoEnums::average_cost, "average_cost", 0x2c },
   { ItemInfoEnums::booking_advisable, "booking_advisable", 0x2d },
   { ItemInfoEnums::admission_charge, "admission_charge", 0x2e },
   { ItemInfoEnums::home_delivery, "home_delivery", 0x2f },
   { ItemInfoEnums::disabled_access, "disabled_access", 0x30 },
   { ItemInfoEnums::takeaway_available, "takeaway_available", 0x31 },
   { ItemInfoEnums::allowed_to_bring_alcohol, "allowed_to_bring_alcohol", 0x32 },
   { ItemInfoEnums::type_food, "type_food", 0x33 },
   { ItemInfoEnums::decor, "decor", 0x34 },
   { ItemInfoEnums::image_url, "image_url", 0x35 },
   { ItemInfoEnums::supplier, "supplier", 0x36 },
   { ItemInfoEnums::owner, "owner", 0x37 },
   { ItemInfoEnums::price_petrol_superplus, "price_petrol_superplus", 0x38 },
   { ItemInfoEnums::price_petrol_super, "price_petrol_super", 0x39 },
   { ItemInfoEnums::price_petrol_normal, "price_petrol_normal", 0x3a },
   { ItemInfoEnums::price_diesel, "price_diesel", 0x3b },
   { ItemInfoEnums::price_biodiesel, "price_biodiesel", 0x3c },
   { ItemInfoEnums::free_of_charge, "free_of_charge", 0x3d },
   { ItemInfoEnums::tracking_data, "tracking_data", 0x3e },
   { ItemInfoEnums::post_address, "post_address", 0x3f },
   { ItemInfoEnums::post_zip_area, "post_zip_area", 0x40 },
   { ItemInfoEnums::post_zip_code, "post_zip_code", 0x41 },
   { ItemInfoEnums::open_for_season, "open_for_season", 0x42 },
   { ItemInfoEnums::ski_mountain_min_max_height, "ski_mountain_min_max_height", 0x43 },
   { ItemInfoEnums::snow_depth_valley_mountain, "snow_depth_valley_mountain", 0x44 },
   { ItemInfoEnums::snow_quality, "snow_quality", 0x45 },
   { ItemInfoEnums::lifts_open_total, "lifts_open_total", 0x46 },
   { ItemInfoEnums::ski_slopes_open_total, "ski_slopes_open_total", 0x47 },
   { ItemInfoEnums::cross_country_skiing_km, "cross_country_skiing_km", 0x48 },
   { ItemInfoEnums::glacier_area, "glacier_area", 0x49 },
   { ItemInfoEnums::last_snowfall, "last_snowfall", 0x4a },
   { ItemInfoEnums::special_flag, "special_flag", 0x4b },
   { ItemInfoEnums::static_id, "dont_show", 0x00 }, // Don't show to user
   { ItemInfoEnums::has_service, "text", 0x01 }, // as text
   { ItemInfoEnums::has_carwash, "text", 0x01 }, // as text
   { ItemInfoEnums::has_24h_self_service_zone, "text", 0x01 }, // as text
   { ItemInfoEnums::drive_in, "text", 0x01 }, // as text
   { ItemInfoEnums::mailbox_collection_time, "text", 0x01 }, // as text
   { ItemInfoEnums::performer, "performer", ItemInfoEnums::performer },
   { ItemInfoEnums::booking_url, "booking_url", ItemInfoEnums::booking_url },
   { ItemInfoEnums::booking_phone_number, "booking_phone_number", ItemInfoEnums::booking_phone_number },
   { ItemInfoEnums::poi_url, "text", 0x01 },
   { ItemInfoEnums::poi_thumb, "text", 0x01 },
   { ItemInfoEnums::average_rating, "text", 0x01 },
   { ItemInfoEnums::provider_info, "text", 0x01 },
   { ItemInfoEnums::more, "text", 0xff },
// Don't forget do add to the documentation (externalapi.tex and
// navserver_prot_2_10.tex) too.
// And tell some client people about it.
};

InfoTypeConverter::InfoTypeConverter() {
   CHECK_ARRAY_SIZE( m_infoTypes, ItemInfoEnums::last_before_bool + 1 );
   for ( size_t i = 0 ; i < NBR_ITEMS( m_infoTypes ) ; ++i ) {
      m_infoTypeMap.insert( make_pair( m_infoTypes[ i ].type, i ) ); 
      m_infoStringMap.insert( make_pair( m_infoTypes[ i ].str, i ) ); 
      m_infoAddTypeMap.insert( make_pair( m_infoTypes[ i ].addType, i ) );
   }

}

const char*
InfoTypeConverter::infoTypeToStr( ItemInfoEnums::InfoType type ) const {
   infoTypeMap::const_iterator findIt = m_infoTypeMap.find( type );
   if ( findIt != m_infoTypeMap.end() ) {
      return m_infoTypes[ findIt->second ].str.c_str();
   } else {
      return "text";
   }
}


ItemInfoEnums::InfoType
InfoTypeConverter::strToInfoType( const char* str ) const {
   infoStringMap::const_iterator findIt = m_infoStringMap.find( str );
   if ( findIt != m_infoStringMap.end() ) {
      return m_infoTypes[ findIt->second ].type;
   } else {
      return ItemInfoEnums::more;
   }
}

uint16
InfoTypeConverter::infoTypeToAdditionalInfoType( ItemInfoEnums::InfoType type )
{
   infoTypeMap::const_iterator findIt = m_infoTypeMap.find( type );
   if ( findIt != m_infoTypeMap.end() ) {
      return m_infoTypes[ findIt->second ].addType;
   } else {
      return 0x01; // ItemInfoEnums::text;
   }
}

ItemInfoEnums::InfoType 
InfoTypeConverter::additionalInfoTypeToInfoType( uint16 type ) {
   infoAddTypeMap::const_iterator findIt = m_infoAddTypeMap.find( type );
   if ( findIt != m_infoAddTypeMap.end() ) {
      return m_infoTypes[ findIt->second ].type;
   } else {
      return ItemInfoEnums::text;
   }
}
