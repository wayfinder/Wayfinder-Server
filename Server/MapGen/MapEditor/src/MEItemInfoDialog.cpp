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
#include "MEItemInfoDialog.h"
#include "MEMapEditorWindow.h"
#include "MEMessageDialog.h"
#include "OldExtraDataUtility.h"
#include "GfxUtility.h"
#include "GfxConstants.h"
#include "OldBuildingItem.h"
#include "FilteredCoord.h"
#include "MEFeatureIdentification.h"
#include "MELogFilePrinter.h"
#include "OldOverviewMap.h"
#include "OldCartographicItem.h"
#include "OldCountryOverviewMap.h"

MEItemInfoDialog* 
MEItemInfoDialog::instance(MEMapArea* mapArea) 
{
   if (_instance == 0) {
      _instance = new MEItemInfoDialog(mapArea);
   }
   return _instance;
}

void
MEItemInfoDialog::deleteInstance()
{
   if (_instance != 0) {
      delete _instance;
   }
}

MEItemInfoDialog::MEItemInfoDialog(MEMapArea* mapArea)
   : Gtk::Window(Gtk::WINDOW_TOPLEVEL)
{
   // window size 
   set_size_request(325, 150);

   // Set the map-area
   MC2_ASSERT(mapArea != NULL);
   m_mapArea = mapArea;

   set_title("ItemInfo");

   Gtk::Box* mainbox = manage(new Gtk::VBox());
   add(*mainbox);
   
   m_itemInfo = manage(new MEItemInfoWidget());

   mainbox->pack_start(*m_itemInfo, true, true);

   m_groupItemInfo = manage( new MEGroupItemInfoWidget() );
   mainbox->pack_start(*m_groupItemInfo, true, true);

   m_streetSegmentItemInfo = manage(new MEStreetSegmentItemInfoWidget());
   mainbox->pack_start(*m_streetSegmentItemInfo, false, false);
 
   m_ferryItemInfo = manage(new MEFerryItemInfoWidget());
   mainbox->pack_start(*m_ferryItemInfo, true, true);

   m_routeableItemInfo = manage(new MERouteableItemInfoWidget());
   mainbox->pack_start(*m_routeableItemInfo, true, true);

   // everything not routeable
   m_waterItemInfo = manage(new MEWaterItemInfoWidget());
   mainbox->pack_start(*m_waterItemInfo, true, true);

   m_parkItemInfo = manage(new MEParkItemInfoWidget());
   mainbox->pack_start(*m_parkItemInfo, true, true);
   
   m_poiItemInfo = manage(new MEPoiItemInfoWidget());
   mainbox->pack_start(*m_poiItemInfo, true, true);
   
   m_buildingItemInfo = manage(new MEBuildingItemInfoWidget());
   mainbox->pack_start(*m_buildingItemInfo, true, true);

   m_cartographicItemInfo = manage(new MECartographicItemInfoWidget());
   mainbox->pack_start(*m_cartographicItemInfo, true, true);

   m_individualBuildingItemInfo = manage(new MEIndividualBuildingItemInfoWidget());
   mainbox->pack_start(*m_individualBuildingItemInfo, true, true);

   // Set the action area.
   Gtk::Box* actionBox = manage(new Gtk::VBox());
   //Gtk::Box* buttonBox = manage(new Gtk::HBox());
   Gtk::HBox* buttonBox = manage(new Gtk::HBox());
   m_coordinateButton = manage(new Gtk::ToggleButton("Coords"));
   buttonBox->pack_start(*m_coordinateButton, true, true);
   m_coordinateButton->signal_clicked().connect(
         sigc::mem_fun(*this, &MEItemInfoDialog::showCoordinatesClicked));
   Gtk::Button* button;

   // Add print matlab-coords button
   button = manage(new Gtk::Button("MidMif"));
   button->signal_clicked().connect( 
         sigc::mem_fun(*this, &MEItemInfoDialog::midmifPressed));
   buttonBox->pack_start(*button, true, true);

#ifdef MAP_EDITABLE
   m_logCommentBox = manage(new MELogCommentInterfaceBox(
                     m_mapArea->getMapCountryName(), m_mapArea->getMapId()));
   actionBox->pack_start(*m_logCommentBox, false, false);

   // Add delete button
   button = manage(new Gtk::Button("Delete"));
   button->signal_clicked().connect( 
      sigc::mem_fun(*this, &MEItemInfoDialog::deletePressed));
   buttonBox->pack_start(*button, true, true);
   
   // Add OK-button
   button = manage(new Gtk::Button("OK"));
   button->signal_clicked().connect( 
      sigc::mem_fun(*this, &MEItemInfoDialog::okPressed));

   buttonBox->pack_start(*button, true, true);
   const char* cancelLabelText = "Cancel";
#else
   const char* cancelLabelText = "Close";
#endif // MAP_EDITABLE

   // Add Cancel-button
   button = manage(new Gtk::Button(cancelLabelText));
   button->signal_clicked().connect(
      sigc::mem_fun(*this, &MEItemInfoDialog::cancelPressed));
   buttonBox->pack_start(*button, true, true);

   actionBox->pack_start(*buttonBox, false, false);
   mainbox->pack_start(*actionBox, false, false);

   // Make sure that it not is possible to change any values
   /*m_itemIDVal->set_editable(false);
   m_itemTypeVal->set_editable(false);
   m_locationVal->set_editable(false);*/
   
   // Display everything
   show_all();
}

