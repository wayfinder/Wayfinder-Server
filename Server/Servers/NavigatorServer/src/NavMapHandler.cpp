/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavMapHandler.h"
#include "InterfaceParserThread.h"
#include "NavPacket.h"
#include "UserData.h"
#include "NavUtil.h"
#include "MC2Coordinate.h"
#include "MapSettings.h"
#include "ExpandRouteRequest.h"
#include "HttpUtility.h"
#include "ExpandItemID.h"
#include "ExpandStringItem.h"
#include "NavMapHelp.h"
#include "GfxUtility.h"
#include "GfxConstants.h"
#include "GfxFeatureMapImageRequest.h"
#include "GfxFeatureMapImagePacket.h"
#include "NavUserHelp.h"
#include "ExpandRoutePacket.h"
#include "RoutePacket.h"
#include "DataBuffer.h"
#include "ClientSettings.h"
#include "ParserDebitHandler.h"

NavMapHandler::NavMapHandler( InterfaceParserThread* thread,
                              NavParserThreadGroup* group,
                              NavUserHelp* userHelp )
      : NavHandler( thread, group ),
        m_mapHelp( new NavMapHelp( thread, group ) ),
        m_userHelp( userHelp )
{
   m_expectations.push_back( 
      ParamExpectation( 1600, NParam::Int32_array, 16, 16 ) );
   m_expectations.push_back(
      ParamExpectation( 1601, NParam::Int32_array, 8, 8 ) );
   m_expectations.push_back( ParamExpectation( 1602, NParam::Uint16 ) );
   m_expectations.push_back( ParamExpectation( 1603, NParam::Uint16 ) );
   m_expectations.push_back( ParamExpectation( 1604, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 1605, NParam::String ) );
   m_expectations.push_back( 
      ParamExpectation( 1606, NParam::Uint16_array, 4, 4 ) );
   m_expectations.push_back( 
      ParamExpectation( 1607, NParam::Uint16_array, 4, 4 ) );
   m_expectations.push_back( ParamExpectation( 1608, NParam::Byte ) );
   m_expectations.push_back( 
      ParamExpectation( 1609, NParam::Byte_array, 10, 10 ) );
   m_expectations.push_back( 
      ParamExpectation( 1610, NParam::Byte_array, 6, 6 ) );

   // Vector Map
   m_expectations.push_back( ParamExpectation( 2400, NParam::String ) );
   // Multi Vector Map
   m_expectations.push_back( ParamExpectation( 4600, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 4601, NParam::Uint32 ) );
   m_expectations.push_back( ParamExpectation( 4602, 
                                               NParam::Byte_array ) );
}


NavMapHandler::~NavMapHandler() {
   delete m_mapHelp;
}


