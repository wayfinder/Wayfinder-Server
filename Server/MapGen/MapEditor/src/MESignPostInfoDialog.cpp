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
#include "MESignPostInfoDialog.h"
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include "OldNode.h"

MESignPostInfoDialog*
MESignPostInfoDialog::_staticInstance;

MESignPostInfoDialog*
MESignPostInfoDialog::instance(
      OldConnection* conn, uint32 toNodeID, MEMapArea* mapArea)
{
   if (_staticInstance == NULL)
      _staticInstance = 
         manage(new MESignPostInfoDialog(conn, toNodeID, mapArea));
   else if (_staticInstance->m_connection != conn) {
      _staticInstance->m_connection = conn;
      _staticInstance->m_toNodeID = toNodeID;
   }
   return _staticInstance;
}

MESignPostInfoDialog::~MESignPostInfoDialog()
{
   // Nothing to do
}

MESignPostInfoDialog::MESignPostInfoDialog(
      OldConnection* conn, uint32 toNodeID, MEMapArea* mapArea)
{

   m_connection = conn;
   m_toNodeID = toNodeID;
   m_mapArea = mapArea;
   m_editSpDialog = NULL;

   char tmpStr[256];
   sprintf( tmpStr, "%s, 0x%x",
            "Sign post info", m_mapArea->getMap()->getMapID() );
   set_title(tmpStr);
   
   // list with all sign posts for this connection
   Gtk::Frame* frame = manage(new Gtk::Frame("Signposts:"));

   // Create ListStore and add to TreeView
   m_listStore = Gtk::ListStore::create(m_columns);
   m_treeView.set_model(m_listStore);

   // Add visible columns to TreeView
   m_treeView.append_column("REMOVE ME", m_columns.m_info);
   m_treeView.set_headers_visible(false);

   // Create selection object to handle selections
   m_selection = m_treeView.get_selection();
   if(!m_selection)
   { 
      // If this doesn't work we're in trouble.
      mc2log << error << "No selection object created for corresponding "
             << "TreeView" << endl;
      MC2_ASSERT(false);
   }


   Gtk::ScrolledWindow* scrolledWin = manage(new Gtk::ScrolledWindow());
   scrolledWin->set_size_request(200, 150);
   scrolledWin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrolledWin->add(m_treeView);
   frame->add(*scrolledWin);
   get_vbox()->pack_start(*frame, true, true, 5);

   // buttons for Edit and Close in the action area
   Gtk::HBox* actionBox = manage(new Gtk::HBox());
   if (m_mapArea != NULL) {
      Gtk::Button* editButton = manage(new Gtk::Button("Edit"));
      editButton->set_size_request(75, 25);
      editButton->signal_clicked().connect(
            sigc::mem_fun(*this, &MESignPostInfoDialog::editSpPressed));
      actionBox->pack_start(*editButton);
   } else {
      // empty label..
      Gtk::Label* editLabel = manage(new Gtk::Label(""));
      editLabel->set_size_request(75, 25);
      actionBox->pack_start(*editLabel);
   }
   Gtk::Button* closeButton = manage(new Gtk::Button("Close"));
   closeButton->signal_clicked().connect(
                  sigc::mem_fun(*this, &MESignPostInfoDialog::closePressed));
   closeButton->set_size_request(75, 25);
   actionBox->pack_start(*closeButton);
   get_action_area()->pack_start(*actionBox);

   // Don't show the dialog now, wait for show()-command.
}

void
MESignPostInfoDialog::show()
{
   redrawSignPostNames();
   show_all();
}

