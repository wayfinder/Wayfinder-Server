/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SearchParserHandler.h"
#include "CompactSearch.h"
#include "StringTable.h"
#include "SearchResultRequest.h"
#include "SearchRequestParameters.h"
#include "ExternalSearchRequestData.h"
#include "ExternalSearchRequest.h"
#include "SearchRequest.h"
#include "ParserThread.h"
#include "ExternalSearchConsts.h"
#include "ExternalSearchHelper.h"
#include "ParserThreadGroup.h"
#include "ClosestSearchRequest.h"
#include "SearchMatch.h"
#include "DeleteHelpers.h"
#include "ServerTileMapFormatDesc.h"
#include "ItemTypes.h"
#include "FixedSizeString.h"
#include "UserData.h"
#include "STLStringUtility.h"
#include "STLUtility.h"
#include "SearchHandler.h"

#include "StringSearchUtility.h"
#include "CoordinateRequest.h"
#include "CoordinatePacket.h"
#include "TopRegionRequest.h"
#include "TopRegionMatch.h"
#include "NameUtility.h"
#include "CategoryTree.h"

#include "MC2CRC32.h"
#include "DataBuffer.h"
#include "Properties.h"
#include "FileUtils.h"

#include "CategoriesData.h"
#include "ClientSettings.h"

#include "SearchHeadingDesc.h"
#include "ParserDebitHandler.h"
#include "PositionInfo.h"
#include "POIImageIdentificationTable.h"
#include "CoordinateTransformer.h"
#include "WGS84Coordinate.h"
#include "ItemInfoPacket.h"
#include "SearchSorting.h"
#include "SearchTypes.h"

#include <boost/lexical_cast.hpp>

#include <iterator>

#define dbgout mc2dbg << "[SearchParserHandler]::"<<__FUNCTION__<<" "

const int32 SearchParserHandler::INVALID_SERVICE_ID = -1;

namespace {

ostream& operator << ( ostream& ostr, const CompactSearchHitType& type ) {
   ostr << "___CompactSearchHitType___" << endl
        << "name: " << type.m_name << endl
        << "nameStringCode: " << type.m_nameStringCode << endl
        << "type: " << type.m_type << endl
        << "typeStringCode: " << type.m_typeStringCode << endl
        << "image: " << type.m_imageName << endl
        << "round: " << type.m_round << endl
        << "serviceID: " << type.m_serviceID << endl
        << "heading: " << type.m_heading << endl
        << "searchTypeMask: 0x" << hex << type.m_searchTypeMask << dec << endl
        << "topRegionID: 0x" << hex << type.m_topRegionID << dec << endl
        << "language: "  << type.m_language << endl
        << "mapRights: 0x" << hex <<type.m_mapRights << dec <<endl
        << "invertRights: " << type.m_invertRights << endl;
   return ostr;
}

ostream& operator << ( ostream& ostr, const CompactSearch& search ) {
   ostr << "What: " << search.m_what << endl;
   ostr << "Where: " << search.m_where << endl;
   ostr << "Area id: " << search.m_areaID << endl;
   ostr << "start/end index: " << search.m_startIndex << "/" 
        << search.m_endIndex << endl;
   ostr << "top region id: " << search.m_topRegionID << endl;
   ostr << "category name: \"" << search.m_categoryName << "\"" << endl;
   ostr << "category ids: ";
   for ( uint32 catIdx = 0;
         catIdx < search.m_categoryIDs.size(); ++catIdx ) {
      ostr << search.m_categoryIDs[ catIdx ] << ", ";
   }
   ostr << endl;
   return ostr;
}


class OverviewResult: public SearchResultRequest {
public:
   explicit OverviewResult( const RequestData& parentOrID );

   /**
    * Adds request and copies matches, the request pointer will be owned by 
    * this object.
    * @param req the request
    * @param addOverview if it should add overview req
    * @param name the name that the original overview match had which
    *        will be included in the new location name: "location ( name )"
    *        unless location name alread contains the "name".
    * @param oMatch The origin of name used for complete region access.
    * 
    */
   void addRequest( SearchResultRequest* req, bool addOverview,
                    const OverviewMatch* oMatch = NULL );

   const vector<VanillaMatch*>& getMatches() const;

   // this class is just a container, no processing is done here.
   void processPacket( PacketContainer* cont) { }

   // always ok
   StringTable::stringCode getStatus() const { return StringTable::OK; }

   const SearchRequestParameters& getSearchParameters() const {
      return m_param;
   }
private:
   typedef STLUtility::AutoContainer< vector<SearchResultRequest* > > Requests;
   Requests m_requests;
   /**
    * unique set of item ids, used so we do not add
    *  more than one unique match
    */
   set<uint32> m_itemIDs; 

   vector<VanillaMatch*> m_matches; 
   SearchRequestParameters m_param;
};


OverviewResult::OverviewResult( const RequestData& parentOrID ):
   SearchResultRequest( parentOrID ) {

}

const vector<VanillaMatch*>& OverviewResult::getMatches() const {
   return m_matches;
}

void OverviewResult::addRequest( SearchResultRequest* req, bool addOverview,
                                 const OverviewMatch* oMatch ) {
   m_requests.push_back( req );

   MC2String overviewName = oMatch ? oMatch->getName0() : "" ;
   MC2String overviewNameLower = StringUtility::copyLower( overviewName );

   // copy all matches
   if ( addOverview ) {
      //m_matches.push_back( req->getOverviewMatch(i) );
   } else {
      // we could reserve, but at this point we dont know if 
      // there are lots of duplicates item ids.
      // m_matches.reserve( m_matches.size() + req->getMatches().size() );

      // if extraName is empty skip it and do a normal match copy
      vector<VanillaMatch*>::const_iterator it = req->getMatches().begin();
      vector<VanillaMatch*>::const_iterator itEnd = req->getMatches().end();
      for ( ; it != itEnd; ++it ) {
         // dont add an already existing item id match
         if ( m_itemIDs.find( (*it)->getItemID() ) != m_itemIDs.end() ) {
            continue;
         }

         m_itemIDs.insert( (*it)->getItemID() );

         // copy match pointer so we can return matches as a one complete
         //  vector
         m_matches.push_back( (*it) );
         VanillaMatch& match = *m_matches.back();

         if ( match.getNbrRegions() == 0 && oMatch != NULL ) {
            // Copy the regions from oMatch
            for ( uint32 i = 0 ; i < oMatch->getNbrRegions() ; ++i ) {
               match.addRegion( static_cast< VanillaRegionMatch*> ( 
                                   oMatch->getRegion( i )->clone() ), true );
            }
            match.updateLocationName( 
               req->getSearchParameters().getRegionsInMatches() );
            mc2dbg4 << "Match " << match.getName() << " has no regions using "
                    << match.getLocationName() << endl;
         }

         // change location name to include overview name in format: 
         // "location ( name )"
         //
         // (convert to utf-8 and compare it )
         // Eeeh, it is in mc2-format and so are all internal strings in mc2.
         // It is either utf-8 or iso-8859-1 but utf-8 is default.
         MC2String locationName = 
            StringUtility::copyLower( match.getLocationName() );
         
         if ( locationName.find( overviewNameLower ) == MC2String::npos ) {
            MC2String addName( MC2String( "(" ) + overviewName + ")" );
            locationName = MC2String( match.getLocationName() ) + 
               " " + addName;
            mc2dbg4 << "ADDING " << overviewName << " to location " 
                    << locationName << endl;

            match.setLocationName( locationName.c_str() );
            // Add region too. Or add to name of last region
            vector<const SearchMatch*> printRegs;
            match.getRegionsToPrint( printRegs );
            if ( !printRegs.empty() ) {
               const_cast< SearchMatch* >( printRegs.back() )->setName( 
                  (MC2String( printRegs.back()->getName() ) + " " + 
                   addName).c_str() );
               mc2dbg4 << "Added to last region too. " 
                       << printRegs.back()->getName() << endl;
            } else {
               // Add one to hold name
               SearchMatchPoints points;
               auto_ptr<VanillaMunicipalMatch> vrm(
                  new VanillaMunicipalMatch( 
                     addName.c_str(),
                     0, // nameInfo
                     IDPair_t().getMapID(),
                     IDPair_t().getItemID(),
                     points,
                     0, // source
                     addName.c_str()/*alphaSortingName*/,
                     0, // location
                     0, // restrictions
                     ItemTypes::municipalItem,/*ItemType*/
                     MAX_UINT16 ) ); // itemsubtype
               match.addRegion( static_cast< VanillaRegionMatch*> ( 
                                   vrm->clone() ), true );
               mc2dbg4 << "Added a region too. " << vrm->getName() << endl;
            }
         } // End if name of searched overview match is not in match
      } // For all matches in req
   } // End else 
}

class MergedResult : public SearchResultRequest {
public:
   explicit MergedResult( const RequestData& parentOrID );

