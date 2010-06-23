/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NationalProperties.h"


#include "stdlib.h"

#include "config.h"
#include "MapGenUtil.h"
#include "MapBits.h"


NationalProperties::zipCodeType_t 
NationalProperties::getZipCodeType(StringTable::countryCode countryCode)
{
   switch (countryCode){
      case StringTable::ENGLAND_CC : {
         return NationalProperties::ukZipCodeType;
      } break;
      case StringTable::IRELAND_CC : {
         return NationalProperties::numberNameZipCodeType;
      } break;
      default: {
         return NationalProperties::symmetricNumberZipCodeType;
      } break;
   } // switch

} // getZipCodeType


uint32
NationalProperties::getAdminAreasMergeDist( StringTable::countryCode       
                                            countryCode)
{
   switch (countryCode){
   default: {
      return 2000;
   } break;
   } // switch

} // getAdminAreasMergeDist


float64
NationalProperties::getStreetMergeSqDist( StringTable::countryCode    
                                          countryCode, uint32 roadClass) 
{
   if ( roadClass > 4 ){
      mc2log << error << "NationalProperties::getStreetMergeSqDist"
             << " Trying to get street merge dist for unsupported"
             << " road class, " << roadClass << endl;
      return MAX_FLOAT64;
   }

   // Default merge distances. One distance per road class 0-4.
   static const float64 defaultSqDists[] = 
                                        { 1500 * 1500,    // Road class 0
                                          1500 * 1500,    // Road class 1
                                          1000 * 1000,    // Road class 2
                                           500 *  500,    // Road class 3
                                           500 *  500  }; // Road class 4

   const float64* mergeSqDists = defaultSqDists;

   switch (countryCode){
//      // Exmaple of how to customize for other countries.
//   case StringTable::NORWAY_CC : {
//       static const float64      tmpMergeSqDists[] =
//                                 { 1500 * 1500,    // Road class 0
//                                   1500 * 1500,    // Road class 1
//                                   1500 * 1500,    // Road class 2
//                                   1000 * 1000,    // Road class 3
//                                    500 *  500  }; // Road class 4
//       mergeSqDists = tmpMergeSqDists;
//   } break;

   default: {
      // Do nothing, default distances are returned.
   } break;
   } // switch

   return mergeSqDists[roadClass];

} // getStreetMergeSqDist



bool
NationalProperties::extendAbbrevStreetNames( StringTable::countryCode
                                             countryCode,
                                             ItemTypes::itemType
                                             itemType )
{
   bool result = false;

   // Extend abbreviations of street segments and streets in 
   // countries where almost all street names have very abbreviated 
   // names. 
   // Otherwise it is impossible to find e.g. "Main Blvd" with the search 
   // string "Main Boulevard"
   // Possible fixme: also add mapOrigin here to be able to distinguish
   // properties for different map suppliers and releases.

   switch (countryCode){
//      // Exmaple of how to customize for some countries.
//      case StringTable::USA_CC:
//      case StringTable::CANADA_CC:
//      case StringTable::NEW_ZEALAND_CC:
//      case StringTable::AUSTRALIA_CC: {
//         if ( ( itemType == ItemTypes::streetSegmentItem ) || 
//              ( itemType == ItemTypes::streetItem) ) 
//         {
//            result = true;
//         }
//      } break;
      default: {
         // Do nothing.
      } break;
   } // switch

   return result;
} // extendAbbrevStreetNames


