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
#include "MEShowExtradataWindow.h"
#include <gtkmm/main.h>
#include <gtkmm/scrolledwindow.h>
#include "GfxConstants.h"
#include "OldMapHashTable.h"
#include <map>
#include "MapBits.h"
#include "NationalProperties.h"

MEShowExtradataWindow* 
MEShowExtradataWindow::m_redWindow = NULL;

MEShowExtradataWindow*
MEShowExtradataWindow::instance(MEMapArea* mapArea)
{

   if (m_redWindow == NULL) {
      // No existing window
      m_redWindow =
           new MEShowExtradataWindow(mapArea);

   } else if (m_redWindow->m_mapArea != mapArea) {
      delete m_redWindow;
      m_redWindow = new MEShowExtradataWindow(mapArea);
   }
   return m_redWindow;
}

MEShowExtradataWindow::~MEShowExtradataWindow() 
{
   m_redWindow = NULL;
}

void
MEShowExtradataWindow::deleteInstance()
{
   if (m_redWindow != NULL) {
      delete m_redWindow;
   }
}

MEShowExtradataWindow::MEShowExtradataWindow(MEMapArea* mapArea)
{
   m_mapArea = mapArea;

   set_title("Show extradata");
   
   // Create the main-box where the frames are added. 
   Gtk::Box* mainbox = manage(new Gtk::VBox(FALSE, 5));
   // (not homogenous, spacing between content)
   Gtk::Frame* frame = NULL;
   Gtk::Button* btn = NULL;
   Gtk::Box* box = NULL; 

   // The file selector
   m_fileSelector = new Gtk::FileSelection("Select extradata file");
   m_fileSelector->get_ok_button()->signal_clicked().connect(
         sigc::mem_fun(*this, &MEShowExtradataWindow::on_fileSelOK));
   m_fileSelector->get_cancel_button()->signal_clicked().connect(
         sigc::mem_fun(*this, &MEShowExtradataWindow::on_fileSelCancel));
   m_fileSelector->hide_fileop_buttons();

   // Frame where to load file
   frame = manage(new Gtk::Frame("Collect extradata"));

   box = manage(new Gtk::HBox(FALSE, 5));
   m_fileNameEntry = manage(new Gtk::Entry());
   box->pack_start(*m_fileNameEntry);
   btn = manage(new Gtk::Button("Browse"));
   btn->signal_clicked().connect(
         sigc::mem_fun(*this, &MEShowExtradataWindow::on_browseForEDFile));
   box->pack_start(*btn, false, false); // don't expand (btn = min size)
   btn = manage(new Gtk::Button("Load"));
   btn->signal_clicked().connect(
         sigc::mem_fun(*this, &MEShowExtradataWindow::on_loadEDFile));
   box->pack_end(*btn, false, false, 5); // don't expand, don't fill, padding
   frame->add(*box);
   mainbox->pack_start(*frame, false, false); // don't expand = the frame stays
                                       // the same size if window is enlarged

   // The extradata is displayed in a CList
   
   frame = manage(new Gtk::Frame("Extradata records in this map"));

   Gtk::ScrolledWindow* scrolledWin = manage(new Gtk::ScrolledWindow());
   scrolledWin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrolledWin->add(m_treeViewED);
   frame->add(*scrolledWin);
   frame->set_size_request(460, 200);   // width, height
   mainbox->pack_start(*frame);

   // Create ListStore and add to TreeView
   m_listStoreED = Gtk::ListStore::create(m_columnsED);
   m_treeViewED.set_model(m_listStoreED);
 
   m_treeViewED.append_column("recId", m_columnsED.m_recId);
   m_treeViewED.append_column("recType", m_columnsED.m_recType);
   m_treeViewED.append_column("value", m_columnsED.m_value);
   m_treeViewED.append_column("orig value", m_columnsED.m_orig_value);
   m_treeViewED.append_column("TArefId", m_columnsED.m_TArefId);

   // Create selection object to handle selections
   m_selectionED = m_treeViewED.get_selection();
   
   if(m_selectionED)
   {
      m_selectionED->signal_changed().connect(
        sigc::mem_fun(*this, &MEShowExtradataWindow::ed_selected));
   } else {
      // If this doesn't work we're in trouble.
      mc2log << error << "No selection object created for corresponding "
             << "TreeView" << endl;
      MC2_ASSERT(false);
   }

   // Set column size- and resize properties.
   Gtk::TreeViewColumn* tmpCol;
   tmpCol = m_treeViewED.get_column(0);
   tmpCol->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
   tmpCol->set_fixed_width(60);   
   tmpCol->set_resizable(true);
 
   tmpCol = m_treeViewED.get_column(1);
   tmpCol->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
   tmpCol->set_fixed_width(120);
   tmpCol->set_resizable(true);

   tmpCol = m_treeViewED.get_column(2);
   tmpCol->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
   tmpCol->set_fixed_width(70);
   tmpCol->set_resizable(true); 

   tmpCol = m_treeViewED.get_column(3);
   tmpCol->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
   tmpCol->set_fixed_width(70);
   tmpCol->set_resizable(true); 

   tmpCol = m_treeViewED.get_column(4);
   tmpCol->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
   tmpCol->set_fixed_width(75);
   tmpCol->set_resizable(true);

   // Highlight and zoom buttons for visualizing a selected extradata record
   frame = manage(new Gtk::Frame("Show the selected record"));
   Gtk::Box* buttonBox = manage(new Gtk::VBox(FALSE, 10));
  
   box = manage(new Gtk::HBox(FALSE, 5));
   btn = manage(new Gtk::Button("Highlight one coord"));
   btn->signal_clicked().connect(
         sigc::mem_fun(*this, &MEShowExtradataWindow::highlightOneCoordPressed));
   box->pack_start(*btn, true, true, 5);
   btn = manage(new Gtk::Button("Highlight all coord"));
   btn->signal_clicked().connect(
         sigc::mem_fun(*this, &MEShowExtradataWindow::highlightAllCoordsPressed));
   box->pack_end(*btn, true, true, 5);
   buttonBox->pack_start(*box, false, false);

   box = manage(new Gtk::HBox(FALSE, 5));
   btn = manage(new Gtk::Button("Clear coord highlight"));
   btn->signal_clicked().connect(
         sigc::mem_fun(*this, &MEShowExtradataWindow::clearHighlightPressed));
   box->pack_start(*btn, true, true, 5);
   btn = manage(new Gtk::Button("Zoom to coordinates"));
   btn->signal_clicked().connect(
         sigc::mem_fun(*this, &MEShowExtradataWindow::zoomPressed));
   box->pack_end(*btn, true, true, 5);
   buttonBox->pack_start(*box, false, false);
   
   frame->add(*buttonBox);
   mainbox->pack_start(*frame);

   // Copiable values
   frame = manage(new Gtk::Frame("Fields for copying some values"));
   box = manage(new Gtk::HBox(FALSE, 5));
   Gtk::Label* label = manage(new Gtk::Label("id"));
   label->set_alignment(XALIGN, YALIGN);
   box->pack_start(*label, false, false);
   m_idVal = manage(new Gtk::Entry());
   m_idVal->set_size_request(60, 20);
   box->pack_start(*m_idVal, true, true);
   m_idVal->set_state(Gtk::STATE_NORMAL);
   label = manage(new Gtk::Label("coords"));
   label->set_alignment(XALIGN, YALIGN);
   box->pack_start(*label, false, false);
   m_latVal = manage(new Gtk::Entry());
   m_latVal->set_size_request(65, 20);
   box->pack_start(*m_latVal, true, true);
   m_latVal->set_state(Gtk::STATE_NORMAL);
   m_lonVal = manage(new Gtk::Entry());
   m_lonVal->set_size_request(65, 20);
   box->pack_start(*m_lonVal, true, true);
   m_lonVal->set_state(Gtk::STATE_NORMAL);
   label = manage(new Gtk::Label("ref"));
   label->set_alignment(XALIGN, YALIGN);
   box->pack_start(*label, false, false);
   m_refVal = manage(new Gtk::Entry());
   m_refVal->set_size_request(70, 20);
   box->pack_start(*m_refVal, true, true);

   frame->add(*box);
   mainbox->pack_start(*frame);

   // Coordinates of the extradata record is displayed in a list
   frame = manage(new Gtk::Frame("Coordinates of the selected record"));
   Gtk::Box* coordBox = manage(new Gtk::VBox(FALSE, 10));
   scrolledWin = manage(new Gtk::ScrolledWindow());
   scrolledWin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
   scrolledWin->add(m_treeViewEDCoords);
   frame->set_size_request(460, 100);   // width, height

   // Create ListStore and and to TreeView
   m_listStoreEDCoords = Gtk::ListStore::create(m_columnsEDCoords);
   m_treeViewEDCoords.set_model(m_listStoreEDCoords);

   // Add visible columns to TreeView
   m_treeViewEDCoords.append_column("mc2 coord", m_columnsEDCoords.m_mc2);
   m_treeViewEDCoords.append_column("wgs84 coords", 
                                    m_columnsEDCoords.m_wgs84);

   // Set column size- and resize properties.
   tmpCol = m_treeViewEDCoords.get_column(0);
   tmpCol->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
   tmpCol->set_fixed_width(170);
   tmpCol->set_resizable(true);

   tmpCol = m_treeViewEDCoords.get_column(1);
   tmpCol->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
   //tmpCol->set_fixed_width(100);
   tmpCol->set_resizable(true);
   coordBox->pack_start(*scrolledWin);
   frame->add(*coordBox);
   mainbox->pack_start(*frame);
    
   // Clear and Close-button
   box = manage(new Gtk::HBox(FALSE, 5));
   btn = manage(new Gtk::Button("Clear window"));
   btn->signal_clicked().connect(sigc::mem_fun(*this, 
					       &MEShowExtradataWindow::clearPressed));
   box->pack_start(*btn, true, true, 5);
   btn = manage(new Gtk::Button("Close window"));
   btn->signal_clicked().connect(sigc::mem_fun(*this, 
					       &MEShowExtradataWindow::closePressed));
   box->pack_end(*btn, true, true, 5);
   mainbox->pack_end(*box, false, false, 10);
   add(*mainbox);
   show_all(); 
}

