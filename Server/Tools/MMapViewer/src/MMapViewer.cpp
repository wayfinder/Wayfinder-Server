/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLUIParser.h"
#include "SlotHandler.h"
#include "EntryUtil.h"
#include "WidgetPtr.h"

#include "FixedSizeString.h"
#include "StringConvert.h"
#include "DeleteHelpers.h"
#include "CommandlineOptionHandler.h"
#include "Properties.h"
#include "URLFetcherNoSSL.h"
#include "TempFile.h"
#include "URL.h"
#include "DataBuffer.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <xercesc/util/PlatformUtils.hpp>

#include <memory>
#include <vector>

/// simple 3D point, using z value as zoom here
struct Point { 
   Point():m_x( 0 ), m_y( 0 ), m_z( 0 ) { }
   Point( int x, int y, int z ): m_x( x ), m_y( y ), m_z( z ) { }

   Point& operator += (const Point& other ) {
      m_x += other.m_x;
      m_y += other.m_y;
      m_z += other.m_z;
      return *this;
   }
   Point& operator -= (const Point& other ) {
      m_x -= other.m_x;
      m_y -= other.m_y;
      m_z -= other.m_z;
      return *this;
   }

   Point operator + ( const Point& other ) {
      Point p = *this;
      p += other;
      return p;
   }
   Point operator - ( const Point& other ) {
      Point p = *this;
      p -= other;
      return p;
   }
   bool operator == ( const Point& other ) {
      return 
         m_x == other.m_x &&
         m_y == other.m_y &&
         m_z == other.m_z; 
      
   }
   int m_x;
   int m_y;
   int m_z; // zoom
};

/// simple MMap viewer
class MMapViewer {
public:
   MMapViewer( const char* uiFile );



   void run() {
      gtk_main();
   }

private:
   void scroll( const Point& diff );


   void scrollLeft() {
      scroll( Point( -m_scrollDiff, 0, 0 ) );
   }
   void scrollRight() { 
      scroll( Point( m_scrollDiff, 0, 0 ) );
   }
   void scrollUp() {
      scroll( Point( 0, m_scrollDiff, 0 ) );
   }

   void scrollDown() {
      scroll( Point( 0, -m_scrollDiff, 0 ) );
   }

   void update();

   /// holds point + image at that point
   struct PointPixmap { 
      PointPixmap( const Point& p,
                   GdkPixbuf* pm ):
         m_pos( p ),
         m_pixmap( pm ) { 
      }

      ~PointPixmap() { 
         gdk_pixbuf_unref( m_pixmap );
      }
      Point m_pos;
      GdkPixbuf* m_pixmap;
   };

   typedef STLUtility::AutoContainer< list<PointPixmap*> > ImageCache;

   GdkPixbuf* getImage( const Point& pos );
   void drawMap();

   GdkPixbuf* findPixmap( const Point& p ) {
      ImageCache::iterator it = m_images.begin();
      ImageCache::iterator itEnd = m_images.end();
      for (; it != itEnd; ++it ) {
         if ( (*it)->m_pos == p ) {
            return (*it)->m_pixmap;
         }
      }
      return NULL;
   }



   void addPixmap( const Point& p, GdkPixbuf* pm ) {
      m_images.push_back( new PointPixmap( p, pm ) );
   }

   // removes any pixmap that is outside the rectangle 
   // of top and (bottom + m_scrollDiff )
   void updateCache( const Point& topLeft, 
                     const Point& bottomRight ) {
      ImageCache::iterator it = m_images.begin();
      ImageCache::iterator itEnd = m_images.end();
      for ( ; it != itEnd; ++it ) {
         const Point& pos = (*it)->m_pos;
         if ( pos.m_x < topLeft.m_x ||
              pos.m_y < topLeft.m_y ||
              pos.m_x > bottomRight.m_x + m_scrollDiff ||
              pos.m_y > bottomRight.m_y + m_scrollDiff ) {
            delete *it;
            m_images.erase( it++ );
         }
      }
   }

   MC2Gtk::SlotHandler m_slots; //< holder for actions
   Point m_currPosition; //< current top left viewing position
   const int m_scrollDiff; //< number of pixels to scroll

   ImageCache m_images; //< cache

   MC2Gtk::WidgetPtr m_image; //< image widget viewed by the user
};

MC2String operator + ( const MC2String& str, const Point& p ) {
   FixedSizeString buff( 64 );
   sprintf( buff, "x=%d&y=%d&zoom=%u", p.m_x, p.m_y, p.m_z );
   return str + buff.c_str();
}

using namespace MC2Gtk;
using namespace XMLUI;

int main( int argc, char** argv ) {

   try {
      XMLPlatformUtils::Initialize();
   } catch(const XMLException &toCatch) {
      cerr << "Error.\n"
           << "  Exception message:"
           << toCatch.getMessage() << endl;
      return 1;
   }
   

   gtk_init( &argc, &argv );

   CommandlineOptionHandler coh( argc, argv, 1 );
   coh.setTailHelp("uifile");
   coh.setSummary("Shows MMap images");

   if ( ! coh.parse() ) {
      cerr << argv[ 0 ] << ": Error on commandline! (Use -h for help) " <<endl;
      return -1;
   }

   if ( ! Properties::setPropertyFileName( coh.getPropertyFileName() ) ) {
      cerr << "No such file or directory: '"
           << coh.getPropertyFileName() << "'" << endl;

   }

   mc2log << warn << "DISCLAIMER: This is just a simple program for viewing MMap tiles." 
          << " Use at own risk. "  << endl << warn
          << "WARNING: Some items may appear eatable. Viewers discretion is advised." << endl;

   const char* userInterface = coh.getTail( 0 );
   try {

      MMapViewer viewer( userInterface );
      viewer.run();

      return 0;
   } catch (const XMLUI::Exception& e ) {
      mc2log << "Problem with xml user interface: " << e.what() << endl;
      mc2log << "Check the user interface file: " << userInterface << endl;
      return -1;
   }

   XMLPlatformUtils::Terminate();

   return 0;
}