uint16 NationalProperties::countrySpeedMatrix[StringTable::NBR_COUNTRY_CODES+1][4]={
      // Please find correct default speed limits and replace the ??? below
      {112,  90,  70, 50}, // England
      {110,  90,  70, 50}, // Sweden
      {130,  90,  70, 50}, // Germany
      {110,  90,  70, 50}, // Denmark some have 130 in reality.
      {120,  90,  70, 50}, // Finland
      { 90,  90,  70, 50}, // Norway
      {120,  90,  70, 50}, // Belgium
      {120,  90,  70, 50}, // Netherlands
      {130,  90,  70, 50}, // Luxembourg
      {112,  90,  70, 50}, // USA            ??? 
      {120,  90,  70, 50}, // SWITZERLAND
      {130,  90,  70, 50}, // AUSTRIA
      {130,  90,  70, 50}, // FRANCE
      {120,  90,  70, 50}, // SPAIN
      {110,  90,  70, 50}, // ANDORRA  ??? 
      {110,  90,  70, 50}, // LIECHTENSTEIN ??? 
      {130,  90,  70, 50}, // ITALY
      {130,  90,  70, 50}, // MONACO   ??? 
      {112,  90,  70, 50}, // IRELAND
      {120,  90,  70, 50}, // PORTUGAL
      {112,  90,  70, 50}, // CANADA ??? 
      {110,  90,  70, 50}, // HUNGARY ???
      {110,  90,  70, 50}, // CZECH REPUBLIC ???
      {110,  90,  70, 50}, // POLAND ???
      {110,  90,  70, 50}, // GREECE ???
      {110,  90,  70, 50}, // ISRAEL ???
      {110,  90,  70, 50}, // BRAZIL ???
      {110,  90,  70, 50}, // SLOVAKIA ???
      {110,  90,  70, 50}, // RUSSIA ???
      {110,  90,  70, 50}, // TURKEY ???
      {110,  90,  70, 50}, // SLOVENIA ???
      {110,  90,  70, 50}, // BULGARIA ???
      {110,  90,  70, 50}, // ROMANIA ???
      {110,  90,  70, 50}, // UKRAINE ???
      {110,  90,  70, 50}, // SERBIA & MONTENEGRO ???
      {110,  90,  70, 50}, // CROATIA ???
      {110,  90,  70, 50}, // BOSNIA & HERZEGOVINA ???
      {110,  90,  70, 50}, // MOLDOVA ???
      {110,  90,  70, 50}, // FYR MACEDONIA ???
      {110,  90,  70, 50}, // ESTONIA ???
      {110,  90,  70, 50}, // LATVIA ???
      {110,  90,  70, 50}, // LITHUANIA ???
      {110,  90,  70, 50}, // BELARUS ???
      {110,  90,  70, 50}, // MALTA ???
      {110,  90,  70, 50}, // CYPRUS ???
      {110,  90,  70, 50}, // ICELAND ???
      {110,  90,  70, 50}, // HONG KONG ???
      {110,  90,  70, 50}, // SINGAPORE ???
      {110,  90,  70, 50}, // AUSTRALIA ???
      {110,  90,  70, 50}, // UAE ???
      {110,  90,  70, 50}, // BAHRAIN ???
      {110,  90,  70, 50}, // AFGHANISTAN ???
      {110,  90,  70, 50}, // ALBANIA ???
      {110,  90,  70, 50}, // ALGERIA ???
      {110,  90,  70, 50}, // AMERICAN_SAMOA ???
      {110,  90,  70, 50}, // ANGOLA ???
      {110,  90,  70, 50}, // ANGUILLA ???
      {110,  90,  70, 50}, // ANTARCTICA ???
      {110,  90,  70, 50}, // ANTIGUA_AND_BARBUDA ???
      {110,  90,  70, 50}, // ARGENTINA ???
      {110,  90,  70, 50}, // ARMENIA ???
      {110,  90,  70, 50}, // ARUBA ???
      {110,  90,  70, 50}, // AZERBAIJAN ???
      {110,  90,  70, 50}, // BAHAMAS ???
      {110,  90,  70, 50}, // BANGLADESH ???
      {110,  90,  70, 50}, // BARBADOS ???
      {110,  90,  70, 50}, // BELIZE ???
      {110,  90,  70, 50}, // BENIN ???
      {110,  90,  70, 50}, // BERMUDA ???
      {110,  90,  70, 50}, // BHUTAN ???
      {110,  90,  70, 50}, // BOLIVIA ???
      {110,  90,  70, 50}, // BOTSWANA ???
      {110,  90,  70, 50}, // BRITISH_VIRGIN_ISLANDS ???
      {110,  90,  70, 50}, // BRUNEI_DARUSSALAM ???
      {110,  90,  70, 50}, // BURKINA_FASO ???
      {110,  90,  70, 50}, // BURUNDI ???
      {110,  90,  70, 50}, // CAMBODIA ???
      {110,  90,  70, 50}, // CAMEROON ???
      {110,  90,  70, 50}, // CAPE_VERDE ???
      {110,  90,  70, 50}, // CAYMAN_ISLANDS ???
      {110,  90,  70, 50}, // CENTRAL_AFRICAN_REPUBLIC ???
      {110,  90,  70, 50}, // CHAD ???
      {110,  90,  70, 50}, // CHILE ???
      {110,  90,  70, 50}, // CHINA ???
      {110,  90,  70, 50}, // COLOMBIA ???
      {110,  90,  70, 50}, // COMOROS ???
      {110,  90,  70, 50}, // CONGO ???
      {110,  90,  70, 50}, // COOK_ISLANDS ???
      {110,  90,  70, 50}, // COSTA_RICA ???
      {110,  90,  70, 50}, // CUBA ???
      {110,  90,  70, 50}, // DJIBOUTI ???
      {110,  90,  70, 50}, // DOMINICA ???
      {110,  90,  70, 50}, // DOMINICAN_REPUBLIC ???
      {110,  90,  70, 50}, // DR_CONGO ???
      {110,  90,  70, 50}, // ECUADOR ???
      {110,  90,  70, 50}, // EGYPT ???
      {110,  90,  70, 50}, // EL_SALVADOR ???
      {110,  90,  70, 50}, // EQUATORIAL_GUINEA ???
      {110,  90,  70, 50}, // ERITREA ???
      {110,  90,  70, 50}, // ETHIOPIA ???
      {110,  90,  70, 50}, // FAEROE_ISLANDS ???
      {110,  90,  70, 50}, // FALKLAND_ISLANDS ???
      {110,  90,  70, 50}, // FIJI ???
      {110,  90,  70, 50}, // FRENCH_GUIANA ???
      {110,  90,  70, 50}, // FRENCH_POLYNESIA ???
      {110,  90,  70, 50}, // GABON ???
      {110,  90,  70, 50}, // GAMBIA ???
      {110,  90,  70, 50}, // GEORGIA_COUNTRY ???
      {110,  90,  70, 50}, // GHANA ???
      {110,  90,  70, 50}, // GREENLAND ???
      {110,  90,  70, 50}, // GRENADA ???
      {110,  90,  70, 50}, // GUADELOUPE ???
      {110,  90,  70, 50}, // GUAM ???
      {110,  90,  70, 50}, // GUATEMALA ???
      {110,  90,  70, 50}, // GUINEA ???
      {110,  90,  70, 50}, // GUINEA_BISSAU ???
      {110,  90,  70, 50}, // GUYANA ???
      {110,  90,  70, 50}, // HAITI ???
      {110,  90,  70, 50}, // HONDURAS ???
      {110,  90,  70, 50}, // INDIA ???
      {110,  90,  70, 50}, // INDONESIA ???
      {110,  90,  70, 50}, // IRAN ???
      {110,  90,  70, 50}, // IRAQ ???
      {110,  90,  70, 50}, // IVORY_COAST ???
      {110,  90,  70, 50}, // JAMAICA ???
      {110,  90,  70, 50}, // JAPAN ???
      {110,  90,  70, 50}, // JORDAN ???
      {110,  90,  70, 50}, // KAZAKHSTAN ???
      {110,  90,  70, 50}, // KENYA ???
      {110,  90,  70, 50}, // KIRIBATI ???
      {110,  90,  70, 50}, // KUWAIT ???
      {110,  90,  70, 50}, // KYRGYZSTAN ???
      {110,  90,  70, 50}, // LAOS ???
      {110,  90,  70, 50}, // LEBANON ???
      {110,  90,  70, 50}, // LESOTHO ???
      {110,  90,  70, 50}, // LIBERIA ???
      {110,  90,  70, 50}, // LIBYA ???
      {110,  90,  70, 50}, // MACAO ???
      {110,  90,  70, 50}, // MADAGASCAR ???
      {110,  90,  70, 50}, // MALAWI ???
      {110,  90,  70, 50}, // MALAYSIA ???
      {110,  90,  70, 50}, // MALDIVES ???
      {110,  90,  70, 50}, // MALI ???
      {110,  90,  70, 50}, // MARSHALL_ISLANDS ???
      {110,  90,  70, 50}, // MARTINIQUE ???
      {110,  90,  70, 50}, // MAURITANIA ???
      {110,  90,  70, 50}, // MAURITIUS ???
      {110,  90,  70, 50}, // MAYOTTE ???
      {110,  90,  70, 50}, // MEXICO ???
      {110,  90,  70, 50}, // MICRONESIA ???
      {110,  90,  70, 50}, // MONGOLIA ???
      {110,  90,  70, 50}, // MONTSERRAT ???
      {110,  90,  70, 50}, // MOROCCO ???
      {110,  90,  70, 50}, // MOZAMBIQUE ???
      {110,  90,  70, 50}, // MYANMAR ???
      {110,  90,  70, 50}, // NAMIBIA ???
      {110,  90,  70, 50}, // NAURU ???
      {110,  90,  70, 50}, // NEPAL ???
      {110,  90,  70, 50}, // NETHERLANDS_ANTILLES ???
      {110,  90,  70, 50}, // NEW_CALEDONIA ???
      {110,  90,  70, 50}, // NEW_ZEALAND ???
      {110,  90,  70, 50}, // NICARAGUA ???
      {110,  90,  70, 50}, // NIGER ???
      {110,  90,  70, 50}, // NIGERIA ???
      {110,  90,  70, 50}, // NIUE ???
      {110,  90,  70, 50}, // NORTHERN_MARIANA_ISLANDS ???
      {110,  90,  70, 50}, // NORTH_KOREA ???
      {110,  90,  70, 50}, // OCCUPIED_PALESTINIAN_TERRITORY ???
      {110,  90,  70, 50}, // OMAN ???
      {110,  90,  70, 50}, // PAKISTAN ???
      {110,  90,  70, 50}, // PALAU ???
      {110,  90,  70, 50}, // PANAMA ???
      {110,  90,  70, 50}, // PAPUA_NEW_GUINEA ???
      {110,  90,  70, 50}, // PARAGUAY ???
      {110,  90,  70, 50}, // PERU ???
      {110,  90,  70, 50}, // PHILIPPINES ???
      {110,  90,  70, 50}, // PITCAIRN ???
      {110,  90,  70, 50}, // QATAR ???
      {110,  90,  70, 50}, // REUNION ???
      {110,  90,  70, 50}, // RWANDA ???
      {110,  90,  70, 50}, // SAINT_HELENA ???
      {110,  90,  70, 50}, // SAINT_KITTS_AND_NEVIS ???
      {110,  90,  70, 50}, // SAINT_LUCIA ???
      {110,  90,  70, 50}, // SAINT_PIERRE_AND_MIQUELON ???
      {110,  90,  70, 50}, // SAINT_VINCENT_AND_THE_GRENADINES ???
      {110,  90,  70, 50}, // SAMOA ???
      {110,  90,  70, 50}, // SAO_TOME_AND_PRINCIPE ???
      {110,  90,  70, 50}, // SAUDI_ARABIA ???
      {110,  90,  70, 50}, // SENEGAL ???
      {110,  90,  70, 50}, // SEYCHELLES ???
      {110,  90,  70, 50}, // SIERRA_LEONE ???
      {110,  90,  70, 50}, // SOLOMON_ISLANDS ???
      {110,  90,  70, 50}, // SOMALIA ???
      {110,  90,  70, 50}, // SOUTH_AFRICA ???
      {110,  90,  70, 50}, // SOUTH_KOREA ???
      {110,  90,  70, 50}, // SRI_LANKA ???
      {110,  90,  70, 50}, // SUDAN ???
      {110,  90,  70, 50}, // SURINAME ???
      {110,  90,  70, 50}, // SVALBARD_AND_JAN_MAYEN ???
      {110,  90,  70, 50}, // SWAZILAND ???
      {110,  90,  70, 50}, // SYRIA ???
      {110,  90,  70, 50}, // TAIWAN ???
      {110,  90,  70, 50}, // TAJIKISTAN ???
      {110,  90,  70, 50}, // TANZANIA ???
      {110,  90,  70, 50}, // THAILAND ???
      {110,  90,  70, 50}, // TIMOR_LESTE ???
      {110,  90,  70, 50}, // TOGO ???
      {110,  90,  70, 50}, // TOKELAU ???
      {110,  90,  70, 50}, // TONGA ???
      {110,  90,  70, 50}, // TRINIDAD_AND_TOBAGO ???
      {110,  90,  70, 50}, // TUNISIA ???
      {110,  90,  70, 50}, // TURKMENISTAN ???
      {110,  90,  70, 50}, // TURKS_AND_CAICOS_ISLANDS ???
      {110,  90,  70, 50}, // TUVALU ???
      {110,  90,  70, 50}, // UGANDA ???
      {110,  90,  70, 50}, // UNITED_STATES_MINOR_OUTLYING_ISLANDS ???
      {110,  90,  70, 50}, // UNITED_STATES_VIRGIN_ISLANDS ???
      {110,  90,  70, 50}, // URUGUAY ???
      {110,  90,  70, 50}, // UZBEKISTAN ???
      {110,  90,  70, 50}, // VANUATU ???
      {110,  90,  70, 50}, // VENEZUELA ???
      {110,  90,  70, 50}, // VIETNAM ???
      {110,  90,  70, 50}, // WALLIS_AND_FUTUNA_ISLANDS ???
      {110,  90,  70, 50}, // WESTERN_SAHARA ???
      {110,  90,  70, 50}, // YEMEN ???
      {110,  90,  70, 50}, // ZAMBIA ???
      {110,  90,  70, 50}, // ZIMBABWE ???

      // Add default speed limits for new countries here !!!
      // HOWTO do this in an easy way?
      
      {110,  90,  70, 50}, // Default ??? 
   };



