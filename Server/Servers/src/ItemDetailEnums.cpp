/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ItemDetailEnums.h"
#include "MC2String.h"

ItemDetailEnums::poi_detail_t 
ItemDetailEnums::poiInfoToPoiDetail(ItemInfoEnums::InfoType type) {
   switch ( type ) {
      case ItemInfoEnums::dont_show :
         return dont_show;
      case ItemInfoEnums::text :
         return text;
      case ItemInfoEnums::vis_address :
         return street_address;
      case ItemInfoEnums::vis_full_address :
         return full_address;
      case ItemInfoEnums::phone_number :
         return phone_number;
      case ItemInfoEnums::url :
         return url;
      case ItemInfoEnums::email :
         return email;
      case ItemInfoEnums::poi_url :
         return poi_url;
      case ItemInfoEnums::poi_thumb :
         return poi_thumb;
      case ItemInfoEnums::long_description :
         return description;
      case ItemInfoEnums::open_hours :
         return open_hours;
      case ItemInfoEnums::provider_info :
         return providor_info;
      case ItemInfoEnums::average_rating :
         return average_rating;
      default :
         return text;
         break;
   }
}


ItemDetailEnums::poi_detail_content_t 
ItemDetailEnums::getContentTypeForPoiInfo( ItemInfoEnums::InfoType type ) {
   switch ( type ) {
      case ItemInfoEnums::phone_number :
         return ItemDetailEnums::content_phone_number;
      case ItemInfoEnums::email :
         return ItemDetailEnums::content_email_address;
      case ItemInfoEnums::url :
      case ItemInfoEnums::poi_url :
      case ItemInfoEnums::poi_thumb :
         return ItemDetailEnums::content_url;
      case ItemInfoEnums::average_rating :
         return ItemDetailEnums::content_integer;
      default:
         return ItemDetailEnums::content_text;
   }
   return ItemDetailEnums::content_text;
}


MC2String 
ItemDetailEnums::poiDetailAsString( ItemDetailEnums::poi_detail_t type ) {
   switch ( type ) {
      case ItemDetailEnums::dont_show :
         return "dont_show";
      case ItemDetailEnums::text :
         return "text";
      case ItemDetailEnums::street_address :
         return "street_address";
      case ItemDetailEnums::full_address :
         return "full_address";
      case ItemDetailEnums::phone_number :
         return "phone_number";
      case ItemDetailEnums::url :
         return "url";
      case ItemDetailEnums::email :
         return "email";
      case ItemDetailEnums::poi_url :
         return "poi_url";
      case ItemDetailEnums::poi_thumb :
         return "poi_thumb";
      case ItemDetailEnums::average_rating :
         return "average_rating";
      case ItemDetailEnums::description :
         return "description";
      case ItemDetailEnums::open_hours :
         return "open_hours";
      case ItemDetailEnums::providor_info :
         return "providor_info";
      default:
         return  "unknown_type";
   }
   return "";
}


MC2String 
ItemDetailEnums::poiContentTypeAsString( 
   ItemDetailEnums::poi_detail_content_t content_type ) {
   switch ( content_type ) {
      case ItemDetailEnums::content_text :
         return "text";
      case ItemDetailEnums::content_phone_number :
         return "phone_number";
      case ItemDetailEnums::content_url :
         return "url";
      case ItemDetailEnums::content_email_address :
         return "email_address";
      case ItemDetailEnums::content_integer :
         return "integer";
      case ItemDetailEnums::content_float :
         return "float";
   }
   return "";
}
