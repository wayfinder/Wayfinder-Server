/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NGPMakerGUI.h"

#include "../include/Request.h"
#include "RequestParser.h"

#include "Slot.h"
#include "STLStringUtility.h"
#include "TCPSocket.h"
#include "StringConvert.h"

#include <gtk/gtk.h>

using namespace MC2Gtk;

namespace NGPMaker {

GtkWidget* createMainView( );
GtkWidget* createRequestBox( const vector<Request>& requests );

gboolean deleteWindowEvent( GtkWidget *widget,
                            GdkEvent  *event,
                            gpointer   data ) {
    // Change TRUE to FALSE and the main window 
   // will be destroyed with a "delete_event". 
   return false;
}

MakerGUI::ParamEntry::
ParamEntry( GtkWidget* entry,
            GtkWidget* check,
            Param* param ):

   m_entryValue( entry ),
   m_checkBox( check ),
   m_param( param ) {

}

void MakerGUI::ParamEntry::updateParam() {
   if ( activated() ) {
      m_param->m_value = getValue();
   }
}

bool MakerGUI::ParamEntry::activated() const {
   return 
      gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( m_checkBox ) );
}

void MakerGUI::ParamEntry::getParam( Param& param ) {
   param = *m_param;
   param.m_value = getValue();
}

MC2String MakerGUI::ParamEntry::getValue() const {
   const gchar* str = gtk_entry_get_text( GTK_ENTRY( m_entryValue ) );
   return str ? str : "";
}

MakerGUI::MakerGUI( int argc, char** argv ):
   Application( argc, argv, Application::INIT_XML ),
   m_window( gtk_window_new( GTK_WINDOW_TOPLEVEL ) ),
   m_requestParser( new RequestParser() ) {

   gtk_window_set_title( GTK_WINDOW( m_window.get() ),
                         "NGPMaker - binary, for your pleasure." );

   gtk_container_add( GTK_CONTAINER( m_window.get() ),
                      createMainView( ) );

   // setup signals
   g_signal_connect( G_OBJECT( m_window.get() ),
                     "delete_event", 
                     G_CALLBACK( deleteWindowEvent ), NULL );


   m_slots.connect( G_OBJECT( m_window.get() ),
                    "destroy", 
                    Slot( *this, &MC2Gtk::Application::end ) );



}

MakerGUI::~MakerGUI() {
   // make sure we dont destory it, let the main
   //  window do that for us
   m_requestView.release( ); 

}

void MakerGUI::show() {
   // show 'em all!
   gtk_widget_show_all( m_window.get() );
}


void MakerGUI::hide() {
   gtk_widget_hide( m_window.get() );
}

void MakerGUI::parseRequestFile( const MC2String& filename ) {
   if ( m_requestParser->parse( filename ) ) {
      setupGUI( m_requestParser->getRequests() );
   } else {
      mc2log << "Parsing of xml file: \"" << filename 
             << "\" failed." << endl;
   }
}

void MakerGUI::setupGUI( vector<Request>& requests ) {
   m_requestView.reset( createRequestBox( requests ) );

   gtk_box_pack_start( GTK_BOX( m_requestBox ),
                       m_requestView.get(),
                       true, true, 10 );
   gtk_widget_show_all( m_requestBox );

}

void MakerGUI::changeRequest() {
   mc2dbg << "Switching request." << endl;
}

void MakerGUI::updateRequestValues() {
   mc2dbg << "Updating requests values from GUI." << endl;
   // setup the current request object 
   // copy everything from the "top" request.
   // 
   m_currRequest = m_requestParser->getRequests()[ 0 ];
   m_currRequest.m_params.clear();
   for ( uint32 i = 0; i < m_entries.size(); ++i ) {
      // copy "activiated" params 
      // ( i.e the params which the check box is checked)
      if ( m_entries[ i ].activated() ) {
         Param param;
         m_entries[ i ].getParam( param );
         m_currRequest.m_params.push_back( param );
      }
   }
}

