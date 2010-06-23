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
#include "ItemInfoRequest.h"
#include "SearchReplyPacket.h"
#include "XMLServerElements.h"
#include "InfoTypeConverter.h"
#include "XMLSearchUtility.h"
#include "XMLCategoryListNode.h"
#include "GetSearchItem.h"
#include "SearchParserHandler.h"
#include "XMLTool.h"
#include "XMLItemInfoUtility.h"
#include "ExternalSearchConsts.h" 
#include "PersistentSearchIDs.h"
#include "SearchHeadingDesc.h"
#include "STLUtility.h"
#include "STLStringUtility.h"
#include "SearchResultRequest.h"
#include "GfxUtility.h"
#include "XMLItemDetailUtility.h"
#include <memory>

#ifdef USE_XML


void addSearchItemToInfoItem( DOMElement* infoItemNode, DOMDocument* reply,
                              uint32 i, ItemInfoRequest* r,
                              VanillaMatch* inputMatch,
                              XMLCommonEntities::coordinateType format,
                              LangTypes::language_t language, bool addCountry, 
                              int indentLevel, bool indent,
                              XMLParserThread* thread,
                              bool usePersistentIds ) {
   SearchRequestParameters params;
   if ( addCountry ) {
      params.setIncludeTopRegionInArea( true );
      params.setRegionsInMatches( params.getRegionsInMatches() | 
                                  SEARCH_COUNTRIES );
   }
   params.setRequestedLang( language );
   VanillaMatch* match = r->getMatches()[ i ];
   auto_ptr<SearchMatch> resultMatch;
   // If we have the expanded input in reply it doesn't have item/map id
   if ( r->getNbrReplyItems() == 1 ) {
      match = inputMatch;
   }
   // Get the full SearchMatch from SearchModule
   if ( GetSearchItem::request( *thread, match, language, resultMatch ) ) {
      if ( inputMatch->getExtSource() != ExternalSearchConsts::not_external ) {
         // External matches can't be looked up so take what we can from r
         resultMatch->setName( r->getReplyName( i ) );
      }
      if ( addCountry ) {
         thread->getSearchHandler().addCountryToResults( 
            params, resultMatch.get() );
      }
   }

   if ( resultMatch.get() != NULL ) {
      uint32 currentHeading = thread->getSearchHandler().getHeadingForMatch( 
         resultMatch.get() );
      DOMElement* element = 
         XMLSearchUtility::appendSearchItem( 
            infoItemNode, reply, static_cast<const VanillaMatch*> (
               resultMatch.get() ),
            ""/*currentLocation*/,
            indentLevel, indent,
            false/*latlonForSearchHits*/,
            resultMatch->getCoords().lat,
            resultMatch->getCoords().lon,
            false/*positionSearchItems*/,
            NULL/*bbox*/,
            format,
            params, usePersistentIds, thread );
      // Add image name
      MC2String imageName = thread->getSearchHandler().
         getImageName( *resultMatch, currentHeading );
      if ( ! imageName.empty() ) {
         XMLTool::addAttrib( element, "image", imageName );
      }
      // Add the heading to the info item
      XMLTool::addAttrib( infoItemNode, "heading", currentHeading );
   } // End if resultMatch is ok
}

struct PersistentIDData {
   PersistentIDData() : isPersistent( false ), 
                        headingID( SearchHeadingDesc::INVALID_HEADING_ID ) {}

   bool isPersistent;
   MC2Coordinate coord;
   MC2String name;
   uint32 headingID;
   MC2String itemIDStr;
};

