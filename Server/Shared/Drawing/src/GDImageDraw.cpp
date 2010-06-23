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

#include "MC2BoundingBox.h"
#include "TextIterator.h"
#include "UTF8Util.h"
#include "MC2Point.h"

#include "GDImageDraw.h"
#include "DebugClock.h"
#include "MC2Point.h"
#include "ScopedArray.h"
#include "GDImagePtr.h"
#include "FilePtr.h"
#include "StringTableUtility.h"
#include "GSystem.h"

// In memory image buffer
#include "gd_io.h"

// ImageMagic for transferpixels
#undef _XOPEN_SOURCE
#include <magick/api.h>

// Properties for paths to images
#include "Properties.h"

// GfxUtility for degree->radian conversion
#include "GfxUtility.h"

// GfxFeature
#include "GfxPolygon.h"


// Thread safety
#include "ISABThread.h"

// StringHandling
#include "StringUtility.h"

// MC2Map <-> image coordiante conversion
#include "MapUtility.h"

// Some low level methods (getcwd)
#include "sockets.h"

#include "Math.h"

bool GDImageDraw::m_imagesInitialized = false;

// Brush cache
BinarySearchTree GDImageDraw::m_brushes;

ISABMonitor GDImageDraw::m_brushMon;

GDImageDraw::BrushNotice GDImageDraw::m_findBrush;

// Font thread safe
ISABMonitor GDImageDraw::m_fontMonitor;

// Image cache
BinarySearchTree GDImageDraw::m_images;

ISABMonitor GDImageDraw::m_imagesMon;

GDImageDraw::ImageNotice GDImageDraw::m_findImage;


FT_Library GDImageDraw::m_ftLibrary = NULL;

// Face cache
BinarySearchTree GDImageDraw::m_faces;

ISABMonitor GDImageDraw::m_facesMon;

GDImageDraw::FaceNotice GDImageDraw::m_findFace;

// Make our own arcs that makes circles that are inflated not imploded
extern int gdCosT[];
extern int gdSinT[];


int getColor( gdImagePtr image, GDUtils::Color::CoolColor color ) {
   int index = 0;
   using namespace GDUtils;
   GSystem::Color gsColor( static_cast< uint32 >( color ) );

   index = gdImageColorExact( image,
                              gsColor.getRed(),
                              gsColor.getGreen(),
                              gsColor.getBlue() );
                              
   if ( index == -1 ) {
      // No such color
      index = gdImageColorAllocate( image, 
                                    gsColor.getRed(),
                                    gsColor.getGreen(),
                                    gsColor.getBlue() );
      if ( index == -1 ) {
         // All colors allocated
         index = gdImageColorClosest( image, 
                                      gsColor.getRed(),
                                      gsColor.getGreen(),
                                      gsColor.getBlue() );
      }
   }

   return index;
}

int getColor( gdImagePtr image, GDUtils::Color::imageColor color ) {
   return getColor( image, GDUtils::Color::makeColor( color ) );
}


GDImageDraw::GDImageDraw( uint32 xSize, 
                          uint32 ySize, 
                          GDUtils::Color::CoolColor color ) 
      : ImageDraw( xSize, ySize )
{
   // Initialize the freetype library.
   ISABSync sync( m_facesMon );
   if (m_ftLibrary == NULL) {
      FT_Init_FreeType( &m_ftLibrary );
   }

#ifdef USE_TRUE_COLOR
   // True color only works in the version we have on centos installs
   m_image = gdImageCreateTrueColor( m_width, m_height );
#else
   m_image = gdImageCreate( m_width, m_height );
#endif
   
   // Fill image
   int white = getColor( color );   
   gdImageFilledRectangle( m_image, 0, 0, m_width, m_height, white );
  
   // Initialize the images
   initializeDrawImages();
}


GDImageDraw::~GDImageDraw() {

   gdImageDestroy( m_image );   
}

byte* 
GDImageDraw::getImageAsBuffer( uint32& size, 
                               ImageDrawConfig::imageFormat format ) 
{
   DebugClock clock;
   byte* out = NULL;

   int gdSize = 0;

#ifndef USE_TRUE_COLOR
   int nbrColors = gdImageColorsTotal( m_image );
   mc2dbg << "[GDID]: Resulting image has " << nbrColors << " colors"
          << endl;
#endif
   
   switch ( format ) {
      case ImageDrawConfig::PNG: {
#ifdef USE_TRUE_COLOR
         // Create temporary 256-color image, since all browsers
         // etc. do not support full color pngs.
         gdImagePtr tmpimage =
            gdImageCreatePaletteFromTrueColor( m_image,
                                               true, /* dither */
                                               256 );
         // This is only found in newer gd         
         void* pngData = gdImagePngPtrEx( tmpimage, &gdSize, 9 );
         // Get rid of the temporary image.
         gdImageDestroy( tmpimage );
#else
         void* pngData = gdImagePngPtr( m_image, &gdSize );
#endif
         size = gdSize;
         out = new byte[ size ];
         memcpy( out, pngData, size );
         gdFree( pngData );
      }
      break;
      case ImageDrawConfig::WBMP: {
//         void* wbmpData = gdImageWBMPPtr( m_image, &gdSize, 1/*fg*/ );
//         size = gdSize;
//         out = new byte[ size ];
//         memcpy( out, wbmpData, size );
//         gdFree( wbmpData );
         // FIXME: Change back to GD if colors are better than in my test!
         out = convertImageToBuffer( size, ImageDrawConfig::WBMP ); 
      }
      break;
      case ImageDrawConfig::JPEG: {
         out = convertImageToBuffer( size, ImageDrawConfig::JPEG ); 
      }
      break;
      case ImageDrawConfig::GIF: {
#ifdef LZW_LICENCED
         void* gifData = gdImageGifPtr( m_image, &gdSize );
         size = gdSize;
         out = new byte[ size ];
         memcpy( out, gifData, size );
         gdFree( gifData );
#else 
         out = convertImageToBuffer( size, ImageDrawConfig::GIF ); 
#endif
      }
      break;
      default: {
         out = convertImageToBuffer( size, format );
      }
      break;
   }

   mc2dbg << "[GDID]: Conversion time " << clock << endl;
   
   return out;
}


void
GDImageDraw::writeText(const char* text, int x, int y,
                       double fontSize,
                       GDUtils::Color::CoolColor color,
                       double angle, const char* fontName )
{
   char font[512];
   getFontPath( font, fontName );
   
   // Set font-size and color
   int col = getColor( color );

   // Write the text in the image
   ISABSync sync( m_fontMonitor );
   char* err = gdImageStringFT( m_image, NULL, col, font,
                               fontSize, angle, x, y, 
                                const_cast<char*>(text));
   if (err != NULL) {
      mc2log << error << "GDImageDraw::writeText error drawing text "
             << endl << "   " << err << " text " << text << endl;
   }
}

void
GDImageDraw::setDrawingProjection( const DrawingProjection* drawingProjection )
{
   m_drawingProjection = drawingProjection;
}