void 
MEItemInfoDialog::setItemToShow(OldItem* item, OldGenericMap* map) 
{
   // Save the item to be able to Cancel
   m_item = item;
   m_map = map;
   
   if ( (item != NULL) && (map != NULL))  {
      // Hide all info-widgets
      m_itemInfo->inactivate();
      m_waterItemInfo->inactivate();
      m_parkItemInfo->inactivate();
      m_routeableItemInfo->inactivate();
      m_groupItemInfo->inactivate();
      m_streetSegmentItemInfo->inactivate(); 
      m_poiItemInfo->inactivate(); 
      m_buildingItemInfo->inactivate();
      m_ferryItemInfo->inactivate();
      m_cartographicItemInfo->inactivate();
      m_individualBuildingItemInfo->inactivate();

      set_size_request(-1, -1);
     
      // General information that is valid for all items
      m_itemInfo->activate(m_mapArea, item);

      // Information for RouteableItem ?
      if (dynamic_cast<OldRouteableItem*>(item) != NULL) {
         m_routeableItemInfo->activate(
                                 m_mapArea, 
                                 dynamic_cast<OldRouteableItem*>(item));
      }

      // Information for SSI?
      if (dynamic_cast<OldStreetSegmentItem*>(item) != NULL) {
         m_streetSegmentItemInfo->activate(
                                 m_mapArea, 
                                 dynamic_cast<OldStreetSegmentItem*>(item));
      }

      // Information for ferry?
      if (dynamic_cast<OldFerryItem*>(item) != NULL) {
         m_ferryItemInfo->activate(
               m_mapArea, dynamic_cast<OldFerryItem*>(item));
      }

      // Information for water?
      if (dynamic_cast<OldWaterItem*>(item) != NULL) {
         m_waterItemInfo->activate(
               m_mapArea, dynamic_cast<OldWaterItem*>(item));
      }

      // Information for park?
      if (dynamic_cast<OldParkItem*>(item) != NULL) {
         m_parkItemInfo->activate(
               m_mapArea, dynamic_cast<OldParkItem*>(item));
      }

      // Is this a group-item??
      if (dynamic_cast<OldGroupItem*>(item) != NULL) {
         m_groupItemInfo->activate(
               m_mapArea, dynamic_cast<OldGroupItem*>(item));
      }

      // Information for poi?
      if (dynamic_cast<OldPointOfInterestItem*>(item) != NULL) {
         m_poiItemInfo->activate(
                  m_mapArea, dynamic_cast<OldPointOfInterestItem*>(item));
      }
      // Information for building?
      if (dynamic_cast<OldBuildingItem*>(item) != NULL) {
         m_buildingItemInfo->activate(
                  m_mapArea, dynamic_cast<OldBuildingItem*>(item));
      }
      
      // Information for cartographic?
      if (dynamic_cast<OldCartographicItem*>(item) != NULL) {
         m_cartographicItemInfo->activate(
               m_mapArea, dynamic_cast<OldCartographicItem*>(item));
      }
      
      // Information for individual building?
      if (dynamic_cast<OldIndividualBuildingItem*>(item) != NULL) {
         m_individualBuildingItemInfo->activate(
                  m_mapArea, dynamic_cast<OldIndividualBuildingItem*>(item));
      }

      // Set the title of the window
      char tmpstr[128];
      if ( item->getNbrNames() > 0 ) {
         sprintf(tmpstr, "%s", map->getFirstItemName(item));
      } else {
         sprintf(tmpstr, "%s", " - no name - ");
      }
      set_title(tmpstr);

      // Print the IDs of the group items of this item.
      mc2log << info << "Groups:====================" << endl;
      if ( item->getNbrGroups() > 0 ) {
         
         for ( uint32 groupIdx = 0; 
               groupIdx < item->getNbrGroups();
               groupIdx++ ){
            
            uint32 groupID = item->getGroup(groupIdx);
            uint32 rawGroupID = item->getUnmaskedGroup(groupIdx);
            const char* extraInfo = "";
            if ( (rawGroupID & 0x80000000) != 0 ){
               extraInfo = "High bit set";
            } else if ( map->isIndexArea(groupID) ) {
               sprintf(tmpstr, "Index area order %u", 
                       map->getIndexAreaOrder(groupID) );
               extraInfo = tmpstr;
            }
            mc2log << info << groupID
                   << "            " << extraInfo << endl;
         }
      }
      else {
         mc2log << info << "No groups" << endl;
      }
      mc2log << info << endl;
      
      
      // Print names.
      mc2log << info << "Names:======================" << endl;
      if ( item->getNbrNames() > 0 ) {
         mc2log << info << "(name offset:language:name)" << endl;
         mc2log << info << "   (raw name char codes as hex)" << endl;
         mc2log << info << endl;
         
         for ( uint32 n=0; n < item->getNbrNames(); n++){
            // Name:
            uint32 strIdx = item->getStringIndex( n );
            const char* name = m_map->getName( strIdx );
            
            // Name lang:
            LangTypes::language_t lang = item->getNameLanguage(n);
            bool twoChars = false; // Implies three character code.
            const char* langStr = 
               LangTypes::getLanguageAsISO639( lang, twoChars );
            
            mc2log << info << n << ":" 
                   << langStr << ":" 
                   << name << endl;
            
            // Name character codes as hex.
            mc2dbg << " "; // Indent. 
            for ( uint32 idx=0; idx<strlen(name); idx++ ){
               mc2dbg << " 0x" << hex 
                      << (uint32)(unsigned char)name[idx] 
                      << dec;
               
               if ( (idx % 9 == 8) && (strlen(name) != (idx+1)) ){
                  // Break the row.
                  mc2dbg << endl;
                  mc2dbg << " "; // Indent. 
               }
            }
            mc2dbg << endl;
         }
         mc2log << info << endl;            
         
      }
      else {
         mc2log << info << "No names" << endl;
      }
   }
   if ( item->getItemType() == ItemTypes::pointOfInterestItem ){
      mc2log << info << "Categories:=================" << endl;
      const set<uint16>& categories = item->getCategories(*map);
      for ( set<uint16>::const_iterator catIt = categories.begin();
            catIt != categories.end(); ++catIt ){
         mc2log << info << "Cat ID: " << *catIt << endl;
      }
   }



   mc2log << info << "Other:======================" << endl;


   // Map rights.
   //UserEnums::URType rights = map->getRights(item->getID());
   //mc2log << "Rights(level):   " << hex << rights.first << endl;
   //mc2log << "Rights(service): " << hex << rights.second << dec << endl;
   const MapRights rights = map->getRights(item->getID());
   mc2log << "Rights: " << hex << rights << dec << endl;

   // Admin area center coord
   MC2Coordinate coord = map->getCentreFromAdminTable(item->getID()); 
   if ( coord.isValid() ){
      mc2log << "Admin center coord: " << coord << endl;
   }

   // GDF ID
   if ( m_mapArea->getGdfID(item).isValid() ){
      mc2log  << "GDF ID: " << m_mapArea->getGdfID(item) << endl;
   }

   // Get lower map item ID:
   uint32 lowerMapID = MAX_UINT32;
   uint32 lowerItemID = MAX_UINT32;
   bool IamLowerMap = true;
   OldOverviewMap* ovrMap = dynamic_cast<OldOverviewMap*>(map);
   if ( ovrMap != NULL ){
      IamLowerMap = false;
      IDPair_t mapAndItemID = ovrMap->lookup(item->getID());
      lowerMapID = mapAndItemID.getMapID();
      lowerItemID = mapAndItemID.getItemID();
   }
   else {
      OldCountryOverviewMap* coOvrMap = 
         dynamic_cast<OldCountryOverviewMap*>(map);
      if ( coOvrMap != NULL ){
         IamLowerMap = false;
         coOvrMap->getOriginalIDs(item->getID(), lowerMapID, lowerItemID);
      }
   }
   if ( !IamLowerMap ) {
      if ( lowerMapID != MAX_UINT32 ){
         mc2log << "Lower map item ID: 0x"
                << hex << lowerMapID << "(" 
                << dec << lowerMapID << "):0x" 
                << hex << lowerItemID << "(" 
                << dec << lowerItemID << ")" << endl;
      }
      else {
         mc2log << "Item not present in lower level map." << endl;
      }
   }
   if ( item->getItemType() == ItemTypes::cartographicItem ){
      OldCartographicItem* cartItem = dynamic_cast<OldCartographicItem*>(item);
      MC2_ASSERT(cartItem != NULL);
      mc2log << "Cartographic type: " << cartItem->getCartographicType() 
             << ":" 
             << cartItem->cartographicTypeToString(cartItem->getCartographicType())
             << endl;
   }

   // Index area order.
   uint32 indexAreaOrder = map->getIndexAreaOrder(item->getID());
   if ( indexAreaOrder != MAX_UINT32 ){
      if ( indexAreaOrder == (MAX_UINT32-1) ){
         mc2log << "Index area: no order given" << endl;
      }
      else {
         mc2log << "Index area: order " << indexAreaOrder << endl;
      }
   }

   // Not part of search index.
   if (map->itemNotInSearchIndex(item->getID()) ){
      mc2log << "Item is marked as not part of search index." << endl;
   }
   else {
      //mc2log << "Item may take part in search index." << endl;
   }

   // Road display class.
   ItemTypes::roadDisplayClass_t roadDispClass =
         map->getRoadDisplayClass(item->getID());
   if ( roadDispClass != ItemTypes::nbrRoadDisplayClasses ){
      mc2log << "Road display class: " << int(roadDispClass) << " " 
             << ItemTypes::getStringForRoadDisplayClass(roadDispClass)
             << endl;
   }
   // Area feature draw display class.
   ItemTypes::areaFeatureDrawDisplayClass_t areaDrawDispClass =
         map->getAreaFeatureDrawDisplayClass(item->getID());
   if ( areaDrawDispClass != ItemTypes::nbrAreaFeatureDrawDisplayClasses ){
      mc2log << "Area feature draw display class: " 
             << int(areaDrawDispClass) << " " 
             << ItemTypes::getStringForAreaFeatureDrawDisplayClass(
                     areaDrawDispClass)
             << endl;
   }

   mc2log << info << "============================" << endl;            
   
   show();
}

