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
// Google Local Search API
// - API: http://code.google.com/apis/ajaxsearch/documentation/reference.html
// - JSON based
// - HTTP GET
// - A maximum of 8 hits per page
// - One field input
// - Can do coordinate search
//
// About this parser:
// - Uses JPath, similar to XPath but for JSON.
// - Composes one field from the "what" and "where" field.
// - Appends the country name to the field
//

#include "GoogleTalker.h"

#include "SearchReplyData.h"
#include "ExternalSearchRequestData.h"
#include "ExternalSearchDesc.h"
#include "ExtServices.h"

#include "URLFetcher.h"
#include "HttpHeader.h"
#include "URL.h"
#include "URLParams.h"

#include "WGS84Coordinate.h"

#include "FileUtils.h"

#include <json_spirit/json_spirit_reader_template.h>

#include "JPathMultiExpression.h"
#include "JPathAssigner.h"
#include "JPathMatchHandler.h"

#include "TalkerUtility.h"
#include "CoordinateTransformer.h"
#include "SearchFields.h"

#include "STLUtility.h"
#include "Properties.h"
#include "StringUtility.h"
#include "MatchHelp.h"
#include "ExtInfoQuery.h"
#include "SearchRequestParameters.h"
#include "Encoding.h"

#include "MaxCount.h"

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

/// WGS84 coordinates as strings
struct WGS84CoordinateString {
   MC2String m_lat;
   MC2String m_lon;
};

/// stream wgs84 coordinate string
ostream& operator << ( ostream& ostr, const WGS84CoordinateString& coord ) {
   return ostr << coord.m_lat << ", " << coord.m_lon;
}

/// Single match parsed from JSON document.
struct GoogleMatch {
   MC2String m_title;
   MC2String m_address;
   MC2String m_city;
   WGS84CoordinateString m_coord;
   MC2String m_url;
};

/// Stream the GoogleMatch.
ostream& operator << ( ostream& ostr, const GoogleMatch& match ) {
   ostr << "Title: " << match.m_title << endl;
   ostr << "Address: " << match.m_address << endl;
   ostr << "City: " << match.m_city << endl;
   ostr << "Coord: " << match.m_coord << endl;
   ostr << "URL: " << match.m_url << endl;
   return ostr;
}

/// The handler for Google matches.
class GoogleMatches: public MC2JSON::JPath::MatchHandler<GoogleMatch> {
public:
};