void
MESignPostInfoDialog::redrawSignPostNames()
{
   char tmpstr[128];

   //m_signpostList->clear();
   m_listStore->clear();
   Gtk::TreeModel::Row row;
   uint index = 0;

   if (m_mapArea != NULL) {
      // without map area the sign post names are unreachable

      //uint32 nbrSignposts = m_connection->getNbrSignPost();
      //      for (uint32 n=0; n<nbrSignposts; n++) {

      OldGenericMap* theMap = m_mapArea->getMap();
      OldNode* toNode = theMap->nodeLookup(m_toNodeID);


      uint32 fromNodeID = m_connection->getConnectFromNode();
      const SignPostTable& spTable = theMap->getSignPostTable();
      const vector<GMSSignPost*>& signPosts =
         spTable.getSignPosts(fromNodeID, toNode->getNodeID());
      for (uint32 i=0; i<signPosts.size(); i++){
         mc2dbg8 << "i:" << i << endl;
         const vector<GMSSignPostSet>& spSets = 
            signPosts[i]->getSignPostSets();
         for (uint32 j=0; j<spSets.size(); j++){
            mc2dbg8 << "j:" << j << endl;
            const vector<GMSSignPostElm>& spElms = 
               spSets[j].getSignPostElements();
            for (uint32 k=0; k<spElms.size(); k++){
               mc2dbg8 << "k:" << k << endl;
               MC2String spElmText = spElms[k].getTextString(*theMap);
               
               /*
                 sprintf(tmpstr, "%s (%s:%s)", 
                 m_mapArea->getMap()->getName(strIndex),
                 LangTypes::getLanguageAsString(strLang),
                 ItemTypes::getNameTypeAsString(strType, true));
               */
               sprintf(tmpstr, "[%i:%i:%i] %s    (%i,%i,%i,%i)", 
                       i, j, k, 
                       spElmText.c_str(),
                       spElms[k].getElementClass(),
                       //spElms[k].getElementType(), 
                       spElms[k].getType(),
                       spElms[k].getAmbigousInfo(),
                       spElms[k].getConnectionType());
               mc2dbg4 << "Adding string to signpost-list:" << tmpstr << endl;
               row = *(m_listStore->append());
               row[m_columns.m_info]=tmpstr; 
               row[m_columns.m_index]=index;
               index++;
            }
         }
      }
   }
   row = *(m_listStore->append());
   
   if (m_mapArea != NULL) {
      row[m_columns.m_info]="-- add signpost --";
   } else {
      row[m_columns.m_info]="---";
   }
   
   row[m_columns.m_index]=index;
   index++;
}

gint
MESignPostInfoDialog::delete_event_impl(GdkEventAny* p0)
{
   hide();
   if (m_editSpDialog != NULL)
      m_editSpDialog->hide();
   return 1;
}

void
MESignPostInfoDialog::closePressed()
{
   hide();
   if (m_editSpDialog != NULL)
      m_editSpDialog->hide();
}

void
MESignPostInfoDialog::editSpPressed()
{
   uint32 spIndex = MAX_UINT32;
   // get the sign post index
   
   /*
   mc2dbg4 << "editSpPressed() selList size = " << selList.size() 
           << ", m_signpostList size = " << m_signpostList->rows().size()
           << ", m_connection nbrSp = " 
           << int(m_connection->getNbrSignPost()) << endl;
   */
   m_selection = m_treeView.get_selection();
   if (m_selection->count_selected_rows() > 0) {
      Gtk::TreeModel::iterator row = m_selection->get_selected();
      if(row)
      {
         spIndex = (*row)[m_columns.m_index];
         mc2dbg4 << "   spIndex = " << spIndex << endl;

         // Edit(remove) if spIndex < m_connection->getNbrSignPost(),
         // add new sign post name otherwise.
         m_editSpDialog = MEEditSignPostDialog::instance(m_mapArea, this);
         m_editSpDialog->setName(spIndex, m_connection, m_toNodeID);
      } else {
         mc2log << error << "No row object created" << endl;
         MC2_ASSERT(false);
      }
   } else {
      mc2dbg << "You must select a signpost name or "
             << "the add signpost-alternative to edit" << endl;
   }
}
