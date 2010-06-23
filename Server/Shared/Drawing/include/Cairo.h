/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GSYSTEM_CAIRO_H
#define GSYSTEM_CAIRO_H

#include "config.h"
#include "GSystem.h"
#include "MC2String.h"

#ifdef HAVE_CAIRO

#include <cairo.h>
#include <vector>
#include <memory>
#include <ft2build.h>
#include <freetype/freetype.h>
#include <map>

namespace GSystem {

namespace Cairo {

/**
 * A surface used by cairo.
 * Uses 24 bits RGB surface internaly.
 */
class Surface: public GSystem::Surface {
public:
   explicit Surface( cairo_surface_t* surface );
   Surface( uint32 width, uint32 height );
   inline ~Surface();
   /// @return width in pixels
   uint32 getWidth() const;
   /// @return height in pixels
   uint32 getHeight() const;

   /// @return internal cairo surface
   cairo_surface_t* getSurface();

private:
   cairo_surface_t* m_surface; ///< internal cairo surface
};


/**
 * Cairo graphic context
 */
class GContext: public GSystem::GContext {
public:
   /// saves and restores internal cairo states in scope
   struct GCRestoreState {
      GCRestoreState( GContext& gc ):m_gc( gc ) { 
         gc.save();
      }
      ~GCRestoreState() {
         m_gc.restore();
      }

      GContext& m_gc;
   };

   explicit GContext( Cairo::Surface& surf );
   ~GContext();
   /// sets foreground color
   inline void setColor( const Color& color );
   /// sets line width
   inline void setLineWidth( uint32 width );
   /// sets fill mode
   inline void setFill( bool state );
   /// sets fill color used if fill mode = true
   inline void setFillColor( const Color& color );
   /// sets line cap style
   inline void setLineCap( LineCap cap );
   /// sets line join style
   inline void setLineJoin( LineJoin join );
   /// @return true if fill is enabled, else false
   inline bool useFill() const { return m_fill; }
   /// @return color to used for filling.
   inline const Color& getFillColor() const { return m_fillColor; }
   /// @return foreground color
   inline const Color& getForegroundColor() const { return m_foregroundColor; }
   inline uint32 getLineWidth() const;
   /// saves internal context state
   inline void save();
   /// restores internal context state
   inline void restore();

   /// @return internal cairo context
   inline cairo_t* getContext() { return m_context; }

private:
   Color m_foregroundColor; ///< foreground color
   bool m_fill; ///< determines if to fill the geometry
   Color m_fillColor; ///< color used for filling
   cairo_t* m_context; ///< cairo implemenation
};

/**
 * Font implementation for cairo.
 * This implementation uses Freetype fonts since we
 * are suppose to load the font directly from a file.
 */
class Font: public GSystem::Font {
public:

   /** 
    * Loads font from file.
    * @param filename         The filename for the font.
    * @param unicodeFallback  A fallback font to use when this font doesn't
    *                         have the required glyphs. Should be a large
    *                         unicode font with characters from many 
    *                         languages. NULL = no fallback font.
    */
   explicit Font( const MC2String& filename, 
                  const Font* unicodeFallback = NULL ) 
      throw ( GSystem::Exception );

   ~Font();

   /**
    * @param gc
    * @param x
    * @param y
    * @param angle in degrees.
    * @param text the null terminating string to draw
    */
   void drawText( Cairo::GContext& context, 
                  int x, int y, double angle,
                  const char* text ) const;
   /**
    * Determines text extents for a string with the font loaded.
    * @param gc
    * @param extents text extents for specificed string
    * @param text the null terminating string to get extents for
    */
   void getTextExtents( GContext& gc, Extents& extents,
                        const char* text ) const;

private:
   /// Does this font support all characters in text?
   bool supportsAllCharacters( const char* text ) const;
   
   FT_Library m_library; ///< used to initialize freetype library 
   FT_Face m_fontFace; ///< the font used for drawing
   cairo_font_face_t* m_cairoFontFace; ///< the cairo font used for drawing
   const Font* m_unicodeFallback; ///< a fallback font or NULL
};

class FontCache: public GSystem::FontCache {
public:
   /**
    * @param unicodeFallback The default fallback font to use in the 
    *                        loaded fonts.
    */
   FontCache( const MC2String& unicodeFallback = "" );
   ~FontCache();
   /// @return pointer to loaded font or NULL on failure
   Font* loadFont( const MC2String& filename );
private:
   typedef std::map<MC2String, Font*> Fonts;
   Fonts m_fonts;
   Font* m_unicodeFallback; ///< The default fallback font or NULL
};

/**
 * Line helping tool. It finalizes the line when it goes out of scope.
 */
class LineDrawing {
public:
   /**
    * Start drawing line segments at point x y.
    */
   LineDrawing( GContext& gc, int32 x, int32 y ):
      m_gc( gc.getContext() ) {
      cairo_move_to( m_gc, x, y );
   }

   ~LineDrawing() {
      cairo_stroke( m_gc );
   }

