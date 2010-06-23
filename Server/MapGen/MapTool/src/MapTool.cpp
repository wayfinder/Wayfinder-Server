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
#include "MC2Logging.h"
#include "IDPairVector.h"
#include <stdlib.h> // For random
#include <vector>
#include "CommandlineOptionHandler.h"
#include "OldGenericMapHeader.h"
#include "OldGenericMap.h"
#include "OldCountryOverviewMap.h"
#include "OldMapHashTable.h"
#include "OldExtraDataUtility.h"
#include <iomanip>

#include "malloc.h"
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include "MC2String.h"

#include "OldStreetSegmentItem.h"
#include "GMSGfxData.h"
#include "OldPointOfInterestItem.h"
#include "OldStreetItem.h"
#include "OldBuiltUpAreaItem.h"
#include "OldMunicipalItem.h"
#include "OldNode.h"

#include "OldExternalConnections.h" // for OldBoundrySegmentsVector
#include "OldOverviewMap.h"

#include "GfxFilterUtil.h"
#include "OldMapFilter.h"
#include "Stack.h"
#include "StringUtility.h"
#include "StringSearchUtility.h"
#include "STLStringUtility.h"
#include "TimeUtility.h"
#include "MapBits.h"
#include "NodeBits.h"

#include "Utility.h"

void
calculateCostStatistics(OldGenericMap* theMap)
{
   // Interval, in "costA" (more or less centimeters).
   const int INTERVAL = 512;
   
   // A LARGE vector with all the costs...
   vector<uint32> costs;
   uint32 allocSize = (theMap->getNbrItemsWithZoom(2) +
                       theMap->getNbrItemsWithZoom(3) +
                       theMap->getNbrItemsWithZoom(4) +
                       theMap->getNbrItemsWithZoom(5) +
                       theMap->getNbrItemsWithZoom(6) +
                       theMap->getNbrItemsWithZoom(7) +
                       theMap->getNbrItemsWithZoom(8) ) * 2;
   costs.reserve(allocSize);
   uint32 costA, costB, costC, costD;
   
   // Check all street segments
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
         OldItem* item = theMap->getItem(z, i);
         if ( (item != NULL) && 
              (item->getItemType() == ItemTypes::streetSegmentItem)) {
            for (uint32 n=0; n<2; ++n) {
               OldNode* node = static_cast<OldStreetSegmentItem*>(item)->getNode(n);
               for (uint32 c=0; c<node->getNbrConnections(); ++c) {
                  OldConnection* con = node->getEntryConnection(c);
                  theMap->getConnectionCost(con, node, 
                                            costA, costB, costC, costD);
                  costs.push_back(costA);
               }
            }
         }
      } // for i
   } // for z

   // Print result
   cout << "Nbr costs: " << costs.size() << endl;
   sort(costs.begin(), costs.end());
   uint32 maxCost = *(costs.end()-1);
   cout << "Cost interval: " << *(costs.begin()) << " - " << maxCost << endl;
   uint32 totNbr = 0;
   vector<uint32>::const_iterator lowerIt = costs.begin();
   for (uint32 curMax=INTERVAL; curMax<maxCost; curMax+=INTERVAL) {
      vector<uint32>::const_iterator higherIt= 
         upper_bound(costs.begin(), costs.end(), curMax);
      uint32 nbrInInterval = (higherIt - lowerIt);
      if (nbrInInterval > 0) {
         cout << "[" << curMax-INTERVAL << ", " << curMax << "] : \t"
              <<  nbrInInterval << endl;
         totNbr += nbrInInterval;
      }
      lowerIt = higherIt;
   }
   cout << "Nbr costs (sum): " << totNbr << endl;


   cout << "Finished..." << endl;

}


void
saveTurndescriptions(OldGenericMap* theMap, const char* fileName)
{
   // Get current time
   uint32 startTime = TimeUtility::getCurrentTime();

   // Stream where to save the turndescriptions
   ofstream log(fileName);

   // The number of saved turndescriptions
   uint32 nbrSavedTurns = 0;

   for (uint32 i=0; i<NUMBER_GFX_ZOOMLEVELS; i++) {
      for (uint32 j=0; j<theMap->getNbrItemsWithZoom(i); j++) {
         OldItem* item = theMap->getItem(i, j);
         if ( (item != NULL) && 
              (item->getItemType() == ItemTypes::streetSegmentItem)) {
            for (uint32 nodeNbr=0; nodeNbr<2; nodeNbr++) {
               OldNode* node = static_cast<OldStreetSegmentItem*>
                                       (item)->getNode(nodeNbr);
               // Get coordinates for this node
               int32 lat1, lon1, lat2, lon2;
               if ( (node == NULL) ||
                    (!theMap->getCoordinatesFromNode(node, lat1, lon1, 
                                                  lat2, lon2))) {
                  mc2log << fatal << here << " Failed to lookup node" << endl;
                  continue;
               }
               
               // Save turndescriptions in all connections
               for (uint32 c=0; c<node->getNbrConnections(); c++) {
                  OldConnection* con = node->getEntryConnection(c);

                  // Get coordinates for from-node
                  OldNode* fromNode = theMap->nodeLookup(
                                             con->getConnectFromNode());
                  int32 fromLat1, fromLon1, fromLat2, fromLon2;
                  if ( (fromNode == NULL) ||
                       (!theMap->getCoordinatesFromNode(fromNode, 
                                                     fromLat1, fromLon1, 
                                                     fromLat2, fromLon2))) {
                     mc2log << fatal << here << "Failed to lookup fromNode"
                            << endl;
                     continue;
                  }

                  log << "setTurnDirection;(" << fromLat1 << ", " << fromLon1
                      << ");(" << fromLat2 << ", " << fromLon2 << ");("
                      << lat1 << ", " << lon1 << ");(" 
                      << lat2 << ", " << lon2 << ");" 
                      << StringTable::getString(
                           ItemTypes::getTurndirectionSC(
                              con->getTurnDirection()),
                           StringTable::ENGLISH)
                      << ";EndOfRecord;" << endl;
                  nbrSavedTurns++;
               }
            }
         }
         if ((nbrSavedTurns%1000) == 0)
            cout << "." << flush;
      }
   }
   cout << endl;
   
   mc2log << info << "Saved " << nbrSavedTurns << " turndescriptions into "
          << fileName << ", in "
          << uint32(TimeUtility::getCurrentTime()-startTime) << "ms" << endl;
}

void
compareTurndescriptions(OldGenericMap* theMap, const char* fileName)
{
   // Get current time
   uint32 startTime = TimeUtility::getCurrentTime();
   
   // Stream where to read the turndescriptions from
   ifstream is(fileName);
   if (!is) {
      mc2log << error << "File not found (\"" << fileName << "\")" << endl;
      return ;
   }
   
   // The number of turndescriptions
   uint32 nbrReadTurns = 0;
   uint32 nbrDiffer = 0;

   vector<MC2String> rec;
   bool recordOK = OldExtraDataUtility::readNextRecordFromFile(is, rec);
   while (recordOK && (rec[0].size() > 0)) {
      // SET_TURNDIRECTION-record definition:
      // 0 "setTurnDirection"
      // 1. (lat1,lon1)
      // 2. (lat2,lon2)
      // 3. (lat1,lon1)
      // 4. (lat2,lon2)
      // 5. value
      
      if (rec[0] == "setTurnDirection") {
         
         OldNode* fromNode = OldExtraDataUtility::getNodeFromStrings(
                                                   theMap, 
                                                   CoordinateTransformer::mc2,
                                                   rec[1], 
                                                   rec[2]);
         OldNode* toNode = OldExtraDataUtility::getNodeFromStrings(
                                                   theMap, 
                                                   CoordinateTransformer::mc2,
                                                   rec[3], 
                                                   rec[4]);
         int val = ItemTypes::getTurnDirection(rec[5].c_str(),
                                               StringTable::ENGLISH);
         if ( (val >= 0) && (fromNode != NULL) && (toNode != NULL)){
            // Get the connection from fromNode to toNode
            OldConnection* con = NULL;
            uint32 j=0;
            uint32 fromNodeID = fromNode->getNodeID();
            while ( (j < toNode->getNbrConnections()) && 
                    (con == NULL) ) {
               OldConnection* tmpCon = toNode->getEntryConnection(j);
               if ( (tmpCon != NULL) && 
                    (tmpCon->getFromNode() == fromNodeID)) {
                  con = tmpCon; 
               }     
               j++;                       
            }

            // Check connection
            if (con != NULL) {
               if ( con->getTurnDirection() != 
                    ItemTypes::turndirection_t(val)) {
                  mc2log << info << "Turndirection differs: fromID=" 
                         << fromNode->getNodeID() << ",toID=" 
                         << toNode->getNodeID() << ", oldDesc="
                         << rec[5] << ", newDesc=" 
                         << StringTable::getString(
                               ItemTypes::getTurndirectionSC(
                                  con->getTurnDirection()),
                               StringTable::ENGLISH) << endl;
                  nbrDiffer++;
               }
            }
         }
         ++nbrReadTurns;
         if ((nbrReadTurns%1000) == 0)
            cout << "." << flush;
      }

      // Get the next record
      recordOK = OldExtraDataUtility::readNextRecordFromFile(is, rec);
   }
   cout << endl;

   // Write status etc.
   mc2log << info << "Compared " << nbrReadTurns << " turns from "
          << fileName << ", "<< nbrDiffer << " differs in " 
          << uint32(TimeUtility::getCurrentTime()-startTime) << "ms" << endl;
}

void
compareTurndescriptions(OldGenericMap* map1, OldGenericMap* map2)
{
   // Get current time
   uint32 startTime = TimeUtility::getCurrentTime();

   // Check if the maps seems to have the same data.
   for (uint32 i=0; i<NUMBER_GFX_ZOOMLEVELS; i++) {
      if (map1->getNbrItemsWithZoom(i) != map2->getNbrItemsWithZoom(i)) {
         mc2log << error << "Nbr items on zoom " << i << " differs" << endl;
         return;
      }

      for (uint32 j=0; j<map1->getNbrItemsWithZoom(i); j++) {
         OldItem* item1 = map1->getItem(i, j);
         OldItem* item2 = map2->getItem(i, j);
         if((item1 != NULL) && (item2 != NULL)){  
            if (item1->getItemType() != item2->getItemType()) {
               mc2log << error << "OldItem types differs" << endl;
               return;
            }
            if ( (item1->getGfxData() != NULL) &&
                 (item1->getGfxData()->getNbrCoordinates(0) != 
                  item2->getGfxData()->getNbrCoordinates(0))) {
               mc2log << error << "Nbr coordinates differ" << endl;
               return;
            }
         }
      }
   }
   
   // Check all connections
   uint32 mapID = map1->getMapID();
   uint32 nbrCheckedTurns = 0;
   uint32 nbrDiffs = 0;
   for (uint32 i=0; i<NUMBER_GFX_ZOOMLEVELS; i++) {
      for (uint32 j=0; j<map1->getNbrItemsWithZoom(i); j++) {
         OldItem* item1 = map1->getItem(i, j);
         OldItem* item2 = map2->getItem(i, j);
         if ( (item1 != NULL) && 
              (item2 != NULL) &&
              (item1->getItemType() == ItemTypes::streetSegmentItem) &&
              (item2->getItemType() == ItemTypes::streetSegmentItem)) {
            // Check both nodes
            for (uint32 nodeNbr=0; nodeNbr<2; nodeNbr++) {
               OldNode* node1 = static_cast<OldStreetSegmentItem*>
                                       (item1)->getNode(nodeNbr);
               OldNode* node2 = static_cast<OldStreetSegmentItem*>
                                       (item2)->getNode(nodeNbr);
               
               // Save turndescriptions in all connections
               for (uint32 c=0; c<node1->getNbrConnections(); c++) {
                  OldConnection* con1 = node1->getEntryConnection(c);
                  OldConnection* con2 = node2->getEntryConnection(c);
                  nbrCheckedTurns++;

                  // Check turndescriptions
                  if (con1->getTurnDirection() != con2->getTurnDirection()) {
                     cerr << "Diff. " << dec
                          << "con1{" << StringTable::getString(
                               ItemTypes::getTurndirectionSC(
                                  con1->getTurnDirection()),
                               StringTable::ENGLISH) << ","
                          << StringTable::getString(
                               ItemTypes::getCrossingKindSC(
                                  con1->getCrossingKind()),
                               StringTable::ENGLISH) << ",from:"
                          << con1->getConnectFromNode() << ",to:"
                          << node1->getNodeID() << "} con2{"
                          << StringTable::getString(
                               ItemTypes::getTurndirectionSC(
                                  con2->getTurnDirection()),
                               StringTable::ENGLISH) << ","
                          << StringTable::getString(
                               ItemTypes::getCrossingKindSC(
                                  con2->getCrossingKind()),
                               StringTable::ENGLISH) << ",from:"
                          << con2->getConnectFromNode() << ",to:"
                          << node2->getNodeID() << "}" << endl;
                     // FromID;ToID;Old turndesc; Old crossingkind;
                     cerr << "TURNDESCRIPTION: " << dec 
                          << mapID << "." << con1->getConnectFromNode() << ";"
                          << mapID << "." << node1->getNodeID() << ";"
                          << int(con1->getTurnDirection()) << ";"
                          << int(con1->getCrossingKind()) << ";"
                          << endl;
                     nbrDiffs++;
                  }
               }
            }
         }
      }
   }
   
   mc2log << info << nbrDiffs << " turns differs (" << nbrCheckedTurns 
          << "), " << uint32(TimeUtility::getCurrentTime()-startTime) << "ms" 
          << endl;
}

