/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CairoImageDraw.h"

#ifdef HAVE_CAIRO

#include "Cairo.h"

#include "DrawSettings.h"
#include "MC2Point.h"
#include "GfxPolygon.h"
#include "DrawingProjection.h"
#include "TextIterator.h"
#include "UTF8Util.h"
#include "Properties.h"
#include "StringTableUtility.h"
#include "GfxData.h"

#include "StringUtility.h"

#include <sys/stat.h>
#include <unistd.h>



using namespace GSystem::Cairo;
ostream& operator << ( ostream& ostr, const GSystem::Color& color ) {
   ostr << "(" << color.getRed() << ", " << color.getGreen() << ", " 
        << color.getBlue() << ")" << endl;
   return ostr;
}

namespace {

MC2String getFontPath( const MC2String& fontName ) {
   const char* fontPath = Properties::getProperty( "FONT_PATH" );
   if ( fontPath == NULL ) {
      fontPath = "./Fonts";
   }
   
   char absolutePath[256];
   absolutePath[ 0 ] = '\0';
   if ( fontPath[ 0 ] != '/') {
      // Add working directory to the relative path
      if ( getcwd( absolutePath, 255 ) != NULL ) {
         strcat( absolutePath, "/" );
      } else {
         mc2log << error << "[CairoImageDraw] getFontPath getcwd failed." << endl;
         perror( "   getcwd error: " ); 
      }
   }

   return MC2String( absolutePath ) + fontPath + "/" + fontName;
}

GSystem::Color getColor( GDUtils::Color::CoolColor color ) {
   return GSystem::Color( static_cast< uint32 >( color ) );
}

GSystem::Color getColor( GDUtils::Color::imageColor color ) {
   return getColor( GDUtils::Color::makeColor( color ) );
}

}

const uint32 CairoImageDraw::OUTLINE_SIZE = 1;

CairoImageDraw::CairoImageDraw( uint32 width, uint32 height,
                                GDUtils::Color::CoolColor color ):
   ImageDraw( width, height ),
   m_surface( new Surface( width, height ) ),
   m_gc( new GContext( *m_surface ) ),
   m_fontCache( 0 ),
   m_drawingProjection( NULL ),
   m_fontPath( ::getFontPath("") ) {

   m_fontCache.reset( new FontCache( m_fontPath + "fallback.ttf" ) );

   m_gc->setFill( true );
   m_gc->setFillColor( ::getColor( color ) );
   m_gc->setLineWidth( 0 );
   GSystem::Cairo::drawRectangle( *m_gc, 0, 0, 
                                  width, height );

   m_gc->setFill( false );
}

CairoImageDraw::~CairoImageDraw() {

}

byte* CairoImageDraw::getImageAsBuffer( uint32& size, 
                                        ImageDrawConfig::imageFormat format ) {
   try {
      pair< unsigned char*, int > content( NULL, 0 );
   
      if ( format == ImageDrawConfig::GIF ) {
         content = GSystem::saveGIF( *m_surface );
      } else if ( format == ImageDrawConfig::JPEG ) {
         content = GSystem::saveJPEG( *m_surface );
      } else {
         content = GSystem::savePNG( *m_surface );
      }

      size = content.second;
      return content.first;
   } catch ( const GSystem::Exception& e ) {
      mc2dbg << error << "[CairoImageDraw] Failed to convert image to buffer."
             << endl;
   }

   return NULL;
}


void CairoImageDraw::drawArc( int cx, int cy,
                              int startAngle, int stopAngle, 
                              int innerRadius, int outerRadius,
                              GDUtils::Color::CoolColor color,
                              int lineWidth ) {
   GContext::GCRestoreState state( *m_gc );
   m_gc->setLineWidth( lineWidth );
   m_gc->setColor( ::getColor( color ) );
   GSystem::Cairo::drawArc( *m_gc, cx, cy, 
                            startAngle, stopAngle,
                            innerRadius );
}

void CairoImageDraw::setDrawingProjection( const DrawingProjection* proj ) {
   m_drawingProjection = proj;
}

