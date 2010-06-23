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
#include "HttpMapFunctions.h"
#include "HttpParserThread.h"
#include "UserData.h"
#include "UserLicenceKey.h"
#include "RoutePacket.h"
#include "ExpandRoutePacket.h"
#include "ExpandItemID.h"
#include "RouteRequest.h"
#include "GDImageDraw.h"
#include "SinglePacketRequest.h"
#include "GfxFeatureMap.h"
#include "GfxPolygon.h"
#include "GfxFeatureMapImageRequest.h"
#include "GfxFeatureMapImagePacket.h"
#include "GfxFeatureMapPacket.h"
#include "HttpUtility.h"
#include "ExpandRouteRequest.h"
#include "HttpFunctionHandler.h"
#include "HttpBody.h"
#include "HttpHeader.h"
#include "HttpInterfaceRequest.h"
#include "STLStringUtility.h"
// htmlShowLocalMap
#include "LocalMapMessageRequest.h"
#include "MimeMessage.h"
#include "ParserTokenHandler.h"
#include "ServerTileMapFormatDesc.h"
#include "TileMapParams.h"
#include "FilteredGfxMapRequest.h"
#include "RouteID.h"
#include "RedLineSettings.h"
#include "CoordinateTransformer.h"
#include <memory>
#include "NameUtility.h"
#include "HttpProjectionFunctions.h"
#include "ParserThreadGroup.h"
#include "CopyrightHandler.h"
#include "ParserUserHandler.h"
#include "InterfaceRequestData.h"
#include "ClientSettings.h"
#include "ParserDebitHandler.h"
#include "ProjectionSettings.h"
#include "ParserExternalAuth.h"

#include "HttpCodes.h"

void
HttpMapFunctions::getSpeedSettings( RedLineSettings& redLineSettings,
                                    stringMap* paramsMap,
                                    bool& allData )
{
   // Simplemap speed settings
   static const char sm_speeds[] = "sm_speed_dist";
   if ( paramsMap->find( sm_speeds ) != paramsMap->end() ) {
      RedLineSettings::speedVect_t speedVect;
      const MC2String* val = (*paramsMap)[ sm_speeds ];
      // sm_speed_dist=0_50,70_300
      // Blargh!! Parse the speed settings.
      // Split on the comma
      vector<MC2String> speedPairs;
      StringUtility::tokenListToVector( speedPairs,
                                        *val,
                                        ',' );      
      for ( vector<MC2String>::const_iterator it = speedPairs.begin();
            it != speedPairs.end();
            ++it ) {
         // Split on underline
         vector<MC2String> speedDist;
         StringUtility::tokenListToVector( speedDist, *it, '_' );
         if ( speedDist.size () != 2 ) {
            mc2dbg << "[HTTPMapFunc]: Invalid speed_dist " << *it << endl;
            allData = false;
            break;
         }
         uint32 speed = 0;
         uint32 dist  = 50;
         speed = strtoul( speedDist[0].c_str(), NULL, 0 );
         dist = strtoul( speedDist[1].c_str(), NULL, 0 );
         speedVect.push_back( make_pair( speed, dist ) );
      }
      redLineSettings.swapSpeedVect( speedVect );
   }
   // Extract speed vector for bbox
   static const char sm_bbox_speeds[] = "sm_bbox_speed_dist";
   if( paramsMap->find( sm_bbox_speeds ) != paramsMap->end() ) {
      RedLineSettings::speedVect_t bboxSpeedVect;
      const MC2String* val = (*paramsMap)[ sm_bbox_speeds ];
      vector<MC2String> speedPairs;
      StringUtility::tokenListToVector( speedPairs,
                                        *val,
                                        ',' );
      for( vector<MC2String>::const_iterator it = speedPairs.begin();
           it != speedPairs.end();
           ++it ) {
         // Split on underline
         vector<MC2String> speedDist;
         StringUtility::tokenListToVector( speedDist, *it, '_' );
         if ( speedDist.size() != 2 ) {
            mc2dbg << "[HTTPMapFunc]: Invalid bbox_speed_dist "
                   << *it << endl;
            allData = false;
            break;
         }
         uint32 speed = 0;
         uint32 dist = 50;
         speed = strtoul( speedDist[0].c_str(), NULL, 0 );
         dist = strtoul( speedDist[1].c_str(), NULL, 0 );
         bboxSpeedVect.push_back( make_pair( speed, dist ) );
      }
      redLineSettings.swapBBoxSpeedVect( bboxSpeedVect );
   }
}

