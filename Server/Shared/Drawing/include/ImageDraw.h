/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef IMAGEDRAW_H
#define IMAGEDRAW_H


#include "config.h"
#include "GDColor.h"
#include "ImageDrawConfig.h"
#include <vector>
#include "GfxUtility.h"
#include "MC2BoundingBox.h"
#include "GfxDataTypes.h"
#include "ItemTypes.h"
#include "GfxFeature.h"
#include "DrawSettings.h"
#include "ImageTable.h"

class DrawingProjection;
class GfxPolygon;
class GfxPOIFeature;

/**
 * Super class for drawing images and saving it into memory in
 * known image formats.
 *
 * This class uses ImageMagick to convert images to desired type.
 */
class ImageDraw {
   public:
      /**
       * Constructs a new Image with the desired dimensions.
       *
       * @param xSize The number of pixels in the x-dimension.
       * @param ySize The number of pixels in the y-dimension.
       */
      ImageDraw( uint32 xSize, uint32 ySize );


      /**
       * Destructor, deallocates image.
       */
      virtual ~ImageDraw();

      /**
       * Returns the image as a buffer of the right format.
       *
       * @param size Set to the length of the retuned buffer.
       * @param format The type of encoding of the buffer.
       */
      virtual byte* getImageAsBuffer( uint32& size, 
                                      ImageDrawConfig::imageFormat format
                                      = ImageDrawConfig::PNG ) = 0;


   virtual void setDrawingProjection( const DrawingProjection* 
                                      drawingProjection ) = 0;
      /**
       * Draws an arc, usually a positioned area.
       *
       * @param cx Center of arc x-value, may be negative of larger than
       *           images x-size.
       * @param cx Center of arc y-value, may be negative of larger than
       *           images y-size.
       * @param startAngle The start angle in degrees.
       * @param stopAngle The stop angle in degrees.
       * @param outerRaduis The outer radius.
       * @param innerRaduis The inner radius, may be 0.
       * @param color The color to draw with.
       * @param lineWidth The linewidth to draw, default 1. 
       */
      virtual void drawArc( int cx, int cy,
                            int startAngle, int stopAngle, 
                            int innerRadius, int outerRadius,
                            GDUtils::Color::CoolColor color,
                            int lineWidth = 1 ) = 0;

      /**
       *  Draws an rectangle.
       */
      virtual void drawRectangle( int x1, int y1, int x2, int y2,
                                  GDUtils::Color::CoolColor color ) = 0;

      /**
       *  Draws a vector with bounding boxes.
       */
      virtual void drawBoundingBox( const MC2BoundingBox& bbox ) = 0;
      /**
       *    Write a text at a given position in the image. 
       *    @param text The text to draw.
       *    @param x    The x-coordinate of the lower left corner of the 
       *                text.
       *    @param y    The y-coordinate of the lower left corner of the 
       *                text.
       *    @param fontSize The size of the text to draw, default 10.0.
       *    @param color    The color of the text, default BLACK.
       *    @param angle    The angle of the text, default 0.0(right).
       *    @param fontName The name of the font to use, 
       *                    default "Vera.ttf".
       */
      virtual void writeText( const char* text, int x, int y,
                              double fontSize = 10.0,
                              GDUtils::Color::CoolColor color = 
                              static_cast< GDUtils::Color::CoolColor>
                              ( 0x000000 ),
                              double angle = 0.0, 
                              const char* fontName = "Vera.ttf" ) = 0;
      
