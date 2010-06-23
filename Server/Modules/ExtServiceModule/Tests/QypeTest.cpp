/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2UnitTest.h"
#include "QypeHandler.h"
#include "MC2UnitTestMain.h"
#include "SearchReplyData.h"
#include "ExternalSearchRequestData.h"
#include "ExternalSearchConsts.h"
#include "SearchRequestParameters.h"
#include "Properties.h"
#include "SearchMatch.h"
#include "TalkerUtility.h"
#include "LangTypes.h"
#include "ItemInfoEnums.h"

/**
 *   Regression test.
 */
MC2_UNIT_TEST_FUNCTION( createSearchMatchesTest ) {
   Properties::setPropertyFileName( "/dev/null" );
   
   QypeHandler talker;
   SearchReplyData reply;

   mc2log << "Test 1" << endl;
   vector< VanillaMatch* > finalMatches;
   vector< QypePlaceInfo > matches;
   LangType lang;
   MC2String id = "12345";
   ItemInfoEnums::InfoTypeFilter filter = ItemInfoEnums::OneSearch_All;
   bool poiInfoRequest = false;

   talker.createSearchMatches( finalMatches,
                                        matches,
                                        lang,
                                        id,
                                        filter,
                                        poiInfoRequest);

   MC2_TEST_CHECK( finalMatches.size() == 0 );

   mc2log << "Test 2" << endl;
   QypePlaceInfo info1;
   matches.push_back( info1 );

   talker.createSearchMatches( finalMatches,
                               matches,
                               lang,
                               id,
                               filter,
                               poiInfoRequest);

   MC2_TEST_CHECK( finalMatches.size() == 1 );
   delete finalMatches[0];
   finalMatches.clear();
   matches.clear();
   
   mc2log << "Test 3" << endl;
   info1.m_coord = "12.12a3,-a.afdf";
   matches.push_back( info1 );
   talker.createSearchMatches( finalMatches,
                               matches,
                               lang,
                               id,
                               filter,
                               poiInfoRequest );
   
   MC2_TEST_CHECK( finalMatches.size() == 1 );
   MC2_TEST_CHECK( ! finalMatches[0]->getCoords().isValid() );
   delete finalMatches[0];
   finalMatches.clear(); 
   matches.clear();
   
   mc2log << "Test 4" << endl;
   info1.m_coord = "12.12343,-12.12343";
   info1.m_id = "adasdas/1234";
   info1.m_qypeCategoryId = "xaxssdad";
   matches.push_back( info1 );
   talker.createSearchMatches( finalMatches,
                               matches,
                               lang,
                               id,
                               filter,
                               poiInfoRequest );
   
   MC2_TEST_CHECK( finalMatches.size() == 1 );
   MC2_TEST_CHECK( finalMatches[0]->getCoords().isValid() );
   delete finalMatches[0];
   finalMatches.clear();
   matches.clear();
   
   mc2log << "Test 5" << endl;
   info1.m_coord = "12.12343,-12.12343";
   info1.m_id = "adasdas/1234";
   info1.m_qypeCategoryId = "abc/places/categories/372";
   matches.push_back( info1 );
   talker.createSearchMatches( finalMatches,
                               matches,
                               lang,
                               id,
                               filter,
                               poiInfoRequest );
   
   MC2_TEST_CHECK( finalMatches.size() == 1 );
   MC2_TEST_CHECK( finalMatches[0]->getCoords().isValid() );
   MC2_TEST_CHECK( dynamic_cast<VanillaCompanyMatch*>(finalMatches[0])->getCategories().size() == 1 );
   delete finalMatches[0];
   finalMatches.clear();
   matches.clear();
}


