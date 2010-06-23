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

#include "SearchViewer.h"

#include "SlotHandler.h"
#include "XMLUIParser.h"
#include "WidgetPtr.h"
#include "EntryUtil.h"
#include "Application.h"

#include "AutoPtr.h"
#include "DataBuffer.h"
#include "URL.h"
#include "URLFetcher.h"
#include "TempFile.h"
#include "FixedSizeString.h"
#include "GfxUtility.h"
#include "MC2Coordinate.h"

#include "CommandlineOptionHandler.h"
#include "Properties.h"
#include "XMLInit.h"
#include "ResultTree.h"

#include "XMLSearchHandler.h"
#include "NavSearchHandler.h"

#include "Auth.h"

#include "StringUtility.h"
#include "UTF8Util.h"
#include "IPnPort.h"
#include "NetUtility.h"
#include "GunzipUtil.h"
#include "FileException.h"
#include "GfxUtility.h"

#include <boost/lexical_cast.hpp>

#include <gtk/gtk.h>

#include <iostream>
#include <algorithm>
#include <memory>
#include <map>
#include <sstream>

#include <math.h>

using namespace MC2Gtk;
using namespace XMLUI;


int main( int argc, char** argv ) try {

   CommandlineOptionHandler coh( argc, argv, 0 );
   coh.setTailHelp("guifile");
   coh.setSummary("Sends search request and parses the result. Requires a"
                  " button with id=\"send_button\" in the gui file.");

   if ( ! coh.parse() ) {
      cerr << argv[ 0 ] << ": Error on commandline! (Use -h for help) " <<endl;
      return -1;
   }

   if ( ! Properties::setPropertyFileName( coh.getPropertyFileName() ) ) {
      cerr << "No such file or directory: '"
           << coh.getPropertyFileName() << "'" << endl;

   }

   SearchViewer::
      SearchViewer app( argc, argv,
                        coh.getTail( 0 ) );
   
   app.run();

   return 0;

} catch ( const std::exception& e ) {
   mc2log << fatal << e.what() << endl;
   return -1;
} catch ( ... ) {
   mc2log << fatal << "Unknown exception!" << endl;
   return -1;
}


MC2String unpackUIData( const MC2String& base64data ) {
   vector<unsigned char> gzdata( base64data.size() );
   // decode
   int strLen = StringUtility::base64Decode( base64data.c_str(), 
                                             reinterpret_cast<byte*>
                                             ( &gzdata[0] ) );
   if ( strLen < 0 ) {
      mc2dbg << error << "Failed to decode data" << endl;
      return "";
   }
   gzdata.resize( strLen );

   // unzip
   vector<unsigned char> 
      unzipedData( GunzipUtil::origLength( &gzdata[0], gzdata.size() ) );

   if ( GunzipUtil::
        gunzip( &unzipedData[0], unzipedData.size(),
                &gzdata[0], gzdata.size() ) < 0 ) {
      mc2dbg << error << "Failed to unzip data." << endl;
      return "";
   }
   return reinterpret_cast<char*>( &unzipedData[0] );
}

