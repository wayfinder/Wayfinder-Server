/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
todo:

implement fuzzysearch
remove some mapID.
use or remove zipcode and cityname

 */


#ifndef SEARCHPACKET_H
#define SEARCHPACKET_H

#include "config.h"
#include "Types.h"
#include "Packet.h"
#include "SearchParams.h"
#include <stdio.h>
#include <typeinfo>
#include "SearchTypes.h"
#include "SearchReplyPacket.h"
#include "SearchRequestPacket.h"
#include "LangTypes.h"
#include <vector>

class VanillaMatch; // forward
class MatchLink; // forward
class MC2BoundingBox;

/**
 *    Called by SearchProcessor to decode vanilla search request packets.
 *    Used to create vanilla search request packets.
 *
 *    Vanilla search is a quick but somewhat restrictive kind of search.
 *    Use it if:
 *    <ul>
 *       <li>You want to find a street and do not care about the street 
 *           number.
 *       <li>Confidence sort is not required.
 *    </ul>
 *
 */
class VanillaSearchRequestPacket : public OldSearchRequestPacket {
   public:
      /**
       *    Default constructor
       */
      VanillaSearchRequestPacket(); 

      /**
       *    Constructor
       *    @param packetID The packetID of the packet.
       *    @param reqID    The requestID of the packet.
       */
      VanillaSearchRequestPacket( uint16 packetID,
                                  uint16 reqID );

      /**
       *    Destructor.
       */
      virtual ~VanillaSearchRequestPacket() {};
      
      /**
       * Encodes the vanilla serch request parameters into the packet
       * @param numMapID the number of mapIDs
       * @param mapID a vector of mapIDs
       * @param zipCode first part of the requested zipcode or "" or null
       * @param city first part of the city name or "" or null
       * @param searchString the string to find matches for
       * @param nbrHits the minimum number of hits that should be returned.
       *                0 is the normal value.
       *                If more than zero, the search restrictions will
       *                be reduced until the requested number of hits has
       *                been produced. That is, if (nbrHits > 0) the
       *                search request might be handled in a fuzzy way.
       * @param matchType the strictness of the string matching
       * @param stringPart the part of the string to match
       * @param sortingType the kind of sorting requested.
       *                    Valid values for vanillasearchrequest are
       *                    SEARCH_NO_SORT and SEARCH_ALPHA_SORT
       * @param categoryType a mask of object types. A combination of
       *                     streets, companies, business categories, ...
       * @param requestedLangauge The requested language.
       * @param dbMask The mask of available databases.
       * @param regionsInMatches The search region types or:ed together 
       *                         that should be in the reply matches.
       * @param topRegion The top region to perform the search in.
       *                  Only applies when making an overviewsearch in 
       *                  SearchRequest.
       */
      void encodeRequest( uint32  numMapID,
                          const uint32* mapID,
                          const char*   zipCode,
                          uint32 nbrLocations,
                          const char** locations,
                          uint32 locationType,
                          uint32 nbrCategories,
                          const uint32* categories,
                          uint32 nbrMasks,
                          const uint32* masks,
                          const uint32* maskItemIDs,
                          const char** maskNames,
                          uint8 dbMask,
                          const char*   searchString,                    
                          uint8   nbrHits,
                          SearchTypes::StringMatching matchType,
                          SearchTypes::StringPart stringPart,
                          SearchTypes::SearchSorting sortingType,
                          uint16 categoryType,
                          uint16 requestedLanguage,
                          uint32 regionsInMatches,
                          StringTable::countryCode topRegion );

      /**
       * Decodes a vanillasearchrequest and inits the parameters.
       * @param zipCode first part of the zipcode
       * @param city first part of the city name
       * @param searchString the search string
       * @param nbrHits the minimum number of requested hits
       * @param matchType the kind of string matching
       * @param stringPart the part of the strings to match
       * @param sortingType the kind of sorting requested
       * @param categoryType object type mask.
       * @param requestedLangauge The requested language.
       * @param dbMask The mask of available databases.
       * @param regionsInMatches The search region types or:ed together 
       *                         that should be in the reply matches.
       * @param topRegion The top region to perform the search in.
       *                  Only applies when making an overviewsearch in 
       *                  SearchRequest.
       */
      void decodeRequest( char*& zipCode,
                          uint32& nbrLocations,
                          char**& locations,
                          uint32& locationType,
                          uint32& nbrCategories,
                          uint32*& categories,
                          uint32& nbrMasks,
                          uint32*& masks,
                          uint32*& maskItemIDs,
                          char**& maskNames,
                          uint8& dbMask,
                          char*& searchString,
                          uint8& nbrHits,
                          SearchTypes::StringMatching& matchType,
                          SearchTypes::StringPart& stringPart,
                          SearchTypes::SearchSorting& sortingType,
                          uint16& categoryType,
                          uint16& requestedLanguage,
                          uint32& regionsInMatches,
                          StringTable::countryCode& topRegion ) const;

};