void
saveTrafficRules(OldGenericMap* theMap, const char* fileName)
{
   // Stream where to save the turndescriptions
   ofstream log(fileName);

   // Statistics
   uint32 nbrNodesSaved = 0;
   uint32 nbrConnectionsSaved = 0;

   for (uint32 i=0; i<NUMBER_GFX_ZOOMLEVELS; i++) {
      for (uint32 j=0; j<theMap->getNbrItemsWithZoom(i); j++) {
         OldItem* item = theMap->getItem(i, j);
         if ( (item != NULL) && 
              (item->getItemType() == ItemTypes::streetSegmentItem)) {
            for (uint32 nodeNbr=0; nodeNbr<2; nodeNbr++) {
               OldNode* node = static_cast<OldStreetSegmentItem*>
                                       (item)->getNode(nodeNbr);
               // Get coordinates for this node
               int32 lat1, lon1, lat2, lon2;
               if ( (node == NULL) ||
                    (!theMap->getCoordinatesFromNode(node, lat1, lon1, 
                                                  lat2, lon2))) {
                  mc2log << fatal << here << " Failed to lookup node" << endl;
                  continue;
               }

               // Check if to save entry restrictions for this node
               if (node->getEntryRestrictions() != ItemTypes::noRestrictions) {
                  log << "setEntryRestrictions;(" 
                      << lat1 << ", " << lon1 << ");(" 
                      << lat2 << ", " << lon2 << ");"
                      << StringTable::getString( 
                            ItemTypes::getEntryRestrictionSC(
                               node->getEntryRestrictions()),
                            StringTable::ENGLISH) 
                      << ";EndOfRecord;" << endl;
                  ++nbrNodesSaved;
               }
               
               // Check all connections
               for (uint32 c=0; c<node->getNbrConnections(); c++) {
                  OldConnection* con = node->getEntryConnection(c);

                  if (con->getVehicleRestrictions() != MAX_UINT32) {

                     // Get coordinates for from-node
                     OldNode* fromNode = theMap->nodeLookup(
                                                con->getConnectFromNode());
                     int32 fromLat1, fromLon1, fromLat2, fromLon2;
                     if ( (fromNode == NULL) ||
                          (!theMap->getCoordinatesFromNode(fromNode, 
                                                        fromLat1, fromLon1, 
                                                        fromLat2, fromLon2))) {
                        mc2log << fatal << here << "Failed to lookup fromNode"
                               << endl;
                        continue;
                     }

                     log << "setVehicleRestrictions;(" 
                         << fromLat1 << ", " << fromLon1 << ");(" 
                         << fromLat2 << ", " << fromLon2 << ");("
                         << lat1 << ", " << lon1 << ");(" 
                         << lat2 << ", " << lon2 << ");0x" 
                         << hex << con->getVehicleRestrictions() << dec 
                         << ";EndOfRecord;" << endl;
                     ++nbrConnectionsSaved;
                  }
               }
            }
         }
      }
   }
   
   mc2log << info << "Saved traffic rules for " << nbrNodesSaved 
          << " nodes and " << nbrConnectionsSaved << " connections" << endl;
}

void
countItems(OldGenericMap* theMap, uint32& totNbrItemsAllMaps )
{
   // Variables to hold the statistics
   int nbrItems[NUMBER_GFX_ZOOMLEVELS];
   uint32 itemMem[NUMBER_GFX_ZOOMLEVELS];

   uint32 nbrItemsPerType[ItemTypes::numberOfItemTypes];
   uint32 itemsPerTypeMem[ItemTypes::numberOfItemTypes];

   int nbrItemsPerZoom[NUMBER_GFX_ZOOMLEVELS][ItemTypes::numberOfItemTypes];
   int itemsPerZoomMem[NUMBER_GFX_ZOOMLEVELS][ItemTypes::numberOfItemTypes];

   int nbrConnections = 0;
   int nbrGfxData = 0;
   int nbrGfxDataSinglePoly = 0;
   int nbrGfxDataSingleCoord = 0;
   int nbrNodes = 0;

   
   // Reset the counters
   for(uint32 i = 0; i < NUMBER_GFX_ZOOMLEVELS; i++ ) {
      nbrItems[i] = 0;
      itemMem[i] = 0;
      for (uint32 t = 0; t < ItemTypes::numberOfItemTypes; t++) {
         nbrItemsPerZoom[i][t] = 0;
         itemsPerZoomMem[i][t] = 0;
      }
   }
   for (uint32 t = 0; t < ItemTypes::numberOfItemTypes; t++) {
      nbrItemsPerType[t] = 0;
      itemsPerTypeMem[t] = 0;
   }
   int totalNbrItems = 0;
   uint32 totalMem = 0;

   
   for (uint32 i=0; i<NUMBER_GFX_ZOOMLEVELS; i++) {
      for (uint32 j=0; j<theMap->getNbrItemsWithZoom(i); j++) {
         OldItem* item = theMap->getItem(i, j);
         if ( (item != NULL) ) {
            uint32 memUsage = item->getMemoryUsage();            
            nbrItems[i]++;
            itemMem[i] += memUsage;
            ItemTypes::itemType type = item->getItemType();
            ++nbrItemsPerZoom[i][type];
            itemsPerZoomMem[i][type] += memUsage;

            if (item->getGfxData() != NULL) {
               ++nbrGfxData;
               if (item->getGfxData()->getNbrCoordinates(0) == 1) {
                  ++nbrGfxDataSingleCoord;
               }

               if (item->getGfxData()->getNbrPolygons() == 1) {
                  ++nbrGfxDataSinglePoly;
               }
            }

            if (type == ItemTypes::streetSegmentItem) {
               nbrNodes += 2;
               OldStreetSegmentItem* ssi = static_cast<OldStreetSegmentItem*>(item);
               nbrConnections += ssi->getNode(0)->getNbrConnections();
               nbrConnections += ssi->getNode(1)->getNbrConnections();
            }
            
         }
      }
      totalNbrItems += nbrItems[i];
      totalMem      += itemMem[i];
      for (uint t = 0; t < ItemTypes::numberOfItemTypes; t++) {
         nbrItemsPerType[t] += nbrItemsPerZoom[i][t];
         itemsPerTypeMem[t] += itemsPerZoomMem[i][t];
      }
   }
   totNbrItemsAllMaps += totalNbrItems;

   // Print the result to standard out
   cout << "Total number of items for map " << theMap->getMapID() << ":"
        << totalNbrItems << endl;
   cout << "Total memory usage for items  "
        << (totalMem / 1024.0) << " kb" << endl;
   cout << "Nbr GfxData " << nbrGfxData << ", #singlePoly="
        << nbrGfxDataSinglePoly << ", #singleCoord=" 
        << nbrGfxDataSingleCoord << endl;

   for (uint32 t = 0; t < ItemTypes::numberOfItemTypes; t++) {
      if (nbrItemsPerType[t] != 0) {
         cout << StringTable::getString(
                     ItemTypes::getItemTypeSC(ItemTypes::itemType(t)), 
                     StringTable::ENGLISH)
              << " nbrItems = " << nbrItemsPerType[t]
              << ", mem usage = " << ( itemsPerTypeMem[t] / 1024.0 ) 
              << " kb" << endl;
         if (ItemTypes::itemType(t) == ItemTypes::streetSegmentItem) {
            cout << "   Nbr nodes " << nbrNodes << ", nbrConnections "
                 << nbrConnections << endl;
         }
      }
   }
      
   for(uint32 i = 0; i < NUMBER_GFX_ZOOMLEVELS; i++ ) {
      if ( nbrItems[i] != 0 ) {
         cout << "Zoomlevel [" << i << "], nbr items = " << nbrItems[i]
              << ", mem usage = " << (itemMem[i] / 1024.0) << " kb"
              << endl;
      }
      for (uint32 t = 0; t < ItemTypes::numberOfItemTypes; t++){
         if (nbrItemsPerZoom[i][t] != 0) {
            cout << "   " << StringTable::getString(
                        ItemTypes::getItemTypeSC(ItemTypes::itemType(t)), 
                        StringTable::ENGLISH)
                 << " nbrItems = " << nbrItemsPerZoom[i][t]
                 << ", mem usage = " << (itemsPerZoomMem[i][t] / 1024.0)
                 << " kb" << endl;
         }
      }
   }
}


void
printItemsOfType(OldGenericMap* theMap, ItemTypes::itemType itemType)
{
   cout << " The " 
        << StringTable::getString(ItemTypes::getItemTypeSC(itemType),
                                  StringTable::ENGLISH) 
        << "s in this map:" << endl;

   uint32 nbrNoName = 0;
   uint32 nbrNoNameBuaInNoNameMunicipal = 0;
   uint32 nbrPolys = 0;
   double polyArea = 0;
   uint32 nbrPolysNoNameBuaInNoNameMunicipal = 0;
   double polyAreaNNBINNM = 0;
   uint32 count = 0;
   for (uint32 i=0; i<NUMBER_GFX_ZOOMLEVELS; i++) {
      for (uint32 j=0; j<theMap->getNbrItemsWithZoom(i); j++) {
         OldItem* item = theMap->getItem(i, j);
         if ( (item != NULL) && 
              (item->getItemType() == itemType) ) {
            count++;
            
            cout << "   " << theMap->getMapID() << ":" << item->getID() << " "
                 << theMap->getFirstItemName(item) << endl;

            GfxData* gfx = item->getGfxData();
            if ( gfx != NULL) {
               nbrPolys += gfx->getNbrPolygons();
               if ( gfx->closed() ) {
                  for (uint32 p = 0; p < gfx->getNbrPolygons(); p++) {
                     polyArea += gfx->polygonArea(p);
                  }
               }
            }
            
            if ( item->getNbrNames() < 1 ) {
               //cout << "No name item " << theMap->getMapID() 
               //     << ":" << item->getID() << endl;
               nbrNoName++;

               if ( itemType == ItemTypes::builtUpAreaItem ) {
               // check if in no-name municipal
                  OldItem* mun = 
                     theMap->getRegion(item, ItemTypes::municipalItem);
                  bool action = false;
                  if ( mun == NULL ) {
                     action = true;
                     cout << "No-name bua in null municipal "
                          << theMap->getMapID() << ":" << item->getID()
                          << " " << theMap->getMapName() << endl; 
                  } else {
                     if ( mun->getNbrNames() < 1 ) {
                        action = true;
                        cout << "No-name bua in no-name municipal "
                             << theMap->getMapID() << ":" << item->getID()
                             << " " << theMap->getMapName() << endl; 
                     } else {
                        cout << "No-name bua in OK municipal "
                             << theMap->getMapID() << ":" << item->getID()
                             << " " << theMap->getMapName() << endl; 
                     }
                  }
                  if (action) {
                     nbrNoNameBuaInNoNameMunicipal++;
                     if ( gfx != NULL) {
                        nbrPolysNoNameBuaInNoNameMunicipal
                              += gfx->getNbrPolygons();
                        if ( gfx->closed() ) {
                           for (uint32 p = 0; 
                                p < gfx->getNbrPolygons(); p++) {
                              polyAreaNNBINNM += gfx->polygonArea(p);
                           }
                        }
                     }
                  }
               }
            }

//            // Print all names:
//            for ( uint32 nameIdx=0; nameIdx<item->getNbrNames(); nameIdx++)
//            {
//               uint32 strIdx = item->getStringIndex(nameIdx);
//               const char* name = theMap->getName(strIdx);
//               cout << "   " << name;
//            }
//            cout << endl;


            /*
            if (item->getNbrNames() > 0) {
               cout << " " << theMap->getMapID() << ":" << item->getID();
               for ( uint32 j = 0; j < item->getNbrNames(); j++ ) {
                  LangTypes::language_t strLang;
                  ItemTypes::name_t strType;
                  uint32 strIdx;

                  item->getNameAndType( j, strLang, strType, strIdx );

                  cout << " " << theMap->getName( strIdx ) << ":"
                       << ItemTypes::getNameTypeAsString(strType, true) << ":"
                       << LangTypes::getLanguageAsString(strLang);
               }
               cout << endl;
            }*/

         }
      }
   }
   mc2dbg << "Printed " << count << " " 
          << ItemTypes::getItemTypeAsString(itemType) << " items"
          << endl;
   if ( itemType == ItemTypes::builtUpAreaItem ) {
      mc2dbg << "Map " << theMap->getMapID() << " " << theMap->getMapName()
             << " nbr no names = " << nbrNoName 
             << " nbr in no-name municipal = " << nbrNoNameBuaInNoNameMunicipal
             << " nbrPolys=" << nbrPolys
             //<< " polyArea=" << polyArea
             << " nbrPolysNoNameBuaInNoNameMunicipal="
             << nbrPolysNoNameBuaInNoNameMunicipal
             << " polyAreaNNBINNM=" << polyAreaNNBINNM
             << " (" << polyAreaNNBINNM/polyArea << ")"
             << endl;
   }
}

void
printExternalConnections(OldGenericMap* theMap)
{
   OldBoundrySegmentsVector* segments = theMap->getBoundrySegments();
   MC2_ASSERT(segments != NULL);

   mc2log << info << "To print info about " << segments->getSize() 
          << " boundry segments" << endl;

   uint32 nbrConnections = 0;
   for (uint32 i=0; i<segments->getSize(); ++i) {
      OldBoundrySegment* seg = static_cast<OldBoundrySegment*>
                                       (segments->getElementAt(i));
      for (uint32 n=0; n<2; ++n) {
         for (uint32 j=0; j<seg->getNbrConnectionsToNode(n); ++j) {
            cout << "      " << seg->getFromMapIDToNode(n, j) << "."
                 << seg->getConnectToNode(n, j)->getConnectFromNode() << " -> "
                 << theMap->getMapID() << "."
                 << seg->getConnectRouteableItemID() 
                 << "  turn "
                 << int (seg->getConnectToNode(n, j)->getTurnDirection())
                 << endl;
            ++nbrConnections;
         }
      }
   }
   mc2log << info << "Totaly " << nbrConnections 
          << " external connections" << endl;
}


void
loadSuspectTurns(vector<pair<MC2String, vector<IDPair_t> > >& outVector,
                 MC2String fileNameStr)
          
