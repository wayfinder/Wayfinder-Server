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
#include "SearchableStringSearch.h"

#include "SearchComparators.h"
#include "StringSearchUtility.h"
#include "SearchResult.h"
#include "SearchQuery.h"

#include "SearchMatchPoints.h"
#include "SearchMapItem.h"
#include "MapBits.h"

#include <algorithm>
#include <map>
#include <iterator>

inline bool isCategorySearch( const SearchQuery& query ) {
   return ! query.getParams().getCategories().empty();
}


// -- SearchableStringSearch

SearchableStringSearch::SearchableStringSearch(const SearchMap2* searchMap)
      : m_searchMap(searchMap)
{
   
}

inline bool
SearchableStringSearch::correctRegion(const SearchMapItem* item,
                                      const SearchQuery& query,
                                      const set<uint32>& indeces,
                                      bool addIfEmpty,
                                      int recurselevel ) const
{
   // FIXME: Store the indeces of the regionidx-combos
   //        that return true or false for the checsk.
   
   if ( recurselevel == 0 ) {
      // No use checking this every time
      if ( query.searchWholeMap() ) {
         // Whole map allowed
         mc2dbg8 << "[SSS]: No regions in query" << endl;
         return true;
      }

      // Top-level regions do not have regions, so
      // this should only be checked on the lowest level.
      if ( addIfEmpty && (item->getNbrRegions(m_searchMap) == 0) ) {
         // Should be found everywhere in map
         mc2dbg8 << "[SSS]: No regions in item" << endl;
         return true;
      }
   }
   
   for ( SearchMapRegionIterator it = item->getRegionBegin(m_searchMap);
         it != item->getRegionEnd(m_searchMap);
         ++it ) {
      mc2dbg8 << "[SSS]: Region = "
              << m_searchMap->getItemByIndex(*it)->getID(m_searchMap)
              << endl;
      // Check region without high bit.
      if ( indeces.find(*it & 0x7fffffff) != indeces.end() ) {
         // Found the index. Nice.
         return true;
      }
   }
         
   // Iterate again. Check regions of regions.
   for ( SearchMapRegionIterator it = item->getRegionBegin(m_searchMap);
         it != item->getRegionEnd(m_searchMap);
         ++it ) {
      // Remove the highest bit and test the regions of the region.
      if ( correctRegion(m_searchMap->getItemByIndex(*it & 0x7fffffff), query,
                         indeces, addIfEmpty, recurselevel+1) ) {
         return true;
      }
   }
   return false;
}

inline bool
SearchableStringSearch::correctType(const SearchMapItem* item,
                                    const SearchQuery& query ) const
{
   // Simple anding.
   return item->getSearchType(m_searchMap) & query.getRequestedTypes();
}

inline bool
SearchableStringSearch::correctCoordinate(const SearchMapItem* item,
                                          const SearchQuery& query ) const
{
   return query.coordinateInsideArea(item->getCoordinate(m_searchMap));
}

// -- SearchableMultiStringSearch

SearchableMultiStringSearch::
SearchableMultiStringSearch(const SearchMap2* searchMap)
      : MultiStringSearch(searchMap), SearchableStringSearch(searchMap)
{
}

bool 
SearchableMultiStringSearch::
shouldAddItem( const SearchMapItem* item, const SearchQuery& query ) const {
   // Check if the item is among the allowed item types.
   if ( ! correctType(item, query ) ) {
      // Type is not among the allowed types - next item
      return false;
   }

   // Keep the match if it is logically or geographically
   // inside the requested regions.
   if ( ( ! correctRegion(item, query, query.getRegionIndeces(),
                          true ) ) &&
        ( ! correctCoordinate(item, query) ) ) {
      // Wrong region and also wrong coordinate.
      return false;
   }
         
   // Keep the match if it is logically inside the requested
   // categories.
   if( ! query.getCategoryIndeces().empty() ) {
      if ( ! correctRegion(item, query, query.getCategoryIndeces(),
                           false ) ) {
         // Wrong category
         return false;
      }
   }
              
   // Check if the item should be removed because it is
   // a shadow item for an item crossing a map border.
   if ( ! item->shouldBeFound( m_map ) ) {
      // This item is part of a multimap item
      return false;
   }

         
   // Check if the item has the correct UserRights
   if ( ! item->allowedByUserRights( m_map, query.getRights() ) ) {
      return false;
   }

   // check for rights
   // If the rights shouldnt be inverted and one of the rights 
   // are set then check for specific right in the search item
   // else;
   // if the rights are inverted and if the rights match the search item
   // it shouldnt include the item.
   //
   // And only check the rights on the underview map, else we dont hit any
   // overview maps to do search within
   //
   if ( ! MapBits::isOverviewMap( item->getID( m_map ).getMapID() ) ) {
      if ( ! query.getParams().invertRights() &&
           query.getParams().getMapRights() != ~MapRights() ) {
         if (! m_map->hasRights( item, query.getParams().getMapRights() ) ) {
            // Does not have the correct rights
            return false;
         }
      } else if ( query.getParams().invertRights() &&
                  m_map->hasRights( item, query.getParams().getMapRights() ) ) {
         // has the rights..but we dont want it.
         return false;
      }
   }
   return true;
}