void MakerGUI::sendNGP() try {
   updateRequestValues();

   MC2String server = gtk_entry_get_text( GTK_ENTRY( m_serverInput ) );
   MC2String portStr = gtk_entry_get_text( GTK_ENTRY( m_portInput ) );

   uint32 port = StringConvert::convert<uint32>( portStr );

   mc2dbg << "Send NGP to: " << server << ":" << port << endl;

   vector<byte> buffer;
   if ( ! saveNGPToBuffer( buffer, m_currRequest ) ) {
      mc2log << "Failed to save request to buffer!" << endl;
      return;
   }

   mc2dbg << "Will send " << buffer.size() << " bytes." << endl;

   // send to server using TCP
   TCPSocket sock;
   if ( ! sock.open() ) {
      mc2log << "Failed to open socket!" << endl;
      return;
   }

   if ( sock.connect( server.c_str(), port ) ) {
      ssize_t size = sock.writeAll( &buffer[ 0 ], buffer.size() );
      mc2log << "Sent " << size << " bytes." << endl;
   } else {
      mc2log << "Failed to connect to: " << server << ":" << port << endl;
   }

} catch ( const StringConvert::ConvertException& e ) {
   mc2log << error << "Send ngp bin failed!" << endl;
   mc2log << error << e.what() << endl;
}

void MakerGUI::saveNGP() {
   updateRequestValues();

   MC2String filename = gtk_entry_get_text( GTK_ENTRY( m_filenameInput ) );
   mc2dbg << "Save NGP to: " << filename << endl;
   if ( ! NGPMaker::saveNGP( filename, m_currRequest ) ) {
      mc2log << "Failed to save ngp request to: " << filename << endl;
   } else {
      mc2log << "Save succesful." << endl;
   }
}

GtkWidget* MakerGUI::createRequestComboBox( vector<Request>& requests ) {
   GtkWidget* combo = gtk_combo_new();
   GList* glist = NULL;
   for ( uint32 i = 0; i < requests.size(); ++i ) {
      glist = g_list_append( glist, const_cast<char*>
                             ( requests[ i ].m_name.c_str() ) );
   }

   gtk_combo_set_popdown_strings( GTK_COMBO( combo ), glist );   

   g_list_free( glist );

   return combo;
}

GtkWidget* MakerGUI::createParamInput( Param& param ) {
   // setup input widgets
   GtkWidget* descEntry = gtk_entry_new();
   gtk_entry_set_text( GTK_ENTRY( descEntry ),
                       param.m_desc.c_str() );
   
   GtkWidget* idEntry = gtk_entry_new();
   gtk_entry_set_text( GTK_ENTRY( idEntry ),
                       STLStringUtility::uint2str( param.m_id ).c_str() );

   GtkWidget* valueEntry = gtk_entry_new();
   gtk_entry_set_text( GTK_ENTRY( valueEntry ),
                       param.m_value.c_str() );

   GtkWidget* optionalCheck = gtk_check_button_new();
   // default active
   gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( optionalCheck ),
                                 true );

   
   // setup layout widget
   GtkWidget* box = gtk_hbox_new( false, // homogeneus
                                  0 ); // spacing

   
   m_entries.push_back( ParamEntry( valueEntry,
                                    optionalCheck,
                                    &param ) );
   // packing options
   bool expand = true;
   bool fill = false;
   uint32 padding = 2;
   
   gtk_box_pack_start( GTK_BOX( box ),
                       descEntry,
                       expand, fill, padding );
   gtk_box_pack_start( GTK_BOX( box ),
                       idEntry,
                       expand, fill, padding );
   gtk_box_pack_start( GTK_BOX( box ),
                       valueEntry,
                       expand, fill, padding );
   gtk_box_pack_start( GTK_BOX( box ),
                       optionalCheck,
                       expand, fill, padding );

   return box;
}


GtkWidget* MakerGUI::createParamFrame( vector<Param>& params ) {
   GtkWidget* frame = gtk_frame_new( "Params" );
   
   GtkWidget* box = gtk_vbox_new( false, // homogeneus
                                  0 ); // spacing
   gtk_container_add( GTK_CONTAINER( frame ), box );
   // packing options
   bool expand = true;
   bool fill = false;
   uint32 padding = 2;
   for ( uint32 i = 0; i < params.size(); ++i ) {
      gtk_box_pack_start( GTK_BOX( box ),
                          createParamInput( params[ i ] ),
                          expand, fill, padding );
   }

   return frame;
}

GtkWidget* MakerGUI::createRequestFrame( Request* request ) {
   GtkWidget* frame = gtk_frame_new( "Request" );

   GtkWidget* box = gtk_vbox_new( false, // homogeneus
                                  0 ); // spacing

   // packing options
   bool expand = true;
   bool fill = false;
   uint32 padding = 2;
   vector<Param> emptyParams;
   gtk_box_pack_start( GTK_BOX( box ),
                       createParamFrame( request ? request->m_params : 
                                         emptyParams ),
                       expand, fill, padding );


   gtk_container_add( GTK_CONTAINER( frame ), box );

   // TODO
   return frame;
}