MEMapArea*
MEItemInfoDialog::getMapArea()
{
   return m_mapArea;
}



gint 
MEItemInfoDialog::delete_event_impl(GdkEventAny* p0)
{
   hide();
   return 1;
}

void
MEItemInfoDialog::showCoordinatesClicked()
{
   mc2dbg4 << here << "MEItemInfoDialog::showCoordinatesClicked()" << endl;

   if (m_coordinateButton->get_active()) {
      GfxData* gfx = m_item->getGfxData();
      if (m_mapArea->highlightCoordinates( gfx )) {
         mc2dbg8 << "   gfxData draw ok" << endl;

         // Print a table with all coordinates, distances and angles
         if (gfx->closed()) {
            cout << "GfxData closed ---";
         } else {
            cout << "GfxData not closed";
         }
         cout << "------------- id = " << m_item->getID() << " (";
         m_map->printItemNames(m_item->getID());
         cout << ") --------" << endl;

         if ( m_mapArea->getMap()->mapIsFiltered() ) {
            cout << "poly \tcoord \tlatitude \tlongitude \tfilterLevel \tdist"
                 << endl;
            for ( uint16 p = 0; p < gfx->getNbrPolygons(); p++ ) {
               GfxData::iterator end = gfx->polyEnd( p );
               uint32 c = 0;
               int prevLat = 0, prevLon = 0;
               for ( GfxData::iterator it = gfx->polyBegin( p );
                     it != end; ++it ) {
                  int32 lat = it->lat;
                  int32 lon = it->lon;
                  uint8 filterLevel =
                     static_cast<FilteredCoord&>(*it).getFilterLevel();

                  float dist = 0;
                  if ( c != 0 ) {
                     dist = sqrt(GfxUtility::squareP2Pdistance_linear(
                        prevLat, prevLon, lat, lon));
                  }

                  cout << " " << p << "\t" << c << "\t" << lat << "\t" 
                       << lon << "\t" << int(filterLevel) << "\t\t" << dist
                       << endl;
                  c++;
                  prevLat = lat;
                  prevLon = lon;
               }
            }
         } else {
            cout << "latitude\tlongitude\tdist.(m)\tang.(deg)\tang.(1/256)" 
                 << endl;
            for (uint32 j=0; j<gfx->getNbrPolygons(); j++){
               cout << "Polygon: " << j << endl;

               cout << gfx->getLat(j,0) << "\t" << gfx->getLon(j,0) 
                    << "\t    -" 
                    << "\t    -" << endl;
               for (uint32 i=1; i<gfx->getNbrCoordinates(j); i++) {
                  float64 rad = GfxUtility::getAngleFromNorth(
                             gfx->getLat(j,i-1), gfx->getLon(j,i-1), 
                             gfx->getLat(j,i), gfx->getLon(j,i));
                  printf("%7d\t%7d\t%6.1f\t\t%3d\t\t%3d\n",
                         gfx->getLat(j,i),
                         gfx->getLon(j,i),
                         sqrt(GfxUtility::squareP2Pdistance_linear(
                             gfx->getLat(j,i-1), gfx->getLon(j,i-1), 
                             gfx->getLat(j,i), gfx->getLon(j,i))),
                         int(rad * GfxConstants::radianTodegreeFactor),
                         int(rad * 256 / 2 / M_PI));
               }
            }
         }

         cout << "----------------------------------------------------------------------------" << endl;
         cout << "length (poly 0) = " << gfx->getLength(0) 
              << ", nbr coords (poly 0) = " << gfx->getNbrCoordinates(0) 
              << endl;
         cout << "total nbrPolys = " << gfx->getNbrPolygons() 
              << ", total nbr coords = " << gfx->getTotalNbrCoordinates()
              << endl;
         if (gfx->getClosed(0) && gfx->getNbrCoordinates(0) > 2) {
            float64 area = 0;
            for (uint32 p=0; p<gfx->getNbrPolygons(); p++) {
               float64 curArea = fabs(gfx->polygonArea(p));
               area += curArea;
            }
            cout << "total gfx polygonArea = " << uint32(rint(area * 
                     GfxConstants::SQUARE_MC2SCALE_TO_SQUARE_METER)) 
                 << " m2" << endl;
         }

      } else {
         mc2dbg2 << "  FAILED to draw gfxData" << endl;
         // The item perhaps had no gfxData,
         // print coords for POIs from offset on ssi
         if ( (gfx == NULL) &&
              (m_item->getItemType() == ItemTypes::pointOfInterestItem) ) {
            OldPointOfInterestItem* poi = 
               static_cast<OldPointOfInterestItem*>(m_item);
            OldItem* ssi = m_map->itemLookup(poi->getStreetSegmentItemID());
            if ( ssi != NULL ) {
               uint16 offset = poi->getOffsetOnStreet();
               int32 lat, lon;
               ssi->getGfxData()->getCoordinate(offset, lat, lon);
               mc2dbg2 << "   poi coord from offset on ssi ("
                       << lat << ";" << lon << ") " << endl;
            }
         }
      }
   } else {
      // Time to switch off the coordinates
      m_mapArea->clearCoordinateHighlight(true);
   }
   //(ShowItemCoordinates::instance())->setGfxData(m_item->getGfxData());
}


