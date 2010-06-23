/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLSearchHandler.h"

#include "NodeAssigner.h"
#include "XMLTool.h"
#include "Auth.h"
#include "IPnPort.h"
#include "XMLTreeFormatter.h"
#include "URLFetcher.h"
#include "URL.h"

#include <sstream>

namespace XMLTool {
template <>
void addAttrib<LangTypes::language_t>( DOMElement* node, const char* name,
                                       const LangTypes::language_t& lang ) {
   addAttrib<MC2String>( node, name, 
                         LangTypes::getLanguageAsString( lang ) );
}

}

namespace {
struct XMLSearchMatch: public SearchViewer::SearchMatch {
   XMLSearchMatch():
      m_node( NULL )
   { }
   const DOMNode* m_node;
};

DOMDocument* 
makeDocument( const char* documentType = "isab-mc2",
              const char* charset = "iso-8859-1",
              const char* standAlone = "",
              const char* xmlVersion = "1.0" ) {
   DOMImplementation* impl = 
      DOMImplementationRegistry::
      getDOMImplementation( X( "LS" ) ); // Load Save

   DOMDocument* reply = impl->
      createDocument( NULL,      // root element namespace URI.
                      X( documentType ),  // root element name
                      NULL ); // document type object (DTD).

   // Create the DOCTYPE
   reply->insertBefore( reply->createDocumentType( X( documentType ) ),
                        reply->getDocumentElement() );

   return reply;
}

}

