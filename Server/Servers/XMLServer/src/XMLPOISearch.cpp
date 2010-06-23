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

#include "XMLUtility.h"
#include "XMLTool.h"
#include "XMLCommonElements.h"
#include "XMLSearchUtility.h"
#include "XMLServerErrorMsg.h"
#include "SearchResultRequest.h"
#include "XMLServerElements.h"
#include "CompactSearch.h"
#include "XMLCategoryListNode.h"
#include "STLUtility.h"
#include "SearchParserHandler.h"
#include "FetchPOIRequest.h"
#include "SearchRequest.h"

bool
XMLParserThread::xmlParsePOISearchRequest( DOMNode* cur,
                                           DOMNode* out,
                                           DOMDocument* reply,
                                           bool indent ) try {
   // DTD:
   //
   // <!ELEMENT poi_search_request (search_item_query?,
   //                               category_list,
   //                               position_item, distance) >
   //
   // <!ATTLIST poi_search_request transaction_id ID #REQUIRED
   //                              start_index %number; #REQUIRED
   //                              end_index %number; #REQUIRED
   //                              language CDATA #REQUIRED
   //                              include_top_region_id %bool; "false"
   //                              use_persistent_ids %bool; "false" >
   //
   // <!ATTLIST poi_search_reply transaction_id ID #REQUIRED >
   // <!ELEMENT poi_search_reply ( search_item_list |
   //                             ( status_code, status_message,
   //                               status_uri?, status_code_extended? ) )>

   DOMElement* root =
      XMLUtility::createStandardReply( *reply, *cur, "poi_search_reply" );

   out->appendChild( root );

   //
   // Parse XML request
   //
   using XMLTool::getAttrib;
   using XMLTool::getNodeValue;

   CompactSearch params;
   getAttrib( params.m_startIndex, "start_index", cur );
   getAttrib( params.m_endIndex, "end_index", cur );
   getAttrib( params.m_language, "language", cur );
   bool includeTopRegionIDsInReply = false;
   getAttrib( includeTopRegionIDsInReply, 
              "include_top_region_id", cur, false );
   bool usePersistentIds = false;
   getAttrib( usePersistentIds, "use_persistent_ids", cur, false );
   bool includeInfoItem = false;
   getAttrib( includeInfoItem, "include_info_item", cur, false );

   try {
      getNodeValue( params.m_what, "search_item_query", cur );
   } catch ( const XMLTool::Exception& whatEx ) { }

   DOMNode* posElement = XMLTool::findNode( cur, "position_item" );

   XMLCommonEntities::coordinateType coordType = XMLCommonEntities::MC2;
   if ( posElement != NULL &&
        posElement->getNodeType() == DOMNode::ELEMENT_NODE ) {

      MC2String errorCode, errorMsg;
      CompactSearch::Location location;
      if ( ! XMLCommonElements::
           getPositionItemData( posElement,
                                location.m_coord.lat, location.m_coord.lon,
                                location.m_angle,
                                errorCode,
                                errorMsg,
                                &coordType ) ) {

         // XXX: XMLServerErrorMsg here? So errorCode is not lost
         throw XMLTool::Exception( errorCode + ":" + errorMsg,
                                   "position_item" );
      }
      params.m_location = location;
      XMLTool::getNodeValue( params.m_distance,
                             "distance", cur, 100001 ); // 100km + 1m
   }

   // cap the radius to 100km + 1m
   params.m_distance = std::min( params.m_distance, 100001 );

   CategoryListNode::readCategoryList( cur, params.m_categoryIDs );

   // Setup default values:
   //
   // Only search in POI database
   params.m_heading = 0;

   //
   // Handle request
   //

   SearchParserHandler& handler = getSearchHandler();

   typedef STLUtility::AutoContainerMap< SearchParserHandler::SearchResults >
      SearchResults;

   POIFetch::SearchResult match;

   // If categories are empty, then use the
   // POI fetching where categories are determined by the radius.
   bool madeSearch = false;
   if ( params.m_categoryIDs.empty() ) {
      match = POIFetch::
         fetchPOIs( *this,
                    POIFetch::Request( params.m_location.m_coord,
                                       params.m_distance,
                                       params.m_startIndex,
                                       params.m_endIndex,
                                       params.m_language,
                                       params.m_what ) );
   } else {
      SearchResults results( handler.compactSearch( params ) );
      madeSearch = true;
      if ( ! results.empty() ) {
         // Take out the first header and use it as match.
         match.reset( results.begin()->second->getSearchResultRequest() );
         results.erase( 0 );
      }
   }

   // If we did not get any result, then create empty results so
   // we can reply with result=0 instead of an empty node.
   if ( match.get() == NULL ) {
      match.reset( new EmptySearchRequest() );
   }

   if ( includeInfoItem ) {
      getSearchHandler().addItemInfo( match.get(), params.m_language, 
                                      params.m_itemInfoFilter );
   }

   //
   // Create XML reply
   //

   SearchRequestParameters searchParams;
   // Include categories in the result
   searchParams.setShouldIncludeCategoriesInResult( true );
   // Add country if requested
   searchParams.setIncludeTopRegionInArea( includeTopRegionIDsInReply );
   // And language
   searchParams.setRequestedLang( params.m_language );

   // Add country regions to the matches
   if ( searchParams.shouldIncludeTopRegionInArea() ) {
      if ( !madeSearch ) {
         getSearchHandler().addCountryToResults( searchParams, match.get() );
      }
      searchParams.setRegionsInMatches( 
         searchParams.getRegionsInMatches() | SEARCH_COUNTRIES );
   }

   // Add xml nodes for each result
   XMLSearchUtility::
      appendSearchItemList( root, reply, // node and document
                            1, false, // indent Level, indent false = off
                            *match, // the request
                            "",    //current location
                            params.m_startIndex,
                            params.m_endIndex,
                            false, // latlon for search hits
                            true, // position search items
                            coordType, // coordinateType systems
                            searchParams, // Info about what to add
                            NULL, // Let the function create the list
                            this, // thread
                            MAX_UINT32, // heading id, is unknown here
                            usePersistentIds,/*If not to use normal itemID*/
                            includeInfoItem ); 

   if ( indent ) {
      XMLUtility::indentPiece( *root, 1 );
   }

   return true;

} catch ( const XMLServerErrorMsg& error ) {
   XMLServerUtility::
      appendStatusNodes( out->getLastChild(), reply, 1, false,
                         error.getCode().c_str(), error.getMsg().c_str(),
                         NULL, //extendedCode
                         error.getURI().c_str() );
   return true;

} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "[POISearchRequest]  " << e.what() << endl;
   XMLServerUtility::
      appendStatusNodes( out->getLastChild(), reply, 1, false,
                         "-1", e.what() );
   return true;
}