uint16
NationalProperties:: getSpeedApproximation(StringTable::countryCode 
                                           countryCode,
                                           int32 level){

   uint32 matrixSize =sizeof( NationalProperties::countrySpeedMatrix ) / 
      sizeof( *NationalProperties::countrySpeedMatrix );
   
   uint32 countrynbr = static_cast<uint32>(countryCode);
   if( (countrynbr < 0) || 
       (countrynbr >= StringTable::NBR_COUNTRY_CODES) ||
       (countrynbr >= matrixSize) )
   {
      countrynbr = matrixSize-1;
      mc2log << warn << "NationalProperties::getCountrySpeedLevel. "
             << "Using default country dependent speeds for country " 
             << "code = " << countrynbr << endl;
   }
   if((level < 0) || (level >= 4)){
      mc2log << error << "NationalProperties::getCountrySpeedLevel. "
             << "Wrong level = " << level << ". Exits!" << endl;
      exit(1);
   }
   
   return NationalProperties::countrySpeedMatrix[countrynbr][level];

} // getSpeedApproximation





CharEncoding*
NationalProperties::getMapToMC2ChEnc( StringTable::countryCode
                                      countryCode,
                                      const char* mapOrigin )
{
   CharEncoding* chEnc = NULL;

   CharEncodingType::charEncodingType toType = 
      CharEncodingType::invalidEncodingType;
   CharEncodingType::charEncodingType fromType = 
      CharEncodingType::invalidEncodingType;

   
   MapGenUtil::mapSupAndVerPair supAndVer =
      MapGenUtil::getMapSupAndVer( mapOrigin );
   MapGenEnums::mapSupplier mapSupplier = supAndVer.first;
   //MapGenEnums::mapVersion mapVersion = supAndVer.second;


   // All maps from OpenStreetMap have char enc UTF-8
   if (mapSupplier == MapGenEnums::OpenStreetMap ) {
      fromType = CharEncodingType::UTF8;
   }
   if (mapSupplier == MapGenEnums::Carmenta ) {
      fromType = CharEncodingType::iso8859_1;
   }

   // Special for certain countries/map suppliers/map versions
   switch (countryCode){

//      // Example ...
//      case StringTable::CROATIA_CC : {
//         if (mapSupplier == MapGenEnums::TeleAtlas ) {
//            fromType = CharEncodingType::iso8859_2;
//         }
//         else if ( mapSupplier == MapGenEnums::Carmenta ) {
//            fromType = CharEncodingType::iso8859_1;
//         }
//         else {
//            mc2log << error << here 
//                   << "NationalProperties::getMapToMC2ChEnc: "
//                   << "Don't know what encoding to use for lang code "
//                   << countryCode << " mapSupplier: " << mapSupplier 
//                   << " mapOrigin: " << mapOrigin << endl;
//            MC2_ADDERT(false);
//         }
//      } break;
      
      
      default:
         break;
   } // switch



   if ( fromType == CharEncodingType::invalidEncodingType ) {
      mc2log << error << "No char encoding fromType defined for "
             << StringTable::getString(
                  StringTable::getCountryStringCode(countryCode),
                  StringTable::ENGLISH)
             << " mapSupplier: " << mapSupplier << " mapOrigin: " 
             << mapOrigin << endl;
      MC2_ASSERT(false);
   }

      // Handle toType
#ifdef MC2_UTF8
   // If the server is compiled to use UTF-8, all encodings should convert
   // to UTF-8.
   if ( fromType == CharEncodingType::invalidEncodingType ){
      // When no special from type have been set it must be latin-1.
      fromType = CharEncodingType::iso8859_1;
   }
   toType = CharEncodingType::UTF8;
#else 
   if ( fromType != CharEncodingType::invalidEncodingType ){
      // We have set a non iso8859-1 encoding to convert from.
      toType = CharEncodingType::iso8859_1;
   }
#endif


   // Only create a char encoding object if needed.
   if ( toType != fromType ){
      bool dieOnError = true;
      chEnc = new CharEncoding( fromType, toType, dieOnError );
      if ( ! chEnc->initedOK() ){
         mc2log << error << "NationalProperties::getMapToMC2ChEnc: "
                << "Could not create char encoding objecto of toType:"
                << toType << " , fromType:" << fromType << ". Exits" 
                << endl;
         MC2_ASSERT(false);
      }
   }
   

   return chEnc;

} // getMapToMC2ChEnc


