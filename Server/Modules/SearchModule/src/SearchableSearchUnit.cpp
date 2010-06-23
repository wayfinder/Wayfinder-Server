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
#include "SearchableSearchUnit.h"
#include "SearchableStringSearch.h"
#include "ModuleMap.h"
#include "DataBuffer.h"
#include "MapPacket.h"
#include "SearchMatch.h"
#include "SearchQuery.h"
#include "SearchResult.h"
#include "SearchSorting.h"
#include "GfxUtility.h"
#include "CountryCodeStringTable.h"
#include "StringSearchUtility.h"
#include "STLStrComp.h"
#include "DeleteHelpers.h"

#include "STLUtility.h"

SearchableSearchUnit::SearchableSearchUnit() : SearchUnit()    
{
   // Nothing yet
}

void
SearchableSearchUnit::createMultiStringSearch()
{
   m_multiSearch = new SearchableMultiStringSearch(m_searchMap);
   m_ownedMultiStringSearch = m_multiSearch;
}

bool
SearchableSearchUnit::loadFromMapModule(uint32 mapID)
{
   char handshake[1024];
   sprintf(handshake, "Please send map number %d", mapID);
   // One try only
   Readable* sock =
      ModuleMap::getMapLoadingReadable(mapID,
                                       MapRequestPacket::MAPREQUEST_SEARCH,
                                       handshake);
   if ( sock == NULL ) {
      return false;
   }

   // Read the size of the real databuffer
   DataBuffer length(4);
   if (sock->read(length.getBufferAddress(), 4) != 4 ) {
      delete sock;
      return false;
   }

   int mapSize = length.readNextLong();
   DataBuffer bigBuf(mapSize);
   
   if ( sock->read(bigBuf.getBufferAddress(), mapSize) != mapSize ) {
      delete sock;
      return false;
   }
   
   delete sock;
   return load(bigBuf);
}

int
SearchableSearchUnit::
expandIDs(vector<pair<uint32, OverviewMatch*> >& expandedMatches,
          const vector<pair<uint32,IDPair_t> >& idsToExpand,
          bool expand) const
{
   // Expand the id:s
   vector<pair<uint32, IDPair_t> > expandedIDs;
   m_searchMap->expandIDs(expandedIDs, idsToExpand, expand);

   // Convert the IDPair_t:s to matches.
   // Fill in the most important features
   for( vector<pair<uint32, IDPair_t> >::const_iterator it
           = expandedIDs.begin();
        it != expandedIDs.end();
        ++it ) {
      // Look up the item
      const SearchMapItem* item = m_searchMap->getItemByItemID(it->second);
      if ( item == NULL ) {
         // Oj. Translated
         item = m_searchMap->getItemByItemID(
            m_searchMap->translateToHigher(it->second));
         if ( item == NULL ) {
            mc2log << fatal << "[SSU]: Item not found and not higher level "
                   << "either: ID: " << it->second << endl;
            continue;
         }
      }
      OverviewMatch* ov = new OverviewMatch(it->second);
      ov->setRadiusMeters(item->getRadiusMeters(m_searchMap));
      // Set coordinates - mostly to avoid error codes when sorting.
      ov->setCoords(item->getCoordinate(m_searchMap));
      // Also set the name. It can be nice when debugging.
      ov->setName(item->getBestName(m_searchMap, LangTypes::swedish).first);
      expandedMatches.push_back(make_pair(it->first, ov));
   }
   return expandedMatches.size();
}

VanillaRegionMatch*
SearchableSearchUnit::createRegionMatch(uint32 itemIdx,
                                        const SearchQuery& query) const
{
   const SearchMapItem* item = m_searchMap->getItemByIndex(itemIdx);
   const LangTypes::language_t lang = query.getParams().getRequestedLanguage();
   // FIXME: Remove this from constuctor
   const SearchMatchPoints points;
   
   VanillaRegionMatch* regionMatch =
      new VanillaRegionMatch(item->getBestName(m_searchMap, lang).first,
                             0, // nameinfo
                             item->getID(m_searchMap).getMapID(),
                             item->getID(m_searchMap).getItemID(),
                             item->getSearchType(m_searchMap),
                             points, // Not important, really.
                             0, // Source
                             item->getBestName(m_searchMap, lang).first, 
                             0, // Location
                             0, // Restrictions
                             item->getItemType(m_searchMap),
                             0); // ItemSubtype - FIXME.
   
   return regionMatch;
}