void
MEShowExtradataWindow::clearPressed()
{
   // unset filename
   m_fileNameEntry->set_text("");

   // clear coordinates ListStore
   m_listStoreEDCoords->clear();

   // clear extradata ListStore
   m_listStoreED->clear();
 
   // clear mapEDRecords
   m_edRecords.clear();

   m_idVal->set_text("");
   m_latVal->set_text("");
   m_lonVal->set_text("");
   m_refVal->set_text("");
}

void
MEShowExtradataWindow::closePressed()
{
   // clear the window and hide
   clearPressed();
   hide();
}

void 
MEShowExtradataWindow::on_fileSelOK()
{
   string fileNameStr = m_fileSelector->get_filename();
   mc2dbg4 << "fileSelOK: " << fileNameStr << endl;

   m_fileNameEntry->set_text(fileNameStr);
   m_fileSelector->hide();   
}

void 
MEShowExtradataWindow::on_fileSelCancel()
{
   mc2dbg4 << here << "CANCEL pressed" << endl;
   m_fileSelector->hide();
}

void
MEShowExtradataWindow::on_browseForEDFile()
{
   m_fileSelector->show();
}


void
MEShowExtradataWindow::on_loadEDFile()
{
   string fileNameStr = m_fileNameEntry->get_text();
   ifstream edFile(fileNameStr.c_str());
   // clear any old extradata lists
   m_listStoreED->clear();
   m_edRecords.clear();

   if ((fileNameStr.length() <= 0) || (!edFile)) {
      mc2dbg << "Can't open file: \"" << fileNameStr << "\"" << endl;
   } else {
      mc2dbg << "Opening file: \"" << fileNameStr << "\"" << endl;

      // Loop the file
      // 1. use EDutility to read records
      // 2. read record comment and extract recordId + mapId + ev. group id
      // 3. read the record and fill the extradata list
    
      char str[50];
      uint32 nbrRec = 0;
      vector<MC2String> edRecord;
      Gtk::TreeModel::Row row;
      bool recordOK = 
         OldExtraDataUtility::readNextRecordFromFile(edFile, edRecord);
      while ( recordOK &&
             (edRecord.size() > 0) &&
             (edRecord[0].size() > 0)) {

         bool thisRecOK = false;
         uint32 recordId = MAX_UINT32;
         uint32 mapId = MAX_UINT32;
         string recTypeStr = "";
         string valStr = "";
         string origValStr = "";
         uint32 groupId = 0;
         string TArefId = "";

         OldExtraDataUtility::record_t recType =
            OldExtraDataUtility::getRecordType(edRecord[0]);
         // read the comment and the belonging record
         if (recType == OldExtraDataUtility::COMMENT) {
            // get record id
            if ((edRecord.size() >= 7) &&
                (edRecord[6].size() != 0) &&
                 StringUtility::onlyDigitsInString(edRecord[6].c_str())) {
               recordId = strtoul(edRecord[6].c_str(), NULL, 10);
            }
            // get map id (if given in comment)
            if ((edRecord.size() >= 12) &&
                (edRecord[11].size() != 0) &&
                 StringUtility::onlyDigitsInString(edRecord[11].c_str())) {
               // mapId = strtoul(edRecord[11].c_str(), NULL, 10);
               // NO, don't use this mapID. It is not always the case that
               // records were created in the countries-directory with all
               // mapIds starting from 0....
               mapId = MAX_UINT32;
            }
            // get orig value str
            if ((edRecord.size() >= 5) &&
                (edRecord[4].size() != 0)) {
               origValStr = edRecord[4];
            }
            // get group id
            if ((edRecord.size() >= 10) &&
                (edRecord[9].size() != 0) &&
                (StringUtility::onlyDigitsInString(edRecord[9].c_str()))) {
               groupId = strtoul(edRecord[9].c_str(), NULL, 10);
            }
            // get map supplier reference id
            if ((edRecord.size() >= 6) &&
                (edRecord[5].size() != 0)) {
               TArefId = edRecord[5];
            }

            // get the ed record
            recordOK =
               OldExtraDataUtility::readNextRecordFromFile(edFile, edRecord);
            if (recordOK) {
               recType = OldExtraDataUtility::getRecordType(edRecord[0]);
               if ((recType == OldExtraDataUtility::COMMENT) ||
                   (recType == OldExtraDataUtility::NUMBER_OF_RECORD_TYPES)) {
                  // error
               } else {
                  // we have the ed record
                  thisRecOK = true;
                  recTypeStr = edRecord[0];
                  valStr = getValueFromRecord(edRecord, recType);
                  if ((mapId == MAX_UINT32) ||
                      (!MapBits::isUnderviewMap(m_mapArea->getMapId()))) {
                     // MapId was not provided in the extradata record comment
                     // or we are looking at an overview map so the mapId
                     // will not match.
                     // Check if one coord in the record fits this map
                     mc2dbg8 << "calling recordFitsThisMap for record="
                             << recordId << endl;
                     bool correctMap = recordFitsThisMap(edRecord);
                     if (correctMap) {
                        mapId = m_mapArea->getMapId();
                        mc2dbg4 << " record " << recordId 
                                << " fits this map " << mapId << endl;
                     }
                  }
               }
            }
         }
         mc2dbg4 << "rec:" << nbrRec << "\t" 
                 << mapId << ": " << recordId << " " << recTypeStr << endl;
         
         // valStr may be empty for removeItem!!! so we can not use that
         // to check that we want to load the record.
         if ( thisRecOK && /*(valStr != "") &&*/
             (recordId != MAX_UINT32) && (mapId != MAX_UINT32) &&
             (mapId == m_mapArea->getMapId())) {
            // Save the extradata record associated with recordId
            m_edRecords.insert(pair< uint32, vector<MC2String> >
                               (recordId, edRecord));
            // Fill the ListStore with info about this ed record
            // record id
            sprintf(str, "%u", recordId);
            row = *(m_listStoreED->append());
            row[m_columnsED.m_recId]=str;
            // record type
            row[m_columnsED.m_recType]=recTypeStr;
            // val str
            if (valStr == "") {
               row[m_columnsED.m_value]="-";
            }
            else {
               row[m_columnsED.m_value]=valStr;
            }
            // orig val str
            if (origValStr == "") {
               row[m_columnsED.m_orig_value]="-";
            }
            else {
               row[m_columnsED.m_orig_value]=origValStr;
            }
            /*// group id
            if (groupId == 0) {
               v.push_back("-");
            } else {
               sprintf(str, "%u", groupId);
               v.push_back(str);
            }*/
            // TA ref Id
            if (TArefId == "") {
               row[m_columnsED.m_TArefId]="-";
            }
            else {
               row[m_columnsED.m_TArefId]=TArefId;
            }
         }

         nbrRec++;
         recordOK =
            OldExtraDataUtility::readNextRecordFromFile(edFile, edRecord);
      } //while
      
      show_all_children();
      
      mc2dbg2 << "Filled m_edRecords with " << m_listStoreED->children().size()
              << " (of total " << nbrRec << ") records for map "
              << m_mapArea->getMapId() << endl;

   }
}

