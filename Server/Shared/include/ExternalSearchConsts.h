/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXTERNAL_SEARCH_CONSTS_H
#define EXTERNAL_SEARCH_CONSTS_H

#include "config.h"

/**
 * Constants common to Servers and ExtSearchModule
 */
class ExternalSearchConsts {   
public:
   /// Types of SearchFields.
     enum search_fields_t {
      /// A country drop down
      country         = 1,
      /// Name or phone number
      name_or_phone   = 2,
      /// Address or city
      address_or_city = 3,
      /// First name
      first_name      = 4,
      /// Last name
      last_name       = 5,
      /// Telephone number
      phone_number    = 6,
      /// Name
      name            = 7,
      /// Zip code
      zip_code        = 8,
      /// City
      city            = 9,
      /// Address
      address         = 10,
      /// Postal area
      postal_area     = 11,
      /// Company
      company         = 12,
      /// Search word
      search_word     = 13,
      /// Company or search word
      company_or_search_word = 14,
      /// City or area
      city_or_area    = 15,
      /// Category name
      category = 16,
      /// Name of the country
      country_name = 17,
      /// Top region center in wgs84 as "lat,lon" format.
      top_region_center = 18,
      /// Top region span in wgs84 as "lat,lon" format.
      top_region_span = 19,
      /// The category id
      category_id = 20
      };

   // Some shared ExtService::service_t from ExtServices.h.
   static const uint32 not_external = 0;
   static const uint32 google_local_search = 1;
   static const uint32 qype = 2;
   static const uint32 adserver = 3;
};

#endif
