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

#include "ItemTypes.h"
#include "StringUtility.h"
#include "StringTable.h"
#include "LangTypes.h"

//Zoomlevel for CityPartItems.
const uint32 ItemTypes::zoomConstCPI = 5;

const uint32 ItemTypes::poiiZoomLevel = 14;


StringCode
ItemTypes::getItemTypeSC(itemType t) 
{
   switch (t) {
      case streetSegmentItem : 
         return (StringTable::STREETSEGMENTITEM);
         break;
      case municipalItem : 
         return (StringTable::MUNICIPALITEM);
         break;
      case waterItem : 
         return (StringTable::WATERITEM);
         break;
      case parkItem : 
         return (StringTable::PARKITEM);
         break;
      case forestItem : 
         return (StringTable::FORESTITEM);
         break;
      case buildingItem : 
         return (StringTable::BUILDINGITEM);
         break;
      case railwayItem : 
         return (StringTable::RAILWAYITEM);
         break;
      case islandItem : 
         return (StringTable::ISLANDITEM);
         break;
      case streetItem : 
         return (StringTable::STREETITEM);
         break;
      case pointOfInterestItem : 
         return (StringTable::COMPANYITEM);
         break;
      case categoryItem : 
         return (StringTable::CATEGORYITEM);
         break;

      case nullItem :
         return (StringTable::NULLITEM);
         break;
      case zipCodeItem :
         return (StringTable::ZIPCODEITEM);
         break;
      case zipAreaItem :
         return (StringTable::ZIPAREAITEM);
         break;
      case builtUpAreaItem :
         return (StringTable::BUILTUPAREAITEM);
         break;
      case cityPartItem:
         return (StringTable::CITYPARTITEM);
         break;
      case routeableItem :
         return (StringTable::ROUTEABLEITEM);
         break;
      case busRouteItem :
         return (StringTable::BUSROUTEITEM);
         break;
      case ferryItem :
         return (StringTable::FERRYITEM);
         break;
      case airportItem :
         return (StringTable::AIRPORTITEM);
         break;
      case aircraftRoadItem :
         return (StringTable::AIRCRAFTROADITEM);
         break;
      case pedestrianAreaItem :
         return (StringTable::PEDESTRIANAREAITEM);
         break;
      case militaryBaseItem :
         return (StringTable::MILITARYBASEITEM);
         break;
      case individualBuildingItem :
         return(StringTable::INDIVIDUALBUILDINGITEM);
         break;
      case subwayLineItem :
         return(StringTable::SUBWAYLINEITEM);
         break;
      case notUsedItemType : 
         return (StringTable::NOTSUPPORTED);
         break;
      case borderItem : 
         return (StringTable::BORDERITEM);
         break;
      case cartographicItem : 
         return (StringTable::CARTOGRAPHICITEM);
         break;
         
      default :
         mc2log << error << here << " ItemType " << uint32(t) 
                << " not supported" << endl;
         return (StringTable::NOTSUPPORTED);
  }
}

const char*
ItemTypes::getItemTypeAsString(ItemTypes::itemType type)
{
   return StringTable::getString(getItemTypeSC(type),
                                 StringTable::ENGLISH);
}

StringCode 
ItemTypes::getIncItemTypeSC(int &pos) 
{
   if ( ( (pos >= 0) && (pos < 8) ) || (pos == 14) ) {
      pos++;
      return (getItemTypeSC( itemType(pos-1) ));
   } else if (pos == 8) {
      pos = 14;
      return (getItemTypeSC(streetItem));
   } else if (pos == 15) {
      pos = -1;
      return (getItemTypeSC(categoryItem));
   } else {
      pos = -1;
      return (StringTable::NOSTRING);
   }
}

ItemTypes::itemType
ItemTypes::getItemTypeFromString(const char* typeStr)
{
   //To ignore case copy the typeStr upper.
   MC2String upperTypeStr = StringUtility::copyUpper(MC2String(typeStr));
   ItemTypes::itemType type = ItemTypes::numberOfItemTypes;

   if (strstr(upperTypeStr.c_str(), "STREETSEGMENTITEM") != NULL)
      type = ItemTypes::streetSegmentItem;
   else if (strstr(upperTypeStr.c_str(), "MUNICIPALITEM") != NULL)
      type = ItemTypes::municipalItem;
   else if (strstr(upperTypeStr.c_str(), "WATERITEM") != NULL)
      type = ItemTypes::waterItem;
   else if (strstr(upperTypeStr.c_str(), "PARKITEM") != NULL)
      type = ItemTypes::parkItem;
   else if (strstr(upperTypeStr.c_str(), "FORESTITEM") != NULL)
      type = ItemTypes::forestItem;
   else if (strstr(upperTypeStr.c_str(), "INDIVIDUALBUILDINGITEM") != NULL)
      type = ItemTypes::individualBuildingItem;
   else if (strstr(upperTypeStr.c_str(), "BUILDINGITEM") != NULL)
      type = ItemTypes::buildingItem;
   else if(strstr(upperTypeStr.c_str(), "RAILWAYITEM") != NULL)
      type = ItemTypes::railwayItem;
   else if(strstr(upperTypeStr.c_str(), "ISLANDITEM") != NULL)
      type = ItemTypes::islandItem;
   else if(strstr(upperTypeStr.c_str(), "STREETITEM") != NULL)
      type = ItemTypes::streetItem;
   else if(strstr(upperTypeStr.c_str(), "ZIPCODEITEM") != NULL)
      type = ItemTypes::zipCodeItem;
   else if(strstr(upperTypeStr.c_str(), "ZIPAREAITEM") != NULL)
      type = ItemTypes::zipAreaItem;
   else if(strstr(upperTypeStr.c_str(), "BUILTUPAREAITEM") != NULL)
      type = ItemTypes::builtUpAreaItem;
   else if(strstr(upperTypeStr.c_str(), "CITYPARTITEM") != NULL)
      type = ItemTypes::cityPartItem;
   else if(strstr(upperTypeStr.c_str(), "POINTOFINTERESTITEM") != NULL)
      type = ItemTypes::pointOfInterestItem;
   else if(strstr(upperTypeStr.c_str(), "CATEGORYITEM") != NULL)
      type = ItemTypes::categoryItem;
   else if(strstr(upperTypeStr.c_str(), "BUSROUTEITEM") != NULL)
      type = ItemTypes::busRouteItem;
   else if(strstr(upperTypeStr.c_str(), "FERRYITEM") != NULL)
      type = ItemTypes::ferryItem;
   else if (strstr(upperTypeStr.c_str(), "AIRPORTITEM") != NULL)
      type = ItemTypes::airportItem;
   else if (strstr(upperTypeStr.c_str(), "AIRCRAFTROADITEM") != NULL)
      type = ItemTypes::aircraftRoadItem;
   else if (strstr(upperTypeStr.c_str(), "PEDESTRIANAREAITEM") != NULL)
      type = ItemTypes::pedestrianAreaItem;
   else if (strstr(upperTypeStr.c_str(), "MILITARYBASEITEM") != NULL)
      type = ItemTypes::militaryBaseItem;
   else if(strstr(upperTypeStr.c_str(), "SUBWAYLINEITEM") != NULL)
      type = ItemTypes::subwayLineItem;
   else if (strstr(upperTypeStr.c_str(), "BORDERITEM") != NULL)
      type = ItemTypes::borderItem;
   else if (strstr(upperTypeStr.c_str(), "CARTOGRAPHICITEM") != NULL)
      type = ItemTypes::cartographicItem;
   else {
      mc2log << error << "ItemTypes::getItemTypeFromString "
                  << "no type matched" << endl;
   }
   
   return type;
}

const char*
ItemTypes::getNameTypeAsString(ItemTypes::name_t nameType, bool shortName) 
{
   switch (nameType)  {
      case ItemTypes::officialName :
         if (shortName)
            return "on";
         else 
            return "officialName";
      case ItemTypes::alternativeName :
         if (shortName)
            return "an";
         else 
            return "alternativeName";
      case ItemTypes::roadNumber :
         if (shortName)
            return "nbr";
         else 
            return "roadNumber";   
      case ItemTypes::abbreviationName :
         if (shortName)
            return "ab";
         else 
            return "abbreviationName";
      case ItemTypes::uniqueName :
         if (shortName)
            return "un";
         else 
            return "uniqueName";
      case ItemTypes::exitNumber :
         if (shortName)
            return "exit";
         else 
            return "exitNumber";
      case ItemTypes::synonymName :
         if (shortName)
            return "syn";
         else 
            return "synonymName";
      default :
         if (shortName)
            return "inval";
         else 
            return "invalidName";
   }
}

