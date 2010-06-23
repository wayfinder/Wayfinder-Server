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
#include "MEMapEditorWindow.h" 
#include "MEDrawSettingsDialog.h"
#include "MEEditNameDialog.h"
#include "METurnRestrictionsDialog.h"
#include "MEMessageDialog.h"
#include "MEShowNodesWindow.h"
#include "MEItemInfoDialog.h"
#include "MEShowExtradataWindow.h"
#include "OldStreetItem.h"
#include "GfxConstants.h"
#include "GDFRef.h"
#include "MapGenUtil.h"
#include "CharEncoding.h"
#include "MapBits.h"

      
MEMapEditorWindow::MEMapEditorWindow(OldGenericMap* map, 
                                     MC2String gdfRefDir) 
    : Gtk::Window(Gtk::WINDOW_TOPLEVEL) 
{


   // Fixme: finalise this if you want some other font in the MapEditor
   // Select a new font
   //Pango::FontDescription f("Serif 20");
   //f->set_family("Courier New");
   //modify_font(f);

   //Glib::RefPtr<Pango::Context> pangoContext = get_pango_context();
   //std::cout << pangoContext->get_font_description().get_family()
   //          << endl;
   //pangoContext->get_font_description().set_family("Courier New");
   //pangoContext->set_font_description()
   //Pango::FontDescription* f = new Pango::FontDescription();
   //f->get_style().set_font(*(new Pango::FontDescription("Courier New")));
   //Pango::FontDescription f = get_style()->get_font();
   //std::cout << "font" << f.to_string() << endl;
   /*
   m_fontSelection = manage(new Gtk::FontSelection());
   add(*m_fontSelection); 
   m_fontSelection->set_font_name("Times New Roman");
   */

   // Handle gdfRef loading.   
   GDFRef* gdfRef = NULL; 
   //
   // Get the gdf ref file name.
   MC2String mapFileName = map->getFilename();
   uint32 pos = mapFileName.rfind("/");
   MC2String onlyMapFileName = mapFileName.substr(pos+1);
   pos = onlyMapFileName.rfind(".bz2");
   onlyMapFileName = onlyMapFileName.substr(0, pos);
   pos = onlyMapFileName.rfind(".gz");
   onlyMapFileName = onlyMapFileName.substr(0, pos);
   MC2String gdfRefFileName = onlyMapFileName + MC2String(".gdf_ref");
  
   // Set the full path together.
   // The gdf ref file can be found in the temp-dir of a finished map gen
   MC2String fullPath;
   if (gdfRefDir == "_FIND_") {
      if (MapGenUtil::fileExists("./" + gdfRefFileName) ){
         fullPath =  "./" + gdfRefFileName;
      }
      else if (MapGenUtil::fileExists("./temp/" + gdfRefFileName) ){
         fullPath = "./temp/" + gdfRefFileName;
      }
      else if (MapGenUtil::fileExists("../temp/" + gdfRefFileName) ){
         fullPath = "../temp/" + gdfRefFileName;
      }
   }
   else if ( gdfRefDir.size() > 0 ){
      fullPath = gdfRefDir + MC2String("/") + gdfRefFileName;
   }
   if ( fullPath.size() > 0 ){
      gdfRef = new GDFRef();
      gdfRef->readGDFRefFile(fullPath);
   }

   char foo[16];
   sprintf(foo, "%d", map->getMapID());
   string titleString;
   titleString.append(foo);
   titleString += " (";
   titleString += map->getMapName();
   titleString += ", ";
   titleString += map->getMapOrigin();
   titleString += ", ";
   titleString += map->getFilename();
   titleString += ")";
   mc2dbg4 << "TITLE: " << titleString << endl;
   set_title(titleString);
   
   // Some variables...
   Gtk::HSeparator* sep;
   Gtk::VBox* tmpVbox;
   Gtk::Label* label;
   Gtk::HBox* tmp_hbox;
   Gtk::Button* button = NULL; 

   // Create the boxes at the top and right-side
   Gtk::Box* main_box = manage(new Gtk::VBox());
   add(*main_box);
   

   Gtk::Box* topmenu = manage(new Gtk::HBox());
   main_box->pack_start(*topmenu, false, true);

   // Populate topmenu
   m_hideBtn = manage(new Gtk::ToggleButton("Hide menu"));
   m_hideBtn->signal_clicked().connect(
      sigc::mem_fun(*this, &MEMapEditorWindow::on_hideBtnClicked));
   topmenu->pack_start(*m_hideBtn, false, false);

   // MEMapArea + right menu in tmp_hbox
   m_filterLevel = 0; // default, show all coordinates
   tmp_hbox = manage( new Gtk::HBox() );
   m_mapArea = manage ( new MEMapArea(map, m_filterLevel) );
   m_mapArea->setGdfRef(gdfRef);
   m_map = map;
   tmp_hbox->pack_start(*m_mapArea);
   m_rightMenu = manage(new Gtk::VBox(FALSE, 10));
   tmp_hbox->pack_start(*m_rightMenu, false, true);
   main_box->pack_start(*tmp_hbox);

   // Add map-version
   tmpVbox = manage(new Gtk::VBox());
   sep = manage(new Gtk::HSeparator());
   tmpVbox->pack_start(*sep);
   char tmpStr[256];
   sprintf(tmpStr, "MapID: %u", map->getMapID());
   label = manage(new Gtk::Label(tmpStr));
   tmpVbox->pack_start(*label);

   // creation time (last saved)
   time_t creationTime = time_t(map->getCreationTime());
   char timebuff[1024];
   sprintf(timebuff, "%04d-%02d-%02d %02d:%02d:%02d",
           localtime(&creationTime)->tm_year + 1900,
           localtime(&creationTime)->tm_mon + 1,
           localtime(&creationTime)->tm_mday,
           localtime(&creationTime)->tm_hour,
           localtime(&creationTime)->tm_min,
           localtime(&creationTime)->tm_sec);
   sprintf(tmpStr, "Mapversion: %u, %s",
           map->getCreationTime(), timebuff);
   label = manage(new Gtk::Label(tmpStr));
   tmpVbox->pack_start(*label);

   // true creation time
   time_t trueCreationTime = time_t(map->getTrueCreationTime());
   sprintf(timebuff, "%04d-%02d-%02d %02d:%02d:%02d",
           localtime(&trueCreationTime)->tm_year + 1900,
           localtime(&trueCreationTime)->tm_mon + 1,
           localtime(&trueCreationTime)->tm_mday,
           localtime(&trueCreationTime)->tm_hour,
           localtime(&trueCreationTime)->tm_min,
           localtime(&trueCreationTime)->tm_sec);
   sprintf(tmpStr, "True creation time: %s", timebuff);
   label = manage(new Gtk::Label(tmpStr));
   tmpVbox->pack_start(*label);

   // latest wasp time
   if (map->getWaspTime() == MAX_UINT32) {
      sprintf(timebuff, "%s", "no wasping");
   } else {
      time_t waspTime = time_t(map->getWaspTime());
      sprintf(timebuff, "%04d-%02d-%02d %02d:%02d:%02d",
              localtime(&waspTime)->tm_year + 1900,
              localtime(&waspTime)->tm_mon + 1,
              localtime(&waspTime)->tm_mday,
              localtime(&waspTime)->tm_hour,
              localtime(&waspTime)->tm_min,
              localtime(&waspTime)->tm_sec);
   }
   sprintf(tmpStr, "Latest WASPing time: %s", timebuff);
   label = manage(new Gtk::Label(tmpStr));
   tmpVbox->pack_start(*label);

   // latest dynamic ed time
   if (map->getDynamicExtradataTime() == MAX_UINT32) {
      sprintf(timebuff, "%s", "no dynamic");
   } else {
      time_t edTime = time_t(map->getDynamicExtradataTime());
      sprintf(timebuff, "%04d-%02d-%02d %02d:%02d:%02d",
              localtime(&edTime)->tm_year + 1900,
              localtime(&edTime)->tm_mon + 1,
              localtime(&edTime)->tm_mday,
              localtime(&edTime)->tm_hour,
              localtime(&edTime)->tm_min,
              localtime(&edTime)->tm_sec);
   }
   sprintf(tmpStr, "Latest dynamic ed time: %s", timebuff);
   label = manage(new Gtk::Label(tmpStr));
   tmpVbox->pack_start(*label);
   sep = manage(new Gtk::HSeparator());
   tmpVbox->pack_start(*sep);
   m_rightMenu->pack_start(*tmpVbox);
  
 
   // Misc functions
   
   // Load an extradata file (including comment rows) to analyse were the
   // extradata corrections fit. E.g. possible to highlight the coordinates
   // involved in the ed records, and zooming to the ed record coords.
   button = manage(new Gtk::Button("Show extradata"));
   button->signal_clicked().connect(
         sigc::mem_fun(*this, &MEMapEditorWindow::showExtradataPressed));
   m_rightMenu->pack_start(*button, false, false);

   // Load a file with result from MapTool --compturns option
   // will load the rows starting with TURNDESCRIPTION:
   tmp_hbox = manage(new Gtk::HBox(FALSE, 10));
   button = manage(new Gtk::Button("Show nodes"));
   button->signal_clicked().connect(
            sigc::mem_fun(*this, &MEMapEditorWindow::showNodesPressed));          
   tmp_hbox->pack_start(*button);
   m_rightMenu->pack_start(*tmp_hbox, false, false);

   // Add an entry and a buttons to be able to search for 
   // and zoom to an ID (itemID, waspID)
   sep = manage(new Gtk::HSeparator());
   m_rightMenu->pack_start(*sep, false, false);
   label = manage(new Gtk::Label("Search for item with ID"));
   m_searchIDTypeCombo = manage(new Gtk::Combo());
   m_searchIDType = m_searchIDTypeCombo->get_entry();
   list<string> IDTypeList;
   IDTypeList.push_back("itemID");
   IDTypeList.push_back("waspID");
   IDTypeList.push_back("item name");
   if ( m_mapArea->loadedGdfRef() ){
      IDTypeList.push_back("gdfID");
   }
   m_searchIDTypeCombo->set_popdown_strings(IDTypeList);
   m_searchIDType->set_text("itemID");
   tmp_hbox = manage(new Gtk::HBox(FALSE, 10));
   tmp_hbox->pack_start(*label);
   tmp_hbox->pack_start(*m_searchIDTypeCombo);
   m_rightMenu->pack_start(*tmp_hbox, false, false);

   tmp_hbox = manage(new Gtk::HBox(FALSE, 10));
   m_searchIDEntry = manage(new Gtk::Entry());
   m_searchIDEntry->signal_focus_out_event().connect(
         sigc::mem_fun(*this, &MEMapEditorWindow::focus_out_event));
   tmp_hbox->pack_start(*m_searchIDEntry);
   button = manage(new Gtk::Button("Paste"));
   button->signal_clicked().connect( sigc::bind<Gtk::Entry*> (
         sigc::mem_fun(*this, &MEMapEditorWindow::pastePressed ), 
         m_searchIDEntry ) );
   tmp_hbox->pack_start(*button);
   button = manage(new Gtk::Button("Search"));
   button->signal_clicked().connect(
         sigc::mem_fun(*this, &MEMapEditorWindow::searchIDPressed));
   tmp_hbox->pack_start( *button );
   button = manage(new Gtk::Button("Zoom"));
   button->signal_clicked().connect(
         sigc::mem_fun(*this, &MEMapEditorWindow::zoomToIDPressed));
   tmp_hbox->pack_start( *button );
   m_rightMenu->pack_start(*tmp_hbox, false, false);

   // Add coord entries and a button to be able to search for a coordinate,
   // as well as one for zooming to the coordinate
   label = manage(
      new Gtk::Label("Search coordinate (lat, lon), mc2 or wgs84_deg"));
   m_rightMenu->pack_start(*label, false, false);
   tmp_hbox = manage(new Gtk::HBox(FALSE, 10));
   m_searchLatEntry = manage(new Gtk::Entry());
   m_searchLatEntry->set_max_length(15);
   m_searchLatEntry->set_size_request(70, 20);
   tmp_hbox->pack_start(*m_searchLatEntry);
   m_searchLonEntry = manage(new Gtk::Entry());
   m_searchLonEntry->set_max_length(15);
   m_searchLonEntry->set_size_request(70, 20);
   tmp_hbox->pack_start(*m_searchLonEntry);
   button = manage(new Gtk::Button("Search"));
   button->signal_clicked().connect(
         sigc::mem_fun(*this, &MEMapEditorWindow::searchCoordinatePressed));
   tmp_hbox->pack_start(*button);
   m_rightMenu->pack_start(*tmp_hbox, false, false);

    
   // Add paste buttons to paste content from clip board
   tmp_hbox = manage(new Gtk::HBox(FALSE, 10));
   button = manage(new Gtk::Button("Paste Lat"));
   button->signal_clicked().connect( sigc::bind<Gtk::Entry*> (
         sigc::mem_fun(*this, &MEMapEditorWindow::pastePressed ), 
         m_searchLatEntry ) );
   tmp_hbox->pack_start(*button);
   button = manage(new Gtk::Button("Paste Lon"));
   button->signal_clicked().connect( sigc::bind<Gtk::Entry*> (
         sigc::mem_fun(*this, &MEMapEditorWindow::pastePressed ), 
         m_searchLonEntry ) );
   tmp_hbox->pack_start(*button);
   button = manage(new Gtk::Button("Zoom"));
   button->signal_clicked().connect(
         sigc::mem_fun(*this, &MEMapEditorWindow::zoomToCoordinatePressed));

   tmp_hbox->pack_start(*button);
   m_rightMenu->pack_start(*tmp_hbox, false, false);
 
   // Add a filter level field and button
   sep = manage(new Gtk::HSeparator());
   m_rightMenu->pack_start(*sep, false, false);
   tmp_hbox = manage(new Gtk::HBox(FALSE, 10));
   label = manage(new Gtk::Label("Filter level"));
   tmp_hbox->pack_start(*label, false, false);
   m_filterLevelIn = manage(new Gtk::Entry());
   m_filterLevelIn->set_max_length(2);
   m_filterLevelIn->set_size_request(10, 20);
   tmp_hbox->pack_start(*m_filterLevelIn);
   button = manage(new Gtk::Button("Update"));
   button->signal_clicked().connect(
      sigc::mem_fun(*this, &MEMapEditorWindow::filterLevelPressed));
   tmp_hbox->pack_start(*button);
   m_rightMenu->pack_start(*tmp_hbox, false, false);

   // Add highlight buttons, clear + re-highlight
   sep = manage(new Gtk::HSeparator());
   m_rightMenu->pack_start(*sep, false, false);
   tmp_hbox = manage(new Gtk::HBox(FALSE, 10));
   button = manage(new Gtk::Button("Clear all highlight"));
   button->signal_clicked().connect(
         sigc::mem_fun(*this, &MEMapEditorWindow::clearHighlightPressed));
   tmp_hbox->pack_start(*button);
   button = manage(new Gtk::Button("Re-highlight ids"));
   button->signal_clicked().connect(
         sigc::mem_fun(*this, &MEMapEditorWindow::highlightLoadedData));

   tmp_hbox->pack_start(*button);
   m_rightMenu->pack_start(*tmp_hbox, false, false);

   // Button for deciding whether to keep highlights on selecting or not
   m_keepHighlightsOnSelectBtn =
      manage(new Gtk::ToggleButton("Keep highlights on select"));
   m_keepHighlightsOnSelectBtn->signal_clicked().connect(
      sigc::mem_fun(*this, 
                    &MEMapEditorWindow::on_keepHighlightsOnSelectBtnClicked));
   m_rightMenu->pack_start(*m_keepHighlightsOnSelectBtn, false, false);
   
   // Add a show draw-settings button
   button = manage(new Gtk::Button("Drawsettings"));
   button->signal_clicked().connect(
         sigc::mem_fun(*this, &MEMapEditorWindow::drawSettingsPressed));
   m_rightMenu->pack_start(*button, false, true, 5);

   // Pack Quit-button and logfile-entry + save-button from end.
   // Add a Quit-button
   button = manage(new Gtk::Button("Quit"));
   button->signal_clicked().connect(sigc::ptr_fun(&Gtk::Main::quit));
   m_rightMenu->pack_end(*button, false, true, 10);

   set_events (get_events() & Gdk::KEY_PRESS_MASK);


#ifdef MAP_EDITABLE
   // Add an entry for the name of the logfile
   tmpVbox = manage(new Gtk::VBox(FALSE, 10));
   sep = manage(new Gtk::HSeparator());
   tmpVbox->pack_start(*sep);

   label = manage(new Gtk::Label("Extradata log filename"));
   m_logFilenameEntry= manage(new Gtk::Entry());
   m_logFilenameEntry->set_text("changelog.ext");
   tmp_hbox = manage(new Gtk::HBox(FALSE, 10));
   tmp_hbox->pack_start(*label);
   tmp_hbox->pack_start(*m_logFilenameEntry);
   tmpVbox->pack_start(*tmp_hbox);

   // Save button, saves the map
   label = manage(new Gtk::Label("Save the map carefully"));
   button = manage(new Gtk::Button("Save"));
   button->signal_clicked().connect(
      sigc::mem_fun(*this, &MEMapEditorWindow::savePressed));
   tmp_hbox = manage(new Gtk::HBox(FALSE, 10));
   tmp_hbox->pack_start(*label);
   tmp_hbox->pack_start(*button);
   label = manage(new Gtk::Label("                "));
   tmp_hbox->pack_start(*label);
   tmpVbox->pack_start(*tmp_hbox);
   //m_rightMenu->pack_end(*tmpVbox, false, true, 5);
   m_rightMenu->pack_end(*tmpVbox, false, false);
#endif

   show_all();
}