namespace SearchViewer {

/// @return gtk image 
GtkWidget* getImage( const MC2String& server,
                     const MC2String& port,
                     const MC2String& imageFile ) {
   // using tmap method
   URL url( MC2String("http://") + server + ":" + port + "/TMap/B" +
            imageFile + ".png" );
   URLFetcherNoSSL fetcher;

   URLFetcherNoSSL::dbPair_t result = fetcher.get( url );
   if ( result.second == NULL ) {
      mc2log << "result code: " << result.first << endl;
      return NULL;
   }

   auto_ptr<DataBuffer> buff( result.second );
   TempFile file( "image", "/tmp" );
   file.write( buff->getBufferAddress(), buff->getBufferSize() );

   return gtk_image_new_from_file( file.getTempFilename().c_str() );
}

auto_ptr<TempFile> getImageFile( const MC2String& imageIn ) {
   MC2String image = StringUtility::trimStartEnd( imageIn );
   if ( image.empty() ) {
      return auto_ptr<TempFile>();
   }

   MC2String server = MC2Gtk::XMLUI::getEntryValue("server");
   MC2String port = MC2Gtk::XMLUI::getEntryValue("port");
   // using tmap method
   URL url( MC2String("http://") + server + ":" + port + "/TMap/B" +
            image + ".png" );
   URLFetcherNoSSL fetcher;
   
   URLFetcherNoSSL::dbPair_t result = fetcher.get( url );
   
   auto_ptr<DataBuffer> buff( result.second );
   if ( result.second == NULL ) {
      return auto_ptr<TempFile>();
   }
   TempFile* file = new TempFile( "image", "/tmp" );
   file->write( buff->getBufferAddress(), buff->getBufferSize() );
   return auto_ptr<TempFile>( file );
}
                                 
gboolean deleteWindowEvent( GtkWidget* widget,
                            GdkEvent* event,
                            gpointer data ) {
    // Change TRUE to FALSE and the main window 
   // will be destroyed with a "delete_event". 
   return false;
}


SearchViewer::SearchViewer( int argc, char** argv,
                            const char* uiFilename ):
   Application( argc, argv, MC2Gtk::Application::INIT_XML ),
   m_debugWindow( NULL ),
   m_xmlHandler( new XMLSearchHandler() ),
   m_navHandler( new NavSearchHandler() ),
   m_activeHandler( m_xmlHandler.get() ) {
try {
   // base64 gzip data of xml ui interface
   const char UIdata[] =
      "H4sIAAAAAAAAA61XO5PbIBDu/SsIVVLc+HGOL+exr0mVmSRNmnQaLK0lchg0gCQnvz4Leli20N"
      "1FE1dov4VvX+zi3Slepfb5aTbbVVwmqiKWWwF7+gOYjjNKeLKnVuWRgBJEVOtQ8jTbZQd17lBc"
      "UwLnnEkn0AVQkrMk4TLd0+WCkiMXogFwa+m21qIjEwYuW5vPbu8C1WeE7I6anYAIdgDhTNMlaI"
      "IbSGFwYcBa1Daoi6qDw700C0lR7o8kFs52T40/d0vJvAFBWv3b+1hDlJRMFBicKlN5Dvpu1eju"
      "5u78NxPlStsgjQM6kuVqtVxOI3BhkRixIEkLRhpywWLoCHme5Q6c6BQzplI6CTvWgANOk3/5pQ"
      "QTE/3kMuwilwOmxQ2FX5Yt29xXWC1tTcClZglX0aGwVsko1arIo0xp/ue6emvlW31vyfkkIi+k"
      "bfX+/Pa1Mzm4RbLyZst3VvKUWaUvvs7HLJsQRcFkWrA0XC0tOMxcBQk32bTEGctGroBHXk3eG2"
      "lAhssR5QOK+2kUJ3YmGbcmyINg5MAh2USHMmCuMQbJGmzAdTexiWBNXYXP5Fz2ytTDF7ITlz5N"
      "6LKPpbGQYw+bRo0jhWhIuQpfbzdxanjg7ETCmFlIFR5v/NgLsrY6Ua0zoG6awaTr4E8kCZiYxD"
      "oO0jswQnDAG+hrN+PSH37kIBKzJU27Cs/Jcfm4J6TnjjO0ypiN/Cdt3HOSbb/pXTnm9W+dqnhP"
      "/xLNt9rR8oKGl4gRHjALLOv/wN1Wyzh9V08jCR2wN52/vIyn0Vy9YmicQfzcnzk49qNY4YDuZs"
      "5n98Ul2rilIw6IQN42m/XD+tN6/TDqtgjd2o8P95vN4vHxajJeRf4fQo8TS7yQdg+/FPTpzDgQ"
      "LZMxkPfkBBafph/GzWh1h8FY4G/MnF76R14vvawaN+fq7y6vTvaubRll+4bXYAphu+f9deeoQU"
      "OHr+rGglm7aC3dzeuTPNT+w/gL+UTME2sMAAA=";

  {// setup system
     UIParser::WidgetVector windows;
     if ( uiFilename == NULL || strlen( uiFilename ) == 0 ) {
        windows = UIParser::parseFromData( unpackUIData( UIdata ) );
     } else {
        windows = UIParser::parseFile( uiFilename );
     }

     for_each( windows.begin(), windows.end(),
               ptr_fun( gtk_widget_show_all ) );
  }
  m_slots.connect( G_OBJECT( XMLUI::UIParser::getWidget( "send_button" ) ),
                   "clicked", Slot( *this, &SearchViewer::search ) );
  m_slots.connect( G_OBJECT( XMLUI::UIParser::getWidget( "top_level_window" ) ),
                   "destroy", 
                   Slot( *this, &MC2Gtk::Application::end ) );
  m_slots.connect( G_OBJECT( XMLUI::UIParser::getWidget( "use_coord" ) ),
                   "toggled", 
                   Slot( *this, &SearchViewer::useCoordToggled ) );
  m_slots.connect( G_OBJECT( XMLUI::UIParser::getWidget( "xml_radio" ) ),
                   "toggled",
                   Slot( *this, &SearchViewer::changeHandler ) );
  m_slots.connect( G_OBJECT( XMLUI::UIParser::getWidget( "nav_radio" ) ),
                   "toggled",
                   Slot( *this, &SearchViewer::changeHandler ) );
  
  g_signal_connect( G_OBJECT( XMLUI::UIParser::getWidget( "top_level_window" ) ),
                    "delete_event", G_CALLBACK( deleteWindowEvent ), NULL );
  

  updateReplyView("xml reply view");
} catch ( const MC2Gtk::XMLUI::Exception& e ) {
   mc2dbg << "SearchViewer: " << e.what() << endl;
   throw MC2Gtk::XMLUI::Exception( MC2String("SearchViewer:" ) +
                                   e.what() );
}
}

SearchViewer::~SearchViewer() {
   m_resultBox.release();
}

void SearchViewer::run() {
   loop();
}

/// @return true if checkbox is checked else false
bool isChecked( const char* checkboxName ) {
   GtkWidget* checkbox = MC2Gtk::XMLUI::UIParser::findWidget( checkboxName );
   if ( checkbox == NULL ) {
      mc2dbg << warn << "Did not find checkbox with name: " << checkboxName << endl;
      return false;
   }
   return gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( checkbox ) );
}