// Append reviews and images to the result
void appendResources( DOMNode* rootNode, const SearchMatch* match, 
                      const SearchParserHandler& searchHandler) {
   const VanillaCompanyMatch* poiMatch = 
      dynamic_cast< const VanillaCompanyMatch* >( match );

   using namespace XMLTool;
   if ( poiMatch != NULL && ( !poiMatch->getReviews().empty() || 
                              !poiMatch->getImageURLs().empty() ) ) {
      const MC2String one( "1" );
      const MC2String zero( "0" );

      DOMElement* resourcesNode = addNode( rootNode, "resources" );

      // Add images group
      const VanillaCompanyMatch::ImageURLs& images = poiMatch->getImageURLs();
      addAttrib( resourcesNode, "number_image_groups", 
                 images.empty() ? zero : one );
      if ( !images.empty() ) {
         DOMElement* imagesNode = addNode( resourcesNode, "image_group" );
         addAttrib( imagesNode, "number_images", images.size() );
         addAttrib( imagesNode, "provider_name", 
                    searchHandler.getProviderName( *match ) );
         addAttrib( imagesNode, "provider_image", 
                    searchHandler.getProviderImageName( *match ) );
         
         // Add the image urls
         for ( VanillaCompanyMatch::ImageURLs::const_iterator it = images.begin();
               it != images.end(); ++it ) {
            DOMElement* imageNode = addNode( imagesNode, "image" );
            addAttrib( imageNode, "url", *it );            
         }
      }

      // Add review group
      const VanillaCompanyMatch::Reviews& reviews = poiMatch->getReviews();
      addAttrib( resourcesNode, "number_review_groups", 
                 reviews.empty() ? zero : one );
      if ( !reviews.empty() ) {
         DOMElement* reviewsNode = addNode( resourcesNode, "review_group" );
         addAttrib( reviewsNode, "number_reviews", reviews.size() );
         addAttrib( reviewsNode, "provider_name", 
                    searchHandler.getProviderName( *match ) );
         addAttrib( reviewsNode, "provider_image", 
                    searchHandler.getProviderImageName( *match ) );

         // Add the reviews
         for ( VanillaCompanyMatch::Reviews::const_iterator it = reviews.begin();
               it != reviews.end(); ++it ) {
            DOMElement* reviewNode = addNode( reviewsNode, "review", 
                                              it->getReviewText() );
            addAttrib( reviewNode, "rating", 
                       STLStringUtility::uint2str( it->getRating() ) ); 
            addAttrib( reviewNode, "date", it->getDate() ); 
            addAttrib( reviewNode, "reviewer", it->getReviewer() ); 
         }
            
      }
   }

}