int
NationalProperties::getMaxRoadClassDiffForBifurc(
                     StringTable::countryCode countryCode )
{
   switch (countryCode){

//      // Example ...
//      case ( StringTable::BRAZIL_CC ) :
//         return 1;
//         break;
      default:
         return 0;
        break;
   } // switch

   return 0;
}


uint32
NationalProperties::getZipMinLength(StringTable::countryCode 
                                    countryCode)
{
   uint32 result = MAX_UINT32;

   switch (countryCode){
   case ( StringTable::SWEDEN_CC ) :
      result =  5;
      break;
   default:
      break;
   } // switch

   return result;

} // getZipMinLength

bool 
NationalProperties::rmNonNumZipNames(StringTable::countryCode countryCode)
{
   uint32 result = false;

   switch (countryCode){
//      // Example ...
//      case ( StringTable::USA_CC ) :
//         result =  true;
//         break;
      default:
         break;
   } // switch

   return result;


} // rmNonNumZipNames


bool 
NationalProperties::mergeSameNameCloseItems(
      StringTable::countryCode countryCode, const char* mapOrigin,
      ItemTypes::itemType itemType,
      uint32 mapID)
{
   bool result = false;

   // return false if it is a item type we don't want to merge
   switch ( itemType ) {
   case ( ItemTypes::parkItem ) :
   case ( ItemTypes::buildingItem ) :
   case ( ItemTypes::waterItem ) :
   case ( ItemTypes::airportItem ) :
   case ( ItemTypes::aircraftRoadItem ) : {
      // ok
   } break;
   default: {
      return false;
   } break;
   }

   // else check combination of country and supplier
   MapGenUtil::mapSupAndVerPair supAndVer =
      MapGenUtil::getMapSupAndVer( mapOrigin );
   MapGenEnums::mapSupplier mapSupplier = supAndVer.first;
   
   // -----------------------------------------------
   // country overview maps
   if ( MapBits::isCountryMap(mapID) ) {
      if ( mapSupplier == MapGenEnums::TeleAtlas ) {
         return true;
      }
   }

   // ---------------------------------------------------
   // underview maps

   // Always merge these item types, regardless of country and supplier
   switch ( itemType ) {
   case ( ItemTypes::airportItem ) : 
   case ( ItemTypes::aircraftRoadItem ) : {
      return true;
   } break;
   default: {
      // nothing, continue below
   } break;
   }
   
   // Then:
   // Merge if special combination of supplier, country, item type

   return result;

} // mergeSameNameCloseItems



