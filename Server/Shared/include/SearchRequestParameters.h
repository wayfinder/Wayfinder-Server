/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHREQUESTPARAMETERS_H
#define SEARCHREQUESTPARAMETERS_H

#include "SearchTypes.h"
#include "ItemTypes.h"
#include "MapRights.h"
#include "StringTable.h"

#include <vector>

class Packet;

/**
 *   Class for keeping search parameters.
 *   This is the preferred method of telling the SearchResultRequests
 *   how to search. The parameters are also sent to the SearchModule
 *   in the SearchRequestPacket.
 *   <br />
 */
class SearchRequestParameters {
public:
   typedef std::set<uint16> Categories;

   /**
    *   Creates a new SearchRequestParameters with the default 
    *   values. <br />
    *   nbrSortedHits = MAX_UINT8 <br />
    *   matchType = SearchTypes::CloseMatch; <br />
    *   stringPart = SearchTypes::Beginning; <br /> 
    *   sortingType = SearchTypes::ConfidenceSort; <br />
    *   sortingOrigin = InvalidCoordinate <br />
    *   bboxrequested = false <br /> 
    *   lookupCoordinates = false <br />
    *   uniqueOrFull = false <br /> 
    *   searchOnlyIfUniqueOrFull = true <br /> 
    *   regionsinmatches = SEARCH_MUNICIPALS, BUAS and CITY_PARTS <br />
    *   overviewregionsinmatches = SEARCH_MUNICIPALS,
    *   BUAS and CITY_PARTS <br />
    *   locationtype = SEARCH_MUNICIPALS, BUAS and CITY_PARTS <br />
    *   reqLanguage = StringTable::SWEDISH; <br />
    *   dbMask = SearchTypes::DefaultDB; <br /> 
    *   addStreetNameToCompanies = false; <br />
    *   editdistancecutoff = 0 <br /> 
    *   minnbrhits = 0 <br /> 
    *   endHitIndex = 500 <br />
    *   addSynonymnames to some pois = false <br />
    */
   SearchRequestParameters():
      m_mapRights( ~MapRights() ),
      m_invertedRight( false ),
      m_expandCityCenter( true ),
      m_includeTopRegionInArea( false ),
      m_shouldIncludeCategoriesInResult( false )
   {
      m_nbrSortedHits = MAX_UINT8;
      m_matchType   = SearchTypes::CloseMatch;
      m_stringPart  = SearchTypes::Beginning;
      m_sortingType = SearchTypes::ConfidenceSort;
      m_bboxRequested = false;
      m_lookupCoordinates = false;
      m_uniqueOrFull = false;
      m_searchOnlyIfUniqueOrFull = true;
      m_regionsInMatches = SEARCH_MUNICIPALS|
                           SEARCH_BUILT_UP_AREAS|
                           SEARCH_CITY_PARTS;
      m_overviewRegionsInMatches = SEARCH_MUNICIPALS|
                                   SEARCH_BUILT_UP_AREAS|
                                   SEARCH_CITY_PARTS |
                                   SEARCH_COUNTRIES;
      m_locationType = SEARCH_MUNICIPALS|
                       SEARCH_BUILT_UP_AREAS|
                       SEARCH_CITY_PARTS;
      m_searchForTypes = SEARCH_STREETS | SEARCH_COMPANIES;
      setRequestedLang(StringTable::SWEDISH);
      m_dbMask = SearchTypes::DefaultDB;
      m_addStreetNameToCompanies = false;
      m_editDistanceCutoff = 0;
      m_nbrHits = 0;
      m_endHitIndex = 500;
      m_addSynonymNameToPOIs = false;
   }

   /**
    *   Saves the parameters into the packet.
    */
   int save(Packet* packet, int& pos) const;

   /**
    *   Loads the parameters from the packet.
    */
   int load(const Packet* packet, int& pos);

   /**
    *   Returns the size of this object in a packet.
    */
   int getSizeInPacket() const;
   
   /**
    *   Returns the type of matching to use.
    */
   inline SearchTypes::StringMatching getMatchType() const;

