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
#include "MEShowNodesWindow.h"
#include "OldNode.h"

MEShowNodesWindow* 
MEShowNodesWindow::m_theTurnDesciptionsWindow = NULL;

MEShowNodesWindow::~MEShowNodesWindow() 
{
   m_theTurnDesciptionsWindow = NULL;
}

MEShowNodesWindow::MEShowNodesWindow(MEMapArea* mapArea)
   : m_selectedRow( NULL ), m_mapArea(mapArea)
{
   set_title("Show nodes");
   set_size_request(300, 400);      

   // Create the main-box where the frames are added. 
   Gtk::Box* mainbox = manage(new Gtk::VBox());
   Gtk::Frame* frame = NULL;
   //Gtk::Adjustment* adj = NULL;
   Gtk::Label* label = NULL;
   Gtk::Button* btn = NULL;

   m_fileSelector = manage(new Gtk::FileSelection("Select turn desc. file"));
   m_fileSelector->get_ok_button()->signal_clicked().connect(
               sigc::mem_fun(*this, &MEShowNodesWindow::on_fileSelOK));

   m_fileSelector->get_cancel_button()->signal_clicked().connect(
                sigc::mem_fun(*this, &MEShowNodesWindow::on_fileSelCancel));              


   m_fileSelector->hide_fileop_buttons();

   // Frame where to load file
   frame = manage(new Gtk::Frame("File"));
   Gtk::Box* box = manage(new Gtk::HBox());
   m_fileNameEntry = manage(new Gtk::Entry());
   box->pack_start(*m_fileNameEntry);
   btn = manage(new Gtk::Button("..."));
   btn->signal_clicked().connect(
            sigc::mem_fun(*this, &MEShowNodesWindow::on_selectFile));            
   box->pack_start(*btn, false, false); 
   btn = manage(new Gtk::Button("Load"));
   btn->signal_clicked().connect(
            sigc::mem_fun(*this, &MEShowNodesWindow::on_loadFile));    
   box->pack_start(*btn, false, false); 
   frame->add(*box);
   mainbox->pack_start(*frame, false, false);

   // Create ListStore and add to TreeView
   m_listStore = Gtk::ListStore::create(m_columns);
   m_treeView.set_model(m_listStore);
 
   m_treeView.append_column("From", m_columns.m_from);
   m_treeView.append_column("To", m_columns.m_to);
   m_treeView.append_column("Old turn", m_columns.m_oldTurn);
   m_treeView.append_column("Old CK", m_columns.m_oldCK);

   // Create selection object to handle selections
   m_selection = m_treeView.get_selection();
   
   if( m_selection )
   {
      m_selection->signal_changed().connect(
        sigc::mem_fun(*this, &MEShowNodesWindow::on_showNode));
   } else {
      // If this doesn't work we're in trouble.
      mc2log << error << "No selection object created for corresponding "
             << "TreeView" << endl;
      MC2_ASSERT(false);
   }

   // Set column size- and resize properties.
   Gtk::TreeViewColumn* tmpCol;
   tmpCol = m_treeView.get_column( 0 );
   tmpCol->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
   tmpCol->set_fixed_width( 90 );   
   tmpCol->set_resizable( true );
 
   tmpCol = m_treeView.get_column( 1 );
   tmpCol->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
   tmpCol->set_fixed_width( 90 );
   tmpCol->set_resizable( true );

   tmpCol = m_treeView.get_column( 2 );
   tmpCol->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
   tmpCol->set_fixed_width( 70 );
   tmpCol->set_resizable( true ); 

   tmpCol = m_treeView.get_column( 3 );
   tmpCol->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
   tmpCol->set_fixed_width( 70 );
   tmpCol->set_resizable( true ); 

   Gtk::ScrolledWindow* scrolledWin = manage(new Gtk::ScrolledWindow());
   scrolledWin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrolledWin->add(m_treeView);
   mainbox->pack_start( *scrolledWin, true, true ); 
   
   // Next and previous turn
   box = manage(new Gtk::HBox());
   btn = manage(new Gtk::Button("Prev"));
   btn->signal_clicked().connect(
            sigc::mem_fun(*this, &MEShowNodesWindow::on_clickPrev));    

   box->pack_start(*btn);
   btn = manage(new Gtk::Button("All"));
   btn->signal_clicked().connect(
            sigc::mem_fun(*this, &MEShowNodesWindow::on_clickShowAll));    
   box->pack_start(*btn);
   btn = manage(new Gtk::Button("Next"));
   btn->signal_clicked().connect(
            sigc::mem_fun(*this, &MEShowNodesWindow::on_clickNext));    
   box->pack_start(*btn);
   mainbox->pack_start(*box, false, false); 
   
   // Frame with information about the selected crossing/connection
   frame = manage(new Gtk::Frame("Selected crossing"));
   Gtk::HBox* hbox = manage(new Gtk::HBox());

   box = manage(new Gtk::VBox());
   label = manage(new Gtk::Label("Current values"));
   label->set_pattern(           "______________");
   box->pack_start(*label);
   m_curTurnDesc = manage(new Gtk::Label(""));
   box->pack_start(*m_curTurnDesc);
   m_curCrossingKind = manage(new Gtk::Label(""));
   box->pack_start(*m_curCrossingKind);
   hbox->pack_start(*box);
   
   box = manage(new Gtk::VBox());
   label = manage(new Gtk::Label("Old values"));
   label->set_pattern(           "__________");
   box->pack_start(*label);
   m_oldTurnDesc = manage(new Gtk::Label(""));
   box->pack_start(*m_oldTurnDesc);
   m_oldCrossingKind = manage(new Gtk::Label(""));
   box->pack_start(*m_oldCrossingKind);
   hbox->pack_start(*box);

   frame->add(*hbox);
   mainbox->pack_start(*frame, false, false); 

   add(*mainbox);
   show_all();
}