void
MEShowExtradataWindow::ed_selected()
{
   m_listStoreEDCoords->clear();
   Gtk::TreeModel::Row row;
  
   Gtk::TreeModel::iterator iter = m_selectionED->get_selected();
   if(iter)
   {
      Gtk::TreePath path = m_listStoreED->get_path(iter);

      mc2dbg8 << " m_listStoreED row=" << path.to_string() 
              << " recid=" << (*iter)[m_columnsED.m_recId] << endl;
  
      
      Glib::ustring idString = (*iter)[m_columnsED.m_recId];
      uint32 recordId = (uint32) g_strtod (idString.c_str(), NULL);
      Glib::ustring refString = (*iter)[m_columnsED.m_TArefId];
      cout << recordId << refString;
    
      m_idVal->set_text( "" );
      m_latVal->set_text( "" );
      m_lonVal->set_text( "" );
      m_refVal->set_text( "" );
   
      // show the coordinates of this record in the coordinates-frame
      if (recordId != MAX_UINT32) {
      
         std::map< uint32, vector<MC2String> >::const_iterator it = 
            m_edRecords.find(recordId);
         if ( it != m_edRecords.end()) {
            // we have the correct pair
            vector<MC2String> edRecord = it->second;
            uint32 firstCoordPos;
            uint32 nbrCoords = getCoordsForRecord(edRecord, firstCoordPos);
            for (uint32 nbr = firstCoordPos;
                 nbr < firstCoordPos+nbrCoords; nbr++) {
               // print both mc2 and wgs84 coords to the coordinate list
               Gtk::TreeModel::Row row;
               row = *(m_listStoreEDCoords->append());
               row[m_columnsEDCoords.m_mc2]=edRecord[nbr];            
               MC2String wgs84coordStr = 
                     getWGS84stringFromMC2CoordString(edRecord[nbr]);
               row[m_columnsEDCoords.m_wgs84]=wgs84coordStr;
            }
         
            mc2dbg8 << " ed_selected() record type=" << edRecord[0] 
                    << " val=" << getValueFromRecord(edRecord) 
                    << " nbrCoords=" << nbrCoords << " firstCoordPos=" 
                    << firstCoordPos << endl;
 
            m_idVal->set_text( idString );
            m_refVal->set_text( refString );

            string mc2latlon = ""; 
            if ( (nbrCoords == 1) || (nbrCoords == 2) ) { 
               mc2latlon = edRecord[firstCoordPos]; 
            } else if ( nbrCoords == 4 ) { 
               mc2latlon = edRecord[firstCoordPos+2]; 
            } 
            int32 mc2lat, mc2lon; 
            bool coordOK = OldExtraDataUtility::strtolatlon( 
                              mc2latlon, mc2lat, mc2lon, 
                              CoordinateTransformer::mc2); 
         
            if ( coordOK ) {
               char mc2Str[50];
               sprintf(mc2Str, "%d", mc2lat);
               m_latVal->set_text( mc2Str );
               sprintf(mc2Str, "%d", mc2lon);
               m_lonVal->set_text( mc2Str );
            }
         }
      }
   }
}