bool 
XMLParserThread::xmlParsePOIInfoRequest( DOMNode* cur, 
                                         DOMNode* out,
                                         DOMDocument* reply,
                                         bool indent )
{
   bool ok = true;
   int indentLevel = 1;
   MC2String errorCode;
   MC2String errorMessage;
   PersistentIDData persistentIDData;

   XMLCommonEntities::coordinateType coordinateSystem =
      XMLCommonEntities::WGS84;

   LangTypes::language_t language = LangTypes::english;
   auto_ptr<VanillaMatch> match;

   // Create poi_info_reply element
   DOMElement* poi_info_reply = reply->createElement( 
      X( "poi_info_reply" ) );
   // Transaction ID
   poi_info_reply->setAttribute( 
      X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
         X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( poi_info_reply );

   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;
   // Whether to include category ids in the reply
   bool includeCategoryID = false;
   // If to add the full search item in the reply
   bool includeFullSearchItem = false;
   bool usePersistentIds = false;

   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(),
                              "position_system" ) ) 
      {
         coordinateSystem = XMLCommonEntities::coordinateFormatFromString( 
            tmpStr, XMLCommonEntities::WGS84 );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "transaction_id" ) ) 
      {
         // Handled above 
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "include_category_id" ) ) {
         includeCategoryID = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "include_full_search_item" ) ) {
         includeFullSearchItem = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "use_persistent_ids" ) ) {
         usePersistentIds = StringUtility::checkBoolean( tmpStr );
      } else {
         mc2log << warn << "XMLParserThread::xmlParsePOIInfoRequest "
                << "unknown attribute for poi_info_request element "
                << "Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }
   
   // Get children
   DOMNode* child = cur->getFirstChild();
   
   while ( child != NULL ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(),
                                    "search_item" ) ) 
            {
               MC2String itemIDStr = XMLUtility::getChildTextStr( 
                  *static_cast<const DOMElement*>( child )->
                  getElementsByTagName( X( "itemid" ) )->item( 0 ) );
               if ( PersistentSearchIDs::isPersistentID( itemIDStr ) ) {
                  persistentIDData.itemIDStr = itemIDStr;
                  if ( ! PersistentSearchIDs::getMatchData( 
                          itemIDStr, getSearchHandler(), 
                          persistentIDData.coord, persistentIDData.name, 
                          persistentIDData.headingID ) ) {
                     mc2log << warn << "XMLParserThread::"
                            << "xmlParsePOIInfoRequest Bad persistent Id: "
                            << MC2CITE( itemIDStr ) << endl;
                     persistentIDData.isPersistent = false;
                  } else {
                     persistentIDData.isPersistent = true;
                  }
               } else {
                  match.reset( XMLSearchUtility::
                               getVanillaMatchFromSearchItem( child, 
                                                              errorCode,
                                                              errorMessage ) );
               }
            } else if ( XMLString::equals( child->getNodeName(),
                                           "language" ) ) 
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               language = getStringAsLanguage( tmpStr );
               delete [] tmpStr;
            } else {
               mc2log << warn << "XMLParserThread::xmlParsePOIInfoRequest "
                      << "odd Element in poi_info_request element: " 
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::xmlParsePOIInfoRequest "
                   << "odd node type in poi_info_request element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }
   

   using XMLServerUtility::appendStatusNodes;

   // Handle Request
   if ( persistentIDData.isPersistent ) {
      mc2dbg << "Is persistent " << endl;
      // Search in the heading for the name at the coordinate
      CompactSearch params;
      // Setup the parameters
      params.m_heading = persistentIDData.headingID;
      params.m_what = persistentIDData.name;
      params.m_location.m_coord = persistentIDData.coord;
      params.m_language = language;
      params.m_smartCategory = false; // No i like things working
      // This seems like it isn't needed, just heading //  params.m_round = ;

      typedef STLUtility::AutoContainerMap< 
         SearchParserHandler::SearchResults > SearchResults;

      // Make the search
      SearchResults results( getSearchHandler().compactSearch( params ) );

      // Find the good result, should only be one heading in the reply.
      if ( ! results.empty() && 
           !(*results.begin()).second->
               getSearchResultRequest()->getMatches().empty() ) { 
         // Set match, hope that the search did good and the best is first
         //   match.reset( (*results.begin()).second->
         //                getMatches().front()->clone() );
         // External searches doesn't do good so we have to do it here
         const vector<VanillaMatch*>& matches = (*results.begin()).second->
               getSearchResultRequest()->getMatches();
         float64 minDist = MAX_FLOAT64;
         size_t bestIndex = 0;
         // For all results find the closest one
         // TODO: Make a function of this
         for ( size_t i = 0 ; i < matches.size() ; ++i ) {
            float64 dist = GfxUtility::squareP2Pdistance_linear( 
               persistentIDData.coord, matches[ i ]->getCoords() );
            if ( dist < minDist ) {
               bestIndex = i;
               minDist = dist;
            }
         }
         match.reset( matches[ bestIndex ]->clone() );
      } else {
         // No result
         mc2log << warn << "XMLParserThread::xmlParsePOIInfoRequest "
                << "No match found for the persistent id: "
                << MC2CITE( persistentIDData.itemIDStr ) 
                << " Faking one from the input." << endl;
         // Faking one using the PersistentIDData
         uint32 searchType = getSearchHandler().getSearchTypeForHeading( 
            persistentIDData.headingID );
         uint32 round = getSearchHandler().getRoundForHeading( 
            persistentIDData.headingID );
         if ( (searchType & SEARCH_COMPANIES) != 0 || round != 0/*Ext*/ ) {
            match.reset( new VanillaCompanyMatch( 
                            IDPair_t(), persistentIDData.name.c_str(), 
                            ""/*locationName*/, 0/*offset*/, 
                            0/*streetNumber*/ ) );
            if ( round != 0/*Ext*/ ) {
               MC2String extID;
               uint32 source = getSearchHandler().getExternalSourceForHeading( 
                  persistentIDData.headingID, extID );
               match->setExtSourceAndID( source, extID );
            }
            // Categories
            set<uint32> categories;
            categories.insert( 288/*Other from poi_category_tree*/ );
            static_cast< VanillaCompanyMatch* >( match.get() )->setCategories(
               categories );
         } else {
            match.reset( new VanillaStreetMatch( 
                            IDPair_t(), persistentIDData.name.c_str(), 
                            ""/*locationName*/, 0/*offset*/, 
                            0/*streetNumber*/ ) );
         }
         // Coord
         match->setCoords( persistentIDData.coord );
      }
   }

      
   if ( match.get() != NULL ) {
      // POI info it!
      auto_ptr<ItemInfoRequest> 
         req( new ItemInfoRequest( getNextRequestID(),
                                   m_group->getTopRegionRequest( this ) ) );

      if ( req->setItem( *match, language ) == false ) {
         mc2log << warn << "XMLParserThread::xmlParsePOIInfoRequest "
                << "Not id nor coords valid for search_item." << endl;
         appendStatusNodes( poi_info_reply, reply, indentLevel + 1, indent,
                            "-1", 
                            "Not id nor coords valid for search_item." );
      }

      mc2dbg << "About to send ItemInfoRequest" << endl;
      putRequest( req.get() );
      mc2dbg << "ItemInfoRequest returned" << endl;

      if ( req->replyDataOk() ) {
         for ( uint32 i = 0 ; i < req->getNbrReplyItems() ; i++ ) {
            const VanillaMatch* infoMatch = req->getMatches()[ i ];
            if ( req->getMatches()[ i ]->getName()[ 0 ] == '\0' ) {
               // Use the input
               infoMatch = match.get();
            } 
            DOMElement* infoItem = XMLItemInfoUtility::appendInfoItem( 
               poi_info_reply, reply, infoMatch, 
               coordinateSystem,
               includeCategoryID,
               indentLevel + 1, false/*indentPiece later*/, this );
            if ( includeFullSearchItem ) {
               addSearchItemToInfoItem( infoItem, reply, i, req.get(),
                                        match.get(), coordinateSystem, 
                                        language, true/*addCointry*/,
                                        indentLevel + 1, 
                                        false/*indentPiece later*/, this,
                                        usePersistentIds );
            } // End if to add search_item
         }
         mc2log << info << "POIInfo OK " << req->getNbrReplyItems()
                << " infos" << endl;
      } else {
         // Database connection error
         const char* errStr = "Connection failed to database.";
         appendStatusNodes( poi_info_reply, reply, indentLevel + 1, indent,
                            "-1", errStr );
         mc2log << info << "POIInfo Error: " << errStr << endl;
      }

   } else {
      // Error search_item error
      appendStatusNodes( poi_info_reply, reply, indentLevel + 1, indent,
                         "-1",
                         "Error search_item not understood." );
      mc2log << info << "POIInfo Error: search_item not understood" 
             << endl;
   }

   if ( indent ) {
      XMLUtility::indentPiece( *poi_info_reply, 1 );
   }

   return ok;
}


