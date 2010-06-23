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
#include "RouteRequest.h"
#include "ExpandedRoute.h"
#include "ExpandStringItem.h"
#include "ExpandItemID.h"
#include "HttpUtility.h"
#include "ExpandedRoute.h"
#include "Name.h"
#include "UserData.h"
#include "HttpInterfaceRequest.h"
#include "RouteID.h"
#include "SearchReplyPacket.h"
#include "RoutePacket.h"
#include "RequestUserData.h"
#include "StringConversion.h"
#include "XMLAuthData.h"
#include "PreCacheTileMapHandler.h"
#include "TileMapTypes.h"
#include "XMLSearchUtility.h"
#include "XMLServerUtility.h"
#include "XMLServerElements.h"
#include "XMLTool.h"
#include "ParserRouteHandler.h"
#include "RouteRequestParams.h"
#include "Utility.h"
#include "StringTableUtility.h"
#include "ParserDebitHandler.h"
#include "TopRegionRequest.h"
#include "PurchaseOptions.h"

#include <set>

class XMLLastRouteData {
public:
   XMLLastRouteData( bool controlledAccess,
                     bool ramp, bool driveOnRightSide )
         : controlledAccess( controlledAccess ), ramp( ramp ),
           driveOnRightSide( driveOnRightSide )
      {
      }

   bool controlledAccess;
   bool ramp;
   bool driveOnRightSide;
};

namespace {

/**
 * Appends a route_landmark_item as child to curr.
 * 
 * @param curr Where to put the route_landmark_item node as last 
 *             child.
 * @param reply The document to make the nodes in.
 * @param landMark The landmark to append.
 * @param itemIndex The index of the route item that this landmark
 *                  belongs to.
 * @param i The index of the landmark in the route item.
 * @param route The expanded route.
 * @param language The prefered language.
 * @param routeTurnData If landmark data should be added to item.
 * @param indentLevel The indent to use.
 * @param indent If indent.
 */
bool appendLandmarkItem( DOMNode* cur, DOMDocument* reply,
                          const ExpandedRouteLandmarkItem* landmark, 
                          uint32 itemIndex, uint32 i,
                          const ExpandedRoute* route,
                          LangTypes::language_t language,
                          bool routeTurnData,
                          int indentLevel, bool indent ) {
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );
   int childIndentLevel = indentLevel + 1;
   MC2String childIndent = indentStr;
   childIndent.append( 3, ' ' );
   char description[4096];
   char tmpStr[512];
   bool ok = true;


   DOMElement* roadLandmarkNode = reply->createElement( 
      X( "route_landmark_item" ) );

   // at_turn
   roadLandmarkNode->setAttribute( 
      X( "at_turn" ), 
      X( StringUtility::booleanAsString( landmark->getAtTurn() ) ) );
   if(landmark->isTraffic()){
      if(landmark->isDetour()){
         roadLandmarkNode->setAttribute( 
            X("is_detour"), X( StringUtility::booleanAsString(true) ) );
      }
      if(landmark->isStart()){
         roadLandmarkNode->setAttribute( 
            X( "is_start" ), X( StringUtility::booleanAsString( true )));
      }
      if(landmark->isStop()){
         roadLandmarkNode->setAttribute(
            X( "is_stop" ), X( StringUtility::booleanAsString(true )));
      }
   }
   
   
   // Description
   route->getRouteLandmarkDescription( itemIndex, i, description, 4096 );

   using XMLServerUtility::appendElementWithText;

   appendElementWithText( roadLandmarkNode, reply, "description", 
                          description,
                          childIndentLevel, indent );

   if ( routeTurnData ) {
      // Add landmark data
    
      // road_side
      appendElementWithText( roadLandmarkNode, reply, "road_side", 
                             StringConversion::sideTypeToString(
                                landmark->getRoadSide() ),
                             childIndentLevel, indent );

      // landmarklocation_type
      appendElementWithText( roadLandmarkNode, reply, 
                             "landmarklocation_type", 
                             StringConversion::landmarkLocationTypeToString(
                                landmark->getLocation() ),
                             childIndentLevel, indent );

      // landmark_type
      appendElementWithText( roadLandmarkNode, reply, "landmark_type", 
                             StringConversion::landmarkTypeToString(
                                landmark->getType() ),
                             childIndentLevel, indent );


      // distance
      sprintf( tmpStr, "%d", landmark->getDist() );
      appendElementWithText( roadLandmarkNode, reply, "distance", 
                             tmpStr,
                             childIndentLevel, indent );

      // name
      appendElementWithText( roadLandmarkNode, reply, "name", 
                             landmark->getName()->getBestName( 
                                language )->getName(),
                             childIndentLevel, indent );
      
   }




   // Add roadLandmarkNode to cur
   if ( indent ) {
      cur->appendChild( reply->createTextNode( XindentStr.XMLStr() ) );
   }
   if ( indent ) {
      // Newline and indent before end route_landmark_item tag   
      roadLandmarkNode->appendChild( reply->createTextNode( 
                                        XindentStr.XMLStr() ) );
   }
   cur->appendChild( roadLandmarkNode );

   return ok;
}
/**
 * Appends a route_road_item as child to curr.
 * 
 * @param curr Where to put the route_road_item node as last child.
 * @param reply The document to make the nodes in.
 * @param format The coordinate format to use.
 * @param roadItem The road item to append.
 * @param lastData The last sent attributes of route.
 * @param indentLevel The indent to use.
 * @param indent If indent.
 */
bool 
appendRoadItem( DOMNode* cur, DOMDocument* reply,
                XMLCommonEntities::coordinateType format,
                const ExpandedRouteRoadItem* roadItem,
                XMLLastRouteData& lastData,
                int indentLevel, bool indent ) {
   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );
   int childIndentLevel = indentLevel + 1;
   char target[512];
   bool ok = true;

   DOMElement* roadItemNode = reply->createElement( 
      X( "route_road_item" ) );
   // speedLimit
   if ( roadItem->getPosSpeedLimit() == MAX_UINT32 ) {
      sprintf( target, "%d", 0 );
   } else {
      sprintf( target, "%d", roadItem->getPosSpeedLimit() );
   }
   roadItemNode->setAttribute( X( "speedLimit" ), X( target ) );

   // TODO: Add more attributes when they are available in the route, 
   //       not just dummy
   // Road Class

   // Road name (name node)

   // Positive speed limit

   // Negative speed limit

   // Multidigitialized

   // ControlledAccess
   bool controlledAccess = roadItem->getControlledAccess();
   if ( controlledAccess != lastData.controlledAccess ) {
      roadItemNode->setAttribute( X( "controlled_access" ), 
                                  X( StringUtility::booleanAsString(
                                        controlledAccess ) ) );
      lastData.controlledAccess = controlledAccess;
   }

   // Ramp
   bool ramp = roadItem->getRamp();
   if ( ramp != lastData.ramp ) {
      roadItemNode->setAttribute( X( "ramp" ), 
                                  X( StringUtility::booleanAsString(
                                        ramp ) ) );
      lastData.ramp = ramp;
   }

   // Roundabout
   // TODO: Not set in Module yet, activate when usefull
//    bool roundabout = roadItem->getRoundabout();
//    if ( roundabout != lastData.roundabout ) {
//       roadItemNode->setAttribute( 
//          X( "roundabout" ), 
//          X( StringUtility::booleanAsString( roundabout ) ) );
//       lastData.roundabout = roundabout;
//    }

   // drive_on_right_side
   bool driveOnRightSide = roadItem->getDriveOnRightSide();
   if ( driveOnRightSide != lastData.driveOnRightSide ) {
      roadItemNode->setAttribute( X( "drive_on_right_side" ), 
                                  X( StringUtility::booleanAsString(
                                        driveOnRightSide ) ) );
      lastData.driveOnRightSide = driveOnRightSide;
   }

   // Is the turn
   roadItemNode->setAttribute( X( "is_turn" ), 
                               X( StringUtility::booleanAsString(
                                     roadItem->getTurn() ) ) );

   // Start level

   // End level

   // PosEntryrestrictions

   // NegEntryrestrictions

   // Coordinates
   using XMLServerUtility::appendElementWithText;
   for ( uint32 i = 0 ; i < roadItem->getNbrCoordinates() ; ++i ) {
      appendElementWithText( roadItemNode, reply, 
                             "lat", XMLCommonEntities::coordinateToString(
                                target, roadItem->getCoordinate( i ).lat,
                                format, true ),
                             childIndentLevel, indent );
      appendElementWithText( roadItemNode, reply, 
                             "lon", XMLCommonEntities::coordinateToString(
                                target, roadItem->getCoordinate( i ).lon,
                                format, false ),
                             childIndentLevel, false ); // No indent
   } 

   // Add roadItemNode to cur
   if ( indent ) {
      cur->appendChild( reply->createTextNode( XindentStr.XMLStr() ) );
   }
   if ( indent ) {
      // Newline and indent before end route_road_item tag   
      roadItemNode->appendChild( reply->createTextNode( 
                                    XindentStr.XMLStr() ) );
   }
   cur->appendChild( roadItemNode );

   return ok;
}


