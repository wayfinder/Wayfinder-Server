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
#include "MEMessageDialog.h"
#include <gtkmm/frame.h>
#include <gtkmm/stock.h>
#include <gtkmm.h>
#include <iostream>

MEMessageDialog* MEMessageDialog::m_messageDialog = NULL;

MEMessageDialog::MEMessageDialog()
{
   set_modal(true);

   // Create all three buttons
   Gtk::Button* okButton = manage(new Gtk::Button("OK"));
   Gtk::Button* yesButton = manage(new Gtk::Button("Yes"));
   Gtk::Button* noButton = manage(new Gtk::Button("No"));

   sigc::mem_fun(*this,&MEMessageDialog::okClicked);

   okButton->signal_clicked().connect(sigc::mem_fun(*this,
						    &MEMessageDialog::okClicked));
   yesButton->signal_clicked().connect(sigc::mem_fun(*this,
						     &MEMessageDialog::yesClicked));
   noButton->signal_clicked().connect(sigc::mem_fun(*this,
						    &MEMessageDialog::noClicked));

   // Create the button-sets
   m_displayButtonSet = manage(new Gtk::HBox());
   m_displayButtonSet->pack_end(*okButton, false, true, 10);
   m_displayButtonSet->set_spacing(10);
   m_inquireButtonSet = manage(new Gtk::HBox());
   m_inquireButtonSet->pack_end(*noButton, false, true, 10);
   m_inquireButtonSet->pack_end(*yesButton, false, true, 10);

   // Create label
   m_label = manage(new Gtk::Label(""));
   m_label->set_justify(Gtk::JUSTIFY_LEFT);
   m_label->set_line_wrap(true);
   m_label->set_size_request(180,100);

   // Add the labels and button-sets to the window
   Gtk::Box* mainBox = manage(new Gtk::VBox());
   Gtk::Frame* frame = manage(new Gtk::Frame());
   frame->add(*m_label);
   mainBox->pack_start(*frame);
   mainBox->pack_start(*m_displayButtonSet, false, true, 10);
   mainBox->pack_start(*m_inquireButtonSet, false, true, 10);
   add(*mainBox);
   set_size_request(200,150);
   show_all();
}

void 
MEMessageDialog::okClicked()
{
   mc2dbg8 << "\"OK\"-button clicked" << endl;
   m_lastButtonClicked = BTN_OK;
   //Gtk::Kit::quit();
   Gtk::Main::quit();
}

void 
MEMessageDialog::yesClicked()
{
   mc2dbg8 << "\"Yes\"-button clicked" << endl;
   m_lastButtonClicked = BTN_YES;
   Gtk::Main::quit();
}

void
MEMessageDialog::noClicked()
{
   mc2dbg8 << "\"No\"-button clicked" << endl;
   m_lastButtonClicked = BTN_NO;
   Gtk::Main::quit();
}

void 
MEMessageDialog::hideAllButtons()
{
   m_displayButtonSet->hide();
   m_inquireButtonSet->hide();
}

gint 
MEMessageDialog::delete_event_impl(GdkEventAny*)
{
   mc2dbg8 << here << " delete_event_impl" << endl;
   return true;
}

void 
MEMessageDialog::center(gint x, gint y, gint w=-1, gint h=-1)
{
   gint dw,dh;
   if (!is_realized()) 
      realize();
   get_window()->get_size(dw, dh);
   if (w == -1) 
      w = x;
   if (h == -1) 
      h = y;
   
   int cx = x + w/2 - dw/2;
   int cy = y + h/2 - dh/2;

   set_position(Gtk::WIN_POS_NONE);
   move(cx, cy);
}

void 
MEMessageDialog::center()
{
   gint w = gdk_screen_width ();
   gint h = gdk_screen_height ();
   center(0,0,w,h);
}


void
MEMessageDialog::displayMessage_impl(const char* message)
{
   if (message != NULL) {
      m_label->set_text(message);
   } else {
      m_label->set_text("INTERNAL ERROR: No text...");
   }
   center();
   show_all();
   hideAllButtons();
   m_displayButtonSet->show();

   // Catch all events!
   Gtk::Main::run();

   hide_all();
}

bool
MEMessageDialog::inquireUser_impl(const char* message)
{
   if (message != NULL) {
      m_label->set_text(message);
   } else {
      m_label->set_text("INTERNAL ERROR: No text...");
   }
   center();
   show_all();
   hideAllButtons();
   m_inquireButtonSet->show();

   // Catch all events!
   Gtk::Main::run();
   hide_all();

   // Check last clicked button and return
   if (m_lastButtonClicked == BTN_YES) {
      return true;
   } else if (m_lastButtonClicked == BTN_NO) {
      return false;
   }
   mc2dbg << warn << "Unknown button klicked: m_lastButtonCLicked = "
          << int(m_lastButtonClicked) << endl;
   return false;
}