   /**
    * Adds request and copies matches, the request pointer will be owned by 
    * this object.
    * @param req the request
    */
   void addRequest( SearchResultRequest* req );

   const vector<VanillaMatch*>& getMatches() const;

   // this class is just a container, no processing is done here.
   void processPacket( PacketContainer* cont) { }

   // always ok
   StringTable::stringCode getStatus() const { return StringTable::OK; }

   const SearchRequestParameters& getSearchParameters() const {
      return m_param;
   }

   void sort( const CompactSearch& params );

   uint32 getTotalNbrMatches() const;
private:
   typedef STLUtility::AutoContainer< vector<SearchResultRequest* > > Requests;
   Requests m_requests;
   /**
    * unique set of item ids, used so we do not add
    *  more than one unique match
    */
   set<uint32> m_itemIDs; 

   vector<VanillaMatch*> m_matches; 
   SearchRequestParameters m_param;
   uint32 m_totalMatches;
};


MergedResult::MergedResult( const RequestData& parentOrID ):
   SearchResultRequest( parentOrID ), m_totalMatches(0) {

}

const vector<VanillaMatch*>& MergedResult::getMatches() const {
   return m_matches;
}

uint32 MergedResult::getTotalNbrMatches() const {
   return m_totalMatches;
}

void MergedResult::addRequest( SearchResultRequest* req ) {
   m_requests.push_back( req );

   m_matches.insert( m_matches.end(), req->getMatches().begin(), 
                     req->getMatches().end() );

   m_totalMatches += req->getTotalNbrMatches();
}

void MergedResult::sort( const CompactSearch& params ) {
   if( params.m_round == 1 ) {
      // External search. Calc dist and sort by distance
      SearchHandler::calcDistsAndSort( m_matches, params.m_location.m_coord, 
                                       params.m_sorting );
   } else {
      // Iternal. Distance available, just sort
      SearchSorting::sortSearchMatches( m_matches, params.m_sorting );
   }
}

SearchResultRequest*
normalSearch( ParserThread* thread, 
              ParserThreadGroup* group,
              const CompactSearch& search,
              const SearchRequestParameters& params );

OverviewResult*
overviewSearch( ParserThread* thread, 
                ParserThreadGroup* group,
                const CompactSearch& search,
                const SearchRequestParameters& params,
                const SearchResultRequest& req );

/**
 * Creates a normal search request
 *
 */
SearchResultRequest* createNormalRequest( ParserThread* thread, 
                                          ParserThreadGroup* group,
                                          const CompactSearch& search,
                                          const SearchRequestParameters& params ) {

   const TopRegionRequest* topReq = group->getTopRegionRequest( thread );
   if ( search.m_areaID.isValid() ) {
      // search in specific area
      vector<IDPair_t> selectedAreas;
      selectedAreas.push_back( search.m_areaID );
      return new SearchRequest( thread->getNextRequestID(), params,
                                selectedAreas, search.m_what.c_str(),
                                topReq, search.m_location.m_coord,
                                NULL, search.m_distance );

   } else if ( search.m_where.empty() ) {
      // ok no where location means use gps or try entire top region

      if ( search.m_location.isValid() ) {
         // search using location

         if ( search.m_distance == -1 ) {
            return new ClosestSearchRequest( thread->getNextRequestID(), params, 
                                             search.m_location.m_coord,
                                             search.m_what.c_str(),
                                             topReq );
         } else {
            // search within radius
            return new SearchRequest( thread->getNextRequestID(), params,
                                      search.m_location.m_coord,
                                      search.m_what,
                                      topReq, 
                                      search.m_distance );
         }
      } else {
         // entire top region
         return new ClosestSearchRequest( thread->getNextRequestID(), params,
                                          search.m_topRegionID,
                                          search.m_what.c_str(), 
                                          topReq );

      }
   } 

   return new SearchRequest( thread->getNextRequestID(), params,
                             search.m_topRegionID, search.m_where.c_str(),
                             search.m_what.c_str(),
                             topReq );

}

/**
 * Handles search request
 */
SearchResultRequest*
handleRequest( ParserThread* thread, 
               ParserThreadGroup* group,
               const CompactSearch& search,
               const SearchRequestParameters& params,
               SearchResultRequest* req ) {
   // sort out overview matches and do area id search
   if ( req->getNbrOverviewMatches() > 1 && 
        req->getMatches().empty() ) {
      mc2dbg << "[SearchParserHandler] Nbr overview matches: " 
             << req->getNbrOverviewMatches() << endl;
      OverviewResult* overview = 
         overviewSearch( thread, group,
                         search,
                         params, *req );
      if ( overview == NULL ) {
         return req;
      }
      overview->addRequest( req, false );
      return overview;
   } 

   return req;
}

void
postSearchAddCountry( SearchMatch* match, ParserThread* thread,
                      const SearchRequestParameters& params,
                      const uint32 useThisTopRegionID = MAX_UINT32 ) {
   // Get the country for the match and add it as a region
   const TopRegionMatch* country = NULL;
   if ( useThisTopRegionID != MAX_UINT32 ) {
      country = thread->getTopRegionRequest()->
         getTopRegionWithID( useThisTopRegionID );
   } else {
      country = thread->getTopRegionRequest()->
         getCountryForMapID( match->getMapID() );
   }
   if ( country != NULL ) {
      // Hm, get the innermost region and add country to it
      SearchMatch* lastRegion = NULL;
      // Nah, just add it as standalone. Should it be in a Zip code or
      // Municipal?
      lastRegion = match;
      if ( lastRegion != NULL ) {
         lastRegion->addRegion( new VanillaCountryMatch( 
            country->getNames()->getBestName( 
               params.getRequestedLang() )->getName(), 
            country->getID() ), true/*Delete it please*/ );
      }
   } else {
      mc2dbg << "No top region for match with mapID "
             << match->getMapID() << endl;
   }
}

void 
postSearchAddCountry( SearchResultRequest* req,
                      ParserThread* thread, 
                      const SearchRequestParameters& params,
                      const uint32 useThisTopRegionID = MAX_UINT32 ) {
   // First fixup hits by adding country to them, if this is needed. 
   // But I can't check that as params are made within SearchParserHandler
   // and no info about what the user wants gets here!
   // But for now you have to call this by your own to get this.
   if ( !req->getMatches().empty() ) {
      typedef vector<VanillaMatch*> VanillaMatchVector;
      VanillaMatchVector& matches = const_cast<VanillaMatchVector&>(
         req->getMatches() );

      for ( VanillaMatchVector::iterator it = matches.begin(), 
               endIt = matches.end() ; it != endIt ; ++it ) {
         postSearchAddCountry( *it, thread, params, useThisTopRegionID );
      } // End for all matches
   } // End if to add country to matches
   
}
 
OverviewResult*
overviewSearch( ParserThread* thread, 
                ParserThreadGroup* group,
                const CompactSearch& search,
                const SearchRequestParameters& params,
                const SearchResultRequest& req  ) {

   if ( req.getStatus() != StringTable::OK ) {
      return NULL;
   }

   // create new search params since the area id will change.
   CompactSearch overviewSearch = search;

   auto_ptr<OverviewResult> 
      result( new OverviewResult( thread->getNextRequestID() ) );

   // holds requests to be sent and handled
   vector<RequestWithStatus*> requests;

   // 1) create normal search requests from overview requests

   set<uint32> mapIDs;
   for ( uint32 i = 0, n = req.getNbrOverviewMatches(); i < n; ++i ) {
      const OverviewMatch* match = req.getOverviewMatch( i );

      // Only search in the same map once for all overview matches
      // if, and only if, the search is a coordinate search.
      // This is because there is no point in searching with location
      // more than once per map. The overview hits are only used to point
      // out which maps that needs to be search in with the location.
      // For all other searches without location, we need to go into each
      // overview and see if we find any matches.
      if ( search.m_location.isValid() &&
           ! mapIDs.insert( match->getMapID() ).second ) {
         continue;
      }

      overviewSearch.m_areaID = match->getID();
      
      SearchResultRequest* normalResults = 
         createNormalRequest( thread, group, 
                              overviewSearch, params );
      if ( normalResults == NULL ) { 
         continue;
      }

      requests.push_back( normalResults );
      
   }

   // 2) send requests
   thread->putRequests( requests );
   
   // 3) handle requests
   for ( uint32 i = 0; i < requests.size(); ++i ) {

      // if request is not ok, delete it and take next
      if ( requests[ i ]->getStatus() != StringTable::OK ) {
         delete requests[ i ];
         continue;
      }
      // add request + overview match name
      const OverviewMatch* oMatch = req.getOverviewMatch( i );
      result->addRequest( handleRequest( thread, group, 
                                         search, params,
                                         static_cast<SearchResultRequest*>
                                         ( requests[ i ] ) ),
                          false, // dont add overview
                          oMatch );
   }

   return result.release();
                   
}

SearchResultRequest*
normalSearch( ParserThread* thread, 
              ParserThreadGroup* group,
              const CompactSearch& search,
              const SearchRequestParameters& params ) {

   SearchResultRequest* req = createNormalRequest( thread, group, 
                                                   search, params );

   thread->putRequest( req );

   SearchResultRequest* res = handleRequest( thread, group, 
                                             search, params, req );
   return res;
}


// Merge the search results from all headings into one list containing all results
void
mergeResultList( ParserThread* thread, const CompactSearch& params, 
                 SearchParserHandler::SearchResults& results ) {
   typedef SearchParserHandler::SearchResults SearchResults;
   MergedResult* allResults = new MergedResult( thread->getNextRequestID() );

   // Add all results to one list
   for( SearchResults::const_iterator it = results.begin(); 
        it != results.end(); ++it ) {
      allResults->addRequest( it->second->getSearchResultRequest() );
   }
   
   // Sort the list with all results
   allResults->sort( params );
   
   SearchResults mergedResults;
   mergedResults.insert( 
      make_pair<uint32, SearchParserHandler::SearchResultsData*> (
         SearchHeadingDesc::MERGED_RESULTS_HEADING, 
         new SearchParserHandler::SearchResultsData( allResults ) ) );

   results.swap( mergedResults );
}

}

