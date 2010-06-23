/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLButton.h"
#include "XMLBox.h"
#include "XMLEntry.h"
#include "XMLSpinButton.h"
#include "XMLWindow.h"
#include "XMLFrame.h"
#include "XMLLabel.h"
#include "XMLInclude.h"
#include "XMLCheckButton.h"
#include "XMLRadioButton.h"

#include "XMLUIParser.h"

namespace MC2Gtk {
namespace XMLUI {

void UIParser::autoRegister() {
   // could do autoregister stuff in each instance
   // of parsers, but that does not work well with
   // static libraries. 
   mc2dbg << "register widgets."  << endl;

   registerParser( new Button(), "button" );
   registerParser( new Box("vbox"), "vbox" );
   registerParser( new Box("hbox"), "hbox" );
   registerParser( new Window(), "window" );
   registerParser( new SpinButton(), "spinbutton" );
   registerParser( new Include(), "include" );
   registerParser( new Frame(), "frame" );
   registerParser( new Label(), "label" );
   registerParser( new Entry(), "entry" );
   registerParser( new CheckButton(), "check_button" );
   registerParser( new RadioButton(), "radio_button_group_vert" );
   registerParser( new RadioButton(), "radio_button_group_horiz" );
   registerParser( new RadioButton(), "radio_button" );
}
}

}
