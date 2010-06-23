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
#include "SearchPacket.h"
#include "UserData.h"
#include "ExpandRequestData.h"
#include "ExpandRequest.h"
#include "SearchReplyData.h"
#include "XMLSearchUtility.h"
#include "XMLServerElements.h"

namespace {

/**
 * Makes an OverviewMatch from data.
 *
 * @param mapID The mapID for the OverviewMatch.
 * @param itemID The itemID for the OverviewMatch.
 * @param name The name for the OverviewMatch.
 * @param itemType The itemType for the OverviewMatch.
 * @return A OverviewMatch or NULL if not suppored itemType.
 */
OverviewMatch* 
makeOverviewMatchFromData( uint32 mapID, uint32 itemID,
                           const char* name,
                           uint32 itemType ) {

   OverviewMatch* match = NULL;
   OverviewMatch* standaloneMatch = NULL;
 
   if ( (itemType & SEARCH_ALL_REGION_TYPES) ) {
      OverviewMatch* tmpMatch = 
         new OverviewMatch( IDPair_t(mapID, itemID) );

      tmpMatch->setItemID( itemID );
      tmpMatch->setName0( name );
      tmpMatch->setType( itemType );

      match = tmpMatch;
      // copy string arrays
      standaloneMatch = 
         new OverviewMatch( *static_cast<OverviewMatch*> ( match ) );
   } else {
      mc2log << warn << "XMLParserThread::makeOverviewMatchFromData "
                "unknown itemType " << itemType << endl
             << "   name " << name << " mapID.itemID " << mapID << "."
             << itemID << endl;
   } 

   delete match;

   return standaloneMatch;
}

}

bool 
XMLParserThread::xmlParseExpandRequest( DOMNode* cur, 
                                        DOMNode* out,
                                        DOMDocument* reply,
                                        bool indent )
{
   bool ok = true;
   int indentLevel = 1;

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );

   SearchRequestParameters params;
   XMLCommonEntities::coordinateType positionSystem =
      XMLCommonEntities::MC2;

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
   params.setAddStreetNamesToCompanies( false );
   params.setTryHarder( true );
   bool locationNameOnlyCountryCity = false;

   MC2String errorCode;
   MC2String errorMessage;

   // Create expand_reply element
   DOMElement* expand_reply = reply->createElement( X( "expand_reply" ) );
   // Transaction ID
   expand_reply->setAttribute( 
      X( "transaction_id" ),
      cur->getAttributes()->getNamedItem( X( "transaction_id" ) )
      ->getNodeValue() );
   out->appendChild( expand_reply );
   if ( indent ) {
      // Newline and indent for expand_reply start tag
      out->insertBefore( 
         reply->createTextNode( X( indentStr.c_str() ) ), expand_reply );
   }

   DOMElement* expand_request = static_cast< DOMElement* >( cur );
   DOMNodeList* headers = expand_request->getElementsByTagName( 
      X( "expand_request_header" ) );

   mc2dbg8 << "headers->getLength() " << headers->getLength() << endl;

   // Read settings
   ok = xmlParseExpandRequestExpandRequestHeader( 
      headers->item( 0 ),
      expand_reply,
      reply,
      indentLevel + 1,
      params,
      positionSystem, locationNameOnlyCountryCity,
      errorCode, errorMessage );

   using XMLServerUtility::appendStatusNodes;

   if ( ok ) {
      // handle the expand_query
      DOMNodeList* queries = expand_request->getElementsByTagName( 
         X( "expand_request_query" ) );
      
      ok = xmlParseExpandRequestExpandQuery(
         queries->item(0),
         expand_reply,
         reply,
         indentLevel + 1,
         indent,
         params,
         positionSystem,
         locationNameOnlyCountryCity );
      
      if ( !ok ) {
         appendStatusNodes( expand_reply, reply, indentLevel + 1, indent,
                            "-1",
                            "Expansion failed." );  
         // Error handled continue
         ok = true;
      }
   } else {
      // The error is in errorCode, errorMessage
      appendStatusNodes( expand_reply, reply, indentLevel + 1, indent,
                         errorCode.c_str(), errorMessage.c_str() );
      ok = true;
   }
   
   if ( indent ) {
      // Newline and indent before end expand_reply tag   
      expand_reply->appendChild( 
         reply->createTextNode( X( indentStr.c_str() ) ) );
   }

   return ok;
}


