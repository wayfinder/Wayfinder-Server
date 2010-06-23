/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "StringConversion.h"


SearchTypes::StringMatching
StringConversion::searchStringMatchingTypeFromString( const char* const s ) {
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!

   if ( StringUtility::strcasecmp( s, "exact" ) == 0 ) {
      return SearchTypes::ExactMatch;
   } else if ( StringUtility::strcasecmp( s, "close" ) == 0 ) {
      return SearchTypes::CloseMatch;
   } else if ( StringUtility::strcasecmp( s, "full" ) == 0 ) {
      return SearchTypes::FullMatch;
   } else if ( StringUtility::strcasecmp( s, "closefull" ) == 0 ) {
      return SearchTypes::CloseFullMatch;
   } else if ( StringUtility::strcasecmp( s, "wildcard" ) == 0 ) {
      return SearchTypes::WildcardMatch;
   } else if ( StringUtility::strcasecmp( s, "allwords" ) == 0 ) {
      return SearchTypes::AllWordsMatch;
   } else if ( StringUtility::strcasecmp( s, "soundex" ) == 0 ) {
      return SearchTypes::SoundexMatch;
   } else if ( StringUtility::strcasecmp( s, "editdistance" ) == 0 ) {
      return SearchTypes::EditDistanceMatch;
   } else {
      return SearchTypes::CloseMatch;
   }
}


const char* 
StringConversion::searchStringMatchingTypeToString( 
   SearchTypes::StringMatching mt )
{
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!

   switch( mt ) {
      case SearchTypes::ExactMatch :
         return "exact" ;
      case SearchTypes::CloseMatch :
         return "close" ;
      case SearchTypes::FullMatch :
         return "full" ;
      case SearchTypes::CloseFullMatch :
         return "closefull" ;
      case SearchTypes::WildcardMatch :
         return "wildcard" ;
      case SearchTypes::AllWordsMatch :
         return "allwords" ;
      case SearchTypes::SoundexMatch :
         return "soundex" ;
      case SearchTypes::EditDistanceMatch :
         return "editdistance" ;
      case SearchTypes::MaxDefinedMatching :
         return "" ;
   }

   // Unreachable code
   return "close";
}


SearchTypes::StringPart
StringConversion::searchStringPartTypeFromString( const char* const s ) {
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!
   if ( StringUtility::strcasecmp( s, "beginning" ) == 0 ) { 
      return SearchTypes::Beginning;
   } else if ( StringUtility::strcasecmp( s, "anywhere" ) == 0 ) { 
      return SearchTypes::Anywhere;
   } else if ( StringUtility::strcasecmp( s, "wildcardpart" ) == 0 ) { 
      return SearchTypes::WildcardPart;
   } else if ( StringUtility::strcasecmp( s, "beginningofword" ) == 0 ) {
      return SearchTypes::BeginningOfWord;
   } else {
      return SearchTypes::Beginning;
   }
}


const char* 
StringConversion::searchStringPartTypeToString( 
   SearchTypes::StringPart sp )
{
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!
   switch( sp ) {
      case SearchTypes::Beginning :
         return "beginning";
      case SearchTypes::Anywhere :
         return "anywhere";
      case SearchTypes::WildcardPart :
         return "wildcardpart";
      case SearchTypes::BeginningOfWord:
         return "beginningofword";
      case SearchTypes::MaxDefinedStringPart :
         return "";
   }

   // Unreachable code
   return "beginning";
}


SearchTypes::SearchSorting
StringConversion::searchSortingTypeFromString( const char* const s ) {
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!
   if ( StringUtility::strcasecmp( s, "no_sort" ) == 0 ) {
      return SearchTypes::NoSort;
   } else if ( StringUtility::strcasecmp( s, "alpha_sort" ) == 0 ) {
      return SearchTypes::AlphaSort;
   } else if ( StringUtility::strcasecmp( s, "confidence_sort" ) == 0 ) {
      return SearchTypes::ConfidenceSort;
   } else if ( StringUtility::strcasecmp( s, 
                                          "bestmatchesonly_sort" ) == 0 ) 
   {
      return SearchTypes::BestMatchesSort;
   } else if ( StringUtility::strcasecmp( s, "distance_sort" ) == 0 ) {
      return SearchTypes::DistanceSort;
   } else if ( StringUtility::strcasecmp( s, "bestdistance_sort" ) == 0 ) {
      return SearchTypes::BestDistanceSort;
   } else {
      return SearchTypes::ConfidenceSort;
   }
}