bool
NavMapHandler::handleMap( UserItem* userItem, 
                          NavRequestPacket* req, 
                          NavReplyPacket* reply )
{
   if ( !checkExpectations( req->getParamBlock(), reply ) ) {
      return false;
   }

   bool ok = true;

   // The params
   const NParamBlock& params = req->getParamBlock();
   NParamBlock& rparams = reply->getParamBlock();
   // The user
   UserUser* user = userItem->getUser();
   uint32 startTime = TimeUtility::getCurrentTime();


   // Start parameter printing
   mc2log << info << "handleMap:";

   StringTable::languageCode language = user->getLanguage();
   if ( params.getParam( 6 ) ) {
      language = NavUtil::mc2LanguageCode( 
         params.getParam( 6 )->getUint16() );
      mc2log << " Lang " << StringTable::getString( 
         StringTable::getLanguageAsStringCode( language ), 
         StringTable::ENGLISH );
   }
   
   // MapBoundingbox
   MC2BoundingBox mapBbox;
   if ( params.getParam( 1600 ) ) {
      MC2Coordinate topLeft( 
         Nav2Coordinate( params.getParam( 1600 )->getInt32Array( 0 ),
                         params.getParam( 1600 )->getInt32Array( 1 ) ) );
      MC2Coordinate bottomRight( 
         Nav2Coordinate( params.getParam( 1600 )->getInt32Array( 2 ),
                         params.getParam( 1600 )->getInt32Array( 3 ) ) );
      mapBbox.setMaxLat( topLeft.lat );
      mapBbox.setMinLon( topLeft.lon );
      mapBbox.setMinLat( bottomRight.lat );
      mapBbox.setMaxLon( bottomRight.lon );
      mc2log << " MapBoundingbox " << mapBbox;
   }

   MC2Coordinate mapPos;
   if ( params.getParam( 1601 ) ) {
      mapPos = MC2Coordinate( 
         Nav2Coordinate( params.getParam( 1601 )->getInt32Array( 0 ),
                         params.getParam( 1601 )->getInt32Array( 1 ) ) );
      mc2log << " MapPosition " << mapPos;
   }

   float32 speed = 666.999;
   if ( params.getParam( 1602 ) ) {
      if ( params.getParam( 1602 )->getUint16() < MAX_INT16 ) {
         speed = float32(params.getParam( 1602 )->getUint16())/32;
         mc2log << " speed " << speed;
      } else {
         mc2log << " speed N/A";
      }
   }

   uint16 heading = MAX_UINT16;
   if ( params.getParam( 1603 ) ) {
      if ( params.getParam( 1603 )->getUint16() < 256 ) {
         heading = uint16( rint( params.getParam( 1603 )->getUint16() * 
                                 360.0 / 256.0 ) );
         mc2log << " heading " << heading << "°";
      } else {
         mc2log << " heading N/A";
      }
   }

   uint32 mapRadius = MAX_UINT32;
   if ( params.getParam( 1604 ) ) {
      mapRadius = params.getParam( 1604 )->getUint32();
      mc2log << " Map radius " << mapRadius;
   }

   MC2String routeIDStr;
   RouteID routeID( 0, 0 );
   if ( params.getParam( 1605 ) ) {
      routeIDStr = params.getParam( 1605 )->getString(
         m_thread->clientUsesLatin1());
      routeID = RouteID( routeIDStr.c_str() );
      mc2log << " routeID " << routeIDStr;
      if ( routeID.isValid() == 0 ) {
         mc2log << " INVALID";
      }
   }

   uint16 imageWidth  = 160;
   uint16 imageHeight = 120;
   if ( params.getParam( 1606 ) ) {
      imageWidth  = params.getParam( 1606 )->getUint16Array( 0 );
      imageHeight = params.getParam( 1606 )->getUint16Array( 1 );
      mc2log << " imageSize " << imageWidth << "," << imageHeight;
   }

   if ( params.getParam( 1607 ) ) {
      mc2log << " imageVbox " 
             << params.getParam( 1607 )->getUint16Array( 0 ) << ","
             << params.getParam( 1607 )->getUint16Array( 1 );
   }

   ImageDrawConfig::imageFormat imageFormat = ImageDrawConfig::GIF;
   if ( params.getParam( 1608 ) ) {
      // Image Format
      imageFormat = navImgFmtToMC2( params.getParam( 1608 )->getByte() );
      mc2log << " imageFormat " << ImageDrawConfig::getImageFormatAsString(
         imageFormat);
   }


   vector< pair<MC2Coordinate, uint16 > > mapItems;
   if ( params.getParam( 1609 ) ) {
      vector< const NParam* > pmapItems;
      params.getAllParams( 1609, pmapItems );
      for ( uint32 i = 0 ; i < pmapItems.size() ; ++i ) {
         mapItems.push_back( 
            make_pair( 
               MC2Coordinate( 
                  Nav2Coordinate(
                     pmapItems[ i ]->getInt32( 2 ),
                     pmapItems[ i ]->getInt32( 6 ) ) ),
               pmapItems[ i ]->getUint16( 0 ) ) );
         mc2log << " MapItem " << mapItems[ i ].first << "," 
                << mapItems[ i ].second;
      }
   }


   MapSettings mapSettings;
   uint16 angle = 0;
   bool showMap = mapSettings.getShowMap();
   bool showTopographMap = mapSettings.getShowTopographMap();
   bool showPOI = mapSettings.getShowPOI();
   bool showRoute = mapSettings.getShowRoute();
   if ( params.getParam( 1610 ) ) {
      vector< const NParam* > pmapInfos;
      params.getAllParams( 1610, pmapInfos );
      for ( uint32 i = 0 ; i < pmapInfos.size() ; ++i ) {
         uint16 type = pmapInfos[ i ]->getUint16( 0 );
         uint32 value = pmapInfos[ i ]->getUint32( 2 );
         mc2log << " mapInfo ";
         switch ( type ) {
            case 0 :
               mc2log << "Invalid " << value;
               break;
            case 1 :
               mc2log << "Category " << value;
               break;
            case 2 :
               mapSettings.setShowTraffic( value != 0 );
               mc2log << "TrafficInformation " << BP(value);
               break;
            case 3 :
               mapSettings.setDrawScale( value != 0 );
               mc2log << "Scale " << BP(value);
               break;
            case 4 :
               showTopographMap = value != 0;
               mc2log << "Topographic " << BP(value);
               break;
            case 5 :
               mc2log << "MapFormat " << value;
               break;
            case 6 :
               angle = uint16( rint( 360.0 / 256.0 * value ) );
               mc2log << "Rotate " << angle;
               break;

         }
      }
      mapSettings.setMapContent( showMap, showTopographMap, showPOI,
                                 showRoute );
   }

   const ClientSetting* clientSetting = m_thread->getClientSetting();
   if ( clientSetting != NULL ) {
      mapSettings.setImageSet( clientSetting->getImageSet() );
   }

   mc2log << endl;

   // Get AURA
   set< uint32 >* allowedMaps = NULL;
   if ( ok ) {
      if ( !m_thread->getMapIdsForUserRegionAccess( user, allowedMaps ) ) {
         ok = false;
         mc2log << warn << "handleMap: getMapIdsForUserRegionAccess"
                << " failed. Error: ";
         if ( TimeUtility::getCurrentTime() - startTime > 3000 ) {
            reply->setStatusCode( 
               NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
            mc2log << "Timeout";
         } else {
            reply->setStatusCode( NavReplyPacket::NAV_STATUS_NOT_OK );
            mc2log << "Error";
         }
         mc2log << endl;
      }
   }


   // Store user position?? (mapPos)


   // Get Route
   ExpandRouteRequest* expReq = NULL;
   PacketContainer* expandRouteCont = NULL;
   RouteReplyPacket* routePack = NULL;
   if ( ok && routeID.isValid() != 0 ) {
      uint32 expandType = (ROUTE_TYPE_STRING | ROUTE_TYPE_NAVIGATOR |
                           ROUTE_TYPE_ITEM_STRING | ROUTE_TYPE_GFX);
      routePack = m_thread->getStoredRouteAndExpand(
         routeID, expandType, language, 
         false/*abbreviate*/, false/*landmarks*/, true/*removeAheadIfDiff*/,
         false/*nameChangeAsWP*/, expReq, expandRouteCont );
      if ( routePack == NULL || expReq == NULL || expandRouteCont == NULL )
      {
         mc2log << warn << "handleMap: getStoredRouteAndExpand failed ";
         if ( TimeUtility::getCurrentTime() - startTime > 3000 ) {
            mc2log << "Timeout";
         } else {
            // No such route?
            mc2log << "Error";
         }
         mc2log << endl;
      }
   } // End if ok to get stored route


   // Make Map bbox
   MC2BoundingBox bbox;
   MC2String inputDebitStr;
   char tmpStr[ 256 ];
   if ( ok ) {
      // Boundingbox --  A complete bounding box.
      if ( mapBbox.isValid() ) {
         bbox = mapBbox;
         inputDebitStr = "BoundingBox [(";
         sprintf( tmpStr, "%d", mapBbox.getMaxLat() );
         inputDebitStr.append( tmpStr );
         inputDebitStr.append( "," );
         sprintf( tmpStr, "%d", mapBbox.getMinLon() );
         inputDebitStr.append( tmpStr );
         inputDebitStr.append( "),(" );
         sprintf( tmpStr, "%d", mapBbox.getMinLat() );
         inputDebitStr.append( tmpStr );
         inputDebitStr.append( "," );
         sprintf( tmpStr, "%d", mapBbox.getMaxLon() );
         inputDebitStr.append( tmpStr );
         inputDebitStr.append( ")]" );
         mc2log << info << "handleMap: Boundingbox " << bbox << endl;
      } 
      // VectorBox   --  The bounding box will be calculated from the
      //                 current position, heading, speed and Route ID.
      else if ( mapPos.isValid() && speed < 500 && heading != MAX_UINT16 )
      {
         bbox = m_mapHelp->handleNavMapVectorBox( 
            mapRadius, uint16(rint(speed*32)), mapPos, heading, 
            expandRouteCont );
         inputDebitStr = "VectorBox Coord (";
         sprintf( tmpStr, "%d", mapPos.lat );
         inputDebitStr.append( tmpStr );
         inputDebitStr.append( "," );
         sprintf( tmpStr, "%d", mapPos.lon );
         inputDebitStr.append( tmpStr );
         inputDebitStr.append( ") speed " );
         sprintf( tmpStr, "%hu", uint16(rint(speed*32)) );
         inputDebitStr.append( tmpStr );
         inputDebitStr.append( " hdg " );
         sprintf( tmpStr, "%hu", heading );
         inputDebitStr.append( tmpStr );
         inputDebitStr.append( " radius " );
         sprintf( tmpStr, "%hu", mapRadius );
         inputDebitStr.append( tmpStr );
         mc2log << info << "handleMap: VectorBox " << bbox << endl;
      }
      // RadiusBox   --  The bounding box will contain a circle
      //                 centered on the current position and with the
      //                 specified map radius.
      else if ( mapPos.isValid() ) {
         uint32 posSize = 60000;
         if ( mapRadius != 0 && mapRadius < 100000000 ) {
            // Not less than 100m
            posSize = uint32( rint( GfxConstants::METER_TO_MC2SCALE * 
                                    MAX( 100, mapRadius ) ) );
         }
         bbox.setMaxLat( mapPos.lat + posSize );
         bbox.setMinLon( mapPos.lon - posSize );
         bbox.setMinLat( mapPos.lat - posSize );
         bbox.setMaxLon( mapPos.lon + posSize );
         inputDebitStr = "RadiusBox Coord (" ;
         sprintf( tmpStr, "%d", mapPos.lat );
         inputDebitStr.append( tmpStr );
         inputDebitStr.append( "," );
         sprintf( tmpStr, "%d", mapPos.lon );
         inputDebitStr.append( tmpStr );
         inputDebitStr.append( ") radius " );
         sprintf( tmpStr, "%hu", mapRadius );
         inputDebitStr.append( tmpStr );
         mc2log << info << "handleMap: RadiusBox " << bbox << endl;
      }
      // RouteBox    --  The bounding box will contain the entire route
      //                 specified by the Route ID.
      else if ( routeID.isValid() != 0 ) {
         if ( expandRouteCont != NULL && 
              static_cast<ReplyPacket*>(expandRouteCont->getPacket() )
              ->getStatus() == StringTable::OK )
         {
            ExpandRouteReplyPacket* expand = 
               static_cast< ExpandRouteReplyPacket* > ( 
                  expandRouteCont->getPacket() );
            ExpandItemID* exp = expand->getItemID();
            ExpandStringItem** stringItems = expand->getStringDataItem();
            uint32 numStringItems = expand->getNumStringData();
            int32 minLat = 0;
            int32 maxLat = 0;
            int32 minLon = 0;
            int32 maxLon = 0;
            // uint16 angle = 0;

            // FullRoute
            HttpUtility::getRouteMC2BoundingBox( 
               expand, exp, 0, numStringItems - 1,
               minLat, maxLat, minLon, maxLon );
            
            // Set bbox
            bbox.setMaxLat( maxLat );
            bbox.setMinLon( minLon );
            bbox.setMinLat( minLat );
            bbox.setMaxLon( maxLon );
            inputDebitStr = "RouteBox ";
            sprintf( tmpStr, "%u", routeID.getRouteIDNbr() );
            inputDebitStr.append( tmpStr );
            inputDebitStr.append( "," );
            sprintf( tmpStr, "%u", routeID.getCreationTime() );
            inputDebitStr.append( tmpStr );
            mc2log << info << "handleMap: RouteBox " << bbox << endl;

            delete exp;
            for ( uint32 i = 0 ; i < numStringItems ; ++i ) {
               delete stringItems[ i ];
            }
            delete [] stringItems;
         } else {
            // Error route specified but now available.
            ok = false;
            mc2log << warn << "handleMap: Have routeID but not route: ";
            if ( TimeUtility::getCurrentTime() - startTime > 3000 ) {
               reply->setStatusCode( 
                  NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
               mc2log << "Timeout";
            } else {
               if ( routePack != NULL ) {
                  // Expansion failed
                  reply->setStatusCode( 
                     NavReplyPacket::NAV_STATUS_NOT_OK );
                  mc2log << "Expansion failed";
               } else {
                  // No such route
                  reply->setStatusCode( 
                     NavReplyPacket::NAV_STATUS_PARAMETER_INVALID );
                  reply->setStatusMessage( "RouteID not valid" );
                  mc2log << "RouteID not valid";
               }
            }
            mc2log << endl;
         }
      }
      // Nothing to make map with, send error
      else {
         mc2log << warn << "handleMap: Nothing to make map with." << endl;
         reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_MISSING_PARAMETER );
         reply->setStatusMessage( "Nothing to make map with" );
         ok = false;
      }


      if ( ok && bbox.isValid() ) {
         // Make sure that the bbox is proportional with the width, height
         int32 lla = bbox.getMinLat();
         int32 llo = bbox.getMinLon();
         int32 ula = bbox.getMaxLat();
         int32 ulo = bbox.getMaxLon();
         uint16 w = imageWidth;
         uint16 h = imageHeight;
         GfxUtility::getDisplaySizeFromBoundingbox( 
            lla, llo, ula, ulo, w, h );
         // Set bbox
         bbox.setMaxLat( ula );
         bbox.setMinLon( llo );
         bbox.setMinLat( lla );
         bbox.setMaxLon( ulo );
      
         // This is nessesary!
         bbox.updateCosLat();

         // Make real request!
         uint32 size = imageWidth * imageHeight / 8 + 4096;
         MC2BoundingBox ccBBox;
         GfxFeatureMapImageRequest* gfxReq = new GfxFeatureMapImageRequest(
            m_thread->getNextRequestID(), &bbox, imageWidth,
            imageHeight, ItemTypes::getLanguageCodeAsLanguageType( language ), 
            routePack, true, true, 
            true,/*drawCopyRight*/ imageFormat, size, &mapSettings,
            m_thread->getTopRegionRequest(), ccBBox );

         for ( uint32 i = 0 ; i < mapItems.size() ; ++i ) {
            const char* symbolImage = "";
            GfxSymbolFeature::symbols symbolType = GfxSymbolFeature::PIN;
            gfxReq->addSymbolToMap( mapItems[ i ].first.lat, 
                                    mapItems[ i ].first.lon, "",
                                    symbolType, symbolImage );
         }
         gfxReq->setMapRotation( angle );
         // Limit map area
         gfxReq->setAllowedMaps( allowedMaps );

         // Wait for the answer
         m_thread->putRequest( gfxReq );


         PacketContainer* gfxReqAnswer = gfxReq->getAnswer();
         if ( gfxReqAnswer == NULL || 
              StringTable::stringCode( 
                 static_cast<ReplyPacket*>(
                    gfxReqAnswer->getPacket() )->getStatus() ) != 
              StringTable::OK )
         {
            // Error
            if ( gfxReqAnswer == NULL ) {
               mc2log << warn  << "handleMap: " 
                      << " gfxReq NULL answer: ";
               if ( TimeUtility::getCurrentTime() - startTime > 3000 ) {
                  reply->setStatusCode( 
                     NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
                  mc2log << "Timeout";
               } else {
                  reply->setStatusCode(
                     NavReplyPacket::NAV_STATUS_NOT_OK );
                  mc2log << "Error";
               }
               mc2log << endl;
            } else {
               if ( StringTable::stringCode( 
                       static_cast<ReplyPacket*>(
                          gfxReqAnswer
                          ->getPacket() )->getStatus() ) == 
                    StringTable::TIMEOUT_ERROR )
               {
                  mc2log << warn  << "handleMap: " 
                         << " gfxReq timed out status." << endl;
                  reply->setStatusCode( 
                     NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
               } else {
                  StringTable::stringCode errCode = 
                     StringTable::stringCode(
                        static_cast<ReplyPacket*>( 
                           gfxReqAnswer->getPacket() )
                        ->getStatus() );
                  mc2log << warn  << "handleMap: " << " gfxReq not ok \""
                         << StringTable::getString( errCode,
                                                    StringTable::ENGLISH )
                         << "\" (" << int(errCode) << ")" << endl;
                  reply->setStatusCode(
                     NavReplyPacket::NAV_STATUS_NOT_OK );
               }
            }
         } else {
            uint32 size = static_cast< GfxFeatureMapImageReplyPacket* >( 
               gfxReqAnswer->getPacket() )->getSize();
            byte* imageBuff = static_cast< GfxFeatureMapImageReplyPacket* >
               ( gfxReqAnswer->getPacket() )->getImageData();
            mc2log << info  << "handleMap: " 
                   << "reply: map size " << size << " bytes Boundingbox: " 
                   << bbox << endl;
            
            // Set reply data
            // MapBoundingbox
            NParam& pbbox = rparams.addParam( NParam( 1700 ) );
            Nav2Coordinate topLeft( 
               MC2Coordinate( bbox.getMaxLat(), bbox.getMinLon() ) );
            Nav2Coordinate bottomRight( 
               MC2Coordinate( bbox.getMinLat(), bbox.getMaxLon() ) );
            pbbox.addInt32( topLeft.nav2lat );
            pbbox.addInt32( topLeft.nav2lon );
            pbbox.addInt32( bottomRight.nav2lat );
            pbbox.addInt32( bottomRight.nav2lon );
            // Image size
            NParam& piwh = rparams.addParam( NParam( 1701 ) );
            piwh.addUint16( imageWidth );
            piwh.addUint16( imageHeight );
            // Real world size
            uint32 widthInMeters = 
               uint32( rint( GfxConstants::MC2SCALE_TO_METER *
                             ( bbox.getMaxLon() - bbox.getMinLon() ) * 
                             GfxUtility::getCoslat( 
                                bbox.getMinLat(), bbox.getMaxLat() ) ) );
            uint32 heightInMeters = 
               uint32( rint( GfxConstants::MC2SCALE_TO_METER *
                             ( bbox.getMaxLat() - bbox.getMinLat() ) ) );
            NParam& prwh = rparams.addParam( NParam( 1702 ) );
            prwh.addUint32( widthInMeters );
            prwh.addUint32( heightInMeters );
            // Image Format
            rparams.addParam( NParam( 1703, 
                                      mc2ImgFmtToNav( imageFormat ) ) );
            // Image
            NParam& pimage = rparams.addParam( NParam( 1704 ) );
            pimage.addByteArray( imageBuff, size );
            // Checkpoint for VectorMap with route following
            // 1705


            // Debit
            MC2Coordinate posCoord = MC2Coordinate::invalidCoordinate;
            if ( mapItems.size() > 0 ) {
               posCoord = mapItems[ 0 ].first;
            }
            MC2String extraInfo = m_userHelp->makeExtraUserInfoStr( 
               params );
            // Set debitamount here untill module sets it ok
            uint32 debitAmount = TimeUtility::getCurrentTime() - startTime;
            if ( !m_thread->getDebitHandler()->makeMapDebit( 
                    userItem, extraInfo.c_str(), 
                    size, debitAmount, bbox, inputDebitStr.c_str(),
                    imageWidth, imageHeight, true, imageFormat, 
                    routeID.getRouteIDNbr(), routeID.getCreationTime(), 
                    mapSettings.getShowMap(), mapSettings.getShowPOI(),
                    mapSettings.getShowTopographMap(), 
                    mapSettings.getShowRoute(), 
                    mapSettings.getShowTraffic(), posCoord ) )
            {
               mc2log << warn  << "handleMap: failed to debit Map." 
                      << endl;
            }

            delete [] imageBuff;
         }

         delete gfxReqAnswer;
         delete gfxReq;
      } // End if ok to make gfx request


      
   } // End if ok to make map
   


   delete allowedMaps;
   delete expReq;
   delete expandRouteCont;
   delete routePack;

   return ok;
}


bool
NavMapHandler::handleVectorMap( UserItem* userItem, 
                                NavRequestPacket* req,
                                NavReplyPacket* reply )
{
   bool ok = true;

   // The params
   const NParamBlock& params = req->getParamBlock();
   NParamBlock& rparams = reply->getParamBlock();

   uint32 startTime = TimeUtility::getCurrentTime();
   MC2String reqStr;

   // Start parameter printing
   mc2log << info << "handleVectorMap:";
   
   if ( params.getParam( 2400 ) ) {
      reqStr = params.getParam( 2400 )->getString(
         m_thread->clientUsesLatin1());
      mc2log << " " << reqStr;
   }
   mc2log << endl;

   rparams.addParam( NParam( 2500, reqStr, 
                             m_thread->clientUsesLatin1() ) );

   DataBuffer* data = m_thread->getTileMap( reqStr.c_str() );

   if ( data != NULL ) {
      NParam& pdata = rparams.addParam( NParam( 2501 ) );
      pdata.addByteArray( data->getBufferAddress(), 
                          data->getCurrentOffset() );
      mc2log << info << "handleVectorMap: Reply " 
             << data->getCurrentOffset() << " bytes" << endl;
   } else { // Error
      mc2log << warn << "handleVectorMap: NULL TileMap answer: ";
      if ( TimeUtility::getCurrentTime() - startTime > 3000 ) {
         reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
         mc2log << "Timeout";
      } else {
         reply->setStatusCode(
            NavReplyPacket::NAV_STATUS_NOT_OK );
         mc2log << "Error";
      }
      mc2log << endl;
   }
   
   delete data;
   
   return ok;
}


bool
NavMapHandler::handleMultiVectorMap( UserItem* userItem, 
                                     NavRequestPacket* req,
                                     NavReplyPacket* reply )
{
   bool ok = true;

   // The params
   const NParamBlock& params = req->getParamBlock();
   NParamBlock& rparams = reply->getParamBlock();

   uint32 startTime = TimeUtility::getCurrentTime();

   vector<MC2SimpleString> tmapParams;
   uint32 inReqSize = 0;
   uint32 startOffset = 0;
   uint32 maxSize = 2048;

   // Start parameter printing
   mc2log << info << "handleMultiVectorMap:";
   
   if ( params.getParam( 4600 ) ) {
      startOffset = params.getParam( 4600 )->getUint32();
      mc2log << " startOffset " << startOffset;
   }

   if ( params.getParam( 4601 ) ) {
      maxSize = params.getParam( 4601 )->getUint32();
      mc2log << " maxSize " << maxSize;
   }

   if ( params.getParam( 4602 ) ) {
      uint32 pos = 0;
      while ( pos < params.getParam( 4602 )->getLength() ) {
         MC2String reqStr = params.getParam( 4602 )->incGetString(            
            m_thread->clientUsesLatin1(), pos);
         mc2log << " " << reqStr;
         tmapParams.push_back( reqStr.c_str() );
      }
      inReqSize = pos;
   }
   mc2log << endl;

   DataBuffer* data = m_thread->getTileMaps( 
      tmapParams, startOffset, maxSize );

   if ( data != NULL ) {
      for ( uint32 pos = 0; pos < data->getCurrentOffset() ; 
            pos += MAX_UINT16 )
      {
         NParam& pdata = rparams.addParam( NParam( 4700 ) );
         uint32 size = MIN( data->getCurrentOffset() - pos, MAX_UINT16 );
         pdata.addByteArray( data->getBufferAddress() + pos, size );
      }
      mc2log << info << "handleMultiVectorMap: Reply " 
             << data->getCurrentOffset() << " bytes Request " 
             << inReqSize << " bytes" << endl;
   } else { // Error
      mc2log << warn << "handleMultiVectorMap: NULL TileMap answer: ";
      if ( TimeUtility::getCurrentTime() - startTime > 3000 ) {
         reply->setStatusCode( 
            NavReplyPacket::NAV_STATUS_REQUEST_TIMEOUT );
         mc2log << "Timeout";
      } else {
         reply->setStatusCode(
            NavReplyPacket::NAV_STATUS_NOT_OK );
         mc2log << "Error";
      }
      mc2log << endl;
   }
   
   delete data;
   
   return ok;
}


ImageDrawConfig::imageFormat 
NavMapHandler::navImgFmtToMC2( byte fmt ) const {
   switch( fmt ) {
      case 0 :
         return ImageDrawConfig::PNG;
      case 1 :
         return ImageDrawConfig::WBMP;
      case 2 :
         return ImageDrawConfig::JPEG;
      case 3 :
         return ImageDrawConfig::GIF;
      default:
         mc2log << warn << "NavMapHandler::navImgFmtToMC2 unknown "
                << int(fmt) << " using gif" << endl;
         return ImageDrawConfig::GIF;
   }
}


byte
NavMapHandler::mc2ImgFmtToNav( ImageDrawConfig::imageFormat fmt ) const {
   switch( fmt ) {
      case ImageDrawConfig::PNG :
         return 0;
      case ImageDrawConfig::WBMP :
         return 1;
      case ImageDrawConfig::JPEG :
         return 2;
      case ImageDrawConfig::GIF :
         return 3;
      case ImageDrawConfig::NBR_IMAGE_FORMATS :
         return 3;
   }

   // We should never reach this
   return 3;
}
