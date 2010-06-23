/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MEMESSAGEDIALOG_H
#define MEMESSAGEDIALOG_H

#include "config.h"
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/main.h>
#include <gtkmm.h>


/**
 *
 *
 */
class MEMessageDialog: public Gtk::Window {
   public:
      static void displayMessage(const char* message) {
         if (m_messageDialog == NULL) {
            m_messageDialog = new MEMessageDialog();
         }
         return m_messageDialog->displayMessage_impl(message);
      }
      
      static bool inquireUser(const char* message) {
         if (m_messageDialog == NULL) {
            m_messageDialog = new MEMessageDialog();
         }
         return m_messageDialog->inquireUser_impl(message);
      }
      
   private:
      /**
       *    Show a message for the user. Only one button, "OK".
       *    @param message The text that will be presented in the dialog.
       */
      void displayMessage_impl(const char* message);

      /**
       *    Show a text for the user. Two buttons; "Yes" and "No". True 
       *    will be returned if the user clicked Yes, false otherwise.
       *    @param message The text that will be presented in the dialog.
       *    @return True if the user clicked "Yes", false if he clicked 
       *            "No".
       */
      bool inquireUser_impl(const char* message);

      enum button_t {
         BTN_OK,
         BTN_YES,
         BTN_NO };
      
      MEMessageDialog();

      Gtk::Box* m_displayButtonSet;
      Gtk::Box* m_inquireButtonSet;

      Gtk::Label* m_label;
      button_t m_lastButtonClicked;

      void okClicked();
      void yesClicked();
      void noClicked();


      void center();
      void center(gint x, gint y, gint w, gint h);
   
      void hideAllButtons();
      gint delete_event_impl(GdkEventAny*);

      static MEMessageDialog* m_messageDialog;
};

#endif