bool
NationalProperties::useIndexAreas(StringTable::countryCode countryCode,
                                 const char* mapOrigin)
{
   MapGenUtil::mapSupAndVerPair supAndVer =
      MapGenUtil::getMapSupAndVer( mapOrigin );
   //MapGenEnums::mapSupplier mapSupplier = supAndVer.first;   
   
   switch (countryCode){

//   // Example
//   case ( StringTable::ENGLAND_CC ) : {
//      if ( mapSupplier == MapGenEnums::TeleAtlas ){
//         return true;
//      }
//   } break;
   
   // When adding new countries here, don't forget to update 
   // indexAreaToUse

   default: break; // To make it compile.
   } // switch

   return false;
}


bool 
NationalProperties::indexAreaToUse(StringTable::countryCode countryCode,
                                   const char* mapOrigin,
                                   uint32 indexAreaOrder){
   MapGenUtil::mapSupAndVerPair supAndVer =
      MapGenUtil::getMapSupAndVer( mapOrigin );
   //MapGenEnums::mapSupplier mapSupplier = supAndVer.first;   

   // indexAreaOrder MAX_UINT32 is for items that are not an index area
   if ( indexAreaOrder == MAX_UINT32 ){
      mc2log << error << "indexAreaOrder " << indexAreaOrder 
             << " indicates not index area item" << endl;
      MC2_ASSERT(false);
   }
   
   switch (countryCode){

//   // Example
//   case ( StringTable::ENGLAND_CC ) : {
//      if ( mapSupplier == MapGenEnums::TeleAtlas ){
//         // To keep sync with useIndexAreas:
//         MC2_ASSERT(useIndexAreas(countryCode, mapOrigin)); 
//         if ( indexAreaOrder == 10 ||
//              indexAreaOrder == 9 ||
//              indexAreaOrder == 8 ||
//              indexAreaOrder == 7 ){
//            return true;
//         }
//         else if ( indexAreaOrder == 1 ){
//            return false;
//         }
//         else {
//            mc2log << error << "Found unhandled index area order:"
//                   << indexAreaOrder
//                   << " for country: "
//                   << countryCode << " " << mapOrigin << endl;
//            MC2_ASSERT(false);
//         }
//      }
//   } break;
   
   default: {
      // To keep sync with useIndexAreas:
      MC2_ASSERT(!useIndexAreas(countryCode, mapOrigin)); 
   } break; 
   } // switch

   return false;
}