void
MEShowExtradataWindow::highlightOneCoordPressed()
{
   // check that one coord is selected and highlight it.

   MC2String coordStr;
   m_selectionEDCoords = m_treeViewEDCoords.get_selection();
   if (m_selectionEDCoords->count_selected_rows() == 1) {
      Gtk::TreeModel::iterator iter = m_selectionEDCoords->get_selected();
      if(iter)
      {
         coordStr = (*iter)[m_columnsEDCoords.m_mc2];
         Gtk::TreePath path = m_listStoreEDCoords->get_path(iter); 
         //std::cout << "path: " << path.to_string() << endl;
         mc2dbg4 << "highlight one coord m_listStoreEDCoords size=" 
                 << m_listStoreEDCoords->children().size()
                 << " row=" << path.to_string() 
                 << " coordStr=" << coordStr << endl;
         int32 lat, lon;
         if (OldExtraDataUtility::strtolatlon(
                  coordStr, lat, lon, CoordinateTransformer::mc2)) {
            mc2dbg4 << " extracted lat=" << lat << " lon=" << lon << endl;
            // highlight the coordinate with blue
            MEGdkColors* cols = MEGdkColors::getColors();
            m_mapArea->highlightCoordinate(
                  lat, lon, cols->m_blue);
         } else {
            mc2log << error << "Could not extract cords from coord str = '"
                   << coordStr << "'" << endl;
         }
      }
   } else {
      mc2log << warn << "A coordinate to be highlighted needs to be chosen" 
             << endl;
   }
}