bool 
XMLParserThread::xmlParseExpandRequestExpandRequestHeader( 
   DOMNode* cur, 
   DOMNode* out,
   DOMDocument* reply,
   int indentLevel,
   SearchRequestParameters& params,
   XMLCommonEntities::coordinateType& positionSystem,
   bool& locationNameOnlyCountryCity,
   MC2String& errorCode, MC2String& errorMessage )
{
   mc2dbg4 << "XMLParserThread::"
           << "xmlParseExpandRequestExpandRequestHeader" << endl;
   bool ok = true;

   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;

   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(),
                              "position_system" ) ) 
      {
         positionSystem = XMLCommonEntities::coordinateFormatFromString( 
            tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "include_top_region_id" ) ) {
         params.setIncludeTopRegionInArea( StringUtility::
                                           checkBoolean( tmpStr ) );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "location_name" ) ) {
         // all_possible|country_city
         locationNameOnlyCountryCity = MC2String( "country_city" ) == tmpStr;
      } else {
         mc2log << warn << "XMLParserThread::"
                   "xmlParseExpandRequestExpandRequestHeader "
                   "unknown attribute Name " << attribute->getNodeName()
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
            } else { // Odd element in user element
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseExpandRequestExpandRequestHeader "
                      << "odd Element in expand_request_header"
                      << " element: " << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseExpandRequestExpandRequestHeader "
                   << "odd node type in epand_request_header element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   return ok;
}