namespace {
/**
 * @param pageID current page id.
 * @param searchIndex Search match index.
 * @return base64 encoded id.
 */
MC2String encodeID( const MC2String& pageID,
                    uint32 searchIndex ) {
   stringstream idstr;
   idstr << pageID << ":" << searchIndex;
   return StringUtility::base64Encode( idstr.str() );
}

/**
 * Decode search match id.
 * @param idBase64 the base64 encoded search match id.
 * @param what Will contain the "what" field on success.
 * @param where Will contain the "where" field on success.
 * @param pageOffset Will contain the page offset on success.
 * @return true if decode succeeded.
 */
bool decodeID( const MC2String& idBase64,
               MC2String& what, MC2String& where,
               uint32& pageOffset ) try {

   MC2String id = Encoding::base64Decode( idBase64 );

   // Format:
   // :what:where:pageOffset
   // "pages" is space separated list with numbers

   typedef boost::char_separator< char > separator;
   typedef boost::tokenizer< separator > tokenizer;
   tokenizer tokens( id, separator( ":" ) );

   // must have five fields
   if ( distance( tokens.begin(), tokens.end() ) != 3 ) {
      return false;
   }

   tokenizer::iterator it = tokens.begin();

   // parse the standard field
   what = *it++;

   where = *it++;

   pageOffset = boost::lexical_cast< uint32 >( *it );
   mc2dbg << "PageOffset: " << pageOffset << endl;
   return true;

} catch ( const boost::bad_lexical_cast& bc ) {
   mc2log << "[GoogleTalker] " << bc.what() << endl;
   return false;
} catch ( const std::exception& e ) {
   mc2log << "[GoogleTalker] " << e.what() << endl;
   return false;
}

/**
 * Create search matches from parsed Google matches.
 * @param finalMatches Will be filled with search matches.
 * @param matches Parsed matches.
 * @param what client search param.
 * @param where client search param.
 * @param pageOffset offset in the page.
 * @param lang Client requested language.
 */
void createSearchMatches( vector< VanillaMatch* >& finalMatches,
                          const vector< GoogleMatch >& matches,
                          const MC2String& what, const MC2String& where,
                          uint32 pageOffset,
                          const LangType& lang,
                          ItemInfoEnums::InfoTypeFilter infoFilter ) {

   MC2String pageID = what + ":" + where + ":";

   mc2dbg << "[Google] nbr matches: " << matches.size() << endl;
   for ( uint32 matchIndex = 0; matchIndex < matches.size(); ++matchIndex ) {
      const GoogleMatch& match = matches[ matchIndex ];

      //
      // setup setup info vector
      //

      TalkerUtility::AddInfo info( lang, infoFilter );

      info.add( match.m_address, ItemInfoEnums::vis_address );
      info.add( match.m_city, ItemInfoEnums::Vis_zip_area );
      info.add( match.m_url, ItemInfoEnums::url );

      info.add( MC2String( "Google" ), ItemInfoEnums::supplier );

      //
      // create company match, no person matches in local searches.
      //
      auto_ptr<SearchMatch> searchMatch( SearchMatch::
                                         createMatch( SEARCH_COMPANIES ) );

      //
      // Setup search match
      //

      MC2String location = match.m_address;
      if ( !location.empty() && !match.m_city.empty() ) {
         location += ", ";
      }
      if ( !match.m_city.empty() ) {
         location += match.m_city;
      }

      searchMatch->setLocationName( location.c_str() );

      MC2Coordinate coord;
      if ( ! match.m_coord.m_lat.empty() &&
           ! match.m_coord.m_lon.empty() ) {
         using boost::lexical_cast;
         coord = CoordinateTransformer::
            transformToMC2( CoordinateTransformer::wgs84deg,
                            lexical_cast<float64>( match.m_coord.m_lat ),
                            lexical_cast<float64>( match.m_coord.m_lon ), 0 );
      }

      searchMatch->setCoords( coord );

      searchMatch->
         setExtSourceAndID( ExtService::google_local_search,
                            encodeID( pageID, pageOffset + matchIndex ) );

      searchMatch->swapItemInfos( info.getInfoVector() );
      searchMatch->setAdditionalInfoExists( info.additionalInfoExits() );

      searchMatch->setName( match.m_title.c_str() );

      VanillaMatch* realMatch =
         dynamic_cast<VanillaMatch*>( searchMatch.release() );

      MC2_ASSERT( realMatch );

      finalMatches.push_back( realMatch );

   }
}

} // anonymous

GoogleTalker::GoogleTalker():
   ExtServiceTalker( "Google" ),
   m_urlFetcher( new URLFetcher() ),
   m_matches( new GoogleMatches() ),
   m_estimatedResultCount( 0 ) {

   using namespace MC2JSON;
   using namespace JPath;
   GoogleMatch& currMatch = m_matches->getCurrMatch();
   // setup path expression
   MC2JSON::JPath::MultiExpression::NodeDescription desc[] = {
      { "responseData/results/GsearchResultClass*", m_matches },
      { "responseData/results/city", makeAssigner( currMatch.m_city ) },
      { "responseData/results/titleNoFormatting",
        makeAssigner( currMatch.m_title ) },
      { "responseData/results/lat", makeAssigner( currMatch.m_coord.m_lat ) },
      { "responseData/results/lng", makeAssigner( currMatch.m_coord.m_lon ) },
      { "responseData/results/url", makeAssigner( currMatch.m_url ) },
      { "responseData/cursor/estimatedResultCount",
        makeAssigner( m_estimatedResultCount ) },
   };

   uint32 descSize = sizeof( desc ) / sizeof( desc[ 0 ] );

   m_expr.reset( new MultiExpression( MultiExpression::
                                      Description( desc, desc + descSize ) ) );

}

GoogleTalker::~GoogleTalker() {
}