MEMapEditorWindow::~MEMapEditorWindow()
{
   // Delete all 'one instance' objects
   MEShowExtradataWindow::deleteInstance();
   MEEditNameDialog::deleteInstance();
   MEItemInfoDialog::deleteInstance();
   MEDrawSettingsDialog::deleteInstance();
   METurnRestrictionsDialog::deleteInstance();
}

bool
MEMapEditorWindow::focus_out_event(GdkEventFocus* f)
{
   Gtk::Widget* widgetInFocus = get_focus();
   if (widgetInFocus != NULL) {
      ((Gtk::Editable*)widgetInFocus)->select_region(0, 10);
   }

   return true;
}

bool
MEMapEditorWindow::on_key_press_event(GdkEventKey* p0)
{
   mc2dbg8 << "key pressed: [" << p0->string << "]" << endl;
   gint i = Gtk::Window::on_key_press_event(p0);
   mc2dbg8 << "Gtk::Window::key_press_event_impl() returned " << i << endl;
   if (0 == i) {
      if ((p0->string[0] == 'd') || (p0->string[0] == 'D'))
      {
         drawSettingsPressed();
         i = 1;
      }
      if ((p0->string[0] == 'q') || (p0->string[0] == 'Q'))
      {
         Gtk::Main::quit();
         i = 1;
      }

   }
   return i;
}