void
SearchableSearchUnit::addRegionsToMatch(SearchMatch* match,
                                        uint32 itemIdx,
                                        const SearchQuery& query,
                                        int level) const
{
   // Example
   // Our item is in A, B, C and D
   // A and B are in C and C is in D
   // Foreach region
   //   if region is inside other region -> remove _other_ region
   //   or if no region is inside a region -> keep region
   // After this call this function for every remaining region.

   // FIXME: Move this functionality to the SearchUnitBuilder.
   // WARNING: There is code similar to this in MapProcessor. Change it
   //          too if you do something useful here.
   
   const SearchMapItem* item = m_searchMap->getItemByIndex(itemIdx);

   uint32 returnRegionTypes = query.getParams().getReturnRegionTypes();
   
   for( SearchMapRegionIterator it = item->getRegionBegin(m_searchMap);
        it != item->getRegionEnd(m_searchMap);
        ++it ) {
      
      mc2dbg8 << "SearchableSearchUnit::addRegionsToMatch regionID:" 
             << (*it) << endl;

      if ( REGION_SHOULD_NOT_BE_DISPLAYED( *it ) ) {
         // The item should only be found in the region.
         continue;
      }

      // Update the points for the match
      if ( query.isOriginalRegionIdx(*it) ) {
         match->getPointsForWriting().setCorrectLocation(true);
      }

      // Add points if the name of the location is the same as the
      // item itself (Lund, Lund)
      if ( item->oneNameSameCase(m_searchMap, 
                                 m_searchMap->getItemByIndex(*it))) {
         match->getPointsForWriting().setCorrectLocation(true);
      }
      
      bool noRegionInside = true;
      // Check type if it-item. Skip it if it is not among the allowed.
      if ( ! (m_searchMap->getItemByIndex(*it)->getSearchType(m_searchMap) &
              returnRegionTypes ) ) {
         continue;
      }
      
      for( SearchMapRegionIterator jt = item->getRegionBegin(m_searchMap);
           jt != item->getRegionEnd(m_searchMap);
           ++jt ) {
         if ( it == jt ) continue;
         if ( REGION_SHOULD_NOT_BE_DISPLAYED(*jt) ) continue;
         // Check type if jt-item
         if ( ! (m_searchMap->getItemByIndex(*jt)->getSearchType(m_searchMap) &
              returnRegionTypes ) ) {
            continue;
         }

         if ( m_searchMap->isInsideRegion(m_searchMap->getItemByIndex(*jt),
                                          *it) ) {
            // A region was inside it
            noRegionInside = false;
            break;
         }
      }
      if ( noRegionInside ) {
         // No other region was inside this region, means lowest level.
         if ( m_searchMap->getItemByIndex(*it)->getSearchType(m_searchMap) &
              query.getParams().getReturnRegionTypes() ) {
            
            // This type of region should be added
            VanillaRegionMatch* regionMatch = createRegionMatch(*it, query);
            
            mc2dbg4 << "[SSU]: Created region " << regionMatch->getName()
                    << " at level = " << level << endl;
            
            // Add the regions to that match too.
            addRegionsToMatch(regionMatch, *it, query, level+1);
            
            // Add our region to the original match.
            match->addRegion(regionMatch, true);
         } else {
            // The type of region was not wanted.
            // Add regions to our old match, but from the region
            // Will not work very well because of the zip-codes
            // E.g. 22220 is in the citypart Väster.
            // Spolegatan is in Innerstaden and 22220, if we just
            // skip the 22220 we will end up with both Innerstaden and
            // Väster.
            //addRegionsToMatch(match, *it, query, level + 1);
         }
      }
   }
   
}

inline pair<const char*, LangTypes::language_t>
SearchableSearchUnit::getNameToUse(const SearchQuery& query,
                                   const SearchResultElement* curElement) const
{
   const SearchMapItem* const curItem = curElement->getItem();
   // Get the matched name
   int matchedName = curElement->getNameNbr();
   if ( curItem->getNameType( m_searchMap, matchedName ) ==
        ItemTypes::synonymName ) {
      if ( curItem->getSearchType( m_searchMap) & SEARCH_ALL_REGION_TYPES ) {
         // Keep synonym names for regions. E.g. Göteborg->Frölunda
         return pair<const char*, LangTypes::language_t>
            (curItem->getName(m_searchMap, matchedName),
             LangTypes::invalidLanguage); // No language available
      } else {
         // Find the best name instead
         return curItem->getBestName(m_searchMap,
                                     query.getParams().getRequestedLanguage(),
                                     matchedName);
      }
   } else {
      // Keep name
      return pair<const char*, LangTypes::language_t>
         (curItem->getName(m_searchMap, matchedName),
          LangTypes::invalidLanguage);  // No language available
   }
}