bool
HttpMapFunctions::htmlMakeImage( stringVector* params,
                                 int paramc, 
                                 stringMap* paramsMap,
                                 HttpHeader* inHead, 
                                 HttpHeader* outHead,
                                 HttpBody* inBody,
                                 HttpBody* outBody,
                                 HttpParserThread* myThread,
                                 HttpVariableContainer* myVar )
{
   myThread->addRequestName( "MAP" );
   bool ok = false; // Map is done
   
   int32 lla = 0;
   int32 llo = 0;
   int32 ula = 0;
   int32 ulo = 0;
   uint16 w = 0;
   uint16 h = 0;
   uint16 size = 4096 + 1;
   bool binaryMap = false; // Map data or Image
   bool exportFormat = true; // If you want a GfxExportFeatureMap.
   uint32 maxScale = INVALID_SCALE_LEVEL;
   uint32 minScale = CONTINENT_LEVEL;
   uint32 filtScale = INVALID_SCALE_LEVEL;
   ImageDrawConfig::imageFormat format = ImageDrawConfig::GIF;
   uint32 imageSize = 0;
   byte* imageBuff = NULL;
   // Route
   bool route = false;
   const char* routeIDStr = NULL;
   uint32 routeID = 0;
   uint32 routeCreateTime = 0;
   RouteReplyPacket* routePack = NULL;
   uint32 UIN = 0;
   const char* extraUserinfo = NULL;
   uint32 validUntil = 0;
   int32 originLat = 0;
   int32 originLon = 0;
   uint32 originMapID = 0;
   uint32 originItemID = 0;
   uint16 originOffset = 0;
   int32 destinationLat = 0;
   int32 destinationLon = 0;
   uint32 destinationMapID = 0;
   uint32 destinationItemID = 0;
   uint16 destinationOffset = 0;
   PacketContainer* cont = NULL;
   // RouteTurn
   bool routeTurn = false;
   int beforeTurnFeatNbr = 0;
   int afterTurnFeatNbr = 0;
   // PositionArc
   bool positionArc = false;
   const char* posArcStr = NULL;
   int cx = 0;
   int cy = 0;
   int startAngle = 0;
   int stopAngle = 0;
   int iR = 0;
   int oR = 0;
   // MapSettings
   MapSettingsTypes::defaultMapSetting mapType = 
      MapSettingsTypes::MAP_SETTING_STD;
   MapSettings* mapSettings = NULL;
   bool haveImageSettings = false;
   struct MapSettingsTypes::ImageSettings imageSettings;
   bool showMap = true;
   bool showTopographMap = true;
   bool showPOI = false;
   bool showRoute = true;
   bool showScale = false;
   bool showTraffic = false;
   // The mapsymbols to put on map
   GfxFeatureMap* symbolMap = NULL;
   // If simple map or not. 0 means no simple map.
   int simpleMap = 0;
   // User Allowed Region Access (AURA)
   set< uint32 >* allowedMaps = NULL;
   UserItem* userItem = NULL;
   MC2Coordinate posCoord;
   const MC2String USERAGENT = "User-Agent";
   // Initialize language
   myVar->currentLanguage = StringTable::ENGLISH;
   char* tmpStr = NULL;
   bool drawCopyRight = true;
   
   // If we have all nessesary params
   bool allData = true;

   // Read lla, llo, ula, ulo, w and h
   // lla
   if ( paramsMap->find( "lla" ) != paramsMap->end() ) {
      lla = strtol( paramsMap->find( "lla" )->second->c_str(), NULL, 10 );
   } else {
      allData = false;
   }

   // llo
   if ( paramsMap->find( "llo" ) != paramsMap->end() ) {
      llo = strtol( paramsMap->find( "llo" )->second->c_str(), NULL, 10 );
   } else {
      allData = false;
   }

   // ula
   if ( paramsMap->find( "ula" ) != paramsMap->end() ) {
      ula = strtol( paramsMap->find( "ula" )->second->c_str(), NULL, 10 );
   } else {
      allData = false;
   }

   // ulo
   if ( paramsMap->find( "ulo" ) != paramsMap->end() ) {
      ulo = strtol( paramsMap->find( "ulo" )->second->c_str(), NULL, 10 );
   } else {
      allData = false;
   }

   // w
   if ( paramsMap->find( "w" ) != paramsMap->end() ) {
      uint32 width = strtoul( paramsMap->find( "w" )->second->c_str(), 
                              &tmpStr, 10 );
      if ( tmpStr != NULL && tmpStr[ 0 ] == '\0' ) {
         w = width;
      } else {
         allData = false;
      }
   } else {
      allData = false;
   }

   
   // h
   if ( paramsMap->find( "h" ) != paramsMap->end() ) {
      uint32 height = strtoul( paramsMap->find( "h" )->second->c_str(), 
                               &tmpStr, 10 );
      if ( tmpStr != NULL && tmpStr[ 0 ] == '\0' ) {
         h = height;
      } else {
         allData = false;
      }
   } else {
      allData = false;
   }

   
   // Sanity check on image size
   w = MIN( w, 1024 );
   h = MIN( h, 1024 );
      
   // Size
   if ( paramsMap->find( "s" ) != paramsMap->end() ) {
      size = MAX( 10, MIN( strtol( paramsMap->find( "s" )->second->c_str(),
                                   NULL, 10 ), MAX_UINT16 ) ); 
   }

   // Gfx/Slim in param 0. Not used anymore

   // Bin/Image
   if ( paramc >= 2 ) {
      if ( StringUtility::strcasecmp( (*params)[1]->c_str(), 
                                      "bin" ) == 0 ) 
      {
         binaryMap = true;
      } else { // Image
         format = ImageDrawConfig::imageFormat( 
            strtol( (*params)[1]->c_str(), NULL, 10 ) ); 
         if ( format >= ImageDrawConfig::NBR_IMAGE_FORMATS ||
              format < 0 ) 
         {
            format = ImageDrawConfig::PNG;
         }
      }
   }
   
   // position (p)
   if  ( paramsMap->find( "p" ) != paramsMap->end() ) {
      posArcStr = (*paramsMap )[ "p" ] ->c_str();
      if ( sscanf( posArcStr, "%d_%d_%d_%d_%d_%d", 
                   &cx, &cy, 
                   &startAngle, &stopAngle, 
                   &iR, &oR ) == 6 ) 
      {
         positionArc = true;
      }
   }
   
   // MapType (mt)
   if  ( paramsMap->find( "mt" ) != paramsMap->end() ) {
      mapType = MapSettings::defaultMapSettingFromString( 
         (*paramsMap )[ "mt" ]->c_str() );
   }
   
   // ImageSettings (is)
   if ( paramsMap->find( "is" ) != paramsMap->end() ) {
      haveImageSettings = MapSettings::imageSettingsFromString( 
         *((*paramsMap )[ "is" ]), imageSettings );
   }
      
   // Authenticate (auth)
   if ( paramsMap->find( "auth" ) != paramsMap->end() ) {
      if ( ! HttpUtility::checkAuth( 
              (*paramsMap )[ "auth" ]->c_str() ) ) 
      {
         mc2log << error << "HttpMapFunctions::htmlMakeImage "
                << "checkHttpAuth failed, skipping request" << endl;
         delete cont;
         delete routePack;
         delete symbolMap;
         delete allowedMaps;
         return false;
      }
   }

   // Scalelevels (sl)
   if  ( paramsMap->find( "sl" ) != paramsMap->end() ) {
      const char* scaleLevelStr = (*paramsMap )[ "sl" ]->c_str();
      uint32 tmpMaxScale = 0;
      uint32 tmpMinScale = 0;
      if ( sscanf( scaleLevelStr, "%d_%d", 
                   &tmpMaxScale, &tmpMinScale ) == 2 ) 
      {
         maxScale = tmpMaxScale;
         minScale = tmpMinScale;
         filtScale = tmpMaxScale;
      }
   }
   
   // Map contents (map,topomap,poi,route)
   if  ( paramsMap->find( "map" ) != paramsMap->end() ) {
      const char* str = (*paramsMap )[ "map" ]->c_str();
      if ( str[ 0 ] == '0' ) {
         showMap = false;
      }
   }
   if  ( paramsMap->find( "topomap" ) != paramsMap->end() ) {
      const char* str = (*paramsMap )[ "topomap" ]->c_str();
      if ( str[ 0 ] == '0' ) {
         showTopographMap = false;
      }
   }
   if  ( paramsMap->find( "poi" ) != paramsMap->end() ) {
      const char* str = (*paramsMap )[ "poi" ]->c_str();
      if ( str[ 0 ] == '1' ) {
         showPOI = true;
      }
   }
   if  ( paramsMap->find( "route" ) != paramsMap->end() ) {
      const char* str = (*paramsMap )[ "route" ]->c_str();
      if ( str[ 0 ] == '0' ) {
         showRoute = false;
      }
   }
   if ( paramsMap->find( "scale" ) != paramsMap->end() ) {
      const char* str = (*paramsMap )[ "scale" ]->c_str();
      if ( str[ 0 ] != '0' ) {
         showScale = true;
      }
   }
   if ( paramsMap->find( "traffic" ) != paramsMap->end() ) {
      const char* str = (*paramsMap )[ "traffic" ]->c_str();
      if ( str[ 0 ] != '0' ) {
         showTraffic = true;
      }
   }
   if ( paramsMap->find( "cr" ) != paramsMap->end() ) {
      const char* str = (*paramsMap )[ "cr" ]->c_str();
      if ( str[ 0 ] == '4' ) {
         drawCopyRight = false;
      }
   }

   // Map symbols (ms)
   if ( paramsMap->find( "ms" ) != paramsMap->end() ) {
      const MC2String* str = paramsMap->find( "ms" )->second;
      symbolMap = new GfxFeatureMap();

      HttpUtility::readMapSymbols( str, symbolMap );
   }
   
   RedLineSettings redLineSettings;
   
   // Simple map. For java clients etc.
   if ( paramsMap->find( "simplemap" ) != paramsMap->end() ) {
      const char* str = (*paramsMap )[ "simplemap" ]->c_str();
      simpleMap = strtol( str, (char**)NULL, 10 );
      // If simplemap & 2 -> add intersecting roads
      redLineSettings.setIncludeConnectingRoads( simpleMap & 2 );
   }
   
   // Extract speed settings
   getSpeedSettings( redLineSettings,
                     paramsMap,
                     allData );
        
   // route (r)
   if  ( showRoute && paramsMap->find( "r" ) != paramsMap->end() ) {
      routeIDStr = (*paramsMap )[ "r" ] ->c_str();
      if ( sscanf( routeIDStr, "%X_%X", 
                   &routeID, &routeCreateTime ) == 2 &&
           routeID != 0 && routeCreateTime != 0 ) 
      {
         // Get stored route
         cont = myThread->getStoredRoute( routeID,
                                          routeCreateTime,
                                          routePack,
                                          UIN,
                                          extraUserinfo,
                                          validUntil,
                                          originLat,
                                          originLon,
                                          originMapID,
                                          originItemID,
                                          originOffset,
                                          destinationLat,
                                          destinationLon,
                                          destinationMapID,
                                          destinationItemID,
                                          destinationOffset );
         if ( cont != NULL ) {
            route = true;
         }
      }
   }
         
   // routeturn (rt)
   if ( showRoute && paramsMap->find( "rt" ) != paramsMap->end() ) {
      if ( sscanf( (*paramsMap )[ "rt" ]->c_str(),
                   "%X_%X", &beforeTurnFeatNbr, 
                   &afterTurnFeatNbr ) == 2 ) 
      {
         routeTurn = true;
      }
   }

   // routeturn (turn)
   if ( showRoute && paramsMap->find( "turn" ) != paramsMap->end() && 
        routePack != NULL )
   {
      int turnNbr = 0;
      if ( sscanf( (*paramsMap )[ "turn" ]->c_str(), "%X", &turnNbr )
           == 1 ) 
      {
         // Get ExpandItemID for route
         ExpandRouteRequest* req = 
            new ExpandRouteRequest( 
               myThread->getNextRequestID(),
               routePack, 
               ( ROUTE_TYPE_STRING |
                 ROUTE_TYPE_ITEM_STRING |
                 ROUTE_TYPE_GFX ),
               myVar->currentLanguage );
         myThread->putRequest( req );

         PacketContainer* ansCont = req->getAnswer();
         if ( ansCont != NULL && static_cast<ReplyPacket*>(
                 ansCont->getPacket() )->getStatus() == StringTable::OK )
         {
            ExpandItemID* exp = static_cast<ExpandRouteReplyPacket*>(
               ansCont->getPacket() )->getItemID();
            // Calculate beforeTurnFeatNbr, afterTurnFeatNbr
            uint32 beforeTurn = 0;
            uint32 afterTurn = 0;
            HttpUtility::turnItemIndexFromStringItemIndex(
               exp, turnNbr, turnNbr, beforeTurn, afterTurn );
            if ( beforeTurn != MAX_UINT32 && afterTurn != MAX_UINT32 ) {
               beforeTurnFeatNbr = beforeTurn;
               afterTurnFeatNbr = afterTurn;
               routeTurn = true;
            } else {
               mc2log << warn << "HttpMapFunctions::htmlMakeImage turn "
                  "parameter: coudn't get grouping from "
                  "expanded route" << endl;
            }
            delete exp;
         } else {
            mc2log << warn << "HttpMapFunctions::htmlMakeImage turn "
                   << "parameter: failed to expand route." << endl;
         }
         delete ansCont;
         delete req;
      }
   }

   const ClientSetting* clientSetting = NULL;
   if ( !setUserFromParams( paramsMap, myThread, myVar, 
                            userItem, clientSetting ) ) {
      delete cont;
      delete routePack;
      delete symbolMap;
      delete allowedMaps;
      return false;
   }

   if ( userItem != NULL ) {
      myVar->currentLanguage = userItem->getUser()->getLanguage();
      myThread->setLogUserName( userItem->getUser()->getLogonID() );
      if ( !myThread->getMapIdsForUserRegionAccess( 
              userItem->getUser(), allowedMaps ) )
      {
         mc2log << info << "HttpMapFunctions::htmlMakeImage "
                << "failed to get allowed mapIDs for user "
                << userItem->getUser()->getLogonID() << "(" 
                << userItem->getUIN() << ")" << endl;
      }
      myThread->setUser( userItem );
   }
   // Read the language. Convert back and forth from StringTable lang.
   {
      uint32 dist;
      myVar->currentLanguage =
         NameUtility::getBestLanguage(
            paramsMap->getISO639( "lang",
                                  ItemTypes::getLanguageCodeAsLanguageType(
                                     myVar->currentLanguage ) ),  dist );
   }
   
   // Make sure that the bbox is proportional with the width and height
   GfxUtility::getDisplaySizeFromBoundingbox( lla, llo,
                                              ula, ulo,
                                              w, h );   

   if( !allData ) {
      // Not all needed data
      mc2log << warn << "HttpMapFunctions::htmlMakeImage not all needed "
             << "params (bbox+w,h) present." << endl;
      myThread->setStatusReply( HttpCode::BAD_REQUEST );
   } else if ( ! simpleMap ) { // GfxFeatureMap
      MC2BoundingBox bbox;
      // must use update() here, so the min/max lat/lon gets corrected.
      // ( if needed )

      bbox.update( MC2BoundingBox( ula, llo,
                                   lla, ulo ) );

      if ( mapType != MapSettingsTypes::MAP_SETTING_STD ) {
         mapSettings = MapSettings::createDefaultMapSettings( mapType );
      } else { // STD
         mapSettings = new MapSettings();
      }
      if ( haveImageSettings ) {
         mapSettings->mergeImageSettings( imageSettings );
      }
      mapSettings->setMapContent( showMap, showTopographMap, 
                                  showPOI, showRoute );
      mapSettings->setDrawScale( showScale );
      mapSettings->setShowTraffic( showTraffic );

      if ( clientSetting != NULL ) {
         mapSettings->setImageSet( clientSetting->getImageSet() );
      }

      MC2BoundingBox ccBBox;

      GfxFeatureMapImageRequest* req = 
         new GfxFeatureMapImageRequest( 
            myThread->getNextRequestID(), &bbox, w, h,
            ItemTypes::getLanguageCodeAsLanguageType( myVar->currentLanguage ),
            routePack, !binaryMap, exportFormat, drawCopyRight,
            format, size, mapSettings, myThread->getTopRegionRequest(),
            ccBBox, maxScale, minScale, filtScale );

      // if not "binary" format, then fetch copyright strings and
      // determine the final copyright string
      if ( ! binaryMap ) {
         MC2String copyrightString = 
            myThread->getGroup()->
            getCopyright( myThread, bbox, 
                          LangTypes::english );
       
         req->setCopyright( copyrightString );
      }

      // TODO: Add positionArc to GfxFeatureMapImageRequest
      if ( routeTurn ) {
         req->setRouteTurn( beforeTurnFeatNbr, afterTurnFeatNbr );
      }
      if ( symbolMap != NULL ) {
         if ( symbolMap->getNbrFeatures() > 0 ) {
            const GfxSymbolFeature* feat = 
               static_cast<const GfxSymbolFeature*> ( 
                  symbolMap->getFeature( 0 ) );
            if ( feat->getNbrPolygons() > 0 ) {
               GfxPolygon* poly = feat->getPolygon( 0 );
               if ( poly->getNbrCoordinates() > 0 ) {
                  posCoord = MC2Coordinate(
                     poly->getLat( 0 ), poly->getLon( 0 ) );
               }
            }
         }
         req->addSymbolMap( symbolMap );
      }
      // The maps allowed to show
      req->setAllowedMaps( allowedMaps );
      // Set the user (may be NULL)
      req->setUser( myThread->getCurrentUser() );

      myThread->putRequest( req );
   
      PacketContainer* ansCont = req->getAnswer();
      if ( ansCont != NULL &&
           static_cast< ReplyPacket* > (
              ansCont->getPacket() )->getStatus() == 
           StringTable::OK ) 
      {
         DEBUG8(cerr << "GfxFeatureMapImageReplyPacket ok" << endl;);
       
         uint32 size = 0;
         if( exportFormat && binaryMap ) {   //GfxExportFeatureMap
            GfxFeatureMapImageReplyPacket* ans = static_cast<
               GfxFeatureMapImageReplyPacket* >( ansCont->getPacket() );
            byte* exportMapData = ans->getGfxExportFeatureMapData();
            uint32 exprotMapDataBufferSize = ans->getSize();
            size = exprotMapDataBufferSize;

            outBody->setBody( exportMapData, 
                              exprotMapDataBufferSize );
               
            delete [] exportMapData;
         } else if( !binaryMap ) {  //Image
            GfxFeatureMapImageReplyPacket* ans = static_cast<
               GfxFeatureMapImageReplyPacket* >( ansCont->getPacket() );
            imageBuff = ans->getImageData();
            imageSize = ans->getSize();
            size = imageSize;
               
            outBody->setBody( imageBuff, imageSize );
            delete [] imageBuff;
         } else {   //GfxFeatureMap
            GfxFeatureMapReplyPacket* ans = static_cast<
               GfxFeatureMapReplyPacket* >( ansCont->getPacket() );

            DataBuffer* ansMapData = ans->getGfxFeatureMapData();
            size = ansMapData->getCurrentOffset();

            outBody->setBody( ansMapData->getBufferAddress(), 
                              ansMapData->getCurrentOffset() );
               
            delete ansMapData;
         }

         if ( userItem != NULL ) {
            const char* extraInfo = NULL;
            if ( inHead->getHeaderValue( &USERAGENT ) != NULL ) {
               extraInfo = inHead->getHeaderValue( &USERAGENT )
                  ->c_str();
            }
            // Debit map
            // Set debitamount here untill module sets it ok
            uint32 debitAmount = 
               ( TimeUtility::getCurrentTime() - 
                 myThread->getCurrRequestStartTime() ) * 1000;
            myThread->getDebitHandler()->makeMapDebit( 
               userItem, extraInfo, size, debitAmount, bbox, 
               inHead->getStartLine()->c_str(),
               w, h, !binaryMap, format, routeID, routeCreateTime,
               mapSettings->getShowMap(), mapSettings->getShowPOI(), 
               mapSettings->getShowTopographMap(), 
               mapSettings->getShowRoute(), 
               mapSettings->getShowTraffic(), posCoord );
         }
         ok = true;
      } else {
         uint16 statusCode = HttpCode::INTERNAL_ERROR;
         mc2log << warn << "htmlMakeImage GfxFeatureMapRequest failed ";
         mc2log << "Status: " << StringTable::getString( 
            req->getStatus(), StringTable::ENGLISH ) << " (" 
                << int(req->getStatus()) << ") Code: ";
         if ( ansCont == NULL ) {
            statusCode = HttpCode::SERVICE_UNAVAILABLE;
         }
         mc2log << statusCode << endl;
         myThread->setStatusReply( statusCode );
      }

      delete ansCont;
      delete req;
      delete mapSettings;
   } else if ( simpleMap >= 1 && simpleMap <= 2  ) {
      // Only version 1-2 supported
      // GfxFeatureMap, but simple map, i.e. a filtered version
      // of the GfxFeatureMap is used, and the binary format is 
      // simple.
      MC2BoundingBox bbox( ula, llo,
                           lla, ulo );
         
      // Create the mapdesc.
      ServerTileMapFormatDesc mapDesc( STMFDParams( LangTypes::english, false ) );
      mapDesc.setData();

      // Max scale now means pixel-to-meter factor.
      FilteredGfxMapRequest* req = 
         new FilteredGfxMapRequest( myThread->getNextRequestID(),
                                    redLineSettings,
                                    mapDesc,
                                    bbox,
                                    TileMapTypes::c_routeLayer,
                                    0, // Importance nbr 0.
                                    maxScale,
                                    routePack,
                                    myThread->getTopRegionRequest(),
                                    true ); // Remove names.

      // The maps allowed to show. Don't think we need this.
      //req->setAllowedMaps( allowedMaps );

      DEBUG8(cerr << "About to send FilteredGfxMapRequest" << endl;);
      DEBUG8(uint32 startTime = TimeUtility::getCurrentTime(); );
      myThread->putRequest( req );
      DEBUG8(uint32 endTime = TimeUtility::getCurrentTime(););
      DEBUG8(cerr << "FilteredGfxMapRequest retuned process time " 
             << (endTime - startTime) << "ms" << endl;);
   
      if ( req->getStatus() == StringTable::OK ) 
      {
         DEBUG8(cerr << "FilteredGfxMapRequest reply ok" << endl;);
       
         uint32 size = 0;
         const GfxFeatureMap* gfxMap = req->getGfxFeatureMap();
         MC2_ASSERT( gfxMap != NULL );

         // Change the version from 1 or 2 to 1
         byte* simpleMapData = gfxMap->getAsSimpleMapData( 1,
                                                           size );
            
         MC2_ASSERT( simpleMapData != NULL );

         outBody->setBody( simpleMapData, 
                           size );
               
         delete [] simpleMapData;

//            if ( userItem != NULL ) {
//               const char* extraInfo = NULL;
//               if ( inHead->getHeaderValue( &USERAGENT ) != NULL ) {
//                  extraInfo = inHead->getHeaderValue( &USERAGENT )
//                     ->c_str();
//               }
//               // Debit map
//               // Set debitamount here untill module sets it ok
//               uint32 debitAmount = 
//                  ( TimeUtility::getCurrentTime() - 
//                    myThread->getCurrRequestStartTime() ) * 1000;
//               myThread->makeMapDebit( 
//                  userItem, extraInfo, size, debitAmount, bbox, 
//                  inHead->getStartLine()->c_str(),
//                  w, h, !binaryMap, format, routeID, routeCreateTime,
//                  mapSettings->getShowMap(), mapSettings->getShowPOI(), 
//                  mapSettings->getShowTopographMap(), 
//                  mapSettings->getShowRoute(), 
//                  mapSettings->getShowTraffic(), posCoord );
//            }
         ok = true;
      } else {
         uint16 statusCode = HttpCode::INTERNAL_ERROR;
         mc2log << warn << "htmlMakeImage FilteredGfxMapRequest failed ";
         mc2log << "Status: " << StringTable::getString( 
            req->getStatus(), StringTable::ENGLISH ) << " ("
                << req->getStatus() << ")" << endl;
         myThread->setStatusReply( statusCode );
      }

      delete req;      
   }

   if ( userItem != NULL ) {
      myThread->setUser( NULL );
   }
   myThread->releaseUserItem( userItem );
   
   delete cont;
   delete routePack;
   delete symbolMap;
   delete allowedMaps;

   return ok;
}