void CairoImageDraw::drawRectangle( int x1, int y1, int x2, int y2,
                                    GDUtils::Color::CoolColor color ) {
   m_gc->setColor( ::getColor( color ) );
   GSystem::Cairo::drawRectangle( *m_gc, x1, y1, x2, y2 );
}

void CairoImageDraw::drawBoundingBox( const MC2BoundingBox& bbox ) {
   MC2Coordinate upperCoord( bbox.getMaxLat(), bbox.getMinLon() );
   MC2Coordinate lowerCoord( bbox.getMinLat(), bbox.getMaxLon() );
   
   MC2Point upperPoint = m_drawingProjection->getPoint( upperCoord );
   MC2Point lowerPoint = m_drawingProjection->getPoint( lowerCoord );

   drawRectangle( upperPoint.getX(), upperPoint.getY(),
                  lowerPoint.getX(), lowerPoint.getY(),
                  static_cast<GDUtils::Color::CoolColor>( 0x000000 ) );
}


void CairoImageDraw::writeText( const char* text, 
                                int x, int y,
                                double fontSize,
                                GDUtils::Color::CoolColor col,
                                double angle, 
                                const char* fontName ) {
   GSystem::Cairo::Font* font = m_fontCache->
      loadFont( m_fontPath + fontName );

   if ( font == NULL ) {
      return;
   }

   font->setSize( fontSize );

   // convert text
   MC2String convertedText;
   for ( mc2TextIterator it = mc2TextIterator( text );
         *it != 0; ++it ) {
      char tmpText[ 10 ];
      UTF8Util::unicodeToUtf8( *it, tmpText );
      convertedText += tmpText;
   }

   font->drawText( *m_gc, x, y, 
                   angle * 180.0 / M_PI, 
                   convertedText.c_str() );

}

void CairoImageDraw::drawTurnArrow( int x0, int y0, int x1, int y1, 
                                    int x2, int y2, int x3, int y3,
                                    int arrowX0, int arrowY0,
                                    int arrowX1, int arrowY1,
                                    int arrowX2, int arrowY2,
                                    int32 arrowAngle, int32 arrowLength,
                                    GDUtils::Color::CoolColor col,
                                    int arrowWidth ) {
   vector<int> resX;
   vector<int> resY;
   float64 angle = 0;
   GSystem::Color color = ::getColor( col );

   GfxUtility::makeSpline( x0, y0, x1, y1, 
                           x2, y2, x3, y3,
                           resX, resY, 0.05, &angle );

   // Draw spline line
   int oldX = MAX_INT32;
   int oldY = MAX_INT32;
   
   if ( resX.size() > 0 ) {
      oldX = resX[ 0 ];
      oldY = resY[ 0 ];
   }

   m_gc->setLineJoin( GSystem::GContext::LINE_JOIN_ROUND );
   m_gc->setLineWidth( arrowWidth );
   m_gc->setLineCap( GSystem::GContext::LINE_CAP_ROUND );
   m_gc->setColor( color );
   {
      // scoped line drawing
      GSystem::Cairo::LineDrawing line( *m_gc, oldX, oldY );

      for ( uint32 i = 1 ; i < resX.size() ; i++ ) {
         line.lineTo( resX[ i ], resY[ i ] );
      }
   }

   float64 arrowAng = GfxConstants::degreeToRadianFactor * arrowAngle;

   // Move arrow so its tail is at the end of spline
   int x = resX[ resX.size() - 1 ] + 
      int32( rint( cos( angle ) * (arrowLength * cos( arrowAng ) ) ) );

   int y = resY[ resX.size() - 1 ] - 
      int32( rint( sin( angle ) * (arrowLength * cos( arrowAng ) ) ) );

   arrowX1 = x;
   arrowY1 = y;

   arrowX0 = int32( rint( x - cos( angle - arrowAng ) * arrowLength ) );
   arrowY0 = int32( rint( y + sin( angle - arrowAng ) * arrowLength ) );
   arrowX2 = int32( rint( x - cos( angle + arrowAng ) * arrowLength ) );
   arrowY2 = int32( rint( y + sin( angle + arrowAng ) * arrowLength ) );


   // Draw Arrow
   vector<GSystem::Point> points;
   points.push_back( GSystem::Point( arrowX0, arrowY0 ) );
   points.push_back( GSystem::Point( arrowX1, arrowY1 ) );
   points.push_back( GSystem::Point( arrowX2, arrowY2 ) );
   
   m_gc->setFillColor( color );
   drawPolygon( *m_gc, points );

}
void CairoImageDraw::drawRouteAsArrow( vector<POINT> route,
                                       vector<POINT> arrowHead,
                                       GDUtils::Color::CoolColor color,
                                       uint32 routeWidth ) {
   mc2dbg << "[CairoImageDraw] " << __FUNCTION__ << " not implemented. " << endl;
}


