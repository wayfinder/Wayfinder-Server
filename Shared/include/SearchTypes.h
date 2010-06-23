/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHTYPES_H
#define SEARCHTYPES_H

#include "config.h"

#define REGION_SHOULD_NOT_BE_DISPLAYED(id) ((id)&0x80000000)

class SearchTypes 
{
  public:
   /**
    *  Sources of information that can be used when searching.
    *  Bitmask -> only powers of 2 allowed.
    *  Currently 8 bits allowed. This will probably be extended to 16 bits.
    *
    *  Dont change the numbering! unless you also change all the
    *  users in the user database!
    */
   enum DBsource {NONE         = 0,
                  UNKNOWN      = 1, // any free information will
                  // also be placed here under UNKNOWN
                  NAV_TECH     = 2,
                  YELLOW_PAGES = 4,
                  PAR          = 8,
                  TELE_ATLAS   = 32,
                  MAX_DEFINED  = 128, // last one
                  TEMP_ERROR   = 255 // temporary error indicator, to be removed...
   };

   /// The databases used by default.
   static const uint8 DefaultDB;

   // All of the above.
   static const uint8 AllDB;

   // points
   static const int matchedStringIsExactPoints;
   static const int matchedStringIsClosePoints;
   static const int matchedStringWithNumbers;

   /**
    * The different types of matching.
    */
   enum StringMatching {ExactMatch           = 0,
                        CloseMatch           = 1,
                        FullMatch            = 2,
                        CloseFullMatch       = 3,
                        WildcardMatch        = 4,
                        AllWordsMatch        = 5,
                        SoundexMatch         = 6,
                        EditDistanceMatch    = 7,
                        MaxDefinedMatching   = 8 // last one, not used
   };

   /**
    * The different parts of a string.
    */
   enum StringPart {Beginning            = 0,
                    Anywhere             = 1,
                    WildcardPart         = 2,
                    BeginningOfWord      = 3,
                    MaxDefinedStringPart = 4
   };

   /**
    * The different types of sorting.
    */
   enum SearchSorting {NoSort           = 0,
                       AlphaSort        = 1,
                       ConfidenceSort   = 2,
                       BestMatchesSort  = 3,
                       DistanceSort     = 4,
                       BestDistanceSort = 7,
                       MaxDefinedSort   = 8
   };

   /**
    * Used to indicate the side on a street.
    */
   enum side_t {
      left_side            = 0, // car is on left side
      right_side           = 1, // car is on right side
      unknown_side         = 2, // car is on unknown side
      undefined_side       = 3, // car is on undefined side
      side_does_not_matter = 4, // both sides are equally easy to start
      left_side_exit       = 5, // car is driving out of an exit on the left side
      right_side_exit      = 6, // car is driving out of an exit on the right side
      max_defined_side     = 7 // error indicator
   };

   /// A bit, that if set flags that the mapID is an external database.
   static const uint32 ExternalDatabaseMapIDFlag;

   /**
    * Get an abbreviation string for a StringMatching type.
    *
    * @param matching The StringMatching type to get string for.
    * @return A string for the StringMatching type.
    */
   static inline const char* getMatchingString( StringMatching matching );

   /**
    * Get an abbreviation string for a StringPart type.
    *
    * @param matching The StringPart type to get string for.
    * @return A string for the StringPart type.
    */
   static inline const char* getPartString( StringPart part );

   /**
    * Get an abbreviation string for a SearchSorting type.
    *
    * @param matching The SearchSorting type to get string for.
    * @return A string for the SearchSorting type.
    */
   static inline const char* getSortingString( SearchSorting sorting );
};

// Searchrequest
#define SEARCH_STREETS             0x1
#define SEARCH_COMPANIES           0x2
#define SEARCH_CATEGORIES          0x4
#define SEARCH_MISC                0x8

/*
 *   @name Masks for different types of locations.
 */
/* 
 *   Municipal.
 */
#define SEARCH_MUNICIPALS 0x20

/* 
 *   Built Up Area.
 */
#define SEARCH_BUILT_UP_AREAS 0x40

/* 
 *   City part.
 */
#define  SEARCH_CITY_PARTS 0x80
/// ZipCodes
#define  SEARCH_ZIP_CODES  0x100
/// Postal Areas
#define  SEARCH_ZIP_AREAS 0x200
/// Person
#define  SEARCH_PERSONS   0x400