bool 
GDImageDraw::drawGfxFeaturePolygon(const GfxFeature* feature,
                                   GfxPolygon* polygon,
                                   DrawSettings* settings,
                                   ImageTable::ImageSet imageSet )
{
   // Get the number of coordinates and make sure that we have any
   const uint32 nbrCoords = polygon->getNbrCoordinates();
   /*
   if (nbrCoords == 0) {
      mc2log << error << "Tries to draw polygon without any coordinates"
             << endl;
      return (false);
   }
   */
  
   // Get the first coordinate in (lat,lon) and (x,y) and the color
   int32 prevLat = polygon->getLat(0);
   int32 prevLon = polygon->getLon(0);
   MC2Coordinate coord( prevLat, prevLon );
   MC2Point tmp = m_drawingProjection->getPoint( coord );
   int prevX = tmp.getX();
   int prevY = tmp.getY();
   
   int color = getColor( settings->m_color );

   switch (settings->m_style) {
      case DrawSettings::LINE :
         
         for ( uint32 i = 1 ; i < nbrCoords; i++ ) {
            int32 lat = polygon->getLat(i, prevLat);
            int32 lon = polygon->getLon(i, prevLon);
            MC2Coordinate currCoord( lat, lon );
            MC2Point p = m_drawingProjection->getPoint( currCoord );

            roundedLine(prevX, prevY, p.getX(), p.getY(), 
                        color, settings->m_lineWidth);
            
            prevLat = lat;
            prevLon = lon;
            prevX = p.getX();
            prevY = p.getY();
         }
         break;

      case DrawSettings::CLOSED :
      case DrawSettings::FILLED : {
         if ( nbrCoords > 1 ) {
            // Fill a temporary array with coordinates
            gdPoint* points = new gdPoint[ nbrCoords ];
            points[0].x = prevX;
            points[0].y = prevY;

            for ( uint32 i = 1; i < nbrCoords; i++ ) {
               int32 lat = polygon->getLat(i, prevLat);
               int32 lon = polygon->getLon(i, prevLon);
               MC2Coordinate currCoord( lat, lon );
               MC2Point tmp = m_drawingProjection->getPoint( currCoord );
               
               points[i].x = tmp.getX(); 
               points[i].y = tmp.getY();
               
               prevLat = lat;
               prevLon = lon;
            }
            
            // Draw the polygon, depending on the polygon should be 
            // filled or not
            if ( settings->m_style == DrawSettings::FILLED ) {
               gdImageFilledPolygon(m_image, points, nbrCoords, 
                                    color );
            } else {
               gdImagePolygon( m_image, points, nbrCoords, 
                               color );
            }
            delete [] points;
         }
      } break;

      case DrawSettings::SYMBOL :
         // Draw a symbol at the coordinates in the feature
         mc2dbg2 << "GDImageDraw::drawgfxFeaturePolygon SYMBOL" << endl;
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

         
   } // end switch

   return (true);
}
namespace {

bool isBold( const char* font ) {
   // ugly hack, but since libgd dont have any way to get the
   // font weight .....*sigh*
   return strstr( font, "Bd.ttf" ) != 0;
}

}

char* GDImageDraw::drawString( gdImagePtr image, int col, const char* fontIn,
                               double fontSize, float64 angle, 
                               int x, int y, const char* textIn ) {
   char* text = const_cast<char*>( textIn );
   char* font = const_cast<char*>( fontIn );
   // draw outline if fontsize i larger than 11
   // or is bold
   if ( fontSize > 11 || isBold( font ) ) {
      int white = ::getColor( image, GDUtils::Color::WHITE );
      int diff = 2;
      gdImageStringFT( image, NULL, white, font,
                       fontSize, angle, x - diff, y - diff, text );
      gdImageStringFT( image, NULL, white, font,
                       fontSize, angle, x, y - diff, text );
      gdImageStringFT( image, NULL, white, font,
                       fontSize, angle, x + diff, y - diff, text );
      gdImageStringFT( image, NULL, white, font,
                       fontSize, angle, x + diff, y, text );
      gdImageStringFT( image, NULL, white, font,
                       fontSize, angle, x + diff, y + diff, text );
      gdImageStringFT( image, NULL, white, font,
                       fontSize, angle, x, y + diff, text );
      gdImageStringFT( image, NULL, white, font,
                       fontSize, angle, x - diff, y + diff, text );
      gdImageStringFT( image, NULL, white, font,
                       fontSize, angle, x - diff, y, text );

   }

   char* err = gdImageStringFT( image, NULL, col, font,
                                fontSize, angle, x, y, text );
  
   return err;
}

void 
GDImageDraw::drawGfxFeatureText( const char* text, 
                                 int32 lat, int32 lon,
                                 double fontSize,
                                 GDUtils::Color::CoolColor color,
                                 double angle,
                                 bool moveText,
                                 const char* fontName )
{
   char font[512];
   getFontPath( font, fontName );

   // Set color
   int col = getColor( color );

   // Get x and y
   MC2Coordinate coord( lat, lon );
   MC2Point tmp = m_drawingProjection->getPoint( coord );
   int x = tmp.getX();
   int y = tmp.getY();

   // make sure we have a valid angle, else gd library might crash.
   // Note: some compilers might not have isnan.
   if ( isnan( angle ) ) {
      mc2log << error << "[GDImageDraw] Angle is not a valid number! "
             << "Setting angle to 0." << endl;

      angle = 0;
   }
   // Move down fontHeight / 2
   
   //int32 charHeight = int32( fontSize )*64/72;
   //x += int32(rint((float64)charHeight / 2 * sin(angle)));
   //y += int32(rint((float64)charHeight / 2 * cos(angle)));

   if ( moveText ) {

      const char space = ' ';

      uint32 index = 0;
      int nbrSpaces = 0;

      const uint32 textLen = strlen( text );
      for (uint i = 0; i < textLen; i++) {
         if( text[i] - space == 0 ) {
            index = i;
            nbrSpaces++;
         }
      }
      
      if ( textLen > 12 && nbrSpaces > 0 ) {
         // + 2 so we can fit the extra \n\r
         ScopedArray<char> textNew( new char[ textLen + 2 ] );

         for (uint32 i = 0; i < textLen; i++) {
            if( i == index) {
               textNew[i] = '\r';
               textNew[i+1] = '\n';
            }
            else if(i < index) 
               textNew[i] = text[i];
            else
               textNew[i+1] = text[i];
         }
         textNew[ textLen + 1 ] = '\0';
         
         // Write the first line of text in the image
         ISABSync sync( m_fontMonitor );
         char* err = drawString( m_image, col, font, fontSize, angle, x, y,
                                   textNew.get() );
         if (err != NULL ) {
            mc2log << error
                   << "GDImageDraw::drawGfxFeatureText error drawing text "
                   << endl << "   " << err << " text " << text << endl;
         }  
      } else {
         // Write the text in the image
         ISABSync sync( m_fontMonitor );
         char* err = drawString( m_image, col, font, fontSize, 
                                   angle, x, y, text );
         if (err != NULL ) {
            mc2log << error
                   << "GDImageDraw::drawGfxFeatureText error drawing text "
                   << endl << "   " << err << " text " << text << endl;
         }
      }
   } else {
      // Write the text in the image
      ISABSync sync( m_fontMonitor );
      char* err = drawString( m_image, col, font, fontSize,
                                angle, x, y, text );

      if (err != NULL ) {
         mc2log << error
                << "GDImageDraw::drawGfxFeatureText error drawing text "
                << endl << "   " << err << " text " << text << endl;
      }
   }
}

