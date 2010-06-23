/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHQUERY_H
#define SEARCHQUERY_H

#include "config.h"
#include <set>

#include "MC2Coordinate.h"
#include "MC2BoundingBox.h"
#include "SearchParameters.h"
#include "SearchMap2.h"

/**
 *   Class containing the search request with tricks
 *   applied to the strings etc.
 */
class SearchQuery {
public:

   /**
    *   Creates a new SearchQuery.
    *   @param searchMap The SearchMap to search in.
    *   @param params    The parameters.
    */
   SearchQuery(const SearchMap2* searchMap,
               const UserSearchParameters& params);
   
   /// Returns the number of query parts
   int getNbrParts() const {
      return m_stringNumbers.size();
   }
   
   /// There may be more than one string.
   const char* getSearchString(int idx) const {
      return m_stringNumbers[idx].first.c_str();
   }

   /// Return the house number for query part idx.
   int getHouseNumber(int idx) const {
      return m_stringNumbers[idx].second;
   }
   
   /// Returns the requested types
   uint32 getRequestedTypes() const {
      return m_params.getRequestedTypes();
   }

   /// Returns a reference to the parameters
   const UserSearchParameters& getParams() const {
      return m_params;
   }

   /// Returns the requested language
   LangTypes::language_t getReqLang() const {
      return m_params.getRequestedLanguage();
   }

   /**
    *   Returns a reference to a set of indeces to
    *   regions where the searchables should be found.
    */
   const set<uint32>& getRegionIndeces() const {
      return m_regionIdx;
   }

   /**
    *   Returns a reference to a set of indeces to
    *   categories where the searchables should be found.
    */
   const set<uint32>& getCategoryIndeces() const {
      return m_categoryIdx;
   }

   /**
    *   Returns true if the whole map should
    *   be searched.
    */
   bool searchWholeMap() const {
      return m_regionIdx.empty() && m_bboxes.empty() &&
             m_categoryIdx.empty();
   }
   
   /**
    *   Returns true if the parameters have requested
    *   exact or full matching.
    */
   bool isExactOrFull() const {
      return (m_params.getMatching() == SearchTypes::FullMatch) ||
         (m_params.getMatching() == SearchTypes::ExactMatch);
   }

   /**
    *   Returns true if the index is an index of an original
    *   region requested by the server (i.e. not added due to
    *   tricks etc.),
    */
   bool isOriginalRegionIdx(uint32 regIdx) const {
      return m_origRegions.find(regIdx) != m_origRegions.end();
   }

   /**
    *   Returns true if the supplied coordinate is inside any
    *   of the areas that aren't described using groups/regions.
    */
   bool inline coordinateInsideArea(const MC2Coordinate& coord) const;

   /**
    *   Returns the user rights from the search packet.
    */
   inline const UserRightsMapInfo& getRights() const {
      return m_params.getRights();
   }
   
private:

   /// Handles the case where a region also has a radius
   inline void handleRadius(const SearchMap2* searchMap,
                            const SearchMapItem* item);
   
   /// Checks if a string+house number is present in m_stringNumbers
   inline bool stringInStringNumbers(MC2String string, int houseNbr);
   
   /// The parameters
   UserSearchParameters m_params;

   /// The string/housenumbers
   vector<pair<MC2String, int> > m_stringNumbers;
   
   /// The set of regionidxes where to find the searchables.
   set<uint32> m_regionIdx;

   // The set of categories where to find the searchables.
   set<uint32> m_categoryIdx;
   
   /// The set of regionidxes from the original paramters.
   set<uint32> m_origRegions;

   /// Vector of bboxes
   vector<MC2BoundingBox> m_bboxes;
};

bool
SearchQuery::coordinateInsideArea(const MC2Coordinate& coord) const
{
   for( vector<MC2BoundingBox>::const_iterator it = m_bboxes.begin();
        it != m_bboxes.end();
        ++it ) {
      if ( it->contains(coord) ) {
         return true;
      }
   }
   return false;
}

bool
SearchQuery::stringInStringNumbers(MC2String str, int houseNbr)
{
   bool retVal = false;

   // loop the m_stringNumbers vector to see if this string is present or not
   pair<MC2String, int> checkPair = make_pair(str, houseNbr);
   int n = 0;
   while ( !retVal && (n < getNbrParts())) {
      if ( (houseNbr == getHouseNumber(n)) &&
           (str == getSearchString(n)) ) {
         retVal = true;
         return retVal;
      }
      n++;
   }
   
   return retVal;
}

#endif