const char* 
StringConversion::searchSortingTypeToString( 
   SearchTypes::SearchSorting so )
{
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!
   switch ( so ) {
      case SearchTypes::NoSort :
         return "no_sort";
      case SearchTypes::AlphaSort :
         return "alpha_sort";
      case SearchTypes::ConfidenceSort :
         return "confidence_sort";
      case SearchTypes::BestMatchesSort :
         return "bestmatchesonly_sort";
      case SearchTypes::DistanceSort :
         return "distance_sort";
      case SearchTypes::BestDistanceSort :
         return "bestdistance_sort";
      case SearchTypes::MaxDefinedSort :
         return "";
   }

   // Unreachable code
   return "confidence_sort";
}


uint32
StringConversion::searchCategoryTypeFromString( const char* const s ) {
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!
   if ( StringUtility::strcasecmp( s, "street" ) == 0 ) {
      return SEARCH_STREETS;
   } else if ( StringUtility::strcasecmp( s, "pointofinterest" ) == 0 ) {
      return SEARCH_COMPANIES;
   } else if ( StringUtility::strcasecmp( s, "category" ) == 0 ) {
      return SEARCH_CATEGORIES;
   } else if ( StringUtility::strcasecmp( s, "misc" ) == 0 ) {
      return SEARCH_MISC;   
   } else if ( StringUtility::strcasecmp( s, "other") == 0 ) {
      return SEARCH_MISC;   
   } else if ( StringUtility::strcasecmp( s, "person") == 0 ) {
      return SEARCH_PERSONS;
   } else {
      return 0;
   }
}


const char* const 
StringConversion::searchCategoryTypeToString( uint32 category ) {
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!
   if ( category == SEARCH_STREETS ) {
      return "street";
   } else if ( category == SEARCH_COMPANIES ) {
      return "pointofinterest";
   } else if ( category == SEARCH_CATEGORIES ) {
      return "category";
   } else if ( category == SEARCH_MISC ) {
      return "misc";
   } else if ( category == SEARCH_PERSONS ) {
      return "person";
   } else {
      return "other";
   } 
}


uint32 
StringConversion::searchLocationTypeFromString( const char* const s ) {
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!
   if ( StringUtility::strcasecmp( s, "municipal" ) == 0 ) {
      return SEARCH_MUNICIPALS;
   } else if ( StringUtility::strcasecmp( s, "city" ) == 0 ) {
      return SEARCH_BUILT_UP_AREAS;
   } else if ( StringUtility::strcasecmp( s, "citypart" ) == 0 ) {
      return SEARCH_CITY_PARTS;
   } else if ( StringUtility::strcasecmp( s, "zipcode" ) == 0 ) {
      return SEARCH_ZIP_CODES;
   } else if ( StringUtility::strcasecmp( s, "ziparea" ) == 0 ) {
      return SEARCH_ZIP_AREAS;
   } else if ( StringUtility::strcasecmp( s, "other" ) == 0 ) {
      return SEARCH_BUILT_UP_AREAS;
   } else {
      return 0;
   }
}


const char* const 
StringConversion::searchLocationTypeToString( uint32 locationType ) {
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!
   if ( locationType == SEARCH_MUNICIPALS ) {
      return "municipal";
   } else if ( locationType == SEARCH_BUILT_UP_AREAS ) {
      return "city";
   } else if ( locationType == SEARCH_CITY_PARTS ) {
      return "citypart";
   } else if ( locationType == SEARCH_ZIP_CODES ) {
      return "zipcode";
   } else if ( locationType == SEARCH_ZIP_AREAS ) {
      return "ziparea";
   } else if ( locationType == SEARCH_COUNTRIES ) {
      return "country";
   } else {
      return "other";
   }
}


const StringTableUtility::distanceFormat 
StringConversion::distanceFormatFromString( const char* const s ) {
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!
   if ( StringUtility::strcasecmp( s, "normal" ) == 0 ) {
      return StringTableUtility::NORMAL;
   } else if ( StringUtility::strcasecmp( s, "compact" ) == 0 ) {
      return StringTableUtility::COMPACT;
   } else {
      return StringTableUtility::NORMAL;
   }
}


const char* 
StringConversion::housenumberStartToString( 
   StringTable::stringCode dir )
{
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!
   if ( dir == StringTable::START_DIRECTION_NBR_INCREASING ) {
      return "increasing";
   } else if ( dir == StringTable::START_DIRECTION_NBR_DECREASING ) {
      return "decreasing";
   } else if ( dir == StringTable::START_DIRECTION_ODDEVEN_LEFTODD ) {
      return "leftodd";
   } else if ( dir == StringTable::START_DIRECTION_ODDEVEN_RIGHTODD ) {
      return "rightodd";
   } else {
      return "unknown";
   }
}