void 
GDImageDraw::drawGfxFeatureText( const char* text, 
                                 const char* altText,
                                 int32 lat, int32 lon,
                                 double fontSize,
                                 GDUtils::Color::CoolColor color,
                                 double angle,
                                 bool moveText,
                                 const char* fontName ) {
   drawGfxFeatureText(text, lat, lon, fontSize, color, angle, moveText, fontName);
}

void 
GDImageDraw::drawGfxFeatureText( const char* text, 
                                 vector<GfxDataTypes::textPos>& textPosition,
                                 double fontSize,
                                 GDUtils::Color::CoolColor color,
                                 const char* fontName )
{  
   char tmpText[10]; // Room for 6 characters
   mc2TextIterator tit = mc2TextIterator( text ); 
   for ( vector<GfxDataTypes::textPos>::iterator it = textPosition.begin();
         it != textPosition.end();
         ++it ) {
      UTF8Util::unicodeToUtf8( *tit, tmpText );
      drawGfxFeatureText(tmpText, (*it).lat, (*it).lon,
                         fontSize,
                         color,
                         (*it).angle, 
                         false, // moveText
                         fontName);
      ++tit;
   }
   mc2dbg8 << "[GDID]: drawGfxFeatureText result image now" 
           << " has " << gdImageColorsTotal( m_image )
           << " colours "
           << endl;
   //   MC2_ASSERT( *tit == 0 );
}

void
GDImageDraw::drawGfxFeatureRoadSign( 
   const char* name, const char* signName,
   int32 lat, int32 lon, float32 border,
   double fontSize,
   GDUtils::Color::CoolColor signColor,
   const char* fontName )
{
   GDUtils::ImagePtr sign( getRoadSign( name, signName,
                                        border, fontSize,
                                        signColor, fontName ) );
   
   if ( sign.get() != NULL ) {
      gdImageSetBrush( m_image, sign.get() );
      MC2Coordinate coord( lat, lon );
      MC2Point p = m_drawingProjection->getPoint( coord );
      gdImageSetPixel( m_image, 
                       p.getX() + (gdImageSX( sign.get() ) - 1)/2,
                       p.getY(),
                       gdBrushed );
      
   } else {
      mc2log << warn << "GDImageDraw::drawGfxFeatureRoadSign no sign"
             << endl << signName << name << endl;
   }
   mc2dbg8 << "[GDID]: drawGfxFeatureRoadSign result image now" 
           << " has " << gdImageColorsTotal( m_image )
           << " colours "
           << endl;         
}


void 
GDImageDraw::initializeDrawImages() {
   // Check if the images are already initialized.
   if (m_imagesInitialized) {
      return;
   }

   m_imagesInitialized = true;
}


void
GDImageDraw::removeDrawImages() {
}


void 
GDImageDraw::drawArc( int cx, int cy,
                      int startAngle, int stopAngle, 
                      int innerRadius, int outerRadius,
                      GDUtils::Color::CoolColor color,
                      int lineWidth )
{
   mc2dbg4 << "GDImageDraw::drawArc( " << cx << "," << cy << ","
           << startAngle << "," << stopAngle << "," << innerRadius << ","
           << outerRadius << " )" << endl;

   int col = getColor( color );
   int startAng = ( startAngle - 90 + 360 ) % 360;
   int stopAng = ( stopAngle - 90 + 360 ) % 360;

   if ( startAng == stopAng ) {
      // Full circle
      startAng = 0; 
      stopAng = 360;
   }
   
   // Inner arc
   if ( innerRadius > 0 ) {   
      gdImageArc( m_image, cx, cy, innerRadius*2, innerRadius*2, 
                  startAng, stopAng, 
                  col );
   }

   // Lines between innner and outer arc, not if full circle
   if ( !(startAng == 0 && stopAng == 360 ) ) {
      // Start line
      float64 sinStart = 
         sin( startAngle * GfxConstants::degreeToRadianFactor );
      float64 cosStart = 
         -cos( startAngle * GfxConstants::degreeToRadianFactor );

      int sx = cx + int( rint( sinStart*innerRadius ) );
      int ex = cx + int( rint( sinStart*outerRadius ) );
      int sy = cy + int( rint( cosStart*innerRadius ) );
      int ey = cy + int( rint( cosStart*outerRadius ) );

      mc2dbg8 << "gdImageLine( " << sx << "," << sy << "," 
              << ex << "," << ey << " )" << endl
              << "Length " << ::sqrt( SQUARE(sx-ex) + SQUARE(sy-ey) ) << endl; 
      gdImageLine( m_image, sx, sy, ex, ey, col );

      // Stop line
      float64 sinStop = 
         sin( stopAngle * GfxConstants::degreeToRadianFactor );
      float64 cosStop = 
         -cos( stopAngle * GfxConstants::degreeToRadianFactor );
      sx = cx + int( rint( sinStop*innerRadius ) );
      ex = cx + int( rint( sinStop*outerRadius ) );
      sy = cy + int( rint( cosStop*innerRadius ) );
      ey = cy + int( rint( cosStop*outerRadius ) );

      mc2dbg8 << "gdImageLine( " << sx << "," << sy << "," << ex << ","
              << ey << " )" << endl << "Length "
              << ::sqrt( SQUARE(sx-ex) + SQUARE(sy-ey)) << endl;
      gdImageLine( m_image, sx, sy, ex, ey, col );
   }

   // OuterArc
   gdImageArc( m_image, cx, cy, (outerRadius+1)*2, (outerRadius+1)*2, 
               startAng, stopAng, 
               col );
}

void
GDImageDraw::drawRectangle( int x1, int y1, int x2, int y2,
                            GDUtils::Color::CoolColor color  )
{
   int col = getColor( color );
   gdImageRectangle( m_image, x1, y1, x2, y2, col );
}

void
GDImageDraw::drawBoundingBox( const MC2BoundingBox& bbox )
{
   MC2Coordinate upperCoord( bbox.getMaxLat(), bbox.getMinLon() );
   MC2Coordinate lowerCoord( bbox.getMinLat(), bbox.getMaxLon() );
   
   MC2Point upperPoint = m_drawingProjection->getPoint( upperCoord );
   MC2Point lowerPoint = m_drawingProjection->getPoint( lowerCoord );

   drawRectangle( upperPoint.getX(), upperPoint.getY(),
                 lowerPoint.getX(), lowerPoint.getY(),
                  GDUtils::Color::makeColor( GDUtils::Color::BLACK ) );
}