void SearchViewer::useCoordToggled() {
   using namespace MC2Gtk::XMLUI;
   GtkEntry* lat = GTK_ENTRY( UIParser::getWidget( "lat_replace" ) );
   GtkEntry* lon = GTK_ENTRY( UIParser::getWidget( "lon_replace" ) );
   GtkEntry* where = GTK_ENTRY( UIParser::getWidget( "where_replace" ) );
   GtkEntry* topRegion = GTK_ENTRY( UIParser::getWidget("top_region_replace"));
   if ( isChecked( "use_coord" ) ) {
      gtk_entry_set_editable( lat, true );
      gtk_entry_set_editable( lon, true );
      gtk_entry_set_editable( topRegion, false );
      gtk_entry_set_editable( where, false );
   } else {
      gtk_entry_set_editable( lat, false );
      gtk_entry_set_editable( lon, false );
      gtk_entry_set_editable( topRegion, true );
      gtk_entry_set_editable( where, true );
   }
}

void SearchViewer::changeHandler() {
   if ( isChecked( "nav_radio" ) ) {
      m_activeHandler = m_navHandler.get();
   } else {
      m_activeHandler = m_xmlHandler.get();
   }
}

template < typename T >
void getEntryValue( T& lhs, const char* entryName ) try {
   lhs = StringConvert::convert<T>
      ( gtk_entry_get_text( GTK_ENTRY( UIParser::
                                       getWidget( entryName) ) ) );

} catch ( const StringConvert::ConvertException& e ) {
   mc2dbg << "While parsing entry name: " << entryName << " > " 
          << e.what() << endl;
}

