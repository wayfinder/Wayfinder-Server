/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AdServerTalker.h"

#include "SearchReplyData.h"
#include "ExternalSearchRequestData.h"
#include "ExternalSearchDesc.h"
#include "SearchReplyData.h"
#include "TalkerUtility.h"
#include "ExtServices.h"
#include "SearchFields.h"

#include "CoordinateTransformer.h"
#include "StringUtility.h"

#include "URL.h"
#include "URLFetcher.h"
#include "HttpHeader.h"

#include "STLStringUtility.h"
#include "Properties.h"
#include "SearchRequestParameters.h"

#include "XPathExpression.h"
#include "XPathMultiExpression.h"
#include "XPathMatchHandler.h"

#include <sax/SAXParseException.hpp>

#include "FileUtils.h"


using namespace XMLTool;
using namespace XMLTool::XPath;

/// Describes a hit from the adertisement server
struct AdServerMatch {
   MC2String m_name; ///< name of the hit
   MC2String m_city; ///< city location
   MC2String m_street; ///< street name
   MC2String m_country; ///< country name
   MC2String m_url; ///< url, if any
   MC2String m_phone; ///< phone number
   MC2String m_email; ///< email address
   MC2String m_category; ///< category string
   float64 m_lat, m_lon;
};


/// the handler for advertisement server matches
class AdServerMatches: public MatchHandler<AdServerMatch> {
public: 
};

namespace {
/**
 * Creates an encoded external search ID.
 * @param what the search item.
 * @param where the search location.
 * @param id number of the search hit.
 * @return encoded external search ID string.
 */
inline MC2String createExtID( const MC2String& what, const MC2String& where,
                              const MC2String& id ) {
    return StringUtility::base64Encode( what + ";" + where + ";" + id );
}

void createSearchMatches( vector<VanillaMatch*>& finalMatches,
                          const vector<AdServerMatch>& matches,
                          const MC2String& what, const MC2String& where,
                          const LangType& lang,
                          uint32 offset, 
                          ItemInfoEnums::InfoTypeFilter infoFilter ) {
   mc2dbg << "[AdServerTalker] nbr matches: " << matches.size() << endl;
   for ( uint32 i = 0; i < matches.size(); ++i ) {
      const AdServerMatch& match = matches[ i ];

      // setup setup info vector

      TalkerUtility::AddInfo info( lang, infoFilter );

      info.add( match.m_street, ItemInfoEnums::vis_address );
      info.add( match.m_city, ItemInfoEnums::Vis_zip_area );
      info.add( match.m_url, ItemInfoEnums::url );
      info.add( match.m_phone, ItemInfoEnums::phone_number );

      info.add( MC2String( "AdServer" ), ItemInfoEnums::supplier );

      // all hits are companies.
      auto_ptr<SearchMatch> searchMatch( SearchMatch::
                                         createMatch( SEARCH_COMPANIES ) );

      searchMatch->setLocationName( match.m_city.c_str() );
  
      MC2Coordinate coord = CoordinateTransformer::
         transformToMC2( CoordinateTransformer::wgs84deg,
                         match.m_lat, match.m_lon, 0 );
  
      searchMatch->setCoords( coord );

      MC2String id( STLStringUtility::uint2str( i + offset ) );
      searchMatch->setExtSourceAndID( ExtService::adserver,
                                      createExtID( what, where, id ));
      
      searchMatch->swapItemInfos( info.getInfoVector() );
      searchMatch->setAdditionalInfoExists( info.additionalInfoExits() );
      MC2String name = match.m_name;
      // add category to name
      if ( ! match.m_category.empty() ) {
         name += "(" + match.m_category + ")";
      }

      searchMatch->setName( name.c_str() );
      
      
      VanillaMatch* realMatch = 
         dynamic_cast<VanillaMatch*>( searchMatch.release() );
      // whoha, this would be something strange.
      MC2_ASSERT( realMatch );

      finalMatches.push_back( realMatch );
      
   }
}

}

AdServerTalker::AdServerTalker():
   ExtServiceTalker( "AdServer" ),
   m_matches( new AdServerMatches() ),
   m_rootExp( new Expression( "/results" ) ),
   m_docCountExp( new Expression( "/results/@total" ) ),
   m_urlFetcher( new URLFetcher() ) {

   AdServerMatch& currMatch = m_matches->getCurrMatch();

   using XMLTool::XPath::makeAssigner;

   // setup expression tree
   MultiExpression::NodeDescription desc[] = {
      { "result*", m_matches },
      { "result/name", makeAssigner( currMatch.m_name ) },
      { "result/city", makeAssigner( currMatch.m_city ) },
      { "result/street", makeAssigner( currMatch.m_street ) },
      { "result/country", makeAssigner( currMatch.m_country ) },
      { "result/url", makeAssigner( currMatch.m_url ) },
      { "result/phone", makeAssigner( currMatch.m_phone ) },
      { "result/email", makeAssigner( currMatch.m_email ) },
      { "result/category", makeAssigner( currMatch.m_category ) },
      { "result/latitude_deg", makeAssigner( currMatch.m_lat ) },
      { "result/longitude_deg", makeAssigner( currMatch.m_lon ) },
   };

   const uint32 descSize = sizeof ( desc ) / sizeof ( desc[0] );
   m_searchExp.reset( new 
                      MultiExpression( MultiExpression::
                                       Description( desc,
                                                    desc + descSize ) ) );
}

