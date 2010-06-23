/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//
// This unit test tests the SearchMatches
//


#include "MC2UnitTestMain.h"
#include "SearchMatch.h"
#include "Packet.h"
#include "ItemInfoEntry.h"



/**
 * Compare the base level, SearchMatch.
 */
void testMatch( SearchMatch& testMatch,
                SearchMatch& otherMatch,
                uint32 line ) {

#define COMPARE_MATCH( function ) \
   MC2_TEST_CHECK_EXT( testMatch.function() == otherMatch.function(), line )

   COMPARE_MATCH( getItemInfos );
   COMPARE_MATCH( getType );
   COMPARE_MATCH( getPointInfo );
   COMPARE_MATCH( getPointsForWriting );
   COMPARE_MATCH( getRestrictions );
   COMPARE_MATCH( getDistance );
   COMPARE_MATCH( getNbrRegions );
   COMPARE_MATCH( getOffset );
   COMPARE_MATCH( getItemSubtype );
   COMPARE_MATCH( getItemType );
   COMPARE_MATCH( getID );
   COMPARE_MATCH( getItemID );
   COMPARE_MATCH( getMapID );
   COMPARE_MATCH( getBBox );
   COMPARE_MATCH( getCoords );
   COMPARE_MATCH( getAngle );
   COMPARE_MATCH( getExtID );
   COMPARE_MATCH( getExtSource );
   COMPARE_MATCH( getTypeid );
   COMPARE_MATCH( getAdditionalInfoExists );
#undef COMPARE_MATCH

   // Compare strings
#define COMPARE_MATCHSTR( function ) \
   MC2_TEST_CHECK_EXT( strcmp( testMatch.function(), \
                               otherMatch.function() ) == 0, line )

   COMPARE_MATCHSTR( getLocationName );
   COMPARE_MATCHSTR( getAlphaSortingName );
   COMPARE_MATCHSTR( getName );

#undef COMPARE_MATCHSTR

}

// Test country match
MC2_UNIT_TEST_FUNCTION( countryMatchTest  ) {
   // 65000 bytes should be enough.
   Packet packet( 65000 );

   VanillaCountryMatch country( "Sweden", 1 );
   country.setAdditionalInfoExists( true );

   int pos = REQUEST_HEADER_SIZE;
   // save in non-compact form
   country.save( &packet, pos, false );


   pos = REQUEST_HEADER_SIZE;
   auto_ptr<SearchMatch> countryCopyPtr( SearchMatch::
                                         createMatch( &packet, pos, false ) );

   VanillaCountryMatch&
      countryCopy( dynamic_cast< VanillaCountryMatch& >( *countryCopyPtr ) );

   MC2_TEST_CHECK( countryCopy.getTopRegionID() == country.getTopRegionID() );

   testMatch( country, countryCopy, __LINE__ );

}

bool eqReview( const POIReview& r1, const POIReview& r2 ) {
   return r1.getRating() == r2.getRating() &&
      r1.getReviewer() == r2.getReviewer() &&
      r1.getReviewText() == r2.getReviewText() &&
      r1.getDate() == r2.getDate();
}

