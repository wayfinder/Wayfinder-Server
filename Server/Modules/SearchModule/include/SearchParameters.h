/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHPARAMETERS_H
#define SEARCHPARAMETERS_H

#include "config.h"

#include <set>
#include <vector>

#include "MC2String.h"
#include "ItemTypes.h"
#include "SearchTypes.h"
#include "MapRights.h"

#include "MC2Coordinate.h"
#include "UserRightsMapInfo.h"

class VanillaSearchRequestPacket;
class ProximitySearchRequestPacket;
class OverviewSearchRequestPacket;
class SearchRequestPacket;
class MC2BoundingBox;
class IDPair_t;


/**
 *   Class for collecting all the parameters of a search
 *   so that there will be less inparameters for a lot of
 *   functions.
 *   FIXME: Some of this stuff isn't used anymore. Remove.
 */
class VanillaSearchParameters {
public:
   typedef uint16 CategoryID;
   typedef set<CategoryID> Categories;
   /**
    *   Destructor.
    */
   virtual ~VanillaSearchParameters();

   /**
    *   Returns the minimum number of hits.
    *   Currently mostly used as a flag telling the
    *   searchmodule that it can do some tricks to get
    *   more searchresults.
    */
   inline int getMinNbrHits() const;

   /**
    *   Sets the minimum number of hits to try to return.
    */
   inline void setMinNbrHits(int nbr);

   /**
    *   Returns the maximum number of hits that shall be returned.
    */
   inline int getEndHitIndex() const;

   /**
    *   Returns the maximum number of hits that shall be returned.
    */
   inline void setEndHitIndex(int nbr);
   
   /**
    *   Returns the searchString.
    */
   inline const char* getSearchString() const;

   /**
    *   Returns the searchString as an MC2String.
    */
   inline const MC2String& getString() const;
   
   /**
    *   Sets a new searchString.
    */
   inline void setSearchString(const MC2String& newVal);

   /**
    *   Returns the type of matching to use.
    */
   inline SearchTypes::StringMatching getMatching() const;

   /**
    *   Sets matching.
    *   @see StringMatching::userSearch
    */
   inline void setMatching(SearchTypes::StringMatching matching);
   
   /**
    *   Returns the type of sorting requested.
    */
   inline SearchTypes::SearchSorting getSorting() const;

   /**
    *   Sets the type of sorting requested.
    */
   inline void setSorting(SearchTypes::SearchSorting s);

   /**
    *   Returns the part-of-string matching type.
    */
   inline SearchTypes::StringPart getStringPart() const;

   /**
    *   Sets the part-of-string matching.
    *   @see StringSearch::reduceRestrictions.
    */
   inline void setStringPart(SearchTypes::StringPart part);
   
   /**
    *   Returns the requested language as an uint32.
    */
   inline LangTypes::language_t getRequestedLanguage() const;

   /**
    *   Returns current restriction (zero means ok, non-zero means
    *   remove if there are others with lower rest).
    */
   inline uint8 getRestriction() const;

   /**
    *   Sets current restriction.
    */
   void setRestriction(uint8 newRest);

   /**
    *   Returns the addpoints parameter.
    */
   inline uint8 getAddPoints() const;

   /**
    *   Returns the editdistancecutoff.
    */
   inline uint16 getEditDistanceCutoff() const;

   /**
    *   Sets new editdistancecutoff.
    */
   inline void setEditDistanceCutoff(uint16 newCutoff);
   
   /**
    *   Returns the number of masks (or mask itemids).
    */
   inline int getNbrMasks() const;

   /**
    *   Returns a pointer to the mask itemids.
    */
   inline const uint32* getMaskItemIDs() const;

   /**
    *   Returns the mask for requested types.
    */
   inline uint32 getRequestedTypes() const;

   /**
    *   Returns the mask for requested types.
    */
   inline void setRequestedTypes(uint32 types);

   /**
    *   Returns the mapid.
    */
   inline uint32 getMapID() const;

   /**
    *   Returns the allowed return region types.
    */
   inline uint32 getReturnRegionTypes() const;