ItemTypes::name_t
ItemTypes::getStringAsNameType(const char* nameType)
{
   if (StringUtility::strcasecmp(nameType, "officialName") == 0)
      return ItemTypes::officialName;
   else if (StringUtility::strcasecmp(nameType, "alternativeName") == 0)
      return ItemTypes::alternativeName;
   else if (StringUtility::strcasecmp(nameType, "roadNumber") == 0)
      return ItemTypes::roadNumber;
   else if (StringUtility::strcasecmp(nameType, "abbreviationName") == 0)
      return ItemTypes::abbreviationName;
   else if (StringUtility::strcasecmp(nameType, "uniqueName") == 0)
      return ItemTypes::uniqueName;
   else if (StringUtility::strcasecmp(nameType, "exitNumber") == 0)
      return ItemTypes::exitNumber;
   else if (StringUtility::strcasecmp(nameType, "synonymName") == 0)
      return ItemTypes::synonymName;
   else
      return ItemTypes::invalidName;
}



LangType 
ItemTypes::getLanguageCodeAsLanguageType( const LanguageCode& language )
{
   switch ( language ) {
      case StringTable::ENGLISH :
         return LangTypes::english;
      case StringTable::SWEDISH :
         return LangTypes::swedish;
      case StringTable::GERMAN :
         return LangTypes::german;
      case StringTable::DANISH :
         return LangTypes::danish;
      case StringTable::FINNISH :
         return LangTypes::finnish;
      case StringTable::NORWEGIAN :
         return LangTypes::norwegian;
      case StringTable::FRENCH :
         return LangTypes::french;
      case StringTable::SPANISH :
         return LangTypes::spanish;
      case StringTable::ITALIAN :
         return LangTypes::italian;
      case StringTable::DUTCH :
         return LangTypes::dutch;
      case StringTable::PORTUGUESE :
         return LangTypes::portuguese;
      case StringTable::AMERICAN_ENGLISH :
         return LangTypes::american;
      case StringTable::HUNGARIAN :
         return LangTypes::hungarian;         
      case StringTable::CZECH :
         return LangTypes::czech;
      case StringTable::GREEK :
         return LangTypes::greek;
      case StringTable::POLISH :
         return LangTypes::polish;
      case StringTable::SLOVAK :
         return LangTypes::slovak;
      case StringTable::RUSSIAN :
         return LangTypes::russian;         
      case StringTable::SLOVENIAN :
         return LangTypes::slovenian;         
      case StringTable::TURKISH :
         return LangTypes::turkish;         
      case StringTable::CHINESE :
         return LangTypes::chinese;
      case StringTable::CHINESE_TRADITIONAL :
         return LangTypes::chineseTraditional;
      case StringTable::SMSISH_ENG :
         return LangTypes::english;
      case StringTable::SMSISH_SWE :
         return LangTypes::swedish;
      case StringTable::SMSISH_GER :
         return LangTypes::german;
      case StringTable::SMSISH_DAN :
         return LangTypes::danish;
      case StringTable::SMSISH_FIN :
         return LangTypes::finnish;
      case StringTable::SMSISH_NOR :
         return LangTypes::norwegian;
      case StringTable::SMSISH_FRA :
         return LangTypes::french;
      case StringTable::SMSISH_SPA :
         return LangTypes::spanish;
      case StringTable::SMSISH_ITA :
         return LangTypes::italian;
      case StringTable::SMSISH_DUT :
         return LangTypes::dutch;
      case StringTable::SMSISH_POR :
         return LangTypes::portuguese;
      case StringTable::SMSISH_AME :
         return LangTypes::american;
      case StringTable::SMSISH_HUN :
         return LangTypes::hungarian;
      case StringTable::SMSISH_CZE :
         return LangTypes::czech;
      case StringTable::SMSISH_GRE :
         return LangTypes::greek;
      case StringTable::SMSISH_POL :
         return LangTypes::polish;
      case StringTable::SMSISH_SVK :
         return LangTypes::slovak;
      case StringTable::SMSISH_RUS :
         return LangTypes::russian;
      case StringTable::SMSISH_SLV :
         return LangTypes::slovenian;
      case StringTable::SMSISH_TUR :
         return LangTypes::turkish;
      case StringTable::SMSISH_CHI :
         return LangTypes::chinese;
      case StringTable::SMSISH_ZHT :
         return LangTypes::chineseTraditional;
   }

   // Unreachable code
   return LangTypes::english;
}


LanguageCode 
ItemTypes::getLanguageTypeAsLanguageCode( const LangType& lang ) {
   switch( lang ) {
      case LangTypes::english:
         return StringTable::ENGLISH;
      case LangTypes::swedish:
         return StringTable::SWEDISH;
      case LangTypes::german:
         return StringTable::GERMAN;
      case LangTypes::danish:
         return StringTable::DANISH;
      case LangTypes::finnish:
         return StringTable::FINNISH;
      case LangTypes::norwegian:
         return StringTable::NORWEGIAN;
      case LangTypes::french:
         return StringTable::FRENCH;
      case LangTypes::spanish:
         return StringTable::SPANISH;
      case LangTypes::italian:
         return StringTable::ITALIAN;
      case LangTypes::dutch:
         return StringTable::DUTCH;
      case LangTypes::portuguese:
         return StringTable::PORTUGUESE;
      case LangTypes::american:
         return StringTable::AMERICAN_ENGLISH;
      case LangTypes::hungarian :
         return StringTable::HUNGARIAN;
      case LangTypes::czech:
         return StringTable::CZECH;
      case LangTypes::greek :
         return StringTable::GREEK;         
      case LangTypes::polish :
         return StringTable::POLISH;       
      case LangTypes::slovak :
         return StringTable::SLOVAK;     
      case LangTypes::russian :
         return StringTable::RUSSIAN;    
      case LangTypes::slovenian :
         return StringTable::SLOVENIAN;    
      case LangTypes::turkish :
         return StringTable::TURKISH;    
      case LangTypes::chinese :
         return StringTable::CHINESE;    
     case LangTypes::chineseTraditional :
     case LangTypes::chineseTradHongKong : // Change to own StringTable lang when available
         return StringTable::CHINESE_TRADITIONAL;

      case LangTypes::welch:
      case LangTypes::albanian :
      case LangTypes::basque :
      case LangTypes::catalan :
      case LangTypes::frisian :
      case LangTypes::irish :
      case LangTypes::galician :
      case LangTypes::letzeburgesch :
      case LangTypes::raetoRomance :
      case LangTypes::serboCroatian :
      case LangTypes::valencian :
      case LangTypes::greekLatinStx :
      case LangTypes::russianLatinStx :
      case LangTypes::arabic :

      case LangTypes::chineseLatinStx :
      case LangTypes::estonian :
      case LangTypes::latvian :
      case LangTypes::lithuanian :
      case LangTypes::thai :
      case LangTypes::bulgarian :
      case LangTypes::cyrillicTranscript :
      case LangTypes::indonesian:
      case LangTypes::malay:

      case LangTypes::icelandic :
      case LangTypes::japanese :
      case LangTypes::amharic :
      case LangTypes::armenian :
      case LangTypes::tagalog :
      case LangTypes::belarusian :
      case LangTypes::bengali :
      case LangTypes::burmese :
      case LangTypes::croatian :
      case LangTypes::farsi :
      case LangTypes::gaelic :
      case LangTypes::georgian :
      case LangTypes::gujarati :
      case LangTypes::hebrew :
      case LangTypes::hindi :
      case LangTypes::kannada :
      case LangTypes::kazakh :
      case LangTypes::khmer :
      case LangTypes::korean :
      case LangTypes::lao :
      case LangTypes::macedonian :
      case LangTypes::malayalam :
      case LangTypes::marathi :
      case LangTypes::moldavian :
      case LangTypes::mongolian :
      case LangTypes::punjabi :
      case LangTypes::romanian :
      case LangTypes::serbian :
      case LangTypes::sinhalese :
      case LangTypes::somali :
      case LangTypes::swahili :
      case LangTypes::tamil :
      case LangTypes::telugu :
      case LangTypes::tibetan :
      case LangTypes::tigrinya :
      case LangTypes::turkmen :
      case LangTypes::ukrainian :
      case LangTypes::urdu :
      case LangTypes::vietnamese :
      case LangTypes::zulu :
      case LangTypes::sesotho :
      case LangTypes::bulgarianLatinStx :
      case LangTypes::bosnian :
      case LangTypes::slavic :
      case LangTypes::belarusianLatinStx :
      case LangTypes::macedonianLatinStx :
      case LangTypes::serbianLatinStx :
      case LangTypes::ukrainianLatinStx :
      case LangTypes::maltese :
      case LangTypes::thaiLatinStx :

      case LangTypes::invalidLanguage:
      case LangTypes::nbrLanguages:
         return StringTable::SMSISH_ENG;
   }

   // Should never get here
   MC2_ASSERT( lang != LangTypes::invalidLanguage );
   return StringTable::SMSISH_ENG;
}