inline
bool
SearchableSearchUnit::addSynonymNameTo( VanillaCompanyMatch* poi,
                                        const SearchQuery& query,
                                        const SearchMapItem* item,
                                        const SearchMapPOIInfo* poiInfo ) const
{
   if ( item->getItemType( m_searchMap ) != ItemTypes::pointOfInterestItem ) {
      return false;
   }

   // Select when to add.
   switch ( ItemTypes::pointOfInterest_t (
      poiInfo->getItemSubType ( m_searchMap ) ) ) {
      case ItemTypes::busStation:
      case ItemTypes::cityCentre:
      case ItemTypes::commuterRailStation:
      case ItemTypes::ferryTerminal:
      case ItemTypes::railwayStation:
      case ItemTypes::postOffice:
      case ItemTypes::tramStation:
         break;
      default:
         // Don't add.
         // RETURN !!
         return false;
         break;
   }
   
   // Get the best synonym name given the requested language.
   const char* synonymName =
      item->getBestNameOfType( m_searchMap,
                               query.getReqLang(),
                               ItemTypes::synonymName);
   
   if ( synonymName == NULL ) {
      // No synonym name could be found.
      return false;
   }

   // AVOID SHOUTING!
   MC2String lowerString =
      MC2String(StringUtility::copyLower(synonymName));
   MC2String capitalString =
      StringUtility::makeFirstInWordCapital(lowerString);
   
   // Now add the name
   poi->setName((MC2String(poi->getName()) + "(" +
                 capitalString + ")").c_str());

   return true;
}