void CairoImageDraw::drawScale( MC2BoundingBox* bbox,
                                StringTable::languageCode lang ) {

   uint16 width = m_surface->getWidth();
   uint16 height = m_surface->getHeight();

   const float64 scalePartOfWidth = 0.40;
   int bottomMargin = 8;
   int sideMargin = 8;
   int barTextSpacing = 2;
   int barWidth = 5;
   int textMargin = 2;

   float64 fontSize = 10.0;
   if ( width > 250 ) { // > 250 pxl
      fontSize = 10.0;
      bottomMargin = sideMargin = 6;
   } else if ( width > 150 ) { // 250-151 pxl
      fontSize = 8.0;
      bottomMargin = sideMargin = 4;
      barWidth = 4;
   } else { // less than 150 pxl
      fontSize = 6.0;
      bottomMargin = sideMargin = 2;
      textMargin = 1;
      barWidth = 3;
   }

   int rightPosX = width - sideMargin;
   int posY = height - bottomMargin;
   int leftPosX = int( rint( rightPosX - scalePartOfWidth*width ) );

   MC2Point right( rightPosX, 0 );
   int32 rightLon = m_drawingProjection->getCoordinate( right ).lon;
   MC2Point left( leftPosX, 0 );
   int32 leftLon = m_drawingProjection->getCoordinate( left ).lon;
   int32 distance = int32( rint( ( (rightLon - leftLon)*
                                   GfxConstants::MC2SCALE_TO_METER*
                                   bbox->getCosLat() ) ) );
      
   char distStr[512];

   //** Make distance nice length
   // First number zeros
   int32 logDist = 10;
   while( distance > logDist ) {
      logDist *= 10;
   }
   logDist /= 10;
   // Then some fine tuning
   int32 niceDist = 0;
   // Should be even ^2 but 1 is allowed too
   if ( 2*logDist < distance ) {
      niceDist = 2*logDist;
   } else {
      niceDist = logDist;
   }
   while ( niceDist + 2*logDist < distance ) {
      niceDist += 2*logDist;
   }
   
   //** Nice distance as string
   distStr[ 0 ] = '\0';
   StringTableUtility::printDistance( distStr, niceDist, 
                                 StringTable::getShortLanguageCode( lang ),
                                 StringTableUtility::NORMAL,
                                 StringTableUtility::METERS );
   
   //** Update leftLon and leftPosX for new nice distance
   leftLon = (rightLon - int32 ( rint( 
      niceDist*GfxConstants::METER_TO_MC2SCALE / bbox->getCosLat() ) ) );
   MC2Coordinate coord( 0, leftLon );
   MC2Point point = m_drawingProjection->getPoint( coord );
   leftPosX = point.getX();   
   
   //** Draw scalebar with four parts
   float64 partWidth = (rightPosX - leftPosX) / 4.0;

   using GSystem::Cairo::drawRectangle;

   // Fill all with white
   GSystem::Color white( 255, 255, 255 );
   GSystem::Color black( 0, 0, 0 );

   m_gc->setFillColor( white );
   m_gc->setFill( true );
   m_gc->setLineWidth( 0 ); // black border
   m_gc->setColor( black ); 

   drawRectangle( *m_gc,
                  leftPosX, posY - barWidth,
                  rightPosX, posY );

   m_gc->setFillColor( black );

   // Leftmost black part

   drawRectangle( *m_gc, 
                  leftPosX, posY - barWidth,
                  leftPosX + int( rint( partWidth ) ), posY );
   // Second black part

   drawRectangle( *m_gc,
                  leftPosX + int( rint( 2*partWidth ) ), posY - barWidth,
                  leftPosX + int( rint( 3*partWidth ) ), posY );


   // Draw distance string in own square text box

   GSystem::Cairo::Font* font = m_fontCache->
      loadFont( m_fontPath + "Vera.ttf" );
   if ( font == NULL ) {
      return;
   }

   font->setSize( fontSize );
   GSystem::Cairo::Font::Extents extents;
   // Get size of distStr
   font->getTextExtents( *m_gc, extents, distStr );


   // Use textArea for textbox
   int textWidth = extents.width;
      
   // Center textbox above scalebar
   int textXCenter = rightPosX - int( rint( 2*partWidth ) );
   int textYBottom = posY - barWidth - barTextSpacing - textMargin;
   // Make sure that text fits in image
   if ( textXCenter + textWidth/2 + textMargin + sideMargin >= width ) {
      textXCenter = width - textWidth/2 - textMargin - sideMargin;
   }


   int x = textXCenter - textWidth/2;
   int y = textYBottom;
   m_gc->setColor( black );
   font->drawText( *m_gc, 
                  x, y, 0, // x, y, angle
                  distStr );

}

