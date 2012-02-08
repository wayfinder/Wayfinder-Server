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

#include "OldOverviewMap.h"

#include "OldStreetSegmentItem.h"
#include "GfxDataFull.h"
#include "GMSGfxData.h"
#include "OldZipCodeItem.h"
#include "OldPointOfInterestItem.h"
#include "OldMunicipalItem.h"

#include "DataBuffer.h"
#include "OldNode.h"

#include "OldBuiltUpAreaItem.h"

#include "OldExternalConnections.h"
#include "StringUtility.h"
#include "StringSearchUtility.h"
#include "DataBufferUtil.h"
#include "TimeUtility.h"
#include "MapBits.h"
#include "NodeBits.h"

#include "Utility.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Note that index 0 of this array is invalid, since an overview map
// must be of maplevel 1 or higher (maplevel 0 means underview map).
const byte
OldOverviewMap::maxZoomLevelForMapLevel[] = { 6, 4, 3, 2 };


OldOverviewMap::OldOverviewMap()
      : OldGenericMap(),         
        m_containingMaps(0, 2)
{
   m_itemNames = new OldItemNames("MISSING");
}

OldOverviewMap::OldOverviewMap(uint32 mapID)
      : OldGenericMap(mapID), 
        m_containingMaps(0, 2)
{
   m_itemNames = new OldItemNames("MISSING");
}

OldOverviewMap::OldOverviewMap(uint32 mapID, const char* path)
   : OldGenericMap(mapID, path),
     m_containingMaps(0, 2)
{
   mc2dbg1 << "OldOverviewMap::OldOverviewMap(" << mapID << ", "
           << path << ")" << endl;
}

bool
OldOverviewMap::addToLookupTable(uint32 origMapID, uint32 origItemID,
                              uint32 newItemID)
{
   return m_idTranslationTable.addElement(newItemID, origMapID, origItemID);
}

bool
OldOverviewMap::removeFromLookupTable(uint32 newItemID)
{
   return m_idTranslationTable.removeElement(newItemID);
}

bool
OldOverviewMap::internalLoad(DataBuffer& dataBuffer)
{
   bool retVal = OldGenericMap::internalLoad(dataBuffer);
   mc2dbg8 << here << " OldGenericMap::internalLoad(dataBuffer) returned" 
           << endl;
   
   if (!retVal) {
      mc2log << fatal << "OldOverviewMap::internalLoad() Error calling "
             << "internalLoad() in superclass" << endl;
      return false;
   }
   uint32 mapSet = MapBits::getMapSetFromMapID(m_mapID);
   // Read the size of the m_containingMaps
   CLOCK_MAPLOAD(uint32 startTime = TimeUtility::getCurrentMicroTime());
   uint32 size = dataBuffer.readNextLong();
   m_containingMaps.setAllocSize(size);
   // Read and insert the elements into the m_containingMaps
   for (uint32 i=0; i<size; i++) {
      uint32 tmpItemID = dataBuffer.readNextLong();
      // these seem to be map IDs! fix mapSet info!
      tmpItemID = MapBits::getMapIDWithMapSet(tmpItemID, mapSet);
      m_containingMaps.addLast(tmpItemID);
   }
   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) 
                        << "] Containing maps loaded in "
                        << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0
                        << " ms" << endl;
                 startTime = TimeUtility::getCurrentMicroTime());
 
   m_idTranslationTable.load(dataBuffer, mapSet);

   CLOCK_MAPLOAD(mc2log << "[" << prettyMapIDFill(m_mapID) 
                        << "] Lookup table sorted in "
                        << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0
                        << " ms" << endl;
                 startTime = TimeUtility::getCurrentMicroTime());

   // Only present for location backwards compability.
   // Remove from here -->
   // Table contains pair of item id and location.
   typedef vector< pair<uint32, uint32> >::const_iterator it_t;
   for ( it_t it = m_tempLocationTable.begin(); 
         it != m_tempLocationTable.end(); ++it ) {
      uint32 id = it->first;
      uint32 location = it->second;
     
      mc2dbg4 << "[OldOverviewMap::internalLoad()] In strange m_tempLocationTable for loop" << endl;
      uint32 underviewMapID, foo;
      lookupNodeID( id, underviewMapID, foo );
      OldItem* item = itemLookup( id );
      
      byte underviewMun = Utility::getMunicipal( location );
      if ( underviewMun != MAX_BYTE ) {
         uint32 municipal = reverseLookup( underviewMapID, 
                                           underviewMun );
         if ( ( municipal != MAX_UINT32 ) && ( municipal != id ) ) {
            addRegionToItem(item, municipal );
         }
      }
      uint16 underviewBua = Utility::getBuiltUpArea( location );
      if ( underviewBua != MAX_UINT16 ) {
         uint32 bua = reverseLookup( underviewMapID, underviewBua );
         if ( ( bua != MAX_UINT32 ) && ( bua != id ) ) {
            addRegionToItem(item, bua );
         }
      }
   }
   m_tempLocationTable.clear();
   // <-- To here

   
   // Print all connections of this overview map
   DEBUG4(
      mc2dbg << "OldConnections in the OldOverviewMap" << endl;
      dumpConnections();
   );

   // Everything OK
   mc2dbg8 << here << " Returning." << endl;
   return true;
}


OldOverviewMap::~OldOverviewMap()
{
}

bool
OldOverviewMap::internalSave(int outfile)
{
   // Save the general map data
   bool retVal = OldGenericMap::internalSave(outfile);
   
   // Save the m_containingMaps
   DataBuffer* dataBuffer = 
      new DataBuffer(4*(m_containingMaps.getSize()+1));
   dataBuffer->fillWithZeros();

   dataBuffer->writeNextLong(m_containingMaps.getSize());
   for (uint32 i=0; i<m_containingMaps.getSize(); i++) {
      dataBuffer->writeNextLong(m_containingMaps.getElementAt(i));
   }
   write(outfile, 
         dataBuffer->getBufferAddress(), 
         dataBuffer->getCurrentOffset());
   delete dataBuffer;

   // Make sure they are sorted..
   m_idTranslationTable.sortElements();  
   DataBufferUtil::saveObject( m_idTranslationTable, outfile );
   
   // Close the output file and return the correct value
   return (retVal);
}


bool
OldOverviewMap::addMap( OldGenericMap* theMap, bool addExternalAndSave, 
                     byte mapLevel )
{
   // Constants used in this method!
   const uint32 MAX_ITEM_SIZE = 7000000;

   // Chack the inparameter
   if (theMap == NULL) {
      mc2log << warn << "OldOverviewMap::addMap the map given as parameter "
              << " is NULL!" << endl;
      return (false);
   }



   uint32 curMapID = theMap->getMapID();

   mc2dbg << "Adding map 0x" << hex << curMapID 
          << "(" << dec << curMapID << ")" 
          << " to map 0x"  << hex << getMapID() 
          << "(" << dec << getMapID() << ")."<< endl;


   // Check that theMap not already is added to this overview map
   if (m_containingMaps.binarySearch(curMapID) < MAX_UINT32) {
      mc2dbg1 << "OldOverviewMap::addMap OldMap with id " << curMapID
              << " already present in this overviewmap" << endl;
      return (false);
   }

   // Make sure the OldItemNames not are NULL
   if (m_itemNames == NULL)
      m_itemNames = new OldItemNames("MISSING");

   // Insert the new mapID to the containingMaps-vector
   m_containingMaps.addLast(curMapID);

   // Add the items to this map!
   DataBuffer* dataBuffer = new DataBuffer(MAX_ITEM_SIZE);

   // A temporary Vector with ID (in this map) of all the items 
   // that have been added
   Vector changedItems(512);

   // A temporary Vector with original ID of all the items that have 
   // been added. To make it possible to add the external connections
   // afterwards
   Vector changedItemsOriginalID(512);

   // Loop over all the zoomlevels that might have StreetSegments at
   // high levels. Ignore all 0-length boundrysegment items.
   OldBoundrySegmentsVector* theBS = theMap->getBoundrySegments();
   
   for ( int currentZoom=0; 
         currentZoom < NUMBER_GFX_ZOOMLEVELS; 
         currentZoom++) 
   {
      
      // Loop over all the items on theMap on the current zoomlevel
      uint32 nbrAddedItems = 0;
      for (uint32 i=0; i< theMap->getNbrItemsWithZoom(currentZoom); i++) {
         OldItem* curItem = theMap->getItem(currentZoom, i);
         if (curItem != NULL) {
            // Don't add boundary segments at this stage.
            if ( (theBS == NULL) ||
                 (theBS->getBoundrySegment(curItem->getID()) == NULL) ) {


               dataBuffer->reset();
               if (toIncludeItem(curItem, theMap, mapLevel)) {
                  // Everything seems ok!
                  
                  // Add the item.
                  if (addOneItem( dataBuffer, 
                                  curItem, 
                                  currentZoom,
                                  changedItems,
                                  changedItemsOriginalID,
                                  theMap ) != MAX_UINT32) {
                     nbrAddedItems++;
                  }
               }
            }
         }
      }
      mc2dbg << "   Added " << nbrAddedItems << " items of zoomlevel:" 
             << currentZoom << endl;
   }


   transferNodeExpansionTableToOverviewMap(theMap, dataBuffer, 
                                           changedItems, 
                                           changedItemsOriginalID);
   

   mc2log << info << "Adds boundry segments to items already added to the"
          << " map." << endl;
   
   // Boundary items scheduled for addition to the map.
   if (theBS != NULL) {
      vector<OldRouteableItem*> boundaryItemsToAdd;
      boundaryItemsToAdd.reserve( theBS->getSize() );
      
      // Make sure the lookup and the reverse lookup table is sorted
      // since we are going to do binary searches.
      m_idTranslationTable.sortElements();

      // Add all 0-length boundrysegment items that already have any
      // neighbors added to this overview map.

      for ( uint32 i = 0; i < theBS->getSize(); i++ ) {
         OldBoundrySegment* bs = 
            static_cast<OldBoundrySegment*> (theBS->getElementAt(i));
         if (bs != NULL) {
            // Find neighbor.
            OldRouteableItem* boundryItem = static_cast<OldRouteableItem*>
               (theMap->itemLookup( bs->getConnectRouteableItemID() ));
            
            uint32 closeToMapNodeIdx = 0;
            if ( bs->getCloseNodeValue() == 
                 OldBoundrySegment::node0close ){
               // This means that node 0 is close to boundry and node 1
               // is close to map.
               closeToMapNodeIdx =1;
            }
            OldNode* closeToMapNode = 
               boundryItem->getNode(closeToMapNodeIdx);
            uint32 closeItemID = 
               closeToMapNode->getEntryConnection(0)->getConnectFromNode()
               & ITEMID_MASK;

            if ((closeItemID != MAX_UINT32) && 
                (reverseLookup(theMap->getMapID(), closeItemID) 
                                                   != MAX_UINT32)) {
               // Schedule this boundary item for addition to the map.
               // Note that we do not add it right away, because that
               // would mean that we had to sort m_reverseLookupTable
               // after each addition -> slow.
               boundaryItemsToAdd.push_back( boundryItem );
            }
         }
      }

      // Add the boundary items.
      for ( vector<OldRouteableItem*>::const_iterator it = 
            boundaryItemsToAdd.begin(); it != boundaryItemsToAdd.end();
            ++it ) {

         // Add this 0-length boundary item.
         if ( addOneItem( dataBuffer, 
                           *it, 
                           GET_ZOOMLEVEL( (*it)->getID() ),
                           changedItems,
                           changedItemsOriginalID,
                           theMap ) == MAX_UINT32) {
            mc2log << here << warn << " Error adding boundary item "
                   << (*it)->getID() << " to overview map." << endl;
         }
      }         
      
      // Clear the vector.
      boundaryItemsToAdd.clear();
   }

   delete dataBuffer;

   // Make sure the lookup and the reverse lookup table is sorted 
   // after all insertions!
   m_idTranslationTable.sortElements();
   
   // Translate the nodeID in all the connections from 
   // (mapID.nodeID) to overviewMapID. Also translate group ids.
   // Also add signpost names to itemnames and change StringIndex
   updateConnectionsAndGroups(changedItems, theMap);

   // add landmarks from theMap to this overview map.
   mc2dbg2 << "Adding landmarks from map " << curMapID << endl;
   typedef OldGenericMap::landmarkTable_t::iterator LI;
   OldGenericMap::landmarkTable_t lmTable = theMap->getLandmarkTable();
   for (LI it = lmTable.begin(); it != lmTable.end(); it++) {
      
      uint32 fromNodeID = uint32 (((*it).first >> 32) & 0x00000000ffffffff);
      uint32 toNodeID = uint32 ((*it).first & 0x00000000ffffffff);

      uint32 overviewFromNodeID = reverseLookupNodeID(curMapID, fromNodeID);
      uint32 overviewToNodeID = reverseLookupNodeID(curMapID, toNodeID);
      if (((*it).second.type != ItemTypes::railwayLM) &&
          (overviewFromNodeID != MAX_UINT32) &&
          (overviewToNodeID != MAX_UINT32)) {
         // the connection exist in the overview map,
         // check also that the landmark item does
         uint32 itemID = reverseLookup(curMapID, (*it).second.itemID);

         if (itemID != MAX_UINT32) {
            // the "landmark item" exists.
            mc2dbg4 << "Adding lm itemID=" << itemID
                    << " " << getFirstItemName(itemID) << endl;
            ItemTypes::lmdescription_t description;
            description.itemID = itemID;
            description.importance = (*it).second.importance;
            description.side = (*it).second.side;
            description.location = (*it).second.location;
            description.type = (*it).second.type;

            uint64 key = (uint64(overviewFromNodeID) << 32) | 
                          uint64(overviewToNodeID);
            m_landmarkTable.insert(
                  pair<uint64, ItemTypes::lmdescription_t>(key, description));
         }
      }
   }
   
   // Fill the m_adminAreaCentres
   fillAdminAreaCentres( theMap, mapLevel );

   fillLanes(theMap);
   fillConnectingLanes(theMap);
   
   // Todo: Add native languages to first level ovmap
   
   if (addExternalAndSave) {
      mc2dbg << "Adding external connections from underview maps." << endl;
      // Add the external connections of the given map to the nodes in
      // this overviewmap
      OldBoundrySegmentsVector* externalConns =
         theMap->getBoundrySegments();

      for (uint32 i=0; i<changedItemsOriginalID.getSize(); i++) {
         uint32 checkOriginalID = changedItemsOriginalID.getElementAt(i);
         OldBoundrySegment* curSeg = 
               externalConns->getBoundrySegment(checkOriginalID);

         if (curSeg != NULL) {
            // The ID was found in the boundry segments array in the 
            // other map
            mc2dbg4 << "   To add external connections, " 
                    << curSeg->getNbrConnectionsToNode(0)
                    << " to node 0 and "
                    << curSeg->getNbrConnectionsToNode(1)
                    << " to node 1" << endl;
            OldConnection* curCon;
            uint32 curConID, overviewID;

            // The corresponding OldRouteableItem in the overview map
            OldRouteableItem* newItem = static_cast<OldRouteableItem*> (
                  itemLookup(changedItems.getElementAt(i)));

            // Connections to node 0
            for (uint32 j=0; 
                 j<curSeg->getNbrConnectionsToNode(0); j++) {
               curCon = curSeg->getConnectToNode0(j);
               curConID = curCon->getConnectFromNode();
               uint32 curFromMapID = curSeg->getFromMapIDToNode(0, j);

               // Make sure that the other street should be included in 
               // this map
               
               overviewID = reverseLookupNodeID(curFromMapID, curConID);

               if (overviewID != MAX_UINT32) {
                  if (newItem->getNode(0)->addConnection(new OldConnection(overviewID), 
                                                         *this ) ){
                     mc2dbg8 << "   Added connection to "
                             << newItem->getID() << " = 0x" << hex
                             << newItem->getID() << dec << " node 0 from " 
                             << overviewID << " = 0x" << hex
                             << overviewID << dec << endl;
                  }
                  else {
                     mc2dbg8 << "Failed adding connection" << endl;
                  }
               }
               else {
                  mc2dbg8 << "Missing ID: " << overviewID << endl;
               }
            }

            // OldConnections to node 1
            for (uint32 j=0; 
                 j<curSeg->getNbrConnectionsToNode(1); j++) {
               curCon = curSeg->getConnectToNode1(j);
               curConID = curCon->getConnectFromNode();
               uint32 curFromMapID = curSeg->getFromMapIDToNode(1, j);
               overviewID = reverseLookupNodeID(curFromMapID, curConID);
               if (overviewID != MAX_UINT32) {
                  newItem->getNode(1)->addConnection(
                                       new OldConnection(overviewID), *this);
                  mc2dbg4 << "   Added connection to "
                          << newItem->getID() << " = 0x" << hex
                          << newItem->getID() << dec << " node 1 from " 
                          << overviewID << " = 0x" << hex
                          << overviewID << dec << endl;
               }
            }
         }
      }

   }


   // Update the boundingbox for this map
   const GfxDataFull* otherGfx = theMap->getGfxData();
   if (otherGfx != NULL) {
      mc2dbg2 << "   Updating gfdData" << endl;
      if (m_gfxData == NULL) {
         m_gfxData = GMSGfxData::createNewGfxData(this, true);
      }
      // OK, the gfx might look funny, but the bbox will be correct...
      getGfxData()->addCoordinate(otherGfx->getMinLat(), 
                                  otherGfx->getMinLon());
      getGfxData()->addCoordinate(otherGfx->getMaxLat(), 
                                  otherGfx->getMaxLon());
   }


   if (addExternalAndSave) {
      // Save the overviewmap to disk
      return (save());
   } else {
      return (true);
   }
}