void
MEMapEditorWindow::on_hideBtnClicked()
{
   if (m_hideBtn->get_active()) 
      m_rightMenu->hide();
   else 
      m_rightMenu->show();
}

void 
MEMapEditorWindow::drawSettingsPressed() 
{
   MEDrawSettingsDialog::instance(m_mapArea)->show();
}

void 
MEMapEditorWindow::showNodesPressed() 
{ 
   mc2dbg8 << "showing MEShowNodesWindow" << endl;
   MEShowNodesWindow::instance(m_mapArea)->show(); 
}

void 
MEMapEditorWindow::showExtradataPressed() 
{
   mc2dbg8 << "Show ShowExtradataWindow!" << endl;
   MEShowExtradataWindow::instance(m_mapArea)->show();
}

void
MEMapEditorWindow::searchIDPressed()
{
   mc2dbg8 << "searchIDPressed search for " << m_searchIDType->get_text()
           << " id=" << m_searchIDEntry->get_text() << endl;

   // Use searchItemIDPressed and searchPoiWaspIDPressed etc methods
   if ( strcmp(m_searchIDType->get_text().c_str(), "itemID") == 0 ) {
      searchItemIDPressed();
   }
   else if ( strcmp(m_searchIDType->get_text().c_str(), "waspID") == 0 ) {
      searchPoiWaspIDPressed();
   }
   else if ( strcmp(m_searchIDType->get_text().c_str(), "gdfID") == 0 ) {
      searchGdfIDIDPressed();
   }
   else if ( strcmp(m_searchIDType->get_text().c_str(), "item name") == 0 ) {
      searchItemNamePressed();
   }
   else {
      mc2dbg << "Searching for id type " << m_searchIDType->get_text()
             << " not implemeneted." << endl;
   }
}

