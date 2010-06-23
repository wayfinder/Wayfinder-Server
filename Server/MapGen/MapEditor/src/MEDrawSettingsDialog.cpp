/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MEDrawSettingsDialog.h"
#include "config.h"
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/combo.h>
#include <list>

MEDrawSettingsDialog*
MEDrawSettingsDialog::instance(MEMapArea* mapArea)
{
   if (_staticInstance == 0)
      _staticInstance = new MEDrawSettingsDialog(mapArea);
   return _staticInstance;
}

void
MEDrawSettingsDialog::deleteInstance()
{
   if (_staticInstance != 0) {
      delete _staticInstance;
   }
}

void
MEDrawSettingsDialog::show()
{
   // Set all the values from the MEMapArea
   for (uint32 i=0; i<uint32(ItemTypes::numberOfItemTypes); i++) {
      if (m_mapArea->isLayerActive(ItemTypes::itemType(i))) {
         m_showLayers[i]->set_active(true);
         m_fillLayers[i]->set_active(
               m_mapArea->isLayerFilled(ItemTypes::itemType(i)));
         m_colorEntriesLayers[i]->set_text(MEGdkColors::getColorString(
            m_mapArea->getDrawColor(ItemTypes::itemType(i))));

      } else {
         m_showLayers[i]->set_active(false);
         m_fillLayers[i]->set_active(false);
         m_colorEntriesLayers[i]->set_text(MEGdkColors::getColorString(
                  m_defaultColors[int(i)]));
      }
   }

   show_all();
}