void
MEShowNodesWindow::on_selectFile()
{
   m_fileSelector->show();
}

void 
MEShowNodesWindow::on_fileSelOK()
{
   string fileNameStr = m_fileSelector->get_filename();
   mc2dbg4 << "file_ok_sel: " << fileNameStr << endl;

   m_fileNameEntry->set_text(fileNameStr);
   m_fileSelector->hide();
   on_loadFile();
}

void 
MEShowNodesWindow::on_fileSelCancel()
{
   mc2dbg8 << here << "CANCEL pressed" << endl;
   m_fileSelector->hide();
}

void
MEShowNodesWindow::on_loadFile()
{
   string fileNameStr = m_fileNameEntry->get_text();
   ifstream turnFile(fileNameStr.c_str());
   mc2dbg << "Opening file: \"" << fileNameStr << "\"" << endl;
   
   m_listStore->clear();
   char str[16];

   char row[256];
   if ((fileNameStr.length() > 0) && (turnFile)) {
      while (!turnFile.eof()) {
         mc2dbg8 << "In while " << endl;
         turnFile.getline(row, 256);
         const char* TURNDESC_REC = "TURNDESCRIPTION:";
         const char* NODE_REC = "NODE:";

         Gtk::TreeModel::Row treeRow;
         //treeRow = *(m_listStore->append());
         //row[m_columnsED.m_recId]=str;


         if (strncmp(row, TURNDESC_REC, strlen(TURNDESC_REC)) == 0) {
            treeRow = *(m_listStore->append());
            mc2dbg8 << "In if 1 " << endl;
            // Format: mapID.FromID;mapID.ToID;Old turndesc; Old crossingkind;
            char * curPos = row+strlen(TURNDESC_REC)+1;
            int oldTurnDesc=0, oldCrossingKind=0;
            uint32 fromID=MAX_UINT32, toID=MAX_UINT32;
            uint32 fromMapID=MAX_UINT32, toMapID=MAX_UINT32-1;
            for (int i=0; i<4; ++i) {
               uint32 curVal = strtoul(curPos, NULL, 0);
               switch (i) {
                  case 0: // FromID
                     fromMapID = curVal;
                     curPos = strchr(curPos, '.') + 1;
                     fromID = strtoul(curPos, NULL, 0);
                     break;
                  case 1: // ToID
                     toMapID = curVal;
                     curPos = strchr(curPos, '.') + 1;
                     toID = strtoul(curPos, NULL, 0);
                     break;
                  case 2: // Old turndesc
                     oldTurnDesc = curVal;
                     break;
                  case 3: // Old crossingkind
                     oldCrossingKind = curVal;
                     break;
                  default:
                     mc2log << error << "Unknown format of file" << endl;
               }
               
               //mc2dbg << i << " val=" << curVal << ", curPos: " 
               //       << curPos << endl;
               curPos = strchr(curPos, ';') + 1;
            } 
            if ( (fromMapID == toMapID) && 
                 (fromMapID == m_mapArea->getMap()->getMapID())) {
               mc2dbg8 << "In if 2 " << endl;
               sprintf(str, "%u", fromID);
               treeRow[m_columns.m_from]=str;
               sprintf(str, "%u", toID);
               treeRow[m_columns.m_to]=str;
               sprintf(str, "%u", oldTurnDesc);
               treeRow[m_columns.m_oldTurn]=str;
               sprintf(str, "%u", oldCrossingKind);
               treeRow[m_columns.m_oldCK]=str;
            } else {
               mc2dbg << "Did not add " << row << " MapID's differ" << endl;
            }
         } else if (strncmp(row, NODE_REC , strlen(NODE_REC)) == 0) {
            treeRow = *(m_listStore->append());
            mc2dbg8 << "In if 3 " << endl;
            // Format: MapID.FromID;
            char * curPos = row+strlen(NODE_REC)+1;
            uint32 mapID = strtoul(curPos, NULL, 0);
            curPos = strchr(curPos, '.') + 1;
            uint32 nodeID = strtoul(curPos, NULL, 0);
            if (mapID == m_mapArea->getMap()->getMapID()) {
               //vector<Gtk::string> v;
               sprintf(str, "%u", nodeID);
               treeRow[m_columns.m_from]=str;
               treeRow[m_columns.m_to]="";
               treeRow[m_columns.m_oldTurn]="";
               treeRow[m_columns.m_oldCK]="";

            }
         }
      }
   }
   
   show();
}

