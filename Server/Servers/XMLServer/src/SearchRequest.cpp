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

#ifdef USE_XML
#include "CoordinateOnItemRequest.h"
#include "SearchRequest.h"
#include "ClosestSearchRequest.h"
#include "SearchPacket.h"
#include "UserData.h"
#include "ProximityRequest.h"
#include "SearchRequestParameters.h"
#include "HttpInterfaceRequest.h"
#include "UTF8Util.h"
#include <wchar.h> 
#include "GfxConstants.h"
#include "TopRegionRequest.h"
#include "StringConversion.h"
#include "ExternalSearchRequest.h"
#include "ExternalSearchRequestData.h"
#include "XMLAuthData.h"
#include "SearchParserHandler.h"
#include "XMLTool.h"
#include "ParserDebitHandler.h"
#include "XMLSearchUtility.h"
#include "XMLServerElements.h"
#include "LangUtility.h"

#include <memory>

// Return a number between 0-size-1. If index below 0 then 0. If
// index above size-1 then size-1.
#define indexInArray( index, size ) ((index) >= 0 ? \
   MIN( index, MAX( 0, size-1) ) :0)

// Return a number between 0-size. If index below 0 then 0. If
// index above size then size.
#define indexForArray( index, size ) ((index) >= 0 ? MIN( index, size ) :0)

// Returns the size of an range(start,end) in array with length size
#define rangeArraySize( start, end, size ) \
((end-start)>=0 ? ((start<size)?\
(MAX(0,(MIN(end,MAX(0,size-1))-MIN(start,MAX(0,size-1))+1))) :0) :0)


namespace {
bool 
readSearchSettings( DOMNode* settings,
                    SearchRequestParameters& params,
                    MC2String& errorCode, MC2String& errorMessage );

bool checkAndSetCategorySearch( const SearchParserHandler& shandler,
                                const MC2String& searchItem,
                                SearchRequestParameters& params ) {
   // at this point we need to do conversion between old
   // style category search with strings and the new one with
   // ids.
   //

   // lets see if we can find the actual 
   // category of this string. Assuming it is a category
   // search at first.
   SearchParserHandler::Category cat = 
      shandler.findCategoryFromListLowerCase( searchItem );
   if ( cat != SearchParserHandler::INVALID_CATEGORY ) {
      // add category found and setup params
      SearchRequestParameters::Categories ids;
      ids.insert( cat.getCategoryID() );
      params.setMatchType( SearchTypes::CloseMatch );
      params.setCategories( ids ); 
      return true;
   }// else no category found, lets leave it alone shall we.

   return false;
}

}

bool 
XMLParserThread::xmlParseSearchRequest( DOMNode* cur, 
                                        DOMNode* out,
                                        DOMDocument* reply,
                                        bool indent )
{
   bool ok = true;
   int indentLevel = 1;
   
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );
   
   bool latlonForSearchHits = false;
   XMLCommonEntities::coordinateType positionSystem =
      XMLCommonEntities::MC2;
   bool positionSearchItems = false;
   bool positionSearchAreas = false;
   int32 searchAreaStartingIndex = 0;
   int32 searchAreaEndingIndex = 49;
   int32 searchItemStartingIndex = 0;
   int32 searchItemEndingIndex = 99;
   SearchRequestParameters params;
   MC2String errorCode;
   MC2String errorMessage;
   
   //   params.setEndHitIndex( searchItemEndingIndex );
//   params.setNbrSortedHits( searchItemEndingIndex );
   params.setMatchType( SearchTypes::CloseMatch );
   params.setStringPart( SearchTypes::Beginning );
   params.setSortingType( SearchTypes::ConfidenceSort );
//   params.m_bboxRequested = false;
   params.setLookupCoordinates( false );
   params.setUniqueOrFull( false );