void SearchViewer::search() try {
   Auth auth( MC2Gtk::XMLUI::getEntryValue("username_replace"), 
              MC2Gtk::XMLUI::getEntryValue("password_replace") );
   // setup search params
   CompactSearch params;

   getEntryValue( params.m_what, "what_replace" );
   getEntryValue( params.m_categoryName, "category_replace" );

   getEntryValue( params.m_startIndex, "start_replace" );
   getEntryValue( params.m_endIndex, "end_replace" );
   getEntryValue( params.m_maxHits, "max_hits_replace" );
   getEntryValue( params.m_heading, "heading_replace" );
   getEntryValue( params.m_round, "round_replace" );

   if ( isChecked( "use_coord" ) ) {
      getEntryValue( params.m_location.m_coord.lat, "lat_replace" );
      getEntryValue( params.m_location.m_coord.lon, "lon_replace" );
      getEntryValue( params.m_distance, "distance_replace" );
   } else {
      getEntryValue( params.m_where, "where_replace" );
      getEntryValue( params.m_topRegionID, "top_region_replace" );
   }

   getEntryValue( params.m_language, "language_replace" );
   MC2String crc;
   getEntryValue( crc, "desc_crc_replace" );
   // setup destination ip and port 
   MC2String addr = MC2Gtk::XMLUI::getEntryValue("server");
   in_addr sin_addr;
   if ( ! NetUtility::gethostbyname( addr.c_str(), sin_addr ) ) {
      mc2dbg << warn << "Failed to lookup: " << addr << endl;
   }
   int32 port;
   getEntryValue( port, "port" );
   // lets search.
   m_activeHandler->search( IPnPort( ntohl( sin_addr.s_addr ), port ), 
                             auth, crc, params );
   updateResultView();

} catch ( const FileUtils::FileException& e ) {
   mc2log << error << e.what() << endl;
} catch ( const XMLUI::Exception& e ) {
   mc2log << error 
          << "Seems to be a problem with user interface xml" << endl;
   mc2log << "Check the user interface xml file. " << endl;
   mc2log << e.what() << endl;
}

void addResultItem( ResultTree& tree, const ItemMatch& item ) {
   auto_ptr<TempFile> tmpFile( getImageFile( item.m_image ) );

   MC2String itemName = item.m_name + ", " + item.m_locationName;
   // add distance to name if we did a proximity search

   if ( isChecked( "use_coord" ) ) {

      MC2Coordinate coord;
      getEntryValue( coord.lat, "lat_replace" );
      getEntryValue( coord.lon, "lon_replace" );
      MC2Coordinate itemCoord( item.m_lat, item.m_lon );

      try {
         itemName += MC2String("(") + boost::lexical_cast< MC2String >
         ( (uint32)sqrt( GfxUtility::
                         squareP2Pdistance_linear( coord, itemCoord )));

         itemName += ")";
      } catch (...) {
      }
   }

   mc2dbg << "Adding item: " << itemName << endl;
   tree.addResult( itemName,
                   tmpFile.get() ? tmpFile->getTempFilename() : "" );
}

void addResults( ResultTree& tree, const SearchMatch::ItemMatches& items ) {
   for ( uint32 i = 0; i< items.size(); ++i ) {
      addResultItem( tree, items[ i ] );
   }
}