void
MEShowNodesWindow::on_clickShowAll()
{
   if( m_listStore->children().size() == 0 ) {
      return;
   }

   MC2BoundingBox bbox;
   vector<uint32> allNodes;
   
   typedef Gtk::TreeModel::Children type_children;
   type_children children = m_listStore->children();
   for(type_children::iterator iter = children.begin();
      iter != children.end(); ++iter)
   {
      uint32 nodeID = processRow(*iter, false);
      allNodes.push_back(nodeID);
   }

   for (vector<uint32>::iterator it=allNodes.begin(); it!=allNodes.end(); ++it) {
      m_mapArea->highlightItem(*it, false, false, false);
      GfxData* gfx = m_mapArea->getMap()->itemLookup(*it)->getGfxData();
      GfxData::const_filter_iterator polyEnd = gfx->endFilteredPoly(0, 0);
      for(GfxData::const_filter_iterator it = gfx->beginFilteredPoly(0, 0);
          it != polyEnd; ++it) {
         MC2Coordinate currCoord = *it;
         bbox.update(currCoord.lat, currCoord.lon);
      }
   }

   const int32 delta = 50000;
   m_mapArea->zoomToBBox(bbox.getMaxLat()+delta, bbox.getMinLon()-delta, 
                         bbox.getMinLat()-delta, bbox.getMaxLon()+delta);
}

void
MEShowNodesWindow::on_clickPrev()
{
   if( !m_selectedRow ) {
      return;
   }

   if( m_selectedRow == m_listStore->children().begin() ) {
      m_selectedRow = m_listStore->children().end();
      --m_selectedRow;
   } else {
      --m_selectedRow;
   }

   m_selection->select( m_selectedRow );
}

void
MEShowNodesWindow::on_clickNext()
{
   if( !m_selectedRow ) {
      return;
   }

   ++m_selectedRow;
   if( m_selectedRow == m_listStore->children().end() ) {
      m_selectedRow = m_listStore->children().begin();
   }
   
   m_selection->select( m_selectedRow );
}