//   params.m_searchOnlyIfUniqueOrFull = true;
   params.setRegionsInMatches( 0 );
   params.setRegionsInOverviewMatches( 0 );
   params.setSearchForLocationTypes( 0 );
   params.setSearchForTypes( 0 );
   params.setRequestedLang( m_user->getUser()->getLanguage() );
   params.setAddStreetNamesToCompanies( true );
   params.setTryHarder( true );

   // Create search_reply element
   DOMElement* search_reply = reply->createElement( X( "search_reply" ) );

   // Transaction ID
   search_reply->setAttribute( 
      X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
         X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( search_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), search_reply );
   }
   
   // Find the search_request_header and (search_query | proximity_query )
   DOMNode* searchRequestHeader = NULL;
   DOMNode* searchQuery = NULL;
   DOMNode* proximityQuery = NULL;
   DOMNode* child = cur->getFirstChild();
   while ( child != NULL ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(),
                                    "search_request_header" ) ) 
            {
               searchRequestHeader = child;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_query" ) ) 
            {
               searchQuery = child;
            } else if ( XMLString::equals( child->getNodeName(), 
                                           "proximity_query" ) ) 
            {
               proximityQuery = child;
            } else { // Odd element 
               mc2log << warn << "XMLParserThread::"
                         "xmlParseSearchRequest "
                         "odd Element in search_request element: "
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         default:
            mc2dbg4 << "XMLParserThread::"
                      "xmlParseSearchRequest "
                      "odd node type in search_request element: " 
                    << child->getNodeName() 
                    << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   if ( ok && searchRequestHeader != NULL ) {
      // Read settings
      ok = xmlParseSearchRequestSearchRequestHeader( 
         searchRequestHeader,
         search_reply, reply, indentLevel + 1,
         params,
         latlonForSearchHits, positionSystem, positionSearchItems,
         positionSearchAreas, 
         searchAreaStartingIndex, searchAreaEndingIndex,
         searchItemStartingIndex, searchItemEndingIndex,
         errorCode, errorMessage );
   } 

   using XMLServerUtility::appendStatusNodes;

   if ( ok ) {
      if ( searchQuery != NULL ) {
         // Read and handle Search Request
         ok = xmlParseSearchRequestSearchQuery(
            searchQuery, search_reply, reply,
            indentLevel + 1, indent,
            params,
            latlonForSearchHits, positionSystem, positionSearchItems,
            positionSearchAreas, 
            searchAreaStartingIndex, searchAreaEndingIndex,
            searchItemStartingIndex, searchItemEndingIndex,
            errorCode, errorMessage );
      } else if ( proximityQuery != NULL ) {
         // Read and handle Proximity request
         ok = xmlParseSearchRequestProximityQuery(
            proximityQuery, search_reply, reply,
            indentLevel + 1, indent,
            params,
            latlonForSearchHits, positionSystem, positionSearchItems,
            positionSearchAreas, 
            searchAreaStartingIndex, searchAreaEndingIndex,
            searchItemStartingIndex, searchItemEndingIndex,
            errorCode, errorMessage );
      } else {
         // No query
         ok = false;
         errorCode = "-1";
         errorMessage = "Missing data to make search from. ";
         if ( searchRequestHeader == NULL ) {
            errorMessage.append( "search_request_header is missing. " );
         }
         if ( searchQuery == NULL ) {
            errorMessage.append( "search_query is missing. " );
         }
         if ( proximityQuery == NULL ) {
            errorMessage.append( "proximity_query is missing. " );
         }
      }
      if ( !ok ) {
         appendStatusNodes( search_reply, reply, 
                            indentLevel + 1, indent,
                            errorCode.c_str(), errorMessage.c_str() );
         // Error handled continue
         ok = true;
         mc2log << info << "SearchRequest: Failed "
                << errorCode << "," << errorMessage << endl;;
      }
   } else {
      // The error is in errorCode, errorMessage
      appendStatusNodes( search_reply, reply, indentLevel + 1, indent,
                         errorCode.c_str(), errorMessage.c_str() );
      // Error handled
      ok = true;
      mc2log << info << "SearchRequest: Failed "
             << errorCode << "," << errorMessage << endl;
   }

   if ( indent ) {
      // Newline and indent before end search_reply tag   
      search_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }

   return ok;
}


bool 
XMLParserThread::xmlParseSearchRequestSearchRequestHeader( 
   DOMNode* cur, 
   DOMNode* out,
   DOMDocument* reply,
   int indentLevel,
   SearchRequestParameters& params,
   bool& latlonForSearchHits,
   XMLCommonEntities::coordinateType& positionSystem,
   bool& positionSearchItems,
   bool& positionSearchAreas,
   int32& searchAreaStartingIndex,
   int32& searchAreaEndingIndex,
   int32& searchItemStartingIndex,
   int32& searchItemEndingIndex,
   MC2String& errorCode, MC2String& errorMessage )
{
   mc2dbg4 << "XMLParserThread::xmlParseSearchRequestSearchRequestHeader" 
           << endl;
   bool ok = true;

   // Go throu attributes
   DOMElement* search_request_header = static_cast< DOMElement* >( cur );
   DOMNamedNodeMap* attributes = search_request_header->getAttributes();
   DOMNode* attribute;

   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(),
                              "position_sytem" ) ) 
      {
         positionSystem = XMLCommonEntities::coordinateFormatFromString(
            tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "position_search_items" ) )
      {
         positionSearchItems = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "position_search_areas" ) )
      {
         positionSearchAreas = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "search_area_starting_index" ) )
      {
         char* tmpPtr = NULL;
         uint32 tmp = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr != tmpStr ) {
            searchAreaStartingIndex = tmp;
         } else {
            mc2log << warn << "XMLParserThread::"
               "xmlParseSearchRequestSearchRequestHeader"
               "search_area_starting_index not a "
               "number value " << tmpStr << endl;
            ok = false;
            errorCode = "-1";
            errorMessage = "search_area_starting_index not a "
               "number.";
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "search_area_ending_index" ) )
      {
         char* tmpPtr = NULL;
         uint32 tmp = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr != tmpStr ) {
            searchAreaEndingIndex = tmp;
         } else {
            mc2log << warn << "XMLParserThread::"
               "xmlParseSearchRequestSearchRequestHeader"
               "search_area_ending_index not a "
               "number value " << tmpStr << endl;
            ok = false;
            errorCode = "-1";
            errorMessage = "search_area_ending_index not a "
               "number.";
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "search_item_starting_index" ) )
      {
         char* tmpPtr = NULL;
         uint32 tmp = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr != tmpStr ) {
            searchItemStartingIndex = tmp;
         } else {
            mc2log << warn << "XMLParserThread::"
               "xmlParseSearchRequestSearchRequestHeader"
               "search_item_starting_index not a "
               "number value " << tmpStr << endl;
            ok = false;
            errorCode = "-1";
            errorMessage = "search_item_starting_index not a "
               "number.";
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "search_item_ending_index" ) )
      {
         char* tmpPtr = NULL;
         uint32 tmp = strtol( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr != tmpStr ) {
            searchItemEndingIndex = tmp;
         } else {
            mc2log << warn << "XMLParserThread::"
               "xmlParseSearchRequestSearchRequestHeader"
               "search_item_ending_index not a "
               "number value " << tmpStr << endl;
            ok = false;
            errorCode = "-1";
            errorMessage = "search_item_ending_index not a "
               "number.";
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "full_search_area_match_purge" ) )
      {
         params.setUniqueOrFull( StringUtility::checkBoolean( tmpStr ) );
      } else {
         mc2log << warn << "XMLParserThread::"
                   "xmlParseSearchRequestSearchRequestHeader "
                   "unknown attribute "
                << "Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }



   DOMNode* child = cur->getFirstChild();

   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(),
                                    "search_preferences" ) ) 
            {
               // Get preferences
               ok = xmlParseSearchRequestSearchRequestHeaderPreferences(
                  child,
                  out,
                  reply,
                  indentLevel + 1,
                  params,
                  errorCode, errorMessage );
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_explicit_itemid" ) )
            {
               // TODO: explicit_itemid is obsolete, backward compability
               //       only
               // Set latlonForSearchHits 
               latlonForSearchHits = true;
            } else { // Odd element in user element
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseSearchRequestSearchRequestHeader "
                      << "odd Element in search_request_header"
                      << " element: " << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseSearchRequestSearchRequestHeader "
                   << "odd node type in search_request_header element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   params.setLookupCoordinates(
      (latlonForSearchHits || positionSearchItems || 
       positionSearchAreas) );

   return ok;
}