{
   // This function is cut from MapEditor...
   ifstream turnFile(fileNameStr.c_str());
   mc2dbg << "Opening file: \"" << fileNameStr << "\"" << endl;
   
   char row[256];
   if ((fileNameStr.length() > 0) && (turnFile)) {
      while (!turnFile.eof()) {
         turnFile.getline(row, 256);
         const char* TURNDESC_REC = "TURNDESCRIPTION:";
         const char* NODE_REC = "NODE:";
         if (strncmp(row, TURNDESC_REC, strlen(TURNDESC_REC)) == 0) {
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
               
               /*mc2dbg << i << " val=" << curVal << ", curPos: " 
                 << curPos << endl;*/
               curPos = strchr(curPos, ';') + 1;
            }
            if ( fromMapID == toMapID ) {
               vector<IDPair_t> ssiVect;
               ssiVect.push_back(IDPair_t(fromMapID, fromID));
               ssiVect.push_back(IDPair_t(toMapID, toID));
               outVector.push_back( make_pair(TURNDESC_REC, ssiVect) );
            } else {
               mc2log << warn << "loadSuspectTurns - different from and to "
                      << " maps not handled " << endl;
            }
         } else if (strncmp(row, NODE_REC , strlen(NODE_REC)) == 0) {
            // Format: MapID.FromID;
            char * curPos = row+strlen(NODE_REC)+1;
            uint32 mapID = strtoul(curPos, NULL, 0);
            curPos = strchr(curPos, '.') + 1;
            uint32 nodeID = strtoul(curPos, NULL, 0);
            vector<IDPair_t> ssiVect;
            ssiVect.push_back(IDPair_t(mapID, nodeID));
            outVector.push_back( make_pair(NODE_REC, ssiVect));
         }
      }
   }
}

void
printSuspectItemCoordinates(OldGenericMap* theMap,
                  const vector<pair<MC2String, vector<IDPair_t> > >& items)
{
   for ( uint i = 0; i < items.size(); ++i ) {
      if ( items[i].second[0].getMapID() != theMap->getMapID() ) {
         continue;
      }
      const vector<IDPair_t>& itemsToCheck = items[i].second;
      uint32 itemID = MAX_UINT32;
      if ( itemsToCheck.size() == 2 ) {
         // Two nodes - check the to - node.
         itemID = itemsToCheck[1].getItemID();
      } else {
         // One node
         itemID = itemsToCheck[0].getItemID();
      }

      
      OldItem* item = theMap->itemLookup(itemID);
      int nodeNbr = !MapBits::isNode0(itemID);
      OldNode* currentNode = (static_cast<OldRouteableItem*> (item))
         ->getNode(nodeNbr);
      if ( currentNode == NULL ) {
         mc2dbg << "printSuspectItemCoordinates: node is NOLL" << endl;
         continue;
      }
      // The coorinates (lat, lon)
      int32 nodeLat = MAX_INT32;
      int32 nodeLon = MAX_INT32;
      theMap->getNodeCoordinates(itemID, nodeLat, nodeLon);
      float64 latDeg;
      float64 lonDeg;
      float64 z;
      CoordinateTransformer::
         transformFromMC2(nodeLat, nodeLon,
                          CoordinateTransformer::wgs84deg,
                          latDeg,
                          lonDeg,
                          z);
      
      char tmpStr[1024];
      sprintf(tmpStr, "%s %.10f %.10f", items[i].first.c_str(),
              latDeg, lonDeg);
      
      cout << tmpStr
           << endl;
   }
}



void 
filterMap( OldGenericMap* theMap ) 
{
   mc2dbg << "Begin filtering of Maps." << endl;
   set<OldGenericMap*> mapsToConsider;
   mapsToConsider.insert( theMap );

   set<ItemTypes::itemType> itemTypes;
   itemTypes.insert( ItemTypes::waterItem );
   itemTypes.insert( ItemTypes::parkItem );

   vector<int> filterMaxDists;
   mc2dbg << "Setting filterlevels." << endl;
   int filterLevel = 1;
   filterMaxDists.push_back( filterLevel );
   mc2dbg << filterLevel;
   
   filterLevel = 2;
   for( int i = 1; i < 15; i++ ) {
      mc2dbg << ", " << filterLevel;
      filterMaxDists.push_back( filterLevel );
      filterLevel = (int)(filterLevel * 1.5);
   }
   mc2dbg << endl;
   
   OldMapFilter mapFilter;

   mapFilter.filter( theMap, mapsToConsider, itemTypes, 
                     false, filterMaxDists /*, 2*/ );
}

bool
checkMapssicoordFile(OldGenericMap* theMap, char* filename)
{
   ifstream coordfile(filename);
   if ( ! coordfile ) {
      cerr << "Cannot open map ssi file '"
           << filename << "'" << endl
           << " - does not exists?" << endl;
      exit(1);
   }
   char coordbuffer[50];
   coordbuffer[0] = '\0';
   bool coordValid = false;

   // loop the map ssi coord file until we have this mapid.
   uint32 curMapID = theMap->getMapID();
   bool cont = true;
   while (cont && !coordfile.eof()) {
      coordfile.getline(coordbuffer, 50);
      char* mapidStr = coordbuffer;
      mc2dbg8 << "  " << curMapID << " mapidStr=" << mapidStr << endl;
      if ( strlen(mapidStr) < 1 ) {
         // eof
      } else {
         char* latStr = strchr(mapidStr, ',')+1;
         char* lonStr = strchr(latStr, ',')+1;
         uint32 mapid = strtoul(mapidStr, NULL, 10);
         int32 lat = strtol(latStr, NULL, 10);
         int32 lon = strtol(lonStr, NULL, 10);
         if (mapid == curMapID) {
            // check if there is a ssi within 2 meters from the coord
            OldMapHashTable* mht = theMap->getHashTable();
            MC2_ASSERT(mht != NULL);
            mht->clearAllowedItemTypes();
            mht->addAllowedItemType(ItemTypes::streetSegmentItem);
            // Find closest street segment
            uint64 dist;
            uint32 closestSSI = mht->getClosest(lon, lat, dist);
            mc2dbg1 << "For map " << curMapID << ": found closest ssi " 
                    << closestSSI << " dist="
                    << sqrt(dist)*GfxConstants::MC2SCALE_TO_METER 
                    << " meters" << endl;

            if (sqrt(dist)*GfxConstants::MC2SCALE_TO_METER < 2) {
               // coord is valid
               coordValid = true;
            }
            
            cont = false;
         } else if (mapid > curMapID) {
            cont = false;
         }
      }
   }
   
   return coordValid;
}

bool
setTimesInMap( OldGenericMap* theMap, const char* timeType,
               const char* timeValue )
{
   bool retVal = false;
   if ( strlen(timeValue) != 19 ) {
      mc2log << error << "Incorrect timeValue" << endl;
      return false;
   }
   
   // We want a uint32 UNIX time from the timeValue-string.
   // 1. Extract year+month+day hour+min+sec from the timeValue-string
   // 2. Create a broken down time (struct tm)
   // 3. Convert to calender time  (time_t ~uint32)

   // 1. Extract
   uint32 year, month, day, hour, min, sec;
   char* yearStr  = StringUtility::newStrDup(timeValue);
   char* monthStr = strchr(yearStr, '-')+1;
   char* dayStr   = strchr(monthStr, '-')+1;
   char* hourStr  = strchr(dayStr, ' ')+1;
   char* minStr   = strchr(hourStr, ':')+1;
   char* secStr   = strchr(minStr, ':')+1;
   mc2dbg8 << " yearStr=" << yearStr << ", monthStr=" << monthStr << endl
           << " dayStr=" << dayStr << ", hourStr=" << hourStr << endl
           << " minStr=" << minStr << ", secStr=" << secStr << endl;
   char* tmpStr = strchr(yearStr, '-');
   tmpStr[0] = '\0';
   tmpStr = strchr(monthStr, '-');
   tmpStr[0] = '\0';
   tmpStr = strchr(dayStr, ' ');
   tmpStr[0] = '\0';
   tmpStr = strchr(hourStr, ':');
   tmpStr[0] = '\0';
   tmpStr = strchr(minStr, ':');
   tmpStr[0] = '\0';
   mc2dbg8 << " yearStr=" << yearStr << ", monthStr=" << monthStr << endl
           << " dayStr=" << dayStr << ", hourStr=" << hourStr << endl
           << " minStr=" << minStr << ", secStr=" << secStr << endl;

   char* dest = NULL;
   if ( Utility::getUint32(yearStr, dest, year) )
      if ( Utility::getUint32(monthStr, dest, month) )
         if ( Utility::getUint32(dayStr, dest, day) )
            if ( Utility::getUint32(hourStr, dest, hour) )
               if ( Utility::getUint32(minStr, dest, min) )
                  if ( Utility::getUint32(secStr, dest, sec) )
                     retVal = true;

   if ( retVal ) {
      
      retVal = false;
      // 2. Create broken down time
      struct tm *newTimeTM = new tm;
      newTimeTM->tm_year = (year - 1900);
      newTimeTM->tm_mon  = (month - 1);
      newTimeTM->tm_mday = day;
      newTimeTM->tm_hour = hour;
      newTimeTM->tm_min  = min;
      newTimeTM->tm_sec  = sec;

      // 3. Get time_t from the broken down time
      time_t newTime = mktime(newTimeTM);

      // Set this time in theMap, waspTime or trueCreationTime
      if (strcasecmp(timeType, "waspTime") == 0) {
         theMap->setWaspTime(newTime);
         retVal = true;
      } else if (strcasecmp(timeType, "trueCreationTime") == 0) {
         theMap->setTrueCreationTime(newTime);
         retVal = true;
      }
      delete newTimeTM;
   }

   delete yearStr;
   return retVal;
}

bool
checkValidTimes(OldGenericMap* theMap)
{
   // Check that trueCreationTime and waspTime are valid in the map.
   // This function is used before running dynamic extra data and 
   // wasp, to make sure the result will be ok.
   
   bool allTimesValid = true;
   uint32 mapId = theMap->getMapID();
   if (theMap->getTrueCreationTime() == MAX_UINT32) {
      cout << "Map " << mapId << " has invalid trueCreationTime" << endl;
      allTimesValid = false;
   }
   if (theMap->getWaspTime() == MAX_UINT32) {
      cout << "Map " << mapId << " has invalid waspTime" << endl;
      allTimesValid = false;
   }

   // Print WASP time and true creation time (asctime prints endl).
   time_t trueCreationTime = time_t(theMap->getTrueCreationTime());
   cout << "True creation time (" << mapId << "): " 
        <<  asctime(localtime(&trueCreationTime));
   time_t waspTime = time_t(theMap->getWaspTime());
   cout << "WASP time (" << mapId << "): " 
        <<  asctime(localtime(&waspTime));
   time_t dynedTime = time_t(theMap->getDynamicExtradataTime());
   cout << "Dyn ed time (" << mapId << "): " 
        <<  asctime(localtime(&dynedTime)) << endl;
   
   return allTimesValid;
}



void
printHeader(OldGenericMap* theMap){
   mc2log << info  << endl;
   mc2log << info  << endl;
   mc2log << info << "Prints map header of map "
          <<theMap->getMapName() << endl;
  
   vector<MC2String> toPrint = theMap->getHeaderAsStrings();

   for ( uint32 i=0; i<toPrint.size(); i++ ){
      mc2log << info << "   " << toPrint[i] << endl;
   }

   mc2log << info  << endl;
} // printHeader



//class NameAndTypeAndLang {
//public:
//   NameAndTypeAndLang( OldGenericMap* map, OldItem* item, uint32 nameOffset){
//      m_type = item->getNameType(nameOffset);
//      m_lang = item->getNameLanguage(nameOffset);
//      uint32 strIdx = item->getStringIndex(nameOffset);
//      m_name = map->getName(strIdx);
//   }
//
//   LangTypes::language_t m_lang;
//   ItemTypes::name_t m_type;
//   MC2String m_name;
//
//}; 
//
//bool operator== ( const NameAndTypeAndLang& lname, 
//                  const NameAndTypeAndLang& rname ) {
//   return ( ( lname.m_type == rname.m_type) &&
//            ( lname.m_lang == rname.m_lang) &&
//            ( lname.m_name == rname.m_name) );
//}
//
//bool operator< ( const NameAndTypeAndLang& lname, 
//                 const NameAndTypeAndLang& rname ) {
//   return ( ( lname.m_type < rname.m_type) &&
//            ( lname.m_lang < rname.m_lang) &&
//            ( lname.m_name < rname.m_name) );
//}
//   




bool
updateMapGfx( OldGenericMap* theMap, const char* fileName )
{
   mc2dbg << "Update map gfx data of map " << theMap->getMapID()
          << " from file " << fileName << endl;
   ifstream mif( fileName );
   if ( !mif ) {
      return false;
   }
   
   GMSGfxData* newGfx = static_cast<GMSGfxData*>
      (GfxData::createNewGfxData( NULL ));
   if ( ! newGfx->createFromMif(mif) ) {
      return false;
   }
   newGfx->updateLength();
   newGfx->sortPolygons();

   mc2dbg << "The map has gfx with " 
          << theMap->getGfxData()->getTotalNbrCoordinates()
          << " coords" << endl
          << "Replacing with " << newGfx->getTotalNbrCoordinates()
          << " coords" << endl;
   theMap->setGfxData( newGfx );
   theMap->setMapGfxDataFiltered(false);  // map gfx data not coord filtered

   return true;
}



