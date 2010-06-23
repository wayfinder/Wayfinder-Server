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

#include "ItemInfoEnums.h"
#include "StringTable.h"

// is_sorted
#include "NonStdStl.h"

#include <algorithm>

#undef ST
#undef IIE
#define ST(x) StringTable::x
#define IIE(x) ItemInfoEnums::x

#undef BEGIN_ARRAY
#undef END_ARRAY
// Nice macros to have when sending arrays around.
#define BEGIN_ARRAY(x) (x)
#define END_ARRAY(x) ( (x) + (sizeof(x)/sizeof((x)[0]) ) )

// Secret namespace
namespace {

   struct table_entry_t {
      ItemInfoEnums::InfoType infoType;
      StringTable::stringCode stringCode;
   };

   static const struct table_entry_t table [] = {
         { IIE(dont_show)               , ST(NOTOK) },
         { IIE(text)                    , ST(NOTOK) },
         { IIE(url)                     , ST(URL  ) },
         { IIE(wap_url)                 , ST(URL  ) },
         { IIE(email)                   , ST(EMAIL) },
         { IIE(phone_number)            , ST(PHONE_NBR) }, 
         { IIE(mobile_phone)            , ST(MOBILE_PHONENUMBER) },
         { IIE(fax_number)              , ST(FAX) },
         { IIE(contact_info)            , ST(NOTOK) },
         { IIE(short_info)              , ST(SHORT_DESCRIPTION) },
         { IIE(vis_address)             , ST(ADDRESS) },
         { IIE(vis_house_nbr)           , ST(VIS_HOUSE_NBR) },
         { IIE(vis_zip_code)            , ST(ZIP_CODE) },
         { IIE(vis_complete_zip)        , ST(ZIP_CODE) },
         { IIE(Vis_zip_area)            , ST(CITY) },
         { IIE(vis_full_address)        , ST(ADDRESS) },
         { IIE(brandname)               , ST(BRANDNAME) },
         { IIE(short_description)       , ST(SHORT_DESCRIPTION) },
         { IIE(long_description)        , ST(LONG_DESCRIPTION) },
         { IIE(citypart)                , ST(CITYPARTITEM) },
         { IIE(state)                   , ST( STATE ) },
         { IIE(neighborhood)            , ST(NEIGHBORHOOD) },
         { IIE(open_hours)              , ST(OPEN_HOURS) },
         { IIE(nearest_train)           , ST(NEAREST_TRAIN) },
         { IIE(start_date)              , ST(START_DATE) },
         { IIE(end_date)                , ST(END_DATE) },
         { IIE(start_time)              , ST(START_TIME) },
         { IIE(end_time)                , ST(END_TIME) },
         { IIE(accommodation_type)      , ST(ACCOMMODATION_TYPE) },
         { IIE(check_in)                , ST(CHECK_IN) },
         { IIE(check_out)               , ST(CHECK_OUT) },
         { IIE(nbr_of_rooms)            , ST(NBR_OF_ROOMS) },
         { IIE(single_room_from)        , ST(SINGLE_ROOM_FROM) },
         { IIE(double_room_from)        , ST(DOUBLE_ROOM_FROM) },
         { IIE(triple_room_from)        , ST(TRIPLE_ROOM_FROM) },
         { IIE(suite_from)              , ST(SUITE_FROM) },
         { IIE(extra_bed_from)          , ST(EXTRA_BED_FROM) },
         { IIE(weekend_rate)            , ST(WEEKEND_RATE) },
         { IIE(nonhotel_cost)           , ST(NONHOTEL_COST) },
         { IIE(breakfast)               , ST(BREAKFAST) },
         { IIE(hotel_services)          , ST(HOTEL_SERVICES) },
         { IIE(credit_card)             , ST(CREDIT_CARD) },
         { IIE(special_feature)         , ST(SPECIAL_FEATURE) },
         { IIE(conferences)             , ST(CONFERENCES) },
         { IIE(average_cost)            , ST(AVERAGE_COST) },
         { IIE(booking_advisable)       , ST(BOOKING_ADVISABLE) },
         { IIE(admission_charge)        , ST(ADMISSION_CHARGE) },
         { IIE(home_delivery)           , ST(HOME_DELIVERY) },
         { IIE(disabled_access)         , ST(DISABLED_ACCESS) },
         { IIE(takeaway_available)      , ST(TAKEAWAY_AVAILABLE) },
         { IIE(allowed_to_bring_alcohol), ST(ALLOWED_TO_BRING_ALCOHOL) },
         { IIE(type_food)               , ST(TYPE_FOOD) },
         { IIE(decor)                   , ST(DECOR) },
         { IIE(image_url)               , ST(IMAGE) },
         { IIE(supplier)                , ST( SUPPLIER ) },
         { IIE(owner)                   , ST(OWNER) },
         { IIE(price_petrol_superplus)  , ST(PRICE_PETROL_SUPERPLUS) },
         { IIE(price_petrol_super)      , ST(PRICE_PETROL_SUPER) },
         { IIE(price_petrol_normal)     , ST(PRICE_PETROL_NORMAL) },
         { IIE(price_diesel)            , ST(PRICE_DIESEL) },
         { IIE(price_biodiesel)         , ST(PRICE_BIODIESEL) },
         { IIE(free_of_charge)          , ST(FREE_OF_CHARGE) },
         { IIE(tracking_data)           , ST(NOSTRING) },
         { IIE(post_address)            , ST(POST_ADDRESS) },
         { IIE(post_zip_area)           , ST(POST_BUILT_UP_AREA) },
         { IIE(post_zip_code)           , ST(POST_ZIPCODEITEM) },
         { IIE(open_for_season)         , ST(OPEN_FOR_SEASON) },
         { IIE(ski_mountain_min_max_height), ST(SKI_MOUNTAIN_MIN_MAX_HEIGHT) },
         { IIE(snow_depth_valley_mountain), ST(SNOW_DEPTH_MOUNTAIN_VALLEY) },
         { IIE(snow_quality)            , ST(SNOW_QUALITY) },
         { IIE(lifts_open_total)        , ST(LIFTS_OPEN_TOTAL) },
         { IIE(ski_slopes_open_total)   , ST(SKI_SLOPES_OPEN_TOTAL) },
         { IIE(cross_country_skiing_km) , ST(CROSS_COUNTRY_SKIING_KM) },
         { IIE(glacier_area)            , ST(GLACIER_AREA) },
         { IIE(last_snowfall)           , ST(LAST_SNOWFALL) },
         { IIE(special_flag)            , ST(NOSTRING) },
         { IIE(static_id)               , ST(NOTOK) },
         { IIE(has_service)             , ST(HAS_SERVICE) },
         { IIE(has_carwash)             , ST(HAS_CARWASH) },
         { IIE(has_24h_self_service_zone), ST(HAS_24H_SELF_SERVICE_ZONE) },
         { IIE(drive_in)                , ST(HAS_DRIVE_IN) },
         { IIE(mailbox_collection_time) , ST(MAILBOX_COLLECTION_TIMES) },
         { IIE(performer)               , ST(PERFORMER) },
         { IIE(booking_url)             , ST(BOOKING_URL) },
         { IIE(booking_phone_number)    , ST(BOOKING_PHONE_NUMBER) },
         { IIE(poi_url)                 , ST(MC2_view_on_xxx_txt) },
         { IIE(poi_thumb)               , ST(MC2_poi_thumbnail_txt) },
         { IIE(average_rating)          , ST(MC2_average_rating_txt) },
         { IIE(provider_info )          , ST(MC2_info_provided_by_txt) },
         { IIE(bool_type)               , ST(NOTOK) },
         { IIE(more)                    , ST(NOTOK) },
   };