   /**
    *   Sets the type of stringmatching to use.
    */
   inline void setMatchType(SearchTypes::StringMatching smat);
      
   /**
    *   Returns the sorting type to use.
    */ 
   inline SearchTypes::SearchSorting getSortingType() const;

   /**
    *   Sets sorting type.
    */
   inline void setSortingType(SearchTypes::SearchSorting st);
   
   /**
    *   Returns the string part.
    */
   inline SearchTypes::StringPart getStringPart() const;

   /**
    *   Sets the string part,
    */
   inline void setStringPart(SearchTypes::StringPart sp);
   
   /**
    *   Returns the minimum number of hits.
    */
   inline int getMinNbrHits() const;

   /**
    *   Sets the try harder flag ( minnbrhits = 1 )
    */
   inline void setTryHarder(bool tryHarder);
   
   /**
    *   @return the types to search for. Streets, companies etc.
    */
   inline uint32 getSearchForTypes() const;

   /**
    *   Sets the types to search for in ordinary matches.
    */
   inline void setSearchForTypes(uint32 searchTypes);

   /**
    *   Sets the types to search for in overview matches.
    */
   inline void setSearchForLocationTypes(uint32 searchTypes);

   /**
    *   Returns which location types to search for.
    */
   inline uint32 getSearchForLocationTypes() const;
   
   /**
    *   Returns the regions types to return in the matches.
    */
   inline uint32 getRegionsInMatches() const;

   /**
    *   Sets the regions to return in ordinary matches.
    */
   inline void setRegionsInMatches(uint32 searchTypes);
   
   /**
    *   Returns the regions types to return in the overview matches.
    */
   inline uint32 getRegionsInOverviewMatches() const;

   /**
    *   Sets the wanted regions in the overview matches.
    */
   inline void setRegionsInOverviewMatches(uint32 searchTypes);
   
   /**
    *   Returns the requested language.
    */
   LangTypes::language_t getRequestedLang() const;

   /**
    *   Sets the language using StringTable::languageCode.
    */
   void setRequestedLang(StringTable::languageCode lang);

   /**
    *   Sets the language using LangTypes::language_t.
    */
   void setRequestedLang(LangTypes::language_t lang);

   /**
    *   Set to true if coordinates should be looked up.
    */
   void setLookupCoordinates(bool lookupCoordinates);
   
   /**
    *   Returns the number of hits to sort in the SM.
    */
   inline int getNbrSortedHits() const;

   /**
    *   Sets the number of hits to sort.
    */
   inline void setNbrSortedHits(int nbrSorted);

   /**
    *   Returns the edit distance cut off.
    */
   inline int getEditDistanceCutOff() const;

   /**
    *   Returns the database mask of the parameters.
    */
   inline uint8 getDBMask() const;

   /**
    *   Sets the database mask of the parameters.
    */
   inline void setDBMask(uint8 dbMask);

   /// @return map rights to search for
   inline const MapRights& getMapRights() const;

   /// Sets the map rights to search for
   inline void setMapRights( const MapRights& rights );
   /**
    * sets whether the rights should be excluded or included
    * If inverted (true), the search results will be only those items that
    * does not have the specific right. If false, the rights must
    * match the item rights.
    * @param invert true if rights should be inverted
    */
   void setInvertRights( bool invert );
   /**
    * @see setInvertRights
    */
   inline bool invertRights() const;

   /**
    *   Sets if street names should be added to company matches.
    */
   inline void setAddStreetNamesToCompanies(bool yes);

   /**
    *   Returns true if the SearchModule should add the names
    *   streets to the company matches.
    */
   inline bool getAddStreetNamesToCompanies() const;

   /**
    *   Returns the index of the last hit, i.e. the
    *   number of hits needed to display the last hit.
    *   If e.g. a client can show 7 hits and want to
    *   show hits 0-6, this should be 7.
    */
   inline uint32 getEndHitIndex() const;

   /**
    *   Sets the endHitIndex.
    */
   inline void setEndHitIndex(uint32 lastHit);