void
mapLab2( OldGenericMap* theMap, const char* action, uint32 mapNbr ) {

   // Set new map gfx data from mif.
   // Creates new gfx from mif file given and replaces the map gfx data.
   // If co map: before adding to merge: create new coordinate filtering and 
   // gfx filtering with GMS.
   if ( strcasecmp(action, "setNewMapGfx") == 0 ) {
      // get mif file name from co map name
      const char* path = "./";
      char* file = new char[32];
      MC2String tmp = theMap->getMapName();
      MC2String mapName = StringUtility::copyLower(tmp);
      sprintf(file, "%s%s.mif", path, mapName.c_str());
      
      if ( updateMapGfx( theMap, file ) ) {
         mc2log << info << "Map gfx data was updated, now has "
                << theMap->getGfxData()->getTotalNbrCoordinates()
                << " coords" << endl;
         theMap->save();
      } else {
         mc2log << error << "Failed to update map gfx for map "
                << theMap->getMapID() << endl;
      }
   }

   else if ( strcasecmp(action, "printNoThroughfareSSIID") == 0 ) {
      uint32 nbrSSI = 0;
      uint32 nbrSSINoThroughfare = 0;
      for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
         for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
            OldItem* item = theMap->getItem(z, i);
            if ( (item != NULL) &&
                 (item->getItemType() == ItemTypes::streetSegmentItem) ) {
               OldStreetSegmentItem* ssi = 
                  dynamic_cast<OldStreetSegmentItem*>(item);
               if ( ssi == NULL ) {
                  continue;
               }
               nbrSSI++;
               
               bool hasNoThroughfare = false;
               for ( uint32 n=0; n < 2; n++ ) {
                  OldNode* node = ssi->getNode(n);
                  if ( node->getEntryRestrictions() 
                           == ItemTypes::noThroughfare ) {
                     hasNoThroughfare = true;
                  }
               }

               if ( hasNoThroughfare ) {
                  nbrSSINoThroughfare++;
                  cout << "noThroughfareSSI " << ssi->getID() << endl;
               }
            }
         }
      }
      mc2log << info << "Number ssi = " << nbrSSI
                     << " nbr with no throughfare = " << nbrSSINoThroughfare
                     << endl;
   }

   else if ( strcasecmp(action, "countSignPostElementClass") == 0 ) {
   
      set<uint32> elemClassesInMap;
      map<uint32, uint32> elemClassNodes;
      const SignPostTable& signPostTable = theMap->getSignPostTable();
      
      for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
         for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
            OldItem* item = theMap->getItem(z, i);
            if ( item == NULL) {
               continue;
            }
            if ( item->getItemType() != ItemTypes::streetSegmentItem ) {
               continue;
            }
            OldStreetSegmentItem* ssi = 
               dynamic_cast<OldStreetSegmentItem*>(item);
            if ( ssi == NULL ) {
               continue;
            }

            for ( uint32 n = 0; n < 2; n++ ) {
               OldNode* node = ssi->getNode(n);
               for (uint32 c =0; c < node->getNbrConnections(); c++ ) {
                  OldConnection* conn = node->getEntryConnection(c);

                  // The sign posts for this conn
                  const vector<GMSSignPost*>& signPosts =
                     signPostTable.getSignPosts( 
                        conn->getFromNode(), node->getNodeID() );
                  
                  for (uint32 i=0; i<signPosts.size(); i++){
                     
                     GMSSignPost* sp = signPosts[i];
                     if ( sp->isEmpty() ) {
                        continue;
                     }
                     vector<GMSSignPostSet> spSets = sp->getSignPostSets();
                     vector<GMSSignPostSet>::const_iterator setIt;
                     for ( setIt = spSets.begin();
                           setIt != spSets.end(); setIt++ ) {
                        GMSSignPostSet spSet = (*setIt);
                        if ( spSet.isEmpty() ) {
                           continue;
                        }
                        vector<GMSSignPostElm> spElems = 
                                 spSet.getSignPostElements();
                        vector<GMSSignPostElm>::const_iterator elemIt;
                        for ( elemIt = spElems.begin();
                              elemIt != spElems.end(); elemIt++ ) {
                           GMSSignPostElm elem = (*elemIt);
                           uint32 elemClass = elem.getElementClass();
                           mc2dbg8 << "SP " << i 
                                   << " elemClass=" << elemClass << endl;
                           elemClassesInMap.insert(elemClass);
                           mc2dbg8 << "SP in "
                                << StringTable::getStringToDisplayForCountry(
                                    theMap->getCountryCode(),
                                    StringTable::ENGLISH)
                                << " elemClass " << elemClass << endl;
                           map<uint32, uint32>::const_iterator 
                              it = elemClassNodes.find(elemClass);
                           if ( it == elemClassNodes.end() ) {
                              elemClassNodes.insert(
                                 make_pair(
                                    elemClass, node->getNodeID()));
                           }
                        }
                        
                     }
                     
                  }
                  
               }
            } // for nodes
         }
      }
      for (set<uint32>::const_iterator sit = elemClassesInMap.begin();
           sit != elemClassesInMap.end(); sit++ ) {
         cout << StringTable::getStringToDisplayForCountry(
                     theMap->getCountryCode(), StringTable::ENGLISH)
              << " SP elem " << *sit << endl;
      }
      for (map<uint32, uint32>::const_iterator it = elemClassNodes.begin();
           it != elemClassNodes.end(); it++ ) {
         cout << StringTable::getStringToDisplayForCountry(
                     theMap->getCountryCode(), StringTable::ENGLISH)
              << " SP elem " << it->first << " node "
              << theMap->getMapID() << ";" << it->second << endl;
      }
   } // countSignPostElementClass

   else if ( strcasecmp(action, "printVirtualRIcoords") == 0 ) {
      // boundry segments = virtual 0-length items
      mc2dbg << "Coordintes for items in the boundry segments vec" << endl;
      OldBoundrySegmentsVector* bsVec = theMap->getBoundrySegments();
      MC2_ASSERT(bsVec != NULL);
      set<MC2Coordinate> bsCoords;
      uint32 nbrVirtuals = 0;
      for (uint32 i=0; i<bsVec->getSize(); ++i) {
         OldBoundrySegment* bs = static_cast<OldBoundrySegment*>
                                          (bsVec->getElementAt(i));
         uint32 riID = bs->getConnectRouteableItemID();
         OldRouteableItem* ri = 
            static_cast<OldRouteableItem*>(theMap->itemLookup(riID));
         if ( ri != NULL ) {
            MC2Coordinate mc2Coord = 
               MC2Coordinate(ri->getGfxData()->getLat(0,0),
                             ri->getGfxData()->getLon(0,0) );
            bsCoords.insert( mc2Coord );
            nbrVirtuals++;
         }
      }
      mc2dbg << "Collected " << bsCoords.size() << " coords from "
             << nbrVirtuals << " bs items" << endl;
      
      mc2dbg << "Print itemID and coordinates of 0-length routeable items"
             << endl;
      uint32 nbrSeg = 0;
      uint32 nbrCentimeterSeg = 0;
      for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
         for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
            OldItem* item = theMap->getItem(z, i);
            if ( item == NULL) {
               continue;
            }
            
            OldRouteableItem* ri = dynamic_cast<OldRouteableItem*>(item);
            if ( ri == NULL ) {
               continue;
            }
            GfxData* gfx = item->getGfxData();
            if ( gfx == NULL ) {
               continue;
            }

            if ( gfx->getLength(0) == 0 ) {
               nbrSeg++;
               cout << "   0-seg " << item->getID() << endl;
               cout << "   coord " << gfx->getLat(0,0) << " " 
                                   << gfx->getLon(0,0) << endl;
            }
            if ( gfx->getLength(0) < 0.01 ) {
               nbrCentimeterSeg++;
            }
         }
      }
      mc2dbg << "Found " << nbrSeg << " items with lenght = 0" << endl
             << "Found " << nbrCentimeterSeg 
             << " items with lenght < 1 cm" << endl;
   }

   
   // Print a certain filter level of the country polygon to mif
   else if ( strcasecmp(action, "printCoPolFiltLevel") == 0 ) {
      // Decide which filt level to print
      uint32 level = 13;
      // Filtered polys will in the end be lines (2 coords). This
      // param tells if to print those line polys or not
      bool includeLinePolys = false;
      cout << "Print map gfx data filter level " << level 
           << ", includeLinePolys=" << int(includeLinePolys)
           << " for " << theMap->getMapName() << endl;
      const GfxData* gfx = theMap->getGfxData();
      uint32 nbrPolysToPrint = gfx->getNbrPolygons();
      cout << " Map gfx data has " << nbrPolysToPrint << " polys" << endl;
      GMSGfxData* newGfx = GMSGfxData::createNewGfxData( theMap );
      uint32 nbrPolysInOutGfx = 0;
      for ( uint32 p = 0; p < nbrPolysToPrint; p++ ) {
         // Store coords of this poly of this filtLevel in a temp gfx
         // then, if we want it in newGfx, copy the coordinates there
         GMSGfxData* tmpGfx = GMSGfxData::createNewGfxData( theMap );
         tmpGfx->addPolygon();
         GfxDataFilterIterator end = gfx->endFilteredPoly( p, level );
         for ( GfxDataFilterIterator it = gfx->beginFilteredPoly( p, level );
               it != end; ++it ) {
            tmpGfx->addCoordinate( (*it).lat, (*it).lon );
         }
         tmpGfx->setClosed( 0, gfx->getClosed( p ) );
         tmpGfx->updateLength();
         int clockwise = tmpGfx->clockWise(0);
         cout << " poly " << p << ":" << gfx->getNbrCoordinates(p) 
              << " -> " << tmpGfx->getNbrCoordinates(0)
              << " cw=" << clockwise << endl;
         if ( includeLinePolys ||
               (!includeLinePolys && (tmpGfx->getNbrCoordinates(0) >= 3)) ) {
            mc2dbg8 << " using " << theMap->getMapName() << " poly " 
                    << p << " cw=" << clockwise << endl;
            newGfx->addPolygon();
            for ( uint32 c = 0; c < tmpGfx->getNbrCoordinates(0); c++ ) {
               newGfx->addCoordinate(
                  tmpGfx->getLat(0,c), tmpGfx->getLon(0,c));
            }
            newGfx->setClosed( nbrPolysInOutGfx, gfx->getClosed( p ) );
            newGfx->updateLength();
            nbrPolysInOutGfx++;
         }
      }
      cout << "print " << theMap->getMapName() 
           << " to file newGfx nbrPolys=" << newGfx->getNbrPolygons()
           << " from=" << gfx->getNbrPolygons() << endl;
      if ( newGfx->getNbrPolygons() > 0 ) {
         ofstream mifFile( "myCountry.mif", ios::app );
         newGfx->printMif(mifFile);
         ofstream midFile( "myCountry.mid", ios::app );
         midFile << mapNbr << ",\"" 
                 << theMap->getMapName() << "\",\""
                 << StringTable::getStringToDisplayForCountry(
                     theMap->getCountryCode(), StringTable::ENGLISH)
                 << "\"" << endl;
      }
      cout << " done" << endl;
   }
   else if ( strcasecmp(action, "dumpBoundrySegments") == 0 ) {
      // print boundry segments
      if ( theMap->getBoundrySegments() != NULL ) {
         theMap->getBoundrySegments()->dump();
      }
   }
   
   else if ( strcasecmp(action, "dumpDisplayClassInfo") == 0 ) {
      mc2dbg << "roadDisplayClass" << endl;
      theMap->printRoadDisplayClassInfo();
      mc2dbg << "areaFeatureDrawDisplayClass" << endl;
      theMap->printAreaFeatureDrawDisplayClassInfo();
  
      if (false) {
         for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
            for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
               OldStreetSegmentItem* ssi = 
                  dynamic_cast<OldStreetSegmentItem*>(theMap->getItem(z, i));
               if ( ssi == NULL ) {
                  continue;
               }
               ItemTypes::roadDisplayClass_t dispClass =
                  theMap->getRoadDisplayClass(ssi->getID());
               cout << ssi->getID() << " " 
                    << int(dispClass) << endl;
            }
         }
      }
   }
   
   else if ( strcasecmp(action, "misc") == 0 ) {


      if (false) {
         // calc distance between two coordinates
         int32 lat1 = 564232127;
         int32 lon1 = 141677779;
         int32 lat2 = 564232148;
         int32 lon2 = 141677961;
         float64 dist = GfxUtility::squareP2Pdistance_linear (
                           lat1, lon1, lat2, lon2 );
         cout << " dist = " << dist << endl;
      }

   }
   
   
   else {
      cout << "mapLab2 nothing todo - exit" << endl;
      MC2_ASSERT(false);
   
   }

}


void
noMapLab( )
{
   // inspect the content of the countryBorders.txt file
   if ( false ) {
      ifstream borderFile("countryBorders.txt");
      const int maxLineLength = 200;
      char lineBuffer[maxLineLength];
      lineBuffer[0] = '\0';
      borderFile.getline( lineBuffer, maxLineLength );
      
      uint32 nbrBordersInFile = 0;
      while ( !borderFile.eof() && (strlen(lineBuffer) > 0) ) {

         if ( strstr(lineBuffer, "BORDERPART") != NULL ) {
            nbrBordersInFile++;
            // Read country for which this borderpart was written
            char* borderCountry = lineBuffer + strlen( "BORDERPART ");
            cout << " border " << nbrBordersInFile 
                 << " borderCountry = '" << borderCountry << "'" << endl;
         }
         // read next line from the file with borders
         borderFile.getline( lineBuffer, maxLineLength );
      }
   }

   
   cout << "noMapLab done" << endl;
}