void
MEMapEditorWindow::pastePressed( Gtk::Entry* entry )
{
   Glib::RefPtr<Gtk::Clipboard> refClipboard = 
         Gtk::Clipboard::get( GDK_SELECTION_PRIMARY );
   refClipboard->request_text( sigc::bind<Gtk::Entry*> (sigc::mem_fun(*this,
              &MEMapEditorWindow::on_clipboardTextReceived ), entry ) );

}

void
MEMapEditorWindow::on_clipboardTextReceived( const Glib::ustring& text, 
                                             Gtk::Entry* entry)
{
   entry->set_text( text );
}



void
MEMapEditorWindow::zoomToIDPressed()
{
   mc2dbg8 << "zoomToIDPressed zoom to " << m_searchIDType->get_text()
           << " id=" << m_searchIDEntry->get_text() << endl;

   if ( strcmp(m_searchIDType->get_text().c_str(), "itemID") == 0 ) {
      zoomToItemPressed();
   }
   else if ( strcmp(m_searchIDType->get_text().c_str(), "waspID") == 0 ) {
      zoomToPoiPressed();
   }
   else if ( strcmp(m_searchIDType->get_text().c_str(), "gdfID") == 0 ) {
      zoomToGdfIdPressed();
   }
   else if ( strcmp(m_searchIDType->get_text().c_str(), "item name") == 0 ) {
      mc2log << error << "Not possible to zoom to item names." 
             << endl;
      // Fixme: implement zoomToItemNamePressed
   }
   else {
      mc2dbg << "Zooming to id type " << m_searchIDType->get_text()
             << " not implemeneted." << endl;
   }
}

void 
MEMapEditorWindow::searchItemIDPressed() 
{
   const char* idString = m_searchIDEntry->get_text().c_str();
   if ((idString != NULL) && (strlen(idString) > 0)) {
      bool clearFirst = true;
      uint32 itemID = (strtoul(idString, NULL, 0));
      itemID = REMOVE_UINT32_MSB(itemID);
      mc2dbg4 << "To show item with ID=" << itemID << endl;
      if (!m_mapArea->highlightItem(itemID, clearFirst)) {
         mc2log << warn << here << " FAILED, did not find item with ID == " 
                << itemID << " among the drawn item types" << endl;
      }
      
      // Display info about the item
      OldItem* item = m_map->itemLookup(itemID);
      if (item != NULL) {
         const char* itemName = "-no name-";
         if ( item->getNbrNames() > 0 ) {
            itemName =
               StringUtility::newStrDup(m_map->getFirstItemName(item));
         }
         mc2log << info << "Found \"" << itemName << "\" (" 
                << StringTable::getString( 
                   ItemTypes::getItemTypeSC(item->getItemType()),
                   StringTable::ENGLISH)
                << " with ID=" << itemID << ")" << endl;
         
         // Show item-info, SSI is a bit special...
         MEItemInfoDialog* dialog = MEItemInfoDialog::instance(m_mapArea);
         dialog->setItemToShow(item, m_mapArea->getMap());
      }
   }
}