MC2String SearchParserHandler::
getCategoryString( const MC2String& clientCategory, 
                   const LangType& language ) const {

  
   // Search for matching category string
   Category cat = findCategoryFromList( clientCategory );
   if ( cat == INVALID_CATEGORY ) {
      return "";
   }

   const char* str = StringTable::getString( cat.m_stringcode,
                                             language );
   return str ? str : "";

}

const SearchParserHandler::Category SearchParserHandler::INVALID_CATEGORY;

SearchParserHandler::
SearchParserHandler( ParserThread* thread,
                     ParserThreadGroup* group,
                     const CompactSearchHitTypeVector& headings ):
   ParserHandler( thread, group ) {

   // Setup the categories that can be serached in/for.
   // TODO: Make this from the union of all CategoriesData sets.
   SearchParserHandler::Category categoryMap[] = {
      { "Airport", StringTable::AIRPORT, 18 },
      { "Banks & ATMs", StringTable::SEARCHCAT_BANK_ATM, 272 },
      { "ATM", StringTable::ATM, 152 },
      { "Bank",  StringTable::BANK, 151 },
      { "Best Gas Prices" , 
        StringTable::BEST_GAS_PRICES, CategoryTreeUtils::INVALID_CATEGORY },
      { "Car dealer", StringTable::CAR_DEALER, 76 },
      { "Cinema", StringTable::CINEMA, 98 },
      { "City Centre", StringTable::CITY_CENTRE, 266 }, 
      { "Doctor", StringTable::DOCTOR, 248 },
      { "Ferry terminal", StringTable::FERRY_TERMINAL, 128 },
      { "Golf course", StringTable::GOLF_COURSE, 48 },
      { "Hotel", StringTable::HOTEL, 118 },
      { "Local rail", StringTable::LOCAL_RAIL, 123 },
      { "Monument", StringTable::HISTORICAL_MONUMENT, 130 },
      { "Museum", StringTable::MUSEUM, 22 },
      { "Music store", StringTable::CAT_MUSIC_AND_VIDEO, 63 },
      { "Nightlife", StringTable::NIGHTLIFE, 6 },
      { "Open parking area",
        StringTable::OPEN_PARKING_AREA, 237 },
      { "Parking garage", StringTable::PARKING_GARAGE, 236 },
      { "Petrol station", StringTable::PETROL_STATION, 103 },
      { "Pharmacy", StringTable::PHARMACY, 245 },
      { "Police station", StringTable::POLICE_STATION, 108 },
      { "Post office", StringTable::POST_OFFICE, 107 },
      { "Rent a car", StringTable::RENT_A_CAR_FACILITY, 15 },
      { "Restaurant", StringTable::RESTAURANT, 85 },
      { "Shopping centre", StringTable::SHOPPING_CENTRE, 59 },
      { "Shopping", StringTable::CAT_SHOPPING__Z__, 9 },
      { "Ski resort", StringTable::SKI_RESORT, 40 },
      { "Subway", StringTable::CAT_SUBWAY, 121 },
      { "Theatre", StringTable::THEATRE, 88 },
      { "Tourist information",
        StringTable::TOURIST_OFFICE, 19 },
      { "Train Station", 
        StringTable::RAILWAY_STATION, 122 },
      { "Vehicle repair facility",
        StringTable::VEHICLE_REPAIR_FACILITY,  104 },
      { "Bars", StringTable::CAT_BARS, 100 },
      { "Cafe", StringTable::CAFE, 86 },
      { "Hospital", StringTable::HOSPITAL, 246 },
      { "Night club", StringTable::CAT_NIGHT_CLUB, 90 },
      { "Parking", StringTable::PARKING, 267 },
      { "Grocery store", StringTable::GROCERY_STORE, 67 },
      { "Travel & Transport", StringTable::SEARCHCAT_TRAVEL, 5 },
   };

   m_categories.insert( categoryMap,
                        categoryMap +
                        sizeof ( categoryMap ) / sizeof ( categoryMap[ 0 ] ) );


   // Add a search heading desc for each image set
   for ( size_t i = 0; i < ImageTable::NBR_IMAGE_SETS; ++i ) {
      ImageTable::ImageSet imageSet = static_cast<ImageTable::ImageSet>( i );
      m_headings.push_back( make_pair( imageSet,
                                       new SearchHeadingDesc( imageSet, headings ) ) );
   }

   // until a proper testing of m_categories can be done,
   // we do this in a new categories
   for ( Categories::const_iterator it = m_categories.begin();
         it != m_categories.end(); ++it ) {
      Category cat = *it;
      cat.m_name = StringUtility::copyLower( cat.m_name );
      m_lowerCaseCategories.insert( cat );
   }
}

SearchParserHandler::~SearchParserHandler() {
   STLUtility::deleteAllSecond( m_headings );
}

bool SearchParserHandler::makeSearchDebit( const CompactSearch& search,
                                           Request& req,
                                           const MC2String& extraString ) { 

   // compose new "where" with orginal "where" + areaID
   MC2String where( search.m_where );
   if ( search.m_areaID.isValid() ) {
      FixedSizeString str( 256 );
      sprintf( str, ":%u:%u", 
               search.m_areaID.getMapID(),
               search.m_areaID.getItemID() );
      where += str;
   }

   return m_thread->getDebitHandler()->makeSearchDebit( 
      m_thread->getCurrentUser(), &req, // user, request
      NULL, // answer container
      where.c_str(),
      search.m_what.c_str(),
      NULL, extraString.c_str() );
}