/**
 *    SearchSelectionReportPacket. Objects of this class sill/should
 *    be used to report which matches the user selected to be able to
 *    give these matches higher priority in future searches.
 *
 */
class SearchSelectionReportPacket : public RequestPacket {
   public:
      /**
       *    Default constructor.
       */
      SearchSelectionReportPacket();

      /** 
       *    Destructor.
       */
      virtual ~SearchSelectionReportPacket() {};

      /** 
       *    Get the number of selections in the packet.
       *    @return The number of selections in the packet
       */
      uint32 getNbrSelections();

      /** 
       *    Add a selection to the packet.
       *    @param mapID   The mapID of the selection.
       *    @param itemID  The itemID of the selection.
       */
      void addSelection(uint32 mapID, uint32 itemID);

      /** 
       *    Get a selection from the packet.
       *    @param index  The index of the selection.
       *    @param mapID  The mapID (outparam).
       *    @param itemID The itemID (outparam).
       */
      void getSelection(uint32 index, uint32 &mapID, uint32 &itemID);
};

/**
 *    Use the usersearchrequest if:
 *    <ul>
 *       <li>There might be a street number in the search string.</li>
 *       <li>You require some kind of confidence sorting.</li>
 *       <li>If you don't know / are unsure of what to use.</li>
 *    </ul>
 *
 *    Called by SearchProcessor to decode search request packets.
 *
 */
class UserSearchRequestPacket : public OldSearchRequestPacket {
public:
   typedef uint16 CategoryID;
   typedef set<CategoryID> Categories;
      /**
       *    Default Constructor
       */
      UserSearchRequestPacket();

      /**
       *    Constructor that sets packet- and requestID.
       *    @param packetID The packetID of this packet, used in the
       *                    request.
       *    @param reqID    The request id of the request that this packet
       *                    belongs to.
       */
      UserSearchRequestPacket( uint16 packetID,
                               uint16 reqID );

      /** 
       *    Destructor (empty).
       */
      virtual ~UserSearchRequestPacket() {};
      
      /**
       *    Encodes the user serch request parameters into the packet
       *    The parameters are similar to those of a vanilla search 
       *    request except the ones specified here.
       *    
       *    Use some kind of best matches only sort if it is unlikely
       *    that the user is interested in matches far down in the 
       *    match list. An example of this is an SMS client, capable 
       *    of listing about 7 matches per SMS. By only requesting the 
       *    first 28 hits to be sorted, the search module will be quick 
       *    even if the user enters a stupid request, such as "m", 
       *    matching 1000s of objects. Response time will be quicker, 
       *    and the end user will probably not notice any difference.
       *    
       *    @param sortingType   The kind of sorting requested. All types 
       *                         of sorting are valid.
       *    @param nbrSortedHits This specifies the number of matches to 
       *                         be sorted if the sortingType is such 
       *                         that all matches are not required to be 
       *                         sorted. For example, 
       *                         SEARCH_BEST_MATCHES_ONLY_SORT. The 
       *                         nbrSortedHits best matches are listed 
       *                         first, and then the rest are left unsorted.
       *    @param nbrLocations  The number of location string in locations.
       *    @param locations     A vector with strings describing locations.
       *    @param locationType  The type of locations to search for.
       *    @param nbrMasks      The number of masks in masks.
       *    @param masks         The vector with the masks limiting the 
       *                         search in the map.
       *    @param maskItemIDs   The item ids of the locations.
       *    @param maskNames     The names describing the masks.
       *    @param editDistanceCutoff The cutoff to use when
       *                              performing edit distance search.
       *                              If set to 0, the default is used.
       * @param regionsInMatches The search region types or:ed together 
       *                         that should be in the reply matches.
       * @param topRegion The top region to perform the search in.
       *                  Only applies when making an overviewsearch in 
       *                  SearchRequest.
       * @param categoryItem Name
       * @param bboxes
       */
      void encodeRequest( uint32  numMapID,
                          const uint32* mapID,
                          const char* zipCode,
                          uint32 nbrLocations,
                          const const_char_p* locations,
                          uint32 locationType,
                          uint32 nbrCategories,
                          const uint32* categories,
                          uint32 nbrMasks,
                          const uint32* masks,
                          const uint32* maskItemIDs,
                          const const_char_p* maskNames,
                          uint8 dbMask,
                          const char* searchString,
                          uint8 nbrHits,
                          SearchTypes::StringMatching matchType,
                          SearchTypes::StringPart stringPart,
                          SearchTypes::SearchSorting sortingType,
                          uint16 categoryType,
                          LangTypes::language_t requestedLanguage,
                          uint8  nbrSortedHits,
                          uint16 editDistanceCutoff,
                          uint32 regionsInMatches,
                          const Categories& realCategories,
                          StringTable::countryCode topRegion,
                          const vector<MC2BoundingBox>& bboxes =
                          vector<MC2BoundingBox>());