template < typename ADDER >
void SearchableMultiStringSearch::
doCategorySearch( ADDER& adder,
                  const SearchQuery& query,
                  const MC2String& itemName) const { 
   typedef std::vector< const SearchMapPOIInfo* > POIInfoVector;
   POIInfoVector poiInfos;

   if ( m_map->getPOIInfoForCategories( query.getParams().getCategories(),
                                        poiInfos ) == 0 ) {
      // no matches
      return;
   }
   // TODO: Try to fix the anywhere search! There are lots of improvements to
   // be made with this.

   pair<MultiSearchNotice*, MultiSearchNotice*> range;
   // TODO: This should actually use the poi infos too, for faster search.
   // search for item name
   if ( query.getParams().getStringPart() == SearchTypes::Anywhere ) {
      // entire range for anywhere search, the string comparision is
      // done int the adding loop
      range.first = &m_stringIdx[ 0 ];
      range.second = &m_stringIdx[ m_nbrStrIdx ];
   } else {
      range = std::equal_range( m_stringIdx,
                                &m_stringIdx[ m_nbrStrIdx ],
                                itemName.c_str(),
                                MultiStringSearchComp( m_map,
                                                       itemName.c_str() ) );
   }
   set<uint32> ids;
   // add poi info item id
   for ( uint32 i = 0; i < poiInfos.size(); ++i ) {
      ids.insert( m_map->getItemByIndex( poiInfos[ i ]->getItemIdx( m_map ) )->
                  getID( m_map ).getItemID() );
   }

   bool tooManyMatches = false;

   // if no item name was supplied, then use the entire range of items
   if ( itemName.empty() ) {
      range.first = &m_stringIdx[ 0 ];
      range.second = &m_stringIdx[ m_nbrStrIdx ];
   } 

   
   for ( MultiSearchNotice* curNotice = range.first;
         curNotice != range.second && ! tooManyMatches;
         ++curNotice ) {
      // if anywhere string, then we must find substring in the string item
      if ( query.getParams().getStringPart() == SearchTypes::Anywhere &&
           ! itemName.empty() &&
           strstr( m_map->getName( curNotice->getStringIndex() ), 
                   itemName.c_str() ) == NULL ) {
         continue;
      }

      // Also loop over the items. Add filter here.
      const int nbrItems = curNotice->getNbrItems( this );
      for( int i = 0; i < nbrItems; ++i ) {
         // only add items with specific id
         if ( ids.find( curNotice->getItem( this, i )->
                        getID( m_map ).getItemID() ) ==
              ids.end() ) {
            continue;
         }
         
         const SearchMapItem* smi = curNotice->getItem( this, i );
         
         if ( shouldAddItem( smi, query ) ) {
            // add item
            adder.addResult( smi, curNotice, i );
            // if exceed max hits then end search
            if ( adder.resultSize() >= adder.maxNbrMatches() ) {
               mc2dbg << "[MSS]: Will not extract more than "
                      << adder.maxNbrMatches() << " matches" << endl;
               tooManyMatches = true;
               break;
            }
         }
      }
   }
   
}