GtkWidget* MakerGUI::createRequestBox( vector<Request>& requests ) {
   GtkWidget* requestBox = gtk_vbox_new( false, // homogeneus
                                         0 ); // spacing
   // packing options
   bool expand = true;
   bool fill = true;
   uint32 padding = 10;

   m_requestCombo = createRequestComboBox( requests );
                    
   gtk_box_pack_start( GTK_BOX( requestBox ),
                       m_requestCombo,
                       expand, fill, padding );

   gtk_box_pack_start( GTK_BOX( requestBox ),
                       createRequestFrame( requests.empty() ? 
                                           NULL : &requests[ 0 ] ),
                       expand, fill, padding );

   return requestBox;
}

GtkWidget* MakerGUI::createRequestView() {
   GtkWidget* requestBox = gtk_vbox_new( false, // homogeneus
                                         0 ); // spacing

   vector<Request> emptyRequest;
   m_requestView.reset( createRequestBox( emptyRequest ) );

   gtk_box_pack_start( GTK_BOX( requestBox ),
                       m_requestView.get(),
                       true, true, 10 );
   return requestBox;
}

GtkWidget* MakerGUI::createSendFrame() {
   // setup input widgets   
   GtkWidget* serverEntry = gtk_entry_new();
   gtk_entry_set_text( GTK_ENTRY( serverEntry ), "server" );
   m_serverInput = serverEntry;

   GtkWidget* portEntry = gtk_entry_new();
   gtk_entry_set_text( GTK_ENTRY( portEntry ), "port" );
   m_portInput = portEntry;

   GtkWidget* sendButton = gtk_button_new_with_label( "Send" );
   m_slots.connect( G_OBJECT( sendButton ),
                    "clicked",
                    Slot( *this, &MakerGUI::sendNGP ) );

   // setup layout widgets
   GtkWidget* frame = gtk_frame_new( "Send" );
   GtkWidget* box = gtk_hbox_new( false, // homogeneus
                                  10 ); // spacing
   // packing options
   bool expand = true;
   bool fill = true;
   uint32 padding = 0;
   
   gtk_box_pack_start( GTK_BOX( box ),
                       serverEntry,
                       expand, fill, padding );
   gtk_box_pack_start( GTK_BOX( box ),
                       portEntry,
                       expand, fill, padding );

   gtk_box_pack_start( GTK_BOX( box ),
                       sendButton,
                       expand, fill, padding );

   gtk_container_add( GTK_CONTAINER( frame ), box );

   return frame;
}

GtkWidget* MakerGUI::createSaveFrame() {

   // setup input widgets
   
   GtkWidget* filenameEntry = gtk_entry_new();
   gtk_entry_set_text( GTK_ENTRY( filenameEntry ), "out.bin" );
   m_filenameInput = filenameEntry;

   GtkWidget* saveButton = gtk_button_new_with_label( "Save" );

   m_slots.connect( G_OBJECT( saveButton ),
                    "clicked",
                    Slot( *this, &MakerGUI::saveNGP ) );

                    
   // setup layout widgets
   GtkWidget* box = gtk_vbox_new( false, // homogeneus
                                  10 ); // spacing
   bool expand = true;
   bool fill = true;
   uint32 padding = 0;
   
   gtk_box_pack_start( GTK_BOX( box ),
                       filenameEntry,
                       expand, fill, padding );
   gtk_box_pack_start( GTK_BOX( box ),
                       saveButton,
                       expand, fill, padding );

   GtkWidget* frame = gtk_frame_new( "Save" );
   gtk_container_add( GTK_CONTAINER( frame ), box );
   return frame;
}

GtkWidget* MakerGUI::createSendSaveView() {
   GtkWidget* box = gtk_vbox_new( false, // homogeneus
                                  0 ); // spacing
   // packing options
   bool expand = true;
   bool fill = true;
   uint32 padding = 0;
   
   gtk_box_pack_start( GTK_BOX( box ),
                       createSendFrame(),
                       expand, fill, padding );
   
   gtk_box_pack_start( GTK_BOX( box ),
                       createSaveFrame(),
                       expand, fill, padding );
   return box;
   
}

GtkWidget* MakerGUI::createMainView( ) {
   GtkWidget* mainBox = gtk_hbox_new( false, // homogeneous
                                      10 ); // spacing
   // packing options
   bool expand = true;
   bool fill = true;
   uint32 padding = 0;
   m_requestBox = createRequestView();
   gtk_box_pack_start( GTK_BOX( mainBox ),
                       m_requestBox,
                       expand, fill, padding );
   
   gtk_box_pack_start( GTK_BOX( mainBox ),
                       createSendSaveView(),
                       expand, fill, padding );

   return mainBox;
}

}