inline SearchMatch*
SearchableSearchUnit::createSearchMatch(const SearchResultElement* curElement,
                                        const SearchQuery& query) const
{
   SearchMatch* curMatch = NULL;
   const SearchMapItem* curItem = curElement->getItem();
   // Extract some variables from the element   
   const IDPair_t id(curItem->getID(m_searchMap));
   // FIXME: Choose a name if it is a synonym name.
   pair<const char*, LangTypes::language_t> name =
      getNameToUse(query, curElement);
   const char* alphaSortingName = name.first;
   const char* locationName = "LOCATION";
   // Adds the matching string (mostly synonymname) to result
   // if not returning matching string.
   const char* matchedName = curItem->getName( 
      m_searchMap, curElement->getNameNbr() );
   MC2String nameAndMatched( name.first );
   
   bool addSynonymNameToPois = query.getParams().getAddSynonymNameToPOIs();
   if ( matchedName != name.first ) { // STRCMP!!!? Works as is!
      nameAndMatched.append( " (" );
      // AVOID SHOUTING!
      MC2String lowerString = StringUtility::copyLower( matchedName );         
      lowerString = StringUtility::makeFirstInWordCapital( lowerString );
      nameAndMatched.append( lowerString );
      nameAndMatched.append( ")" );
      name.first = nameAndMatched.c_str();      
      // Don't add the synonyme to the poi since it is probably added
      // already. A check is really needed to see if the extra name
      // added here really is a synonym name.
      addSynonymNameToPois = false;
   }
   
   const SearchMatchPoints& points = curElement->getPoints();

   LangTypes::language_t language = LangTypes::invalidLanguage;
   if( name.second == LangTypes::invalidLanguage ) {
      uint32 stringIndex = curElement->getNameInfo();
      language = GET_STRING_LANGUAGE(stringIndex);
   } else {
      language = name.second;
   }
   switch ( curItem->getSearchType(m_searchMap) ) {
      case SEARCH_STREETS: 
         curMatch = new VanillaStreetMatch( id,
                                            name.first,
                                            locationName,
                                            0,   // offset
                                            0,   // house number
                                            CountryCodeStringTable::streetNumberFirst(m_searchMap->getCountryCode(), language),
                                            CountryCodeStringTable::streetNumberComma(m_searchMap->getCountryCode(), language) );
         break;
      case SEARCH_COMPANIES:
         curMatch = new VanillaCompanyMatch( id, name.first, locationName,
                                             0,   // offset
                                             0 ); // house number
         break;
      case SEARCH_MISC:
         curMatch = new VanillaMiscMatch( id, name.first, locationName);
         break;
      case SEARCH_CATEGORIES:
         curMatch = new VanillaCategoryMatch( id, name.first, locationName);
         break;
      case SEARCH_MUNICIPALS:
      case SEARCH_BUILT_UP_AREAS:
      case SEARCH_CITY_PARTS:
      case SEARCH_ZIP_CODES:
      case SEARCH_ZIP_AREAS:
         // All these will be the same in the packet and will
         // be correctly typed when read from the packet in the
         // server.
         curMatch =
            new VanillaRegionMatch( name.first,
                                    0, // nameInfo
                                    id.getMapID(),
                                    id.getItemID(),
                                    curItem->getSearchType(m_searchMap),
                                    points,
                                    0, // source
                                    alphaSortingName,
                                    0, // location
                                    0, // restrictions
                                    curItem->getItemType(m_searchMap),
                                    MAX_UINT16 ); // itemsubtype,
                                                  // only companies.
         break;
   }
   if ( curMatch == NULL ) {
      return curMatch;
   }
   // Set some variables

   // Set the points
   curMatch->setPoints(curElement->getPoints());
   // Set the map rights
   curMatch->setMapRights( curElement->getMapRights() );
   // Add the regions.
   addRegionsToMatch(curMatch,
                     curItem->getIndex(m_searchMap),
                     query);
   
   // And set a location name
   curMatch->updateLocationName(query.getParams().getReturnRegionTypes());
   
   // Set itemtype
   curMatch->setItemType(curItem->getItemType(m_searchMap));
   // Set cordinates
   curMatch->setCoords(curItem->getCoordinate(m_searchMap));
   mc2dbg8 << "[SSU]: Setting coordinate = " << curMatch->getCoords()
           << " for " << *curMatch << endl;
   
   // Set distance
   if ( query.getParams().getSortOrigin().isValid() ) {
      mc2dbg8 << "[SSU]: Sort origin is valid" << endl;
      const MC2Coordinate& origin(query.getParams().getSortOrigin());
      if ( curMatch->getCoords().isValid() ) {
         double distanceSq =
            GfxUtility::squareP2Pdistance_linear(origin.lat,
                                                 origin.lon,
                                                 curMatch->getCoords().lat,
                                                 curMatch->getCoords().lon);
         double distance = ::sqrt(distanceSq);
         uint32 intDist = uint32(distance);
         curMatch->setDistance(intDist);
         mc2dbg8 << "[SSU]: Set distance = " << intDist << endl;
      } else {
         curMatch->setDistance(MAX_UINT32);
      }
   }
   
   if ( curItem->getSearchType(m_searchMap) & SEARCH_COMPANIES ) {
      // We must find out some more information about companies.
      // Side wasn't set by the old module, so I will not set it
      // here
      const SearchMapPOIInfo* poiInfo = m_searchMap->getPOIInfoFor(curItem);
      if ( poiInfo ) {
         mc2dbg8 << "[SSU]: Found poiinfo for " << curItem->getID(m_searchMap)
                 << endl;
         
         VanillaCompanyMatch* company =
            static_cast<VanillaCompanyMatch*>(curMatch);
         // Set itemsubtype
         company->setItemSubType(poiInfo->getItemSubType(m_searchMap));

         // Add synonym name if it is not added already.
         if ( addSynonymNameToPois ) {
            addSynonymNameTo( company, query, curItem, poiInfo );
         }

         company->setSpecialImage( poiInfo->getSpecialImage() );

         company->setCategories( poiInfo->getCategories() );

         if ( query.getParams().getAddSSINameToCompanies() ) {
            // Save the clean company name
            company->setCleanCompanyName( company->getName() );
            if ( poiInfo->getNbrAddresses(m_searchMap) != 0 ) {
               // Set this to 0 because otherwise maybe the gfxclient
               // will write the streetnumber twice. 
               // The street number is included if not zero later.
               company->setStreetNbr( 0 ); 
               
               mc2dbg8 << "[SSU]: Number on street  = "
                       << poiInfo->getNumberOnStreet(m_searchMap)
                       << endl;
               const char* oldName = curMatch->getName();
               pair<const char*, LangTypes::language_t> addressPair =
                  poiInfo->getBestAddress(
                     m_searchMap,
                     query.getParams().getRequestedLanguage() );
               char* bestAddress =
                  StringUtility::newStrDup(addressPair.first);
               const char* commaSpace = ", ";
               // Remove commas(',') from bestAddress
               char* pos = bestAddress;
               while ( *pos != '\0' ) {
                  if ( *pos == ',' ) {
                     *pos = ' ';
                  }
                  ++pos;
               }
               char* newName = new char[strlen(oldName)+1 +
                                       strlen(bestAddress) + 1 +
                                       strlen(commaSpace) + 1 + 20];
               if ( poiInfo->getNumberOnStreet(m_searchMap) != 0 ) {
                  bool streetNumberFirst =
                     CountryCodeStringTable::streetNumberFirst(m_searchMap->getCountryCode(), addressPair.second);
                  bool streetNumberComma =
                     CountryCodeStringTable::streetNumberComma(m_searchMap->getCountryCode(), addressPair.second);
                  if( streetNumberFirst ) {
                     if( streetNumberComma ) {
                        sprintf( newName, "%s%s%u, %s",
                                 oldName, commaSpace,
                                 poiInfo->getNumberOnStreet(m_searchMap),
                                 bestAddress);
                     } else {
                        sprintf( newName, "%s%s%u %s",
                                 oldName, commaSpace,
                                 poiInfo->getNumberOnStreet(m_searchMap),
                                 bestAddress);
                     }
                  } else {
                     if( streetNumberComma ) {
                        sprintf( newName, "%s%s%s, %u",
                                 oldName, commaSpace, bestAddress,
                                 poiInfo->getNumberOnStreet(m_searchMap));
                     } else {
                        sprintf( newName, "%s%s%s %u", 
                                 oldName, commaSpace, bestAddress, 
                                 poiInfo->getNumberOnStreet(m_searchMap));
                     }
                  }
               } else {
                  sprintf( newName, "%s%s%s", 
                           oldName, commaSpace, bestAddress );
               }
               company->setName(newName);
               delete [] newName;
               delete [] bestAddress;
            } else { // else remove , from name
               // TODO: Remove comma from companyname if 
               // AddStreetNamesToCompanies is true and
               // no address was added to name. Wayfinder(Nav2) extracts
               // address by looking for ','. 20030711
               const char* oldName = curMatch->getName();
               if ( strchr( oldName, ',' ) != NULL ) {
                  char* newName = new char[ strlen( oldName ) + 1 ];
                  strcpy( newName, oldName );
                  char* cPos = strchr( newName, ',' );
                  while ( cPos != NULL ) {
                     cPos[ 0 ] = ' '; // Removing ','
                     cPos = strchr( cPos, ',' );
                  }
                  company->setName( newName );
                  delete [] newName;
               } // Else no ',' to remove do nothing
            } // End else remove comma
         }
      }
   }

   if ( curItem->getSearchType(m_searchMap) & SEARCH_STREETS ) {
      // Set the house number so that the server can send it to
      // MapModule for expansion.
      VanillaStreetMatch* street = 
         static_cast<VanillaStreetMatch*>(curMatch);
      // Some special searches doesn't have parts, like proximity so check
      if ( curElement->getQueryPart() < query.getNbrParts() ) {
         street->setStreetNbr( query.getHouseNumber(
                                  curElement->getQueryPart() ) );
      }
      street->setStreetSegmentID(id.getItemID());
   }

   return curMatch;
}

