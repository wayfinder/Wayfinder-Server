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

#include "MapDrawer.h"
#include "MapUtility.h"
#include "StringUtility.h"
#include "AbbreviationTable.h"
#include "GfxDataFull.h"
#include "GfxFeatureMap.h"
#include "GfxPolygon.h"
#include "GfxRoadPolygon.h"
#include "MapSettings.h"
#include "MC2Point.h"
#include "gd.h"
#include "ScopedArray.h"
#include "DeleteHelpers.h"

#include "CairoImageDraw.h"
#include "GDImageDraw.h"

#include "Properties.h"

#include "TextIterator.h"
#include "DebugClock.h"

#include "MapFontSettings.h"
#include "MapDrawingCommon.h"
#include "MapTextPlacement.h"
#include "ObjectBoxes.h"

typedef MapDrawingCommon::featurenotice_t featurenotice_t;

/**
 * Draws the route inside a map as an arrow.
 *
 * @param map The GfxFeatureMap with the route to use.
 * @param mapRotation Angle to routate the map as the arrow
 *                    is drawn. 0 => north up, one lap is 360
 *                    degrees, clockwise rotation.
 * @param afterTurnId The id for the route feature after the turn
 *                    this crossing arrow is drawn for.
 *
 *                    NB.
 *                    This is the index counting from the first rotue
 *                    feature, i.e. if the empty features should be
 *                    used to convert this id to the feature id in
 *                    the GfxFeatureMap, 1 must be added in order to
 *                    compensate for the route origin.
 *
 * @param arrowHeadSize The length of the arrow.
 * @param col The color to draw the arrow with.
 * @return True if turn arrow was drawn, false if not.
 */
void drawRouteAsArrow( GfxFeatureMap* map,
                       ImageDraw* imageDraw,
                       const DrawingProjection* drawingProjection,
                       int32 mapRotation,
                       uint32 afterTurnId, uint32 arrowHeadSize,
                       uint32 routeWidth,
                       GDUtils::Color::CoolColor color );
/**
 * Draws a turn arrow on current image.
 *
 * @param map The GfxFeatureMap with the route to use.
 * @param beforeTurn The index of the feature before the turn.
 *      
 *                    NB.
 *                    This is the index counting from the first rotue
 *                    feature, i.e. if the empty features should be
 *                    used to convert this id to the feature id in
 *                    the GfxFeatureMap, 1 must be added in order to
 *                    compensate for the route origin.
 *
 * @param afterTurn The index of the feature after the turn.
 *
 *                    NB.
 *                    This is the index counting from the first rotue
 *                    feature, i.e. f the empty features should be
 *                    used to convert this id to the feature id in
 *                    the GfxFeatureMap, 1 must be added in order to
 *                    compensate for the route origin.
 *
 * @param arrowAngle The angle of the arrow.
 * @param arrowLength The length of the arrow.
 * @param col The color to draw the turn arrow with.
 * @return True if turn arrow was drawn, false if not.
 */
bool drawRouteTurn( GfxFeatureMap* map,
                    ImageDraw* imageDraw,
                    const DrawingProjection* drawingProjection,
                    uint32 beforeTurn, uint32 afterTurn,
                    int32 arrowAngle, int32 arrowLengh,
                    GDUtils::Color::CoolColor col );

MapDrawer::MapDrawer()
{
   m_drawingProjection = NULL;
}

MapDrawer::~MapDrawer()
{
}

/**
 * Determines if cairo should be used.
 * @param w Transforms width to match the renderer.
 * @param h Transforms height to match the renderer.
 * @return Whether or not to use cairo.
 */
bool useCairo( uint32& w, uint32& h ) {
   const char* library = Properties::getProperty( "IMAGEDRAW_LIB", "gd");
   bool useCairo = strcasecmp( library, "cairo" ) == 0;

#ifndef HAVE_CAIRO
   useCairo = false;
#endif

#ifdef NEWER_GD
   if ( ! useCairo ) {
      // the aka "newer" gd has size bugs
      // somewhere between version 2.0.1 and 2.0.31
      w++;
      h++;
   }
#endif

   return useCairo;
}

/**
 * Creates an imagedraw renderer.
 * @param w width of the image.
 * @param h height of the image.
 * @param color the water color.
 * @return an imagedraw to be used for rendering.
 */
inline ImageDraw* createImageDraw( uint32 w, uint32 h,
                                   GDUtils::Color::CoolColor color ) {
   bool cairo = useCairo( w, h );
#ifdef HAVE_CAIRO
   if ( cairo ) {
      return new CairoImageDraw( w, h, color );
   } else {
      return new GDImageDraw( w, h, color );
   }
#else
   // disable compiler varning
   cairo = false;
   return new GDImageDraw( w, h, color );
#endif
}

/**
 * Creates an imagedraw renderer.
 * @param w Width in pixel of the image.
 * @param h Height in pixel of the image.
 * @return an imagedraw to be used for rendering.
 */
inline ImageDraw* createImageDraw( uint32 w, uint32 h ) {
   bool cairo = useCairo( w, h );
#ifdef HAVE_CAIRO
   if ( cairo ) {
      return new CairoImageDraw( w, h );
   } else {
      return new GDImageDraw( w, h );
   }
#else
   cairo = false; // disables the ability to fire laser
   return new GDImageDraw( w, h );
#endif
}

void
MapDrawer::init( uint32 screenX, uint32 screenY,
                 MapSettings* mapSettings ) {
   m_imageDraw.reset( NULL );

   if ( mapSettings != NULL ) {
      // Fetch color of water items.
      MapSetting* waterSetting =
         mapSettings->getSettingFor( GfxFeature::WATER, CONTINENT_LEVEL );

      if ( waterSetting != NULL ){
         // Water color determines the background color of the image.
         m_imageDraw.reset( createImageDraw( screenX, screenY,
                                             waterSetting->m_drawColor ) );
      } else {
         m_imageDraw.reset( createImageDraw( screenX, screenY ) );
      }

      m_drawingProjection = mapSettings->getDrawingProjection();
      m_imageDraw->setDrawingProjection( m_drawingProjection );

   } else {
      m_imageDraw.reset( createImageDraw( screenX, screenY ) );
   }
}