StringCode
ItemTypes::getStreetNumberTypeSC(streetNumberType t) 
{
   switch (t) {
      case (noStreetNumbers) : 
         return (StringTable::NOSTREETNUMBERS);
         break;
      case (mixedStreetNumbers) : 
         return (StringTable::MIXEDSTREETNUMBERS);
         break;
      case (leftEvenStreetNumbers) : 
         return (StringTable::LEFTEVENSTREETNUMBERS);
         break;
      case (leftOddStreetNumbers) : 
         return (StringTable::RIGHTEVENSTREETNUMBERS);
         break;
      case (irregularStreetNumbers) : 
         return (StringTable::IRREGULARSTREETNUMBERS);
         break;
      default :
         return (StringTable::NOTSUPPORTED);
   }
}

StringCode 
ItemTypes::getIncStreetNumberTypeSC(int &pos) 
{
   if ( (pos >= 0) && (pos < 4) ) {
      pos++;
      return ( getStreetNumberTypeSC(streetNumberType(pos-1)) );
   } else {
      pos = -1;
      return (StringTable::NOSTRING);
   }
}


ItemTypes::streetNumberType
ItemTypes::getStreetNumberTypeFromHouseNumbering( uint16 leftSideStart,
      uint16 leftSideEnd, uint16 rightSideStart, uint16 rightSideEnd)
{
   //Deals with the case "no house numbers".
   if ( (leftSideStart == 0)  &&
        (leftSideEnd == 0 )   &&
        (rightSideStart == 0) &&
        (rightSideEnd == 0) ) {
      return ItemTypes::noStreetNumbers;

   // Deals with the cases:
   // "left side has even house numbers, and right side has odd house numbers"
   // "left side has even house numbers, and right side has no house numbers"
   // "left side has no house numbers, and right side has odd house numbers"
   } else if ( ((leftSideStart % 2) == 0) &&
               ((leftSideEnd % 2) == 0) &&
               ( ( ((rightSideStart % 2) != 0) &&
                   ((rightSideEnd % 2) != 0 ) ) ||
                 ( (rightSideStart == 0) &&
                   (rightSideEnd == 0)) ) ) {
      return ItemTypes::leftEvenStreetNumbers;

   // Deals with the cases:
   // "left side has odd house numbers, and right side has even house numbers"
   // "left side has odd house numbers, and right side has no house numbers"
   // "left side has no house numbers, and right side has even house numbers"
   } else if ( ((rightSideStart % 2) == 0) &&
               ((rightSideEnd % 2) == 0) &&
               ( ( ((leftSideStart % 2) != 0) &&
                   ((leftSideEnd % 2) != 0) ) ||
                 ( (leftSideStart == 0) &&
                   (leftSideEnd == 0)) ) ) {
      return ItemTypes::leftOddStreetNumbers;
     
   //Deals with mixed house numbers
   } else {
      return ItemTypes::mixedStreetNumbers;
   }
      
}


StringCode
ItemTypes::getStreetConditionSC(streetCondition t) 
{
   switch (t) {
      case (pavedStreet) :
         return (StringTable::PAVED);
         break;
      case (unpavedStreet) :
         return (StringTable::UNPAVED);
         break;
      case (poorConditionStreet) :
         return (StringTable::POOR_CONDITION);
         break;
      default :
         return (StringTable::NOTSUPPORTED);
   }
}

StringCode 
ItemTypes::getIncStreetConditionSC(int &pos) 
{
   if ( (pos >= 0) && (pos < 3) ) {
      pos++;
      return ( getStreetConditionSC( streetCondition(pos-1) ) );
   } else {
      pos = -1;
      return (StringTable::NOSTRING);
   }
}


StringCode
ItemTypes::getSpeedLimitSC(speedLimit t)
{
   switch (t) {
      case (speed_10) :
         return (StringTable::SPEED_10);
         break;
      case (speed_20) :
         return (StringTable::SPEED_20);
         break;
      case (speed_30) :
         return (StringTable::SPEED_30);
         break;
      case (speed_50) :
         return (StringTable::SPEED_50);
         break;
      case (speed_70) :
         return (StringTable::SPEED_70);
         break;
      case (speed_90) :
         return (StringTable::SPEED_90);
         break;
      case (speed_110) :
         return (StringTable::SPEED_110);
         break;
      default :
         return (StringTable::NOTSUPPORTED);
   }
}

StringCode 
ItemTypes::getIncSpeedLimitSC(int &pos) 
{
   if ( (pos >= 0) && (pos < 7) ) {
      pos++;
      return (getSpeedLimitSC( speedLimit(pos-1) ));
   } else {
      pos = -1;
      return (StringTable::NOSTRING);
   }
}



StringCode
ItemTypes::getEntryRestrictionSC(entryrestriction_t t) 
{
   switch (t) {
      case (noRestrictions) :
         return (StringTable::NORESTRICTIONS);
         break;
      case (noThroughfare) :
         return (StringTable::NOTHROUGHFARE);
         break;
      case (noEntry) :
         return (StringTable::NOENTRY);
         break;
      case (noWay) :
         return (StringTable::NOWAY);
         break;
      default :
         return (StringTable::NOTSUPPORTED);
   }
}

StringCode 
ItemTypes::getIncEntryRestrictionSC(int &pos) 
{
   if ( (pos >= 0) && (pos < 3) ) {
      // Any of the values except for the last
      pos++;
      return (getEntryRestrictionSC( entryrestriction_t(pos-1) ));
   } else if (pos == 3) {
      // The last one
      pos = -1;
      return (getEntryRestrictionSC( entryrestriction_t(3) ));
   } else {
      // Invalid value
      pos = -1;
      return (StringTable::NOSTRING);
   }
}

int
ItemTypes::getEntryRestriction(const char* str, const LanguageCode& lc)
{
   // Make sure that str is ok
   if ( (str == NULL) || (strlen(str) < 1)) {
      return -1;
   }

   // Check the entry-restrictions one-by-one
   if (StringUtility::strcasecmp(
            str, 
            StringTable::getString(StringTable::NORESTRICTIONS, lc)) == 0) {
      return int(noRestrictions);
   } else if (StringUtility::strcasecmp(
               str, 
               StringTable::getString(StringTable::NOTHROUGHFARE, lc)) == 0) {
      return int(noThroughfare);
   } else if (StringUtility::strcasecmp(
               str, 
               StringTable::getString(StringTable::NOENTRY, lc)) == 0) {
      return int(noEntry);
   } else if (StringUtility::strcasecmp(
               str, 
               StringTable::getString(StringTable::NOWAY, lc)) == 0) {
      return int(noWay);
   }

   // Did not found any matching entryrestriction, return -1
   return -1;
}



StringCode
ItemTypes::getRoadClassSC(roadClass t) 
{
   switch (t) {
      case (mainRoad) :
         return (StringTable::MAINCLASSROAD);
         break;
      case (firstClassRoad) :
         return (StringTable::FIRSTCLASSROAD);
         break;
      case (secondClassRoad) :
         return (StringTable::SECONDCLASSROAD);
         break;
      case (thirdClassRoad) :
         return (StringTable::THIRDCLASSROAD);
         break;
      case (fourthClassRoad) :
         return (StringTable::FOURTHCLASSROAD);
         break;
      default :
         return (StringTable::NOTSUPPORTED);
   }
}