void
SearchableSearchUnit::
convertInternalResultToMatches(SearchUnitSearchResult& result,
                               const SearchQuery& query,
                               const SearchResult& internalResult) const
{
   for( SearchResult::const_iterator it = internalResult.begin();
        it != internalResult.end();
        ++it ) {
      const SearchResultElement* curElement = *it;
      const SearchMapItem* curItem = curElement->getItem();
      mc2dbg8 << "[SSU]: Hit has id = " << curItem->getID(m_searchMap)
              << " and index " << curItem->getIndex(m_searchMap) << endl;
      result.push_back(
         static_cast<VanillaMatch*>(createSearchMatch(curElement,query)));
   }
   
}

int
SearchableSearchUnit::
sortMatchesRemovingUnsorted( SearchUnitSearchResult& result,
                             const UserSearchParameters& params) const
{
   // Sort the result.
   uint32 nbrSorted = SearchSorting::sortSearchMatches(result,
                                                       params.getSorting());
   
   // Remove the matches that are not sorted.
   if ( nbrSorted < result.size() ) {
      for ( SearchUnitSearchResult::iterator it = result.begin() + nbrSorted;
            it != result.end();
            ++it ) {
         delete *it;
         *it = NULL;
      }
      result.resize( nbrSorted );
   }
   return nbrSorted;
}

int
SearchableSearchUnit::search( SearchUnitSearchResult& result,
                              const UserSearchParameters& params) const
{
   SearchResult internalResult(m_searchMap);
   SearchQuery query(m_searchMap, params);
   
   // Search 
   m_multiSearch->search(internalResult, query);

   // Convert the result
   convertInternalResultToMatches(result, query, internalResult);

   // Sort the result.
   int nbrSorted = sortMatchesRemovingUnsorted( result, params );

   mc2dbg << "[SSU]: nbrSorted = " << nbrSorted << endl;
   
   return result.size();
}