bool 
HttpMapFunctions::htmlShowLocalMap( stringVector* params,
                                    int paramc, 
                                    stringMap* paramsMap,
                                    HttpHeader* inHead, 
                                    HttpHeader* outHead,
                                    HttpBody* inBody,
                                    HttpBody* outBody,
                                    HttpParserThread* myThread,
                                    HttpVariableContainer* myVar )
{
   myThread->addRequestName( "SHOW_LOCAL_MAP" );
   bool allData = true;

   int32 lla = 0;
   int32 llo = 0;
   int32 ula = 0;
   int32 ulo = 0;
   uint16 w = 0;
   uint16 h = 0;
   char* tmpStr = NULL;
   GfxFeatureMap* symbolMap = new GfxFeatureMap();
   const char* localMapString = NULL;
   const char* signature = NULL;


   //** Indata
   // lla
   if ( paramsMap->find( "lla" ) != paramsMap->end() ) {
      lla = strtol( paramsMap->find( "lla" )->second->c_str(), NULL, 10 );
   } else {
      allData = false;
   }

   // llo
   if ( paramsMap->find( "llo" ) != paramsMap->end() ) {
      llo = strtol( paramsMap->find( "llo" )->second->c_str(), NULL, 10 );
   } else {
      allData = false;
   }

   // ula
   if ( paramsMap->find( "ula" ) != paramsMap->end() ) {
      ula = strtol( paramsMap->find( "ula" )->second->c_str(), NULL, 10 );
   } else {
      allData = false;
   }

   // ulo
   if ( paramsMap->find( "ulo" ) != paramsMap->end() ) {
      ulo = strtol( paramsMap->find( "ulo" )->second->c_str(), NULL, 10 );
   } else {
      allData = false;
   }

   // us
   if ( paramsMap->find( "us" ) != paramsMap->end() ) {
      // Get user settings
      if ( HttpUtility::parseUserSettings( (*paramsMap)["us"]->c_str(),
                                           myVar->currentLanguage ) ) 
      {
         
      } else {
         allData = false;
      }
   } else {
      allData = false; 
   }

   //** Optional stuff 
   // w
   if ( paramsMap->find( "w" ) != paramsMap->end() ) {
      uint32 width = strtoul( paramsMap->find( "w" )->second->c_str(), 
                              &tmpStr, 10 );
      if ( tmpStr != NULL && tmpStr[ 0 ] == '\0' ) {
         w = width;
      }
   }
   
   // h
   if ( paramsMap->find( "h" ) != paramsMap->end() ) {
      uint32 height = strtoul( paramsMap->find( "h" )->second->c_str(), 
                               &tmpStr, 10 );
      if ( tmpStr != NULL && tmpStr[ 0 ] == '\0' ) {
         h = height;
      }
   }
   
   // all ms
   if ( paramsMap->find( "ms" ) != paramsMap->end() ) {
      const MC2String* str = paramsMap->find( "ms" )->second;
      HttpUtility::readMapSymbols( str, symbolMap );
   }

   // Adjust coordinates see HttpUtility::makeLocalMapLink
   llo = ulo - llo;
   lla = ula - lla;
   for ( uint32 i = 0 ; i < symbolMap->getNbrFeatures() ; i++ ) {
      if ( symbolMap->getFeature( i )->getType() == GfxFeature::SYMBOL )
      {
         const GfxSymbolFeature* feat = static_cast< 
            const GfxSymbolFeature* > ( symbolMap->getFeature( i ) );
         
         feat->getPolygon( 0 )->setStartLat( 
            ula - feat->getPolygon( 0 )->getLat( 0 ) );
         feat->getPolygon( 0 )->setStartLon( 
            ulo - feat->getPolygon( 0 )->getLon( 0 ) );
      }
   }

   // localMapString (lm)
   if ( paramsMap->find( "lm" ) != paramsMap->end() ) {
      localMapString = (*paramsMap)[ "lm" ]->c_str();
   } else {
      localMapString = "";
   }

   // Signature (sig)
   if ( paramsMap->find( "sig" ) != paramsMap->end() ) {
      signature = (*paramsMap)[ "sig" ]->c_str();
   } else {
      signature = "";
   }

   const ClientSetting* clientSetting = NULL;
   if ( paramsMap->find( "c" ) != paramsMap->end() ) {
      clientSetting = getClientSettingFromParams( paramsMap, myThread );
   }

   if ( allData ) {
      ImageDrawConfig::imageFormat imageFormat = 
         HttpUtility::getImageFormatForRequest( 
            inHead, ImageDrawConfig::PNG );

      MC2String copyright = myThread->getGroup()->
         getCopyright( myThread, MC2BoundingBox( ula, llo, lla, ulo ),
                       LangTypes::english );

      LocalMapMessageRequest* localMapReq = new LocalMapMessageRequest(
         myThread->getNextRequestID(),
         ula, llo, lla, ulo,
         myVar->currentLanguage, 500, 500,
         localMapString, myThread->getTopRegionRequest(),
         clientSetting,
         symbolMap, 
         false, false, true, RouteMessageRequestType::HTML_CONTENT, 
         MAX_UINT32, false, NULL, NULL, NULL, signature,
         imageFormat, copyright );
      
      const MimeMessage* message = localMapReq->getMimeMessage();
      if ( message->getNbrMimeParts() >= 1 ) {
         const MimePart* page = message->getMimePart( 0 );
         uint32 size = 0;
         byte* buff = page->getUnencodedContent( size );
         if ( buff != NULL ) {
            buff[ size ] = '\0';
            MC2String data( (char*)buff );
            outBody->setBody( &data );
         }
         
         delete [] buff;
      }
      
      delete localMapReq;
   } else {
      outBody->addString( "Not enough data." );
   }


   delete symbolMap;

   return true;
}


