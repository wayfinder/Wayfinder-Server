/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GDIMAGEDRAW_H
#define GDIMAGEDRAW_H


#include "config.h"

#include "BinarySearchTree.h"
#include "DrawSettings.h"
#include "ImageDraw.h"
#include "ISABThread.h"
#include "GfxDataTypes.h"
#include "DrawingProjection.h"
#include "PixelBox.h"

#include "MC2BoundingBox.h"

// GD the graphics library for fast image creation
#include "gd.h"

// Freetype
#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef gdFTEX_Big5
// Don't know how to detect gd version, so I used gdFTEX_Big5
// Probably means the version we use in Centos installation.
#define NEWER_GD
#endif

#ifdef NEWER_GD
#define USE_TRUE_COLOR
#endif

#undef USE_TRUE_COLOR

/**
 * Class for drawing images in memory and saving it into memory in
 * known image formats.
 *
 */
class GDImageDraw : public ImageDraw {
   public:
      /**
       * Constructs a new Image with the desired dimensions.
       *
       * @param xSize The number of pixels in the x-dimension.
       * @param ySize The number of pixels in the y-dimension.
       * @param color Background color of the image. Default set
       *              to light blue.
       */
      GDImageDraw( uint32 xSize, 
                   uint32 ySize,
                   GDUtils::Color::CoolColor color = 
                   static_cast< GDUtils::Color::CoolColor >
                   ( 0x89A1B7 ) );


      /**
       * Destructor, deallocates image.
       */
      ~GDImageDraw();


      /**
       * Returns the image as a buffer of the right format.
       *
       * @param size Set to the length of the retuned buffer.
       * @param format The type of encoding of the buffer.
       */
      virtual byte* getImageAsBuffer( uint32& size, 
                                      ImageDrawConfig::imageFormat format =
                                      ImageDrawConfig::PNG );



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
      void writeText( const char* text, int x, int y,
                      double fontSize = 10.0,
                      GDUtils::Color::CoolColor color =
                      static_cast< GDUtils::Color::CoolColor >
                      ( 0x000000 ),
                      double angle = 0.0, 
                      const char* fontName = "Vera.ttf" );      

      void setDrawingProjection( const DrawingProjection* drawingProjection );
      
      /**
       *    Draw one GfxFeature-polygon with given settings.
       *    @param   feature  The GfxFeature the polygon belongs to.
       *    @param   polygon  The polygon that will be draw.
       *    @param   settings The draw-settings to use when drawing the 
       *                      polygon.
       *    @param   imageSet The image set to use.
       */
      bool drawGfxFeaturePolygon(const GfxFeature* feature,
                                 GfxPolygon* polygon,
                                 DrawSettings* settings, 
                                 ImageTable::ImageSet imageSet );


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
      void drawGfxFeatureText( const char* text, int32 lat, int32 lon,
                               double fontSize = 10.0,
                               GDUtils::Color::CoolColor color =
                               static_cast< GDUtils::Color::CoolColor >
                               ( 0x000000 ),
                               double angle = 0.0,
                               bool moveText = false,
                               const char* fontName = "Vera.ttf");

   void drawGfxFeatureText( const char* text, 
                            const char* altText,
                            int32 lat, int32 lon,
                            double fontSize = 10.0,
                            GDUtils::Color::CoolColor color = 
                            static_cast< GDUtils::Color::CoolColor>
                            ( 0x000000 ),
                            double angle = 0.0,
                            bool moveText = false,
                            const char* fontName = "Vera.ttf");
      
      /**
       */
      void drawGfxFeatureText( const char* text, 
                               vector<GfxDataTypes::textPos>& textPosition,
                               double fontSize = 10.0,
                               GDUtils::Color::CoolColor color =
                               static_cast< GDUtils::Color::CoolColor >
                               ( 0x000000 ),
                               const char* fontName = "Vera.ttf" );


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
      void drawGfxFeatureRoadSign( 
         const char* name, const char* signName,
         int32 lat, int32 lon, float32 border = 0.10,
         double fontSize = 10.0,
         GDUtils::Color::CoolColor signColor =
         static_cast< GDUtils::Color::CoolColor >( 0xFFFFFF ),
         const char* fontName = "Vera.ttf" );

      


