/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H

#include "config.h"

#include "SearchMapItem.h"
#include "SearchMap2.h"
#include "SearchMatch.h"
#include "SearchMatchPoints.h"
#include "NameUtility.h"
#include "DeleteHelpers.h"
#include "MapRights.h"

#include <vector>

/**
 *    Class that contains the result passed from the
 *    SearchableSearchUnit to the SearchProcessor.
 *    The iterator should always contain VanillaMatch.
 */
class SearchUnitSearchResult : public vector<VanillaMatch*> {
public:
   ~SearchUnitSearchResult() {
      STLUtility::deleteValues( *this );
   }
   
};

/**
 *    One result item. Clean up later.
 */
class SearchResultElement {
public:
   /**
    *   @param item    The found item.
    *   @param partNbr Part number in query.
    *   @param points  The points so far.
    *   @param nameNbr The name number found in item.
    */
   SearchResultElement(const SearchMapItem* item,
                       int partNbr,
                       SearchMatchPoints* points,
                       int nameNbr,
                       uint32 stringInfo,
                       const MapRights& rights)
         : m_item(item),
           m_partNbr(partNbr),
           m_points(points),
           m_nameNbr(nameNbr),
           m_stringInfo(stringInfo),
           m_mapRights( rights ) {
   }

   ~SearchResultElement() {
      delete m_points;
   }
   
   const SearchMapItem* getItem() const {
      return m_item;
   }

   /// Get the points for writing...
   const SearchMatchPoints& getPoints() const {
      return *m_points;
   }

   /// Get the name number that was hit
   uint8 getNameNbr() const {
      return m_nameNbr;
   }

   /// Returns which part of the query that was used
   int getQueryPart() const {
      return m_partNbr;
   }

   /// Returns the name info of the result
   uint32 getNameInfo() const {
      return m_stringInfo;
   }

   /// Returns the map rights of this element
   MapRights getMapRights() const {
      return m_mapRights;
   }

private:
   /// Found item
   const SearchMapItem* m_item;
   /// Part of query so that we can find the house-numbers etc.
   int m_partNbr;
   /// The points for confidence
   SearchMatchPoints* m_points;
   /// Name number in item.
   int m_nameNbr;
   /// String information about the item
   uint32 m_stringInfo;
   /// The rights of the item
   MapRights m_mapRights;
};

/**
 *   Class that holds the SearchResult after searching.
 *   (Internally in the SearchModule, the packets use
 *   SearchMatch).
 */
class SearchResult {
public:

   /**
    *   Creates a new result to use.
    */
   SearchResult(const SearchMap2* theMap) : m_map(theMap) {
      // UGGLE: getNbrItems in map and sizeof(uint32), please.
      m_indexArray = new uint32[theMap->end()-theMap->begin()];
      memset(m_indexArray, 0xff, (theMap->end()-theMap->begin())*4);
   }

   /**
    *
    */
   ~SearchResult() {
      STLUtility::deleteValues( m_storage );
      delete [] m_indexArray;
   }
   
   /**
    *   Adds one item to the result.
    *   @param item    Element to add or delete.
    *   @param reqLang Language requested by the user.
    */
   inline bool addResult(SearchResultElement* item,
                         LangTypes::language_t reqLang);

   /// Type of storage used.
   typedef vector<SearchResultElement*> storage_t;
   
   /**
    *    Iterator.
    */
   typedef storage_t::iterator iterator;
   
   /**
    *    Constant iterator.
    */
   typedef storage_t::const_iterator const_iterator;

   const_iterator begin() const { return m_storage.begin(); }
   const_iterator end() const { return m_storage.end(); }
   uint32 size() const { return m_storage.size(); }
private:
   /**
    *   Map of items.
    */
   storage_t m_storage;

   /**
    *   Pointer to the SearchMap. Needed to get
    *   a lot of stuff from the items.
    */
   const SearchMap2* m_map;

   /**
    *   Array as large as the map containing the index
    *   in the vector where the item is or MAX_UINT32
    *   if it hasn't been added yet.
    */
   uint32* m_indexArray;

   /**
    *   Array used when comparing two names.
    */
   uint32 m_nameCompIdx[2];
};

inline bool
SearchResult::addResult(SearchResultElement* resItem,
                        LangTypes::language_t reqLang)
{
   const uint32 index = resItem->getItem()->getIndex(m_map);
   if ( m_indexArray[index] == MAX_UINT32 ) {
      m_indexArray[index] = m_storage.size();
      m_storage.push_back(resItem);
      return true;
   } else {
      // If we got more points this time -> remove old one
      // FIXME: Add check for language etc.
      SearchResultElement* oldElem = m_storage[m_indexArray[index]];

      // Replace if the new element has more points than the
      // other one.
      bool replace = resItem->getPoints() > oldElem->getPoints();

      // Override the points if the lanugages differ, but
      // only if the matches have the same points.
      if ( ( oldElem->getNameInfo() != resItem->getNameInfo() ) &&
           ( oldElem->getPoints() == resItem->getPoints() ) ) {
         // Check if one name is better than the other
         m_nameCompIdx[0] = oldElem->getNameInfo();
         m_nameCompIdx[1] = resItem->getNameInfo();
         if ( NameUtility::getBestName(2, m_nameCompIdx, reqLang) == 1 ) {
            // The index of the resItem was returned. The old one should
            // be replaced
            replace = true;
         } else {
            replace = false;
         }
      }

      
      if ( replace ) {
         // Remove the one in the vector.
         delete m_storage[ m_indexArray[index] ];
         m_storage[ m_indexArray[index] ] = resItem;
      } else {
         delete resItem;
      }
      return replace;
   }
}

#endif