      /**
       * Draw a spline turn arrow using four spline controlpoints and
       * and an arrow. The arrow is drawn either using the three
       * arrow points or the arrowAngle and arrowLength parameters.
       * It is up to the subclass to deside.
       * @param x0 The x value of the first controlpoint.
       * @param y0 The y value of the first controlpoint.
       * @param x1 The x value of the second controlpoint.
       * @param y1 The y value of the second controlpoint.
       * @param x2 The x value of the third controlpoint.
       * @param y2 The y value of the third controlpoint.
       * @param x3 The x value of the fourth controlpoint.
       * @param y3 The y value of the fourth controlpoint.
       * @param arrowX0 The x value of the left arrowpoint.
       * @param arrowY0 The y value of the left arrowpoint.
       * @param arrowX1 The x value of the center arrowpoint.
       * @param arrowY1 The y value of the center arrowpoint.
       * @param arrowX2 The x value of the right arrowpoint.
       * @param arrowY2 The y value of the right arrowpoint.
       * @param arrowAngle The alternative to using the arrow points
       *                   is to use this as the angle of the arrow.
       * @param arrowLength The alternative to using the arrow points
       *                    is to use this as the length of the arrow.
       * @param col The color to draw the turn arrow with.
       * @param arrowWidth The width of the arrows line. Default 1.
       */
      virtual void drawTurnArrow( int x0, int y0, int x1, int y1, 
                                  int x2, int y2, int x3, int y3,
                                  int arrowX0, int arrowY0,
                                  int arrowX1, int arrowY1,
                                  int arrowX2, int arrowY2,
                                  int32 arrowAngle, int32 arrowLength,
                                  GDUtils::Color::CoolColor col,
                                  int arrowWidth = 1 ) = 0;
      /**
       * Draws an scale.
       *
       * @param bbox The boundingbox for the image.
       * @param lang The language of the scale, default English.
       */
      virtual void drawScale( MC2BoundingBox* bbox,
                              StringTable::languageCode lang = 
                              StringTable::ENGLISH ) = 0;
      /**
       *    Draw one GfxFeature-polygon with given settings.
       *    @param   feature  The GfxFeature the polygon belongs to.
       *    @param   polygon  The polygon that will be draw.
       *    @param   settings The draw-settings to use when drawing the 
       *                      polygon.
       *    @param   imageSet The image set to use.
       */
      virtual bool drawGfxFeaturePolygon( const GfxFeature* feature,
                                          GfxPolygon* polygon,
                                          DrawSettings* settings,
                                          ImageTable::ImageSet imageSet ) = 0;
   /**
       *    Write a text at a given position in the bbox.
       *    @param text The text to draw.
       *    @param x    The x-coordinate of the lower left corner of the 
       *                text.
       *    @param y    The y-coordinate of the lower left corner of the 
       *                text.
       *    @param fontSize The size of the text to draw, default 10.0.
       *    @param color    The color of the text, default BLACK.
       *    @param angle    The angle of the text, default 0.0(right).
       *    @param splitText  If you want to write the text on two lines
       *                      the text is long.
       *    @param fontName The name of the font to use,        
       */
      virtual void drawGfxFeatureText( const char* text, int32 lat, int32 lon,
                                       double fontSize = 10.0,
                                       GDUtils::Color::CoolColor color = 
                                       static_cast< GDUtils::Color::CoolColor>
                                       ( 0x000000 ),
                                       double angle = 0.0,
                                       bool moveText = false,
                                       const char* fontName = "Vera.ttf") = 0;

      /**
       *    Write a text at a given position in the bbox.
       *    @param text The text to draw.
       *    @param altLangText The text to draw in alternative language 
       *                       (often english).Drawn below the main text
                               if main text is on one line.
       *    @param x    The x-coordinate of the lower left corner of the 
       *                text.
       *    @param y    The y-coordinate of the lower left corner of the 
       *                text.
       *    @param fontSize The size of the text to draw, default 10.0.
       *    @param color    The color of the text, default BLACK.
       *    @param angle    The angle of the text, default 0.0(right).
       *    @param splitText  If you want to write the text on two lines
       *                      the text is long.
       *    @param fontName The name of the font to use,        
       */
      virtual void drawGfxFeatureText( const char* text, 
                                       const char* altLangText,
                                       int32 lat, int32 lon,
                                       double fontSize = 10.0,
                                       GDUtils::Color::CoolColor color = 
                                       static_cast< GDUtils::Color::CoolColor>
                                       ( 0x000000 ),
                                       double angle = 0.0,
                                       bool moveText = false,
                                       const char* fontName = "Vera.ttf") = 0;

      /**
       */
      virtual void drawGfxFeatureText( const char* text, 
                                       vector<GfxDataTypes::textPos>& textPosition,
                                       double fontSize = 10.0,
                                       GDUtils::Color::CoolColor color =
                                       static_cast< GDUtils::Color::CoolColor>
                                       ( 0x000000 ),
                                       const char* fontName = "Vera.ttf" ) = 0;
      /**
       *    Makes a roadsign at a given position in the bbox.
       *    @param name      The text to draw.
       *    @param signName  The name of the file to draw, including path.
       *    @param lat       The latitude of the sign.
       *    @param lon       The longitude of the sign.
       *    @param border    The frame around the text in the image.
       *    @param fontSize  The initial fontSize to start testing with.
       *    @param signcolor The color of the text, default WHITE.
       *    @param fontName  The name of the font to use, 
       *                     default "Vera.ttf".
       */
      virtual void 
      drawGfxFeatureRoadSign( const char* name, const char* signName,
                              int32 lat, int32 lon, 
                              float32 border = 0.10,
                              double fontSize = 10.0,
                              GDUtils::Color::
                              CoolColor signColor =
                              static_cast<GDUtils::Color::CoolColor>
                              ( 0xFFFFFF ),
                              const char* fontName = "Vera.ttf" ) = 0;
      /**
       * Draw the route as an arrow. The arrow head is drawn at the 
       * final point in route. All coordinates should be screen/image
       * coordinates.
       *
       * @param route Vector containing the points forming the route.
       *              These points should end at a distance from the 
       *              edge making it possible to draw the arrow head.
       * @param arrowHead Vector containing the points forming the arrow
       *                  head. Exactly three points should be contained
       *                  in the vector, ordered: left corner, top, 
       *                  right corner.
       * @param color The color used for drawing the whole arrow, 
       *              including both the route and the arrow head.
       * @param routeWidth The width used when drawing the route part of 
       *                   the arrow.
       */
      virtual void drawRouteAsArrow( vector<POINT> route,
                                     vector<POINT> arrowHead,
                                     GDUtils::Color::CoolColor color,
                                     uint32 routeWidth ) = 0;
      
