/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCH_PARSER_HANDLER_H
#define SEARCH_PARSER_HANDLER_H

#include "config.h"
#include "CompactSearch.h"
#include "ParserHandler.h"
#include "LangTypes.h"
#include "MC2String.h"
#include "StringTable.h"
#include "ImageTable.h"
#include "ItemInfoEnums.h"

#include <map>
#include <set>
#include <iosfwd>

// Forward declarations
class ParserThread;
class SearchResultRequest;
class Request;
class SearchMatch;
class RequestWithStatus;
class ExternalSearchDesc;
class TopRegionMatch;
class CoordinateRequest;
class TopRegionRequest;
class SearchHeadingDesc;
class SearchRequestParameters;
class VanillaMatch;

/** 
 * Handles the "new" compact search with "fewer" parameters.
 *
 * A search is done by filling in CompactSearch structure and calling
 * compactSearch.
 *
 * It also handles the search hit images using getImageName from
 * a SearchMatch.
 * It can be used to determine what kind of headings the search might return
 * with getSearchHitTypes.
 *
 */
class SearchParserHandler : public ParserHandler {
public:
   /**
    * Describes a category in the category list for the client.
    */
   struct Category {

      typedef uint16 CategoryID;

      bool operator < ( const Category& other ) const {
         return m_name < other.m_name;
      }
      bool operator != ( const Category& cat ) const {
         return ! ( *this == cat );
      }

      bool operator == ( const Category& cat ) const {
         return cat.m_name == m_name;
      }

      CategoryID getCategoryID() const { return m_categoryID; }
      StringTable::stringCode getStringCode() const { return m_stringcode; }

      /// Internal name of the category
      MC2String m_name; 
      /// String code to translate the category
      StringTable::stringCode m_stringcode;
      /// Category id for the tree
      CategoryID m_categoryID; 
   };

   /// An invalid category instance
   static const Category INVALID_CATEGORY; 

   typedef set<Category> Categories;
   
   /// Struct for holding the needed data for the mapping
   /// of heading id during a search.
   class SearchResultsData {

   private: 
      /// Member for the search request.
      SearchResultRequest* m_searchResultReq;
      /// Member for the new heading hit type.
      CompactSearchHitType m_oneSearchHeadingHitType;

   public:
      /// Constructor
      explicit SearchResultsData(
         SearchResultRequest* request,                        
         const CompactSearchHitType& hitType = CompactSearchHitType());

      /// Destructor
      ~SearchResultsData();

      /// @return A pointer to search request
      SearchResultRequest* getSearchResultRequest() {
         return m_searchResultReq; }

      /// Access to the hit type object i.e. heading description.
      /// @return A reference to the hit type.
      const CompactSearchHitType& getOneSearchHeadingType() {
         return m_oneSearchHeadingHitType;
      }

      /// Tells if the heading shall be used instead of this object.
      /// @return Status of the hit type object. 
      bool useHeading() { return !m_oneSearchHeadingHitType.isValid(); }
   };

   /// map of heading number -> search result request
   typedef std::map<uint32, SearchResultsData* > SearchResults;

   /**
    * @param thread the thread in which this parser handler works
    * @param group group to which this parser belongs to
    */
   SearchParserHandler( ParserThread* thread,
                        ParserThreadGroup* group,
                        const CompactSearchHitTypeVector& headings =
                        CompactSearchHitTypeVector() );

   /**
    * Destructor of the SearchParserHandler class.
    */
   ~SearchParserHandler();

   /**
    * Composes a list of search hit types.
    * @param language translate the names to this language if possible.
    * @param descVersion description version ( = 0 for old java clients )
    * @return compact search hit types, example POI, street.
    */
   CompactSearchHitTypeVector getSearchHitTypes( LangType::language_t language, 
                                                 uint32 descVersion ) const;

   /**
    * Make a crc of search hit types.
    *
    * @param language The language to use.
    * @return The crc for search desc.
    */
   MC2String getSearchHitTypesCRC( LangTypes::language_t language ) const;

   /** 
    * Do a compact search with what/where + top region
    *
    * @param search search parameters
    * @return Containers with SearchRequest pointers
    */
   SearchResults compactSearch( const CompactSearch& search );