bool
HttpMapFunctions::setUserFromParams( stringMap* paramsMap,
                                     HttpParserThread* myThread,
                                     HttpVariableContainer* myVar,
                                     UserItem*& userItem,
                                     const ClientSetting*& clientSetting )
{
   const char* sessionID = NULL;
   const char* sessionKey = NULL;

   if ( paramsMap->find( "sesi" ) != paramsMap->end() ) {
      sessionID = (*paramsMap )[ "sesi" ]->c_str();
   }
   if ( paramsMap->find( "si" ) != paramsMap->end() ) {
      sessionID = (*paramsMap )[ "si" ]->c_str();
   }
   if ( paramsMap->find( "sesk" ) != paramsMap->end() ) {
      sessionKey = (*paramsMap )[ "sesk" ]->c_str();
   }
   if ( paramsMap->find( "sk" ) != paramsMap->end() ) {
      sessionKey = (*paramsMap )[ "sk" ]->c_str();
   }
   
   clientSetting = getClientSettingFromParams( paramsMap, myThread );

   const ParserExternalAuthHttpSettings* peh = NULL;
   if ( clientSetting != NULL ) {
      peh = myThread->getExternalAuth()->getNavHttpSetting( 
         clientSetting->getClientType() );
   }

   // userSession (sesi,sesk)
   if ( sessionID != NULL && sessionKey != NULL ) {
      if ( !myThread->getUserBySession( sessionID, sessionKey,
                                        userItem, true /*useCache*/ ) ||
           userItem == NULL )
      {
         mc2log << info << "HttpMapFunctions::htmlMakeImage failed to "
                << "get user from session. ID: " 
                << sessionID << " key: " << sessionKey << endl;
      }
   }

   if ( userItem == NULL && 
        paramsMap->find( "idk" ) != paramsMap->end() ) 
   {
      uint32 nbrUsers = 0;
      UserIDKey userIDKey( MAX_UINT32 );
      userIDKey.setIDKey( (*paramsMap )[ "idk" ]->c_str() );
      userIDKey.setIDType( UserIDKey::account_id_key );
      bool commOk = myThread->getUserFromUserElement( 
         &userIDKey, userItem, nbrUsers, true );
      if ( !commOk || userItem == NULL ) {
         mc2log << info << "HttpMapFunctions::htmlMakeImage failed to "
                << "get user from id-key. IDKey: " 
                << (*paramsMap )[ "idk" ]->c_str() << endl;
      }
   }

   // Hardware parameter
   if ( userItem == NULL && peh == NULL /*And not External auth*/
        && paramsMap->find( "hwd" ) != paramsMap->end() ) {
      UserLicenceKeyVect hwKeys;
      UserLicenceKey hwKey;
      // For all hwd
      const MC2String* hwParam = (*paramsMap)[ "hwd" ];
      const MC2String* hwtParam = NULL;
      uint32 hwPos = 0;
      uint32 hwtPos = 0;
      if ( paramsMap->find( "hwdt" ) != paramsMap->end() ) {
         hwtParam = (*paramsMap)[ "hwdt" ];
      }
      const char* hwStr = HttpHeader::getFirstStringIn( hwParam, hwPos );
      const char* hwtStr = NULL;
      if ( hwtParam != NULL ) {
         hwtStr = HttpHeader::getFirstStringIn( hwtParam, hwtPos );
      }
      while ( hwStr != NULL ) {
         hwKeys.push_back( UserLicenceKey( MAX_UINT32 ) );
         hwKeys.back().setProduct( clientSetting->getProduct() );
         if ( hwtStr != NULL ) {
            hwKeys.back().setKeyType( hwtStr );            
            hwtStr = HttpHeader::getNextStringIn( hwtParam, hwtPos );
         }
         hwKeys.back().setLicence( hwStr );

         hwStr = HttpHeader::getNextStringIn( hwParam, hwPos );
      }
      // Find and set best
      myThread->getUserHandler()->setBestLicenceKey( hwKeys, hwKey );
      // Find user for best key
      uint32 nbrUsers = 0;
      bool commOk = myThread->getUserHandler()->getUserFromLicenceKeys(
         hwKeys, userItem, nbrUsers );
      if ( !commOk || userItem == NULL ) {
         mc2log << info << "HttpMapFunctions::htmlMakeImage failed to "
                << "get user from licenceKey: " 
                << hwKey << endl;
      }
      if ( paramsMap->find( "uin" ) != paramsMap->end() ) {
         uint32 UIN = strtoul( (*paramsMap )[ "uin" ]->c_str(), NULL, 0 );
         if ( userItem != NULL && userItem->getUIN() != UIN ) {
            mc2log << info << "HttpMapFunctions::htmlMakeImage param UIN "
                   << UIN << " not hardware owner " 
                   << userItem->getUser()->getLogonID() << "("
                   << userItem->getUIN() << ") Of key " << hwKey << endl;
         }
      }
   } // End if there is a hwd parameter

   if ( userItem == NULL && paramsMap->find( "uin" ) != paramsMap->end() &&
        paramsMap->find( "tok" ) != paramsMap->end() )
   {
      uint32 UIN = strtoul( (*paramsMap )[ "uin" ]->c_str(), NULL, 0 );
      if ( myThread->getUser( UIN, userItem, true  ) && userItem != NULL ) 
      {
         if ( myThread->getTokenHandler()->getToken(
                 userItem->getUser(), (*paramsMap )[ "tok" ]->c_str(), 
                 NULL/*No client type*/ ) == NULL ) {
            mc2log << info << "HttpMapFunctions::htmlMakeImage user "
                   << userItem->getUser()->getLogonID() << "(" << UIN 
                   << ") doesn't have token " 
                   << (*paramsMap )[ "tok" ]->c_str() << endl;
         }
      } else {
         mc2log << info << "HttpMapFunctions::htmlMakeImage failed to "
                << "get user from UIN " << UIN << "("
                << (*paramsMap )[ "uin" ]->c_str() << ") and token " 
                << (*paramsMap )[ "tok" ]->c_str() << endl;
      }
   }

   // The Java Maplib idkey
   if ( userItem == NULL && paramsMap->find( "uin" ) != paramsMap->end() &&
        paramsMap->find( "mlk" ) != paramsMap->end() ) {
      uint32 UIN = strtoul( (*paramsMap )[ "uin" ]->c_str(), NULL, 0 );
      if ( myThread->getUser( UIN, userItem, true  ) && userItem != NULL ) {
         
         if ( myThread->getUserHandler()->getIDKey(
                 userItem->getUser(), UserIDKey::account_id_key,
                 MC2String( "mlk-" ) + *(*paramsMap )[ "mlk" ] ) == NULL ) {
            mc2log << info << "HttpMapFunctions::htmlMakeImage user "
                   << userItem->getUser()->getLogonID() << "(" << UIN 
                   << ") doesn't have idkey " 
                   << (*paramsMap )[ "mlk" ]->c_str() << endl;
            // Set error as mlk is not forgiving
            myThread->setStatusReply( HttpCode::FORBIDDEN );
            myThread->releaseUserItem( userItem );
            return false;
         }
      } else {
         mc2log << info << "HttpMapFunctions::htmlMakeImage failed to "
                << "get user from UIN " << UIN << "("
                << (*paramsMap )[ "uin" ]->c_str() << ") and mlk " 
                << (*paramsMap )[ "mlk" ]->c_str() << endl;
         // Set error as mlk is not forgiving
         myThread->setStatusReply( HttpCode::FORBIDDEN );
         myThread->releaseUserItem( userItem );
         return false;
      }
   }


   // Last way out is to trust client and use it's uin directly without any
   // checks
   if ( userItem == NULL && paramsMap->find( "uin" ) != paramsMap->end() ) {
      uint32 UIN = strtoul( (*paramsMap )[ "uin" ]->c_str(), NULL, 0 );
      if ( myThread->getUser( UIN, userItem, true  ) && userItem != NULL ) {
         // Ok, use it.
      } else {
         mc2log << info << "HttpMapFunctions::htmlMakeImage failed to "
                << "get user from UIN " << UIN << "("
                << (*paramsMap )[ "uin" ]->c_str() << ")" << endl;
      }
   }

   if ( userItem != NULL ) {
      myVar->currentLanguage = userItem->getUser()->getLanguage();
      myThread->setLogUserName( userItem->getUser()->getLogonID() );
   }

   return true;
}

