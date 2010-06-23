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
#include "MEGroupItemInfoWidget.h"
#include <gtkmm/scrolledwindow.h>
#include "MEMapArea.h"
#include "OldGenericMap.h"
#include <gtkmm.h>
#include "StringUtility.h"
#include "Utility.h"

MEGroupItemInfoWidget::MEGroupItemInfoWidget()
   : MEAbstractItemInfoWidget("Group")
{
   Gtk::Box* mainBox = manage(new Gtk::VBox());
   
   // Create the groups ListStore and add to groups TreeView  
   m_listStore = Gtk::ListStore::create(m_columns);
   m_treeView.set_model(m_listStore);
   m_treeView.append_column("Pos", m_columns.m_pos);
   m_treeView.append_column("Tpe", m_columns.m_tpe);
   m_treeView.append_column("ID", m_columns.m_ID);
   m_treeView.append_column("Name", m_columns.m_name);

   // Set column size- and resize properties.
   Gtk::TreeViewColumn* tmpCol;
   tmpCol = m_treeView.get_column(0);
   tmpCol->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
   tmpCol->set_fixed_width(40);
   tmpCol->set_resizable(true);

   tmpCol = m_treeView.get_column(1);
   tmpCol->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
   tmpCol->set_fixed_width(40);
   tmpCol->set_resizable(true);

   tmpCol = m_treeView.get_column(2);
   tmpCol->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
   tmpCol->set_fixed_width(70);
   tmpCol->set_resizable(true);

   // Create selection object to handle chosen rows
   m_selection = m_treeView.get_selection();
   if (!m_selection) {
      mc2log << error << "No selection object created for corresponding "
             << "TreeView" << endl;
      MC2_ASSERT(false);
   }

   // Create a scrolled window to pack the TreeView widget into */
   Gtk::ScrolledWindow *scrolledWindow = manage(new Gtk::ScrolledWindow());
   scrolledWindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
   scrolledWindow->add(m_treeView);
   mainBox->pack_start(*scrolledWindow);

   // Add highlight + select buttons
   Gtk::Box* buttonBox = manage(new Gtk::HBox());
   Gtk::Button* button = manage(new Gtk::Button("Highlight items in group"));
   button->signal_clicked().connect(
      sigc::mem_fun(*this, &MEGroupItemInfoWidget::highlightPressed));
   buttonBox->pack_start(*button);
   
   button = manage(new Gtk::Button("Recursive"));
   button->signal_clicked().connect(
      sigc::mem_fun(*this, &MEGroupItemInfoWidget::highlightRecursivePressed));
   buttonBox->pack_start(*button);

   button = manage(new Gtk::Button("Select"));
   button->signal_clicked().connect(
            sigc::mem_fun(*this, &MEGroupItemInfoWidget::selectItemInGroup));
   buttonBox->pack_start(*button);
   
   mainBox->pack_start(*buttonBox, false, false);
   
   // Add to this frame
   add(*mainBox);
}

MEGroupItemInfoWidget::~MEGroupItemInfoWidget()
{
   // Nothing allocated, so nothing to do
}

void
MEGroupItemInfoWidget::activate(MEMapArea* mapArea, OldGroupItem* group)
{
   MEAbstractItemInfoWidget::activate(mapArea, group);

   m_listStore->clear();
   Gtk::TreeModel::Row row;  
 
   char posStr[16];
   char idStr[16];

   // Items that are in this group
   uint32 nbrItemsInGroup = group->getNbrItemsInGroup();
   for (uint32 i=0; i<nbrItemsInGroup; ++i) {
      // add itemNumber, itemID and names to the list
      sprintf(posStr, "%u", i);
      row = *(m_listStore->append());
      row[m_columns.m_pos]=posStr;
      
      const uint32 id = group->getItemNumber(i);
      MC2String typeIdStr = getItemTypeIDForItemInGroup(id);
      row[m_columns.m_tpe]=typeIdStr.c_str();
      
      sprintf(idStr, "%u", id);
      row[m_columns.m_ID]=idStr;
      
      MC2String nameStr = getNameStrForItemInGroup(id);
      row[m_columns.m_name]=nameStr.c_str();
   }
   mc2dbg << "Number items in group = " << nbrItemsInGroup << endl;

   // Items that have this item as group
   if ( nbrItemsInGroup == 0 ) {
      mc2dbg << "MEGroupItemInfoWidget: No members from group item, " 
             << "loops map" << endl;
      OldGenericMap* theMap = m_mapArea->getMap();
      vector<uint32> items = 
         theMap->getItemsInGroup(m_item->getID());
      for (uint32 i=0; i<items.size(); i++){
         sprintf(posStr, "%u", i);
         row = *(m_listStore->append());
         row[m_columns.m_pos]=posStr;
         
         MC2String typeIdStr = getItemTypeIDForItemInGroup( items[i] );
         row[m_columns.m_tpe]=typeIdStr.c_str();
         
         sprintf(idStr, "%u", items[i]);
         row[m_columns.m_ID]=idStr;
         
         MC2String nameStr = getNameStrForItemInGroup( items[i] );
         row[m_columns.m_name]=nameStr.c_str();
         
         // print max 500 items in the items in group list
         if ( i == 499 ) {
            i = items.size();
         }
      }
      mc2dbg << " -> Number items in group = " << items.size() << endl;
      vector<uint32> ssi = theMap->getItemsInGroup(
               m_item->getID(), ItemTypes::streetSegmentItem);
      mc2dbg << " -> Number ssi in group = " << ssi.size() << endl;
   }
   
   show();
}