void SearchViewer::updateResultView() {
   GtkWidget* resultWindow = UIParser::getWidget( "result_window" );
   m_treeView.reset( NULL );
   m_resultBox.reset( gtk_hbox_new( true, 10 ) );      

   gtk_box_pack_start_defaults( GTK_BOX( resultWindow ),
                                m_resultBox.get() );

   m_treeView.reset( new ResultTree() );

   gtk_box_pack_start_defaults( GTK_BOX( m_resultBox.get() ),
                                m_treeView->getWidget() );
   // update crc entry
   const Headings& headings = m_activeHandler->getMatches();
   GtkWidget* crcEntry = UIParser::findWidget( "desc_crc_replace" );
   if ( crcEntry != NULL ) {
      mc2dbg << "New crc: " << headings.m_crc << endl;
      gtk_entry_set_text( GTK_ENTRY( crcEntry ), headings.m_crc.c_str() );
   }
   mc2dbg << "headings.m_resultHeadings.size() = " 
          << headings.m_resultHeadings.size() << endl;
   // add headings with results to tree
   Headings::ResultHeadings::const_iterator it = headings.m_resultHeadings.begin();
   for ( ; it != headings.m_resultHeadings.end(); ++it ) {
      const DescMatch& desc = (*it);
      // setup label for tree root
      stringstream str;
      str << desc.m_name << " " << (int)desc.m_matches.m_startIndex 
          << "-" << desc.m_matches.m_endIndex 
          << " ( " << desc.m_matches.m_totalNbrItems << " )" << endl;
      auto_ptr<TempFile> imageFile( getImageFile( desc.m_image ) );

      m_treeView->addHeading( str.str(), imageFile->getTempFilename() );
      addResults( *m_treeView, desc.m_matches.m_items );
   }

   MC2String replyStr;
   m_activeHandler->getReplyDebugData( replyStr, 80 );
   updateReplyView( replyStr );
}


void applyColorToTags( GtkTextBuffer* buffer ) {

   GtkTextTag* normalTag = 
      gtk_text_buffer_create_tag( buffer, "tags",
                                  "foreground", "blue", 
                                  NULL );

   char tags[][64] = {
      "search_hit_list",
      "<search_item ",
      "</search_item>",
      "compact_search_reply",
      "name>",
      "image_name",
      "lat",
      "lon",
      "location_name",
      "search_area ",
      "</search_area>",
      "itemid",
      "areaid",
      "search_hit_type",
      "isab-mc2",
   };
   const uint32 numberOfTags = sizeof( tags ) / sizeof( tags[ 0 ] );

   for ( uint32 i = 0; i < numberOfTags; ++i ) {
      GtkTextIter start, end;
      gtk_text_buffer_get_start_iter( buffer, &start );
      while ( gtk_text_iter_forward_search( &start,
                                            tags[ i ],
                                            GTK_TEXT_SEARCH_TEXT_ONLY,
                                            &start,
                                            &end,
                                            NULL ) ) {
         gtk_text_buffer_apply_tag( buffer, normalTag, &start, &end );
         start = end;
      }
   }


}
                       
void SearchViewer::updateReplyView( const MC2String& data ) {

   GtkWidget* scrolled = gtk_scrolled_window_new( NULL, NULL );

   GtkWidget* view = gtk_text_view_new();

   gtk_text_view_set_editable( GTK_TEXT_VIEW( view ), false );

   GtkTextBuffer* buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( view ) );

   gtk_text_buffer_set_text( buffer, UTF8Util::isoToUtf8( data ).c_str(), -1 );

   applyColorToTags( buffer );

   gtk_container_add( GTK_CONTAINER( scrolled ), view );
   GtkWidget* topBox = XMLUI::UIParser::getWidget( "top_box" );

   gtk_widget_set_size_request( scrolled, 500, -1 );

   gtk_box_pack_start_defaults( GTK_BOX( topBox ), scrolled );
   //   gtk_box_pack_start_defaults( GTK_BOX( m_resultBox.get() ), scrolled );

   gtk_widget_show_all( topBox );//m_resultBox.get() );
   if ( m_debugWindow ) {
      gtk_widget_destroy( m_debugWindow );
   }

   m_debugWindow = scrolled;
   
}

}