void
MEMapEditorWindow::zoomToItemPressed()
{
   // get the item, copied from searchItemIDPressed
   const char* idString = m_searchIDEntry->get_text().c_str();
   if ((idString != NULL) && (strlen(idString) > 0)) {
      uint32 itemID = (strtoul(idString, NULL, 0));
      itemID = REMOVE_UINT32_MSB(itemID);
      mc2dbg4 << "Zoom to item with ID=" << itemID << endl;
      zoomToItem(itemID);
   }
}

void
MEMapEditorWindow::zoomToItem(uint32 itemID){
   if (m_mapArea->itemAmongShownLayers(itemID)) {
      OldItem* item = m_map->itemLookup(itemID);
      GfxData* gfx = item->getGfxData();
      
      if ((gfx != NULL) &&
          (gfx->getTotalNbrCoordinates() > 0)) {
         // Build itembbox from the gfx
         MC2BoundingBox itembbox;
         for (uint16 p=0; p<gfx->getNbrPolygons(); ++p) {
            GfxData::const_filter_iterator polyEnd = 
               gfx->endFilteredPoly(p,
                                    m_filterLevel);
            for( GfxData::const_filter_iterator it =
                    gfx->beginFilteredPoly(p, m_filterLevel);
                 it != polyEnd; ++it) {
               MC2Coordinate currCoord = *it;
               itembbox.update(currCoord.lat, currCoord.lon);
            }
         }
         const int32 delta = 100000;
         m_mapArea->zoomToBBox( itembbox.getMaxLat()+delta, 
                                itembbox.getMinLon()-delta,
                               itembbox.getMinLat()-delta, 
                                itembbox.getMaxLon()+delta );
      } else if (item->getItemType() == ItemTypes::streetItem){
         // Build itembbox from contained street segment items.
         OldStreetItem* si = dynamic_cast<OldStreetItem*>(item);
         MC2BoundingBox itembbox;
         for (uint32 j = 0; j < si->getNbrItemsInGroup(); j++) {
            OldItem* ssi = m_map->itemLookup(si->getItemNumber(j));
            if ((ssi != NULL) && 
                (ssi->getItemType() == ItemTypes::streetSegmentItem) &&
                (ssi->getGfxData() != NULL)) {
               GfxData* ssigfx = ssi->getGfxData();
               for (uint16 p=0; p<ssigfx->getNbrPolygons(); ++p) {
                  GfxData::const_filter_iterator polyEnd =
                     ssigfx->endFilteredPoly(p,
                                             m_filterLevel);
                  for(GfxData::const_filter_iterator it =
                         ssigfx->beginFilteredPoly(p,
                                                   m_filterLevel);
                      it != polyEnd; ++it) {
                     MC2Coordinate currCoord = *it;
                     itembbox.update(currCoord.lat, currCoord.lon);
                  }
               }
            }
         }
         const int32 delta = 100000;
         m_mapArea->zoomToBBox( itembbox.getMaxLat()+delta, 
                                itembbox.getMinLon()-delta,
                                itembbox.getMinLat()-delta, 
                                itembbox.getMaxLon()+delta );
      } else {
         // If poi try to get coordinate from offset on ssi
         // Can't zoom to item with no coordinates
         mc2dbg << "Can't zoom to item - it has no gfx data." << endl;
      }
   }
}

bool
MEMapEditorWindow::getMc2Coordinates(
      char* latString, char* lonString, int32& mc2lat, int32& mc2lon )
{
   bool retVal = true;
   if ( (latString != NULL) && (strlen(latString) > 0) &&
        (lonString != NULL) && (strlen(lonString) > 0)) {
      mc2lat = strtol(latString, NULL, 10);
      mc2lon = strtol(lonString, NULL, 10);
      mc2dbg8 << " lat " << mc2lat << " lon " << mc2lon << endl;
      if ( (abs(mc2lat) < 91) && (abs(mc2lon) < 181) ) {
         float64 wgslat = strtod(latString, NULL);
         float64 wgslon = strtod(lonString, NULL);
         mc2lat = int32(wgslat * GfxConstants::degreeFactor);
         mc2lon = int32(wgslon * GfxConstants::degreeFactor);
         mc2dbg << "Converted wgs84 (" << wgslat << "," << wgslon
                << ") to mc2 (" << mc2lat << "," << mc2lon << ")" << endl;
      }
      // else we have mc2
      
   } else {
      retVal = false;
      mc2dbg << "getMc2Coordinates no good coordinate strings," 
             << " latString=" << latString  << " lonString="
             << lonString << endl;
   }
   mc2dbg8 << " getMc2Coordinates returns " << int(retVal)
           << " (" << mc2lat << "," << mc2lon << ")" << endl;

   return retVal;
}

void 
MEMapEditorWindow::searchCoordinatePressed() 
{
   char* latString = 
      StringUtility::newStrDup(m_searchLatEntry->get_text().c_str());
   char* lonString = 
      StringUtility::newStrDup(m_searchLonEntry->get_text().c_str());
   mc2dbg4 << "search for coord (" << latString << ","
           << lonString << ")" << endl;
   
   int32 lat, lon;
   if ( getMc2Coordinates(latString, lonString, lat, lon) ) {
      MEGdkColors* cols = MEGdkColors::getColors();
      m_mapArea->highlightCoordinate(lat, lon, cols->m_red);
   }

   delete latString;
   delete lonString;
}