/**
 * Takes a stringItem and makes appends a route_reply_item with
 * the data to list.
 * @param list The route_reply_items to append the route_reply_item
 *             to.
 * @param reply The document to make the nodes in.
 * @param routeItem The ExpandedRouteItem to make node for.
 * @param indentLevel The indent to use.
 * @param indent If indentation should be used.
 * @param itemIndex The index of this item.
 * @param route The ExpandedRoute with start data.
 * @param language The prefered language.
 * @param routeImageLinks If there should be a link to an image 
 *                        over the turn.
 * @param routeTurnData If extra elements with turn data should be
 *                      added.
 * @param routeRoadData If extra elements with road data should be
 *                      added.
 * @param routeLandmarks If extra elements with landmark data should
 *                       be added.
 * @param routeTurnImageWidth The set maximum width of the turn 
 *                            image.
 * @param routeTurnImageHeight The set maximum height of the turn 
 *                             image.
 * @param routeImageFormat The default image format.
 * @param lastData The last sent attributes of route.
 */
void 
appendRouteItem( DOMNode* list,
                 DOMDocument* reply,
                 const ExpandedRouteItem* routeItem,
                 int indentLevel,
                 bool indent,
                 uint32 itemIndex,
                 const ExpandedRoute* route,
                 StringTable::languageCode language,
                 bool routeImageLinks,
                 bool routeTurnData,
                 bool routeRoadData,
                 bool routeLandmarks,
                 uint32 routeTurnImageWidth, uint32 routeTurnImageHeight,
                 ImageDrawConfig::imageFormat routeImageFormat,
                 MapSettingsTypes::defaultMapSetting routeImageDisplayType,
                 struct MapSettingsTypes::ImageSettings& imageSettings,
                 bool routeTurnBoundingbox,
                 XMLCommonEntities::coordinateType bboxCoordinateSystem,
                 const MC2BoundingBox& turnBoundingbox,
                 XMLLastRouteData& lastData,
                 const HttpHeader* inHead ) {

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );
   int childIndentLevel = indentLevel + 1;
   MC2String childIndent = indentStr;
   childIndent.append( 3, ' ' );
   XStr XchildIndent( childIndent.c_str() );
   
   DOMElement* stringNode = reply->createElement( 
      X( "route_reply_item" ) );

   // Description
   uint32 maxRouteDescLen = 4096;
   char desc[ maxRouteDescLen + 1 ];
   route->getRouteDescription( itemIndex, desc, maxRouteDescLen );
   using XMLServerUtility::appendElementWithText;
   appendElementWithText( stringNode, reply, 
                          "description", desc,
                          childIndentLevel, indent );
   
   if ( routeTurnData ) {
      // turn
      appendElementWithText( stringNode, reply, 
                             "turn", 
                             XMLServerUtility::
                             routeturnTypeToString( routeItem->getTurnType(),
                                                    routeItem->isEndOfRoad() ),
                             childIndentLevel, indent );

      // distance and time if not start
      if ( routeItem->getTurnType() != ExpandedRouteItem::START ) {
         // distance
         char dist[20];
         sprintf( dist, "%u", routeItem->getDist() );
         appendElementWithText( stringNode, reply, 
                                "distance", dist, 
                                childIndentLevel, indent );

         // time
         char time[20];
         sprintf( time, "%u", routeItem->getTime() );
         appendElementWithText( stringNode, reply, 
                                "time", time, 
                                childIndentLevel, indent );
      }

      // roadname
      appendElementWithText( stringNode, reply, 
                             "roadname", 
                             routeItem->getIntoRoadName()->getBestName(
                                ItemTypes::getLanguageCodeAsLanguageType( 
                                   language ) )->getName(),
                             childIndentLevel, indent );

      if ( routeItem->getTurnType() != ExpandedRouteItem::START &&
           routeItem->getTurnType() != ExpandedRouteItem::FINALLY )
      {
         // ExitCount
         char exitc[20];
         if ( routeItem->getTurnNumber() > /*9*/15 ) { 
            // TODO: Invalid only if overviewmap but I can't check that as 
            // ExpandItemID only has non overview mapIDs.
            strcpy( exitc, "inf" );
         } else {
            sprintf( exitc, "%u", routeItem->getTurnNumber() );
         }
         appendElementWithText( stringNode, reply, 
                                "exitcount", exitc, 
                                childIndentLevel, indent );
      }

      if ( ! routeItem->getSignPosts().empty() ) {
         MC2String text;
         MC2String exitNbr;

         for ( ExpandStringSignPosts::const_iterator it = 
                  routeItem->getSignPosts().begin() ; 
               it != routeItem->getSignPosts().end() ; ++it ) {
            if ( (*it).getElementType() == ExpandStringSignPost::exit ) {
               if ( !exitNbr.empty() ) {
                  exitNbr += " ";
               }
               exitNbr = (*it).getText();
            } else {
               if ( !text.empty() ) {
                  text += " ";
               }
               text = (*it).getText();
            }
         }

         // signposttext
         appendElementWithText( stringNode, reply, 
                                "signposttext", 
                                text.c_str(), 
                                childIndentLevel, indent );
         
         // SignPostExitNbr
         appendElementWithText( stringNode, reply, 
                                "signpostexitnbr", 
                                exitNbr.c_str(), 
                                childIndentLevel, indent );

         // SignPostRouteNbr
         appendElementWithText( stringNode, reply, 
                                "signpostroutenbr", 
                                "",
                                childIndentLevel, indent );
      }

      // start_dir if start
      if ( routeItem->getTurnType() == ExpandedRouteItem::START ) {
         appendElementWithText( stringNode, reply, 
                                "start_dir", StringTable::getString( 
                                   Utility::getCompass( 
                                      route->getOrigin( 0 )->getAngle(),
                                      3 ),
                                   StringTable::ENGLISH ),
                                childIndentLevel, indent );

         // Starthousenumberdirection
         const char* housenumberStartDir = "";
         if ( route->getOrigin( 0 )->getDirectionOddEven() != 
              ItemTypes::unknown_oddeven_t )
         {
            housenumberStartDir = 
               StringConversion::housenumberStartToString(
                  ItemTypes::getStartDirectionOddEvenSC(
                     route->getOrigin( 0 )->getDirectionOddEven() ) );
         } else {
            housenumberStartDir = 
               StringConversion::housenumberStartToString(
                  ItemTypes::getStartDirectionHousenumberSC(
                     route->getOrigin( 0 )->getDirectionHousenumber() ));
         }
         appendElementWithText( stringNode, reply, 
                                "route_housenumber_start_direction", 
                                housenumberStartDir, 
                                childIndentLevel, indent );
      }

      // TransportationType
      appendElementWithText( stringNode, reply, 
                             "transporation_type", 
                             ExpandStringItem::transportationTypeToString( 
                                routeItem->getTransportation() ),
                             childIndentLevel, indent );

      if ( routeItem->getTurnType() != ExpandedRouteItem::START &&
           routeItem->getTurnType() != ExpandedRouteItem::FINALLY ) 
      {
         // crossing_type
         appendElementWithText( stringNode, reply, 
                                "crossing_type", 
                                StringConversion::crossingKindToString( 
                                   routeItem->getCrossingType() ),
                                childIndentLevel, indent );
      }

      if ( routeItem->getNbrExpandedRouteRoadItems() > 0 ) {
         // controlled_access   %bool; #IMPLIED
         bool controlledAccess = routeItem->getExpandedRouteRoadItem( 0 )
            ->getControlledAccess();
         if ( controlledAccess != lastData.controlledAccess ) {
            stringNode->setAttribute( 
               X( "controlled_access" ), X( StringUtility::booleanAsString(
                                               controlledAccess ) ) );
            lastData.controlledAccess = controlledAccess;
         }
         // ramp                %bool; #IMPLIED
         bool ramp = routeItem->getExpandedRouteRoadItem( 0 )->getRamp();
         if ( ramp != lastData.ramp ) {
            stringNode->setAttribute( 
               X( "ramp" ), X( StringUtility::booleanAsString( ramp ) ) );
            lastData.ramp = ramp;
         }
         // roundabout          %bool; #IMPLIED
         // TODO: Not set in Module yet, activate when usefull
//          bool roundabout = routeItem->getExpandedRouteRoadItem( 0 )
//             ->getRoundabout();
//          if ( roundabout != lastData.roundabout ) {
//             stringNode->setAttribute( 
//                X( "roundabout" ), X( StringUtility::booleanAsString(
//                 roundabout ) ) );
//             lastData.roundabout = roundabout;
//          }
         // drive_on_right_side
         bool driveOnRightSide = routeItem->getExpandedRouteRoadItem( 0 )
            ->getDriveOnRightSide();
         if ( driveOnRightSide != lastData.driveOnRightSide ) {
            stringNode->setAttribute( 
               X( "drive_on_right_side" ), 
               X( StringUtility::booleanAsString( driveOnRightSide ) ) );
            lastData.driveOnRightSide = driveOnRightSide;
         }
      }

   } // End route turn data

   if ( routeImageLinks ) {
      char url[ 4096 ];
      char widthStr[ 11 ];
      char heightStr[ 11 ];
      uint16 width = routeTurnImageWidth;
      uint16 height = routeTurnImageHeight;
            
      HttpUtility::makeMapURL( 
         url, widthStr, heightStr,
         inHead,
         route->getRouteID(),
         route->getRouteCreateTime(),
         MAX_UINT32, MAX_UINT32, // No feature index
         itemIndex, // Turn index
         width, height, 
         routeItem->getTurnBoundingbox().getMinLat(),
         routeItem->getTurnBoundingbox().getMinLon(),
         routeItem->getTurnBoundingbox().getMaxLat(),
         routeItem->getTurnBoundingbox().getMaxLon(),
         routeImageFormat,
         routeImageDisplayType,
         &imageSettings );

      // route_turn_link
      if ( indent ) {
         stringNode->appendChild( reply->createTextNode(
                                     XchildIndent.XMLStr() ) );
      }
      DOMElement* route_turn_link = reply->createElement( 
         X( "route_turn_link" ) );
      route_turn_link->appendChild( reply->createCDATASection(
                                       X( url ) ) );
      stringNode->appendChild( route_turn_link );

      // route_turn_width
      appendElementWithText( stringNode, reply, 
                             "route_turn_width", widthStr,
                             childIndentLevel, indent );

      // route_turn_height
      appendElementWithText( stringNode, reply, 
                             "route_turn_height", heightStr,
                             childIndentLevel, indent );
   }

   if ( routeTurnBoundingbox ) {
      XMLSearchUtility::
         appendBoundingbox( stringNode,
                         reply,
                         bboxCoordinateSystem,
                         routeItem->getTurnBoundingbox().getMaxLat(),
                         routeItem->getTurnBoundingbox().getMinLon(),
                         routeItem->getTurnBoundingbox().getMinLat(),
                         routeItem->getTurnBoundingbox().getMaxLon(),
                         indentLevel + 1, indent );

      // Also add the exact turning point (Were to say exit)
      XMLSearchUtility::
         appendPositionItem( routeItem->getLat(), routeItem->getLon(), 
                             MAX_UINT16,
                             bboxCoordinateSystem, stringNode, reply,
                             indentLevel + 1, indent );
   }

   if ( routeRoadData ) {
      // route_road_item*
      for ( uint32 j = 0 ; j < routeItem->getNbrExpandedRouteRoadItems() ;
               ++j )
      {
         ::appendRoadItem( stringNode,
                           reply,
                           bboxCoordinateSystem,
                           routeItem->getExpandedRouteRoadItem( j ),
                           lastData,
                           indentLevel + 1, indent );
      }
   }

   // Landmark
   if ( routeLandmarks ) {
      // route_landmark_item*
      for ( uint32 j = 0 ; 
            j < routeItem->getNbrExpandedRouteLandmarkItems() ; ++j )
      {
         appendLandmarkItem( stringNode,
                             reply,
                             routeItem->getExpandedRouteLandmarkItem( j ), 
                             itemIndex, j, route,
                             ItemTypes::getLanguageCodeAsLanguageType( 
                                language ),
                             routeTurnData, indentLevel + 1, indent );
      }
   }

   // Add stringNode to list
   if ( indent ) {
      list->appendChild( reply->createTextNode( XindentStr.XMLStr() ) );
   }
   list->appendChild( stringNode );
   if ( indent ) {
      stringNode->appendChild( reply->createTextNode( 
                                  XindentStr.XMLStr() ) );
   }
}

}

