/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLRadioButton.h"

#include "MC2String.h"
#include "XMLTool.h"
#include "XMLUIParser.h"
#include <gtk/gtk.h>

namespace MC2Gtk {
namespace XMLUI {

GtkWidget* RadioButton::parse( const DOMElement* node ) {
   using namespace XMLTool;
   if ( XMLString::equals( node->getNodeName(), "radio_button" ) ) {
      MC2String label;
      XMLTool::getAttrib( label, "label", node, MC2String() );
      return gtk_radio_button_new_with_label( NULL, label.c_str() );
   }

   bool expand, fill;
   uint32 padding;
   bool vert = XMLString::equals( node->getNodeName(), 
                                  "radio_button_group_vert" );
   bool homogeneous = true;
   int spacing = 10;
   getAttrib( expand, "expand", node, true );
   getAttrib( fill, "fill", node, true );
   getAttrib( padding, "padding", node, (uint32)0 );
   getAttrib( homogeneous, "homogeneous", node, true );
   getAttrib( spacing, "spacing", node, 10 );

   GtkWidget* box = NULL;
   if ( vert ) {
      box = gtk_vbox_new( homogeneous, spacing );
   } else {
      box = gtk_hbox_new( homogeneous, spacing );
   }

   gtk_box_set_homogeneous( GTK_BOX( box ), homogeneous );
   
   UIParser::WidgetVector children = parseChildren( node );
   UIParser::WidgetVector::iterator it = children.begin();
   UIParser::WidgetVector::iterator itEnd = children.end();
   GSList* group = NULL;
   for ( ; it != itEnd; ++it ) {
      // get the first item as a group
      if ( group == NULL ) {
         group = gtk_radio_button_get_group( GTK_RADIO_BUTTON( *it ) );
      } else {
         gtk_radio_button_set_group( GTK_RADIO_BUTTON( *it ),
                                     group );
      }

      gtk_box_pack_start( GTK_BOX( box ), 
                          *it, expand, fill, padding );
   }

   return box;
}

}
}