   struct TableComparator {
      bool operator()(const table_entry_t& a,
                      const table_entry_t& b ) {
         return a.infoType < b.infoType;
      }
      bool operator()( const table_entry_t& a,
                       ItemInfoEnums::InfoType b ) {
         return a.infoType < b;
      }
         
   };
   
   static bool checkSorting() {
      MC2_ASSERT( is_sorted( BEGIN_ARRAY(table),
                             END_ARRAY( table ),
                             TableComparator() ) );

      // The two types bool_type and more are strange so we have to add 2
      CHECK_ARRAY_SIZE( table, ItemInfoEnums::last_before_bool + 2 );
      
      for ( int i = 0; i < ItemInfoEnums::last_before_bool; ++i ) {
         ItemInfoEnums::InfoType t = ItemInfoEnums::InfoType(i);
         const struct table_entry_t* found =
            std::lower_bound( BEGIN_ARRAY(table),
                              END_ARRAY( table ),
                              t,
                              TableComparator() );
         
         MC2_ASSERT( found != END_ARRAY( table ) &&
                     found->infoType == t );
      }

      return true;
   }
   
   bool initialized = checkSorting();
   
}


StringCode
ItemInfoEnums::infoTypeToStringCode( ItemInfoEnums::InfoType t )
{
   const struct table_entry_t* found =
      std::lower_bound( BEGIN_ARRAY(table),
                        END_ARRAY( table ),
                        t,
                        TableComparator() );

   if ( found != END_ARRAY( table ) &&
        found->infoType == t ) {
      return found->stringCode;
   }
   // Not found
   return StringTable::NOTOK;                          
}

const char*
ItemInfoEnums::infoTypeToString( LangTypes::language_t lang,
                                 ItemInfoEnums::InfoType t,
                                 const char* defVal,
                                 bool smsish )
{
   StringCode strCode( infoTypeToStringCode( t ) );
   if ( strCode == StringTable::NOTOK ) {
      return defVal;
   }
   // Ok - found   
   return StringTable::getString( strCode, lang );
}
