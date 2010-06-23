/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GSYSTEM_H
#define GSYSTEM_H

#include "config.h"

#include "MC2String.h"
#include "MC2Point.h"

#include <stdexcept>
#include <MC2String.h>
#include <map>

namespace GSystem {

/**
 * Exception thrown by various subsystems in GSystem
 */
class Exception: public std::exception {
public:
   /**
    * @param what exception message
    */
   Exception( const MC2String& what ) throw():
      m_what( what ) {
   }
   ~Exception() throw() {
   }
   const char* what() const throw() { return m_what.c_str(); }

private:
   MC2String m_what; ///< exception message
};

typedef MC2Point Point;

/**
 * Describes a color
 */
class Color {
public:

   Color( int32 red, int32 green, int32 blue ):
      m_red( red ), m_green( green ), m_blue( blue ),
      m_pixelValue( ( ( red & 0xFF ) << 16 ) |
                    ( ( green & 0xFF ) << 8 ) |
                    ( blue & 0xFF ) ) 
   { }

   explicit Color( uint32 value = 0 ):
      m_red( ( value & 0xFF0000) >> 16 ),
      m_green( ( value & 0x00FF00 ) >> 8 ),
      m_blue( value & 0x0000FF ),
      m_pixelValue( value ) {
      
   }
   
   int32 getRed() const { return m_red; }
   int32 getBlue() const { return m_blue; }
   int32 getGreen() const { return m_green; }
   int32 getValue() const { return m_pixelValue; }
private:
   int32 m_red;
   int32 m_green;
   int32 m_blue;
   int32 m_pixelValue;

};

/**
 * Graphic Context for drawing
 */
class GContext {
public:
   /**
    * Cap style for lines
    */
   enum LineCap {
      LINE_CAP_BUTT, ///< default, cuts line directly
      LINE_CAP_ROUND, ///< round cap
      LINE_CAP_SQUARE ///< square cap
   };

   /**
    * Join style for lines
    */
   enum LineJoin {
      LINE_JOIN_MITER, ///< default, miter join style
      LINE_JOIN_BEVEL, ///< bevel join style
      LINE_JOIN_ROUND ///< round join style
   };
   virtual ~GContext() { }
   /// set foreground color
   virtual void setColor( const Color& color ) = 0;
   /// set line width
   virtual void setLineWidth( uint32 width ) = 0;
   /// set color to use for filling
   virtual void setFillColor( const Color& color ) = 0;
   /// set fill 
   virtual void setFill( bool state ) = 0;
   /// @return true if fill mode is used
   virtual bool useFill() const = 0;
   /// set line cap style
   virtual void setLineCap( LineCap cap ) = 0;
   /// set line join style
   virtual void setLineJoin( LineJoin join ) = 0;
   /// @return color used when filling
   virtual const Color& getFillColor() const = 0;
};

/**
 * Interface for surfaces which is used
 * as canvas for drawing
 */
class Surface {
public:
   virtual ~Surface() { }
   virtual uint32 getWidth() const = 0;
   virtual uint32 getHeight() const = 0;
};

/**
 * Describes a font used for text drawing
 */
class Font {
public:
   struct Extents {
      uint32 width;  ///< width in pixels
      uint32 height; ///< height in pixels
      double x_advance; ///< x advance in pixel 
      double y_advance; ///< y advance in pixel
   };

   /// slant style 
   enum Slant { 
      SLANT_NORMAL, ///< default, normal
      SLANT_ITALIC, ///< italic
      SLANT_OBLIQUE ///< oblique
   };
   /// weight style
   enum Weight {
      WEIGHT_NORMAL, ///< default, normal
      WEIGHT_BOLD   ///< bold
   };
   enum Option {
      OPTION_NORMAL,
   };

   Font( Slant slant, Weight weight,
         int32 options = OPTION_NORMAL );
   virtual ~Font() { }
   
   Slant getSlant() const { return m_slant; }
   Weight getWeight() const { return m_weight; }
   uint32 getOptions() const { return m_options; }
   const Color& getOutlineColor() const { return m_outlineColor; }
   uint32 getOutlineSize() const { return m_outlineSize; }
   void setOutlineSize( int32 size ) { 
      m_outlineSize = size;
   }
   void setOutlineColor( const Color& color ) {
      m_outlineColor = color;
   }

   inline void setSize( double size ) { m_size = size; }
   inline double getSize() const { return m_size; }
   /**
    * @param gc
    * @param x
    * @param y
    * @param angle in degrees.
    * @param text the null terminating string to draw
    */
   /*
   virtual void drawText( GContext& context, 
                          int x, int y, double angle,
                          const char* text ) const = 0;
   */
   /**
    * Determines text extents for a string with the font loaded.
    * @param gc
    * @param extents text extents for specificed string
    * @param text the null terminating string to get extents for
    */
   /*
   virtual void getTextExtents( GContext& gc, Extents& extents,
                                const char* text ) const = 0;
   */
protected:
   void setSlant( Slant slant ) { m_slant = slant; }
   void setWeight( Weight flag ) { m_weight = flag; }
private:
   Slant m_slant; ///< slant of font
   Weight m_weight; ///< weight of font
   uint32 m_options; ///< various font options
   Color m_outlineColor; ///< color used for outline
   uint32 m_outlineSize; ///< size of outline in pixels
   double m_size; ///< font size
};

/**
 * Cache system for class Font
 */
class FontCache {
public:
   explicit FontCache();
   ~FontCache();

   Font* loadFont( const MC2String& filename );

private:
   typedef std::map<MC2String, Font*> Fonts;
   Fonts m_fonts;
};

}

#endif
