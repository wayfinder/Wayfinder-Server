/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Cairo.h"

#ifdef HAVE_CAIRO

#include <cairo-ft.h>
#include <memory>

#include "ScopedArray.h"
#include "ImageMagick.h"
#include "DeleteHelpers.h"
#include "TextIterator.h"

#ifndef M_PI
#define M_PI 3.141592
#endif


namespace {

inline void setColor( cairo_t* gc, const GSystem::Color& col ) {
   // cairo uses color range from 0.0 to 1.0

   cairo_set_source_rgb( gc, 
                         col.getRed() / 255.0,
                         col.getGreen() / 255.0,
                         col.getBlue() / 255.0 );
}

/// sets fill color and fills the surface 
inline void fill( GSystem::Cairo::GContext& cairoGC ) {
   cairo_t* gc = cairoGC.getContext();
   setColor( gc, cairoGC.getFillColor() );
   if ( cairoGC.getLineWidth()  == 0 ) {
      cairo_fill( gc );
   } else {
      cairo_fill_preserve( gc );
   }
}
}

namespace GSystem {
namespace Cairo {
inline void drawText( cairo_t* gc, int x, int y, double angle, const char* text  ) {
   cairo_move_to( gc, x, y );
   if ( angle != 0 ) {
      double angleRad = angle * M_PI / 180.0;
      cairo_rotate( gc, angleRad );
      cairo_show_text( gc, text );
      cairo_rotate( gc, -angleRad ); // restore angle
   } else {
      cairo_show_text( gc, text );
   }
}

inline void drawOutline( Cairo::GContext& cairoGC, 
                         int x, int y, double angle,
                         const Color& color,
                         int os, const char* text ) {
   MC2_ASSERT( os > 0 );
   cairo_t* gc = cairoGC.getContext();
   ::setColor( gc, color );
   if ( os == 1 ) {
      /* 
       * (-os,-os)
       *     +--+--+--+
       *     |x |x |x |
       *     +--+--+--+
       *     |x |  |x |
       *     +--+--+--+
       *     |x |x |x |
       *     +--+--+--+
       *           (+os,+os)
       */
      // draw text in all direction (eight places)
      drawText( gc, x + os, y, angle, text );
      drawText( gc, x + os, y + os, angle, text );
      drawText( gc, x , y + os, angle, text );
      drawText( gc, x - os, y + os, angle, text );
      drawText( gc, x + os, y - os, angle, text );
      drawText( gc, x - os, y, angle, text );
      drawText( gc, x - os, y - os, angle, text );
      drawText( gc, x, y - os, angle, text );
   } else {
      // draw text in all direction


      /* 
       * (-os,-os)
       *     +--+--+--+--+--+
       *     |c |x |x |x |c |
       *     +--+--+--+--+--+
       *     |x |i |i |i |x |
       *     +--+--+--+--+--+
       *     |x |i |  |i |x |
       *     +--+--+--+--+--+
       *     |x |i |i |i |x |
       *     +--+--+--+--+--+
       *     |c |x |x |x |c |
       *     +--+--+--+--+--+
       *                 (+os,+os)
       */

      // first draw the sections marked 'x' and 'c'

      // draw upper and lower horizontal area
      for ( int i = -os; i < os; ++i ) {
         drawText( gc, x + i, y - os, angle, text );         
         drawText( gc, x + i, y + os, angle, text );
      }
      // draw both vertical sides,
      // These sides only needs to be drawn 2*os times less,
      // on each side, than the horizontal side.
      // ( due to the overlapping corners 'c')
      // 
      for ( int i = -os + 1; i < os - 1; ++i ) {
         drawText( gc, x - os, y + i, angle, text );
         drawText( gc, x + os, y + i, angle, text );
      }
      // now the section 'i' needs to be drawn;
      // Reduce the offset and do the drawOutline again.
      // The recursion will end once os == 1
      drawOutline( cairoGC, x, y, angle, 
                   color, os - 1, text );
   }
}

Surface::Surface( cairo_surface_t* surface ):
   m_surface( surface ) {

}
Surface::Surface( uint32 width, uint32 height ):
   m_surface( cairo_image_surface_create( CAIRO_FORMAT_RGB24,
                                          width, height ) ) {
      
}


Font::Font( const MC2String& filename,
            const Font* unicodeFallback ) throw ( GSystem::Exception ) :
   GSystem::Font( GSystem::Font::SLANT_NORMAL, 
                  GSystem::Font::WEIGHT_NORMAL ),
   m_library( NULL ),
   m_fontFace( NULL ),
   m_cairoFontFace( NULL ),
   m_unicodeFallback( unicodeFallback ) {

   FT_Init_FreeType( &m_library );
   // FIXME: error checking
   FT_New_Face( m_library,
                filename.c_str(),
                0,
                &m_fontFace );

   if ( m_fontFace == NULL ) {
      throw GSystem::
         Exception( MC2String("[Cairo::Font] Failed to load font: ") + 
                    filename );
   }

   m_cairoFontFace = 
      cairo_ft_font_face_create_for_ft_face( m_fontFace, 
                                             FT_LOAD_FORCE_AUTOHINT );

   cairo_font_face_reference( m_cairoFontFace );
   // setup parent Font stuff
   if ( m_fontFace->style_flags & FT_STYLE_FLAG_BOLD ) {
      setWeight( Font::WEIGHT_BOLD );
   }

   if ( m_fontFace->style_flags & FT_STYLE_FLAG_ITALIC ) {
      setSlant( Font::SLANT_ITALIC );
   }

}

Font::~Font() {
   cairo_font_face_destroy( m_cairoFontFace );
   FT_Done_FreeType( m_library );
}


void Font::drawText( Cairo::GContext& context,
                     int x, int y, double angle,
                     const char* text ) const {

   // Should we fall back on a different font?
   if ( m_unicodeFallback != NULL &&
        !supportsAllCharacters( text ) ) {
      m_unicodeFallback->drawText( context, x, y, angle, text );
      return;
   }

   cairo_t* gc = context.getContext();
   cairo_set_font_face( gc, m_cairoFontFace );
   cairo_set_font_size( gc, getSize() );

   // if outline size is larger than zero then 
   // draw the outline first and then the real text
   if ( getOutlineSize() > 0 ) {
      Cairo::drawOutline( context, x, y, angle, 
                          getOutlineColor(), 
                          getOutlineSize(), 
                          text );
   }
   ::setColor( gc, context.getForegroundColor() );

   Cairo::drawText( gc, x, y, angle, text );
}

void Font::getTextExtents( GContext& gc, Extents& extents,
                           const char* text ) const {

   // Should we fall back on a different font?
   if ( m_unicodeFallback != NULL &&
        !supportsAllCharacters( text ) ) {
      m_unicodeFallback->getTextExtents( gc, extents, text );
      return;
   }

   cairo_set_font_face( gc.getContext(), m_cairoFontFace );
   cairo_set_font_size( gc.getContext(), getSize() );

   cairo_text_extents_t cairoExtents;
   cairo_text_extents( gc.getContext(), text, &cairoExtents );

   // fill in our Extents structure

   extents.x_advance = cairoExtents.x_advance;
   extents.y_advance = cairoExtents.y_advance;
   extents.width = (uint32)cairoExtents.width;
   extents.height = (uint32)cairoExtents.height;
}

bool Font::supportsAllCharacters( const char* text ) const {
   mc2TextIterator itr( text );
   
   while ( *itr ) {
      // Does the font have a glyph for this character?
      if ( FT_Get_Char_Index( m_fontFace, *itr ) == 0 ) {
         return false;
      }
      ++itr;
   }

   return true;
}

FontCache::FontCache( const MC2String& unicodeFallback )
: m_unicodeFallback( NULL ) {
   if ( unicodeFallback != "" ) {
      m_unicodeFallback = loadFont( unicodeFallback );
   }
}

FontCache::~FontCache() {
   STLUtility::deleteAllSecond( m_fonts );
}

Font* FontCache::loadFont( const MC2String& filename ) try {

   Fonts::iterator it = m_fonts.find( filename );
   if ( it != m_fonts.end() ) {
      return  (*it).second;
   }
   Font* font = new Font( filename, m_unicodeFallback );
   m_fonts[ filename ] = font;
   return font;

} catch ( const Exception& e ) {
   return NULL;
}

void drawLine( Cairo::GContext& gc,
               int32 x1, int32 y1,
               int32 x2, int32 y2 ) {
   cairo_t* context = gc.getContext();
   cairo_move_to( context,
                  x1, y1 );
   cairo_line_to( context,
                  x2, y2 );
   cairo_stroke( context );
}

void drawArc( Cairo::GContext& gc,
              int32 cx, int32 cy,
              uint32 radius,
              int32 startAngle, int32 endAngle ) {
   GContext::GCRestoreState state( gc );
   cairo_t* context = gc.getContext();
   // move to center and draw arc
   cairo_move_to( context, cx + radius, cy );
   cairo_arc( context, cx, cy,
              radius,
              startAngle * M_PI / 180.0,
              endAngle * M_PI / 180 );
   if ( gc.useFill() ) {
      ::fill( gc );
   }
   ::setColor( context, gc.getForegroundColor() );
   cairo_stroke( context );
}

void drawRectangle( Cairo::GContext& gc,
                    int32 topX, int32 topY,
                    int32 bottomX, int32 bottomY ) {
   cairo_t* context = gc.getContext();
   
   cairo_rectangle( context, 
                    topX, topY,
                    bottomX - topX, bottomY - topY );
   // fill rectangle, if specified
   if ( gc.useFill() ) {
      ::fill( gc );
   } 
   ::setColor( context, gc.getForegroundColor() );
   cairo_stroke( context );
}

void drawPolygon( Cairo::GContext& gc,
                  const std::vector<Point>& points ) {
   if ( points.size() <= 1 ) {
      return;
   }
   cairo_t* context = gc.getContext();
   cairo_move_to( context, points[ 0 ].getX(), points[ 0 ].getY() );

   for ( uint32 i = 1; i < points.size(); ++i ) {
      cairo_line_to( context, 
                     points[ i ].getX(), points[ i ].getY() );
   }
   cairo_close_path( context );

   if ( gc.useFill() ) {
      ::fill( gc );
   }

   ::setColor( context, gc.getForegroundColor() );
   cairo_stroke( context );
}

std::auto_ptr<Surface> loadPNG( const MC2String& filename ) {
   std::auto_ptr<Cairo::Surface> 
      surf( new Cairo::
            Surface( cairo_image_surface_create_from_png( filename.c_str())));

   // check if the loading succeeded, or if the image was empty
   cairo_status_t surfaceStatus 
      = cairo_surface_status( surf.get()->getSurface() );
   if ( surfaceStatus != CAIRO_STATUS_SUCCESS ||
        surf->getWidth() == 0 ||
        surf->getHeight() == 0 ) {
      return std::auto_ptr<Surface>();
   }

   return surf;
}

void blitSurface( GContext& gc, Surface& surf, int x, int y ) {
   cairo_set_source_surface( gc.getContext(), 
                             surf.getSurface(),
                             x - static_cast< int >( surf.getWidth() / 2 ),
                             y - static_cast< int >( surf.getHeight() / 2 ) );

   cairo_paint( gc.getContext() );
}

} // cairo

               
std::auto_ptr<GSystem::Magick::Image>
saveImageBase( Cairo::Surface& surf,
               const char* magick ) {

   // need to convert our ARGB surface to RGB and then read it 
   // into Magick::Image 
   // Couldn't get Magick::Image to read ARGB directly for some reason.
   const void* orgdata = cairo_image_surface_get_data( surf.getSurface() );
   using GSystem::Magick::Image;
   //   using namespace Magick;

   uint32 dataSize32 = surf.getWidth() * surf.getHeight();
   ScopedArray<char> rgb( new char[ surf.getWidth() * surf.getHeight() * 3 ] );
   const int* oldrgb= reinterpret_cast<const int*>( orgdata );
   for ( uint32 i = 0, j = 0; i < dataSize32; ++i, j += 3 ) {
      rgb[ j ] = ( oldrgb[ i ] >> 16 ) & 0xFF;
      rgb[ j + 1 ] = ( oldrgb[ i ] >> 8 ) & 0xFF;
      rgb[ j + 2 ] = oldrgb[ i ] & 0xFF;
   }

   // read RGB data
   auto_ptr<Image> image( new Image( surf.getWidth(), surf.getHeight(),
                                     "RGB",
                                     CharPixel, rgb.get() ) );

   // setup quantization for 256 colors and file format
   image->quantizeColorSpace( RGBColorspace );
   image->quantizeColors( 256 );
   image->quantize();

   image->magick( magick );
   image->setImageType( PaletteType );

   return image;
}

pair< unsigned char*, int >
saveImage( Cairo::Surface& surf,
           const char* magick ) {
   auto_ptr<GSystem::Magick::Image> image( saveImageBase( surf, magick ) );
   pair< unsigned char*, int> content( image->getBuffer() );
   unsigned char* data = new unsigned char[ content.second ];
   memcpy( data, content.first, content.second );
   return make_pair( data, content.second );
}

bool saveImage( Cairo::Surface& surf,
                const MC2String& filename,
                const char* magick ) {
   auto_ptr<GSystem::Magick::Image> image( saveImageBase( surf, magick ) );
   return image->write( filename );
}

pair< unsigned char*, int >
savePNG( Cairo::Surface& surf ) {
   return saveImage( surf, "PNG8" );
}

pair< unsigned char*, int >
saveGIF( Cairo::Surface& surf ) {
   return saveImage( surf, "GIF" );
}

pair< unsigned char*, int >
saveJPEG( Cairo::Surface& surf ) {
   return saveImage( surf, "JPEG" );
}

bool savePNG( Cairo::Surface& surf, const MC2String& filename ) {
   return saveImage( surf, filename, "PNG8" );
}

bool saveGIF( Cairo::Surface& surf, const MC2String& filename ) {
   return saveImage( surf, filename, "GIF" );
}

bool saveJPEG( Cairo::Surface& surf, const MC2String& filename ) {
   return saveImage( surf, filename, "JPEG" );
}

}
#endif // HAVE_CAIRO