void 
GDImageDraw::drawTurnArrow( int x0, int y0, int x1, int y1, 
                            int x2, int y2, int x3, int y3,
                            int arrowX0, int arrowY0,
                            int arrowX1, int arrowY1,
                            int arrowX2, int arrowY2,
                            int32 arrowAngle, int32 arrowLength,
                            GDUtils::Color::CoolColor col,
                            int arrowWidth )
{
   mc2dbg4 << "GDImageDraw::drawTurnArrow( " << x0 << "," << y0 << ", "
           << x1 << "," << y1 << ", " << x2 << "," << y2 << ", " << x3 << ","
           << y3 << endl << arrowX0 << "," << arrowY0 << ", " << arrowX1
           << "," << arrowY1 << ", " << arrowX2 << "," << arrowY2 << " )"
           << endl;

   vector<int> resX;
   vector<int> resY;
   float64 angle = 0;
   int color = getColor( col );

   GfxUtility::makeSpline( x0, y0, x1, y1, 
                           x2, y2, x3, y3,
                           resX, resY, 0.05, &angle );

   // Draw spline line
   int x = 0;
   int y = 0;
   int oldX = MAX_INT32;
   int oldY = MAX_INT32;
   
   if ( resX.size() > 0 ) {
      oldX = resX[ 0 ];
      oldY = resY[ 0 ];
   }
   for ( uint32 i = 1 ; i < resX.size() ; i++ ) {
      x = resX[ i ];
      y = resY[ i ];
      roundedLine( oldX, oldY, x, y, color, arrowWidth );
      oldX = x;
      oldY = y;
   }

   float64 arrowAng = GfxConstants::degreeToRadianFactor * arrowAngle;
   x = resX[ resX.size() - 1 ];
   y = resY[ resX.size() - 1 ];

   mc2dbg8 << "angle " << (angle*GfxConstants::radianTodegreeFactor)
           << " angle + arrowAng " 
           << (angle + arrowAng)*GfxConstants::radianTodegreeFactor
           << " angle - arrowAng " 
           << (angle - arrowAng)*GfxConstants::radianTodegreeFactor << endl;

   // Move arrow so its tail is at the end of spline
   x += int32( rint( cos( angle ) * (arrowLength * cos( arrowAng ) ) ) );
   y -= int32( rint( sin( angle ) * (arrowLength * cos( arrowAng ) ) ) );
   arrowX1 = x;
   arrowY1 = y;

   arrowX0 = int32( rint( x - cos( angle - arrowAng ) * arrowLength ) );
   arrowY0 = int32( rint( y + sin( angle - arrowAng ) * arrowLength ) );
   arrowX2 = int32( rint( x - cos( angle + arrowAng ) * arrowLength ) );
   arrowY2 = int32( rint( y + sin( angle + arrowAng ) * arrowLength ) );

   mc2dbg8 << "Arrow points " << arrowX0 << "," << arrowY0 << ", "
           << arrowX1 << "," << arrowY1 << ", "
           << arrowX2 << "," << arrowY2 << endl;

   // Draw Arrow
   gdPoint points[ 3 ];
   
   points[ 0 ].x = arrowX0;
   points[ 0 ].y = arrowY0;
   points[ 1 ].x = arrowX1;
   points[ 1 ].y = arrowY1;
   points[ 2 ].x = arrowX2;
   points[ 2 ].y = arrowY2;
   
   gdImageFilledPolygon( m_image, points, 3, color );
   using namespace GDUtils;

}

void
GDImageDraw::drawRouteAsArrow(vector<POINT> route,
                              vector<POINT> arrowHead,
                              GDUtils::Color::CoolColor color,
                              uint32 routeWidth)
{
   // Check in parameters.
   if (arrowHead.size() != 3){
       mc2log << error << "MapDrawer::drawRouteAsArrow. "
              << "Wrong number of coordinates for arrow head. "
              << "Number of coordinates are " << arrowHead.size()
              << " There should be three."
              << endl; 
   }
   if(route.size() < 2){
       mc2log << warn << "MapDrawer::drawRouteAsArrow. "
              << "Too few coordinates for the route."
              << endl; 
   }

   // Draw the route part.
   for (int32 i = 0; i < static_cast<int32>( route.size() - 1 ); i++){
      roundedLine(route[i].x, route[i].y, 
                  route[i+1].x, route[i+1].y, 
                  color, routeWidth);
   }

   // Draw the arrow head.
   const uint32 nbrArrowHeadPoints = 3;
   gdPoint arrowHeadPoints[nbrArrowHeadPoints];
   for (uint32 i = 0; i < nbrArrowHeadPoints; i++ ){
      arrowHeadPoints[i].x = arrowHead[i].x;
      arrowHeadPoints[i].y = arrowHead[i].y;         
   }

   gdImageFilledPolygon(m_image,
                        arrowHeadPoints,
                        nbrArrowHeadPoints, 
                        getColor( color ) );
}


void 
GDImageDraw::drawScale( MC2BoundingBox* bbox, 
                        StringTable::languageCode lang ) 
{
   uint16 width = gdImageSX( m_image );
   uint16 height = gdImageSY( m_image );
   const float64 scalePartOfWidth = 0.40;
   int bottomMargin = 8;
   int sideMargin = 8;
   int barTextSpacing = 2;
   int barWidth = 5;
   int textMargin = 2;
   bool textBox = false;
   const char* fontName = "Vera.ttf";
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
   int white = getColor( GDUtils::Color::WHITE );
   int black = getColor( GDUtils::Color::BLACK );

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
   // Fill all with white
   gdImageFilledRectangle( m_image, leftPosX, posY - barWidth,
                           rightPosX, posY, white );
   // Black border
   gdImageRectangle( m_image, leftPosX, posY - barWidth,
                     rightPosX, posY, white );
   // Leftmost black part
   gdImageFilledRectangle( m_image, leftPosX, posY - barWidth,
                           leftPosX + int( rint( partWidth ) ), 
                           posY, black );
   // Second black part
   gdImageFilledRectangle( m_image, 
                           leftPosX + int( rint( 2*partWidth ) ), 
                           posY - barWidth,
                           leftPosX + int( rint( 3*partWidth ) ), 
                           posY, black );

   //** Draw distance string in own square text box
   char font[512];
   getFontPath( font, fontName );
   int textArea[ 8 ];
   
   ISABSync* sync = new ISABSync( m_fontMonitor );
   // Get size of distStr
   char* err = gdImageStringFT( NULL, textArea, black, font,
                                fontSize, 0.0, leftPosX, posY, distStr );
   if ( err == NULL ) { // Ok
      // Use textArea for textbox
      int textWidth = textArea[ 4 ] - textArea[ 6 ];
      int textHeight = textArea[ 1 ] - textArea[ 7 ];
      
      // Center textbox above scalebar
      int textXCenter = rightPosX - int( rint( 2*partWidth ) );
      int textYBottom = posY - barWidth - barTextSpacing - textMargin;
      // Make sure that text fits in image
      if ( textXCenter + textWidth/2 + textMargin + sideMargin >= width ) {
         textXCenter = width - textWidth/2 - textMargin - sideMargin;
      }

      if ( textBox ) {
         gdImageFilledRectangle( m_image, 
                                 (textXCenter - textWidth/2 - textMargin ),
                                 (textYBottom - textHeight - textMargin),
                                 (textXCenter + textWidth/2 + textMargin),
                                 (textYBottom + textMargin), 
                                 white );
      }
      
      int x = textXCenter - textWidth/2;
      int y = textYBottom;
      // Finally draw distStr
      gdImageStringFT( m_image, textArea, black, font,
                       fontSize, 0.0, x, y, distStr );
   } else {
      mc2log << error << "GDImageDraw::drawScale error drawing text "
             << endl << "   " << err << " font " << font << endl;
   }
   delete sync;
   
}


void 
GDImageDraw::drawPoint( int x, int y, GDUtils::Color::CoolColor color,
                        int radius ) 
{
   mc2dbg4 << "GDImageDraw::drawPoint( " << x << ", " << y << ", "
           << (int)color << ", " << radius << " )" << endl;
   if ( radius <= 1 ) {
      gdImageSetPixel( m_image, x, y, getColor( color ) );
   } else {
      int col = getColor( color );
      gdImageFilledArc( m_image, x, y, radius, radius, 0, 360, col, 
                        gdArc );
   }
}


