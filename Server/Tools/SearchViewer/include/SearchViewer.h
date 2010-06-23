/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SEARCHVIEWER_H
#define SEARCHVIEWER_H

#include "config.h"
#include "Application.h"
#include "SlotHandler.h"
#include "WidgetPtr.h"


#include <memory>
#include <map>

class ResultTree;

namespace SearchViewer {
class SearchHandler;
class NavSearchHandler;
class XMLSearchHandler;

/**
 * Main application class for SearchViewer
 */
class SearchViewer: public MC2Gtk::Application {
public:

   SearchViewer( int argc, char** argv,
                 const char* xmlFilename );
   ~SearchViewer();

   void search();
   void run();

private:
   /// call back to switch to coordinate search
   void useCoordToggled();
   /// callback to change search handler
   void changeHandler();
   void updateResultView();
   void updateReplyView( const MC2String& data );
   
   MC2Gtk::SlotHandler m_slots;
   MC2Gtk::WidgetPtr m_resultBox;  

   GtkWidget* m_debugWindow;
   std::auto_ptr<ResultTree> m_treeView;
   std::auto_ptr<SearchHandler> m_xmlHandler;
   std::auto_ptr<SearchHandler> m_navHandler;
   SearchHandler* m_activeHandler;

};

}

#endif // SEARCHVIEWER_H
