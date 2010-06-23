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
#include "MEItemInfoWidget.h"
#include "MEMapArea.h"
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>

#ifdef MAP_EDITABLE
   #define ADD_NAME_STRING "--- Add name ---"
   #include "MEEditNameDialog.h"
#endif



MEItemInfoWidget::MEItemInfoWidget()
    : MEAbstractItemInfoWidget("General")
{
   Gtk::Table* table = manage(new Gtk::Table(5, 2));

   // Item type
   Gtk::Label* label = manage(new Gtk::Label("Item type"));
   label->set_alignment(XALIGN, YALIGN);
   table->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
   m_itemTypeVal = manage(new Gtk::Entry());
   table->attach(*m_itemTypeVal, 1, 2, 0, 1, 
                 Gtk::EXPAND | Gtk::FILL, 
                 Gtk::FILL);

   // ID
   label = manage(new Gtk::Label("Item ID"));
   label->set_alignment(XALIGN, YALIGN);
   table->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
   m_itemIDVal = manage(new Gtk::Entry());
   table->attach(*m_itemIDVal, 1, 2, 1, 2, 
                 Gtk::EXPAND | Gtk::FILL, 
                 Gtk::FILL);


   /* Groups
   */
   label = manage(new Gtk::Label("Groups"));
   label->set_alignment(XALIGN, YALIGN);
   //table->attach(*label, 0, 1, 3, 4, FIXED_OPT, FIXED_OPT);
     
   // Create scrolled window and add viewer
   Gtk::ScrolledWindow* scrolledWin = manage(new Gtk::ScrolledWindow());
   scrolledWin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrolledWin->set_shadow_type(Gtk::SHADOW_OUT);
   scrolledWin->add(m_treeViewGroups);
   table->attach(*scrolledWin, 1, 2, 3, 4, 
                 Gtk::EXPAND | Gtk::FILL, 
                 Gtk::EXPAND | Gtk::FILL);

   // Create ListStore and add to treeViewer.
   m_listStoreGroups = Gtk::ListStore::create(m_columnsGroups);
   m_treeViewGroups.set_model(m_listStoreGroups);

   // Create selection object
   m_selectionGroups = m_treeViewGroups.get_selection();

   // Add columns to the treeviewer
   m_treeViewGroups.append_column("Pos", m_columnsGroups.m_pos);
   m_treeViewGroups.append_column("ID", m_columnsGroups.m_ID);
   m_treeViewGroups.append_column("Type", m_columnsGroups.m_type);
   m_treeViewGroups.append_column("Name", m_columnsGroups.m_name);

   // Set column widths
   m_treeViewGroups.get_column(0)->set_min_width(30);
   m_treeViewGroups.get_column(1)->set_min_width(60);
   m_treeViewGroups.get_column(2)->set_min_width(60);
   m_treeViewGroups.get_column(3)->set_min_width(60);

   // Add a button to be able to select group
   Gtk::Box* tmpBox1 = manage(new Gtk::VBox());
   tmpBox1->pack_start(*label);
   Gtk::Button* tmpButton1=manage(new Gtk::Button("Select"));
   tmpBox1->pack_start(*tmpButton1, false, false);
   tmpButton1->signal_clicked().connect(
         sigc::mem_fun(*this, &MEItemInfoWidget::groupClicked));
   table->attach(*tmpBox1, 0, 1, 3, 4, 
                 Gtk::FILL, 
                 Gtk::FILL | Gtk::EXPAND);

   m_listStoreGroups = Gtk::ListStore::create(m_columnsGroups);
   m_treeViewGroups.set_model(m_listStoreGroups);

   /* Names
   */

   // Label
   label = manage(new Gtk::Label("Names"));
   label->set_alignment(XALIGN, YALIGN);

#ifdef MAP_EDITABLE   
   // Add a button to be able to edit the names of this item
   Gtk::Box* tmpBox = manage(new Gtk::VBox());
   tmpBox->pack_start(*label);
   Gtk::Button* tmpButton=manage(new Gtk::Button("Edit"));
   //tmpButton->set_usize(20,12);
   tmpBox->pack_start(*tmpButton, false, false);
   tmpButton->signal_clicked().connect(
         sigc::mem_fun(*this, &MEItemInfoWidget::editNamePressed));
   table->attach(*tmpBox, 0, 1, 4, 5, 
                 Gtk::FILL, 
                 Gtk::FILL | Gtk::EXPAND);
#else
   table->attach(*label, 0, 1, 4, 5, 
                 Gtk::FILL, 
                 Gtk::FILL | Gtk::EXPAND);
#endif // MAP_EDITABLE
   
   // Create scrollwindow and add viewer
   scrolledWin = manage(new Gtk::ScrolledWindow());
   scrolledWin->set_shadow_type(Gtk::SHADOW_OUT);
   scrolledWin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrolledWin->add(m_treeViewNames);
   table->attach(*scrolledWin, 1, 2, 4, 5, Gtk::FILL, Gtk::FILL);   

   // Create ListStore and add to TreeViewer 
   m_listStoreNames = Gtk::ListStore::create(m_ColumnsNames);
   m_treeViewNames.set_model(m_listStoreNames);

   // Create selection object
   m_selectionNames = m_treeViewNames.get_selection();

   // Add columns to TreeViewer
   m_treeViewNames.append_column("Names", m_ColumnsNames.m_name);
   m_treeViewNames.set_headers_visible(false);

   //m_itemIDVal->set_state(GTK_STATE_INSENSITIVE);
   m_itemIDVal->set_editable(false);
   //m_itemTypeVal->set_state(GTK_STATE_INSENSITIVE);
   m_itemTypeVal->set_editable(false);
#ifdef MAP_EDITABLE
   m_selectionNames->set_mode(Gtk::SELECTION_SINGLE);
#else
   m_selectionNames->set_mode(Gtk::SELECTION_NONE);
#endif

   
   // Add the table to this frame
   add(*table);
}