void 
GDImageDraw::drawSquare(int x, int y, GDUtils::Color::CoolColor color,
                        int side ) 
{
   mc2dbg4 << "GDImageDraw::drawSquare( " << x << ", " << y << ", "
           << (int)color << ", " << side << " )" << endl;
   if ( side <= 1 ) {
      gdImageSetPixel( m_image, x, y, getColor( color ) );
   } else {
      int side_2 = side / 2;
      gdImageFilledRectangle(m_image, x-side_2, y-side_2,
                                      x+side_2, y+side_2, getColor(color));
   }
}


bool
GDImageDraw::getGlyphDimensions( int fontSize,
                                 const char* fontName,
                                 const char* text,
                                 vector<GfxDataTypes::dimensions>& dimensions,
                                 float64 xFactor,
                                 float64 yFactor )
{

   FT_Face face = loadFace( fontName ); 

   if (face == NULL) {
      return (false);
   }

   FT_Error error;
   
   error = FT_Set_Char_Size( face, 
                             0,
                             int(fontSize * 64), 
                             0,
                             0 ); // 0 - defaults to 72 dpi

    for (mc2TextIterator it = mc2TextIterator(text);
        *it != 0; ++it ) {
      // Seems like FT_Get_Char_Index wants unicode. Nice.
      FT_UInt glyph_index = FT_Get_Char_Index( face, *it ); 

      error = FT_Load_Glyph( face, 
                             glyph_index,
                             FT_LOAD_DEFAULT );
      if ( error ) {
         return (false);
      }

      GfxDataTypes::dimensions dim;
      
      // WARNING: We increase the width by one pixel because the text looks
      // too compressed otherwise (very strange).
      dim.width = 
         int32( ((1+((face)->glyph->advance.x >> 6)) * xFactor) + 0.5 );
      dim.height = 
         int32( (((face)->glyph->advance.y >> 6) * yFactor) + 0.5 );
      dim.width++;
      dimensions.push_back( dim );
   }
  
   return (true);
}

void 
GDImageDraw::getPixel( uint32 x, uint32 y, 
                       unsigned char& red, 
                       unsigned char& green, 
                       unsigned char& blue )
{
   int color = gdImageGetPixel( m_image, x, y );
   red = gdImageRed( m_image, color );
   green = gdImageGreen( m_image, color );
   blue = gdImageBlue( m_image, color );
}


void 
GDImageDraw::transferPixels( void* image ) {
   Image* imImage = static_cast< Image* >( image );

   PixelPacket* q = NULL;
   int quantumFactor = imImage->depth -8;
   for ( int y = 0 ; y < (int)m_height ; y++ ) {
      q = GetImagePixels( imImage, 0, y, m_width, 1 );
      if ( q == NULL ) {
         break;
      }
      for ( int x = 0 ; x < (int)m_width ; x++ ) {
         int color = gdImageGetPixel( m_image, x, y );
         q->red = gdImageRed( m_image, color ) << quantumFactor;
         q->green = gdImageGreen( m_image, color ) << quantumFactor;
         q->blue = gdImageBlue( m_image, color ) << quantumFactor;

         q++;
      }
      if ( ! SyncImagePixels( imImage ) ) {
         break;
      }
   }
}


int 
GDImageDraw::getColor( GDUtils::Color::CoolColor color ) {
   return ::getColor( m_image, color );
}

int 
GDImageDraw::getColor( GDUtils::Color::imageColor color ) {
   return ::getColor( m_image, color );
}


void 
GDImageDraw::roundedLine( int x1, int y1, int x2, int y2,
                          int color, int width )
{
   if ( width >= 2 ) {
      // Get brush to draw with
      gdImagePtr brush = getBrush( width, 
                                   gdImageRed( m_image, color ),
                                   gdImageGreen( m_image, color ),
                                   gdImageBlue( m_image, color ) );
      
      // Draw using brush
      gdImageSetBrush( m_image, brush );
      gdImageLine( m_image, 
                   x1, y1, 
                   x2, y2, gdBrushed );
   } else {
      gdImageLine( m_image, 
                   x1, y1, 
                   x2, y2, color );
   }
}


void
GDImageDraw::drawSymbol( DrawSettings::symbol_t t, int x, int y,
                         DrawSettings* settings,
                         const GfxFeature* feature,
                         ImageTable::ImageSet imageSet )
{
   using namespace GDUtils;
   // Vector containing images of the symbols to draw.
   vector<gdImagePtr> symbolImages;
   switch (t) {
      case DrawSettings::ROUTE_ORIGIN_SYMBOL :
         mc2dbg4 << "GDImageDraw::drawSymbol ROUTE_ORIGIN_SYMBOL" << endl;
         // Drive, Walk or bike and starting left or right and U-turn
         if ( settings->m_transportationType == ItemTypes::drive ) {
            if ( settings->m_startingAngle <= 127 ) {
               if ( settings->m_initialUTurn ) {
                  symbolImages.push_back( loadImage( "carRightU.png" ) );
               } else {
                  symbolImages.push_back( loadImage( "carRight.png" ) );
               }
            } else { 
               if ( settings->m_initialUTurn ) {
                  symbolImages.push_back( loadImage( "carLeftU.png" ) );
               } else {
                  symbolImages.push_back( loadImage( "carLeft.png" ) );
               } 
            }
         } else if ( settings->m_transportationType == ItemTypes::walk ) {
            symbolImages.push_back(loadImage( "man.png" ));
         } else if ( settings->m_transportationType == ItemTypes::bike ) {
            if ( settings->m_startingAngle <= 127 ) {
               symbolImages.push_back( loadImage( "bikeRight.png" ) );
            } else { 
               symbolImages.push_back( loadImage( "bikeLeft.png" ) );
            }
         } else { // Default
            gdImagePtr routeOriginImg = loadImage( "route_orig.png" );
            symbolImages.push_back( routeOriginImg );
         }
         break;
      case DrawSettings::ROUTE_DESTINATION_SYMBOL : {
         mc2dbg4 << "GDImageDraw::drawSymbol ROUTE_DESTINATION_SYMBOL" 
                 << endl;
         gdImagePtr routeDestinationImg = loadImage( "route_dest.png" );
         symbolImages.push_back( routeDestinationImg );
         // Compensate so that the flag stands on the route.
         x += 5;
         y -= gdImageSY( routeDestinationImg ) / 2 - 1;
      }
         break;
      case DrawSettings::ROUTE_PARK_SYMBOL :
         mc2dbg4 << "GDImageDraw::drawSymbol ROUTE_PARK_SYMBOL" << endl;
         symbolImages.push_back( 
               getSymbolImage( settings->m_featureType, imageSet ) );
         break;   
      case DrawSettings::SQUARE_3D_SYMBOL :
         mc2dbg4 << "GDImageDraw::drawSymbol SQUARE_3D_SYMBOL" << endl;
         drawSquare(x+3, y+3, Color::makeColor( Color::BROWN ), 10);
         drawSquare(x, y, Color::makeColor( Color::INDIANRED ), 10);
         return;
      case DrawSettings::SMALL_CITY_SYMBOL :
         mc2dbg4 << "GDImageDraw::drawSymbol SQUARE_3D_SYMBOL" << endl;
         drawPoint(x, y, Color::makeColor( Color::LIGHTSLATEGREY ), 10);
         return;
      case DrawSettings::POI_SYMBOL : {
         mc2dbg4 << "GDImageDraw::drawSymbol POI_SYMBOL" << endl;
         mc2dbg2 << " poi type == " << int(settings->m_poiType) 
                 << " = " << StringTable::getString( 
                    ItemTypes::getPOIStringCode( settings->m_poiType ),
                    StringTable::ENGLISH ) << endl;
         gdImagePtr symbolImage;
         const GfxPOIFeature* poi = static_cast<const GfxPOIFeature*>(feature);
         symbolImage = getPOISymbolImage( settings->m_poiType,
                                          imageSet,
                                          poi->getExtraInfo() );
         symbolImages.push_back( symbolImage );
         break;
      }
      case DrawSettings::PIN : {
         mc2dbg4 << "GDImageDraw::drawSymbol PIN" << endl;
         gdImagePtr symbolImage = loadImage( "mappin_centered.png" );
         symbolImages.push_back( symbolImage );
         break;
      }
      case DrawSettings::ROADWORK : {
         mc2dbg4 << "GDImageDraw::drawSymbol ROADWORK" << endl;
         gdImagePtr symbolImage = loadImage( "tat_roadwork.png" );
         symbolImages.push_back( symbolImage );
         break;
      }
      case DrawSettings::SPEED_CAMERA : {
         mc2dbg4 << "GDImageDraw::drawSymbol SPEED_CAMERA" << endl;
         gdImagePtr symbolImage = loadImage( "tat_speedcamera.png" );
         symbolImages.push_back( symbolImage );
         break;
      }
      case DrawSettings::USER_DEFINED_SPEED_CAMERA : {
         mc2dbg4 << "GDImageDraw::drawSymbol USER_DEFINED_SPEED_CAMERA"
                 << endl;
         gdImagePtr symbolImage = loadImage( 
            "user_defined_speedcamera.png" );
         symbolImages.push_back( symbolImage );
         break;
      }
      case DrawSettings::DANGER : {
         mc2dbg4 << "GDImageDraw::drawSymbol DANGER" << endl;
         gdImagePtr symbolImage = loadImage( "tat_trafficinformation.png" );
         symbolImages.push_back( symbolImage );
         break;
      }
      case DrawSettings::USER_DEFINED :   
         mc2dbg4 << "GDImageDraw::drawSymbol USER_DEFINED " 
              << settings->m_symbolImage << endl;
         MC2String imgName( settings->m_symbolImage );
         if ( imgName.find( "." ) == MC2String::npos ) {
            imgName.append( ".png" );
         }
         symbolImages.push_back( loadImage( imgName.c_str() ) );
         break;
   } // end of switch

   if ( symbolImages.empty() ) {
      // No image, draw default "symbol"
      mc2log << error << "GDImageDraw::drawSymbol Implement default symbol!"
             << endl;
   }

   // Draw the symbols and delete the generated images
   vector<gdImagePtr>::iterator it = symbolImages.begin();
   vector<gdImagePtr>::iterator itEnd = symbolImages.end();
   for ( ; it != itEnd; ++it ) {
      if ( *it != NULL ) {
         // Create the bruch and draw image
         gdImageSetBrush( m_image, *it );
         gdImageSetPixel( m_image, x, y, gdBrushed );
         mc2dbg8 << "[GDID]: Result image now" 
                 << " has " << gdImageColorsTotal( m_image )
                 << " colours "
                 << endl;      
      }
   }

}