   /** 
    * Returns image name ( without file ending ) for a search match.
    * @param match The search hit to determine filename for
    * @param headingID The heading ID to fallback on if the match
    *                  did not have any image.
    * @return image name for the search match or default heading image name.
    */
   MC2String getImageName( const SearchMatch& match,
                           uint32 headingID = MAX_UINT32 ) const;

   /**
    * Returns the category image name (without file ending) for a search match.
    * @param match The search hit to determine category image for.
    * @return      An image name as string. Empty if no image exist.
    */
   MC2String getCategoryImageName( const SearchMatch& match ) const;

   /**
    * Returns the brand image name (without file ending) for a search match.
    * @param match The search hit to determine brand image for.
    * @return      An image name as string. Empty if no image exist.
    */
   MC2String getBrandImageName( const SearchMatch& match ) const;

   /**
    * Returns the heading/provider image name (without file ending) 
    * for a search match.
    * @param match The search hit to determine provider image for.
    * @return          An image name as string. Empty if no image exist.
    */
   MC2String getProviderImageName( const SearchMatch& match ) const;

   /**
    * Returns the heading/provider name for a search match.
    * @param match The search hit to determine provider name for.
    * @return The provider name.
    */
   MC2String getProviderName( const SearchMatch& match ) const;

   /**
    * Searches for a specific category in the category list. 
    * This is mainly used for converting old category search ( < wf8 )
    * names into category ids.
    * @param categoryName the full name of the category to search for
    * @return category that matched or INVALID_CATEGORY if it didn't find any.
    */
   Category findCategoryFromList( const MC2String& category ) const;

   /**
    * Same as above, temporary until a proper testing of the above function
    * works.
    */
   Category findCategoryFromListLowerCase( const MC2String& category ) const;

   /// Information about a region search at a certain point.
   struct RegionInfo {
      RegionInfo();
      ~RegionInfo();
      /// Information on which headings will be searched.
      CompactSearchHitTypeVector m_headings;
      /// Information about the top region at the position.
      const TopRegionMatch* m_topMatch;
   };

   /// Request information about a region.
   struct RegionRequest {
      MC2Coordinate m_position; ///< Coordinate in the position.
      /// Translate region name to this language.
      LangTypes::language_t m_language;

   };

   /**
    * Get region information about a position
    * @param request position anywhere in a region to which we want
    *                information about and the language.
    * @param info Information about the region, top region and which providers
    *             it has.
    */
   bool getRegionInfo( const RegionRequest& request, RegionInfo& info );

   /**
    * Adds country regions to the matches in req.
    *
    * @param params The search settings, things like language is used.
    * @param req The request to change.
    */
   void addCountryToResults( const SearchRequestParameters& params,
                             SearchResultRequest* req );

   /**
    * Adds country regions to the matches in results.
    *
    * @param params The search settings, things like language is used.
    * @param results The results to change.
    */
   void addCountryToResults( const SearchRequestParameters& params,
                             SearchResults& req );

   /**
    * Add country regions to the match.
    *
    * @param params The search settings, things like language is used.
    * @param result The result to change.
    */
   void addCountryToResults( const SearchRequestParameters& params,
                             SearchMatch* match );

   /**
    * Tries to get the heading for the match.
    *
    * @param match The match to find a heading for.
    * @return The heading for match, MAX_UINT32 if no match.
    */
   uint32 getHeadingForMatch( const SearchMatch* match ) const;

   /**
    * Add item information to internal search results.
    * @param infoTypeFilter Filter for which item infos to be returned.
    */
   void addItemInfo( SearchResults& req, LangTypes::language_t lang, 
                     ItemInfoEnums::InfoTypeFilter infoTypeFilter );

   /**
    * Add item information to internal search results.
    * @param infoTypeFilter Filter for which item infos to be returned.
    */
   void addItemInfo( SearchResultRequest* req, LangTypes::language_t lang,
                     ItemInfoEnums::InfoTypeFilter infoTypeFilter);

   /**
    * Return the searchTypeMask for the heading.
    *
    * @param headingID The heading id to get searchTypes for.
    * @return The searchType for the heading, 0 if no such heading.
    */
   uint32 getSearchTypeForHeading( uint32 headingID ) const;

   /**
    * Return the round for the heading.
    *
    * @param headingID The heading id to get searchTypes for.
    * @return The round for the heading, MAX_UINT32 if no such heading.
    */
   uint32 getRoundForHeading( uint32 headingID ) const;