   /**
    *   Returns true if the name of the street
    *   should be added to poi-matches.
    */
   inline bool getAddSSINameToCompanies() const;

   /**
    *   Returns the sort origin.
    */
   inline const MC2Coordinate& getSortOrigin() const;

   /**
    *   Returns a reference to the items to expand in
    *   proximity "search".
    */
   inline const vector<IDPair_t>& getProximityItems() const;

   /**
    *   Returns true if synonym names should be added to the
    *   names of poi:s.
    */
   inline bool getAddSynonymNameToPOIs() const;

   inline vector<uint32> getReturnCategories() const;

   /**
    * @see SearchRequestParameters
    * @return the maprights to search for
    */
   inline const MapRights& getMapRights() const;
   inline bool invertRights() const;

   void setMapRights( const MapRights& rights ) { m_mapRights = rights; }
   void setInvertRights( bool invertRights ) { m_invertRights = invertRights; }
   inline const Categories& getCategories() const { 
      return m_categories; 
   }

   /// Set the point of interest types to fetch.
   void setPOITypes( const set< uint32 >& poiTypes ) {
      m_poiTypes = poiTypes;
   }

   /// @return point of interest types to search for.
   const set< uint32 >& getPOITypes() const {
      return m_poiTypes;
   }

   /// @return true if the request is a poi info fetch.
   bool isPOIRequest() const {
      return ! m_poiTypes.empty();
   }

protected:
   /// Specific poi types to search for.
   set< uint32 > m_poiTypes;

   /**
    *   May only be used by the subclasses.
    */
   VanillaSearchParameters() {}

   /**
    *   Not implemented. Here to warn.
    */
   VanillaSearchParameters& operator=(const VanillaSearchParameters& other);

   /// The locations to search in
   vector<uint32> m_regionIDs;

   /// The categories to search for
   vector<uint32> m_returnCategories;
   
   /**
    *   The search string.
    */
   MC2String m_searchString;

   /// Specific map rights for the item   
   MapRights m_mapRights;
   /// if the rights should be excluded
   bool m_invertRights;

   /// name of the category to search for
   Categories m_categories;

   /**
    *   The minimum number of hits to return.
    */
   int m_minNbrHits;

   /**
    *   The maximum number of hits to return.
    */
   int m_endHitIndex;
   
   /**
    *   The type of matching to use.
    */
   SearchTypes::StringMatching m_matching;

   /**
    *   The part of the string to match.
    */
   SearchTypes::StringPart m_stringPart;

   /**
    *   The sorting type.
    */
   SearchTypes::SearchSorting m_sorting;

   /**
    *   I think it should be streets or companies or something.
    */
   uint32 m_requestedTypes;

   /**
    *   The preferred language.
    */
   uint16 m_language;

   /**
    *   Don't know.
    */
   uint16 m_editDistanceCutoff;

   /**
    *   Don't know.
    */
   uint8 m_addPoints;

   /**
    *    The id of the map to searh in.
    */
   uint32 m_mapID;

   /**
    *    Regiontypes to return.
    */
   uint32 m_returnRegionTypes;

   /**
    *    True if the addresses of the POI:s should be added to the result.
    */
   bool m_addSSIToCompanies;
   
   /**
    *    Vector of boundingboxes to search in besides the
    *    maskitems.
    */
   vector<MC2BoundingBox> m_bboxes;

   /**
    *    Origin from which to sort matches.
    */
   MC2Coordinate m_sortOrigin;

   /**
    *    ID:s of items to convert to matches. (Proximity)
    */
   vector<IDPair_t> m_itemsToConvertToMatches;

   /**
    *    If true, a synonym name will be added to some poi-types.
    */
   bool m_addSynonymNameToPOIs;

   /**
    *    If set to something, something has been done to the matches.
    */
   uint8 m_restriction;
   
};

class UserSearchParameters : public VanillaSearchParameters {

public:

   UserSearchParameters(const SearchRequestPacket* packet);   
   inline int getMinNbrSorted() const;
   
   /**
    *   Returns the rights for the current user.
    */
   inline const UserRightsMapInfo& getRights() const {
      return m_rights;
   }

protected:

   UserSearchParameters() {};

   /// The minimum number of sorted matches.
   int m_minNbrSorted;
   /// The user rights for the map(s).
   UserRightsMapInfo m_rights;
};

// -- Inlined methods are implemented below the line

inline int
VanillaSearchParameters::getMinNbrHits() const
{
   return m_minNbrHits;
}

inline void
VanillaSearchParameters::setMinNbrHits(int nbr)
{
   m_minNbrHits = nbr;
}

inline int
VanillaSearchParameters::getEndHitIndex() const
{
   return m_endHitIndex;
}

inline void
VanillaSearchParameters::setEndHitIndex(int nbr) 
{
   m_endHitIndex = nbr;
}

inline const char*
VanillaSearchParameters::getSearchString() const
{
   return m_searchString.c_str();
}

inline const MC2String&
VanillaSearchParameters::getString() const
{
   return m_searchString;
}

inline void
VanillaSearchParameters::setSearchString(const MC2String& newStr)
{
   m_searchString = newStr;
}

inline SearchTypes::StringMatching
VanillaSearchParameters::getMatching() const
{
   return m_matching;
}

inline void
VanillaSearchParameters::setMatching(SearchTypes::StringMatching matching)
{
   m_matching = matching;
}

inline SearchTypes::SearchSorting
VanillaSearchParameters::getSorting() const
{
   return m_sorting;
}

inline void
VanillaSearchParameters::setSorting(SearchTypes::SearchSorting s)
{
   m_sorting = s;
}

inline SearchTypes::StringPart
VanillaSearchParameters::getStringPart() const
{
   return m_stringPart;
}

inline void
VanillaSearchParameters::setStringPart(SearchTypes::StringPart part)
{
   m_stringPart = part;
}

inline LangTypes::language_t
VanillaSearchParameters::getRequestedLanguage() const
{
   return LangTypes::language_t(m_language);
}

inline uint8
VanillaSearchParameters::getRestriction() const
{
   return m_restriction;
}

inline uint8
VanillaSearchParameters::getAddPoints() const
{
   return m_addPoints;
}

inline uint16
VanillaSearchParameters::getEditDistanceCutoff() const
{
   return m_editDistanceCutoff;
}

inline void
VanillaSearchParameters::setEditDistanceCutoff(uint16 newCutoff)
{
   m_editDistanceCutoff = newCutoff;
}

inline int
VanillaSearchParameters::getNbrMasks() const
{
   return m_regionIDs.size();
}


inline const uint32*
VanillaSearchParameters::getMaskItemIDs() const
{
   // This is perhaps ugly
   return &m_regionIDs.front();
}

inline vector<uint32>
VanillaSearchParameters::getReturnCategories() const
{
   return m_returnCategories;
}


inline const MapRights&
VanillaSearchParameters::getMapRights() const
{
   return m_mapRights;
}

inline bool
VanillaSearchParameters::invertRights() const 
{
   return m_invertRights;
}

inline uint32
VanillaSearchParameters::getRequestedTypes() const
{
   return m_requestedTypes;
}

inline void
VanillaSearchParameters::setRequestedTypes(uint32 types)
{
   m_requestedTypes = types;
}

inline uint32
VanillaSearchParameters::getMapID() const
{
   return m_mapID;
}

inline uint32
VanillaSearchParameters::getReturnRegionTypes() const
{
   return m_returnRegionTypes;
}

// -- User

inline int
UserSearchParameters::getMinNbrSorted() const
{
   return m_minNbrSorted;
}

inline bool
VanillaSearchParameters::getAddSSINameToCompanies() const
{
   return m_addSSIToCompanies;
}

inline const MC2Coordinate&
VanillaSearchParameters::getSortOrigin() const
{
   return m_sortOrigin;
}

inline const vector<IDPair_t>&
VanillaSearchParameters::getProximityItems() const
{
   return m_itemsToConvertToMatches;
}

inline bool
VanillaSearchParameters::getAddSynonymNameToPOIs() const
{
   return m_addSynonymNameToPOIs;
}

#endif