MEItemInfoWidget::~MEItemInfoWidget()
{
   // Nothing to do
}

void 
MEItemInfoWidget::activate(MEMapArea* mapArea, OldItem* item)
{
   MEAbstractItemInfoWidget::activate(mapArea, item);

   OldGenericMap* theMap = m_mapArea->getMap();
   MC2_ASSERT(theMap != NULL);

   char tmpstr[128];

   Gtk::TreeModel::Row row;
   m_listStoreGroups->clear();

   // Item type
   MC2String typeString = StringTable::getString(
         ItemTypes::getItemTypeSC(item->getItemType()),
         MEItemInfoDialog::m_language);
   if ( theMap->isIndexArea(item->getID()) ) {
      sprintf( tmpstr, "(ia order %u)", 
               theMap->getIndexAreaOrder(item->getID()));
      typeString = typeString + " " + tmpstr;
   }
   if ( theMap->itemNotInSearchIndex(item->getID()) ) {
      typeString = typeString + " (not in search index)";
   }
   m_itemTypeVal->set_text( typeString.c_str() );

   // Item ID
   sprintf(tmpstr, "%u = 0x%x", item->getID(), item->getID());
   m_itemIDVal->set_text(tmpstr);

   
   // Groups
   //m_groupList->clear();
   char idStr[16];
   char noLocationRegionStr[16]; // high bit set
   char typeStr[128];
   char nameStr[1024];
   uint32 maxLength = 1024;

   for (uint32 i=0; i<item->getNbrGroups(); ++i) {
      uint32 groupID = item->getGroup(i);
      OldItem* groupItem = mapArea->getMap()->itemLookup(groupID);
 
      row = *(m_listStoreGroups->append()); 
      row[m_columnsGroups.m_pos] = i;

      strcpy( noLocationRegionStr, "" );
      if ( (item->getUnmaskedGroup(i) & 0x80000000 ) != 0 ){
         // high bit set
         strcpy( noLocationRegionStr, " x" );
         cout << " noLocationRegionStr" << endl;
      }
      sprintf(idStr, "%u%s", groupID, noLocationRegionStr);
      row[m_columnsGroups.m_ID]=idStr;
  
      if ( groupItem != NULL ) {
         sprintf(typeStr, "%s", 
                StringTable::getString(
                    ItemTypes::getItemTypeSC(groupItem->getItemType()),
                    StringTable::ENGLISH));
         row[m_columnsGroups.m_type]=typeStr;

         // Show all the names
         strcpy( nameStr, "-no name-" );
         for (uint32 j=0; j<groupItem->getNbrNames(); ++j) {
            if (j != 0) {
               sprintf(nameStr, "%s, %s", 
                       nameStr,
                       mapArea->getMap()->getName(groupItem->getStringIndex(j)));
            } else {
               sprintf(nameStr, "%s", 
                       mapArea->getMap()->getName(groupItem->getStringIndex(j)));
            }
            if ( strlen(nameStr) > maxLength ) {
               mc2log << error << here << " nameStr of group " << groupID
                      << " is too long (" << strlen(nameStr) 
                      << " vs. " << maxLength << ")" << endl
                      << " nameStr=" << nameStr << endl;
               MC2_ASSERT(false);
            }
         }
         mc2dbg8 << "member of group nameStr = " << nameStr << endl;
         row[m_columnsGroups.m_name]=nameStr;
      } else {
         // Group item is NULL, but we still want to see it!
         strcpy( typeStr, "Unknown" );
         row[m_columnsGroups.m_type]=typeStr;
         strcpy( nameStr, "Group item NULL!" );
         row[m_columnsGroups.m_name]=nameStr;
      }
      //m_groupList->rows().push_back(v);
   }

   // Names
   redrawNames(item, theMap);
   show(); 
}