bool 
XMLParserThread::xmlParseSearchRequestSearchQuery(
   DOMNode* cur, 
   DOMNode* out,
   DOMDocument* reply,
   int indentLevel,
   bool indent,
   const SearchRequestParameters& params,
   bool latlonForSearchHits,
   XMLCommonEntities::coordinateType positionSystem,
   bool positionSearchItems,
   bool positionSearchAreas,
   int32 searchAreaStartingIndex,
   int32 searchAreaEndingIndex,
   int32 searchItemStartingIndex,
   int32 searchItemEndingIndex,
   MC2String& errorCode, MC2String& errorMessage )
{
   bool ok = true;

   mc2dbg4 << "XMLParserThread::xmlParseSearchRequestSearchQuery" << endl;
//   DEBUG4(XMLTreeFormatter::printTree( cur ));
   
   
   // Get the area query or item
   bool searchForArea = true;
   // If search for location
   char* locationString = NULL;
   // If have location
   uint32 areaMapID = 0;
   uint32 areaMaskID = 0;
   uint32 areaItemID = 0;
   char* locationName = NULL;
   uint32 areaType = 0;

   // item search
   char* searchString = NULL;
   bool searchForItem = false;
   TopRegionMatch* topRegion = NULL;

   // Search parameters
   SearchRequestParameters searchParams( params );
   searchParams.setEndHitIndex( searchAreaEndingIndex );
   searchParams.setNbrSortedHits( searchAreaEndingIndex + 1 );
   searchParams.setRegionsInMatches( SEARCH_ALL_REGION_TYPES );
   searchParams.setRegionsInOverviewMatches( 
      SEARCH_ALL_REGION_TYPES );

   // Go throu children and get data.
   DOMNode* child = cur->getFirstChild();

   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            if ( XMLString::equals( child->getNodeName(),
                                    "top_region" ) ) 
            {
               topRegion = getTopRegionMatchFromTopRegion( 
                  child, errorCode, errorMessage );
               if ( topRegion == NULL ) {
                  mc2log << "XMLParserThread::"
                         << "xmlParseSearchRequestSearchQuery "
                         << "bad top_region." << endl;
                  // errorCode, errorMessage already filled in
                  ok = false;
               }
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_area_query" ) )
            {
               searchForArea = true;
               locationString = XMLUtility::getChildTextValue( child );
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_area" ) ) 
            {
               searchForArea = false;
               // Get search_area data, 
               if ( ! XMLSearchUtility::
                    getSearchAreaData( child,
                                       areaMapID, areaItemID,
                                       areaMaskID,
                                       locationName, areaType,
                                       errorCode, errorMessage ) )
               {
                  mc2log << "XMLParserThread::"
                         << "xmlParseSearchRequestSearchQuery "
                         << "bad search_area " << endl;
                  ok = false;
               }
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_item_query" ) )
            {
               // Get the item query
               searchForItem = true;
               searchString = XMLUtility::getChildTextValue( child );
               if ( searchString[ 0 ] == '\0' ) {
                  searchForItem = false; // No need to search for ""
               } else {
                  // house_number
                  DOMAttr* hn = static_cast<DOMElement*> ( child )
                     ->getAttributeNode( X( "house_number" ) );
                  if ( hn != NULL ) {
                     char* tmpStr = XMLUtility::transcodefromucs( 
                        hn->getNodeValue() );
                     // Append to searchString
                     char* tmp = new char[ strlen( searchString ) + 
                                           strlen( tmpStr ) + 2 ];
                     strcpy( tmp, tmpStr );
                     strcat( tmp, " " );
                     strcat( tmp, searchString );
                     delete [] searchString;
                     searchString = tmp;
                     delete [] tmpStr;
                  }
               }
               if ( searchForItem ) {
                  // now lets check and see if this is some kind of
                  // category search
                  // ( Note: must const cast here)
                  if ( ::checkAndSetCategorySearch( getSearchHandler(),
                                                    searchString,
                                                    searchParams ) ) {
                     delete [] searchString;
                     // remove search string, the category is set
                     searchString = StringUtility::newStrDup( "" );
                  }
               }
            } else {
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseSearchRequestSearchQuery "
                      << "search_area_node has unknown name. "
                      << "DTDs must be wrong!"
                      << "NodeName: " << child->getNodeName() 
                      << "search_area" << endl;
            } 
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseSearchRequestSearchQuery odd "
                   << "node type in search_area_node element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType()<< endl;
            break;
      }
      child = child->getNextSibling();
   }

   uint32 topRegionID = StringTable::SWEDEN_CC;
   if ( topRegion ) { 
      // Use selected
      topRegionID = topRegion->getID();
   } else {
      // Check "default" country
      if ( m_group->getTopRegionRequest( this )->getTopRegionWithID(
              topRegionID ) == NULL ) 
      {
         // Ok use first then
         topRegionID = m_group->getTopRegionRequest( this )
            ->getTopRegion( 0 )->getID();
      }
   }

   if ( !checkUserRegionAccess( topRegionID, 
                                m_user->getUser(),
                                m_authData->urmask ) )
   {
      ok = false;
      errorCode = "-5";
      errorMessage.append( "Outside allowed area." );
   }

   using XMLServerUtility::appendStatusNodes;

   if ( ok ) {
      // Search
      SearchResultRequest* req = NULL;

      char* requestLocationString = StringUtility::trimStart( 
         locationString );
      StringUtility::trimEnd( requestLocationString );
      const char* requestSearchString = const_cast<const char*>
         ( StringUtility::trimStart( searchString ) );
      StringUtility::trimEnd( const_cast<char*>( requestSearchString ) );
      if ( requestSearchString == NULL ) {
         requestSearchString = "";
      }
      
      if ( searchForArea ) {
         if ( MC2String(requestLocationString) != "storkafinger" &&
              requestLocationString[ 0 ] != '\0' ) 
         {
            req = new SearchRequest( getNextRequestID(), searchParams,
                                     topRegionID, requestLocationString,
                                     requestSearchString,
                                     m_group->getTopRegionRequest(this) );
         } else {
            // Use "closest search" to search the whole country
            // I.e. don't use the location string.
            mc2log << "[XSR]: Searching top region " << topRegionID << endl;
            req = new ClosestSearchRequest(
               getNextRequestID(),
               searchParams,
               topRegionID,
               requestSearchString,
               m_group->getTopRegionRequest(this));
         }
      } else {
         searchParams.setEndHitIndex( searchItemEndingIndex );
         searchParams.setNbrSortedHits( searchItemEndingIndex + 1 );
         // Have selected area
         vector<IDPair_t> selectedAreas;
         selectedAreas.push_back( IDPair_t( areaMapID, areaItemID ) );
         req = new SearchRequest( getNextRequestID(), searchParams,
                                  selectedAreas, requestSearchString,
                                  m_group->getTopRegionRequest(this) );
      }

      mc2dbg8 << "About to send searchrequest" << endl;
      putRequest( req );
      mc2dbg8 << "Searchrequest done" << endl;
   
      if ( req->getStatus() == StringTable::OK ) {
         // Debit auth user for single search
         MC2String locName;
         if ( locationString != NULL ) {
            locName = locationString;
         } else if ( !searchForArea && locationName != NULL ) {
            // Set locationString to name of supplied search_area
            locName = locationName;
            char areaData[256];
            sprintf( areaData, ":%u:%u", areaMapID, areaItemID );
            locName.append( areaData );
         }
         
         // Set debitamount here untill module sets it ok
         req->setProcessingTime( ( TimeUtility::getCurrentTime() - 
                                   m_irequest->getStartTime() ) * 1000 );
         ok = getDebitHandler()->makeSearchDebit( m_user,
                                                  req,
                                                  NULL,
                                                  locName.c_str(),
                                                  searchString );
                      
         // Done with debit now print matches if debit succeded
         using namespace XMLSearchUtility;
         if ( ok ) {
            // Get matches and print them in nice elements
            if ( searchForArea ) {
               // Get overviewmatches from req.
               MC2String indentStr( indentLevel*3, ' ' );
               indentStr.insert( 0, "\n" );
               XStr XindentStr( indentStr.c_str() );
               DOMElement* search_area_list = reply->createElement( 
                  X( "search_area_list" ) );

               vector<OverviewMatch*> matches;

               for ( uint32 i = 0 ; i < req->getNbrOverviewMatches() ; ++i)
               {
                  matches.push_back( req->getOverviewMatch( i ) );
               }

               searchAreaStartingIndex = indexForArray(
                  searchAreaStartingIndex, 
                  int32(req->getNbrOverviewMatches()) );
               searchAreaEndingIndex = indexForArray(
                  searchAreaEndingIndex, 
                  int32(req->getNbrOverviewMatches()) );
               if ( req->getNbrOverviewMatches() > 0 &&
                    searchAreaEndingIndex >= 
                    int32(req->getNbrOverviewMatches()) &&
                    searchAreaEndingIndex > searchAreaStartingIndex )
               {
                  searchAreaEndingIndex = 
                     int32(req->getNbrOverviewMatches()) - 1;
               }

               int32 nbrAreaMatches = rangeArraySize(
                  searchAreaStartingIndex, searchAreaEndingIndex,
                  int32(req->getNbrOverviewMatches()) );

               char ctmp[20];
               // numberitems
               sprintf( ctmp, "%d", nbrAreaMatches );
               search_area_list->setAttribute( X( "numberitems" ), 
                                               X( ctmp ) );
               // ending_index
               sprintf( ctmp, "%d", searchAreaEndingIndex );
               search_area_list->setAttribute( X( "ending_index" ), 
                                               X( ctmp ) );
               // starting_index
               sprintf( ctmp, "%d", searchAreaStartingIndex );
               search_area_list->setAttribute( X( "starting_index" ), 
                                               X( ctmp ) );
               // total_numberitems
               sprintf( ctmp, "%d", req->getNbrOverviewMatches() );
               search_area_list->setAttribute( X( "total_numberitems" ), 
                                               X( ctmp ) );
               

               if ( indent ) {
                  out->appendChild( 
                     reply->createTextNode( XindentStr.XMLStr() ) );
               }
               out->appendChild( search_area_list );

               for ( int32 i = searchAreaStartingIndex ; 
                     i < int32(req->getNbrOverviewMatches()) &&
                     i <= searchAreaEndingIndex ; 
                     i++ ) 
               {
                  MC2BoundingBox bbox(matches[i]->getBBox());
                  // Print search_area
                  appendSearchArea( 
                     search_area_list, reply, matches[ i ],
                     indentLevel + 1, indent,
                     positionSearchAreas,
                     positionSearchAreas ? matches[ i ]->getCoords().lat :
                     MAX_INT32,
                     positionSearchAreas ? matches[ i ]->getCoords().lon :
                     MAX_INT32,
                     &bbox,
                     positionSystem, params );
               }

               if ( nbrAreaMatches > 0 && indent ) {
                  // Add newline and indent before search_area_list endtag
                  search_area_list->appendChild( 
                     reply->createTextNode( XindentStr.XMLStr() ) );
               }
            }
     
            if ( ((!searchForArea || req->getNbrOverviewMatches() == 1)
                  && searchForItem) || 
                 !req->getMatches().empty() ) 
            {
               /* (!searchForArea || (req->getUniqueOrFull() &&
                                     req->getNbrOverviewMatches() == 1) ) &&
                                     searchForItem ) */
            
               const char* currentLocation = locationName;

               if ( currentLocation == NULL && searchForArea &&
                    req->getNbrOverviewMatches() > 0 ) 
               {
                  // try areaMatch
                  currentLocation = 
                     req->getOverviewMatch( 0 )->getName0();
               } else {
                  if ( currentLocation == NULL ) {
                     currentLocation = "";
                  }
               }

               appendSearchItemList( out, reply, indentLevel, indent,
                                     *req, currentLocation, 
                                     searchItemStartingIndex, 
                                     searchItemEndingIndex, 
                                     latlonForSearchHits, 
                                     positionSearchItems,
                                     positionSystem, params );


            }

            mc2log << info << "SearchRequest OK";
            if ( searchForArea ) {
               mc2log << " locationString \"" << locationString << "\"";
            } else {
               mc2log << " areaMapID " << areaMapID << " areaItemID "
                      << areaItemID << " areaName " << locationName;
            }
            if ( searchForItem ) {
               mc2log << " searchString \"" << searchString << "\"";
            }
            if ( searchForArea ) {
               mc2log << " nbrAreaMatches " 
                      << req->getNbrOverviewMatches();
            }
            if ( searchForItem ) {
               mc2log << " nbrItemMatches " << req->getMatches().size();
            }
            mc2log << " TID " << hex << topRegionID << dec << endl;
            mc2log << endl;
         } else {
            mc2log << warn << "XMLParserThread::"
                      "xmlParseSearchRequestSearchQuery search debit "
                      "failed, when communicating with database." << endl;
            // Send we're sorry but we can't give you the data becauce
            // debiting failed status
            appendStatusNodes( out, reply, indentLevel, indent,
                               "-1",
                               "Search debit failed." );
            ok = true;
         }
         
      } else {
         const char* errorStr = NULL;
         MC2String errorCode = "-1";
         errorStr = StringTable::getString( 
            req->getStatus(), m_authData->stringtClientLang );
         if ( req->getStatus() == StringTable::TIMEOUT_ERROR ) {
            errorCode = "-3";
         } else if ( req->getStatus() == StringTable::OUTSIDE_ALLOWED_AREA)
         {
            errorCode = "-5";
         }
         mc2log << warn << "XMLParserThread::"
                   "xmlParseSearchRequestSearchQuery SearchRequest failed"
                << "Status: " << errorStr<< endl;
      
         appendStatusNodes( out, reply,
                            indentLevel, indent,
                            errorCode.c_str(), errorStr );
         // Handled, not ok but handled
         ok = true;
      }
      
      delete req;
   } else {
      // Error returned to caller
      mc2log << info << "SearchRequest: Failed "
             << errorCode << ", " << errorMessage << endl;
   }
   
   delete topRegion;
   delete [] locationString;
   delete [] locationName;
   delete [] searchString;
   
   return ok;
}


bool 
XMLParserThread::xmlParseSearchRequestProximityQuery(
            DOMNode* cur, 
            DOMNode* out,
            DOMDocument* reply,
            int indentLevel,
            bool indent,
            const SearchRequestParameters& params,
            bool latlonForSearchHits,
            XMLCommonEntities::coordinateType positionSystem,
            bool positionSearchItems,
            bool positionSearchAreas,
            int32 searchAreaStartingIndex,
            int32 searchAreaEndingIndex,
            int32 searchItemStartingIndex,
            int32 searchItemEndingIndex,
            MC2String& errorCode, MC2String& errorMessage )
{
   bool ok = true;

   bool hasPositionItem = false;
   int32 lat = MAX_INT32;
   int32 lon = MAX_INT32;
   uint16 angle = MAX_UINT16;
   bool hasSearchItem = false;
   VanillaMatch* vanillaMatch = NULL;
   bool hasDistance = false;
   uint32 distance = 0;
   bool hasSearchString = false;
   MC2String searchString;
   bool hasBoundingbox = false;
   MC2BoundingBox bbox;
   set<uint32>* allowedMaps = NULL;
   uint32 now = TimeUtility::getRealTime();

   SearchRequestParameters searchParams( params );
   searchParams.setEndHitIndex( searchItemEndingIndex );
   searchParams.setNbrSortedHits( searchItemEndingIndex + 1 );
   searchParams.setRegionsInMatches( SEARCH_ALL_REGION_TYPES );
   searchParams.setRegionsInOverviewMatches( 
      SEARCH_ALL_REGION_TYPES );

   // Go throu children and get data.
   DOMNode* child = cur->getFirstChild();

   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            if ( XMLString::equals( child->getNodeName(),
                                    "search_item" ) ) 
            {
               // Get search_item position
               vanillaMatch = XMLSearchUtility::
                  getVanillaMatchFromSearchItem( child, 
                                                 errorCode, errorMessage );
               if ( vanillaMatch != NULL ) {
                  hasSearchItem = true;
               } else {
                  // getVanillaMatchFromSearchItem sets error strings
                  ok = false;
               }
            } else if ( XMLString::equals( child->getNodeName(),
                                           "position_item" ) ) 
            {
               // Get position
               if ( XMLCommonElements::getPositionItemData( 
                       child, lat, lon, angle, errorCode, errorMessage ) ) 
               {
                  hasPositionItem = true;
               } else {
                  // getPositionItemData sets errorcode and message
                  ok = false;
               }
            } else if ( XMLString::equals( child->getNodeName(),
                                           "distance" ) ) 
            {
               // Check for number
               ScopedArray<char> tmpStr(XMLUtility::getChildTextValue( child ));
               char* tmpPtr = NULL;
               uint32 tmp = strtol( tmpStr.get(), &tmpPtr, 10 );
               if ( tmpPtr != tmpStr.get() ) {
                  distance = tmp;
               } else {
                  mc2log << warn << "XMLParserThread::"
                     "xmlParseSearchRequestProximityQuery"
                     "distance not a "
                     "number value " << tmpStr.get() << endl;
                  ok = false;
                  errorCode = "-1";
                  errorMessage = "distance not a number.";
               }

               hasDistance = true;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_item_query" ) )
            {
               hasSearchString = true;
               ScopedArray<char> tmpStr(XMLUtility::getChildTextValue( child ));
               searchString = tmpStr.get();
               // now lets check and see if this is some kind of
               // category search
               // ( Note: must const cast here)
               if ( ::checkAndSetCategorySearch( getSearchHandler(),
                                                 searchString, 
                                                 searchParams ) ) {
                  // remove search string, the category is set
                  searchString = "";
               }

            } else if ( XMLString::equals( child->getNodeName(),
                                           "boundingbox" ) ) 
            {
               hasBoundingbox = true;
               if ( !XMLCommonElements::readBoundingbox( 
                       child, bbox, errorCode, errorMessage ) ) 
               {
                  ok = false;
                  mc2log << warn << "XMLParserThread::"
                            "xmlParseSearchRequestProximityQuery"
                         << " bad boundingbox!" << endl;
               }
            } else {
               mc2log << warn << "XMLParserThread::"
                         "xmlParseSearchRequestProximityQuery "
                         "proximity_query has unknown child: "
                      << "NodeName: " << child->getNodeName() << endl;
            } 
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                      "xmlParseSearchRequestProximityQuery odd "
                      "node type in proximity_query element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType()<< endl;
            break;
      }
      child = child->getNextSibling();
   }

   if ( ok && !getMapIdsForUserRegionAccess( m_user->getUser(), 
                                             allowedMaps, now,
                                             m_authData->urmask ) ) 
   {
      errorCode = "-3";
      errorMessage = "Problem when checking Allowed User Region Access.";
      ok = false;
   } 

   using namespace XMLSearchUtility;

   if ( ok && (hasSearchItem || hasPositionItem || hasBoundingbox) ) {

      if ( !hasSearchString && (hasDistance || hasBoundingbox) ) {
         // Proximity search
         ProximityRequest* req = NULL;
         if ( hasBoundingbox ) {
            set<ItemTypes::itemType> types;
            ItemTypes::searchTypeToItemTypes(
               types, params.getSearchForTypes() );
            req = new ProximityRequest( getNextRequestID(), searchParams,
                                        bbox, types );
         } else {
            ProximityPositionRequestPacket* p = new 
               ProximityPositionRequestPacket( 0, 0 );
            uint32 itemID = MAX_UINT32;
            uint16 offset = 0;

            // Get data (set mapID)
            PositionRequestPacketContent contentType = 
               COVEREDIDSREQUEST_CONTENT_INVALID;
            if ( hasSearchItem ) {
               contentType = COVEREDIDSREQUEST_CONTENT_ITEMID;
               // Set mapID
               p->setMapID( vanillaMatch->getMapID() );
               itemID = vanillaMatch->getMainItemID();
               if ( vanillaMatch->getType() == SEARCH_STREETS ) {
                  VanillaStreetMatch* vsm = 
                     static_cast< VanillaStreetMatch* > ( vanillaMatch );
                  offset = vsm->getOffset();
                  if ( vsm->getStreetNbr() != 0 && 
                       vsm->getStreetSegmentID() != MAX_UINT32 ) 
                  {
                     itemID = vsm->getStreetSegmentID();
                  }
               }
            } else if ( hasPositionItem ) {
               contentType = COVEREDIDSREQUEST_CONTENT_LAT_LON;
            }


            p->encodeRequest( contentType, lat, lon, distance, itemID, 
                              offset, PROXIMITY_SEARCH_ITEMS_MATCHES, 
                              searchItemEndingIndex, /* nbrHits */ 
                              hasSearchString, searchString.c_str(),
                              params.getSearchForTypes(),
                              params.getMatchType(), 
                              params.getStringPart(), 
                              params.getSortingType(), 
                              params.getNbrSortedHits(), 
                              params.getRequestedLang(), 
                              params.getSearchForLocationTypes() );

            req = new ProximityRequest( getNextRequestID(), p );
         } // end else not bbox

         mc2dbg8 << "About to send ProximityRequest" << endl;
         putRequest( req );
         mc2dbg8 << "ProximityRequest done" << endl;

         PacketContainer* ansCont = req->getAnswer();
         if ( ansCont != NULL &&
              static_cast<ReplyPacket*>( 
                 ansCont->getPacket() )->getStatus() == StringTable::OK )
         {
            // Debit auth user for proximity
            ok = getDebitHandler()->makeProximityDebit( 
               m_user,
               req,
               ansCont,
               hasSearchString ? searchString.c_str() : NULL,
               lat, lon, distance );

            if ( ok ) {
               appendSearchItemList( out, reply, indentLevel, indent,
                                     *req, "" /*currentLocation*/, 
                                     searchItemStartingIndex, 
                                     searchItemEndingIndex, 
                                     latlonForSearchHits, 
                                     positionSearchItems,
                                     positionSystem, params );
            
            } else {
               mc2log << warn << "XMLParserThread::"
                  "xmlParseSearchRequestProximityQuery search debit "
                  "failed." << endl;
               // Send we're sorry but we can't give you the data becauce
               // debiting failed status
               ok = false;
               errorCode = "-1";
               errorMessage = "Search debit failed.";
            }
         } else {
            errorCode = "-1";
            errorMessage = "Proximity failed: ";
            if ( ansCont != NULL ) {
               if ( StringTable::stringCode( static_cast<ReplyPacket*>( 
                       ansCont->getPacket() )->getStatus() ) == 
                    StringTable::TIMEOUT_ERROR ) 
               {
                  errorCode = "-3";
               }
               errorMessage.append( StringTable::getString( 
                  StringTable::stringCode( static_cast<ReplyPacket*>( 
                     ansCont->getPacket() )->getStatus() ),
                  StringTable::ENGLISH ) );
            } else {
               errorMessage.append( "Internal error while proximity." );
            }

            mc2log << warn << "XMLParserThread::"
               "xmlParseSearchRequestProximityQuery ProximityRequest "
               "failed"
                   << "Message: " << errorMessage << endl;
            ok = false;
         }
         delete ansCont;
         delete req;
      } else if ( (hasDistance || hasBoundingbox) && hasSearchString ) { 
         // Has searchstring
         SearchRequest* req = NULL;

         if ( hasBoundingbox ) {
            req = new SearchRequest( getNextRequestID(), searchParams,
                                     searchString, bbox, 
                                     m_group->getTopRegionRequest(this) );
         } else {
            // Radius
            if ( hasSearchItem ) {
               uint32 mapID = vanillaMatch->getMapID();
               uint32 itemID = vanillaMatch->getMainItemID();
               if ( vanillaMatch->getType() == SEARCH_STREETS ) {
                  VanillaStreetMatch* vsm = 
                     static_cast< VanillaStreetMatch* > ( vanillaMatch );
                  if ( vsm->getStreetNbr() != 0 && 
                       vsm->getStreetSegmentID() != MAX_UINT32 ) 
                  {
                     itemID = vsm->getStreetSegmentID();
                  }
               }
               req = new SearchRequest( getNextRequestID(), searchParams,
                                        IDPair_t( mapID, itemID ),
                                        searchString, 
                                        m_group->getTopRegionRequest(this),
                                        distance);
            } else if ( hasPositionItem ) {
               req = new SearchRequest( getNextRequestID(), searchParams,
                                        MC2Coordinate( lat, lon ),
                                        searchString,
                                        m_group->getTopRegionRequest(this),
                                        distance);
            }
         } // End else not hasBoundingbox

         req->setAllowedMaps( allowedMaps );

         mc2dbg8 << "About to send Search-ProximityRequest" << endl;
         putRequest( req );
         mc2dbg8 << "Search-ProximityRequest done" << endl;
         
         if ( req->getStatus() == StringTable::OK ) {
            // Debit auth user for proximity
            if ( !hasBoundingbox ) {
               ok = getDebitHandler()->makeProximityDebit( 
                  m_user, req, params,
                  hasSearchString ? searchString.c_str() : NULL,
                  MC2Coordinate( lat, lon ), distance );
            } else {
               // TODO: Add boundingbox debit 
               MC2Coordinate center;
               bbox.getCenter( center.lat, center.lon );
               uint32 radius = 0;
               uint32 radiusMeters = 0;
               if ( bbox.getWidth() > bbox.getHeight() ) {
                  radius = bbox.getWidth() / 2;
               } else {
                  radius = bbox.getHeight() / 2;
               }
               radiusMeters = 
                  uint32( radius * GfxConstants::MC2SCALE_TO_METER );
               ok = getDebitHandler()->makeProximityDebit( 
                  m_user, req, params,
                  hasSearchString ? searchString.c_str() : NULL,
                  center, radiusMeters );
            }

            if ( ok ) {
               appendSearchItemList( out, reply, indentLevel, indent,
                                     *req, ""/*currentLocation*/, 
                                     searchItemStartingIndex, 
                                     searchItemEndingIndex, 
                                     latlonForSearchHits, 
                                     positionSearchItems,
                                     positionSystem, params );
               

            } else {
               mc2log << warn << "XMLParserThread::"
                  "xmlParseSearchRequestProximityQuery search-proximity "
                  "debit failed." << endl;
               // Send we're sorry but we can't give you the data becauce
               // debiting failed status
               ok = false;
               errorCode = "-1";
               errorMessage = "Search debit failed.";
            }

         } else {
            errorCode = "-1";
            errorMessage = "Proximity failed: ";
            if ( StringTable::stringCode( req->getStatus() ) == 
                 StringTable::TIMEOUT_ERROR ) 
            {
               errorCode = "-3";
            }
            errorMessage.append( 
               StringTable::getString( req->getStatus(),
               StringTable::ENGLISH ) );

            mc2log << warn << "XMLParserThread::"
                      "xmlParseSearchRequestProximityQuery "
                      "Search-ProximityRequest failed Message: " 
                   << errorMessage << endl;
            ok = false;
         }
         delete req;
      } else if ( hasSearchString ) {
         // Has searchstring but no bbox nor distance
         SearchResultRequest* req = NULL;
         MC2Coordinate sortOrigin;

         if ( hasSearchItem ) {
            uint32 mapID = vanillaMatch->getMapID();
            uint32 itemID = vanillaMatch->getMainItemID();
            if ( vanillaMatch->getType() == SEARCH_STREETS ) {
               VanillaStreetMatch* vsm = 
                  static_cast< VanillaStreetMatch* > ( vanillaMatch );
               if ( vsm->getStreetNbr() != 0 && 
                    vsm->getStreetSegmentID() != MAX_UINT32 ) 
               {
                  itemID = vsm->getStreetSegmentID();
               }
            }
            req = new SearchRequest( 
               getNextRequestID(), searchParams, IDPair_t( mapID, itemID ),
               searchString, m_group->getTopRegionRequest( this ) );
            static_cast< SearchRequest* > ( req )
               ->setAllowedMaps( allowedMaps );
         } else if ( hasPositionItem ) {
            sortOrigin = MC2Coordinate( lat, lon );
            req = new ClosestSearchRequest(
               getNextRequestID(), searchParams,
               sortOrigin, searchString,
               m_group->getTopRegionRequest( this ), 
               allowedMaps ? *allowedMaps :  set<uint32>() );
         }
         
         mc2dbg8 << "About to send ClosestSearch-ProximityRequest" << endl;
         putRequest( req );
         mc2dbg8 << "ClosestSearch-ProximityRequest done" << endl;
         
         if ( req->getStatus() == StringTable::OK ) {
            // Debit auth user
            if ( hasSearchItem ) {
               sortOrigin = static_cast<SearchRequest*>( req )
                  ->getPosition();
               ok = getDebitHandler()->makeProximityDebit( 
                  m_user, req, params, searchString.c_str(),
                  sortOrigin, distance );
            } else {
               ok = getDebitHandler()->makeProximityDebit( 
                  m_user, req, params, searchString.c_str(),
                  sortOrigin, distance );
            }
            
            if ( ok ) {
               appendSearchItemList( out, reply, indentLevel, indent,
                                     *req, ""/*currentLocation*/, 
                                     searchItemStartingIndex, 
                                     searchItemEndingIndex, 
                                     latlonForSearchHits, 
                                     positionSearchItems,
                                     positionSystem, params );
               

            } else {
               mc2log << warn << "XMLParserThread::"
                  "xmlParseSearchRequestProximityQuery "
                  "closest-search-proximity "
                  "debit failed." << endl;
               // Send we're sorry but we can't give you the data becauce
               // debiting failed status
               ok = false;
               errorCode = "-1";
               errorMessage = "Search debit failed.";
            }

         } else {
            errorCode = "-1";
            errorMessage = "Proximity failed: ";
            if ( StringTable::stringCode( req->getStatus() ) == 
                 StringTable::TIMEOUT_ERROR ) 
            {
               errorCode = "-3";
            }
            errorMessage.append( 
               StringTable::getString( req->getStatus(),
               StringTable::ENGLISH ) );

            mc2log << warn << "XMLParserThread::"
                      "xmlParseSearchRequestProximityQuery "
                      "Closest-Search-ProximityRequest failed Message: " 
                   << errorMessage << endl;
            ok = false;
         }


         delete req;
      }

   } else {
      // Error retuned to caller
   }


   delete allowedMaps;
   delete vanillaMatch;

   return ok;
}