SearchParserHandler::SearchResults
SearchParserHandler::
compactSearch( const CompactSearch& search ) {
   // Note:
   // At this point we assume the user have region access
   // ( so check region access before calling this function )


   mc2dbg << "Searching for: " << endl << search << endl;


   // 1) find heading or round type

   uint32 round = search.m_round;
   ServiceID serviceID = INVALID_SERVICE_ID;

   if ( search.m_heading != -1 ) {
      const SearchHeadingDesc* headingDesc = getHeadingDesc();
      CompactSearchHitType hitType =
         headingDesc->getHitTypeFromHeading( search.m_heading );

      if ( hitType.m_name.empty() ) {
         mc2log << warn << "[SearchParserHandler] No valid heading. (heading=" 
                << search.m_heading << ")" << endl;
         return SearchResults();
      }

      round = hitType.m_round;
      serviceID = hitType.m_serviceID;
   }

   // 2) search

   SearchResults results;


   MC2String strippedWhat = StringSearchUtility::
      convertIdentToClose( search.m_what );
   MC2String strippedWhere = 
      StringSearchUtility::
      convertIdentToClose( search.m_where );
   MC2String strippedCategoryName = 
      StringSearchUtility::
      convertIdentToClose( search.m_categoryName );

   // must have something to search for, so do
   // sanity checking on inputs here
   if ( strippedWhat.empty() && strippedWhere.empty() &&
        strippedCategoryName.empty() && 
        search.m_categoryIDs.empty() ) {
      // invalid input
      results = createEmptyHeadings( search, serviceID );
   } else {
      if ( round == 0 ) {
         results = normalSearch( search );
      } else {
         results = externalSearch( search, serviceID );
      }
   }

   
   // compose a debit string so the debit log can
   // be interpreted correctly for compact search
   MC2String debitString("CompactSearch:");
   SearchResults::const_iterator it = results.begin();
   SearchResults::const_iterator itEnd = results.end();
   for ( ; it != itEnd; ++it ) {
      if ( it != results.begin() ) {
         debitString += ",";
      }
      debitString += STLStringUtility::uint2str( (*it).first );
   }

   if ( ! search.m_categoryIDs.empty() ) {
      debitString += ":Category:";
      for ( CompactSearch::CategoryIDs::const_iterator
            catIt = search.m_categoryIDs.begin(),
            catItEnd = search.m_categoryIDs.end();
            catIt != catItEnd;
            ++catIt ) {
         debitString += STLStringUtility::uint2str( *catIt );
         debitString += ",";
      }
   }

   // 3) make debit only ONE debit if any search was successfull.
   //    Note: should there be a special debit for proximity search?
   it = results.begin();
   for (; it != itEnd; ++it ) {
      SearchResultRequest* req = (*it).second->getSearchResultRequest();
      if ( req->getStatus() == StringTable::OK &&
           makeSearchDebit( search, *req, debitString ) ) {
         // end loop after first successfull debit
         break;
      }
   }
   
   if ( search.m_oneResultList ) {
      // Merge all results into one list
      mergeResultList( m_thread, search, results );

      // Convert to "sane item infos". This will also format the address
      // based on the country of the match
      if ( !results.empty() ) {         
         const vector<VanillaMatch*>& matches = 
            results.begin()->second->getSearchResultRequest()->getMatches();

         // Only convert the matches that are actually returned.
         uint32 topRegion = MAX_UINT32;
         uint32 numberMatches = MIN( search.m_maxHits, matches.size() );
         if ( numberMatches > 0 ) {
            topRegion = getTopRegionForMatch( matches[0] );
         }

         for(uint32 i = 0 ; i < numberMatches; ++i ) {
            matches[i]->mergeToSaneItemInfos( search.m_language, topRegion );
         }
      }      
   }
   
   return results;
}

CompactSearchHitTypeVector 
SearchParserHandler::
getSearchHitTypes( LangType::language_t language, uint32 descVersion ) const {

   // copy headings since we are about to modify the name
   // and top region id
   CompactSearchHitTypeVector hitTypes;

   const SearchHeadingDesc* headingDesc = getHeadingDesc();
   headingDesc->getTranslatedHeadings( *m_thread->getCurrentUser()->getUser(),
                                       language,
                                       descVersion != 0,
                                       hitTypes );

   return hitTypes;
}

MC2String
SearchParserHandler::
getSearchHitTypesCRC( LangTypes::language_t language ) const {
   // calculate crc based on version 1
   CompactSearchHitTypeVector types( getSearchHitTypes( language, 1 ) );

   // calculate crc buffer size
   CompactSearchHitTypeVector::const_iterator it = types.begin();
   CompactSearchHitTypeVector::const_iterator itEnd = types.end();
   uint32 bufferSize = 0;
   for (; it != itEnd; ++it ) {
      // round + heading + string length with \0
      bufferSize +=  
         2 * sizeof( uint32 ) + 
         (*it).m_name.size() + 1 +
         (*it).m_imageName.size() + 1 +
         sizeof( uint32 ); // image data crc
   }

   const MC2String imagesPath = Properties::getProperty( "IMAGES_PATH" );
   // write buffer
   DataBuffer crcBuff( bufferSize*2 ); // *2, just for alignment
   crcBuff.fillWithZeros();

   for ( it = types.begin(); it != itEnd; ++it ) {
      crcBuff.writeNextLong( (*it).m_round );
      crcBuff.writeNextLong( (*it).m_heading );
      crcBuff.writeNextString( (*it).m_name.c_str() );
      // only add top_region_id tag if it has valid top region id
      if ( (*it).m_topRegionID != MAX_UINT32 ) {
         crcBuff.writeNextLong( (*it).m_topRegionID );
      }

      if ( ! (*it).m_imageName.empty() ) {
         crcBuff.writeNextString( (*it).m_imageName.c_str() );
         // calculate crc for image data
         char* imageContent;
         uint32 imageFileSize;
         if ( FileUtils::
              getFileContent( MC2String( imagesPath + "/" + (*it).m_imageName +
                                         ".png" ).c_str(),
                              imageContent, &imageFileSize ) ) {
            
            uint32 imageCRC = MC2CRC32::crc32( reinterpret_cast<const byte*>
                                               ( imageContent ), imageFileSize );
            delete [] imageContent;

            crcBuff.writeNextLong( imageCRC );
         }
      }
   }

   // Make crc
   uint32 crc = MC2CRC32::crc32( (byte*)crcBuff.getBufferAddress(),
                                 crcBuff.getCurrentOffset() );
   MC2String crcStr;
   STLStringUtility::uint2strAsHex( crc, crcStr );

   return crcStr;
}




SearchParserHandler::SearchResults 
SearchParserHandler::
createEmptyHeadingsForExternal( const CompactSearch& searchIn,
                                ServiceID serviceID ) {

   SearchResults result;

   // need to modify the search a bit, so copy in parameter
   CompactSearch search = searchIn;

   // if specific location lat/lon was specified we need
   // to find a value for the "where" field AND the top region id
   if ( search.m_location.isValid() ) { 
      search.m_where = PositionInfo::
         getMunicipalFromPosition( *m_thread,
                                   search.m_location.m_coord,
                                   search.m_language,
                                   m_group->getTopRegionRequest( m_thread ),
                                   search.m_topRegionID );
      mc2dbg << "[SPH::externalSearch] searching with where="
             << search.m_where << " instead of location: " 
             << search.m_location.m_coord << endl;
   }

   const SearchHeadingDesc* headingDesc = getHeadingDesc();

   if ( search.m_topRegionID == MAX_UINT32 ) {
      // we need to determine top region id
      CompactSearchHitType hitType = 
         headingDesc->getHitTypeFromHeading( search.m_heading );
      
      if ( hitType.isValid() ) {
         search.m_topRegionID = hitType.m_topRegionID;
      }
   }
   
   CompactSearchHitTypeVector headings;
   headingDesc->getHeadings( *m_thread->getCurrentUser()->getUser(),
                             headings );
   
   for ( CompactSearchHitTypeVector::const_iterator it = headings.begin();
         it != headings.end(); ++it ) {
      if ( (*it).m_topRegionID == search.m_topRegionID ) {
         // check service id if search was specified for a specific heading
         if ( serviceID != INVALID_SERVICE_ID &&
              serviceID != (ServiceID)(*it).m_serviceID ) {
            continue;
         }

         // must have a valid hit type.
         if ( ! (*it).isValid() ) {
            continue;
         }
      
         ExternalSearchRequestData emptyData;
         result.
            insert( make_pair( (*it).m_heading,
                               new SearchResultsData( 
                                  new ExternalSearchRequest( 
                                     m_thread->getNextRequestID(),
                                     emptyData ) ) ) );
         
      }
   }
   
   return result;
}

SearchParserHandler::SearchResults 
SearchParserHandler::
createEmptyHeadings( const CompactSearch& search,
                     ServiceID serviceID ) {
   SearchResults results;

   // if specific heading ID or round 0 then fetch
   // all headings from descriptor and add them
   if ( search.m_heading >= 0 ||
        search.m_round == 0 ) { 
   
      const SearchHeadingDesc* headingDesc = getHeadingDesc();
      if ( search.m_heading >= 0 )  {
         // specific heading, only add the one we have
         CompactSearchHitType heading =
            headingDesc->getHitTypeFromHeading( search.m_heading );
         if ( headingDesc->canSearchHeading( heading,
                                             *m_thread->getCurrentUser()->
                                             getUser() ) ) {
            results.insert( make_pair( heading.m_heading,
                                       new SearchResultsData( 
                                          new EmptySearchRequest() ) ) );
         }

      } else if ( search.m_round == 0 ) {
         // no specific heading, add all valid headings
         CompactSearchHitTypeVector headings;
         headingDesc->getHeadings( *m_thread->getCurrentUser()->getUser(),
                                   headings );
         
         CompactSearchHitType oneSearchHitType;
         for ( size_t i = 0; i < headings.size(); ++i ) {
            if ( headings[ i ].m_round == 0 ) {
               if ( !search.m_oneResultList ) { 
                  // Several searches i.e. compact search
                  results.insert( make_pair( headings[ i ].m_heading,
                                             new SearchResultsData( 
                                             new EmptySearchRequest() ) ) );
               } else {
                  // One search, only do one search does not care about headings
                  oneSearchHitType += headings[ i ];
               } 
            }
         }
         
         // Add the new search hit type to the description object
         if ( search.m_oneResultList ) {
            oneSearchHitType.m_heading = SearchHeadingDesc::INVALID_HEADING_ID;
            results.insert( make_pair( SearchHeadingDesc::INVALID_HEADING_ID, 
                                       new SearchResultsData( 
                                          new EmptySearchRequest(), 
                                          oneSearchHitType ) ) );
         }
      }
   } else if ( search.m_round == 1 ) {
      // else external search, must do some special handling of them.
      // create empty headings for external search
      return createEmptyHeadingsForExternal( search, serviceID );
   }

   return results;
}

