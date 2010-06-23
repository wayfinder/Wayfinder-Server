/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ITEM_DETAIL_ENUMS_H
#define ITEM_DETAIL_ENUMS_H

#include "config.h"

#include "ItemInfoEnums.h"
#include "MC2String.h"

/**
 * Class ItemDetailsEnums for handling the details for a POI.
 */
class ItemDetailEnums {

public:
   
   /// Enum for the type of POI details
   enum poi_detail_t {
      dont_show      = 0x0000, 
      text           = 0x0001, 
      street_address = 0x0002, 
      full_address   = 0x0003,
      phone_number   = 0x0004,
      url            = 0x0005,
      email          = 0x0006,
      poi_url        = 0x0007,
      poi_thumb      = 0x0008,
      average_rating = 0x0009,
      description    = 0x000a,
      open_hours     = 0x000b,
      providor_info  = 0x000c,
   };

   /// Enum for the content type of POI details.
   enum poi_detail_content_t {
      content_text          = 0x00,
      content_phone_number  = 0x01,
      content_url           = 0x02,
      content_email_address = 0x03,
      content_integer       = 0x04,
      content_float         = 0x05,
   };

   /**
    * Converts from ItemInfoEnums::InfoType to poi_detail_t
    *
    * @param type The InfoType to convert.
    * @return The poi_detail_content_t value
    */
   static poi_detail_t poiInfoToPoiDetail(ItemInfoEnums::InfoType type);

   /**
    * Return the content type for a ItemInfoEnums::InfoType
    *
    * @param type The InfoType to find content type for.
    * @return The poi_detail_t value
    */
   static poi_detail_content_t getContentTypeForPoiInfo( 
      ItemInfoEnums::InfoType type );

   /**
    * Translates a poi_detail_t to a string representation
    *
    * @param type The poi_detail_t to translate.
    * @return A string representation
    */
   static MC2String poiDetailAsString( ItemDetailEnums::poi_detail_t type );

   /**
    * Translates the poi_detail_content_t to a string representation
    *
    * @param content_type The poi_detail_content_t to translate.
    * @return A string representation
    */
   static MC2String poiContentTypeAsString( 
      ItemDetailEnums::poi_detail_content_t content_type );
};

#endif