const ClientSetting* 
HttpMapFunctions::getClientSettingFromParams( stringMap* paramsMap,
                                              HttpParserThread* myThread ) {
   // c param for client type
   MC2String clientType;
   if ( paramsMap->find( "c" ) != paramsMap->end() ) {
      clientType = *(*paramsMap)[ "c" ];
   }
   return myThread->getGroup()->getSetting( clientType.c_str(), 
                                            "" ); // clientTypeOptions
}

bool
HttpMapFunctions::readProjectionParameters( stringMap* paramsMap,
                                            int& x,
                                            int& y,
                                            int& zoom,
                                            MC2String& lang )
{
   map<MC2String, MC2String*>::iterator it;
   x = MAX_INT32;
   y = MAX_INT32;
   zoom = MAX_INT32;
   lang = "";
   for( it = paramsMap->begin(); it != paramsMap->end(); ++it ) {
      if( it->first == "x" ) {
         MC2String xStr = *(it->second);
         x = atoi(xStr.c_str());
      } else if( it->first == "y" ) {
         MC2String yStr = *(it->second);
         y = atoi(yStr.c_str());
      } else if( it->first == "zoom" ) {
         MC2String zoomStr = *(it->second);
         zoom = atoi(zoomStr.c_str());
      } else if( it->first == "lang" ) {
         lang = *(it->second);
      }
   }
   if( (x != MAX_INT32) && (y != MAX_INT32) && (zoom != MAX_INT32) ) {
      return true;
   } else {
      return false;
   }
}