uint32
OldOverviewMap::fillLanes(OldGenericMap* otherMap)
{
   uint32 nbrLanes = 0;
   
   // Get lanes of other map.
   const ItemMap< vector<GMSLane> >& otherLanes = 
      otherMap->getNodeLaneVectors();
   for ( ItemMap< vector<GMSLane> >::const_iterator lanesIt = 
            otherLanes.begin();
         lanesIt != otherLanes.end(); ++lanesIt ){
      uint32 otherNodeID = lanesIt->first;
      uint32 ovrID = reverseLookupNodeID(otherMap->getMapID(), otherNodeID);
      if (ovrID == MAX_UINT32 ){
         // This node is not included in the overview map.
         continue;
      }
      if ( m_nodeLaneVectors.find(ovrID) != m_nodeLaneVectors.end() ){
         mc2log << error << "OldOverviewMap::fillLanes: This node already "
                << "have a node vector." << endl;
         MC2_ASSERT(false);
      }
      // Set lanes in this map.
      m_nodeLaneVectors[ovrID] = lanesIt->second;
      nbrLanes++;
   }
   mc2log << info << "fillLanes: Added " << nbrLanes << " lanes of nodes"
          << " from map 0x" << hex << otherMap->getMapID() << dec << endl;
   return nbrLanes;
} // fillLanes

uint32
OldOverviewMap::fillConnectingLanes(OldGenericMap* otherMap)
{
   uint32 nbrLanes = 0;
   const ConnectingLanesTable& lanesTable = otherMap->getConnectingLanes();
   ConnectingLanesTable::connLanesByFromAndToNode_t clByToAndFromNode =
      lanesTable.getFirstConnectingLanes();
   ConnectingLanesTable::connLanesByFromAndToNode_t noMoreData =
      make_pair( make_pair(MAX_UINT32,MAX_UINT32), MAX_UINT32 );
   uint32 i=0;
   while ( clByToAndFromNode != noMoreData ){
      uint32 otherFromNode = clByToAndFromNode.first.first;
      uint32 otherToNode = clByToAndFromNode.first.second;
      uint32 connLanesBits = clByToAndFromNode.second;
      mc2dbg8 << i << "/" << lanesTable.size() << endl;
      mc2dbg8 << "fill" << otherFromNode << ":" << otherToNode << ":" 
             << connLanesBits << endl;
      uint32 fromNodeID = 
         reverseLookupNodeID(otherMap->getMapID(), otherFromNode);
      uint32 toNodeID = 
         reverseLookupNodeID(otherMap->getMapID(), otherToNode);
      if ( ( fromNodeID != MAX_UINT32) && (toNodeID != MAX_UINT32) ){
         // This connection exists in this map.
         m_connectingLanesTable.insertConnectingLanes(fromNodeID, toNodeID, 
                                                      connLanesBits);
         nbrLanes++;
      }
      clByToAndFromNode = lanesTable.getNextConnectingLanes();
      i++;
   }
   mc2log << info << "fillConnectingLanes: Added " << nbrLanes 
          << " connecting lanes"
          << " from map 0x" << hex << otherMap->getMapID() << dec << endl;

   return nbrLanes;

} // fillConnectingLanes






uint32
OldOverviewMap::fillAdminAreaCentres( OldGenericMap* otherMap, byte mapLevel )
{
   // No admin areas in super overview (ov level 2) and city centre POIs 
   // are complete only in underview maps,
   // so admin area centres table should be filled only in ov level 1.
   if ( mapLevel != 1 ) {
      return 0;
   }
   
   uint32 nbrCentres = 0;
   uint32 otherMapId = otherMap->getMapID();

   // Get admin centres in otherMap
   adminAreaCentreTable_t centresInOther = otherMap->getAdminAreaCentres();
   // if there are no centres in otherMap, there is nothing to add to the
   // admin centres in this ov map.
   if ( centresInOther.size() == 0 ) {
      mc2dbg2 << "fillAdminAreaCentres: (" << getMapID()
              << ") No admin area centres in the other map " << otherMapId 
              << " to create from" << endl;
      return 0;
   }
   
   // Translate admin area id to overview id and add to the overview centres
   adminAreaCentreTable_t adminCentresInOverview = getAdminAreaCentres();
   adminAreaCentreTable_t::const_iterator it;
   for ( it = centresInOther.begin(); it != centresInOther.end(); it++ ) {
      uint32 ovId = reverseLookup(otherMapId, it->first);
      if ( ovId != MAX_UINT32 ) {
         adminCentresInOverview.insert( make_pair(ovId, it->second) );
         nbrCentres++;
      } else {
         // error, the admin area in otherMap does not exist in ov-map!
         mc2log << error << "fillAdminAreaCentres: admin area "
                << otherMapId << ":" << it->first 
                << " does not exist in ov-map" << endl;
      }
   }

   // Replace the admin centres in this ov map
   setAminAreaCentres( adminCentresInOverview );

   mc2log << info << "fillAdminAreaCentres: (" << getMapID()
          << ") added " << nbrCentres
          << " admin area centre coordinates to the adminAreaCentres table"
          << " from map " << otherMapId << ", now totally "
          << adminCentresInOverview.size() << " coords" << endl;
   return nbrCentres;
}

bool
OldOverviewMap::updateAdminAreaCentres( OldGenericMap* otherMap, byte mapLevel )
{
   // For dynamic map generation...
   // Since it is not possible to detect if a removed item in the otherMap
   // was either an admin area item (mun or bua) or a city centre POI,
   // we really need to re-create admin area centres from the otherMap
   // and compare with what is stored in the table of this overview map.
   // If anything differs, the centres need to be replaced.

   // return value: false=no update was made, true=update was made
   bool retVal = false;

   if ( mapLevel != 1 ) {
      return false;
   }

   // Get the admin area centres from the table in this overview map
   adminAreaCentreTable_t adminCentresInOverview = getAdminAreaCentres();
   
   // Get admin centres from otherMap
   uint32 otherMapId = otherMap->getMapID();
   adminAreaCentreTable_t otherCentres = otherMap->getAdminAreaCentres();
   mc2dbg << "OMap updateAdminAreaCentres (" << getMapID() << ") from map "
          << otherMapId << ", which has " << otherCentres.size()
          << " admin centres" << endl;
   // If there are no centres in otherMap, they may have been removed
   // Make the same updates to this overview.

   // Get admin centres from this overview map that originates in otherMap
   // store otherMap ids (not overview ids)
   adminAreaCentreTable_t myCentres;
   adminAreaCentreTable_t::const_iterator aacCIt;
   for ( aacCIt = adminCentresInOverview.begin();
         aacCIt != adminCentresInOverview.end(); aacCIt++ ) {
      uint32 ovId = aacCIt->first;
      IDPair_t uvIdsPair = lookup( ovId );
         // if  uvIdsPair MAX_UINT32,MAX_UINT32
         // might be centre for a BUA grouping same-name buas
      if ( uvIdsPair.getMapID() == otherMapId ) {
         myCentres.insert( make_pair(uvIdsPair.getItemID(), aacCIt->second) );
      }
   }
   mc2dbg << "updateAdminAreaCentres: this ov (" << getMapID() << ") has "
          << adminCentresInOverview.size() << " admin centres, "
          << myCentres.size() << " originates from other map" << endl;

   // now. make sure that otherCentres & myCentres are exactly equal
   // if not, the admin centres need update
   bool updateAdminCentres = false;
   if ( otherCentres.size() != myCentres.size() ) {
      updateAdminCentres = true;
   }
   if ( otherCentres != myCentres ) {
      updateAdminCentres = true;
   }

   // Need to update m_adminAreaCentres (adminCentresInOverview) ?
   if ( updateAdminCentres ) {
      mc2log << info 
             << "updateAdminAreaCentres: update admin area centres"
             << " before:" << myCentres.size() 
             << " now:" << otherCentres.size() << " centres" << endl;
      // erase all admin area centres originating from otherMap
      // (the ones that are in myCentres table)
      for ( aacCIt = myCentres.begin();
            aacCIt != myCentres.end(); aacCIt++ ) {
         adminCentresInOverview.erase( aacCIt->first );
      }
      
      // then add the ones from the otherCentres table (don't forget
      // to translate to overview ids)
      for ( aacCIt = otherCentres.begin();
            aacCIt != otherCentres.end(); aacCIt++ ) {
         uint32 ovId = reverseLookup(otherMapId, aacCIt->first);
         if ( ovId != MAX_UINT32 ) {
            adminCentresInOverview.insert( make_pair(ovId, aacCIt->second) );
         } else {
            // error, the admin area in otherMap does not exist in ov-map!
            mc2log << error << "updateAdminAreaCentres: admin area "
                   << otherMapId << ":" << aacCIt->first 
                   << " does not exist in ov-map!!" << endl;
         }
      }

      // Replace the admin centres in this ov map
      setAminAreaCentres( adminCentresInOverview );
      mc2dbg << "updateAdminAreaCentres: this ov (" << getMapID() 
             << ") now has "
             << adminCentresInOverview.size() << " admin centres" << endl;

      retVal = true;
   } else {
      mc2dbg << "updateAdminAreaCentres: this ov (" << getMapID()
             << ") no update" << endl;
   }

   return retVal;
} // updateAdminAreaCentres


void
OldOverviewMap::transferNodeExpansionTableToOverviewMap(
             OldGenericMap* theMap, 
             DataBuffer* dataBuffer,
             Vector& changedItems,
             Vector& changedItemsOriginalID )
{
   mc2log << info << "Transfers node expansion table to overview map."
          << endl;

   // Add the elements in the m_nodeExpansionTable, which have from node
   // and to node, which both have been added to the map.
   const map<OldGenericMap::multiNodes_t, OldGenericMap::expandedNodes_t>*
      nodeExpTable = theMap->getNodeExpansionTable();
   map<OldGenericMap::multiNodes_t, OldGenericMap::expandedNodes_t>::
      const_iterator expTblIt = nodeExpTable->begin();
   //
   // Vector with items scheduled for addition to the map because they are
   // referred by the nodes stored in the expand nodes vectors of the
   // elements in node expand table to add to the overview map, and not 
   // already present in the map.
   //
   // pair <
   //    uint32: The old ID of the node, of which we need to add the item.
   //    pair <
   //       multiNodes_t: The the from and to node that cased the need to 
   //                     add this item.
   //       uint32: The index in the expandedNodes vector in 
   //               m_nodeExpansionTable of this from and to node.
   //    >
   // >
   vector< pair< uint32, pair <OldGenericMap::multiNodes_t, uint32 > > >
      expNodesToAddToOvr;
   while ( expTblIt != nodeExpTable->end() ){
      uint32 origMapID = theMap->getMapID();
      uint32 fromNodeOrigID = expTblIt->first.first;
      uint32 toNodeOrigID = expTblIt->first.second;

      
      uint32 fromNodeNewID = 
         reverseLookupNodeID( origMapID, fromNodeOrigID);
      if ( fromNodeNewID != MAX_UINT32 ){
         uint32 toNodeNewID = 
            reverseLookupNodeID( origMapID, toNodeOrigID);
         if ( toNodeNewID != MAX_UINT32 ){
            // Both the to node and the from node of this multiconnection
            // has been added to this map. Add the nodeExpansinoTable 
            // element.
            mc2dbg8 << "Found node to add. fromNodeNewID: " 
                    << fromNodeNewID <<" toNodeNewID: " << toNodeNewID 
                    << endl;
            OldGenericMap::expandedNodes_t expNodes = expTblIt->second;
            OldGenericMap::expandedNodes_t::const_iterator expNodeIt =
               expNodes.begin();
            OldGenericMap::multiNodes_t fromAndToNode( fromNodeNewID, 
                                                    toNodeNewID);
            OldGenericMap::expandedNodes_t expNodesList;
            
            while ( expNodeIt != expNodes.end() ){
               uint32 expNodeOrigID = *expNodeIt;
               uint32 expNodeNewID = 
                  reverseLookupNodeID( origMapID, expNodeOrigID);
               expNodesList.push_back(expNodeNewID);
               mc2dbg8 << "Added expand node with new ID: " 
                       << expNodeNewID 
                       << " to multiconnection with from node: " 
                       << fromNodeNewID << " to node: " << toNodeNewID 
                       << endl;
               if ( expNodeNewID == MAX_UINT32 ) {
                  // This node is not yet added to the map.
                  // Shedule it for addition.
                  // The ID will be changed to the right one on addition,
                  // using the fromAndToNode and expNodeIdx.

                  uint32 expNodeIdx = expNodesList.size()-1;
                  MC2_ASSERT( (int32)expNodeIdx >= 0);
                  // expNode ID and expNode index in expNodesList.
                  pair<uint32, pair <OldGenericMap::multiNodes_t, uint32 > > 
                     nodeToAdd( expNodeOrigID, 
                                make_pair( fromAndToNode,
                                           expNodeIdx));
                  
                  expNodesToAddToOvr.push_back( nodeToAdd );
                  mc2dbg8 << "Scheduled item of expand node for addition. "
                          << "Old node ID: " << expNodeOrigID << endl;
                  // Do not add here because we do not want to sort the
                  // m_idTranslationTable all the time.
               }
               ++expNodeIt;
            }
            m_nodeExpansionTable.insert(make_pair( fromAndToNode,
                                                   expNodesList ) );

         }
      }
      ++expTblIt;
   }
      
   // Add the items, of which the nodes were present some expNode list of 
   // a multiconnection but not found in the overview map.
   vector< pair< uint32, pair <OldGenericMap::multiNodes_t, uint32 > > >::
      const_iterator  expNodeToAddIt = expNodesToAddToOvr.begin();
   mc2dbg8 << "expNodesToAddToOvr size(): " << expNodesToAddToOvr.size() 
           << endl;
   vector<uint32> addedItems; 
   bool printedInfo = false; // Handles prints to the generation log.
   while ( expNodeToAddIt != expNodesToAddToOvr.end() ){
      uint32 itemOrigID = MapBits::nodeItemID( expNodeToAddIt->first );
      if ( find(addedItems.begin(), addedItems.end(), itemOrigID) !=
           addedItems.end() )
      {
         // Already added this item, do nothing.
         mc2dbg8 << "Do not add item with orig ID: " << itemOrigID
                 << " because it has already been added." << endl;
      }
      else {
         if (!printedInfo){
            mc2log << info << "Adding items because they are referred by "
                   << "one or more nodes in the expansion node list of "
                   << "some multi connection(s)." << endl;
            mc2dbg << "OldItem IDs follows:" << endl;
            printedInfo=true;
         }
         OldItem* curItem = theMap->itemLookup( itemOrigID );
         int currentZoom = GET_ZOOMLEVEL(itemOrigID);
         bool added = addOneItem( dataBuffer, 
                                  curItem, 
                                  currentZoom,
                                  changedItems,
                                  changedItemsOriginalID,
                                  theMap );
         if (added){
            MC2_ASSERT(changedItemsOriginalID.size() > 0);
            uint32 addedItemOrigID = 
               changedItemsOriginalID[changedItems.size()-1];
            addedItems.push_back(addedItemOrigID);
            mc2dbg << "Added item with orig ID: " 
                   << addedItemOrigID 
                   << endl;
         }
         else{
            // Could not add this item. Severe error.
            mc2log << error << "Could not add item referrenced by multi "
                   << "connectionto expand item to overview map." << endl;
            mc2log << error << "Old node ID: " << expNodeToAddIt->first 
                   << " old item id: " << itemOrigID << endl;
            MC2_ASSERT(false);
         }
      }
      ++expNodeToAddIt;
   } // while

   if ( addedItems.size() > 0 ){
      // If we added items while the ID translation table need to be
      // sorted again.
      m_idTranslationTable.sortElements();
   }

   
   // Loop over the items to add and update the node id in the expansion
   // list in m_nodeExpansionTable.
   expNodeToAddIt = expNodesToAddToOvr.begin();
   while ( expNodeToAddIt != expNodesToAddToOvr.end() ){
   

      // Update the item id of the expand node that referred to an item
      // earlier not added to the map.
      OldGenericMap::multiNodes_t fromAndToNode =
         expNodeToAddIt->second.first;
      uint32 expNodeIdx = expNodeToAddIt->second.second;
      map<OldGenericMap::multiNodes_t, OldGenericMap::expandedNodes_t>::
         iterator it = m_nodeExpansionTable.find(fromAndToNode);
      if (it != m_nodeExpansionTable.end() ){
         OldGenericMap::expandedNodes_t* expNodesList = &(it->second);
         MC2_ASSERT( (expNodeIdx >= 0) && 
                     (expNodeIdx < expNodesList->size() ) );
         // Iterate through the expand node list until we find the
         // list item with the right index.
         OldGenericMap::expandedNodes_t::iterator expNodeListIt = 
            expNodesList->begin();
         for (uint32 i=0; i<expNodeIdx; i++){
            expNodeListIt++;
         }
         uint32 origNodeID = expNodeToAddIt->first;
         mc2dbg8 << "Id in expand node list: " << *expNodeListIt << endl;
         *expNodeListIt = reverseLookupNodeID( theMap->getMapID(), 
                                               origNodeID );
         mc2dbg8 << "Updated node from original ID: " <<  origNodeID 
                 << " to new ID: " << *expNodeListIt << endl;
 
      }
      else {
         MC2_ASSERT(false);
      }
      ++expNodeToAddIt;
   }

} // transferNodeExpansionTableToOldOverviewMap
   