MC2_UNIT_TEST_FUNCTION( createURLTest ) {
   Properties::setPropertyFileName( "/dev/null" );
   QypeHandler talker;

   talker.m_locatorCache[ "lund sweden" ] = "se044-lund";

   // Proximity and category search
   {
      MC2String expected("http://api.qype.com/v1/positions/"
                         "0.001034745946527491,0.005690474063163334/places"
                         "?in_category=372&order=distance&radius=13&per_page=15"
                         "&consumer_key=");
      expected.append( Properties::getProperty( "QYPE_CONSUMER_KEY", "" ) );
      ExternalSearchRequestData::stringMap_t values;
      values[ExternalSearchConsts::city_or_area] = "lund";
      values[ExternalSearchConsts::country_name] = "sweden";
      values[ExternalSearchConsts::company_or_search_word] = "search_word";
      values[ExternalSearchConsts::category] = "airport";
      values[ExternalSearchConsts::category_id] = "18";
      
      ExternalSearchRequestData data( SearchRequestParameters(),
                                      0, // service
                                      values,
                                      0, // start hit idx
                                      15, // nbr of hits
                                      ItemInfoEnums::All,
                                      MC2Coordinate( 12345, 67890 ),
                                      13000 // distance 
                                      );

      MC2String url = talker.createURL( data );
      mc2log << "[QypteTalkerTest] Proximity and category: " << url << endl;
      if ( url != expected ) {
         mc2log << "  Ex: " << expected << endl << "  Re: " << url << endl;
      }
      MC2_TEST_CHECK( url == expected );
   }
   // Proximity and what
   {
      MC2String expected("http://api.qype.com/v1/positions/"
                         "0.001034745946527491,0.005690474063163334/places"
                         "?show=search_word&order=distance&radius=13&per_page=16"
                         "&consumer_key=");
      expected.append( Properties::getProperty( "QYPE_CONSUMER_KEY", "" ) );
      ExternalSearchRequestData::stringMap_t values;
      values[ExternalSearchConsts::city_or_area] = "lund";
      values[ExternalSearchConsts::country_name] = "sweden";
      values[ExternalSearchConsts::company_or_search_word] = "search_word";
      
      ExternalSearchRequestData data( SearchRequestParameters(),
                                      0, // service
                                      values,
                                      0, // start hit idx
                                      16, // nbr of hits
                                      ItemInfoEnums::All,
                                      MC2Coordinate( 12345, 67890 ),
                                      13000 // distance 
                                      );
      
      MC2String url = talker.createURL( data );
      mc2log << "[QypteTalkerTest] Proximity and what:     " << url << endl;
      if ( url != expected ) {
         mc2log << "  Ex: " << expected << endl << "  Re: " << url << endl;
      }
      MC2_TEST_CHECK( url == expected );
   }

   // Where and category search
   {
      MC2String expected ("http://api.qype.com/v1/places"
                          "?in_category=372&in_locator=se044-lund&per_page=17"
                          "&consumer_key=");
      expected.append( Properties::getProperty( "QYPE_CONSUMER_KEY", "" ) );
      ExternalSearchRequestData::stringMap_t values;
      values[ExternalSearchConsts::city_or_area] = "lund";
      values[ExternalSearchConsts::country_name] = "sweden";
      values[ExternalSearchConsts::company_or_search_word] = "search_word";
      values[ExternalSearchConsts::category] = "airport";
      values[ExternalSearchConsts::category_id] = "18";
      
      ExternalSearchRequestData data( SearchRequestParameters(),
                                      0, // service
                                      values,
                                      0, // start hit idx
                                      17, // nbr of hits
                                      ItemInfoEnums::All,
                                      MC2Coordinate(),
                                      0 // distance 
                                      );

      MC2String url = talker.createURL( data );
      mc2log << "[QypteTalkerTest] Where and category:     " << url << endl;
      if ( url != expected ) {
         mc2log << "  Ex: " << expected << endl << "  Re: " << url << endl;
      }
      MC2_TEST_CHECK( url == expected);
   }

   // Where and what
   {
      MC2String expected("http://api.qype.com/v1/places"
                         "?show=search_word&in_locator=se044-lund&per_page=18"
                         "&consumer_key=");
      expected.append( Properties::getProperty( "QYPE_CONSUMER_KEY", "" ) );
      ExternalSearchRequestData::stringMap_t values;
      values[ExternalSearchConsts::city_or_area] = "lund";
      values[ExternalSearchConsts::country_name] = "sweden";
      values[ExternalSearchConsts::company_or_search_word] = "search_word";
      
      ExternalSearchRequestData data( SearchRequestParameters(),
                                      0, // service
                                      values,
                                      0, // start hit idx
                                      18, // nbr of hits
                                      ItemInfoEnums::All,
                                      MC2Coordinate(),
                                      0 // distance 
                                      );

      MC2String url = talker.createURL( data );
      mc2log << "[QypteTalkerTest] Where and what:         " << url << endl;
      if ( url != expected ) {
         mc2log << "  Ex: " << expected << endl << "  Re: " << url << endl;
      }
      MC2_TEST_CHECK( url == expected);
   }
}