byte*
MapDrawer::drawGfxFeatureMap(GfxFeatureMap* featureMap,
                             uint32& size,
                             MapSettings* mapSettings,
                             int16 mapRotation,
                             RouteArrowType::arrowType arrowType,
                             uint32 beforeTurn,
                             uint32 afterTurn,
                             int32 arrowAngle, int32 arrowLengh,
                             GDUtils::Color::CoolColor arrowColor,
                             bool drawText,
                             ImageDrawConfig::imageFormat format,
                             const char* copyright,
                             bool drawCopyRight,
                             const POIImageIdentificationTable* imageTable )
{
   mc2dbg8 << here << "MapDrawer::drawGfxFeatureMap(" << featureMap
           << "," << size << "," << mapSettings <<"," <<(int)arrowType
           << "," << beforeTurn << "," << afterTurn << "," << arrowAngle
           << "," << arrowLengh << "," << int(arrowColor) << "," << drawText
           << "," << int(format) << endl;

   DebugClock drawGfxFeatureClock;

   uint16 screenX = featureMap->getScreenX();
   uint16 screenY = featureMap->getScreenY();
   if ( screenX < 10 ) {
      screenX = 10;
   }
   if ( screenY < 10 ) {
      screenY = 10;
   }
   init(screenX, screenY, mapSettings);
   // Check inparameter
   size = 0;
   if (m_imageDraw.get() == NULL) {
      mc2log << error << "MapDrawer::drawGfxFeatureMap m_imageDraw == NULL"
             << endl;
      return NULL;
   }

   // Create an vector and fill it with feature-notices
   vector<featurenotice_t> notices;
   // Container of boundingboxes of objects that must not be overlapped.
   MapDrawingCommon::ObjectBoxes objectBBoxes;
   STLUtility::AutoContainer< vector<GfxData*> > gfxTextArray;


   MC2BoundingBox bbox = m_drawingProjection->getBoundingBox();

   /* We do not need these boxes anymore since we now calculate
      boundingbox stuff correctly...

      // only add these boxes for scale level > 2, so street text will
      // be drawn correctly
      if ( featureMap->getScaleLevel() > 2 ) {
      // Put four MC2-BoundingBoxes around the map to prevent parts of texts
      // from being placed out outside the map

      // add collision boxes around our main box for text placement
      // help.
      int32 lonInc = bbox.getLonDiff() / 2;
      int32 latInc = bbox.getHeight() / 2;
      // top box
      objectBBoxes.
      push_back( MC2BoundingBox( bbox.getMaxLat() + latInc, 
      bbox.getMinLon() - lonInc,
      bbox.getMaxLat(),
      bbox.getMaxLon() + lonInc ));
      // left box
      objectBBoxes.
      push_back( MC2BoundingBox( bbox.getMaxLat(),
      bbox.getMinLon() - lonInc,
      bbox.getMinLat(), 
      bbox.getMinLon() ));
      // right box
      objectBBoxes.
      push_back( MC2BoundingBox( bbox.getMaxLat(),
      bbox.getMaxLon(),
      bbox.getMinLat(), 
      bbox.getMaxLon() + lonInc ));
      // bottom box
      objectBBoxes.
      push_back( MC2BoundingBox( bbox.getMinLat(),
      bbox.getMinLon() - lonInc,
      bbox.getMinLat() - latInc,
      bbox.getMaxLon() + lonInc ));

      }
   */

   sortAndCreateFeatureNotices( featureMap, mapSettings,
                                notices,
                                true, // single draw
                                true, // check for overlapping objects
                                false,  // don't include hidden features
                                true, // filter pois
                                &objectBBoxes );


   // Set members
   m_scaleLevel = featureMap->getScaleLevel();
   ImageDraw* image = m_imageDraw.get();
   mc2dbg4 << "   scalelevel set to " << m_scaleLevel << " (from map)"
           << endl;
   // Draw all the notices
   DebugClock noticeClock;
   DrawSettings settings;
   
   for ( vector<featurenotice_t>::iterator
            f = notices.begin(); f != notices.end(); ++f ) {
      if (MapUtility::
          getDrawSettings( f->m_feature->getType(),
                           m_scaleLevel,
                           mapSettings,
                           &settings,
                           f->m_feature,
                           f->m_border,
                           f->m_feature->getPolygon( f->m_polygonIndex ),
                           featureMap,
                           f->m_poiStatus, 
                           imageTable ) ) {
         if ( ! m_imageDraw->
              drawGfxFeaturePolygon( f->m_feature,
                                     f->m_feature->
                                     getPolygon(f->m_polygonIndex),
                                     &settings,
                                     mapSettings->getImageSet() ) ) {
            mc2log << warn << "MapDrawer::drawGfxFeatureMap Failed to draw!"
                   << endl;
            mc2dbg << "MapDrawer::drawGfxFeatureMap Failed to draw!"
                   << endl;
         }
      }
   }
   mc2dbg4 << "Draw all feature notices took "
           << noticeClock << endl;
   mc2dbg1 << "MapDrawer::drawGfxFeatureMap init and draw features took "
           << drawGfxFeatureClock << endl;

   // Draw routeturnarrow
   if (arrowType == RouteArrowType::TURN_ARROW ){
      drawRouteTurn( featureMap, m_imageDraw.get(),
                     m_drawingProjection,
                     beforeTurn, afterTurn,
                     arrowAngle, arrowLengh, arrowColor );
   }
   // Draw route as arrow
   if (arrowType == RouteArrowType::ROUTE_AS_ARROW ){
      mc2dbg8 << "MapDrawer::drawGfxFeatureMap. "
              << "Draw route as arrow." << endl;

      MapSetting* secondStreetSetting =
         mapSettings->getSettingFor( GfxFeature::STREET_SECOND,
                                     CONTINENT_LEVEL);

      uint32 routeWidth = secondStreetSetting->m_lineWidth;

      uint32 arrowHeight = routeWidth * 2;
      drawRouteAsArrow( featureMap,
                        m_imageDraw.get(),
                        m_drawingProjection,
                        mapRotation,
                        afterTurn, arrowHeight,
                        routeWidth,  arrowColor );
   }
   if ( drawText ) {
      DebugClock drawTextClock;

      // Set the text-related variables in the featureNotices
      // Create an vector and fill it with feature-notices
      vector<MapTextPlacement::featuretextnotice_t> textnotices;
      MapTextPlacement::
         initializeText( featureMap, mapSettings, textnotices,
                         objectBBoxes, image,gfxTextArray,
                         m_drawingProjection );

      uint32 totalDrawNormalTextTime = 0;
      uint32 totalDrawRotadedTextTime = 0;

      // Draw all the notices
      for ( MapTextPlacement::featureTextNoticeConstIt
               f = textnotices.begin() ;
            f < textnotices.end() ;
            f++) {
         
         if ( ! f->m_feature->getDisplayText() ) {
            continue;
         }
         const GfxFeature* feature = f->m_feature;
         // TODO: Check for negative getDrawTextStart!! in the future
         const char* name = feature->getName();
         const char* baseName = 
            ( feature->getType() == GfxFeature::LAND ) ? 
            feature->getBasename().c_str() : NULL;

         mc2dbg8 << name << endl;

         const char* fontName = "";
         GDUtils::Color::CoolColor color =
            GDUtils::Color::makeColor( GDUtils::Color::BLACK );

         // Always draw streetnames rotated,
         // and ferry names
         bool drawRotatedText = 
            MapDrawingCommon::isStreetFeature( *feature ) || 
            feature->getType() == GfxFeature::FERRY;

         int32 fontSizeNotUsed = 12;
         MapFontSettings::
            getFontSettings( feature, f->m_setting, fontName, 
                             fontSizeNotUsed, color, m_scaleLevel );

         mc2dbg2 << here << " drawRotatedText = " << drawRotatedText
                 << endl;
         uint32 textPlacingTime, startTime;
         if (drawRotatedText) {
            // STREET and FERRY
            startTime = TimeUtility::getCurrentTime();
            // Draw roadSigns
            if ((feature->getTextLat() != 0) &&
                (feature->getTextLon() != 0) ) {
               // Make roadsign
               MC2String signName = "";
               MC2String number = "";
               double fontSize = 10.0;
               GDUtils::Color::CoolColor signColor =
                  GDUtils::Color::makeColor( GDUtils::Color::WHITE );

               if( (name[0] == 'I') &&
                   (feature->getCountryCode() == StringTable::USA_CC) ) {
                  signName = "interstate.png";
                  for( uint32 i = 2; i < strlen(name); i++) {
                     number += name[i];
                  }
               } else if( (name[0] == 'U') && (name[1] == 's') &&
                          (feature->getCountryCode() ==
                           StringTable::USA_CC) ) {
                  signName = "highway.png";
                  for( uint32 i = 3; i < strlen(name); i++) {
                     number += name[i];
                  }
                  signColor = GDUtils::Color::makeColor( GDUtils::Color::BLACK );
               } else if( name[0] == 'E' ) {
                  signName = "eroadsign.png";
                  number = name;
               } else {
                  signName = "roadsign.png";
                  number = name;
               }

               mc2dbg2   << "roadsign '" << name << "' "
                         << signName << " feat->type="
                         << feature->getType() << " ("
                         << feature->getTextLat() << ","
                         << feature->getTextLon() << ")"
                         << " fontSize: "<< fontSize
                         << " signColor: " << signColor << endl;

               m_imageDraw->
                  drawGfxFeatureRoadSign( number.c_str(), signName.c_str(),
                                          feature->getTextLat(),
                                          feature->getTextLon(),
                                          0.10, fontSize,
                                          signColor, "VeraBd.ttf" );

               // Draw road-ferry texts
            } else {
               vector<GfxDataTypes::textPos> textOut = f->m_curvedTextOut;
               m_imageDraw->drawGfxFeatureText( name,
                                                textOut,
                                                feature->getFontSize(), 
                                                color, fontName );
            }
            textPlacingTime = TimeUtility::getCurrentTime() - startTime;
            totalDrawRotadedTextTime += textPlacingTime;

         } else {
            startTime = TimeUtility::getCurrentTime();
            m_imageDraw->
               drawGfxFeatureText( name,
                                   baseName,
                                   feature->getTextLat(),
                                   feature->getTextLon(),
                                   feature->getFontSize(),
                                   color,
                                   ((( 360 - feature->getDrawTextStart() 
                                       + 90 ) % 360 ) *
                                    GfxConstants::degreeToRadianFactor),
                                   MapTextPlacement::
                                   shouldSplitText( *feature ),
                                   fontName );
            textPlacingTime = TimeUtility::getCurrentTime() - startTime;
            totalDrawNormalTextTime = totalDrawNormalTextTime +
               textPlacingTime;

         }
         mc2dbg2 << "MapDrawer::drawGfxFeatureMap drawNormalTextTime = "
                 << totalDrawNormalTextTime << " ms" << endl;
         mc2dbg2 << "MapDrawer::drawGfxFeatureMap drawRotadedTextTime = "
                 << totalDrawRotadedTextTime << " ms" << endl;
         mc2dbg2 << "MapDrawer::drawGfxFeatureMap total init and draw text time = "
                 << drawTextClock << " ms" << endl;
      }
      /*
      // Draw text bounding boxes
      vector<GfxData*>::iterator it;
      for( it = gfxTextArray.begin(); it != gfxTextArray.end(); ++it) {
      GfxData* gfxData = *it;
      MC2BoundingBox textBBox;
      gfxData->getMC2BoundingBox(textBBox);
      m_imageDraw->drawBoundingBox( textBBox );
      }

      // Draw object bboxes
      vector<MC2BoundingBox>::iterator bboxIt;
      for( bboxIt = objectBBoxes.begin(); bboxIt != objectBBoxes.end();
      ++bboxIt) {
      MC2BoundingBox bbox = *bboxIt;
      m_imageDraw->drawBoundingBox( bbox );
      }
      */
   }

   // Draw scale
   if ( screenX > 75 &&
        mapSettings != NULL && mapSettings->getDrawScale() )
      {
         m_imageDraw->drawScale( &bbox );
      }

   DEBUG2(
          // Write the name of the current scalelevel.
          m_imageDraw->writeText(MapUtility::getScaleLevelName(m_scaleLevel),
                                 10, 12););
   if ( drawText && screenX > 150 && drawCopyRight ) {
      // Write copyright
      float64 fontSize = 10.0;
      int bottomMargin = 8;
      int sideMargin = 5;
      if ( screenX > 400 ) { // > 400 pxl
         fontSize = 12.0;
         bottomMargin = sideMargin = 8;
      } else if (  screenX > 300 ) { // > 300 pxl
         fontSize = 10.0;
         bottomMargin = sideMargin = 6;
      } else if ( screenX > 250 ) { // > 250 pxl
         fontSize = 9.0;
         bottomMargin = sideMargin = 4;
      } else { // less than or equal to 250 pxl
         fontSize = 8.0;
         bottomMargin = sideMargin = 3;
      }

      string newCopyright = copyright;
      if (MapTextPlacement::isSmallImage( screenX ) ) {
         // shorten the copyright string
         // TODO: Set up copyright here
         // newCopyright = "© ";
      }
      mc2log << "Draws copyright \"" << newCopyright << "\" with sideMargin "
             << sideMargin << " bottomMargin " << bottomMargin
             << " fontSize " << fontSize << endl;
      m_imageDraw->writeText( newCopyright.c_str(),
                              sideMargin, screenY-bottomMargin, fontSize );
   }

#ifdef NEWER_GD
   if ( typeid( *m_imageDraw ) == typeid( GDImageDraw ) ) {
      m_imageDraw->cutImage( screenX, screenY );
   }
#endif

   // Get the return-value and set the size
   byte* imageBuffer = m_imageDraw->getImageAsBuffer(size, format);

   mc2dbg1 << "MapDrawer::drawGfxFeatureMap took "
           << drawGfxFeatureClock << endl;

   return imageBuffer;
}

