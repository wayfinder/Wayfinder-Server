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
#include "Properties.h"
#include "CommandlineOptionHandler.h"
#include "PropertyHelper.h"
#include "DeleteHelpers.h"

#include <math.h>

#ifdef HAVE_CAIRO

//using namespace GSystem;
using GSystem::Color;
using namespace GSystem::Cairo;
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

}

class TestObject {
public:
   explicit TestObject( GContext& gc ):m_gc( gc ) { }

   virtual void test( int x, int y, uint32 width, uint32 height ) = 0;
protected:
   GContext& m_gc;
};

#define TESTCLASS(classname) class classname : public TestObject { \
public: \
   explicit classname( GContext& gc ):TestObject( gc ) { } \
   void test( int x, int y, uint32 width, uint32 height ); \
};

TESTCLASS( TestRectangle );
TESTCLASS( TestArc );
TESTCLASS( TestText );
TESTCLASS( TestPolygon );
TESTCLASS( TestDonut );

void TestRectangle::test( int x, int y,
                          uint32 width, uint32 height ) {
   
   // draw with border
   m_gc.setLineWidth( 1 );
   m_gc.setColor( Color( 255, 255, 255 ) );
   m_gc.setFill( false );
   drawRectangle( m_gc, x, y, x + width, y + height );
   
   // draw filled
   m_gc.setLineWidth( 0 );
   m_gc.setFill( true );
   m_gc.setFillColor( Color( 255, 0, 0 ) );
   drawRectangle( m_gc, x + width / 2, y + height / 2, 
                  x + width  - width / 4, y + height - height / 4 );

   // draw filled with border
   m_gc.setLineWidth( 5 );
   m_gc.setFill( true );
   m_gc.setFillColor( Color( 0, 255, 0 ) );
   m_gc.setColor( Color( 150, 150, 255 ) );
   drawRectangle( m_gc,
                  x + width / 4, y + height / 4,
                  x + width / 2, y + height / 2 );
}

void TestArc::test( int x, int y,
                    uint32 width, uint32 height ) {
   const uint32 arcSize = width / 8;
   //      static_cast<uint32>( sqrt( width * width + height * height ) / 2 );

   // draw with border in 90 degrees

   m_gc.setLineWidth( 1 );
   m_gc.setColor( Color( 255, 255, 255 ) );
   m_gc.setFill( false );
   drawArc( m_gc, x + width / 2, y + height / 2,
            arcSize, 270, 360 );

   // draw with border in 360 degrees
   m_gc.setColor( Color( 0, 0, 255 ) );
   drawArc( m_gc, 
            x + width / 2 - arcSize - 1, 
            y + height / 2 - arcSize - 1,
            arcSize / 2, 0, 360 );
   
   // draw filled in 90 degrees
   m_gc.setFill( true );
   m_gc.setFillColor( Color( 255, 0, 0 ) );
   m_gc.setLineWidth( 0 );
   drawArc( m_gc, 
            x + width / 2 + 1, 
            y + height / 2 - arcSize - 1,
            arcSize, 270, 360 );

   // draw filled in 360 degrees
   m_gc.setFill( true );
   m_gc.setFillColor( Color( 0, 255, 0 ) );
   m_gc.setLineWidth( 0 );
   drawArc( m_gc, 
            x + width / 2 + width / 4, 
            y + height - height / 4,
            arcSize, 0, 360 );

   // draw filled with border 180 degrees
   m_gc.setFill( true );
   m_gc.setFillColor( Color( 0, 255, 255 ) ); // yellow
   m_gc.setLineWidth( 1 );
   m_gc.setColor( Color( 255, 255, 0 ) ); // orange

   drawArc( m_gc, 
            x + width / 2 + arcSize, 
            y + height / 2 + arcSize,
            arcSize, 0, 180 );

   // draw filled with border 3600 degrees
   m_gc.setLineWidth( 10 );
   drawArc( m_gc,
            x + width / 2 - arcSize - 10,
            y + height / 2 + arcSize - 10,
            arcSize, 0, 360 );
}

