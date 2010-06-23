/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ITEMINFOENUMS_H
#define ITEMINFOENUMS_H

#include "config.h"
#include "LangTypes.h"

class StringCode;

class ItemInfoEnums {
   public:
   /**
    * Enum for types of information.
    * All unrecognized types are considered text. 
    */
   enum InfoType {
      dont_show    = 0x00,
      text         = 0x01,
      url          = 0x02,
      wap_url      = 0x03,
      email        = 0x04,
      phone_number = 0x05,
      mobile_phone = 0x06,
      fax_number   = 0x07,
      contact_info = 0x08,
      short_info   = 0x09,
      
      vis_address  = 0x0a,
      vis_house_nbr= 0x0b,
      vis_zip_code = 0x0c,
      vis_complete_zip=0x0d,
      Vis_zip_area = 0x0e,
      vis_full_address=0x0f,
      brandname    = 0x10,
      short_description=0x11,
      long_description=0x12,
      citypart     = 0x13,
      state        = 0x14,
      neighborhood = 0x15,
      open_hours   = 0x16,
      nearest_train= 0x17,
      start_date   = 0x18,
      end_date     = 0x19,
      start_time   = 0x1a,
      end_time     = 0x1b,
      accommodation_type=0x1c,
      check_in     = 0x1d,
      check_out    = 0x1e,
      nbr_of_rooms = 0x1f,
      single_room_from=0x20,
      double_room_from=0x21,
      triple_room_from=0x22,
      suite_from   = 0x23,
      extra_bed_from=0x24,
      weekend_rate = 0x25,
      nonhotel_cost= 0x26,
      breakfast    = 0x27,
      hotel_services=0x28,
      credit_card  = 0x29,
      special_feature=0x2a,
      conferences  = 0x2b,
      average_cost = 0x2c,
      booking_advisable=0x2d,
      admission_charge=0x2e,
      home_delivery= 0x2f,
      disabled_access=0x30,
      takeaway_available=0x31,
      allowed_to_bring_alcohol=0x32,
      type_food    = 0x33,
      decor        = 0x34,
      image_url    = 0x35,
      supplier     = 0x36,
      owner        = 0x37,
      price_petrol_superplus = 0x38,
      price_petrol_super = 0x39,
      price_petrol_normal = 0x3a,
      price_diesel = 0x3b,
      price_biodiesel = 0x3c,
      free_of_charge = 0x3d,
      tracking_data = 0x3e,
      post_address = 0x3f,
      post_zip_area = 0x40,
      post_zip_code = 0x41,
      open_for_season = 0x42,
      ski_mountain_min_max_height = 0x43,
      snow_depth_valley_mountain = 0x44,
      snow_quality = 0x45,
      lifts_open_total = 0x46,
      ski_slopes_open_total = 0x47,
      cross_country_skiing_km = 0x48,
      glacier_area = 0x49,
      last_snowfall = 0x4a,
      special_flag = 0x4b,
      
      static_id     = 0x4c,
      
      has_service = 0x4d,
      has_carwash = 0x4e,
      has_24h_self_service_zone = 0x4f,
      drive_in = 0x50,
      mailbox_collection_time = 0x51,
      performer            = 0x52,
      booking_url          = 0x53,
      booking_phone_number = 0x54,
      
      poi_url              = 0x55,
      poi_thumb            = 0x56,
      average_rating       = 0x57,
      provider_info        = 0x58,

      last_before_bool,    // Should be last before bool
      bool_type    = 0xfe,
      more         = 0xff
   };
   
   /**
    * Enum for filtering the ItemInfo in the search match.
    */
   enum InfoTypeFilter {
      All,           ///< All item info. No filtering at all.
      OneSearch_All, ///< All item info:s for onesearch request.
      None           ///< No item info shall be returned.
   };
   
   /*
    *    Usage:
    *    * String codes != NOTOK will be used as key in the correct language
    *    * String code NOTOK means that the string in english
    *      in the database will be used no matter of the
    *      requested language.
    */
   static StringCode infoTypeToStringCode( ItemInfoEnums::InfoType t );
   
   /**
    *    Uses StringTable to get a string representing the type of
    *    key sent in. If the language is not found, another language
    *    (english) will be used. If no string is found, defVal will
    *    will be returned.
    *    @param lang   The requested language.
    *    @param t      The requested info type.
    *    @param defVal Default value to return if not found.
    */
   static const char* infoTypeToString ( LangTypes::language_t lang,
                                         ItemInfoEnums::InfoType t,
                                            const char* defVal = NULL,
                                         bool smsish = false );
   
private:
   /**
    * Private Constructor to avoid use.
    */
   ItemInfoEnums();   
};



#endif // ITEMINFOENUMS_H