void
MEShowNodesWindow::on_showNode()
{
   m_selectedRow = m_selection->get_selected();
   if( m_selectedRow )
   {
      Gtk::TreePath path = m_listStore->get_path( m_selectedRow );
      mc2dbg8 << "Row " << path.to_string() << " selected!!!" << endl;
      Glib::ustring tmpUstring = path.to_string();
      const char* tmpCharStr = tmpUstring.c_str();
      uint32 rowNbr = strtoul(tmpCharStr, NULL, 0);
      mc2dbg8 << "Row " << rowNbr << " selected!!!" << endl;

      uint32 fromNodeID = processRow( *m_selectedRow );

      OldItem* item = m_mapArea->getMap()->itemLookup(fromNodeID);
      int32 lat, lon;
      if ((fromNodeID & 0x80000000) != 0) {
         lat = item->getGfxData()->getLat(0,0);
         lon = item->getGfxData()->getLon(0,0);
      } else {
         lat = item->getGfxData()->getLastLat(0);
         lon = item->getGfxData()->getLastLon(0);
      }

      const int32 delta = 50000;
      m_mapArea->zoomToBBox(lat+delta, lon-delta, lat-delta, lon+delta);
   }
}


uint32
MEShowNodesWindow::processRow(Gtk::TreeModel::Row row, bool locateAndRedraw) 
{
   
   Glib::ustring rowStr = row[m_columns.m_from];
   const char* str = rowStr.c_str();
   uint32 fromNodeID = strtoul(str, NULL, 0);
   rowStr = row[m_columns.m_to];
   str = rowStr.c_str();

   if (strlen(str) == 0) {
      // NODE
      // Zoom to this area in the map.
      if (locateAndRedraw) {
         mc2dbg << "highlighting node " << fromNodeID << endl;
         m_mapArea->highlightItem(fromNodeID);
      }
   } else {
   
      uint32 toNodeID = strtoul(str, NULL, 0);

      OldNode* node = m_mapArea->getMap()->nodeLookup(toNodeID);
      if ( (node!= NULL) && 
           (node->getEntryRestrictions() == ItemTypes::noWay) ) {
         mc2dbg << "   fromNodeID=" << fromNodeID << ", toNodeID=" 
                << toNodeID << " (toNode = noWay)" << endl;
      } else {
         mc2dbg << "   fromNodeID=" << fromNodeID << ", toNodeID=" 
                << toNodeID << endl;
      }

      // Zoom to this area in the map.
      if (locateAndRedraw) {
         m_mapArea->highlightConnection(toNodeID, fromNodeID);
      }

      char tmpStr[128];
      // Current values
      StringTable::languageCode lang = StringTable::ENGLISH;
      OldConnection* con = NULL;
      m_mapArea->getMap()->getConnection(fromNodeID & 0x7fffffff, 
                                         toNodeID&0x7fffffff, con);
      if (con != NULL) {
         sprintf(tmpStr, "%s", 
                 StringTable::getString(
                    ItemTypes::getTurndirectionSC(con->getTurnDirection()), lang));
         m_curTurnDesc->set_text(tmpStr);
         sprintf(tmpStr, "%s", 
                 StringTable::getString(
                    ItemTypes::getCrossingKindSC(con->getCrossingKind()), lang));
         m_curCrossingKind->set_text(tmpStr);
      } else {
         mc2log << error << "No connection between " << fromNodeID << " and "
                << toNodeID << endl;
      }

      // Old values
      rowStr = row[m_columns.m_oldTurn];
      str = rowStr.c_str();
      uint32 tmpInt = strtoul(str, NULL, 0);
      ItemTypes::turndirection_t oldTurnDesc = 
         (ItemTypes::turndirection_t) tmpInt;

      sprintf(tmpStr, "%s", 
              StringTable::getString(
                 ItemTypes::getTurndirectionSC(oldTurnDesc), lang));
      m_oldTurnDesc->set_text(tmpStr);

      rowStr = row[m_columns.m_oldCK];
      str = rowStr.c_str();
      tmpInt = strtoul(str, NULL, 0);
      ItemTypes::crossingkind_t oldCrossingKind =
         (ItemTypes::crossingkind_t) tmpInt;

      sprintf(tmpStr, "%s", 
              StringTable::getString(
                 ItemTypes::getCrossingKindSC(oldCrossingKind), lang));
      m_oldCrossingKind->set_text(tmpStr);
   }

   return fromNodeID;
}