void
MEShowExtradataWindow::highlightAllCoordsPressed()
{
   // loop all coordinates in the m_edCoordsList and highlight them.
   MC2String coordStr;
   Gtk::TreeModel::Row row;
   MEGdkColors* cols = MEGdkColors::getColors();
   uint rowIndex = 0;

   typedef Gtk::TreeModel::Children type_children; //minimise code length.
   type_children children = m_listStoreEDCoords->children();
   uint nbrRows = children.size();

   for(type_children::iterator iter = children.begin();
      iter != children.end(); ++iter)
   {
      row = *iter;
      coordStr = row[m_columnsEDCoords.m_mc2];
      int32 lat, lon;
      
      if (OldExtraDataUtility::strtolatlon(
               coordStr, lat, lon, CoordinateTransformer::mc2)) {
         if ((nbrRows == 4) && (rowIndex < 2)) {
            // from node of a connection, highlight in orange
            m_mapArea->highlightCoordinate(
                  lat, lon, cols->m_orange);
         } else {
            // other coordinates, highlight in red
            m_mapArea->highlightCoordinate(
                  lat, lon, cols->m_red);
         }
      } else {
         mc2log << error << "Could not extract coords from coord str = '"
                << coordStr << "'" << endl;
      }
     
      rowIndex++;
   }
}