bool 
XMLParserThread::xmlParseRouteRequest( DOMNode* cur, 
                                       DOMNode* out,
                                       DOMDocument* reply,
                                       bool indent )
{
   bool ok = false;
   int indentLevel = 1;

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );

   // Route settings
   byte routeCostA = 0;
   byte routeCostB = 0;
   byte routeCostC = 0;
   byte routeCostD = 0;
   bool avoidtollroad = false;
   bool avoidhighway = false;
   ItemTypes::vehicle_t routeVehicle = ItemTypes::passengerCar;
   StringTableUtility::distanceFormat distFormat = StringTableUtility::NORMAL;
   StringTableUtility::distanceUnit routeMeasurement = StringTableUtility::METERS;
   StringTable::languageCode language = StringTable::ENGLISH;
   bool routeImageLinks = false;
   bool routeTurnData = false;
   bool routeRoadData = false;
   bool routeItems = true;
   bool abbreviateRouteNames = true;
   bool routeLandmarks = false;
   uint32 routeOverviewImageWidth = 100;
   uint32 routeOverviewImageHeight = 100;
   uint32 routeTurnImageWidth = 100;
   uint32 routeTurnImageHeight = 100;
   ImageDrawConfig::imageFormat routeImageFormat = ImageDrawConfig::PNG;
   MapSettingsTypes::defaultMapSetting routeImageDisplayType = 
      MapSettingsTypes::MAP_SETTING_STD;
   struct MapSettingsTypes::ImageSettings imageSettings;
   XMLCommonEntities::coordinateType bboxCoordinateSystem =
      XMLCommonEntities::MC2;
   bool routeTurnBoundingbox = false;
   RouteID previous_route_id;
   MC2String reason( "user_request" );

   imageSettings.reset();

   // Create route_reply element
   DOMElement* route_reply = 
      reply->createElement( X( "route_reply" ) );
   // Transaction ID
   route_reply->setAttribute( 
      X( "transaction_id" ), cur->getAttributes()->getNamedItem( 
         X( "transaction_id" ) )->getNodeValue() );
   out->appendChild( route_reply );
   if ( indent ) {
      // Newline
      out->insertBefore( 
         reply->createTextNode( XindentStr.XMLStr() ), route_reply );
   }

   // Read and handle Route Request
   // Find the two routeable_item_lists and put into function 
   // xmlParseRouteRequest...

   DOMNode* origin_routeable_item_list = NULL;
   DOMNode* destination_routeable_item_list = NULL;

   // Go throu children and get data.
   DOMNode* child = cur->getFirstChild();
      
   ok = true;
   while ( child != NULL && ok) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            if ( XMLString::equals( child->getNodeName(),
                                    "routeable_item_list" ) ) 
            {
               if ( origin_routeable_item_list == NULL ) {
                  origin_routeable_item_list = child;
               } else if (destination_routeable_item_list == NULL ) {
                  destination_routeable_item_list = child;
               } else {
                  mc2log << warn << "XMLParserThread::xmlParseRouteRequest"
                         << " more than two routeable_item_list in "
                         << "route_request! Skipping extra lists." << endl;
               }
            } else if ( XMLString::equals( child->getNodeName(),
                                           "route_request_header" ) )
            {
               // Read settings
               ok = xmlParseRouteRequestRouteRequestHeader( 
                  child,
                  route_reply,
                  reply,
                  indentLevel + 1,
                  routeCostA,
                  routeCostB,
                  routeCostC,
                  routeCostD,
                  avoidtollroad,
                  avoidhighway,
                  routeVehicle,
                  distFormat,
                  routeMeasurement,
                  language,
                  routeImageLinks,
                  routeTurnData,
                  routeRoadData,
                  routeItems,
                  abbreviateRouteNames,
                  routeLandmarks,
                  routeOverviewImageWidth,
                  routeOverviewImageHeight,
                  routeTurnImageWidth,
                  routeTurnImageHeight,
                  routeImageFormat,
                  routeImageDisplayType,
                  imageSettings,
                  bboxCoordinateSystem,
                  routeTurnBoundingbox,
                  previous_route_id,
                  reason );
            } else {
               mc2log << warn << "XMLParserThread::xmlParseRouteRequest "
                      << "route_request child node has unknown name. "
                      << "NodeName: " << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::xmlParseRouteRequest odd "
                   << "node type in route_request element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   // Now set at least costnad b
   if ( routeCostA == 0 &&
        routeCostB == 0 &&
        routeCostC == 0 ) {
      routeCostB = 1;
   }

   using XMLServerUtility::appendStatusNodes;

   if ( ok ) {
      // Make the route and print or print error in reply->
      ok = xmlHandleRouteRequest( origin_routeable_item_list,
                                  destination_routeable_item_list,
                                  route_reply,
                                  reply,
                                  indentLevel +1,
                                  routeCostA,
                                  routeCostB,
                                  routeCostC,
                                  routeCostD,
                                  avoidtollroad,
                                  avoidhighway,
                                  routeVehicle,
                                  distFormat,
                                  routeMeasurement,
                                  language,
                                  routeImageLinks,
                                  routeTurnData,
                                  routeRoadData,
                                  routeItems,
                                  abbreviateRouteNames,
                                  routeLandmarks,
                                  routeOverviewImageWidth,
                                  routeOverviewImageHeight,
                                  routeTurnImageWidth,
                                  routeTurnImageHeight,
                                  routeImageFormat,
                                  routeImageDisplayType,
                                  imageSettings,
                                  bboxCoordinateSystem,
                                  routeTurnBoundingbox,
                                  previous_route_id,
                                  reason,
                                  indent );
      if ( !ok ) {
         route_reply->setAttribute( X( "route_id" ),
                                    X( RouteID(0,0).toString().c_str() ) );
         appendStatusNodes( route_reply, reply, indentLevel + 1, indent,
                            "-1",
                            "Error making route." );
         ok = true;
      }
   } else {
      // The only possible error is in getUser.      
      route_reply->setAttribute( X( "route_id" ),
                                 X( RouteID(0,0).toString().c_str() ) );
      appendStatusNodes( route_reply, reply, indentLevel + 1, indent,
                         "-1",
                         "Database error retreiving user information." );
      ok = true;
   }
   
   if ( indent ) {
      // Newline and indent before end route_reply tag   
      route_reply->appendChild( reply->createTextNode( 
                                   XindentStr.XMLStr() ) );
   }


   return ok;
}