bool
NationalProperties::useCountryNameForNoNameBuas(
      StringTable::countryCode countryCode, const char* mapOrigin )
{
   bool result = false;
   
   MapGenUtil::mapSupAndVerPair supAndVer =
      MapGenUtil::getMapSupAndVer( mapOrigin );
   MapGenEnums::mapSupplier mapSupplier = supAndVer.first;   
   MapGenEnums::mapVersion mapVersion = supAndVer.second;


   if ( (mapSupplier == MapGenEnums::Carmenta) &&
         mapVersion == MapGenEnums::Carmenta_200809) {
      result = true;
   }
   else {
      mc2log << error << here 
             << "NationalProperties::useCountryNameForNoNameBuas "
             << "Don't know what to do for county code "
             << countryCode << " mapOrigin: " << mapOrigin << endl;
      exit(1);
   }

   // Handle the country dependent exceptions from the generic rules above
   // here
   switch (countryCode){
   default: {
      // Do nothing, no country dependent exceptions right now.
   } break;
   } // switch

   return result;
}

bool
NationalProperties::removeNoNameBuasInNoNameMunicipals(
      StringTable::countryCode countryCode, const char* mapOrigin )
{
   bool result = false;
   
   MapGenUtil::mapSupAndVerPair supAndVer =
      MapGenUtil::getMapSupAndVer( mapOrigin );
   //MapGenEnums::mapSupplier mapSupplier = supAndVer.first;   
   //MapGenEnums::mapVersion mapVersion = supAndVer.second;

   switch (countryCode){

//   // Example
//   case ( StringTable::UGANDA_CC ) :
//   case ( StringTable::ZAMBIA_CC ) :
//   case ( StringTable::ZIMBABWE_CC ) : {
//      if (mapSupplier == MapGenEnums::TeleAtlas) {
//         return true;
//      }
//   } break;

   default: {
      // Do nothing
   } break;
   } // switch

   return result;
}

