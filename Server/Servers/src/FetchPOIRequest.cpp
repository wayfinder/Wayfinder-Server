/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "FetchPOIRequest.h"

#include "MapRequester.h"
#include "SearchRequestParameters.h"
#include "GfxConstants.h"
#include "MC2BoundingBox.h"
#include "ProximityRequest.h"
#include "ArrayTools.h"
#include "SearchRequest.h"

namespace POIFetch {

//
// Setup of ranges:
// 0-100km
// 0-20km
// 0-4km
//
// City centres are not included in any range, because...
// they do not want in them in the search hits.
//


// 0-4km
ItemTypes::pointOfInterest_t poisRange1[] = {
   ItemTypes::amusementPark,
   ItemTypes::atm,
   ItemTypes::bank,
   ItemTypes::bowlingCentre,
   ItemTypes::buddhistSite,
   ItemTypes::busStation,
   ItemTypes::cafe,
   ItemTypes::casino,
   ItemTypes::church,
   ItemTypes::cinema,
   ItemTypes::cityHall,
   ItemTypes::commuterRailStation,
   ItemTypes::company,
   ItemTypes::courtHouse,
   ItemTypes::ferryTerminal,
   ItemTypes::golfCourse,
   ItemTypes::groceryStore,
   ItemTypes::hinduTemple,
   ItemTypes::historicalMonument,
   ItemTypes::iceSkatingRink,
   ItemTypes::library,
   ItemTypes::marina,
   ItemTypes::mosque,
   ItemTypes::museum,
   ItemTypes::nightlife,
   ItemTypes::parkAndRide,
   ItemTypes::placeOfWorship,
   ItemTypes::policeStation,
   ItemTypes::postOffice,
   ItemTypes::railwayStation,
   ItemTypes::recreationFacility,
   ItemTypes::restArea,
   ItemTypes::restaurant,
   ItemTypes::school,
   ItemTypes::shop,
   ItemTypes::skiResort,
   ItemTypes::sportsActivity,
   ItemTypes::subwayStation,
   ItemTypes::synagogue,
   ItemTypes::theatre,
   ItemTypes::tollRoad,
   ItemTypes::touristAttraction,
   ItemTypes::touristOffice,
   ItemTypes::tramStation,
   ItemTypes::university,
   ItemTypes::vehicleRepairFacility,
   ItemTypes::winery,
   ItemTypes::wlan
};
set<ItemTypes::pointOfInterest_t> poisRange4km( BEGIN_ARRAY( poisRange1 ),
                                                END_ARRAY( poisRange1 ) );
// 0-20km
ItemTypes::pointOfInterest_t poisRange2[] = {
   ItemTypes::openParkingArea,
   ItemTypes::parkingGarage,
   ItemTypes::petrolStation,
   ItemTypes::rentACarFacility,
   ItemTypes::hospital,
   ItemTypes::hotel,
};
set<ItemTypes::pointOfInterest_t> poisRange20km( BEGIN_ARRAY( poisRange2 ),
                                                 END_ARRAY( poisRange2 ) );

// 0-100km
ItemTypes::pointOfInterest_t poisRange3[] = {
   ItemTypes::airport
};

set<ItemTypes::pointOfInterest_t> poisRange100km( BEGIN_ARRAY( poisRange3 ),
                                                  END_ARRAY( poisRange3 ) );

/**
 * Determine which item types to include at a certain radius.
 * @param radius Search radius in meters.
 * @param itemTypes Will return the items to include in the search.
 */
void getSearchTypes( uint32 radius,
                     SearchRequestParameters::POITypeSet& poiTypes ) {

   // 4km
   if ( radius <= 4000 ) {
      poiTypes = poisRange4km;
   }

   // 20km
   if ( radius <= 20000 ) {
      poiTypes.insert( poisRange20km.begin(),
                       poisRange20km.end() );
   }

   // 100km
   if ( radius <= 100000 ) {
      poiTypes.insert( poisRange100km.begin(),
                       poisRange100km.end() );
   }

}

SearchResult fetchPOIs( MapRequester& requester, const Request& request ) {

   //
   // Setup default parameters
   //
   SearchRequestParameters params;

   params.setSortingType( SearchTypes::DistanceSort );
   params.setLookupCoordinates( false );
   params.setUniqueOrFull( false );
   params.setSearchForLocationTypes( SEARCH_ALL_REGION_TYPES );
   params.setRequestedLang( request.getLanguage() );
   params.setAddStreetNamesToCompanies( false );
   params.setTryHarder( true );
   params.setEndHitIndex( request.getEndIndex() );
   params.setNbrSortedHits( request.getEndIndex() - 1 );
   params.setRegionsInMatches( SEARCH_ALL_REGION_TYPES );
   params.setRegionsInOverviewMatches( SEARCH_ALL_REGION_TYPES );
   params.setSearchForTypes( SEARCH_COMPANIES | SEARCH_MISC );
   // set specific poi types.
   SearchRequestParameters::POITypeSet poiTypes;
   getSearchTypes( request.getRadius(), poiTypes );
   params.setPOITypes( poiTypes );

   // No need to do any search if we do not have any poi types to
   // search for.
   if ( poiTypes.empty() ) {
      return SearchResult( new EmptySearchRequest() );
   }

   // Do a proximity search for POI types.
   set< ItemTypes::itemType > itemTypes;
   itemTypes.insert( ItemTypes::pointOfInterestItem );
   auto_ptr< SearchResultRequest >
      searchRequest;

   // Select proximity request when there is no search string.
   if ( request.getSearchString().empty() ) {
      searchRequest.
         reset( new
                ProximityRequest( requester.getNextRequestID(),
                                  params,
                                  MC2BoundingBox( request.getCenter(),
                                                  request.getRadius() ),
                                  itemTypes,
                                  request.getSearchString().c_str() ) );
   } else {
      searchRequest.
         reset( new
                SearchRequest( requester.getNextRequestID(),
                               params,
                               request.getCenter(),
                               request.getSearchString().c_str(),
                               requester.getTopRegionRequest(),
                               request.getRadius() ) );
   }

   requester.putRequest( searchRequest.get() );

   // If request failed, then no result...obviously
   if ( searchRequest->getStatus() != StringTable::OK ) {
      return SearchResult( new EmptySearchRequest() );
   }

   return searchRequest;
}

} // POIFetch