void
mapLabManyMaps( OldGenericMap* firstMap, uint32 nbrMaps,
                const char*  action, CommandlineOptionHandler coh )
{
   if (action == NULL) {
      mc2log << error << "Need to give action" << endl;
      return;
   }
   
   mc2dbg << "Do mapLabManyMaps " << action << endl;
   const char* mcmName2 = NULL;
   OldGenericMap* map2 = NULL;
   if (nbrMaps == 2) {
      mcmName2 = coh.getTail(1);
      map2 = OldGenericMap::createMap(mcmName2);
      mc2dbg << "Comparing 2 maps, action " << action << endl
             << "1: " << firstMap->getMapName() << ": " 
                      << firstMap->getFilename() << endl
             << "2: " << map2->getMapName() << ": "
                      << map2->getFilename() << endl;
   }

   if ( (strcasecmp(action, "compareAreaFeatureDrawDisplayClass") == 0) &&
        (nbrMaps == 2) ) {

      /*
      map<ItemTypes::areaFeatureDrawDisplayClass_t, uint32> firstMapsDisplayClasses;
      map<ItemTypes::areaFeatureDrawDisplayClass_t, uint32> secondMapsDisplayClasses;
      map<ItemTypes::areaFeatureDrawDisplayClass_t, uint32>::iterator it;
      */
      for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
         for (uint32 i=0; i<firstMap->getNbrItemsWithZoom(z); i++) {
            OldItem* item = firstMap->getItem(z, i);
            if ( item == NULL ) {
               continue;
            }
            uint32 itemID = item->getID();
            ItemTypes::areaFeatureDrawDisplayClass_t dp =
               firstMap->getAreaFeatureDrawDisplayClass(itemID);

            // Comparing with display class of some item in other map
            // assuming that first and second map originate from the same map
            // so itemIDs are linked to the same items.
            ItemTypes::areaFeatureDrawDisplayClass_t dp2 =
               map2->getAreaFeatureDrawDisplayClass(itemID);
            if ( dp != dp2 ) {
               MC2String dpString = "-none-";
               if ( dp != ItemTypes::nbrAreaFeatureDrawDisplayClasses ) {
                  dpString = 
                     ItemTypes::getStringForAreaFeatureDrawDisplayClass(dp);
               }
               MC2String dp2String = "-none-";
               if ( dp2 != ItemTypes::nbrAreaFeatureDrawDisplayClasses ) {
                  dp2String = 
                     ItemTypes::getStringForAreaFeatureDrawDisplayClass(dp2);
               }
               cout << "Differ for item " << itemID 
                    << " " 
                    << ItemTypes::getItemTypeAsString(item->getItemType()) 
                    << " 1: " << dpString
                    << " 2: " << dp2String
                    << endl;
            }
            /*
            if ( dp != ItemTypes::nbrAreaFeatureDrawDisplayClasses ) {
               it = firstMapsDisplayClasses.find(dp);
               if ( it != firstMapsDisplayClasses.end() ) {
                  it->second++;
               } else {
                  firstMapsDisplayClasses.insert( make_pair(dp, 1) );
               }
            } */
         }
      }

      mc2dbg1 << "AreaFeatureDrawDisplayClass first map " 
              << firstMap->getFilename() << endl;
      firstMap->printAreaFeatureDrawDisplayClassInfo();
      mc2dbg1 << "AreaFeatureDrawDisplayClass second map " 
              << map2->getFilename() << endl;
      map2->printAreaFeatureDrawDisplayClassInfo();

   } 
   else if ( (strcasecmp(action, "misc") == 0) && (nbrMaps == 2) ) {

      // compare gfxData of all all items of one type
      ItemTypes::itemType type = ItemTypes::streetSegmentItem;
      uint32 nbrItemsCompared = 0;
      uint32 nbrGfxDataDiffers = 0;
      uint32 nbrLengthDiffers = 0;
      uint32 nbrLengthDiffers1percent = 0;
      uint32 nbrLengthDiffers1meter = 0;
      for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
         for (uint32 i=0; i<firstMap->getNbrItemsWithZoom(z); i++) {
            OldItem* item1 = firstMap->getItem(z, i);
            if ( (item1 == NULL) ||
                 (item1->getItemType() != type) ) {
               continue;
            }
            uint32 itemID = item1->getID();
            OldItem* item2 = map2->itemLookup(itemID);
            if ( (item2 == NULL) || 
                 (item2->getItemType() != type) ) {
               cout << "hmmm, item " << itemID 
                    << " exists in map1 but not in map2" << endl;
               continue;
            }

            GfxData* gfx1 = item1->getGfxData();
            GfxData* gfx2 = item2->getGfxData();
            if ( (gfx1 == NULL) || (gfx2 == NULL) ) {
               continue;
            }

            nbrItemsCompared++;
            if ( ! gfx1->equals(gfx2) ) {
               nbrGfxDataDiffers++;
            }
            float64 length1 = gfx1->getLength(0);
            float64 length2 = gfx2->getLength(0);
            if ( length1 != length2 ) {
               nbrLengthDiffers++;
            }
            float64 lengthDiff = abs(length1 - length2);
            if ( lengthDiff > (length1*0.01) ) {
               nbrLengthDiffers1percent++;
               cout << "xxxxx length differ>1% for item " << itemID
                    << " length1=" << length1 << " length2=" << length2
                    << " diff=" << lengthDiff 
                    << " = " << ( lengthDiff / length1 ) * 100 << " %"
                    << endl;
            }
            if ( lengthDiff > 1 ) {
               nbrLengthDiffers1meter++;
               cout << "XXXXX length differ > 1 m for item " << itemID
                    << " length1=" << length1 << " length2=" << length2
                    << " diff=" << lengthDiff 
                    << " = " << ( lengthDiff / length1 ) * 100 << " %"
                    << endl;
            }

         }
      }
      cout << "Compared " << nbrItemsCompared << " items, "
           << nbrGfxDataDiffers << " differ, " 
           << nbrLengthDiffers << " differ in length, "
           << nbrLengthDiffers1percent << " differ > 1 % in length, "
           << nbrLengthDiffers1meter << " differ > 1 meter in length"
           << endl;

      
   } // nbrMaps == 2

   else {
      cout << "mapLabManyMaps: no action defined" << endl;
   }
} // mapLabManyMaps


void
countNameLanguages(OldGenericMap* theMap,
                   uint32 nbrNamesPerLang[LangTypes::nbrLanguages + 1],
                   uint32& nbrItemsWithName)
{
   uint32 myNbrLangs[LangTypes::nbrLanguages + 1];
   uint32 myNbrLangsON[LangTypes::nbrLanguages + 1];
   uint32 myNbrLangsRoadNbr[LangTypes::nbrLanguages + 1];
   for (uint32 i = 0; i <=LangTypes::nbrLanguages; i++) {
      myNbrLangs[i] = 0;
      myNbrLangsON[i] = 0;
      myNbrLangsRoadNbr[i] = 0;
   }
   
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i< theMap->getNbrItemsWithZoom(z); i++) {
         OldItem* item = theMap->getItem(z, i);
         if ( (item != NULL) &&
              ( (item->getItemType() == ItemTypes::streetSegmentItem) /*||
                (item->getItemType() == ItemTypes::builtUpAreaItem) ||
                (item->getItemType() == ItemTypes::municipalItem)*/ ) ) {
            if (item->getNbrNames() > 0) {
               nbrItemsWithName++;
            }
            bool hasEngName = false;
            for (uint32 n = 0; n < item->getNbrNames(); n++) {
               
               LangTypes::language_t strLang;
               ItemTypes::name_t strType;
               uint32 strIdx;

               item->getNameAndType( n, strLang, strType, strIdx );

               nbrNamesPerLang[strLang]++;
               myNbrLangs[strLang]++;

               if (strType == ItemTypes::officialName) {
                  myNbrLangsON[strLang]++;
               }
               if (strType == ItemTypes::roadNumber) {
                  myNbrLangsRoadNbr[strLang]++;
               }
               if ( strLang == LangTypes::english ) {
                  hasEngName = true;
                  mc2dbg8 << "llll eng " << item->getID() << " it="
                          << int(item->getItemType()) << endl;
               }
            }
            if (item->getItemType() == ItemTypes::pointOfInterestItem) {
               int pt = 111111;
               OldPointOfInterestItem* poi =
                  dynamic_cast<OldPointOfInterestItem*>(item);
               if ( poi != NULL ) {
                  pt = int(poi->getPointOfInterestType());
               }
               mc2dbg8 << "llll poi hasEngName=" << int(hasEngName)
                    << " nbrNames=" << int(item->getNbrNames())
                    << " pt=" << pt
                    << " " << item->getID() <<  endl;
            }
         }
      }
   }

   StringTable::countryCode cc = theMap->getCountryCode();
   const char* cname = StringTable::getString(
               StringTable::getCountryStringCode(cc), StringTable::ENGLISH);
   cout << "NAME " << cname << ":" << theMap->getMapID()
        << " total nbr item names = " << theMap->getTotalNbrItemNames()
        << endl;
   for (uint32 i = 0; i <=LangTypes::nbrLanguages; i++) {
      if (myNbrLangs[i] > 0) {
         cout << "NAME " << cname << ":" << theMap->getMapID() 
              << " lang " << i << " "
              << LangTypes::getLanguageAsString( LangTypes::language_t(i))
              << " \t" << myNbrLangs[i];
         if (myNbrLangsON[i] > 0)
            cout << " (on " << myNbrLangsON[i] << ")";
         if (myNbrLangsRoadNbr[i] > 0)
            cout << " (rnbr " << myNbrLangsRoadNbr[i] << ")";
         cout << endl;
      }
   }
   uint32 nbrNative = theMap->getNbrNativeLanguages();
   for (uint32 i = 0; i < nbrNative; i++) {
      cout << "NAME " << cname << ":" << theMap->getMapID() 
           << " native " << int(theMap->getNativeLanguage(i)) << " "
           << LangTypes::getLanguageAsString(theMap->getNativeLanguage(i)) 
           << endl;
   }
}


void
printNbrItemsOfZooms( OldGenericMap* theMap ){



   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {

      uint32 initialValue = 0;
      uint32 nbrItemTypes = ItemTypes::numberOfItemTypes;
      vector<uint32> nbrItemsPerItemType(nbrItemTypes, initialValue);
      uint32 nbrNullItems = 0;
      
      for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
         OldItem* item = theMap->getItem(z, i);
         if (item != NULL) {
            (nbrItemsPerItemType[item->getItemType()])++;
            //cout << nbrItemsPerItemType[item->getItemType()] << endl;
         }
         else {
            nbrNullItems++;
         }
      }
      
      // Print data of this zoom level.
      cout << "Zoom: " << z << endl;
      for (uint32 i=0; i<nbrItemsPerItemType.size(); i++){
         if ( nbrItemsPerItemType[i] != 0 ){
            ItemTypes::itemType itemType= 
               static_cast<ItemTypes::itemType>(i); 
            cout << "   " << setw(2) << i << ":" 
                 << setw(20) << ItemTypes::getItemTypeAsString(itemType) 
                 << setw(10) << nbrItemsPerItemType[i] << endl;
         }
      }
      cout << "   " << "  " << ":" 
           << setw(20) << "null item"
           << setw(10) << nbrNullItems << endl;
      cout << "   " << "  " << ":" 
           << setw(20) << "total nbr items"
           << setw(10) <<  theMap->getNbrItemsWithZoom(z) << endl;
       
   }
} // printNbrItemsOfZooms



// Struct holding extConnInfo used when looping all maps in this dir
struct allExtConnInfo_t {
   uint32 toMap;     // Id of the map we are looking in
   uint32 riID;      // Id of the routeable item that is the virtual
   uint32 fromMap;   // Id of the map where the connection comes FROM
   uint32 fromNode;  // Id of the node in the other map
   int32 lat;
   int32 lon;
   uint32 roadClass;
   ItemTypes::itemType type;
   
   static bool less(const allExtConnInfo_t& a, const allExtConnInfo_t& b) {
      if ( a.toMap < b.toMap )
         return true;
      if ( a.fromMap < b.fromMap )
         return true;
      if ( a.riID < b.riID)
         return true;
      if ( a.fromNode < b.fromNode)
         return true;
      else
         return false;
   }

   bool operator<(const allExtConnInfo_t& other) const {
      return less(*this, other);
   }
};

struct borderExtConns_t {
   StringTable::countryCode country1;
   StringTable::countryCode country2;
   int32 lat;
   int32 lon;
   
   static bool less(const borderExtConns_t& a, const borderExtConns_t& b) {
      if ( int(a.country1) < int(b.country1) )
         return true;
      if ( int(a.country2) < int(b.country2) )
         return true;
      if ( a.lat < b.lat)
         return true;
      if ( a.lon < b.lon)
         return true;
      else
         return false;
   }

   bool operator<(const borderExtConns_t& other) const {
      return less(*this, other);
   }

};