   /**
    *   Returns true if the bounding box has been requested.
    */
   inline bool getBBoxRequested() const;

   /**
    *   Returns true if coordinates should be looked up.
    */
   inline bool getLookupCoordinates() const;

   /**
    *   Returns true if the SearchRequest should only 
    *   keep searching in the overview matches if the number
    *   of overview matches is one.
    */
   inline bool searchOnlyIfUniqueOrFull() const;

   /**
    *   True if all other matches should be removed if there is one
    *   match that is is a full match.
    */
   inline bool uniqueOrFull() const;


   /**
    *   Set the uniqueOrFull value.
    */
   inline void setUniqueOrFull( bool value );

   /**
    *   Returns true if a synonym name should be added to
    *   some poi:s that are known to be difficult to
    *   identify otherwise.
    */
   inline bool getAddSynonymNameToPOIs() const;

   /**
    *   Sets whether to add synonym name to some pois.
    */
   inline void setAddSynonymNameToPOIs(bool yesno);

   /**
    *   Returns the categories to return in the matches.
    */
   inline const vector<uint32>& getCategoriesInMatches() const;

   /**
    *   Sets the categories to return in the matches.
    */
   inline void setCategoriesInMatches(vector<uint32> categories);
   /// @param categories the categories to use.
   inline void setCategories( const Categories& categories );

   /// @return categories
   inline const Categories& getCategories() const;
   /// Expand city center if there is one overview hit and no item hits.
   inline bool expandCityCenter() const;
   inline void setExpandCityCenter( bool expandCityCenter );

   typedef set< ItemTypes::pointOfInterest_t > POITypeSet;

   /**
    * Set a set of point of interest types to include in the search.
    * This will set the search as a "fetch all poi"-search type.
    * The "all poi" includes only the types specified by \c poiTypes.
    * @param poiTypes POI types to search for.
    */
   inline void setPOITypes( const POITypeSet& poiTypes );

   /**
    * @see setPOITypes
    * @return point of interest set to filter on.
    */
   inline const POITypeSet& getPOITypes() const;
   /// Set include top region in area flag.
   inline void setIncludeTopRegionInArea( bool value );

   /// @return true if top region should be included in search_area
   inline bool shouldIncludeTopRegionInArea() const;

   /// Set whether to include category ids in the result.
   void setShouldIncludeCategoriesInResult( bool value ) {
      m_shouldIncludeCategoriesInResult = value;
   }

   /// @return Whether to include category ids in the result.
   bool shouldIncludeCategoriesInResult() const {
      return m_shouldIncludeCategoriesInResult;
   }

private:
   /** String matching */
   SearchTypes::StringMatching m_matchType;
   
   /** String part */
   SearchTypes::StringPart m_stringPart;
   
   /** Type of sorting */
   SearchTypes::SearchSorting m_sortingType;
   
   /** Requested region types in answer for detailed search */
   uint32 m_regionsInMatches;
   
   /** Requested region types in answer to overview search */
   uint32 m_overviewRegionsInMatches;

   /** A vector with the categories to search for */
   vector<uint32> m_categoriesInMatches; 
   
   /** True if the search is a usersearch */
   bool m_isUserSearch;
   
   /** Cutoff for edit distance */
   uint16 m_editDistanceCutoff;
   
   /**
    *   Requested language. Is really LangTypes::language_t. But SearchHandler
    *   must be changed first.
    */
   LangTypes::language_t m_reqLanguage;
   
   /** Number of sorted hits */
   uint8 m_nbrSortedHits;
   
   /**
    *   Minimum number of hits.
    *   Change name of this when searchandler is changed
    */
   uint8 m_nbrHits;
   
   /// If locations must be unique of full match to match.
   bool m_uniqueOrFull;
   
   /// If string search should be sent when location isn't unique of full
   bool m_searchOnlyIfUniqueOrFull;
   
   /// True if coordinates should be looked up.
   bool m_lookupCoordinates;
   