bool
OldOverviewMap::mapCommonNames(vector<OldItem*>& items,
                            map< uint32, set<OldItem*> >& resultItemsByItemID)
{
   bool result = true;
                            
   vector< pair <OldItem*, bool> > adminAreas;
   for ( vector<OldItem*>::const_iterator itemIt = items.begin(); 
         itemIt!= items.end(); ++itemIt ){
      adminAreas.push_back(make_pair(*itemIt, false) );
   }
      

   // Loop all admin areas
   vector< pair <OldItem*, bool> >::iterator admIt = adminAreas.begin();
   while ( admIt != adminAreas.end() ) {
      // Search forward among admin areas for areas with the same names.
      if ( admIt->second == true ){
         // This one has already been processed, skip it.
         admIt++;
         continue;
      }

      vector< pair <OldItem*, bool> >::iterator admSearchIt = admIt;
      ++admSearchIt; // Go to next item.
      while ( admSearchIt != adminAreas.end() ){

         if ( admIt->first->hasCommonLangName( admSearchIt->first ) ){
            // One name match with both string index and language type
            // between the items.
            
            uint32 firstItemID = admIt->first->getID();
            if ( firstItemID != MAX_UINT32 ){

               // Iterator used for inserting items with common names.
               map< uint32, set<OldItem*> >::iterator insertIt = 
                  resultItemsByItemID.find( firstItemID );

               if ( insertIt == resultItemsByItemID.end() ){
                  // A set for this item has not been added before.

                  // Add a new element to resultItemsByItemID.
                  // insert() only adds the item if it is not already
                  // present.
                  set<OldItem*> itemSet;
                  pair< map< uint32, set<OldItem*> >::iterator, bool > res =
                     resultItemsByItemID.insert( make_pair( firstItemID, 
                                                      itemSet ));
                  insertIt = res.first;
                  
                  if ( res.second == false ){
                     mc2log << error << "OldOverviewMap::mapCommonNames "
                            << " Could not insert item to stl::map." 
                            << " OldItem ID: " << firstItemID << endl;
                     exit(1);
                  }

                  // Because this item had not been inserted before,
                  // add it first in its set.
                  insertIt->second.insert( admIt->first );
                  admIt->second = true; // Have been processed = true.
               }
               insertIt->second.insert( admSearchIt->first );
               admSearchIt->second = true; // Have been processed = true
            }
         }
         ++admSearchIt;
      }
      ++admIt;
   }



   // Print content of the common name map.
#  ifdef DEBUG_LEVEL_8
      map< uint32, set<OldItem*> >::const_iterator commonNamePrintIt = 
         resultItemsByItemID.begin();
      while ( commonNamePrintIt != resultItemsByItemID.end() ){
         mc2dbg << "OldItem with ID: " << commonNamePrintIt->first
                << endl;
         set< OldItem*>::const_iterator itemIt = 
            commonNamePrintIt->second.begin();
         while ( itemIt != commonNamePrintIt->second.end() ){
            mc2dbg << "   " << this->getBestItemName( (*itemIt)->getID() ) 
                   << " ID: " << (*itemIt)->getID() << endl;
            ++itemIt;
         }
         ++commonNamePrintIt;
      }
#  endif   // DEBUG_LEVEL_8

      return result;
} // mapCommonNames


OldItem*
OldOverviewMap::addGroupItem( OldItem* nameItem, 
                              ItemTypes::itemType groupItemType, 
                              set<OldItem*>& items)
{
   OldItem* groupItem = NULL;

   // Clear out all items not having only names present in the name 
   // item as an extra precaution.
   set<OldItem*>::iterator changeItemIt = items.begin();
   while ( changeItemIt != items.end() ) {
      if ( ! nameItem->hasAllLangNames(*changeItemIt) ){  
         //mc2log << warn 
         //       <<  "Found item with some other name than the name" 
         //       << " item. Keeps it." << endl;
         mc2log << warn << "Removes item ID:" 
                << (*changeItemIt)->getID()
                << " from items to group, not all names same." << endl;
         set<OldItem*>::iterator rmIt = changeItemIt;
         ++changeItemIt;  
         items.erase(rmIt);
      }
      else {
         ++changeItemIt;
      }
   }
   
   // If there are no items left to merge after removing items not 
   // having the right names, continue with next merge list
   if ( items.size() < 1 ){
      mc2log << warn << "Nothing left to merge, no group item created."
             << endl;
      groupItem=NULL;
      return groupItem;
   }
   
   
   // Create new group item.
   if ( groupItemType == ItemTypes::ItemTypes::builtUpAreaItem){
      groupItem = new OldBuiltUpAreaItem( MAX_UINT32 );
   }
   else if ( groupItemType == ItemTypes::ItemTypes::municipalItem){
      groupItem = new OldMunicipalItem( MAX_UINT32 );
   }
   else {
      mc2log << error << "Invalid group item type, exits!" << endl;
      exit(1);
   }
   
   // Set names to new group item.
   LangTypes::language_t nameLang;
   ItemTypes::name_t nameType;
   uint32 nameStrIdx;
   for (byte nameOffset = 0; 
        nameOffset<nameItem->getNbrNames(); 
        nameOffset++ )
      {
         nameItem->getNameAndType( nameOffset, 
                                   nameLang,
                                   nameType,
                                   nameStrIdx );
         groupItem->addName(nameLang, nameType, nameStrIdx  );      
      }
   // Set names to grouped items.
   set<OldItem*>::const_iterator itemIt = items.begin();
   while ( itemIt != items.end() ) {
      // Don't change the names of the name item.
      if ( (*itemIt) != nameItem ){
         (*itemIt)->removeAllNames();
         for (byte nameOffset = 0; 
              nameOffset<nameItem->getNbrNames(); 
              nameOffset++ )
            {
               nameItem->getNameAndType( nameOffset, 
                                         nameLang,
                                         nameType,
                                         nameStrIdx );
               (*itemIt)->addName(nameLang, nameType, nameStrIdx  );      
            }
      }
      ++itemIt;
   }
   
   // Add the group item to the overview map.
   uint32 groupItemID = 
      this->addItem(groupItem, GET_ZOOMLEVEL(nameItem->getID()));
   if ( groupItemID == MAX_UINT32 ){
      mc2log << error << "Could not add new group item to overview. Exits!"
             << endl;
      delete groupItem;
      groupItem = NULL;
      MC2_ASSERT(false);
   }
   else {
      mc2dbg << "Added group item for grouping admin areas. Item ID: "
             << groupItemID << endl;
   }

   // Add a good admin centre to the group item
   uint32 largestItemID = MAX_UINT32;
   uint64 largestArea = 0;
   uint32 largestItemIDwithValidCentreCoord = MAX_UINT32;
   itemIt = items.begin();
   while ( itemIt != items.end() ) {
      GfxData* gfx = (*itemIt)->getGfxData();
      if ( gfx != NULL ) {
         if ( gfx->getBBoxArea_mc2() > largestArea ) {
            largestArea = gfx->getBBoxArea_mc2();
            largestItemID = (*itemIt)->getID();
            MC2Coordinate centre = getCentreFromAdminTable(largestItemID);
            if ( centre.isValid() ) {
               largestItemIDwithValidCentreCoord = largestItemID;
            }
         }
      }
      ++itemIt;
   }
   if ( largestItemID != MAX_UINT32 ) {
      MC2Coordinate centre = getCentreFromAdminTable(largestItemID);
      if ( centre.isValid() ) {
         mc2dbg << "Set admin centre coord for the group item, copy from "
                << largestItemID << " " << centre << endl;
         adminAreaCentreTable_t newCentres;
         newCentres.insert( make_pair(groupItemID,centre) );
         addAdminCenters( newCentres );
      } else {
         mc2log << warn << "No valid admin area centre for the group item "
                << this->getBestItemName( groupItemID )
                << ", tried to copy from " << largestItemID
                << " bbox area " << (largestArea *
                     GfxConstants::SQUARE_MC2SCALE_TO_SQUARE_METER)
                << " m2" << endl;
         if ( largestItemIDwithValidCentreCoord != MAX_UINT32 ) {
            mc2log << warn << "Not largest bbox area id="
                   << largestItemIDwithValidCentreCoord
                   << " has admin centre" << endl;
         }
      }
   }

   // If all included items are index areas, mark also the group item
   uint32 indexAreaOrder = MAX_UINT32;
   bool setIndexArea = false;
   itemIt = items.begin();
   while ( itemIt != items.end() ) {
      uint32 tmpOrder = getIndexAreaOrder( (*itemIt)->getID() );
      if ( tmpOrder != MAX_UINT32 ) {
         if ( indexAreaOrder == MAX_UINT32 ) {
            setIndexArea = true;
            indexAreaOrder = tmpOrder;
         }
         else {
            if ( indexAreaOrder != tmpOrder ) {
               // different orders of the index areas to merge, skip?
               setIndexArea = false;
            }
         }
      }
      ++itemIt;
   }
   if ( setIndexArea ) {
      setIndexAreaOrder( groupItemID, indexAreaOrder );
      mc2dbg << "Set index area order " << indexAreaOrder 
             << " for the group item " << endl;
   }


   
   // Add the new group to the items to group.
   itemIt = items.begin();
   uint32 nbrInhabitants = 0;
   while ( itemIt != items.end() ) {
      // Print groups
      OldItem* groupPrintItem = (*itemIt);
      mc2dbg << "Groups of item:" << groupPrintItem->getID() 
             << " Best name:" 
             << this->getBestItemName( groupPrintItem->getID() ) 
             << endl;
      for ( uint32 gIdx = 0; gIdx < groupPrintItem->getNbrGroups(); gIdx++){
         uint32 gID = groupPrintItem->getGroup( gIdx );
         OldItem* tmpGroupItem = this->itemLookup( gID );
         if ( tmpGroupItem == NULL){
            mc2dbg  << "   Group with ID:" << gID << " is NULL." << endl;
         }
         else {
            mc2dbg  << "   group: " << tmpGroupItem->getID() 
                    << " Best name: " 
                    <<  this->getBestItemName( tmpGroupItem->getID() ) 
                    << endl;
         } 
      }
      
      // Remove all current groups, and add the newly created group item
      if ( !(*itemIt)->removeAllGroups() ){
         mc2log << error << "Could not remove all groups of item ID:"
                << (*itemIt)->getID() << endl;
         exit(1);
      }
      if ( ! addRegionToItem(*itemIt, groupItemID) ){
         mc2log << error << "Could not add new group item ID:" 
                << groupItemID << " to item ID:" << (*itemIt)->getID() 
                << endl;
         exit(1);
      }
      else {
         mc2dbg << " Added group ID: " << groupItemID 
                << " to item ID: "
                << (*itemIt)->getID() << endl;
      }
         
      // Print groups again
      mc2dbg << " Groups of item:" << groupPrintItem->getID() 
             << " Best name:" 
             << this->getBestItemName( groupPrintItem->getID() ) 
             << endl;
      for ( uint32 gIdx = 0; gIdx < groupPrintItem->getNbrGroups();
            gIdx++)
      {
         uint32 gID = groupPrintItem->getGroup( gIdx );
         OldItem* tmpGroupItem = this->itemLookup( gID );
         if ( tmpGroupItem == NULL){
            mc2dbg  << "   Group with ID:" << gID << " is NULL." 
                    << endl;
         }
         else {
            mc2dbg  << "   group: " << tmpGroupItem->getID() 
                    << " Best name: " 
                    <<  this->getBestItemName( tmpGroupItem->getID() ) 
                    << endl;
         } 
      }

      //  Handle number of inhabitants.
      if ( groupItemType == ItemTypes::builtUpAreaItem){
         OldBuiltUpAreaItem* bua = 
            dynamic_cast<OldBuiltUpAreaItem*>(*itemIt);
         // Sometimes municipals are grouped in
         // BUAs. Therefore it is OK for an item here to not be a BUA.
         if ( bua != NULL ){
            nbrInhabitants+=bua->getNbrInhabitants();
         }
      }

      ++itemIt;
   }

   // Handle number of inhabitants.
   
   if ( groupItemType == ItemTypes::builtUpAreaItem){
      OldBuiltUpAreaItem* bua = 
         dynamic_cast<OldBuiltUpAreaItem*>(groupItem);
      if ( bua == NULL ){
         mc2log << error << "BUA(groupItem): " << groupItem->getID() 
                << " is null." << endl;
         exit(1);
      }
      bua->setNbrInhabitants(nbrInhabitants);
   }

   
   return groupItem;
} // addGroupItem


