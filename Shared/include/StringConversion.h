/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STRINGCONVERSION_H
#define STRINGCONVERSION_H

#include "config.h"
#include "SearchTypes.h"
#include "StringUtility.h"
#include "ItemTypes.h"
#include "StringTableUtility.h"

/**
 * Class for enum to string and vise versa. 
 *
 */
class StringConversion {
   public:
      /**
       * Converts a string to StringMatchingT, 
       * default is SEARCH_CLOSE_MATCH.
       * @param s The string to get the StringMatchingT from.
       * @return A StringMatchingT matching the string s.
       */
      static SearchTypes::StringMatching
         searchStringMatchingTypeFromString( const char* const s );


      /**
       * Converts StringMatchingT to a string.
       *
       * @param mt The StringMatchingT to turn into a string.
       * @return A string matching the StringMatchingT mt.
       */
      static const char* searchStringMatchingTypeToString( 
         SearchTypes::StringMatching mt );


      /**
       * Converts a string to StringPartT, 
       * default is SEARCH_BEGINNING_OF_STRING.
       * @param s The string to get the StringPartT from.
       * @return A StringPartT matching the string s.
       */
      static SearchTypes::StringPart searchStringPartTypeFromString( 
         const char* const s );


      /**
       * Converts StringPartT to a string.
       *
       * @param sp The StringPartT to turn into a string.
       * @return A string matching the StringPartT sp.
       */
      static const char* searchStringPartTypeToString( 
         SearchTypes::StringPart sp );


      /**
       * Converts a string to SortingT, default is SEARCH_CONFIDENCE_SORT.
       * @param s The string to get the SortingT from.
       * @return A string SortingT matching the string s.
       */
      static SearchTypes::SearchSorting
         searchSortingTypeFromString( const char* const s );


      /**
       * Converts a SortingT to a string.
       *
       * @param so The SortingT to turn into a string.
       * @return A string matching the SortingT so.
       */
      static const char* searchSortingTypeToString( 
         SearchTypes::SearchSorting so );


      /**
       * Converts a string to category types, default is 0.
       * @param s The string to get the category type from.
       * @return A categoty type matching the string s.
       */
      static uint32 searchCategoryTypeFromString( const char* const s );


      /**
       * Converts a category type to a string, default is "".
       * @param category One of SEARCH_STREETS, SEARCH_COMPANIES or 
       *                 SEARCH_CATEGORIES.
       * @return A string representing the category type.
       */
      static const char* const searchCategoryTypeToString( 
         uint32 category );


      /**
       * Converts a string to location types, default is 0.
       * @param s The string to get the location types from.
       * @return A location type matching the string s.
       */
      static uint32 searchLocationTypeFromString( const char* const s );


      /**
       * Converts a location type to a string, default is "".
       * @param category One of SEARCH_MUNICIPALS, SEARCH_BUILT_UP_AREAS or
       *                 SEARCH_CITY_PARTS.
       * @return A string representing the location type.
       */
      static const char* const searchLocationTypeToString( 
         uint32 locationType );
                                                           

      /**
       * Converts a string to distanceFormat, default is NORMAL.
       * @param s The string to get the distanceFormat from.
       * @return A distanceFormat.
       */
      static const StringTableUtility::distanceFormat distanceFormatFromString( 
         const char* const s );


      /**
       * Returns a string representing the housenumberstartdirection
       * stringcode in parameter.
       * 
       * @param dir The stringCode for the dir.
       * @return String representing the turn if dir isn't a turn
       *         stringCode then "" is returned.
       */
      static const char* housenumberStartToString( 
         StringTable::stringCode dir );


      /**
       * Returns a string representing the crossingkind in parameter.
       * 
       * @param crossing The crossingkind_t for the turn.
       * @return String representing the crossingkind, if crossing isn't a
       *         crossingkind_t then undefined crossing is returned.
       */
      static const char* crossingKindToString( 
         ItemTypes::crossingkind_t crossing );


      /**
       * Converts a string to vehicle type, default is passenger car.
       * @param s The string to get the vehicle type from.
       * @return A vehicle type matching the string s.
       */
      static ItemTypes::vehicle_t vehicleTypeFromString( 
         const char* const s );


      /**
       * Converts a vehicle type to a string.
       * @param vehicle The vehicle type to get string for.
       * @return A string representing the vehicle type.
       */
      static const char* const vehicleTypeToString( 
         ItemTypes::vehicle_t vehilce );


      /**
       * Converts a landmark_t to a string.
       * @param lm The landmark type to get string for.
       * @return A string representing the landmark type.
       */
      static const char* const landmarkTypeToString( 
         ItemTypes::landmark_t lm );


      /**
       * Converts a landmarklocation_t to a string.
       * @param lm The landmarklocation type to get string for.
       * @return A string representing the landmarklocation type.
       */
      static const char* const landmarkLocationTypeToString( 
         ItemTypes::landmarklocation_t lm );


      /**
       * Converts a side_t to a string.
       * @param side The side type to get string for.
       * @return A string representing the side type.
       */
      static const char* const sideTypeToString( 
         SearchTypes::side_t side );


   private:
      StringConversion();
};


#endif // STRINGCONVERSION_H