MC2String
MEGroupItemInfoWidget::getNameStrForItemInGroup( uint32 id )
{
   // Collect all the names
   MC2String nameStr = "";
   OldItem* nameItem = m_mapArea->getMap()->itemLookup(id);
   if ( nameItem != NULL ) {
      for (uint32 j=0; j<nameItem->getNbrNames(); ++j) {
         if (j != 0) {
            nameStr += ", ";
         }
         nameStr += m_mapArea->getMap()->getName(nameItem->getStringIndex(j));
      }
   } else {
      // Couldn't look up group!
      nameStr = "COULDN'T LOOKUP ITEM IN GROUP!!!";
   }
   
   DEBUG4(
   if (dynamic_cast<OldStreetSegmentItem*>(nameItem) != NULL) {
      OldStreetSegmentItem* ssi =
         dynamic_cast<OldStreetSegmentItem*>(nameItem);
      cout << "SSI Info: ID: 0x" << hex << id << ", Names: " << nameStr 
           << ", Nbr L/R/Type: " << dec << ssi->getLeftSideNbrStart() << "-"
           << ssi->getLeftSideNbrEnd() << "/" << ssi->getRightSideNbrStart()
           << "-" << ssi->getRightSideNbrEnd() << "/" 
           << int(ssi->getStreetNumberType()) << endl;
   });

   return nameStr;
}

MC2String
MEGroupItemInfoWidget::getItemTypeIDForItemInGroup( uint32 id )
{
   char idStr[16];
   MC2String typeIdStr = "";
   OldItem* item = m_mapArea->getMap()->itemLookup(id);
   if ( item != NULL ) {
      ItemTypes::itemType type = item->getItemType();
      sprintf(idStr, "%u", uint32(type));
      typeIdStr = idStr;
   } else {
      // Couldn't look up group!
      typeIdStr = "XX";
   }

   return typeIdStr;
}


void
MEGroupItemInfoWidget::highlightRecursivePressed()
{
   doHighlight(true); // recursive
}

void
MEGroupItemInfoWidget::highlightPressed()
{
   doHighlight(false); // not recursive
}

void
MEGroupItemInfoWidget::doHighlight( bool recursive )
{
   OldGroupItem* group = dynamic_cast<OldGroupItem*>(m_item);
   MC2_ASSERT(group != NULL);

   m_mapArea->clearAllHighlight();
   uint32 nbrHighlighted = 0;
   for (uint32 i=0; i<group->getNbrItemsInGroup(); ++i) {
      const uint32 id = group->getItemNumber(i);
      mc2dbg4 << "MEGroupItemInfoWidget highlighting item " << id << endl;
      m_mapArea->highlightItem(id, false, false, false);
      nbrHighlighted++;
   }
   if (group->getNbrItemsInGroup()  == 0){
      mc2dbg << "MEGroupItemInfoWidget: No members from group item, " 
             << "loops map" << endl;
      OldGenericMap* theMap = m_mapArea->getMap();
      set<uint32> items;
      if ( ! recursive ) {
         vector<uint32> tmpItems = 
            theMap->getItemsInGroup(m_item->getID());
         for (uint32 i=0; i<tmpItems.size(); i++){
            items.insert( tmpItems[i] );
         }
      } else {
         items = theMap->getItemsInGroupRecursive(m_item->getID());
      }
      for ( set<uint32>::const_iterator setIT = items.begin();
            setIT != items.end(); setIT++ ) {
         mc2dbg8 << "   highlighting item " << *setIT << ": " 
                 << theMap->getBestItemName( *setIT ) << endl;
         m_mapArea->highlightItem(*setIT, false, false, false);
         nbrHighlighted++;
      }
   }

   m_mapArea->redraw();
   mc2dbg << "Highlighted " << nbrHighlighted << " items" << endl;
   
}

void
MEGroupItemInfoWidget::selectItemInGroup()
{
   mc2dbg4 << "selectItemInGroup" << endl;

   if (m_selection->count_selected_rows() > 0) {
      Gtk::TreeModel::iterator row = m_selection->get_selected();
      if(row)
      {
         MC2String str = (*row)[m_columns.m_ID];
         mc2dbg8 << "  str = " << str << endl;
         // get item id
         uint32 itemInGroupID = 0;
         char* dest = NULL;
         if ( Utility::getUint32( str.c_str(), dest, itemInGroupID ) ) {
            mc2dbg2 << "selectItemInGroup id = " << itemInGroupID << endl;
            if (!m_mapArea->highlightItem(itemInGroupID, 
                                          true /* clearFirst */)) { 
               mc2log << warn << here 
                      << " FAILED, did not find item with ID == " 
                      << itemInGroupID 
                      << " among the drawn item types" 
                      << endl;
            }
            OldItem* item = m_mapArea->getMap()->itemLookup(itemInGroupID);
            if ( item == NULL ){
               mc2log << error << "Could not find item ID: " << itemInGroupID
                      << endl;
            }
            else {
               // Show item-info, SSI is a bit special...
               MEItemInfoDialog* dialog = 
                  MEItemInfoDialog::instance(m_mapArea);
               dialog->setItemToShow(item, m_mapArea->getMap());
            }
         
         }
      }      
   } else {
      mc2dbg2 << "selectItemInGroup: no item selected" << endl;
   }
}