bool
drawRouteTurn( GfxFeatureMap* map,
               ImageDraw* imageDraw,
               const DrawingProjection* drawingProjection,
               uint32 beforeTurn, uint32 afterTurn,
               int32 arrowAngle, int32 arrowLengh,
               GDUtils::Color::CoolColor col ) {
   bool drawn = false;

   uint32 routeStartIndex = MAX_UINT32;
   uint32 routeStopIndex = 0;
   int32 currIndex = map->getNbrFeatures() - 1;

   // Find stop
   while ( currIndex >= 0 &&
           (map->getFeature( currIndex )->getType() !=
            GfxFeature::ROUTE_DESTINATION &&
            map->getFeature( currIndex )->getType() !=
            GfxFeature::ROUTE ) ) {
      currIndex--;
   }

   if ( currIndex >= 0 && map->getFeature( currIndex )->getType() ==
        GfxFeature::ROUTE_DESTINATION ) {
      routeStopIndex = currIndex;
   } else if ( currIndex >= 0 &&
               map->getFeature( currIndex )->getType() ==
               GfxFeature::ROUTE ) {
      // Check if empty next then it is ROUTE_DESTINATION.
      if ( map->getFeature( currIndex + 1 ) != NULL &&
           map->getFeature( currIndex + 1 )->getType() ==
           GfxFeature::EMPTY ) {
         routeStopIndex = currIndex + 1;
      }
   }

   // Find start
   currIndex--;
   while ( currIndex >= 0 &&
           (map->getFeature( currIndex )->getType() ==
            GfxFeature::ROUTE ||
            map->getFeature( currIndex )->getType() ==
            GfxFeature::EMPTY ||
            map->getFeature( currIndex )->getType() ==
            GfxFeature::PARK_CAR) ) {
      currIndex--;
   }

   if ( currIndex >= 0 &&
        (map->getFeature( currIndex )->getType() ==
         GfxFeature::ROUTE_ORIGIN ||
         map->getFeature( currIndex )->getType() == GfxFeature::EMPTY) ) {
      routeStartIndex = currIndex;
   } else if ( currIndex + 1 >= 0 &&
               map->getFeature( currIndex + 1 ) != NULL &&
               (map->getFeature( currIndex + 1 )->getType()
                == GfxFeature::EMPTY ||
                map->getFeature( currIndex + 1 )->getType() ==
                GfxFeature::ROUTE_ORIGIN) ) {
      routeStartIndex = currIndex + 1;
   }

   if ( routeStartIndex < routeStopIndex ) {
      uint32 nbrRouteFeatures = (routeStopIndex-routeStartIndex) + 1;
      if ( beforeTurn == 0 && afterTurn == 0 ) {
         // Origin
         afterTurn++;
      } else {
         // ExpandItemID has no origin/destination
         beforeTurn++;
         afterTurn++;
         if ( beforeTurn == afterTurn &&
              beforeTurn == (nbrRouteFeatures - 2) ) {
            // Destination
            afterTurn = nbrRouteFeatures -1;
         }
      }

      // Have a route (At least both origin and destination)
      // Get features at turn
      const GfxFeature* beforeFeature = NULL;
      const GfxFeature* afterFeature = NULL;

      beforeFeature = map->getFeature( routeStartIndex + beforeTurn );
      afterFeature = map->getFeature( routeStartIndex + afterTurn );

      if ( beforeFeature != NULL && afterFeature != NULL &&
           beforeFeature->getType() != GfxFeature::EMPTY &&
           afterFeature->getType() != GfxFeature::EMPTY ) {
         if ( afterFeature->getType() == GfxFeature::PARK_CAR ) {
            // Do nothing as PARK_CAR symbol is allready drawn
         } else {
            // Get coordinates around turn
            POINT src[3];
            POINT dst[4];
            POINT arrow[3];
            bool draw = false;
            MC2BoundingBox bbox;
            map->getMC2BoundingBox( &bbox );
            float64 xFactor = 0;
            float64 yFactor = 0;
            uint16 width = map->getScreenX();
            uint16 height = map->getScreenY();
            MapUtility::makeDrawItemParameters( width,
                                                height,
                                                bbox, xFactor,
                                                yFactor );

            MC2Coordinate coord;
            MC2Point point(0, 0);

            if ( beforeFeature->getType() == GfxFeature::ROUTE_ORIGIN ) {
               // Strait arrow in start direction
               coord = MC2Coordinate( beforeFeature->getPolygon( 0 )
                                      ->getLat(0),
                                      beforeFeature->getPolygon( 0 )
                                      ->getLon(0) );
               point = drawingProjection->getPoint( coord );
               src[1].x = point.getX();
               src[1].y = point.getY();
               // Second coordinate in afterFeature
               int32 lon = afterFeature->getPolygon( 0 )
                  ->getLon(0);
               int32 lat = afterFeature->getPolygon( 0 )
                  ->getLat(0);
               src[ 2 ].x = src[ 1 ].x;
               src[ 2 ].y = src[ 1 ].y;
               uint32 i = 1;

               while ( src[ 1 ].x == src[ 2 ].x &&
                       src[ 1 ].y == src[ 2 ].y &&
                       i < afterFeature->getPolygon( 0 )
                       ->getNbrCoordinates() ) {
                  lon = afterFeature->getPolygon( 0 )
                     ->getLon( i, lon );
                  lat = afterFeature->getPolygon( 0 )
                     ->getLat( i, lat );
                  coord = MC2Coordinate( lat, lon );
                  point = drawingProjection->getPoint( coord );
                  src[ 2 ].x = point.getX();
                  src[ 2 ].y = point.getY();
                  i++;
               }

               // Extrapolate first point
               src[ 0 ].x = 2*src[ 1 ].x - src[ 2 ].x;
               src[ 0 ].y = 2*src[ 1 ].y - src[ 2 ].y;
               draw = true;
            } else if ( afterFeature->getType() ==
                        GfxFeature::ROUTE_DESTINATION ) {
               // Draw nothing as ROUTE_DESTINATION symbol is drawn
            } else {
               // Before turn, second last coorinate in beforeFeature
               if ( beforeFeature->getPolygon( 0 )->getNbrCoordinates()
                    > 1 ) {
                  int32 lon = beforeFeature->getPolygon( 0 )
                     ->getLon( 0 );
                  int32 lat = beforeFeature->getPolygon( 0 )
                     ->getLat( 0 );
                  coord  = MC2Coordinate( lat, lon );
                  point = drawingProjection->getPoint( coord );
                  int oldX = point.getX();
                  int oldY = point.getY();
                  int x = oldX;
                  int y = oldY;
                  int newX = 0;
                  int newY = 0;
                  for ( uint32 i = 1 ;
                        i < beforeFeature->getPolygon( 0 )
                           ->getNbrCoordinates() - 1;
                        i++ ) {
                     lon = beforeFeature->getPolygon( 0 )
                        ->getLon( i, lon );
                     lat = beforeFeature->getPolygon( 0 )
                        ->getLat( i, lat );
                     coord = MC2Coordinate( lat, lon );
                     point = drawingProjection->getPoint( coord );
                     newX = point.getX();
                     if ( newX != x ) {
                        oldX = x;
                        x = newX;
                     }
                     newY = point.getY();
                     if ( newY != y ) {
                        oldY = y;
                        y = newY;
                     }
                  }
                  src[ 0 ].x = oldX;
                  src[ 0 ].y = oldY;
               } else { // Use previous features last coordinate
                  const GfxFeature* prevFeature =
                     map->getFeature( routeStartIndex + beforeTurn - 1);

                  int32 lon = prevFeature->getPolygon( 0 )
                     ->getLon( 0 );
                  int32 lat = prevFeature->getPolygon( 0 )
                     ->getLat( 0 );
                  coord = MC2Coordinate( lat, lon );
                  point = drawingProjection->getPoint( coord );
                  int oldX = point.getX();
                  int oldY = point.getY();
                  int x = oldX;
                  int y = oldY;
                  int newX = 0;
                  int newY = 0;
                  for ( uint32 i = 1 ;
                        i < prevFeature->getPolygon( 0 )
                           ->getNbrCoordinates();
                        i++ ) {
                     lon = prevFeature->getPolygon( 0 )
                        ->getLon( i, lon );
                     lat = prevFeature->getPolygon( 0 )
                        ->getLat( i, lat );
                     coord = MC2Coordinate( lat, lon );
                     point = drawingProjection->getPoint( coord );
                     newX = point.getX();
                     if ( newX != x ) {
                        oldX = x;
                        x = newX;
                     }
                     newY = point.getY();
                     if ( newY != y ) {
                        oldY = y;
                        y = newY;
                     }
                  }
                  src[ 0 ].x = oldX;
                  src[ 0 ].y = oldY;
               }

               // At turn
               coord = MC2Coordinate( afterFeature->getPolygon( 0 )
                                      ->getLat( 0 ),
                                      afterFeature->getPolygon( 0 )
                                      ->getLon( 0 ) );
               point = drawingProjection->getPoint( coord );
               src[ 1 ].x = point.getX();
               src[ 1 ].y = point.getY();

               // After turn, second coordinate in afterFeature
               if ( afterFeature->getPolygon( 0 )->getNbrCoordinates()
                    > 1 ) {
                  int32 lon = afterFeature->getPolygon( 0 )
                     ->getLon( 0 );
                  int32 lat = afterFeature->getPolygon( 0 )
                     ->getLat( 0 );
                  src[ 2 ].x = src[ 1 ].x;
                  src[ 2 ].y = src[ 1 ].y;
                  uint32 i = 1;

                  while ( src[ 1 ].x == src[ 2 ].x &&
                          src[ 1 ].y == src[ 2 ].y &&
                          i < afterFeature->getPolygon( 0 )
                          ->getNbrCoordinates() ) {
                     lon = afterFeature->getPolygon( 0 )
                        ->getLon( i, lon );
                     lat = afterFeature->getPolygon( 0 )
                        ->getLat( i, lat );
                     coord = MC2Coordinate( lat, lon );
                     point = drawingProjection->getPoint( coord );
                     src[ 2 ].x = point.getX();
                     src[ 2 ].y = point.getY();
                     i++;
                  }
               } else { // Use next features first coordinate
                  const GfxFeature* nextFeature =
                     map->getFeature( routeStartIndex + afterTurn + 1);
                  int32 lon = nextFeature->getPolygon( 0 )
                     ->getLon( 0 );
                  int32 lat = nextFeature->getPolygon( 0 )
                     ->getLat( 0 );
                  src[ 2 ].x = src[ 1 ].x;
                  src[ 2 ].y = src[ 1 ].y;
                  uint32 i = 1;

                  while ( src[ 1 ].x == src[ 2 ].x &&
                          src[ 1 ].y == src[ 2 ].y &&
                          i < nextFeature->getPolygon( 0 )
                          ->getNbrCoordinates() ) {
                     lon = nextFeature->getPolygon( 0 )
                        ->getLon( i, lon );
                     lat = nextFeature->getPolygon( 0 )
                        ->getLat( i, lat );
                     coord = MC2Coordinate( lat, lon );
                     point = drawingProjection->getPoint( coord );
                     src[ 2 ].x = point.getX();
                     src[ 2 ].y = point.getY();
                     i++;
                  }
               }

               if ( src[ 1 ].x == src[ 2 ].x &&
                    src[ 1 ].y == src[ 2 ].y ) {
                  // Then move 2:nd point ahead
                  src[ 2 ].x = 2*src[ 1 ].x - src[ 0 ].x;
                  src[ 2 ].y = 2*src[ 1 ].y - src[ 0 ].y;
               }
               draw = true;
            }
            if ( draw ) {
               dst[0].y = MIN_INT32; //Used to check after call
               // Calculate cornerDistance, shortDistance, longDistance
               int cornerDistance = 20;
               int shortDistance = 3;
               int longDistance = 10;
               int arrowLenghAdjusted = arrowLengh;
               int arrowWidth = 2;
               cornerDistance = int( ceil( xFactor* 5000 ) );
               shortDistance =  int ( ceil( 2500 * xFactor ) );
               longDistance = int ( ceil( 7000 * xFactor ) );
               arrowLenghAdjusted = abs( int ( ceil(
                                                    5600 * xFactor / ::sin( arrowAngle ) ) ) );
               arrowWidth = int ( ceil( 1800 * xFactor ) );

               GfxUtility::calcArrowPoints( src, dst, arrow,
                                            cornerDistance,
                                            shortDistance,
                                            longDistance, false,
                                            arrowLenghAdjusted,
                                            arrowAngle,
                                            false );

               if ( dst[0].y != int32( MIN_INT32 ) ) {
                  imageDraw->drawTurnArrow( dst[0].x, dst[0].y,
                                            dst[1].x, dst[1].y,
                                            dst[2].x, dst[2].y,
                                            dst[3].x, dst[3].y,
                                            arrow[ 0 ].x, arrow[ 0 ].y,
                                            arrow[ 1 ].x, arrow[ 1 ].y,
                                            arrow[ 2 ].x, arrow[ 2 ].y,
                                            arrowAngle,
                                            arrowLenghAdjusted,
                                            col, arrowWidth );
               } else {
                  mc2log << warn << "GfxUtility::calcArrowPoints failed"
                         << endl
                         << "   src[ 0 ].x " << src[ 0 ].x << endl
                         << "   src[ 0 ].y " << src[ 0 ].y << endl
                         << "   src[ 1 ].x " << src[ 1 ].x << endl
                         << "   src[ 1 ].y " << src[ 1 ].y << endl
                         << "   src[ 2 ].x " << src[ 2 ].x << endl
                         << "   src[ 2 ].y " << src[ 2 ].y << endl;
               }
            }
         }
      } else {
         mc2log << warn << "MapDrawer::drawRouteTurn failed to get "
            "turn features." << endl;
      }
   } else {
      mc2log << warn << "MapDrawer::drawRouteTurn no route in map."
             << endl;
   }
   return drawn;
}

