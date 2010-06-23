/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MEDRAWSETTINGS_H
#define MEDRAWSETTINGS_H

#include "config.h"
#include "MEMapArea.h"
#include "MEItemLayer.h"
#include <gtkmm/dialog.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/entry.h>

/**
 *    Handle which item-layers that are shown and wich are not.
 *    Also have options to affect the drawing of the items in the
 *    layers.
 *
 */
class MEDrawSettingsDialog : public Gtk::Dialog {
   public:
      /**
       *    Get the one and only instance of this class. 
       */
      static MEDrawSettingsDialog* instance(MEMapArea* mapArea);

      /**
       *    Free the memory of the one instance of this class.
       */
      static void deleteInstance();

      /**
       *    Show the dialog. Get the current draw settings from the 
       *    map area.
       */
      void show();

   protected:
      /**
       *    Constructor declaired private to make sure not called from
       *    outside.
       */
      MEDrawSettingsDialog(MEMapArea* mapArea);

      /**
       *    Static member to hold the one and only instance of this class.
       */
      static MEDrawSettingsDialog* _staticInstance;

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
       *    Array with buttons used to set if the item types should be 
       *    drawn or not.
       */
      Gtk::ToggleButton* m_showLayers[int(ItemTypes::numberOfItemTypes)];

      /**
       *    Array with buttons used to set if the polygon for the item 
       *    types should be filled or not.
       */
      Gtk::ToggleButton* m_fillLayers[int(ItemTypes::numberOfItemTypes)];

      /**
       *    Array with the selected colors for all the item-types.
       */
      Gtk::Entry* m_colorEntriesLayers[int(ItemTypes::numberOfItemTypes)];

      /**
       *    Array with the default colors for the different item-types.
       */
      MEGdkColors::color_t 
         m_defaultColors[int(ItemTypes::numberOfItemTypes)];
      
      /**
       *    The map area where the maps is drawn.
       */
      MEMapArea* m_mapArea;
};

#endif