SearchParserHandler::SearchResults
SearchParserHandler::normalSearch( const CompactSearch& searchOrg ) {
   using namespace CategoryTreeUtils;
   dbgout << endl;

   CompactSearch search = searchOrg;

   // setup search params
   SearchTypes::StringPart part = SearchTypes::BeginningOfWord;
   SearchTypes::StringMatching match = SearchTypes::CloseMatch;

   if ( searchOrg.m_language == LangTypes::chinese ) {
      mc2dbg << "[SearchParserHandler] Searching for chinese." << endl;
      part = SearchTypes::Anywhere;
      match = SearchTypes::CloseFullMatch;
   }

   SearchRequestParameters params;

   params.setStringPart( part );
   params.setMatchType( match );

   if ( search.m_oneResultList && 
        ! ( search.m_sorting == SearchTypes::DistanceSort && 
           !search.m_location.isValid() ) ) {
      // One search sets sorting explicit, however distance sort is not possible
      // without a valid position
      params.setSortingType( search.m_sorting );
   } else {
      // Figure out sorting in old Compact Search
      // sort by distance if we are searching with coordinate
      // otherwise, use confidence sort
      if ( ! search.m_areaID.isValid() &&
           search.m_location.isValid() && search.m_where.empty() ) {
         params.setSortingType( SearchTypes::DistanceSort );
      } else {
         params.setSortingType( SearchTypes::ConfidenceSort );
      }
   }
   
   params.setLookupCoordinates( false );
   params.setUniqueOrFull( false );
   params.setSearchForLocationTypes( SEARCH_ALL_REGION_TYPES );
   params.setRequestedLang( search.m_language );
   params.setAddStreetNamesToCompanies( true );
   params.setTryHarder( true );
   params.setEndHitIndex( search.m_endIndex );
   params.setNbrSortedHits( search.m_endIndex + 1 );
   params.setRegionsInMatches( SEARCH_ALL_REGION_TYPES );
   params.setRegionsInOverviewMatches( SEARCH_ALL_REGION_TYPES );
   
   SearchResults result = createEmptyHeadings( search, -1 ); // no service id

   if ( search.isCategorySearch() ) {
      CategorySet categories;
      
      const CategoryTree& tree = *m_thread->getGroup()->getCategoryTree();
      
      // if category query mode, then search for categories
      if ( search.m_findCategories ) {
         tree.findCategories( search.m_categoryName,
                              search.m_language,
                              categories );
      } else if ( search.m_categoryIDs.empty() && 
                  search.m_categoryName.empty() && search.m_smartCategory ) {
         handleSmartCategorySearch( search, categories, tree );
      } else if ( search.m_categoryIDs.empty() ) {
         // find one specific category (and its subcategories) from the 
         // list, using the name
         // TODO: sort the list and do better search
         Categories::const_iterator catIt = m_categories.begin();
         Categories::const_iterator catItEnd = m_categories.end();
         for ( ; catIt != catItEnd; ++catIt ) {
            if ( (*catIt).m_name == search.m_categoryName ) {
               tree.getAllCategories( (*catIt).m_categoryID, categories );
               break;
            }
         }
      } else {
         for ( CompactSearch::CategoryIDs::const_iterator
                  catIt = search.m_categoryIDs.begin(),
                  catItEnd = search.m_categoryIDs.end();
               catIt != catItEnd; ++catIt ) {
            // add this category id and get all sub categories too
            tree.getAllCategories( *catIt, categories );
         }
      }
      // If we have categories and the search string is empty
      // then we must tell the search handler to not expand
      // city center overviews.
      if ( ! categories.empty() && 
           search.m_what.empty() ) {
         params.setExpandCityCenter( false );
      }

      mc2dbg << "[SearchParserHandler] Number categories found: "
             <<  categories.size() << endl;

      mc2dbg << "[SearchParserHandler] Setting categories: ";
      transform( categories.begin(), categories.end(),
                 ostream_iterator<MC2String>( mc2log, " " ),
                 CategoryFunctor::TranslateWithID( 
                    tree, LangTypes::english ) );
      mc2dbg << endl;

      params.setCategories( categories );
      if ( categories.empty() && ! search.m_smartCategory ) {
         return result;
      }
   }

   dbgout << search << endl;

   const SearchHeadingDesc* headingDesc = getHeadingDesc();
   for ( SearchResults::iterator resultIt = result.begin();
         resultIt != result.end(); ++resultIt ) {
      try {
         const CompactSearchHitType* hitType;
         if ( resultIt->second->useHeading() ) {
            hitType = &headingDesc->
               getHeading( resultIt->first );
         } else {
            hitType = &resultIt->second->getOneSearchHeadingType();
         }

         params.setSearchForTypes( hitType->m_searchTypeMask );
         params.setInvertRights( hitType->m_invertRights );
         params.setMapRights( MapRights( static_cast<MapRights::Masks>
                                         ( hitType->m_mapRights ) ) );

         delete resultIt->second;
         // reset ptr, the normalSearch could throw.
         resultIt->second = NULL;
         resultIt->second = new SearchResultsData(
            ::normalSearch( m_thread, m_group, search, params ) );

      } catch ( const out_of_range& rangeErr ) {
         delete resultIt->second;
         mc2log << error << "[SearchParserHandler] "
                << rangeErr.what() << endl;
      }
   }

   // Get poi info for all hits, if requested
   if ( search.m_includeInfoItem ) {
      addItemInfo( result, params.getRequestedLang(), 
                   search.m_itemInfoFilter );
   }

   return result;
}

void
SearchParserHandler::getItemInfo( InfoMap& infoMap, 
                                  LangTypes::language_t lang,
                                  ItemInfoEnums::InfoTypeFilter infoTypeFilter ) {
   // Send information per map (all at once)
   vector< PacketContainer* > packets;
   for ( InfoMap::const_iterator it = infoMap.begin(), 
            endIt = infoMap.end() ; it != endIt ; ++it ) {
      packets.push_back( new PacketContainer( 
                            new GetItemInfoRequestPacket( 
                               it->first, lang, infoTypeFilter,
                               m_thread->getCurrentUser()->getUser(), 
                               it->second ) ) );
      packets.back()->setModuleType( MODULE_TYPE_MAP );
   }
   vector< PacketContainer* > replies;
   m_thread->putRequest( packets, replies );

   // Add information to reply matches
   // for all packets add to matching in infoMap 
   size_t packetPos = 0;
   for ( InfoMap::iterator it = infoMap.begin(), 
            endIt = infoMap.end() ; it != endIt ; ++it, ++packetPos ) {
      if ( replies[ packetPos ] == NULL ) {
         mc2dbg << "[SPH] Info reply packet for mapID " << MC2HEX( it->first )
                << " is NULL" << endl;
         continue;
      }
      ItemInfoReplyPacket* r = static_cast<ItemInfoReplyPacket*> (
         replies[ packetPos ]->getPacket() );
      if ( !r->getInfo( it->second ) ) {
         mc2dbg << "[SPH] Info reply is missing matches! " << endl;
         continue;
      }
   } // End for all maps
}

void
SearchParserHandler::addItemInfo( SearchResults& req, 
                                  LangTypes::language_t lang,
                                  ItemInfoEnums::InfoTypeFilter infoTypeFilter) {
   // Collect data per mapID
   InfoMap infoMap;
   for ( SearchResults::iterator resultIt = req.begin(), endIt = req.end();
         resultIt != endIt ; ++resultIt ) {
      vector<VanillaMatch*>& matches = const_cast<vector<VanillaMatch*>&>
         ( resultIt->second->getSearchResultRequest()->getMatches() );
      for ( size_t i = 0, end = matches.size() ; i < end ; ++i ) {
         infoMap[ matches[ i ]->getMapID() ].push_back( matches[ i ] );
      }
   } // For allresults

   // Get information and add it to result
   getItemInfo( infoMap, lang, infoTypeFilter );
}

void
SearchParserHandler::addItemInfo( SearchResultRequest* req,
                                  LangTypes::language_t lang,
                                  ItemInfoEnums::InfoTypeFilter infoTypeFilter) {
   // Collect data per mapID
   InfoMap infoMap;
   vector<VanillaMatch*>& matches = const_cast<vector<VanillaMatch*>&>
      ( req->getMatches() );
   for ( size_t i = 0, end = matches.size() ; i < end ; ++i ) {
      infoMap[ matches[ i ]->getMapID() ].push_back( matches[ i ] );
   }

   // Get information and add it to result
   getItemInfo( infoMap, lang, infoTypeFilter );
}