      /**
       *    Get the dimensions of the glyphs of the specified font/fontSize
       *    and text. Unless xFactor/yFactor is supplied, the dimension
       *    vector will keep the width and height in pixels of each glyph 
       *    in the text. dimension[i] will correspond to the dimensions
       *    of the glyph at position i in text.
       *    
       *    @param   fontSize    The number of points of the font.
       *    @param   fontName    The name of the font.
       *    @param   text        The text.
       *    @param   dimensions  Out parameter. Vector of dimension structs,
       *                         where dimensions[i] corresponds to the
       *                         width and height of glyph text[i] 
       *                         ( i < strlen(text)).
       *    @param   xFactor     Optional paramater. Factor to multiply
       *                         the width dimensions with.
       *    @param   yFactor     Optional paramater. Factor to multiply
       *                         the height dimensions with.
       *    @return  True if the fonts and glyphs could be accessed
       *             succesfully, false otherwise.
       */
      virtual bool getGlyphDimensions( int fontSize,
                                       const char* fontName,
                                       const char* text,
                                       vector<GfxDataTypes::dimensions>& dimensions,
                                       float64 xFactor = 1.0,
                                       float64 yFactor = 1.0 ) = 0;
   /**
    * Get dimension of an entire string.
    * @param fontSize The size of the font to be used.
    * @param fontName Font name to be used.
    * @param text The text to measure the size of.
    * @param xFactor Optional factor to multiply the width dimensions with.
    * @param yFactor Optional factor to multiply the height dimensions with.
    * @return Dimensions of the text with the font and font size selected.
    */
   virtual GfxDataTypes::dimensions 
   getStringDimension( int fontSize,
                       const char* fontName,
                       const char* text,
                       float64 xFactor = 1.0, 
                       float64 yFactor = 1.0 ) const {
      // TODO: Implement in gd
      GfxDataTypes::dimensions dim = { 0, 0 };
      return dim;
   }

   /**
    * Resize image without scaling.
    * @param width in pixels
    * @param height in pixels
    */
   virtual void cutImage( uint32 width, uint32 height ) = 0;

protected:

   static MC2String getImageFullPath( const MC2String& filename );

   /**
    * Gets the filename for a drawsetting symbol.
    * @param sym the symbol to get the filename for
    * @param setting draw settings for some specific symbols
    * @param feature specific poi feature
    * @param imageSet The image set to use.
    * @return filename for the symbol
    */
   static MC2String getSymbolFilename( DrawSettings::symbol_t sym,
                                       const DrawSettings& setting,
                                       const GfxFeature* feature,
                                       ImageTable::ImageSet imageSet );
   /**
    * Gets the filename for a gfxfeature
    * @param type the gfx feature type
    * @param imageSet the image set to use.
    * @return filename of the gfx feature image
    */
   static MC2String getSymbolFilename( GfxFeature::gfxFeatureType type,
                                       ImageTable::ImageSet imageSet );

   /**
    * Gets the filename for a POI.
    * @param type the point of interest type
    * @param imageSet the image set to use.
    * @param extraPOIInfo extra information for some specific pois
    * @return filename to POI image
    */
   static MC2String getPOISymbolFilename( ItemTypes::pointOfInterest_t type,
                                          ImageTable::ImageSet imageSet,
                                          byte extraPOIInfo = MAX_BYTE );


      /**
       * Returns the RGB value of a pixel.
       * @param x The x-coordinate of the pixel to get.
       * @param y The y-coordinate of the pixel to get.
       * @param red Set to the red part of the pixel color.
       * @param green Set to the green part of the pixel color.
       * @param blue Set to the blue part of the pixel color.
       */

      virtual void getPixel( uint32 x, uint32 y, 
                             unsigned char& red, 
                             unsigned char& green, 
                             unsigned char& blue ) = 0;

      
      /**
       * Transfers the pixels from the image to the ImageMagick image.
       * This default implementation calls getPixel.
       * @param image The ImageMagick image to set pixels in.
       */
      virtual void transferPixels( void* image );


      /**
       * Transforms the image using ImageMagic.
       * Transfers image pixels using getPixel.
       *
       * @param image The image to transform.
       * @param format The format to transform to.
       * @param size Set to the length of the retuned buffer.
       * @param format The type of encoding of the buffer.
       * @return A vector with the image or NULL if not supported.
       */
      byte* convertImageToBuffer( uint32& size, 
                                  ImageDrawConfig::imageFormat format = 
                                  ImageDrawConfig::GIF );

      
      /**
       * The width of the image.
       */
      int32 m_width;


      /**
       * The height of the image.
       */
      int32 m_height;
};


#endif // IMAGEDRAW_H
