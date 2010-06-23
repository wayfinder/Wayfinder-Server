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

#include "ServerRegionIDs.h"
#include "TopRegionRequest.h"

using XMLServerUtility::appendStatusNodes;

namespace StringConvert {

template <>
IDPair_t convert<IDPair_t>( const MC2String& str ) 
   throw (StringConvert::ConvertException) {
   auto_ptr<SearchMatch> match( SearchMatch::createMatch(str) );
   if ( match.get() == NULL ) {
      throw 
         StringConvert::
         ConvertException(MC2String("Could not assign IDPair_t from string:")+
                          str);
                             
   }
   return match->getID();
}

}
namespace {

void addSubHeadings( DOMNode* node, uint32 version, 
                     const LangType& lang ) {
   if ( version > 0 ) {
      // only for those clients that read them.
      MC2String str = StringTable::getString( StringTable::SPONSORED_RESULTS,
                                              lang );
      XMLTool::addNode( node, "ad_results_text", str );
      str = StringTable::getString( StringTable::ALL_RESULTS, lang );
      XMLTool::addNode( node, "all_results_text", str );
   }
}

}


bool
XMLParserThread::xmlParseCompactSearchRequest( DOMNode* cur,
                                               DOMNode* out,
                                               DOMDocument* reply,
                                               bool indent )
