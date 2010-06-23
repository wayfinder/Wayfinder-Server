/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSTool.h"

#include <map>
#include <vector>

#include "GMSMap.h"
#include "NationalProperties.h"
#include "OldItem.h"
#include "OldWaterItem.h"
#include "OldStreetSegmentItem.h"
#include "OldNode.h"
#include "GMSCountryOverviewMap.h"
#include "STLStringUtility.h"
#include "GMSPolyUtility.h"



bool 
GMSTool::processMap(const MC2String& option, GMSMap* theMap)
{
   mc2log << info << "GMSTool processing map: 0x" 
          << hex << theMap->getMapID() 
          << dec << "(" << theMap->getMapID() << ")" << endl;
   mc2log << info << "GMSTool option: " << option << endl;

   bool saveMap = false;

   // Keep the content of each if statement short, maximum of ten rows.
   // If you need more, create a private method. Document the functionality of
   // each if statement.

   /**
    * Merges the polygons of each item in the map where it is possible.
    */
   if (option == "mergeItemPolygons"){
      uint32 nbrMerged=GMSPolyUtility::mergeItemPolygons( theMap );
      mc2log << info << "Merged polygons of " << nbrMerged << " items." 
             << endl;
      saveMap = ( nbrMerged > 0 );
   }
   /**
    * Counts coordinates of all water items.
    */
   else if (option == "countWaterItemCoords"){
      uint32 nbrCoords = getNbrCoordsOfItems( theMap, ItemTypes::waterItem );
      mc2log << info << " Found " << nbrCoords << " water coords in map." 
             << endl;
      saveMap = false;
   }
   /**
    * Test to load and save the map.
    */
   else if (option == "loadAndSave"){
      saveMap = true;
   }

   /**
    * Test to load and delete the map.
    */
   else if (option == "loadAndDelete"){
      //delete theMap; // No need to delete the map. It is done before exit.
   }

   else if (option == "checkCenterPoints"){
      checkCenterPoints(theMap);
   }
   else if (option == "mergeCloseItems"){
      GMSPolyUtility::mergeSameNameCloseItems(
            theMap, ItemTypes::parkItem, true);
      saveMap = true;
   }
   else if (option == "printMunBuaTree"){
      printMunBuaTree(theMap);
   }
   else if (option == "printLanesSsiIDs"){
      printLanesSsiIDs(theMap);
   }
   else if (option == "printMultiDigSsiIDs"){
      printMultiDigSsiIDs(theMap);
   }
   else if (option == "printNoThroughfareSsiIDs"){
      printNoThroughfareSsiIDs(theMap);
   }
   else if (option == "printCtrlAccSsiIDs"){
      printCtrlAccSsiIDs(theMap);
   }
   else if (option == "printNbrCoordsInCoPolFiltLevels"){
      OldGenericMap* map = dynamic_cast<OldGenericMap*>(theMap);
      GMSCountryOverviewMap* countryMap = 
         dynamic_cast<GMSCountryOverviewMap*>(map);
      printNbrCoordsInCoPolFiltLevels(countryMap);
   }
   else if ( option == "printConnLanesItemIDs" ){
      printConnLanesItemIDs(theMap);
   }
   

   // Fill in with more options here.

   else {
      mc2log << error << "Unknown option: " << option << endl;
      exit(1);
   }
   return saveMap;
} // processMap




uint32 
GMSTool::getNbrCoordsOfItems(GMSMap* theMap, ItemTypes::itemType itemType){
   uint32 totalNbrCoords=0;
   uint32 otherWaterCoords=0;
   uint32 oceanWaterCoords=0;
   uint32 notOceanOtherWaterCoords=0;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
         OldItem* item = theMap->getItem(z, i);
         if (item == NULL){
            continue;
         }
         if ( ( item->getItemType() != itemType ) && 
              ( itemType != ItemTypes::numberOfItemTypes ) ){
            continue;
         }
         GfxData* gfx = item->getGfxData();
         if ( gfx == NULL ){
            continue;
         }
         totalNbrCoords += gfx->getTotalNbrCoordinates();
         OldWaterItem* waterItem = dynamic_cast<OldWaterItem*>(item);
         if (waterItem == NULL){
            continue;
         }
         if (waterItem->getWaterType() == ItemTypes::otherWaterElement){
            otherWaterCoords += gfx->getTotalNbrCoordinates();
         }
         else if (waterItem->getWaterType() == ItemTypes::ocean){
            oceanWaterCoords += gfx->getTotalNbrCoordinates();
         }
         else {
            notOceanOtherWaterCoords += gfx->getTotalNbrCoordinates();
         }
      }
   }

   if ( otherWaterCoords != 0 ){
      mc2log << info << " Other water coords: " << otherWaterCoords << endl;
   }
   if ( oceanWaterCoords != 0 ){
      mc2log << info << " Ocean water coords: " 
             << oceanWaterCoords << endl;
   }
   if ( notOceanOtherWaterCoords != 0 ){
      mc2log << info << " Not other or ocean water coords: " 
             << notOceanOtherWaterCoords << endl;
   }

   return totalNbrCoords;

} // getNbrCoordsOfItems