AdServerTalker::~AdServerTalker() {

}

int AdServerTalker::
sendRequest( MC2String& xml_result, MC2String& url, 
             const ExternalSearchRequestData& searchData ) {
   
   // setup URL to request data from
   url = Properties::getProperty( "ADSERVER_URL", "http://adserver/");
   
   url += "?country=sweden"; // XXX todo: change
   url += "&offset=";
   STLStringUtility::int2str( searchData.getStartHitIdx(), url );

   url += "&maxhits=";
   STLStringUtility::int2str( searchData.getEndHitIdx() - 
                              searchData.getStartHitIdx(), url );

   url += "&what=" +  
      TalkerUtility::getWhatField( searchData, 
                                   TalkerUtility::FIELD_YELLOWPAGES_WITH_CATEGORY, true );

   url += "&where=" + StringUtility::
      URLEncode( searchData.getVal(ExternalSearchDesc::city_or_area ) );

   
   mc2dbg << "[AdServerTalker] url = " << url << endl;
   
   HttpHeader urlHeader;
   return m_urlFetcher->get( xml_result, url, 5000, &urlHeader );
}


int AdServerTalker::
doQuery( SearchReplyData& reply,
         const ExternalSearchRequestData& searchData, 
         int nbrRetries ) {
   return 0;

   MC2String url;
   MC2String result;

   int res = -1;
   for ( int i = 0; res < 0 && i < nbrRetries; ++i ) {
      res = sendRequest( result, url, searchData );
   }

   if ( res < 0 ) {
      return res;
   }

   if ( result.empty() ) {
      mc2dbg << "[AdServerTalker] No Result to parse." << endl;
      return -1;
   }

   return parseResult( reply, searchData, result, url );
}


int AdServerTalker::
doQuery( SearchReplyData& reply,
         const ExtService& service, 
         const MC2String& externalIDbase64, 
         const LangType& lang, 
         int nbrRetries ) 
try {
   SearchRequestParameters params;   
   params.setRequestedLang( lang );
   params.setEndHitIndex( 0 );

   // create external ID
   MC2String externalID;
   {
      char output[ externalIDbase64.size() ];
      if ( StringUtility::base64Decode( externalIDbase64.c_str(),
                                        (byte*)output) != -1 ) {
         externalID = output;
      }
   }
   
   
   ExternalSearchRequestData::stringMap_t values;

   // parse external id
   uint32 afterWhat = externalID.find_first_of( ";" );
   uint32 afterWhere = externalID.find_first_of( ";", afterWhat + 1 );

   mc2dbg << "[AdServerTalker] ExternalID: " << externalID << endl;

   values[ ExternalSearchDesc::company_or_search_word ] = 
      externalID.substr( 0, afterWhat );

   mc2dbg << "[AdServerTalker] what: " 
          << values[ ExternalSearchDesc::company_or_search_word ] << endl;

   values[ ExternalSearchDesc::city_or_area ] = 
      externalID.substr( afterWhat + 1, afterWhere - afterWhat - 1 );

   mc2dbg << "[AdServerTalker] where: "
          << values[ ExternalSearchDesc::city_or_area ] << endl;

   uint offset = StringConvert::
      convert<uint32>( externalID.
                       substr( afterWhere + 1, 
                               externalID.size() - afterWhere + 1) );
   mc2dbg << "[AdServerTalker] Item offset: " << offset << endl;
   // setup search data
   ExternalSearchRequestData 
      data( params, service, 
            values, // what/where
            offset, // start offset
            1 ); // max hits

   return doQuery( reply, data, nbrRetries );

} catch ( const std::out_of_range& idErr ) {
   mc2dbg << warn << "[AdServer] bad external ID: " << externalIDbase64
          << "( " << idErr.what() << " )" << endl;
   return -1;
} catch ( const StringConvert::ConvertException& e ) {
   mc2dbg << warn << "[AdServer] Could not convert external ID: " << externalIDbase64
          << "( " << e.what() << " )" << endl;
   return -1;
}

int AdServerTalker::
parseResult( SearchReplyData& reply, 
             const ExternalSearchRequestData& searchData,
             const MC2String& xml_result,
             const MC2String& url ) 
