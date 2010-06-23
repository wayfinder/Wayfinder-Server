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
#include "MEIndividualBuildingItemInfoWidget.h"
#include "MEMapArea.h"
#include <gtkmm/combo.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/adjustment.h>
#include <list>


MEIndividualBuildingItemInfoWidget::MEIndividualBuildingItemInfoWidget()
   : MEAbstractItemInfoWidget("Individual building")
{
   Gtk::Table* table = manage(new Gtk::Table(1, 2));

   Gtk::Label* label = NULL;

   // Individual building type
   label = manage(new Gtk::Label("IBI type"));
   label->set_alignment(XALIGN, YALIGN);
   table->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
   
   m_individualBuildingTypeVal = manage(new Gtk::Entry());
   table->attach(*m_individualBuildingTypeVal, 1, 2, 0, 1, 
                 Gtk::FILL | Gtk::EXPAND, 
                 Gtk::FILL);

   // Set editable
   m_individualBuildingTypeVal->set_editable(false);
   
   // Add the table to this frame
   add(*table);
}

MEIndividualBuildingItemInfoWidget::~MEIndividualBuildingItemInfoWidget()
{

}

void
MEIndividualBuildingItemInfoWidget::activate(
   MEMapArea* mapArea, OldIndividualBuildingItem* ibi)
{
   MEAbstractItemInfoWidget::activate(mapArea, ibi);
   MC2String typeString = 
      OldIndividualBuildingItem::buildingTypeToString( 
            ibi->getBuildingType());
   m_individualBuildingTypeVal->set_text(typeString.c_str());

   show_all();
}

#ifdef MAP_EDITABLE
void
MEIndividualBuildingItemInfoWidget::saveChanges(
   MELogCommentInterfaceBox* logCommentBox)
{
   mc2log << fatal << here << "Edit ibi not implemented!" << endl;
   return;
   
   OldGenericMap* theMap = m_mapArea->getMap();
   MC2_ASSERT(theMap != NULL);

   // See e.g. WaterItemInfoWidget for how to implement

}
#endif // MAP_EDITABLE