int
SearchableSearchUnit::
overviewSearch(SearchUnitSearchResult& result,
               const UserSearchParameters& params) const
{
   // Add everything that does not have to do with
   // converting matches to overview matches in this function.

   // Start by doing a normal search.
   search(result, params);

   int nbrRemoved = 0;

   // Try a new method.
   // Insert all the uppercase names in a map and then n² will have
   // a much smaller n. The disappointing thing is that the map can
   // take some time to build.
   // The time for 1700 hits for "a" in France has been reduced from
   // 10 seconds to 1 second, though (whopper).
   mc2dbg8 << "[SSU]: START building name removal map" << endl;
   typedef map<const char*, set<SearchUnitSearchResult::iterator>,
               strNoCaseCompareLess > nameMap_t;
   nameMap_t nameMap;
   for ( SearchUnitSearchResult::iterator it = result.begin();
        it != result.end();
         ++it ) {
      const SearchMapItem* searchItem =
         m_searchMap->getItemByItemID( (*it)->getID() );
      int nbrNames = searchItem->getNbrNames( m_searchMap );
      for ( int i = 0; i < nbrNames; ++i ) {
         // Do not add synonyms!
         if ( searchItem->getNameType( m_searchMap, i ) !=
              ItemTypes::synonymName ) {
            const char* curName = searchItem->getName( m_searchMap, i );
            nameMap[ curName ].insert( it );
         }
      }
   }
   mc2dbg8 << "[SSU]: DONE building name removal map" << endl;

   // Some interesting values.
   uint32 maxNameMapSize = 0;
   const char* maxName = "";
   // The old searchmodule used to remove the municipal
   // if there was a match with the same for a builtup area.
   // We will also do that for municipals inside builtup areas.
   mc2dbg8 << "[SSU]: START Removing areas with same names" << endl;
   // Must store the matches that are going to be deleted since
   // we might use their names in the name map.
   vector<SearchUnitSearchResult::value_type> toDelete;
   // First loop over the map with common names.
   for ( nameMap_t::iterator nit = nameMap.begin();
         nit != nameMap.end();
         ++nit ) {
      if( nit->second.size() < 2 ) {
         // Nothing to compare.
         continue;
      }
      if ( nit->second.size() > maxNameMapSize ) {
         maxName = nit->first;
         maxNameMapSize = nit->second.size();
      }
      // Here the number of matches shouldn't be very high.
      for( set<SearchUnitSearchResult::iterator>::iterator
              xit = nit->second.begin();
           xit != nit->second.end();
           ++xit ) {
         SearchUnitSearchResult::iterator it = *xit;
         for( set<SearchUnitSearchResult::iterator>::iterator xjt =
                 nit->second.begin();
              xjt != nit->second.end();
              ++xjt ) {
            SearchUnitSearchResult::iterator jt = *xjt;
            if ( (it == jt) || (*it == NULL) || (*jt == NULL) ) {
               // Same position or already removed
               continue;
            }         
            const SearchMatch* munOrBua = *it;
            if ( ! ( munOrBua->getType() & 
                     (SEARCH_MUNICIPALS | SEARCH_BUILT_UP_AREAS ) ) ) {
               // No, it wasn't a municipal / bua
               continue;
            }
            
            // Also get the other item         
            const SearchMatch* other = *jt;
            if ( munOrBua->getType() == other->getType() ) {
               // Same type
               continue;
            }
            
            // Get the item
            const SearchMapItem* munOrBuaItem =
               m_searchMap->getItemByItemID( munOrBua->getID() );
            
            const SearchMapItem* otherItem =
               m_searchMap->getItemByItemID( other->getID() );
            
            // Check if the other region is inside the outer area
            // and if they have the same names.
            if ( m_searchMap->
                 isInsideRegion(otherItem,
                                munOrBuaItem->getIndex(m_searchMap))
                 &&
                 otherItem->oneNameSameCase(m_searchMap, munOrBuaItem) ) {
               
               // Remove it
               /*mc2dbg << "[SSSU]: Removing outer area "
                 << munOrBuaItem->getID(m_searchMap)
                 << MC2CITE(munOrBuaItem->getBestName(
                 m_searchMap,
                 LangTypes::english).first)
                 << endl;
                 delete *it;
                 *it = NULL;*/
               mc2dbg8 << "[SSSU]: Removing inner area "
                       << otherItem->getID(m_searchMap)
                       << MC2CITE(otherItem->getBestName(
                                     m_searchMap,
                                     LangTypes::english).first)
                       << endl;
               // Since this item was in an item with the same name
               // we will add some points to it.
               (*it)->getPointsForWriting().setCorrectLocation(true);
               toDelete.push_back( *jt );
               *jt = NULL;
               ++nbrRemoved;
            }
         }
      }
   }

   mc2dbg8 << "[SSU]: DONE Removing " << nbrRemoved
          << " areas with same names" << endl;
   
   mc2dbg8 << "[SSU]: Size of name map = " << nameMap.size() << endl;
   mc2dbg8 << "[SSU]: Max size of entry = " << maxNameMapSize << endl;
   mc2dbg8 << "[SSU]: Most common name = " << maxName << endl;
   
   STLUtility::deleteValues( toDelete );
   
   // Check for zipcodes that are full matches - remove the others.
   bool fullZipFound = false;
   for( SearchUnitSearchResult::iterator it = result.begin();
        it != result.end();
        ++it ) {
      const SearchMatch* zipCode = *it;
      if ( zipCode == NULL ) {
         continue;
      }
      if ( ! ( zipCode->getType() & SEARCH_ZIP_CODES ) ) {
         continue;         
      }
      if ( zipCode->getPoints().isFullMatch() ) {
         fullZipFound = true;
         break;
      }
   }
   
   if ( fullZipFound ) {
      // Remove the zipcodes that weren't full and not inside another zipcode
      // which is full
      // FIXME: Some kind of check if the zips have similar names?
      for( SearchUnitSearchResult::iterator it = result.begin();
           it != result.end();
           ++it ) {
         const SearchMatch* zipCode = *it;
         if ( zipCode == NULL ) {
            continue;
         }
         if ( ! ( zipCode->getType() & SEARCH_ZIP_CODES ) ) {
            continue;
         }
         if ( ! zipCode->getPoints().isFullMatch() ) {
            for( SearchUnitSearchResult::iterator itt = result.begin();
                 itt != result.end();
                 ++itt ) {
               const SearchMatch* currSearchMatch = *itt;
               if( currSearchMatch != NULL ) {
                  if( currSearchMatch->getPoints().isFullMatch() ) {
                     for( const SearchMatch* curReg = currSearchMatch;
                          curReg != NULL;
                          curReg = curReg->getRegion(0))
                     {
                        if( curReg->getID().second ==
                            currSearchMatch->getID().second )
                        {
                           // Remove not full zipcode
                           mc2dbg << "[SSSU]: Removing zipcode "
                                  << zipCode->getID()
                                  << endl;
                           delete *it;
                           *it = NULL;
                           ++nbrRemoved;
                           break;
                        } 
                     }
                  }
                  if( *it == NULL )
                     break;
               }
            }
         }
      }         
   }
   
   return result.size() - nbrRemoved;
}