try {

   mc2dbg << "[AdServerTalker] Parse results. " << endl;

   // Create the source from where we read the XML.
   
   auto_ptr<XercesDOMParser> xmlParser( TalkerUtility::
                                        parseXMLSource( url, xml_result ) );
   
   if ( xmlParser.get() == NULL ) {
      mc2dbg << "[AdServerTalker] Parsing failed." << endl;
      return -1;
   }

   if ( xmlParser->getDocument() == NULL ) {
      mc2dbg << "[AdServerTalker] no document node." << endl;
      return 0;
   }

   DOMNode* rootNode = xmlParser->getDocument()->getFirstChild();

   if ( rootNode == NULL ) {
      mc2dbg << "Failed to find rootNode." << endl;
      rootNode = xmlParser->getDocument();
      if ( rootNode == NULL ) {
         mc2dbg << "Failed even more.. yiiiiha" << endl;
         return -1;
      }
   }

   mc2dbg << "Parse root exp." << endl;
   mc2dbg << "m_rootExp = " << m_rootExp.get() << endl;
   mc2dbg << "rootNode = " << rootNode << endl;
   Expression::result_type rootNodes = m_rootExp->evaluate( rootNode );
   mc2dbg << "Done parsing root exp." << endl;
   // get maxhits 
   uint32 maxHits = 0;
   {
      Expression::result_type docCountNodes = 
         m_docCountExp->evaluate( rootNode );

      if ( ! docCountNodes.empty() ) { 
         StringConvert::assign( maxHits, XMLUtility::
                                getChildTextStr( *docCountNodes[ 0 ] ) );
      } else {
         mc2dbg << warn 
                << "[AdServerTalker] could not find: " 
                << m_docCountExp->getExpression()
                << endl;
      }
   }

   if ( rootNodes.empty() ) {
      mc2dbg << "[AdServerTalker] no root nodes." << endl;
      return 0;
   }

   // clear old matches
   m_matches->reset();
   m_searchExp->evaluate( rootNodes[ 0 ] );
   m_matches->handleNewMatch(); // finalize the last match

   mc2log << "[AdServerTalker] Total number of matches:" << maxHits << endl;

   // Special case, when getting poi info.
   // We are only interested in one item
   if ( searchData.getNbrWantedHits() == 1 && 
        m_matches->getMatches().size() > 1 ) {

      // now lets erase the correct offset
      uint32 realOffset = searchData.getStartHitIdx();

      // cut out everything except the one result
      m_matches->getMatches().
         erase( m_matches->getMatches().begin() + realOffset + 1, 
                m_matches->getMatches().end() );

      m_matches->getMatches().
         erase( m_matches->getMatches().begin(), 
                m_matches->getMatches().end() - 1 );
   }
   // lets insert some fake matches
   AdServerMatch fakeMatch = {
      "Top hit advertisement",

      "Lund",
      "Barav. 1",
      "Sweden",
      "http://www.wayfinder.com/",
      "00000000",
      "please_dont_reply@localhost.localdomain",
      "",
      0, 0,
   };
   m_matches->getMatches().push_back( fakeMatch );
   fakeMatch.m_name = "Advertisement text here.";
   m_matches->getMatches().push_back( fakeMatch );
   fakeMatch.m_name = "Advertisement text here.";
   m_matches->getMatches().push_back( fakeMatch );
   fakeMatch.m_name = "Advertisement text here.";
   m_matches->getMatches().push_back( fakeMatch );

   MC2String what = TalkerUtility::getWhatField( searchData, 
                                                 TalkerUtility::FIELD_YELLOWPAGES_WITH_CATEGORY );
   MC2String where = 
      searchData.getVal( ExternalSearchDesc::city_or_area );
   
    // check for ambiguities in the city name.
   if ( m_matches->getMatches().empty() ) {
      reply.setNbrHits( searchData.getStartHitIdx(),
                        reply.getOverviewMatches().size(),
                        0 );
                        
   } else {
      ::createSearchMatches( reply.getMatchVector(), m_matches->getMatches(),
                             what, where,
                             searchData.getLang(),
                             searchData.getStartHitIdx(),
                             searchData.getInfoFilterLevel() );
      // update number of hits
      reply.setNbrHits( searchData.getStartHitIdx(),
                        reply.getMatchVector().size(),
                        max( static_cast<size_t>( maxHits ), 
                             reply.getMatchVector().size() ) );
   }

   return 1;

} catch ( const XMLException& e ) {
   mc2log << error 
          << "[AdServerTalker]: an XMLerror occured "
          << "during parsing of initialize request: "
          << e.getMessage() << " line " 
          << e.getSrcLine() << endl;
   return -1;
} catch ( const SAXParseException& e) {
   mc2log << error
          << "[AdServerTalker]: an SAXerror occured "
          << "during parsing of initialize request: "
          << e.getMessage() << ", "
          << "line " << e.getLineNumber() << ", column " 
          << e.getColumnNumber() << endl;
   return -2;
} catch ( const std::exception& e ) {
   mc2log << error 
          << "[AdServerTalker] Exception: " << e.what() << endl;
   return -3;
}