void
MEShowExtradataWindow::clearHighlightPressed()
{
   // remove all coordinate highlights
   if (m_mapArea != NULL) {
      m_mapArea->clearCoordinateHighlight(true);
   }
}

void
MEShowExtradataWindow::zoomPressed()
{
   MC2String coordStr;
   Gtk::TreeModel::Row row;

   typedef Gtk::TreeModel::Children type_children; //minimise code length.
   type_children children = m_listStoreEDCoords->children();
   uint nbrRows = children.size();

   if (nbrRows > 0) {
      MC2BoundingBox bbox;
      mc2dbg4 << "Zoom to coordinates" << endl;

      for(type_children::iterator iter = children.begin();
         iter != children.end(); ++iter)
      {
         row = *iter;
         coordStr = row[m_columnsEDCoords.m_mc2];
         int32 lat, lon;

         if (OldExtraDataUtility::strtolatlon(
                  coordStr, lat, lon, CoordinateTransformer::mc2)) {
            mc2dbg4 << " extracted lat=" << lat << " lon=" << lon << endl;
            bbox.update(lat, lon);
         } else {
            mc2log << error << "Could not extract coords from coord str = '"
                   << coordStr << "'" << endl;
         }
      }

      const int32 delta = 40000;
      m_mapArea->zoomToBBox(bbox.getMaxLat()+delta, bbox.getMinLon()-delta,
                            bbox.getMinLat()-delta, bbox.getMaxLon()+delta);
   }
}