bool 
XMLParserThread::xmlParseSearchRequestSearchRequestHeaderPreferences( 
   DOMNode* cur, 
   DOMNode* out,
   DOMDocument* reply,
   int indentLevel,
   SearchRequestParameters& params,
   MC2String& errorCode, MC2String& errorMessage )
{
   bool ok = true;
   mc2dbg4 << "XMLParserThread::"
           << "xmlParseSearchRequestSearchRequestHeaderPreferences"<< endl;

   DOMNode* settings = NULL;
   DOMNode* user_id = NULL;
   DOMNode* user_session_idNode = NULL;
   DOMNode* user_session_keyNode = NULL;
   DOMNode* uinNode = NULL;


   DOMNode* child = cur->getFirstChild();
   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(),
                                    "search_settings" ) ) 
            {
               settings = child;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "user_id" ) ) 
            {
               user_id = child;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "user_session_id" ) ) 
            {
               user_session_idNode = child;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "user_session_key" ) )
            {
               user_session_keyNode = child;
            } else if ( XMLString::equals( child->getNodeName(), "uin" ) )
            {
               uinNode = child; 
            } else {
               mc2log << warn << "XMLParserThread::"
                  "xmlParseSearchRequestSearchRequestHeaderPreferences"
                         "odd Element in search_preferences element: " 
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                      "xmlParseSearchRequestSearchRequestHeaderPreferences"
                      " odd node type in search_preferences element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   if ( user_id != NULL || user_session_idNode != NULL || uinNode != NULL )
   {
      mc2dbg8 << "Preferences is user_id or user_session_id or uin"
              << endl;
      // Get user
      MC2String userID;
      MC2String user_session_id;
      MC2String user_session_key;
      uint32 uin = 0;

      if ( user_id != NULL ) {
         char* tmpStr = XMLUtility::getChildTextValue( user_id );
         userID = tmpStr;
         delete [] tmpStr;
      } else if ( user_session_idNode != NULL ) {
         char* tmpStr = XMLUtility::getChildTextValue( 
            user_session_idNode );
         user_session_id = tmpStr;
         delete [] tmpStr;

         // Get user_session_key
         if ( user_session_keyNode != NULL ) {
            char* tmpStr =  XMLUtility::getChildTextValue( 
               user_session_keyNode );
            user_session_key = tmpStr;
            delete [] tmpStr;
         } else {
            mc2log << warn << "XMLParserThread::"
               "xmlParseSearchRequestSearchRequestHeaderPreferences "
               "No user_session_key with user_session_id." << endl;
            ok = false;
            errorCode = "-1";
            errorMessage = "No user_session_key with user_session_id.";
         }
      } else if ( uinNode != NULL ) {
         char* tmpStr = XMLUtility::getChildTextValue( uinNode );
         char* endPtr = NULL;
         uin = strtoul( tmpStr, &endPtr, 10 );
         if ( endPtr == NULL || *endPtr != '\0' || uin == 0 ) {
            ok = false;
            errorCode = "-1";
            errorMessage = "Problem parsing uin not a valid number.";
            mc2log << warn << "XMLParserThread::"
               "xmlParseSearchRequestSearchRequestHeaderPreferences "
                   << "uin not a valid number. " << MC2CITE( tmpStr )
                   << endl;
         }
         delete [] tmpStr;
      }

      if ( ok && 
           (!userID.empty() || 
            ( !user_session_id.empty() && !user_session_key.empty() ) ||
            uin != 0 ) )
      {
         UserItem* userItem = NULL;
         bool status = true;

         if ( !userID.empty() ) {
            status = getUser( userID.c_str(), userItem, true );
         } else if ( uin != 0 ) {
            status = getUser( uin, userItem, true );
         } else {
            status = getUserBySession( 
               user_session_id.c_str(), user_session_key.c_str(), 
               userItem, true );
         }

         if ( status ) {
            if ( userItem != NULL ) {
               // Get settings from user
               params.setMatchType( 
                  SearchTypes::StringMatching(
                     userItem->getUser()->getSearch_type() ) );
               params.setStringPart( 
                  SearchTypes::StringPart(
                     userItem->getUser()->getSearch_substring() ) );
               params.setSortingType( 
                  SearchTypes::SearchSorting(
                     userItem->getUser()->getSearch_sorting() ) );
               params.setSearchForTypes(
                  userItem->getUser()->getSearchForTypes() );
               params.setSearchForLocationTypes( 
                  userItem->getUser()->getSearchForLocationTypes() );
               params.setRequestedLang( 
                  userItem->getUser()->getLanguage());
               // Forced settings
               if ( settings != NULL ) {
                  mc2dbg8 << "Reading settings element after user" << endl;
                  ok = ::readSearchSettings( settings, params,
                                             errorCode, errorMessage );
               }

            } else {
               mc2log << warn << "XMLParserThread::"
                  "xmlParseSearchRequestSearchRequestHeaderPreferences "
                  "unknown user, userID: " << userID 
                      << " user_session_id " << user_session_id 
                      << " user_session_key " << user_session_key << endl;
               ok = false;
               errorCode = "-1";
               errorMessage = "Failed to find user."; 
            }
         } else {
            mc2log << warn << "XMLParserThread::"
                      "xmlParseSearchRequestSearchRequestHeaderPreferences"
                      " Failed to get user userID: " << userID 
                   << " user_session_id " << user_session_id 
                   << " user_session_key " << user_session_key << endl;
            ok = false;
            errorCode = "-1";
            errorMessage = "Database error retreiving user information.";
         }

         releaseUserItem( userItem );
      } else {
         if ( ok ) {
            mc2log << warn << "XMLParserThread::"
                      "xmlParseSearchRequestSearchRequestHeaderPreferences"
                      " Not enough input to find user with."
                   << endl;
            ok = false;
            errorCode = "-1";
            errorMessage = "Not enough input to find user with.";
         }
      }
   } else if ( settings != NULL ) {
      mc2dbg8 << "Preferences is search_settings " << endl;
      ok = ::readSearchSettings( settings, params,
                                 errorCode, errorMessage );
   } else {
      mc2log << warn << "XMLParserThread::"
                "xmlParseSearchRequestSearchRequestHeaderPreferences "
                "unknown preferences type! "
             << "Nodename " << settings->getNodeName() << endl;
      ok = false;
      errorCode = "-1";
      errorMessage = "Unknown preferences type.";
   }

   return ok;
}

