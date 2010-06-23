/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include <algorithm>
#include "XMLParserThread.h"


#ifdef USE_XML
#include "SortDistRequest.h"
#include "UserFavoritesRequest.h"
#include "GfxUtility.h"
#include "ServerSubRouteVector.h"
#include "RouteRequest.h"
#include "UserData.h"
#include "MC2String.h"
#include "SearchReplyPacket.h"
#include "RoutePacket.h"
#include "RequestUserData.h"
#include "XMLServerElements.h"
#include "XMLSearchUtility.h"
#include "ParserDebitHandler.h"

namespace {

/**
 * Appends a sort_dist_item to cur.
 * @param cur Where to put the sort_dist_item node as 
 *            last child.
 * @param reply The document to make the node in.
 * @param indentLevel The indent to use.
 * @param indent If to use indent.
 * @param distance The distance in meters to the destination.
 * @param estimatedTime The estimated time to destination.
 *                      Not added if MAX_UINT32.
 * @param The element of the destination.
 */
void 
appendSortDistItem( DOMNode* cur, DOMDocument* reply,
                    int indentLevel, bool indent,
                    uint32 distance, uint32 estimatedTime,
                    DOMElement* requestElement );

}

class sortDistNotice {
   public:
      /** 
       * Constructs a new sortDistNotice.
       */
      sortDistNotice( int32 lat, int32 lon, uint32 index,
                      int32 originLat, int32 originLon ) 
      {
         m_sqdist = GfxUtility::squareP2Pdistance_linear( 
            lat, lon, originLat, originLon );
         m_index = index;
      }


      bool operator < ( const sortDistNotice& other ) const {
         return m_sqdist < other.m_sqdist;
      }


      inline uint32 getIndex() {
         return m_index;
      }

   
      inline float64 getSquareDist() {
         return m_sqdist;
      }


   private:
      float64 m_sqdist;
      uint32 m_index;
};