const char*
StringConversion::crossingKindToString( ItemTypes::crossingkind_t crossing ) {
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!
   switch (crossing) {
      case ItemTypes::UNDEFINED_CROSSING :
         return "undefined_crossing";
      case ItemTypes::NO_CROSSING :
         return "no_crossing";
      case ItemTypes::CROSSING_3WAYS_T :
         return "crossing_3ways_t";
      case ItemTypes::CROSSING_3WAYS_Y :
         return "crossing_3ways_y";
      case ItemTypes::CROSSING_4WAYS :
         return "crossing_4ways";
      case ItemTypes::CROSSING_5WAYS :
         return "crossing_5ways";
      case ItemTypes::CROSSING_6WAYS :
         return "crossing_6ways";
      case ItemTypes::CROSSING_7WAYS :
         return "crossing_7ways";
      case ItemTypes::CROSSING_8WAYS :
         return "crossing_8ways";
      case ItemTypes::CROSSING_2ROUNDABOUT :
         return "crossing_2roundabout";
      case ItemTypes::CROSSING_3ROUNDABOUT :
         return "crossing_3roundabout";
      case ItemTypes::CROSSING_4ROUNDABOUT :
         return "crossing_4roundabout";
      case ItemTypes::CROSSING_4ROUNDABOUT_ASYMMETRIC :
         return "crossing_4roundabout_asymmetric";
      case ItemTypes::CROSSING_5ROUNDABOUT :
         return "crossing_5roundabout";
      case ItemTypes::CROSSING_6ROUNDABOUT :
         return "crossing_6roundabout";
      case ItemTypes::CROSSING_7ROUNDABOUT :
         return "crossing_7roundabout";
   }
   
   mc2log << warn << "StringConversion::crossingKindToString "
      "unknown crossingkind_t " << int(crossing) << endl;
   return "undefined_crossing";
}


ItemTypes::vehicle_t 
StringConversion::vehicleTypeFromString( const char* const s ) {
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!
   if ( StringUtility::strcmp( s, "passengercar" ) == 0 ) {
      return ItemTypes::passengerCar;
   } else if ( StringUtility::strcmp( s, "pedestrian" ) == 0 ) {
      return ItemTypes::pedestrian;
   } else if ( StringUtility::strcmp( s, "publictransportation" ) == 0 ) {
      return ItemTypes::publicTransportation;
   } else if ( StringUtility::strcmp( s, "emergencyvehicle" ) == 0 ) {
      return ItemTypes::emergencyVehicle;
   } else if ( StringUtility::strcmp( s, "taxi" ) == 0 ) {
      return ItemTypes::taxi;
   } else if ( StringUtility::strcmp( s, "publicbus" ) == 0 ) {
      return ItemTypes::publicBus;
   } else if ( StringUtility::strcmp( s, "deliverytruck" ) == 0 ) {
      return ItemTypes::deliveryTruck;
   } else if ( StringUtility::strcmp( s, "transporttruck" ) == 0 ) {
      return ItemTypes::transportTruck;
   } else if ( StringUtility::strcmp( s, "highoccupancyvehicle" ) == 0 ) {
      return ItemTypes::highOccupancyVehicle;
   } else if ( StringUtility::strcmp( s, "bicycle" ) == 0 ) {
      return ItemTypes::bicycle;
   } else if ( StringUtility::strcmp( s, "passcarclosedseason" ) == 0 ) {
      return ItemTypes::passCarClosedSeason;
   } else if ( StringUtility::strcmp( s, "motorcycle" ) == 0 ) {
      return ItemTypes::motorcycle;
   } else if ( StringUtility::strcmp( s, "moped" ) == 0 ) {
      return ItemTypes::moped;
   } else if ( StringUtility::strcmp( s, "privatebus" ) == 0 ) {
      return ItemTypes::privateBus;
   } else if ( StringUtility::strcmp( s, "militaryvehicle" ) == 0 ) {
      return ItemTypes::militaryVehicle;
   } else if ( StringUtility::strcmp( s, "residentialvehicle" ) == 0 ) {
      return ItemTypes::residentialVehicle;
   } else if ( StringUtility::strcmp( s, "carwithtrailer" ) == 0 ) {
      return ItemTypes::carWithTrailer;
   } else if ( StringUtility::strcmp( s, "farmvehicle" ) == 0 ) {
      return ItemTypes::farmVehicle;
   } else if ( StringUtility::strcmp( s, "privatevehicle" ) == 0 ) {
      return ItemTypes::privateVehicle;
   } else if ( StringUtility::strcmp( s, "waterpollutingload" ) == 0 ) {
      return ItemTypes::waterPollutingLoad;
   } else if ( StringUtility::strcmp( s, "explosiveload" ) == 0 ) {
      return ItemTypes::explosiveLoad;
   } else if ( StringUtility::strcmp( s, "dangerousload" ) == 0 ) {
      return ItemTypes::dangerousLoad;
   } else {
      return ItemTypes::passengerCar;
   }
}