/// Temporary class 
class CatIDFinder {
public:
   CatIDFinder( SearchParserHandler::Category::CategoryID categoryID ) 
         : m_categoryID( categoryID ) {}

   bool operator () ( const SearchParserHandler::Category& c ) {
      return c.getCategoryID() == m_categoryID;
   }

private:
   SearchParserHandler::Category::CategoryID m_categoryID;
};

SearchParserHandler::SearchResults
SearchParserHandler::externalSearch( const CompactSearch& searchOrgParam, 
                                     ServiceID serviceID ) {
   dbgout << endl;

   CompactSearch search = searchOrgParam;
   const SearchHeadingDesc* headingDesc = getHeadingDesc();

   // send external searches.
   // use all external searches for the current top region.

   // if specific location lat/lon was specified we need
   // to find a value for the "where" field and the top region id
   if ( search.m_location.isValid() ) { 
      search.m_where = PositionInfo::
      getMunicipalFromPosition( *m_thread,
                                search.m_location.m_coord,
                                search.m_language, 
                                m_group->
                                getTopRegionRequest( m_thread ),
                                search.m_topRegionID );
      mc2dbg << "[SPH::externalSearch] searching with where="
             << search.m_where << " instead of location: " 
             << search.m_location.m_coord << endl;
   }

   if ( search.m_topRegionID == MAX_UINT32 ) {
      // we need to determine top region id
      CompactSearchHitType hitType = headingDesc->
         getHitTypeFromHeading( search.m_heading );

      if ( hitType.isValid() ) {
         search.m_topRegionID = hitType.m_topRegionID;
      }
   }

   SearchResults result;

   ServiceIDs serviceIDs;// holds service IDs for each request

   vector<RequestWithStatus*> requests;
  
   createRequests( searchOrgParam, search, serviceID, serviceIDs, 
                   requests );

   // send request
   m_thread->putRequests( requests );

   // handle request
   for ( uint32 i = 0; i < requests.size(); ++i ) {
      // ok to static cast here, we know its SearchResultRequest, see above
      SearchResultRequest* req = 
         static_cast<SearchResultRequest*>( requests[ i ] );  

      SearchHeadingDesc::HeadingID heading =
         headingDesc->findServiceHeading( serviceIDs[ i ] );

      if ( heading == SearchHeadingDesc::INVALID_HEADING_ID ) {
         mc2log << warn << "[SearchParserHandler] Unknown heading!" << endl;
      }

      // got a search hit, append to search hits
      SearchResultsData* data = new SearchResultsData( req );
      if ( ! result.insert( make_pair( heading, data ) ).second ) {
         // this should normaly not happen,
         // but if for some reason the heading was not found in multiple 
         // request they all get MAX_UINT32 heading, which is very wrong.
         mc2log << warn << "[SearchParserHandler] Could not append search hit"
                << " for heading " << heading << ", because it already have a"
                << " request." << endl;
         // free the request since it wont be anyone else
         // to take care of it after this.
         delete data; // deletes the req also
      }
   }

   return result;
}
namespace {
/**
 * Set the country name in search fields.
 * @param searchFields Set country name in this map.
 * @param topRequest
 * @param language
 */
void setCountryName( ExternalSearchRequestData::stringMap_t& searchFields,
                     const TopRegionRequest* topRequest,
                     uint32 topRegionID,
                     LangType language ) {
   // Get the top region match from top region id
   const TopRegionMatch* topMatch =
      topRequest->getTopRegionWithID( topRegionID );
   if ( topMatch == NULL ) {
      return;
   }

   searchFields[ ExternalSearchConsts::country_name ] =
      topMatch->getName( language );

   //
   // Setup top_region_center and top_region_span as two wgs84 strings
   // in the format "lat,lon"
   //
   WGS84Coordinate topLeft = CoordinateTransformer::
      transformToWGS84( topMatch->getBoundingBox().getCorner(MC2BoundingBox::top_left ) );
   WGS84Coordinate bottomRight = CoordinateTransformer::
      transformToWGS84(topMatch->
                       getBoundingBox().
                       getCorner( MC2BoundingBox::bottom_right ) );

   WGS84Coordinate center = CoordinateTransformer::
      transformToWGS84( topMatch->getBoundingBox().getCenter() );

   WGS84Coordinate span = CoordinateTransformer::
      transformToWGS84(MC2Coordinate( topMatch->getBoundingBox().getHeight(),
                                      topMatch->getBoundingBox().getLonDiff()));

   MC2String spanStr =
      boost::lexical_cast<MC2String>( span.lat ) +
      "," +
      boost::lexical_cast<MC2String>( span.lon );

   searchFields[ ExternalSearchConsts::top_region_span ] = spanStr;

   MC2String centerStr =
      boost::lexical_cast<MC2String>( center.lat ) +
      "," +
      boost::lexical_cast<MC2String>( center.lon );

   searchFields[ ExternalSearchConsts::top_region_center ] = centerStr;
}

} // anonymous

void
SearchParserHandler::createRequests( const CompactSearch& searchOrgParam,
                                     CompactSearch& search,
                                     ServiceID serviceID,
                                     ServiceIDs& serviceIDs,
                                     vector< RequestWithStatus* >& requests ) {
   
   
   const SearchHeadingDesc* headingDesc = getHeadingDesc();
   
   // Search all services available
   CompactSearchHitTypeVector headings;
   headingDesc->getHeadings(*m_thread->getCurrentUser()->getUser(),
                            headings);
   
   for ( CompactSearchHitTypeVector::iterator it = headings.begin();
         it != headings.end(); ++it ) {
      CompactSearchHitType& hitType = *it;
      if ( hitType.m_topRegionID == search.m_topRegionID ||
           hitType.m_topRegionID == MAX_UINT32 ) {
         // check service id if search was specified for a specific heading
         if ( serviceID != INVALID_SERVICE_ID &&
              serviceID != static_cast<ServiceID>( hitType.m_serviceID ) ) {
            continue;
         }
         
         // must have a valid hit type.
         if ( ! hitType.isValid() ) {
            continue;
         }
         
         ExternalSearchRequestData::stringMap_t searchFields;
         
         if ( ! setTopRegionField( searchFields, hitType.m_topRegionID, 
                                  hitType.m_serviceID ) ) {
            continue;
         }

         // Do not use Qype as external provider for compact search
         if ( ! search.m_oneResultList && 
              hitType.m_serviceID == ExternalSearchConsts::qype) {
            continue;
         }
         
         // set country name, translated to english,
         // currently only google local search that uses it.
         ::setCountryName( searchFields,
                           m_thread->getTopRegionRequest(),
                           search.m_topRegionID,
                           LangTypes::english );
         
         MC2String categoryName;
         uint32 categoryId = 0;
         // Find the service
         if ( hitType.isValid() ) {
            // Find the translated category name
            // if this is a category search.
            if ( ! searchOrgParam.m_categoryName.empty() ) {
               // get category search name and only update if it was successful
               categoryName = 
                  getCategoryString( search.m_categoryName, hitType.m_language );
            } else if ( ! searchOrgParam.m_categoryIDs.empty() ) {
               // Do something to search in category
               // TODO: Add a tree to find categories in...
               categoryId = searchOrgParam.m_categoryIDs[ 0 ];
               Categories::const_iterator findIt = 
                  std::find_if( m_categories.begin(), m_categories.end(), 
                                CatIDFinder( categoryId ) );
               if ( findIt != m_categories.end() ) {
                  categoryName = StringTable::getString( (*findIt).m_stringcode,
                                                         hitType.m_language );
                  mc2dbg << "[SPH] using category name \"" << categoryName 
                         << "\" for external search." << endl;
               }
            } // End ele if has valid m_categoryID
         } // End if CompactSearchHitType was found
         
         if ( hitType.m_serviceID == ExternalSearchConsts::not_external ) {
            continue;
         }  
 
         if ( hitType.m_providerType == CompactSearchHitType::yellow_pages ) {
            if ( !categoryName.empty() ) {
               // Use category if provided
               searchFields[ ExternalSearchConsts::category ] = categoryName;
               searchFields[ ExternalSearchConsts::category_id ] = 
                  boost::lexical_cast< MC2String > (categoryId );
            }
            searchFields[ ExternalSearchConsts::company_or_search_word ] = search.m_what.c_str();
            searchFields[ ExternalSearchConsts::city_or_area ] = search.m_where.c_str();
         } else {
            if ( !categoryName.empty() ) {
               // Category not supported for white pages
               continue;
            } else {
               searchFields[ ExternalSearchConsts::name_or_phone ] = search.m_what.c_str();
               searchFields[ ExternalSearchConsts::address_or_city ] = search.m_where.c_str();
            }
         }
         
         if ( !searchOrgParam.m_categoryName.empty() ) {
            mc2dbg << "[SPH] Searching for category \"" 
                   << categoryName << "\"" << endl;
         }
         // setup external search request
         SearchRequestParameters params;
         params.setRequestedLang( search.m_language );
         
         serviceIDs.push_back( hitType.m_serviceID );
         
         ExternalSearchRequestData 
            reqData( params, 
                     hitType.m_serviceID,
                     searchFields,
                     search.m_startIndex,
                     search.m_endIndex - search.m_startIndex + 1,
                     search.m_itemInfoFilter,
                     search.m_location.m_coord,
                     search.m_distance );
         
         ExternalSearchRequest* req = new
            ExternalSearchRequest( m_thread->getNextRequestID(), reqData );
         
         requests.push_back( req );
      }
   }
}