bool
OldOverviewMap::groupSameNameAdminIndexAreas(
      set<ItemTypes::itemType> itemTypes)
{
   bool result = true;
   if ( itemTypes.size() == 0 ){
      mc2log << error << "No item types to merge, exits!" << endl;
      exit(1);
   }   

   // Print what to do.
   mc2log << info << "OldOverviewMap::groupSameNameAdminIndexAreas, "
          << "item types:";
   for ( set<ItemTypes::itemType>::const_iterator typeIt = 
            itemTypes.begin();
         typeIt != itemTypes.end(); ++typeIt ){
      if ( typeIt != itemTypes.begin() ){
         mc2log << ", ";
      }
      mc2log << (int)*typeIt;
   }
   mc2log << endl;

   bool indexAreas = NationalProperties::useIndexAreas(
         this->getCountryCode(), this->getMapOrigin());
   if ( ! indexAreas ) {
      mc2log << error << "Incorrect using this method "
             << "if not having index areas" << endl;
      MC2_ASSERT(false);
   }
   
   // Collect the items to group.
   vector< OldItem* > adminAreas;
   for(uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z, i);
         if (item != NULL) {

            // Is this one of the item types to group.
            bool found = false;
            set<ItemTypes::itemType>::const_iterator typeIt = 
               itemTypes.begin();
            while ( !found && typeIt != itemTypes.end() ){
               if ( item->getItemType() == *typeIt ){
                  found=true;
               }
               ++typeIt;
            }

            if ( found ){
               if ( isIndexArea(item->getID()) ) {
                  adminAreas.push_back(item);
               }
            }
         }
      }
   }
   mc2dbg1 << "Number admin areas=" << adminAreas.size() << endl;
   if ( adminAreas.size() == 0 ) {
      mc2dbg << "No items to group - return" << endl;
      return result;
   }

   // Mapping item ID of first item found to have equal names to another 
   // item, to all items, including itself, which have equal names with 
   // the item with the key item ID.

   // Common names by item ID. The key is the item ID of the first
   // item added to the set.
   map< uint32, set<OldItem*> > itemsByItemID;
   
   // Puts the result in itemsByItemID.
   mapCommonNames( adminAreas, itemsByItemID );
   
   
   // Print info about which commonName groups we have
   mc2dbg << "Number common name groups=" << itemsByItemID.size() << endl;
   for ( map< uint32, set<OldItem*> >::iterator itemSetIt = 
            itemsByItemID.begin();
         itemSetIt != itemsByItemID.end(); ++itemSetIt ){
      if ( itemSetIt->second.size() > 1 ) {
         mc2dbg << " item " << itemSetIt->first << " "
                << getBestItemName( itemSetIt->first )
                << " size=" << itemSetIt->second.size() << endl;
      }
      for ( set<OldItem*>::const_iterator tmpIt = 
               itemSetIt->second.begin();
               tmpIt != itemSetIt->second.end(); tmpIt++ 
          ) {
         IDPair_t uvIdsPair = lookup( (*tmpIt)->getID() );
         mc2dbg << "   item " << (*tmpIt)->getID()
                << " ia order=" << getIndexAreaOrder((*tmpIt)->getID())
                << " (uv valid=" << int(uvIdsPair.isValid())
                << " " << uvIdsPair.getMapID() << ":"
                << uvIdsPair.getItemID() << ") "
                << " type=" << int((*tmpIt)->getItemType())
                << endl;
         for ( uint32 gIdx = 0; gIdx < (*tmpIt)->getNbrGroups(); gIdx++){
            uint32 gID = (*tmpIt)->getGroup( gIdx );
            OldItem* groupItem = this->itemLookup( gID );
            if ( groupItem == NULL){
               mc2dbg << "     group " << gID << " is NULL."
                      << endl;
            }
            else {
               mc2dbg << "     group " << groupItem->getID()
                    << " Best name: " 
                    <<  this->getBestItemName( groupItem->getID() ) 
                    << endl;
            }
         }
      }
   }

   mc2dbg1 << "Group same-ia-order same-group-name items" << endl;
   // Group close items.
   // Close = 
   // 1) the items have the same index area order
   // 2) the items have a group with the same name
   //    special case: the items have no group at all

   // The item initially used when finding other items close to it is
   // the  key of this map. The item vector should keep all other items
   // close enough to be merged with the initial item. I.e. close 
   // enough to the initial item or one of the items in the item 
   // vector.
   map< OldItem*, list<OldItem*> > mergeLists;

   // Loop all sets with common names items.   
   map< uint32, set<OldItem*> >::iterator commonNamesIt = 
      itemsByItemID.begin();
   while ( commonNamesIt != itemsByItemID.end() ){

      set<OldItem*>* commonNameItems = &(commonNamesIt->second);
      set<OldItem*>::const_iterator itemIt = commonNameItems->begin();

      // Check this set to see if a particular item already have been 
      // processed. 
      set<OldItem*> processedItems;

      while ( itemIt != commonNameItems->end() ){
         //Loop all items with common name.

         if ( processedItems.find( (*itemIt) ) == processedItems.end() ){
            // Only process this item if it has not been processed before.
         
            // Returns items after itemIt in the set, which are close to
            // it.
            set<OldItem*>::const_iterator startIt = itemIt;
            vector<OldItem*> itemsClose = 
                  nextItemsSameIAOrderInSameNameGroup( 
                        *commonNameItems, ++startIt, *itemIt );

            if ( itemsClose.size() > 0 ){
               // Get a pointer to the merge list to fill in.
               map< OldItem*, list<OldItem*> >::iterator currListVecIt =
                  mergeLists.find(*itemIt);
               if ( currListVecIt == mergeLists.end() ){
               // Create merge list for this item.
                  list<OldItem*> itemList;
                  pair< map< OldItem*, list<OldItem*> >::iterator, bool > res =
                     mergeLists.insert( make_pair( *itemIt, 
                                                     itemList ) );
                  currListVecIt = res.first;

                  if ( res.second == false ){
                     mc2log << error 
                         << "OldOverviewMap::groupSameNameAdminIndexAreas: "
                            << "Could not create new item merge list for "
                            << "item with OldItemID: " << (*itemIt)->getID()
                            << endl;
                     exit(1);
                  }
               }
               list<OldItem*>* currMergeList = &(currListVecIt->second);
               moveToMergeList( itemsClose, 
                                currMergeList,
                                &processedItems );
            }
         }

         ++itemIt;   
      }

      ++commonNamesIt;
   }
   if ( mergeLists.size() == 0 ) {
      mc2dbg << "No items to group - return" << endl;
      return result;
   }

   
   // Print the result to the log as info.
   mc2dbg << "=== Items to group in overview ===" << endl;
   printItemsToGroup( mergeLists );
   mc2dbg << "=== End ===" << endl;

   
   
   // Create group item for the items with the same name close to each 
   // other. Change name of all items grouped this way so all have exactly
   // the same names.
   mc2dbg << "=== Create group item for same-name close items ===" 
          << endl;
   map< OldItem*, list<OldItem*> >::iterator mergeItemsIt = 
      mergeLists.begin();
   while ( mergeItemsIt != mergeLists.end() ){
      OldItem* firstItem = mergeItemsIt->first;
      list<OldItem*>& itemList = mergeItemsIt->second;
      mc2dbg1 << "First item " << firstItem->getID() << " "
              << getBestItemName(firstItem->getID()) << endl;
      

      // Find an item to use the name of.

      // Set name to all grouped items incluing new one.
      OldItem* nameItem = firstItem;
                             // This item's names are set to all grouped 
                             // items because the grouping in the search
                             // module does not work if they do not have
                             // exact the same names.
      
      // Print what names to use for these items.
      mc2dbg1 << "Names used for grouped items:" << endl;
      for (byte nameOffset = 0; 
           nameOffset<nameItem->getNbrNames(); 
           nameOffset++ )
      {
         LangTypes::language_t nameLang;
         ItemTypes::name_t nameType;
         uint32 nameStrIdx;
         nameItem->getNameAndType( nameOffset, 
                                   nameLang,
                                   nameType,
                                   nameStrIdx );
         mc2dbg1 << "   " << getName(nameStrIdx) << "(" 
                 << LangTypes::getLanguageAsISO639(nameLang) << ", "
                 << ItemTypes::getNameTypeAsString(nameType, true/*short*/)
                 << ")" << endl;
      }



      // Check if the index area items to group are located in
      // another index area. Save it, and use it as group for the new 
      // BUA group item to create.
      const OldItem* groupToSet = NULL;
      if ( isIndexArea(firstItem->getID()) &&
           firstItem->getNbrGroups() > 0 ) {
         MC2_ASSERT(firstItem->getNbrGroups() == 1);
         OldItem* tmpGroupItem = itemLookup( firstItem->getGroup(0) );
         if ( isIndexArea(tmpGroupItem->getID()) ) {
            groupToSet = tmpGroupItem;
         }
         
         if ( groupToSet != NULL ) {
            mc2dbg << "Found group to use " << groupToSet->getID()
                   << " " << this->getBestItemName( groupToSet->getID() ) 
                   << endl;
         } else {
            mc2dbg << "No group to use" << endl;
         }

      }




      // These are the items to create a group for.
      set<OldItem*> allItems;
      allItems.insert(firstItem);
      for ( list<OldItem*>::const_iterator itemListIt = itemList.begin();
            itemListIt != itemList.end(); ++itemListIt ) {
         allItems.insert(*itemListIt);
      }
      OldItem* groupItem = addGroupItem(nameItem, 
                                        nameItem->getItemType(),
                                        allItems);
      if ( groupItem == NULL ){
         mc2log << warn << "No group created for item ID:" 
                << nameItem->getID() << ", best name:" 
                << getBestItemName(nameItem->getID()) << endl;

         ++mergeItemsIt;
         continue;
      }


      // Set the group we found earlier to the group item.
      if ( groupToSet != NULL ){
         addRegionToItem(groupItem, groupToSet->getID() );
         mc2log << info << "Added index area with item ID: " 
                << groupToSet->getID() << ", best name: " 
                << this->getBestItemName( groupToSet->getID() )
                << " as group to the new group item with item ID: " 
                << groupItem->getID() << endl;
      }
         
      
      ++mergeItemsIt;

   } // while mergeLists
   return result;

} // groupSameNameAdminIndexAreas



bool
OldOverviewMap::groupAdminAreas(ItemTypes::itemType itemType,
                             uint32 maxMergeDist )
{
   bool result = true;

   if ( ( itemType != ItemTypes::builtUpAreaItem ) && 
        ( itemType != ItemTypes::municipalItem ) // &&
        //( itemType != ItemTypes::zipCodeItem ) 
      )
   {
      mc2log << warn 
             << "OldOverviewMap::groupAdminAreas"
             << "unsupported itemtype supplied" << (int)itemType << endl;
      return false;
   }
   mc2log << info << "Running OldOverviewMap::groupAdminAreas for item type "
          << (int)itemType << " with merge distance: " << maxMergeDist 
          << " meters." << endl;

   // Determine the zoomlevel of the adimin areas.
   uint32 adminZoom = 0;
   if ( itemType == ItemTypes::zipCodeItem ) {
      adminZoom = 10;
   }

   // Gather the admin areas, which will be grouped in this method.
   // Vector containing all items and a bool telling whether the item has 
   // been added to a common name list already.
   vector< OldItem* > adminAreas;
   for (uint32 i=0; i < getNbrItemsWithZoom( adminZoom ); i++) {
      OldItem* item = getItem(adminZoom, i);
      if ((item != NULL) &&
          (item->getItemType() == itemType)) 
      {
         // index areas are not grouped in this method
         if ( ! isIndexArea(item->getID()) ) {
            adminAreas.push_back(item);
         }
      }
   }
   if ( adminAreas.size() == 0 ) {
      mc2dbg << "No items to group - return" << endl;
      return result;
   }

   
   // Mapping item ID of first item found to have equal names to another 
   // item, to all items, including intself, which have equal names with 
   // the item with the key item ID.

   // Common names by item ID. The key is the item ID of the first
   // item added to the set.
   map< uint32, set<OldItem*> > itemsByItemID;
   
   // Puts the result in itemsByItemID.
   mapCommonNames( adminAreas, itemsByItemID );


   // Group close items.

   // The item initially used when finding other items close to it is
   // the  key of this map. The item vector should keep all other items
   // close enough to be merged with the initial item. I.e. close 
   // enough to the initial item or one of the items in the item 
   // vector.
   map< OldItem*, list<OldItem*> > mergeLists;

   // Loop all sets with common names items.   
   map< uint32, set<OldItem*> >::iterator commonNamesIt = 
      itemsByItemID.begin();
   while ( commonNamesIt != itemsByItemID.end() ){

      set<OldItem*>* commonNameItems = &(commonNamesIt->second);
      set<OldItem*>::const_iterator itemIt = commonNameItems->begin();

      // Check this set to see if a particular item already have been 
      // processed. 
      set<OldItem*> processedItems;

      while ( itemIt != commonNameItems->end() ){
         //Loop all items with common name.

         if ( processedItems.find( (*itemIt) ) == processedItems.end() ){
            // Only process this item if it has not been processed before.
         
            // Returns items after itemIt in the set, which are close to
            // it.
            set<OldItem*>::const_iterator startIt = itemIt;
            vector<OldItem*> itemsClose = nextItemsClose( *commonNameItems, 
                                                    ++startIt,
                                                    *itemIt,
                                                    maxMergeDist );

            if ( itemsClose.size() > 0 ){
               // Get a pointer to the merge list to fill in.
               map< OldItem*, list<OldItem*> >::iterator currListVecIt =
                  mergeLists.find(*itemIt);
               if ( currListVecIt == mergeLists.end() ){
               // Create merge list for this item.
                  list<OldItem*> itemList;
                  pair< map< OldItem*, list<OldItem*> >::iterator, bool > res =
                     mergeLists.insert( make_pair( *itemIt, 
                                                     itemList ) );
                  currListVecIt = res.first;

                  if ( res.second == false ){
                     mc2log << error << "OldOverviewMap::groupAdminAreas: "
                            << "Could not create new item merge list for "
                            << "item with OldItemID: " << (*itemIt)->getID()
                            << endl;
                     exit(1);
                  }
               }
               list<OldItem*>* currMergeList = &(currListVecIt->second);
               moveToMergeList( itemsClose, 
                                  currMergeList,
                                  &processedItems );
               
               // Go through all items in merge lists and add items close 
               // to them from the common name set.
               list<OldItem*>::iterator mergeItemIt = 
                  currMergeList->begin();
               while ( mergeItemIt != currMergeList->end() ){
                  vector<OldItem*> moreItemsClose = 
                     nextItemsClose( *commonNameItems, 
                                     startIt,
                                     *mergeItemIt,
                                     maxMergeDist );
                  
                  moveToMergeList( moreItemsClose, 
                                    currMergeList,
                                    &processedItems );
                  
                  ++mergeItemIt;
               }
            }
         }

         ++itemIt;   
      }

      ++commonNamesIt;
   }

   if ( mergeLists.size() == 0 ) {
      mc2dbg << "No items to group - return" << endl;
      return result;
   }


   // Print the result to the log as info.
   mc2dbg << "=== Items to group in overview ===" << endl;
   printItemsToGroup( mergeLists );
   mc2dbg << "=== End ===" << endl;



   // Create group item for the items with the same name close to each 
   // other. Change name of all items grouped this way so all have exactly
   // the same names.
   mc2dbg << "=== Create group item for same-name close items ===" 
          << endl;
   map< OldItem*, list<OldItem*> >::iterator mergeItemsIt = 
      mergeLists.begin();
   while ( mergeItemsIt != mergeLists.end() ){
      OldItem* firstItem = mergeItemsIt->first;
      list<OldItem*>& itemList = mergeItemsIt->second;
      

      // Find an item to use the name of.

      // Set name to all grouped items incluing new one.
      OldItem* nameItem = NULL; // This item's names are set to all grouped 
                             // items because the grouping in the search
                             // module does not work if they do not have
                             // exact the same names.
      if ( itemType == ItemTypes::builtUpAreaItem ){
         nameItem = findNameItem( firstItem, itemList );
      }
      if ( nameItem == NULL ){
         // Did not find a specific name item, set the first bua as name 
         // item.
         nameItem = firstItem;
      }

      
      // Print what names to use for these items.
      mc2dbg << "Names used for grouped items:" << endl;
      for (byte nameOffset = 0; 
           nameOffset<nameItem->getNbrNames(); 
           nameOffset++ )
      {
         LangTypes::language_t nameLang;
         ItemTypes::name_t nameType;
         uint32 nameStrIdx;
         nameItem->getNameAndType( nameOffset, 
                                   nameLang,
                                   nameType,
                                   nameStrIdx );
         mc2dbg << "   " << getName(nameStrIdx) << "(" 
                << LangTypes::getLanguageAsISO639(nameLang) << ", "
                << ItemTypes::getNameTypeAsString(nameType, true/*short*/)
                << ")" << endl;
      }



      // Try to find a municipal group for the new BUA group item to 
      // create. The municipal is used as group if it has one name common
      // with the BUA. Using the name item for name comparing since the new
      // group item is not yet created.
         const OldItem* groupToSet = NULL;      
      if ( itemType == ItemTypes::builtUpAreaItem ){
         mc2dbg << "Trying to find municipal to set as group to new group"
                << " item." << endl;
         
         // Check groups of first item.
         for ( uint32 groupOffset = 0; 
               groupOffset<firstItem->getNbrGroups();
               groupOffset++)
         {
            const OldItem* tmpGroupItem = 
               this->itemLookup( firstItem->getGroup(groupOffset) );
            if ( ( tmpGroupItem->getItemType() ==
                   ItemTypes::municipalItem )  &&
                 this->oneNameSameCase( tmpGroupItem, nameItem ) )
            {
               groupToSet = tmpGroupItem;
            }
         }
         // Check groups of ohter items.
         list<OldItem*>::const_iterator itemIt = itemList.begin();
         while ( (itemIt != itemList.end() ) && 
                 (groupToSet == NULL) )
         {
            for ( uint32 groupOffset = 0; 
                  groupOffset<(*itemIt)->getNbrGroups();
                  groupOffset++)
            {
               const OldItem* tmpGroupItem = 
                  this->itemLookup( (*itemIt)->getGroup(groupOffset) );
               if ( ( tmpGroupItem->getItemType() == 
                      ItemTypes::municipalItem ) &&
                    this->oneNameSameCase( tmpGroupItem, nameItem ) )
               {
                  groupToSet = tmpGroupItem;
               }
            }
            ++itemIt;
         }
         if ( groupToSet != NULL ) {
            mc2dbg << "Found group to use " << groupToSet->getID()
                   << " " << this->getBestItemName( groupToSet->getID() ) 
                   << endl;
         } else {
            mc2dbg << "No group to use" << endl;
         }

      } // look for municipal group for new BUA group item.





      

      // These are the items to create a group for.
      set<OldItem*> allItems;
      allItems.insert(firstItem);
      for ( list<OldItem*>::const_iterator itemListIt = itemList.begin();
            itemListIt != itemList.end(); ++itemListIt ) {
         allItems.insert(*itemListIt);
      }
      OldItem* groupItem = addGroupItem(nameItem, itemType, allItems);
      if ( groupItem == NULL ){
         mc2log << warn << "No group created for item ID:" 
                << nameItem->getID() << ", best name:" 
                << getBestItemName(nameItem->getID()) << endl;

         ++mergeItemsIt;
         continue;
      }


      // Set the group we found earlier to the group item.
      if ( groupToSet != NULL ){
         addRegionToItem(groupItem, groupToSet->getID() );
         mc2log << info << "Added municipal with item ID: " 
                << groupToSet->getID() << ", best name: " 
                << this->getBestItemName( groupToSet->getID() )
                << " as group to the new group item with item ID: " 
                << groupItem->getID() << endl;
      }
         
      
      ++mergeItemsIt;

   } // while mergeLists

   return result;

} // groupAdminAreas