/**
 * Calculates the coordinates for the arrow in a manner specified
 * by an arrow size and the direction and position of the last line
 * segment before the arrow head.
 *
 * @param arrowHeadSize The distance between the last line 
 *                      forming the arrow and the top of the 
 *                      arrow.
 * @param pointAtArrowHeadBaseY Y coordinate of the point closest
 *                              to the arrow head.
 * @param pointAtArrowHeadBaseX X coordinate of the point closest
 *                              to the arrow head.
 * @param pointBeforeArrowHeadBaseY  Y coordinate of the point 
 *                                   second closest to the arrow
 *                                   head.
 * @param pointBeforeArrowHeadBaseX  X coordinate of the point 
 *                                   second closest to the arrow
 *                                   head.
 * @param arrowHeadCoordinates Outparameter Vector containing the
 *                             calculated coordinates for drawing
 *                             the arrow head. They are ordered, 
 *                             left coordinate, top coordinate, 
 *                             right coordinate. The returned vector
 *                             will only contain these three points.
 */
bool
calcArrowHeadCoords( uint32 arrowHeadSize,
                     int32 pointAtArrowHeadBaseY,
                     int32 pointAtArrowHeadBaseX,
                     int32 pointBeforeArrowHeadBaseY,
                     int32 pointBeforeArrowHeadBaseX,
                     vector<POINT>& arrowHeadCoordinates) {
   bool result = false;

   /* Illustration:
    *
    *            \
    *        dl  /|
    *           / | dy
    *         \/__|
    *         /|dx
    *        / |
    *   dL  /  |
    *      /   | dY
    *     /    |
    *    /     |
    *  \/______|
    *     dX
    *
    *
    * The base of the arrow symbol is put where dL and dl meet.
    */
   POINT lastPoint;
   lastPoint.y = pointAtArrowHeadBaseY;
   lastPoint.x = pointAtArrowHeadBaseX;

   POINT secoundLastPoint;
   secoundLastPoint.y = pointBeforeArrowHeadBaseY;
   secoundLastPoint.x = pointBeforeArrowHeadBaseX;

   int32 dX = lastPoint.x - secoundLastPoint.x;
   int32 dY = lastPoint.y - secoundLastPoint.y;
   double dL = sqrt(::pow(dX,2) + ::pow(dY,2));

   if (dL == 0){
      mc2log << error << "MapDrawer::calculateArrowHeahCoords. "
             << "Prevented division by zero. dL is == 0. "
             << "Could not calculate the coordinates."
             << endl;
      result = false;
   }
   else{

      int32 dl = arrowHeadSize;
      double quota = dl / dL;

      int32 dx = static_cast<int>(floor(0.5 + dX * quota));
      int32 dy = static_cast<int>(floor(0.5 + dY * quota));

      //Calculate the arrow head coordinates.
      POINT left;
      left.x = lastPoint.x - dy;
      left.y = lastPoint.y + dx;

      POINT top;
      top.x = lastPoint.x + dx;
      top.y = lastPoint.y + dy;

      POINT right;
      right.x = lastPoint.x + dy;
      right.y = lastPoint.y - dx;

      arrowHeadCoordinates.empty();
      arrowHeadCoordinates.push_back(left);
      arrowHeadCoordinates.push_back(top);
      arrowHeadCoordinates.push_back(right);

      result = true;
   }

   return result;
}