void 
GMSTool::checkCenterPoints(GMSMap* theMap){

   // Count different type of items center points.

   uint32 buaInOvrNbrCenters=0;
   uint32 buaNoGrpNbrCenters=0;
   uint32 municipalNbrCenters=0;
   uint32 zipCodeNbrCenters=0;

   uint32 buaInOvrNoCenters=0;
   uint32 buaNoGrpNoCenters=0;
   uint32 municipalNoCenters=0;
   uint32 zipCodeNoCenters=0;

   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
         OldItem* item = theMap->getItem(z, i);
         if (item == NULL){
            continue;
         }
         MC2Coordinate coord = theMap->getCentreFromAdminTable(item->getID());
         
         if ( item->getItemType() == ItemTypes::builtUpAreaItem ){
            if ( theMap->itemUsedAsGroup(item) ){
               if ( coord.isValid() ){
                  buaInOvrNbrCenters++;
               }
               else {
                  buaInOvrNoCenters++;
               }
            }
            else {
               // This is a non locality index north american BUA.
               if ( coord.isValid() ){
                  buaNoGrpNbrCenters++;
               }
               else {
                  buaNoGrpNoCenters++;
               }
            }
         }
         else if ( item->getItemType() == ItemTypes::municipalItem ){
            if ( coord.isValid() ){
               municipalNbrCenters++;
            }
            else {
               municipalNoCenters++;
            }
         }
         else if ( item->getItemType() == ItemTypes::zipCodeItem ){
            if ( coord.isValid() ){
               zipCodeNbrCenters++;
            }
            else {
               zipCodeNoCenters++;
            }
         }
      }
   }
   mc2log << info << "Number of admin centers / no admin centers." << endl;
   mc2log << info << "BUA in ovr: " << buaInOvrNbrCenters 
          << " / " << buaInOvrNoCenters << endl;
   mc2log << info << "BUA no grp: " << buaNoGrpNbrCenters 
          << " / " << buaNoGrpNoCenters << endl;
   mc2log << info << "Mun: " << municipalNbrCenters 
          << " / " << municipalNoCenters << endl;
   mc2log << info << "Zips: " << zipCodeNbrCenters 
          << " / " << zipCodeNoCenters << endl;
   mc2log << info << "Total size: " << theMap->getAdminAreaCentres().size() 
          << endl;
}


void 
GMSTool::printMunBuaTree(GMSMap* theMap){
   // Parent in first and child in second of the pair. MAX_INT32 in first if 
   // a child have no parent.
   treeType_t tree;

   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
         OldItem* item = theMap->getItem(z,i);
         if ( item == NULL ){
            continue;
         }
         ItemTypes::itemType itemType = item->getItemType();
         if ( ! (itemType == ItemTypes::municipalItem ||
                 itemType == ItemTypes::builtUpAreaItem ) ){
            continue;
         }
         for (uint32 g=0; g<item->getNbrGroups(); g++){
            uint32 groupItemID = item->getGroup(g);
            tree.insert(make_pair(groupItemID, item->getID()));
         }
         if ( item->getNbrGroups() == 0 ){
            tree.insert(make_pair(MAX_UINT32, item->getID()));
         }
      }
   }
   mc2dbg << "Size of tree is " << tree.size() << endl;
   
   while ( tree.size() > 0 ){
      treeType_t::iterator it = tree.find(MAX_UINT32);
      if ( it == tree.end() ){
         mc2dbg << error 
                << "Early end, size of tree is " << tree.size() << endl;
         MC2_ASSERT(false);
      }
      MC2String indent = "   ";
      printTreeNode(theMap,
                    indent, 
                    it->second, // Item ID
                    tree );
      mc2dbg8 << "erasing " << it->second << endl;
      tree.erase(it);
      mc2dbg8 << "Size of tree is " << tree.size() << endl;

      
   }


} // printMunBuaTree

void 
GMSTool::printTreeNode(GMSMap* theMap,MC2String indent, 
                       uint32 itemID, treeType_t& tree){

   OldItem* item = theMap->itemLookup(itemID);
   cout << indent << itemID                           // ID
        << " (" << item->getItemType() << "):"        // type
        << theMap->getBestItemName(itemID) << endl;   // name
   indent = indent + "   ";
   treeType_t::iterator it = tree.find(itemID);
   while (it != tree.end() ){
      printTreeNode(theMap, indent, it->second, tree);
      mc2dbg8 << "erasing " << it->second << endl;
      tree.erase(it);
      mc2dbg8 << "Size of tree is " << tree.size() << endl;

      it = tree.find(itemID);
   }
} // printTreeNode