MC2String
SearchParserHandler::
getClientSpecificImageName( uint32 poiType ) const {
   const ClientSetting* clientSetting = m_thread->getClientSetting();
   if ( clientSetting != NULL ) {
      return m_group->getCategories()->
         getSearchListImageName( clientSetting->getImageSet(),
                                 poiType );
   }
   return "";
}


namespace {

MC2String getSpecialPOIImage( const ParserThreadGroup& group,
                              const ParserThread& thread,
                              const SearchMatch& match ) {

   const ClientSetting* settings = thread.getClientSetting();
   const POIImageIdentificationTable& poiImageTable =
      group.getPOIImageIdTable();

   MC2String imageName;

   // Some POIs have a special image set in the map data
   // and this overrides everything.

   // TODO:
   // ok, this is the ugly hack, this should actually be
   // in navclientsettings, for some kind of:
   // uses-special-images-with-this-prefix-setting...
   // but as it is now, this have to do

   if ( match.getType() == SEARCH_COMPANIES &&
        settings &&
        strcmp( settings->getCategoryPrefix(), "iphone" ) == 0 ) {
      const VanillaCompanyMatch* company =
         dynamic_cast< const VanillaCompanyMatch* >( &match );
      if ( company && ! company->getSpecialImage().empty() ) {
         // got a special image, get the full name from
         // the poi image table and append the client
         // specific postfix
         imageName =
            poiImageTable.getFullImageName( match.getItemSubtype(),
                                            company->getSpecialImage() );


         imageName += "_";
         imageName += settings->getCategoryPrefix();

      }

   }

   // Special images
   if ( match.getType() == SEARCH_COMPANIES &&
        settings && settings->getImageSet() == ImageTable::DEFAULT ) {
      const VanillaCompanyMatch* company =
         dynamic_cast< const VanillaCompanyMatch* >( &match );
      if ( company && ! company->getSpecialImage().empty() ) {
         // get the search result list image name from the poi image table,
         // if there is one
         imageName = 
            poiImageTable.getSearchResultImageName( match.getItemSubtype(),
                                                    company->getSpecialImage() );
      }
   }

   return imageName;
}

}

MC2String
SearchParserHandler::getImageName( const SearchMatch& match,
                                   uint32 headingID ) const
{
   if ( match.getExtSource() != 0 ) {
      // External provider/heading. Find the provider image.
      const SearchHeadingDesc* headingDesc = getHeadingDesc();
      CompactSearchHitType type = 
         headingDesc->findServiceHeadingType( match.getExtSource() );

      if ( type.isValid() ) {
         return type.m_imageName;
      }
     
      return "";
   }

   const ServerTileMapFormatDesc* desc =
      m_group->getTileMapFormatDesc( STMFDParams( LangTypes::english,
                                                  false ), m_thread );

   MC2String imageName;
   if ( match.getItemType() == ItemTypes::pointOfInterestItem ) {
      // Get the brand icon e.g. McDonalds
      imageName = ::getSpecialPOIImage( *m_group, *m_thread, match );

      if ( imageName.empty() ) {
         ItemTypes::pointOfInterest_t poiType =
            static_cast< ItemTypes::pointOfInterest_t >
            ( match.getItemSubtype() );
         // Get the brand icon e.g. McDonalds
         imageName = getClientSpecificImageName( poiType ); 
         // If no image, try to get image from map descriptor
         if ( imageName.empty() && desc ) {
            imageName = desc->getPOIImageName( poiType );
         }
      }
   }

   if (imageName.empty() ) {
      // Might be parks or parking area.
      imageName = desc ? desc->getItemImageName( match.getItemType() ) : "";
   }

   // if image is empty then use the default for the heading
   if ( imageName.empty() && headingID != MAX_UINT32 ) { 
      try { 
         const SearchHeadingDesc* headingDesc = getHeadingDesc();
         return headingDesc->getHeading( headingID ).m_imageName;
      } catch ( const  std::out_of_range& rangeErr ) {
         mc2log << error << "[SearchParserHandler] getImageName: "
                << rangeErr.what() << endl;
         // the heading id seems to be wrong
         return "";
      }
   }

   return imageName;
}


MC2String
SearchParserHandler::getCategoryImageName( const SearchMatch& match ) const {
   typedef ItemTypes::pointOfInterest_t POIType;

   MC2String imageName;
   const ServerTileMapFormatDesc* desc = m_group->getTileMapFormatDesc( 
      STMFDParams( LangTypes::english, false ),
      m_thread );
   
   if ( match.getItemType() == ItemTypes::pointOfInterestItem ) {
      POIType poiType = static_cast< POIType >( match.getItemSubtype() );
      imageName = getClientSpecificImageName( poiType );
      
      // If no image, try to get image from map descriptor
      if ( imageName.empty() && ( desc != NULL ) ) {
         imageName = desc->getPOIImageName( poiType );
      }
   }

   // A second possibility. Might be parks or parking area.
   if ( imageName.empty() && desc != NULL ) {
      imageName = desc->getItemImageName( match.getItemType() );
   }
   
   return imageName;
}


MC2String
SearchParserHandler::getBrandImageName( const SearchMatch& match ) const {
   if ( match.getItemType() == ItemTypes::pointOfInterestItem ) {
      return ::getSpecialPOIImage( *m_group, *m_thread, match );
   }
   
   return "";
}


MC2String 
SearchParserHandler::getProviderImageName( const SearchMatch& match ) const {
   // External heading/provider
   if ( match.getExtSource() != 0 ) {
      CompactSearchHitType type = getHeadingDesc()->
         findServiceHeadingType( match.getExtSource() );
      if ( type.isValid() ) {
         return type.getImageName();
      }
   } else {
      // Internal provider
      try {
         SearchHeadingDesc::HeadingID headingID;
         
         headingID = getHeadingForMatch ( &match );

         if ( headingID != MAX_UINT32 ) {
            CompactSearchHitType type = 
               getHeadingDesc()->getHeading( headingID );
            // Only get the image if not Places or addresses
            if ( type.getHeading() > 1 ) {
               return type.getImageName();
            }
         }
      } catch ( const  std::out_of_range& rangeErr ) {
         mc2log << error << "[SearchParserHandler] getProviderImageName: "
                << rangeErr.what() << endl;
      }
   }
   return "";
}

MC2String 
SearchParserHandler::getProviderName( const SearchMatch& match ) const {
   // External heading/provider
   if ( match.getExtSource() != 0 ) {
      CompactSearchHitType type = getHeadingDesc()->
         findServiceHeadingType( match.getExtSource() );
      if ( type.isValid() ) {
         return type.m_name;
      }
   } else {
      // Internal provider
      try {
         SearchHeadingDesc::HeadingID headingID;
         
         headingID = getHeadingForMatch ( &match );

         if ( headingID != MAX_UINT32 ) {
            CompactSearchHitType type = 
               getHeadingDesc()->getHeading( headingID );
            // Only get the image if not Places or addresses
            if ( type.getHeading() > 1 ) {
               return type.m_name;
            }
         }
      } catch ( const  std::out_of_range& rangeErr ) {
         mc2log << error << "[SearchParserHandler] getProviderName: "
                << rangeErr.what() << endl;
      }
   }
   return "";
}

SearchParserHandler::Category
SearchParserHandler::findCategoryFromList( const MC2String& str ) const {
   // setup category
   Category cat = { str, StringTable::NOSTRING };
   Categories::const_iterator it = m_categories.find( cat );
   if ( it == m_categories.end() ) {
      return INVALID_CATEGORY;
   }

   return (*it);
}

SearchParserHandler::Category
SearchParserHandler::
findCategoryFromListLowerCase( const MC2String& inStr ) const {
   MC2String str = StringUtility::copyLower( inStr );
   // setup category
   Category cat = { str, StringTable::NOSTRING };
   Categories::const_iterator it = m_lowerCaseCategories.find( cat );
   if ( it == m_lowerCaseCategories.end() ) {
      return INVALID_CATEGORY;
   }

   return (*it);
}