void
collectBorderExtConns( const char* directory )
{

   // There are two options for printing ext conns
   // 1. Print all border ext conns to/from a specific country, regardless of
   //    the supplier of this country and its neighbours
   bool onlyConnsToOneCountry = false;
   StringTable::countryCode connsToCountry = StringTable::FRANCE_CC;
   // 2. Print border ext conns between different countries
   // Set to true if only ext conns between countries of different suppliers
   // should be printed.
   bool diffSuppliers = true;

   // Loop all underview maps in the directory
   // Collect info about external connections between countries (of different
   // map suppliers/releases?)
   // Print info to text file the external connections that are between
   // different countries (and suppliers?)
   
   // translating mapID to countryCode
   map<uint32,StringTable::countryCode> mapId2Country;
   
   // translating mapID to mapOrigin 
   map<uint32, MC2String> mapId2Origin;

   set<allExtConnInfo_t> allExtConnInfo;
   
   // Make sure we have a '/' in the directory path-end
   MC2String dir = MC2String(directory) + "/";
   uint32 curMapID = 0;
   //uint32 curMapID = 0xa1; // start with some other map
   bool cont = true;
   while ( cont ) {
      OldGenericMap* curMap = OldGenericMap::createMap(curMapID, dir.c_str() );
      if ( curMap == NULL ) {
         cont = false;
      } else {
         /*
         // limit nbr of maps 
         if ( curMapID > 370) {
            cont = false;
         }*/
         // Do processing
         mc2dbg << "xxx Processing map " << curMapID
                << " country=" << int(curMap->getCountryCode()) 
                << " origin=" << curMap->getMapOrigin() << endl;

         // Store countryCode+mapOrigin for this mapID
         mapId2Country.insert(
            make_pair( curMapID, curMap->getCountryCode() ));
         mapId2Origin.insert( make_pair(
            curMapID, StringUtility::newStrDup(curMap->getMapOrigin()) ) );
         
         // Find all external connections TO this map, store info
         OldBoundrySegmentsVector* bsVec = curMap->getBoundrySegments();
         MC2_ASSERT(bsVec != NULL);
         for (uint32 i=0; i<bsVec->getSize(); ++i) {
            OldBoundrySegment* bs = static_cast<OldBoundrySegment*>
                                             (bsVec->getElementAt(i));
            uint32 riID = bs->getConnectRouteableItemID();
            OldRouteableItem* ri = 
               static_cast<OldRouteableItem*>(curMap->itemLookup(riID));
            
            if ( ri != NULL ) {
               for (uint32 n=0; n<2; ++n) {
                  for (uint32 j=0; j<bs->getNbrConnectionsToNode(n); ++j) {

                     uint32 fromMap = bs->getFromMapIDToNode(n, j);
                     uint32 fromNode = 
                        bs->getConnectToNode(n, j)->getConnectFromNode();
                  
                     allExtConnInfo_t info;
                     info.toMap = curMapID;
                     info.fromMap = fromMap;
                     info.riID = riID;
                     info.fromNode = fromNode;
                     info.lat = ri->getGfxData()->getLat(0,0);
                     info.lon = ri->getGfxData()->getLon(0,0);
                     info.roadClass = ri->getRoadClass();
                     info.type = ri->getItemType();
                     allExtConnInfo.insert(info);
                     
                     mc2dbg8 << curMapID << ";" << fromMap << ";"
                        << info.lat << ";" << info.lon << ";"
                        << info.roadClass << ";" << int(info.type) << endl;
                  }
                  
               }
            }
         }
         mc2dbg << "xxx Collected ext conn info for map " << curMapID
                << ", nbr extConnInfos=" << allExtConnInfo.size() << endl;
         
         
         // get next mapID
         curMapID = MapBits::nextMapID(curMapID);
      }
      // delete the map
      delete curMap;
   }

   // Print info about country+origin for all maps
   map<uint32,StringTable::countryCode>::const_iterator cit;
   for ( cit=mapId2Country.begin(); cit != mapId2Country.end(); cit++ ) {
      cout << " map ID " << cit->first << " country=" << cit->second
           << " origin=" << mapId2Origin.find(cit->first)->second << endl;
   }

   // Loop the allExtConnInfo
   
   set<borderExtConns_t> borderExtConns;
   
   set<allExtConnInfo_t>::const_iterator conInfoIt;
   for ( conInfoIt = allExtConnInfo.begin();
            conInfoIt!= allExtConnInfo.end(); conInfoIt++ ) {
      StringTable::countryCode country1 = StringTable::NBR_COUNTRY_CODES;
      MC2String sup1 = "unknown";
      if ( mapId2Country.find(conInfoIt->fromMap) != mapId2Country.end()) {
         country1 = mapId2Country.find(conInfoIt->fromMap)->second;
         sup1 = mapId2Origin.find(conInfoIt->fromMap)->second;
      }
      StringTable::countryCode country2 = StringTable::NBR_COUNTRY_CODES;
      MC2String sup2 = "unknown";
      if ( mapId2Country.find(conInfoIt->toMap) != mapId2Country.end() ) {
         country2 = mapId2Country.find(conInfoIt->toMap)->second;
         sup2 = mapId2Origin.find(conInfoIt->toMap)->second;
      }
      bool print = false;
      /*cout << " c1 " << country1 << " c2 " << country2 
           << " connsToCountry " << connsToCountry
           << " nbrcc " << StringTable::NBR_COUNTRY_CODES 
           << endl;*/
      
      if ( onlyConnsToOneCountry ) {
         // All conns to/from this country
         if ( (country1 < StringTable::NBR_COUNTRY_CODES) &&
              (country2 < StringTable::NBR_COUNTRY_CODES) &&
              (country1 != country2) &&
              ( (country1==connsToCountry) ||
                (country2==connsToCountry) ) ) {
            // conns to/from the particual country
            print = true;
         }
      } else {
         // All conns between different countries
         // (possibly of different suppliers)
         if ( (country1 < StringTable::NBR_COUNTRY_CODES) &&
              (country2 < StringTable::NBR_COUNTRY_CODES) &&
              (country1 != country2) ) {
            // different countries
            
            // different suppliers?
            if ( diffSuppliers )  {
               if ( (sup1 != "unkown") && (sup2 != "unknown") &&
                    (sup1 != sup2) ) {
                  print = true;
               }
            } else {
               print = true;
            }
         }
      }
      if ( print ) {
         borderExtConns_t bec;
         bec.country1 = 
            StringTable::countryCode( MIN(int(country1), int(country2)) );
         bec.country2 =
            StringTable::countryCode( MAX(int(country1), int(country2)) );
         bec.lat = conInfoIt->lat;
         bec.lon = conInfoIt->lon;
         borderExtConns.insert( bec );
         mc2dbg8 << " tttt " << bec.country1 << " " << bec.country2
                 << " " << bec.lat << " " << bec.lon << endl;
      }
   }
   cout << "Collected " << borderExtConns.size() << " ext conns" 
        << " between countries";
   if ( diffSuppliers) { cout << " and suppliers"; }
   cout << endl;

   // Print info to text file
   if ( borderExtConns.size() > 0 ) {
      char dateStr[11];
      char timeStr[9];
      uint32 time = TimeUtility::getRealTime();
      StringUtility::makeDateStr( time, dateStr, timeStr );
      char fileName[256];
      sprintf( fileName,
               "externalConnectionsInfo_%s_%s.txt", dateStr, timeStr);
      ofstream file ( fileName );
      for ( set<borderExtConns_t>::const_iterator becIT = 
                                       borderExtConns.begin();
            becIT != borderExtConns.end(); becIT++ ) {
         cout << becIT->country1 << " " << becIT->country2 << " "
              << becIT->lat << " " << becIT->lon
              << " \t(" << StringTable::getString(
                    StringTable::getCountryStringCode( becIT->country1),
                    StringTable::ENGLISH)
              << " <-> " << StringTable::getString(
                    StringTable::getCountryStringCode( becIT->country2),
                    StringTable::ENGLISH) << ")"
              << endl;
         file << becIT->country1 << " " << becIT->country2 << " "
              << becIT->lat << " " << becIT->lon
              << " \t(" << StringTable::getString(
                    StringTable::getCountryStringCode( becIT->country1),
                    StringTable::ENGLISH)
              << " <-> " << StringTable::getString(
                    StringTable::getCountryStringCode( becIT->country2),
                    StringTable::ENGLISH) << ")"
              << endl;
      }
   }
}

void
checkBorderExtConns( const char* directory, CommandlineOptionHandler coh )
{
   if ( coh.getTailLength() == 0 )  {
      cerr << "No extConn file in tail" << endl;
      exit (1);
   }
   const char* allExtConnsFileName = coh.getTail(0);
   ifstream extConnInfoFile(allExtConnsFileName);
   if ( !extConnInfoFile ) {
      cerr << "Cannot open extConn file '" << allExtConnsFileName
           << "'" << endl;
      exit(1);
   }

   // 1.
   // OK, read the ext conn file, store in a set of extConnInfos
   uint32 nbrInfos = 0;
   set<borderExtConns_t> borderExtConns;
   uint32 country1 = MAX_UINT32;
   uint32 country2 = MAX_UINT32;
   int32 lat; int32 lon;
   char* tmp = new char[100];
   extConnInfoFile >> country1;
   while ( !extConnInfoFile.eof() ) {
      // continue read the row
      extConnInfoFile >> country2;
      extConnInfoFile >> lat;
      extConnInfoFile >> lon;
      nbrInfos++;

      borderExtConns_t bec;
      bec.country1 = StringTable::countryCode(country1);
      bec.country2 = StringTable::countryCode(country2);
      bec.lat = lat;
      bec.lon = lon;
      borderExtConns.insert(bec);
      
      mc2dbg1 << "ext conn info " << nbrInfos << ": " << country1 << " "
              << country2 << " " << lat << " " << lon << endl;

      // read "      (Austria <-> Hungary)" ?
      //extConnInfoFile >> tmp;
      //extConnInfoFile >> tmp;
      //extConnInfoFile >> tmp;
      
      // next row
      extConnInfoFile >> country1;

      if (nbrInfos > 10000) {
         cout << "Now we have > 10 000 extConns in the infile!" << endl;
         cout << "Check if the infile is formatted according to the "
              << "code that is parsing it!!!" << endl;
         cout << "There must be no '(Austria <-> Hungary)' info" << endl;
         exit(1);
      }
   }
   delete tmp;
   cout << "Read " << nbrInfos << "=" << borderExtConns.size() 
        << " from extConnFile" << endl;

   // 2.
   // Loop the 00 maps in the given directory and collect coordinates of
   // ALL virtual items (countryID, lat, lon)
   // Make sure we have a '/' in the directory path-end
   MC2String dir = MC2String(directory) + "/";
   cout << "Looping maps in " << dir << endl;
   multimap<uint32, MC2Coordinate> virtualsInCountry; //countryCode mc2coord
   uint32 curMapID = 0;
   uint32 nbrVirtuals = 0;
   bool cont = true;
   while ( cont ) {
      OldGenericMap* curMap = OldGenericMap::createMap(curMapID, dir.c_str() );
      if ( curMap == NULL ) {
         cont = false;
      } else {
         
         OldBoundrySegmentsVector* bsVec = curMap->getBoundrySegments();
         MC2_ASSERT(bsVec != NULL);
         uint32 cc = int(curMap->getCountryCode());
         for (uint32 i=0; i<bsVec->getSize(); ++i) {
            OldBoundrySegment* bs = static_cast<OldBoundrySegment*>
                                             (bsVec->getElementAt(i));
            uint32 riID = bs->getConnectRouteableItemID();
            OldRouteableItem* ri = 
               static_cast<OldRouteableItem*>(curMap->itemLookup(riID));
            if ( ri != NULL ) {
               MC2Coordinate mc2Coord = 
                  MC2Coordinate(ri->getGfxData()->getLat(0,0),
                                ri->getGfxData()->getLon(0,0) );
               virtualsInCountry.insert( make_pair(cc, mc2Coord) );
               nbrVirtuals++;
            }
         }
         
         // get next mapID
         curMapID = MapBits::nextMapID(curMapID);
      }
      // delete the map
      delete curMap;
   }
   cout << "Collected " << virtualsInCountry.size() << " ("
        << nbrVirtuals << ") virtuals from the maps" << endl;

   // 3.
   // Compare the virtual coordinates with the content of the extConnInfos
   // (the virtualCountry can match country1 or country2 of the extConnInfos)
   // If one of the extConnInfos does not have a virtualCoordinate, this
   // must be printed - and taken care of in stitching, else everything
   // can be re-used
   // a. Loop the extConnInfos.
   //    For each info check if there are any virtuals in any of the
   //    countries (country1 country2).
   multimap<uint32, MC2Coordinate>::const_iterator vicIT;
   uint32 nbrBecs = 0;
   map<uint32, borderExtConns_t> notFoundExtConns;
   for ( set<borderExtConns_t>::const_iterator 
               becIT = borderExtConns.begin();
         becIT != borderExtConns.end(); becIT++ ) {
      bool becCountryInVirtuals = false;
      bool foundInBoth = true;
      // find country1 in virtualsInCountry
      uint32 nbrC1 = virtualsInCountry.count( int(becIT->country1) );
      if ( nbrC1 > 0 ) {
         becCountryInVirtuals = true;
         bool found = false;
         for (vicIT = virtualsInCountry.lower_bound(int(becIT->country1));
              vicIT != virtualsInCountry.upper_bound(int(becIT->country1));
              vicIT++ ) {
            if ( (vicIT->second.lat == becIT->lat) &&
                 (vicIT->second.lon == becIT->lon) ) {
               found = true;
               cout << "bec1 " << nbrBecs << " \""
                    << becIT->country1 << " " << becIT->country2 << " "
                    << becIT->lat << " " << becIT->lon << "\""
                    << " found in virtuals" << endl;
            } else if ( GfxUtility::squareP2Pdistance_linear(
                          vicIT->second.lat, vicIT->second.lon,
                          becIT->lat, becIT->lon ) < 1 ) {
               found = true;
               cout << "bec1 " << nbrBecs << " \""
                    << becIT->country1 << " " << becIT->country2 << " "
                    << becIT->lat << " " << becIT->lon << "\""
                    << " found close in virtuals" << endl;
            }
         }
         if ( !found ) {
            cout << "bec1 " << nbrBecs << " \""
                 << becIT->country1 << " " << becIT->country2 << " "
                 << becIT->lat << " " << becIT->lon << "\""
                 << " NOT FOUND in virtuals" << endl;
            //notFoundExtConns.insert( make_pair(nbrBecs, (*becIT)) );
            foundInBoth = false;
         }
      }
      // find country2 in virtualsInCountry
      uint32 nbrC2 = virtualsInCountry.count( int(becIT->country2) );
      if ( nbrC2 > 0 ) {
         becCountryInVirtuals = true;
         bool found = false;
         for (vicIT = virtualsInCountry.lower_bound(int(becIT->country2));
              vicIT != virtualsInCountry.upper_bound(int(becIT->country2));
              vicIT++ ) {
            if ( (vicIT->second.lat == becIT->lat) &&
                 (vicIT->second.lon == becIT->lon) ) {
               found = true;
               cout << "bec2 " << nbrBecs << " \""
                    << becIT->country1 << " " << becIT->country2 << " "
                    << becIT->lat << " " << becIT->lon << "\""
                    << " found in virtuals" << endl;
            } else if ( GfxUtility::squareP2Pdistance_linear(
                          vicIT->second.lat, vicIT->second.lon,
                          becIT->lat, becIT->lon ) < 1 ) {
               found = true;
               cout << "bec2 " << nbrBecs << " \""
                    << becIT->country1 << " " << becIT->country2 << " "
                    << becIT->lat << " " << becIT->lon << "\""
                    << " found close in virtuals" << endl;
            }
         }
         if ( !found ) {
            cout << "bec2 " << nbrBecs << " \""
                 << becIT->country1 << " " << becIT->country2 << " "
                 << becIT->lat << " " << becIT->lon << "\""
                 << " NOT FOUND in virtuals" << endl;
            //notFoundExtConns.insert( make_pair(nbrBecs, (*becIT)) );
            foundInBoth = false;
         }
      }
      if ( becCountryInVirtuals && !foundInBoth ) {
         notFoundExtConns.insert( make_pair(nbrBecs, (*becIT)) );
      }
      nbrBecs++;
   }

   // Print summary of border ext conns that were not found in the new maps
   if ( notFoundExtConns.size() > 0 ) {
      cout << "These external connections are missing:" << endl;
      for ( map<uint32, borderExtConns_t>::const_iterator
               it = notFoundExtConns.begin();
            it != notFoundExtConns.end(); it++ ) {
         cout << "   bec " << it->first << ": "
              << it->second.country1 << " " << it->second.country2
              << " " << it->second.lat << " " << it->second.lon 
              << " \t(" << StringTable::getString(
                    StringTable::getCountryStringCode( it->second.country1), 
                    StringTable::ENGLISH)
              << " <-> " << StringTable::getString(
                    StringTable::getCountryStringCode( it->second.country2), 
                    StringTable::ENGLISH) << ")"
              << endl;
              // also print the country names..
      }
   } else {
      cout << "All external connections are ok!" << endl;
   }
}