bool
HttpMapFunctions::processProjectionRequest(
                                  DrawingProjection* projection,
                                  const LangType& language,
                                  HttpHeader* inHead,
                                  HttpBody* inBody,
                                  stringMap* paramsMap,
                                  HttpHeader* outHead,
                                  HttpBody* outBody,
                                  HttpParserThread* myThread,
                                  uint32 now,
                                  stringVector* params,
                                  uint32 zoom )
{
   auto_ptr<MapSettings> mapSettings ( new MapSettings() );
   struct MapSettingsTypes::ImageSettings imageSettings;
   bool haveImageSettings = MapSettings::imageSettingsFromString( 
      "%FF%FC%02", imageSettings );
   if( haveImageSettings )
      mapSettings->mergeImageSettings( imageSettings );
   mapSettings->setMapContent( true/*showMap*/, true/*showTopographMap*/,
                               false/*showPOI*/, false/*showRoute*/ );
   mapSettings->setDrawScale( false );
   mapSettings->setShowTraffic( false );
   mapSettings->setDrawingProjection( projection );
   
   MC2BoundingBox box = projection->getLargerBoundingBox();
   uint32 nbrPixels = ProjectionSettings::getPixelSize( *params );

   MC2BoundingBox
      ccBBox( box.getMaxLat() + projection->getLatDiff( nbrPixels ),
              box.getMinLon() - projection->getLonDiff( nbrPixels ),
              box.getMinLat() - projection->getLatDiff( nbrPixels ),
              box.getMaxLon() + projection->getLonDiff( nbrPixels ) );
   
   auto_ptr<GfxFeatureMapImageRequest> gfxRequest (
      new GfxFeatureMapImageRequest( myThread->getNextRequestID(),
                                     &box,
                                     nbrPixels, nbrPixels,
                                     language,
                                     NULL/*routePack*/, true, false,
                                     false, ImageDrawConfig::PNG,
                                     17971, mapSettings.get(),
                                     myThread->getTopRegionRequest(),
                                     ccBBox,
                                     projection->getScaleLevel(),
                                     CONTINENT_LEVEL,
                                     INVALID_SCALE_LEVEL,
                                     false, // dont extract for tiles
                                     zoom // projection zoom
                                     ) );

   // Set the user (may be NULL) Might not be needed as no POIs, but JIC
   gfxRequest->setUser( myThread->getCurrentUser() );
      
   myThread->putRequest( gfxRequest.get() );
   
   if( gfxRequest->getStatus() == StringTable::OK ) {
      // Must be deleted by getter
      auto_ptr<PacketContainer> ansCont ( gfxRequest->getAnswer() );
      
      GfxFeatureMapImageReplyPacket* replyPacket =
         static_cast<GfxFeatureMapImageReplyPacket*>(ansCont->getPacket());
      
      ScopedArray<byte> imageBuff( replyPacket->getImageData() );
      uint32 imageSize = replyPacket->getSize();
      
      outBody->setBody( imageBuff.get(), imageSize );
      
      MC2String contentType("Content-Type");
      outHead->addHeaderLine(&contentType,
                             new MC2String("image/png") );
      
      return true;
   }
   return false;
}

