/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHABLE_SEARCHUNIT_H
#define SEARCHABLE_SEARCHUNIT_H

#include "config.h"

#include "SearchUnit.h"
#include "MC2String.h"
#include "LangTypes.h"

#include <vector>

class IDPair_t;
class UserSearchParameters;

class SearchMapItem;
class SearchMapPOIInfo;

class SearchableMultiStringSearch;
class SearchResult;
class SearchUnitSearchResult;
class VanillaRegionMatch;
class VanillaCompanyMatch;
class OverviewMatch;

class SearchMatch;
class SearchResultElement;
class SearchQuery;

/**
 *   This is the class that handles all the searching.
 *   @see SearchUnit.
 */
class SearchableSearchUnit : public SearchUnit {
public:
   /**
    *   Creates new, empty SearchUnit. Must be initialized
    *   using load.
    */
   SearchableSearchUnit();

   /**
    *   Receives a SearchUnit from the MapModule.
    */
   bool loadFromMapModule(uint32 mapID);

   /**
    *    Translates the supplied overview items into low level and
    *    expands the items if there are more than one underview item
    *    associated with the overview item. (E.g. municipals that are
    *    in more than one map).
    *    @param expandedIDs vector containing index in in-vector in first
    *                       and expanded item in second. Can be more than
    *                       one result for one incoming item.
    *    @param idsToExpand The id:s to lookup.
    *    @return expandedIDs.size().
    */
   int expandIDs(vector<pair<uint32, OverviewMatch*> >& expandedIDs,
                 const vector<pair<uint32, IDPair_t> >&
                 idsToExpand,
                 bool expand) const;

   /**
    *    Performs a search that is similar to the old
    *    UserSearch.
    *    @param result The matches are put here.
    *    @param params The parameters to use when searching.
    *    @return Size of result.
    */
   int search(SearchUnitSearchResult& result,
              const UserSearchParameters& params) const;

   /**
    *    Performs a search similar to the old OverviewSearch.
    *    @param result The matches are put here.
    *    @param params The parameters to use when searching.
    *    @return Size of result.    
    */
   int overviewSearch(SearchUnitSearchResult& result,
                      const UserSearchParameters& params) const;

   /**
    *    Performs a search similar to the old OverviewSearch.
    *    uses the other overview search and then converts the
    *    matches into overview matches.
    */
   int overviewSearch(vector<OverviewMatch*>& result,
                      const UserSearchParameters& params,
                      uint8 nbrRemovedCharacters = 0) const;

   /**
    *    Converts the items in itemIDs into a searchresult.
    *    @param result The items as a SearchUnitSearchResult.
    *    @param params The search parameters.
    *    @param itemIDs Item id:s to convert into matches.
    */
   int proximitySearch(SearchUnitSearchResult& result,
                       const UserSearchParameters& params,
                       const vector<IDPair_t>& itemIDs) const;
   
protected:
   
   /**
    *   Creates a new SearchableMultiStringSearch.
    *   (In load).
    */
   virtual void createMultiStringSearch();
   
private:

   /**
    *   Returns true if the string is ok for more zip-code
    *   searching.
    *   @param str String to check.
    *   @return True if the string is OK for more zip-code searching.
    */
   static bool okForMoreZipCodeSearching( const MC2String& str );
   
   /**
    *   Converts the result in <code>internalResult</code> into
    *   matches suitable for SearchProcessor and packets.
    */
   void convertInternalResultToMatches(
      SearchUnitSearchResult& result,
      const SearchQuery& query,
      const SearchResult& internalResult) const;

   /**
    *   Converts the result to overviewmatches.
    */
   void convertMatchesToOverviewMatches(
      vector<OverviewMatch*>& result,
      const SearchUnitSearchResult& tempRes,
      uint8 nbrRemovedCharacters) const;
   
   /**
    *   Adds regions to the match.
    *   @param match   The match that should be returned to the
    *                  Server.
    *   @param itemIdx The itemIndex of the item of the match.
    *   @param level   For debug, don't use.
    */
   void addRegionsToMatch(SearchMatch* match,
                          uint32 itemIdx,
                          const SearchQuery& query,
                          int level = 0) const;

   /**
    *   Creates a SearchMatch from the supplied SearchResultElement.
    *   @param elem The SearchResultElement.
    *   @param query The search query.
    *   @return A SearchMatch to send to the server.
    */
   SearchMatch* createSearchMatch(const SearchResultElement* elem,
                                  const SearchQuery& query) const;

   /**
    *   Creates a VanillaRegionMatch from the item at index
    *   itemIdx.
    *   @param itemIdx Index in the map for the item.
    *   @return New VanillaRegionMatch corresponding to the
    *           itemIdx.
    */
   VanillaRegionMatch* createRegionMatch(uint32 itemIdx,
                                         const SearchQuery& query) const;

   /**
    *   Returns the name in the item to use.
    *   If the matched name is a synonym name and the item is not
    *   in SEARCH_ALL_REGION_TYPES the synonym name will be replaced
    *   by the best name in the requested language.
    *   @param query The query is needed for the language.
    *   @param curElement The element is needed of course.
    *   @return Name to use.
    */
   inline pair<const char*, LangTypes::language_t>
         getNameToUse(const SearchQuery& query,
                      const SearchResultElement* curElement) const;

   /**
    *   Adds a good synonym name to a poi.
    *   @param poi     The poi to add a synonym name to the name of.
    *   @param query   Query to get requested language from.
    *   @param item    The corresponding item in the map.
    *   @param poiInfo The poi-info for the item.
    *   @return True if the name was changed.
    */
   inline bool addSynonymNameTo( VanillaCompanyMatch* poi,
                                 const SearchQuery& query,
                                 const SearchMapItem* item,
                                 const SearchMapPOIInfo* poiInfo ) const;

   /**
    *   Sorts the matches according to the parameters and removes all
    *   unsorted matches ( number of sorted matches is limited ).
    *   @param result The search result. Will be modified.
    *   @param params The search settings.
    */
   int sortMatchesRemovingUnsorted( SearchUnitSearchResult& result,
                                    const UserSearchParameters& params) const;
   
   /**
    *   Pointer with correct type to the
    *   MultiStringSearch. Do not delete.
    *   The pointer to delete is in superclass.
    */
   SearchableMultiStringSearch* m_multiSearch;
};

#endif