template <typename ADDER>
void
SearchableMultiStringSearch::
doAnywhereSearch( ADDER& adder,
                  const SearchQuery& query,
                  const MC2String& itemName ) const {
   
   mc2dbg8 << "[SMSS]: Searching substring: " << itemName << endl;

   bool tooManyMatches = false;
   for ( int32 stringIdx = 0; 
         stringIdx < m_nbrStrIdx && ! tooManyMatches; ++stringIdx ) {
      
      MultiSearchNotice* curNotice = &m_stringIdx[ stringIdx ];

      // find substring
      if ( strstr( m_map->getName( curNotice->getStringIndex() ), 
                   itemName.c_str() ) == NULL ) {
         continue;
      }

      const uint32 nbrItems = curNotice->getNbrItems( this );
      for ( uint32 itemIdx = 0; itemIdx < nbrItems; ++itemIdx ) {
         
         const SearchMapItem* item = curNotice->getItem( this, itemIdx );

         if ( !shouldAddItem( item, query ) ) {
            continue;
         }
         
         // Add the result
         adder.addResult( item, curNotice, itemIdx );

         // if exceed max hits then end search
         if ( adder.resultSize() >= adder.maxNbrMatches() ) {
            mc2dbg << "[MSS]: Will not extract more than "
                   << adder.maxNbrMatches() << " matches" << endl;
            tooManyMatches = true;
            break;
         }
         
      }
             
   }
}

template<class ADDER> 
int
SearchableMultiStringSearch::internalSearch(ADDER& adder,
                                            const SearchQuery& query,
                                            const char* str) const
{
   
   MC2String newString = StringSearchUtility::convertIdentToClose( str );

   // Get the notices that match the beginning of the string.
   pair<MultiSearchNotice*, MultiSearchNotice*> range;

   // if this is not a full match search or it is a sub string search 
   // ( anywhere search ) then check for overview and category search.
   if ( query.getParams().getMatching() != SearchTypes::FullMatch || 
        query.getParams().getStringPart() == SearchTypes::Anywhere ) {
      // if its overview map or its not a category search;
      // do normal string search, else do category search
      
      if ( MapBits::isOverviewMap( m_map->getMapID() ) ||
           ! isCategorySearch( query ) ) {
         if ( query.getParams().getStringPart() == SearchTypes::Anywhere ) {
            doAnywhereSearch( adder, query, newString );
            // we are done searching and adding results here
            return 0; 
         } else {
            // else this is not full match nor is it a substring search
            // do normal begining of word search
            range = 
               std::equal_range( m_stringIdx,
                                 &m_stringIdx[ m_nbrStrIdx ],
                                 newString.c_str(),
                                 MultiStringSearchComp( m_map,
                                                        newString.c_str() ) );
         }
      } else if ( ::isCategorySearch( query ) ) {
         doCategorySearch( adder, query, newString );
         return 0;
      }

   } else {
      // Compare the whole string.
      range = std::equal_range( m_stringIdx,
                                &m_stringIdx[ m_nbrStrIdx ],
                                newString.c_str(),
                                MultiStringSearchFullComp( m_map ) );
   }


   // Loop over the range and do some checks before adding the matches.
   bool tooManyMatches = false;

   for ( MultiSearchNotice* curNotice = range.first;
         curNotice != range.second && !tooManyMatches;
         ++curNotice ) {
      mc2dbg2 << "[MSS]: StringMatch \""
              << m_map->getName(curNotice->getStringIndex())
              << "\"" << endl;
      // Also loop over the items. Add filter here.
      const int nbrItems = curNotice->getNbrItems(this);
      for( int i = 0; i < nbrItems; ++i ) {
         // determine if we should add this item
         const SearchMapItem* item = curNotice->getItem(this, i);
         if ( shouldAddItem( item, query ) ) {
            // add item
            adder.addResult(item, curNotice, i);
            // if exceed max hits then end search
            if ( adder.resultSize() >= adder.maxNbrMatches() ) {
               mc2dbg << "[MSS]: Will not extract more than "
                       << adder.maxNbrMatches() << " matches" << endl;
               tooManyMatches = true;
               break;
            }
         }
      }
   }
   
   mc2dbg8 << "[MSS]: ------- END OF MATCHES -------" << endl;
   mc2dbg8 << "[MSS]: NbrMatches = " << adder.resultSize() << endl;
   
   return 0;
}

