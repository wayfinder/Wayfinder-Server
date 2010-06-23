/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef METURNRESTRICTIONSDIALOG_H
#define METURNRESTRICTIONSDIALOG_H

#include "config.h"
#include "OldConnection.h"
#include <gtkmm/dialog.h>
#include <gtkmm/togglebutton.h>
#include "MERouteableItemInfoWidget.h"

/**
 *
 */
class METurnRestrictionsDialog : public Gtk::Dialog {
   public:
      /**
       *    Get the one and only instance of this class. 
       */
      static METurnRestrictionsDialog* instance();

      /**
       *    Free the memory of the one instance of this class.
       */
      static void deleteInstance();

      /**
       *    Delete this dialog.
       */
      virtual ~METurnRestrictionsDialog();

      /**
       *    Show the dialog.
       */
      void show(uint32 restr,
                MEConnectionInfoBox* caller);

   protected:
      /**
       *    Constructor declaired private to make sure not called from
       *    outside.
       */
      METurnRestrictionsDialog();

      /**
       *    Static member to hold the one and only instance of this class.
       */
      static METurnRestrictionsDialog* _staticInstance;

      /**
       *    Called when the window is desroyed.
       */
      virtual gint delete_event_impl(GdkEventAny* p0);

      /**
       *    Method that is called when the cancel-button is pressed. This
       *    will cancel all changes that are done to the settings.
       */
      void cancelPressed();

      /**
       *    Method that is called when the ok-button is pressed. This
       *    will commit all changes that are done to the settings.
       */
      void okPressed();

      /**
       *    Array with buttons used to set which vehicles that are allowed.
       */
      Gtk::ToggleButton* m_vehicles[32];

      /**
       *    The number of supported vehicle types.
       */
      static const uint32 m_nbrTypes = 21;

      /**
       *    The value that is given to the show-method.
       */
      uint32 m_originalVal;

      /**
       *    The currently displayed connection.
       */
      OldConnection* m_connection;

      /**
       *    A pointer back to the caller to be able to save the vehicle 
       *    restrictions when OK is clicked.
       */
      MEConnectionInfoBox* m_caller;

      /**
       *    An an entry-box where to write the value for quick edit 
       *    of the restriction value.
       */
      Gtk::Entry* m_restrictionVal;
};

#endif