OldItem*
OldOverviewMap::findNameItem( OldItem* firstItem, 
                               list<OldItem*> itemList ) const
{
   OldItem* nameItem = NULL;

   // For built up areas, try to locate a BUA with a municipal with
   // the same name as group or a municipal with same name with this
   // BUA as group.
   
   // Check for a municipal as group for a BUA.
   
   // Check groups of first bua item.
   if ( getGroupWithCommonLangName(firstItem,
                            ItemTypes::municipalItem) != NULL )
   {
      // Found municipal group with common name with firstItem.
      nameItem = firstItem;
   }

   // Check groups of the other bua items.
   list<OldItem*>::const_iterator itemIt = itemList.begin();
   while ( ( nameItem == NULL ) && (itemIt != itemList.end() ) )
   {
      OldItem* item = (*itemIt);
      if ( getGroupWithCommonLangName( item,
                         ItemTypes::municipalItem ) != NULL )
      {
         // Found municipal group with common name with item.
         nameItem = item;
      }
      ++itemIt;
   }

   // Check built up areas as group for municipals.

   // Includes all bua items.
   list<OldItem*> buaItems( itemList.size() );
   copy(itemList.begin(), itemList.end(), buaItems.begin());
   buaItems.push_front(firstItem);
   
   uint32 z=0;
   while( (nameItem == NULL) && (z<NUMBER_GFX_ZOOMLEVELS ) ){
      uint32 i=0;
      while ( (nameItem == NULL) && ( i<getNbrItemsWithZoom(z) ) ){
         OldItem* curMunItem = getItem(z, i);
         if ( (curMunItem != NULL) && 
              (curMunItem->getItemType() == 
               ItemTypes::municipalItem ) 
            )
         {
            // 1) Which of the BUAs are groups of this municipal
            list<OldItem*> buaGroupsOfMun;
            for ( list<OldItem*>::const_iterator buaIt = 
                     buaItems.begin();
                  buaIt != buaItems.end();
                  ++buaIt )
            {
               if ( hasRegion( curMunItem, (*buaIt) ) ){
                  buaGroupsOfMun.push_back( (*buaIt) );
               }
            }

            // 2) Among these BUAs is there one with a common name 
            //    with the municipal.
            list<OldItem*>::const_iterator buaIt = 
               buaGroupsOfMun.begin();
            while ( ( nameItem == NULL ) && 
                    ( buaIt != buaGroupsOfMun.end() ) )
            {
               if ( curMunItem->hasCommonLangName( (*buaIt) ) ){
                  // Found BUA, which is group to the municipal and
                  // has a common name with it.
                  nameItem = (*buaIt);
               }
               ++buaIt;
            }
         }
         i++;
      } // while items
      z++;
   } // while zoomlevels
   
   return nameItem;
}

void
OldOverviewMap::printItemsToGroup(
   map< OldItem*, list<OldItem*> > mergeLists ) const
{
   mc2dbg << "Number of groups of items: " << mergeLists.size()
          << endl;
   map< OldItem*, list<OldItem*> >::const_iterator printIt = 
      mergeLists.begin();
   while ( printIt != mergeLists.end() ){
      mc2log << info << "Items to group in overview:" << endl;
      OldItem* firstItem = printIt->first;
      mc2log << info  << "   ItemID: " << firstItem->getID() 
             << " Best name: " 
             <<  this->getBestItemName( firstItem->getID() ) << endl;

      // Print groups
      OldItem* groupPrintItem = firstItem;
      if ( groupPrintItem->getNbrGroups() == 0 ) {
         mc2dbg << "   No groups for item:" << groupPrintItem->getID() 
                << endl;
      } else {
         mc2dbg << "   Groups of item:" << groupPrintItem->getID() 
                << " Best name:" 
                << this->getBestItemName( groupPrintItem->getID() ) 
                << endl;
         for ( uint32 gIdx = 0; 
               gIdx < groupPrintItem->getNbrGroups(); gIdx++){
            uint32 gID = groupPrintItem->getGroup( gIdx );
            OldItem* groupItem = this->itemLookup( gID );
            if ( groupItem == NULL){
               mc2dbg  << "      Group with ID:" << gID << " is NULL." 
                       << endl;
            }
            else {
               mc2dbg  << "      group: " << groupItem->getID() 
                       << " Best name: " 
                       <<  this->getBestItemName( groupItem->getID() ) 
                       << endl;
            }
         }
      }

      list<OldItem*> items = printIt->second;
      list<OldItem*>::const_iterator otherItemsIt = items.begin();
      while ( otherItemsIt != items.end() ){
         OldItem* otherItem = *otherItemsIt;
         mc2log << info << "   ItemID: " << otherItem->getID() 
                << " Best name: " 
                << this->getBestItemName( otherItem->getID() ) << endl;

         // Print groups
         OldItem* groupPrintItem = otherItem;
         if ( groupPrintItem->getNbrGroups() == 0 ) {
            mc2dbg << "   No groups for item:" << groupPrintItem->getID() 
                   << endl;
         } else {
            mc2dbg << "   Groups of item:" << groupPrintItem->getID() 
                   << " Best name:" 
                   << this->getBestItemName( groupPrintItem->getID() ) 
                   << endl;
            for ( uint32 gIdx = 0; 
                  gIdx < groupPrintItem->getNbrGroups(); gIdx++){
               uint32 gID = groupPrintItem->getGroup( gIdx );
               OldItem* groupItem = this->itemLookup( gID );
               if ( groupItem == NULL){
                  mc2dbg  << "      Group with ID:" << gID << " is NULL." 
                          << endl;
               }
               else {
                  mc2dbg  << "      group: " << groupItem->getID() 
                          << " Best name: " 
                          <<  this->getBestItemName( groupItem->getID() ) 
                          << endl;
               }
            }
         }
         
         ++otherItemsIt;
      }
      ++printIt;
   }
}

OldItem*
OldOverviewMap::getGroupWithCommonLangName( OldItem* item, 
                                         ItemTypes::itemType 
                                         itemType ) const
{
   OldItem* commonNameGroup = NULL;
   uint32 g = 0; // Group offset in item.
   while ( (commonNameGroup == NULL) && (g<item->getNbrGroups()) ){
      uint32 groupItemID = item->getGroup(g);
      OldItem* groupItem = itemLookup(groupItemID);
      if ( groupItem != NULL ){
         if ( groupItem->getItemType() == itemType ){
            if ( groupItem->hasCommonLangName( item ) ){
               commonNameGroup = groupItem;
            }
         }
      } else {
         mc2log << warn << here << "Group item ID:" << groupItemID 
                << " of item ID:" << item->getID() 
                << " not present in the map." << endl;
      }
      g++;
   }
   return commonNameGroup;
}


vector <OldItem*>
OldOverviewMap::nextItemsSameIAOrderInSameNameGroup( 
      const set<OldItem*>& items, 
      set<OldItem*>::const_iterator& startAtIt,
      const OldItem* compareItem ) const
{
   vector<OldItem*> result;
   if ( compareItem == NULL ) {
      mc2log << error 
             << "OldOverviewMap::nextItemsSameIAOrderInSameNameGroup. "
             << "Got null item to compare with." << endl;
      return result;
   }

   // Get index area order and the group of compareItem 
   uint32 compareIndexAreaOrder = getIndexAreaOrder(compareItem->getID());
   uint32 compareNbrGroups = compareItem->getNbrGroups();
   mc2dbg8 << "nextItemsSameIAOrderInSameNameGroup for "
           << compareItem->getID() << " compareNbrGroups="
           << compareNbrGroups 
           << " iaOrder=" << compareIndexAreaOrder 
           << " items.size=" << items.size() << endl;
   
   //MC2_ASSERT(compareNbrGroups <= 1); 
   // Does not work to assert, the mergeIndexAreas()-fkn will handle
   // all cases. But print warn.info, and print the groups of the item
   if ( compareNbrGroups > 1 ) {
      mc2log << warn << "nextItemsSameIAOrderInSameNameGroup for " 
           << compareItem->getID() 
           << " (" << getBestItemName(compareItem->getID()) << ")"
           << " compareNbrGroups=" 
           << compareNbrGroups 
           << " iaOrder=" << compareIndexAreaOrder 
           << " items.size=" << items.size() << endl;
      for ( uint32 g = 0; g < compareNbrGroups; g++ ) {
         OldItem* compareGroupItem = itemLookup( compareItem->getGroup(g) );
         MC2_ASSERT(compareGroupItem != NULL);
         mc2dbg1 << " group " << g << " " << compareGroupItem->getID()
                 << " iaOrder=" << getIndexAreaOrder( compareGroupItem->getID() )
                 << " nbrNames=" << int(compareGroupItem->getNbrNames())
                 << " " << getBestItemName( compareGroupItem->getID() ) << endl;
      }
   }
   DEBUG8(
   for ( uint32 g = 0; g < compareNbrGroups; g++ ) {
      OldItem* compareGroupItem = itemLookup( compareItem->getGroup(g) );
      MC2_ASSERT(compareGroupItem != NULL);
      mc2dbg8 << " group " << g << " " << compareGroupItem->getID()
              << " iaOrder=" << getIndexAreaOrder( compareGroupItem->getID() )
              << " nbrNames=" << int(compareGroupItem->getNbrNames())
              << " " << getBestItemName( compareGroupItem->getID() ) << endl;
   });

   // Check if the item has same index area order as compare item
   // Check if their group(s) have the same 
   // - index area order as the group(s) of compareItem
   // - names as the group(s) of compareItem
   // If the compare item had no group, look for other items with no group
   for ( set<OldItem*>::const_iterator it = startAtIt;
         it != items.end(); it++ ) {

      mc2dbg8 << "check item " << (*it)->getID()
              << " iaOrder=" << getIndexAreaOrder((*it)->getID()) << endl;

      if ( mergeIndexAreas(compareItem, (*it)) ) {
         mc2dbg8 << " ADD from mergeIndexAreas" << endl;
         result.push_back(*it);
      }
      
   }
   
   return result;
} // nextItemsSameIAOrderInSameNameGroup

vector <OldItem*>
OldOverviewMap::nextItemsClose( const set<OldItem*>& items, 
                             set<OldItem*>::const_iterator& startAtIt,
                             const OldItem* compareItem,
                             uint32 maxDist ) const
{
   vector<OldItem*> result;
   uint64 sqMaxDist = maxDist * maxDist;

   if ( compareItem == NULL ) {
      mc2log << error << "OldOverviewMap::nextItemsClose. Got null item to "
             << "compare with." << endl;
      return result;
   }

   GfxData* cmpGfx = compareItem->getGfxData();
   if ( cmpGfx != NULL){

      set<OldItem*>::const_iterator it = startAtIt;
      while ( it != items.end() ){
         GfxData* currGfx = (*it)->getGfxData();
         
         if ( currGfx != NULL ){
            uint64 sqDist = cmpGfx->minSquareDistTo( currGfx );
            if ( sqDist <= sqMaxDist ){
               mc2dbg8 << "Added item ID: " << (*it)->getID() 
                      << " to merge vector candidates. Dist=" << sqDist
                      << endl;
               result.push_back(*it);
            } 
            else {
               mc2dbg8 << "Did not add item ID: " << (*it)->getID() 
                      << " to merge vector candidates. Dist=" << sqDist
                      << endl;
            }
         }
         else {
            mc2log << error << "OldOverviewMap::nextItemsClose " 
                   << "Cannot compare distance between items with no "
                   << "gfx data. ItemID: " << (*it)->getID() 
                   << " " << getBestItemName( (*it)->getID() ) << endl;
         }
         ++it;
      }
   }
   else {
      mc2log << error << "OldOverviewMap::nextItemsClose " 
             << "Cannot compare distance between items with no "
             << "gfx data. CompareItemID: " << compareItem->getID()
             << " " << getBestItemName( compareItem->getID() ) << endl;
   }
   return result;
} //nextItemsClose


void
OldOverviewMap::moveToMergeList( const vector<OldItem*>& itemsToAdd, 
                                list<OldItem*>* mergeList,
                                set<OldItem*>* processedItems) const
{

   vector<OldItem*>::const_iterator addItemIt =
      itemsToAdd.begin();
   while ( addItemIt != itemsToAdd.end() ){
      
      if ( find( mergeList->begin(), mergeList->end(), *addItemIt ) == 
           mergeList->end() )
      {
         // Add items to merge vector 
         mergeList->push_back(*addItemIt);
         
         processedItems->insert(*addItemIt);
         
         mc2dbg8 << "Moved item ID: " << (*addItemIt)->getID() 
                << " to merge vector." << endl;
         
      }
      else {
         mc2dbg8 << "Did not move item ID: " << (*addItemIt)->getID() 
                << " to merge vector." << endl;
      }
      
      ++addItemIt;
   }
} // moveToMergeVector