try {
   //
   // Compact search request DTD:
   //
   // <!ELEMENT category_query ( #PCDATA ) >
   // <!ELEMENT category_id ( #PCDATA ) >
   // <!ELEMENT category_list (category_id+) >
   //
   // <!ELEMENT compact_search_request ( search_item_query,
   //                                    ( category_query | category_name  |
   //                                      category_id | category_list )?,
   //                                    ( (search_area_query, top_region_id)|
   //                                      search_area|
   //                                     (position_item, distance?) ) ) >
   // <!ATTLIST compact_search_request transaction_id ID #REQUIRED
   //                                  start_index %number; #REQUIRED
   //                                  end_index %number; #REQUIRED
   //                                  max_hits %number; #REQUIRED
   //                                  language CDATA #REQUIRED
   //                                  round %number; #IMPLIED
   //                                  heading %number; #IMPLIED
   //                                  uin %number; #IMPLIED
   //                                  version %number; "0"
   //                                  category_search %bool; "false"
   //                                  include_category_id %bool; "false"
   //                                  include_top_region_id %bool; "false"
   //                                  use_persistent_ids %bool; "false" >
   //
   DOMElement* root =
      XMLUtility::createStandardReply( *reply, *cur, "compact_search_reply" );

   out->appendChild( root );

   // setup default param
   CompactSearch params;

   using namespace XMLTool;

   // get attrib values
   // must have these
   getAttrib( params.m_startIndex, "start_index", cur );
   getAttrib( params.m_endIndex, "end_index", cur );
   getAttrib( params.m_maxHits, "max_hits", cur );
   getAttrib( params.m_language, "language", cur );

   // these attributes are optional
   getAttrib( params.m_round, "round", cur, MAX_UINT32 );
   getAttrib( params.m_heading, "heading", cur, -1 );
      
   bool categorySearch = false;
   getAttrib( categorySearch, "category_search", cur, false );
   bool includeCategoriesInReply = false;
   getAttrib( includeCategoriesInReply, "include_category_id", cur, false );
   bool includeTopRegionIDsInReply = false;
   getAttrib( includeTopRegionIDsInReply, 
              "include_top_region_id", cur, false );
   bool usePersistentIds = false;
   getAttrib( usePersistentIds, "use_persistent_ids", cur, false );
   getAttrib( params.m_includeInfoItem, "include_info_item", cur, false );

   // determine if we should use another user 
   uint32 uin = 0;
   getAttrib( uin, "uin", cur, (uint32)0 );
   uint32 searchVersion = 0;
   getAttribValue( searchVersion, "version", cur );

   // get category type
   getNodeValue( params.m_categoryName, "category_name", cur, MC2String() );

   MC2String positionSystemString( "MC2" );
   getAttribValue( positionSystemString, "position_system", cur );

   XMLCommonEntities::coordinateType
      replyPositionSystem = XMLCommonEntities::
      coordinateFormatFromString( positionSystemString.c_str(),
                                  XMLCommonEntities::MC2 );

   if ( params.m_categoryName.empty() ) {
      // try category_query instead
      getNodeValue( params.m_categoryName, "category_query", cur, MC2String());
      // if not empty, then we are suppose to search for a category name too
      if ( ! params.m_categoryName.empty() ) {
         params.m_findCategories = true;
      } else {
         uint32 catid = CategoryTreeUtils::INVALID_CATEGORY;
         getNodeValue( catid, "category_id", cur,
                       catid );
         if ( catid != CategoryTreeUtils::INVALID_CATEGORY ) {
            params.m_categoryIDs.push_back( catid );
         }
      }
   }

   CategoryListNode::readCategoryList( cur, params.m_categoryIDs );

   // all required main attributes are set.
   // now lets continue with elements shall we
   uint32 nbrFields = 0;
   try { 
      getNodeValue( params.m_what, "search_item_query", cur ); 
      nbrFields++;
   } catch ( const XMLTool::Exception& whatEx ) { }
   
   try {
      getNodeValue( params.m_where, "search_area_query", cur );
      nbrFields++;
   } catch ( const XMLTool::Exception& whereEx ) { }


   if ( categorySearch &&
        params.m_categoryName.empty() ) {
      params.m_categoryName = params.m_what;
      // must clear the "what" value, so it searches ALL
      // the items with category search.
      // This needs to be done for the old clients (wf7) that sends
      // "category_search" attribute.
      params.m_what = "";
   }

   bool useTopRegion = true; // if the current search uses top region
   
   try {
      DOMNode* search_area = findNode( cur, "search_area" );
      if ( search_area == NULL ) {
         throw XMLTool::Exception( "No such node", "search_area" );
      }
      IDPair_t areaID;
      char* locationName = NULL;
      MC2String errorCode, errorMsg;
      uint32 areaMask_notused, areaType_notused;
      bool ret = XMLSearchUtility::
         getSearchAreaData( search_area,
                            areaID.first, areaID.second,
                            areaMask_notused, locationName, areaType_notused,
                            errorCode, errorMsg );
      // dont think we need locationName 
      params.m_where = locationName ? locationName : "";
      delete [] locationName;
      
      if ( ! ret ) {
         throw XMLTool::Exception( errorCode + ":" + errorMsg, "search_area" );
      }
      
      // finaly set area id param
      params.m_areaID = areaID;
      useTopRegion = false;

   } catch ( const XMLTool::Exception& e ) { 
      if ( nbrFields == 0 ) {
         throw e;
      }
   }


   // optional value
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

         // TODO: XMLServerErrorMsg here? So errorCode is not lost
         throw XMLTool::Exception( errorCode + ":" + errorMsg, 
                                   "position_item" );
      }
      params.m_location = location;
      useTopRegion = false;

      getNodeValue( params.m_distance, "distance", cur, 100000 ); // 100km

   }


   // if location or area id is specified, we should not use top region
   if ( useTopRegion ) {
      // must have top_region_id element
      getNodeValue( params.m_topRegionID, "top_region_id", cur );

      if ( ! checkUserRegionAccess( params.m_topRegionID,
                                    m_user->getUser(), 
                                    m_authData->urmask ) ) {
         throw XMLServerErrorMsg( "-5", "Outside allowed area.",
                                  "" );
      }
   }

   std::auto_ptr< UserSwitch > userSwitch;
   if ( uin != 0 ) {
      UserItem* newUser = NULL;
      if ( getUser( uin, newUser, true ) && newUser != NULL ) {
         mc2dbg << "[CompactSearchRequest] switching to user uin = " 
                << uin << endl;
         const ClientSetting* settings = NULL;
         UserElement* el = 
            newUser->getUser()->
            getElementOfType( 0, UserConstants::TYPE_LAST_CLIENT );
         if ( el ) {
            UserLastClient* last = static_cast< UserLastClient* >( el );
            settings = getGroup()->getSetting( last->getClientType().c_str(),
                                               "");
         }
         // the user switch will release the new user item
         userSwitch.reset( new UserSwitch( *this, newUser, settings ) );
      } else {
         mc2dbg << "[CompactSearchRequest] failed to get user for uin = " 
                << uin << endl;
      }
   }

   // Check with external access if user is allowed to search
   if( ! checkAllowedToSearch( root, reply, params, indent) ) {
      // Not allowed! 
      // Error messages already added to XML reply, just return.
      return true;
   }


   // ok ol' chap, now all params should be in order. 
   // lets call the handler... to... handle... the params 

   SearchParserHandler& handler = getSearchHandler();

   typedef STLUtility::AutoContainerMap< SearchParserHandler::SearchResults >
      SearchResults;

   SearchResults results( handler.compactSearch( params ) );

   if ( results.empty() ) { 
      mc2log << warn << "[CompactSearchRequest]: no results!" << endl;
   }

   SearchRequestParameters searchParams;   
   searchParams.setShouldIncludeCategoriesInResult( includeCategoriesInReply );
   searchParams.setIncludeTopRegionInArea( includeTopRegionIDsInReply );
   // And language
   searchParams.setRequestedLang( params.m_language );

   if ( searchParams.shouldIncludeTopRegionInArea() ) {
      searchParams.setRegionsInMatches( 
         searchParams.getRegionsInMatches() | SEARCH_COUNTRIES );
      getSearchHandler().addCountryToResults( searchParams, results );
   }

   // add additional heading text for advertisement and all results
   if ( ! results.empty() ) {
      addSubHeadings( root, searchVersion, params.m_language );
   }

   // Add xml nodes for each result
   SearchResults::const_iterator resultIt = results.begin();
   SearchResults::const_iterator resultItEnd = results.end();
   for ( ; resultIt != resultItEnd; ++resultIt ) {
      SearchResults::key_type heading = (*resultIt).first;
      const SearchResultRequest* match = 
         (*resultIt).second->getSearchResultRequest();
      DOMElement* listElement = NULL;

      if ( match->getMatches().empty() && 
           ( match->getNbrOverviewMatches() > 1 ||
             // the external search might have number overview matches = 1
             dynamic_cast<const ExternalSearchRequest*>( match ) != 0 ) ) {
         listElement = addNode( root, "search_hit_list" );
         if ( match->getTotalNbrMatches() > 0 ) {
            // add additional heading text for advertisement and all results,
            // for each heading. Only for those clients that read them.
            addSubHeadings( listElement, searchVersion, params.m_language );
         }
         XMLSearchUtility::
            appendSearchAreaList( root, reply,  // node and document
                                  1, false, // indent Level, indent false = off
                                  *match, // the request
                                  "",    //current location
                                  params.m_startIndex, params.m_endIndex, // start, end index
                                  false, // latlon for search hits
                                  true, // position search items
                                  replyPositionSystem,
                                  searchParams,
                                  listElement );
      } else { // normal heading 
         // The first element creates the list
         listElement = addNode( root, "search_hit_list" );
         if ( match->getTotalNbrMatches() > 0 ) {
            // add additional heading text for advertisement and all results,
            // for each heading. Only for those clients that read them.
            addSubHeadings( listElement, searchVersion, params.m_language );
         }

         XMLSearchUtility::
            appendSearchItemList( root, reply, // node and document
                                  1, false, // indent Level, indent false = off
                                  *match, // the request
                                  "",    //current location
                                  params.m_startIndex, params.m_endIndex, // start, end index
                                  false, // latlon for search hits
                                  true, // position search items
                                  replyPositionSystem,
                                  searchParams,
                                  listElement,  // the list element
                                  this, // thread
                                  heading, // heading id of the list
                                  usePersistentIds,/*If not use normal itemID*/
                                  params.m_includeInfoItem/*ItemInfo*/ );
      }

      // add additional attribs to list
      addAttrib( listElement, "heading", heading );
      if ( heading >= SearchHeadingDesc::ADVERTISEMENT_HEADING ) {
         const ExternalSearchRequest* req = 
            dynamic_cast<const ExternalSearchRequest*>( match );
         if ( req ) {
            addAttrib( listElement, "top_hits", req->getNbrTopHits() );
         }
      }

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
   mc2log << error << "[CompactSearchRequest]  " << e.what() << endl;
   appendStatusNodes( out->getLastChild(), reply, 1, false,
                      "-1", e.what() );
   return true;
}