void 
MEMapEditorWindow::zoomToCoordinatePressed() 
{
   char* latString = 
      StringUtility::newStrDup(m_searchLatEntry->get_text().c_str());
   char* lonString = 
      StringUtility::newStrDup(m_searchLonEntry->get_text().c_str());
   mc2dbg4 << "zooming to coord (" << latString << ","
           << lonString << ")" << endl;
   
   int32 lat, lon;
   if ( getMc2Coordinates(latString, lonString, lat, lon) ) {
      // Make sure that the coordinate is inside the BBox for the map
      const GfxData* mapGfx = m_map->getGfxData();
      MC2BoundingBox bbox;
      mapGfx->getMC2BoundingBox(bbox);
      if (bbox.contains(lat, lon)) {
         mc2dbg4 << "mapbbox contains the coordinate (" << lat
                 << "," << lon << ")" << endl;
         const int32 delta = 60000;
         m_mapArea->zoomToBBox( lat+delta, lon-delta, 
                                lat-delta, lon+delta );
      } else {
         mc2dbg2 << "mapbbox does not contain the coordinate (" 
                 << lat << "," << lon << ")" << endl;
      }
   }

   delete latString;
   delete lonString;
}

void 
MEMapEditorWindow::searchPoiWaspIDPressed() 
{
   const char* idString = m_searchIDEntry->get_text().c_str();
   if ((idString != NULL) && (strlen(idString) > 0)) {
      bool clearFirst = true;
      uint32 waspID = (strtoul(idString, NULL, 0));
      mc2dbg4 << "To show poi with waspId = " << waspID << endl;

      // Find the poi with this waspID
      OldPointOfInterestItem* curPoi = NULL;
      uint32 z = 0;
      while ((curPoi == NULL) && (z < NUMBER_GFX_ZOOMLEVELS)) {
         uint32 i = 0;
         while ((curPoi == NULL) && (i < m_map->getNbrItemsWithZoom(z))) {
            OldPointOfInterestItem* poi = 
               static_cast<OldPointOfInterestItem*>(m_map->getItem(z, i));
            if ((poi != NULL) &&
                (poi->getWASPID() == waspID)) {
               // found the poi we were looking for.
               curPoi = poi;
            }
            i++;
         }
         z++;
      }
      
      if (curPoi != NULL) {
         uint32 poiId = curPoi->getID();
      
         if (!m_mapArea->highlightItem(poiId, clearFirst)) {
            mc2log << error << warn << "FAILED to find poi with waspId "
                   << waspID << " (poiId " << poiId 
                   << ") among the drawn item types" << endl;
         } 

         // Display info about the poi
         const char* itemName = "-no name-";
         if ( curPoi->getNbrNames() > 0 ) {
            itemName = StringUtility::newStrDup(
                         m_map->getFirstItemName(curPoi));
         }
         mc2log << info << "Found \"" << itemName << "\" (" 
                << StringTable::getString(
                     ItemTypes::getItemTypeSC(curPoi->getItemType()),
                     StringTable::ENGLISH) << " with ID=" 
                << poiId << ")" << endl;
         
         // Show item-info, SSI is a bit special...
         MEItemInfoDialog* dialog = MEItemInfoDialog::instance(m_mapArea);
         dialog->setItemToShow(curPoi, m_mapArea->getMap());

      } else {
         mc2dbg1 << "Poi with waspId " << waspID << " is not located "
                 << "in this map" << endl;
      }
   } 
}

void 
MEMapEditorWindow::searchGdfIDIDPressed()
{
   const char* idString = m_searchIDEntry->get_text().c_str();
   if ((idString != NULL) && (strlen(idString) > 0)) {
      uint32 gdfID = (strtoul(idString, NULL, 0));

      // Find the items with this GDF ID and highlight them
      uint32 singleItemID = m_mapArea->highlightFromGdfID(gdfID);

      // If one was found, select it
      if ( singleItemID != MAX_UINT32 ){
         OldItem* item = m_map->itemLookup(singleItemID);
         if (item != NULL ){
            // Show item-info
            MEItemInfoDialog* dialog = 
               MEItemInfoDialog::instance(m_mapArea);
            dialog->setItemToShow(item, m_mapArea->getMap());
         }
         else {
            mc2log << error << "Item ID: " << singleItemID << " is NULL!"
                   << endl;
         }
      }
      else {
         mc2dbg << "No unique item for GDF ID: " << gdfID << endl;
      }
   }
}

void
MEMapEditorWindow::zoomToPoiPressed()
{
   // get the poi, copied from searchPoiWaspIDPressed
   const char* idString = m_searchIDEntry->get_text().c_str();
   if ((idString != NULL) && (strlen(idString) > 0)) {
      uint32 waspID = (strtoul(idString, NULL, 0));
      mc2dbg4 << "Zoom to poi with waspId = " << waspID << endl;

      // Find the poi with this waspID
      OldPointOfInterestItem* curPoi = NULL;
      uint32 z = 0;
      while ((curPoi == NULL) && (z < NUMBER_GFX_ZOOMLEVELS)) {
         uint32 i = 0;
         while ((curPoi == NULL) && (i < m_map->getNbrItemsWithZoom(z))) {
            OldPointOfInterestItem* poi = 
               static_cast<OldPointOfInterestItem*>(m_map->getItem(z, i));
            if ((poi != NULL) &&
                (poi->getWASPID() == waspID)) {
               // found the poi we were looking for.
               curPoi = poi;
            }
            i++;
         }
         z++;
      }
      
      if (curPoi != NULL) {
         uint32 poiId = curPoi->getID();
   
         // Zoom to the poi
         mc2dbg4 << "Zoom to poi with id=" << poiId << endl;
         if (m_mapArea->itemAmongShownLayers(poiId)) {
            GfxData* gfx = curPoi->getGfxData();
            MC2BoundingBox itembbox;
            bool zoomToPoi = false;
           
            if ((gfx != NULL) &&
                (gfx->getTotalNbrCoordinates() > 0)) {
               // Build itembbox from the gfx
               for (uint16 p=0; p<gfx->getNbrPolygons(); ++p) {
                  GfxData::const_filter_iterator polyEnd =
                     gfx->endFilteredPoly(p, m_filterLevel);
                  for(GfxData::const_filter_iterator it =
                         gfx->beginFilteredPoly(p, m_filterLevel);
                      it != polyEnd; ++it) {
                     MC2Coordinate currCoord = *it;
                     itembbox.update(currCoord.lat, currCoord.lon);
                  }
               }
               zoomToPoi = true;
               
            } else {
               // Try to get coordinate from offset on ssi
               // The ssiId and offset is only valid in underview maps
               mc2dbg1 << "Poi " << poiId << " has no gfxData, find "
                       << "coordinate using offset on ssi" << endl;
               if (MapBits::isUnderviewMap(m_map->getMapID())) {
                  OldItem* ssi = m_map->itemLookup(
                                 curPoi->getStreetSegmentItemID());
                  if ((ssi != NULL)  && (ssi->getGfxData() != NULL)) {
                     GfxData* ssiGfx = ssi->getGfxData();
                     int32 lat, lon;
                     if (ssiGfx->getCoordinate(
                              curPoi->getOffsetOnStreet(), lat, lon)) {
                        itembbox.update(lat, lon);
                        zoomToPoi = true;
                     }
                  }
               }
               
               // Can't zoom to poi with no coordinates
               if (!zoomToPoi)
                  mc2dbg << "Can't zoom to poi - it has no gfx data." << endl;
            }

            if (zoomToPoi) {
               const int32 delta = 100000;
               m_mapArea->zoomToBBox(
                     itembbox.getMaxLat()+delta, itembbox.getMinLon()-delta,
                     itembbox.getMinLat()-delta, itembbox.getMaxLon()+delta );
            }
         }
      }
   }
}