uint32
OldOverviewMap::addOneItem( DataBuffer* dataBuffer,
                         OldItem* curItem,
                         uint32 currentZoom,
                         Vector& changedItems, 
                         Vector& changedItemsOriginalID,
                         OldGenericMap* theMap )
{
   dataBuffer->reset();
   if (! saveOneItem( dataBuffer, curItem )) {
      return (MAX_UINT32);
   }
   dataBuffer->reset();

   uint32 origItemID = curItem->getID();
   uint32 newItemID = addItemToDatabase(dataBuffer, currentZoom, true);
  
   if (newItemID < MAX_UINT32) {
      // Add to the local Vector with added items
      // There are callers that depend on the fact that the new item
      // id is added last in changedItems. Don't change this bahaviour
      // without handling it.
      changedItems.addLast(newItemID);
      changedItemsOriginalID.addLast(origItemID);

      // Add to the lookuptable
      addToLookupTable(theMap->getMapID(), origItemID, newItemID);

      // Add the names to the itemnames and change StringIndex
      OldItem* newItem = itemLookup(newItemID);
      
      // Names
      // remove all and add all from curItem to get correct strIx
      updateNames(newItem, curItem, theMap);

      // Remove some memberships of groups (only keep bua and mun groups
      // belonging to bua or mun items).
      removeUnwantedGroups(newItem, theMap);
      
      // Add groups for point of interests.
      addGroupsToPoi(newItem, curItem, theMap);
      
      mc2dbg4 << "Member of " << uint32(newItem->getNbrGroups()) 
              << " groups" << endl;
      OldGroupItem* group = dynamic_cast<OldGroupItem*>(newItem);
      if (group != NULL) {
         // Remove all items in this group!
         uint32 nbrItemsInGroups = group->getNbrItemsInGroup();
         for (uint32 i=0; i<nbrItemsInGroups; ++i ) {
            mc2dbg4 << "Removing item from group" << endl;
            group->removeItemNumber( group->getNbrItemsInGroup() - 1 );
         }
         mc2dbg4 << "Nbr items in group:"
                 << uint32(group->getNbrItemsInGroup()) << endl;
      }

      if ( theMap->isIndexArea(origItemID) ) {
         setIndexAreaOrder(newItemID, theMap->getIndexAreaOrder(origItemID));
         mc2dbg8 << "Added IA bua order="
                 << theMap->getIndexAreaOrder(origItemID)
                 << " origItemID=" 
                 << theMap->getMapID() << ";" << origItemID
                 << " newItemID=" << newItemID 
                 << " " << theMap->getFirstItemName(origItemID) << endl;
      }
    
      // Special for POI:s.
      OldPointOfInterestItem* curPoi = 
         dynamic_cast<OldPointOfInterestItem*> ( curItem );
      if ( curPoi != NULL ) {
         OldPointOfInterestItem* newPoi = 
            static_cast<OldPointOfInterestItem*> ( newItem );
         if ( curPoi->getGfxData() == NULL ) {
            // If no gfxdata present, create a gfxdata from the associated
            // streetsegment.
            
            MC2Coordinate coord;
            theMap->getOneGoodCoordinate( coord, curPoi );
            GfxDataFull* gfx = GMSGfxData::createNewGfxData( this,
                                                             true ); // new polygon
            gfx->addCoordinate( coord.lat, coord.lon );
            gfx->setClosed( 0, false );
            newPoi->setGfxData( gfx );
         }

         // Disable the associated streetsegment item.
         newPoi->setStreetSegmentItemID( MAX_UINT32 );
      }

      mc2dbg4 << "OldItem with ID = " << curItem->getID() 
              << " added to the database" << endl;

   } else {
      mc2dbg1 << "FAILED to add item with ID = " 
              << curItem->getID() << endl;
      return MAX_UINT32;
   }

   return (newItemID);
 
}


bool
OldOverviewMap::ssiConnectedToOverviewRoundabout(OldGenericMap* theMap, 
                                const int maxZoomLevelStreetSegmentItems,
                                OldStreetSegmentItem* ssi)
{
   bool retVal = false;
  
   // Check both nodes.
   for (byte i = 0; i < 2; i++) {
      OldNode* node = ssi->getNode(i);
      // Check all connections to the node.
      for (uint32 j = 0; j < node->getNbrConnections(); j++) {
         OldConnection* conn = node->getEntryConnection(j);
         OldStreetSegmentItem* connectedSSI = dynamic_cast<OldStreetSegmentItem*> 
            (theMap->itemLookup(conn->getConnectFromNode()));
         if ( (connectedSSI != NULL) && 
              (connectedSSI->isRoundabout() ||
               connectedSSI->isRoundaboutish()) &&
              ( int(connectedSSI->getID() >> 27) 
                 <= maxZoomLevelStreetSegmentItems) ) {
            // The ssi is connected to a roundabout or roundaboutish
            // that is added to the overview map. 
            // Add this ssi also then!
            mc2dbg4 << "   Added roundabout(ish) exit segment." << endl;
            retVal = true;
         }
      }
   }

   return (retVal);
}

bool
OldOverviewMap::toIncludeItem(OldItem* origItem, OldGenericMap* origMap,
                           byte mapLevel)
{
   bool retVal = false;

   ItemTypes::itemType origItemType = 
         ItemTypes::itemType(origItem->getItemType());

   // Built up areas.
   if ( ( mapLevel == 1 ) &&
        ( (origItemType == ItemTypes::builtUpAreaItem) ||
          (origItemType == ItemTypes::municipalItem) ) ){
      if ( origMap->itemUsedAsGroup(origItem) ){
         // Only add built up areas and municipals, 
         // which are used as groups.
         retVal = true;

         // will e.g. skip buas and municipals that are not in search index
         // when using index areas
      }
   }

   if ( ( (origItemType == ItemTypes::streetSegmentItem) &&
          ( (int(origItem->getID() >> 27) 
             <= OldOverviewMap::maxZoomLevelForMapLevel[ mapLevel ]) ||
            ( ssiConnectedToOverviewRoundabout(origMap, 
                     OldOverviewMap::maxZoomLevelForMapLevel[ mapLevel ],
                     static_cast<OldStreetSegmentItem*> (origItem) ) 
              ) 
            )
          ) || 
        (origItemType == ItemTypes::ferryItem) ||
        ( ( mapLevel == 1 ) &&
          ( (origItemType == ItemTypes::zipCodeItem) ||   
            origMap->isUniqueCityCentre( origItem ) )
   
          )
        ) 
   {
      retVal = true;
   }
   
   return retVal;

}

bool
OldOverviewMap::changeNodeID( OldNode* node, 
                           uint32 origMapID, 
                           uint32 overviewItemID)
{
   // Make sure the inparameters are OK
   if ((node == NULL) || (origMapID == MAX_UINT32))
      return (false);

   // Set the ID of the node
   if (node->isNode0())
      node->setNodeID(overviewItemID);
   else
      node->setNodeID(overviewItemID | 0x80000000);

   // Set the ID of all the connections and remove those that
   // don't excist
   OldConnection* curCon;
   uint32 oldNodeID, newNodeID;
   uint32 i=0; 
   while (i<node->getNbrConnections()) {
      curCon = node->getEntryConnection(i);
      oldNodeID = curCon->getFromNode();
      newNodeID = reverseLookupNodeID(origMapID, oldNodeID);
      if (newNodeID < MAX_UINT32) {
         // The connection is present in this map, update the 
         // connection ID and increase the loop variable
         curCon->setFromNode(newNodeID);
         i++;
      } else {
         // The connection is _not_ present in this map, remove the 
         // connection ID but do not change the loop variable!

         mc2dbg8 << "Deleted connection " << i << " from node ID: " 
                 << node->getNodeID() << endl;
         node->deleteConnection(i, *this);
      }
   }

   return (true);
}


uint32
OldOverviewMap::reverseLookupNodeID(uint32 mapID, uint32 nodeID) const
{
   uint32 itemID =
      m_idTranslationTable.translateToHigher(mapID, nodeID & 0x7fffffff);
   uint32 nodePattern = nodeID & 0x80000000;
   return (itemID | nodePattern);
}

uint32
OldOverviewMap::reverseLookup(uint32 mapID, uint32 itemID) const
{
   return m_idTranslationTable.translateToHigher(mapID, itemID);
}

bool
OldOverviewMap::lookupNodeID(uint32 overviewNodeID,
                          uint32& mapID,
                          uint32& nodeID) const
{

   IDPair_t fullID =
      m_idTranslationTable.translateToLower(overviewNodeID&0x7fffffff);
   
   if (fullID.getMapID() != MAX_UINT32) {
      mapID = fullID.getMapID();
      uint32 nodePattern = overviewNodeID & 0x80000000;
      nodeID = fullID.getItemID() | nodePattern;
      return true;
   } else {
      mapID = MAX_UINT32;
      nodeID = MAX_UINT32;
      return false;
   }
}

IDPair_t
OldOverviewMap::lookup( uint32 overviewID ) const
{
   return m_idTranslationTable.translateToLower(overviewID);
}

void
OldOverviewMap::updateExternalConnectionsFromMap(map<uint32, MC2String>&
                                                 lowerMapNameByID,
                                                 OldGenericMap* otherMap, 
                                                 byte mapLevel)
{
   // Check inparameter
   if (otherMap == NULL) {
      return;
   }

   // Add the external connections of the given map to the nodes in
   // this overviewmap
   OldBoundrySegmentsVector* externalConns = otherMap->getBoundrySegments();
   if (externalConns == NULL) {
      return;
   }

   if (m_segmentsOnTheBoundry == NULL) {
      m_segmentsOnTheBoundry = new OldBoundrySegmentsVector();
   }

   m_idTranslationTable.sortElements();

   for (uint32 i=0; i<externalConns->getSize(); i++) {
      OldBoundrySegment* curSeg = (OldBoundrySegment*)
            externalConns->getElementAt(i);

      if (curSeg != NULL) {
         // The ID was found in the boundry segments array in the 
         // other map
         mc2dbg8 << "   To add external connections to: " 
                 << curSeg->getRouteableItemID() << " " 
                 << curSeg->getNbrConnectionsToNode(0)
                 << " to node 0 and "
                 << curSeg->getNbrConnectionsToNode(1)
                 << " to node 1" << endl;
         OldConnection* curCon;
         uint32 curConID, overviewID;

         // The corresponding OldStreetSegmentItem in the overview map
         uint32 trueID = reverseLookup(otherMap->getMapID(), 
                             curSeg->getConnectRouteableItemID());
         OldRouteableItem* newItem =  NULL;
         if (trueID != MAX_UINT32) {
            newItem = static_cast<OldRouteableItem*>(itemLookup(trueID));

            bool hasConnectionsToOtherCountry = false;
            
            // Connections to node 0
            for (uint32 j=0; 
                 j<curSeg->getNbrConnectionsToNode(0); j++) {
               curCon = curSeg->getConnectToNode0(j);
               curConID = curCon->getConnectFromNode();
               uint32 curFromMapID = curSeg->getFromMapIDToNode(0, j);

                  
               overviewID = reverseLookupNodeID(curFromMapID, curConID);
               
               if (overviewID == MAX_UINT32) {
                  // This indicates that item to connect to is not 
                  // included in this map.
                  mc2dbg8 << "overviewID == MAX_UINT32" << endl;
                  if ( lowerMapNameByID.find(curFromMapID) 
                       != lowerMapNameByID.end() ){
                     // An item not added to this map because it was not
                     // important enough, do nothing
                  }
                  else {
                     // This item possibly exists in another overview map.
                     hasConnectionsToOtherCountry = true;
                  }
               } else {
                  // add conn, transfer the turn description from curConn
                  OldConnection* newConn = new OldConnection(overviewID);
                  newConn->setTurnDirection( curCon->getTurnDirection());
                  if (newItem->getNode(0)->addConnection(
                                             newConn, *this ) ) {
                     mc2dbg8 << "   Added connection to "
                             << newItem->getID() << " = 0x" << hex
                             << newItem->getID() << dec << " node 0 from " 
                             << overviewID << " = 0x" << hex
                             << overviewID << dec << endl;
                  } else {
                     mc2dbg8 << "   Connection to "
                             << newItem->getID() << " = 0x" << hex
                             << newItem->getID() << dec << " node 0 from " 
                             << overviewID << " = 0x" << hex
                             << overviewID << dec << " ALREADY ADDED" 
                             << endl;
                     delete newConn;
                  }
               }
            }
            
            // Connections to node 1
            for (uint32 j=0; 
                 j<curSeg->getNbrConnectionsToNode(1); j++) {
               curCon = curSeg->getConnectToNode1(j);
               curConID = curCon->getConnectFromNode();
               uint32 curFromMapID = curSeg->getFromMapIDToNode(1, j);

               overviewID = reverseLookupNodeID(curFromMapID, curConID);
               if (overviewID == MAX_UINT32) {
                  // This indicates that item to connect to is not 
                  // included in this map.
                  mc2dbg8 << "overviewID == MAX_UINT32" << endl;
                  if ( lowerMapNameByID.find(curFromMapID) 
                       != lowerMapNameByID.end() ){
                     // An item not added to this map because it was not
                     // important enough, do nothing
                  }
                  else {
                     // This item possibly exists in another overview map.
                     hasConnectionsToOtherCountry = true;
                  }
               } else {
                  // add conn, transfer the turn description from curConn
                  OldConnection* newConn = new OldConnection(overviewID);
                  newConn->setTurnDirection( curCon->getTurnDirection());
                  if (newItem->getNode(1)->addConnection(
                                                         newConn, *this) ) {
                     mc2dbg8 << "   Added connection to "
                             << newItem->getID() << " = 0x" << hex
                             << newItem->getID() << dec 
                             << " node 1 from " 
                             << overviewID << " = 0x" << hex
                             << overviewID << dec << endl;
                  } else {
                     mc2dbg8 << "   Connection to "
                             << newItem->getID() << " = 0x" << hex
                             << newItem->getID() << dec 
                             << " node 1 from " 
                             << overviewID << " = 0x" << hex
                             << overviewID << dec << "ALREADY ADDED" 
                             << endl;
                     delete newConn;
                  }
               }
            }
            
            // Add this boundary segment if either it has connections
            // to another country, or if it doesn't have any connections
            // at all.
            if (hasConnectionsToOtherCountry || 
                ( (curSeg->getNbrConnectionsToNode(0) == 0) &&
                  (curSeg->getNbrConnectionsToNode(1) == 0) )) {
               // Add boundary segment to the overview map.
               // Find the corresponding item id.
               mc2dbg8 << here << " Adding new boundary segment to "
                  << "overview map 0x" << hex << getMapID() << dec
                  << ", item id = " << newItem->getID() 
                  << ", underview = (" << otherMap->getMapID() << ", " 
                  << curSeg->getConnectRouteableItemID() << ")" << endl;
                                                            
               m_segmentsOnTheBoundry->addBoundrySegment(
                                        newItem->getID(), 
                                        curSeg->getCloseNodeValue());
            }
            
         }
      }
   }
   
   m_segmentsOnTheBoundry->sort();
   
}

void
OldOverviewMap::createZipCodeAgglomerations()
{
   // Create zip code agglomerations.
   mc2dbg1 << "OldOverviewMap::createZipCodeAgglomerations" << endl;
   
   uint32 n = this->createZipMergeGroups();
   mc2dbg << "Added " << n << " new OldZipCodeItem-groups" << endl;


   mc2log << info << "Creating zip overviews." << endl;
   // Test how to create zip code overveiws.
   NationalProperties::zipCodeType_t zipCodeType = 
      NationalProperties::getZipCodeType(this->getCountryCode());
   
   switch (zipCodeType){
      case NationalProperties::symmetricNumberZipCodeType: {
         uint32 maxZipCodeLength = 7;
         uint32 minZipCodeLength = 4;
         
         if ( zipCodeType == NationalProperties::symmetricNumberZipCodeType && 
              this->itemNameMaxLength ( ItemTypes::zipCodeItem ) > maxZipCodeLength ) {
            mc2log << error << "Symmetric zip code item name "
                   << "with length " << this->itemNameMaxLength ( ItemTypes::zipCodeItem )
                   << "- need to adjust the rule here!"
                   << endl;
	    maxZipCodeLength = this->itemNameMaxLength ( ItemTypes::zipCodeItem );
         }

         for ( uint32 i = maxZipCodeLength;
               i >= minZipCodeLength; i-- ) {
            n = this->createZipOverviews(i, zipCodeType);
            mc2dbg << "Added " << n << " new zipOverviews with "
                   << i-1 << " letters" << endl;
         }
      } break;
      case NationalProperties::ukZipCodeType:{
         // UK complete zips have 4 levels:
         // - postal area
         // - postal district
         // - sector
         // - full zip location
         n = this->createZipOverviews(4, zipCodeType);
         mc2dbg << "Added " << n << " new UK zip ovr with 3 groups" 
                << endl;
         n = this->createZipOverviews(3, zipCodeType);
         mc2dbg << "Added " << n << " new UK zip ovr with 2 groups"
                << endl;
         
      } break;
      case NationalProperties::numberNameZipCodeType:{
         // Zip code type consisting of a name followed
         // by a number. For example used in Ireland
         // Example: "Dublin 12"
         // 
         // For this type, do nothing
         mc2dbg << "No zipOvervews added (numberNameZipCodeType) "
                << endl;

      } break;
      default: {
         // Unkown zip code type.
         // moment.
         mc2log << error  << "Unknown zip code type while "
                << "adding zipOverviews." << endl;
         MC2_ASSERT(false);
      } break;
   } // switch
}