MEDrawSettingsDialog::MEDrawSettingsDialog(MEMapArea* mapArea)
{
   MC2_ASSERT(mapArea != NULL);
   m_mapArea = mapArea;

   char foo[16];
   sprintf(foo, "0x%x", mapArea->getMapId());
   string titleString = "Drawsettings map ";
   titleString.append(foo);
   set_title(titleString);

   const uint32 nbrTypes = uint32(ItemTypes::numberOfItemTypes);
   Gtk::AttachOptions xopt = Gtk::FILL | Gtk::EXPAND;
   Gtk::AttachOptions yopt = Gtk::FILL;

   // Create a list with the colors
   list<string> colorList;
   for (int i=0; i<int(MEGdkColors::invalidColor); i++) {
      colorList.push_back(
         MEGdkColors::getColorString(MEGdkColors::color_t(i)));
   }

   // Create and add the main table to the m_mainbox
   Gtk::Table* mainTable = manage(new Gtk::Table(nbrTypes, 4, false));
   get_vbox()->pack_start(*mainTable, false, false, 0);
   for (uint32 i=0; i<uint32(ItemTypes::numberOfItemTypes); i++) {
      Gtk::Label* label = NULL;
      if (i == uint32(ItemTypes::routeableItem)) {
         // This is actually the map-boundry!!!
         label = new Gtk::Label("Map boundary");
      } else {
         label = new Gtk::Label(
            StringTable::getString(
               ItemTypes::getItemTypeSC(ItemTypes::itemType(i)), 
               StringTable::ENGLISH));
      }
      mainTable->attach(*label, 0, 1, i, i+1, xopt, yopt);
      m_showLayers[i] = manage(new Gtk::ToggleButton("Show"));
      mainTable->attach(* m_showLayers[i], 1, 2, i, i+1, xopt, yopt);
      m_fillLayers[i] = manage(new Gtk::ToggleButton("Fill"));
      mainTable->attach(*m_fillLayers[i], 2, 3, i, i+1, xopt, yopt);
      Gtk::Combo* combo = manage(new Gtk::Combo());
      combo->set_popdown_strings(colorList);
      m_colorEntriesLayers[i] = combo->get_entry();
      mainTable->attach(*combo, 3, 4, i, i+1, xopt, yopt);
   }

   // Add OK and cancel-button
   Gtk::Button* cancelButton = manage(new Gtk::Button("Cancel"));
   Gtk::Button* okButton = manage(new Gtk::Button("OK"));
   cancelButton->signal_clicked().connect(
      sigc::mem_fun(*this, &MEDrawSettingsDialog::cancelPressed));
   okButton->signal_clicked().connect(
      sigc::mem_fun(*this, &MEDrawSettingsDialog::okPressed));
   Gtk::HBox* box = manage(new Gtk::HBox());
   box->set_size_request(450, 2);
   box->pack_start(*cancelButton, true, true);
   box->pack_start(*okButton, true, true);
   //get_action_area()->set_size_request(200, 200);
   get_action_area()->pack_start(*box, true, true);
   //get_vbox()->pack_start(*box, true, true);

   // Reset
   for (uint32 i=0; i<uint32(ItemTypes::numberOfItemTypes); i++) {
      m_defaultColors[i] = MEGdkColors::black;
   }

   // Set default-colors
   m_defaultColors[int(ItemTypes::streetSegmentItem)] = MEGdkColors::black;
   m_defaultColors[int(ItemTypes::municipalItem)] = MEGdkColors::black;
   m_defaultColors[int(ItemTypes::waterItem)] = MEGdkColors::lightBlue;
   m_defaultColors[int(ItemTypes::parkItem)] = MEGdkColors::green;
   m_defaultColors[int(ItemTypes::forestItem)] = MEGdkColors::green;
   m_defaultColors[int(ItemTypes::buildingItem)] = MEGdkColors::rosyBrown;
   m_defaultColors[int(ItemTypes::railwayItem)] = MEGdkColors::black;
   m_defaultColors[int(ItemTypes::islandItem)] = MEGdkColors::black;
   m_defaultColors[int(ItemTypes::streetItem)] = MEGdkColors::orange;
   m_defaultColors[int(ItemTypes::nullItem)] = MEGdkColors::black;
   m_defaultColors[int(ItemTypes::zipCodeItem)] = MEGdkColors::black;
   m_defaultColors[int(ItemTypes::builtUpAreaItem)] = MEGdkColors::indianRed;
   m_defaultColors[int(ItemTypes::cityPartItem)] = MEGdkColors::black;
   m_defaultColors[int(ItemTypes::pointOfInterestItem)] = MEGdkColors::black;
   m_defaultColors[int(ItemTypes::categoryItem)] = MEGdkColors::black;
   m_defaultColors[int(ItemTypes::routeableItem)] = MEGdkColors::orange; // used for map gfx data
   m_defaultColors[int(ItemTypes::busRouteItem)] = MEGdkColors::black;
   m_defaultColors[int(ItemTypes::ferryItem)] = MEGdkColors::blue;
   m_defaultColors[int(ItemTypes::airportItem)] = MEGdkColors::white;
   m_defaultColors[int(ItemTypes::aircraftRoadItem)] = MEGdkColors::peru;
   m_defaultColors[int(ItemTypes::pedestrianAreaItem)] = MEGdkColors::black;
   m_defaultColors[int(ItemTypes::militaryBaseItem)] = MEGdkColors::dimGrey;
   m_defaultColors[int(ItemTypes::individualBuildingItem)] = MEGdkColors::orange;
   m_defaultColors[int(ItemTypes::subwayLineItem)] = MEGdkColors::peru;
   m_defaultColors[int(ItemTypes::borderItem)] = MEGdkColors::green;
   m_defaultColors[int(ItemTypes::cartographicItem)] = MEGdkColors::grey;

}

gint
MEDrawSettingsDialog::delete_event_impl(GdkEventAny* p0)
{
   hide();
   return 1;
}


void
MEDrawSettingsDialog::cancelPressed()
{
   hide();
}

void
MEDrawSettingsDialog::okPressed()
{

   // Set all the values from the MEMapArea
   for (uint32 i=0; i<uint32(ItemTypes::numberOfItemTypes); i++) {
      if (m_showLayers[i]->get_active()) {
         m_mapArea->addItemLayer(ItemTypes::itemType(i), false);
         // Color
         char* tmpStr = StringUtility::newStrDup(
               m_colorEntriesLayers[i]->get_text().c_str());
         MEGdkColors::color_t col = MEGdkColors::getColorFromString(tmpStr);
         delete tmpStr;
         m_mapArea->setItemLayerParameters(ItemTypes::itemType(i), 
                                           m_fillLayers[i]->get_active(),
                                           col);
      } else {
         m_mapArea->removeItemLayer(ItemTypes::itemType(i), false);
      }
   }
   m_mapArea->recalculateAndRedraw();


   hide();
}

      