void
GMSTool::printLanesSsiIDs(GMSMap* theMap){
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
         OldRouteableItem* rItem = 
            dynamic_cast<OldRouteableItem*>(theMap->getItem(z,i));
         if ( rItem != NULL ){
            for( int32 n=0; n<2; n++){
               // Get the nodes.
               OldNode* node = rItem->getNode(n);
               if (node->getLanes(*theMap).size() > 0){
                  cout << rItem->getID() << endl;
               }
            }
         }
      }
   }
} // printLanesSsiIDs

void
GMSTool::printMultiDigSsiIDs(GMSMap* theMap){
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
         OldStreetSegmentItem* ssi = 
            dynamic_cast<OldStreetSegmentItem*>(theMap->getItem(z,i));
         if ( ssi != NULL ){
            if ( ssi->isMultiDigitised() ) {
               cout << ssi->getID() << endl;
            }
         }
      }
   }
} // printMultiDigSsiIDs

void
GMSTool::printNoThroughfareSsiIDs(GMSMap* theMap){
   set<uint32> ids;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
         OldStreetSegmentItem* ssi = 
            dynamic_cast<OldStreetSegmentItem*>(theMap->getItem(z,i));
         if ( ssi != NULL ){
            for( int32 n=0; n<2; n++){
               // Get the nodes.
               OldNode* node = ssi->getNode(n);
               if (node->getEntryRestrictions() == ItemTypes::noThroughfare){
                  ids.insert( ssi->getID() );
               }
            }
         }
      }
   }
   set<uint32>::const_iterator it;
   for ( it = ids.begin(); it != ids.end(); it++) {
      cout << *it << endl;
   }
} // printNoThroughfareSsiIDs

void
GMSTool::printCtrlAccSsiIDs(GMSMap* theMap){
   set<uint32> ids;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
         OldStreetSegmentItem* ssi = 
            dynamic_cast<OldStreetSegmentItem*>(theMap->getItem(z,i));
         if ( ssi != NULL ){
            if ( ssi->isControlledAccess() ) {
               ids.insert( ssi->getID() );
            }
         }
      }
   }
   set<uint32>::const_iterator it;
   for ( it = ids.begin(); it != ids.end(); it++) {
      cout << *it << endl;
      //cout << theMap->getMapID() << ":" << *it << endl;
   }
} // printCtrlAccSsiIDs


void 
GMSTool::printNbrCoordsInCoPolFiltLevels(
               GMSCountryOverviewMap* countryMap)
{
   if (countryMap != NULL) {
      GMSGfxData* mapGfx = countryMap->getGfxData();

      uint32 nbrPolysToPrint = 1;
      for (uint32 level = 0; level <= 15; level++ ) {
         
         for ( uint32 p = 0; p < nbrPolysToPrint; p++ ) {
            // Store coords of this poly of this filtLevel in a temp gfx
            // then, if we want it in newGfx, copy the coordinates there
            GMSGfxData* tmpGfx = GMSGfxData::createNewGfxData( countryMap );
            tmpGfx->addPolygon();
            GfxDataFilterIterator end = mapGfx->endFilteredPoly( p, level );
            for ( GfxDataFilterIterator it = 
                  mapGfx->beginFilteredPoly( p, level );
                  it != end; ++it ) {
               tmpGfx->addCoordinate( (*it).lat, (*it).lon );
            }
            tmpGfx->setClosed( 0, mapGfx->getClosed( p ) );
            tmpGfx->updateLength();
            cout << "Level " << level 
                 << " poly " << p << ":" << mapGfx->getNbrCoordinates(p) 
                 << " -> " << tmpGfx->getNbrCoordinates(0) << endl;
         }
      }
   }

} // printNbrCoordsInCoPolFiltLevels

void
GMSTool::printConnLanesItemIDs(GMSMap* theMap)
{
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
         OldStreetSegmentItem* ssi = 
            dynamic_cast<OldStreetSegmentItem*>(theMap->getItem(z, i));
         if (ssi == NULL){
            continue;
         }
         bool hasConnectingLaneInfo = false;
         for (uint32 n=0; n<2; n++){
            OldNode* node = ssi->getNode(n);
            for ( uint32 c=0; c<node->getNbrConnections(); c++){
               OldConnection* conn = node->getEntryConnection(c);
               uint32 fromNodeID = conn->getFromNode();
               if ( node->getConnectedLanes(*theMap, fromNodeID) 
                    != MAX_UINT32 ){
                  hasConnectingLaneInfo = true;
               }
            }
         }
         if ( hasConnectingLaneInfo ){
            cout << ssi->getID() << endl;
         }
      }
   }
} // printConnLanesItemIDs