namespace SearchViewer {

class XMLSearchHandler::SearchMatchHandler:public MatchHandler< XMLSearchMatch > {
public:
   void operator()( const DOMNode* n ) {
      MatchHandler< ::XMLSearchMatch >::operator()( n );
      // set match node
      getCurrMatch().m_node = n;
   }
};

class XMLSearchHandler::HeadingMatchHandler: public MatchHandler< ItemMatch > {
public:
};
class XMLSearchHandler::DescMatchHandler: public MatchHandler< DescMatch > {
public:
};

XMLSearchHandler::XMLSearchHandler():
   SearchHandler( "XML" ),
   m_matches( new XMLSearchHandler::SearchMatchHandler() ),
   m_headingMatches( new XMLSearchHandler::HeadingMatchHandler() ),
   m_descMatches( new XMLSearchHandler::DescMatchHandler() ) {

   using namespace XMLTool::XPath;

   // setup search match expressions

   {
      SearchMatch& currMatch =  m_matches->getCurrMatch();
      MultiExpression::NodeDescription desc[] = {
         { "search_hit_list*", m_matches.get() },
         { "search_hit_list/@starting_index",
           makeAssigner( currMatch.m_startIndex ) },
         { "search_hit_list/@ending_index",
           makeAssigner( currMatch.m_endIndex ) },
         { "search_hit_list/@total_numberitems",
           makeAssigner( currMatch.m_totalNbrItems ) },
         { "search_hit_list/@numberitems",
           makeAssigner( currMatch.m_nbrItems ) },
         { "search_hit_list/@heading",
           makeAssigner( currMatch.m_heading ) }
      };
      const uint32 descSize = sizeof ( desc ) / sizeof ( desc[0] );
      m_searchExp.reset( new 
                         MultiExpression( MultiExpression::
                                          Description( desc,
                                                       desc + descSize ) ) );
   }

   {
      ItemMatch& currMatch =  m_headingMatches->getCurrMatch();
      MultiExpression::NodeDescription desc[] = {
         { "search_item*", m_headingMatches.get() },
         { "search_item/@search_item_type", makeAssigner( currMatch.m_type ) },
         { "search_item/@image", makeAssigner( currMatch.m_image ) },
         { "search_item/name", makeAssigner( currMatch.m_name ) },
         { "search_item/itemid", makeAssigner( currMatch.m_itemID ) },
         { "search_item/location_name", 
           makeAssigner( currMatch.m_locationName ) },
         { "search_item/lat", makeAssigner( currMatch.m_lat ) },
         { "search_item/lon", makeAssigner( currMatch.m_lon ) },
      };
      const uint32 descSize = sizeof ( desc ) / sizeof ( desc[0] );
      m_hitExp.reset( new 
                      MultiExpression( MultiExpression::
                                       Description( desc,
                                                    desc + descSize ) ) );
   } 
   {
      DescMatch& currMatch = m_descMatches->getCurrMatch();
      MultiExpression::NodeDescription desc[] = {
         { "search_hit_type*", m_descMatches.get() },
         { "search_hit_type/@heading", makeAssigner( currMatch.m_headingNum)},
         { "search_hit_type/image_name", makeAssigner( currMatch.m_image ) },
         { "search_hit_type/name", makeAssigner( currMatch.m_name ) },
      };
      const uint32 descSize = sizeof ( desc ) / sizeof ( desc[0] );
      m_descExp.reset( new 
                       MultiExpression( MultiExpression::
                                        Description( desc,
                                                     desc + descSize ) ) );
   }

}

DOMElement* createRequest( DOMNode* doc, 
                           const MC2String& reqName,
                           const MC2String& transactionID ) {
   DOMElement* req = XMLTool::addNode( doc, reqName.c_str() );
   XMLTool::addAttrib( req, "transaction_id", transactionID );
   return req;
}

/**
 * Creates a search_desc_request.
 */
void createSearchDescRequest( DOMNode* doc, const MC2String& crc,
                              LangTypes::language_t lang ) {
   
   DOMElement* reqNode = createRequest( doc, "search_desc_request", "id1" );
   XMLTool::addAttrib( reqNode, "language", lang );
   XMLTool::addAttrib( reqNode, "crc", crc );
}

/**
 * Creates a compact_search_request from compact search parameters.
 */
void createCompactSearchRequest( DOMNode* doc,
                                 const CompactSearch& params ) {
   using namespace XMLTool;
   DOMElement* reqNode = createRequest( doc, "compact_search_request", "id2" );
   addAttrib( reqNode, "start_index", params.m_startIndex );
   addAttrib( reqNode, "end_index", params.m_endIndex );
   addAttrib( reqNode, "max_hits", params.m_maxHits );
   addAttrib( reqNode, "language", params.m_language );
   addAttrib( reqNode, "heading", params.m_heading );
   addAttrib( reqNode, "round", params.m_round );
   
   addNode( reqNode, "search_item_query", params.m_what );
   addNode( reqNode, "category_name", params.m_categoryName );

   if ( ! params.m_areaID.isValid() &&
        ! params.m_location.isValid() ) {
      addNode( reqNode, "search_area_query", params.m_where );
      addNode( reqNode, "top_region_id", params.m_topRegionID );
   } 
   // !! TODO: else check area ID before location
   // ...
   else if ( params.m_location.isValid() ) {
      DOMElement* posItem = addNode( reqNode, "position_item" );
      addAttrib( posItem, "position_system", MC2String( "MC2" ) );
      addNode( posItem, "lat", params.m_location.m_coord.lat );
      addNode( posItem, "lon", params.m_location.m_coord.lon );
      addNode( posItem, "angle", MC2String( "0" ) );
      addNode( reqNode, "distance", params.m_distance );
   }
}

void createAuth( DOMNode* doc, const Auth& auth ) {
   using namespace XMLTool;
   DOMElement* authNode = addNode( doc, "auth" );
   addAttrib( authNode, "indentingandlinebreaks", MC2String( "true" ) );
   addNode( authNode, "auth_user", auth.getUsername() );
   addNode( authNode, "auth_passwd", auth.getPassword() );
}

void XMLSearchHandler::getReplyDebugData( MC2String& data, 
                                          uint32 maxColumns ) {
   data = m_replyData;
}

void XMLSearchHandler::search( const IPnPort& address,
                               const Auth& auth,
                               const MC2String& crc,
                               const CompactSearch& params ) {
   // reset answer
   m_headings.m_resultHeadings.clear();

   mc2dbg << "Searching " << endl
          << "Address: " << address << endl
          << "Params: " << endl;
   stringstream paramStr;
   paramStr << params << endl;
   cout << paramStr.str() << endl;


   // create compact search request 
   DOMDocument* doc = makeDocument();
   mc2dbg << "Initial doc: " << XMLTreeFormatter::makeStringOfTree( doc )
          << endl;

   DOMNode* firstTag = doc->getLastChild();
   createAuth( firstTag, auth );
   createSearchDescRequest( firstTag, crc, params.m_language );
   createCompactSearchRequest( firstTag, params );

   MC2String requestStr = XMLTreeFormatter::makeStringOfTree( doc );


   paramStr << "============ Sending ==========" << endl; 
   XMLTreeFormatter::printIndentedTree( doc, paramStr );
   paramStr << "===============================" << endl;
   m_replyData = "Searching using:\n" + paramStr.str();


   doc->release();

   // send compact search request   
   MC2String answer;
   MC2String url = "http://";
   url += address.toString() + "/";
   mc2dbg << "URL: " << url << endl;
   int ret = URLFetcherNoSSL().post( answer, url, requestStr );
   if ( ret < 0 ) {
      mc2dbg << warn << "Failed connect to: " << url << endl;
      m_replyData += "Failed connect to: " + url + "\n";
      return;
   }

   m_replyData += answer;
   parseResultFromData( answer );
}

const Headings& XMLSearchHandler::getMatches() const {
   return m_headings;
}

void XMLSearchHandler::parseResultFromData( const MC2String& xmlSource ) {
   mc2dbg << "Parse result from data." << endl;
   XercesDOMParser parser;
   parser.setValidationScheme( XercesDOMParser::Val_Auto );
   parser.setIncludeIgnorableWhitespace( false );
   // remove some stuff so we can parse the document correctly
   MC2String data = xmlSource;
   size_t pos = data.find("<isab-mc2>");
   if ( pos != MC2String::npos ) {
      while ( data[ pos - 1 ] != '>' ) {
         mc2dbg << "erasing: " << (int)data[ pos - 1 ] << endl;
         data.erase( --pos, 1 );
      }
      pos = data.find( " SYSTEM \"isab-mc2.dtd\"" );
      if ( pos != MC2String::npos ) {
         data.erase( pos, strlen( " SYSTEM \"isab-mc2.dtd\"" ) );
      }
   }

   char* subStr = StringUtility::strstr( data.c_str(), "<?xml" );
   if ( subStr == NULL ) {
      mc2dbg << "[SearchViewer] no <?xml tag found. " << endl;
      return;
   }

   const MC2String tmp_xml_string = subStr;
   mc2dbg << "Tmp xml string: " << tmp_xml_string << endl;
   XStr str( "" );
   MemBufInputSource 
      source( reinterpret_cast<const XMLByte*>( tmp_xml_string.data() ), 
              tmp_xml_string.length(),
              str.XMLStr() );
   parser.parse( source );
   DOMDocument* doc = parser.getDocument();

   parseResult( doc );
}

void XMLSearchHandler::parseResult( const MC2String& filename ) {
   mc2dbg << "Parse result from filename: " << filename << endl;
   XercesDOMParser parser;
   parser.setValidationScheme( XercesDOMParser::Val_Auto );
   parser.setIncludeIgnorableWhitespace( false );

   parser.parse( filename.c_str() );

   if ( parser.getDocument() == NULL ) {
      mc2log << warn << " no document! " << endl;
      return;
   }

   DOMDocument *doc = parser.getDocument();
   parseResult( doc );
}

void XMLSearchHandler::parseResult( const DOMDocument* doc ) {
      

   if ( doc->getFirstChild() == NULL ) {
      mc2dbg << warn << "Could not find first child." << endl;
      return;
   }

   // see if we got any search hit desc, and update our description
   using namespace XMLTool::XPath;
   vector<const DOMNode*> descNodes( Expression
                                     ( "/isab-mc2/search_desc_reply" ).
                                     evaluate( doc->getFirstChild() ) );
   if ( ! descNodes.empty() ) {

      XMLTool::getAttrib( m_headings.m_crc, "crc", descNodes.back(), MC2String() );

      mc2dbg << "[XMLSearchHandler] Found search_desc_reply." << endl;
      // see if we got a search_desc_reply
      m_descMatches->reset();
      m_descExp->evaluate( descNodes.back() );
      m_descMatches->handleNewMatch(); // finalize matches

      if ( ! m_descMatches->getMatches().empty() ) {
         // ok we got a new search_hit_desc_reply 
         // lets update the description
         m_headings.m_headings.clear();
         const vector< DescMatch >& headingMatches = 
            m_descMatches->getMatches();
         
         for ( uint32 i = 0; i < headingMatches.size(); ++i ) {
            m_headings.m_headings[ headingMatches[ i ].m_headingNum ] = 
               headingMatches[ i ];
         }
      } else {
         mc2dbg << "No search_desc_reply. could be crc match." << endl;
      }
   }


   XMLTool::XPath::Expression 
      searchHitListExp( "/isab-mc2/compact_search_reply" );
   vector<const DOMNode*> replyNodes( searchHitListExp.
                                      evaluate( doc->getFirstChild() ) );
   if ( replyNodes.empty() ) {
      return;
   }

   // evaluate result
   m_matches->reset();
   m_searchExp->evaluate( replyNodes.front() );
   // finalize matches
   m_matches->handleNewMatch();

   const vector< XMLSearchMatch >& headings( m_matches->getMatches() );

   // add headings as root nodes and add sub nodes for result

   for ( uint32 headingIdx = 0; headingIdx < headings.size(); ++headingIdx ) {
      const XMLSearchMatch& match = headings[ headingIdx ];
      Headings::HeadingMap::iterator headingIt = 
         m_headings.m_headings.find( match.m_heading );
      if ( headingIt == m_headings.m_headings.end() ) {
         mc2dbg << warn 
                << "Could not find heading id: " << match.m_heading << endl;
         continue;
      } 

      (*headingIt).second.m_matches = match;
      addResult( (*headingIt).second.m_matches, match.m_node );
      m_headings.m_resultHeadings.push_back( (*headingIt).second ); 
                                        
   }

}

void XMLSearchHandler::addResult( SearchMatch& match, const DOMNode* resultNode ) {
   if ( resultNode->getFirstChild() == NULL ) {
      mc2dbg << warn 
             << "[XMLSearchHandler] Could not find search_item child." << endl;
      return;
   }

   m_headingMatches->reset();
   m_hitExp->evaluate( resultNode );
   // finalize matches
   m_headingMatches->handleNewMatch();

   const vector< ItemMatch > items( m_headingMatches->getMatches() );
   match.m_items = items;

}


}