gdImagePtr 
GDImageDraw::getSymbolImage( GfxFeature::gfxFeatureType type,
                             ImageTable::ImageSet imageSet ) const {
   return loadImage( ImageDraw::getSymbolFilename( type, imageSet ).c_str() );
}

gdImagePtr 
GDImageDraw::getPOISymbolImage( ItemTypes::pointOfInterest_t type,
                                ImageTable::ImageSet imageSet,
                                byte extraPOIInfo ) const {
   return loadImage( ImageDraw::getPOISymbolFilename( type, 
                                                      imageSet, 
                                                      extraPOIInfo ).c_str() );
}

gdImagePtr
GDImageDraw::getBrush( int width, int red, int green, int blue ) {
   ISABSync sync( m_brushMon );

   m_findBrush.m_width = width;
   m_findBrush.m_red = red;
   m_findBrush.m_green = green;
   m_findBrush.m_blue = blue;

   BrushNotice* res = static_cast<BrushNotice*>( 
      m_brushes.equal( &m_findBrush ) );

   if ( res == NULL ) {
      // Make new
      int extraSize = 0;
      bool circle = Math::odd( width ) || width > 6;
      if ( circle ) {
         extraSize = 2;  
      }
      gdImagePtr brush = gdImageCreate( width+extraSize, width+extraSize );
      // gdImageColorAllocate can't fail since this is the one color!
      int bcolor = gdImageColorAllocate( brush, red, green, blue );

      // Set transparent color
      int transCol = gdImageColorAllocate( brush, ~red, ~green, ~blue );

      gdImageColorTransparent( brush, transCol );

      // Fill with transparent
      gdImageFilledRectangle( brush, 0, 0, 
                              width-1+extraSize, width-1+extraSize, 
                              transCol );
      if ( circle ) {
         // Circle, even numbers are like nearest larger odd number
         int mid = width/2 + 1;
         gdImageFilledArc( brush, mid, mid, width, width, 
                           0, 360, bcolor, gdEdged );
      } else {
         // Square with no courners
         gdImageFilledRectangle( brush, 0, 0, width-1, width-1, bcolor );
         int removeSize = width/2 -1;

         if ( removeSize > 1 ) {
            gdPoint points[3];
            // Top left
            points[0].x = 0;
            points[0].y = 0;
            points[1].x = 0;
            points[1].y = removeSize-1;
            points[2].x = removeSize-1;
            points[2].y = 0;
            gdImageFilledPolygon( brush, points, 3, transCol );

            // Top right
            points[0].x = width-1;
            points[0].y = 0;
            points[1].x = width-1;
            points[1].y = removeSize-1;
            points[2].x = width-1-(removeSize-1);
            points[2].y = 0;
            gdImageFilledPolygon( brush, points, 3, transCol );

            // Bottom left
            points[0].x = 0;
            points[0].y = width-1;
            points[1].x = 0;
            points[1].y = width-1-(removeSize-1);
            points[2].x = removeSize-1;
            points[2].y = width-1;
            gdImageFilledPolygon( brush, points, 3, transCol );

            // Bottom right
            points[0].x = width-1;
            points[0].y = width-1;
            points[1].x = width-1;
            points[1].y = width-1-(removeSize-1);
            points[2].x = width-1-(removeSize-1);
            points[2].y = width-1;
            gdImageFilledPolygon( brush, points, 3, transCol );
         } else if ( removeSize == 1 ) {
            // Just remove the corners
            gdImageSetPixel( brush, 0, 0, transCol);
            gdImageSetPixel( brush, width-1, 0, transCol);
            gdImageSetPixel( brush, 0, width-1, transCol);
            gdImageSetPixel( brush, width-1, width-1, transCol);
         }
      }
      res = new BrushNotice( brush, width, red, green, blue );
      m_brushes.add( res );
   }

   return res->getBrush();
}