MMapViewer::MMapViewer( const char* uiFile ):
   m_scrollDiff( 180 ) {
   UIParser::WidgetVector widgets = UIParser::parseFile( uiFile );
   for_each( widgets.begin(), widgets.end(),
             ptr_fun( gtk_widget_show_all ) );
   
   // setup actions

   m_slots.connect( G_OBJECT( UIParser::getWidget( "scroll_left" ) ),
                    "clicked", Slot( *this, &MMapViewer::scrollLeft ) );
   m_slots.connect( G_OBJECT( UIParser::getWidget( "scroll_right" ) ),
                    "clicked", Slot( *this, &MMapViewer::scrollRight ) );
   m_slots.connect( G_OBJECT( UIParser::getWidget( "scroll_up" ) ),
                    "clicked", Slot( *this, &MMapViewer::scrollUp ) );
   m_slots.connect( G_OBJECT( UIParser::getWidget( "scroll_down" ) ),
                    "clicked", Slot( *this, &MMapViewer::scrollDown ) );
   m_slots.connect( G_OBJECT( UIParser::getWidget( "update" ) ),
                    "clicked", Slot( *this, &MMapViewer::update ) );
   
   // assign from entry values
   update();
}


void MMapViewer::scroll( const Point& diff ) {
   m_currPosition += diff;

   // update gui
   GtkWidget* entry = UIParser::getWidget( "scroll_position_x" );
   FixedSizeString intValue( 64 );
   sprintf( intValue, "%d", m_currPosition.m_x );
   gtk_entry_set_text( GTK_ENTRY( entry ), intValue.c_str() );

   entry = UIParser::getWidget( "scroll_position_y" );
   sprintf( intValue, "%d", m_currPosition.m_y );
   gtk_entry_set_text( GTK_ENTRY( entry ), intValue.c_str() );

   entry = UIParser::getWidget( "zoom" );
   sprintf( intValue, "%d", m_currPosition.m_z );
   gtk_entry_set_text( GTK_ENTRY( entry ), intValue.c_str() );

   drawMap();
   
}
void MMapViewer::update() { 

   // assign curr position from entry values
   StringConvert::assign( m_currPosition.m_x, 
                          MC2Gtk::XMLUI::
                          getEntryValue( "scroll_position_x" ));
   
   StringConvert::assign( m_currPosition.m_y, 
                          MC2Gtk::XMLUI::
                          getEntryValue( "scroll_position_y" ) ); 
   
   StringConvert::assign( m_currPosition.m_z,
                          MC2Gtk::XMLUI::
                          getEntryValue( "zoom" ) ); 
   // redraw map
   drawMap();

 }

void MMapViewer::drawMap() {
   const uint32 mapSize = 3;
   GdkPixbuf* mainBuff = gdk_pixbuf_new( GDK_COLORSPACE_RGB, 
                                         true, 8,
                                         m_scrollDiff * mapSize,
                                         m_scrollDiff * mapSize );
   // draw entire mapSize x mapSize map
   // so for each tile get an image, unless its in cache
   for ( uint32 y = 0; y < mapSize; ++y ) {
      Point position = m_currPosition + Point( 0, y*m_scrollDiff, 0 );
      for ( uint32 x = 0; x < mapSize; ++x ) {
         
         position += Point( m_scrollDiff, 0, 0 );
         // cached tile?
         GdkPixbuf* buff = findPixmap( position );
         if ( buff == NULL ) {
            // no cache: get a new from the server
            buff = getImage( position );
            addPixmap( position, buff );
         }
         // got a valid pixmap?
         if ( buff == NULL ) {
            mc2log << "Failed to get image. " << endl;
            continue;
         }

         // copy pixmap to our image area
         int width = gdk_pixbuf_get_width( buff );
         int height = gdk_pixbuf_get_height( buff );
         gdk_pixbuf_copy_area( buff, 
                               0, 0,   
                               width, height,
                               mainBuff, 
                               x * width, 
                               (mapSize-1) * height - y * height );
        
      }
   }
   // remove unused tiles 
   mc2log << warn 
          << "Cache not cleared, memory will be exhausted....at some point." 
          << endl;

   m_image.reset( gtk_image_new_from_pixbuf( mainBuff ) );
   GtkWidget* frame = UIParser::getWidget( "image_frame" );
   gtk_container_add( GTK_CONTAINER( frame ), m_image.get() );
   gtk_widget_show_all( frame );
   
}  

GdkPixbuf* MMapViewer::getImage( const Point& pos ) {
   // compose url
   MC2String urlStr;
   urlStr = "http://" +
      getEntryValue( "server" ) + ":" +
      getEntryValue( "port" ) + "/MMap?" + pos +
      "&lang=en";

   mc2log << urlStr << endl;

   URLFetcherNoSSL fetcher;
   URLFetcherNoSSL::dbPair_t result = fetcher.get( URL( urlStr ) );
   if ( result.second == NULL ) { 
      mc2log << "result code: " << result.first << endl;
      return NULL;
   }

   auto_ptr<DataBuffer> buff( result.second );
   TempFile file( "image", "/tmp" );
   file.write( buff->getBufferAddress(), buff->getBufferSize() );

   GError* error = NULL;
   GdkPixbuf* pm = gdk_pixbuf_new_from_file( file.getTempFilename().c_str(), 
                                             &error );

   if ( pm == NULL ) {
      mc2log << error << "Image create error: " << error->message << endl;
   }

   return pm;
} 


