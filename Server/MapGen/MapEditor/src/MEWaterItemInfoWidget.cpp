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
#include "MEWaterItemInfoWidget.h"
#include "MEMapEditorWindow.h"
#include <gtkmm/combo.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/adjustment.h>
#include <list>
#include "MEFeatureIdentification.h"
#include "MELogFilePrinter.h"

MEWaterItemInfoWidget::MEWaterItemInfoWidget()
   : MEAbstractItemInfoWidget("Water")
{
   Gtk::Table* table = manage(new Gtk::Table(1, 2));

   Gtk::Label* label = NULL;

   // Water type
   label = manage(new Gtk::Label("Water type"));
   label->set_alignment(XALIGN, YALIGN);
   table->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
   m_waterTypeValCombo = manage(new Gtk::Combo());
   m_waterTypeVal = m_waterTypeValCombo->get_entry();
   table->attach(*m_waterTypeValCombo, 1, 2, 0, 1, 
                 Gtk::FILL | Gtk::EXPAND, 
                 Gtk::FILL);
   // add water type string to the combo
   list<string> waterTypeList;
   waterTypeList.push_back("ocean");
   waterTypeList.push_back("lake");
   waterTypeList.push_back("river");
   waterTypeList.push_back("canal");
   waterTypeList.push_back("harbour");
   waterTypeList.push_back("otherWaterElement");
   waterTypeList.push_back("unknownWaterElement");
   m_waterTypeValCombo->set_popdown_strings(waterTypeList);

   // Set editable
#ifdef MAP_EDITABLE
   //m_waterTypeVal->set_state(GTK_STATE_NORMAL);
   m_waterTypeVal->set_state(Gtk::STATE_NORMAL);
#else
   m_waterTypeVal->set_editable(false);
#endif

   // Add the table to this frame
   add(*table);
}

MEWaterItemInfoWidget::~MEWaterItemInfoWidget()
{

}

void
MEWaterItemInfoWidget::activate(MEMapArea* mapArea, 
                                OldWaterItem* wi)
{
   MEAbstractItemInfoWidget::activate(mapArea, wi);

   // Water type
   const char* tmpstr = ItemTypes::getWaterTypeAsString(
                  ItemTypes::waterType(wi->getWaterType()));
   m_waterTypeVal->set_text(tmpstr);

   show_all();
}

#ifdef MAP_EDITABLE
void
MEWaterItemInfoWidget::saveChanges(MELogCommentInterfaceBox* logCommentBox)
{
   MC2_ASSERT(m_item != NULL);
   
   // Check this water item if anything changed
   OldWaterItem* wi = static_cast<OldWaterItem*>(m_item);

   // water type
   bool waterTypeChanged = false;
   ItemTypes::waterType wiType = ItemTypes::waterType(wi->getWaterType());
   int newType = ItemTypes::getWaterTypeFromString(
                     m_waterTypeVal->get_text().c_str());
   if (newType >= 0) {
      if (int(wiType) != newType)
         waterTypeChanged = true;
   }
   
   if (waterTypeChanged) {
      // get item identification string
      MC2String identString;
      bool identOK = 
         MEFeatureIdentification::getItemIdentificationString(
                        identString, m_mapArea->getMap(), m_item);

      if ( ! identOK ) {
         mc2log << warn << "Failed to get item identification string for "
                << m_item->getID() << " - abort change" << endl;
         return;
      }
      mc2dbg8 << "MEWaterItemInfoWidget: Got item identification string, \""
              << identString << "\"" << endl;
      
      // Create log comment string
      char origValStr[64];
      sprintf(origValStr, "%s", ItemTypes::getWaterTypeAsString(wiType));
      vector<MC2String> logStrings = logCommentBox->getLogComments(origValStr);

      // make update
      wi->setWaterType( ItemTypes::waterType(newType) );

      // Create new val strings
      vector<MC2String> newValStrings;
      newValStrings.push_back( m_waterTypeVal->get_text() );

      // Print to log file
      MELogFilePrinter::print(
            logStrings, "setWaterType", identString, newValStrings );
      mc2dbg << "Water type set to: " << m_waterTypeVal->get_text() << endl;
      
   }
}
#endif // MAP_EDITABLE