      /**
       *    Decodes a usersearchrequest and inits the parameters.
       *    The parameters are similar to the vanillaserchrequestpacket 
       *    except the one specified here.
       *    @param nbrSortedHits The minimum number of sorted hits.
       *    @param regionsInMatches The search region types or:ed together 
       *                            that should be in the reply matches.
       * @param topRegion The top region to perform the search in.
       *                  Only applies when making an overviewsearch in 
       *                  SearchRequest.
       */
      void decodeRequest( char*& zipCode,
                          uint32& nbrLocations,
                          char**& locations,
                          uint32& locationType,
                          uint32& nbrCategories,
                          uint32*& categories,
                          uint32& nbrMasks,
                          uint32*& masks,
                          uint32*& maskItemIDs,
                          char**& maskNames,
                          uint8& dbMask,
                          char*& searchString,
                          uint8 &nbrHits,
                          SearchTypes::StringMatching& matchType,
                          SearchTypes::StringPart& stringPart,
                          SearchTypes::SearchSorting& sortingType,
                          uint16 &categoryType,
                          uint16 &requestedLanguage,
                          uint8 &nbrSortedHits,
                          uint16& editDistanceCutoff,
                          uint32& regionsInMatches,
                          Categories& realCategories,
                          StringTable::countryCode& topRegion,
                          vector<MC2BoundingBox>& bboxes) const;


      /**
       *    Get the serarch string of this packet. The pointer that is
       *    returned must not be deleted and will be invalid when this
       *    packet is deleted.
       *    @return  The search string in this packet.
       */
      const char* getSearchString() const;
};


/**
 *    Expands a category in the normal searchmodule way.
 *
 */
class ExpandCategory2SearchRequestPacket : public OldSearchRequestPacket {
   public:
   /**
    * Default Constructor
    */
   ExpandCategory2SearchRequestPacket();

   /**
    *    Constructor
    *    @param packetID The packetID of the request
    *    @param reqID    The request id of the request
    */
   ExpandCategory2SearchRequestPacket( uint16 packetID, uint16 reqID );

   /** 
    *    Destructor (empty).
    */
   virtual ~ExpandCategory2SearchRequestPacket() {};
   
   /**
    * @see UserSearchRequestPacket for a description of
    *      some of the params to encode and decode.
    * @param searchString Is ignored.
    * @param nbrCategories Is assumed to be 1.
    * @param regionsInMatches The search region types or:ed together 
    *                         that should be in the reply matches.
    * @param topRegion The top region to perform the search in.
    *                  Only applies when making an overviewsearch in 
    *                  SearchRequest.
    */
   void encodeRequest( uint32  numMapID,
                       const uint32* mapID,
                       const char* zipCode,
                       uint32 nbrLocations,
                       const char** locations,
                       uint32 locationType,
                       uint32 nbrCategories,
                       const uint32* categories,
                       uint32 nbrMasks,
                       const uint32* masks,
                       const uint32* maskItemIDs,
                       const char** maskNames,
                       uint8 dbMask,
                       const char* searchString,
                       uint8 nbrHits,
                       SearchTypes::StringMatching matchType,
                       SearchTypes::StringPart stringPart,
                       SearchTypes::SearchSorting sortingType,
                       uint16 categoryType,
                       uint16 requestedLanguage,
                       uint8  nbrSortedHits,
                       uint32 regionsInMatches,
                       StringTable::countryCode topRegion );


   /**
    *    @see encodeRequest.
    *    @param topRegion The top region to perform the search in.
    *                     Only applies when making an overviewsearch in 
    *                     SearchRequest.
    */
   void decodeRequest( char*& zipCode,
                       uint32& nbrLocations,
                       char**& locations,
                       uint32& locationType,
                       uint32& nbrCategories,
                       uint32*& categories,
                       uint32& nbrMasks,
                       uint32*& masks,
                       uint32*& maskItemIDs,
                       char**& maskNames,
                       uint8& dbMask,
                       char*& searchString,
                       uint8 &nbrHits,
                       SearchTypes::StringMatching& matchType,
                       SearchTypes::StringPart& stringPart,
                       SearchTypes::SearchSorting& sortingType,
                       uint16 &categoryType,
                       uint16 &requestedLanguage,
                       uint8 &nbrSortedHits,
                       uint32& regionsInMatches,
                       StringTable::countryCode& topRegion ) const;
};

#endif