bool 
XMLParserThread::getCoordinatesForMatches( VanillaVector& matches,
                                           Vector& latitudes,
                                           Vector& longitudes,
                                           bool*& hasLatLon )
{
   mc2dbg4 << "XMLParserThread::getCoordinatesForMatches" << endl;
   bool ok = false;
      
   ok = true;
   latitudes.setAllocSize( matches.size() );
   longitudes.setAllocSize( matches.size() );

   CoordinateOnItemRequest* req = new CoordinateOnItemRequest( 
      getNextRequestID() );

   for ( uint32 i = 0 ; i < matches.size() ; i++ ) {
      uint16 offset = 0x7fff;
      uint32 itemID = matches[ i ]->getMainItemID();
      if ( matches[ i ]->getType() == SEARCH_STREETS ) {
         VanillaStreetMatch* vsm = static_cast< VanillaStreetMatch* > ( 
            matches[ i ] );
         offset = vsm->getOffset();
         if ( vsm->getStreetNbr() != 0 && 
              vsm->getStreetSegmentID() != MAX_UINT32 ) 
         {
            itemID = vsm->getStreetSegmentID();
         }
      } else if ( matches[ i ]->getType() == SEARCH_COMPANIES ) {
         offset = static_cast< VanillaCompanyMatch* > ( 
            matches[ i ] )->getOffset(); 
      }
      req->add( matches[ i ]->getMapID(), itemID,
                offset );
   }

   // Send request
   ThreadRequestContainer* reqCont = new ThreadRequestContainer( req );
   mc2dbg8 << "About to send CoordinateOnItemRequest" << endl;
   putRequest( reqCont );
   mc2dbg8 << "CoordinateOnItemRequest returned" << endl;
      
   // Get results
   uint32 mapID = 0;
   uint32 itemID = 0;
   uint16 offset = 0;
   int32 lat = 0;
   int32 lon = 0;
   bool hasLat = false;
   for ( uint32 i = 0 ; i < matches.size() ; i++ ) {
      req->getResult( i, mapID, itemID, offset, lat, lon, hasLat );
      mc2dbg8 << "[XMLSR]: Setting lat/lon " << lat << "," << lon
              << " old : " << matches[i]->getCoords() << endl;
      latitudes[ i ] = lat;
      longitudes[ i ] = lon;
      hasLatLon[ i ] =  hasLat;
   }

   delete req;
   delete reqCont;

   return ok;
}