bool 
NationalProperties::eliminateHolesAndSelftouch(
      StringTable::countryCode countryCode, const char* mapOrigin,
      ItemTypes::itemType itemType)
{
   bool result = false;

   // return false if it is a item type we don't want to merge
   switch ( itemType ) {
   case ( ItemTypes::individualBuildingItem ) :
   case ( ItemTypes::aircraftRoadItem ) : {
      // ok
   } break;
   default: {
      return false;
   } break;
   }


   // Always process these item types, regardless of country and supplier
   switch ( itemType ) {
   case ( ItemTypes::individualBuildingItem ) : 
   case ( ItemTypes::aircraftRoadItem ) : {
      return true;
   } break;
   default: {
      // nothing, continue below
   } break;
   }
   
   // else check combination of country and supplier
//   MapGenUtil::mapSupAndVerPair supAndVer =
//      MapGenUtil::getMapSupAndVer( mapOrigin );
//   MapGenEnums::mapSupplier mapSupplier = supAndVer.first;
   

   return result;

} // eliminateHolesAndSelftouch

uint32
NationalProperties::AFDDCInteriorItemMaxLength(
      StringTable::countryCode countryCode,
      ItemTypes::areaFeatureDrawDisplayClass_t afddc )
{
   uint32 maxLength = MAX_UINT32;

   // maxlength history for waterInCityPark
   // prior Oct 1 2009: 4000
   // after Oct 1 2009: 15000
   // after Nov 23 2009: MAX_UINT32 for all countries

   // Set a shorter distance if the calculation of area feature draw display
   // class takes too long time. The downside is that not all display classes
   // will be set, so you will loose some features in map display (they will
   // not be drawn because of the problem with flooded holes)

   switch (countryCode){

//   // Example
//   case ( StringTable::AUSTRALIA_CC ) : {
//      if( afddc == ItemTypes::waterInCityPark ) {
//         maxLength = 1000;         
//      }
//   } break;
   
   default: {
      maxLength = MAX_UINT32;
   } break; 
   } // switch
   
   return maxLength;
} // maxWaterSizeForAFDDC

