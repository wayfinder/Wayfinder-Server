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

#include "SearchStruct.h"
#include <set>

class SearchResult;
class SearchQuery;
class SearchMapItem;
class MultiSearchNotice;

/**
 *   Class containing some utility functions that
 *   should be possible to use in more than one
 *   version of the SearchStruct. 
 */
class SearchableStringSearch {
protected:
   /**
    *   Stores the map 
    */
   SearchableStringSearch(const SearchMap2* searchMap);

   /**
    *   Returns true if the item is in one of the regions
    *   that are allowed according to the query.
    *   @param item  The item to check.
    *   @param query The search query.
    *   @param indeces The indeces.
    *   @param addIfEmpty Add if nbr regions = 0.
    *   @param recurseLevel Used internally to keep track
    *                       of when to allow region-less items
    */
   inline bool correctRegion(const SearchMapItem* item,
                             const SearchQuery& query,
                             const set<uint32>& indeces,
                             bool addIfEmpty,
                             int recurseLevel = 0) const;

   /**
    *   Returns true if the item has one of the requested types
    *   in the query.
    *   @param item  The item to check.
    *   @param query The query to get the parameters from.
    */
   inline bool correctType(const SearchMapItem* item,
                           const SearchQuery& query) const;

   /**
    *   Returns true if the item is inside an allowed boundingbox.
    *   
    */
   inline bool correctCoordinate(const SearchMapItem* item,
                                 const SearchQuery& query) const;
   
//private:
   /**
    *   Pointer to the SearchMap. Needed for lots of functions.
    */
   const SearchMap2* m_searchMap;
   
};

/**
 *   MultiStringSearch including functions for searching.
 *   (To avoid cluttering the Modules/ with too many files).
 *   All member data must be possible to save into a DataBuffer
 *   so if you need to add something you must add it to the
 *   MultiStringSearch and WriteableMultiStringSearch.
 */
class SearchableMultiStringSearch : public MultiStringSearch,
                                    public SearchableStringSearch {
public:
   /**
    *   Constructor. Use load after this.
    */
   SearchableMultiStringSearch(const SearchMap2* searchMap);

   /**
    *   Searches and puts the results from the query into 
    *   result.
    *   @param result Where to put the results.
    *   @param query  The query.
    */
   int search(SearchResult& result,
              const SearchQuery& query) const;

   /**
    *   Returns matches where all the words in str
    *   are present.
    *   @param result The result is put here.
    *   @param str    The string to look for.
    */
   int allWordsSearch(SearchResult& result,
                      const SearchQuery& query) const;
private:

   /**
    * Determines if the item should be added based on a set of rules.
    * @param item the item to evaluate.
    * @param query search parameters
    * @return true if the item should be added
    */
   bool shouldAddItem( const SearchMapItem* item, const SearchQuery& query ) const;

   /**
    *   Searches for part number partnum in the
    *   supplied query.
    */
   int allWordsSearch(SearchResult& result,
                      const SearchQuery& query,
                      int partNum) const;
   
   /**
    *   Internal function for searching
    *   ADDER.addResult will be called everytime
    *   something should be added. This is so that
    *   the same foundation of search functionality
    *   can be used both in ordinary search and allWords.
    */
   template<class ADDER> int internalSearch(ADDER& adder,
                                            const SearchQuery& query,
                                            const char* str) const;


   template <typename ADDER>
   void doCategorySearch( ADDER& adder,
                          const SearchQuery& query,
                          const MC2String& itemName ) const;

   /**
    * Search anywhere in the string.
    * @param adder
    * @param query
    * @param itemName a pre-converted item string.
    */
   template <typename ADDER>
   void doAnywhereSearch( ADDER& adder,
                          const SearchQuery& query,
                          const MC2String& itemName ) const;
};




