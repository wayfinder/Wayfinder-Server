/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NGPMAKERGUI_H
#define NGPMAKERGUI_H

#include "Application.h"
#include "WidgetPtr.h"
#include "SlotHandler.h"
#include "../include/Request.h"

#include <memory>

namespace NGPMaker {

class RequestParser;
class Request;
class Param;

/**
 * Graphical User Interface for NGPMaker in Gtk+
 * Note: Still incomplete, mostly for viewing xml requests.
 *
 */
class MakerGUI: public MC2Gtk::Application {
public:

   MakerGUI( int argc, char** argv );
   ~MakerGUI();
   /**
    * Parses request description file and updates user interface
    */
   void parseRequestFile( const MC2String& filename );
   /// show gui
   void show();
   /// hide gui
   void hide();
private:
   /// save request to file
   void saveNGP();
   /// send request to server
   void sendNGP();
   /// update request values from user interface
   void updateRequestValues();

   /// called when request type has changed by combo box
   void changeRequest();
   /// setup gui for a set of requests
   void setupGUI( vector<Request>& requests );

   GtkWidget* createMainView();
   GtkWidget* createSendSaveView();
   GtkWidget* createSaveFrame();
   GtkWidget* createSendFrame();
   GtkWidget* createRequestView();
   GtkWidget* createRequestBox( vector<Request>& requests );
   GtkWidget* createRequestFrame( Request* request );
   GtkWidget* createParamFrame( vector<Param>& params );
   GtkWidget* createParamInput( Param& param );
   GtkWidget* createRequestComboBox( vector<Request>& requests );

   MC2Gtk::WidgetPtr m_window; ///< top level window for GUI
   MC2Gtk::WidgetPtr m_requestView;
   GtkWidget* m_requestCombo; ///< combo box with requests
   GtkWidget* m_requestBox; ///< contains request view
   GtkWidget* m_serverInput; ///< server entry input
   GtkWidget* m_portInput; ///< port entry input
   GtkWidget* m_filenameInput; ///< filename entry input
   MC2Gtk::SlotHandler m_slots; ///< signal slot handler
   /// parser for xml request description file
   auto_ptr<RequestParser> m_requestParser;
   Request m_currRequest; ///< current request in edit

   /// holds widget connection information to parameter
   class ParamEntry {
   public:
      ParamEntry( GtkWidget* entry,
                  GtkWidget* check,
                  Param* m_param );
      /// updates param from entry string value
      void updateParam();
      /// @return true if param should be used
      bool activated() const;
      /// @return string value from GUI that this param has
      MC2String getValue() const;
      void getParam( Param& param );
      
   private:
      GtkWidget* m_entryValue; ///< string value from gui
      GtkWidget* m_checkBox; ///< check box for option
      Param* m_param; ///< the parameter that 
   };
   vector<ParamEntry> m_entries; ///< param input entries
};

}

#endif