uint32
OldOverviewMap::createZipMergeGroups()
{
   mc2dbg << "OldOverviewMap::createZipMergeGroups" << endl;

   ItemTypes::itemType type = ItemTypes::zipCodeItem;

   uint32 nbrAddedGroups = 0;
   
   // Get all items of current type. Stored in a separate datastructure to
   // make it easier to add the new group items...
   typedef multimap<uint32, OldItem*> itemByStrIdx_t;
   itemByStrIdx_t itemByStrIdx;
   uint32 nbrItems = 0;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); ++i) {
         OldItem* curItem = getItem(z, i);
         if ( (curItem != NULL) && (curItem->getItemType() == type) ){
            nbrItems++;
            for (byte n=0; n<curItem->getNbrNames(); n++){
               itemByStrIdx.insert(make_pair( curItem->getStringIndex(n),
                                              curItem ));
            }
         }
      }
   }
   mc2dbg << "   Collected " << nbrItems << " items" << endl;
   mc2dbg << "   itemByStrIdx size: " << itemByStrIdx.size() << endl;
   typedef  vector<OldItem*>::const_iterator itemIt_t;
   vector<OldItem*> concat;

   itemByStrIdx_t::const_iterator strIdxIt = itemByStrIdx.begin();
   while ( strIdxIt != itemByStrIdx.end() ){
      itemByStrIdx_t::const_iterator endIt =
         itemByStrIdx.upper_bound(strIdxIt->first);      
      while(strIdxIt != endIt){
         OldItem* item = strIdxIt->second;

         // Check groups.
         if (item->getNbrGroups() != 0 ){
            mc2dbg << error << here << " Not handling items with groups" 
                   << endl;
            exit(1);
         }
         
         concat.push_back(item);
         ++strIdxIt;
      }

      // Anything to concatinate?
      if (concat.size() > 1) {
         mc2dbg << "         Collected " << concat.size() << " items" << endl;
         OldGroupItem* newGroupItem = NULL;
         bool addBBoxAsGfx = false;
         switch (type) {
         case ItemTypes::zipCodeItem : 
            newGroupItem = new OldZipCodeItem(MAX_UINT32);
            addBBoxAsGfx = true;
            break;
         default :
            mc2log << fatal << here << " Unhandled itemType; "
                   << uint32(type) << endl;
            MC2_ASSERT(false);
         }
         
         // Add bbox of the items to the new groupitem?
         if ( addBBoxAsGfx ) {
            MC2BoundingBox bbox;
            for (itemIt_t it2=concat.begin(); it2!=concat.end(); ++it2) {
               if ( (*it2)->getGfxData() != NULL ) {
                  MC2BoundingBox curBBox;
                  (*it2)->getGfxData()->getMC2BoundingBox( curBBox );
                  bbox.update( curBBox );
               }
            }
            if ( bbox.isValid() ) {
               newGroupItem->setGfxData( GMSGfxData::createNewGfxData( this, 
                                                                    &bbox ) );
            }
         }
         
         ++nbrAddedGroups;
         uint32 newID = addItem(newGroupItem, 
                                GET_ZOOMLEVEL(concat[0]->getID()));
         mc2dbg << "Adding group " << getFirstItemName(concat[0]) << ", id="
                << newGroupItem->getID() << "=" << newID << " at zoomLevel "
                << GET_ZOOMLEVEL(concat[0]->getID()) << "=" 
                << GET_ZOOMLEVEL(newID) << endl;
         addNameToItem(newGroupItem, 
                       getFirstItemName(concat[0]), 
                       concat[0]->getNameLanguage(0), 
                       concat[0]->getNameType(0));
         for (itemIt_t it2=concat.begin(); it2!=concat.end(); ++it2) {
            bindItemToGroup(*it2, newGroupItem);
            mc2dbg << "   Adding item " << getFirstItemName(*it2) << ", id="
                   << (*it2)->getID() << " to group "
                   << getFirstItemName(newGroupItem) << ", id=" 
                   << newGroupItem->getID() << endl;
         }
      } 
      concat.clear();
      
   } // outer while
   return nbrAddedGroups;
}

uint32
OldOverviewMap::createZipOverviews(uint32 nbrCharacters, 
                                NationalProperties::zipCodeType_t zipCodeType) 
{
   // Collect all items with name containing nbrCharacters chars
   const uint32 z = 10;
   vector<OldItem*> items;
   for (uint32 i=0; i<getNbrItemsWithZoom(z); ++i) {
      OldItem* curItem = getItem(z, i);
      if ( (curItem != NULL) && 
           (curItem->getItemType() == ItemTypes::zipCodeItem)) {
         if (curItem->getNbrNames() != 1){
            mc2log << error << "Zip item not having 1 name ID: " 
                   << curItem->getID() << endl;
            MC2_ASSERT(false);
         }
         if (zipCodeType == 
             NationalProperties::symmetricNumberZipCodeType){
            if (strlen(getName(curItem->getStringIndex(0))) == 
                nbrCharacters) {
               items.push_back(curItem);
            }
         }
         else if (zipCodeType == 
                  NationalProperties::ukZipCodeType){
            if ( StringSearchUtility::getUKPostalCodeNbrGroups(
                  getName(curItem->getStringIndex(0))) == nbrCharacters ){
               items.push_back(curItem);
            }
         }
         else {
            mc2log << error << "Unknown zipCodeType " << int(zipCodeType)
                   << endl;
            MC2_ASSERT(false);
         }
      }
   }
   mc2dbg << "Found " << items.size() << " items with " << nbrCharacters 
          << "-sized names" << endl;

   // Got all items with the correct length of their names in items-array
   uint32 nbrNewGroups = 0;
   while (items.size() > 0) {
      OldItem* curItem = items.back();
      items.pop_back();

      const uint32 n= nbrCharacters-1;
      MC2String newName(getName(curItem->getStringIndex(0)));
      if (zipCodeType == 
          NationalProperties::symmetricNumberZipCodeType){
         newName[n] = '\0';
      }
      else if (zipCodeType == 
               NationalProperties::ukZipCodeType){
         newName = StringSearchUtility::removeUKPostalCodeGroup(newName);
      }
      else {
         mc2log << error << "Unknown zipCodeType " << int(zipCodeType)
                   << endl;
         MC2_ASSERT(false);
      }

      // Create or find the zip code to use as the parent.
      set<OldItem*> sameNameItems;
      bool reuseZips = false; // Set to true if to reuse zips with same name.
      if (reuseZips){
         getItemsWithName(newName.c_str(), 
                          sameNameItems, 
                          ItemTypes::zipCodeItem);
      }
      OldGroupItem* newGroupItem = NULL;
      if ( sameNameItems.size() == 0){
         newGroupItem = new OldZipCodeItem(MAX_UINT32);
         uint32 newGroupItemID = 
            addItem(newGroupItem, GET_ZOOMLEVEL(curItem->getID()));
         addNameToItem(newGroupItem, newName.c_str(),
                       curItem->getNameLanguage(0), curItem->getNameType(0));
         ++nbrNewGroups;
         mc2dbg << "Adding new group " << newGroupItemID
                << " with name: " << newName << endl;
      }
      else if (sameNameItems.size() == 1){
         newGroupItem = 
            dynamic_cast<OldGroupItem*>(*(sameNameItems.begin()));
         MC2_ASSERT(newGroupItem != NULL);
         mc2dbg << "Using group with name: " << newName << " ID: " 
                << newGroupItem->getID() << endl;
      }
      else {
         newGroupItem = dynamic_cast<OldGroupItem*>
            (getTopGroup(sameNameItems));
         if (newGroupItem == NULL){
            mc2log << error << "Found more than one zip item with the wanted "
                   << " name, not in the same hierarchy, when fixing zip "
                   << " overview hierarchy. Don't know"
                   << " what to do, exits." << endl;
            set<OldItem*>::const_iterator it=sameNameItems.begin();
            while ( it != sameNameItems.end()){
               mc2log << "Item ID: " << (*it)->getID() << endl;
               ++it;
            }
            save();
            MC2_ASSERT(false);
         }
         mc2dbg << "Using existing top group with name: " << newName 
                << " ID: " << newGroupItem->getID() << endl;
      }

      
      // Put the zips in the parent zip.
      MC2BoundingBox bbox;
      if (curItem->getGfxData() != NULL){
         MC2BoundingBox tmpBBox;
         curItem->getGfxData()->getMC2BoundingBox(tmpBBox);
         bbox.update(tmpBBox);
      }

      if (bindItemToGroup(curItem, newGroupItem)){
         mc2dbg << "Binding item ID: " << curItem->getID()
                << "\" " << getName(curItem->getStringIndex(0))
                << "\" to group " << newGroupItem->getID() 
                << "\" " << newName << "\" (A)" << endl;
      }
      else {
         mc2log << error << "Failed to bind groups: " 
                << curItem->getID()
                << "\" " << getName(curItem->getStringIndex(0))
                << "\" to group " << newGroupItem->getID() 
                << "\" " << newName << "\"" << endl;
         exit(1);
      }

      uint32 i = 0;
      while (i < items.size()) {
         MC2String cmpName = getName(items[i]->getStringIndex(0));
         if ( (zipCodeType == 
               NationalProperties::symmetricNumberZipCodeType) ) {
            cmpName = cmpName.substr(0, n);
         }
         else if (zipCodeType ==
                  NationalProperties::ukZipCodeType){
            cmpName = 
               StringSearchUtility::removeUKPostalCodeGroup(cmpName);
         }
         else {
            mc2log << error << "Unknown zipCodeType " << int(zipCodeType)
                   << endl;
            MC2_ASSERT(false);
         }         

         if (strcmp(newName.c_str(), 
                    cmpName.c_str()) == 0)
         {
            // The n first characters match! Add to newGroupItem and remove
            // from items
            if (bindItemToGroup(items[i], newGroupItem) ){
               mc2dbg << "Binding item ID: " << items[i]->getID() << " \"" 
                      << getName(items[i]->getStringIndex(0))
                      << "\"to group ID: " << newGroupItem->getID() 
                      << " \"" 
                      << newName << "\" (B)" << endl;
            }
            else {
               mc2dbg << "Failed binding item ID: " << items[i]->getID() 
                      << " \"" 
                      << getName(items[i]->getStringIndex(0))
                      << "\"to group ID: " << newGroupItem->getID() 
                      << " \"" 
                      << newName << "\"" << endl;
               exit(1);
            }
            if (items[i]->getGfxData() != NULL) {
               MC2BoundingBox tmpBBox;
               items[i]->getGfxData()->getMC2BoundingBox(tmpBBox);
               bbox.update(tmpBBox);
            }

            items.erase(items.begin() + i);
         } else {
            ++i;
         }
      }
      GfxDataFull* newGfxData = GMSGfxData::createNewGfxData(this, &bbox);
      newGroupItem->setGfxData(newGfxData);
   }
   return nbrNewGroups;
}  // createZipOverviews 


void
OldOverviewMap::updateNames(OldItem* newItem,
                         OldItem* otherItem, OldGenericMap* otherMap)
{
   // Update the names
   mc2dbg4 << "updating names for item " << newItem->getID()
           << " " << getFirstItemName(newItem) << endl;
   newItem->removeAllNames();
   for (byte i=0; i<otherItem->getNbrNames(); i++) {
      LangTypes::language_t curLang;
      ItemTypes::name_t curType;
      uint32 oldIndex;
      otherItem->getNameAndType(i, curLang, curType, oldIndex);
      const char* nameOfItem = otherMap->getName(oldIndex);
      if (addNameToItem(newItem, nameOfItem, curLang, curType)
            == MAX_UINT32) {
         mc2log << error << "Error adding name to database" << endl;
      }
   }
   mc2dbg4 << "updated names for item " << newItem->getID()
           << " to " << getFirstItemName(newItem) << endl;
}

void
OldOverviewMap::removeUnwantedGroups(OldItem* newItem, OldGenericMap* otherMap)
{
   // Only keep bua and mun groups belonging to bua or municipal items
   
   uint32 nbrGroups = newItem->getNbrGroups();
   uint32 idx = nbrGroups - 1;
   for ( uint32 i = 0; i < nbrGroups; ++i ) {
      bool removeGroup = true;
      // Only bua and mun groups belonging to bua:s or mun
      if ( ( newItem->getItemType() == ItemTypes::municipalItem ) ||
           ( newItem->getItemType() == ItemTypes::builtUpAreaItem ) ) {
         // Get the item type of the group.
         ItemTypes::itemType groupIT = 
            otherMap->itemLookup( newItem->getGroup( idx ) )
               ->getItemType();
         if ( ( groupIT == ItemTypes::municipalItem ) ||
              ( groupIT == ItemTypes::builtUpAreaItem ) ) {
            removeGroup = false;
         }
      }
      
      if ( removeGroup ) {
         newItem->removeGroup( idx );
      }
      --idx;
   }
}

void
OldOverviewMap::addGroupsToPoi(OldItem* newItem,
                            OldItem* otherItem, OldGenericMap* otherMap)
{
   // Add bua and mun groups to pois
   
   OldPointOfInterestItem* poi = 
      dynamic_cast<OldPointOfInterestItem*> ( otherItem );
   if ( poi != NULL ) {
      // Get the streetsegment for the poi.
      OldItem* ssi = otherMap->itemLookup( poi->getStreetSegmentItemID() );
      if ( ssi != NULL ) {

         // The region types to add.
         set<ItemTypes::itemType> regionTypes;
         regionTypes.insert( ItemTypes::municipalItem );
         regionTypes.insert( ItemTypes::builtUpAreaItem );
         // Add the regions.
         for ( set<ItemTypes::itemType>::const_iterator it =
               regionTypes.begin(); it != regionTypes.end(); ++it ) {
            for ( uint32 i = 0; 
                  i < otherMap->getNbrRegions( ssi, *it );
                  ++i ) {
               // Fixme? consider noLocationRegion (high bit) ??
               addRegionToItem( newItem, 
                     otherMap->getRegionIDIndex( ssi, i, *it ) );
            }
         }
      }
   } 
}