void
MEMapEditorWindow::zoomToGdfIdPressed(){
   const char* idString = m_searchIDEntry->get_text().c_str();
   if ((idString != NULL) && (strlen(idString) > 0)) {
      uint32 gdfID = (strtoul(idString, NULL, 0));
      
      // Find the items with this GDF ID and highlight them
      uint32 singleItemID = m_mapArea->highlightFromGdfID(gdfID);
      
      // If one was found, select it
      if ( singleItemID != MAX_UINT32 ){
         zoomToItem(singleItemID);
      }
      else {
         mc2dbg << "Cannot zoom, no unique item for GDF ID: " 
                << gdfID << endl;
      }
   }
} // zoomToGdfIdPressed



void 
MEMapEditorWindow::searchItemNamePressed() 
{
   if ( m_searchIDEntry->get_text().c_str() == NULL ){
      return;
   }
   MC2String nameString = m_searchIDEntry->get_text().c_str();
   if (nameString.size() > 0) {
      // convert latin-1 to UTF8 (input is latin-1)
      MC2String encodedName = nameString;
#ifdef MC2_UTF8
   // Running on centos, input is UTF8 already
   // If running on some machine interpretting input as latin-1 use these rows!
//      CharEncoding* charEncoder = new CharEncoding(
//         CharEncodingType::iso8859_1, CharEncodingType::UTF8, true );
//      if ( charEncoder != NULL ) {
//         charEncoder->convert(encodedName, encodedName); // src,dest
//      }
#endif
      // Get all items with this name
      vector<OldItem*> items = m_map->searchItemsWithName(encodedName);
      // some items has the name many times (different langs)
      multimap<ItemTypes::itemType, uint32> sortedItemIDs;
      std::map<uint32, uint32> nbrNamesPerItemID;
      std::map<uint32, uint32>::iterator iter;
      for ( uint32 i=0; i<items.size(); i++){
         iter = nbrNamesPerItemID.find(items[i]->getID());
         if ( iter != nbrNamesPerItemID.end()) {
            iter->second++;
         } else {
            nbrNamesPerItemID.insert( make_pair(items[i]->getID(),1) );
            sortedItemIDs.insert(make_pair(items[i]->getItemType(), 
                                           items[i]->getID()));
         }
      }

      multimap<ItemTypes::itemType, uint32>::const_iterator it;
      if ( sortedItemIDs.size() == 0 ) {
         mc2dbg << "No items found by name: " << encodedName << endl;
      } else {
         m_mapArea->clearAllHighlight();
         mc2log << info << "Listing items found by name: " 
                << encodedName << endl;
         for ( it = sortedItemIDs.begin();
               it != sortedItemIDs.end(); it++ ){
            m_mapArea->highlightItem(it->second, 
                                     false, // clearFirst
                                     false, // locate, 
                                     false //redrawScreen
                                     );
            MC2String extraPrint = "";
            if ( m_map->itemNotInSearchIndex(it->second) ) {
               extraPrint = " (not in search index)";
            }
            if ( m_map->isIndexArea(it->second) ) {
               char tmpStr[20];
               sprintf(tmpStr, " (ia order %d)",
                        m_map->getIndexAreaOrder(it->second));
               extraPrint = tmpStr;
            }
            mc2log << info << "   " 
                   <<  ItemTypes::getItemTypeAsString(it->first) 
                   << ":\t"  << it->second 
                   << " \t(name "
                   << nbrNamesPerItemID.find(it->second)->second
                   << " times)" 
                   << extraPrint << endl;
         }
         m_mapArea->redraw();
         mc2log << info << "Found " << sortedItemIDs.size()
                << " items by name: " << encodedName << endl;
      }
   }
}

//void
//MEMapEditorWindow::zoomToItemNamePressed()
//{
//Implement this one by zooming to bounding box of all highlighted items.
   // get the item, copied from searchItemIDPressed
//   const char* idString = m_searchIDEntry->get_text().c_str();
//   if ((idString != NULL) && (strlen(idString) > 0)) {
//      uint32 itemID = (strtoul(idString, NULL, 0));
//      itemID = REMOVE_UINT32_MSB(itemID);
//      mc2dbg4 << "Zoom to item with ID=" << itemID << endl;
//      zoomToItem(itemID);
//   }
//}