uint32
MEShowExtradataWindow::getCoordsForRecord(
      vector<MC2String> edRecord, uint32 &firstCoordPos)
{
   // retVal = nbr coords in the ed records
   uint32 retVal = 0;
   firstCoordPos = 0;

   OldExtraDataUtility::record_t recType =
         OldExtraDataUtility::getRecordType(edRecord[0]);
   switch(recType) {
      
      case OldExtraDataUtility::ADD_NAME_TO_ITEM :
      case OldExtraDataUtility::UPDATE_NAME:
      case OldExtraDataUtility::REMOVE_ITEM:
      case OldExtraDataUtility::REMOVE_NAME_FROM_ITEM:
      case OldExtraDataUtility::SET_WATER_TYPE:
         retVal = 1;
         firstCoordPos = 3;
         break;

      case OldExtraDataUtility::SET_RAMP :
      case OldExtraDataUtility::SET_ROUNDABOUT :
      case OldExtraDataUtility::SET_ROUNDABOUTISH :
      case OldExtraDataUtility::SET_MULTIDIGITISED :
      case OldExtraDataUtility::SET_CONTROLLEDACCESS :
      case OldExtraDataUtility::SET_ENTRYRESTRICTIONS :
      case OldExtraDataUtility::SET_SPEEDLIMIT:
      case OldExtraDataUtility::SET_LEVEL :
      case OldExtraDataUtility::SET_HOUSENUMBER :
      case OldExtraDataUtility::SET_TOLLROAD:
         retVal = 2;
         firstCoordPos = 1;
         break;

      case OldExtraDataUtility::SET_TURNDIRECTION :
      case OldExtraDataUtility::SET_VEHICLERESTRICTIONS : 
      case OldExtraDataUtility::ADD_SIGNPOST :
      case OldExtraDataUtility::REMOVE_SIGNPOST :
         retVal = 4;
         firstCoordPos = 1;
         break;

      case OldExtraDataUtility::COMMENT:
      case OldExtraDataUtility::ADD_CITYPART :
      case OldExtraDataUtility::ADD_LANDMARK:
      case OldExtraDataUtility::ADD_SINGLE_LANDMARK:
      default:
         // nothing
         break;
   }

   return retVal;
}

string
MEShowExtradataWindow::getValueFromRecord(
      vector<MC2String> edRecord, OldExtraDataUtility::record_t recType)
{
   string retVal = "";
   if (recType == OldExtraDataUtility::NUMBER_OF_RECORD_TYPES) {
      // type not provided in function-call, get it here.
      recType = OldExtraDataUtility::getRecordType(edRecord[0]);
   }
   switch(recType) {

      case OldExtraDataUtility::SET_RAMP :
      case OldExtraDataUtility::SET_ROUNDABOUT :
      case OldExtraDataUtility::SET_ROUNDABOUTISH :
      case OldExtraDataUtility::SET_MULTIDIGITISED :
      case OldExtraDataUtility::SET_CONTROLLEDACCESS :
      case OldExtraDataUtility::SET_ENTRYRESTRICTIONS :
      case OldExtraDataUtility::SET_SPEEDLIMIT:
      case OldExtraDataUtility::SET_LEVEL :
      case OldExtraDataUtility::SET_TOLLROAD:
         retVal = edRecord[3];
         break;            

      case OldExtraDataUtility::REMOVE_ITEM:
         retVal = edRecord[4];
         break;

      case OldExtraDataUtility::SET_TURNDIRECTION :
      case OldExtraDataUtility::SET_VEHICLERESTRICTIONS : 
         retVal = edRecord[5];
         break;

      case OldExtraDataUtility::UPDATE_NAME:
      case OldExtraDataUtility::SET_WATER_TYPE:
         retVal = edRecord[7];
         break;

      case OldExtraDataUtility::ADD_NAME_TO_ITEM :
         //retVal = 7+8+9;
         retVal = edRecord[7] + ":" + edRecord[8] + ":" + edRecord[9];
         break;

      case OldExtraDataUtility::SET_HOUSENUMBER :
         retVal = edRecord[3] + ":" + edRecord[4] + ":" + edRecord[5] + ":" +
                  edRecord[6] + ":" + edRecord[7];
         break;

      case OldExtraDataUtility::REMOVE_NAME_FROM_ITEM:
         retVal = edRecord[4] + ":" + edRecord[5] + ":" + edRecord[6];
         break;

      case OldExtraDataUtility::ADD_SIGNPOST :
      case OldExtraDataUtility::REMOVE_SIGNPOST :
         retVal = edRecord[5] + ":" + edRecord[6] + ":" + edRecord[7];
         break;
      
      case OldExtraDataUtility::COMMENT:
      case OldExtraDataUtility::ADD_CITYPART :
      case OldExtraDataUtility::ADD_LANDMARK:
      case OldExtraDataUtility::ADD_SINGLE_LANDMARK:
      default :
         // nothing
         break;
   }
   return retVal;
}

