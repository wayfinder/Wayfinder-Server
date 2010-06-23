/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CAIRO_IMAGEDRAW_H
#define CAIRO_IMAGEDRAW_H

#include "ImageDraw.h"
#include "DrawSettings.h"
#include "MC2BoundingBox.h"

#include <memory>

#ifdef HAVE_CAIRO

namespace GSystem {
namespace Cairo {
class Surface;
class GContext;
class FontCache;
class Font;
}
}
class MC2Point;
class GfxData;

/**
 * Image drawer for Cairo: a vector graphic library
 */
class CairoImageDraw: public ImageDraw {
public:
   CairoImageDraw( uint32 width, uint32 height,
                   GDUtils::Color::CoolColor color = 
                   static_cast< GDUtils::Color::CoolColor >
                   ( 0x33AFE1 ) );
   ~CairoImageDraw();

   /// @see ImageDraw

   void setDrawingProjection( const DrawingProjection* proj );
   /// @see ImageDraw
   byte* getImageAsBuffer( uint32& size, 
                           ImageDrawConfig::imageFormat format
                           = ImageDrawConfig::PNG );
   /// @see ImageDraw
   void drawArc( int cx, int cy,
                 int startAngle, int stopAngle, 
                 int innerRadius, int outerRadius,
                 GDUtils::Color::CoolColor color,
                 int lineWidth = 1 );
   /// @see ImageDraw
   void drawRectangle( int x1, int y1, int x2, int y2,
                       GDUtils::Color::CoolColor color );
   /// @see ImageDraw
   void drawBoundingBox( const MC2BoundingBox& bbox );

   /// @see ImageDraw
   void writeText( const char* text, int x, int y,
                   double fontSize = 10.0,
                   GDUtils::Color::CoolColor color =
                   static_cast< GDUtils::Color::CoolColor >( 0x000000 ),
                   double angle = 0.0, 
                   const char* fontName = "Vera.ttf" );

   /// @see ImageDraw
   void drawTurnArrow( int x0, int y0, int x1, int y1, 
                       int x2, int y2, int x3, int y3,
                       int arrowX0, int arrowY0,
                       int arrowX1, int arrowY1,
                       int arrowX2, int arrowY2,
                       int32 arrowAngle, int32 arrowLength,
                       GDUtils::Color::CoolColor col,
                       int arrowWidth = 1 );
   /// @see ImageDraw
   void drawScale( MC2BoundingBox* bbox,
                   StringTable::languageCode lang = 
                   StringTable::ENGLISH );

   /**
    * Draw polygons from a GfxData with lines.
    * @param data Should contain all polygons to be drawn.
    * @param lineWidth Pixel width of the line.
    * @param color The color of the line.
    */
   void drawGfxData( const GfxData& data,
                     uint32 lineWidth,
                     GDUtils::Color::imageColor color );

   /// @see ImageDraw
   bool drawGfxFeaturePolygon( const GfxFeature* feature,
                               GfxPolygon* polygon,
                               DrawSettings* settings,
                               ImageTable::ImageSet imageSet );
   /// @see ImageDraw
   void drawGfxFeatureRoadSign( const char* name, const char* signName,
                                int32 lat, int32 lon, 
                                float32 border = 0.10,
                                double fontSize = 10.0,
                                GDUtils::Color::
                                CoolColor signColor =
                                static_cast< GDUtils::Color::CoolColor >( 0xFFFFFF ),
                                const char* fontName = "Vera.ttf" );
   /// @see ImageDraw
   void drawRouteAsArrow( vector<POINT> route,
                          vector<POINT> arrowHead,
                          GDUtils::Color::CoolColor color,
                          uint32 routeWidth );
   /// @see ImageDraw
   void drawGfxFeatureText( const char* text, 
                            int32 lat, int32 lon,
                            double fontSize = 10.0,
                            GDUtils::Color::
                            CoolColor color =
                            static_cast< GDUtils::Color::CoolColor >( 0x000000 ),
                            double angle = 0.0,
                            bool moveText = false,
                            const char* fontName = "Vera.ttf");

   /// @see ImageDraw
   void drawGfxFeatureText( const char* text, 
                            const char* altLangText,
                            int32 lat, int32 lon,
                            double fontSize = 10.0,
                            GDUtils::Color::CoolColor color = 
                            static_cast< GDUtils::Color::CoolColor>
                            ( 0x000000 ),
                            double angle = 0.0,
                            bool moveText = false,
                            const char* fontName = "Vera.ttf");

   /// @see ImageDraw
   void drawGfxFeatureText( const char* text, 
                            vector<GfxDataTypes::textPos>& textPosition,
                            double fontSize = 10.0,
                            GDUtils::Color::CoolColor color =
                            static_cast< GDUtils::Color::CoolColor >( 0x000000 ),
                            const char* fontName = "Vera.ttf" );

   /// @see ImageDraw
   void getPixel( uint32 x, uint32 y, 
                  unsigned char& red, 
                  unsigned char& green, 
                  unsigned char& blue );

   /// @see ImageDraw
   bool getGlyphDimensions( int fontSize,
                            const char* fontName,
                            const char* text,
                            vector<GfxDataTypes::dimensions>& dimensions,
                            float64 xFactor = 1.0,
                            float64 yFactor = 1.0);
   /// @see ImageDraw
   GfxDataTypes::dimensions 
   getStringDimension( int fontSize,
                       const char* fontName,
                       const char* text,
                       float64 xFactor, float64 yFactor ) const;

   void cutImage( uint32 width, uint32 height );



private:
   inline void drawText( GSystem::Cairo::Font& font,
                         float64 fontSize, double angle,
                         const MC2Point& startPos,
                         const char* text );

   void drawSymbol( DrawSettings::symbol_t sym,
                    int x, int y,
                    DrawSettings* settings,
                    const GfxFeature* feature,
                    ImageTable::ImageSet imageSet );

   void drawTexts(const MC2String& string1, const MC2String& string2,
                  GSystem::Cairo::Font* font1, float64 fontSize1,
                  GSystem::Cairo::Font* font2, float64 fontSize2, 
                  const MC2Point& startPos, double angle );

   auto_ptr<GSystem::Cairo::Surface> m_surface; ///< surface to draw on
   auto_ptr<GSystem::Cairo::GContext> m_gc; ///< graphics context
   auto_ptr<GSystem::Cairo::FontCache> m_fontCache; ///< font cache
   const DrawingProjection* m_drawingProjection; ///< drawing projection
   MC2String m_fontPath; ///< paths to all fonts, ends with '/'.
   /// size in pixels for the outline towards one side.
   static const uint32 OUTLINE_SIZE;
};
#endif // HAVE_CAIRO

#endif // CAIRO_IMAGEDRAW_H