StringCode 
ItemTypes::getIncRoadClassSC(int &pos) 
{
   if ( (pos >= 0) && (pos < 5) ) {
      pos++;
      return (getRoadClassSC( roadClass(pos-1) ));
   } else {
      pos = -1;
      return (StringTable::NOSTRING);
   }
}

const ItemTypes::vehicle_t ItemTypes::firstNonSupportedVehicle = motorcycle;


StringCode 
ItemTypes::getVehicleSC(vehicle_t t)
{
   switch (t) {
      case ItemTypes::passengerCar:
         return (StringTable::PASSENGERCARS);

      case ItemTypes::transportTruck :
         return (StringTable::TRANSPORTATIONTRUCK);

      case ItemTypes::publicBus :
         return (StringTable::PUBLICBUS);

      case ItemTypes::bicycle :
         return (StringTable::BICYCLE);

      case ItemTypes::taxi :
         return (StringTable::TAXI);

      case ItemTypes::emergencyVehicle :
         return (StringTable::EMERGENCYVEHICLE);

      case ItemTypes::highOccupancyVehicle :
         return (StringTable::HIGHOCCUPANCYVEHICLE);

      case ItemTypes::pedestrian :
         return (StringTable::PEDESTRIAN);

      case ItemTypes::residentialVehicle :
         return (StringTable::RESIDENTIALVEHICLE);

      case ItemTypes::carWithTrailer :
         return (StringTable::CARWITHTRAILER);

      case ItemTypes::privateBus :
         return (StringTable::PRIVATEBUS);

      case ItemTypes::militaryVehicle :
         return (StringTable::MILITARYVEHICLE);

      case ItemTypes::deliveryTruck :
         return (StringTable::DELIVERYTRUCK);

      case ItemTypes::passCarClosedSeason :
         return (StringTable::PASSCARCLOSEDSEASON);
         
      case ItemTypes::motorcycle :
         return (StringTable::MOTORCYCLE);

      case ItemTypes::moped :
         return (StringTable::MOPED);

      case ItemTypes::farmVehicle :
         return (StringTable::FARMVEHICLE);

      case ItemTypes::privateVehicle :
         return (StringTable::PRIVATEVEHICLE);

      case ItemTypes::waterPollutingLoad :
         return (StringTable::WATERPOLLUTINGLOAD);

      case ItemTypes::explosiveLoad :
         return (StringTable::EXPLOSIVELOAD);

      case ItemTypes::dangerousLoad :
         return (StringTable::DANGEROUSLOAD);

      case ItemTypes::publicTransportation :
         return (StringTable::PUBLICTRANSPORTATION);

      default  :
         return (StringTable::NOSTRING );
   }
}

int 
ItemTypes::getVehicleIndex( vehicle_t t )
{
   int pos = getFirstVehiclePosition();
   int index = 0;
   StringTable::stringCode strCode = getVehicleSC(t);
   StringTable::stringCode strCode2 = getIncVehicleSC(pos); 
   while( (strCode != strCode2) && (pos != -1 ) ){
      index++;
      strCode2 = getIncVehicleSC(pos); 
   }
   return index;
}


StringCode 
ItemTypes::getIncVehicleSC(int &pos, bool allTypes) 
{
   StringTable::stringCode strCode = getVehicleSC(vehicle_t(pos));
   
   // Nostring or the last one
   if (strCode == StringTable::NOSTRING) {
      pos = -1;
   } else {
      switch(vehicle_t(pos)){
      case passengerCar :
         pos = pedestrian;
         break;
      case pedestrian :
         pos = publicTransportation;
         break;
      case publicTransportation :
         pos = emergencyVehicle;
         break;
      case emergencyVehicle :
         pos = taxi;
         break;
      case taxi :
         pos = publicBus;
         break;
      case publicBus :
         pos = deliveryTruck;
         break;
      case deliveryTruck :
         pos = transportTruck ;
         break;
      case transportTruck  :
         pos = highOccupancyVehicle;
         break;
      case highOccupancyVehicle :
         pos = bicycle;
         break;
      case passCarClosedSeason :
         pos = motorcycle;
         break;
      case bicycle :
         pos = passCarClosedSeason;
         break;
      case motorcycle :
         pos = moped;
         break;
      case moped :
         pos = privateBus;
         break;
      case privateBus :
         pos = militaryVehicle;
         break;
      case militaryVehicle :
         pos = residentialVehicle;
         break;
      case residentialVehicle :
         pos = carWithTrailer ;
         break;
      case carWithTrailer  :
         pos = farmVehicle;
         break;
      case farmVehicle :
         pos = privateVehicle;
         break;
      case privateVehicle :
         pos = waterPollutingLoad;
         break;                                                             
      case waterPollutingLoad :
         pos = explosiveLoad ;
         break;
      case explosiveLoad  :
         pos = dangerousLoad;
         break;
      case dangerousLoad :
         pos = -1;
         break;
      // avoidTollRoad and avoidHighway only included to get rid of warning
      case avoidTollRoad:
         pos = -1;
         break;
      case avoidHighway:
         pos = -1;
         break;
      }
   }
   
   // Make sure only the supported vehicle types will be listed.
   if ((! allTypes) && (pos == firstNonSupportedVehicle)) {
      pos = -1;
   }
   
   return strCode;
}


ItemTypes::vehicle_t 
ItemTypes::getVehicleFromString( 
   const char* const vehicleType,
   vehicle_t defaultType )
{
   vehicle_t vRes = defaultType;
   int vPos = getFirstVehiclePosition();
   int currPos = vPos;
   
   StringTable::stringCode sc = getIncVehicleSC( vPos );

   while ( sc != StringTable::NOSTRING ) {
      if ( StringUtility::strcasecmp( StringTable::getString( 
         sc, StringTable::ENGLISH ), vehicleType ) == 0 )
      {
         // Found
         vRes = vehicle_t( currPos );
         break;
      }
      currPos = vPos;
      sc = getIncVehicleSC( vPos );
   }

   return vRes;
}


ItemTypes::vehicle_t 
ItemTypes::transportation2Vehicle( transportation_t trans )
{
   switch ( trans ) {
      case undefined : // This should not happen
      case drive :
         return passengerCar;
      case walk :
         return pedestrian;
      case bike :
         return bicycle;
      case bus :
         return publicBus;
      // No default to make sure function gets updated when new types are
      // added.
   }

   // We should never reach this code but the compiler complains
   return passengerCar;
}

ItemTypes::transportation_t
ItemTypes::vehicle2Transportation( vehicle_t vehicleT )
{
   switch ( vehicleT ) {
      case passengerCar:
      case emergencyVehicle:
      case taxi:
      case deliveryTruck:
      case transportTruck:
      case highOccupancyVehicle:
      case motorcycle:
      case moped:
      case militaryVehicle:
      case residentialVehicle:
      case carWithTrailer:
      case farmVehicle:
      case privateVehicle:
      case waterPollutingLoad:
      case explosiveLoad:
      case dangerousLoad:
      case privateBus:
      case passCarClosedSeason:
      case avoidTollRoad:
      case avoidHighway:
         return drive;
      case pedestrian:
         return walk;
      case bicycle:
         return bike;
      case publicBus:
      case publicTransportation:
         return bus;
         // No default to make sure function gets updated when new types are
         // added.
   }
   // We should never reach this code but the compiler complains
   return drive;
}

ItemTypes::transportation_t
ItemTypes::vehicle2Transportation( uint32 vehicleMask )
{
   return vehicle2Transportation( vehicle_t(vehicleMask));
}