bool 
XMLParserThread::xmlParseRouteRequestRouteRequestHeader( 
   DOMNode* cur, 
   DOMNode* out,
   DOMDocument* reply,
   int indentLevel,
   byte& routeCostA,
   byte& routeCostB,
   byte& routeCostC,
   byte& routeCostD,
   bool& avoidtollroad,
   bool& avoidhighway,
   ItemTypes::vehicle_t& routeVehicle,
   StringTableUtility::distanceFormat& distFormat,
   StringTableUtility::distanceUnit& routeMeasurement,
   StringTable::languageCode& language,
   bool& routeImageLinks,
   bool& routeTurnData,
   bool& routeRoadData,
   bool& routeItems,
   bool& abbreviateRouteNames,
   bool& routeLandmarks,
   uint32& routeOverviewImageWidth,
   uint32& routeOverviewImageHeight,
   uint32& routeTurnImageWidth,
   uint32& routeTurnImageHeight,
   ImageDrawConfig::imageFormat& routeImageFormat,
   MapSettingsTypes::defaultMapSetting& routeImageDisplayType,
   struct MapSettingsTypes::ImageSettings& imageSettings,
   XMLCommonEntities::coordinateType& bboxCoordinateSystem,
   bool& routeTurnBoundingbox,
   RouteID& previous_route_id,
   MC2String& reason )
{
   bool ok = false;

   // Attributes
   XMLTool::getAttribValue( reason, "reroute_reason", cur );
   MC2String oldRouteID;
   XMLTool::getAttribValue( oldRouteID, "previous_route_id", cur );
   previous_route_id = RouteID( oldRouteID.c_str() );

   // Go throu children and handle route_preferences
   DOMNode* child = cur->getFirstChild();
      
   ok = true;
   while ( child != NULL && ok) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            if ( XMLString::equals( child->getNodeName(),
                                    "route_preferences" ) ) 
            {
               ok = 
                  xmlParseRouteRequestRouteRequestHeaderRoutePreferences(
                     child,
                     out,
                     reply,
                     indentLevel,
                     routeCostA,
                     routeCostB,
                     routeCostC,
                     routeCostD,
                     avoidtollroad,
                     avoidhighway,
                     routeVehicle,
                     distFormat,
                     routeMeasurement,
                     language,
                     routeImageLinks,
                     routeTurnData,
                     routeRoadData,
                     routeItems,
                     abbreviateRouteNames,
                     routeLandmarks,
                     routeOverviewImageWidth,
                     routeOverviewImageHeight,
                     routeTurnImageWidth,
                     routeTurnImageHeight,
                     routeImageFormat,
                     routeImageDisplayType,
                     imageSettings,
                     bboxCoordinateSystem,
                     routeTurnBoundingbox );
            } else {
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseRouteRequestRouteRequestHeader "
                      << "route_preferences child node has unknown name. "
                      << "NodeName: " << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseRouteRequestRouteRequestHeader odd "
                   << "node type in route_preferences element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType() << endl;
            break;
      }
      child = child->getNextSibling();
   }

   return ok;
}