void
SearchableSearchUnit::
convertMatchesToOverviewMatches(vector<OverviewMatch*>& result,
                                const SearchUnitSearchResult& tempRes,
                                uint8 nbrRemovedCharacters) const
{
   for( SearchUnitSearchResult::const_iterator it = tempRes.begin();
        it != tempRes.end();
        ++it ) {
      const SearchMatch* vanilla = *it;
      if ( vanilla == NULL ) {
         continue;
      }
      // Create the overviewMatch - copy the stuff in the
      // other match.
      OverviewMatch* om = new OverviewMatch( *vanilla );
      
      // Also look up the radius
      const SearchMapItem* searchItem =
         m_searchMap->getItemByItemID( om->getID() );
      om->setRadiusMeters( searchItem->getRadiusMeters( m_searchMap ) );
      om->setNbrRemovedCharacters(nbrRemovedCharacters);
      
      IDPair_t higherID( om->getID() );
      IDPair_t lowerID( m_searchMap->translateToLower(higherID) );
      // If the lower id cannot be found it should be an item
      // that consists of several other items, but where we only
      // want one hit. Send the overview id and let the server
      // expand the id later.
      if ( lowerID.isValid() ) {
         // It was possible to translate -> change the id of the
         // overview match.
         om->setMapID(lowerID.getMapID());
         om->setItemID(lowerID.getItemID());
      }
      
      // The overview id should always be overview ID.
      om->setOverviewID( higherID );      

      // Give the regions the same treatment as the match
      // itself.
      const int nbrRegions = om->getNbrRegions();
      for( int i = 0; i < nbrRegions; ++i ) {
         VanillaRegionMatch* region = om->getRegion(i);
         // Original map ID
         uint32 raddMapID = region->getMapID();
         uint32 raddItemID = region->getItemID();
         // Lookup the overview id
         IDPair_t rhigher(raddMapID, raddItemID);
         IDPair_t rlower(
            m_searchMap->translateToLower(rhigher));
         // If the lower id cannot be found it should be an item
         // that consists of several other items, but where we only
         // want one hit. Send the overview id and let the server
         // expand the id later.
         if ( rlower.isValid() ) {
            raddMapID = rlower.getMapID();
            raddItemID = rlower.getItemID();
         }
         
         region->setMapID(raddMapID);
         region->setItemID(raddItemID);
      }
      result.push_back(om);
   }
}