bool
MEShowExtradataWindow::recordFitsThisMap(vector<MC2String> edRecord)
{
   bool retVal = false;
   
   // Get first coord pair of this record
   // (does not have to be the identifying coord)
   uint32 firstCoordPos = 0;
   if (getCoordsForRecord(edRecord, firstCoordPos) > 0) {
      int32 lat, lon;
      if (OldExtraDataUtility::strtolatlon(edRecord[firstCoordPos],
               lat, lon, CoordinateTransformer::mc2)) {
         // We have a lat and lon
         // Check if it is close to any item

         OldGenericMap* curMap = m_mapArea->getMap();
         MC2BoundingBox bbox;
         curMap->getGfxData()->getMC2BoundingBox(bbox);
         if (bbox.contains(lat, lon)) {
         
            uint64 dist;
            OldMapHashTable* mht = curMap->getHashTable();
            mht->clearAllowedItemTypes();
            // do we want all item types allowed?
            // NO NOT zipCodes, they can be bounding boxes outside map..
            mht->addAllowedItemType(ItemTypes::streetSegmentItem);
            mht->addAllowedItemType(ItemTypes::ferryItem);
            mht->addAllowedItemType(ItemTypes::islandItem);
            // add buas, if we are in an underview map and
            // not dealing with index areas
            if ( (MapBits::isUnderviewMap(curMap->getMapID())) &&
                 ! NationalProperties::useIndexAreas(
                  curMap->getCountryCode(), curMap->getMapOrigin()) ) {
               mht->addAllowedItemType(ItemTypes::builtUpAreaItem);
               mc2dbg8 << "allow bua" << endl;
            }

            if (!MapBits::isUnderviewMap(curMap->getMapID())) {
               // We are looking at a overview or country overview, add
               // also municipals since ssi+ferry do not cover all areas.
               mht->addAllowedItemType(ItemTypes::municipalItem);
            }
            uint32 closestID = mht->getClosest(lon, lat, dist);
            mc2dbg4 << " Found closest item "
                    << curMap->getFirstItemName(closestID) << " in "
                    << sqrt(dist) * GfxConstants::MC2SCALE_TO_METER 
                    << " meters " << endl;
            if ((sqrt(dist) * GfxConstants::MC2SCALE_TO_METER) < 500) {
               // one item is close enough
               retVal = true;
            }
         }

      }
   }

   return retVal;
}

MC2String
MEShowExtradataWindow::getWGS84stringFromMC2CoordString(
                                 MC2String mc2CoordString)
{
   MC2String wgs84String = "(wgs84)";

   int32 mc2lat, mc2lon;
   bool coordOK = OldExtraDataUtility::strtolatlon(
                     mc2CoordString, mc2lat, mc2lon, 
                     CoordinateTransformer::mc2);
   if (coordOK) {
      float64 wgslat, wgslon, wgsh;
      CoordinateTransformer::transformFromMC2(
            mc2lat, mc2lon,
            CoordinateTransformer::wgs84deg,
            wgslat, wgslon, wgsh);
      char wgsStr[50];
      sprintf(wgsStr, "(%.7f, %.7f)", wgslat, wgslon);
      wgs84String = wgsStr;
   }

   return wgs84String;
}