// Test company match
MC2_UNIT_TEST_FUNCTION( companyMatchTest  ) {
   // 65000 bytes should be enough.
   Packet packet( 65000 );

   MC2String name( "The Search Match" );
   MC2String locationName( "London City" );

   VanillaCompanyMatch theMatch( IDPair_t(),
                                 name.c_str(), 
                                 locationName.c_str(),/*locationName*/
                                 0/*offset*/, 
                                 0/*streetNumber*/ );

   // Add regions
   MC2String regionStr( "Big London" );
   VanillaRegionMatch region1( regionStr.c_str(), 5,2,33,SEARCH_BUILT_UP_AREAS, SearchMatchPoints(), 1, regionStr.c_str(),
                               44, 3, ItemTypes::municipalItem, 2 );

   MC2String regionStr2( "Small London" );
   VanillaRegionMatch region2( regionStr2.c_str(), 6,1,13,SEARCH_BUILT_UP_AREAS, SearchMatchPoints(), 2, regionStr2.c_str(),
                               45, 4, ItemTypes::municipalItem, 3 );

   theMatch.addRegion( &region1 );
   theMatch.addRegion( &region2 );
   
   // Add categories
   vector< uint16 > categories;
   categories.push_back( 21 );
   categories.push_back( 13 );
   categories.push_back( 456 );
   categories.push_back( 8 );
   theMatch.setCategories( categories );

   // Add Item infos
   SearchMatch::ItemInfoVector infoVec;
   ItemInfoEntry e1( "KeyNumber1", "Value1", ItemInfoEnums::vis_address );
   ItemInfoEntry e2( "ZipCode", "24755", ItemInfoEnums::vis_zip_code );
   infoVec.push_back( e1 );
   infoVec.push_back( e2 );
     theMatch.swapItemInfos( infoVec );
   theMatch.setAdditionalInfoExists( true );

   // Add reviews
   VanillaCompanyMatch::Reviews reviews;
   reviews.push_back( POIReview( 5, "Delfinen", "2010-01-05", "The place to eat when working in Lund :-)") );
   reviews.push_back( POIReview( 3, "user123", "2010-02-20", "OK place." ) );
   theMatch.swapReviews( reviews );

   // Add ImageURLs
   VanillaCompanyMatch::ImageURLs urls;
   urls.push_back( "http://assets1.qypecdn.net/uploads/photos/0060/6634/Bild087_gallery.jpg?34198" );
   urls.push_back( "http://assets1.qypecdn.net/uploads/photos/0026/9171/Bild074_gallery.jpg?34198" );
   theMatch.swapImageURLs( urls );

   int pos = REQUEST_HEADER_SIZE;
   theMatch.save( &packet, pos, false );

   pos = REQUEST_HEADER_SIZE;
   auto_ptr<SearchMatch> companyCopyPtr( SearchMatch::
                                         createMatch( &packet, pos, false ) );

   VanillaCompanyMatch&
      companyCopy( dynamic_cast< VanillaCompanyMatch& >( *companyCopyPtr ) );

   // Check image URLs and reviews
   MC2_TEST_CHECK( equal( theMatch.getImageURLs().begin(), theMatch.getImageURLs().end(),
                          companyCopy.getImageURLs().begin() ) );

   MC2_TEST_CHECK( equal( theMatch.getReviews().begin(), theMatch.getReviews().end(),
                          companyCopy.getReviews().begin(), eqReview ) );

   testMatch( theMatch, companyCopy, __LINE__ );
    
}

// Test mergeToSaneItemInfos
MC2_UNIT_TEST_FUNCTION( mergeToSaneItemInfosTest ) {
   std::auto_ptr<SearchMatch> sm(new SearchMatch(0, 0, IDPair_t(), 
  				 "mergeToSaneItemInfosTest", "locName") );

   // Add some item infos
   SearchMatch::ItemInfoVector itemInfos;
   itemInfos.push_back( ItemInfoEntry("key:text","val:text", 
                                      ItemInfoEnums::text));
   itemInfos.push_back( ItemInfoEntry("key:url","val:url", 
                                      ItemInfoEnums::url) );
   itemInfos.push_back( ItemInfoEntry("key:address","val:address", 
                                      ItemInfoEnums::vis_address) );
   itemInfos.push_back( ItemInfoEntry("key:house_nbr","val:house_nbr", 
                                      ItemInfoEnums::vis_house_nbr) );
   itemInfos.push_back( ItemInfoEntry("key:zip_code","val:zip_code", 
                                      ItemInfoEnums::vis_zip_code) );
   itemInfos.push_back( ItemInfoEntry("key:complete_zip","val:complete_zip", 
                                      ItemInfoEnums::vis_complete_zip) );
   itemInfos.push_back( ItemInfoEntry("key:zip_area","val:zip_area", 
                                      ItemInfoEnums::Vis_zip_area) );
   itemInfos.push_back( ItemInfoEntry("key:full_address","val:full_address", 
                                      ItemInfoEnums::vis_full_address) );
   
   // Add the item infos to the SearchMatch
   sm->swapItemInfos( itemInfos );

   // Call tested method
   sm->mergeToSaneItemInfos( LangTypes::english, 0 );

   // Make some checks
   const SearchMatch::ItemInfoVector resVector = sm->getItemInfos();
   MC2_TEST_CHECK( resVector.size() == 4 );
   for ( SearchMatch::ItemInfoVector::const_iterator it = resVector.begin() ;
         it != resVector.end() ; ++it ) {
      if ( ! ( it->getInfoType() == ItemInfoEnums::text ||
               it->getInfoType() == ItemInfoEnums::url  ||
               it->getInfoType() == ItemInfoEnums::vis_address ||
               it->getInfoType() == ItemInfoEnums::vis_full_address ) ) {
         mc2log << "Failed test mergeToSaneItemInfosTest! ItemInfo type " 
                << it->getInfoType() << " " << it->getKey() 
                << " " << it->getVal() << endl;
         MC2_TEST_CHECK( false );
      } else {
         MC2_TEST_CHECK( true );
      }
   }
}