bool
SearchableSearchUnit::okForMoreZipCodeSearching( const MC2String& str )
{
   // Minimum length 3 when removing characters and 2 when there is
   // a digit in the string.
   const uint ZIP_CODE_MIN_LENGTH = 3;
   
   char* close = StringUtility::newStrDup(
      StringSearchUtility::convertIdentToExact( str ).c_str());

   const bool lengthOK = strlen(close) > ZIP_CODE_MIN_LENGTH;
   bool res = lengthOK ||
      ( ( strlen(close) > (ZIP_CODE_MIN_LENGTH - 1 ) ) &&
      StringUtility::containsDigit(close) );
   
   delete [] close;

   return res;
}

int
SearchableSearchUnit::
overviewSearch(vector<OverviewMatch*>& result,
               const UserSearchParameters& params,
               uint8 nbrRemovedCharacters) const
{
   // FIXME: Use TopRegions to remove some of the matches.
   SearchUnitSearchResult tempRes;
   
   // Use the other search function
   overviewSearch(tempRes, params);
   // Tricks for Zip codes. The zipcodes in e.g. GB do not have
   // complete names so we have to remove the end of the codes
   // until we find something.
   if ( params.getRequestedTypes() & SEARCH_ZIP_CODES ) {
      if ( tempRes.empty () &&
           okForMoreZipCodeSearching( params.getString() ) ) {
         // Copy the parameters, allow only zips and remove a character from
         // the string
         UserSearchParameters paramsCopy(params);
         paramsCopy.setRequestedTypes( SEARCH_ZIP_CODES );
         // Set restrictions so that the server will sort out the 
         // ones that are worst.
         paramsCopy.setRestriction(0x80);
         
         // Remove last character
         MC2String newStr( params.getSearchString(),
                           params.getString().size() - 1 );
         paramsCopy.setSearchString( newStr );
         mc2dbg << "[SSU]: " << MC2CITE(params.getString()) << "->"
                << MC2CITE(newStr) << endl;
         
         MC2_ASSERT ( newStr.size() < params.getString().size() );
         // Do the searching.
         overviewSearch( result, paramsCopy,
                         nbrRemovedCharacters+1 );      
      }
   }
   
   // Now convert the SearchMatches to OverviewMatches.
   convertMatchesToOverviewMatches(result, tempRes,
                                   nbrRemovedCharacters);
   
   return result.size();
}

int
SearchableSearchUnit::proximitySearch(SearchUnitSearchResult& result,
                                      const UserSearchParameters& params,
                                      const vector<IDPair_t>& itemIDs) const
{
   mc2dbg << "[SSU]: prox - nbrItems = " << itemIDs.size() << endl;
   // Create the query.
   SearchQuery query(m_searchMap, params);

   // Create fake matches for the items (as if they were found when searching)
   SearchResult internalResult(m_searchMap);

   for( vector<IDPair_t>::const_iterator it = itemIDs.begin();
        it != itemIDs.end();
        ++it ) {      
      const SearchMapItem* curItem = m_searchMap->getItemByItemID(*it);
      if ( curItem == NULL ) {
         mc2log << warn << "[SSU]: prox - Item " << *it << " not found in map"
                << endl;
         continue;
      }

      // filter out specific pois
      if ( params.isPOIRequest() ) {
         const SearchMapPOIInfo* poiInfo =
            m_searchMap->getPOIInfoFor( curItem );
         if ( poiInfo &&
              ! STLUtility::has( params.getPOITypes(),
                                 poiInfo->getItemSubType( m_searchMap ) ) ) {
            // skip this poi type since it's not in our
            // requested poi types
            continue;
         }
      }
      SearchMatchPoints* points = new SearchMatchPoints;
      const int nameNbr =
         curItem->getBestNameNbr(m_searchMap,
                                 query.getParams().getRequestedLanguage());

      SearchResultElement* curElement =
         new SearchResultElement(curItem,
                                 0,
                                 points,
                                 nameNbr,
                                 curItem->getNameInfo(m_searchMap, nameNbr),
                                 m_searchMap->getRights( curItem )
                                 );
      internalResult.addResult(curElement, query.getReqLang() );
   }

   // Convert the result to matches
   convertInternalResultToMatches(result, query, internalResult);

   // Sort the result.
   int nbrSorted = sortMatchesRemovingUnsorted( result, params );

   mc2dbg << "[SSU]: nbrSorted = " << nbrSorted << endl;

   // Done!
   return result.size();
}