StringCode 
ItemTypes::getIncTurnDirectionSC(int &pos) 
{
   switch (pos) {
      case (ItemTypes::UNDEFINED) :
         pos = 1;
         return (StringTable::UNDEFINED_TURN);
      break;

      case (ItemTypes::LEFT) :
         pos = 2;
         return (StringTable::LEFT_TURN);
      break;
      
      case (ItemTypes::AHEAD) :
         pos = 3;
         return (StringTable::AHEAD_TURN);
      break;

      case (ItemTypes::RIGHT) :
         pos = 4;
         return (StringTable::RIGHT_TURN);
      break;

      case (ItemTypes::UTURN) :
         pos = 5;
         return (StringTable::U_TURN);
      break;

      case (ItemTypes::FOLLOWROAD) :
         pos = 6;
         return (StringTable::FOLLOWROAD_TURN);
      break;

      case (ItemTypes::ENTER_ROUNDABOUT) :
         pos = 7;
         return (StringTable::ENTER_ROUNDABOUT_TURN);
      break;

      case (ItemTypes::EXIT_ROUNDABOUT) :
         pos = 8;
         return (StringTable::EXIT_ROUNDABOUT_TURN);
      break;

      case (ItemTypes::RIGHT_ROUNDABOUT) :
         pos = 9;
         return (StringTable::RIGHT_ROUNDABOUT_TURN);
      break;

      case (ItemTypes::LEFT_ROUNDABOUT) :
         pos = 10;
         return (StringTable::LEFT_ROUNDABOUT_TURN);
      break;

      case (ItemTypes::ON_RAMP) :
         pos = 11;
         return (StringTable::ON_RAMP_TURN);
      break;
      
      case (ItemTypes::OFF_RAMP) :
         pos = 12;
         return (StringTable::OFF_RAMP_TURN);
      break;

      case (ItemTypes::ENTER_BUS) :
         pos = 13;
         return (StringTable::ENTER_BUS_TURN);
      break;

      case (ItemTypes::EXIT_BUS) :
         pos = 14;
         return (StringTable::EXIT_BUS_TURN);
      break;

      case (ItemTypes::CHANGE_BUS) :
         pos = 15;
         return (StringTable::CHANGE_BUS_TURN);
      break;

      case (ItemTypes::KEEP_RIGHT) :
         pos = 16;
         return (StringTable::KEEP_RIGHT);
      break;

      case (ItemTypes::KEEP_LEFT) :
         pos = 17;
         return (StringTable::KEEP_LEFT);
      break;

      case (ItemTypes::ENTER_FERRY) :
         pos = 18;
         return (StringTable::ENTER_FERRY_TURN);
      break;

      case (ItemTypes::EXIT_FERRY) :
         pos = 19;
         return (StringTable::EXIT_FERRY_TURN);
      break;

      case (ItemTypes::CHANGE_FERRY) :
         pos = 20;
         return (StringTable::CHANGE_FERRY_TURN);
      break;
      
      case (ItemTypes::OFF_RAMP_LEFT) :
         pos = 21;
         return (StringTable::LEFT_OFF_RAMP_TURN);
      break;
      
      case (ItemTypes::OFF_RAMP_RIGHT) :
         //pos = 22; // multi connection turn has no string,
         pos = 23;   // so we go directly to ahead-in-roundabout
         return (StringTable::RIGHT_OFF_RAMP_TURN);
      break;  
      
      //case (ItemTypes::MULTI_CONNECTION_TURN) :
      //   pos = 23;
      //   return (StringTable::NOSTRING);
      //break;  

      case (ItemTypes::AHEAD_ROUNDABOUT) :
         pos = -1;
         return (StringTable::AHEAD_ROUNDABOUT_TURN);
      break;


      default:
         pos = -1;
         return (StringTable::NOSTRING);
   }
}

StringCode 
ItemTypes::getIncCrossingKindSC(int &pos) 
{
   switch (pos) {
      case (ItemTypes::UNDEFINED_CROSSING) : 
         pos = 1;
         return (StringTable::UNDEFINED_CROSSING);
      
      case (ItemTypes::NO_CROSSING) : 
         pos = 2;
         return (StringTable::NO_CROSSING);
      
      case (ItemTypes::CROSSING_3WAYS_T) : 
         pos = 3;
         return (StringTable::CROSSING_3WAYS_T);
      
      case (ItemTypes::CROSSING_3WAYS_Y) :
         pos = 4;
         return (StringTable::CROSSING_3WAYS_Y);
      
      case (ItemTypes::CROSSING_4WAYS) : 
         pos = 5;
         return (StringTable::CROSSING_4WAYS);
      
      case (ItemTypes::CROSSING_5WAYS) : 
         pos = 6;
         return (StringTable::CROSSING_5WAYS);
      
      case (ItemTypes::CROSSING_6WAYS) :
         pos = 7;
         return (StringTable::CROSSING_6WAYS);
      
      case (ItemTypes::CROSSING_7WAYS) : 
         pos = 8;
         return (StringTable::CROSSING_7WAYS);
      
      case (ItemTypes::CROSSING_8WAYS) :
         pos = 9;
         return (StringTable::CROSSING_8WAYS);
      
      case (ItemTypes::CROSSING_2ROUNDABOUT) :
         pos = 10;
         return (StringTable::CROSSING_2ROUNDABOUT);

      case (ItemTypes::CROSSING_3ROUNDABOUT) : 
         pos = 11;
         return (StringTable::CROSSING_3ROUNDABOUT);

      case (ItemTypes::CROSSING_4ROUNDABOUT) :
         pos = 12;
         return (StringTable::CROSSING_4ROUNDABOUT);

      case (ItemTypes::CROSSING_4ROUNDABOUT_ASYMMETRIC) :
         pos = 13;
         return (StringTable::CROSSING_4ROUNDABOUT_ASYM);

      case (ItemTypes::CROSSING_5ROUNDABOUT) :
         pos = 14;
         return (StringTable::CROSSING_5ROUNDABOUT);
      
      case (ItemTypes::CROSSING_6ROUNDABOUT) :
         pos = 15;
         return (StringTable::CROSSING_6ROUNDABOUT);
      
      case (ItemTypes::CROSSING_7ROUNDABOUT) :
         pos = -1;
         return (StringTable::CROSSING_7ROUNDABOUT);
      
      default:
         pos = -1;
         return (StringTable::NOSTRING);
   }
}

int
ItemTypes::getWaterTypeFromString(const char* waterTypeStr)
{
   // Make sure that waterTypeStr is ok
   if ( (waterTypeStr == NULL) || (strlen(waterTypeStr) < 1)) {
      return -1;
   }
   
   // Check the water types
   if (StringUtility::strcasecmp(waterTypeStr, "ocean") == 0)
      return int(ItemTypes::ocean);
   
   else if (StringUtility::strcasecmp(waterTypeStr, "lake") == 0)
      return int(ItemTypes::lake);
   
   else if (StringUtility::strcasecmp(waterTypeStr, "river") == 0)
      return int(ItemTypes::river);
   
   else if (StringUtility::strcasecmp(waterTypeStr, "canal") == 0)
      return int(ItemTypes::canal);
   
   else if (StringUtility::strcasecmp(waterTypeStr, "harbour") == 0)
      return int(ItemTypes::harbour);

   else if (StringUtility::strcasecmp(waterTypeStr, "otherWaterElement") == 0)
      return int(ItemTypes::otherWaterElement);

   else if (StringUtility::strcasecmp(waterTypeStr, "unknownWaterElement") == 0)
      return int(ItemTypes::unknownWaterElement);
   
   // Did not find any matching watertype, return -1
   return -1;
}

const char* 
ItemTypes::getWaterTypeAsString(waterType type)
{
   switch (type) {
      case ocean :
         return "ocean";
         break;
      case lake :
         return "lake";
         break;
      case river :
         return "river";
         break;
      case canal :
         return "canal";
         break;
      case harbour :
         return "harbour";
         break;
      case otherWaterElement :
         return "otherWaterElement";
         break;
      case unknownWaterElement :
         return "unknownWaterElement";
         break;
   }

   return "ocean";
}

int
ItemTypes::getParkTypeFromString(const char* parkTypeStr)
{
   // Make sure the string is OK
   if ( (parkTypeStr == NULL) || (strlen(parkTypeStr) < 1)) {
      return -1;
   }
      
   if (StringUtility::strcasecmp(parkTypeStr, "cityPark") == 0)
      return int(ItemTypes::cityPark);

   else if (StringUtility::strcasecmp(parkTypeStr,
                                      "regionOrNationalPark") == 0)
      return int(ItemTypes::regionOrNationalPark);

   // Did not find any matching parktype, return -1
   return  -1;
}

int
ItemTypes::getFerryTypeFromString(const char* ferryTypeStr)
{
   // Make sure the string is OK
   if ( (ferryTypeStr == NULL) || (strlen(ferryTypeStr) < 1)) {
      return -1;
   }
      
   if (StringUtility::strcasecmp(ferryTypeStr, "operatedByShip") == 0)
      return int(ItemTypes::operatedByShip);

   else if (StringUtility::strcasecmp(ferryTypeStr,
                                      "operatedByTrain") == 0)
      return int(ItemTypes::operatedByTrain);

   // Did not find any matching ferrytype, return -1
   return  -1;
}


