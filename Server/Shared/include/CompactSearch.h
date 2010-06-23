/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COMPACT_SEARCH_HIT 
#define COMPACT_SEARCH_HIT 

#include "config.h"
#include "LangTypes.h"
#include "MC2String.h"
#include "IDPairVector.h"
#include "MC2Coordinate.h"
#include "StringTable.h"
#include "CategoryTree.h"
#include "SearchTypes.h"
#include "ItemInfoEnums.h"

#include <vector>

/**
 * Describes a search heading.
 */
struct CompactSearchHitType {
   /// Checks if this object has been initiated.
   /// @return true if valid, else false.
   bool isValid() const { return ! m_name.empty(); }

   /// operator== compares to objects.
   /// @param other Object to compare.
   /// @return true if equal, false if not.
   bool operator==( const CompactSearchHitType& other ) const;

   
   /// operator+= merges two CompactSearchHitType:s to make it possible to do
   /// one search instead of one.
   /// @param other Add type to this instance.
   /// @return Reference to this object.
   CompactSearchHitType& operator+=( const CompactSearchHitType& other );
   
   /// Type of provider. YP and WP uses different search fields. 
   /// This enum tells the fields used by the service.
   enum provider_type_t {
      yellow_pages = 0,
      white_pages = 1
   };

   /// @return The image name
   MC2String getImageName() { return m_imageName; }
   
   /// @return the heading id.
   uint32 getHeading() { return m_heading; }


// Members:

   /// name in a specific language (i.e POI )
   MC2String m_name;        
   /// string code to be used for translation of the name
   StringTable::stringCode m_nameStringCode;
   /// describes the type of search hit in a specific language (e.g. yellow pages)
   MC2String m_type;
   /// string code for the type of the service (e.g. yellow pages/white pages)
   StringTable::stringCode m_typeStringCode;
   /// name of the image to display
   MC2String m_imageName;   
   /// Round number to search in ( 0 fast, 1 slow etc )
   uint32 m_round;          
   /// service id for this hit type. 0 for internal
   uint32 m_serviceID;      
   /// heading number
   uint32 m_heading; 
   /// Search types to include if internal search
   uint32 m_searchTypeMask; 
   /// top region id  MAX_UINT32 = internal search
   uint32 m_topRegionID;    
   /// native language for the search engine
   LangTypes::language_t m_language; 
   /// specific rights to search for or MAX_UINT64 if not
   /// search for rights
   uint64 m_mapRights; 
   /// whether the rights should be inverted
   bool m_invertRights; 
   /// search provider type, WP, YP etc
   provider_type_t  m_providerType;   
};

/// a vector of compact search hits types, i.e search headings
typedef std::vector<CompactSearchHitType> CompactSearchHitTypeVector;

/**
 * Holds information about a compact search. 
 * Fill in this structure and send it to SearchParserHandler.
 *
 * If area id is valid then the m_where field must be ignored.
 * If location is valid then the m_topRegion field must be ignored, and
 * the top region will come from the position.
 *
 * Non-empty m_categoryName means category search with m_categoryName as the
 * name of the category ( Restaurant, Airport etc ) and m_what as the name of 
 * a specific item in the category ( mcdonalds, starbucks etc ) and
 * m_where contains the location ( city, municipal etc ).
 *
 * If m_heading is not equal to -1 then the m_round will be ignored and
 * calculated from m_heading.
 *
 */
struct CompactSearch {
   CompactSearch():
      m_categoryIDs(),
      m_startIndex( 0 ),
      m_endIndex( 0 ),
      m_maxHits( 0 ),
      m_topRegionID( MAX_UINT32 ),
      m_round( MAX_UINT32 ),
      m_heading( -1 ),
      m_language( LangTypes::nbrLanguages ),
      m_distance( -1 ),
      m_findCategories( false ),
      m_smartCategory( true ),
      m_includeInfoItem( false ),
      m_oneResultList( false ),
      m_sorting( SearchTypes::DistanceSort ),
      m_itemInfoFilter( ItemInfoEnums::All )
   { }

   /**
    * @return True if the search arguments defines a category search.
    */
   bool isCategorySearch() const {
      // if category id is valid or category name is valid
      // then this is a category search.
      return 
         ! m_categoryIDs.empty() || 
         ! m_categoryName.empty() || 
         m_smartCategory;
   }

   /// Remove any leading and trailing whitespace from input strings.
   void cleanInput();

   /// @return true if the input parameters ( what, where and category id are
   /// valid ).
   bool validInput() const;

   MC2String m_what;     ///< name, company, street etc.
   MC2String m_where;    ///< location; city name, municipal name etc.

   typedef vector< CategoryTreeUtils::CategoryID > CategoryIDs;

   /** 
    * IDs of categories, this should be used instead of
    * category name if a full match should be found.
    */
   CategoryIDs m_categoryIDs;
   MC2String m_categoryName; //< category name, Restaurant, Airport etc
   IDPair_t m_areaID;    ///< specific area id, can be invalid
   uint32 m_startIndex ; ///< start index
   uint32 m_endIndex;    ///< ending index 
   uint32 m_maxHits;     ///< maximum number of hits
   uint32 m_topRegionID; ///< top region ID
   uint32 m_round;       ///< round (ignored if heading != -1 )
   int32 m_heading;      ///< heading
   LangTypes::language_t m_language; ///< current language
   /// holds position and angle
   struct Location {
      Location():m_angle( 0 ) { }
      MC2Coordinate m_coord; ///< position in mc2 
      uint16 m_angle;        ///< angle in degrees
      /// @return true if coordinate, i.e location is valid
      bool isValid() const {  
         return m_coord.isValid();
      }
   } m_location;
   int32 m_distance; ///< search radius in meters
   bool m_findCategories; ///< whether to search for matching categories or not.
   /// wheter we should look for categories if none are supplied.
   bool m_smartCategory;
   /// If to include poi/extra information in the search hits.
   bool m_includeInfoItem;
   bool m_oneResultList; ///< When true all results are merged into one result list
   SearchTypes::SearchSorting m_sorting; ///< The sort order to use in merged result list
   ItemInfoEnums::InfoTypeFilter m_itemInfoFilter; ///< Filter setting for ItemInfo
};

#endif // COMPACT_SEARCH_HIT_H
