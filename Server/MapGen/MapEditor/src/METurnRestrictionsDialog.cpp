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
#include "METurnRestrictionsDialog.h"
#include <gtkmm/label.h>
#include <gtkmm/table.h>


METurnRestrictionsDialog*
METurnRestrictionsDialog::instance()
{
   if (_staticInstance == NULL)
      _staticInstance = new METurnRestrictionsDialog();
   return _staticInstance;

}

void
METurnRestrictionsDialog::deleteInstance()
{
   if (_staticInstance != 0) {
      delete _staticInstance;
   }
}

METurnRestrictionsDialog::~METurnRestrictionsDialog()
{
   // Nothing to do
}

void
METurnRestrictionsDialog::show( uint32 val, MEConnectionInfoBox* caller)
{
   m_caller = caller;
   
   // Reset the vehicle restrictions
   mc2dbg4 << "Setting vehicle restrictions 0x" << hex << val << dec << endl;
   
   // Check all types
   uint32 i = 0;
   int t = ItemTypes::getFirstVehiclePosition();
   while (t >= 0) {
      if ((val & t) != 0) {
         m_vehicles[i]->set_active(true);
         mc2dbg4 << "True for " << t << endl;
      } else {
         m_vehicles[i]->set_active(false);
         mc2dbg4 << "False for " << t << endl;
      }
      ItemTypes::getIncVehicleSC(t, true);
      i++;
   }

   // prefill the origvalue in the quick edit box
   char tmpStr[128];
   sprintf(tmpStr, "0x%x", val);
   m_restrictionVal->set_text(tmpStr);

   // Save the original value to be able to compare before saveing
   m_originalVal = val;

   show_all();
}

METurnRestrictionsDialog::METurnRestrictionsDialog()
{
   const Gtk::AttachOptions xopt = Gtk::FILL;
   const Gtk::AttachOptions yopt = Gtk::FILL;

   // Create and add the main table to the m_mainbox
   Gtk::Table* mainTable = manage(new Gtk::Table(m_nbrTypes+1, 2, false));
   get_vbox()->pack_start(*mainTable, false, false, 0);
   int t = ItemTypes::getFirstVehiclePosition();
   uint32 i=0;
   while (t >= 0) {
      Gtk::Label* label = new Gtk::Label(
            StringTable::getString(
                    ItemTypes::getVehicleSC(ItemTypes::vehicle_t(t)), 
                    StringTable::ENGLISH));
      mainTable->attach(*label, 0, 1, i, i+1, xopt, yopt);
      m_vehicles[i] = manage(new Gtk::ToggleButton("Allowed"));
      mainTable->attach(* m_vehicles[i], 1, 2, i, i+1, xopt, yopt);

      // Update loop variables
      ItemTypes::getIncVehicleSC(t, true);
      ++i;
   }
   // spacing before the quick edit box
   Gtk::Label* label = new Gtk::Label(" ");
   mainTable->attach(*label, 0, 1, i, i+1, xopt, yopt);
   i++;

   // Add box for quick edit (editing the hex value manually)
   Gtk::HBox* box = manage(new Gtk::HBox());
   label = new Gtk::Label("quick edit (hex value) ");
   m_restrictionVal = new Gtk::Entry();
   
   
   box->pack_start(*label);
   box->pack_start(*m_restrictionVal);
   get_vbox()->pack_start(*box);

   // Add OK and cancel-button
   Gtk::Button* cancelButton = manage(new Gtk::Button("Cancel"));
   Gtk::Button* okButton = manage(new Gtk::Button("OK"));
   cancelButton->signal_clicked().connect(
      sigc::mem_fun(*this, &METurnRestrictionsDialog::cancelPressed));
   okButton->signal_clicked().connect(
      sigc::mem_fun(*this, &METurnRestrictionsDialog::okPressed));
   box = manage(new Gtk::HBox());
   box->pack_start(*cancelButton);
   box->pack_start(*okButton);
   get_action_area()->pack_start(*box);

}

gint
METurnRestrictionsDialog::delete_event_impl(GdkEventAny* p0)
{
   hide();
   return 1;
}

void
METurnRestrictionsDialog::cancelPressed()
{
   hide();
}

void
METurnRestrictionsDialog::okPressed()
{
   // Store the vehicle types in val
   uint32 val = m_originalVal;
   // first check the quick edit box if anything is changed here
   const char* str = m_restrictionVal->get_text().c_str();
   if ((str != NULL) && (strlen(str) > 0)) {
      if ((strlen(str) > 10) || (strlen(str) < 3) || 
          (str[0] != '0') || (str[1] != 'x')) {
         mc2dbg << "quick-edit string incorrect '" << str << "'" << endl;
      } else {
         int x = strtoul(str, NULL, 16);
         val = x;
      }
   }

   if ( m_originalVal == val) {
      // if nothing changed, check the buttons
      val = MAX_UINT32;
      uint32 i = 0;
      int t = ItemTypes::getFirstVehiclePosition();
      while (t >= 0) {
         if (! m_vehicles[i]->get_active()) {
            val ^= t;
         }
         ItemTypes::getIncVehicleSC(t, true);
         i++;
      }
      // Note: does not work properly e.g. when origVal = 0x0 or 0x80
      // and the OK-button is pressed without changing anything.
      // The result will then be newVal = 0xffc00000 or 0xffc00080
   }

   if ( m_originalVal != val) {
      // Set the vehicle restrictions in the connection-box
      m_caller->updateVehicleRestrictions(val);
   }
   hide();
}