void
HttpMapFunctions::setProjectionCacheHeaders( HttpHeader* outHead,
                                             HttpParserThread* myThread,
                                             uint32 date,
                                             uint32 modifyTime )
{
   MC2String typeString( "" );
   char dateStr[1024];
   
   // Set expires date to two weeks into the future to get page cached
   const uint32 CACHE_TIME = (3600*24*14);
   typeString = "Expires";
   myThread->makeDateStr( dateStr, date + CACHE_TIME );
   outHead->addHeaderLine(&typeString, new MC2String(dateStr));

   // Set Last-modified
   typeString = "Last-modified";
   myThread->makeDateStr( dateStr, modifyTime );
   outHead->addHeaderLine( typeString, dateStr );
   
   // Set Cache-Control to public
   typeString = "Cache-Control";
   MC2String ccv/*( "public" )*/;
   ccv.append( "max-age=" );
   STLStringUtility::uint2str( CACHE_TIME, ccv );
   outHead->addHeaderLine( &typeString, 
                           new MC2String( ccv ) );
}

const uint32 ETAG_VERSION = 12;
const uint32 MODIFY_TIME = 1072094201 + ETAG_VERSION*3600;

bool
HttpMapFunctions::gmap( stringVector* params,
                        int paramc, 
                        stringMap* paramsMap,
                        HttpHeader* inHead, 
                        HttpHeader* outHead,
                        HttpBody* inBody,
                        HttpBody* outBody,
                        HttpParserThread* myThread,
                        HttpVariableContainer* myVar )
{
   myThread->addRequestName( "GMAP" );
   uint32 date = TimeUtility::getRealTime();
   return HttpMapFunctions::handleProjectionMapRequest(
      DrawingProjection::braunProjection,
      inHead, inBody, paramsMap,
      outHead, outBody, myThread,
      date, date/*MODIFY_TIME*/,
      ETAG_VERSION,
      params );
}