      /**
       *    Initializes the static images, used when drawing features that
       *    are represented by an image. This will be called when a new
       *    object is created, but will return emediately every time except
       *    for the first call. The static member m_imagesInitialized is
       *    used to ensure this.
       *    @see removeDrawImages.
       */
      static void initializeDrawImages();


      /**
       *    Removes the static images.
       *    @see initializeDrawImages.
       */
      static void removeDrawImages();


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
                            int lineWidth = 1 );
      
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
                                  int32 arrowAngle, int32 arrowLengh,
                                  GDUtils::Color::CoolColor col,
                                  int arrowWidth = 1 );
      
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
       * @param routeWidth The width used when drawingthe route part of 
       *                   the arrow.
       */
      virtual void drawRouteAsArrow(vector<POINT> route,
                                    vector<POINT> arrowHead,
                                    GDUtils::Color::CoolColor color,
                                    uint32 routeWidth);

      /**
       * Draws an scale.
       *
       * @param bbox The boundingbox for the image.
       * @param lang The language of the scale, default English.
       */
      virtual void drawScale( MC2BoundingBox* bbox,
                              StringTable::languageCode lang = 
                              StringTable::ENGLISH );


      /**
       *    Draw a point at given coordinates.
       *    @param x       The x-coordinate for the center of the point.
       *    @param y       The y-coordinate for the center of the point.
       *    @param color   The color that will be used when drawing the 
       *                   point.
       *    @param radius  Optional parameter that is the radius of the
       *                   point.
       */
      void drawPoint( int x, int y, GDUtils::Color::CoolColor color,
                      int radius = 1 );

      /**
       *
       */
      void drawSquare(int x, int y, GDUtils::Color::CoolColor color,
                      int side = 1 );

      /**
       *  Draws a rectangle.
       */
      virtual void drawRectangle( int x1, int y1, int x2, int y2,
                                  GDUtils::Color::CoolColor color );
      /**
       *  Draws a mc2 bounding box.
       */ 
      virtual void drawBoundingBox( const MC2BoundingBox& bbox );

      /**
       *
       */
      PixelBox getStringAsPixelBox( const char* fontName,
                                    int fontSize,
                                    const char* text);
      
      

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
      bool getGlyphDimensions( int fontSize,
                               const char* fontName,
                               const char* text,
                               vector<GfxDataTypes::dimensions>& dimensions,
                               float64 xFactor = 1.0,
                               float64 yFactor = 1.0 );

      void cutImage( uint32 sizeX, uint32 sizeY );
      
   protected:
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
                             unsigned char& blue );


      /**
       * Transfers the pixels from the image to the ImageMagick image.
       * This implementation tries to be better than the default one.
       *
       * @param image The ImageMagick image to set pixels in.
       */
      virtual void transferPixels( void* image );

   private:
      /**
       * Get a color.
       * @param color The color requested.
       * @return The index of the color or the closest color in the image
       *         palette.
       */
      int getColor( GDUtils::Color::CoolColor color );
      int getColor( GDUtils::Color::imageColor color );

      /**
       * Draws a wide line with rounded start and end.
       * @param x1 The x value of the start point.
       * @param y1 The y value of the start point.
       * @param x2 The x value of the end point.
       * @param y2 The y value of the end point.
       * @param color GD image palette index of color to use.
       * @param width The thickness of the line.
       */
      void roundedLine( int x1, int y1, int x2, int y2,
                        int color, int width );


      /**
       *    Draw a symbol on the image. E.g. the car at route origin or the
       *    flag at the destination.
       *    @param t The type of the symbol as defined in the DrawSettings.
       *    @param x The x-cooridinate for the center of the symbol.
       *    @param y The y-cooridinate for the center of the symbol.
       *    @param settings The DrawSettings used to determine the POI 
       *           type.
       *    @param feature The GfxFeature.
       *    @param imageSet The image set to use.
       */
      void drawSymbol(DrawSettings::symbol_t t, int x, int y,
                      DrawSettings* settings,
                      const GfxFeature* feature,
                      ImageTable::ImageSet imageSet );


      /**
       * Get the image for the symbol.
       * @param type The type of symbol image to get.
       * @param imageSet The image set to use.
       * @return The symbol's image or NULL if not such image.
       */
      gdImagePtr getSymbolImage( GfxFeature::gfxFeatureType type,
                                 ImageTable::ImageSet imageSet ) const;

      
      /**
       * Get the image for the POI symbol.
       * @param type The poi-type of symbol image to get.
       * @param imageSet The image set to use.
       * @return The symbol's image or NULL if not such image.
       */
      gdImagePtr getPOISymbolImage( 
            ItemTypes::pointOfInterest_t type,
            ImageTable::ImageSet imageSet,
            byte extraPOInfo = MAX_BYTE ) const;

      /**
       * The image to draw on.
       */
      gdImagePtr m_image;


      /**
       *    Variable used to make sure that the images not are initialized
       *    more than once.
       */
      static bool m_imagesInitialized;
      
      const DrawingProjection* m_drawingProjection;

      /**
       * Get the brush with the parameters.
       * Monitor method.
       */
      static gdImagePtr getBrush( int width, 
                                  int red, int green, int blue );
      

      /**
       * Make a new image with the signText on it.
       * @param sign The background image to draw text on.
       * @param signText The text to draw.
       * @param x The x value of the upper left corner of the drawable area.
       * @param y The y value of the upper left corner of the drawable area.
       * @param width The width of the drawable area.
       * @param height The height of the drawable area.
       * @param color The color of the text.
       * @param fontSize The initial fontSize to start testing with.
       * @param fontName The name of the font to use.
       */
      static gdImagePtr makeRoadSign( 
         gdImagePtr sign, const char* signText,
         int x, int y, int width, int height, 
         GDUtils::Color::CoolColor color,
         float32 fontSize, const char* fontName );


      /**
       * Stores brushes for later reuse.
       */
      static BinarySearchTree m_brushes;

      
      /**
       * Protects the m_brushes.
       */
      static ISABMonitor m_brushMon;


      /**
       * Protects the font system.
       */
      static ISABMonitor m_fontMonitor;


      /**
       * Class for holding a brush.
       * Identified by Width and Color.
       */
      class BrushNotice: public BinarySearchTreeNode {
         public:
            BrushNotice( gdImagePtr brush, int width,
                         int red, int green, int blue );
            BrushNotice();

            virtual ~BrushNotice();

            /**
             *    @name Operators.
             *    The operators needed to keep the tree in order.
             */
            //@{
               /** 
                *   True if node's key is higher. 
                */
               virtual bool
                  operator >  (const BinarySearchTreeNode &node) const;

               /** 
                *   True if node's key is lower. 
                */
               virtual bool
                  operator <  (const BinarySearchTreeNode &node) const;
               
               /** 
                *   True if keys match.
                */
               virtual bool
                  operator == (const BinarySearchTreeNode &node) const;
            //@}

            gdImagePtr getBrush() const;
         
            gdImagePtr m_brush;
            int m_width;
            int m_red;
            int m_green;
            int m_blue;
      };

      
      /**
       * Used for finding brushes.
       */
      static BrushNotice m_findBrush;


      /**
       * Get the roadsign with the parameters.
       * Monitor method.
       */
      static gdImagePtr getRoadSign( const char* name, 
                                     const char* signName,
                                     float32 border,
                                     double fontSize,
                                     GDUtils::Color::CoolColor signColor,
                                     const char* fontName );


      /**
       * Stores images for later reuse.
       */
      static BinarySearchTree m_images;

      
      /**
       * Protects the m_imagecache.
       */
      static ISABMonitor m_imagesMon;


      /**
       * Class for holding an image.
       * Identified by name.
       */
      class ImageNotice: public BinarySearchTreeNode {
         public:
            ImageNotice( gdImagePtr image, 
                         const char* name, const char* extraName = "" );
            ImageNotice();

            virtual ~ImageNotice();

            /**
             *    @name Operators.
             *    The operators needed to keep the tree in order.
             */
            //@{
               /** 
                *   True if node's key is higher. 
                */
               virtual bool
                  operator >  (const BinarySearchTreeNode &node) const;

               /** 
                *   True if node's key is lower. 
                */
               virtual bool
                  operator <  (const BinarySearchTreeNode &node) const;
               
               /** 
                *   True if keys match.
                */
               virtual bool
                  operator == (const BinarySearchTreeNode &node) const;
            //@}

            gdImagePtr getImage() const;
         
            void setName( const char* name, const char* extraName = "" );

            gdImagePtr m_image;
            char* m_name;
      };

      
      /**
       * Used for finding image.
       */
      static ImageNotice m_findImage;


      /**
       * The freetype library.
       */
      static FT_Library m_ftLibrary;

      
      /**
       * Stores faces for later reuse. Face cache.
       */
      static BinarySearchTree m_faces;

      
      /**
       * Protects the m_faces.
       */
      static ISABMonitor m_facesMon;
      

      /**
       * Class for holding a freetype face.
       * A face object contains all the instance and glyph 
       * independent data of a font file typeface.     
       * Identified by name.
       */
      class FaceNotice: public BinarySearchTreeNode {
         public:
            /**
             * Constructor with parameters.
             * @param   face  The face.
             * @param   name  The name of the face.
             */
            FaceNotice( FT_Face face, 
                        const char* name );

            /**
             * Construct without parameters. Use as key when searching
             * for a face.
             */
            FaceNotice();

            /**
             * Destructor.
             */
            virtual ~FaceNotice();

            /**
             *    @name Operators.
             *    The operators needed to keep the tree in order.
             */
            //@{
               /** 
                *   True if node's key is higher. 
                */
               virtual bool
                  operator >  (const BinarySearchTreeNode &node) const;

               /** 
                *   True if node's key is lower. 
                */
               virtual bool
                  operator <  (const BinarySearchTreeNode &node) const;
               
               /** 
                *   True if keys match.
                */
               virtual bool
                  operator == (const BinarySearchTreeNode &node) const;
            //@}

            /**
             * Get the face.
             * @return  The face.
             */
            FT_Face getFace() const;
         
            /**
             * Set the name.
             * @param   name  The name.
             */
            void setName( const char* name );

         private:
            /**
             * The face.
             */
            FT_Face m_face;

            /**
             * The name.
             */
            char* m_name;
      };

      
      /**
       * Used for finding face in the face cache.
       */
      static FaceNotice m_findFace;
      
      
      /**
       * Loads an image and checks transparensy.
       * NB! File must be PNG!
       * @param fileName The file of the image.
       * @param useCache If the image cache should be used when retreiving
       *                 and to store the image, default true.
       * @param useImagePath If the imagepath in the properties should be
       *                     used, defult true.
       * @return New image or NULL if error. If useCache then the image
       *         is stored in the cache and shouldn't be deallocated.
       */
      static gdImagePtr loadImage( const char* fileName, 
                                   bool useCache = true,
                                   bool useImagePath = true );
      
      /**
       * Loads a face (A face object contains all the instance and glyph 
       * independent data of a font file typeface).
       * @param fontName Name of the face. Excluding path.
       * @param useCache If the face cache should be used when retreiving
       *                 and to store the face, default true.
       * @return New face or NULL if error. If useCache then the image
       *         is stored in the cache and shouldn't be deallocated.
       */
      static FT_Face loadFace( const char* fontName, 
                               bool useCache = true );

      
      /**
       * Makes a path to a font. 
       *
       * @param font Resulting path is printed into this string.
       * @param fontName The font to use.
       */
      static void getFontPath( char* font, 
                               const char* fontName = "Vera.ttf" );

   static char* drawString( gdImagePtr image, int col, const char* fontIn,
                            double fontSize, float64 angle, 
                            int x, int y, const char* textIn );
};


#endif // GDIMAGEDRAW_H