class SearchAdder {
public:
   /**
    *    Creates a new SearchAdder which adds search results
    *    directly to the outgoing result.
    */
   SearchAdder(const SearchableMultiStringSearch* mss,
               const SearchMap2* theMap,
               const SearchQuery& query,
               SearchResult& result,
               int queryPart,
               const char* searchString)
         : m_mss(mss),
           m_map(theMap),
           m_query(query),
           m_result(result),
           m_searchString(searchString),
           m_exactOrFull( query.isExactOrFull()) {
      
      m_strLen = UTF8Util::nbrChars(searchString);
      m_queryPart = queryPart;
      m_exactSearchString = StringUtility::newStrDup(
         MC2String(StringSearchUtility::
                   convertIdentToExact(m_searchString)).c_str());
      
   }

   /**
    *   Deletes allocated stuff.
    */
   ~SearchAdder() {
      delete [] m_exactSearchString;
   }
   
   /**
    *    Adds one result to the outgoing result. 
    */
   void addResult(const SearchMapItem* item,
                  const MultiSearchNotice* curNotice,
                  int itemOffsetInNotice) {
      
      const bool matchesBeginningOfString =
         curNotice->getStringPartMask(m_mss, itemOffsetInNotice) &
         BEGINNING_OF_STR;
      const bool matchesBeginningOfWord =
         curNotice->getStringPartMask(m_mss, itemOffsetInNotice) &
         BEGINNING_OF_WORD;
      
      // Some extra checks for exact of full match.
      if ( MC2_UNLIKELY( m_exactOrFull ) ) {
         MC2String name = item->
            getName( m_map,curNotice->
                     getNameNbr( m_mss, itemOffsetInNotice ) );
         MC2String exactString = 
            StringSearchUtility::convertIdentToExact( name );

         bool shouldAdd = true;
         // Filter out the ones that are not full here.
         if ( m_query.getParams().getMatching() == SearchTypes::FullMatch ) {
            if ( strcmp( m_exactSearchString, exactString.c_str() ) != 0 ) {
               shouldAdd = false;
            }
         } else {
            // Exactmatching, the string must begin with the exactString.
            if ( strstr( exactString.c_str(), 
                         m_exactSearchString) != exactString ) {

               shouldAdd = false;
            }
         }

         if ( ! shouldAdd ) {
            mc2dbg8 << "[SA]: Not full" << endl;
            // Don't add
            return;
         }
      }

      mc2dbg8 << "[SA] : \""
              << m_map->getName(curNotice->getStringIndex())
              << "\" matchesBeginningOfWord = "
              << matchesBeginningOfWord << ", matchesBeginningOfString = "
              << matchesBeginningOfString
              << ", word nbr = "
              << int(curNotice->getWordNbr(m_mss, itemOffsetInNotice))
              << endl;

      // number of parts can be zero in category search without itemName
      if ( m_queryPart < m_query.getNbrParts() &&
           m_query.getHouseNumber(m_queryPart) != 0 ) {
         if ( (item->getSearchType(m_map) & SEARCH_STREETS) == 0 ) {
            // No need to try to add stuff that has got number
            // but aren't streets.
            return;
         }
      }
      
      SearchMatchPoints* points = new SearchMatchPoints;
      
      points->setMatchedStringIs(SearchMatchPoints::close);
      points->setMatching(m_query.getParams().getMatching());
      points->setMatchesBeginningOfWord( matchesBeginningOfWord );
      // Fix some way to speed this up
      if ( UTF8Util::nbrChars( m_map->getName(curNotice->getStringIndex()) ) ==
           m_strLen ) {
         // Whole string matches
         if ( matchesBeginningOfString ) {
            points->setMatchedStringIs(SearchMatchPoints::full_close);
         } else if ( matchesBeginningOfWord ) {
            // Whole word matches
            points->setMatchesEndOfWord(true);
         }
      } 
      // Set position in string.
      if ( matchesBeginningOfString ) {
         points->setPositionInString(0);
      } else {
         points->setPositionInString(
            curNotice->getWordNbr(m_mss, itemOffsetInNotice));
      }
      // setTypes?
      uint16 subType = MAX_UINT16;
      if ( m_map->getPOIInfoFor( item ) != NULL ) {
         subType = m_map->getPOIInfoFor( item )->getItemSubType( m_map );
      }
      points->setTypes( item->getSearchType( m_map ),
                        item->getItemType( m_map ),
                        subType );

      int nameNbr = curNotice->getNameNbr(m_mss, itemOffsetInNotice );
      
      m_result.addResult(         
         new SearchResultElement(item,
                                 m_queryPart,
                                 points,                                 
                                 nameNbr,
                                 item->getNameInfo(m_map, nameNbr),
                                 m_map->getRights( item ) ),
         m_query.getReqLang());
   }