StringCode 
ItemTypes::getPOIStringCode(pointOfInterest_t t)
{
   switch (t) {
      case company :
         return StringTable::COMPANY;
      case airport :
         return StringTable::AIRPORT;
      case amusementPark :
         return StringTable::AMUSEMENT_PARK;
      case atm :
         return StringTable::ATM;
      case automobileDealership :
         return StringTable::AUTOMOBILE_DEALERSHIP;
      case bank :
         return StringTable::BANK;
      case bowlingCentre :
         return StringTable::BOWLING_CENTRE;
      case busStation :
         return StringTable::BUS_STATION;
      case businessFacility :
         return StringTable::BUSINESS_FACILITY;
      case casino :
         return StringTable::CASINO;
      case cinema :
         return StringTable::CINEMA;
      case cityCentre :
         return StringTable::CITY_CENTRE;
      case cityHall :
         return StringTable::CITY_HALL;
      case communityCentre :
         return StringTable::COMMUNITY_CENTRE;
      case commuterRailStation :
         return StringTable::COMMUTER_RAIL_STATION;
      case courtHouse :
         return StringTable::COURT_HOUSE;
      case exhibitionOrConferenceCentre :
         return StringTable::EXHIBITION_OR_CONFERENCE_CENTRE;
      case ferryTerminal :
         return StringTable::FERRY_TERMINAL;
      case frontierCrossing :
         return StringTable::FRONTIER_CROSSING;
      case golfCourse :
         return StringTable::GOLF_COURSE;
      case groceryStore :
         return StringTable::GROCERY_STORE;
      case historicalMonument :
         return StringTable::HISTORICAL_MONUMENT;
      case hospital :
         return StringTable::HOSPITAL;
      case hotel :
         return StringTable::HOTEL;
      case iceSkatingRink :
         return StringTable::ICE_SKATING_RINK;
      case library :
         return StringTable::LIBRARY;
      case marina :
         return StringTable::MARINA;
      case motoringOrganisationOffice :
         return StringTable::MOTORING_ORGANISATION_OFFICE;
      case museum :
         return StringTable::MUSEUM;
      case nightlife :
         return StringTable::NIGHTLIFE;
      case openParkingArea :
         return StringTable::OPEN_PARKING_AREA;
      case parkAndRide :
         return StringTable::PARK_AND_RIDE;
      case parkingGarage :
         return StringTable::PARKING_GARAGE;
      case petrolStation :
         return StringTable::PETROL_STATION;
      case policeStation :
         return StringTable::POLICE_STATION;
      case publicSportAirport :
         return StringTable::PUBLIC_SPORT_AIRPORT;
      case railwayStation :
         return StringTable::RAILWAY_STATION;
      case recreationFacility :
         return StringTable::RECREATION_FACILITY;
      case rentACarFacility :
         return StringTable::RENT_A_CAR_FACILITY;
      case restArea :
         return StringTable::REST_AREA;
      case restaurant :
         return StringTable::RESTAURANT;
      case school :
         return StringTable::SCHOOL;
      case shoppingCentre :
         return StringTable::SHOPPING_CENTRE;
      case skiResort :
         return StringTable::SKI_RESORT;
      case sportsActivity :
         return StringTable::SPORTS_ACTIVITY;
      case sportsCentre :
         return StringTable::SPORTS_CENTRE;
      case theatre :
         return StringTable::THEATRE;
      case touristAttraction :
         return StringTable::TOURIST_ATTRACTION;
      case touristOffice :
         return StringTable::TOURIST_OFFICE;
      case university :
         return StringTable::UNIVERSITY;
      case vehicleRepairFacility :
         return StringTable::VEHICLE_REPAIR_FACILITY;
      case winery :
         return StringTable::WINERY;
      case tramStation :
         return StringTable::TRAM_STATION;
      case industrialComplex:
         return StringTable::INDUSTRIAL_COMPLEX;
      case cemetery:
         return StringTable::CEMETERY;
      case publicIndividualBuilding:
         return StringTable::PUBLIC_INDIVIDUAL_BUILDING;
      case otherIndividualBuilding:
         return StringTable::OTHER_INDIVIDUAL_BUILDING;      
      case postOffice :
         return StringTable::POST_OFFICE;
      case shop:
         return StringTable::POI_SHOP;
      case multi :
      case notCategorised:
      case unknownType :
         return StringTable::UNKNOWN_TYPE;
      case airlineAccess :
         return StringTable::AIRLINE_ACCESS;
      case beach :
         return StringTable::BEACH;
      case campingGround :
         return StringTable::CAMPING_GROUND;
      case carDealer :
         return StringTable::CAR_DEALER;
      case concertHall :
         return StringTable::CONCERT_HALL;
      case tollRoad :
         return StringTable::TOLL_ROAD;
      case culturalCentre :
         return StringTable::CULTURAL_CENTRE;
      case dentist :
         return StringTable::DENTIST;
      case doctor :
         return StringTable::DOCTOR;
      case driveThroughBottleShop :
         return StringTable::DRIVE_THROUGH_BOTTLE_SHOP;
      case embassy :
         return StringTable::EMBASSY;
      case entryPoint :
         return StringTable::ENTRY_POINT;
      case governmentOffice :
         return StringTable::GOVERNMENT_OFFICE;
      case mountainPass :
         return StringTable::MOUNTAIN_PASS;
      case mountainPeak :
         return StringTable::MOUNTAIN_PEAK;
      case musicCentre :
         return StringTable::MUSIC_CENTRE;
      case opera :
         return StringTable::OPERA;
      case parkAndRecreationArea :
         return StringTable::PARK_AND_RECREATION_AREA;
      case pharmacy :
         return StringTable::PHARMACY;
      case placeOfWorship :
         return StringTable::PLACE_OF_WORSHIP;
      case rentACarParking :
         return StringTable::RENT_A_CAR_PARKING;
      case restaurantArea :
         return StringTable::RESTAURANT_AREA;
      case scenicView :
         return StringTable::SCENIC_VIEW;
      case stadium :
         return StringTable::STADIUM;
      case swimmingPool :
         return StringTable::SWIMMING_POOL;
      case tennisCourt :
         return StringTable::TENNIS_COURT;
      case vetrinarian :
         return StringTable::VETRINARIAN;
      case waterSports :
         return StringTable::WATER_SPORTS;
      case yachtBasin :
         return StringTable::YACHT_BASIN;
      case zoo :
         return StringTable::ZOO;
      case wlan :
         return StringTable::WLAN;
      case noType :
         return StringTable::POI_NOTYPE;
      case invalidPOIType :
         return StringTable::UNKNOWN_TYPE;
      case church :
         return StringTable::CHURCH;
      case mosque :
         return StringTable::MOSQUE;
      case synagogue :
         return StringTable::SYNAGOGUE;
      case subwayStation :
         return StringTable::SUBWAY_STATION;
      case cafe :
         return StringTable::CAFE;
      case hinduTemple :
         return StringTable::HINDU_TEMPLE;
      case buddhistSite :
         return StringTable::BUDDHIST_SITE;
      case busStop :
         return StringTable::BUS_STOP_POI_TYPE;
      case taxiStop :
         return StringTable::TAXI_STOP_POI_TYPE;
      case nbr_pointOfInterest :
         return StringTable::NOSTRING;
   }

   return StringTable::NOSTRING;
}

int
ItemTypes::getLandmarkLocationFromString(const char* str)
{
   // Make sure the string is OK
   if ( (str == NULL) || (strlen(str) < 1)) {
      return -1;
   }
      
   if (StringUtility::strcasecmp(str, "after") == 0)
      return int(ItemTypes::after);
   
   else if (StringUtility::strcasecmp(str, "before") == 0)
      return int(ItemTypes::before);

   else if (StringUtility::strcasecmp(str, "in") == 0)
      return int(ItemTypes::in);

   else if (StringUtility::strcasecmp(str, "at") == 0)
      return int(ItemTypes::at);

   else if (StringUtility::strcasecmp(str, "pass") == 0)
      return int(ItemTypes::pass);

   else if (StringUtility::strcasecmp(str, "into") == 0)
      return int(ItemTypes::into);
   
   else if (StringUtility::strcasecmp(str, "arrive") == 0)
      return int(ItemTypes::arrive);

   // Did not find any matching landmark location, return -1
   return  -1;
}


