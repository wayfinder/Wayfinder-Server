/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "EntryUtil.h"

#include "XMLUIParser.h"
#include "AutoPtr.h"

#include <sstream>

template <>
void AutoPtr<GList>::destroy() {
   if ( get() ) {
      g_list_free( get() );
      release();
   }
}

namespace MC2Gtk {
namespace XMLUI {

MC2String getEntryValue( const MC2String& id ) {  

   GtkWidget* widget = UIParser::getWidget( id );

   // if widget is box then find first entry in the box and use it 
   // as the value
   if ( GTK_IS_BOX( widget ) ) {
      AutoPtr<GList> 
         list( gtk_container_get_children( GTK_CONTAINER( widget ) ) );
      for (GList* child = list->next; child != NULL; child = child->next ) {
         
         if ( GTK_IS_ENTRY( child->data ) ) {
            return gtk_entry_get_text( GTK_ENTRY( child->data ) );
         }
      }

   } else if( GTK_IS_ENTRY( widget ) ) {
      return gtk_entry_get_text( GTK_ENTRY( widget ) );
   } else if ( GTK_IS_SPIN_BUTTON( widget ) ) {
      stringstream str;
      str << gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( widget ) );
      return str.str();
   }

   mc2log << warn << "Could not find entry with id \"" << id << "\"" << endl;
   return "";
}

bool setEntryValue( const MC2String& id, const MC2String& value ) try {
   gtk_entry_set_text( GTK_ENTRY( UIParser::getWidget( id ) ),
                       value.c_str() );
   return true;
} catch ( const Exception& e ) {
   return false;
}

}
}