   /**
    *    Returns the number of matches added so far.
    */
   int resultSize() const {
      return m_result.size();
   }
   
   /**
    *    Returns true if there are too many matches.
    */
   int maxNbrMatches() const {
      return MaxExtractNbrMatches;
   }
   
private:
   /// SearchableMultiStringSearch needed for the get-functions.
   const SearchableMultiStringSearch* m_mss;
   /// SearchMap
   const SearchMap2* m_map;
   /// The search query
   const SearchQuery& m_query;
   /// The search result.
   SearchResult& m_result;
   /// The string that we are looking for
   const char* m_searchString;
   /// The number of characters in the search string.
   int m_strLen;
   /// The part of the query (needed for housenbrs etc)
   int m_queryPart;
   /// True if exact or full matches are requested.
   const bool m_exactOrFull;
   /// String converted to exact
   char* m_exactSearchString;
};

int
SearchableMultiStringSearch::search(SearchResult& result,
                                    const SearchQuery& query) const 
{
   const int bef = result.size();
   
   const UserSearchParameters& params = query.getParams();

   for ( int i = 0; i < query.getNbrParts(); ++i ) {
      SearchAdder adder(this, m_map, query,
                        result,
                        i,
                        query.getSearchString(i));
      internalSearch(adder, query, query.getSearchString(i) );
   }

   // if category search and search for all categories ( empty itemName string):
   if ( query.getNbrParts() == 0 && ::isCategorySearch( query ) ) {
      SearchAdder adder(this, m_map, query,
                        result,
                        0, "" );
      internalSearch(adder, query, "" );
   }

   const bool notFullOrExact = params.getMatching() != SearchTypes::FullMatch
      && params.getMatching() != SearchTypes::ExactMatch;

   // Also try the allwords search in some (most) cases.
   // Don't use for zip codes
   if ( (params.getStringPart() != SearchTypes::Beginning ) &&
        (notFullOrExact) )  {
      if ( result.size() < MaxExtractNbrMatches ) {
         allWordsSearch(result, query);
      }
   }   
   int aft = result.size();
   return aft - bef;
}

class AllWordsSearchAdder {
   
public:
   /**
    *    The AllWordsSearchAdder should be used for adding
    *    indeces 
    */
   AllWordsSearchAdder(const SearchableMultiStringSearch* mss,
                       const SearchMap2* theMap,
                       const vector<char*>& wordVector)
         : m_mss(mss), m_map(theMap), m_wordVector(wordVector)
      {}                       

   /**
    *    Adds one result to the internal structure.
    *    @param item               The found item.
    *    @param curNotice          The notice found.
    *    @param itemOffsetInNotice Item has this offset in curNotice.
    */
   void addResult(const SearchMapItem* item,
                  const MultiSearchNotice* curNotice,
                  int itemOffsetInNotice) {
      if ( curNotice->getStringPartMask(m_mss, itemOffsetInNotice) &
           BEGINNING_OF_WORD ) {
         const uint32 itemIdx = item->getIndex(m_map);
         const uint8 nameNbr = curNotice->getNameNbr(m_mss,itemOffsetInNotice);

         // Check if the other words matched too.
         const char* curName = item->getName(m_map, nameNbr);
         // This is probably slow. Fix later.
         // Other FIXME. Could mayby split up in words and then convert.
         MC2String compString = 
            StringSearchUtility::convertIdentToClose( curName, true );
         if ( ! StringSearchUtility::allWordsMatch ( m_wordVector,
                                                     compString.c_str() ) ) {
            mc2dbg8 << "[SSS]: Removing stripped: "
                    << MC2CITE(compString)
                    << endl;
         } else {            
            m_result.insert(make_pair(itemIdx, nameNbr));
         }
      }
   }

   /**
    *    Returns the number of matches added so far.
    */
   int resultSize() const {
      return m_result.size();
   }
   
   /**
    *    Returns true if there are too many matches.
    */
   int maxNbrMatches() const {
      return MAX_INT32;
   }