namespace {

const int32 PAGE_SIZE = 8; // 8 = page size for "rsz=large"

MC2String createSearchURL( const ExternalSearchRequestData& searchData,
                           MC2String& what, MC2String& where ) {

   what = TalkerUtility::getWhatField( searchData, 
                                       TalkerUtility::FIELD_YELLOWPAGES_WITH_CATEGORY );
   where = searchData.getVal( ExternalSearchDesc::city_or_area );

   URLParams params;
   params.add( "v", "1.1" );
   params.add( "mrt", "localonly" );

   // add coordinate, if we have any
   if ( searchData.getCoordinate().isValid() ) {
      WGS84Coordinate coord = CoordinateTransformer::
         transformToWGS84( searchData.getCoordinate() );
      MC2String coords = boost::lexical_cast< MC2String >( coord.lat );
      coords += ",";
      coords += boost::lexical_cast< MC2String >( coord.lon );
      params.add( "sll", coords );
      params.add( "q", what );
   } else {
      // Add the top region bounding box
      params.add( "sll", searchData.
                  getVal( ExternalSearchDesc::top_region_center ) );
      params.add( "sspn", searchData.
                  getVal( ExternalSearchDesc::top_region_span ) );

      // The country name and location needs to be added if
      // no coordinate was supplied.
      params.add( "q", what + " " + where );
   }

   params.add( "rsz", "large" ); // = 8 hits per page
   // set the start to even number of pages
   params.add( "start", ( searchData.getStartHitIdx() / PAGE_SIZE ) * PAGE_SIZE );

   MC2String url( Properties::
                  getProperty( "GOOGLE_LOCAL_SEARCH_URL",
                               "http://ajax.googleapis.com/ajax/services/search/local" ) );

   url += params;

   return url;
}

} // anonymous

int 
GoogleTalker::doQuery( SearchReplyData& reply,
                       const ExternalSearchRequestData& searchData,
                       int nbrRetries ) try {

   MC2String what, where;
   MC2String url = ::createSearchURL( searchData, what, where );
   mc2dbg << "[GoogleTalker] url = " << url << endl;

   MC2String jsonResult;
   HttpHeader urlHeader;
   m_urlFetcher->get( jsonResult, url,
                      5000,  // timeout in ms
                      &urlHeader );

   mc2dbg8 << "[GoogleTalker] jsonResult: " << jsonResult << endl;

   MC2JSON::JPath::NodeType jsonNode;
   if ( ! json_spirit::read_string( jsonResult, jsonNode ) ) {
      mc2dbg << "[GoogleTalker] No result parsed." << endl;
      return 0;
   }

   // clear old matches
   m_estimatedResultCount = 0;
   m_matches->reset();

   // evaluate jpath expression on json document
   m_expr->evaluate( jsonNode );
   m_matches->handleNewMatch(); // finalize the last match


   const int32 LAST_OFFSET = 32; // final offset of page 4

   // trim matches, so we actually get the number of matches we want
   TalkerUtility::cutMatches( searchData, m_matches->getMatches(),
                              ::PAGE_SIZE );

   // Setup reply
   ::createSearchMatches( reply.getMatchVector(), m_matches->getMatches(),
                          what, where,
                          searchData.getStartHitIdx(),
                          searchData.getLang(),
                          searchData.getInfoFilterLevel() );


   // Use the maximum number of result, if there should
   // be any problem with estimated result count.
   uint32 maxHits = TalkerUtility::
         maxCount( searchData.getStartHitIdx(),
                   searchData.getEndHitIdx(),
                   reply.getMatchVector().size(),
                   m_estimatedResultCount,
                   LAST_OFFSET, ::PAGE_SIZE );

   reply.setNbrHits( searchData.getStartHitIdx(),
                     reply.getMatchVector().size(),
                     maxHits );
   // reset matches
   m_matches->reset();
   m_estimatedResultCount = 0;

   return 1;
} catch ( const std::exception& e ) {
   mc2log << warn << "[GoogleTalker] doQuery: " << e.what() << endl;
   return 0;
}


int 
GoogleTalker::doQuery( SearchReplyData& reply,
                       const ExtService& service,
                       const MC2String& externalId,
                       const LangType& lang,
                       int nbrRetries ) {

   return 0;
}

int 
GoogleTalker::doInfoQuery( ExtInfoQuery& info ) {
   SearchRequestParameters params;
   params.setRequestedLang( info.getLang() );
   params.setEndHitIndex( 0 );
   //
   // Decode search match id and then do a search with it to fetch one item
   //
   MC2String what, where;
   uint32 startOffset = 0;

   if ( ! decodeID( info.getExternalID(), what, where, startOffset ) ) {
      mc2log << "[GoogleTalker] bad search id: "
             << "\"" << info.getExternalID() << "\"" << endl;
      return 0;
   }

   ExternalSearchRequestData::stringMap_t values;
   values[ ExternalSearchDesc::company_or_search_word ] = what;
   values[ ExternalSearchDesc::city_or_area ] = where;
   // ignore ::country here, the top region setting will fail in doQuery,
   // which is what we want since we already have country name in the
   // where field.
   ExternalSearchRequestData data( params, info.getServiceID(),
                                   values,
                                   startOffset, // start offset
                                   1 ); // max hits

   return doQuery( info.getReply(), data, info.getNbrRetries() );
}