SearchParserHandler::RegionInfo::RegionInfo():
   m_headings(),
   m_topMatch( NULL ) {
}

SearchParserHandler::RegionInfo::~RegionInfo() {
   delete m_topMatch;
}

bool SearchParserHandler::
getRegionInfo( const RegionRequest& request, RegionInfo& info ) {
   uint32 dist = 0; // not used
   const TopRegionRequest* topReq = m_group->
      getTopRegionRequest( m_thread );
      
   // find "where" field from municipal or something.
   CoordinateRequest req( m_thread->getNextRequestID(),
                          topReq,
                          NameUtility::
                          getBestLanguage( request.m_language, dist ) );
   if ( ! PositionInfo::getInfoFromPosition( *m_thread,
                                             req, topReq,
                                             request.m_position,
                                             info.m_topMatch ) ) {
      return false;
   }

   // we need to clone it, topReq will otherwise destroy it.
   if ( info.m_topMatch ) {
      info.m_topMatch = new TopRegionMatch( *info.m_topMatch );
   }

   //
   // Determine which headings we can search in, in this top region
   //
   const SearchHeadingDesc* headingDesc = getHeadingDesc();

   CompactSearchHitTypeVector headings;
   headingDesc->getHeadings( *m_thread->getCurrentUser()->getUser(),
                             headings);

   for ( CompactSearchHitTypeVector::iterator it = headings.begin();
         it != headings.end(); ++it ) {
      if ( (*it).m_topRegionID == info.m_topMatch->getID() ) {
         if ( ! (*it).isValid() ) {
            continue;
         }
         
         // Validate top region for this descriptor
         map<int, MC2String> searchFields;
         if ( ! setTopRegionField( searchFields,
                                   info.m_topMatch->getID(),
                                   (*it).m_serviceID) ) {
            continue;
         }
         
         // translate the heading we found and add it to reply
         headingDesc->translateHitType( *it, request.m_language );
         info.m_headings.push_back( *it ); 
         
      }
   }
   
   return true;
}

bool
SearchParserHandler::setTopRegionField( map<int, MC2String>& searchFields, uint32 topRegion, uint32 serviceId ) {  

   StringTable::countryCode countryCode = ExternalSearchHelper::topRegionToCountryCode(topRegion, serviceId);

   if ( countryCode == StringTable::NBR_COUNTRY_CODES ) {
      return false;
   }

   // Country field
   searchFields[ ExternalSearchConsts::country ] = STLStringUtility::uint2str( countryCode );

   return true;
}

void
SearchParserHandler::addCountryToResults( 
   const SearchRequestParameters& params,
   SearchResultRequest* req ) {
   postSearchAddCountry( req, m_thread, params );
}

void
SearchParserHandler::addCountryToResults( 
   const SearchRequestParameters& params,
   SearchResults& results ) {

   for ( SearchResults::const_iterator it = results.begin() ; 
         it != results.end() ; ++it ) {
      SearchResultRequest* resReq = (*it).second->getSearchResultRequest();
      if ( resReq->getStatus() == 
           StringTable::OK ) {
         const uint32 heading = (*it).first;
         const SearchHeadingDesc* headingDesc = getHeadingDesc();
         CompactSearchHitType hitType =
            headingDesc->getHitTypeFromHeading( heading );
         if ( hitType.m_round == 0 ) {
            // Use the mapIDs to get the Country
            postSearchAddCountry( resReq, m_thread, params );
         } else if ( hitType.m_topRegionID != MAX_UINT32 ) {
            // Use that topRegion
            postSearchAddCountry( resReq, m_thread, params, 
                                  hitType.m_topRegionID );
         } // TODO: Else we might do some lookup
      }
   }
}

void 
SearchParserHandler::addCountryToResults( 
   const SearchRequestParameters& params, SearchMatch* match ) {
   if ( match->getExtSource() == ExternalSearchConsts::not_external ) {
      postSearchAddCountry( match, m_thread, params );
   } else {
      // External
      CompactSearchHitType hitType = 
         getHeadingDesc()->findServiceHeadingType( 
            match->getExtSource() );
      if ( hitType.m_topRegionID != MAX_UINT32 ) {
         // Use that topRegion
         postSearchAddCountry( match, m_thread, params, 
                               hitType.m_topRegionID );
      }
   }
}

uint32
SearchParserHandler::getHeadingForMatch( const SearchMatch* match ) const {
   uint32 heading = MAX_UINT32;

   if ( match->getExtSource() == ExternalSearchConsts::not_external ) {
      // Check round 0 headings if match's type is in m_searchTypeMask
      const SearchHeadingDesc* headingDesc = getHeadingDesc();
      heading = headingDesc->findSearchTypeHeading( match->getType() );
   } else {
      // External
      heading = getHeadingDesc()->findServiceHeading( 
         match->getExtSource() );
   }

   return heading;
}

uint32
SearchParserHandler::getSearchTypeForHeading( uint32 headingID ) const {
   uint32 searchType = 0;

   const SearchHeadingDesc* headingDesc = getHeadingDesc();
   CompactSearchHitType hitType = headingDesc->getHitTypeFromHeading( 
      headingID );
   if ( hitType.isValid() ) {
      searchType = hitType.m_searchTypeMask;
   }

   return searchType;
}

uint32
SearchParserHandler::getRoundForHeading( uint32 headingID ) const {
   uint32 round = MAX_UINT32;

   const SearchHeadingDesc* headingDesc = getHeadingDesc();
   CompactSearchHitType hitType = headingDesc->getHitTypeFromHeading( 
      headingID );
   if ( hitType.isValid() ) {
      round = hitType.m_round;
   }

   return round;
}

uint32
SearchParserHandler::getExternalSourceForHeading( uint32 headingID,
                                                  MC2String& extID ) const {
   uint32 source = MAX_UINT32;

   const SearchHeadingDesc* headingDesc = getHeadingDesc();
   CompactSearchHitType hitType = headingDesc->getHitTypeFromHeading( 
      headingID );
   if ( hitType.isValid() ) {
      source = hitType.m_serviceID;
      extID = hitType.m_name;
   }

   return source;
   
}

inline const SearchHeadingDesc* 
SearchParserHandler::getHeadingDesc() const {

   const ClientSetting* clientSetting = m_thread->getClientSetting();

   if ( clientSetting ) {
      // The list should be short, so do linear search.
      for ( size_t i = 0; i < m_headings.size(); ++i ) {
         if ( m_headings[ i ].first == clientSetting->getImageSet() ) {
            return m_headings[ i ].second;
         }
      }
   }

   // return default type if we couldn't find a specific one.
   return m_headings[ 0 ].second;
}

void
SearchParserHandler::handleSmartCategorySearch( 
   CompactSearch& search,
   CategoryTreeUtils::CategorySet& categories,
   const CategoryTreeUtils::CategoryTree& tree ) {
   // Will conatin the words that we did not find any categories for
   vector< MC2String > newWhat;
   // Find categories by looking for exact matches in the category tree
   MC2String what( search.m_what );
   if ( search.m_what.empty() && ! search.m_categoryName.empty() ) {
      what = search.m_categoryName;
   }
#if 1
   tree.findNameCategories
#else
   tree.findCategories
#endif
      ( what,
        search.m_language,
        categories,
        true,
        &newWhat );
   // Construct a new string with the words not related to a category.
   // This will lead to a search within the categories that where found
   // and conatins the words in m_what, if any are present.
   search.m_what = STLStringUtility::join( newWhat );
}


SearchParserHandler::SearchResultsData::SearchResultsData(
   SearchResultRequest* request,
   const CompactSearchHitType&  hitType) :
      m_searchResultReq( request ),
      m_oneSearchHeadingHitType( hitType ) {
}

SearchParserHandler::SearchResultsData::~SearchResultsData() {
   delete m_searchResultReq;
   m_searchResultReq = NULL;
}

uint32 
SearchParserHandler::getTopRegionForMatch( const SearchMatch* match ) const {
   if ( match->getExtSource() == ExternalSearchConsts::not_external ) {
      // Internal
      const TopRegionMatch* topRegMatch = m_thread->getTopRegionRequest()->
         getCountryForMapID( match->getMapID() );
      if ( topRegMatch != NULL ) {
         return topRegMatch->getID();
      }
   } else {
      // External
      CompactSearchHitType hitType = 
         getHeadingDesc()->findServiceHeadingType( 
            match->getExtSource() );
      if ( hitType.m_topRegionID != MAX_UINT32 ) {
         // Use that topRegion
         return hitType.m_topRegionID;
      } 
   }
   
   // Fallback solution, lookup from coordinate
   return PositionInfo::getTopRegionFromPosition( *m_thread,
                                                  m_thread->getTopRegionRequest(),
                                                  match->getCoords() );    
}