void TestText::test( int x, int y,
                     uint32 width, uint32 height ) {
   const MC2String text( "Testing text drawing" );
   Font font( ::getFontPath( "Vera.ttf" ) );
   font.setSize( 12 );
   // determine text extents
   Font::Extents extents;
   font.getTextExtents( m_gc, extents, text.c_str() );
   
   // draw with outline

   m_gc.setColor( Color( 255, 0, 0 ) );
   font.setOutlineColor( Color( 255, 255, 255 ) );
   font.setOutlineSize( 1 );

   font.drawText( m_gc, 
                  x, y + extents.height + 1, 0,
                  text.c_str() );

   // new size and new extents
   font.setSize( 20 );
   font.getTextExtents( m_gc, extents, text.c_str() );
   // draw text normal in a circle
   m_gc.setColor( Color( 255, 255, 255 ) );
   font.setOutlineSize( 0 );
   const double radius = extents.width;
   const double stepSize = 360.0 / text.size();

   for ( uint32 i = 0; i < text.size(); ++i ) {
      char character[ 2 ] = { text[ i ], 0 };
      double degrees = 270 + i * stepSize;

      int tx = static_cast<int32>
         ( x + width / 2 + radius * cos( degrees * M_PI / 180 ) );
      int ty = static_cast<int32>
         ( y + height / 2 + radius * sin( degrees * M_PI / 180 ) );

      font.
         drawText( m_gc,
                   tx, ty,
                   90 + degrees,
                   character );
   }
   
   font.drawText( m_gc,
                  x + width / 2, y + height / 2,
                  0, " 0 degrees" );
   font.drawText( m_gc,
                  x + width / 2, y + height / 2,
                  90, " 90 degrees" );
   font.drawText( m_gc,
                  x + width / 2, y + height / 2,
                  180, " 180 degrees" );
   font.drawText( m_gc,
                  x + width / 2, y + height / 2,
                  270, " 270 degrees" );
                  
}

void TestPolygon::test( int x, int y,
                        uint32 width, uint32 height ) {
   using GSystem::Point;

   Point points[] = {
      Point( x, y ),
      Point( x + width / 4, y ),
      Point( x + width / 4, y + height / 4 ),

      Point( x + width / 2, y ),
      Point( x + width, y ),
      Point( x + width / 2, y + height / 2 ),
      

      Point( x, y + height ),
      Point( x, y + height / 2 ),
      Point( x + width / 2, y + height / 2 ),
      Point( x + width / 2, y + height / 2 + height / 4 ),
      Point( x + width, y + height / 2 + height / 4 ),
      Point( x + width, y + height )

   };
   // draw triangle with border
   m_gc.setLineWidth( 1 );
   m_gc.setColor( Color( 255, 255, 255 ) );
   m_gc.setFill( false );
   drawPolygon( m_gc, vector<Point>( &points[ 0 ], &points[ 3 ] ) );
   // draw triangle filled
   m_gc.setLineWidth( 0 );
   m_gc.setFillColor( Color( 127, 255, 127 ) );
   m_gc.setFill( true );
   drawPolygon( m_gc, vector<Point>( &points[ 3 ], &points[ 6 ] ) );

   // draw filled with border
   m_gc.setFillColor( Color( 255, 127, 0 ) );
   m_gc.setLineWidth( 1 );
   drawPolygon( m_gc, vector<Point>( &points[ 6 ], &points[ 12 ] ) );

}