StringCode 
ItemTypes::getLandmarkLocationSC(landmarklocation_t t)
{
   switch (t) {
      case after :
         return StringTable::LMLOCATION_AFTER;
      case before :
         return StringTable::LMLOCATION_BEFORE;
      case in :
         return StringTable::LMLOCATION_IN;
      case at :
         return StringTable::LMLOCATION_AT;
      case pass :
         return StringTable::LMLOCATION_PASS;
      case into :
         return StringTable::LMLOCATION_INTO;
      case arrive :
         return StringTable::LMLOCATION_ARRIVE;
      default :
         return StringTable::NOSTRING;
   }
   return StringTable::NOSTRING;
}

int
ItemTypes::getLandmarkSideFromString(const char* str)
{
   // Make sure the string is OK
   if ( (str == NULL) || (strlen(str) < 1)) {
      return -1;
   }
      
   if (StringUtility::strcasecmp(str, "left_side") == 0)
      return int(SearchTypes::left_side);
   
   if (StringUtility::strcasecmp(str, "right_side") == 0)
      return int(SearchTypes::right_side);

   if (StringUtility::strcasecmp(str, "unknown_side") == 0)
      return int(SearchTypes::unknown_side);

   if (StringUtility::strcasecmp(str, "undefined_side") == 0)
      return int(SearchTypes::undefined_side);

   if (StringUtility::strcasecmp(str, "side_does_not_matter") == 0)
      return int(SearchTypes::side_does_not_matter);

   // the other side_t's are not used for landmark side description.

   return -1;
}

StringCode 
ItemTypes::getLandmarkSideSC(SearchTypes::side_t t)
{
   switch (t) {
      case SearchTypes::left_side :
         return StringTable::LMSIDE_LEFT;
      case SearchTypes::right_side :
         return StringTable::LMSIDE_RIGHT;
      case SearchTypes::side_does_not_matter :
         return StringTable::LMSIDE_BOTH;
      default :
         return StringTable::NOSTRING;
   }
   return StringTable::NOSTRING;
}


StringCode 
ItemTypes::getTurndirectionSC(turndirection_t t)
{
   switch (t) {
      case UNDEFINED :
         return StringTable::UNDEFINED_TURN;
      case LEFT :
         return StringTable::LEFT_TURN;
      case AHEAD :
         return StringTable::AHEAD_TURN;
      case RIGHT :
         return StringTable::RIGHT_TURN;
      case UTURN :
         return StringTable::U_TURN;
      case FOLLOWROAD :
         return StringTable::FOLLOWROAD_TURN;
      case ENTER_ROUNDABOUT :
         return StringTable::ENTER_ROUNDABOUT_TURN;
      case EXIT_ROUNDABOUT :
         return StringTable::EXIT_ROUNDABOUT_TURN;
      case RIGHT_ROUNDABOUT :
         return StringTable::RIGHT_ROUNDABOUT_TURN;
      case LEFT_ROUNDABOUT :
         return StringTable::LEFT_ROUNDABOUT_TURN;
      case ON_RAMP :
         return StringTable::ON_RAMP_TURN;
      case OFF_RAMP :
         return StringTable::OFF_RAMP_TURN;
      case ENTER_BUS :
         return StringTable::ENTER_BUS_TURN;
      case EXIT_BUS :
         return StringTable::EXIT_BUS_TURN;
      case CHANGE_BUS :
         return StringTable::CHANGE_BUS_TURN;
      case KEEP_RIGHT :
         return StringTable::KEEP_RIGHT;
      case KEEP_LEFT :
         return StringTable::KEEP_LEFT;
      case ENTER_FERRY :
         return StringTable::ENTER_FERRY_TURN;
      case EXIT_FERRY :
         return StringTable::EXIT_FERRY_TURN;
      case CHANGE_FERRY :
         return StringTable::CHANGE_FERRY_TURN;
      case OFF_RAMP_LEFT :
         return StringTable::LEFT_OFF_RAMP_TURN;
      case OFF_RAMP_RIGHT :
         return StringTable::RIGHT_OFF_RAMP_TURN;
      case MULTI_CONNECTION_TURN :
         return StringTable::NOSTRING;
      case AHEAD_ROUNDABOUT :
         return StringTable::AHEAD_ROUNDABOUT_TURN;
   }
   return StringTable::NOSTRING;
}


int
ItemTypes::getTurnDirection(const char* str, const LanguageCode& lc)
{
   // Make sure that str is ok
   if ( (str == NULL) || (strlen(str) < 1)) {
      return -1;
   }

   // Check the turn-directions one-by-one
   if (StringUtility::strcasecmp(
            str, 
            StringTable::getString(StringTable::UNDEFINED_TURN, lc)) == 0) {
      return int(UNDEFINED);
   } else if (StringUtility::strcasecmp(
               str, 
               StringTable::getString(StringTable::LEFT_TURN, lc)) == 0) {
      return int(LEFT);
   } else if (StringUtility::strcasecmp(
               str, 
               StringTable::getString(StringTable::AHEAD_TURN, lc)) == 0) {
      return int(AHEAD);
   } else if (StringUtility::strcasecmp(
               str, 
               StringTable::getString(StringTable::RIGHT_TURN, lc)) == 0) {
      return int(RIGHT);
   } else if (StringUtility::strcasecmp(
               str, 
               StringTable::getString(StringTable::U_TURN, lc)) == 0) {
      return int(UTURN);
   } else if (StringUtility::strcasecmp(
               str, 
               StringTable::getString(StringTable::FOLLOWROAD_TURN, lc)) == 0) {
      return int(FOLLOWROAD);
   } else if (StringUtility::strcasecmp(
               str, StringTable::getString(
                  StringTable::ENTER_ROUNDABOUT_TURN, lc)) == 0) {
      return int(ENTER_ROUNDABOUT);
   } else if (StringUtility::strcasecmp(
               str, StringTable::getString(
                  StringTable::EXIT_ROUNDABOUT_TURN, lc)) == 0) {
      return int(EXIT_ROUNDABOUT);
   } else if (StringUtility::strcasecmp(
               str, StringTable::getString(
                  StringTable::RIGHT_ROUNDABOUT_TURN, lc)) == 0) {
      return int(RIGHT_ROUNDABOUT);
   } else if (StringUtility::strcasecmp(
               str, StringTable::getString(
                  StringTable::LEFT_ROUNDABOUT_TURN, lc)) == 0) {
      return int(LEFT_ROUNDABOUT);
   } else if (StringUtility::strcasecmp(
               str, StringTable::getString(
                  StringTable::AHEAD_ROUNDABOUT_TURN, lc)) == 0) {
      return int(AHEAD_ROUNDABOUT);
   } else if (StringUtility::strcasecmp(
               str, StringTable::getString(
                  StringTable::ON_RAMP_TURN, lc)) == 0) {
      return int(ON_RAMP);
   } else if (StringUtility::strcasecmp(
               str, StringTable::getString(
                  StringTable::OFF_RAMP_TURN, lc)) == 0) {
      return int(OFF_RAMP);
   } else if (StringUtility::strcasecmp(
               str, StringTable::getString(
                  StringTable::LEFT_OFF_RAMP_TURN, lc)) == 0) {
      return int(OFF_RAMP_LEFT);
   } else if (StringUtility::strcasecmp(
               str, StringTable::getString(
                  StringTable::RIGHT_OFF_RAMP_TURN, lc)) == 0) {
      return int(OFF_RAMP_RIGHT);   
   } else if (StringUtility::strcasecmp(
               str, StringTable::getString(
                  StringTable::ENTER_BUS_TURN, lc)) == 0) {
      return int(ENTER_BUS);
   } else if (StringUtility::strcasecmp(
               str, StringTable::getString(
                  StringTable::EXIT_BUS_TURN, lc)) == 0) {
      return int(EXIT_BUS);
   } else if (StringUtility::strcasecmp(
               str, StringTable::getString(
                  StringTable::CHANGE_BUS_TURN, lc)) == 0) {
      return int(CHANGE_BUS);
   } else if (StringUtility::strcasecmp(
               str, StringTable::getString(
                  StringTable::KEEP_RIGHT, lc)) == 0) {
      return int(KEEP_RIGHT);
   } else if (StringUtility::strcasecmp(
               str, StringTable::getString(
                  StringTable::KEEP_LEFT, lc)) == 0) {
      return int(KEEP_LEFT);
   } else if (StringUtility::strcasecmp(
               str, StringTable::getString(
                  StringTable::ENTER_FERRY_TURN, lc)) == 0) {
      return int(ENTER_FERRY);
   } else if (StringUtility::strcasecmp(
               str, StringTable::getString(
                  StringTable::EXIT_FERRY_TURN, lc)) == 0) {
      return int(EXIT_FERRY);
   } else if (StringUtility::strcasecmp(
               str, StringTable::getString(
                  StringTable::CHANGE_FERRY_TURN, lc)) == 0) {
      return int(CHANGE_FERRY);
   }

   // Did not found any matching entryrestriction, return -1
   return -1;
}