bool 
XMLParserThread::getCoordinatesForMatches( vector<OverviewMatch*>& matches,
                                           Vector& latitudes,
                                           Vector& longitudes,
                                           bool*& hasLatLon )
{
   bool ok = true;
      
   latitudes.setAllocSize( matches.size() );
   longitudes.setAllocSize( matches.size() );

   CoordinateOnItemRequest* req = new CoordinateOnItemRequest( 
      getNextRequestID() );

   for ( uint32 i = 0 ; i < matches.size() ; i++ ) {
      uint16 offset = 0x7fff;
      uint32 itemID = matches[ i ]->getItemID();
      req->add( matches[ i ]->getMapID(), itemID,
                offset );
   }

   // Send request
   mc2dbg8 << "About to send CoordinateOnItemRequest" << endl;
   putRequest( req );
   mc2dbg8 << "CoordinateOnItemRequest returned" << endl;
      
   // Get results
   uint32 mapID = 0;
   uint32 itemID = 0;
   uint16 offset = 0;
   int32 lat = 0;
   int32 lon = 0;
   bool hasLat = false;
   for ( uint32 i = 0 ; i < matches.size() ; i++ ) {
      req->getResult( i, mapID, itemID, offset, lat, lon, hasLat );
      latitudes[ i ] = lat;
      longitudes[ i ] = lon;
      hasLatLon[ i ] =  hasLat;
   }

   delete req;

   return ok;

}