bool 
XMLParserThread::xmlParsePOIDetailRequest( DOMNode* cur, 
                                           DOMNode* out,
                                           DOMDocument* reply,
                                           bool indent ) 
try {
   LangTypes::language_t language = LangTypes::english;
   auto_ptr<SearchMatch> match;
   MC2String errorCode;
   MC2String errorMessage;
   int indentLevel = 1;

   // Create poi_detail_reply element
   DOMElement* poi_detail_reply =
      XMLUtility::createStandardReply( *reply, *cur, "poi_detail_reply" );

   out->appendChild( poi_detail_reply );
   
   // Attributes
   MC2String languageStr;
   XMLTool::getAttrib( languageStr, "language", cur, MC2String("eng") );
   language = getStringAsLanguage( languageStr.c_str() );
   
   // Get cild elements
   DOMNode* itemElem = XMLTool::findNode( cur, "itemid" );
   match.reset( XMLSearchUtility::getVanillaMatchFromItemId( itemElem ) );
   
   using XMLServerUtility::appendStatusNodes;
   
   if ( match.get() != NULL ) {
      // POI info it!
      auto_ptr<ItemInfoRequest> 
         req( new ItemInfoRequest( getNextRequestID(),
                                   m_group->getTopRegionRequest( this ) ) );
      
      if ( req->setItem( *match, language ) == false ) {
         mc2log << warn << "[XMLParserThread]::xmlParsePOIDetailRequest "
                << "Not id nor coords valid for search_item." << endl;
         appendStatusNodes( poi_detail_reply, reply, indentLevel + 1, indent,
                            "-1", 
                            "Not id nor coords valid for search_item." );
      } else {
      
         // Put the request
         putRequest( req.get() );
         
         if ( req->replyDataOk() ) {
            for ( uint32 i = 0 ; i < req->getNbrReplyItems() ; i++ ) {
               SearchMatch* infoMatch = req->getMatches()[ i ];
               if ( req->getMatches()[ i ]->getName()[ 0 ] == '\0' ) {
                  // Use the input
                  infoMatch = match.get();
               } 
               infoMatch->mergeToSaneItemInfos( language, 
                   getSearchHandler().getTopRegionForMatch( infoMatch ) );
               
               XMLItemDetailUtility::appendDetailItem( poi_detail_reply, 
                                                       infoMatch);

               // Add reviews and images
               appendResources( poi_detail_reply, infoMatch, getSearchHandler() );
            }
            mc2log << info << "[XMLParserThread]::xmlParsePOIDetailRequest "
               "POIInfo OK " << req->getNbrReplyItems() << " infos" << endl;
         } else {
            // Database connection error
            const char* errStr = "Connection failed to database.";
            appendStatusNodes( poi_detail_reply, reply, indentLevel + 1, indent,
                               "-1", errStr );
            mc2log << info << "POIDetail Error: " << errStr << endl;
         }
      }
   } else {
      // Error search_item error
      appendStatusNodes( poi_detail_reply, reply, indentLevel + 1, indent,
                         "-1",
                         "Error search_match not understood." );
      mc2log << info << "POIDetail Error: search_match not understood" 
             << endl;
   }

   if ( indent ) {
      XMLUtility::indentPiece( *poi_detail_reply, 1 );
   }

   return true;
} catch ( const XMLTool::Exception& e ) {
   mc2log << error << "[POIDetailRequest]  " << e.what() << endl;
   XMLServerUtility::appendStatusNodes( out->getLastChild(), reply, 1, false,
                                        "-1", e.what() );
   return true;
}

#endif // USE_XML