   /// True if bboxes should be looked up too.
   bool m_bboxRequested;
   
   /// Type of location to search for when overview searching.
   uint32 m_locationType;
   
   /// Database mask
   uint8 m_dbMask;
   /// specific map rights mask to search for
   MapRights m_mapRights;
   /** whether the rights should be matched as including or if they should be
    * exclude the matching search itemx
    */
   bool m_invertedRight;

   /// True if streetnames should be added to companies
   bool m_addStreetNameToCompanies;
   
   /// Thing to search for. Should be uint32, but SearchHandler...
   uint16 m_searchForTypes;

   /// The index after last index displayed in client
   uint32 m_endHitIndex;

   /// True if synonym names should be added to pois
   bool m_addSynonymNameToPOIs;

   /// ids of all categories to search in
   Categories m_categories; 

   /// Expand city center if there is one overview hit and no item hits.
   bool m_expandCityCenter;

   /**
    * A unique set of poi types to search for.
    * @see setPOITypes
    */
   POITypeSet m_poiTypes;

   /// Whether to include top region id in search area.
   bool m_includeTopRegionInArea;

   /// Whether to include categories in the result.
   bool m_shouldIncludeCategoriesInResult;

   /// FIXME: SearchHandler is friend for now.
   friend class SearchHandler;
};

// - Implementation of inlined methods.

inline void 
SearchRequestParameters::setExpandCityCenter( bool expandCityCenter ) {
   m_expandCityCenter = expandCityCenter;
}


inline bool 
SearchRequestParameters::expandCityCenter() const {
   return m_expandCityCenter;
}

inline SearchTypes::StringMatching
SearchRequestParameters::getMatchType() const
{
   return m_matchType;
}

inline void
SearchRequestParameters::setMatchType(SearchTypes::StringMatching sm)
{
   m_matchType = sm;
}

inline SearchTypes::SearchSorting
SearchRequestParameters::getSortingType() const
{
   return m_sortingType;
}

inline void
SearchRequestParameters::setSortingType(SearchTypes::SearchSorting ss)
{
   m_sortingType = ss;
}

inline SearchTypes::StringPart
SearchRequestParameters::getStringPart() const
{
   return m_stringPart;
}

inline void
SearchRequestParameters::setStringPart(SearchTypes::StringPart sp)
{
   m_stringPart = sp;
}

inline int
SearchRequestParameters::getMinNbrHits() const
{
   return m_nbrHits;
}

inline void
SearchRequestParameters::setTryHarder(bool tryHarder)
{
   if ( tryHarder ) {
      m_nbrHits = 1;
   } else {
      m_nbrHits = 0;
   }
}

inline uint32
SearchRequestParameters::getSearchForTypes() const
{
   return m_searchForTypes;
}

inline void
SearchRequestParameters::setSearchForTypes(uint32 searchtypes)
{
   m_searchForTypes = searchtypes;
}

inline void
SearchRequestParameters::setSearchForLocationTypes(uint32 searchtypes)
{
   m_locationType = searchtypes;
}

inline uint32
SearchRequestParameters::getSearchForLocationTypes() const
{
   return m_locationType;
}

inline uint32
SearchRequestParameters::getRegionsInMatches() const
{
   return m_regionsInMatches;
}

inline void
SearchRequestParameters::setRegionsInMatches(uint32 searchTypes)
{
   m_regionsInMatches = searchTypes;
}

inline uint32
SearchRequestParameters::getRegionsInOverviewMatches() const
{
   return m_overviewRegionsInMatches;
}

inline void
SearchRequestParameters::setRegionsInOverviewMatches(uint32 searchTypes)
{
   m_overviewRegionsInMatches = searchTypes;
}

inline LangTypes::language_t
SearchRequestParameters::getRequestedLang() const
{
   return m_reqLanguage;
}

inline const vector<uint32>&
SearchRequestParameters::getCategoriesInMatches() const
{
   return m_categoriesInMatches;
}

inline void
SearchRequestParameters::setCategoriesInMatches( vector<uint32> categories ) 
{
   m_categoriesInMatches = categories;
}


