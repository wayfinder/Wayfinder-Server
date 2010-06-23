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
#include "MEPoiItemInfoWidget.h"
#include "MEMapEditorWindow.h"
#include <gtkmm/combo.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/adjustment.h>
#include "MapBits.h"
#include <list>


MEPoiItemInfoWidget::MEPoiItemInfoWidget()
   : MEAbstractItemInfoWidget("Poi")
{
   Gtk::Table* table = manage(new Gtk::Table(2, 2)); // rows, cols

   Gtk::Label* label = NULL;

   // WASP id
   label = manage(new Gtk::Label("WASP id"));
   label->set_alignment(XALIGN, YALIGN);
   table->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
   m_waspIdVal = manage(new Gtk::Entry());
   table->attach(*m_waspIdVal, 1, 2, 0, 1, 
                 Gtk::EXPAND | Gtk::FILL, 
                 Gtk::FILL);
   // poi type
   label = manage(new Gtk::Label("poi type"));
   label->set_alignment(XALIGN, YALIGN);
   table->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
   m_poiTypeVal = manage(new Gtk::Entry());
   table->attach(*m_poiTypeVal, 1, 2, 1, 2, 
                 Gtk::EXPAND | Gtk::FILL, 
                 Gtk::FILL);

   // ssi id
   label = manage(new Gtk::Label("ssi id"));
   label->set_alignment(XALIGN, YALIGN);
   table->attach(*label, 0, 1, 2, 3, Gtk::FILL, Gtk::FILL);
   m_ssiIdVal = manage(new Gtk::Entry());
   table->attach(*m_ssiIdVal, 1, 2, 2, 3, 
                 Gtk::EXPAND | Gtk::FILL, 
                 Gtk::FILL);

   // offset on ssi
   label = manage(new Gtk::Label("ssi offset"));
   label->set_alignment(XALIGN, YALIGN);
   table->attach(*label, 0, 1, 3, 4, Gtk::FILL, Gtk::FILL);
   m_ssiOffsetVal = manage(new Gtk::Entry());
   table->attach(*m_ssiOffsetVal, 1, 2, 3, 4, 
                 Gtk::EXPAND | Gtk::FILL, 
                 Gtk::FILL);

   // poi categories
   label = manage(new Gtk::Label("categories"));
   label->set_alignment(XALIGN, YALIGN);
   table->attach(*label, 0, 1, 4, 5, Gtk::FILL, Gtk::FILL);
   m_listStore = Gtk::ListStore::create(m_columns);
   m_treeView.set_model(m_listStore);

   // Add column to treeview and change text color
   m_treeView.append_column("REMOVE ME", m_columns.m_category);
   Gtk::TreeViewColumn* col;
   col = m_treeView.get_column(0);
   col->set_fixed_width(60);
   std::vector<Gtk::CellRenderer*> rends = col->get_cell_renderers();
   Gtk::CellRenderer* categoryRend = rends[0];
   Gtk::CellRendererText* cellRendText = 
      dynamic_cast<Gtk::CellRendererText*>(categoryRend);
   if (cellRendText != NULL) {
      cellRendText->property_foreground() = "dark gray";
      cellRendText->property_height() = 50;
      cellRendText->property_xalign() = 0.8;
      cellRendText->property_yalign() = 0.0;
      //cellRendText->set_fixed_size(30, 5);
      //cellRendText->property_size() = 10;
      //cellRendText->property_ypad() = 1;
      //cellRendText->set_fixed_height_from_font(20);
   } else {
      mc2log << info << "Upcast failed" << endl;
      MC2_ASSERT(false);
   }

   m_treeView.set_headers_visible(false);

   Gtk::ScrolledWindow* scrolledWin = manage(new Gtk::ScrolledWindow());
   scrolledWin->set_size_request(200,40);   

   scrolledWin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrolledWin->add(m_treeView);
   table->attach(*scrolledWin, 1, 2, 4, 5, 
                 Gtk::FILL | Gtk::EXPAND, 
                 Gtk::FILL | Gtk::EXPAND);
   // Set editable
   //m_waspIdVal->set_state(GTK_STATE_INSENSITIVE);
   //m_poiTypeVal->set_state(GTK_STATE_INSENSITIVE);
   //m_ssiIdVal->set_state(GTK_STATE_INSENSITIVE);
   //m_ssiOffsetVal->set_state(GTK_STATE_INSENSITIVE);
   // editable false -> not possible to edit value, but still possible to copy
   m_waspIdVal->set_editable(false);
   m_poiTypeVal->set_editable(false);
   m_ssiIdVal->set_editable(false);
   m_ssiOffsetVal->set_editable(false);
  
   // Disable chosing a category
   m_selection = m_treeView.get_selection(); 
   m_selection->set_mode(Gtk::SELECTION_NONE);   
   
   // Add the table to this frame
   add(*table);
}

MEPoiItemInfoWidget::~MEPoiItemInfoWidget()
{

}

void
MEPoiItemInfoWidget::activate(MEMapArea* mapArea, 
                              OldPointOfInterestItem* poi)
{
   MEAbstractItemInfoWidget::activate(mapArea, poi);

   char tmpstr[128];
   Gtk::TreeModel::Row row;   

   // WASP id
   sprintf(tmpstr, "%d", uint32(poi->getWASPID()));
   m_waspIdVal->set_text(tmpstr);
   
   // poi type
   const char* poiTypeStr = StringTable::getString(
      ItemTypes::getPOIStringCode(poi->getPointOfInterestType()),
      StringTable::ENGLISH);
   m_poiTypeVal->set_text(poiTypeStr);
   
   // ssi id and offset on ssi (only valid if underview map)
   if ((MapBits::isUnderviewMap(mapArea->getMapId())) && 
       (poi->getStreetSegmentItemID() != MAX_UINT32)) {
      sprintf(tmpstr, "%d", uint32(poi->getStreetSegmentItemID()));
      m_ssiIdVal->set_text(tmpstr);
      float offset = (poi->getOffsetOnStreet() / MAX_UINT16 * 100.0);
      sprintf(tmpstr, "%d", uint32(offset));
      m_ssiOffsetVal->set_text(tmpstr);
   } else {
      m_ssiIdVal->set_text("");
      m_ssiOffsetVal->set_text("");
   }
   
   // poi categories
   m_listStore->clear();
   const set<uint16>& categories = poi->getCategories( *(mapArea->getMap()));
   for ( set<uint16>::const_iterator catIt = categories.begin();
         catIt != categories.end(); ++catIt ){
      sprintf(tmpstr, "%d", *catIt);
      row = *(m_listStore->append());
      row[m_columns.m_category] = tmpstr;
   }

   show_all();
}

#ifdef MAP_EDITABLE
void
MEPoiItemInfoWidget::saveChanges(MELogCommentInterfaceBox* logCommentBox)
{
   mc2log << fatal << here << "Edit poi item not implemented!" << endl;
   return;
   
   OldGenericMap* theMap = m_mapArea->getMap();
   MC2_ASSERT(theMap != NULL);

   // See e.g. WaterItemInfoWidget for how to implement

}
#endif // MAP_EDITABLE