void CairoImageDraw::drawGfxData( const GfxData& data,
                                  uint32 lineWidth,
                                  GDUtils::Color::imageColor color ) {

   m_gc->setLineJoin( GSystem::GContext::LINE_JOIN_ROUND );
   m_gc->setLineWidth( lineWidth );
   m_gc->setColor( ::getColor( color ) );

   for ( uint32 polyIndex = 0; polyIndex < data.getNbrPolygons();
         ++polyIndex ) {
      MC2Coordinate startCoord = data.getCoordinate( polyIndex, 0 );
      MC2Point startPos( m_drawingProjection->getPoint( startCoord ) );
      uint32 nbrCoords = data.getNbrCoordinates( polyIndex );

      GSystem::Cairo::LineDrawing line( *m_gc,
                                        startPos.getX(), startPos.getY() );
      for ( uint32 coordIndex = 1; coordIndex < nbrCoords; ++coordIndex ) {
         MC2Coordinate coord = data.getCoordinate( polyIndex, coordIndex );
         MC2Point pixelPoint( m_drawingProjection->getPoint( coord ) );
         line.lineTo( pixelPoint.getX(), pixelPoint.getY() );
      }
   }
}

bool CairoImageDraw::
drawGfxFeaturePolygon( const GfxFeature* feature,
                       GfxPolygon* polygon,
                       DrawSettings* settings,
                       ImageTable::ImageSet imageSet ) {
   
   const uint32 nbrCoords = polygon->getNbrCoordinates();

   int32 prevLat = polygon->getLat( 0 );
   int32 prevLon = polygon->getLon( 0 );
   MC2Point tmp = m_drawingProjection->
      getPoint( MC2Coordinate( prevLat, prevLon ) );
   int32 prevX = tmp.getX();
   int32 prevY = tmp.getY();
   
   switch ( settings->m_style ) {
   case DrawSettings::LINE: {



      m_gc->setLineJoin( GSystem::GContext::LINE_JOIN_ROUND );
      m_gc->setLineWidth( settings->m_lineWidth );
      m_gc->setColor( ::getColor( settings->m_color ) );

      if ( settings->m_lineWidth >= 2 ) {
         m_gc->setLineCap( GSystem::GContext::LINE_CAP_ROUND );
      }

      // Special handling for the dashed ferry and railway lines
      double dashes[] = {15.0, 5.0};
      if ( feature->getType() == GfxFeature::FERRY ) {
         cairo_set_dash( m_gc->getContext(), dashes, 2, 0);
      } else if ( feature->getType() == GfxFeature::RAILWAY ) {
         cairo_set_dash( m_gc->getContext(), dashes, 0, 0);
         m_gc->setColor( GSystem::Color( 0xa6, 0xa6, 0xa6 ) );
         
         { // Extra block for scope of the line
            GSystem::Cairo::LineDrawing line( *m_gc, prevX, prevY );
            int pLat = prevLat;
            int pLon = prevLon;
            
            for ( uint32 i = 1; i < nbrCoords; ++i ) {
               int32 lat = polygon->getLat( i, pLat );
               int32 lon = polygon->getLon( i, pLon );
               MC2Coordinate currCoord( lat, lon );
               MC2Point p( m_drawingProjection->getPoint( currCoord ) );
               
               line.lineTo( p.getX(), p.getY() );
               pLat = lat;
               pLon = lon;
            }
         }
         // Set rail dashes and real color.
         double railDashes[] = {4.0, 6.0};
         cairo_set_dash( m_gc->getContext(), railDashes, 2, 0);
         m_gc->setColor( ::getColor( settings->m_color ) );
      }else {
         cairo_set_dash( m_gc->getContext(), dashes, 0, 0); 
      }

      GSystem::Cairo::LineDrawing line( *m_gc, prevX, prevY );
      
      
      for ( uint32 i = 1; i < nbrCoords; ++i ) {
         int32 lat = polygon->getLat( i, prevLat );
         int32 lon = polygon->getLon( i, prevLon );
         MC2Coordinate currCoord( lat, lon );
         MC2Point p( m_drawingProjection->getPoint( currCoord ) );
         
         line.lineTo( p.getX(), p.getY() );
         prevLat = lat;
         prevLon = lon;
         prevX = p.getX();
         prevY = p.getY();
      }
   } break;
   
   case DrawSettings::CLOSED:
   case DrawSettings::FILLED: {
      
      if ( nbrCoords <= 1 ) {
         break;
      }

      using GSystem::Point;

      vector<Point> points;
      points.reserve( nbrCoords );
      points.push_back( Point( prevX, prevY ) );
      
      for ( uint32 i = 1; i < nbrCoords; i++ ) {
         int32 lat = polygon->getLat( i, prevLat );
         int32 lon = polygon->getLon( i, prevLon );
         MC2Coordinate currCoord( lat, lon );

         points.push_back( m_drawingProjection->getPoint( currCoord ) );

         prevLat = lat;
         prevLon = lon;
      }

      m_gc->setFill( settings->m_style == DrawSettings::FILLED );

      if ( m_gc->useFill() ) { 
         m_gc->setFillColor( ::getColor( settings->m_color ) );
         m_gc->setLineWidth( 0 ); 
      } else {
         m_gc->setColor( ::getColor( settings->m_color ) );
         m_gc->setLineWidth( settings->m_lineWidth ); 
      }
      drawPolygon( *m_gc, points );
      
   } break;

   case DrawSettings::SYMBOL:

      drawSymbol( settings->m_symbol, prevX, prevY, settings,
                  feature, imageSet );
      for ( uint32 i = 1 ; i < nbrCoords; i++ ) {
         int32 lat = polygon->getLat(i, prevLat);
         int32 lon = polygon->getLon(i, prevLon);
         MC2Coordinate currCoord( lat, lon );
         MC2Point p = m_drawingProjection->getPoint( currCoord );
         drawSymbol( settings->m_symbol, p.getX(), p.getY(), settings,
                     feature, imageSet );
         
         prevLat = lat;
         prevLon = lon;
      }

      break;
   }

   return true;
}