inline void
SearchRequestParameters::setCategories( const Categories& categories ) 
{
   m_categories = categories;
}

inline const SearchRequestParameters::Categories& 
SearchRequestParameters::getCategories() const {
   return m_categories;
}

inline void
SearchRequestParameters::setRequestedLang(LangTypes::language_t lang)
{
   m_reqLanguage = lang;
}

inline void
SearchRequestParameters::setLookupCoordinates(bool lookup)
{
   m_lookupCoordinates = lookup;
}

inline void
SearchRequestParameters::setRequestedLang(StringTable::languageCode lang)
{
   setRequestedLang(ItemTypes::getLanguageCodeAsLanguageType(lang));
}

inline int
SearchRequestParameters::getNbrSortedHits() const
{
   return m_nbrSortedHits;
}

inline void
SearchRequestParameters::setNbrSortedHits(int nbrSorted)
{
   m_nbrSortedHits = nbrSorted;
}

inline int
SearchRequestParameters::getEditDistanceCutOff() const
{
   return m_editDistanceCutoff;
}

inline uint8
SearchRequestParameters::getDBMask() const
{
   return m_dbMask;
}

inline void
SearchRequestParameters::setDBMask(uint8 dbMask)
{
   m_dbMask = dbMask;
}

inline const MapRights&
SearchRequestParameters::getMapRights() const 
{
   return m_mapRights;
}

inline void
SearchRequestParameters::setMapRights( const MapRights& rights ) 
{
   m_mapRights = rights;
}

inline void SearchRequestParameters::setInvertRights( bool invert ) {
   m_invertedRight = invert;
}

inline bool SearchRequestParameters::invertRights() const {
   return m_invertedRight;
}

inline void
SearchRequestParameters::setAddStreetNamesToCompanies(bool yes)
{
   m_addStreetNameToCompanies = yes;
}

inline bool
SearchRequestParameters::getAddStreetNamesToCompanies() const
{
   return m_addStreetNameToCompanies;
}

inline uint32
SearchRequestParameters::getEndHitIndex() const
{
   return m_endHitIndex;
}

inline void
SearchRequestParameters::setEndHitIndex(uint32 endIdx)
{
   m_endHitIndex = endIdx;
}

inline bool
SearchRequestParameters::getBBoxRequested() const
{
   return m_bboxRequested;
}

inline bool
SearchRequestParameters::getLookupCoordinates() const
{
   return m_lookupCoordinates;
}

inline bool
SearchRequestParameters::searchOnlyIfUniqueOrFull() const
{
   return m_searchOnlyIfUniqueOrFull;
}

inline bool
SearchRequestParameters::uniqueOrFull() const
{
   return m_uniqueOrFull;
}


inline void 
SearchRequestParameters::setUniqueOrFull( bool value ) {
   m_uniqueOrFull = value;
}

inline bool
SearchRequestParameters::getAddSynonymNameToPOIs() const
{
   return m_addSynonymNameToPOIs;
}

inline void
SearchRequestParameters::setAddSynonymNameToPOIs(bool yesno)
{
   m_addSynonymNameToPOIs = yesno;
}

/**
 * Set a set of point of interest types to include in the search.
 * This will set the search as a "fetch all poi"-search type.
 * The "all poi" includes only the types specified by \c poiTypes.
 * @param poiTypes POI types to search for.
 */
inline void
SearchRequestParameters::setPOITypes( const POITypeSet& poiTypes ) {
   m_poiTypes = poiTypes;
}


inline const SearchRequestParameters::POITypeSet&
SearchRequestParameters::getPOITypes() const {
   return m_poiTypes;
}

/// Set include top region in area flag.
inline void SearchRequestParameters::setIncludeTopRegionInArea( bool value ) {
   m_includeTopRegionInArea = value;
}

/// @return true if top region should be included in search_area
inline bool SearchRequestParameters::shouldIncludeTopRegionInArea() const {
   return m_includeTopRegionInArea;
}

#endif




