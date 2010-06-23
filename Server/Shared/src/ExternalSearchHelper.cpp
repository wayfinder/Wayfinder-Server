/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ExternalSearchHelper.h"
#include "ExternalSearchConsts.h"
#include "LangTypes.h"
#include "StringUtility.h"


namespace {

   class StringHelper {
   public:
      StringHelper( LangTypes::language_t lang ) : m_lang ( lang ) {}
      
      MC2String getCapStr( StringCode code ) const {
         return StringUtility::makeFirstCapital(StringTable::getString(
                                                   code, m_lang ) );
      }
      
      MC2String getStr( StringCode code ) const {
         return StringUtility::makeFirstCapital(StringTable::getString(
                                                   code, m_lang ) );
      }
      
      MC2String getCountryName( StringTable::countryCode cc ) const {
         return StringUtility::makeFirstCapital(
            StringTable::getString( StringTable::getCountryStringCode( cc ),
                                    m_lang ) );
      }
         
      
   private:
      /// The language to be used.
      LangTypes::language_t m_lang;
   };
}


MC2String 
ExternalSearchHelper::getFieldName( 
   ExternalSearchConsts::search_fields_t field,
   LangTypes::language_t lang ) 
{
   StringHelper st(lang);
   switch ( field ) {
      case ExternalSearchConsts::country:
         // FIXME:
         return st.getCapStr( StringTable::COUNTRY );
      case ExternalSearchConsts::name_or_phone:
         return st.getCapStr( StringTable::NAME ) + '/' +
            st.getStr( StringTable::PHONE_NBR );
      case ExternalSearchConsts::address_or_city:
         return st.getCapStr( StringTable::ADDRESS ) + '/' +
            st.getStr( StringTable::CITY );
      case ExternalSearchConsts::first_name:
         return st.getCapStr( StringTable::FIRST_NAME );
      case ExternalSearchConsts::last_name:
         return st.getCapStr( StringTable::LAST_NAME );
      case ExternalSearchConsts::phone_number:
         return st.getCapStr( StringTable::PHONE_NBR );
      case ExternalSearchConsts::name:
         return st.getCapStr( StringTable::NAME );
      case ExternalSearchConsts::zip_code:
         return st.getCapStr( StringTable::ZIP_CODE );
      case ExternalSearchConsts::city:
         return st.getCapStr( StringTable::CITY );
      case ExternalSearchConsts::address:
         return st.getCapStr( StringTable::ADDRESS );
         // Should really be postal area!
      case ExternalSearchConsts::postal_area:
         return st.getCapStr( StringTable::CITY );
      case ExternalSearchConsts::company:
         return st.getCapStr( StringTable::COMPANY );
      case ExternalSearchConsts::search_word:
         return st.getCapStr( StringTable::SEARCH_WORD );
      case ExternalSearchConsts::company_or_search_word:
         return st.getCapStr( StringTable::COMPANY ) + '/' +
            st.getStr( StringTable::SEARCH_WORD );
      case ExternalSearchConsts::city_or_area:
         return st.getCapStr( StringTable::CITY ) + '/' +
            st.getStr( StringTable::LAND_AREA );
      case ExternalSearchConsts::category:
         return st.getCapStr( StringTable::CATEGORY );
      case ExternalSearchConsts::country_name:
         return st.getCapStr( StringTable::COUNTRY );
      case ExternalSearchConsts::top_region_center:
         return st.getCapStr( StringTable::COUNTRY );
      case ExternalSearchConsts::top_region_span:
         return st.getCapStr( StringTable::COUNTRY );
      case ExternalSearchConsts::category_id:
         return st.getCapStr( StringTable::CATEGORY );   
   }
   MC2_ASSERT( false );
   return "";
} 

StringTable::countryCode 
ExternalSearchHelper::topRegionToCountryCode( uint32 topRegion, 
                                              uint32 serviceId ) 
{
   StringTable::countryCode countryCode = StringTable::NBR_COUNTRY_CODES;
   
   switch ( topRegion ) {
      case 0:
         countryCode = StringTable::ENGLAND_CC;
         break;
      case 1:
         countryCode = StringTable::SWEDEN_CC;
         break;
      case 2:
         countryCode = StringTable::GERMANY_CC;
         break;
      case 3:
         countryCode = StringTable::DENMARK_CC;
         break;
      case 4:
         countryCode = StringTable::FINLAND_CC;
         break;
      case 5:
         countryCode = StringTable::NORWAY_CC;
         break;
      case 6:
         countryCode = StringTable::BELGIUM_CC;
         break;
      case 7:
         countryCode = StringTable::NETHERLANDS_CC;
         break;
      case 61:
         countryCode = StringTable::IRELAND_CC;
         break;
      case 64:
         countryCode = StringTable::FRANCE_CC;
         break;
      case 65:
         countryCode = StringTable::SPAIN_CC;
         break;
      case 67:
         countryCode = StringTable::ITALY_CC;
         break;
      case 70:
         countryCode = StringTable::PORTUGAL_CC;
         break;
      case 75:
         countryCode = StringTable::CZECH_REPUBLIC_CC;
         break;
      case 84:
         countryCode = StringTable::SINGAPORE_CC;
         break;
      default:
         break;
   }
   
   // USA is special with many top regions
   if ( serviceId == ExternalSearchConsts::google_local_search ) { 
      countryCode = static_cast<StringTable::countryCode>( topRegion );
   } else if ( serviceId == ExternalSearchConsts::qype ) { 
      countryCode = static_cast<StringTable::countryCode>( topRegion );
   } else if ( countryCode == StringTable::NBR_COUNTRY_CODES ) {
      mc2log << warn << "[ExternalSearchHelper] No valid top region id: " 
             << topRegion << endl;
   }

   return countryCode;
}