gdImagePtr 
GDImageDraw::makeRoadSign( gdImagePtr sign, const char* signText,
                           int x, int y, int width, int height, 
                           GDUtils::Color::CoolColor color,
                           float32 fontSize,
                           const char* fontName )
{
   gdImagePtr resIm = sign;
   using namespace GDUtils;
   int fontArea[ 8 ];
   memset( fontArea, 0, 8 * sizeof( int ) );
   GSystem::Color gsColor( static_cast< uint32 >( color ) );
   int fcolor = gdImageColorAllocate( 
      resIm,
      gsColor.getRed(),
      gsColor.getGreen(),
      gsColor.getBlue() );

   char font[512];
   double angle = 0.0;

   getFontPath( font, fontName );
   
   ISABSync sync( m_fontMonitor );
   
   gdImageStringFT( NULL, fontArea, fcolor, font,
                    fontSize, angle, x, y, 
                    const_cast<char*>( signText ) );
   
   // Draw it
   // Center extra space
   int widthDiff = ( width - (fontArea[4] - fontArea[6]) );
   int heightDiff = ( height - (fontArea[1] - fontArea[7]) );
   int xCenter = x;
   int yCenter = y;

   if ( widthDiff > 0 ) {
      int diff = (widthDiff)/2;
      xCenter += diff;
   }      
   if ( heightDiff > 0 ) {
      int diff = (heightDiff)/2;
      yCenter -= diff;
   }
   gdImageStringFT( resIm, NULL, fcolor, font,
                    fontSize, angle, 
                    xCenter, (yCenter + height), 
                    const_cast<char*>( signText ) );
   
   return resIm;
}


gdImagePtr 
GDImageDraw::getRoadSign( const char* name, 
                          const char* signName,
                          float32 border,
                          double fontSize ,
                          GDUtils::Color::CoolColor signColor,
                          const char* fontName )
{
   ISABSync sync( m_imagesMon );
   // load image without cache
   gdImagePtr sign = loadImage( signName, false );
         
   int borderWidth = int( rint( gdImageSX( sign ) * border ) );
   sign = makeRoadSign( 
      sign, name, 
      borderWidth, borderWidth, 
      gdImageSX( sign ) - 2*borderWidth, 
      gdImageSY( sign ) - 2*borderWidth,
      signColor, fontSize, fontName );
   
   return sign;
}


//**********************************************************************
// BrushNotice
//**********************************************************************


GDImageDraw::BrushNotice::BrushNotice( gdImagePtr brush, int width,
                                       int red, int green, int blue )
      : BinarySearchTreeNode(),
        m_brush( brush ),
        m_width( width ),
        m_red( red ),
        m_green( green ),
        m_blue( blue )
{
}


GDImageDraw::BrushNotice::BrushNotice()
      : BinarySearchTreeNode(),
        m_brush( NULL ),
        m_width( 0 ),
        m_red( 0 ),
        m_green( 0 ),
        m_blue( 0 )
{
}


GDImageDraw::BrushNotice::~BrushNotice() {
   if ( m_brush != NULL ) {
      gdImageDestroy( m_brush );  
   }
}


bool
GDImageDraw::BrushNotice:: operator >  (
   const BinarySearchTreeNode &node) const 
{
   const BrushNotice& other = static_cast< const BrushNotice& > ( node );
   if ( m_width != other.m_width ) {
      return m_width > other.m_width;
   } else if ( m_red != other.m_red ) {
      return m_red > other.m_red;
   } else if ( m_green != other.m_green ) {
      return m_green > other.m_green;
   } else {
      return m_blue > other.m_blue ;
   }
}


bool
GDImageDraw::BrushNotice:: operator <  (
   const BinarySearchTreeNode &node) const 
{
   const BrushNotice& other = static_cast< const BrushNotice& > ( node );
   if ( m_width != other.m_width ) {
      return m_width < other.m_width;
   } else if ( m_red != other.m_red ) {
      return m_red < other.m_red;
   } else if ( m_green != other.m_green ) {
      return m_green < other.m_green;
   } else {
      return m_blue < other.m_blue ;
   }
}


bool
GDImageDraw::BrushNotice:: operator ==  (
   const BinarySearchTreeNode &node) const 
{
   const BrushNotice& other = static_cast< const BrushNotice& > ( node );
   if ( m_width != other.m_width ) {
      return m_width == other.m_width;
   } else if ( m_red != other.m_red ) {
      return m_red == other.m_red;
   } else if ( m_green != other.m_green ) {
      return m_green == other.m_green;
   } else {
      return m_blue == other.m_blue ;
   }
}


gdImagePtr
GDImageDraw::BrushNotice::getBrush() const {
   return m_brush;
}


//**********************************************************************
// ImageNotice
//**********************************************************************


GDImageDraw::ImageNotice::ImageNotice( gdImagePtr image, 
                                       const char* name, 
                                       const char* extraName )
      : BinarySearchTreeNode(),
        m_image( image ),
        m_name( NULL )
{
   setName( name, extraName );
}


GDImageDraw::ImageNotice::ImageNotice()
      : BinarySearchTreeNode(),
        m_image( NULL ),
        m_name( NULL )
{
}


GDImageDraw::ImageNotice::~ImageNotice() {
   if ( m_image != NULL ) {
      gdImageDestroy( m_image );
   }
   delete [] m_name;
}


bool
GDImageDraw::ImageNotice:: operator >  (
   const BinarySearchTreeNode &node) const 
{
   const ImageNotice& other = static_cast< const ImageNotice& > ( 
      node );
   return strcmp( m_name, other.m_name ) > 0;
}


bool
GDImageDraw::ImageNotice:: operator <  (
   const BinarySearchTreeNode &node) const 
{
   const ImageNotice& other = static_cast< const ImageNotice& > ( 
      node );
   return strcmp( m_name, other.m_name ) < 0;
}


bool
GDImageDraw::ImageNotice:: operator ==  (
   const BinarySearchTreeNode &node) const 
{
   const ImageNotice& other = static_cast< const ImageNotice& > ( 
      node );
   return strcmp( m_name, other.m_name ) == 0;
}


gdImagePtr
GDImageDraw::ImageNotice::getImage() const {
   return m_image;
}


void 
GDImageDraw::ImageNotice::setName( const char* name, 
                                   const char* extraName ) 
{
   delete [] m_name;
   m_name = new char[ strlen( name ) + strlen( extraName ) + 1 ];
   strcpy( m_name, name );
   strcat( m_name, extraName );   
}

// Some utility functions
namespace {
inline bool colorUsed( gdImagePtr img,
                       int color )
{
   int sizex = gdImageSX( img );
   int sizey = gdImageSY( img );
   for ( int x = 0; x < sizex; ++x ) {
         for ( int y = 0; y < sizey; ++y ) {
         int pixColor = gdImageGetPixel(img, x, y);
         if ( pixColor == color ) {
            return true;
         }
      }
   }
   // No match
   return false;
}

// Function to transfer all the pixels from one image to a new one
// and by this avoiding the trouble with allocated unused colors.
// If colors are present in the image they seem to be allocated in the
// big image even if they are never used.
inline gdImagePtr
copyImageIndexed( gdImagePtr orig, int sizex = -1, int sizey = -1 )
{
   if( sizex < 0 ) {
      sizex = gdImageSX( orig );
   }
   if( sizey < 0 ) {
      sizey = gdImageSY( orig );
   }
   
   gdImagePtr dest = gdImageCreate( sizex, sizey );

   // Transfer the pixels one by one.
   for ( int x = 0; x < sizex; ++x ) {
      for ( int y = 0; y < sizey; ++y ) {
         int colorInOrig = gdImageGetPixel( orig, x, y);
         int colorInDest =
            gdImageColorResolveAlpha( dest,
                                      gdImageRed( orig, colorInOrig ),
                                      gdImageGreen( orig, colorInOrig ),
                                      gdImageBlue( orig, colorInOrig ),
                                      gdImageAlpha( orig, colorInOrig ) );
         gdImageSetPixel( dest, x, y, colorInDest );
      }
   }
   
   // Destroy original
   gdImageDestroy( orig );
   return dest;
}
}  // namespace