bool 
XMLParserThread::xmlHandleRouteRequest( 
   DOMNode* origin_list,
   DOMNode* destination_list,
   DOMElement* out,
   DOMDocument* reply,
   int indentLevel,
   byte routeCostA,
   byte routeCostB,
   byte routeCostC,
   byte routeCostD,
   bool& avoidtollroad,
   bool& avoidhighway,
   ItemTypes::vehicle_t routeVehicle,
   StringTableUtility::distanceFormat distFormat,
   StringTableUtility::distanceUnit routeMeasurement,
   StringTable::languageCode language,
   bool routeImageLinks,
   bool routeTurnData,
   bool routeRoadData,
   bool routeItems,
   bool abbreviateRouteNames,
   bool routeLandmarks,
   uint32 routeOverviewImageWidth,
   uint32 routeOverviewImageHeight,
   uint32 routeTurnImageWidth,
   uint32 routeTurnImageHeight,
   ImageDrawConfig::imageFormat routeImageFormat,
   MapSettingsTypes::defaultMapSetting routeImageDisplayType,
   struct MapSettingsTypes::ImageSettings& imageSettings,
   XMLCommonEntities::coordinateType bboxCoordinateSystem,
   bool routeTurnBoundingbox,
   RouteID previous_route_id,
   MC2String reason,
   bool indent )
{
   MC2_ASSERT( m_user->getUser() );
   
   bool ok = false;

   MC2String indentStr( indentLevel*3, ' ' );
   indentStr.insert( 0, "\n" );
   XStr XindentStr( indentStr.c_str() );
   
   // Expand lists to routeable items
   VanillaVector originMatches;
   VanillaVector destinationMatches;
   MC2String errorCode;
   MC2String errorMessage;
   uint32 uturncost = 0;

   using XMLServerUtility::appendStatusNodes;

   // Check if may route (Has access to service WF but maybe not route)
   if ( !checkIfIronUserMay( m_user->getUser(), 
                             m_authData->clientSetting, 
                             UserEnums::UR_ROUTE ) )
   {
      mc2log << info << "XMLParserThread::xmlHandleRouteRequest: "
             << "User has no route access." << endl;
      errorCode = "-209";
      errorMessage = "Routing not allowed.";

      appendStatusNodes( out, reply, indentLevel, indent,
                         errorCode.c_str(), errorMessage.c_str(),
                         "94209" /* 0x17001 */ );
      ok = true; // Status printed
      // Set route_id in route_reply
      DOMElement* route_reply = static_cast< DOMElement* >( out );
      route_reply->setAttribute( X( "route_id" ),
                                 X( RouteID(0,0).toString().c_str() ) );
      return ok;
   }

   if ( !readAndPositionRouteableItemList( origin_list, originMatches, 
                                           errorCode, errorMessage ) )
   {
      MC2String errStr = "Problem with route origin: ";
      errStr.append( errorMessage );

      appendStatusNodes( out, reply, indentLevel, indent,
                         errorCode.c_str(), errStr.c_str() );
      ok = true; // Status printed
      // Set route_id in route_reply
      DOMElement* route_reply = static_cast< DOMElement* >( out );
      route_reply->setAttribute( X( "route_id" ),
                                 X( RouteID(0,0).toString().c_str() ) );
      
      // Delete all in originMatches
      for ( uint32 i = 0 ; i < originMatches.size() ; i++ ) {
         delete originMatches[ i ];
      }
      return ok;
   }

   if ( !readAndPositionRouteableItemList( destination_list, 
                                           destinationMatches, 
                                           errorCode, errorMessage ) ) 
   {
      MC2String errStr = "Problem with route destination: ";
      errStr.append( errorMessage );

      appendStatusNodes( out, reply, indentLevel, indent,
                         errorCode.c_str(), errStr.c_str() );
      ok = true; // Status printed
      // Set route_id in route_reply
      DOMElement* route_reply = static_cast< DOMElement* >( out );      
      route_reply->setAttribute( X( "route_id" ),
                                 X( RouteID(0,0).toString().c_str() ) );

      // Delete all in originMatches
      for ( uint32 i = 0 ; i < originMatches.size() ; i++ ) {
         delete originMatches[ i ];
      }
      // Delete all in destinationMatches
      for ( uint32 i = 0 ; i < destinationMatches.size() ; i++ ) {
         delete destinationMatches[ i ];
      }
      return ok;
   }


   uint32 expandType = (ROUTE_TYPE_STRING  |
                        ROUTE_TYPE_ITEM_STRING);
   expandType |= (ROUTE_TYPE_GFX |
                  ROUTE_TYPE_GFX_COORD);
   if ( routeRoadData || true ) { 
      // This is needed to get the attributes of the turns (driveOnRight)
      expandType |= ROUTE_TYPE_ALL_COORDINATES;
      // Can not have both navigator and gfx coord
      expandType &= ~uint32(ROUTE_TYPE_GFX_COORD);
   }

   RouteRequest* req = new RouteRequest( m_user->getUser(),
      getNextRequestID(), 
      expandType, 
      language,
      false, 1,
      m_group->getTopRegionRequest(this));

   RouteAllowedMap* maps = NULL;
   if ( getMapIdsForUserRegionAccess( m_user->getUser(), maps, 
                                      m_authData->urmask ) )
   {
      mc2dbg2 << "getMapIdsForUserRegionAccess returned " << maps << endl;
      req->setAllowedMaps( maps );
   } else {
      errorCode = "-3";
      errorMessage = "Problem when checking Allowed User Region Access.";

      appendStatusNodes( out, reply, indentLevel, indent,
                         errorCode.c_str(), errorMessage.c_str() );
      // Set route_id in route_reply
      DOMElement* route_reply = static_cast< DOMElement* >( out );
      route_reply->setAttribute( X( "route_id" ),
                                 X( RouteID(0,0).toString().c_str() ) );

      // Delete all in originMatches
      for ( uint32 i = 0 ; i < originMatches.size() ; i++ ) {
         delete originMatches[ i ];
      }
      // Delete all in destinationMatches
      for ( uint32 i = 0 ; i < destinationMatches.size() ; i++ ) {
         delete destinationMatches[ i ];
      }
      // Delete RouteRequest
      delete req->getAnswer();
      delete req;

      ok = true; // Status printed
      return ok;
   }

   // Add origins to route request
   for ( uint32 i = 0 ; i < originMatches.size() ; i++ ) {
      req->addOrigin( *originMatches[ i ] );
      if ( originMatches[ i ]->getCoords().isValid() &&
           originMatches[ i ]->getAngle() != MAX_UINT16 ) 
      {
         uturncost = 1; 
      }
   }
   
   // Add destinations to route request
   for ( uint32 i = 0 ; i < destinationMatches.size() ; i++ ) {
      req->addDestination( *destinationMatches[ i ] );
   }

   // Route settings   
   req->setRouteParameters( false,          // isStartTime
                            routeCostA,
                            routeCostB,
                            routeCostC,
                            routeCostD,
                            routeVehicle,
                            0,             // Time
                            uturncost,    // U-Turn Cost
                            abbreviateRouteNames,
                            routeLandmarks,
                            avoidtollroad,
                            avoidhighway );
   
   req->setCompareDisturbanceRoute(routeCostC != 0);

   RouteRequestParams rrp;
   req->extractParams( rrp );
   if ( reason == "traffic_info_update" &&
        ! getRouteHandler().routeChanged( 
           previous_route_id, 
           rrp,
           m_user->getUser(),
           getTopRegionRequest() ) ) {
      // Keep old route. 
      mc2log << info << "handleRoute: Keep old route: "
             << previous_route_id << endl;
      
      appendStatusNodes( out, reply, indentLevel, indent,
                         "-505", "Keep your route, it is up to date." );

      // Delete all in originMatches
      for ( uint32 i = 0 ; i < originMatches.size() ; i++ ) {
         delete originMatches[ i ];
      }
      // Delete all in destinationMatches
      for ( uint32 i = 0 ; i < destinationMatches.size() ; i++ ) {
         delete destinationMatches[ i ];
      }
      delete maps;
      delete req->getAnswer();
      delete req;
      ok = true; // Status printed
      return ok;
   }

   // Send route
   mc2dbg4 << "About to send RouteRequest" << endl;
   uint32 startProcessTime = TimeUtility::getCurrentMicroTime();

   sendRequest( req );

   // While RouteRequest is processing do the checkService
   // Only check when it is a user request.
   bool checkServiceFailed( false );
   if ( reason == "user_request" ) {
      set< MC2String > checkedServiceIDs;
      PurchaseOptions purchaseOptions( m_authData->clientType );
      // Check with external access authority if user is allowed to
      // route to desitnation
      for ( uint32 i = 0 ; i < destinationMatches.size() ; i++ ) {
         uint32 topregionID = MAX_UINT32;
         MC2Coordinate destinationCoord = MC2Coordinate::invalidCoordinate;
         if ( destinationMatches[ i ]->getID().isValid() ) {
            topregionID = getTopRegionRequest()->
               getCountryForMapID( destinationMatches[ i ]->getMapID() )->
                  getID();
         } else if ( destinationMatches[ i ]->getCoords().isValid() ) {
            destinationCoord = destinationMatches[ i ]->getCoords();
         }

         if ( ! checkService( getClientSetting(), getHttpInterfaceRequest(),
                              OperationType::ROUTE_RIGHT, purchaseOptions,
                              checkedServiceIDs, destinationCoord, topregionID,
                              m_authData->clientLang ) ) {
            checkServiceFailed = true;
         }
      }
      // check service failed
      if ( checkServiceFailed ) {
         using namespace PurchaseOption;
         if ( purchaseOptions.getReasonCode() == NEEDS_TO_BUY_APP_STORE_ADDON )
         {
            // User not allowed to route, present purchase options
            // return the uri to the client so it can send it to the web server
            // FIXME: App store id not supported by xml-protocol nor 
            //        java-clients.
            mc2log << info
               << "handleRoute:checkService User has no accsess to do a route."
               << " Returning error. "
               << endl;
            appendStatusNodes( out, reply, indentLevel, indent, "-1", 
                               "Routing not allowed.",
                               NULL, purchaseOptions.getURL().c_str() );
            ok = true; // Status printed
         } else {
            mc2log << error << "handleRoute:checkService General error." 
                   << endl;
            if ( purchaseOptions.getReasonCode() == SERVICE_NOT_FOUND ) {
               // Service id not possible to purchase
               mc2log << " Service id not possible to purchase";

               MC2String noServiceIdURL( "http://show_msg/?txt=" );
               noServiceIdURL += StringUtility::URLEncode(
                  StringTable::getString( StringTable::WF_NO_BILL_AREA,
                                          m_authData->clientLang ) );

               appendStatusNodes( out, reply, indentLevel, indent, "-1",
                                  StringTable::
                                  getString( StringTable::WF_NO_BILL_AREA,
                                             m_authData->clientLang ),
                                  NULL,
                                  noServiceIdURL.c_str() );
               ok = true; // Status printed
            }
         }
      }
   }

   // Wait for the RouteRequest to finish
   waitForRequest( req->getID() );

   uint32 stopProcessTime = TimeUtility::getCurrentMicroTime();
   mc2dbg4 << "XMLParserThread::xmlHandleRouteRequest "
              "RouteRequest time "
           << (stopProcessTime - startProcessTime)/1000 << " ms" 
           << endl;
   mc2dbg4 << "RouteRequest done" << endl;
   mc2dbg8 << "XMLParserThread::xmlHandleRouteRequest nbrSent " 
           << req->getNbrSentPackets() << " nbrResent "
           << req->getNbrResentPackets() << " nbrReceived " 
           << req->getNbrReceivedPackets() << " process time module "
           << req->getProcessingTime() << " nbrBytesReceived "
           << req->getNbrReceivedBytes()
           << endl;

   if ( req->getStatus() == StringTable::OK && ! checkServiceFailed ) {
      // The route reply
      uint32 startProcessTime = TimeUtility::getCurrentMicroTime();
      ExpandedRoute* route = req->getExpandedRoute();
      uint32 stopProcessTime = TimeUtility::getCurrentMicroTime();
      mc2dbg << "XMLParserThread::xmlHandleRouteRequest "
                 "getExpandedRoute time "
              << (stopProcessTime - startProcessTime) << " us" 
              << endl;

      {
         set<int> layerIDs;
         layerIDs.insert( TileMapTypes::c_mapLayer );
         layerIDs.insert( TileMapTypes::c_routeLayer );
         layerIDs.insert( TileMapTypes::c_poiLayer );
         mc2dbg <<"[XMLParserThread]: Adding route to precache handler." << endl;
         // add to precache
         m_group->
            preCacheRoute( route,
                           RouteID( req->getRouteID(), req->getRouteCreateTime() ),
                           ItemTypes::getLanguageCodeAsLanguageType( language ),
                           layerIDs, 
                           320 ); // pixel size
      }



      // Make routedebit
      // Set debitamount here untill module sets it ok
      req->setProcessingTime( ( TimeUtility::getCurrentTime() - 
                                m_irequest->getStartTime() ) * 1000 );
      if ( getDebitHandler()->makeRouteDebit( m_user, req ) ) {
         // Store route
         storeRoute( req, m_user->getUser()->getUIN(), "XML", 
                     m_authData->urmask );

         // Make route_reply_header
         DOMElement* route_reply_header = 
            reply->createElement( X( "route_reply_header" ) );
         out->appendChild( route_reply_header );
         if ( indent ) {
            // Newline
            out->insertBefore( reply->createTextNode( 
                                  XindentStr.XMLStr() ), 
                               route_reply_header );
         }
   
         bool compact = distFormat == StringTableUtility::COMPACT;
         route->setRouteDescriptionProperties(
            language,
            compact,
            9, 
            !compact, // Signposts
            distFormat, routeMeasurement,
            false, // wap format the text
            "/" // nameSeparator
            );
      
         // Add header data to route_reply_header
         int32 routeReplyHeaderIndentLevel = indentLevel + 1;
         MC2String route_reply_header_indentStr = indentStr;
         route_reply_header_indentStr.append( 3, ' ' );
         XStr Xroute_reply_header_indentStr( 
            route_reply_header_indentStr.c_str() );

         static int maxStr = 1024;
         char ctmp[maxStr];
         // total_distance
         route->getTotalDistanceStr( ctmp, maxStr );

         using XMLServerUtility::appendElementWithText;

         appendElementWithText( route_reply_header, reply, 
                                "total_distance", ctmp,
                                routeReplyHeaderIndentLevel, indent );

         // total_distance_nbr
         sprintf( ctmp, "%u", route->getTotalDistance() );         
         appendElementWithText( route_reply_header, reply, 
                                "total_distance_nbr", ctmp,
                                routeReplyHeaderIndentLevel, indent );

         
         // total_time
         route->getTotalTimeStr( ctmp, maxStr );
         appendElementWithText( route_reply_header, reply, 
                                "total_time", ctmp,
                                routeReplyHeaderIndentLevel, indent );

         // total_time_nbr
         sprintf( ctmp, "%u", route->getTotalTime() );
         appendElementWithText( route_reply_header, reply, 
                                "total_time_nbr", ctmp,
                                routeReplyHeaderIndentLevel, indent );

         // total_standstilltime
         route->getTotalStandstillTimeStr( ctmp, maxStr );
         appendElementWithText( route_reply_header, reply, 
                                "total_standstilltime", ctmp,
                                routeReplyHeaderIndentLevel, indent );

         // total_standstilltime_nbr
         sprintf( ctmp, "%u", route->getTotalStandStillTime() );
         appendElementWithText( route_reply_header, reply, 
                                "total_standstilltime_nbr", ctmp,
                                routeReplyHeaderIndentLevel, indent );
         
         // average_speed
         if ( route->getTotalTime() > 0 ) {
            route->getAverageSpeedStr( ctmp, 200 );
         } else {
            // N/A
            strcpy( ctmp, "N/A" );
         }
         appendElementWithText( route_reply_header, reply, 
                                "average_speed", ctmp,
                                routeReplyHeaderIndentLevel, indent );

         // average_speed_nbr
         if ( route->getTotalTime() > 0 ) {
            sprintf( ctmp, "%f", 
                     double( route->getTotalDistance())/
                     route->getTotalTime() );
         } else {
            // No time -> no speed
            strcpy( ctmp, "0.0" );
         }
         appendElementWithText( route_reply_header, reply, 
                                "average_speed_nbr", ctmp,
                                routeReplyHeaderIndentLevel, indent );

         // routing_vehicle
         appendElementWithText( 
            route_reply_header, reply, "routing_vehicle", 
            StringTable::getString( 
               ItemTypes::getVehicleSC( route->getStartingVehicle() ),
               StringTable::getNormalLanguageCode( language ) ),
            routeReplyHeaderIndentLevel, indent );

         // routing_vehicle_type
         appendElementWithText( 
            route_reply_header, reply, "routing_vehicle_type", 
            StringConversion::vehicleTypeToString( routeVehicle ),
            routeReplyHeaderIndentLevel, indent );

         // boundingbox
         int32 northLat = route->getRouteBoundingBox().getMaxLat();
         int32 westLon = route->getRouteBoundingBox().getMinLon();
         int32 southLat = route->getRouteBoundingBox().getMinLat();
         int32 eastLon = route->getRouteBoundingBox().getMaxLon();
         XMLSearchUtility::
            appendBoundingbox( route_reply_header,
                               reply,
                               bboxCoordinateSystem,
                               northLat, westLon, southLat, eastLon,
                               indentLevel + 1, indent );

      
         if ( routeImageLinks ) {
            // route_overview_link
            
            if ( indent ) {
               // Indent
               route_reply_header->appendChild( reply->createTextNode( 
                  Xroute_reply_header_indentStr.XMLStr() ) );
            }
            // route_overview_link element
            DOMElement* route_overview_link = reply->createElement( 
               X( "route_overview_link" ) );
            char url[ 4096 ];
            char widthStr[ 11 ];
            char heightStr[ 11 ];
            uint16 width = routeOverviewImageWidth;
            uint16 height = routeOverviewImageHeight;

            HttpUtility::makeMapURL( 
               url, widthStr, heightStr,
               m_inHead, route->getRouteID(),
               route->getRouteCreateTime(),
               MAX_UINT32, MAX_UINT32, // No turn
               MAX_UINT32,            // No turn
               width, height, 
               route->getRouteBoundingBox().getMinLat(),
               route->getRouteBoundingBox().getMinLon(),
               route->getRouteBoundingBox().getMaxLat(),
               route->getRouteBoundingBox().getMaxLon(),
               routeImageFormat,
               routeImageDisplayType,
               &imageSettings );
            route_overview_link->appendChild( reply->createCDATASection(
                                                 X( url ) ) );
            route_reply_header->appendChild( route_overview_link );

            // route_overview_width
            appendElementWithText( route_reply_header, reply, 
                                   "route_overview_width", widthStr,
                                   routeReplyHeaderIndentLevel, indent );

            // route_overview_height
            appendElementWithText( route_reply_header, reply, 
                                   "route_overview_height", heightStr,
                                   routeReplyHeaderIndentLevel, indent );
         }

         if ( indent ) {
            // Newline and indent before end route_reply_header tag
            route_reply_header->appendChild( 
               reply->createTextNode( XindentStr.XMLStr() ) );
         }
      

         // Make origin and destination nodes in reply      
         // Add route_origin elements
         if ( indent ) {
            // Indent
            out->appendChild( reply->createTextNode( 
                                 XindentStr.XMLStr() ) );
         }
         // route_origin element
         DOMElement* route_origin = reply->createElement( 
            X( "route_origin" ) );
         for ( uint32 i = 0 ; i < req->getNbrValidOrigins() ; i++ ) {
            VanillaMatch* originVM = 
               getVanillaMatchFromRouteRequestValidMatch( req, i, true,
                                                          originMatches );
            if ( originVM != NULL ) {
               XMLSearchUtility::
                  appendSearchItem( route_origin, reply, 
                                    originVM, "", indentLevel + 1, indent );
            }
            delete originVM;
         }

         if ( indent ) {
            route_origin->appendChild( reply->createTextNode( 
                                          XindentStr.XMLStr() ) );
         }
         out->appendChild( route_origin );

         // Add route_destination element
         if ( indent ) {
            // Indent
            out->appendChild( reply->createTextNode( 
                                 XindentStr.XMLStr() ) );
         }
         // route_destination element
         DOMElement* route_destination = reply->createElement( 
            X( "route_destination" ) );
         for ( uint32 i = 0 ; i < req->getNbrValidDestinations() ; i++ ) {
            VanillaMatch* destinationVM = 
               getVanillaMatchFromRouteRequestValidMatch( 
                  req, i, false, destinationMatches);
            if ( destinationVM != NULL ) {
               XMLSearchUtility::
                  appendSearchItem( route_destination, reply, 
                                    destinationVM, "", indentLevel + 1, 
                                    indent );
            }
            delete destinationVM;
         }

         if ( indent ) {
            route_destination->appendChild( reply->createTextNode( 
                                               XindentStr.XMLStr() ) );
         }
         out->appendChild( route_destination );
   
         // Make route nodes in route_reply_items in reply
         if ( indent ) {
            // Indent
            out->appendChild( reply->createTextNode( 
                                 XindentStr.XMLStr() ) );
         }

         // Periodic traffic info interval
         out->setAttribute( 
            X( "ptui" ), 
            XUint32( m_group->getPeriodicTrafficUpdateInterval() ) );

         // route_reply_items element
         DOMElement* route_reply_items = reply->createElement( 
            X( "route_reply_items" ) );
         out->appendChild( route_reply_items );

         if ( routeItems ) {
            XMLLastRouteData lastData( false, false, false );
            if ( route->getNbrExpandedRouteItems() > 0 ) {
               // Set to not first attributes to send the first attributes
               const ExpandedRouteItem* routeItem = 
                  route->getExpandedRouteItem( 0 );
               lastData.controlledAccess = !routeItem->
                  getExpandedRouteRoadItem( 0 )->getControlledAccess();
               lastData.ramp = !routeItem->getExpandedRouteRoadItem( 0 )
                  ->getRamp();
               lastData.driveOnRightSide = !routeItem->
                  getExpandedRouteRoadItem( 0 )->getDriveOnRightSide();
            }
            for ( uint32 i = 0 ; i < route->getNbrExpandedRouteItems() ;
                  i++ ) 
            {
               const ExpandedRouteItem* routeItem = 
                  route->getExpandedRouteItem( i );
               // Make route_reply_item for the ExpandedRouteItem
               appendRouteItem( route_reply_items,
                                reply,
                                routeItem,
                                indentLevel + 1, indent,
                                i,
                                route,
                                language,
                                routeImageLinks,
                                routeTurnData,
                                routeRoadData,
                                routeLandmarks,
                                routeTurnImageWidth, routeTurnImageHeight,
                                routeImageFormat,
                                routeImageDisplayType,
                                imageSettings,
                                routeTurnBoundingbox,
                                bboxCoordinateSystem,
                                routeItem->getTurnBoundingbox(),
                                lastData, m_inHead );
            }

            if ( indent ) {
               // Newline and indent for route_reply_items end tag
               route_reply_items->appendChild( reply->createTextNode( 
                                                  XindentStr.XMLStr() ) );
            }
         }

         uint32 nbrOrigins = route->getNbrOrigins();
         uint32 nbrDestinations = route->getNbrDestinations();
         mc2log << info << "XMLParserThread::xmlHandleRouteRequest "
                << "routeID " << route->getRouteID() 
                << " routeCreateTime " <<  route->getRouteCreateTime();
         mc2log << " origin";
         if ( nbrOrigins > 1 ) {
            mc2log << "s: ";
         } else {
            mc2log << ": ";
         }
         for ( uint32 i = 0 ; i < nbrOrigins ; i++ ) {
            const ExpandedRouteMatch* origin = route->getOrigin( i );
            if ( i != 0 ) {
               mc2log << ", ";
            }
            mc2log << "[" << origin->getName()->getName() << ", " 
                   << origin->getLat() << ", " << origin->getLon() << "]";
         }
         mc2log << " destination";
         if ( nbrDestinations > 1 ) {
            mc2log << "s: ";
         } else {
            mc2log << ": ";
         }
         for ( uint32 i = 0 ; i < nbrDestinations ; i++ ) {
            const ExpandedRouteMatch* dest = route->getDestination( i );
            if ( i != 0 ) {
               mc2log << ", ";
            }
            mc2log << "[" << dest->getName()->getName() << ", " 
                   << dest->getLat() << ", " << dest->getLon() << "]";
         }
         mc2log << " distance: " << route->getTotalDistance() 
                << " time: " << route->getTotalTime();
         mc2log << endl;

      } else {
         mc2log << warn << "XMLParserThread::xmlHandleRouteRequest "
                   "debiting of route failed when communicating with "
                   " UserModule" << endl;
         // Send we're sorry but we can't give you the data becauce
         // debiting failed status
         appendStatusNodes( out, reply, indentLevel, indent,
                            "-1",
                            "Route debit failed." );
      }

      ok = true;
   } else {
      mc2log << warn << "XMLParserThread::xmlHandleRouteRequest "
                "no answer or not ok from RouteModule" << endl;
      // Routing failed
      MC2String errCode = "-1";
      MC2String errStr = "Routing failed: ";
      errStr.append( StringTable::getString( req->getStatus(),
                                             StringTable::ENGLISH ) );
      if ( checkServiceFailed ) {
         errStr.append( " Check service failed" );
      }

      if ( req->getStatus() == StringTable::TIMEOUT_ERROR ) {
         errCode = "-3";
      } else if ( req->getStatus() == StringTable::MAPNOTFOUND ) {
         errCode = "-4";
      } else if ( req->getStatus() == StringTable::ERROR_NO_ROUTE ) {
         errCode = "-501";
      } else if ( req->getStatus() == StringTable::TOO_FAR_TO_WALK ) {
         errCode = "-502";
      } else if ( req->getStatus() == 
                  StringTable::ERROR_NO_VALID_START_ROUTING_NODE ||
                  req->getStatus() == 
                  StringTable::ONE_OR_MORE_INVALID_ORIGS ) 
      {
         errCode = "-503";
      } else if ( req->getStatus() == 
                  StringTable::ERROR_NO_VALID_END_ROUTING_NODE ||
                  req->getStatus() == 
                  StringTable::ONE_OR_MORE_INVALID_DESTS ) 
      {
         errCode = "-504";
      } else if ( req->getStatus() == 
                  StringTable::DESTINATION_OUTSIDE_ALLOWED_AREA ||
                  req->getStatus() == 
                  StringTable::ORIGIN_OUTSIDE_ALLOWED_AREA ||
                  req->getStatus() == StringTable::OUTSIDE_ALLOWED_AREA )
      {
         errCode = "-5";
      }
      if ( ! ok ) {
         // status node not appended, do it!
         appendStatusNodes( out, reply, indentLevel, indent,
                            errCode.c_str(), errStr.c_str() );
         // only dump the state when we append the status node here
         req->dumpState();
         ok = true;  // Status printed
      }
   }

   // Set route_id in route_reply
   DOMElement* route_reply = static_cast< DOMElement* >( out );
   if ( !checkServiceFailed && req->getStatus() == StringTable::OK ) {
      route_reply->setAttribute( X( "route_id" ),
                                 X( RouteID(req->getRouteID(),
                                            req->getRouteCreateTime()).
                                    toString().c_str() ) );      
   } else {
      route_reply->setAttribute( X( "route_id" ),
                                 X( RouteID(0,0).toString().c_str() ) );
   }

   // Delete all in originMatches
   for ( uint32 i = 0 ; i < originMatches.size() ; i++ ) {
      delete originMatches[i];
   }
   // Delete all in destinationMatches
   for ( uint32 i = 0 ; i < destinationMatches.size() ; i++ ) {
      delete destinationMatches[i];
   }


   delete req->getAnswer();
   delete req;
   delete maps;

   return ok;
}