void CairoImageDraw::drawSymbol( DrawSettings::symbol_t sym,
                                 int x, int y,
                                 DrawSettings* settings,
                                 const GfxFeature* feature,
                                 ImageTable::ImageSet imageSet ) {

   if ( sym == DrawSettings::SQUARE_3D_SYMBOL ) {
      m_gc->setFill( true );
      m_gc->setFillColor( ::getColor( GDUtils::Color::BROWN ) );
      m_gc->setLineWidth( 0 );
      GSystem::Cairo::drawRectangle( *m_gc, x + 3, y + 3, x + 13, y + 13 );
      m_gc->setFillColor( ::getColor( GDUtils::Color::INDIANRED ) );
      GSystem::Cairo::drawRectangle( *m_gc, x, y, x + 10, y + 10 );
      return;
   } else if ( sym == DrawSettings::SMALL_CITY_SYMBOL ) {
      m_gc->setFill( true );
      m_gc->setFillColor( ::getColor( GDUtils::Color::LIGHTSLATEGREY ) );
      m_gc->setLineWidth( 0 );
      GSystem::Cairo::drawArc( *m_gc, x, y, 10, 0, 360 );
      return;      
   } 

   MC2String filename = getSymbolFilename( sym, *settings, feature, imageSet );
   // if size is less or equal to 4 then we can be sure it does not have ".png"
   // ending. But if its larger then we compare the last four bytes and append
   // the ".png" ending if it does not match.
   if ( filename.size() <= 4 ||
        ( filename.size() > 4 &&
          filename.compare( filename.size() - 4, 4, ".png" ) != 0 ) ) {
      filename += ".png";
   } 
   auto_ptr<GSystem::Cairo::Surface> 
      surf( GSystem::Cairo::loadPNG( filename ) );

   if ( surf.get() == NULL ) {
      mc2dbg << warn << "[CairoImageDraw] Failed to load png: " << filename << endl;
      return;
   }

   // compensate route
   if ( sym == DrawSettings::ROUTE_DESTINATION_SYMBOL ) {
      // Compensate so that the flag stands on the route.
      x += 5;
      y -= surf->getHeight() / 2 - 1;
   }

   blitSurface( *m_gc, *surf, x, y );
}

