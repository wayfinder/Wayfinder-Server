/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLParser.h"
#include "XMLTool.h"

namespace MC2Gtk {
namespace XMLUI {

class TextField: public Parser {
public:
   TextField( ) {
      //      UIParser::registerParser( this, "textfield");
   }

   GtkWidget* parse( const DOMElement* node );

};

//TextField AutoRegTextField;

GtkWidget* TextField::parse( const DOMElement* node ) {
   MC2String empty;

   MC2String labelStr, valueStr, name;

   XMLTool::getAttrib( labelStr, "label", node, empty );
   XMLTool::getAttrib( valueStr, "value", node, empty );
   XMLTool::getAttrib( name, "name", node, empty );


   GtkWidget* label = gtk_label_new( labelStr.c_str() );
   GtkWidget* entry = gtk_entry_new();
   gtk_entry_set_text( GTK_ENTRY( entry ), valueStr.c_str() );

   GtkWidget* box = gtk_hbox_new( false, 10 );
   gtk_box_pack_start_defaults( GTK_BOX( box ), label );
   gtk_box_pack_start_defaults( GTK_BOX( box ), entry );


   return box;
}

} // XMLUI

} // MC2Gtk