const char* const 
StringConversion::vehicleTypeToString( ItemTypes::vehicle_t vehilce ) {
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!
   switch( vehilce ) {
      case ItemTypes::passengerCar :
         return "passengercar";
      case ItemTypes::pedestrian :
         return "pedestrian";
      case ItemTypes::publicTransportation :
         return "publictransportation";
      case ItemTypes::emergencyVehicle :
         return "emergencyvehicle";
      case ItemTypes::taxi :
         return "taxi";
      case ItemTypes::publicBus :
         return "publicbus";
      case ItemTypes::deliveryTruck :
         return "deliverytruck";
      case ItemTypes::transportTruck  :
         return "transporttruck";
      case ItemTypes::highOccupancyVehicle :
         return "highoccupancyvehicle";
      case ItemTypes::bicycle :
         return "bicycle";
      case ItemTypes::passCarClosedSeason :
         return "passcarclosedseason";
      case ItemTypes::motorcycle :
         return "motorcycle";
      case ItemTypes::moped :
         return "moped";
      case ItemTypes::privateBus :
         return "privatebus";
      case ItemTypes::militaryVehicle :
         return "militaryvehicle";
      case ItemTypes::residentialVehicle :
         return "residentialvehicle";
      case ItemTypes::carWithTrailer  :
         return "carwithtrailer";
      case ItemTypes::farmVehicle :
         return "farmvehicle";
      case ItemTypes::privateVehicle :
         return "privatevehicle";
      case ItemTypes::waterPollutingLoad :
         return "waterpollutingload";
      case ItemTypes::explosiveLoad  :
         return "explosiveload";
      case ItemTypes::dangerousLoad :
         return "dangerousload";
      case ItemTypes::avoidTollRoad :
         return "avoidTollRoad";
      case ItemTypes::avoidHighway :
         return "avoidHighway";
   }
   
   // We never gets here
   return "passengercar";
}


const char* const 
StringConversion::landmarkTypeToString( ItemTypes::landmark_t lm ) {
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!
   switch( lm ) {
      case ItemTypes::builtUpAreaLM:
         return "builtUpArea";
      case ItemTypes::railwayLM:
         return "railway";
      case ItemTypes::areaLM:
         return "area";
      case ItemTypes::poiLM:
         return "poi";
      case ItemTypes::signPostLM:
         return "signPost";
      case ItemTypes::countryLM:
         return "country";
      case ItemTypes::countryAndBuiltUpAreaLM:
         return "countryAndBuiltUpArea";
      case ItemTypes::passedStreetLM:
         return "passedStreet";
      case ItemTypes::accidentLM:
         return "accident";
      case ItemTypes::roadWorkLM:
         return "roadwork";
      case ItemTypes::cameraLM:
         return "camera";
      case ItemTypes::speedTrapLM:
         return "speedTrap";
      case ItemTypes::policeLM:
         return "police";
      case ItemTypes::weatherLM:
         return "weather";
      case ItemTypes::trafficOtherLM:
         return "trafficGen";
      case ItemTypes::blackSpotLM:
         return "blackspot";
      case ItemTypes::userDefinedCameraLM:
         return "userDefinedCamera";
   }

   // Unreachable code
   return "";
}


const char* const 
StringConversion::landmarkLocationTypeToString( 
   ItemTypes::landmarklocation_t lm )
{
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!
   switch( lm ) {
      case ItemTypes::after:
         return "after";
      case ItemTypes::before:
         return "before";
      case ItemTypes::in:
         return "in";
      case ItemTypes::at:
         return "at";
      case ItemTypes::pass:
         return "pass";
      case ItemTypes::into:
         return "into";
      case ItemTypes::arrive:
         return "arrive";
      case ItemTypes::undefinedlocation:
         return "undefinedlocation";
   }

   // Unreachable code
   return "";
}


const char* const 
StringConversion::sideTypeToString( SearchTypes::side_t side ) {
   // PLEASE NOTE THAT THESE STRINGS ARE USE IN THE EXTERNAL API
   // WHEN CHANGING HERE CHANGE THERE TOO!!
   switch( side ) {
      case SearchTypes::left_side :
         return "left_side";
      case SearchTypes::right_side :
         return "right_side";
      case SearchTypes::unknown_side :
         return "unknown_side";
      case SearchTypes::undefined_side :
         return "undefined_side";
      case SearchTypes::side_does_not_matter :
         return "side_does_not_matter";
      case SearchTypes::left_side_exit :
         return "left_side_exit";
      case SearchTypes::right_side_exit :
         return "right_side_exit";
      case SearchTypes::max_defined_side:
         return "max_defined_side";
   }

   // Unreachable code
   return "";
}