   /**
    * Return the external source for the heading.
    *
    * @param headingID The heading id to get external source  for.
    * @param extID Set to the string for the source.
    * @return The external source  for the heading, MAX_UINT32 if no 
    *         such heading.
    */
   uint32 getExternalSourceForHeading( uint32 headingID,
                                       MC2String& extID ) const;

   /**
    * Tries to get the top region for the match.
    *
    * @param match The match to find top region for.
    * @return The top region for match, MAX_UINT32 if no match.
    */
   uint32 getTopRegionForMatch( const SearchMatch* match ) const;

private:


   /// Type of service ID
   typedef int32 ServiceID;
   /// Vector containg service ids
   typedef vector< ServiceID > ServiceIDs;
   /// Type for invalid service id
   static const int32 INVALID_SERVICE_ID;

   /**
    * creates search debit for user.
    * @param search search parameters
    * @param req the request that was successfull
    * @param extraString a string with all the headings searched in
    * @return true if debit was successfull
    */
   bool makeSearchDebit( const CompactSearch& search, 
                         Request& req,
                         const MC2String& extraString );

   /**
    * Executes an external search.
    * @param search specific search parameters
    * @param serviceID the ID of the service to be used
    * @return results from external search
    */
   SearchResults externalSearch( const CompactSearch& search, 
                                 ServiceID serviceID );

   /**
    * Execute a normal internal search.
    * @param search specific search parameters for normal search
    * @return results from normal search.
    */
   SearchResults normalSearch( const CompactSearch& search );

   /**
    * Translates clients category string to specified language.
    * @return translated category name
    */
   MC2String getCategoryString( const MC2String& clientCategory, 
                                const LangType& language ) const;

   /**
    * Determine which headings the "search" params with the current user can
    * search in and create empty headings for them.
    * @param search Search parameters.
    * @param serviceID ID for external service to be passed to
    *                     createEmptyHeadingsForExternal if "search" param is
    *                     an external search.
    * @return empty headings that the user can search in.
    */
   SearchResults createEmptyHeadings( const CompactSearch& search,
                                      int32 serviceID );

   /**
    * This creates empty headings for external services.
    * @see createEmptyHeadings
    * @param search Search parameters.
    * @param serviceID id for external service.
    * @return empty headings for external services. 
    */
   SearchResults 
   createEmptyHeadingsForExternal( const CompactSearch& search,
                                   int32 serviceID );

   
   /**
    * This creates the request.
    * @param searchOrgParam The original search parameters.
    * @param search The search parameters.
    * @param serviceID The service id.
    * @param serviceIDs Holds service IDs for each request.
    * @param requests The requests we create.
    */
   void createRequests( const CompactSearch& searchOrgParam,
                        CompactSearch& search,
                        ServiceID serviceID,
                        ServiceIDs& serviceIDs, 
                        vector< RequestWithStatus* >& requests );

   /**
    * Sets the top region field
    */
   bool setTopRegionField( map<int, MC2String>& searchFields , uint32 topRegion, uint32 serviceId );

   /// @return Search heading descriptor for the current user
   inline const SearchHeadingDesc* getHeadingDesc() const;

   /**
    * Handle the smart searching with categories in what field.
    *
    * @param search The CompactSearch, what field may be updated.
    */
   void handleSmartCategorySearch( CompactSearch& search,
                                   CategoryTreeUtils::CategorySet& categories,
                                   const CategoryTreeUtils::CategoryTree& tree );

   /**
    * Determine image name from client type and poi type.
    * @param poiType Type specific image name.
    * @return Image name.
    */
   MC2String getClientSpecificImageName( uint32 poiType ) const;

   /// Map of VanillaMatches per mapID.
   typedef map < uint32, vector< VanillaMatch* > > InfoMap;

   /// Get information and fill into the matches.
   /// @param infoTypeFilter Filter for which item infos to be returned.
   void getItemInfo( InfoMap& infoMap, LangTypes::language_t lang,
                     ItemInfoEnums::InfoTypeFilter infoTypeFilter);

   /// all categories handled by our internal search
   Categories m_categories;

   /// ugly hack, until a proper testing can be done
   Categories m_lowerCaseCategories;

   /// map ImageSet to heading descriptors
   vector < pair<ImageTable::ImageSet, SearchHeadingDesc*> > m_headings;
};

#endif