void
OldOverviewMap::updateConnectionsAndGroups(Vector changedItems,
                                        OldGenericMap* otherMap)
{
   uint32 otherMapID = otherMap->getMapID();

   // Translate the nodeID in all the connections from 
   // (otherMapID.nodeID) to overviewMapID. Also update group IDs.
   OldRouteableItem* ri;
   uint32 overviewItemID = 0;
   for (uint32 i=0; i<changedItems.getSize(); i++) {
      overviewItemID = changedItems.getElementAt(i);
      OldItem* item = itemLookup(overviewItemID);
      ri = dynamic_cast<OldRouteableItem*> (item);
      if (ri != NULL) {
         mc2dbg4 << "   Changing ID in connections" << endl;
         
         // Change node IDs.
         changeNodeID(ri->getNode(0), otherMapID, overviewItemID);
         changeNodeID(ri->getNode(1), otherMapID, overviewItemID);
         
         // Also update sing posts.
         for (uint32 j = 0; j < 2; j++) {
            OldNode* node = ri->getNode(j);
            for (uint32 k = 0; k < node->getNbrConnections(); k++) {
               OldConnection* conn = node->getEntryConnection(k);

               // Get all IDs.
               uint32 thisFromNodeID = conn->getConnectFromNode();
               uint32 thisToNodeID = node->getNodeID();
                  
               uint32 dummyMapID;
               uint32 otherFromNodeID;
               if (! lookupNodeID(thisFromNodeID, 
                                  dummyMapID, otherFromNodeID) ){
                  mc2log << error << "Could not find other from node ID for "
                         << "node:"
                         << thisFromNodeID << endl;
               }
               uint32 otherToNodeID;
               if (! lookupNodeID(thisToNodeID, dummyMapID, otherToNodeID) ){
                  mc2log << error << "Could not find other to "
                         << "node ID for node:"
                         << thisFromNodeID << endl;
               }

               // Copy the singposts.
               updateSignposts(thisFromNodeID, thisToNodeID,
                               otherFromNodeID, otherToNodeID,
                               otherMap);

            }
         }
      }

      // Also update the group ids.
      for ( uint32 j = 0; j < item->getNbrGroups(); ++j ) {
         item->setGroup( j, reverseLookup( otherMapID, 
                                           item->getGroup( j ) ) ); 
         // Fixme?: high bit ?
      }
   }
         
   // Fix missing opposing connections.
   for (uint32 i=0; i<changedItems.getSize(); i++) {
      overviewItemID = changedItems.getElementAt(i);
      OldItem* item = itemLookup(overviewItemID);
      ri = dynamic_cast<OldRouteableItem*> (item);
      if (ri != NULL) {
         // Fix opposing connections in overview (needed for dynamic updates)
         for ( uint32 n=0; n<2; n++ ){
            OldNode* ovrToNode = ri->getNode(n);
            for ( uint32 c=0; c<ovrToNode->getNbrConnections(); c++ ){
               OldConnection* conn = ovrToNode->getEntryConnection(c);
               OldConnection* oppConn = 
                  getOpposingConnection(conn, ovrToNode->getNodeID());
               if ( oppConn == NULL && (!conn->isMultiConnection()) ){
                  // Opposing connection missing in overview, fix this by 
                  // copying the opposing connection from the underview map.

                  // Find underview opposing connection, i.e. we want the
                  // connection going from item of toNode to the item of 
                  // fromNode. This is done by using the opposite node of 
                  // these. (connection is stored in opposing fromNode).
                  uint32 fromNodeOvrID = conn->getConnectFromNode();
                  uint32 undMapID = MAX_UINT32;
                  uint32 fromNodeUndID = MAX_UINT32;
                  bool ok = lookupNodeID(fromNodeOvrID, 
                                         undMapID, 
                                         fromNodeUndID);
                  MC2_ASSERT(ok);
                  uint32 oppFromNodeUndID = 
                     OldNode::oppositeNodeID(fromNodeUndID);
                  OldNode* undOppFromNode = 
                     otherMap->nodeLookup(oppFromNodeUndID);
                  // 
                  uint32 toNodeUndID = MAX_UINT32;
                  ok = lookupNodeID(ovrToNode->getNodeID(), 
                                    undMapID, toNodeUndID);
                  MC2_ASSERT(ok);
                  uint32 oppToNodeUndID = OldNode::oppositeNodeID(toNodeUndID);
                  // Print the situation.
                  mc2dbg8 << "Ovr node IDs, toNode: " 
                          << ovrToNode->getNodeID() 
                          << " fromNode: " << fromNodeOvrID << endl;
                  mc2dbg8 << "Und node IDs, toNode: " << toNodeUndID 
                          << " fromNode: " << fromNodeUndID << endl;
                  //
                  OldConnection* undConn =  // the connection to copy.
                     undOppFromNode->getEntryConnectionFrom(oppToNodeUndID);
                  if (undConn == NULL){
                     mc2log << error << "Failed to get underview connection."
                            << endl;
                     mc2dbg << "To node ID: " << undOppFromNode->getNodeID()
                            << " from node ID: " << oppToNodeUndID 
                            << " und map ID: " << otherMap->getMapID() << endl;
                     MC2_ASSERT(false);
                  }
                  // Create new connection in overview.
                  OldConnection* ovrConn = 
                     new OldConnection(ovrToNode->getOppositeNodeID() );
                  OldNode* ovrOppFromNode = 
                     nodeLookup(OldNode::oppositeNodeID(fromNodeOvrID)); 
                  ovrOppFromNode->addConnection( ovrConn, *this );
                  mc2dbg << "Opposing conn caused adding connection to "
                         << "overview node "
                         << ovrOppFromNode->getNodeID() << " = 0x" << hex
                         << ovrOppFromNode->getNodeID() << dec << " from " 
                         << ovrToNode->getOppositeNodeID() << " = 0x" << hex
                         << ovrToNode->getOppositeNodeID() << dec << endl;

                  // Uppdate attributes from underview.
                  if (!ovrConn->updateConnAttributesFromConn(undConn)){
                     mc2log << error << " Could not update attributes from "
                            << "connection in underview." << endl;
                     mc2dbg << " opposing ovrFromNode: " 
                            << ovrOppFromNode->getNodeID() 
                            << " undOppFromNode: " 
                            << undOppFromNode->getNodeID()
                            << endl;
                     MC2_ASSERT(false);
                  }
               }
            }
         }
      }
   }
}

void
OldOverviewMap::updateSignposts(uint32 thisFromNodeID, 
                                uint32 thisToNodeID,
                                uint32 otherFromNodeID, 
                                uint32 otherToNodeID,
                                const OldGenericMap* otherMap){

   m_signPostTable.removeSignPosts(thisFromNodeID,
                                   thisToNodeID);
   
   uint32 nbrCopied = 
      m_signPostTable.copySignPostsFromOtherMap(*this,
                                                thisFromNodeID, 
                                                thisToNodeID,
                                                *otherMap,
                                                otherFromNodeID,
                                                otherToNodeID);
   if ( nbrCopied > 0 ){
      mc2dbg8 << "Updated " << nbrCopied << " sing posts: " 
              << thisFromNodeID << ":" 
              << thisToNodeID
              << ":" << otherFromNodeID << ":" << otherToNodeID << ":" 
              << otherMap->getMapID() << endl;
   }
}

bool
OldOverviewMap::applyChangesFromOtherMap(
            OldGenericMap* otherMap, vector<uint32> itemsAffectedInOtherMap,
            byte mapLevel, bool& overviewMapChanged, 
            multimap<uint32,uint32>& changedOverviewIds)
{
   mc2dbg1 << "OMap applyChangesFromOtherMap map level " << int(mapLevel)
           << ", nbr affected items " << itemsAffectedInOtherMap.size() 
           << endl;
   overviewMapChanged = false;

   if (itemsAffectedInOtherMap.size() == 0) {
      return true;
   }

   // Check if otherid exists in the ovmap
   //    Look up otheritem in other map
   //    If otheritem is not present in other map it was removed (e.g. poi)
   //       Remove also in o map
   //    Else
   //       Update "attributes" (name, gfxdata, conn, ...)
   // If not use existing functions to determine if it should be (new poi)
   //    Use existing functions to add it to omap inlcuding groups,
   //    updating lookuptables etc.
   
   // Add any new items using a databuffer, constant from 
   // OldOverviewMap::addMap
   const uint32 MAX_ITEM_SIZE = 7000000;
   DataBuffer* dataBuffer = new DataBuffer(MAX_ITEM_SIZE);

   // A temporary Vector with ID (in this map) of all the items 
   // that have been added
   Vector newItems(512);

   // A temporary Vector with original ID of all the items that have 
   // been added.
   Vector newItemsOriginalID(512);


   uint32 otherItemNbr = 0;
   uint32 otherMapId = otherMap->getMapID();
   vector<uint32>::const_iterator it;
   for (it = itemsAffectedInOtherMap.begin();
        it != itemsAffectedInOtherMap.end(); it++) {
      
      uint32 otherItemId = (*it);
      OldItem* otherItem = otherMap->itemLookup(otherItemId);
      mc2dbg1 << "aCFOM, trying otherItem nbr " << otherItemNbr << ":" 
              << otherMapId << "." << otherItemId << endl;
      uint32 overviewId = reverseLookup(otherMapId, otherItemId);
      if (overviewId != MAX_UINT32) {
         // otherItem exists in overview map
         if ((otherItem == NULL) ||
             (otherItem->getItemType() == ItemTypes::nullItem)) {
            // otherItem has been removed from otherMap
            mc2dbg1 << "aCFOM, otherItem was removed from otherMap, " 
                    << "removing overview item " << overviewId << endl;
            removeItem(overviewId); // also removes from lookup-table
                                    // and adminAreaCentres table
            overviewMapChanged = true;
            changedOverviewIds.insert(make_pair(getMapID(), overviewId));
            
         } else {
            OldItem* overviewItem = itemLookup(overviewId);
            if (overviewItem != NULL) {
               mc2dbg1 << "aCFOM, updating attributes of overview item " 
                       << overviewId << endl;

               // Update all attributes of the overview item, including
               // names, groups, connection attributes etc.
               updateAttributes(overviewItem, otherItem, otherMap);
               
               overviewMapChanged = true;
               changedOverviewIds.insert(make_pair(getMapID(), overviewId));
            }
         }
         
      } else {
         // The otherItem does not exist in overview map,
         // check if it is a new item that should be included (poi)
         mc2dbg1 << "aCFOM, otherItem does NOT exist in overview map" << endl;
         mc2dbg2 << " otherItem itself";
         if (otherItem == NULL) {
            mc2dbg2 << " is NULL" << endl;
         } else {
            mc2dbg2 << " exists" << endl;
         }
         if ((otherItem == NULL) ||
             (otherItem->getItemType() == ItemTypes::nullItem)) {
            // The other item was perhaps removed from the other map
            // The item could else be a street item that was re-generated
            // (in EDR: removed all si and created new ones)
            //
            // In any other case ending up here means that something is wrong
            // and that the provided item id is invalid.
            mc2dbg1 << "The otherItem " << otherItemId
                    << " exists neither in ov map nor in other map" << endl;
         } 
         else if ((otherItem != NULL) &&
                  toIncludeItem(otherItem, otherMap, mapLevel)) {
            mc2dbg4 << "aCFOM, otherItem should be added to overview map" 
                    << endl;
            // find zoom from otherMap
            uint32 otherZoomLevel = (otherItemId & 0x78000000) >> 27;
            dataBuffer->reset();
            
            uint32 newOverviewId = 
               addOneItem( dataBuffer, otherItem, otherZoomLevel,
                           newItems, newItemsOriginalID, otherMap );
            if (newOverviewId != MAX_UINT32) {
               mc2dbg1 << "aCFOM, otherItem added to overview map, newId " 
                       << newOverviewId << endl;
               overviewMapChanged = true;

               changedOverviewIds.insert(
                     make_pair(getMapID(), newOverviewId));

               // groups and poi-specifics are ok.
               // groupids are underviews
               
               // After-process for all new items is done below
               //
               // Sort reverse_lookup
               // Change nodeids (function)
               // Fix signpostnames
               // Fix group-ids
            }
         }
      }
      
      otherItemNbr++;
   }

   // Do some after processing on new items
   if (newItems.size() != 0) {
      
      // Make sure the lookup and the reverse lookup table is sorted 
      // after the insertions!
      m_idTranslationTable.sortElements();
  
      // Translate the nodeID in all the connections from 
      // (mapID.nodeID) to overviewMapID. Also translate group ids.
      // Also add signpost names to itemnames and change StringIndex.
      updateConnectionsAndGroups(newItems, otherMap);
   }

   // Make sure the m_adminAreaCentres of this overview is ok.
   // Since it is not possible to detect if a removed item in the otherMap
   // was either an admin area item (mun or bua) or a city centre POI,
   // we really need to re-create admin area centres from the otherMap
   // and compare with what is stored in the table of this overview map.
   // If anything differs, the centres are replaced.
   if ( updateAdminAreaCentres( otherMap, mapLevel ) ) {
      overviewMapChanged = true;
   }
   
   delete dataBuffer;
   return true;
}

bool
OldOverviewMap::updateAttributes(OldItem* overviewItem,
      OldItem* otherItem, OldGenericMap* otherMap)
{
   bool retVal = false;
   mc2dbg4 << "OMap updateAttributes" << endl;

   // Let the overview item update itself for most attributes
   // ssi, ferry, poi etc (item attributes and node attributes)
   if (overviewItem->
         updateAttributesFromItem(otherItem, false)) { // not same map
      retVal = true;
   }

   // GfxData not handled in item for overviews.
   // At least handle pois since they could have moved in WASP
   if ((overviewItem->getItemType() == ItemTypes::pointOfInterestItem) &&
       (otherItem->getItemType() == ItemTypes::pointOfInterestItem)) {
      GfxData* myGfx = overviewItem->getGfxData();
      GfxData* otherGfx = otherItem->getGfxData();
      GfxDataFull* newGfx = 
         GMSGfxData::createNewGfxData( this, true ); // new polygon
      
      if (otherGfx == NULL) {
         // The other item had no gfxData, create it in the same 
         // way as in addOneItem-method.
         MC2Coordinate coord;
         otherMap->getOneGoodCoordinate( coord, otherItem );
         newGfx->addCoordinate( coord.lat, coord.lon );
         newGfx->setClosed( 0, false );
      }
      else {
         int32 lat = otherGfx->getLat(0,0);
         int32 lon = otherGfx->getLon(0,0);
         newGfx->addCoordinate( lat, lon );
         newGfx->setClosed( 0, false );
      }

      if ( (myGfx == NULL) ||
           ((myGfx != NULL) && !myGfx->equals(newGfx)) ) {
         overviewItem->setGfxData( newGfx );
         retVal = true;
      }
   }
   
   // Names (stringIndex)
   updateNames(overviewItem, otherItem, otherMap);
   
   // Groups
   // Updated in OldItem by copying all groups from otherItem. NB underview ids.
   // Remove groups that should not be present in o-map
   // (keep bua and mun groups belonging to bua and municipal items)
   removeUnwantedGroups(overviewItem, otherMap);
   // Add bua and mun groups to point of interest items (underview ids)
   addGroupsToPoi(overviewItem, otherItem, otherMap);
   
   // Update group ids to overview ids
   uint32 otherMapId = otherMap->getMapID();
   for ( uint32 j = 0; j < overviewItem->getNbrGroups(); ++j ) {
      overviewItem->setGroup(
            j, reverseLookup(otherMapId, overviewItem->getGroup(j)) ); 
      // fixme: what about high bit?
   }
   
   // Update connection attributes (depends on connfromids) including 
   // signpost names (depends on stringIndex)
   OldRouteableItem* myRI = dynamic_cast<OldRouteableItem*> (overviewItem);
   OldRouteableItem* otherRI = dynamic_cast<OldRouteableItem*> (otherItem);
   if ((myRI != NULL) && (otherRI != NULL)) {
      for (uint32 n = 0; n < 2; n++) {
         OldNode* myNode = myRI->getNode(n);
         OldNode* otherNode = otherRI->getNode(n);
         for (uint32 myc = 0; myc < myNode->getNbrConnections(); myc++) {
            OldConnection* myConn = myNode->getEntryConnection(myc);
            if (myConn != NULL) {
               uint32 myFromNodeID = myConn->getConnectFromNode();

               // Find this conn in otherNode
               OldConnection* otherConn = NULL;
               uint32 otherc = 0;
               while ((otherc < otherNode->getNbrConnections()) &&
                      (otherConn == NULL)) {
                  uint32 otherFromNodeID = otherNode->
                     getEntryConnection(otherc)->getConnectFromNode();
                  uint32 otherFromNodeOverviewID = 
                     reverseLookupNodeID(otherMapId, otherFromNodeID);
                  if ((otherFromNodeOverviewID != MAX_UINT32) &&
                      (myFromNodeID == otherFromNodeOverviewID)) {
                     otherConn = otherNode->getEntryConnection(otherc);
                  } else {
                     otherc++;
                  }
               }

               if (otherConn != NULL) {
                  // We have the connections, update connection attributes!
                  if (myConn->updateConnAttributesFromConn(otherConn))
                     retVal = true;

                  // Update sign posts.
                  if ((myConn->getNbrSignPost(*this, myNode->getNodeID() ) > 0) || 
                      (otherConn->getNbrSignPost(*otherMap, otherNode->getNodeID()) > 0) ) {
                     mc2dbg2 << "updating signposts, nbr in myConn " 
                             << int(myConn->getNbrSignPost(*this, 
                                                           myNode->getNodeID())) 
                             << " otherConn "
                             << int(otherConn->getNbrSignPost(*otherMap, 
                                                              otherNode->getNodeID())) 
                             << endl;
                     updateSignposts(myConn->getConnectFromNode(),
                                     myNode->getNodeID(), 
                                     otherConn->getConnectFromNode(),
                                     otherNode->getNodeID(),
                                     otherMap);
                     mc2dbg2 << "updated signposts, nbr in myConn " 
                             << int(myConn->getNbrSignPost(*this,
                                                           myNode->getNodeID())) 
                             << " otherConn "
                             << int(otherConn->getNbrSignPost(*otherMap, 
                                                              otherNode->getNodeID())) 
                             << endl;
                  }

               }
               // else otherConn not included in overview map
            }
         }
      }
   }

   return retVal;
}



bool
OldOverviewMap::removeAllGfxData( ItemTypes::itemType itemType )
{
   bool result = true;

   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z, i);
         if ( item != NULL ){
            result = result && item->removeAllGroups();
         }
      }
   }
   return result;
}

bool 
OldOverviewMap::removeItem(uint32 localID, 
                           bool updateHashTable /* = true */,
                           bool updateAdminCenters /* = true */, 
                           bool unusedUkZips /* = false */)
{
   // normal removeItem
   bool result = OldGenericMap::removeItem( localID, updateHashTable, 
                                         updateAdminCenters, unusedUkZips );
   // remove from look-up table
   if ( result ){
      result = removeFromLookupTable( localID );
   }
   
   return result;
}