void CairoImageDraw::
drawGfxFeatureRoadSign( const char* name, const char* signName,
                        int32 lat, int32 lon, 
                        float32 border,
                        double fontSize,
                        GDUtils::Color::CoolColor signColor,
                        const char* fontName ) {

   auto_ptr<GSystem::Cairo::Surface>
      surf( GSystem::Cairo::loadPNG( getImageFullPath( signName ) ) );

   if ( surf.get() == NULL ) {
      return;
   }
   GSystem::Cairo::Font* font = m_fontCache->
      loadFont( m_fontPath + fontName );
   if ( font == NULL ) {
      return;
   }

   MC2Point p = m_drawingProjection->getPoint( MC2Coordinate( lat, lon ) );

   // draw background surface first
   blitSurface( *m_gc, *surf, 
                p.getX() + surf->getWidth() / 2, p.getY() );

   // draw text on background
   m_gc->setColor( ::getColor( signColor ) );
   

   font->setSize( fontSize * 1.3 );
   GSystem::Cairo::Font::Extents extents;
   font->getTextExtents( *m_gc, extents, name );

   int centerX = surf->getWidth() / 2;
   //   int centerY = surf->getHeight() / 2;
   
   int textPosX = p.getX() + centerX - extents.width / 2;
   int textPosY = p.getY() + extents.height / 2;

   font->drawText( *m_gc, textPosX, textPosY, 0, name );
}

namespace { 

/**
 * Determine if outline should be used.
 * @param font the current font in use
 * @param fontSize the size of the font
 * @return true if outline should be used
 */
bool useOutline( const Font& font, float64 fontSize ) {
   const float64 boldFontSize = 11;

   return  
      font.getWeight() == GSystem::Font::WEIGHT_BOLD ||
      fontSize > boldFontSize;
}

 
}

void CairoImageDraw::drawText( GSystem::Cairo::Font& font,
                               float64 fontSize, double angle,
                               const MC2Point& startPos,
                               const char* text ) {
   if ( useOutline( font, fontSize ) ) {
      MC2String str = UTF8Util::mc2ToUtf8( text );
      font.setOutlineColor( GSystem::Color( 255, 255, 255 ) );
      font.setOutlineSize( CairoImageDraw::OUTLINE_SIZE );
      font.drawText( *m_gc, startPos.getX(), startPos.getY(), 
                     angle * 180 / M_PI, 
                     str.c_str() );
      font.setOutlineSize( 0 ); // restore outline size
   } else {
      font.drawText( *m_gc, startPos.getX(), startPos.getY(), 
                     angle * 180 / M_PI,
                     UTF8Util::mc2ToUtf8( text ).c_str() );
   }
}