/**
 * Countries. Not Actually part of any search, its here for
 * completing the search matches with country area.
 */
#define  SEARCH_COUNTRIES 0x800

const uint32 SEARCH_UNDEFINED_TYPES_BITS =
MAX_UINT16 -
SEARCH_STREETS - SEARCH_COMPANIES - SEARCH_CATEGORIES - SEARCH_MISC -
SEARCH_MUNICIPALS - SEARCH_BUILT_UP_AREAS - SEARCH_CITY_PARTS;

const uint32 SEARCH_REGION_TYPES = SEARCH_MUNICIPALS+
SEARCH_BUILT_UP_AREAS+
SEARCH_CITY_PARTS;

const uint32 SEARCH_ALL_REGION_TYPES = SEARCH_MUNICIPALS |
                                       SEARCH_BUILT_UP_AREAS |
                                       SEARCH_CITY_PARTS |
                                       SEARCH_ZIP_CODES |
                                       SEARCH_ZIP_AREAS |
                                       SEARCH_COUNTRIES;

// Priority
#define SEARCH_REPLY_PRIO   DEFAULT_PACKET_PRIO
// == 7 if it doesn't work in clients
#define SEARCH_REQUEST_PRIO DEFAULT_PACKET_PRIO
// == 7 if it doesn't work in clients

// Global cut off Settings
// XXX should be 2000.
const int32 MaxSortingLimit = 1000;
//#define MAX_SORTING_LIMIT        20000
/* Sorting of matches will not be performed if there are more than
   MAX_SORTING_LIMIT matches. This is to prevent bad behavior in the
   case of some unlucky inprecise search requests.

   If MAX_SORTING_LIMIT = MAX_EXTRACT_NBR_MATCHES, sorting will not be
   performed in the worst case, since we use two extractMatches(...)
   -> MAX_EXTRACT_NBR_MATCHES+1 is the total maximum of matches.
*/
const uint32 MaxExtractNbrMatches = 35000;
//#define MAX_EXTRACT_NBR_MATCHES  20000
// XXX should be 2000.

/* Extraction of matches will stop once the number of extracted
   matches reaches MAX_EXTRACT_NBR_MATCHES.
   The user should enter a better search request. */

/// The number of bytes that must be available in VSRP when adding new match
#define VP_MIN_FREE 3000
/// The number of bytes that must be available when adding part of a match.
#define VP_SOME_LESS_THAN_MIN_FREE 2000

/**
 *  When the VanillaSearchReplyPacket is more full than this no more matches
 *  will be added
 */
#define PACKET_FULL_LIMIT int(getBufSize()-VP_MIN_FREE)


// ========================================================================
//                                      Implementation of inlined methods =


inline const char* 
SearchTypes::getMatchingString( StringMatching matching ) {
   switch ( matching ) {
      case SearchTypes::ExactMatch:
         return "EM";
      case SearchTypes::CloseMatch:
         return "CM";
      case SearchTypes::FullMatch:
         return "FM";
      case SearchTypes::CloseFullMatch:
         return "CFM";
      case SearchTypes::WildcardMatch:
         return "WM";
      case SearchTypes::AllWordsMatch:
         return "AM";
      case SearchTypes::SoundexMatch:
         return "SM";
      case SearchTypes::EditDistanceMatch:
         return "EDM";
      case SearchTypes::MaxDefinedMatching:
         return "";      
   }
   return "WTF";
}


inline const char* 
SearchTypes::getPartString( StringPart part ) {
   switch ( part ) {
      case SearchTypes::Beginning:
         return "BP";
      case SearchTypes::BeginningOfWord:
         return "BWP";
      case SearchTypes::Anywhere:
         return "AP";
      case SearchTypes::WildcardPart:
         return "WP";
      case SearchTypes::MaxDefinedStringPart:
         return "";
   }
   return "WTF";
}


inline const char* 
SearchTypes::getSortingString( SearchSorting sorting ) {
   switch ( sorting ) {
      case SearchTypes::NoSort:
         return "NS";
      case SearchTypes::AlphaSort:
         return "AS";
      case SearchTypes::ConfidenceSort:
         return "CS";
      case SearchTypes::BestMatchesSort:
         return "BS";
      case SearchTypes::DistanceSort:
         return "DS";
      case SearchTypes::BestDistanceSort:
         return "BDS";
      case SearchTypes::MaxDefinedSort:
         return "";
   }
   return "WTF";
}

#endif