void 
MEItemInfoWidget::redrawNames(  const OldItem* item,
                              const OldGenericMap* theMap) {
   
   MC2_ASSERT(item != NULL);
  
   Gtk::TreeModel::Row row;
 
   char tmpstr[128];
   // Set the names
   LangTypes::language_t strLang;
   ItemTypes::name_t strType;
   uint32 strIndex;
   m_listStoreNames->clear();

   for (byte j=0; j<item->getNbrNames(); j++) {
      row = *(m_listStoreNames->append());
      item->getNameAndType(j, strLang, strType, strIndex);
      sprintf(tmpstr, "%s (%s:%s)", 
                      theMap->getName(strIndex),
                      LangTypes::getLanguageAsString(strLang),
                      ItemTypes::getNameTypeAsString(strType, true));
      row[m_ColumnsNames.m_name] = tmpstr;
      row[m_ColumnsNames.m_index] = j;
      mc2dbg4 << "Adding string to name-list:" << tmpstr << endl;
   }

#ifdef MAP_EDITABLE
   row = *(m_listStoreNames->append());
   row[m_ColumnsNames.m_name]=ADD_NAME_STRING;
   row[m_ColumnsNames.m_index] = item->getNbrNames();
#endif

   show_all();
}

#ifdef MAP_EDITABLE
void
MEItemInfoWidget::editNamePressed()
{
   mc2dbg4 << "MEItemInfoWidget::editNamePressed()" << endl;
   
   int nameIndex = MAX_INT32;
   // Get the name-index
   
   if (m_selectionNames->count_selected_rows() > 0) {
      Gtk::TreeModel::iterator row = m_selectionNames->get_selected();
      if(row)
      {
         nameIndex = (*row)[m_ColumnsNames.m_index];
         mc2dbg8 << "   nameIndex = " << nameIndex << endl;
         mc2dbg2 << "Number of selected names = " 
                 << m_selectionNames->count_selected_rows()
                 << ", Number of names (ListStore) = " 
                 << m_listStoreNames->children().size()
                 << ", Number of names (item) = " 
                 << int(m_item->getNbrNames())
                 << ", nameIndex = " << nameIndex << endl;
   
         // Edit if nameIndex < m_item->getNbrNames(), add new name otherwise.
         (MEEditNameDialog::instance(m_mapArea, this))
            ->setName(nameIndex, m_item, m_mapArea->getMap());
      }
   } else {
      mc2dbg << "Select a name or the addName-alternative to edit!" << endl;
   }
}
#endif

void
MEItemInfoWidget::groupClicked()
{
   mc2dbg4 << "MEItemInfoWidget::groupClicked()" << endl;

   int groupIndex = MAX_INT32;

   if (m_selectionGroups->count_selected_rows() > 0) {
      Gtk::TreeModel::iterator row = m_selectionGroups->get_selected();
      if(row)
      {
         groupIndex = (*row)[m_columnsGroups.m_pos];
         mc2log << info << "Nr of selected rows = " 
                 << m_selectionGroups->count_selected_rows() 
                 << ", Nbr of groups (ListStore)= " 
                 << m_listStoreGroups->children().size() 
                 << ", Nbr of groups (OldItem) = "  
                 << int(m_item->getNbrGroups()) 
                 << ", groupsIndex = " << groupIndex << endl;

         bool clearFirst = true;
         uint32 groupID = m_item->getGroup(groupIndex);
         if (!m_mapArea->highlightItem(groupID, clearFirst)) {
            mc2log << warn << here 
                   << " FAILED, did not find item with ID == " 
                   << groupID << " among the drawn item types" << endl;
         }
         OldItem* groupItem = m_mapArea->getMap()->itemLookup(groupID);
         if ( groupItem == NULL ){
            mc2log << error << "Could not find group item ID: " << groupID
                   << endl;
         }
         else {
            // Show item-info, SSI is a bit special...
            MEItemInfoDialog* dialog = MEItemInfoDialog::instance(m_mapArea);
            dialog->setItemToShow(groupItem, m_mapArea->getMap());
         }
      }
   } else {
      mc2log << info << "No group selected" << endl; 
   }
}