void CairoImageDraw::
drawGfxFeatureText( const char* text, 
                    const char* altLangText,
                    int32 lat, int32 lon,
                    double fontSize,
                    GDUtils::Color::CoolColor col,
                    double angle,
                    bool moveText,
                    const char* fontFilename ) {

   angle = -angle;

   MC2Point startPos = m_drawingProjection->
      getPoint( MC2Coordinate( lat, lon ) );

   if ( isnan( angle ) ) {
      mc2dbg << error << "[CairoImageDraw] angle is not a valid number!" 
             << endl;
      // the freakin' MapDrawer...! one day I will come for you.
      // fallback to zero
      angle = 0;
   }

   GContext::GCRestoreState state( *m_gc );

   GSystem::Cairo::Font* font = 
      m_fontCache->loadFont( m_fontPath + fontFilename );
   if ( font == NULL ) {
      return;
   }

   font->setSize( fontSize * 1.3 );
   m_gc->setColor( ::getColor( col ) );

   // see we need to split the string
   if ( moveText ) {
      MC2String newText = text;
      size_t splitPos = newText.find_last_of( ' ' );
      if ( splitPos != MC2String::npos ) {
         // split the string into two and then draw them
         MC2String firstString = newText.substr( 0, splitPos );
         MC2String secondString = 
            newText.substr( splitPos + 1, 
                            splitPos - newText.size() - 1 );

         drawTexts( firstString, secondString, font, fontSize, font, fontSize, startPos, angle);

         // the entire string has been drawn FOO! nuttin' more to do.
         return;
      }
   }

   if ( altLangText != NULL && strcmp(text, altLangText ) != 0 ) {
      MC2String mainText( text );
      MC2String altText( altLangText );

      GSystem::Cairo::Font* font2 = new GSystem::Cairo::Font( m_fontPath + fontFilename );
      if ( font2 == NULL ) {
         // Fallback to same font
         font2 = font;
      }

      float64 fontSize2 = fontSize * 0.9;
      font2->setSize( fontSize2 );
      drawTexts( mainText, altText, font, fontSize, font2, fontSize2, startPos, angle);
   } else {
      // Only one text to draw
      drawText( *font, fontSize, angle, startPos, text );
   }
}

void CairoImageDraw::
drawGfxFeatureText( const char* text, 
                    int32 lat, int32 lon,
                    double fontSize,
                    GDUtils::Color::CoolColor col,
                    double angle,
                    bool moveText,
                    const char* fontFilename ) {
   drawGfxFeatureText(text, NULL, lat, lon, fontSize, col, angle, moveText, fontFilename);
}

void CairoImageDraw::
drawGfxFeatureText( const char* text, 
                    vector<GfxDataTypes::textPos>& textPosition,
                    double fontSize,
                    GDUtils::Color::CoolColor color,
                    const char* fontName ) {

   char tmpText[10]; // Room for 6 characters
   mc2TextIterator tit = mc2TextIterator( text ); 
   for ( vector<GfxDataTypes::textPos>::iterator it = textPosition.begin();
         it != textPosition.end();
         ++it ) {
      UTF8Util::unicodeToUtf8( *tit, tmpText );
      drawGfxFeatureText( tmpText, (*it).lat, (*it).lon,
                          fontSize,
                          color,
                          (*it).angle, 
                          false, // moveText
                          fontName );
      ++tit;
   }
}

