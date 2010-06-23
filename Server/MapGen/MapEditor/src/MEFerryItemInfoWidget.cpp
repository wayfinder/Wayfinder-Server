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
#include "MEFerryItemInfoWidget.h"
#include "MEMapEditorWindow.h"
#include <gtkmm/combo.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/adjustment.h>
#include <list>


MEFerryItemInfoWidget::MEFerryItemInfoWidget()
   : MEAbstractItemInfoWidget("Ferry")
{
   Gtk::Table* table = manage(new Gtk::Table(1, 2));

   Gtk::Label* label = NULL;

   // Road class
   label = manage(new Gtk::Label("Road class"));
   label->set_alignment(XALIGN, YALIGN);
   table->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
   m_roadClassVal = manage(new Gtk::Entry());
   table->attach(*m_roadClassVal, 1, 2, 0, 1, 
                 Gtk::FILL | Gtk::EXPAND,
                 Gtk::FILL);

   // Ferry type
   label = manage(new Gtk::Label("Ferry type"));
   label->set_alignment(XALIGN, YALIGN);
   table->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
   
   m_ferryTypeVal = manage(new Gtk::Entry());
   table->attach(*m_ferryTypeVal, 1, 2, 1, 2, 
                 Gtk::FILL | Gtk::EXPAND, 
                 Gtk::FILL);

   // Set editable
   m_roadClassVal->set_state(Gtk::STATE_INSENSITIVE);
   m_ferryTypeVal->set_editable(false);
   
   // Add the table to this frame
   add(*table);
}

MEFerryItemInfoWidget::~MEFerryItemInfoWidget()
{

}

void
MEFerryItemInfoWidget::activate(MEMapArea* mapArea, 
                              OldFerryItem* ferry)
{
   MEAbstractItemInfoWidget::activate(mapArea, ferry);

   char tmpstr[128];

   // Road class (seems to be random values..)
   sprintf(tmpstr, "%d", uint32(ferry->getRoadClass()));
   m_roadClassVal->set_text(tmpstr);
   
   // Ferry type
   ItemTypes::ferryType type = ItemTypes::ferryType(ferry->getFerryType());
   if ( type == ItemTypes::operatedByShip ) {
      sprintf(tmpstr, "%s", "operatedByShip");
   } else if ( type == ItemTypes::operatedByTrain ) {
      sprintf(tmpstr, "%s", "operatedByTrain");
   } else {
      sprintf(tmpstr, "%s", "undefined");
   }
   m_ferryTypeVal->set_text(tmpstr);

   show_all();
}

#ifdef MAP_EDITABLE
void
MEFerryItemInfoWidget::saveChanges(MELogCommentInterfaceBox* logCommentBox)
{
   mc2log << fatal << here << "Edit ferry item not implemented!" << endl;
   return;
   
   OldGenericMap* theMap = m_mapArea->getMap();
   MC2_ASSERT(theMap != NULL);

   // See e.g. MEWaterItemInfoWidget for how to implement

}
#endif // MAP_EDITABLE