   /// Continue to draw a line to specific point
   inline void lineTo( int32 x, int32 y ) {
      cairo_line_to( m_gc, x, y );
   }

private:
   cairo_t* m_gc; ///< context for drawing
};


/**
 * Draws a line from x1 y1 to x2 y2
 */               
void drawLine( GContext& gc,
               int32 x1, int32 y1,
               int32 x2, int32 y2 );

/**
 * Draws an arc.
 * @param gc
 * @param x
 * @param y
 * @param radius of the 
 * @param startAngle angle in degrees
 * @param stopAngle angle in degrees
*/
void drawArc( GContext& gc,
              int32 x, int32 y,
              uint radius,
              int32 startAngle, int32 stopAngle );

/**
 * Draws a rectangle from top position topX topY to
 * bottom position bottomX bottomY
 * @param gc
 * @param topX top left corner
 * @param topY top left corner
 * @param bottomX bottom right corner
 * @param bottomY bottom right corner
 */              
void drawRectangle( GContext& gc,
                    int32 topX, int32 topY,
                    int32 bottomX, int32 bottomY );

/**
 * Draws a polygon from a set of points.
 * @param gc
 * @param points a vector of points to draw a polygon for
 */
void drawPolygon( GContext& gc,
                  const std::vector<Point>& points );
 
/**
 * Loads a png file.
 * @param filename the filename of the png file
 * @return pointer to allocated surface on success.
 */
std::auto_ptr<Surface> loadPNG( const MC2String& filename );

/**
 * Paint a surface to another surface
 * @param gc graphic context, the destination context.
 * @param surf the source surface
 * @param x position
 * @param y position
 */
void blitSurface( GContext& gc, Surface& surf, int x, int y );

}

/**
 * Saves surface to png file
 * @param surf the surface to save.
 * @param filename the filename of the png file.
 * @return true if save was successful.
 */
bool savePNG( Cairo::Surface& surf, const MC2String& filename );

/**
 * Saves surface to gif file
 * @param surf the surface to save.
 * @param filename the filename of the gif file.
 * @return true if save was successful.
 */
bool saveGIF( Cairo::Surface& surf, const MC2String& filename );

/**
 * Saves surface to jpeg file.
 * @param surf The surface to save.
 * @param filename The filename of the jpeg file.
 * @return true if save was successful.
 */
bool saveJPEG( Cairo::Surface& surf, const MC2String& filename );

//
// inlines
//
namespace Cairo {
Surface::~Surface() {
   cairo_surface_destroy( m_surface );
}

inline
uint32 Surface::getWidth() const { 
   return cairo_image_surface_get_width( const_cast< cairo_surface_t* >
                                         ( m_surface ) ); 
}
inline
uint32 Surface::getHeight() const { 
   return cairo_image_surface_get_height( const_cast< cairo_surface_t* >
                                          ( m_surface ) ); 
}

inline
cairo_surface_t* Surface::getSurface() { 
   return m_surface; 
}

inline
GContext::GContext( Cairo::Surface& surface ):
   m_context( cairo_create( surface.getSurface() ) ) {
}

inline
GContext::~GContext() {
   cairo_destroy( m_context );
}

inline
void GContext::setColor( const Color& col ) {
   m_foregroundColor = col;
   cairo_set_source_rgb( m_context, 
                         col.getRed() / 255.0,
                         col.getGreen() / 255.0,
                         col.getBlue() / 255.0 );
}

inline
void GContext::setLineWidth( uint32 width ) {
   cairo_set_line_width( m_context, width );
}

inline uint32 GContext::getLineWidth() const {
   return static_cast<uint32>( cairo_get_line_width( m_context ) );
}

inline
void GContext::setFill( bool state ) {
   m_fill = state;
}

inline
void GContext::setFillColor( const Color& color ) {
   m_fillColor = color;
}

inline
void GContext::setLineCap( LineCap cap ) {
   switch ( cap ) {
   case LINE_CAP_BUTT:
      cairo_set_line_cap( m_context, CAIRO_LINE_CAP_BUTT );
      break;
   case LINE_CAP_ROUND:
      cairo_set_line_cap( m_context, CAIRO_LINE_CAP_ROUND );
      break;
   case LINE_CAP_SQUARE:
      cairo_set_line_cap( m_context, CAIRO_LINE_CAP_SQUARE );
      break;
   }
}

inline 
void GContext::setLineJoin( LineJoin join ) {
   switch ( join ) {
   case LINE_JOIN_MITER:
      cairo_set_line_join( m_context, CAIRO_LINE_JOIN_MITER );
      break;
   case LINE_JOIN_BEVEL:
      cairo_set_line_join( m_context, CAIRO_LINE_JOIN_BEVEL );
      break;
   case LINE_JOIN_ROUND:
      cairo_set_line_join( m_context, CAIRO_LINE_JOIN_ROUND );
      break;
   }
}

inline
void GContext::save() {
   cairo_save( m_context );
}

inline
void GContext::restore() {
   cairo_restore( m_context );
}

} // Cairo

/**
 * Saves surface to PNG and returns the buffer of the PNG.
 * Throws exception on error.
 * @param surf The surface to save as PNG.
 */
pair< unsigned char*, int > savePNG( Cairo::Surface& surf );

/**
 * Saves surface to GIF and returns the buffer of the GIF.
 * Throws exception on error.
 * @param surf The surface to save as GIF.
 */
pair< unsigned char*, int > saveGIF( Cairo::Surface& surf );

/**
 * Saves surface to JPEG and returns the buffer of the JPEG.
 * Throws exception on error.
 * @param surf The surface to save as JPEG.
 */
pair< unsigned char*, int > saveJPEG( Cairo::Surface& surf );

}
#endif // HAVE_CAIRO

#endif