bool 
XMLParserThread::xmlParseRouteRequestRouteRequestHeaderRoutePreferences( 
   DOMNode* cur, 
   DOMNode* out,
   DOMDocument* reply,
   int indentLevel,
   byte& routeCostA,
   byte& routeCostB,
   byte& routeCostC,
   byte& routeCostD,
   bool& avoidtollroad,
   bool& avoidhighway,
   ItemTypes::vehicle_t& routeVehicle,
   StringTableUtility::distanceFormat& distFormat,
   StringTableUtility::distanceUnit& routeMeasurement,
   StringTable::languageCode& language,
   bool& routeImageLinks,
   bool& routeTurnData,
   bool& routeRoadData,
   bool& routeItems,
   bool& abbreviateRouteNames,
   bool& routeLandmarks,
   uint32& routeOverviewImageWidth,
   uint32& routeOverviewImageHeight,
   uint32& routeTurnImageWidth,
   uint32& routeTurnImageHeight,
   ImageDrawConfig::imageFormat& routeImageFormat,
   MapSettingsTypes::defaultMapSetting& routeImageDisplayType,
   struct MapSettingsTypes::ImageSettings& imageSettings,
   XMLCommonEntities::coordinateType& bboxCoordinateSystem,
   bool& routeTurnBoundingbox )
{
   bool ok = false;

   MC2String userID;
   MC2String user_session_id;
   MC2String user_session_key;
   uint32 uin = 0;
   bool hasUserSettings = false;

   DOMElement* route_preferences = static_cast< DOMElement* >( cur );

   // Go throu attributes and set route settings
   DOMNamedNodeMap* attributes = route_preferences->getAttributes();
   DOMNode* attribute;

   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(),
                              "route_description_type" ) ) 
      {
         distFormat = StringConversion::distanceFormatFromString( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_image_links" ) ) 
      {
         routeImageLinks = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_turn_data" ) ) 
      {
         routeTurnData = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_road_data" ) ) 
      {
         routeRoadData = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_items" ) ) 
      {
         routeItems = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_landmarks" ) ) 
      {
         routeLandmarks = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_measurment_system" ) ) 
      {
         
         if((StringUtility::strcasecmp( tmpStr, "imperial" ) == 0) ||
            (StringUtility::strcasecmp(tmpStr , "yards" ) == 0 )){
            routeMeasurement = StringTableUtility::YARDS;
         } else if ( StringUtility::strcasecmp( tmpStr, "feet" ) == 0 ) {
            routeMeasurement = StringTableUtility::FEET;
         } else {
            routeMeasurement = StringTableUtility::METERS;
         }
         
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "abbreviate_route_names" ) )
      {
         abbreviateRouteNames = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_overview_image_width" ) ) 
      {
         char* tmpPtr = NULL;
         uint32 tmp = strtoul( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr != tmpStr ) {
            routeOverviewImageWidth = tmp;
         } else {
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseRouteRequestRouteRequestHeader"
                   << "RoutePreferences "
                   << "route_overview_image_width not number"
                   << "   value " << tmpStr << endl;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_overview_image_height" ) ) 
      {
         char* tmpPtr = NULL;
         uint32 tmp = strtoul( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr != tmpStr ) {
            routeOverviewImageHeight = tmp;
         } else {
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseRouteRequestRouteRequestHeader"
                   << "RoutePreferences "
                   << "route_overview_image_height not number"
                   << "   value " << tmpStr << endl;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_turn_image_width" ) ) 
      {
         char* tmpPtr = NULL;
         uint32 tmp = strtoul( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr != tmpStr ) {
            routeTurnImageWidth = tmp;
         } else {
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseRouteRequestRouteRequestHeader"
                   << "RoutePreferences "
                   << "route_turn_image_width not number"
                   << "   value " << tmpStr << endl;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_turn_image_height" ) ) 
      {
         char* tmpPtr = NULL;
         uint32 tmp = strtoul( tmpStr, &tmpPtr, 10 );
         if ( tmpPtr != tmpStr ) {
            routeTurnImageHeight = tmp;
         } else {
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseRouteRequestRouteRequestHeader"
                   << "RoutePreferences "
                   << "route_turn_image_height not number"
                   << "   value " << tmpStr << endl;
         }
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_image_default_format" ) ) 
      {
         routeImageFormat = ImageDrawConfig::imageFormatFromString( 
            tmpStr, routeImageFormat );         
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_image_display_type" ) ) 
      {
         routeImageDisplayType = MapSettings::defaultMapSettingFromString( 
            tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "route_boundingbox_position_sytem" ) )
      {
         bboxCoordinateSystem = 
            XMLCommonEntities::coordinateFormatFromString( tmpStr );
      } 
      else if ( XMLString::equals( attribute->getNodeName(),
                                   "route_turn_boundingbox" ) ) 
      {
         routeTurnBoundingbox = StringUtility::checkBoolean( tmpStr );
      } else {
         mc2log << warn << "XMLParserThread::"
                << "xmlParseRouteRequestRouteRequestHeader"
                << "RoutePreferences "
                << "unknown attribute "
                << "Name " << attribute->getNodeName()
                << " Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }   


   // Go throu children and handle route settings
   DOMNode* child = cur->getFirstChild();
   
   ok = true;
   while ( child != NULL && ok) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            if ( XMLString::equals( child->getNodeName(), "user_id" ) ) {
               char* tmpStr =  XMLUtility::getChildTextValue( child );
               userID = tmpStr;
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(), 
                                           "user_session_id" ) ) 
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               user_session_id = tmpStr;
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "user_session_key" ) )
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               user_session_key = tmpStr;
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(), "uin" ) ){
               char* tmpStr =  XMLUtility::getChildTextValue( child );
               char* endPtr = NULL;
               uin = strtoul( tmpStr, &endPtr, 10 );
               if ( endPtr == NULL || *endPtr != '\0' || uin == 0 ) {
                  ok = false;
                  //errorCode = "-1";
                  //errorMessage = "Problem parsing, uin not a valid number.";
                  mc2log << warn << "XMLParserThread::"
                         << "xmlParseRouteRequestRouteRequestHeader "
                         << "uin not a valid number. " << MC2CITE( tmpStr )
                         << endl;
               }
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "route_settings" ) ) 
            {
               ok = xmlParseRouteRequestRouteRequestHeaderRouteSettings(
                  child,
                  out,
                  reply,
                  indentLevel,
                  routeCostA,
                  routeCostB,
                  routeCostC,
                  routeCostD,
                  avoidtollroad,
                  avoidhighway,
                  routeVehicle,
                  distFormat,
                  language );
               if ( distFormat == StringTableUtility::COMPACT ) {
                  language = StringTable::getShortLanguageCode(
                     language );
               }
               hasUserSettings = true;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "image_settings" ) ) 
            {
               if ( !readImageSettings( child, imageSettings ) ) {
                  mc2log << warn << "XMLParserThread::"
                     "xmlParseRouteRequestRouteRequestHeader"
                     "RoutePreferences readImageSettings failed" << endl;
               }
            } else {
               mc2log << warn <<  "XMLParserThread::"
                      <<"xmlParseRouteRequestRouteRequestHeader"
                      <<"RoutePreferences "
                      <<"route_preferences child node has unknown name. "
                      << "NodeName: " << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseRouteRequestRouteRequestHeader"
                   <<"RoutePreferences odd "
                   <<"node type in route_preferences element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType()<< endl;
            break;
      }
      child = child->getNextSibling();
   }

   if ( !hasUserSettings && 
        (!userID.empty() || uin != 0 ||
         ( !user_session_id.empty() && !user_session_key.empty() ) ) )
   {
      UserItem* userItem = NULL;
      bool status = true;

      if ( !userID.empty() ) {
         status = getUser( userID.c_str(), userItem, true );
      } else if ( uin != 0 ) {
         status = getUser( uin, userItem, true );
      } else {
         status = getUserBySession( 
            user_session_id.c_str(), user_session_key.c_str(), userItem,
            true );
      }

      if ( status ) {
         if ( userItem != NULL ) {
            UserUser* user = userItem->getUser();

            // Get route settings
            language = user->getLanguage();
            if ( distFormat == StringTableUtility::COMPACT ) {
               language = StringTable::getShortLanguageCode( language );
            }
            routeCostA = user->getRouting_costA();
            routeCostB = user->getRouting_costB();
            routeCostC = user->getRouting_costC();
            routeCostD = user->getRouting_costD();
            routeVehicle = user->getRouting_vehicle(); 
         } else {
            mc2log << warn << "XMLParserThread::"
                      "xmlParseRouteRequestRouteRequestHeader "
                      "unknown user, userID: " << userID 
                   << " user_session_id " << user_session_id 
                   << " user_session_key " << user_session_key << endl;
            ok = false;
         }
      } else {
         mc2log << warn << "XMLParserThread::"
                   "xmlParseRouteRequestRouteRequestHeader "
                   "Database connection error, "
                   "couldn't get user from userID: " << userID 
                << " user_session_id " << user_session_id 
                << " user_session_key " << user_session_key << endl;
         ok = false;
      }

      releaseUserItem( userItem );
   } else {
      if ( !hasUserSettings ) {
         ok = false;
      }
   }



   return ok;
}


bool 
XMLParserThread::xmlParseRouteRequestRouteRequestHeaderRouteSettings(
   DOMNode* cur, 
   DOMNode* out,
   DOMDocument* reply,
   int indentLevel,
   byte& routeCostA,
   byte& routeCostB,
   byte& routeCostC,
   byte& routeCostD,
   bool& avoidtollroad,
   bool& avoidhighway,
   ItemTypes::vehicle_t& routeVehicle,
   StringTableUtility::distanceFormat& distFormat,
   StringTable::languageCode& language )
{
   bool ok = false;


   DOMElement* route_settings = static_cast< DOMElement* >( cur );

   // Go throu attributes and set route settings
   DOMNamedNodeMap* attributes = route_settings->getAttributes();
   DOMNode* attribute;

   for ( uint32 i = 0 ; i < attributes->getLength() ; i++ ) {
      attribute = attributes->item( i );
      
      char* tmpStr = XMLUtility::transcodefromucs( 
         attribute->getNodeValue() );
      if ( XMLString::equals( attribute->getNodeName(),
                              "route_vehicle" ) ) 
      {
         routeVehicle = StringConversion::vehicleTypeFromString( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "avoid_toll_road" ) ) 
      {
         avoidtollroad = StringUtility::checkBoolean( tmpStr );
      } else if ( XMLString::equals( attribute->getNodeName(),
                                     "avoid_highway" ) ) 
      {
         avoidhighway = StringUtility::checkBoolean( tmpStr );
      } else {
         mc2log << warn << "XMLParserThread::"
                << "xmlParseRouteRequestRouteRequestHeader"
                << "RouteSettings unknown attribute "
                << "Name " << attribute->getNodeName()
                << "Type " << attribute->getNodeType() << endl;
      }
      delete [] tmpStr;
   }   
   

   // Go throu children and handle route settings
   DOMNode* child = cur->getFirstChild();
   
   ok = true;
   while ( child != NULL && ok) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            if ( XMLString::equals( child->getNodeName(),
                                    "route_costA" ) ) 
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               routeCostA = strtol( tmpStr, NULL, 10 );
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "route_costB" ) ) 
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               routeCostB = strtol( tmpStr, NULL, 10 );
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "route_costC" ) ) 
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               routeCostC = strtol( tmpStr, NULL, 10 );
               delete [] tmpStr;
            } else if ( XMLString::equals( child->getNodeName(),
                                           "language" ) ) 
            {
               char* tmpStr = XMLUtility::getChildTextValue( child );
               language = getLanguageCode(tmpStr );
               delete [] tmpStr; 
            } else {
               mc2log << warn << "XMLParserThread::"
                      << "xmlParseRouteRequestRouteRequestHeader"
                      << "RouteSettings "
                      << "route_settings child node has unknown name. "
                      << "NodeName: " << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                   << "xmlParseRouteRequestRouteRequestHeader"
                   << "RouteSettings odd "
                   << "node type in route_settings element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType()<< endl;
            break;
      }
      child = child->getNextSibling();
   }

   return ok;
}


bool 
XMLParserThread::readAndPositionRouteableItemList( 
   DOMNode* searchItemList,
   VanillaVector& vanillaVector,
   MC2String& errorCode, MC2String& errorMessage )
{
   bool ok = true;
   
   // Get children
   DOMNode* child = searchItemList->getFirstChild();
   
   while ( child != NULL && ok ) {
      switch ( child->getNodeType() ) {
         case DOMNode::ELEMENT_NODE :
            // See if the element is a known type
            if ( XMLString::equals( child->getNodeName(),
                                    "search_item" ) ) 
            {
               VanillaMatch* match = XMLSearchUtility::
                  getVanillaMatchFromSearchItem( child, 
                                                 errorCode, errorMessage );
               if ( match != NULL ) {
                  vanillaVector.push_back( match );
               } else {
                  mc2log << warn << "XMLParserThread::"
                         << "readAndPositionSearchItemList "
                         << "search_item extraction error" << endl;
                  XMLTreeFormatter::printTree( child );
                  mc2log << endl;
                  ok = false;
               }
            } else  if ( XMLString::equals( child->getNodeName(),
                                            "position_item" ) ) 
            {
               int32 lat = 0;
               int32 lon = 0;
               uint16 angle = MAX_UINT16;
               if ( XMLCommonElements::getPositionItemData( 
                       child, lat, lon, angle, 
                       errorCode, errorMessage ) )
               {
                  VanillaMatch* match = new VanillaStreetMatch( 
                     IDPair_t(), "", "", 0, 0 );
                  match->setCoords( MC2Coordinate( lat, lon ) );
                  match->setAngle( angle );
                  vanillaVector.push_back( match ); 
               } else {
                  ok = false;
                  mc2log << warn << "XMLParserThread::"
                         << "readAndPositionRouteableItemList "
                         << "bad position_item";
                  XMLTreeFormatter::printTree( child );
                  mc2log << endl;
               }
            } else {
               mc2log << warn << "XMLParserThread::"
                      << "readAndPositionSearchItemList "
                      << "odd Element in search_area element: " 
                      << child->getNodeName() << endl;
            }
            break;
         case DOMNode::COMMENT_NODE :
            // Ignore comments
            break;
         default:
            mc2log << warn << "XMLParserThread::"
                   << "readAndPositionSearchItemList odd "
                   << "node type in routeable_item_list element: " 
                   << child->getNodeName() 
                   << " type " << child->getNodeType()<< endl;
            break;
      }
      child = child->getNextSibling();
   }
   
   return ok;
}


VanillaMatch* 
XMLParserThread::getVanillaMatchFromRouteRequestValidMatch(
   RouteRequest* routeReq,
   uint32 index,
   bool origin,
   VanillaVector matches ) const
{
   VanillaMatch* match = NULL;

   uint32 mapID = 0;
   uint32 itemID = 0;
   const char* name = NULL;
   int32 lat = 0;
   int32 lon = 0;
   ItemTypes::itemType type = ItemTypes::nullItem;
   bool ok = false;

   if ( origin ) {
      if ( routeReq->getValidOrigin( index,
                                     mapID,
                                     itemID,
                                     name,
                                     lat,
                                     lon,
                                     type ) ) 
      {
         ok = true;
      } else {
         mc2log << warn << "XMLParserThread::"
                << "getVanillaMatchFromRouteRequestValidMatch "
                << "originindex out of bounds."
                << "   index " << index << " nbr " 
                << routeReq->getNbrValidOrigins() << endl;
      }
   } else {
      if ( routeReq->getValidDestination( index,
                                          mapID,
                                          itemID,
                                          name,
                                          lat,
                                          lon,
                                          type ) ) 
      {
         ok = true;
      } else {
         mc2log << warn << "XMLParserThread::"
                << "getVanillaMatchFromRouteRequestValidMatch "
                << "destinationindex out of bounds."
                << "   index " << index << " nbr " 
                << routeReq->getNbrValidDestinations() << endl;
      }

   }

   if ( ok ) {
      // Get location, if any from matches input, via mapID and itemID
      const char* locationName = "";
      uint32 searchType = XMLSearchUtility::
         itemTypeToSearchItemType( type );

      match = new VanillaStreetMatch( IDPair_t(mapID, itemID),
                                      "", /* Name */
                                      locationName, 
                                      MAX_UINT16, /* Offset */
                                      0 /* Streetnbr */);
      
      VanillaVectorIt inputMatch = 
         find_if(matches.begin(), matches.end(), 
                 bind2nd( equalVanillaMatch(), match));      
      if ( inputMatch != matches.end() ) {
         // Found match
         locationName = (*inputMatch)->getLocationName();
      }
      delete match;
      match = NULL;

      match = XMLSearchUtility::
         makeVanillaMatchFromData( mapID, itemID, 0, name, 
                                   locationName, 0, searchType );
   }

   return match;
}





#endif // USE_XML