StringCode 
ItemTypes::getCrossingKindSC(crossingkind_t t)
{
   switch (t) {
      case UNDEFINED_CROSSING :
         return StringTable::UNDEFINED_CROSSING;
      case NO_CROSSING :
         return StringTable::NO_CROSSING;
      case CROSSING_3WAYS_T :
         return StringTable::CROSSING_3WAYS_T;
      case CROSSING_3WAYS_Y :
         return StringTable::CROSSING_3WAYS_Y;
      case CROSSING_4WAYS :
         return StringTable::CROSSING_4WAYS;
      case CROSSING_5WAYS :
         return StringTable::CROSSING_5WAYS;
      case CROSSING_6WAYS :
         return StringTable::CROSSING_6WAYS;
      case CROSSING_7WAYS :
         return StringTable::CROSSING_7WAYS;
      case CROSSING_8WAYS :
         return StringTable::CROSSING_8WAYS;
      case CROSSING_2ROUNDABOUT :
         return StringTable::CROSSING_2ROUNDABOUT;
      case CROSSING_3ROUNDABOUT :
         return StringTable::CROSSING_3ROUNDABOUT;
      case CROSSING_4ROUNDABOUT :
         return StringTable::CROSSING_4ROUNDABOUT;
      case CROSSING_4ROUNDABOUT_ASYMMETRIC :
         return StringTable::CROSSING_4ROUNDABOUT_ASYM;
      case CROSSING_5ROUNDABOUT :
         return StringTable::CROSSING_5ROUNDABOUT;
      case CROSSING_6ROUNDABOUT :
         return StringTable::CROSSING_6ROUNDABOUT;
      case CROSSING_7ROUNDABOUT :
         return StringTable::CROSSING_7ROUNDABOUT;
   }

   return StringTable::NOSTRING;

}


const uint32 ItemTypes::zoomlevelVect[] =
   { 2, 3, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6 };


const uint32
ItemTypes::c_itemTypeToSearchTypeTable[] =
{
   SEARCH_STREETS,        // streetSegmentItem
   SEARCH_MUNICIPALS ,    // municipalItem
   SEARCH_MISC,           // waterItem
   SEARCH_MISC,           // parkItem
   SEARCH_MISC,           // forestItem
   SEARCH_MISC,           // buildingItem
   SEARCH_MISC,           // railwayItem
   SEARCH_MISC,           // islandItem
   SEARCH_STREETS,        // streetItem
   0,                     // nullItem
   SEARCH_ZIP_CODES,      // zipCodeItem
   SEARCH_BUILT_UP_AREAS, // builtUpAreaItem
   SEARCH_CITY_PARTS,     // cityPartItem
   SEARCH_ZIP_AREAS,      // zipAreaItem
   SEARCH_COMPANIES,      // pointOfInterestItem
   SEARCH_CATEGORIES,     // categoryItem
   0,                     // routeableItem
   0,                     // busRouteItem
   0,                     // ferryItem
   SEARCH_COMPANIES,      // airportItem
   0,                     // aircraftRoadItem
   0,                     // pedestrianAreaItem
   SEARCH_MISC,           // militaryBaseItem
   SEARCH_MISC,           // individualBuildingItem
   SEARCH_COMPANIES,      // subwayLineItem
   0,                     // notUsedItemType
   0,                     // borderItem
   SEARCH_MISC,           // cartographicItem
};

void
ItemTypes::searchTypeToItemTypes(set<ItemTypes::itemType>& typ,
                                 uint32 searchType)
{
   // Should yield "size of array `kalle' is negative" if the
   // c_itemTypeToSearchTypeTable has the wrong number of entries.
   CHECK_ARRAY_SIZE( c_itemTypeToSearchTypeTable,
                     ItemTypes::numberOfItemTypes );
   for ( int i = 0; i < numberOfItemTypes; ++i ) {
      ItemTypes::itemType curType = ItemTypes::itemType(i);
      // Check if the itemtype converts to the search type.
      if ( itemTypeToSearchType(curType) & searchType ) {
         typ.insert(curType);
      }
   }
}


StringCode 
ItemTypes::getStartDirectionHousenumberSC( routedir_nbr_t t)
{
   switch (t) {
      case unknown_nbr_t :
         return StringTable::START_DIRECTION_NBR_UNKNOWN;

      case increasing :
         return StringTable::START_DIRECTION_NBR_INCREASING;

      case decreasing :
         return StringTable::START_DIRECTION_NBR_DECREASING;
   }
   // To make compiler happy
   return StringTable::START_DIRECTION_NBR_UNKNOWN;
}

StringCode 
ItemTypes::getStartDirectionOddEvenSC(routedir_oddeven_t t)
{
   switch (t) {
      case unknown_oddeven_t :
         return StringTable::START_DIRECTION_ODDEVEN_UNKNOWN;

      case leftOddRightEven :
         return StringTable::START_DIRECTION_ODDEVEN_LEFTODD;

      case rightOddLeftEven :
         return StringTable::START_DIRECTION_ODDEVEN_RIGHTODD;
   }
   // To make compiler happy
   return StringTable::START_DIRECTION_ODDEVEN_UNKNOWN;
}

const char* 
ItemTypes::getStringForRoadDisplayClass(roadDisplayClass_t dpClass )
{
   switch (dpClass) {
      case partOfWalkway :
         return "partOfWalkway";
         break;
      case partOfPedestrianZone :
         return "partOfPedestrianZone";
         break;
      case roadForAuthorities :
         return "roadForAuthorities";
         break;
      case entranceExitCarPark :
         return "entranceExitCarPark";
         break;
      case etaParkingGarage :
         return "etaParkingGarage";
         break;
      case etaParkingPlace :
         return "etaParkingPlace";
         break;
      case etaUnstructTrafficSquare :
         return "etaUnstructTrafficSquare";
         break;
      case partOfServiceRoad :
         return "partOfServiceRoad";
         break;
      case roadUnderContruction :
         return "roadUnderContruction";
         break;
      default :
         mc2log << error << here << " Display class " << uint32(dpClass)
                << " not supported - please add here" << endl;
         return "notSupportedPleaseAdd";
         break;
   }

   return "notSupportedPleaseAdd";
}

const char* 
ItemTypes::getStringForAreaFeatureDrawDisplayClass(
   areaFeatureDrawDisplayClass_t dpClass )
{
   switch (dpClass) {
      case waterInCityPark :
         return "waterInCityPark";
         break;
      case waterInCartographic :
         return "waterInCartographic";
         break;
      case waterInBuilding :
         return "waterInBuilding";
         break;
      case waterOnIsland :
         return "waterOnIsland";
         break;
      case islandInBua :
         return "islandInBua";
         break;
      case buaOnIsland :
         return "buaOnIsland";
         break;
      case cartographicInCityPark :
         return "cartographicInCityPark";
         break;
      case cartographicInForest :
         return "cartographicInForest";
         break;
      case IIWIPOutsideParkOutsideBua : 
         return "IIWIPOutsideParkOutsideBua";
         break;
      case IIWIPOutsideParkInsideBua : 
         return "IIWIPOutsideParkInsideBua";
         break;
      case IIWIPInsidePark : 
         return "IIWIPInsidePark";
         break;
      default :
         mc2log << error << here << " Display class " << uint32(dpClass)
                << " not supported - please add here" << endl;
         return "notSupportedPleaseAdd";
         break;
   }

   return "notSupportedPleaseAdd";
}