bool 
XMLParserThread::xmlParseSortDistRequest( DOMNode* cur, 
                                          DOMNode* out,
                                          DOMDocument* reply,
                                          bool indent )
{
   // The user is needed for routing.
   MC2_ASSERT( m_user->getUser() );
   bool ok = true;
   int indentLevel = 1;
   MC2String errorCode;
   MC2String errorMessage;

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );

   int childIndentLevel = indentLevel + 1;
   MC2String childIndentStr( childIndentLevel*3, ' ' );
   childIndentStr.insert( 0, "\n" );
   XStr XchildIndentStr( childIndentStr.c_str() );

   // Sort dist data
   uint32 maxNumberReplyItems = 1;
   XMLParserThreadConstants::sortDistType sortDistance = 
      XMLParserThreadConstants::RADIUS;
   RouteTypes::routeCostType routeCost = RouteTypes::TIME;
   XMLCommonEntities::coordinateType positionSystem =
      XMLCommonEntities::MC2;
   ItemTypes::vehicle_t routeVehicle = ItemTypes::passengerCar;
   bool originIsPositionItem = false;
   int32 originLat = MAX_INT32;
   int32 originLon = MAX_INT32;
   uint16 originAngle = MAX_UINT16;
   bool originIsSearchItem = false;
   VanillaMatch* originMatch = NULL;
   const char* originName = "";
   VanillaVector destVanillaVector;
   bool allFavorites = false;
   MC2String userID;
   MC2String userSessionID;
   MC2String userSessionKey;
   list<MC2String> warnings;
   UserItem* userItem = NULL;
   vector< const UserFavorite* > favoriteVector;
   UserFavoritesRequest* userFavReq = NULL;

   // Create sort_dist_reply element
   DOMElement* sort_dist_reply = reply->createElement( 
      X( "sort_dist_reply" ) );
   // Transaction ID
   sort_dist_reply->setAttribute( 
      X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
         X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( sort_dist_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( reply->createTextNode( XindentStr.XMLStr() ),
                         sort_dist_reply );
   }


   // Attributes
   DOMNamedNodeMap* attributes = cur->getAttributes();
   DOMNode* attribute;
   
   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(),
                              "max_number_reply_items" ) ) 
      {
         char* tmp = NULL;
         uint32 maxNbr = strtoul( tmpStr, &tmp, 10 );
         if ( tmp != tmpStr ) {
            maxNumberReplyItems = maxNbr;
         } else {
            mc2log << warn << "XMLParserThread::"
                      "xmlParseSortDistRequest "
                      "bad max_number_reply_items " 
                   << tmpStr << endl;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "sort_distance" ) ) 
      {
         sortDistance = XMLParserThreadConstants::stringToSortDistType(
            tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_cost" ) ) 
      {
         routeCost = RouteTypes::stringToRouteCostType( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "position_system" ) ) 
      {
         positionSystem = XMLCommonEntities::coordinateFormatFromString(
            tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_vehicle" ) ) 
      {
         routeVehicle = ItemTypes::getVehicleFromString( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "transaction_id" ) ) 
      {
         // Handled above 
      } else {
         mc2log << warn << "XMLParserThread::xmlParseSortDistRequest "
                   "unknown attribute for sort_dist_request "
                   "element "
                << " Name " << attribute->getNodeName()
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
                                    "position_item" ) ) 
            {
               if ( XMLCommonElements::getPositionItemData( 
                       child, originLat, originLon, originAngle,
                       errorCode, errorMessage ) ) 
               {
                  originIsPositionItem = true;
               } else {
                  mc2log << warn << "XMLParserThread::"
                            "xmlParseSortDistRequest failed to parse "
                            "position_item:"  << errorMessage << endl;
                  errorMessage.insert( 0, 
                                       "Failed parsing position_item: " );
                  warnings.push_back( errorMessage );
               }
            } else if ( XMLString::equals( child->getNodeName(),
                                           "search_item" ) ) 
            {
               originMatch = XMLSearchUtility::
                  getVanillaMatchFromSearchItem( child, 
                                                 errorCode, errorMessage );
               if ( originMatch == NULL ) {
                  mc2log << warn << "XMLParserThread::"
                            "xmlParseSortDistRequest failed to parse "
                            "search_item" << endl; 
                  errorMessage.insert( 0, "Failed parsing search_item: " );
                  warnings.push_back( errorMessage );
               } else {
                  originIsSearchItem = true;
                  originName = originMatch->getName();
               }
            } else if ( XMLString::equals( child->getNodeName(),
                                           "routeable_item_list" ) )
            {
               if ( !readAndPositionRouteableItemList( 
                       child, destVanillaVector,
                       errorCode, errorMessage ) )
               {
                  warnings.push_back( 
                     "Failed parsing routeable_item_list." );
                  mc2log << warn << "XMLParserThread::"
                            "xmlParseSortDistRequest failed to parse "
                            "routeable_item_list" << endl;    
               }
               warnings.push_back( 
                  "routeable_item_list not yet supported." );
               mc2log << warn << "XMLParserThread::"
                         "xmlParseSortDistRequest no support for "
                         "routeable_item_list." << endl;    
            } else if ( XMLString::equals( child->getNodeName(),
                                           "all_favorites" ) ) 
            {
               allFavorites = true;                  
               readAllFavorites( child, userID, 
                                 userSessionID, userSessionKey );
            } else {
               mc2log << warn << "XMLParserThread::"
                         "xmlParseSortDistRequest "
                         "odd Element in sort_dist_request element: " 
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                      "xmlParseSortDistRequest odd "
                      "node type in sort_dist_request  element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }
   

   // Get favorites
   if ( allFavorites ) {
      if ( (!userID.empty() && getUser( userID.c_str(), userItem, true)) ||
           (!userSessionID.empty() && 
            getUserBySession( userSessionID.c_str(), 
                              userSessionKey.c_str(), userItem, true ) ) ) 
      {
         if ( userItem != NULL ) {
            UserUser* user = userItem->getUser();
            // Check access
            if ( m_user->getUser()->getEditUserRights() ||
                 m_user->getUIN() == user->getUIN() ) 
            {
               userFavReq = new UserFavoritesRequest( 
                  getNextRequestID(), user->getUIN() );

               ThreadRequestContainer* reqCont = 
                  new ThreadRequestContainer( userFavReq );
               mc2dbg8 << "About to send UserFavoritesRequest" << endl;
               putRequest( reqCont );
               mc2dbg8 << "UserFavoritesRequest returned" << endl;

               if ( StringTable::stringCode( userFavReq->getStatus() ) == 
                    StringTable::OK ) 
               {
                  const UserFavorite* favorite = NULL;
                  // AddFav
                  favorite = userFavReq->getAddFav();
                  while ( favorite != NULL ) {
                     favoriteVector.push_back( favorite );
                     favorite = userFavReq->getAddFav();
                  }
                  if ( favoriteVector.size() == 0 ) {
                     warnings.push_back( "No favorites for user_id." );
                     mc2log << warn << "XMLParserThread::"
                               "xmlParseSortDistRequest No favorites "
                               "for user_id." << endl;  
                  }
               } else {
                  // Error
                  MC2String err = "Failed to get user's favorites: ";
                  err.append( StringTable::getString( 
                     StringTable::stringCode( 
                        userFavReq->getStatus() ), 
                     StringTable::ENGLISH ) );
                  warnings.push_back( err.c_str() );
                  mc2log << warn << "XMLParserThread::"
                        "xmlParseSortDistRequest UserFavoritesRequest "
                        "failed." << StringTable::getString( 
                           StringTable::stringCode( 
                              userFavReq->getStatus() ), 
                           StringTable::ENGLISH )
                         << endl;
               }
               
               delete reqCont;

            } else {
              warnings.push_back( "No access to user." );
              mc2log << warn << "XMLParserThread::"
                        "xmlParseSortDistRequest No access to user."
                     << endl; 
            }
         } else {
            warnings.push_back( "Unknown user." );
            mc2log << warn << "XMLParserThread::"
                      "xmlParseSortDistRequest Unknown user "
                   << endl;
         }
      } else {
         if ( userID.empty() && userSessionID.empty() ) {
           warnings.push_back( 
              "No data in all_favorites to get user from." );
            mc2log << warn << "XMLParserThread::xmlParseSortDistRequest "
                      "No data in all_favorites to get user from"
                   << endl;
         } else {
            warnings.push_back( 
               "Database connection error, please try again." );
            mc2log << warn << "XMLParserThread::"
                      "xmlParseSortDistRequest Database connection error, "
                      "failed to get user from all_favorites"
                   << endl;  
         }
      }
   }

   bool originSearchItemOk = originIsSearchItem && originMatch != NULL;
   // Position search_item
   if ( originSearchItemOk &&
        ( (destVanillaVector.size() > 0 && false /*NYS*/) || 
          ( allFavorites && favoriteVector.size() > 0 ) ) ) 
   {
      VanillaVector matches;
      Vector lats;
      Vector lons;
      bool hasLatLon = false;
      bool* hasLatLons = &hasLatLon;
      matches.push_back( originMatch );
      getCoordinatesForMatches( matches, lats, lons, hasLatLons );
      if ( hasLatLons[ 0 ] ) {
         originLat = lats[ 0 ];
         originLon = lons[ 0 ];
      } else {
         warnings.push_back( "Failed to position search_item." );
         mc2log << warn << "XMLParserThread::"
                   "xmlParseSortDistRequest failed to position "
                   "search_item "
                << endl;
         originSearchItemOk = false;
      }
   }

   using XMLServerUtility::appendStatusNodes;
   
   if ( ( (originIsPositionItem && originLat != MAX_INT32) ||
          (originSearchItemOk ) ) &&
        ( (destVanillaVector.size() > 0 && false /*NYS*/) || 
          ( allFavorites && favoriteVector.size() > 0 ) ) )
   {
      DOMElement* sort_dist_list = reply->createElement( 
         X( "sort_dist_list" ) );
      bool ok = true;
      
      if ( sortDistance == XMLParserThreadConstants::RADIUS ) {
         vector< sortDistNotice > sortDistVector;

         if ( destVanillaVector.size() > 0 ) {
            // Routeable items
            for ( uint32 i = 0 ; i < destVanillaVector.size() ; i++ ) {
//               sortDistVector;
            }
         } else { 
            // Favorites
            for ( uint32 i = 0 ; i < favoriteVector.size() ; i++ ) {
               const UserFavorite* favorite = favoriteVector[ i ];
               sortDistVector.push_back( sortDistNotice( 
                  favorite->getLat(), favorite->getLon(), i,
                  originLat, originLon ) );
            }
            sort( sortDistVector.begin(), sortDistVector.end() );
            
         }

         int32 bestItemLat = MAX_INT32;
         int32 bestItemLon = MAX_INT32;
         const char* bestItemName = "";
         uint32 bestSortDistance = 0;
         uint32 bestSortEstimatedTime = 0;
         uint32 numberReplyItems = maxNumberReplyItems;
         // Append sort_dist_items
         if ( destVanillaVector.size() > 0 ) {
            // TODO: Add here
            
         } else {
            for ( uint32 i = 0 ; 
                  i < maxNumberReplyItems && i < favoriteVector.size() ;
                  i++ )
            {
               
               DOMElement* favElement = makeUserFavoriteElement(
                favoriteVector[ sortDistVector[ i ].getIndex() ],
                positionSystem, false, reply, childIndentLevel + 2, indent );

               uint32 distance = 0 ;
               uint32 estimatedTime = MAX_UINT32;
               
               distance = uint32 ( rint( sqrt( 
                  sortDistVector[ i ].getSquareDist() ) ) );
               
               appendSortDistItem( sort_dist_list, reply, 
                                   childIndentLevel + 1, indent,
                                   distance, estimatedTime,
                                   favElement );
            }

            // Debit info
            if ( !favoriteVector.empty() ) {
               const UserFavorite* favorite = favoriteVector[ 
                  sortDistVector[ 0 ].getIndex() ];
               bestItemLat = favorite->getLat();
               bestItemLon = favorite->getLon();
               bestItemName = favorite->getName();
               bestSortDistance = uint32 ( rint( sqrt( 
                  sortDistVector[ 0 ].getSquareDist() ) ) );
            }
            numberReplyItems = 
               MIN( maxNumberReplyItems, favoriteVector.size() );
         }

         // Debit
         getDebitHandler()->makeSortDistDebit( 
            m_user, originName, originLat,
            originLon, sortDistance, routeCost,
            routeVehicle, numberReplyItems, 
            bestSortDistance, bestSortEstimatedTime,
            bestItemName, bestItemLat, bestItemLon );

         mc2log << info << "SortDist: Radius origcoord (" << originLat
                << "," << originLon << " \"" << originName << "\""
                << " nbrReplyItems " << numberReplyItems << " Closest: "
                << bestSortDistance << "m (" << bestItemLat << ","
                << bestItemLon << ")" << endl;


      } else { // Route
         uint32 expandType = 0;
         DEBUG1(expandType = (ROUTE_TYPE_STRING  |
                              ROUTE_TYPE_ITEM_STRING);
                expandType |= (ROUTE_TYPE_GFX |
                               ROUTE_TYPE_GFX_COORD););
         uint32 nbrDestinations = 0;
         if ( destVanillaVector.size() > 0 ) {
            nbrDestinations = destVanillaVector.size();
         } else {
            nbrDestinations = favoriteVector.size();
         }
         uint32 nbrWantedRoutes = MIN( maxNumberReplyItems, 
                                       nbrDestinations );
         RouteRequest* routeReq = new RouteRequest( m_user->getUser(),
                                                    getNextRequestID(),
                                                    expandType,
                                                    StringTable::ENGLISH,
                                                    false, 0, 
                                                    m_group->getTopRegionRequest(this),
                                                    NULL, NULL,
                                                    nbrWantedRoutes );
         routeReq->setRouteParameters( false,          // isStartTime
                                       routeCost,
                                       routeVehicle,
                                       0,             // Time
                                       0 );          // TurnCost
         
         // Origin
         routeReq->addOriginCoord( originLat, originLon, originAngle );
         
         // Destinations
         if ( destVanillaVector.size() > 0 ) {
            // TODO: Add here
            
         } else {
            for ( uint32 i = 0 ; i < favoriteVector.size() ; i++ ) {
               const UserFavorite* favorite = favoriteVector[ i ];
               int index = routeReq->addDestinationCoord(
                  favorite->getLat(), favorite->getLon() );
               mc2dbg8 << "Index for " << favorite->getName() << " is "
                       << index << endl;
            }
         }
         
         // Send route
         ThreadRequestContainer* reqCont = 
            new ThreadRequestContainer( routeReq );
         mc2dbg8 << "About to send RouteRequest" << endl;
         putRequest( reqCont );
         mc2dbg8 << "RouteRequest done" << endl;

         if ( routeReq->getStatus() == StringTable::OK ) {
            ServerSubRouteVectorVector* routes = routeReq->getRoute();

            int32 bestItemLat = MAX_INT32;
            int32 bestItemLon = MAX_INT32;
            const char* bestItemName = "";
            uint32 bestSortDistance = 0;
            uint32 bestSortEstimatedTime = 0;
            uint32 numberReplyItems = maxNumberReplyItems;

            if ( destVanillaVector.size() > 0 ) {
               // TODO: Add here
            
            } else {
               for ( uint32 i = 0 ; 
                     i < maxNumberReplyItems && 
                     i < favoriteVector.size() &&
                     i < routes->size() ;
                     i++ )
               {
                  SubRouteVector* route = (*routes)[ i ];
                  DOMElement* favElement = makeUserFavoriteElement(
                     favoriteVector[ route->getOriginalDestIndex() ],
                     positionSystem, false, reply, 
                     childIndentLevel + 2, indent );
                  mc2dbg8 << "Index " << route->getOriginalDestIndex() 
                          << " is " 
                          << favoriteVector[ 
                             route->getOriginalDestIndex() ]->getName() 
                          << endl;
                  uint32 distance = uint32( rint( 
                     route->getTotalDistanceCm() / 100.0 ) );
                  uint32 estimatedTime = route->getTotalTimeSec( routeCost
                     == RouteTypes::TIME_WITH_DISTURBANCES );
                  
                  appendSortDistItem( sort_dist_list, reply, 
                                      childIndentLevel + 1, indent,
                                      distance, estimatedTime,
                                      favElement );
               }

               // Debit info
               if ( !favoriteVector.empty() && routes->size() > 0 ) {
                  SubRouteVector* route = (*routes)[ 0 ];
                  const UserFavorite* favorite = favoriteVector[ 
                     route->getOriginalDestIndex() ];
                  bestItemLat = favorite->getLat();
                  bestItemLon = favorite->getLon();
                  bestItemName = favorite->getName();
                  bestSortDistance = uint32( rint( 
                     route->getTotalDistanceCm() / 100.0 ) );
                  bestSortEstimatedTime = route->getTotalTimeSec( 
                     routeCost == RouteTypes::TIME_WITH_DISTURBANCES );
               }
               numberReplyItems = 
                  MIN( maxNumberReplyItems, MIN( favoriteVector.size(),
                                                 routes->size() ) );
            }
            
            // Debit
            getDebitHandler()->makeSortDistDebit( 
               m_user, originName, originLat,
               originLon, sortDistance, routeCost,
               routeVehicle, numberReplyItems, 
               bestSortDistance, bestSortEstimatedTime,
               bestItemName, bestItemLat, bestItemLon );

            mc2log << info << "SortDist: Route origcoord (" << originLat
                   << "," << originLon << " \"" << originName << "\""
                   << " nbrReplyItems " << numberReplyItems << " Closest: "
                   << bestSortDistance << "m " << bestSortEstimatedTime 
                   << "s (" << bestItemLat << ","
                   << bestItemLon << ")" << endl;
            
         } else {
            ok = false;
            appendStatusNodes( 
               sort_dist_reply, reply, indentLevel + 1, indent,
               "-1", "Routing failed." ); 
            mc2log << warn << "XMLParserThread::xmlParseSortDistRequest "
                      "Routing failed." << endl;  
            routeReq->dumpState();
         }

         delete reqCont;
         delete routeReq;
      } // else Route

      if ( ok ) {
         if ( indent ) {
            // Add newline and indent before sort_dist_list endtag
            sort_dist_list->appendChild( 
               reply->createTextNode( XchildIndentStr.XMLStr() ) );
         }
         if ( indent ) {
            sort_dist_reply->appendChild( 
               reply->createTextNode( XchildIndentStr.XMLStr() ) );
         }
         sort_dist_reply->appendChild( sort_dist_list );
      } // Else status error already added

      
   } else {
      MC2String err = "";
      list<MC2String>::iterator it = warnings.begin();
      while ( it != warnings.end() ) {
         err.append( *it );
         ++it;
         if ( it != warnings.end() ) {
            err.append( "\r\n" );
         }
      }
      appendStatusNodes( 
         sort_dist_reply, reply, 
         indentLevel + 1, indent,
         "-1", err.c_str() );
   }


   if ( indent ) {
      // Newline and indent before end sort_dist_reply tag   
      sort_dist_reply->appendChild( 
         reply->createTextNode( XindentStr.XMLStr() ) );
   }

   delete originMatch;
   // Delete all in destVanillaVector
   for ( uint32 i = 0 ; i < destVanillaVector.size() ; i++ ) {
      delete destVanillaVector[ i ];
   }
   destVanillaVector.clear();

   delete userFavReq;
   releaseUserItem( userItem );

   return ok;
}


void 
XMLParserThread::readAllFavorites( DOMNode* cur, 
                                   MC2String& userID,
                                   MC2String& userSessionID,
                                   MC2String& userSessionKey )
{
   char* tmpStr = NULL;

   // Children
   DOMNode* favChild = cur->getFirstChild();

   while ( favChild != NULL ) {
      switch ( favChild->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( favChild->getNodeName(), "user_id" ) )
            {
               tmpStr = XMLUtility::getChildTextValue( favChild );
               userID = tmpStr; 
               delete [] tmpStr;
            } else if ( XMLString::equals( favChild->getNodeName(),
                                           "user_session_id" ) )
            {
               tmpStr = XMLUtility::getChildTextValue( favChild );
               userSessionID = tmpStr;
               delete [] tmpStr;
            } else if ( XMLString::equals( favChild->getNodeName(),
                                           "user_session_key" ) )
            {
               tmpStr = XMLUtility::getChildTextValue( favChild );
               userSessionKey = tmpStr;
               delete [] tmpStr;
            } else {
               mc2log << warn << "XMLParserThread::"
                         "readAllFavorites "
                         "odd Element in all_favorites element: " 
                      << favChild->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                      "readAllFavorites odd "
                      "node type in all_favorites element: " 
                   << favChild->getNodeName() 
                   << " type " << favChild->getNodeType() 
                   << endl;
            break;
      }
      favChild = favChild->getNextSibling();
   }
}

namespace {
void 
appendSortDistItem( DOMNode* cur, DOMDocument* reply,
                    int indentLevel, bool indent,
                    uint32 distance, uint32 estimatedTime,
                    DOMElement* requestElement ) {
   char tmp[20];
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );
   int childIndentLevel = indentLevel + 1;
   MC2String childIndentStr( childIndentLevel*3, ' ' );
   childIndentStr.insert( 0, "\n" );

   DOMElement* sort_dist_item = reply->createElement( 
      X( "sort_dist_item" ) );

   // distance
   sprintf( tmp, "%u", distance );
   sort_dist_item->setAttribute( X( "distance" ), X( tmp ) );
   if ( estimatedTime != MAX_UINT32 ) {
      // estimated_time
      sprintf( tmp, "%u", estimatedTime );
      sort_dist_item->setAttribute( X( "estimated_time" ), X( tmp ) );
   }

   if ( indent ) {
      sort_dist_item->appendChild( reply->createTextNode( 
                                      X( childIndentStr.c_str() ) ) );
   }
   sort_dist_item->appendChild( requestElement );

   if ( indent ) {
      cur->appendChild( reply->createTextNode( XindentStr.XMLStr() ) );
   }
   if ( indent ) {
      sort_dist_item->appendChild( reply->createTextNode( 
                                      XindentStr.XMLStr() ) );
   }
   cur->appendChild( sort_dist_item );
}

}

#endif // USE_XML

