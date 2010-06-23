/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//
// This program loads street information and tries to
// place the text on it. The street is rendered to an image which
// is then displayed in a GTK window
//

#include "GfxDataFull.h"

#include "CairoImageDraw.h"
#include "ScopedArray.h"
#include "TempFile.h"
#include "DrawingProjection.h"
#include "ScreenSize.h"
#include "StreetTextPlacer.h"

// Gtk stuff
#include "Application.h"
#include "Slot.h"
#include "SlotHandler.h"

#include <gtk/gtk.h>

#include <memory>
#include <fstream>

/**
 * Street information to be rendered on to an image.
 */
struct StreetData {
   StreetData():
      m_gfxData( new GfxDataFull() ) {
   }

   ~StreetData() {

   }

   /// box that contains this street
   MC2BoundingBox m_box;
   /// Screen size
   ScreenSize m_screenSize;

   /// bounding box width / screen width
   float32 m_xFactor;

   /// bounding box height / screen height
   float32 m_yFactor;

   /// Font to be used.
   MC2String m_fontName;
   /// Size of the font.
   uint32 m_fontSize;
   /// Name to be placed.
   MC2String m_name;
   /// Width of the street in pixels.
   uint32 m_streetWidth;
   /// Street polygon
   auto_ptr<GfxDataFull> m_gfxData;
};

/**
 * Loads street information from a file.
 * @param filename Name of the file.
 * @return street
 */
auto_ptr<StreetData> createStreet( const char* filename ) {
   ifstream file( filename );

   if ( ! file ) {
      mc2dbg << "Failed to open file: " << filename << endl;
      return auto_ptr<StreetData>();
   }

   auto_ptr<StreetData> street( new StreetData() );
   bool first = true;
   // Format:
   // bounding box
   // width height
   // font name
   // font size
   // street name
   // street width
   // lat0 lon0
   // lat1 lon1
   // lat2 lon2
   // ...
   //


   int32 maxLat = 0;
   int32 maxLon = 0;
   int32 minLat = 0;
   int32 minLon = 0;
   // load bounding box
   file >> maxLat >> minLon >> minLat >> maxLon >> ws;

   street->m_box = MC2BoundingBox( maxLat, minLon,
                                   minLat, maxLon );

   uint32 screenWidth = 0;
   uint32 screenHeight = 0;
   file >> screenWidth >> screenHeight >> ws;
   street->m_screenSize = ScreenSize( screenWidth, screenHeight );

   street->m_xFactor = street->m_box.getWidth() / ( float32 )( screenWidth );
   street->m_yFactor = street->m_box.getHeight() / ( float32 )( screenHeight );

   getline( file, street->m_fontName );
   file >> street->m_fontSize >> ws;
   getline( file, street->m_name );
   file >> street->m_streetWidth >> ws;

   while ( ! file.eof() ) {
      MC2Coordinate coord;
      file >> coord.lat >> coord.lon;
      if ( !coord.isValid() ) {
         continue;
      }

      if ( first ) {
         street->m_gfxData->addCoordinate( coord.lat, coord.lon,
                                           true ); // first coordinate
         first = false;
      } else {
         street->m_gfxData->addCoordinate( coord );
      }
   }

   return street;
}

/**
 * Creates a image widget containing the rendered image.
 * @param drawer Contains image information.
 * @return image widget.
 */
GtkWidget* createImageWindow( ImageDraw& drawer ) {
   GtkWidget* window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
   gtk_window_set_title( GTK_WINDOW( window ), "Text Placement" );

   // Fetch data from drawer and write it to temporary file which we
   // then load as image.
   uint32 imageSize = 0;
   ScopedArray<byte> imageData( drawer.getImageAsBuffer( imageSize ) );
   TempFile file( "image", "/tmp" );
   file.write( imageData.get(), imageSize );

   GtkWidget* image = gtk_image_new_from_file(file.getTempFilename().c_str());

   gtk_container_add( GTK_CONTAINER( window ), image );

   return window;
}

// This was not declared, but its implemented
ostream& operator << ( ostream& ostr, const GfxDataTypes::textPos& pos );

/**
 * Place text on a street and render it to an image.
 * @param drawer Renders street to an image.
 * @param street Render data.
 * @return true on success.
 */
bool placeText( CairoImageDraw& drawer, const StreetData& street ) {

   // get glyph dimensions
   vector< GfxDataTypes::dimensions > dimensions;
   if ( ! drawer.getGlyphDimensions( street.m_fontSize,
                                     street.m_fontName.c_str(),
                                     street.m_name.c_str(),
                                     dimensions,
                                     street.m_xFactor, street.m_yFactor ) ) {
      mc2dbg << "Failed to get glyph dimensions for font:"
             << street.m_fontName << endl;
      return false;
   }

   vector< GfxDataTypes::textPos > textOut;

   StreetTextPlacer streetTextPlacer;
   if ( ! streetTextPlacer.
        placeStreetText( street.m_gfxData->polyBegin(0),
                         street.m_gfxData->polyEnd(0),
                         dimensions,
                         textOut ) ) {
      mc2dbg << "Failed to place text!" << endl;
      return false;
   }

   // output text placement
   copy( textOut.begin(), textOut.end(),
         ostream_iterator< GfxDataTypes::textPos >( cout, "\n" ) );

   drawer.drawGfxData( *street.m_gfxData, street.m_streetWidth,
                       GDUtils::Color::RED );

   // draw the placed text to image buffer
   drawer.drawGfxFeatureText( street.m_name.c_str(),
                              textOut,
                              street.m_fontSize,
                              static_cast<GDUtils::Color::CoolColor>( 0 ),
                              street.m_fontName.c_str() );
   return true;
}

/**
 * Show a GUI with the rendered image.
 * @param drawer Contains the image to show
 * @param argc
 * @param argv
 */
void showGUI( ImageDraw& drawer, int argc, char** argv ) {

   // setup Gtk window to view the text placement in
   MC2Gtk::Application app( argc, argv,
                            0 ); // no startup flags

   GtkWidget* window = createImageWindow( drawer );
   MC2Gtk::SlotHandler slots;

   gtk_widget_show_all( window );

   slots.connect( G_OBJECT( window ),
                  "destroy",
                  Slot( app, &MC2Gtk::Application::end ) );
   app.loop();

}

int main( int argc, char** argv ) {

   if ( argc < 2 ) {
      mc2dbg << "Usage: " << argv[ 0 ] << " datafile" << endl;
      return 1;
   }

   auto_ptr<StreetData> street( createStreet( argv[ 1 ] ) );
   if ( street.get() == NULL ) {
      mc2dbg << "Failed to create street from data file:" << argv[ 1 ] << endl;
      return 2;
   }

   // setup a drawer
   CosLatProjection projection( street->m_box,
                                street->m_screenSize.getWidth(),
                                street->m_screenSize.getHeight() );
   CairoImageDraw drawer( street->m_screenSize.getWidth(),
                          street->m_screenSize.getHeight() );
   drawer.setDrawingProjection( &projection );

   if ( ! placeText( drawer, *street ) ) {
      return 3;
   }

   showGUI( drawer, argc, argv );

   return 0;
}