namespace {
bool 
readSearchSettings( DOMNode* settings,
                    SearchRequestParameters& params,
                    MC2String& errorCode, MC2String& errorMessage ) {
   bool ok = true;

   uint16 categoryType = 0;
   uint32 locationType = 0;
   uint32 regionsInMatches = 0;
   uint32 overviewRegionsInMatches = 0;

   // Read attributes
   DOMNamedNodeMap* attributes = settings->getAttributes();
   DOMNode* attribute;

   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
         
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );

      if ( XMLString::equals( attribute->getNodeName(),
                              "matchtype" ) )  {
         params.setMatchType( StringConversion::
                              searchStringMatchingTypeFromString( tmpStr ) );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "wordmatch" ) ) 
      {
         params.setStringPart(
            StringConversion::searchStringPartTypeFromString( tmpStr ) );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "sorttype" ) ) 
      {
         params.setSortingType(
            StringConversion::searchSortingTypeFromString( tmpStr ) );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "minimum_numberhits" ) )
      {
         char* tmpPtr = NULL;
         uint32 tmp = strtoul( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr != tmpStr ) {
            params.setTryHarder( tmp );
         } else {
            mc2log << warn << "XMLParserThread::readSearchSettings "
                      "setting minimum_numberhits not a "
                      "number, value \"" << tmpStr << "\"" << endl;
            ok = false;
            errorCode = "-1";
            errorMessage = "minimum_numberhits not a number.";
         }
      } else {
         mc2log << warn << "XMLParserThread::readSearchSettings "
                   "unknown attribute Name " 
                << attribute->getNodeName()
                << "Type " << attribute->getNodeType() << endl;
      }
         
      delete [] tmpStr;
   }

   // Read settings
   DOMNode* child = settings->getFirstChild();
   
   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(),
                                    "search_for_municipal" ) ) 
            {
               locationType |= SEARCH_MUNICIPALS;  
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_for_city" ) ) 
            {
               locationType |= SEARCH_BUILT_UP_AREAS;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_for_citypart" ) )
            {
               locationType |= SEARCH_CITY_PARTS;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_for_zipcode" ) )
            {
               locationType |= SEARCH_ZIP_CODES;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_for_ziparea" ) )
            {
               locationType |= SEARCH_ZIP_AREAS;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_for_street" ) )
            {
               categoryType |= SEARCH_STREETS;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_for_company" ) )
            {
               categoryType |= SEARCH_COMPANIES;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_for_category" ) )
            {
               categoryType |= SEARCH_CATEGORIES;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_for_misc" ) ) 
            {
               categoryType |= SEARCH_MISC;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "show_search_area_municipal" ) )
            {
               overviewRegionsInMatches |= SEARCH_MUNICIPALS;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "show_search_area_city" ) ) 
            {
               overviewRegionsInMatches |= SEARCH_BUILT_UP_AREAS;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "show_search_area_city_part" ) )
            {
               overviewRegionsInMatches |= SEARCH_CITY_PARTS;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "show_search_area_zipcode" ) )
            {
               overviewRegionsInMatches |= SEARCH_ZIP_CODES;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "show_search_area_ziparea" ) )
            {
               overviewRegionsInMatches |= SEARCH_ZIP_AREAS;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "show_search_area_country" ) )
            {
               overviewRegionsInMatches |= SEARCH_COUNTRIES;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "show_search_item_municipal" ) )
            {
               regionsInMatches |= SEARCH_MUNICIPALS;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "show_search_item_city" ) )
            {
               regionsInMatches |= SEARCH_BUILT_UP_AREAS;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "show_search_item_city_part" ) )
            {
               regionsInMatches |= SEARCH_CITY_PARTS;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "show_search_item_zipcode" ) )
            {
               regionsInMatches |= SEARCH_ZIP_CODES;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "show_search_item_ziparea" ) )
            {
               regionsInMatches |= SEARCH_ZIP_AREAS;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "language" ) ) 
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               params.setRequestedLang( LangUtility::getLanguageCode( tmpStr ) );
               delete [] tmpStr;
            } else { // Odd element in user element
               mc2log << warn << "XMLParserThread::readSearchSettings"
                         "odd Element in search_settings element: "
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         case DOMNode::TEXT_NODE :
            // Ignore stray texts
            break;
         default:
            mc2log << warn << "XMLParserThread::readSearchSettings"
                      "odd node type in search_settings element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   if ( categoryType != 0 ) {
      params.setSearchForTypes( categoryType );
   }
   if ( locationType != 0 ) {
      params.setSearchForLocationTypes( locationType );
   }
   if ( regionsInMatches != 0 ) {
      params.setRegionsInMatches( regionsInMatches );
   }
   if ( overviewRegionsInMatches != 0 ) {
      params.setRegionsInOverviewMatches( overviewRegionsInMatches );
   }

   return ok;
}

}
#endif // USE_XML