gdImagePtr 
GDImageDraw::loadImage( const char* fileName, 
                        bool useCache,bool useImagePath ) 
{
   gdImagePtr res = NULL;
   if ( fileName != NULL && strlen( fileName ) != 0 ) {
      MC2String filePath;
      if ( useImagePath ) {
         const char* imagesPath = Properties::getProperty( "IMAGES_PATH" );
         if ( imagesPath == NULL ) {
            imagesPath = "./";
         }
         filePath = MC2String( imagesPath ) + "/" + fileName;
      } else {
         filePath = fileName;
      }

      ImageNotice* imNotice = NULL;

      if ( useCache ) {
         ISABSync sync( m_imagesMon );
               
         m_findImage.setName( filePath.c_str(), "" );

         imNotice = static_cast<ImageNotice*>( 
            m_images.equal( &m_findImage ) );
         if ( imNotice != NULL ) {
            res = imNotice->getImage();
         }
      }

      if ( res == NULL ) {
         FileUtils::FilePtr file( fopen( filePath.c_str(), "rb" ) );
         if ( file.get() != NULL ) {
            // Read
            res = gdImageCreateFromPng( file.get() );
            if ( res == NULL ) {
               mc2log << warn << "Failed to load png file: " 
                      << filePath << endl;
               return NULL;
            }
            // Remove unused colors from image created with
            // Adobe something.
            res = copyImageIndexed( res );

            // Print color information
#if 0
            mc2dbg << "[GDID]: Image " << MC2CITE( fileName )
                   << " has " << gdImageColorsTotal( res )
                   << " colours "
                   << endl;
            for ( int i = 0 ; i < gdImageColorsTotal( res ) ; i++ ) {
               mc2dbg << "[GDID]: Color[" << i << "] = "
                      << MC2HEX( uint32 ( gdImageRed( res, i ) << 16 |
                                          gdImageGreen( res, i ) << 8 |
                                          gdImageBlue( res, i ) ) )
                      << endl;
               bool used = colorUsed( res, i );
               mc2dbg << "[GDID]: Color used = "
                      << used << endl;
            }
#endif       
            
            mc2dbg << "[GDID]: Image " << MC2CITE( fileName )
                   << " now has " << gdImageColorsTotal( res )
                   << " colours "
                   << endl;

            if ( res && gdImageGetTransparent( res ) == -1 ) {
               // No transparent color
               int col = -1;
               for ( int i = 0 ; i < gdImageColorsTotal( res ) ; i++ ) {
                  if ( gdImageAlpha( res, i ) == gdAlphaTransparent ) {
                     col = i;
                     break;
                  }
               }
               if ( col != -1 ) {
                  gdImageColorTransparent( res, col );
               }
            }
            
         } else {
            mc2log << error << here << " failed to open image file " 
                   << filePath << endl;
         }
      }
      
      if ( useCache && imNotice == NULL && res != NULL ) {
         // New image add it to cache
         ISABSync sync( m_imagesMon );
         
         imNotice = new ImageNotice( res, filePath.c_str() );
         m_images.add( imNotice );
      }

   }
   
   return res;
}


void 
GDImageDraw::getFontPath( char* font, const char* fontName ) {
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
         mc2log << error << "GDImageDraw::getFontPath getcwd failed." << endl;
         perror( "   getcwd error: " ); 
      }
   }

   sprintf( font, "%s%s/%s", absolutePath, fontPath, fontName );   
}


//**********************************************************************
// FaceNotice
//**********************************************************************

GDImageDraw::FaceNotice::FaceNotice( FT_Face face, 
                                     const char* name )
      : BinarySearchTreeNode(),
        m_face( face ),
        m_name( NULL )
{
   setName( name );
}


GDImageDraw::FaceNotice::FaceNotice()
      : BinarySearchTreeNode(),
        m_face( NULL ),
        m_name( NULL )
{
}


GDImageDraw::FaceNotice::~FaceNotice() 
{
   if (m_face != NULL) {
      FT_Done_Face( m_face );
   }
   delete [] m_name;
}


bool
GDImageDraw::FaceNotice:: operator >  (
   const BinarySearchTreeNode &node) const 
{
   const FaceNotice& other = static_cast< const FaceNotice& > ( 
      node );
   return strcmp( m_name, other.m_name ) > 0;
}


bool
GDImageDraw::FaceNotice:: operator <  (
   const BinarySearchTreeNode &node) const 
{
   const FaceNotice& other = static_cast< const FaceNotice& > ( 
      node );
   return strcmp( m_name, other.m_name ) < 0;
}


bool
GDImageDraw::FaceNotice:: operator ==  (
   const BinarySearchTreeNode &node) const 
{
   const FaceNotice& other = static_cast< const FaceNotice& > ( 
      node );
   return strcmp( m_name, other.m_name ) == 0;
}


FT_Face
GDImageDraw::FaceNotice::getFace() const {
   return m_face;
}


void 
GDImageDraw::FaceNotice::setName( const char* name ) 
{
   delete [] m_name;
   m_name = new char[ strlen( name ) + 1 ];
   strcpy( m_name, name );
}


FT_Face 
GDImageDraw::loadFace( const char* fontName, 
                       bool useCache ) 
{
   FT_Face res = NULL;
   
   if ( fontName != NULL ) {
      // Get the font including path
      char font[256];
      getFontPath(font, fontName);

      FaceNotice* fNotice = NULL;

      if ( useCache ) {
         // Try finding it in the cache.
         ISABSync sync( m_facesMon );
               
         m_findFace.setName( font );

         fNotice = static_cast<FaceNotice*>( 
            m_faces.equal( &m_findFace ) );
         if ( fNotice != NULL ) {
            res = fNotice->getFace();
         }
      }

      if ( res == NULL ) {
         // Wasn't found in cache
         if ( m_ftLibrary == NULL ) {
            return (NULL);
         }

         FT_Error error;

         error = FT_New_Face( m_ftLibrary,
                              font,
                              0,
                              &res );
         if ( error == FT_Err_Unknown_File_Format ) {
            //... the font file could be opened and read, but it appears
            //... that its font format is unsupported
            mc2log << error << here << " Font format unsupported for " 
                   << font << endl;
            return (NULL);
         } else if ( error ) {
            //... another error code means that the font file could not
            //... be opened or read, or simply that it is broken...
            mc2log << error << here << " Could not open font " << font
                   << endl;
            return (NULL);
         }

      } 
      
      if ( useCache && fNotice == NULL && res != NULL ) {
         // New face add it to cache
         ISABSync sync( m_facesMon );
         
         fNotice = new FaceNotice( res, font );
         m_faces.add( fNotice );
      }
   }
   
   return res;

}

void
GDImageDraw::cutImage( uint32 sizeX, uint32 sizeY ) 
{
   m_image = copyImageIndexed( m_image, sizeX, sizeY );
   // update size
   m_width = gdImageSX( m_image );
   m_height = gdImageSY( m_image );
}