void 
MEMapEditorWindow::clearHighlightPressed() 
{ 
   m_mapArea->clearAllHighlight();
}

void
MEMapEditorWindow::filterLevelPressed()
{
   uint32 newFiltLevel = atoi(m_filterLevelIn->get_text().c_str());
   if ( (newFiltLevel < 0) || (newFiltLevel > 15) ) {
      mc2dbg2 << "Cannot change to filter level " << newFiltLevel
              << " (from " << int(m_filterLevel) << ")" << endl;
   }
   else
   if ( newFiltLevel != m_filterLevel ) {
      mc2dbg2 << "Changing filter level from " << int(m_filterLevel)
              << " to " << newFiltLevel << endl;
      if ( ! m_mapArea->getMap()->mapIsFiltered() )
         mc2dbg2 << "Items in the map have undefined filter levels!" << endl;
      if ( ! m_mapArea->getMap()->mapGfxDataIsFiltered() )
         mc2dbg2 << "The map gfx data has undefined filter levels!" << endl;
      m_filterLevel = newFiltLevel;
      m_mapArea->setFilterLevel( m_filterLevel );
      m_mapArea->recalculateAndRedraw();
   }
}


void
MEMapEditorWindow::highlightLoadedData()
{  
   highlightLoadedItemIDs();
   highlightLoadedCoords();
}


void 
MEMapEditorWindow::highlightItems(vector< pair<uint32,MEGdkColors::color_t> >& 
                                  items )
{
   m_loadedItemIds = items;
   highlightLoadedItemIDs();
} // highlightItems

void
MEMapEditorWindow::highlightLoadedItemIDs()
{
   mc2dbg1 << "m_loadedItemIds.size(): " << m_loadedItemIds.size() << endl;
   multimap<MEGdkColors::color_t, uint32> itemIDsByColor;
   for ( uint32 i=0; i<m_loadedItemIds.size(); i++){
      itemIDsByColor.insert(make_pair(m_loadedItemIds[i].second, 
                                      m_loadedItemIds[i].first) );
   }
   
   multimap<MEGdkColors::color_t, uint32>::const_iterator colIt = 
      itemIDsByColor.begin();
   uint32 nbrNotOK = 0;
   while ( colIt != itemIDsByColor.end() ){
      bool clearFirst = false;
      bool locate = false;
      bool redrawScreen = false;

      bool result = 
         m_mapArea->highlightItem(colIt->second,
                                  clearFirst, 
                                  locate, 
                                  redrawScreen,
                                  colIt->first);
      if ( !result ){
         nbrNotOK++;
         mc2dbg8 << " Could not highlight: " << colIt->second << endl;
      }
      ++colIt;
   }
   if (nbrNotOK > 0) {
      mc2dbg1 << "No highlight of " << nbrNotOK << " ids" << endl;
   }
   m_mapArea->redraw();
} // highlightLoadedItemIDs


void MEMapEditorWindow::highlightCoords( vector<MC2Coordinate>& coords )
{
   m_loadedCoords = coords;
   highlightLoadedCoords();
} // highlightLoadedItemIDs

void
MEMapEditorWindow::highlightLoadedCoords()
{
   mc2dbg << info << "In highlightLoadedCoords" << endl;
   MEGdkColors* cols = MEGdkColors::getColors();
   Gdk::Color color = cols->m_purple1;

   bool redrawScreen = false;

   mc2dbg1 << "m_loadedCoords.size(): " << m_loadedCoords.size() << endl;
   uint32 nbrNotOK = 0;
   for ( uint32 i=0; i<m_loadedCoords.size(); i++){
      if ( i == m_loadedCoords.size()-1 ){
         // Last item, redraw.
         redrawScreen = true;
      }
      

      int32 lat = m_loadedCoords[i].lat;
      int32 lon = m_loadedCoords[i].lon;


      // Determine color to print with.
      MC2Coordinate coord(lat,lon);
      if (m_map->coordExistInMap(coord)){
         color = cols->m_green3;
      }
      else {
         color = cols->m_purple1;
      }

      bool result = m_mapArea->highlightCoordinate(lat, lon, color,
                                                   MEItemLayer::crossSymbol,
                                                   2, // line width
                                                   redrawScreen);
      if ( !result ){
         nbrNotOK++;
         mc2dbg8 << " Could not highlight: " 
                 << m_loadedCoords[i].lat << " " << m_loadedCoords[i].lon 
                 << endl;
      }
   }
   if (nbrNotOK > 0) {
      mc2dbg1 << "Failed highlighting " << nbrNotOK << " coords" << endl;
   }
} // highlightLoadedCoords




#ifdef MAP_EDITABLE

const char* 
MEMapEditorWindow::getLogFileName() 
{
   return m_logFilenameEntry->get_text().c_str();
}

void 
MEMapEditorWindow::savePressed() 
{
   if ( ! MEMessageDialog::inquireUser("Really save map?") ) {
      // Leave the window open!
      mc2dbg1 << here << " Does not save map!" << endl;
      return;
   }

   if (m_map->save()) {
      mc2log << info << " Map saved OK" << endl;
   } else {
      mc2log << error << " FAILED to save map" << endl;
   }
}

void 
MEMapEditorWindow::setGdfRef(GDFRef* gdfRef){
   m_mapArea->setGdfRef(gdfRef);
}

void 
MEMapEditorWindow::initiateShowPOIs(){
   // Turn off streets.
   m_mapArea->removeItemLayer(ItemTypes::streetSegmentItem, false);
 
   // Trurn company on
   //m_mapArea->addItemLayer(ItemTypes::pointOfInterestItem, false );

   // Change color of background
   m_mapArea->setBgColor(MEGdkColors::gray16);
}

void
MEMapEditorWindow::on_keepHighlightsOnSelectBtnClicked()
{
   if (m_keepHighlightsOnSelectBtn->get_active()) 
      m_mapArea->setKeepHighlightsOnSelect(true);
   else 
      m_mapArea->setKeepHighlightsOnSelect(false);
}



#endif 