void TestDonut::test( int x, int y,
                      uint32 width, uint32 height ) {
   // testing complex paths, like donut.
   // the fill rule is important here.

   cairo_t* gc = m_gc.getContext();
   m_gc.setLineWidth( 5 );
   cairo_move_to( gc, x + width / 4, y  + height / 4 );
   cairo_line_to( gc, 
                  x + width / 2 + width / 4,
                  y + height / 4 );

   cairo_rel_line_to( gc, 0, height / 2 );
   cairo_rel_line_to( gc, -width / 2, 0 );
   cairo_close_path( gc );

   cairo_move_to( gc, 
                  x + width / 4 + width / 8, 
                  y + height / 4 + height / 8 );
   cairo_line_to( gc, 
                  x + width / 2 + width / 8,
                  y + height / 4 + height / 8 );
   cairo_rel_line_to( gc, 0, height / 4 );
   cairo_rel_line_to( gc, -width / 4, 0 );
   cairo_close_path( gc );
   

   // the second path is a hole.
   cairo_set_fill_rule( gc, CAIRO_FILL_RULE_EVEN_ODD );

   cairo_set_source_rgb( gc, 0, 0, 1.0 );
   cairo_fill_preserve( gc );
   cairo_set_source_rgb( gc, 1, 1, 1 );
   cairo_stroke( gc );
}

void test( const MC2String& filename,
           uint32 width, uint32 height ) {

   Surface surface( width, height );
   GContext gc( surface );

   STLUtility::AutoContainer< vector< TestObject*> > tests;
   tests.push_back( new TestRectangle( gc ) );
   tests.push_back( new TestArc( gc ) );
   tests.push_back( new TestText( gc ) );
   tests.push_back( new TestPolygon( gc ) );
   tests.push_back( new TestDonut( gc ) );

   const uint32 numTests = tests.size();
   // half the test should be on the upper half of the screen

   const uint32 testWidth = width / ( numTests / 2 );
   const uint32 testHeight = height / ( numTests / 2 + numTests % 2);

   int x = 0;
   int y = 0;
   for ( uint32 i = 0; i < numTests; ++i ) {
      mc2dbg << "test( " << x << ", " << y << ", " 
             << testWidth << ", " << testHeight << ")" << endl;
      tests[ i ]->test( x, y, testWidth, testHeight );
      x += testWidth;
      if ( x + testWidth > width ) {
         x = 0;
         y += testHeight;
      }
   }

   // save to file, determine fileformat first

   MC2String::size_type dotPos = filename.find_last_of( "." );
   MC2String fileformat;
   if ( dotPos != MC2String::npos ) {
      MC2String format = filename.substr( dotPos + 1, 
                                          dotPos -
                                          filename.size() - 1 );
      if ( strcasecmp( format.c_str(), "png" ) == 0 ) {
         fileformat = "PNG";
      } else if ( strcasecmp( format.c_str(), "gif" ) == 0 ) {
         fileformat = "GIF";
      }
   }

   if ( fileformat.empty() ) {
      mc2dbg << warn << "Failed to determine fileformat." 
             << " Will use PNG format to save. " << endl;
      fileformat = "PNG";
   }

   if ( fileformat == "PNG" ) {
      GSystem::savePNG( surface, filename );
   } else if ( fileformat == "GIF" ) {
      GSystem::saveGIF( surface, filename );
   }
}

int main( int argc, char** argv )
try {   
   PropertyHelper::PropertyInit propInit;

   CommandlineOptionHandler coh( argc, argv );
   coh.setTailHelp( "image file" );
   coh.setSummary( "Draws tests to an image surface"
                   " and stores it as either png or gif" );
   uint32 width = 0, height = 0;
   const char* destination = NULL;

   coh.addOption( "", "--width",
                  CommandlineOptionHandler::uint32Val,
                  1, &width, "1024",
                  "Destination image width." );
   coh.addOption( "", "--height",
                  CommandlineOptionHandler::uint32Val,
                  1, &height, "1024",
                  "Destination image height." );
   coh.addOption( "-d", "--destination",
                  CommandlineOptionHandler::stringVal,
                  1, &destination, "test.png",
                  "Destination image filename." );


   if ( ! coh.parse() ) {
      return 1;
   }

   test( destination, width, height );

   return 0;
} catch ( const GSystem::Exception& e ) {
   mc2dbg << fatal << e.what() << endl;
   return 255;
}

#else

int main( int argc, char** argv ) {
   mc2dbg  << "dont have cairo... recompile me please." << endl;
   return 0;
}
#endif // HAVE_CAIRO