void CairoImageDraw::
drawTexts( const MC2String& string1, const MC2String& string2,
           GSystem::Cairo::Font* font1, float64 fontSize1,
           GSystem::Cairo::Font* font2, float64 fontSize2,
           const MC2Point& startPos, double angle ) {

   // determine the next row y-position for the first part
   // of the string
   Font::Extents extents1;
   font1->getTextExtents( *m_gc, extents1, string1.c_str() );
   if ( ::useOutline( *font1, fontSize1 ) ) {
      // outline size on both sides, up and down or left and right.
      extents1.height += OUTLINE_SIZE * 2;
   }

   Font::Extents extents2; 
   font2->getTextExtents( *m_gc, extents2, string2.c_str() );
   if ( ::useOutline( *font2, fontSize2 ) ) {
      // outline size on both sides, up and down or left and right.
      extents2.height += OUTLINE_SIZE * 2;
   }

   // Center the second text below the first   
   int32 firstTextDelta = 0;
   int32 secondTextDelta = 0;

   if( extents2.width > extents1.width ) {
      // Second text is longer, move the first to the right
      firstTextDelta = ( extents2.width - extents1.width ) / 2;
   } else {
      // Move second text
      secondTextDelta = ( extents1.width - extents2.width ) / 2;
   }

   // start position is in the lower left parts, so we need
   // to draw it one text-line above this
   MC2Point firstPoint( startPos.getX() + firstTextDelta, 
                        startPos.getY() - extents2.height );
   
   // draw first part of the string
   drawText( *font1, fontSize1, angle, firstPoint,
             string1.c_str() );
   
 
   MC2Point secondPoint( startPos.getX() + secondTextDelta,
                         startPos.getY() + OUTLINE_SIZE );
   
   drawText( *font2, fontSize2, angle, 
             secondPoint,
             string2.c_str() );
   
   
}

void CairoImageDraw::getPixel( uint32 x, uint32 y, 
                               unsigned char& red, 
                               unsigned char& green, 
                               unsigned char& blue ) {
   mc2dbg << "[CairoImageDraw] " << __FUNCTION__ << endl;

}

bool CairoImageDraw::
getGlyphDimensions( int fontSize,
                    const char* fontFilename,
                    const char* text,
                    vector<GfxDataTypes::dimensions>& dimensions,
                    float64 xFactor,
                    float64 yFactor ) {
   using GSystem::Cairo::Font;

   Font* font = m_fontCache->loadFont( m_fontPath + fontFilename );
   if ( font == NULL ) {
      return false;
   }

   font->setSize( fontSize * 1.3 ); // magic 1.3 factor...

   Font::Extents extents;

   char tmpText[ 10 ];
   for ( mc2TextIterator it = mc2TextIterator( text );
         *it != 0; ++it ) {

      UTF8Util::unicodeToUtf8( *it, tmpText );
      font->getTextExtents( *m_gc, extents, tmpText );

      GfxDataTypes::dimensions dim;
      
      dim.width = 
         static_cast<int32>( ( extents.x_advance + 1  ) * xFactor );
      dim.height = 
         static_cast<int32>( extents.height * yFactor );
  
      dimensions.push_back( dim );
   }

   return true;

}
GfxDataTypes::dimensions 
CairoImageDraw::getStringDimension( int fontSize,
                                    const char* fontName,
                                    const char* text,
                                    float64 xFactor, float64 yFactor ) const {
   Font* font = m_fontCache->loadFont( m_fontPath + fontName );
   GfxDataTypes::dimensions dim = { 0, 0 };
   if ( font == NULL ) {
      mc2dbg << "Failed to load font: " << fontName << endl;
      return dim;
   }
   Font::Extents extents;
   font->setSize( fontSize * 1.3 ); // ...magic 1.3 factor..
   font->getTextExtents( *m_gc, extents, text );

   dim.width = static_cast< uint32 >( ceil(extents.x_advance * xFactor) );
   dim.height = static_cast< uint32 >( ceil(extents.height * yFactor * 0.7) );
   if ( ::useOutline( *font, fontSize ) ) {
      dim.width += static_cast< uint32 >( ceil(OUTLINE_SIZE * 2 * xFactor) );
      dim.height += static_cast< uint32 >( ceil(OUTLINE_SIZE * 2 * yFactor) );
   }

   return dim;
}

void CairoImageDraw::cutImage( uint32 width, uint32 height ) {
   
}

#endif // HAVE_CAIRO