void 
MEItemInfoDialog::okPressed() {
#ifdef MAP_EDITABLE
   // Ask all active Widgets to save themselvs
   m_itemInfo->saveChanges(m_logCommentBox);
   if (m_routeableItemInfo->isActive())
      m_routeableItemInfo->saveChanges(m_logCommentBox);
   if (m_groupItemInfo->isActive())
      m_groupItemInfo->saveChanges(m_logCommentBox);
   if (m_streetSegmentItemInfo->isActive())
      m_streetSegmentItemInfo->saveChanges(m_logCommentBox);
   if (m_waterItemInfo->isActive())
      m_waterItemInfo->saveChanges(m_logCommentBox);
#else
   hide();
#endif
}


#ifdef MAP_EDITABLE
void
MEItemInfoDialog::deletePressed()
{
   if ( ! MEMessageDialog::inquireUser("Delete this item?") ) {
      // Leave the window open!
      mc2dbg1 << here << " Does not delete item!" << endl;
      return;
   }

   // Create log comment string, no original value
   vector<MC2String> logStrings = m_logCommentBox->getLogComments(); 
   
   MC2_ASSERT(m_map != NULL);

   // Get item identification strings etc before removing
   // Suggestion: Represent the ssi with two coordinates instead ??
   // (node 0 coordinates)
   MC2String identString;
   bool identOK = MEFeatureIdentification::getItemIdentificationString(
                     identString, m_mapArea->getMap(), m_item,
                     -1,         // no preferred name idx
                     false );    // don't include nameType and language

   if ( ! identOK ) {
      mc2log << warn << "Failed to get item identification string for "
             << m_item->getID() << " - abort change" << endl;
      return;
   }
   mc2dbg8 << "MEItemInfoDialog: Got item identification string, \""
           << identString << endl;

   // If we try to remove a ssi, check if there is a POI connected to
   // the ssi. Then it has to be moved, not to be removed from the map.
   if ( m_item->getItemType() == ItemTypes::streetSegmentItem ) {
      // The hardway, find pois connected to this ssi
      uint32 ssiID = m_item->getID();
      vector<uint32> poiIDs;
      for (uint32 i = 0;
           i < m_map->getNbrItemsWithZoom(ItemTypes::poiiZoomLevel); i++) {
         OldPointOfInterestItem* poi = static_cast<OldPointOfInterestItem*>
            (m_map->getItem(ItemTypes::poiiZoomLevel, i));
         if ((poi != NULL) && 
             (poi->getStreetSegmentItemID() == ssiID)) {
            poiIDs.push_back(poi->getWASPID());
         }
      }

      if ( poiIDs.size() > 0 ) {
         // write all waspIDs to a tmpStr..
         char tmpStr[1000];
         sprintf(tmpStr, "This SSI has %d POIs waspID:\n", poiIDs.size());
         mc2log << warn << "This ssi has " << poiIDs.size() << " POIs, waspID:";
         for ( vector<uint32>::const_iterator it = poiIDs.begin();
               it != poiIDs.end(); it++ ) {
            mc2log << " " << (*it);
            sprintf(tmpStr, "%s %d", tmpStr, (*it));
         }
         mc2log << endl
                << "Take action in WASP db:" << endl
                << " - move the POIs to avoid having them removed "
                << "from the map!" << endl;
         // Open a dialog to make the user alert on this.
         sprintf(tmpStr, "%s%s %s", tmpStr, "\n", "Take action in WASP db");
         MEMessageDialog::displayMessage( tmpStr );
      }
   }


   if (OldExtraDataUtility::removeItem(m_map, m_item)) {
                         
      // Empty new value vector
      vector<MC2String> newValStrings;
      
      // Print to log file
      MELogFilePrinter::print(
            logStrings, "removeItem", identString, newValStrings );
      mc2dbg1 << "Item removed from map!" << endl;

      if ( m_item->getItemType() == ItemTypes::streetSegmentItem ) {
         MC2String str = "You might want to add map ssi coordinate";
         str += " in the ed record. Especially if this ssi is close to";
         str += " the map border, you want to prevent that a ssi is";
         str += " removed from the neighbour map";
         MEMessageDialog::displayMessage( str.c_str() );
      }
      
      
      m_mapArea->recalculateAndRedraw();
      hide();
   }
}
#endif

void
MEItemInfoDialog::midmifPressed()
{
   if ( (m_mapArea != NULL) && 
        (m_item != NULL) && (m_item->getGfxData() != NULL) ) {

      ofstream mifFile("item.mif");
      ofstream midFile("item.mid");
   
      // Print mif header
      if ( OldGenericMap::writeItemMifHeader(
                  mifFile, m_item->getItemType()) ) {

         // Print item
         m_item->printMidMif( midFile, mifFile, m_map->getItemNames() );
         
         //mapID used when printing water
         if (m_item->getItemType() == ItemTypes::waterItem) {
            midFile << "," << m_mapArea->getMapId() << endl;
         }
         mc2dbg1 << "Printed item " << m_item->getID() 
                 << " to file \"item.mid/mif\"" << endl;
      }
   }
}