uint32
NationalProperties::AFDDCExteriorItemMinLength(
      StringTable::countryCode countryCode,
      ItemTypes::areaFeatureDrawDisplayClass_t afddc )
{
   uint32 minLength = 0;

   // Set a larger distance if the calculation of area feature draw display
   // class takes too long time. The downside is that not all display classes
   // will be set, so you will loose some features in map display (they will
   // not be drawn because of the problem with flooded holes)
   
   switch (countryCode){
   
//   // Example
//   case ( StringTable::AUSTRALIA_CC ) : {
//      if( afddc == ItemTypes::waterInCityPark ) {
//         minLength = 1000;         
//      }
//   } break;
   
   default: {
      minLength = 0;
   } break; 
   } // switch
   
   return minLength;
} // maxWaterSizeForAFDDC


int32
NationalProperties::AFDDCPriority( 
   ItemTypes::areaFeatureDrawDisplayClass_t type ) {

   // When adding a new AFDDC for an item type, two things have to be 
   // done. 1) A drawing order has to be decided for the AFDDC 2) a 
   // priority has to be determined based on the drawing order. 
   // As an example, assume that a new waterInXXX is defined, which is
   // drawn after waterOnIsland, but before waterInCartographic. The
   // priority should then be set to a priority higher than the priority 
   // of waterOnIsland, and to a priority lower than the priority
   // of waterInCartographic. 

   // NOTE - Highest number = highest priority
   int32 priority = -MAX_INT32;

   // Water
   if( type == ItemTypes::waterOnIsland ) {
      priority = 0;
   } else if ( type == ItemTypes::waterInCartographic ) {
      priority = 1;
   } else if( type == ItemTypes::waterInBuilding ) {
      priority = 2;
   } else if( type == ItemTypes::waterInCityPark ) {
      priority = 3;

   // BUAS 
   } else if( type == ItemTypes::buaOnIsland ) {
      priority = 1;

   // Islands
   } else if( type == ItemTypes::islandInBua ) {
      priority = 1;
   } else if( type == ItemTypes::IIWIPOutsideParkOutsideBua) {
      priority = 2;
   } else if( type == ItemTypes::IIWIPOutsideParkInsideBua ) {
      priority = 3;
   } else if( type == ItemTypes::IIWIPInsidePark ) {
      priority = 4;
   
   // Cartographic
   } else if( type == ItemTypes::cartographicInCityPark ) {
      priority = 1;
   } else if( type == ItemTypes::cartographicInForest ) {
      priority = 1;
   } else {
      mc2log << error 
             << "NationalProperties::AFDDCPriority "
             << "unknown areaFeatureDrawDisplayClass_t type" 
             << endl;
      MC2_ASSERT(false);
   }

   return priority;
}