int main(int argc, char* argv[]) 
{
   CommandlineOptionHandler coh(argc, argv, 0);
   coh.setTailHelp("mcm-file(s)");
   coh.setSummary("The path to the map:s to load.\n"
                  "Notice that this is not only the map ID:s, "
                  "but the full filename of the maps.");

   // -------------------------------------------------------- creationTime
   bool o_mapInfo = false;
   coh.addOption("-t", "--creationtime",
                 CommandlineOptionHandler::presentVal,
                 0, &o_mapInfo, "F",
                 "Print the name, creation-time etc. for each map.");

   // --------------------------------------------------------- memoryUsage
   bool o_memoryUsage = false;
   coh.addOption("-m", "--memory",
                 CommandlineOptionHandler::presentVal,
                 0, &o_memoryUsage, "F",
                 "Print the memory usage for each map.");

   // ------------------------------------------------------------- country
   bool o_country = false;
   coh.addOption("-c", "--country",
                 CommandlineOptionHandler::presentVal,
                 0, &o_country, "F",
                 "Print the country and some other country specific"
                 "attributes for each map.");
   // ---------------------------------------------------------- countItems
   bool o_countItems = false;
   coh.addOption("-i", "--items",
                 CommandlineOptionHandler::presentVal,
                 0, &o_countItems, "F",
                 "Displays information about the number of items on "
                 "the maps.");

   // ------------------------------------------------------- print items
   char* o_printItemsOfType = NULL;
   coh.addOption("-w", "--printItemsOfType",
                 CommandlineOptionHandler::stringVal,
                 1, &o_printItemsOfType, "\0",
                 "Print the name of all items in each map. Specify item "
                 "type with this option.");

   /// ---------------------------------------------------------- saveturns
   char* o_saveTurndescriptions;
   coh.addOption("-s", "--saveturns",
                 CommandlineOptionHandler::stringVal,
                 1, &o_saveTurndescriptions, "\0",
                 "Save all turndescriptions in the maps to file. "
                 "The generated file could be used together with the -C "
                 "option.");
   
   /// ---------------------------------------------------------- checkturns
   char* o_checkTurndescriptions;
   coh.addOption("-C", "--checkturns",
                 CommandlineOptionHandler::stringVal,
                 1, &o_checkTurndescriptions, "\0",
                 "Check all turndescriptions in the file against the maps. "
                 "The format of the file is the same as saved with the -s "
                 "option.");

   /// -------------------------------------------------------- compareturns
   bool o_compTurndescriptions;
   coh.addOption("-D", "--compturns",
                 CommandlineOptionHandler::presentVal,
                 0, &o_compTurndescriptions, "F",
                 "Compare all turndescriptions in the _two_ mcm-maps. "
                 "Please note that you must give two maps as argument, "
                 "and that the only thing that should differ between the " 
                 "maps are the turndescriptions (the ItemIDs etc must be the "
                 "same in both maps). Typical use is when one original map has "
                 "been processed with a new generateTurnDescription-method "
                 "and you want to compare which turn descriptions were changed "
                 "with the new method.\n"
                 "The first map is the \"old\" and the second is the \"new\". "
                 "The result of this comparison can be analysed in "
                 "MapEditor with the ShowNodes functionality.");

   /// ------------------------------------------------------------- savemap
   bool o_saveMap;
   coh.addOption("-S", "--savemap",
                 CommandlineOptionHandler::presentVal,
                 0, &o_saveMap, "F",
                 "Save the loaded map to disc again. Will change creation "
                 "time of the map. Could also be usefull to test small "
                 "changes in the map format.");

   // --------------------------------- lookup coordinates for suspect turns
   char* o_suspectTurnsFile = NULL;
   coh.addOption("", "--suspectTurnsFile",
                 CommandlineOptionHandler::stringVal,
                 1, &o_suspectTurnsFile, "\0",
                 "Reads suspect turns and looks up coordinates for them");
   

   // --------------------------------- print external connections
   bool o_printExternalConnections = false;
   coh.addOption("-e", "--printExternalConnections",
                 CommandlineOptionHandler::presentVal,
                 0, &o_printExternalConnections, "F",
                 "Print external connections.");

   // --------------------------print map header
   bool o_printHeader = false;
   coh.addOption("", "--printHeader",
                 CommandlineOptionHandler::presentVal,
                 0, &o_printHeader, "F",
                 "Prints the information stored in the header of the map");

   // --------------------------print number of items for zoom levels.
   bool o_printNbrItemsOfZooms = false;
   coh.addOption("", "--printNbrItemsOfZooms",
                 CommandlineOptionHandler::presentVal,
                 0, &o_printNbrItemsOfZooms, "F",
                 "Print nbr items per zoom level sorted by item type.");

   // --------------------------------- filter maps
   bool o_filterMap = false;
   coh.addOption("", "--filterMap",
                 CommandlineOptionHandler::presentVal,
                 0, &o_filterMap, "F",
                 "Filter map, some items in it, "
                 "with OldMapFilter.filter method.");


   // ---------------------- check mapssicoord files..
   char* o_checkmapssicoordFile = NULL;
   coh.addOption("", "--checkmapssicoordFile",
                 CommandlineOptionHandler::stringVal,
                 1, &o_checkmapssicoordFile, "\0",
                 "Check if the coordinates in the map ssi coord file "
                 "are valid. They have to be within 2 meters from any ssi "
                 "in the map concerned to be valid. The file contains "
                 "one coordinate row with \"mapid,latitude,longitude\" "
                 "for each map like this:\n"
                 "0,707567586,131709825");
   
   // ------------------------- set some times in map header
   char* o_setMapTimeType = NULL;
   coh.addOption("", "--setMapTimeType",
                 CommandlineOptionHandler::stringVal,
                 1, &o_setMapTimeType, "\0",
                 "Set one time in the map header. Give the time type as "
                 "option, choose between:\n"
                 "waspTime\n"
                 "trueCreationTime\n"
                 "Combine with --setMapTimeValue to give time value.");

   char* o_setMapTimeValue = NULL;
   coh.addOption("", "--setMapTimeValue",
                 CommandlineOptionHandler::stringVal,
                 1, &o_setMapTimeValue, "\0",
                 "Set one time in the map header. Give the time value as "
                 "option, \"yyyy-mm-dd hh:mm:ss\" (don't forget \"-char).\n"
                 "Combine with --setMapTimeType to give time type.");

   // ----------- check that the map has valid trueCreationTime and waspTime
   bool o_checkValidTimes = false;
   coh.addOption("", "--checkValidTimes",
                 CommandlineOptionHandler::presentVal,
                 0, &o_checkValidTimes, "F",
                 "Check that the map has valid trueCreationTime and "
                 "waspTime. If any of the times are invalid (MAX_UINT32) "
                 "the function will print to cout and exit.\n"
                 "\tMap <mapid> has invalid trueCreationTime\n"
                 "\tMap <mapid> has invalid waspTime\n");

   // ------------------------------------------------ different tests 2
   char* o_mapLab2 = NULL;
   coh.addOption("", "--mapLab2",
                 CommandlineOptionHandler::stringVal,
                 1, &o_mapLab2, "\0",
                 "Some tests on a map. What this method does depends on "
                 "compilation.\n"
                 " - setNewMapGfx (new gfx on co maps from co pol mif file\n"
                 " - printCoPolFiltLevel (one filt level to 'myCountry.mif')\n"
                 " - dumpBoundrySegments \n"
                 " - printVirtualRIcoords\n"
                 " - dumpDisplayClassInfo\n"
                 " - misc (edit to choose action)");
  
   // ------------------------------------------- different tests no map
   bool o_noMapLab = false;
   coh.addOption("", "--noMapLab",
                 CommandlineOptionHandler::presentVal,
                 0, &o_noMapLab, "F",
                 "Some tests without a map. What this method does depends on "
                 "compilation.");
   
   // ----------------------------------------- different tests many maps
   uint32 o_mapLabManyMaps = 0;
   coh.addOption("", "--mapLabManyMaps",
                 CommandlineOptionHandler::uint32Val,
                 1, &o_mapLabManyMaps, "0",
                 "Some tests on many maps, the number of maps give with "
                 "this option. "
                 "Decide action with option --mapLabManyMapsAction.");
   char* o_mapLabManyMapsAction = NULL;
   coh.addOption("", "--mapLabManyMapsAction",
                 CommandlineOptionHandler::stringVal,
                 1, &o_mapLabManyMapsAction, "\0",
                 "Which test to perform for the mapLabManyMaps\n"
                 " - compareAreaFeatureDrawDisplayClass (if 2 maps)\n"
                 " - misc (edit to choose action)");

   // --------------------------------------------------- count languages
   bool o_countLanguages = false;
   coh.addOption("", "--countLang",
                 CommandlineOptionHandler::presentVal,
                 0, &o_countLanguages, "F",
                 "Count number of names per language in all maps in tail.");

   // ------------------------- set copyright
   char* o_setCopyRight = NULL;
   coh.addOption("", "--setCopyRight",
                 CommandlineOptionHandler::stringVal,
                 1, &o_setCopyRight, "\0",
                 "Set copy right in map.");

   // ---------- collect external connections between countries/map suppliers
   char* o_collectBorderExtConns = false;
   coh.addOption("", "--collectBorderExtConns",
                 CommandlineOptionHandler::stringVal,
                 1, &o_collectBorderExtConns, "\0",
                 "Collect external connections between countries (of "
                 "different map suppliers/releases) in a merge directory "
                 "(specify directory). Useful to detect stitch needs when "
                 "migrating stitching to new map release.\n"
                 "Writes border ext conn info to text file. You need to sort "
                 "this file and make content uniq!");
   
   // ----------------------- check old border ext conn info on new maps
   char* o_checkBorderExtConns = false;
   coh.addOption("", "--checkBorderExtConns",
                 CommandlineOptionHandler::stringVal,
                 1, &o_checkBorderExtConns, "\0",
                 "Check uv-maps of a new release if they still have virtual "
                 "items on the same coordinates where there were external "
                 "connections in the previous map release. Specify directory "
                 "with this option. Use the text file with border ext conn "
                 "info (from --collectBorderExtConns sorted&uniq) in tail.\n"
                 "Prints result to std out.");
   


   // Parse command-line
   if(!coh.parse()) {
      cerr << argv[0] << ": Error on commandline! (-h for help)" << endl;
      exit(1);
   }

   // First the tests, that do not require any files in tail
   if (o_noMapLab) {
      noMapLab( );
      cout << " done" << endl;
      exit(0);
      return 0;
   }

   if ( o_collectBorderExtConns != NULL ) {
      collectBorderExtConns( o_collectBorderExtConns );
      cout << "Done!" << endl;
      return 0;
   }
         
   // Then make sure that there is at least one file in tail
   if ( coh.getTailLength() == 0 ) {
      mc2log << error << argv[0] << ": Tail too small!" << endl;
      exit(1);
   }
   
   // Handle options that have a file in tail (no mcm map in tail)
   if ( o_checkBorderExtConns != NULL ) {
      checkBorderExtConns( o_checkBorderExtConns, coh );
      cout << "Done!" << endl;
      exit(0);
      return 0;
   }
   
   //Declare vector for counting number name per lang fo all maps in tail.
   uint32 nbrNamesPerLang[LangTypes::nbrLanguages + 1];
   uint32 nbrItemsWithName;
   if ( o_countLanguages ) {
      nbrItemsWithName = 0;
      for (uint32 i = 0; i <=LangTypes::nbrLanguages; i++) {
         nbrNamesPerLang[i] = 0;
      }
   }

   // For count nbr items in the maps
   uint32 totNbrItemsAllMaps = 0;





   // Loop over all the maps
   int nbrMaps = coh.getTailLength();
   for (int i=0; i<nbrMaps; i++) {
      mc2dbg << "Looping map " << i << " of " << nbrMaps << endl;

      // mallinfo
      struct mallinfo minfo = mallinfo();
      cerr << "Before: arena=" << minfo.arena << "="
           << minfo.arena/1024.0/1024.0 << "Mb, uordblks="
           << minfo.uordblks << "=" << minfo.uordblks/1024.0/1024.0 
           << "Mb" << endl;

      /*char t;
      cin >> t;*/

      
      const char* mcmName = coh.getTail(i);
      OldGenericMap* theMap = OldGenericMap::createMap(mcmName);

      if (theMap != NULL) {
         // Print mcm-file
         cout << "--------------------------------------------------------"
              << endl << "   file: " << mcmName << endl;

         // Handle creationTime
         if (o_mapInfo) {
            cout << "   name = " << theMap->getMapName() << endl;
            cout << "   map origin = " << theMap->getMapOrigin() << endl;
            time_t creationTime = time_t(theMap->getCreationTime());
            cout << "   creationTime = last saved [" << theMap->getMapID() 
                 << " " << theMap->getCreationTime() << "] = " 
                 << asctime(localtime(&creationTime));
            time_t trueCreationTime = time_t(theMap->getTrueCreationTime());
            cout << "   true creationTime [" << theMap->getMapID() 
                 << " " << theMap->getTrueCreationTime() << "] = " 
                 << asctime(localtime(&trueCreationTime));
            time_t dynEDTime = time_t(theMap->getDynamicExtradataTime());
            cout << "   dyn ED time " << theMap->getDynamicExtradataTime()
                 << " = " << asctime(localtime(&dynEDTime));
            time_t waspTime = time_t(theMap->getWaspTime());
            cout << "   wasp time " << theMap->getWaspTime()
                 << " = " << asctime(localtime(&waspTime));
            
            cout << "   country = " << int(theMap->getCountryCode()) << " = " 
                 << StringTable::getString(
                       StringTable::getCountryStringCode(
                          theMap->getCountryCode()), 
                       StringTable::ENGLISH) << endl;
            cout << "   driving side = ";
            if (theMap->driveOnRightSide()){
               cout << "right" << endl;
            }
            else {
               cout << "left" << endl;
            }
            cout << " filename  " << theMap->getFilename() << endl;
            MC2BoundingBox bbox;
            theMap->getMapBoundingBox(bbox);
            cout << " mapbbox  " << bbox << endl;
            cout << " mapIsFiltered in coord levels = " 
                 << int(theMap->mapIsFiltered()) << endl;
            cout << " mapGfxDataIsFiltered in coord levels = "
                 << int(theMap->mapGfxDataIsFiltered()) << endl;
            if (dynamic_cast<OldCountryOverviewMap*>(theMap) != NULL) {
               // Got a country overview map, list info about all the maps 
               // in that
               OldCountryOverviewMap* com = 
                  static_cast<OldCountryOverviewMap*>(theMap);
               for (uint32 i=0; i<com->getNbrMaps(); ++i) {
                  uint32 creationTime = 0;
                  int32 maxLat, minLon, minLat, maxLon;
                  uint32 mapID = com->getMapData(i, creationTime, 
                                                 maxLat, minLon, 
                                                 minLat, maxLon);
                  time_t ct = time_t(creationTime);
                  cout << "      CountryMap: creationTime [" << mapID << " "
                       << creationTime << "] = " << asctime(localtime(&ct));
               }
               // count border items
               uint32 nbrBorders = 0;
               for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
                  for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
                     OldItem* item = theMap->getItem(z, i);
                     if ( (item != NULL) && 
                          (item->getItemType() == ItemTypes::borderItem)) {
                        nbrBorders++;
                     }
                  }
               }
               cout << " nbr border items = " << nbrBorders << endl;
            }
            // print admin area centres
            if ( o_mapLab2 != NULL ) {
               theMap->dumpAdminAreaCentres();
               o_mapLab2 = NULL;
            }
            
         }

         // Handle memoryUsage
         if (o_memoryUsage) {
            uint32 mem = theMap->getMemoryUsage();
            cout << "   memoryUsage = " << mem << " bytes = " 
                 << mem/1024.0 << " kb = " << mem/1024.0/1024.0 << " Mb" 
                 << endl;

            // Try mallinfo
            struct mallinfo mi = mallinfo();
            cout << "   mallinfo:" << endl;
            cout << "      total space allocated from system (arena): " 
                 << mi.arena << "b = " << mi.arena/1024.0/1024.0 
                 << "Mb" << endl;
            cout << "      number of non-inuse chunks (ordblks): " 
                 << mi.ordblks << endl;
            /*cout << "      unused -- always zero (smblks): " 
                 << mi.smblks << endl; */
            cout << "      number of mmapped regions (hblks): " 
                 << mi.hblks << endl;
            cout << "      total space in mmapped regions (hblkhd): "
                 << mi.hblkhd << endl;
            /* cout << "      unused -- always zero (usmblks): " 
                 << mi.usmblks << endl; */
            /* cout << "      unused -- always zero (fsmblks): " 
                 << mi.fsmblks << endl; */
            cout << "      total allocated space (uordblks): " 
                 << mi.uordblks << endl;
            cout << "      total non-inuse space (fordblks): " 
                 << mi.fordblks << endl;
            cout << "      top-most, releasable (via malloc_trim) space "
                 << "(keepcost): " << mi.keepcost << endl;
            
            // rusage
            struct rusage ru;
            getrusage(RUSAGE_SELF, &ru);
            cout << "   rusage:" << endl;
            cout << "      maximum resident set size (ru_maxrss):" 
                 << ru.ru_maxrss << endl;
            cout << "      integral shared memory size (ru_ixrss): " 
                 << ru.ru_ixrss << endl;
            cout << "      integral unshared data size (ru_idrss): " 
                 << ru.ru_idrss  << endl;
            cout << "      integral unshared stack size (ru_isrss): " 
                 << ru.ru_isrss << endl;
            cout << "      page reclaims (ru_minflt): " 
                 << ru.ru_minflt << endl;
            cout << "      page faults (ru_majflt): " 
                 << ru.ru_majflt << endl;
            cout << "      swaps (ru_nswap): " << ru.ru_nswap << endl;
            cout << "      block input operations (ru_inblock): " 
                 << ru.ru_inblock << endl;
            cout << "      block output operations (ru_oublock): " 
                 << ru.ru_oublock << endl;
            cout << "      messages sent (ru_msgsnd): " 
                 << ru.ru_msgsnd << endl;
            cout << "      messages received (ru_msgrcv): " 
                 << ru.ru_msgrcv << endl;
            cout << "      signals received (ru_nsignals): " 
                 << ru.ru_nsignals << endl;
            cout << "      voluntary context switches (ru_nvcsw): " 
                 << ru.ru_nvcsw << endl;
            cout << "      involuntary context switches (ru_nivcsw): " 
                 << ru.ru_nivcsw << endl;

         }

         // Handle country
         if (o_country) {
            cout << "   country = " 
                 << StringTable::getString(
                       StringTable::getCountryStringCode(
                          theMap->getCountryCode()), 
                       StringTable::ENGLISH) << endl;
            if (theMap->driveOnRightSide())
               cout << "   Driving on the right side of the road" << endl;
            else
               cout << "   Driving on the left side of the road" << endl;

            cout << "   " << theMap->getNbrNativeLanguages() 
                 << " native languages" << endl;
            for (uint32 i = 0; i < theMap->getNbrNativeLanguages(); i++) {
               cout << "    mapID " << theMap->getMapID() 
                    << " native " << int(theMap->getNativeLanguage(i)) << " "
                    << LangTypes::getLanguageAsString(
                        theMap->getNativeLanguage(i)) 
                    << endl;
            }
            cout << "   " << theMap->getNbrCurrencies() 
                 << " currencies" << endl;
         }
         
         // Print the name of all items with spec type in the map
         if (o_printItemsOfType != NULL) {
            ItemTypes::itemType type = 
               ItemTypes::getItemTypeFromString(o_printItemsOfType);
            if ( (int) type > ( (int) ItemTypes::numberOfItemTypes -1) ) {
               // no valid itemtype
               mc2log << fatal << "OldItemType could not be extracted "
                      << "from \"" << o_printItemsOfType << "\"" << endl;
               exit(1);
            } else {
               printItemsOfType(theMap, type);
            }
         }


         // Count the items on the map
         if ( o_countItems ) {
            countItems(theMap, totNbrItemsAllMaps);
            //calculateCostStatistics(theMap);
         }

         if (o_saveTurndescriptions != NULL) {
            mc2dbg1 << "To save turndescriptions to " 
                    << o_saveTurndescriptions << endl;
            saveTurndescriptions(theMap, o_saveTurndescriptions);
            //saveTrafficRules(theMap, o_saveTurndescriptions);
         }

         if (o_checkTurndescriptions != NULL) {
            mc2dbg1 << "To check turndescriptions from " 
                    << o_checkTurndescriptions << endl;
            compareTurndescriptions(theMap, o_checkTurndescriptions);
         }

         if (o_compTurndescriptions) {
            if (nbrMaps == 2) {
               mc2dbg1 << "To compare turndescriptions in map " 
                       << coh.getTail(0) << " and " << coh.getTail(1) 
                       << endl;
               const char* mcmName2 = coh.getTail(1);
               OldGenericMap* map2 = OldGenericMap::createMap(mcmName2);
               compareTurndescriptions(theMap, map2);
               mc2log << "Done - exiting..." << endl;
               exit(1);
            } else {
               mc2log << fatal << "Must give two maps as argument to "
                      << "compare turndescriptions" << endl;
            }
         }

         // do something on many maps in tail.
         if ( o_mapLabManyMaps > 0 ) {
            mc2dbg << "Do mapLab on mapy maps, nbr maps="
                   << o_mapLabManyMaps << endl;
            if ( nbrMaps < int(o_mapLabManyMaps) ) {
               mc2log << error << "nbrMaps (" << nbrMaps << ") less than "
                      << o_mapLabManyMaps << endl;
            }
            if ( o_mapLabManyMapsAction == NULL ) {
               mc2log << error << "Need to give o_mapLabManyMapsAction"
                      << endl;
               exit(1);
            }
            mapLabManyMaps( theMap, o_mapLabManyMaps, 
                            o_mapLabManyMapsAction, coh );
            mc2dbg << "Done!  exit...." << endl;
            exit(1);
         }

         // Save the map to disc again
         if (o_saveMap) {
            theMap->save();
         }


         if ( o_printExternalConnections ){
            mc2log << "Prints external connections" << endl;
            printExternalConnections(theMap);
         }
         
         if (o_printHeader){
            printHeader(theMap);
         }

         if (o_printNbrItemsOfZooms){
            mc2log << info << "Prints number of items per zoom level and"
                   << "item the in map with ID: 0x" << hex 
                   << theMap->getMapID() << "." << dec << endl;
            printNbrItemsOfZooms(theMap);
         }

         if ( o_filterMap ) {
            filterMap( theMap );
            theMap->save();
         }
         

         if ( o_suspectTurnsFile != NULL ) {
            mc2dbg << "[MapTool]: Will read suspect turns from file \""
                   << o_suspectTurnsFile << '"' << endl;
            vector<pair<MC2String, vector<IDPair_t> > > suspectVect;
            loadSuspectTurns(suspectVect, o_suspectTurnsFile);
            mc2dbg << "[MapTool]: Size of suspectVect = "
                   << suspectVect.size() << endl;
            printSuspectItemCoordinates( theMap, suspectVect );
         }

         // check map ssi coord file validity
         if ( o_checkmapssicoordFile != NULL ) {
            bool valid = checkMapssicoordFile(theMap, o_checkmapssicoordFile);
            mc2log << info << "The coordinate is ";
            if (valid)
               mc2log << "valid";
            else
               mc2log << "invalid (or missing)";
            mc2log << " for map " << theMap->getMapID() << endl;
         }

         // Set some times in the map header
         if ( (o_setMapTimeType != NULL) && (o_setMapTimeValue != NULL) ) {
            mc2dbg1 << "Try to set " << o_setMapTimeType << " to \""
                    << o_setMapTimeValue << "\"" << endl;
            mc2dbg1 << "Before map waspTime=" << theMap->getWaspTime()
                    << " trueCreationTime=" << theMap->getTrueCreationTime()
                    << endl; 
            if (setTimesInMap(theMap, o_setMapTimeType, o_setMapTimeValue)) {
               time_t trueTime = 
                  static_cast<time_t>(theMap->getTrueCreationTime());
               time_t waspTime =
                  static_cast<time_t>(theMap->getWaspTime());
               mc2dbg1 << "After: waspTime=" << waspTime << " "
                       << asctime(localtime(&waspTime));
               mc2dbg1 << "After: trueCreationTime=" << trueTime << " "
                       << asctime(localtime(&trueTime));
               theMap->save();
            }
         }

         // Check that the map has valid trueCreationTime and waspTime
         if ( o_checkValidTimes ) {
            if (!checkValidTimes(theMap)) {
               // NB! don't change this string!!!!
               cout << "One or several times are invalid" << endl;
               exit (1);
            }
         }

         if (o_mapLab2 != NULL) {
            mc2log << "Will perform some test on map with ID: 0x" << hex
                   << theMap->getMapID() <<  "." << dec << endl;
            mapLab2(theMap, o_mapLab2, i);
         }

         if ( o_countLanguages ) {
            countNameLanguages(theMap, nbrNamesPerLang, nbrItemsWithName);
         }
         
         if ( o_setCopyRight != NULL) {
            cout << "Will set copy right to \"" << o_setCopyRight << "\""
                 << endl;
            cout << "Old copy right \"" << theMap->getCopyrightString()
                 << "\"" << endl;
            theMap->setCopyrightString( o_setCopyRight );
            cout << "New copy right \"" << theMap->getCopyrightString()
                 << "\"" << endl;
            theMap->save();
         }
         
      } else {
         MC2WARNING("Failed to load map");
      }


      delete theMap;

      minfo = mallinfo();
      cerr << "After: arena=" << minfo.arena << "="
           << minfo.arena/1024.0/1024.0 << "Mb, uordblks="
           << minfo.uordblks << "=" << minfo.uordblks/1024.0/1024.0 
           << "Mb" << endl;

   }

   
   // Print result of counting nbr names per language.
   if ( o_countLanguages ) {
      cout << "Number items with name " << nbrItemsWithName << endl;
      cout << "Number names per language for " << nbrMaps << " maps" << endl;
      for (uint32 i = 0; i <=LangTypes::nbrLanguages; i++) {
         if (nbrNamesPerLang[i] > 0) {
            cout << "  lang " << i << " "
                 << LangTypes::getLanguageAsString( LangTypes::language_t(i))
                 << " \t" << nbrNamesPerLang[i] << endl;
         }
      }
   }

   if ( o_countItems ) {
      cout << "Count items: Number of items in all maps "
           << totNbrItemsAllMaps << endl;
   }

   return 0;
}