   /// Set to put the (temporary) result into.
   typedef set<pair<uint32,uint8> > result_t;
   
   /**
    *    Returns the result.
    */
   result_t& getResult() {
      return m_result;
   }
   
private:
   const SearchableMultiStringSearch* m_mss;
   const SearchMap2* m_map;
   result_t m_result;
   const vector<char*>& m_wordVector;
};


int
SearchableMultiStringSearch::
allWordsSearch(SearchResult& result,
               const SearchQuery& query) const
{
   // Get the searchstrings and do one search for each.
   int bef = result.size();
   for ( int i = 0; i < query.getNbrParts(); ++i ) {
      allWordsSearch(result, query, i);
   }
   int aft = result.size();
   return aft - bef;
}

int
SearchableMultiStringSearch::
allWordsSearch(SearchResult& result,
               const SearchQuery& query,
               int partNum) const
{
   const char* curString = query.getSearchString(partNum);
   // Convert the searchstring into words
   typedef vector<MC2String> StringVector;   
   StringVector closeWordsStr;


   {
      MC2String curStringCopy = curString;
      vector<MC2String> words;
      StringSearchUtility::splitIntoWords(words, curStringCopy);
      // Convert each of the words into close match
      for ( vector<MC2String>::iterator it = words.begin();
            it != words.end();
            ++it ) {
         MC2String closeWord = StringSearchUtility::convertIdentToClose(*it);
         if ( ! closeWord.empty() ) {
            closeWordsStr.push_back(closeWord);
         }
      }
   }
   
   // Return if there were too few words
   if ( closeWordsStr.size() < 2 ) {
      mc2dbg << "[SSS]: Returning early - number of words for part"
             << partNum
             << " = "
             << closeWordsStr.size() << endl;
      
      return 0;
   }

   
   // Copy the closewords before sorting.
   vector<const char*> wordsOrigOrder;
   vector<char*> closeWords;

   closeWords.resize( closeWordsStr.size() );
   for ( int i = 0, n = closeWordsStr.size(); i < n ; ++i ) {
      closeWords[i] = const_cast<char*>( closeWordsStr[i].c_str() );
   }

   wordsOrigOrder.insert(wordsOrigOrder.begin(),
                         closeWords.begin(), closeWords.end());
   // Descending string length needed.
   std::sort(closeWords.begin(), closeWords.end(),
             StringSearchUtility::DescendingStringLength());
   
   for( vector<char*>::iterator it = closeWords.begin();
        it != closeWords.end();
        ++it ) {
      mc2dbg << "[SSS]: Word = \"" << *it << "\"" << endl;
   }
   
   // Create the adder for step one
   AllWordsSearchAdder adder(this, m_map, closeWords);
   // Search for longest word here.
   // FIXME: Try all the words, but don't add first. (equal_range only)
   internalSearch(adder, query, closeWords[0]);
   
   AllWordsSearchAdder::result_t& tempRes(adder.getResult());
   
   mc2dbg << "[SSS]: Number of allword hits " << tempRes.size() << endl;
   
   // Add the remaining items to the result.
   for ( AllWordsSearchAdder::result_t::iterator it = tempRes.begin();
         it !=tempRes.end();         
         ++it ) {
      // Item index is in first
      const SearchMapItem* item = m_map->getItemByIndex(it->first);
      MC2_ASSERT( item );
      // The name number is in second.
      const char* curName = item->getName(m_map, it->second);
      mc2dbg8 << "[SSS]: Hit : " << curName << endl;

      SearchMatchPoints* points = new SearchMatchPoints();

      points->setMatching(SearchTypes::AllWordsMatch);
      points->setMatchesBeginningOfWord(true);
      bool firstWordBegins = false;
      points->setAllWordsCorrectOrder(
         StringSearchUtility::wordsCorrectOrder(wordsOrigOrder,
                                                curName,
                                                &firstWordBegins));
      if ( firstWordBegins ) {
         // Not really applicable here.
         points->setPositionInString(0);
      } else {
         points->setPositionInString(1);
      }
      
      
      SearchResultElement* elem =
         new SearchResultElement(item,
                                 partNum,
                                 points,
                                 it->second,
                                 item->getNameInfo(m_map, it->second),
                                 m_map->getRights( item ) );
                                 
      result.addResult(elem, query.getReqLang() );
      
   }
  
   return 0;
}
