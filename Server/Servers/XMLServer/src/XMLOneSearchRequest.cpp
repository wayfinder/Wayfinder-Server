/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLParserThread.h"
#include "DeleteHelpers.h"
#include "StringConvert.h"
#include "IDPairVector.h"
#include "SearchMatch.h"
#include "CompactSearch.h"
#include "SearchParserHandler.h"
#include "SearchHeadingDesc.h"
#include "StringConvert.h"
#include "SearchResultRequest.h"
#include "UserSwitch.h"
#include "SearchTypes.h"
#include "STLStringUtility.h"

#include "ExternalSearchRequest.h"

#include "XMLSearchUtility.h"
#include "XMLTool.h"
#include "XMLUtility.h"
#include "XMLCommonElements.h"
#include "XMLServerErrorMsg.h"
#include "XMLAuthData.h"
#include "XMLServerElements.h"
#include "XMLSearchUtility.h"
#include "XMLCategoryListNode.h"

using XMLServerUtility::appendStatusNodes;

namespace {
SearchTypes::SearchSorting getSorting( MC2String& strSorting ) {
   uint32 sort = STLStringUtility::strtoul( strSorting );
   if( sort == 1 ) {
      return SearchTypes::AlphaSort;
   } else {
      // Default
      return SearchTypes::DistanceSort;
   }
}
}


bool
XMLParserThread::xmlParseOneSearchRequest( DOMNode* cur,
                                           DOMNode* out,
                                           DOMDocument* reply,
                                           bool indent )
try {
   //
   // One search request DTD:
   //
   // <!ENTITY % sorting_t "(alfa_sort|distance_sort)">
   // <!ENTITY % search_for_type_t "(address|all)">
   //
   // <!ELEMENT one_search_request ( search_match_query?, 
   //                            category_list?,
   //                            ( ( position_item, distance? ) |
   //                            ( query_location, top_region_id ) ) ) >
   // <!ATTLIST one_search_request transaction_id    ID            #REQUIRED
   //                          max_number_matches %number;     #REQUIRED
   //                          language          %language_t;  #REQUIRED
   //                          round             %number;      #REQUIRED
   //                          version           %number;      #REQUIRED
   //                          include_info_fields %bool;      #IMPLIED
   //                          position_system %position_system_t; "MC2"
   //                          sorting           %sorting_t;   #REQUIRED 
   //                          search_type %search_for_type_t; "all" >
   // <!ELEMENT search_match_query ( #PCDATA )>
   // <!ELEMENT query_location ( #PCDATA )
   //
   DOMElement* root =
      XMLUtility::createStandardReply( *reply, *cur, "one_search_reply" );

   out->appendChild( root );

   // setup default param
   CompactSearch params;

   // This is one_search_request...
   params.m_oneResultList = true;

   using namespace XMLTool;

   // get attrib values
   // must have these
   getAttrib( params.m_maxHits, "max_number_matches", cur );
   getAttrib( params.m_language, "language", cur );
   getAttrib( params.m_round, "round", cur );

   MC2String strSorting;
   getAttrib( strSorting, "sorting", cur );
   params.m_sorting = getSorting( strSorting );

   uint32 searchVersion = 0;
   getAttribValue( searchVersion, "version", cur );

   // Set start and end index from max hit
   params.m_startIndex = 0;
   params.m_endIndex = params.m_maxHits - 1;

    // these attributes are optional
   getAttrib( params.m_includeInfoItem, "include_detail_fields", cur, true );
   if ( params.m_includeInfoItem ) {
      params.m_itemInfoFilter = ItemInfoEnums::OneSearch_All;
   } else {
      params.m_itemInfoFilter = ItemInfoEnums::None;
   }
   
   MC2String positionSystemString( "MC2" );
   getAttribValue( positionSystemString, "position_system", cur );

   XMLCommonEntities::coordinateType
      replyPositionSystem = XMLCommonEntities::
      coordinateFormatFromString( positionSystemString.c_str(),
                                  XMLCommonEntities::MC2 );

   MC2String searchType( "all" );
   getAttrib( searchType, "search_type", cur, searchType );
   if ( searchType == "address" &&  params.m_round == 0 ) {
      // We only want to search for address      
      params.m_heading = 1; // Addresses heading
   }

    // all required main attributes are set.
   // now lets continue with elements shall we
   bool searchQueryProvided = false;
   try { 
      getNodeValue( params.m_what, "search_match_query", cur ); 
      searchQueryProvided = true;
   } catch ( const XMLTool::Exception& e ) { }

   // Get category ids if any
   CategoryListNode::readCategoryList( cur, params.m_categoryIDs );

   if( !searchQueryProvided && params.m_categoryIDs.empty() ) {
      // No search data provided.
      throw MC2Exception( 
         "Nothing to search for. Please provide a search_match_query or a category_list." );
   }
 
   // Get the location
   DOMNode* posElement = findNode( cur, "position_item" );

   if ( posElement != NULL &&
        posElement->getNodeType() == DOMNode::ELEMENT_NODE ) {

      MC2String errorCode, errorMsg;
      CompactSearch::Location location;

      // Read coordinates from position_item, including coordinate type
      if ( ! XMLCommonElements::
           getPositionItemData( posElement,
                                location.m_coord.lat, location.m_coord.lon,
                                location.m_angle, 
                                errorCode, 
                                errorMsg ) ) {

         throw XMLTool::Exception( errorCode + ":" + errorMsg, 
                                   "position_item" );
      }
      params.m_location = location;

      getNodeValue( params.m_distance, "distance", cur, 100000 ); // 100km
   }


   if ( posElement == NULL ) {
      // Position not set. Get query_location and top_region_id instead
      getNodeValue( params.m_where, "query_location", cur );
      getNodeValue( params.m_topRegionID, "top_region_id", cur );      
   }

   // Check with external access if user is allowed to search
   if( ! checkAllowedToSearch( root, reply, params, indent) ) {
      // Not allowed! 
      // Error messages already added to XML reply, just return.
      return true;
   }

   // Get the results
   SearchParserHandler& handler = getSearchHandler();

   typedef STLUtility::AutoContainerMap< SearchParserHandler::SearchResults >
      SearchResults;

   SearchResults results( handler.compactSearch( params ) );

   if ( results.empty() ) { 
      mc2log << warn << "[OneSearchRequest]: no results!" << endl;
   }

   // Add xml nodes for each result
   SearchResults::const_iterator resultIt = results.begin();
   if ( resultIt != results.end() ) {
      XMLSearchUtility::
         appendSearchListElement( root, reply, 
                                  resultIt->second->getSearchResultRequest(),
                                  params.m_maxHits, replyPositionSystem, 
                                  params.m_language, this);
   }

   if ( indent ) {
      XMLUtility::indentPiece( *root, 1 );
   }

   return true;

} catch ( const XMLServerErrorMsg& error ) {
   appendStatusNodes( out->getLastChild(), reply, 1, false,
                      error.getCode().c_str(), error.getMsg().c_str(), 
                      NULL/*extendedCode*/, error.getURI().c_str() );
   return true;
} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "[OneSearchRequest]  " << e.what() << endl;
   appendStatusNodes( out->getLastChild(), reply, 1, false,
                      "-1", e.what() );
   return true;
} catch ( const MC2Exception& e ) {
   mc2log << info << "[OneSearchRequest]  " << e.what() << endl;
   appendStatusNodes( out->getLastChild(), reply, 1, false,
                      "-1", e.what() );
   return true;
}