void
drawRouteAsArrow( GfxFeatureMap* map,
                  ImageDraw* imageDraw,
                  const DrawingProjection* drawingProjection,
                  int32 mapRotation,
                  uint32 afterTurnId, uint32 arrowHeadSize,
                  uint32 routeWidth,
                  GDUtils::Color::CoolColor color ) {
   mc2dbg8 << here
           << " afterTurnId = " << afterTurnId << endl
           << "mapRotation = " << mapRotation << endl
           << "arrowHeadSize = " << arrowHeadSize << endl;

   // Find first route item and check if a route is present in the map.
   uint32 i=0;
   uint32 firstEmptyFeatureId = MAX_UINT32;
   GfxFeature::gfxFeatureType featureType =  map->getFeature(i)->getType();
   mc2dbg8 << here << " Number of features in map = "
           << (int)map->getNbrFeatures() <<  endl;

   while( ( i < map->getNbrFeatures() ) &&
          ( featureType != GfxFeature::ROUTE ) ) {
      if ( (featureType == GfxFeature::EMPTY) &&
           (firstEmptyFeatureId == MAX_UINT32) ) {
         // Store the first empty id.
         firstEmptyFeatureId = i;
      }

      if ( (firstEmptyFeatureId != MAX_UINT32) &&
           ( (featureType != GfxFeature::EMPTY) &&
             (featureType != GfxFeature::ROUTE) ) ) {
         // The first empty feature was followd by a feature type other
         // than ROUTE or EMPTY. The stored value is not the one we're
         // looking for. Reset firstEmptyFeatureId.
         firstEmptyFeatureId = MAX_UINT32;
         mc2dbg8 << here
                 << " Set firstEmptyFeatureId back to MAX_UINT32" << endl;
      }

      i++;
      if ( i < map->getNbrFeatures() ) {
         featureType =  map->getFeature(i)->getType();
      }
   }
   uint32 firstRouteItemId = i;


   mc2dbg8 << here << " First empty item in map = "
           << firstEmptyFeatureId << endl;

   mc2dbg8 << here << " First route item in map = "
           << firstRouteItemId << endl;

   if (firstRouteItemId >= map->getNbrFeatures() ){
      return;
   }
   // A route was present in the map.

   uint32 afterTurnMapId = MAX_UINT32; //The after turn id, counted from
   //the first item in the map.

   if (firstEmptyFeatureId != MAX_UINT32){
      //There were empty features before the route items.
      afterTurnMapId = afterTurnId + 1 + firstEmptyFeatureId;
      // +1 must be used to compensate for the route origin feature.
   } else {
      afterTurnMapId = afterTurnId + firstRouteItemId;
      // +1 must be used to compensate for the route origin feature.
   }

   mc2dbg8 << here << " After turn id in map = "
           << afterTurnMapId << endl;

   // Find last route item.
   uint32 lastItemId = map->getNbrFeatures() - 1;
   i = lastItemId;
   while ( ( i > firstRouteItemId ) &&
           ( map->getFeature(i)->getType() != GfxFeature::ROUTE ) ) {
      i--;
   }
   uint32 lastRouteItemId = i;
   mc2dbg8 << here << " Last Route Item id in map = "
           << lastRouteItemId << endl;

   // Get parameters needed for transformation.
   uint16 width = map->getScreenX();
   uint16 height = map->getScreenY();

   mc2dbg8 << here << " Image size, image x=" << width
           << "image y=" <<  height << endl;

   // Bounding box corresponding to the sceen or image to draw to.
   MC2BoundingBox screenBBox(height, 0, 0, width);

   // Transform coordinates.

   vector<POINT> routeImageCoordinates;
   uint32 afterTurnCoordinateId = MAX_UINT32;

   const GfxPolygon* oldPolygon = NULL;
   for (uint32 i = firstRouteItemId; i < lastRouteItemId + 1; i++ ){
      const GfxFeature* actFeature = map->getFeature(i);

      for (uint32 j = 0; j < actFeature->getNbrPolygons(); j++){
         const GfxPolygon* actPolygon =
            actFeature->getPolygon(j);

         for(uint32 k = 0; k < actPolygon->getNbrCoordinates(); k++){
            int32 actLat = actPolygon->getLat(k);
            int32 actLon = actPolygon->getLon(k);

            bool addThisCoordinate = false;
            if (oldPolygon != NULL) {
               uint32 oldPolygonLastCoordId =
                  oldPolygon->getNbrCoordinates()-1;

               if((k==0) &&
                  (oldPolygon!=NULL) &&
                  (actLat ==
                   oldPolygon->getLat(oldPolygonLastCoordId)) &&
                  (actLon ==
                   oldPolygon->getLon(oldPolygonLastCoordId)) ) {
                  // Do not need to add this coordinate, already added.
                  addThisCoordinate = false;

                  if ( ( i == afterTurnMapId ) && ( j == 0 ) ){

                     // This is the coordinate right after the turn.
                     // Since the prior coordinate is the same as this,
                     // its id is set as the afterTurnCoordinateId.
                     afterTurnCoordinateId =
                        routeImageCoordinates.size() - 1;
                     mc2dbg8 << here
                             << " Id in coordinage vector for the "
                             << "after turn coordinate set to "
                             << afterTurnCoordinateId << endl;
                  }
               } else{
                  addThisCoordinate = true;
               }
            } // (oldPolygon not NULL)
            else{
               addThisCoordinate = true;
            }

            if (addThisCoordinate){
               if ( ( i == afterTurnMapId ) &&
                    ( j == 0 ) &&
                    ( k == 0 ) ){
                  // This is the coordinate right after the turn.
                  // Save its id in the transformed vector.
                  afterTurnCoordinateId =
                     routeImageCoordinates.size();
                  mc2dbg8 << here
                          << " Id in coordinage vector for the "
                          << "after turn coordinate set to "
                          << afterTurnCoordinateId << endl;
               }
               MC2Coordinate coord( actLat, actLon );
               MC2Point point = drawingProjection->getPoint( coord );

               POINT actPoint = {point.getX(), point.getY()};

               routeImageCoordinates.push_back(actPoint);
               mc2dbg8 << here
                       << " i: " << i
                       << " j: " << j
                       << " k: " << k
                       << " Point: x="  << actPoint.x
                       << " y=" << actPoint.y << endl;
            }  // ( add this coordinate)
            else {
               mc2dbg8 << here
                       << " i: " << i
                       << " j: " << j
                       << " k: " << k
                       << " Point was not added." << endl;
            }
         } // (for every coordinate)

         oldPolygon = actPolygon;
      } // (for every polygon)

   } // (for every feature)


   // Bounding box inside the screen bounding box, This edge
   // represeinting the distance from the screen bounding box edge,
   // the arrow heads base must be put in order to make the whole
   // arrow visible.
   MC2BoundingBox innerBBox(height - arrowHeadSize,
                            arrowHeadSize,
                            arrowHeadSize,
                            width - arrowHeadSize );

   if ( afterTurnCoordinateId != MAX_UINT32 ) {
      int32 afterTurnX =
         ( routeImageCoordinates[afterTurnCoordinateId].x);
      int32 afterTurnY =
         ( routeImageCoordinates[afterTurnCoordinateId].y);

      if ( innerBBox.contains(afterTurnY, afterTurnX ) ){
         // Find the fist point before the turn that is located
         // outside the outer bounding box.
         uint32 i = afterTurnCoordinateId;

         bool found = false;
         while( ( !found ) &&
                ( i > 0 ) ) {
            i--;
            int x = routeImageCoordinates[i].x;
            int y = routeImageCoordinates[i].y;

            found = !(screenBBox.contains( y, x) );

         }
         uint32 firstRouteCoordinateId = i;
         mc2dbg8 << here << " firstRouteCoordinateId = "
                 << firstRouteCoordinateId << endl;

         // Find the first point after the turn that is located
         // outside the outer bounding box.
         i = afterTurnCoordinateId;

         found = false;
         while ( ( !found ) &&
                 ( i < routeImageCoordinates.size() ) ){

            int x = routeImageCoordinates[i].x;
            int y = routeImageCoordinates[i].y;

            found = !(screenBBox.contains( y, x ) );
            i++;
         }
         uint32 lastPointId = i - 1;

         // Count backwards until a line segment that intersects
         // with the inner bounding box is found.
         found = false;
         i = lastPointId;
         while ( (!found) &&
                 ( i > afterTurnCoordinateId ) ){
            int32 x0 = routeImageCoordinates[i].x;
            int32 y0 = routeImageCoordinates[i].y;
            int32 x1 = routeImageCoordinates[i-1].x;
            int32 y1 = routeImageCoordinates[i-1].y;

            found = innerBBox.intersects( y0, x0, y1, x1 );
            i--;
         }
         uint32 lastRouteCoordinateId = i + 1;
         mc2dbg8 << here
                 << " Id in coordinate vector for the "
                 << "last route coordinate used for drawing"
                 << "the route set to "
                 << lastRouteCoordinateId << endl;

         // Clip the found line segment where it enters the
         // inner bounding box, entering from the end of the route.

         int32 x0 = routeImageCoordinates[lastRouteCoordinateId].x;
         int32 y0 = routeImageCoordinates[lastRouteCoordinateId].y;
         int32 x1 = routeImageCoordinates[lastRouteCoordinateId - 1].x;
         int32 y1 = routeImageCoordinates[lastRouteCoordinateId - 1].y;

         int32 clippedY0 = MAX_INT32;
         int32 clippedX0 = MAX_INT32;

         mc2dbg8 << here << " Last point in coordinage vector x=" << x0
                 << " y=" << y0 << endl;
         mc2dbg8 << here
                 << " Secound last point in coordinage vector x=" << x1
                 << " y=" << y1 << endl;
         DEBUG8 (
                 mc2dbg8 << here << "Inner bounding box." << endl;
                 innerBBox.dump();
                 )

            bool clipped =
            innerBBox.clipToFirstIntersectionWithEdge(y0, x0,
                                                      y1, x1,
                                                      clippedY0,
                                                      clippedX0);
         //Prevents the compiler from nagging about not using this
         //variable
         clipped = clipped;

         DEBUG8(
                if (clipped){
                   // The line segment was clipped.
                   mc2dbg8 << here << "The line segment was clipped."
                           << endl;
                }
                else{
                   // The line segemnt was left untouched.
                   mc2dbg8 << here << "The line segment was NOT clipped."
                           << endl;
                }
                )

            if ( (y1 == clippedY0) &&
                 (x1 == clippedX0) ){
               mc2dbg8 << here
                       << " Last point was equal to the point before."
                       << endl;

               lastRouteCoordinateId--;
               if (lastRouteCoordinateId < 0){
                  lastRouteCoordinateId = 0;
               }
            }

         routeImageCoordinates[lastRouteCoordinateId].y = clippedY0;
         routeImageCoordinates[lastRouteCoordinateId].x = clippedX0;

         vector<POINT> arrowHeadCoordinates;
         mc2dbg8 << here << " Clipped last point in coordinate vector"
                 << " x =" << clippedX0
                 << " y = " << clippedY0 << endl;

         // Get coordinates for arrow head.
         bool arrowHeadOk =
            calcArrowHeadCoords( arrowHeadSize,
                                 routeImageCoordinates[lastRouteCoordinateId].y,
                                 routeImageCoordinates[lastRouteCoordinateId].x,
                                 routeImageCoordinates[lastRouteCoordinateId-1].y,
                                 routeImageCoordinates[lastRouteCoordinateId-1].x,
                                 arrowHeadCoordinates );

         if (arrowHeadOk) {
            // Cuts away the elements after the lastRouteCoordinateId
            // element.
            routeImageCoordinates.resize(lastRouteCoordinateId + 1);

            // Do not use the elements before the firstRouteCoordinateId
            // element.
            vector<POINT> routePointsToUse;
            for (uint32 k = firstRouteCoordinateId;
                 k < routeImageCoordinates.size();
                 k ++ ) {
               routePointsToUse.push_back( routeImageCoordinates[k] );
            }

            imageDraw->drawRouteAsArrow( routePointsToUse,
                                         arrowHeadCoordinates,
                                         color,
                                         routeWidth );
         } else{
            mc2log << error << "MapDrawer::drawRouteAsArrow. "
                   << "Could not draw arrow head."
                   << endl;
         }
      } else {
         mc2log << info << "afterTurnY = " << afterTurnY
                << " afterTurnX = " << afterTurnX << endl;
         mc2log << error << "MapDrawer::drawRouteAsArrow. "
                << "After turn route first point was outside the "
                << "boundingbox."
                << endl;
      }
   } else{
      mc2log << info << "afterTurnMapId = " << afterTurnMapId << endl;
      mc2log << info << "afterTurnId = " << afterTurnId << endl;
      mc2log << error << "MapDrawer::drawRouteAsArrow. "
             << "Coordinate transformation never found "
             << "afterTurn coordinate."
             << endl;
   }
}