bool
HttpMapFunctions::mmap( stringVector* params,
                        int paramc, 
                        stringMap* paramsMap,
                        HttpHeader* inHead, 
                        HttpHeader* outHead,
                        HttpBody* inBody,
                        HttpBody* outBody,
                        HttpParserThread* myThread,
                        HttpVariableContainer* myVar )
{
   myThread->addRequestName( "MMAP" );
   uint32 date = TimeUtility::getRealTime();
   return HttpMapFunctions::handleProjectionMapRequest(
         DrawingProjection::mercatorProjection,
         inHead, inBody, paramsMap,
         outHead, outBody, myThread,
         date, date/*MODIFY_TIME*/,
         ETAG_VERSION,
         params );
}

bool
HttpMapFunctions::zoomSettings( stringVector* params,
                        int paramc, 
                        stringMap* paramsMap,
                        HttpHeader* inHead, 
                        HttpHeader* outHead,
                        HttpBody* inBody,
                        HttpBody* outBody,
                        HttpParserThread* myThread,
                        HttpVariableContainer* myVar )
{
   myThread->addRequestName( "PROJ_SETTINGS" );
   return HttpProjectionFunctions::
      handleProjectionSettingsRequest(
         params, inHead, inBody, paramsMap,
         outHead, outBody, TimeUtility::getRealTime() );
}

// Dissabling ETag for now as it is a constant and doesn't reflect actual
// changed or not
#undef ENABLE_ETAG
#ifdef ENABLE_ETAG
static MC2String makeProjETag( DrawingProjection::projection_t projectionType,
                               int x,
                               int y,
                               int zoom,
                               LangTypes::language_t lang)
{
   char temp[1024];
   const char* mapt = "";
   if( projectionType == DrawingProjection::mercatorProjection ) {
      mapt = "MMap";
   } else if( projectionType == DrawingProjection::braunProjection ) {
      mapt = "GMap";
   }
   const char* lang_str = LangTypes::getLanguageAsISO639( lang );
   sprintf( temp, "\"%s?x=%d&y=%d&zoom=%d&lang=%s_%d\"",
            mapt, x, y, zoom, lang_str, ETAG_VERSION );
   return temp;
}
#endif


bool
HttpMapFunctions::handleProjectionMapRequest( 
                   DrawingProjection::projection_t projectionType,
                   HttpHeader* inHead,
                   HttpBody* inBody,
                   stringMap* paramsMap,
                   HttpHeader* outHead,
                   HttpBody* outBody,
                   HttpParserThread* myThread,
                   uint32 now,
                   const uint32 modifyTime,
                   const uint32 etagVersion,
                   stringVector* params )
{
   // FIXME: Use SimpleProjParams instead
   int x, y, zoom;
   MC2String lang;
   bool parametersOK =
      readProjectionParameters( paramsMap, x, y, zoom, lang );

   LangTypes::language_t langType = LangTypes::invalidLanguage;
   if ( ! lang.empty() ) {
      langType = LangTypes::getISO639AsLanguage( lang.c_str() );
      if ( langType == LangTypes::invalidLanguage ) {
         langType = LangTypes::getISO639AndDialectAsLanguage( lang.c_str() );
      }
   }

   if ( parametersOK ) {
      // Check If-Modified-Since
      const MC2String* ifModifiedSince =
         inHead->getHeaderValue( "If-Modified-Since" );
      if( ifModifiedSince != NULL ) {
         uint32 time =
            myThread->dateStrToInt( (*ifModifiedSince).c_str() );
         if( time >= modifyTime ) {
            // Set code 304
            myThread->setStatusReply( HttpCode::NOT_MODIFIED );
            return false;
         }
      }

// Dissabling ETag for now as it is a constant and doesn't reflect actual
// changed or not
#ifdef ENABLE_ETAG
      // Check If-None-Match
      const MC2String* ifNoneMatch =
         inHead->getHeaderValue( "If-None-Match" );
      if( ifNoneMatch != NULL ) {         
         MC2String etag = makeProjETag( projectionType, x, y, zoom, langType );
         if( *ifNoneMatch == etag ) {
            // Set code 304
            myThread->setStatusReply( HttpCode::NOT_MODIFIED );
            return false;
         } 
      }
#endif
      uint32 nbrPixels = ProjectionSettings::getPixelSize( *params );

      auto_ptr<DrawingProjection> projection;
      if( projectionType == DrawingProjection::mercatorProjection ) {
         projection.
            reset( new MercatorProjection( x, y, zoom, nbrPixels ) );
      } else if( projectionType ==
                 DrawingProjection::braunProjection ) {
         projection
            .reset( new BraunProjection( x, y, zoom, nbrPixels ) );

      } else {
         // Should not happen, set code to 404
         myThread->setStatusReply( HttpCode::NOT_FOUND );
         return false;
      }
      projection->init();
      
      if( projection->getStatus() == StringTableUTF8::OK ) {
         bool ok = processProjectionRequest( projection.release(),
                                             langType,
                                             inHead,
                                             inBody,
                                             paramsMap,
                                             outHead,
                                             outBody,
                                             myThread,
                                             now,
                                             params,
                                             zoom );
         if( ok ) {
            // Set the cache headers
            setProjectionCacheHeaders( outHead, myThread,
                                       now, modifyTime );

// Dissabling ETag for now as it is a constant and doesn't reflect actual
// changed or not
#ifdef ENABLE_ETAG
            // Set the ETag header
            outHead->addHeaderLine( "ETag",
                                    makeProjETag( projectionType, x, y, zoom,
                                                  langType ) );
#endif

            return true;
         } else {
            // Set error code 503
            MC2String typeString = "Cache-Control";
            outHead->addHeaderLine(&typeString,
                                   new MC2String( "no-cache" ) );
            
            myThread->setStatusReply( HttpCode::SERVICE_UNAVAILABLE );
            return false;
         }
      } else {
         // Set error code 404
         myThread->setStatusReply( HttpCode::NOT_FOUND );
         return false;
      }
   }
   // Set error code 404
   myThread->setStatusReply( HttpCode::NOT_FOUND );
   return false;
}