bool 
XMLParserThread::xmlParseExpandRequestExpandQuery(
   DOMNode* cur, 
   DOMNode* out,
   DOMDocument* reply,
   int indentLevel,
   bool indent,
   const SearchRequestParameters& params,
   XMLCommonEntities::coordinateType positionSystem,
   bool locationNameOnlyCountryCity )
{
   bool ok = false;
   
   mc2dbg4 << "XMLParserThread::xmlParseExpandRequestExpandQuery" << endl;
   VanillaVector matches;
   vector<OverviewMatch*> overviewMatches;
         
   // Get the area query or item
   bool hasArea = false;
   // If search_area
   uint32 areaMapID = 0;
   uint32 areaMaskID = 0;
   uint32 areaItemID = 0;
   char* areaName = NULL;
   uint32 areaType = 0;
   // search_item data
   bool hasItem = false;
   uint32 itemMapID = 0;
   uint16 itemOffset = 0;
   uint32 itemItemID = 0;
   char* itemName = NULL;
   uint32 itemType = 0;
   uint32 itemStreetNbr = 0;

   vector<bool> hasLats;
   vector<int32> lats;
   vector<int32> lons;
   vector<MC2BoundingBox*> bboxes;
   MC2BoundingBox bbox;
   MC2String errorCode;
   MC2String errorMessage;


   // If result is in matches
   bool expanded = false;
   // If error has been printed
   bool errorPrinted = false;

   // Go throu children and get data.
   DOMNode* child = cur->getFirstChild();

   using XMLServerUtility::appendStatusNodes;

   while ( child != NULL ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            if ( XMLString::equals( child->getNodeName(),
                                    "search_area" ) ) 
            {
               // Get search_area data, 
               hasArea = XMLSearchUtility::
                  getSearchAreaData( child,
                                     areaMapID,
                                     areaItemID,
                                     areaMaskID,
                                     areaName,
                                     areaType,
                                     errorCode,
                                     errorMessage );

            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_item" ) ) 
            {
               // Get search_item data, 
               // can't return false as the node name is right
               hasItem = XMLSearchUtility::
                  getSearchItemData( child,
                                     itemMapID,
                                     itemItemID,
                                     itemOffset,
                                     itemName,
                                     itemStreetNbr,
                                     itemType,
                                     errorCode,
                                     errorMessage );
            } else if ( XMLString::equals( child->getNodeName(),
                                           "position_item" ) ) 
            {
               // Position position_item 
               VanillaMatch* match = NULL;
               match = getVanillaMatchFromPositionItem( 
                  child,
                  errorCode, 
                  errorMessage,
                  params.getRequestedLang(),
                  locationNameOnlyCountryCity );
               expanded = true;
               if ( match != NULL ) {
                  matches.push_back( match );
                  ok = true;
                  hasItem = true;
               } else {
                  // Failed
                  errorMessage.insert( 
                     0, "Expansion of position_item failed: " );
                  mc2log << warn << "XMLParserThread::"
                            "xmlParseExpandRequestExpandQuery "
                         << errorMessage << endl;
                  XMLTreeFormatter::printTree( 
                     child, mc2log << warn << "Item: " );
                  mc2log << endl;
                  appendStatusNodes( out, reply, indentLevel, indent,
                                     errorCode.c_str(), 
                                     errorMessage.c_str() );
                  errorPrinted = true;
                  ok = false;
               }
            } else {
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseExpandRequestExpandQuery "
                      << "expand_request_query node has unknown name. "
                      << "DTDs must be wrong!"
                      << "NodeName: " << child->getNodeName() << endl;
            } 
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseExpandRequestExpandQuery odd "
                   << "node type in expand_request_query element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType()<< endl;
            break;
      }
      child = child->getNextSibling();
   }
   
   
   if ( !expanded ) {
      // Expand
      if ( hasArea && hasItem && itemType == SEARCH_CATEGORIES ) {
         ok = expandCategory( matches,
                              itemMapID,
                              itemItemID,
                              params,
                              false ); // Don't expand categories with one 
                                      //  category in it.
         if ( !ok ) {
            // Failed
            appendStatusNodes( out, reply, indentLevel, indent,
                               "-1",
                               "Expansion of search_item failed." );
            errorPrinted = true;
            mc2log << info << "Expand: Error Expansion of search_item "
                   << "failed" << endl;
         }
      } else if ( (hasItem && itemType != SEARCH_CATEGORIES) ||
                  (hasArea && !hasItem) ) 
      {
         // Get boundingbox for item/area
         CoordinateOnItemRequest* req = new CoordinateOnItemRequest( 
            getNextRequestID(), true );

         uint32 mapID = 0;
         uint32 itemID = 0;
         uint16 offset = 0;
         const char* name = NULL;
         uint32 streetNbr = 0;
         uint32 type = 0;

         if ( hasItem ) {
            mapID = itemMapID;
            itemID = itemItemID;
            offset = itemOffset;
            name = itemName;
            streetNbr = itemStreetNbr;
            type = itemType;
         } else {
            mapID = areaMapID;
            itemID = areaItemID;
            offset = MAX_UINT16 / 2;
            name = areaName;
            streetNbr = 0;
            type = areaType;
         }

         req->add( mapID, itemID, offset );
         putRequest( req );
         
         int32 lat = 0;
         int32 lon = 0;
         bool hasLat = false;

         req->getResultWithBoundingbox( 0, mapID, itemID, offset, 
                                        lat, lon, hasLat, bbox );
         if ( hasLat ) {
            if ( hasItem ) {
               VanillaMatch* match = XMLSearchUtility::
                  makeVanillaMatchFromData( mapID, itemID, 
                                            offset, name, "",
                                            streetNbr, type );

               matches.push_back( match );
            } else {
               OverviewMatch* match = 
                  ::makeOverviewMatchFromData( mapID, itemID, 
                                               name, type );

               overviewMatches.push_back( match );
            }
            hasLats.push_back( hasLat );
            lats.push_back( lat );
            lons.push_back( lon );
            bboxes.push_back( &bbox );

            ok = true;
         } else {
            mc2log << warn << "XMLParserThread::"
                      "xmlParseExpandRequestExpandQuery "
                      "Localization of search_item failed."
                   << " itemMapID "<< itemMapID << " itemItemID " 
                   << itemItemID << " itemOffset " << itemOffset << endl;
            // Failed
            appendStatusNodes( out, reply, indentLevel, indent,
                               "-1",
                               "Localization of search_item failed." );
            errorPrinted = true;  
            ok = false;
         }
         
         delete req;
      } else {
         // Unsupported
         appendStatusNodes( out, reply, indentLevel, indent,
                            "-1",
                            "Expansion of the input "
                            "is not supported." );
         errorPrinted = true;
         ok = false;
         mc2log << info << "Expand: Error Expansion of the input "
                << "is not supported" << endl;
      }
   }


   // Print the answer
   if ( ok ) {
      MC2String indentStr( indentLevel*3, ' ' );
      indentStr.insert( 0, "\n" );

      // We did not handle the normal params before,
      // so in order to handle it in the same way; we are only
      // copying the "new" "include top region"-parameter for now.

      SearchRequestParameters newParams;
      newParams.
         setIncludeTopRegionInArea( params.shouldIncludeTopRegionInArea() );
      newParams.setRequestedLang( params.getRequestedLang() );
      

      if ( hasItem ) {
         DOMElement* search_item_list = reply->createElement( 
            X( "search_item_list" ) );
      
         char ctmp[30];
         sprintf( ctmp, "%lu", static_cast<unsigned long>( matches.size() ) );
         search_item_list->setAttribute( X( "numberitems" ), X( ctmp ) );
         if ( indent ) {
            out->appendChild( reply->createTextNode( 
                                 X( indentStr.c_str() ) ) );
         }
         out->appendChild( search_item_list );

         for ( uint32 i = 0 ; i < matches.size() ; i++ ) {
            // Print search_item
            XMLSearchUtility::
               appendSearchItem( search_item_list,
                                 reply,
                                 matches[ i ],
                                 areaName ? areaName : "",
                                 indentLevel + 1, indent,
                                 false, 
                                 i < lats.size() ? lats[ i ] : MAX_INT32,
                                 i < lons.size() ? lons[ i ] : MAX_INT32,
                                 i < hasLats.size() ? hasLats[ i ] : false,
                                 i < bboxes.size() ? bboxes[ i ] : NULL,
                                 positionSystem,
                                 newParams );

            delete matches[ i ]; // Don't use index i again!
         }
         if ( matches.size() > 0 ) {
            if ( indent ) {
               // Add newline and indent before search_item_list endtag
               search_item_list->appendChild( 
                  reply->createTextNode( X( indentStr.c_str() ) ) );
            }
         }
         mc2log << info << "Expand: OK " << matches.size() << " matches"
                << endl;
      } else {
         DOMElement* search_area_list = reply->createElement( 
            X( "search_area_list" ) );
      
         char ctmp[30];
         sprintf( ctmp, "%lu", 
                  static_cast<unsigned long>( overviewMatches.size() ) );
         search_area_list->setAttribute( X( "numberitems" ), X( ctmp ) );
         if ( indent ) {
            out->appendChild( reply->createTextNode( 
                                 X( indentStr.c_str() ) ) );
         }
         out->appendChild( search_area_list );
      
         for ( uint32 i = 0 ; i < overviewMatches.size() ; i++ ) {
            // Print search_area
            XMLSearchUtility::
               appendSearchArea( search_area_list,
                                 reply,
                                 overviewMatches[ i ],
                                 indentLevel + 1, indent,
                                 i < hasLats.size() ? hasLats[ i ] : false,
                                 i < lats.size() ? lats[ i ] : MAX_INT32,
                                 i < lons.size() ? lons[ i ] : MAX_INT32,
                                 i < bboxes.size() ? bboxes[ i ] : NULL,
                                 positionSystem,
                                 newParams );

            delete overviewMatches[ i ]; // Don't use index i again!
         }
         if ( overviewMatches.size() > 0 ) {
            if ( indent ) {
               // Add newline and indent before search_area_list endtag
               search_area_list->appendChild( 
                  reply->createTextNode( X( indentStr.c_str() ) ) );
            }
         }
         mc2log << info << "Expand: OK " << overviewMatches.size()
                << " overviewMatches" << endl;
      }

      
   }

   if ( errorPrinted ) {
      // Error handled continue
      ok = true;
   }

   return ok;
}


bool 
XMLParserThread::expandCategory( VanillaVector& matches,
                                 uint32 itemMapID,
                                 uint32 itemItemID,
                                 const SearchRequestParameters& params,
                                 bool expandUnique )
{
   bool ok = false;

#if 0   
   set<ItemTypes::itemType> itemTypes;
   ExpandRequestData reqData( IDPair_t( itemMapID, itemItemID ),
                              params.getRequestedLang(),
                              itemTypes );
   
   ExpandRequest* req = new ExpandRequest( getNextRequestID(),
                                           reqData );
   
   mc2dbg << "[XMLParserThread]: About to send ExpandRequest." << endl;
   putRequest( req );
   mc2dbg << "[XMLParserThread]: ExpandRequest returned." << endl;

   if( req->getStatus() == StringTable::OK ) {
      const vector<VanillaMatch*>& reqMatches = req->getMatches();
      vector<VanillaMatch*>::const_iterator it;
      for(it = reqMatches.begin(); it != reqMatches.end(); ++it) {
         matches.push_back( (*it)->clone() );
      }
      ok = true;
   }
   delete req;

#endif

   return ok;
}


#endif // USE_XML

