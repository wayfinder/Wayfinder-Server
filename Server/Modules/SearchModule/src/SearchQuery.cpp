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

#include "MC2String.h"

#include "SearchQuery.h"
#include "StringSearchUtility.h"
#include "AbbreviationTable.h"

inline void
SearchQuery::handleRadius(const SearchMap2* searchMap,
                          const SearchMapItem* item) 
{
   if ( item->getRadiusMeters(searchMap) == 0 ) {
      // No radius for the item
      return;
   }

   mc2dbg << "[SQ]: Coord is " << item->getCoordinate(searchMap)
          << " and radius is " << item->getRadiusMeters(searchMap)
          << endl;
   
   if ( ! item->getCoordinate(searchMap).isValid() ) {
      mc2log << warn << "[SQ]: Item " << item->getID(searchMap)
             << " has radius but invalid center" << endl;
      return;
   }

   // Make bounding box using the radius.
   m_bboxes.push_back(MC2BoundingBox(item->getCoordinate(searchMap),
                                     item->getRadiusMeters(searchMap) ) );
   mc2dbg << "[SQ]: Added bbox " << m_bboxes.back() << " for item "
          << item->getID(searchMap) << endl;
   
}

SearchQuery::SearchQuery(const SearchMap2* searchMap,
                         const UserSearchParameters& params)
      : m_params(params)
{
   MC2_ASSERT( searchMap );

   // Translate the region ID:s into indeces.
   for ( int i = 0; i < m_params.getNbrMasks(); ++i ) {
      // WARNING! Map id is used here.
      IDPair_t curID(searchMap->getMapID(),
                     params.getMaskItemIDs()[i]);
      if ( ! curID.isValid() ) {
         mc2log << warn << "[SQ] UserSearchParameters contains"
                << " invalid mask item ids! Ignoring id. " 
                <<  " id = " << curID << endl;
         continue;
      }
      const SearchMapItem* item = searchMap->getItemByItemID(curID);

      if ( item == NULL ) {
         mc2log << warn << "[SQ] Could not find item by item id. id = " 
                << curID << endl;
         continue;
      }

      m_regionIdx.insert(item->getIndex(searchMap));
      m_origRegions.insert(item->getIndex(searchMap));

      handleRadius(searchMap, item);

      // Add the municipal of BUA
      for( SearchMapRegionIterator it = item->getRegionBegin(searchMap);
           it != item->getRegionEnd(searchMap);
           ++it ) {
         const SearchMapItem* newRegion = searchMap->getItemByIndex(*it);
         if ( ! (newRegion->getSearchType(searchMap) & SEARCH_ZIP_CODES ) ) {
            if ( params.getMinNbrHits() > 0 ||
                 newRegion->oneNameSimilar(searchMap, item) ) {
               mc2dbg2 << "[SQ]: Adding region: small region("
                      << item->getID(searchMap)
                      << MC2CITE(item->getBestName(searchMap,
                                                   LangTypes::english).first)
                      << ")->big region(" 
                      << newRegion->getID(searchMap) 
                      << MC2CITE(newRegion->getBestName(searchMap,
                                                        LangTypes::english).first)
                      << ")" << endl;
               m_regionIdx.insert( newRegion->getIndex(searchMap) );
            }
         }
      }
   }

   // First take the one without number ( if the string isn't empty ).
   if ( ! m_params.getString().empty() ) {
      m_stringNumbers.push_back(
         make_pair( m_params.getString(), 0 ));
      
      // FullMatch does not want the number to be removed.
      if ( m_params.getMatching() != SearchTypes::FullMatch ) {
         // Split the string into housenumber and string - should this
         // functionality be here?
         MC2String stripped;
         int houseNbr;
         StringSearchUtility::getStreetNumberAndSearchString(
            m_params.getString(), houseNbr, stripped);
         
         if ( houseNbr != 0 && stripped.size() >= 3 ) {
            m_stringNumbers.push_back(
               make_pair(stripped, houseNbr));
         }
         
         // Expand addresses
         // Insert expanded strings into the m_stringNumbers vector.
         // Only expand when we know that it is an adress (nbrMasks > 0)
         if ( m_params.getNbrMasks() > 0 ) {
            StringTable::countryCode cc = searchMap->getCountryCode();

            // Get native languages from the country
            set<LangTypes::language_t> nativeLangs;
            searchMap->getNativeLanguages(nativeLangs);
            set<LangTypes::language_t>::const_iterator langIt;

            // Expand for each native language to all combinations
            // Also requested language from the search params
            vector<MC2String>::const_iterator cit;
            for (langIt = nativeLangs.begin();
                 langIt != nativeLangs.end(); langIt++) {
               
               vector<MC2String> expandedStrings = 
                  AbbreviationTable::fullExpand( stripped.c_str(),
                                                 *langIt, cc );
               
               // Insert into m_stringNumbers (if not there before)
               for ( cit = expandedStrings.begin();
                     cit != expandedStrings.end(); cit++ ) {
                  if ( !stringInStringNumbers(*cit, houseNbr) ) {
                     m_stringNumbers.push_back( make_pair(*cit, houseNbr) );
                     mc2dbg8 << "expanded string \"" << *cit << "\" "
                             << houseNbr << endl;
                  }
                  //else there before, don't include.
               }
            }
            
         }

         // Expand areas, no native languages for overview map, 
         // expand for invalidLanguage meaning all languages.
         else if ( m_params.getNbrMasks() == 0 ) {
            StringTable::countryCode cc = searchMap->getCountryCode();
            
            vector<MC2String> expandedStrings = 
               AbbreviationTable::fullExpand(
                     stripped.c_str(), LangTypes::invalidLanguage, cc, false );
            
            // Insert into m_stringNumbers (if not there before)
            vector<MC2String>::const_iterator cit;
            for ( cit = expandedStrings.begin();
                  cit != expandedStrings.end(); cit++ ) {
               if ( !stringInStringNumbers(*cit, houseNbr) ) {
                  m_stringNumbers.push_back( make_pair(*cit, houseNbr) );
                  mc2dbg8 << "expanded string \"" << *cit << "\" " 
                          << houseNbr << endl;
               }
               //else there before, don't include.
            }
         }
      }
   }
   const vector<uint32>& categories = m_params.getReturnCategories();
   vector<uint32>::const_iterator it;
   for(it = categories.begin(); it != categories.end(); ++it) {
      m_categoryIdx.insert( *it );
   }
}