/* For debug
void MapDrawer::dumpFeat( const featurenotice_t& feat ) {
   if ( feat.m_feature ) {
      mc2dbg << "\"" << feat.m_feature->getName() << "\"";
      mc2dbg << ", t " << feat.m_feature->getType();
      if ( feat.m_feature->getType() == GfxFeature::POI ) {
         mc2dbg << " poi: " 
                << static_cast<const GfxPOIFeature*>( feat.m_feature )->getPOIType()
                << endl;
      }
   }
   mc2dbg << " do " << feat.m_drawOrder 
          << ", b " << feat.m_border
          << ", i " << feat.m_featureIndex
          << ", pi " << feat.m_polygonIndex
          << ", ps " << feat.m_poiStatus
          << ", v " << feat.m_visible << endl;
             
}

void MapDrawer::dumpFeat( const featuretextnotice_t& feat ) {

  mc2dbg << "s: " << feat.m_scaleLevel 
         << ", timp: " << feat.m_textImportanceLevel 
         << ", l: " << feat.m_length;
  if ( feat.m_feature ) {
    mc2dbg << ", n: \"" << feat.m_feature->getName() << "\"" 
           << ", c: " << feat.m_feature->getTextLat()
           << " " << feat.m_feature->getTextLon()
           << " " << GfxFeature::getFeatureTypeAsString( *feat.m_feature )
           << " , " 
           << feat.m_feature->getPolygon( 0 )->getCoord( 0 );
  }

  mc2dbg << endl;
}
*/

