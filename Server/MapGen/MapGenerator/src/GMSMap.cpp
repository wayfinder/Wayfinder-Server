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

#include "GMSMap.h"
#include "Stack.h"

#include "StringUtility.h"
#include "OldMapHashTable.h"

#include "GMSItem.h"
#include "GMSAirportItem.h"
#include "GMSAircraftRoadItem.h"
#include "GMSBuildingItem.h"
#include "GMSBuiltUpAreaItem.h"
#include "GMSBusRouteItem.h"
#include "GMSCategoryItem.h"
#include "GMSCityPartItem.h"
#include "GMSFerryItem.h"
#include "GMSForestItem.h"
#include "GMSIndividualBuildingItem.h"
#include "GMSIslandItem.h"
#include "GMSMilitaryBaseItem.h"
#include "GMSCartographicItem.h"
#include "GMSMunicipalItem.h"
#include "GMSParkItem.h"
#include "GMSPedestrianAreaItem.h"
#include "GMSPointOfInterestItem.h"
#include "GMSRailwayItem.h"
#include "GMSStreetItem.h"
#include "GMSStreetSegmentItem.h"
#include "GMSWaterItem.h"
#include "GMSZipCodeItem.h"
#include "OldNullItem.h"
#include "OldSubwayLineItem.h"
#include "OldZipAreaItem.h"

#include "SetTurnDescriptions.h"
#include "NodeBits.h"
#include "Crossing.h"
#include "GMSCountryOverviewMap.h"
#include "OldOverviewMap.h"

#include "GMSUtility.h"
#include "GfxConstants.h"
#include "GfxUtility.h"

#include "OldExternalConnections.h"

#include "IDPairVector.h"
#include "NationalProperties.h"
#include "StringSearchUtility.h"
#include "AbbreviationTable.h"
#include "MC2Coordinate.h"
#include "GDFRef.h"
#include "ZipSideFile.h"
#include "MapGenUtil.h"
#include "TimeUtility.h"
#include "MapBits.h"
#include "Math.h"

#include "Utility.h"

#include <typeinfo>
#include <map>
#include <sys/types.h>
#include <dirent.h>
#include <math.h>
#ifdef _WIN32
#include <fstream.h>
#else
#include <fstream>
#endif



#ifdef DEBUG_LEVEL_CON
#define   DEBUG_CON(a)   a
#else
#define   DEBUG_CON(a)
#endif

#ifdef DEBUG_LEVEL_STREETS
#define DEBUG_ST(a) a
#else
#define DEBUG_ST(a)
#endif

/*
 *    The maximum number of municipals in the map-file. This is not the
 *    same as the number of municipals in the map, since every island is
 *    one municipal of its owm in the .map-file.
 */
#define MAX_NBR_MAP_MUNICIPALS 512

#include "UTF8Util.h"

// Moved from TextIterator and changed
/**
 *  Creates an MC2String from the text between begin and end. The 
 *  parameters begin and end must point into the same string and 
 *  end must be the same or larger than begin.
 *
 *  @param begin The place to start the MC2String at.
 *  @param begin The place to end the MC2String at.
 *
 *  @return Returns a MC2Strig that is a copy of the text between begin
 *          and end.
 */
static MC2String copyMC2String( utf8TextIterator it,
                                const utf8TextIterator& end )
{
   MC2String res;
   for ( ;
         it != end;
         ++it ) {
      res += UTF8Util::ucsToUtf8(*it);
   }
   return res;
}


OldGenericMap*
GMSMap::createMap(uint32 id, const char* path, bool loadMap)
{
   // Variables to return/get the map status
   OldGenericMap* theNewMap = NULL;
 
   // Create a map with correct type
   if (MapBits::isUnderviewMap(id)) {
      theNewMap = new GMSMap(id, path);
   } else if (MapBits::isOverviewMap(id)) {
      mc2dbg << here << " creating OldOverviewMap (NOT GMS-map)" << endl;
      theNewMap = OldGenericMap::createMap(id, path);
      loadMap = false;
   } else {
      theNewMap = new GMSCountryOverviewMap(id, path);
   }


   if ( (theNewMap != NULL) && 
        !MapBits::isOverviewMap(id) ) 
   {
      // Create allocators
      theNewMap->createAllocators();
   }
   if ( (theNewMap != NULL) && loadMap) {

      // Fill the new map with data
      bool newMapOK = theNewMap->load();

      // Find out if the map is ok and set the status of the creation.
      // If it's not OK, it will be deleted!
      if (!newMapOK) {
         delete theNewMap;
         theNewMap = NULL;
      }
   }

   return (theNewMap);
}

OldGenericMap*
GMSMap::createMap(const char* mcmName)
{
   // Extract the ID and path from mcmName
   char path[256];
   path[0] = '\0';
   uint32 mapID;
   bool ok = false;

   // Find the last '/'
   char* slashPtr = StringUtility::strrchr(mcmName, '/');
   if (slashPtr == NULL) {
      // No slash, se if id + ".mcm" is provided
      if (sscanf(mcmName, "%x.mcm", &mapID) == 1) {
         strcat(path, "./");
         ok = true;
      }
   } else {
      // Got a slash, get mapID
      if (sscanf(slashPtr, "/%x.mcm", &mapID) == 1) {
         // Got mapID, get path
         int n = slashPtr-mcmName+1;
         if (strncpy(path, mcmName, n) != NULL) {
            path[n] = '\0';
            ok = true;
         }
      }
   }
   
   mc2dbg << "   mcmName=" << mcmName << endl;
   mc2dbg << "   path=" << path << ", mapID=" << mapID << endl;

   if (ok) {
      // Create from other createMap-method
      return createMap(mapID, path);
   } else {
      return NULL;
   }
}

GMSMap::GMSMap(uint32 mapID, const char* path)
      : OldMap(mapID, path)
{
   mc2dbg2 << "Creating GMSMap!" << endl;

   m_itemNames = new GMSItemNames();
   municipalOffset = 0;
   municipalVector = new uint32[MAX_NBR_MAP_MUNICIPALS];
   for (uint32 i=0; i<MAX_NBR_MAP_MUNICIPALS; i++) {
      municipalVector[i] = UNUSED;
   }
   m_itemsFromExtraItemMap = NULL;

}


GMSMap::~GMSMap() {
   delete [] municipalVector;
   delete m_itemsFromExtraItemMap;
   mc2dbg4 << "GMSMap destroyed" << endl;
}

void
GMSMap::createAllocators()
{
   m_streetSegmentItemAllocator = new MC2Allocator<GMSStreetSegmentItem>(0);
   m_streetItemAllocator = new MC2Allocator<GMSStreetItem>(0);
   m_municipalItemAllocator = new MC2Allocator<GMSMunicipalItem>(0);
   m_cityPartItemAllocator = new MC2Allocator<GMSCityPartItem>(0);
   m_waterItemAllocator = new MC2Allocator<GMSWaterItem>(0);
   m_parkItemAllocator = new MC2Allocator<GMSParkItem>(0);
   m_forestItemAllocator = new MC2Allocator<GMSForestItem>(0);
   m_buildingItemAllocator = new MC2Allocator<GMSBuildingItem>(0);
   m_railwayItemAllocator = new MC2Allocator<GMSRailwayItem>(0);
   m_islandItemAllocator = new MC2Allocator<GMSIslandItem>(0);
   m_zipCodeItemAllocator = new MC2Allocator<GMSZipCodeItem>(0);
   m_zipAreaItemAllocator = new MC2Allocator<OldZipAreaItem>(0);
   m_pointOfInterestItemAllocator = 
      new MC2Allocator<GMSPointOfInterestItem> (0);
   m_categoryItemAllocator = new MC2Allocator<GMSCategoryItem>(0);
   m_builtUpAreaItemAllocator = new MC2Allocator<GMSBuiltUpAreaItem> (0);
   m_busRouteItemAllocator = new MC2Allocator<GMSBusRouteItem>(0);
   m_airportItemAllocator = new MC2Allocator<GMSAirportItem>(0);
   m_aircraftRoadItemAllocator = new MC2Allocator<GMSAircraftRoadItem> (0);
   m_pedestrianAreaItemAllocator = new MC2Allocator<GMSPedestrianAreaItem> (0);
   m_militaryBaseItemAllocator = new MC2Allocator<GMSMilitaryBaseItem> (0);
   m_individualBuildingItemAllocator =
      new MC2Allocator<GMSIndividualBuildingItem> (0);
   m_subwayLineItemAllocator = new MC2Allocator<OldSubwayLineItem> (0);
   m_nullItemAllocator = new MC2Allocator<OldNullItem> (0);
   m_ferryItemAllocator = new MC2Allocator<GMSFerryItem> (0);
   m_gfxDataAllocator = new MC2Allocator<GMSGfxData>(0);
   // These will not be used in a GMSMap, so we dont need to create
   // subclasses for the "smaller" GfxData's
   m_gfxDataSingleSmallPolyAllocator = NULL;
   m_gfxDataSingleLineAllocator = NULL;
   m_gfxDataSinglePointAllocator = NULL;
   m_gfxDataMultiplePointsAllocator = NULL;
   m_nodeAllocator = new MC2Allocator<GMSNode>(0);
   m_connectionAllocator = new MC2Allocator<OldConnection>(0);
   m_connectionArrayAllocator = new MC2ArrayAllocator<OldConnection*>(0);
   m_connectionArrayAllocator->setWarnSingleAlloc( false );
   m_simpleItemAllocator = new MC2Allocator<OldItem>(0);
   m_cartographicItemAllocator = new MC2Allocator<GMSCartographicItem> (0);
}

void
GMSMap::initNonItemAllocators( uint32 nbrGfxDatas, 
                               uint32 nbrGfxDataSingleSmallPoly,
                               uint32 nbrGfxDataSingleLine, 
                               uint32 nbrGfxDataSinglePoint,
                               uint32 nbrGfxDataMultiplePoints, 
                               uint32 nbrNodes, 
                               uint32 nbrConnections ) 
{
   // In GMSMap, all GfxDatas are GMSGfxDatas. 
   nbrGfxDatas += nbrGfxDataSingleSmallPoly + 
                  nbrGfxDataSingleLine +
                  nbrGfxDataSinglePoint + 
                  nbrGfxDataMultiplePoints;
   m_gfxDataAllocator->reallocate(nbrGfxDatas);
  
   // No more tricks needed.
   m_nodeAllocator->reallocate(nbrNodes);
   m_connectionAllocator->reallocate(nbrConnections);
   m_connectionArrayAllocator->reallocate(nbrConnections);
}

GMSGfxData*
GMSMap::createNewGfxData(DataBuffer* dataBuffer) 
{
   mc2dbg8 << here << " createNewGfxData in GMSMap!" << endl;
   //return (new GMSGfxData(dataBuffer));
   return GMSGfxData::createNewGfxData(dataBuffer, this);
}

OldConnection* 
GMSMap::createNewConnection(DataBuffer* dataBuffer)
{
   return (OldConnection::createNewConnection(dataBuffer, this));
}

OldItem*
GMSMap::createNewItem(DataBuffer* dataBuffer)
{
   mc2dbg8 << here << " createNewItem in subclass!" << endl;
   return (GMSItem::createNewItem(dataBuffer, this));
}

void GMSMap::setName(char *n)
{
   delete m_name;
   m_name = new char[strlen(n)+1];
   strcpy(m_name, n);
}




// Adds a connection between the segments that curNode and otherNode are
// located at:
//              curNode
//   O-------------O    O---------O
//                   otherNode
void
GMSMap::addConnection(OldNode* curNode, OldNode* otherNode, 
                      uint32 distOtherNode, map<uint32, uint32>* vtByNode)
{

   // Create and add connection: curNode <-- otherNode ^ 0x80000000
   uint32 tmpNodeID = otherNode->getNodeID() ^ 0x80000000;
   OldConnection* tmpCon = new OldConnection(tmpNodeID);
   if ( !(curNode->addConnection(tmpCon, *this)) ) {
      mc2dbg4 << "Could not add the connection" << endl;
   }
   // Set allowed vehicles:
   if ( vtByNode != NULL ) {
      tmpCon->setVehicleRestrictions( 
      (*vtByNode)[ tmpNodeID ] & (*vtByNode)[ curNode->getNodeID() ] );
   }

   // Create and add connection: otherNode <--- curNode ^ 0x80000000
   tmpNodeID = curNode->getNodeID() ^ 0x80000000;
   tmpCon = new OldConnection(tmpNodeID);
   if ( !(otherNode->addConnection(tmpCon, *this )) ) {
      mc2dbg4 << "Could not add the connection" << endl;
   }
   if ( vtByNode != NULL ) {
      tmpCon->setVehicleRestrictions( 
      (*vtByNode)[ tmpNodeID ] & (*vtByNode)[ otherNode->getNodeID() ] );
   }
}





bool
GMSMap::compactBuiltUpAreas()
{
   const int MAX_NBR_BUAS = 1024;
   const uint64 MAX_SQDIST_SAME_BUA = 10000*10000; // (1 mil)^2

   // Create and fill one array with pointers to all the built up areas
   OldItem** allBUAs = new OldItem*[MAX_NBR_BUAS];
   uint32 nbrBUAs = 0;
   DEBUG1(cerr << "Before BUA concationation" << endl;);
   for (uint32 i=0; i<getNbrItemsWithZoom(0); i++) {
      OldItem* tmpItem = getItem(0, i);
      if (tmpItem->getItemType() == ItemTypes::builtUpAreaItem) {
         DEBUG1(cerr << i << ". " << getFirstItemName(tmpItem) << endl;);
         allBUAs[nbrBUAs] = tmpItem;
         nbrBUAs++;
      }
   }
   DEBUG1(cerr << "====================, nbrBUAs = " << nbrBUAs << endl);

   // Create and fill one array with pointers to all the municipals
   OldItem** allMunicipals = new OldItem*[MAX_NBR_BUAS];
   uint32 nbrMunicipals = 0;
   for (uint32 i=0; i<getNbrItemsWithZoom(0); i++) {
      OldItem* tmpItem = getItem(0, i);
      if (tmpItem->getItemType() == ItemTypes::municipalItem) {
         DEBUG4(cerr << i << ". " << getItemName(tmpItem) << endl;);
         allMunicipals[nbrMunicipals] = tmpItem;
         nbrMunicipals++;
      }
   }

   // Check all the built up areas. Remove those who are outside the map 
   // and concatinate those who have the same name.
   for (uint32 nameBUApos=0; nameBUApos < nbrBUAs; nameBUApos++) {
      OldItem* nameBUA = allBUAs[nameBUApos];
      if (nameBUA != NULL) {
         bool oneSet = true;

         GMSGfxData* curBUAGfx = (GMSGfxData*) nameBUA->getGfxData();

         // Check that this BUA is inside any municipal in this map
         bool BUAok = false;
         for (uint32 i=0; i<nbrMunicipals; i++) {
            GfxData* munGfx = allMunicipals[i]->getGfxData();
            for (uint32 p=0; p<curBUAGfx->getNbrPolygons(); p++) {
               for (uint32 j=0; j<curBUAGfx->getNbrCoordinates(p); j++) {
                  if (munGfx->insidePolygon(curBUAGfx->getLat(p,j),
                                            curBUAGfx->getLon(p,j)) > 0) {
                     DEBUG8(cerr << getFirstItemName(nameBUA) 
                            << " inside municipal " 
                            << getFirstItemName(allMunicipals[i]) << endl);
                     BUAok = true;
                  }
               }
            }
         }

         if (!BUAok) {  
            // This BUA is not OK!!!
            mc2log << warn << "BUA " << getFirstItemName(nameBUA) 
                   << " (pos="
                   << nameBUApos << ", ID=" 
                   << nameBUA->getID() << ") not inside any "
                   << "municipal on this map!" << endl;
            oneSet = false;
            removeItem(nameBUA->getID());
            allBUAs[nameBUApos] = NULL;
         } else {
            // Include all BUAs with correct name into the nameBUA
            for (uint32 checkBUApos = nameBUApos+1;
                 checkBUApos < nbrBUAs;
                 checkBUApos++) {
               if ( ( allBUAs[checkBUApos] != NULL) &&
                    ( nameBUA->getStringIndex(0) == 
                      allBUAs[checkBUApos]->getStringIndex(0)) &&
                    (nameBUA->getGfxData()->minSquareDistTo(
                       allBUAs[checkBUApos]->getGfxData()) < 
                     MAX_SQDIST_SAME_BUA)
                    ) {
                  DEBUG1(cerr << "   Adding polygon of BUA " 
                         << getFirstItemName(nameBUA) << "(pos="
                         << (uint32) checkBUApos
                         << ") to BUA " << nameBUApos << endl);
                  if (nameBUA->getGfxData()->
                      addPolygon(allBUAs[checkBUApos]->getGfxData())) {
                     cout << "BUA " << checkBUApos << " (ID = " 
                          << allBUAs[checkBUApos]->getID()
                          << ") added" << endl;
                     // change locations for items in the added bua.
                     changeLocation(allBUAs[checkBUApos]->getID(),
                                    nameBUA->getID(), 
                                    ItemTypes::builtUpAreaItem, false);
                     removeItem(allBUAs[checkBUApos]->getID());
                     allBUAs[checkBUApos] = NULL;
                  } else {
                     cout << "BUA " << allBUAs[checkBUApos]->getID()
                          << " does not fit in!" << endl;
                  }
               }
            }
         }
      }
   }
         
   // Find the first empty slot
   uint32 curFirstFreeID = 0;
   findEmptySlot(0, curFirstFreeID);
   for (uint32 i=0; i<getNbrItemsWithZoom(0); i++) {
      if ( (curFirstFreeID < i) && 
           (itemLookup(i)->getItemType() == ItemTypes::builtUpAreaItem)) {
         swapItems(curFirstFreeID, i);
         changeLocation(i, curFirstFreeID, ItemTypes::builtUpAreaItem);
         findEmptySlot(0, curFirstFreeID);
      }
   }

   DEBUG1(cerr << "After BUA concationation" << endl;);
   nbrBUAs = 0;
   for (uint32 i=0; i<getNbrItemsWithZoom(0); i++) {
      OldItem* tmpItem = getItem(0, i);
      if (tmpItem->getItemType() == ItemTypes::builtUpAreaItem) {
         DEBUG1(cerr  << getFirstItemName(tmpItem) << " ("<< i << ")" << endl;);
         nbrBUAs++;
      }
   }
   DEBUG1(cerr << "====================, nbrBUAs = " << nbrBUAs << endl);

   return (true);
} // compactBuiltUpAreas


bool
GMSMap::compactMunicipals() 
{
   mc2dbg1 << "GMSMap::compactMunicipals" << endl;
   
   // First make sure the municipalVector is updated
   updateMunicipalArray();
   
   mc2dbg1 << "GMSMap::compactMunicipals for " << municipalOffset 
           << " municipals" << endl;
   // Create and fill one array with pointers to all the municipals
   OldItem** allMunicipals = new OldItem*[municipalOffset];
   for (uint32 i=0; i<municipalOffset; i++) {
      cout << " municipal " << municipalVector[i] << endl;
      allMunicipals[i] = itemLookup(municipalVector[i]);
      if (allMunicipals[i] == NULL) {
         cerr << "ERROR (NULL) in municipal vector, i == " 
              << (uint32) i << endl;
      }
   }

   // Check all the municipals and concatinate those who have 
   // the same name. Start with the one that have the most nbr coords
   // (to make sure polygon 0 has the most nbr coords).
   uint32 nbrRemovedMunicipals = 0;
   for (uint32 nameMunicipal=0; 
        nameMunicipal<municipalOffset;
        nameMunicipal++) {
      if (allMunicipals[nameMunicipal] != NULL) {
         // Find the municipal with the most nbr coordinates that
         // have the same name as allMunicipals[nameMunicipal]
         OldItem* maxNbrCoordMunicipal = allMunicipals[nameMunicipal];
         for (uint32 checkMunicipal = nameMunicipal+1;
              checkMunicipal < municipalOffset;
              checkMunicipal++) {
            if ( ( allMunicipals[checkMunicipal] != NULL) &&
                 ( allMunicipals[nameMunicipal]->getStringIndex(0) == 
                   allMunicipals[checkMunicipal]->getStringIndex(0)) &&
                 ( allMunicipals[checkMunicipal]->getGfxData()
                   ->getTotalNbrCoordinates() > maxNbrCoordMunicipal
                   ->getGfxData()->getTotalNbrCoordinates())
                 ) {
               DEBUG4(cerr << (uint32) checkMunicipal 
                      << " has more coordinates than " 
                      << (uint32) maxNbrCoordMunicipal->getID() 
                      << endl);
               maxNbrCoordMunicipal = allMunicipals[checkMunicipal];
            }
         }

         // Now, include all municipals with correct name
         // into the maxNbrCoordMunicipal
         for (uint32 checkMunicipal = nameMunicipal;
              checkMunicipal < municipalOffset;
              checkMunicipal++) {
            if ( ( allMunicipals[checkMunicipal] != NULL) &&
                 ( maxNbrCoordMunicipal != 
                   allMunicipals[checkMunicipal]) &&
                 ( maxNbrCoordMunicipal->getStringIndex(0) == 
                   allMunicipals[checkMunicipal]->getStringIndex(0))
                 ) {
               mc2dbg1 << "   includeing municipal "
                       << (uint32) checkMunicipal
                       << " into maxNbrCoordMunicipal "
                       << nameMunicipal << endl;
               (maxNbrCoordMunicipal->getGfxData())
                  ->addPolygon(allMunicipals[checkMunicipal]
                               ->getGfxData());
               mc2dbg8 << "   To remove municipal " 
                       << (uint32) checkMunicipal << endl;

               // change locations for items in the added municipal.
               changeLocation(allMunicipals[checkMunicipal]->getID(),
                              maxNbrCoordMunicipal->getID(),
                              ItemTypes::municipalItem, false);
               removeItem(allMunicipals[checkMunicipal]->getID());
               allMunicipals[checkMunicipal] = NULL;
               nbrRemovedMunicipals++;
            }
         }
      }
   }
         
   DEBUG1(cerr << (uint32) (municipalOffset-nbrRemovedMunicipals) 
          << " true municipal(s) on this map" << endl);

   // Make sure that the municipals have the lowest ID's

   // Find the first empty slot
   uint32 curFirstFreeID = 0;
   bool ok = findEmptySlot(0, curFirstFreeID);
   if ( ok ) {
      for (uint32 i=0; i<getNbrItemsWithZoom(0); i++) {
         if ( (curFirstFreeID < i) && 
              (itemLookup(i)->getItemType() == ItemTypes::municipalItem)) {
            mc2dbg << "   Swap " << curFirstFreeID << " with " << i << endl;
            mc2dbg << "   Swap " << getFirstItemName(curFirstFreeID) << " ("
                   << ItemTypes::getItemTypeAsString(
                           itemLookup(curFirstFreeID)->getItemType())
                   << ") with " << getFirstItemName(i) << " ("
                   << ItemTypes::getItemTypeAsString(
                           itemLookup(i)->getItemType())
                   << ")" << endl;
                          
            swapItems(curFirstFreeID, i);
            changeLocation(i, curFirstFreeID, ItemTypes::municipalItem);
            ok = findEmptySlot(0, curFirstFreeID);
            if ( ! ok ) {
               break;
            }
         }
      }
   }

   // Check that the municipals are first in the array...
   // This is for debugging only!
   uint32 fooCnt = 0;
   OldMunicipalItem* curCA = dynamic_cast<OldMunicipalItem*>(itemLookup(fooCnt));
   while (curCA != NULL) {
      cout << "ID = " << fooCnt << ", name = " 
           << getFirstItemName(curCA) << endl;
      fooCnt++;
      curCA = dynamic_cast<OldMunicipalItem*>(itemLookup(fooCnt));
   }
   cout << "Total number of municipals : " << fooCnt << endl;

   // Update the municipal-array with the new ID's
   updateMunicipalArray();

   return (true);
} // compactMunicipals

uint32
GMSMap::changeLocation(uint32 oldLocation, uint32 newLocation,
      ItemTypes::itemType locationType, bool changeLocationItem)
{
   mc2dbg2 << "changeLocation: " 
           << StringTable::getString(
                 ItemTypes::getItemTypeSC(ItemTypes::itemType(locationType)),
                 StringTable::ENGLISH) << "-location "
           << oldLocation << " -> " << newLocation << endl;
   
   uint32 nbrChanges = 0;

   for(uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z, i);
         if (item != NULL) {
            
            ItemTypes::itemType type = item->getItemType();
            bool found = false;
            uint32 j = 0;
            while ( !found && ( j < item->getNbrGroups() ) ) {
               if ( item->getGroup( j ) == oldLocation ) {
                  found = true;
               } else {
                  ++j;
               }
            }
             
            // Don't change any location when:
            // - municipalitem and bualocation
            // - changeLocationItem is false, e.g. when buas have 
            // been merged the bua to be removed need not to be updated
            // 
            if( found &&
                  
                !((type == ItemTypes::municipalItem) &&
                  (locationType != ItemTypes::municipalItem)) &&
                  
                !(!changeLocationItem && 
                  (type == locationType) &&
                  (item->getID() == oldLocation)) ) {
                  
               nbrChanges++;
               removeRegionFromItem(item, oldLocation);
               if (!addRegionToItem(item, newLocation)){
                  mc2log << error << "Could not change location of item: "
                         << item->getID() << " to " << newLocation 
                         << " from " << oldLocation << endl;
               }
            }
               
         }
      }
   }
   mc2dbg4 << " Changed location for " << nbrChanges << " items." << endl; 
   
   return nbrChanges;
}

bool
GMSMap::setMapGfxDataConvexHull()
{
   mc2dbg1 << "Calculate convex hull for map gfx data" << endl;

   // Use street segmens and points of interest
   GfxData* gfx;
   GfxDataFull* allItems = GMSGfxData::createNewGfxData(NULL, true);
   for (uint32 curZoom=0; curZoom < NUMBER_GFX_ZOOMLEVELS; curZoom++) {
      for ( uint32 itemNbr = 0;
            itemNbr < getNbrItemsWithZoom(curZoom); itemNbr++) {
         
         OldItem* item = getItem(curZoom, itemNbr);
         
         if ( item == NULL ) {
            continue;
         }

         if ( (gfx = item->getGfxData()) == NULL ) {
            continue;
         }

         if (item->getItemType() == ItemTypes::streetSegmentItem) {
            allItems->addCoordinate(gfx->getLat(0,0), gfx->getLon(0,0));
            allItems->addCoordinate(gfx->getLastLat(0), gfx->getLastLon(0));
         }
         else if( item->getItemType() == ItemTypes::pointOfInterestItem ) {
            allItems->addCoordinate(gfx->getLat(0,0), gfx->getLon(0,0));
         }
         else if ( ( item->getItemType() == ItemTypes::municipalItem ) ||
                   ( item->getItemType() == ItemTypes::islandItem ) ) {
            for ( uint32 p=0; p<gfx->getNbrPolygons(); p++ ){
               GfxData::const_iterator cIt = gfx->polyBegin(p);
               while ( cIt != gfx->polyEnd(p) ){
                  allItems->addCoordinate(cIt->lat, cIt->lon);
                  ++cIt;
               }
            }         
         }
      }
   }
   
   // Don't replace the gfx data here, if there are NO ssi in this map,
   // the convex hull will not be built. (leave the map gfx as it is...)
   //delete m_gfxData;
   //m_gfxData = GMSGfxData::createNewGfxData(NULL, true);
   
   // Calculate the convex hull.
   Stack* stack = new Stack;
   if (allItems->getConvexHull(stack, 0)) {
      m_gfxData = GMSGfxData::createNewGfxData(NULL, true);
      
      uint32 size = stack->getStackSize();
      for (uint32 i = 0; i < size; i++) {
         uint32 idx = stack->pop();
         m_gfxData->addCoordinate(allItems->getLat(0,idx), 
                                  allItems->getLon(0,idx));
      }
   } else {
      // Something went wrong.
      mc2log << warn << "GMSMap::setMapGfxDataConvexHull "
             << "Could not create convex hull" << endl;
      return false;
   }
   m_gfxData->setClosed(0, true);
   m_gfxData->updateLength();
   
   delete stack;
   delete allItems;
   return true;
}


void GMSMap::generateStreetsFromStreetSegments()
{
   DEBUG1(uint32 startTime = TimeUtility::getCurrentTime());

   mc2log << info << "GMSMap Generating streets from streetsegments!"
          << endl;


   mc2log << info << "GMSMap Checking for streets already present and "
          << "removes them." << endl;
   uint32 nbrRmStreets = 0;
   for ( uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++ ){
      for ( uint32 i=0; i<getNbrItemsWithZoom(z); i++ ){
         
         OldItem* item = getItem(z, i);
         if ( item->getItemType() == ItemTypes::streetItem ){
            if (removeItem( item->getID(), false ) ){
               nbrRmStreets++;
            }
            else {
               mc2log << error << "Could not remove street item. "
                      << "Already removed " << nbrRmStreets << " streets."
                      << "Exits!" << endl;
               exit(1);
            }
         }
      }
   }
   if ( nbrRmStreets > 0 ){
      mc2log << info << "   Removed " << nbrRmStreets << " streets." 
             << endl;
      mc2log << info << "   Removing groups pointing at streets." << endl;
      if ( ! removeGroups( ItemTypes::streetItem ) ){
         mc2log << "Could not remove street item groups. Exits!" << endl;
         exit(1);
      }
      mc2log << info << "   Rebuilding hash table." << endl;
      buildHashTable();
   }

   
   OldItem* tempItem;
   
   // Counter for the number of newly created streets
   uint32 nbrOfStreets = 0; 
   
   // The total number of strings
   uint32 nbrOfStrings = m_itemNames->getNbrStrings();

   // Go through all zoomlevels - make streets of
   // streetsegments from different zoomLevels
   // Reset the vector of streets
   GMSStreetItem* streetVector[nbrOfStrings];

   for(uint32 j = 0; j < nbrOfStrings; j++)
      streetVector[j] = NULL;

   mc2log << info << "GMSMap Creating one street per SSI name." << endl;
   for(int zoomLevel=0; zoomLevel < NUMBER_GFX_ZOOMLEVELS; zoomLevel++) {
      // Now collect the already existing streets
      uint32 nbrOfItemsWithZoom = getNbrItemsWithZoom(zoomLevel);
      for(uint32 itemID=0; itemID < nbrOfItemsWithZoom; itemID++) {
         tempItem = getItem(zoomLevel, itemID);
         if (  (tempItem != NULL) &&
               (tempItem->getItemType() == ItemTypes::streetItem) ) {
            mc2log << fatal << here << "Found a street in map. "
                   << "Don't know what to do - exiting" 
                 << endl;
            exit(1);
         }
      } // next itemID

      // Now sort all the street segments that have the same name together
      for (  uint32 streetSegID = 0; 
             streetSegID < nbrOfItemsWithZoom; 
             streetSegID++) {
         tempItem = getItem(zoomLevel, streetSegID);
         if(tempItem != NULL) {
            if ( tempItem->getItemType() == ItemTypes::streetSegmentItem ) {
               
               // Add the item as a street for each name it has.
               for (uint32 name = 0; name < tempItem->getNbrNames(); name++) {
                  uint32 stringIdx = tempItem->getStringIndex(name);
               
                  if (streetVector[stringIdx] == NULL) {
                     // Use dummy ID to make sure it is initialized
                     streetVector[stringIdx] = new GMSStreetItem(MAX_UINT32);
                     streetVector[stringIdx]->
                        addName( tempItem->getNameLanguage(name),
                                 tempItem->getNameType(name),
                                 tempItem->getStringIndex(name));
                     nbrOfStreets++;
                  }
                  streetVector[stringIdx]->addItem(
                     zoomLevel << 27 | streetSegID);
               } // for name
            } // endif
         } // endif (tempItem != NULL)      
      } // next ssi         
   } // next zoomlevel            
   mc2log << info << "Created " << nbrOfStreets << " unsplit streets." 
          << endl;



   // Now we have collected all streetssegments with the same names as 
   // streets in streetVector. We now need to split these streets so that 
   // only streetsegments that are close to each other belong to the 
   // same street.

   mc2log << info << "GMSMap Splitting streets on distance." << endl;
   uint32 nbrSplitStreets = 0;
   for (uint32 i = 0; i < nbrOfStrings; i++) {
      if (streetVector[i] != NULL) {
         GMSStreetItem* street = streetVector[i];
         vector<u32set> theSplitStreets;
         
         // Split the streets
         // -----------------------------------------------------------
         splitStreets(street, theSplitStreets);
         
         mc2dbg4 << "Split " << getName(i) << endl;
         for ( vector<u32set>::iterator it = theSplitStreets.begin();
               it != theSplitStreets.end(); ++it ) {
            if (! it->empty()) {
               GMSStreetItem* streetToAdd = new GMSStreetItem;;
               nbrSplitStreets++;
               // We now support streets with only one streetsegment as
               // well.
                  
               // Add to street with correct zoomlevel to map
               // The zoomlevel for the street should be 1 above the 
               // highest zoomleve of the streetsegments.
               // -----------------------------------------------------
               addItem(streetToAdd, 
                       GET_ZOOMLEVEL(*(it->begin())) - 1);
               
               // Add name
               // -----------------------------------------------------
               ItemTypes::name_t nameType = street->getNameType(0);
               
               addNameToItem(streetToAdd,
                             getName(street->getStringIndex(0)), 
                             street->getNameLanguage(0),
                             nameType);
               
               // Sort the streetsegments in the streets. 
               // Largest address range first.
               // -----------------------------------------------------
               
               u32mmap sortedSSIIDs;
               sortStreetOrder(streetToAdd, 
                               *it, 
                               sortedSSIIDs);
               
               mc2dbg4 << "Added street " 
                       << getName(streetToAdd->getStringIndex(0))
                       << " with " << sortedSSIIDs.size()
                       << " streetsegments" << endl;

               mc2dbg4 << "Street id = " << streetToAdd->getID() 
                       << " (0x" << hex << streetToAdd->getID() 
                       << ")" << dec << endl;
               
               // Bind the streetsegment items to this street.
               // -----------------------------------------------------
               for ( u32mmap::iterator jt = sortedSSIIDs.begin();
                     jt != sortedSSIIDs.end();
                     ++jt ) {
                  OldItem* item = itemLookup(jt->second);
                  addRegionToItem(item, streetToAdd->getID());
                  streetToAdd->addItem(jt->second);
                  DEBUG4(cout << "     ssi id = " << item->getID() 
                         << " (0x" << hex << item->getID() 
                         << ")" << dec << endl);
               }
            } 
         }
         // Also delete streetVector[i]
         delete streetVector[i];
      } // streetVec != NULL
   } // nbrStrings
   mc2log << info << "Number streets after splitting: " << nbrSplitStreets
          << endl;
   

   // Do not set any GfxData for the streets in order to save memory.
   // and the GfxData is no longer needed for making extradata corrections
   // to streets

   // Removes all but one of all streets including exactly the same SSIs
   // and add names from all streets removed to the remaining one.
   mc2log << info << "Merging streets including the same SSIs." << endl;
   uint32 nbrRemoved = this->mergeStreetsWithSameSSIs();
   mc2log << info << "   Removed " << nbrRemoved << " streets when"
          << " merging." << endl;

   // Extend abbreviated street names if needed.
   ItemTypes::itemType itemType = ItemTypes::streetItem;
   if ( NationalProperties::extendAbbrevStreetNames( 
                                           this->getCountryCode(),
                                           itemType ) ){
      mc2log << info << "Extends abbreviated street names." << endl;
      // Add longer form of abbreviations.
      this->extendAbbreviations( itemType );
   }
   
   DEBUG1(cout << "Generating streets from streetsegments took "
          << float64(TimeUtility::getCurrentTime() - startTime) / 1000
          << " seconds." << endl);
} // generateStreetsFromStreetSegments

namespace {

   /**
    *    Less than comparison operator for sets of uint32.
    *    Replaces the slow built in method of set.
    */
   struct stlUint32SetsLessThan
   {
      bool operator()( const set<uint32>& set1, 
                       const set<uint32>& set2 )const
      {
         bool result = false;
         
         if ( set1.size() < set2.size() ){
            result = true;
         }
         else if ( set1.size() > set2.size() ){
            result = false;
         }
         else {
            // The sets have equal size;
            
            bool differs = false;
            
            set<uint32>::const_iterator it1 = set1.begin();
            set<uint32>::const_iterator it2 = set2.begin();
            while ( ( ! differs ) && ( it1 != set1.end() ) ){
               differs = !(*it1 == *it2);
               
               if (!differs){
                  ++it1;
                  ++it2;
               }
            }
            if (differs){
               if (*it1 < *it2){
                  result = true;
               }
               else{
                  result = false;
               }
            }
            else {
               result = false;
            }
         }
         return result;
      }
      
   }; // stlUint32SetsLessThan
}; // anonymous

uint32
GMSMap::mergeStreetsWithSameSSIs()
{
   // Removes all but one of all streets including exactly the same SSIs
   // and add names from all streets removed to the remaining one.
   uint32 removedStreets = 0;

   // The stlUint32SetsLessThan could be un-necessary.
   //      I think set has operator<
   map< set<uint32>, OldItem*, stlUint32SetsLessThan > streetBySSIs;
   set<uint32> streetIDsToRemove;
   for(uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z, i);
         if ( (item != NULL) && 
              (item->getItemType() == ItemTypes::streetItem ) )
         {
            OldStreetItem* street = dynamic_cast<OldStreetItem*>(item);
            if ( street == NULL ) {
               mc2log << error << here << " OldStreetItem is NULL, exits!" 
                      << endl;
               exit(1);
            }
            mc2dbg8 << "   Street to remove:" <<  street->getID() 
                   << endl;
            mc2dbg8 << "   Included SSI IDs:" << endl;

            // Get all SSIs of this street and put it in a set.
            set<uint32> ssiSet;
            for (uint32 i=0; i<street->getNbrItemsInGroup(); i++){
               uint32 ssiID = street->getItemNumber(i);
               OldStreetSegmentItem* ssiItem = 
                  dynamic_cast<OldStreetSegmentItem*>(this->itemLookup(ssiID));
               if ( ssiItem == NULL ){
                  mc2log << error << here << " SSIItem is NULL, exits!" 
                         << endl;
                  exit(1);
               }
               if ( ssiItem->getItemType() == 
                    ItemTypes::streetSegmentItem ){
                  ssiSet.insert(ssiID);
                  mc2dbg8 << "      " << ssiID << endl;
               }

            }

            pair< map<set <uint32>, OldItem* >::iterator, bool> result = 
               streetBySSIs.insert( make_pair(ssiSet, street) );
            if ( ! result.second ){
               // This SSI-set is already present in the STL:map, add the 
               // names of this street to the street in the STL:map with
               // the same SSIs.

               // Get some info for debugging.
               uint32 firstSSI = MAX_UINT32;
               if ( result.first->first.size() != 0 ){
                  firstSSI = *(result.first->first.begin());
               }

               if ( street->getNbrNames() != 1 ){
                  mc2log << error << here << " OldStreetItem number of names "
                         << "!= 1, nbrNames == "
                         << street->getNbrNames() << endl;
                  exit(1);
               }
               
               // Debug print:
               DEBUG8 (
               mc2dbg8 << "   Matched SSI IDs:" << endl;
               set <uint32> matchedSSIs = (result.first)->first;
               for ( set <uint32>::const_iterator ssiIt = 
                        matchedSSIs.begin();
                     ssiIt != matchedSSIs.end();
                     ++ssiIt )
               {
                  mc2dbg8 << "      " << *ssiIt << endl;
               });
                
               
               
               
               LangTypes::language_t langType;
               ItemTypes::name_t nameType;
               uint32 strIndex;
               street->getNameAndType(0, langType, nameType, strIndex);

               map< set< uint32 >, OldItem* >::iterator streetInMapIt = 
                  result.first;
               this->addNameToItem( streetInMapIt->second, 
                                    this->getName(strIndex), 
                                    langType, 
                                    nameType);
 
               // Since its name has been transferred to another street
               // we now can remove teh street from this map.
               uint32 removeID = street->getID();
               mc2dbg4 << "To remove street:" << removeID 
                       << ":" << getFirstItemName(street->getID()) 
                       <<  " (first SSI:" << firstSSI << ")" 
                       << " because street " 
                       << streetInMapIt->second->getID() <<  endl;
               streetIDsToRemove.insert(removeID);
               removedStreets++;
            }
         } 
      }   // for item in zoom level
   }   // for zoomlevel


   // Remove the collected IDs
   mc2dbg << "To remove the collected streets size=" 
          << streetIDsToRemove.size() << endl;
   this->removeItems(streetIDsToRemove);
   
   return removedStreets;


} // mergeStreetsWithSameSSIs()



uint32
GMSMap::splitStreets(GMSStreetItem* street, 
                     vector<u32set>& theSplitStreets)
{

   // See NationalProperties::getStreetMergeSqDistfor which distaces that
   // are used for street merging.

   uint32 nbrItems = street->getNbrItemsInGroup();
   
   uint32 nbrStreets = 0;
   
   // Build the hashtable to the map if it isn't done yet
   if (m_hashTable == NULL)
      buildHashTable();
   
   for (uint32 i = 0; i < nbrItems; i++) {
      GMSStreetSegmentItem* ssi =
         static_cast<GMSStreetSegmentItem *>
         (itemLookup(street->getItemNumber(i)));
      GfxData* gfx = ssi->getGfxData();

      // Find the vector to add this street to.
      // -------------------------------------------------------------
      vector<u32set>::iterator it = theSplitStreets.begin();
      bool present = false;
      while ( (it != theSplitStreets.end()) && (!present)) {
         if ( it->find( ssi->getID() ) != 
              it->end() ) {
            // Found!
            present = true;
         } else {
            ++it;
         }
      }
      if (!present) {
         // Create a new street vector
         u32set streets;
         streets.insert( ssi->getID() );
         theSplitStreets.push_back( streets );
         it = theSplitStreets.begin() + nbrStreets;
         nbrStreets++;
      }

      // Check if this ssi is close to any other ssi's. If so, add them
      // to the street.
      // --------------------------------------------------------------
      
      for (uint32 k = 0; k < nbrItems; k++) {
         if (i != k) {
            GMSStreetSegmentItem* otherSSI =
               static_cast<GMSStreetSegmentItem *>
               (itemLookup(street->getItemNumber(k)));
            GfxData* otherGfx = otherSSI->getGfxData();

            // Pick roadclass dependent distance to use.
            uint32 highestRoadClass = MAX(otherSSI->getRoadClass(),
                                          ssi->getRoadClass() );
            float64 maxSqDist = NationalProperties::getStreetMergeSqDist(
                                                    this->getCountryCode(),
                                                    highestRoadClass );
            if ( maxSqDist == MAX_FLOAT64 ){
               mc2log << error << "GMSMap::splitStreets"
                      << " Failed to get merge distance for ssi"
                      << " with ID: " << ssi->getID() 
                      << " and otherSSI with ID: " <<otherSSI->getID()
                      << endl;
               exit(1);
            }
            

            // Check if ssi and otherSSI is close to another.
            //
            // Get bounding boxes
            MC2BoundingBox bbox1;
            gfx->getMC2BoundingBox(bbox1);
            MC2BoundingBox bbox2;
            otherGfx->getMC2BoundingBox(bbox2);
            // Calculate distance. ( +1 to make sure the distance is not to
            // small )            
            uint32 meterDist = uint32(sqrt(maxSqDist))+1;
            // Expand one of the boxes.
            bbox2.increaseMeters(meterDist+1);
            if ( bbox2.overlaps(bbox1) ){
               // streets are close enough to check distance
               if (gfx->minSquareDistTo(otherGfx) <= maxSqDist) {
                  // streets are close enough
                  it->insert( otherSSI->getID() );
               }
            }
         }
      } 
   }  // for (i < nbrItems)
   
   // Merge streets with common streetsegments
   // -----------------------------------------------------------------
   
   // There may be streets that we have split, but that really have
   // common streetsegments. Therefore we need to do this merge.
   // It is done in this way in order to speed up the process.
   // Merge the streets until no more streets have been merged.
   while (mergeStreets(theSplitStreets));
      
   // Merge streets lying on a line.
   if ( getCountryCode() == StringTable::USA_CC ) {
      mergeStreetsOnLine( theSplitStreets );
   }

   // return the number of allocated streets in the arrray, 
   return (nbrStreets);

}

bool
GMSMap::mergeStreets(vector<u32set>& theSplitStreets)
{
   bool merged = false;
  
   // Merge
   for ( vector<u32set>::iterator i = theSplitStreets.begin();
         i != theSplitStreets.end(); ++i ) {
         
      for ( vector<u32set>::iterator j = i + 1; 
            j != theSplitStreets.end(); ++j ) {
         u32set result;

         set_intersection( i->begin(), i->end(), 
                           j->begin(), j->end(), 
                           inserter(result, result.begin()) );
         if (! result.empty() ) {
            // Common elements were found. Merge!
            merged = true;
            result.clear();
            merge( i->begin(), i->end(), 
                   j->begin(), j->end(), 
                   inserter(result, result.begin()) );
            // Add the merged result to (*i).
            i->clear();
            i->insert( result.begin(), result.end() );
            j->clear();
         }
      }
   }

   // Return whether we merged any streets or not.
   return (merged);
}


bool
GMSMap::mergeStreetsOnLine( vector<u32set>& theSplitStreets )
{
   if ( theSplitStreets.size() < 2 ) {
      return false;
   }
   
   typedef pair<int32, int32> i32pair;
   typedef multimap<i32pair, uint32> nodesByCoord_t;
   nodesByCoord_t nodesByCoord;
   
   map<uint32, uint32> streetIdxByNode;
   
   uint32 idx = 0;
   for ( vector<u32set>::const_iterator it = theSplitStreets.begin();
         it != theSplitStreets.end(); ++it ) {
      
      for ( u32set::const_iterator jt = it->begin(); 
            jt != it->end(); ++jt ) {
      
         OldItem* item = itemLookup( *jt );
         GfxData* gfx = item->getGfxData();

         // OldNode 0
         uint32 node0 = item->getID();
         nodesByCoord.insert( 
            make_pair<i32pair, uint32>  ( 
               i32pair( gfx->getLat( 0, 0 ), gfx->getLon( 0, 0 ) ), 
               node0 ) );
         
         // OldNode 1
         uint32 node1 = TOGGLE_UINT32_MSB( item->getID() );
         nodesByCoord.insert( 
            make_pair<i32pair, uint32>  ( 
               i32pair( gfx->getLastLat( 0 ), gfx->getLastLon( 0 ) ), 
               node1 ) );
         
         // Store which index in theSplitStreets these nodes have.
         streetIdxByNode[ node0 ] = idx;
         streetIdxByNode[ node1 ] = idx;
      }
      ++idx;
   }

   u32set endNodes;
   for ( nodesByCoord_t::const_iterator it =
            nodesByCoord.lower_bound( i32pair( MIN_INT32, MIN_INT32 ) );
         it != nodesByCoord.end(); 
         it = nodesByCoord.upper_bound( it->first ) ) {
      
      // Collect all nodes at the ends, i.e. they are the only ones with
      // a certain coordinate.
      if ( nodesByCoord.count( it->first ) == 1 ) {
         endNodes.insert( it->second );
         mc2dbg8 << "Adding endnode " << it->second << endl;
      } else {
         mc2dbg8 << "Skipping nonendnode " << it->second << endl;
      }
   }
   
   mc2dbg8 << "endNodes.size() == " << endNodes.size() << endl;

   u32map streetTable;
   for ( uint32 i = 0; i < theSplitStreets.size(); ++i ) {
      if ( theSplitStreets[i].empty() ) {
         mc2dbg8 << i << " is empty" << endl;
      }
      streetTable[ i ] = i;
   }
   
   for ( u32set::const_iterator it = endNodes.begin();
         it != endNodes.end(); ++it ) {
      u32set::const_iterator jt = it;
      for ( ++jt ; jt != endNodes.end(); ++jt ) {
         if ( streetTable[ streetIdxByNode[ *it ] ] != 
              streetTable[ streetIdxByNode[ *jt ] ] ) {
            if ( sameStreet( *it, *jt ) ) {
               mc2dbg8 << "sameStreet returned true" << endl;
               mc2dbg8 << "Merging " << streetIdxByNode[ *it ] << " with "
                      << streetIdxByNode[ *jt ] << endl;
               uint32 oldIdx = streetTable[ streetIdxByNode[ *jt ] ];
               uint32 newIdx = streetTable[ streetIdxByNode[ *it ] ];
               for ( u32map::iterator kt = streetTable.begin(); 
                     kt != streetTable.end(); ++kt ) {
                  if ( kt->second == oldIdx ) {
                     kt->second = newIdx;
                  }
               }
            }
         }
      }
   }

   uint32 ii = 0;
   for ( u32map::const_iterator it = streetTable.begin();
         it != streetTable.end(); ++it ) {
      mc2dbg8 << "streetsTable[" << it->first << "] = " << it->second 
              << endl;
      ++ii;
   }

   // Reverse the street table
   u32mmap reverseStreetTable;
   
   for ( u32map::const_iterator it = streetTable.begin();
         it != streetTable.end(); ++it ) {
      reverseStreetTable.insert( 
            make_pair<uint32, uint32>( it->second, it->first ) );
   }

   vector<u32set> theMergedStreets;
   for ( u32mmap::const_iterator it = reverseStreetTable.lower_bound( 0 );
         it != reverseStreetTable.end(); 
         it = reverseStreetTable.upper_bound( it->first ) ) {
      pair<u32mmap::const_iterator, u32mmap::const_iterator> rangeIt =
         reverseStreetTable.equal_range( it->first );
      u32set streets;
      mc2dbg8 << " street: " << it->first << endl;
      for ( u32mmap::const_iterator jt = rangeIt.first; 
            jt != rangeIt.second; ++jt ) {
         mc2dbg8 << " " << jt->second << endl;
         streets.insert( theSplitStreets[ jt->second ].begin(),
                         theSplitStreets[ jt->second ].end() );
      }
      if (! streets.empty() ) {
         theMergedStreets.push_back( streets );
      }
   }
   
   mc2dbg8 << "theSplitStreets.size() = " << theSplitStreets.size()
           << ", theMergedStreets.size() = " << theMergedStreets.size()
           << endl;
   theSplitStreets = theMergedStreets;
   
   // Return if we merged any streets or not.
   return (true);
}


bool
GMSMap::sameStreet( uint32 firstNodeID, uint32 secondNodeID )
{
   
 /* Checks if the geometry of the two nodes make it probable that
    both belong to the same street.
   
    For instance:
   
     \ 
      \ firstNode
       o
   
            
            o secondNode
             \ 
              \ 
               \ 
    
    Two things are checked. First the angle between the last segment
    of the nodes should not differ to much.
    Secondly the perpendicular distance between the two (almost)
    parallell lines should not be too far.
   
   
     first node is point x0,y1; vector v; v going trough point x0,y1 is L1
          | x0,y0
    ------x--------------- L1
          |L3
          |
    ------x-------x------- L2
    x3,y3 |      x1,y1
          
    second node is point x1,y1; v going through x1,y1 is L2
    L1 is parallell with L2
    
    Intersection of L3 with L2 occurs at point x3,y3.
   
    Perpendicular vector to v is w. (v * w = 0)
    w going through x0, y0 is L3.
   
    
    v = (alpha, beta)
    w * v = 0 => w = (-beta, alpha)
   
    L1 : x = x0 + t*alpha
         y = y0 + t*beta
   
    L2 : x = x1 + t*alpha
         y = y1 + t*beta
   
    L3 : x = x0 + s*(-beta)
         y = y0 + s*alpha
    
    Calculate intersection of L3 and L2.
    Solve the following equation system:
   
    x1 + t*alpha = x0 + s*(-beta)
    y1 + t*beta  = y0 + s*alpha
   
    Solution is :
         (x0 - x1)*alpha + (y0 - y1)*beta
    t =  --------------------------------
             alpha*alpha + beta*beta
   
    Gives us x3, y3 point as:
   
    x3 = x1 + alpha*t
    y3 = y1 + beta*t
   
    The distance between (x0,y0), (x3,y3) gives us the perpendicular
    distance between the two parallell lines.
    
    Note that v is calculated by looking at the segment at first node.
    Then if the direction of the segment of the other node is almost
    the same, then the two lines are considered parallell, and the
    distance between them are measured.
   */

   OldItem* item1 = itemLookup( firstNodeID & 0x7fffffff );
   OldItem* item2 = itemLookup( secondNodeID & 0x7fffffff );

   mc2dbg8 << "sameStreet: " << getFirstItemName( item1 ) << endl;
   mc2dbg8 << "Checking " << firstNodeID << " and " << secondNodeID << endl;
   
   // Perform checks of indata.
   if ( item1 == NULL || item1->getGfxData() == NULL || 
        item1->getGfxData()->getNbrCoordinates( 0 ) < 2 ||
        item2 == NULL || item2->getGfxData() == NULL ||
        item2->getGfxData()->getNbrCoordinates( 0 ) < 2 ) {
      mc2dbg8 << "Check of indata failed" << endl;
      return false;
   }

   GfxData* gfx1 = item1->getGfxData();
   GfxData* gfx2 = item2->getGfxData();
  

   // Coordinates for the nodes and their respective previous coordinates
   // in the GfxData.
   int32 x0, y0, x0prev, y0prev, x1, y1, x1prev, y1prev;
  
   if ( MapBits::isNode0( firstNodeID ) ) {
      y0 = gfx1->getLat( 0, 0 );
      x0 = gfx1->getLon( 0, 0 );
      y0prev = gfx1->getLat( 0, 1 );
      x0prev = gfx1->getLon( 0, 1 );
   } else {
      y0 = gfx1->getLastLat( 0 );
      x0 = gfx1->getLastLon( 0 );
      y0prev = gfx1->getLat( 0, gfx1->getNbrCoordinates( 0 ) - 2 );
      x0prev = gfx1->getLon( 0, gfx1->getNbrCoordinates( 0 ) - 2 );
   }
   if ( MapBits::isNode0( secondNodeID ) ) {
      y1 = gfx2->getLat( 0, 0 );
      x1 = gfx2->getLon( 0, 0 );
      y1prev = gfx2->getLat( 0, 1 );
      x1prev = gfx2->getLon( 0, 1 );
   } else {
      y1 = gfx2->getLastLat( 0 );
      x1 = gfx2->getLastLon( 0 );
      y1prev = gfx2->getLat( 0, gfx1->getNbrCoordinates( 0 ) - 2 );
      x1prev = gfx2->getLon( 0, gfx1->getNbrCoordinates( 0 ) - 2 );
   }
  
   // Make sure that the coordinates are not too far from each other.
   // A limit of 150 km is used here.
   if ( GfxUtility::squareP2Pdistance_linear( y0, x0, y1, x1 ) >
        SQUARE(150000.0) ) {
      mc2dbg8 << "sameStreet: distance between points too far." << endl;
      return false;
   }
   
   // Calculate angle between the two segments by using scalar product.
   
   int32 x0diff = x0 - x0prev;
   int32 y0diff = y0 - y0prev;
   int32 x1diff = x1 - x1prev;
   int32 y1diff = y1 - y1prev;

   if ( ( x0diff == 0 && y0diff == 0 ) ||
        ( x1diff == 0 && y1diff == 0 ) ) {
      mc2log << warn << "Either of the vectors are 0." << endl;
      return false;
   }
   
   // cos(angle) = v1*v2 / |v1|*|v2|
   float64 cosangle = ( x0diff * int64( x1diff ) + 
                        y0diff * int64( y1diff ) ) /
                      ( sqrt( SQUARE( int64( x0diff ) ) + 
                              SQUARE( int64( y0diff ) ) ) *
                        sqrt( SQUARE( int64( x1diff ) ) + 
                              SQUARE( int64( y1diff ) ) ) );
  
   mc2dbg8 << "angle between vectors " << acos( cosangle ) * 360 / 2 / M_PI
          << " (fabs(cosangle) = " << fabs(cosangle) 
          << " and cos( M_PI / 6.0 ) = "
          << cos( M_PI / 6.0 ) << endl;
   
   // pi/6 = 30 degrees is max difference allowed for the vectors.
   if ( fabs(cosangle) < cos( M_PI / 6.0 ) ) {
      mc2dbg8 << "angle is too large" << endl;
      return false;
   }

   float64 alpha = x0diff;
   float64 beta = y0diff;
   float64 t = ( ( x0 - x1 ) * alpha + ( y0 - y1 ) * beta ) /
               float64( SQUARE( alpha ) + SQUARE( beta ) );
   
   float64 x3 = x1 + alpha * t;
   float64 y3 = y1 + beta * t;
  
   mc2dbg8 << "x0 = " << x0 << ", y0 = " << y0 << endl;
   mc2dbg8 << "x1 = " << x1 << ", y1 = " << y1 << endl;
   mc2dbg8 << "x3 = " << (int)x3 << ", y3 = " << (int)y3 << endl;
   mc2dbg8 << "alpha = " << alpha << ", beta = " << beta << ", t = " << t
          << endl;
   
   // Check distance between (x0,y0) and (x3,y3)

   float64 dist = sqrt( SQUARE( x3 - x0 ) + SQUARE( y3 - y0 ) );
   
   float64 maxDist = 250 * GfxConstants::METER_TO_MC2SCALE; // 250 meters
   mc2dbg8 << "distance is " << dist << endl;
   mc2dbg8 << "max distance is " << maxDist << endl;
   
   if ( dist < maxDist ) {
      mc2dbg8 << "distance close enough" << endl;
      return true;
   } else {
      mc2dbg8 << "distance not close enough" << endl;
      return false;
   }
}


void
GMSMap::sortStreetOrder(GMSStreetItem* street, 
                        u32set& ssiIDs, 
                        u32mmap& sortedSSIIDs)
{
   // Sort the itemsInGroup vector in the street so that the
   // the streetsegment that covers the least address range 
   // should be first in the vector.
   
   for ( u32set::iterator it = ssiIDs.begin();
         it != ssiIDs.end();
         ++it ) {
      OldStreetSegmentItem* ssi = static_cast<OldStreetSegmentItem*> 
            (itemLookup(*it));
      
      uint32 addressRange;
      if (ssi->getStreetNumberType() == ItemTypes::noStreetNumbers) {
         addressRange = 0;
      } else {
         addressRange = abs(ssi->getRightSideNbrEnd() - 
                            ssi->getRightSideNbrStart()) +
            abs(ssi->getLeftSideNbrEnd() - 
                ssi->getLeftSideNbrStart());
      }

      sortedSSIIDs.insert( pair<uint32, uint32>(addressRange, *it) );
   }
   
   // Now the container is sorted so that the smallest addressrange comes
   // first in the container.
}


class ZipCompareSort {
   public:
      bool operator()(pair<GMSStreetSegmentItem*, uint32> x, 
                      pair<GMSStreetSegmentItem*, uint32> y) {
         return StringUtility::strcasecmp(
                     x.first->getTmpZipCode(x.second),
                     y.first->getTmpZipCode(y.second)) < 0;
      }
};

OldZipCodeItem*
GMSMap::findZipCodeItem(const char* zipCode) 
{
   uint32 zipNameIndex = m_itemNames->stringExist(zipCode);
   if (zipNameIndex == MAX_UINT32) {
      // No item with that name!
      return NULL;
   }
   
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
         OldZipCodeItem* item = dynamic_cast<OldZipCodeItem*>(getItem(z, i));
         if (item != NULL) {
            for (uint32 n=0; n<item->getNbrNames(); ++n) {
               if (zipNameIndex == item->getStringIndex(n)) {
                  return item;
               }
            } //for n
         }
      } // for i
   } // for z

   return NULL;
}

bool
GMSMap::addSSIToZipCodesUsingGfx()
{
   // In case zipcodes are already added as attributes from
   // streetsegments we are not interested in creating zipcodes from
   // geometry. In that case remove all zipcodes that have zero
   // streetsegments.
   
   bool zipcodesAlreadyComplete = false;

   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; ++z) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); ++i) {
         GMSZipCodeItem* zip = dynamic_cast<GMSZipCodeItem*>
                        (getItem(z, i));
         if ( ( zip != NULL ) && ( zip->getNbrItemsInGroup() > 0 ) ) {
            zipcodesAlreadyComplete = true;
         }
      }
   }

   // Remove all zipcodes that doesn't have any streetsegments.
   if ( zipcodesAlreadyComplete ) {
      uint32 nbrRemoved = 0;
      for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; ++z) {
         for (uint32 i=0; i<getNbrItemsWithZoom(z); ++i) {
            GMSZipCodeItem* zip = dynamic_cast<GMSZipCodeItem*>
                           (getItem(z, i));
            if ( ( zip != NULL ) && ( zip->getGfxData() != NULL ) &&
                 ( zip->getNbrItemsInGroup() == 0 ) ) {
               mc2dbg1 << "GMSMap::addSSIToZipCodesUsingGfx - "
                       << "Complete zipcodes already found!" 
                       << " Removing incomplete zipcode "
                       << getFirstItemName( zip->getID() ) 
                       << " [" << zip->getID() << "]"
                       << endl;
               removeItem( zip->getID() );
               nbrRemoved++;
            }
         }
      }
      mc2log << info << "GMSMap::addSSIToZipCodesUsingGfx - "
             << "Complete zipcodes already found! "
             << nbrRemoved << " uncomplete zipcodes removed." << endl;
      return false;
   }
  
   // Everything is OK. Zipcodes not complete yet, so create them by
   // using the gfxdata.
   
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; ++z) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); ++i) {
         GMSZipCodeItem* zip = dynamic_cast<GMSZipCodeItem*>
                        (getItem(z, i));
         if ( ( zip != NULL ) && ( zip->getGfxData() != NULL ) &&
              ( zip->getNbrItemsInGroup() == 0 ) ) {
            GfxData* zipGfx = zip->getGfxData();
            //MC2BoundingBox zipBBox;
            //zip->getMC2BoundingBox( zipBBox );
            
            // Loop through all ssi:s and check if they are inside
            // the zipcode.
            for (uint32 sz=0; sz<NUMBER_GFX_ZOOMLEVELS; ++sz) {
               for (uint32 j=0; j<getNbrItemsWithZoom(sz); ++j) {
                  GMSStreetSegmentItem* ssi = 
                     dynamic_cast<GMSStreetSegmentItem*> (getItem(sz, j));
                  if ( ( ssi != NULL ) && 
                       ( ssi->getGfxData() != NULL ) &&
                       ( ssi->getGfxData()->getNbrCoordinates(0) > 0 ) ) {
                     GfxData* ssiGfx = ssi->getGfxData();
                     if ( ( zipGfx->insidePolygon( 
                              ssiGfx->getLat( 0, 0 ),
                              ssiGfx->getLon( 0, 0 ) ) > 0 ) ||
                          ( zipGfx->insidePolygon( 
                              ssiGfx->getLastLat( 0 ),
                              ssiGfx->getLastLon( 0 ) ) > 0 ) ) {
                        bindItemToGroup( ssi, zip );
                     }
                  }
               }
            }
         }
      }
   }

   return true;
}
      

uint32
GMSMap::createZipsFromGMSStreetSegmentItems() 
{
         
   // Store pairs of <"pointers to all GMSStreetSegments", zipOffset> in 
   // a vector and sort by zip-code
   vector< pair<GMSStreetSegmentItem*, uint32> > itemsWithZip;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; ++z) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); ++i) {
         GMSStreetSegmentItem* ssi = dynamic_cast<GMSStreetSegmentItem*>
                                                 (getItem(z, i));
         if (ssi != NULL)
            for (uint32 j=0; j<ssi->getNbrTmpZipCodes(); ++j)
               itemsWithZip.push_back( make_pair(ssi, j) );
      }
   }
   if ( itemsWithZip.size() == 0 ) {
      mc2dbg8 << "no temp zip codes to create zip code items from" << endl;
      return 0;
   }
   sort(itemsWithZip.begin(), itemsWithZip.end(), ZipCompareSort());

   // For each zip-code create a new OldZipCodeItem and bind the streets
   // to the group
   vector< pair<GMSStreetSegmentItem*, uint32> >::const_iterator it;
   const char* curZipCode = NULL;
   GMSZipCodeItem* curZipItem = NULL;
   uint32 nbrCreatedZipCodeItems = 0;
   for (it=itemsWithZip.begin(); it!=itemsWithZip.end(); ++it) {
      const char* tmpZipCode = (*it).first->getTmpZipCode( (*it).second);
      if ( (it == itemsWithZip.begin()) ||
           (StringUtility::strcasecmp(curZipCode, tmpZipCode) != 0)) {
         // New zip code, create new zip and add to that
         curZipCode = (*it).first->getTmpZipCode( (*it).second);
         mc2dbg1 << "New zip code, " << curZipCode << " ------------" << endl;
         curZipItem = dynamic_cast<GMSZipCodeItem*>
                                  (findZipCodeItem(curZipCode));
         if (curZipItem == NULL) {
            // Did not found any zip-code with this name, create a new!
            curZipItem = new GMSZipCodeItem();
            addItem(curZipItem, 10);
            // 
            addNameToItem(curZipItem, curZipCode, 
                          LangTypes::invalidLanguage,
                          ItemTypes::roadNumber);
            ++nbrCreatedZipCodeItems;
         }
      }
      // Continue with the same zip code, add ssi to curZipItem
      bindItemToGroup((*it).first, curZipItem);
      mc2dbg8 << "Binding " 
              << getFirstItemName( getRegionID( (*it).first, 
                                ItemTypes::builtUpAreaItem ))
              << "." << getFirstItemName((*it).first) << "(" 
              << (*it).first->getID() << ") to " 
              << getFirstItemName(curZipItem) << endl;
   }

   // Set GfxData (bbox) for all the OldZipCodeItems
   const uint32 z = 10;
   for (uint32 i=0; i<getNbrItemsWithZoom(z); ++i) {
      OldZipCodeItem* zip = dynamic_cast<OldZipCodeItem*>(getItem(z, i));
      if ((zip != NULL) && (zip->getNbrItemsInGroup() > 0)) {
         MC2BoundingBox bbox;
         if (zip->getGfxData() != NULL) {
            MC2BoundingBox gfxBBox;
            zip->getGfxData()->getMC2BoundingBox(gfxBBox);
            bbox.update(gfxBBox);
         }
         for (uint32 g=0; g<zip->getNbrItemsInGroup(); ++g) {
            OldItem* item = itemLookup(zip->getItemNumber(g));
            if ( (item != NULL) && (item->getGfxData() != NULL)) {
               MC2BoundingBox gfxBBox;
               item->getGfxData()->getMC2BoundingBox(gfxBBox);
               bbox.update(gfxBBox);
            }
         }
         // Make sure not to have any "line" bounding boxes.
         if ( bbox.getArea() == 0 ){
            bbox.increaseMeters(1); // add one meter to each side.
         }

         GfxDataFull* zipGfx = GMSGfxData::createNewGfxData(this, &bbox);
         zip->setGfxData(zipGfx);
      } else {
         mc2dbg8 << "zip == NULL, or no items in it" << endl;
      }
   }

   return nbrCreatedZipCodeItems;
}


void
GMSMap::updateMunicipalArray()
{
   Vector newMunVector(4);
   mc2log << info << "updateMunicipalArray" << endl;
   for (uint32 i=0; i< getNbrItemsWithZoom(0); i++) {
      OldItem* item = itemLookup(i);
      if ( (item != NULL) && 
           (item->getItemType() == ItemTypes::municipalItem) &&
           (item->getGfxData() != NULL)) {
         newMunVector.addLast(i);
      }
   }
   municipalOffset = newMunVector.getSize();
   delete [] municipalVector;
   if (municipalOffset > 0) {
      municipalVector = new uint32[municipalOffset];
   } else {
      municipalVector = NULL;
   }
   for (uint32 i=0; i<municipalOffset; i++) {
      municipalVector[i] = newMunVector.getElementAt(i);
   }
}

bool
GMSMap::removeEmptyLocationItems( ItemTypes::itemType itemType )
{
   if ( ( itemType != ItemTypes::builtUpAreaItem ) && 
        ( itemType != ItemTypes::municipalItem ) &&
        ( itemType != ItemTypes::zipCodeItem ) ) {
      return false;
   }

   set<uint32> foundLocations;
   uint32 nbrRemovedItems = 0;

   // Store all locations that has an item referring to them.
   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; ++z ) {
      for ( uint32 i = 0; i < getNbrItemsWithZoom( z ); ++i ) {
         OldItem* item = getItem( z, i );
         if ( item == NULL ) {
            continue;
         }
         if ( item->getItemType() == itemType ) {
            continue;
         }
         // For index areas, collect recursive, since
         // the items may have only the moste detailed index area
         // as group.
         bool recursive =
                ( NationalProperties::useIndexAreas(
                      getCountryCode(), getMapOrigin()) &&
                   (itemType == ItemTypes::builtUpAreaItem) );
         vector<OldItem*> groups = getRegions(item, itemType, recursive);
         for ( uint32 g=0; g<groups.size(); g++){
            foundLocations.insert( groups[g]->getID() );
         }
         
      }
   }

   // Remove location items that doesn't have anyone referring to them.
   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; ++z ) {
      for ( uint32 i = 0; i < getNbrItemsWithZoom( z ); ++i ) {
         OldItem* item = getItem( z, i );
         if ( ( item != NULL ) && 
              ( item->getItemType() == itemType ) &&
              ( !itemNotInSearchIndex(item->getID()) ) ) {
            // If itemNotInSearchIndex, this is not a location item, but for
            // instance a BUA only used for display.
            // So we want to keep it, even if it has no items referring to it

            // Check if someone has this item as location.
            if ( foundLocations.find( item->getID() ) == 
                 foundLocations.end() ) {
               mc2dbg1 << "GMSMap::removeEmptyLocationItems : Removing "
                      << item->getItemTypeAsString() << " "
                      << getFirstItemName( item ) 
                      << " " << item->getID() << endl;
               removeItem( item->getID() );
               nbrRemovedItems++;
            }
         }
      }
   }
     
   // Update municipal array in case of municipal item.
   if ( itemType == ItemTypes::municipalItem ) {
      updateMunicipalArray();
   }
   
   mc2dbg << "GMSMap::removeEmptyLocationItems removed "
          << nbrRemovedItems 
          << " " << ItemTypes::getItemTypeAsString(itemType)
          << " items" << endl;
   return true;
}

bool 
GMSMap::removeDuplicatedStreetNames()
{
   bool result = true;
   uint32 namesRemoved = 0;
   
   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; ++z ) {
      for ( uint32 i = 0; i < getNbrItemsWithZoom( z ); ++i ) {
         OldItem* item = getItem( z, i );
         if ( ( item == NULL ) ||
              ( item->getItemType() != ItemTypes::streetSegmentItem ) ) 
         {
            continue;
         }
         // When we get here item is a street segment item.
         for ( int32 n1=item->getNbrNames()-1; n1>=0; n1--){
            set<uint32> offsetsToRemove;

            uint32 strIdx1 = item->getStringIndex(n1);
            const char* name1 = this->getName(strIdx1);
            LangTypes::language_t lang1 = item->getNameLanguage(n1);

            int32 n2 = n1-1;
            bool equalNames = false;
            while( ( n2 >= 0 ) && ( !equalNames ) ){
               LangTypes::language_t lang2 = item->getNameLanguage(n2);

               if ( lang1 != lang2 ){
                  // Not comparing names of different languages.
                  n2--;
                  continue;
               }


               uint32 strIdx2 = item->getStringIndex(n2);
               const char* name2 = this->getName(strIdx2);
               
               equalNames = (strcmp( name1, name2 ) == 0 );
               if ( equalNames ){
                  if ( item->getNameType(n1) != ItemTypes::officialName ){
                     // Checked name is not an official name, remove it.
                     offsetsToRemove.insert(n1);
                  }
                  else if (item->getNameType(n2) == 
                           ItemTypes::officialName ){
                     // Both checked name and the name checked against are
                     // official names. Remove checked name.
                     offsetsToRemove.insert(n1);
                  }
                  else {
                     // Checked name is an official name and the name 
                     // checked against is not, remove name checked 
                     // against.
                     offsetsToRemove.insert(n2);
                  }
               }      
               n2--;
            }

            // Remove names of this item starting at the back of the set.
            set<uint32>::const_reverse_iterator rOffIt = 
               offsetsToRemove.rbegin();
            while ( rOffIt != offsetsToRemove.rend() ){

               const char* nameTypeStr =    // Save name type for print.
                  ItemTypes::getNameTypeAsString(
                             item->getNameType(*rOffIt));

               result = item->removeNameWithOffset(*rOffIt);

               if (!result){
                  mc2log << error << "GMSMAP::removeDuplicatedStreetNames"
                         << " Could not remove name with offset " << n1
                         << " from item with ID " << item->getID() << endl;
                  return result;
               }
               else {
                  mc2dbg2 << "GMSMAP::removeDuplicatedStreetNames"
                         << " Removed name:\"" << name1 << "\" of item "
                         << item->getID() << " nameType:" << nameTypeStr 
                         << endl;
                  namesRemoved++;
               }
               rOffIt++;
            }
         }
      }
   }
   
   mc2log << info << "GMSMap::removeDuplicatedStreetNames removed " 
          << namesRemoved << " names." << endl;

   return result;
} // removeDuplicatedStreetNames

bool
GMSMap::initTurnDescriptions(uint32 itemID)
{
   OldItem* item;
   OldStreetSegmentItem* ssi;
   mc2dbg1 << "initTurnDescriptions for map:"<< this->getMapID()
           << endl;
   int nbrcross = 0;

   if (itemID != MAX_UINT32) {
      // Set turndescriptions for just this item (if it is a ssi)
      mc2dbg1 << "setting turn description for ssi " << itemID << endl;
      OldStreetSegmentItem* ssi = 
         dynamic_cast<OldStreetSegmentItem*>(itemLookup(itemID));
      if ( (ssi != NULL) &&
           (ssi->getItemType() == ItemTypes::streetSegmentItem) &&
           (ssi->getGfxData() != NULL)) {
         
         OldNode* node0 = ssi->getNode(0);
         OldNode* node1 = ssi->getNode(1);
         if (static_cast<GMSNode*>(node0)->turnUnSet())
         {
            Crossing newCrossing(node0, this);
            newCrossing.setTurnDescriptions();
            nbrcross++;
            newCrossing.setRoundaboutCrossingKind();
         }
         if (static_cast<GMSNode*>(node1)->turnUnSet())
         {
            Crossing newCrossing(node1, this);
            newCrossing.setTurnDescriptions();
            nbrcross++;
            newCrossing.setRoundaboutCrossingKind();
         }
      }
      
   } else {
      // Set turndescriptions for all nodes
      mc2dbg1 << "setting turn description for the whole map" << endl;
      
      // Correct multidig for roundabout connections.
      mc2dbg1 << "Correct multidig for roundabout connections." << endl;
      for(uint32 t=0; t < NUMBER_GFX_ZOOMLEVELS; t++)
      {
         for(uint32 u = 0; u < getNbrItemsWithZoom(t); u++)
         {
            item = getItem(t, u);
            if (  (item != NULL) && 
                  (item->getItemType() == ItemTypes::streetSegmentItem) &&
                  (item->getGfxData() != NULL) )
            {
               ssi = (OldStreetSegmentItem*) item;
               OldNode* node0 = ssi->getNode(0);
               OldNode* node1 = ssi->getNode(1);
               OldNode* node = NULL;
                  
               // Could this ssi be of intrest
               // its not multi, rndb, ramp or ctrlAcc (should it be multi)
               if((node0 != NULL) && (node1 != NULL) &&
                  !ssi->isRoundabout() && !ssi->isRamp() &&
                  !ssi->isMultiDigitised() && !ssi->isControlledAccess()){

                  // is it one way?
                  if((node0->getEntryRestrictions() == ItemTypes::noWay) &&
                     (node1->getEntryRestrictions() != ItemTypes::noWay))
                     node = node0;
                  else if((node1->getEntryRestrictions() == ItemTypes::noWay)
                          &&
                          (node0->getEntryRestrictions() != ItemTypes::noWay))
                     node = node1;
               
                  if((node != NULL) && 
                     (node->getEntryRestrictions() == ItemTypes::noWay)){
                     
                     Crossing firstCrossing(node, this);
                     if(firstCrossing.partOfRoundabout()){
                        // Ok we have found a true oneway roundabout entry
                        // that is neither ramp nor multi.
                        // Lets see where this leads.
                        OldNode* startNode = node;
                        bool search = true;
                        bool found = false;
                        bool leaveRbt = false;
                        int nbrCross = 0;
                        vector<OldStreetSegmentItem*> ssivector;
                        vector<OldStreetSegmentItem*>::iterator it;
                        while(search && !leaveRbt && (nbrCross < 10)){
                           OldNode* nextNode = nodeLookup(
                              TOGGLE_UINT32_MSB(node->getNodeID()));
                           Crossing nextCrossing(nextNode, this);
                           nbrCross++; // lets not do this forever.
                           node = nextCrossing.getNoEntryNode(leaveRbt);
                           if(node != NULL){
                              ssi = dynamic_cast<OldStreetSegmentItem*>
                                 (itemLookup(
                                    REMOVE_UINT32_MSB(node->getNodeID())));
                           } else {
                              ssi = NULL;
                           }
                           if((node == NULL) || (ssi == NULL)){
                              search = false;
                           } else {
                              if(node == startNode){
                                 found = true;
                                 search = false;
                              }
                              // dont set multi in the roundabout
                              if(!ssi->isRoundabout()){
                                 ssivector.push_back(ssi);
                              }
                           }
                        }
                        
                        if(found){
                           // Set the multidig atribute for the segments
                           for(it = ssivector.begin() ; it != ssivector.end();
                               it++){
                              static_cast<OldStreetSegmentItem*>
                                 (*it)->setMultiDigitisedValue(true);
                           }
                           cerr << "Correcting multidig at node : "
                                << startNode->getNodeID() << endl;
                        }
                        ssivector.clear();
                     }
                  }
               }
            }
         }
      }
      
                     
      

      
      mc2dbg1 << "Set TD" << endl;
      for(uint32 t=0; t < NUMBER_GFX_ZOOMLEVELS; t++)
      {
         for(uint32 u = 0; u < getNbrItemsWithZoom(t); u++)
         {
            item = getItem(t, u);
            if (  (item != NULL) && 
                  (item->getItemType() == ItemTypes::streetSegmentItem) &&
                  (item->getGfxData() != NULL) )
            {
               

               ssi = (OldStreetSegmentItem*) item;
               OldNode* node0 = ssi->getNode(0);
               OldNode* node1 = ssi->getNode(1);
               if (static_cast<GMSNode*>(node0)->turnUnSet())
               {
                  Crossing newCrossing(node0, this);
                  newCrossing.setTurnDescriptions();
                  nbrcross++;
                  newCrossing.setRoundaboutCrossingKind();
               }
               if (static_cast<GMSNode*>(node1)->turnUnSet())
               {
                  Crossing newCrossing(node1, this);
                  newCrossing.setTurnDescriptions();
                  nbrcross++;
                  newCrossing.setRoundaboutCrossingKind();
               }
            }
            else if((item != NULL) && 
                    (item->getItemType() == ItemTypes::ferryItem)&&
                    (item->getGfxData() != NULL))
            {
               OldFerryItem* fItem = static_cast<OldFerryItem*>(item);
               OldNode* node0 = fItem->getNode(0);
               OldNode* node1 = fItem->getNode(1);
               if (static_cast<GMSNode*>(node0)->turnUnSet())
               {
                  Crossing::getToFerryItemTurndescriptions(node0, this);
                  nbrcross++;
               }
               if (static_cast<GMSNode*>(node1)->turnUnSet())
               {
                  Crossing::getToFerryItemTurndescriptions(node1, this);
                  nbrcross++;
               }
               
            }
            
         }
      }
   } // Set turndescriptions for all nodes
   return true;
}


int
GMSMap::setCityPartLocation()
{
   int nbrLocationsSet = 0;


   // Set logical location for the all the "other" items in the city
   // parts (Items with type != Municipal, BuiltUpArea and CityPart
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++){
      // For all items
      for (uint32 i = 0; i <getNbrItemsWithZoom(z); i++){
         OldItem* curItem = getItem(z, i);
        
         if ((curItem != NULL) &&
             (curItem->getItemType() != ItemTypes::municipalItem) &&
             (curItem->getItemType() != ItemTypes::builtUpAreaItem) &&
             (curItem->getItemType() != ItemTypes::cityPartItem) &&
             (getRegionID( curItem, ItemTypes::builtUpAreaItem ) != 
              MAX_UINT32 ) &&
             (curItem->getGfxData() != NULL)) {
            
            // Got an item to set the city-part location for
            // (it is located in a bua)
            OldBuiltUpAreaItem* bua = static_cast<OldBuiltUpAreaItem*>
               (getRegion( curItem, ItemTypes::builtUpAreaItem ) );
            uint32 j=0;
            
            OldCityPartItem* curCityPart = NULL;
            while ( (bua != NULL) &&
                    (j < bua->getNbrItemsInGroup() )) {
               uint32 curItemID = bua->getItemNumber(j);
               curCityPart = 
                  dynamic_cast<OldCityPartItem*>(itemLookup(curItemID));
               if (curCityPart != NULL) {
                  int32 lat = curItem->getGfxData()->getLat(0,0);
                  int32 lon = curItem->getGfxData()->getLon(0,0);
                  if (curCityPart->getGfxData()
                        ->insidePolygon(lat, lon) > 0) {
                     addRegionToItem( curItem, curCityPart );
                     nbrLocationsSet++;
                     j = MAX_UINT32-1; //To be sure to break the while-loop
                  }
               }
               j++;
            }
         }
      }
   }


   mc2dbg << "GMSMap::setCityPartLocation added " << nbrLocationsSet
          << " locations" << endl;

   return (nbrLocationsSet);
}


uint32
GMSMap::removeNonSearchIdxGroups(){

   // Make sure that no item is using a non-search-index area as location.
   // It's safe to do this now, since it's impossible to add non-search-index
   // groups with addRegionToItem.
   mc2dbg << "Removing non search index items as groups." << endl;
   uint32 nbrRemovedGroups = 0;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; ++z) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z, i);
         if (item == NULL) {
            continue;
         }
         for ( int32 g=item->getNbrGroups()-1; g>=0; g-- ){
            uint32 groupID = item->getGroup(g);
            if (itemNotInSearchIndex(groupID)){
               item->removeGroup(g);
               nbrRemovedGroups++;
            }
         }
      }
   }
   mc2dbg << "Removed " << nbrRemovedGroups << " non search index items as "
          << "groups." << endl;
   return nbrRemovedGroups;
}

void
GMSMap::setAllItemLocation(bool updateOnlyNonValid)
{
   mc2dbg << "To set itemLocation (municipalItem)" << endl;
   setItemLocation(ItemTypes::municipalItem, updateOnlyNonValid);

   mc2dbg << "To set itemLocation (builtUpAreaItem)" << endl;
   setItemLocation(ItemTypes::builtUpAreaItem, updateOnlyNonValid);

   mc2dbg << "To set itemLocation (cityPartItem)" << endl;
   setCityPartLocation();


   // Make sure metropolitan bua:s don't have any municipal location.
   // And let municipals inside metropolitan bua:s have bua location.
   mc2dbg << "Calling updateMetropolitanBuaMunLoc" << endl;
   uint32 nbrChanged = updateMetropolitanBuaMunLoc();
   mc2log << info << "[GM]: setAllItemLocation - updated " << nbrChanged
          << " metropolitan buas or municipals." << endl;
   
   mc2dbg << "All locations set" << endl;

}

bool
GMSMap::updateLocation(OldBuiltUpAreaItem* bua) 
{
   if (bua == NULL)
      return (false);
   
   GfxData* gfx = bua->getGfxData();
   if (gfx == NULL) {
      return (false);
   }
   
   // Go through all streetsegmentitems and set update their location
   // if they are inside this item.
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldStreetSegmentItem* curSSI = 
            dynamic_cast<OldStreetSegmentItem*> (getItem(z,i));
         if ((curSSI != NULL) && (curSSI->getGfxData() != NULL)) {
            // Check if the ssi is inside 
            if (gfx->insidePolygon(curSSI->getGfxData()->getLastLat(0),
                                   curSSI->getGfxData()->getLastLon(0))) {
               // Update location for the ssi.
               addRegionToItem( curSSI, bua );
            }
         }
      }
   }

   // Need to update all groups of ssis to the street !
   updateStreetLocationAll(ItemTypes::builtUpAreaItem);
   
   return (true);
   
}


void
GMSMap::setItemLocation(ItemTypes::itemType type,
                        bool onlyUpdateNonValid)
{
   Vector typeIDs(4, 4);
   multimap<uint32,uint32> indexAreasBasedOnOrder;
   set<uint32> indexAreaOrders;

   // Check all items at level 0 -- to find the ID of the items 
   // with correct type
   for (uint32 curItemID =0; curItemID < getNbrItemsWithZoom(0); curItemID++) {
      OldItem* curItem = itemLookup(curItemID);
      if ( (curItem != NULL) && (curItem->getItemType() == type)) {
         // Found one item! Just make sure it has GfxData also, and can be 
         // used as group.
         if ( !itemNotInSearchIndex(curItem->getID()) ) {
            if ( curItem->getGfxData() != NULL ) {
               typeIDs.addLast(curItemID);
               mc2dbg8 << "Found item (type=" << (uint32) type << ") with ID " 
                       << curItemID << endl;
            }
            uint32 indexAreaOrder = getIndexAreaOrder(curItem->getID());
            if ( indexAreaOrder < MAX_UINT32 ) {
               indexAreasBasedOnOrder.insert(
                  make_pair(indexAreaOrder,curItem->getID()) );
               indexAreaOrders.insert(indexAreaOrder);
            }
         }
      }
   }
   mc2dbg1 << "Totaly found " << typeIDs.getSize()
           << " items with type = " << (uint32) type << endl;
   if ( NationalProperties::useIndexAreas(
            getCountryCode(), getMapOrigin()) ) {
      mc2dbg1 << "Stored ids for " << indexAreasBasedOnOrder.size()
              << " index areas based on order" << endl;
      // Find pointer to the last key (index area order)
      for ( set<uint32>::const_reverse_iterator setIt = indexAreaOrders.rbegin();
            setIt != indexAreaOrders.rend(); setIt++ ) {
         mc2dbg1 << " Index area order " << *setIt
                 << " has " << indexAreasBasedOnOrder.count(*setIt) 
                 << " items" << endl;
      }
   }
 
   bool dealingWithIndexAreas = 
         (type == ItemTypes::builtUpAreaItem &&
          NationalProperties::useIndexAreas(getCountryCode(),
                                            getMapOrigin()) );
   // Set the type in all the items
   uint32 nbrLocationsSet = 0;
   for ( int z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      mc2dbg4 << "Entering zoomlevel " << z << endl;
      for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
         OldItem* curItem = getItem(z, i);
         
         // Street items are handled further down seperately.
         if ((curItem != NULL) && 
             (curItem->getItemType() != ItemTypes::streetItem)) {
         
            bool setLocation = true;
            if (onlyUpdateNonValid) {
               // Only update location if the location is not already
               // valid
               if ( getRegionID( curItem, type ) != MAX_UINT32 ) {
                  setLocation = false;
               }
            }
           
            // Don't handle municipals, buas or zipcodes.
            // Will be set by updateMetropolitanBuaMunLoc.
            if ( curItem->getItemType() == ItemTypes::municipalItem || 
                 curItem->getItemType() == ItemTypes::builtUpAreaItem ||
                 curItem->getItemType() == ItemTypes::zipCodeItem ) {
               setLocation = false;
            }

            if ( itemNotInSearchIndex(curItem->getID()) ){
               setLocation = false;
            }
            
            if (setLocation) {
               GfxData* curGfx = curItem->getGfxData();
               if (curGfx != NULL) {
                  // This item has an geographical representation
                  uint32 lastCoordIdx = curGfx->getNbrCoordinates(0)-1;
                  mc2dbg8 << i << "   lastCoordIdx=" << lastCoordIdx << endl;
                  if (curItem->getItemType() != type) {
                     
                     bool findByGfxData = false;
                     if ( (typeIDs.getSize() == 0) ||
                          dealingWithIndexAreas ) {
                        //There were no items with type "type" with gfxData
                        //(e.g. mun in England have no gfxData)
                        
                        // OR

                        // When using index areas, we don't want to rely on 
                        // their geometry, and therefore always use the 
                        // location of closest SSI.

                       
                        // Grab the hash table
                        OldMapHashTable* mht = getHashTable();
                        MC2_ASSERT(mht != NULL);
                        mht->clearAllowedItemTypes();
                        mht->addAllowedItemType(
                              ItemTypes::streetSegmentItem);
                        // Find closest street segment
                        int32 lat = curGfx->getLat(0,0);
                        int32 lon = curGfx->getLon(0,0);
                        uint64 dist;
                        uint32 nearestSSI = mht->getClosest(lon, lat, dist);
                        mc2dbg8 << "Found close ssi, dist = " 
                                << dist << endl;
                        if (nearestSSI != MAX_UINT32) {
                           OldItem* ssi = itemLookup(nearestSSI);
                           MC2_ASSERT(ssi != NULL);
                           // Copy ssi location to curItem
                           uint32 regionIDtoAdd = MAX_UINT32;
                           if (dealingWithIndexAreas) {
                              // Find the index area group of ssi
                              // that has the highest index area order
                              uint32 mostDetailedID =
                                 getMostDetailedIndexArea(
                                    getRegions(ssi,type),
                                    indexAreasBasedOnOrder,
                                    indexAreaOrders);
                              if ( mostDetailedID != MAX_UINT32 ) {
                                 regionIDtoAdd = mostDetailedID;
                              } else {
                                 //findByGfxData = true;
                                 // but this has not been implemented
                                 mc2log << warn 
                                        << "No idx area "
                                        << "for curItem=" << curItem->getID()
                                        << " from nearestSSI=" << nearestSSI
                                        << endl;
                                 // perhaps a part of the country that was not
                                 // covered by idx areas, 
                                 // simply, copy ssi location to curItem
                                 regionIDtoAdd = getRegionID( ssi, type );
                                 mc2log << warn 
                                        << "Will use the normal ssi region "
                                        << regionIDtoAdd
                                        << endl;
                              }
                              mc2dbg8 << "mostDetailedID for curItem="
                                   << curItem->getID() << " from ssi="
                                   << nearestSSI << " is "
                                   << getIndexAreaOrder(mostDetailedID) << ":"
                                   << mostDetailedID << endl;
                           }
                           else {
                              // simply, copy ssi location to curItem
                              regionIDtoAdd = getRegionID( ssi, type );
                           }
                           if ( regionIDtoAdd != MAX_UINT32 ) {
                              if ( ! onlyUpdateNonValid ) {
                                 clearRegionsForItem( curItem, type );
                              }
                              addRegionToItem( curItem, regionIDtoAdd );
                              nbrLocationsSet++;
                              DEBUG4(
                                 uint32 curStringIndex = 
                                 curItem->getStringIndex(0);
                                 if (curStringIndex != 0) 
                              {
                                 mc2dbg4 << getName(curStringIndex)
                                         << " is located in "
                                         << getItemName(
                                               getRegionID( ssi, type ) )
                                         << " (ID = " 
                                         << getRegionID( ssi, type )
                                         << ", type = " << (uint32) type << ")"
                                         << endl;
                              }
                                 );
                           }
                        }
                     } else {
                        findByGfxData = true;
                     }

                     if ( findByGfxData ) {
                        // There exists gfx datas for the item type to set
                        // location to.

                        bool found = false;
                        uint32 j = 0;
                        while ((j < typeIDs.getSize()) && (!found))  {
                           // Check OldItem with correct type number j
                           GfxData* typeGfx = 
                              itemLookup(typeIDs.getElementAt(j))
                              ->getGfxData();
                           mc2dbg8 << "   typeGfx->getNbrcoords()=" 
                                   << typeGfx->getNbrCoordinates(0) << endl;
                           // In case we are setting the location of a 
                           // builtup area or citypart, 
                           // we need to check that the item
                           // is actually inside a cityareaitem. This
                           // check is done by picking a random coordinate
                           // inside the item.
                           
                           if ( ( curItem->getItemType() == 
                                  ItemTypes::builtUpAreaItem ) ||
                                ( curItem->getItemType() == 
                                  ItemTypes::cityPartItem ) ) {
                              int32 randLat;
                              int32 randLon;
                              if (! curGfx->getRandomCoordinateInside(
                                                randLat,
                                                randLon)) {
                                 mc2log << error 
                                        << "Could not find a random "
                                        << "coordinate inside group item "
                                        << getName(curItem->
                                              getStringIndex(0))
                                        << endl;
                              } 
                              else if ( typeGfx->
                                        insidePolygon(randLat, 
                                                      randLon) == 2) {
                                 found = true;
                              }
                           } else // No bua or citypart
                              if ((typeGfx->insidePolygon(
                                   curGfx->getLat(0,0), 
                                   curGfx->getLon(0,0)) > 0) ||
                                   (typeGfx->insidePolygon(
                                      curGfx->getLat(0,lastCoordIdx),
                                      curGfx->getLon(0,lastCoordIdx)) > 0) ) {
                                 found = true;
                              }
                           
                           if (found) {
                              if ( ! onlyUpdateNonValid ) {
                                 clearRegionsForItem( curItem, type );
                              }
                              addRegionToItem( curItem, 
                                               typeIDs.getElementAt(j) );
                              nbrLocationsSet++;

                              // add the city parts to the bua
                              // (compare EDR:handleAddCitypartRecord)
                              if ( ( type == ItemTypes::builtUpAreaItem ) &&
                                   ( curItem->getItemType() == 
                                     ItemTypes::cityPartItem ) ) {
                                 OldBuiltUpAreaItem* bua = 
                                    static_cast<OldBuiltUpAreaItem*>
                                    (itemLookup(typeIDs.getElementAt(j)));
                                 if (bua != NULL) {
                                    bua->addItem(curItem->getID());
                                    mc2dbg8 << "Adding citypart '" 
                                       << getFirstItemName(curItem) 
                                       << "' -> bua '" << getFirstItemName(bua)
                                       << "'" << endl;
                                 }
                              }
                              
                              DEBUG4(
                                 uint32 curStringIndex = 
                                 curItem->getStringIndex(0);
                                 if (curStringIndex != 0) 
                                 mc2dbg4 << getName(curStringIndex)
                                         << " is located in "
                                         << getItemName(
                                            typeIDs.getElementAt(j))
                                         << " (ID = " 
                                         << typeIDs.getElementAt(j)
                                         << ", type = " 
                                         << (uint32) type << ")"
                                         << endl;
                              );

                           } else {
                              j++;
                           }
                        }//while
                     }
                  } else if ( (curItem->getItemType() == type) &&
                              (typeIDs.linearSearch(curItem->getID())
                               < MAX_UINT32 )) {
                     if ( ! onlyUpdateNonValid ) {
                        clearRegionsForItem( curItem, type );
                        // This is perhaps fixed by updateMetropolitanBuaMunLoc
                     }
                  }
               } else {
                  // curItem->getGfxData() == NULL
                  if (curItem->getItemType()
                      == ItemTypes::pointOfInterestItem) {
                     // do nothing, no location for POI wanted
                  }
               }
            }
         } else {
            if (curItem == NULL) {
               mc2log << error << here << " OldItem " << i << " on zoom " 
                      << z << " not added -- it is NULL!!!" 
                      << endl;
            } else {
               // curItem is a OldStreetItem
               mc2dbg8 << here << " Found OldStreetItem" << endl;
            }
         }
      }
   }

   // Set the location of the streetitems.
   // This is done last since the streetsegments location must be set first
   // Need to update all groups of ssis to the street !
   updateStreetLocationAll(type);
   
   mc2dbg1 << (uint32) type << "-location of all items set!" << endl;
   mc2dbg1 << (uint32) type << "-location added to " << nbrLocationsSet 
           << " items" << endl;

} // setItemLocation


bool
GMSMap::updateStreetLocationAll(ItemTypes::itemType type)
{
   // All zoomlevels
   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      // All items in each zoomlevel
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z, i);
         // Street?
         if (item->getItemType() == ItemTypes::streetItem) {
            OldStreetItem* street = static_cast<OldStreetItem*> (item);
            for ( uint32 j = 0; j < street->getNbrItemsInGroup(); ++j ) {
               OldItem* ssi = itemLookup( street->getItemNumber( j ) );


               for ( uint32 g=0; g<ssi->getNbrGroups(); g++){
                  OldItem* group = itemLookup(ssi->getGroup(g));
                  if ( ( group != NULL ) && 
                       ( group->getItemType() == type) ){

                     bool noLocationRegion = false;
                     if ( (ssi->getUnmaskedGroup(g) & 0x80000000 ) != 0 ){
                        noLocationRegion = true;
                     }

                     // Will check if the region already exists before 
                     //adding.
                     addRegionToItem( street, 
                                      group->getID(), 
                                      noLocationRegion );
                  }
               }
            }
         }
      }
   }
   
   return (true); 
}


uint32
GMSMap::changeZoomLevel(OldItem* item, uint32 zoomLevel)
{
   // Set the old place in m_itemsZoom to NULL
   uint32 localID = item->getID();
   m_itemsZoom[(localID & 0x78000000) >> 27][localID & 0x07ffffff] = NULL;

   // Add item with new zoom level
   uint32 newID = addItem(item, zoomLevel);

   // Debug print for nodes changing IDs.
   if ( item->getItemType() == ItemTypes::streetSegmentItem ){
      // OldNode 0
      mc2dbg8 << "OldNodeChZoom Old ID:" << localID 
              << " new ID:" << newID << endl;
      // OldNode 1
      uint32 node1OldID = (localID | 0x80000000);
      uint32 node1NewID = (newID | 0x80000000);
      mc2dbg8 << "OldNodeChZoom Old ID:" << node1OldID 
              << " new ID:" << node1NewID << endl; 
   }
   
   // Update all groups containing this item.
   for (uint32 i = 0; i < item->getNbrGroups(); i++) {
      uint32 groupID = item->getGroup(i);
      OldGroupItem* groupItem = static_cast<OldGroupItem*> (itemLookup(groupID));
      groupItem->removeItemWithID(localID);
      if (newID != MAX_UINT32) {
         groupItem->addItem(newID);
      }
   }


   // Update the itemID in misc containers with items ids, see removeItem

   // adminAreaCentres table
   if ( hasAdminAreaCentre(localID) ) {
      mc2log << error << "Need update of admin centre for localID="
             << localID << endl;
      MC2_ASSERT(false);
   }

   // item category table
   ItemMap< set<uint16> >::iterator catIt = m_itemCategories.find(localID);
   if ( catIt !=  m_itemCategories.end() ){
      OldItem* newItem = itemLookup(newID);
      if (newItem != NULL ) {
         newItem->addCategories( *this, catIt->second );
      }
      m_itemCategories.erase(catIt);
   }
   
   // index area order table
   ItemMap< uint32 >::iterator indexAreaIt = 
                  m_indexAreasOrder.find(localID);
   if ( indexAreaIt != m_indexAreasOrder.end() ){
      uint32 order = indexAreaIt->second;
      m_indexAreasOrder.erase(indexAreaIt);
      if (newID != MAX_UINT32) {
         setIndexAreaOrder(newID, order );
      }
   }

   // non searchable items table
   ItemMap< bool >::iterator nonSearchIt = 
      m_nonSearchableItems.find(localID);
   if ( nonSearchIt != m_nonSearchableItems.end() ){
      m_nonSearchableItems.erase(nonSearchIt);
      if (newID != MAX_UINT32) {
         setItemNotInSearchIndex(newID);
      }
   }
   
   return (newID);
}


void
GMSMap::removeObjectsInZoomLevel(uint32 zoomLevel)
{
   // This method does not check that all objects are NULL
   // in the specified zoom level.

   m_itemsZoomSize[zoomLevel] = 0;
   m_itemsZoomAllocated[zoomLevel] = 0;
}


void
GMSMap::buildHashTable()
{
   OldMap::buildHashTable();
}




bool
GMSMap::addExternalItemToMap(OldGenericMap* externalMap, 
                             OldItem* externalItem,
                             set<IDPair_t>& addedIDs,
                             bool parseOnly )
{
   mc2dbg8 << "checking ext item " << externalItem->getID() << endl;

   GfxData* gfx = externalItem->getGfxData();
   if (gfx == NULL)
      return (false);

   // Make sure that the length is ok
   gfx->updateLength();
   MC2BoundingBox bbox;
   gfx->getMC2BoundingBox(bbox);
  
   MC2BoundingBox mapBBox;
   m_gfxData->getMC2BoundingBox(mapBBox);
   
   bool insideMap = true;
   
   if ( ! mapBBox.overlaps(&bbox) ) {
      // The boundingboxes doesn't even overlap each other.
      insideMap = false;
   }
  
   if ( insideMap ) {
      
      // Check if the item will fit in this map.
      // Check if the middle of the boundingbox is inside the item.
      int32 middleLat = bbox.getMinLat() + (bbox.getHeight() / 2);
      int32 middleLon = bbox.getMinLon() + (bbox.getLonDiff() / 2);
      insideMap = true;
      if (gfx->insidePolygon(middleLat, middleLon) > 0) {
         // Ok, so this point was inside the item.
         // Now check if it is not inside this map.
         if (m_gfxData->insidePolygon(middleLat, middleLon) == 0) {
            // Wasn't inside
            insideMap = false;
         }
      } else {
         // The middle of the bounding box wasn't inside this item. 
         // (Could be a donutshaped item, or open polygon.) Just check 
         // if a point on the boundary of the item is not inside the map.
         // (check both ends, first and last coord 
         // (i.e. for ssi = both nodes))
         if ( (m_gfxData->insidePolygon(
               gfx->getLastLat(0), gfx->getLastLon(0)) == 0) &&
              (m_gfxData->insidePolygon(
               gfx->getLat(0,0), gfx->getLon(0,0)) == 0) ) {
            insideMap = false;
         }
      }
   }
  
   // We are not interested in external items that are outside the map.
   // But make exceptions for routeable items, which typically from 
   // outerworld-maps (ocean maps) can be outside the map bbox (mostly 
   // ferry items, but also street segments)
   OldRouteableItem* ri = 
      dynamic_cast<OldRouteableItem*> (externalItem);
   if ( (! insideMap) && ( ri == NULL ) ) {
      // A non-routeable item is outside the map.
      // Exit method.
      return false;
   }
   // For street segments, we don't want the distance from the map bbox
   // to the externalItem to be too large (takes too long time to process)
   if ( (! insideMap) && 
        (externalItem->getItemType() == ItemTypes::streetSegmentItem) ) {
      MC2BoundingBox tmpMapBBox = mapBBox;
      tmpMapBBox.increaseMeters(10000); // 10 km Fixme: make it not hardcoded
      if ( ! tmpMapBBox.overlaps(&bbox) ) {
         mc2dbg8 << "not considering " << externalItem->getID() << endl;
         return false;
      }
   }
   
   // Check that the item does not already exist.
   if (itemExists(externalItem)) {
      return (false);
   }
   
   mc2dbg8 << "checking ext item " << externalItem->getID() << endl;
   
   int64 dist[2];
   bool routeableItemOutsideMap = ( (ri != NULL) && ( ! insideMap ) );
   
   // Make some additional checks for routeable items..
   if (ri != NULL) {

      if (gfx->getLength(0) == 0) {
         // Don't add the item if it is a virtual ri
         return (false);
      }       
      
      // update the hashTable
      buildHashTable();      
      
      if ( routeableItemOutsideMap ) {
         // Trying to add a routeable item that is NOT inside the map.
         // Don't check distance to map gfx data border.
         if (!updateConnections(ri, 
                                true, // fakeUpdate = true
                                2,    // maxDist = 2 m
                                true, // ri is foreign 
                                false) ) { // don't onlyConnectToFerries
            return false;
         }
      } else {
         // Do the usual checks. Make sure it is inside the map.
         
         dist[0] = m_gfxData->signedSquareDistTo(gfx->getLat(0,0),
                                               gfx->getLon(0,0));
         dist[1] = m_gfxData->signedSquareDistTo(
            gfx->getLastLat(0),
            gfx->getLastLon(0));

         if ( ((dist[0] > 0) && (dist[1] > 0)) || 
              (!updateConnections(ri, 
                                  true, // fakeUpdate = true
                                  2,    // maxDist = 2 m
                                  true)) ) { // ri is foreign
            // The nodes are either outside the map, or
            // the ri is not close enough to any other ri's for connections
            // to be created.
            return (false);
         }
      }
      
   }

   if ( parseOnly ) {
      // We checked that this item should be added to the map,
      // with parseOnly return true.
      return true;
   }

   // Ok, we want to add this item!
   internalAddExtItem( externalItem, externalMap, addedIDs,
                       false ); // don't onlyConnectToFerries
   
   // Everything ok
   return (true);

} // addExternalItemToMap

void
GMSMap::internalAddExtItem( OldItem* externalItem, 
                            OldGenericMap* externalMap, 
                            set<IDPair_t>& addedIDs,
                            bool onlyConnectToFerries)
{

   // Save the OldItem into a databuffer
   DataBuffer dataBuffer(3500000, "internalAddExtItem");
   saveOneItem(&dataBuffer, externalItem);
   dataBuffer.reset();

   // Add item to this map
   uint32 zoomLevel = (externalItem->getID() & 0x7fffffff) >> 27;
   uint32 newID = addItemToDatabase(&dataBuffer, zoomLevel, true);
   m_itemsFromExtraItemMap->addLast(newID);
   
   OldItem* item = itemLookup(newID);
   mc2dbg1 << "Added " << externalItem->getItemTypeAsString() 
           << " with ID = " << newID << " (extID "
           << externalItem->getID() << ")" << endl;

   // Remove all groups.
   int32 nbrGroups = item->getNbrGroups();
   for ( int32 i = ( nbrGroups - 1 ); i >= 0; --i ) {
      item->removeGroup( i );
   }
   
   // Remove all incorrect names (the stringindex is only
   // valid for the boundary map names), and replace them
   // with the names from externalMap.
   item->removeAllNames();
   for (uint32 i = 0; i < externalItem->getNbrNames(); i++) {
      LangTypes::language_t lang = 
         externalItem->getNameLanguage(i);
      ItemTypes::name_t nameType = 
         externalItem->getNameType(i);
      uint32 strIdx = externalItem->getStringIndex(i);
      mc2dbg1 << "Name " << i << ": " << externalMap->getName(strIdx) 
              << endl;
      addNameToItem( item, 
                     externalMap->getName(strIdx),
                     lang,
                     nameType );
   }

   // Do some special stuff if the item added was a routeable item.
   OldRouteableItem* newRI = dynamic_cast<OldRouteableItem*> (item);
   if (newRI != NULL) {

      // Remove all current connections for the ri.
      for (int i=0; i<2; i++) {
         OldNode* node = newRI->getNode(i);
         node->deleteAllConnections(*this);
      }

      // Make sure that both the nodes have the correct ID
      newRI->getNode(0)->setNodeID(newID & 0x7fffffff);
      newRI->getNode(1)->setNodeID(newID | 0x80000000);

      // Add the connections to this segments on this map
      updateConnections(newRI,
                        false,  // fakeUpdate = false
                        2,      // maxDist = 2 m
                        true,   // newRI is foreign
                        onlyConnectToFerries);

      // Assume one of the nodes is close to the mapboundry
      // The node, TO which _no_ connections lead, should have a virtual 
      // segment to be added to the segmnetOnBoundry-array.
      // If both nodes have connections, create two virual ssi's.
      mc2dbg2 << "To create \"virtual\" segments for itemID 0x"
              << hex << newID << "=" << dec << newID 
              << " for addition to the segmentsOnBoundry-array" << endl;

      uint32 start = 0, end = 1;
      if (newRI->getNode(start)->getNbrConnections() != 0) {
         start = 1;
         end = 2;
      }
      if (newRI->getNode(start)->getNbrConnections() != 0) {
         mc2dbg2 << "Both nodes have conenctions to other nodes, "
                 << "creating virtual ssi's in both nodes." << endl;
         start = 0;
         end = 2;
      }

      for (uint32 n = start; n < end; n++) {

         uint32 bsID = addToBoundary( newRI->getNode( n )->getNodeID() );
         if ( bsID != MAX_UINT32 ) {
            m_itemsFromExtraItemMap->addLast( bsID );
         }
      }
      
      // sort the boundry segments vector
      if (m_segmentsOnTheBoundry == NULL){
         mc2log << error << here << "m_segmentsOnTheBoundry == NULL" 
                << endl;
         exit(1);
      }
      m_segmentsOnTheBoundry->sort();
   }
   m_itemsFromExtraItemMap->sort();
   
   // Update locations if the added item is a bua
   if ( (item->getItemType() == ItemTypes::builtUpAreaItem)) {
      updateLocation(static_cast<OldBuiltUpAreaItem*> (item));
   }



   // Update addedIDs.
   addedIDs.insert( IDPair_t( externalMap->getMapID(), 
                              externalItem->getID() ) );
  
   // Update maphashtable
   MC2BoundingBox bb;
   if ( item->getGfxData() != NULL ) {
      item->getGfxData()->getMC2BoundingBox( bb );
      m_hashTable->addItem( &bb, item->getID() ); 
   }
   
   // Special case for routeable items:
   OldRouteableItem* externalRI = dynamic_cast<OldRouteableItem*> (externalItem);
   if ( externalRI != NULL ) {
      
      // A routeable item was just added.
      // Add all other routeable items that are connected to this
      // item that is not already added.
      // This must be done because some routeable items may not be
      // added otherwise. I.e. the first routeableitem in the map
      // is may not closest to the mapborder and will therefor otherwise
      // not be added.

      for ( uint32 i = 0; i < 2; ++i ) {
         OldNode* node = externalRI->getNode( i );

         for ( uint32 j = 0; j < node->getNbrConnections(); ++j ) {
            OldConnection* conn = node->getEntryConnection( j );
            uint32 otherItemID = (conn->getConnectFromNode() & 0x7fffffff);
            // Make sure we don't go into eternal loop. Don't add it if
            // it's already processed.
            if ( addedIDs.find( IDPair_t( externalMap->getMapID(), 
                                          otherItemID ) ) == 
                 addedIDs.end() ) {
               // Get the item.
               OldItem* otherItem = externalMap->itemLookup( otherItemID );
               if ( ( otherItem != NULL ) && 
                    ( otherItem->getGfxData() != NULL ) &&
                    ( otherItem->getGfxData()->getLength( 0 ) != 0 ) ) {
                  // Add this routeable item as well.
                  // Recursive call!
                  internalAddExtItem( otherItem, externalMap, addedIDs,
                                      onlyConnectToFerries );
               }
            }
         }
      }
   }
   
}


bool
GMSMap::addExtraItemsFromMap( OldGenericMap* otherMap,
                              set<IDPair_t>& addedIDs,
                              bool addMunicipals )
{
   mc2dbg1 << "addExtraItemsFromMap otherMap=" << otherMap->getMapID() 
           << endl;

   // Make sure that this map has a gfxData
   if (m_gfxData == NULL) {
      cerr << "   m_gfxData == NULL" << endl;
      return (false);
   }
   
   // Make sure that we have a hashTable
   if (m_hashTable == NULL) {
      cerr << "   mapHashTable == NULL" << endl;
      return (false);
   }


  if (m_segmentsOnTheBoundry == NULL){
         mc2log << error << here << "m_segmentsOnTheBoundry == NULL" 
                << endl;
         MC2_ASSERT(false);
  }
  else {
     m_segmentsOnTheBoundry->sort();
  }

   // Add items from the other map, loop until there are no items left in 
   // the otherMap or no more items were added to this current map.
   // (to take care of the case that there might be several ssi's 
   //  "in a row" in the othermap)

   if (m_itemsFromExtraItemMap == NULL) {
      m_itemsFromExtraItemMap = new Vector();
   }
   uint32 nbrItemsInOtherMap = otherMap->getNbrItems();
   uint32 nbrAdded = MAX_UINT32;
   set<uint32> virtualMatchIds;
   mc2dbg2 << "nbrItemsInOtherMap = " << nbrItemsInOtherMap << endl;
   mc2dbg4 << "m_itemsFromExtraItemMap->getSize() = "
        << m_itemsFromExtraItemMap->getSize() << endl;
   while ((nbrItemsInOtherMap > 0) && (nbrAdded > 0)) {
      nbrAdded = 0;
      // Loop over all items in the other map
      for (uint32 curZoom=0; curZoom<NUMBER_GFX_ZOOMLEVELS; curZoom++) {
         for (uint32 curIndex = 0; 
              curIndex < otherMap->getNbrItemsWithZoom(curZoom); 
              curIndex++) 
         {
            OldItem* otherItem = otherMap->getItem(curZoom, curIndex);
            GfxData* otherGfx;
            // Check that the item has a gfx data.
            // Also check that the item id is not present in addedIDs
            // which indicates
            // that the item has not yet been added to any map.
            if ((otherItem != NULL) &&
                ((otherGfx = otherItem->getGfxData()) != NULL) ) {
                
                if ( addedIDs.find( IDPair_t( otherMap->getMapID(),
                                          otherItem->getID() ) )
                    == addedIDs.end()) {
                
                  // Check if we want to add municipals
                  if ((addMunicipals) || 
                      ((!addMunicipals) && 
                       (otherItem->getItemType() 
                           != ItemTypes::municipalItem))) {
                  
                     if ( addExternalItemToMap(otherMap, 
                                               otherItem,
                                               addedIDs) ) {
                        nbrAdded++;
                     }
                     
                  }
               } else if ( virtualMatchIds.find(otherItem->getID())
                              == virtualMatchIds.end() ) {
                  // otherItem was added to another map.
                  // We want to make sure that there is a virtual in this map
                  // were to make external connections to the virtual item 
                  // that was added from extra item to the other map
                  
                  // if routeable item
                  if ( (otherItem->getItemType() 
                           == ItemTypes::streetSegmentItem) ||
                       (otherItem->getItemType()
                           == ItemTypes::ferryItem) ) {
                  
                     if (addVirtualToMatchExtraItem(otherMap, otherItem)) {
                        virtualMatchIds.insert(otherItem->getID());
                     }
                  
                  }
               }
            }
         }
      }
      nbrItemsInOtherMap -= nbrAdded;
      mc2dbg2 << "added " << nbrAdded << " items from otherMap -> "
              << "nbrItemsInOtherMap = " << nbrItemsInOtherMap << endl;
      mc2dbg4 << "m_itemsFromExtraItemMap->getSize() = "
              << m_itemsFromExtraItemMap->getSize() << endl;
   }

   return (true);
}

bool
GMSMap::addVirtualToMatchExtraItem( OldGenericMap* otherMap,
                                       OldItem* otherItem )
{
   bool createdVirtual = false;
   set<IDPair_t> tmpIDs;
   if ( addExternalItemToMap(otherMap, 
                             otherItem,
                             tmpIDs,
                             true /*parseOnly*/)) {
      
      // find the closest routeable item in this map
      // and make sure it has a virtual item so it can connect
      // to otherItem
      mc2dbg4 << " addVirtualToMatchExtraItem in map " << getMapID() 
              << " extra item = "
              << otherMap->getMapID() << ":" << otherItem->getID() << endl;


      // find my ssi/ferry items that are close to the otherItem
      GfxData* otherGfx = otherItem->getGfxData();
      //buildHashTable();
      m_hashTable->clearAllowedItemTypes();
      m_hashTable->addAllowedItemType(ItemTypes::streetSegmentItem);
      m_hashTable->addAllowedItemType(ItemTypes::ferryItem);
      set<uint32> allCloseIDs;
      bool kill = false;
      uint32 c = 0;
      for ( uint32 i = 0; i < 2; i++ ) {
         if ( i == 1 ) {
            c = otherGfx->getNbrCoordinates(0) -1;
         }
         Vector* closeIDs = m_hashTable->getAllWithinRadius_meter(
            otherGfx->getLon(0,c), otherGfx->getLat(0,c), 2, kill);
         if ( closeIDs != NULL ) {
            for ( uint32 j=0; j < closeIDs->getSize(); j++ ) {
               allCloseIDs.insert( closeIDs->getElementAt(j) );
            }
         }
      }
      mc2dbg4 << "addVirtualToMatchExtraItem: " << allCloseIDs.size()
              << " of my items are close to otherItem" << endl;
      
      if (m_segmentsOnTheBoundry == NULL){
         mc2log << error << here << "m_segmentsOnTheBoundry == NULL" << endl;
         exit(1);
      }
      // For each item find the node(s) that "connects" to the otherItem
      // and for each node that is close to the otherItem, add a virtual
      // - not if the item itself already is a virtual item...
      for ( set<uint32>::const_iterator it = allCloseIDs.begin();
            it != allCloseIDs.end(); it++ ) {
         mc2dbg4 << "addVirtualToMatchExtraItem (*it):" << (*it) << endl;
         OldRouteableItem* myRI =
               dynamic_cast<OldRouteableItem*>(itemLookup(*it));
         if ( myRI == NULL ) {
            continue;
         }
         if ( m_segmentsOnTheBoundry->getBoundrySegment(*it) != NULL ) {
            // not adding virtuals to virtual item
            mc2dbg4 << " addVirtualToMatchExtraItem: not adding virtuals "
                    <<"to virtual " << (*it) << endl;
            continue;
         }
         GfxData* myGfx = myRI->getGfxData();
         if ( myGfx == NULL ) {
            continue;
         }
         
         for (uint32 n = 0; n < 2; n++ ) {
            
            // Get the current node and its coordinates
            uint32 c = 0;
            if (n == 1) {
               c = myGfx->getNbrCoordinates(0) - 1;
            }
            int32 myLat = myGfx->getLat(0,c);
            int32 myLon = myGfx->getLon(0,c);
            
            float64 sqDist0 =
               GMSUtility::getDistanceToNode(0, otherGfx, myLat, myLon);
            float64 sqDist1 =
               GMSUtility::getDistanceToNode(1, otherGfx, myLat, myLon);

            // my node is close (connecting) to one of the nodes of otherItem
            // make sure we have a virtual!
            if ( MIN(sqDist0, sqDist1) == 0 ) {
               mc2dbg4 << "addVirtualToMatchExtraItem: add virtual at node "
                       << myRI->getNode(n)->getNodeID() << endl;
               
               uint32 bsID = addToBoundary( myRI->getNode(n)->getNodeID() );
               if ( bsID != MAX_UINT32 ) {
                  mc2dbg << "addVirtualToMatchExtraItem: virtual " << bsID 
                         << " for my item " << getMapID() << ":" << (*it)
                         << " node:" << n
                         << " matching other item " << otherMap->getMapID()
                         << ":" << otherItem->getID() << endl;
                  m_itemsFromExtraItemMap->addLast( bsID );
                  createdVirtual = true;
                  // sort the boundry segments vector
                  if (m_segmentsOnTheBoundry == NULL){
                     mc2log << error << here
                            << "m_segmentsOnTheBoundry == NULL" << endl;
                     exit(1);
                  }
                  m_segmentsOnTheBoundry->sort();
                  
                  m_itemsFromExtraItemMap->sort();
               }
           
            }
            
         }
      }
   
   }
   return createdVirtual;
}

bool
GMSMap::itemExists(OldItem* item)
{
   GfxData* gfx = item->getGfxData();
   if (gfx != NULL) {
      Vector* closeIDs;
      bool killVector = false;
      m_hashTable->clearAllowedItemTypes();
      m_hashTable->addAllowedItemType( item->getItemType() );
      
      closeIDs = m_hashTable->getAllWithinRadius_meter(
         gfx->getLon(0,0), gfx->getLat(0,0), 5, killVector);
      
      m_hashTable->clearAllowedItemTypes();

      bool exists = false;
      if (closeIDs != NULL) {
         for (uint32 idIndex=0; 
              idIndex<closeIDs->getSize(); idIndex++) {
            
            // Check if item with idIndex has the same gfx as item
            GfxData* idGfx = 
               itemLookup(closeIDs->getElementAt(idIndex))->getGfxData();
            
            if (gfx->getTotalNbrCoordinates() == 
                idGfx->getTotalNbrCoordinates()) {
               // The two gfxData have the same number of gfxDatas,
               // check all the coordinates
               uint32 nbrSame = 0;
               for (uint32 p=0; p<gfx->getNbrPolygons(); ++p) {
                  for (uint32 i=0; i<gfx->getNbrCoordinates(p); i++) {
                     if (gfx->equals(p, i, idGfx, p, i))
                        nbrSame++;
                  }
               }

               if (nbrSame == gfx->getTotalNbrCoordinates())
                  exists = true;
            }
         }
      }
      
      if (killVector)
         delete closeIDs;
      
      return (exists);
      
   } else {
      mc2log << warn 
             << "GMSMap::itemExists Gfxdata null for item. "
             << "Returning false" << endl;
      return (false);
   }
}

bool 
GMSMap::itemsShareNameStrIdx(OldItem* firstItem, OldItem* secondItem){
   // check if same name (independent of name type and lang)
   bool result = false;
   for (uint32 m = 0; m < firstItem->getNbrNames(); m++) {
      for (uint32 n = 0; n < secondItem->getNbrNames(); n++) {
         if (firstItem->getStringIndex(m) == 
             secondItem ->getStringIndex(n)) {
            if (result) {
               // we'll never get here, only for debug.
               // already one name same
               mc2dbg8 << ", ("
                       << getName(firstItem->getStringIndex(m))
                       << ")";
            } else {
               // one name same
               mc2dbg << "   Item: " << firstItem->getID() << " and " 
                      << secondItem->getID() << " same name: " 
                      << getName(firstItem->getStringIndex(m)) 
                      << endl;
               result = true;
               return result;
            }
         }
      }
   }
   return result;
} // itemsShareNameStrIdx


bool
GMSMap::mergeSameNameAdminAreas( ItemTypes::itemType itemType,
                                 uint32 maxMergeDist, 
                                 bool updateLocation,
                                 bool simplifyMergedGfxs )
{
   //TODO: Change all the names here that are called bua to
   // adminarea. Method originally only handled bua:s.

   if ( ( itemType != ItemTypes::builtUpAreaItem ) && 
        ( itemType != ItemTypes::municipalItem ) &&
        ( itemType != ItemTypes::zipCodeItem ) ) {
      mc2log << warn 
             << "GMSMap::mergeSameAdminAreas "
             << "wrong itemtypes supplied" << (int)itemType << endl;
      return false;
   }
   
   mc2dbg << "mergeSameNameAdminAreas starts, itemType = "
          << (int)itemType 
          << ", maxMergeDist = " 
          << maxMergeDist << endl;

   uint32 adminZoom = 0;
   if ( itemType == ItemTypes::zipCodeItem ) {
      adminZoom = 10;
   }

   // Loop and count buas
   uint32 nbrBuas = 0;
   for (uint32 i=0; i < getNbrItemsWithZoom( adminZoom ); i++) {
      OldItem* item = getItem(adminZoom, i);
      if ((item != NULL) &&
          (item->getItemType() == itemType)) {
         mc2dbg << " " << nbrBuas << ": buaID=" << item->getID() 
                << " name=" << getFirstItemName(item) << endl;
         nbrBuas++;
      }
   }
   if (nbrBuas < 2) {
      mc2dbg << "No buas to merge, nbrBuas=" << nbrBuas << endl;
      return true;
   }

   mc2dbg << "There are " << nbrBuas << " built-up areas in the map." << endl;
   
   // Loop again and store e.g. buaIDs and GfxDatas in vectors
   //
   // Vector to store the itemIDs of buas
   uint32 buaIDs[nbrBuas];
   // Vector with a copy of the gfxdata of all buas, used to mark that
   // the bua has been checked or merged.
   GfxDataFull** gfxs = new GfxDataFull*[nbrBuas];
   // Whether or not the Gfxdata in gfxs are real or approximated.
   bool realGfxData[nbrBuas];
   // Vector with info about into which bua (itemID) a certain bua is merged
   uint32 mergedInto[nbrBuas];
   // Vector with info about which buas that have been extended, i.e.
   // that have had another bua added into its gfx.
   uint32 extended[nbrBuas];
   bool useIndexAreas = NationalProperties::useIndexAreas(getCountryCode(),
                                                          getMapOrigin());
   
   uint32 n = 0;
   for (uint32 i=0; i < getNbrItemsWithZoom( adminZoom ); i++) {
      OldItem* item = getItem(adminZoom, i);
      if ((item != NULL) &&
          (item->getItemType() == itemType)) {
         mc2dbg8 << " buaID " << item->getID() << " "
                 << getFirstItemName(item) << endl;
         buaIDs[n] = item->getID();
         mergedInto[n] = MAX_UINT32; // so far not merged into any bua
         extended[n] = 0;
         
         if ( item->getGfxData() != NULL )  {
            //store the buas
            gfxs[n] = item->getGfxData();
            realGfxData[n] = true;
         } else {
            // Create approximate gfxdata. 
            // (Using convex hull of items having this one as location)
            realGfxData[n] = false;
            // Temporary gfx containing all start and end of all ssi:s
            // with the item as location.
            GfxDataFull* allSSI = 
               GMSGfxData::createNewGfxData(NULL, true);  
            for (uint32 curZoom=0; curZoom < NUMBER_GFX_ZOOMLEVELS; 
                 curZoom++) {
               for ( uint32 itemNbr = 0;
                     itemNbr < getNbrItemsWithZoom(curZoom); itemNbr++) {
                  OldStreetSegmentItem* ssi = dynamic_cast<OldStreetSegmentItem*>
                     (getItem(curZoom, itemNbr));
                  if ( (ssi != NULL) && 
                       (getRegionID( ssi, itemType ) == item->getID()) && 
                       (ssi->getGfxData() != NULL) ) {
                     GfxData* tmpGfx = ssi->getGfxData();
                     allSSI->addCoordinate(tmpGfx->getLat(0,0), 
                                           tmpGfx->getLon(0,0));
                     allSSI->addCoordinate(tmpGfx->getLastLat(0), 
                                           tmpGfx->getLastLon(0));
                  }
               }
            }
            // Create convex hull out of allSSI
            GfxDataFull* convHullGfx = 
               GMSGfxData::createNewGfxData(NULL, true);

            Stack stack;
            if (allSSI->getConvexHull(&stack, 0)) {
               uint32 size = stack.getStackSize();
               for (uint32 i = 0; i < size; i++) {
                  uint32 idx = stack.pop();
                  convHullGfx->addCoordinate( allSSI->getLat(0,idx),
                                              allSSI->getLon(0,idx) );
               }  
               gfxs[n] = convHullGfx;
            } else {
               // Could not create convex hull...
               delete convHullGfx;
               gfxs[n] = NULL;
            }
            delete allSSI;
         }

         // Only for debug
         if ( useIndexAreas &&
              (! itemNotInSearchIndex(item->getID()))  ) {
            uint32 indexAreaOrder = getIndexAreaOrder(item->getID());
            uint32 nbrGroups = item->getNbrGroups();
            mc2dbg8 << item->getID() 
                 << " search item, nbrGroups=" << nbrGroups
                 << " nbrNames=" << int(item->getNbrNames()) 
                 << " indexAreaOrder=" << indexAreaOrder << endl;
         }
         ++n;
      }
   }
   mc2dbg << "Stored " << n << " buaIDs and gfxDatas" << endl;
   if (n != nbrBuas) {
      mc2log << error << "Number of buas don't match" << endl;
      return false;
   }
   

   // Max dist between 2 buas for them to be merged (meters squared)
   uint64 maxMergeSqDist = maxMergeDist * maxMergeDist;
   
   // Loop all the buas and try to add gfx's from other buas into this
   for (uint32 i = 0; i < nbrBuas; i++) {
      if (gfxs[i] != NULL) {
         // bua nbr i is not already merged into another bua

         OldItem* curBua = itemLookup(buaIDs[i]);
         mc2dbg << "To merge gfxs into gfx " << i << ", " 
                << getFirstItemName(buaIDs[i]) << ", ID: " << buaIDs[i] 
                << endl;
         
         // The GfxData to use to check if other buas are close.
         GfxDataFull* chkMergedGfxData = GMSGfxData::createNewGfxData(NULL,false);
         // The GfxData containing the merged gfxdata.
         GfxDataFull* mergedGfxData = 
            GMSGfxData::createNewGfxData(NULL,false);
         
         // Add curBua's gfx to the chkMergedGfxData (add all polygons)
         mc2dbg8 << "nbr polygons in gfx " << i << "="
                 << gfxs[i]->getNbrPolygons() << endl;
         for (uint16 p = 0;
              p < gfxs[i]->getNbrPolygons(); p++) {
            // Always add to the gfxdata used for checking
            if (!chkMergedGfxData->addPolygon(
                     gfxs[i], false, p)) {
               mc2log << error << "Failed to add Gfx, returning false" << endl;
               return false;
            }
            
            // Only add real gfxdatas to the merged gfx.
            if (realGfxData[i]) {
               if (!mergedGfxData->addPolygon(
                   gfxs[i], false, p)) {
                  mc2log << error << "Failed to add Gfx, "
                         << "returning false" << endl;
                  return false;
               }
            }
         }

         // Mark curGfx as added
         if ( ! realGfxData[i] ) {
            delete gfxs[i];
         }
         gfxs[i] = NULL;
         
         // Loop until nothing is changed
         // 
         bool oneMerged = true;
         while (oneMerged) {
            oneMerged = false;
            for (uint32 j = i+1; j < nbrBuas; j++) {
               if (gfxs[j] != NULL) {
                  mc2dbg8 << " Checking gfx " << j << ", bua "
                          << getFirstItemName(buaIDs[j]) << endl;

                  // check if same name (independent of name type and lang)
                  OldItem* otherBua = itemLookup(buaIDs[j]);
                  // itemsShareNameStrIdx is not a member method.
                  bool merge = itemsShareNameStrIdx(curBua, otherBua);

                  // Check the index area order.
                  uint32 curIndexAreaOrder = 
                     getIndexAreaOrder(curBua->getID());
                  uint32 otherIndexAreaOrder = 
                     getIndexAreaOrder(otherBua->getID());
                  if ( curIndexAreaOrder != otherIndexAreaOrder ){
                     // At least one of the items is an index area, and the
                     // other is not, or have a different index area order.
                     merge = false; // Don't merge different index area order
                  }

                  // Check if items are part of search index
                  bool handledCloseForIA = false;
                  if (merge && useIndexAreas) {
                     bool curNotInSearchIndex = 
                              itemNotInSearchIndex(curBua->getID());
                     bool otherNotInSearchIndex = 
                              itemNotInSearchIndex(otherBua->getID());
                     if ( ( int(curNotInSearchIndex) && 
                            ! int(otherNotInSearchIndex) ) 
                            ||
                          ( ! int(curNotInSearchIndex) && 
                            int(otherNotInSearchIndex) ) ) {
                        // Don't merge if one item is part of search index 
                        // and the other is not
                        merge = false; 
                     }

                     if ( merge && useIndexAreas && 
                          (!curNotInSearchIndex) &&
                          (!otherNotInSearchIndex) ) {
                        // Use index areas and both items in search index

                        if ( isIndexArea(curBua->getID()) && 
                             isIndexArea(otherBua->getID()) ) {
                           // actually index areas (not e.g. zip codes)

                           // really check that the index areas are equal,
                           // i.e. located in index areas with some index 
                           // area order and same name.
                           // Recursive all the way to the top index area
                           if ( ! mergeIndexAreas( curBua, otherBua) ) {
                              merge = false;
                           }
                           handledCloseForIA = true;
                        }
                     }
                  }


                  // check if close
                  if (merge) {
                     if ( handledCloseForIA ) {
                        // not considering close, already handled in 
                        // the call of mergeIndexAreas
                     }
                     else if ( maxMergeDist != MAX_UINT32 ) {
                        // Check if the gfx[j] is close enough to be 
                        // added to the chkMergedGfxData
                        
                        // First check distance between bounding boxes.
                        MC2BoundingBox bBox1;
                        chkMergedGfxData->getMC2BoundingBox(bBox1);

                        MC2BoundingBox bBox2;
                        gfxs[j]->getMC2BoundingBox(bBox2);
                        bBox2.increaseMeters(maxMergeDist); // expand.

                        merge = false;
                        uint64 dist = MAX_UINT32;
                        if ( bBox1.overlaps( bBox2 ) ){
                           // Seems to be close enough for distance check.
                           dist = chkMergedGfxData->minSquareDistTo(gfxs[j]);
                           if (dist < maxMergeSqDist) {
                              // gfx's close
                              mc2dbg << " close (" << uint32 (sqrt(dist))
                                      << " meters)" << endl;
                              merge = true;
                           }
                           else {
                              mc2dbg << " not close (" 
                                      << uint32 (sqrt(dist)) << " meters)" 
                                      << endl;
                           }
                        } 
                        if ( !merge ){
                          // not close, don't merge
                           if ( dist != MAX_UINT32 ){
                           mc2dbg << " not close (" 
                                   << uint32 (sqrt(dist)) << " meters)" 
                                   << endl;
                           }
                           else {
                           mc2dbg << " not close bounding boxes to far" 
                                   << endl;

                           }
                        }
                     }
                  }
                 
                  // merge the gfxDatas
                  if (merge) {
                     // add all polygons from gfxs[j] to mergedGfxData
                     bool ok = true;
                     uint16 poly = 0;
                     mc2dbg8 << "  nbr polygons in gfx " << j << "="
                             << gfxs[j]->getNbrPolygons() << endl; 
                     
                     while ( ok && (poly < gfxs[j]->getNbrPolygons())) {
                        if ( realGfxData[j] ) {
                           if (!mergedGfxData->addPolygon(
                                    gfxs[j], false, poly)) {
                              ok = false;
                           }
                        }
                        
                        if (!chkMergedGfxData->addPolygon(
                                 gfxs[j], false, poly)) {
                           ok = false;
                        }
                        
                        poly++;
                     }

                     if (ok) {
                        oneMerged = true;
                        mc2dbg1 << "   Gfx nbr " << j << " merged, ID: "
                                << buaIDs[j] << " into " << buaIDs[i] << endl;
                        // mark bua[j] used
                        if (! realGfxData[j]) {
                           delete gfxs[j];
                        }
                        gfxs[j] = NULL;
                        // mark that otherBua (j) is merged into curBua (i)
                        mergedInto[j] = buaIDs[i];
                        // mark that curBua (i) has been extended with another
                        extended[i] = 1;

                        // In case of merging zipcodes, copy the groups to the
                        // merged zipcode.
                        if ( itemType == ItemTypes::zipCodeItem ) {
                           OldZipCodeItem* curZip = 
                              static_cast<OldZipCodeItem*> (curBua);
                           OldZipCodeItem* otherZip = 
                              static_cast<OldZipCodeItem*> (otherBua);
                           for ( uint32 k = 0; 
                                 k < otherZip->getNbrItemsInGroup(); 
                                 ++k ) {
                              bindItemToGroup(otherZip->getItemNumber( k ),
                                              curZip->getID() );
                              
                           }
                        }
                        else {
                           uint32 nbrChanged = 
                              changeGroupOfItems(otherBua->getID(), 
                                                 curBua->getID());
                           mc2dbg << "Changed group " << otherBua->getID()
                                  << "->" << curBua->getID() << " of " 
                                  << nbrChanged 
                                  << " items." << endl;
                        }
                     } else {
                        mc2dbg2 << "  Gfx nbr " << i << " not merged" << endl;
                     }

                  }
               }
            }
         }
       
         if ( mergedGfxData->getTotalNbrCoordinates() == 0 ) {
            delete mergedGfxData;
            mergedGfxData = NULL;
         } else {
            mergedGfxData->setClosed(0,true);
            mergedGfxData->updateLength();
         }
         // Replace the gfxData of curBua with the mergedGfxData
         curBua->setGfxData(mergedGfxData);
         // Delete the gfxdata used for checking.
         delete chkMergedGfxData;
      }
   }

   // The buas, whose gfxData has been merged into another bua,
   // should be removed from the map.
   for (uint32 n = 0; n < nbrBuas; n++) {

      if (mergedInto[n] != MAX_UINT32) {

         // Finally remove the bua
         mc2dbg << "Removing merged bua nbr " << n << ", id=" << buaIDs[n]
                 << " name=" << getFirstItemName(buaIDs[n]) << endl;
         removeItem(buaIDs[n]);
      }
   }

   if (simplifyMergedGfxs) {
      mc2dbg2 << "Simplifying merged gfxs" << endl;
      for (uint32 n = 0; n < nbrBuas; n++) {
         if (extended[n] == 1) {
            mc2dbg2 << "For bua " << n << ", id=" << buaIDs[n]
                    << " name=" << getFirstItemName(buaIDs[n]) << endl;
            OldItem* bua = itemLookup(buaIDs[n]);
            if ( bua->getGfxData() != NULL ) {
               GfxDataFull* newGfx = 
                  GMSGfxData::removeInsideCoordinates(bua->getGfxData());
               bua->setGfxData(newGfx);
            }
         }
      }
   }
  
   // Delete the memory allocated here
   delete[] gfxs;

   mc2dbg1 << "mergeSameNameAdminAreas ends" << endl;
   return true;
}

// Add the connections inside this map.
bool
GMSMap::updateConnections(OldRouteableItem* item, 
                          bool fakeUpdate,
                          uint32 maxDist,
                          bool itemIsForeign,
                          bool onlyConnectToFerries)
{
   uint32 sqMaxDist = SQUARE(maxDist);
   
   GfxData* gfx = item->getGfxData();
   if (gfx == NULL) {
      cerr << "updateConnections, GfxData == NULL!!!" << endl;
      return (false);
   }

   //
   //          otherSSI            item
   //    *-------------------*  *-----------*
   //                           ^           ^
   //                           |           |
   //                         curNode  oppositeNode

   float64* sqDistCurNodeOtherSSI = new float64[2];
   float64* sqDistOppositeNodeOtherSSI = new float64[2];
   m_hashTable->clearAllowedItemTypes();
   if (! onlyConnectToFerries ) {
      m_hashTable->addAllowedItemType(ItemTypes::streetSegmentItem);
   }
   m_hashTable->addAllowedItemType(ItemTypes::ferryItem);
   
   bool addedConnections = false;
   bool fitsThisMap = false;
   
   // Check both nodes for connections on this map
   uint32 nodeNbr = 0;
   while ((nodeNbr < 2) && (addedConnections || !fitsThisMap)) {
      fitsThisMap = true;
      mc2dbg8 << "Checking id " << item->getID() << " node " << nodeNbr << endl;

      // Get the current node and its coordinates
      OldNode* curNode = item->getNode(nodeNbr);
      uint32 n = 0;
      if (nodeNbr == 1) {
         n = gfx->getNbrCoordinates(0) - 1;
      }
      int32 curLat = gfx->getLat(0,n);
      int32 curLon = gfx->getLon(0,n);

      // Get the opposite node and its coordinates
      n = 0;
      if (nodeNbr == 0) {
         n = gfx->getNbrCoordinates(0) - 1;
      }
      int32 oppositeLat = gfx->getLat(0,n);
      int32 oppositeLon = gfx->getLon(0,n);

      // Get all SSI close to curNode
      mc2dbg4 << "getAllWithinRadius(" << curLon << ", "
              << curLat << ", " << maxDist << ")" << endl;
      bool killTmpCloseIDs= false;
      Vector* tmpCloseIDs = m_hashTable->getAllWithinRadius_meter(
         curLon, curLat, maxDist, killTmpCloseIDs);

      if ((tmpCloseIDs == NULL) || (tmpCloseIDs->getSize() == 0)) {
         // no ssi close to the item
         fitsThisMap = false;
      } else if ( (tmpCloseIDs->getSize() == 1) && !itemIsForeign &&
                  (tmpCloseIDs->getElementAt(0) == item->getID()) ) {
         // just one item close = this one (curNode = a dead end) 
         // curNode does not fit this map, continue the while-loop
         fitsThisMap = false;
      } else {
         // Get the minimum distance to the far node on otherSSI
         // (of the shortest one of the otherSSIs)
         float64 minSqDist = MAX_FLOAT64;
         for (uint32 i=0; i<tmpCloseIDs->getSize(); i++) {
            GfxData* otherGfx = itemLookup(tmpCloseIDs->getElementAt(i))
               ->getGfxData();
            float64 sqD0 = 
               GMSUtility::getDistanceToNode(0, otherGfx, curLat, curLon);
            float64 sqD1 = 
               GMSUtility::getDistanceToNode(1, otherGfx, curLat, curLon);

            if (sqD0 > sqD1)
               minSqDist = MIN(minSqDist, sqD0);
            else 
               minSqDist = MIN(minSqDist, sqD1);
         }
         minSqDist /= 2.0;
         mc2dbg8 << "New radius: curNode=" << curNode->getNodeID()
                 << ", sqMinDist=" << minSqDist << ", minDist="
                 << uint32(sqrt(minSqDist)) << endl;

         bool killCloseIDs = false;
         Vector* closeIDs = NULL;
         if (minSqDist > sqMaxDist) {
            // The items in tmpCloseIDs will do
            closeIDs = new Vector(*tmpCloseIDs);
            mc2dbg8 << "Copied tmpCloseIDs to closeIDs" << endl;
            //use sqMaxDist
            minSqDist = sqMaxDist;
         } else {
            // Found a smaller radius (the shortest ssi among tmpCloseIDs is 
            // shorter than sqMaxDist), make sure it is gt 10 and get the items
            mc2dbg8 << "To get close items again" << endl;
            uint32 minDistMC2 = MAX(
               uint32( sqrt(minSqDist) * GfxConstants::METER_TO_MC2SCALE),
               10);
            // Get candidates within sqrt(minSqDist)
            closeIDs = m_hashTable->getAllWithinRadius_MC2Scale(
               curLon, curLat, minDistMC2, killCloseIDs);
            mc2dbg8 << "size of closeIDs = " << closeIDs->getSize() 
                    << ", radius(mc2scale)= " << minDistMC2 << endl;
         }

         if ((closeIDs == NULL) || (closeIDs->getSize() == 0) ) {
            // no ssi close to the item
            fitsThisMap = false;
         } else if ( (tmpCloseIDs->getSize() == 1) && !itemIsForeign &&
                     (tmpCloseIDs->getElementAt(0) == item->getID()) ) {
            // just one item close = this one (curNode = a dead end) 
            // curNode does not fit this map, continue the while-loop
            fitsThisMap = false;
         } else { 
            // Check all StreetSegment candidates and add connections to them
            uint32 i = 0;
            mc2dbg4 << "nbr close ssi's " << closeIDs->getSize() << endl;
            while (fitsThisMap && (i < closeIDs->getSize())) {
               OldRouteableItem* otherSSI = (OldRouteableItem*) 
                  itemLookup(closeIDs->getElementAt(i));
               GfxData* otherGfx = otherSSI->getGfxData();
               if ( (itemIsForeign ||
                     (closeIDs->getElementAt(i) != item->getID())) && 
                    (otherGfx != NULL) ) {

                  // Calculate distance from curNode to nodes on otherSSI
                  sqDistCurNodeOtherSSI[0] = GMSUtility::getDistanceToNode(
                     0, otherGfx, curLat, curLon);
                  sqDistCurNodeOtherSSI[1] = GMSUtility::getDistanceToNode(
                     1, otherGfx, curLat, curLon);

                  // Calculate distance from oppositeNode to nodes on otherSSI
                  sqDistOppositeNodeOtherSSI[0] = 
                     GMSUtility::getDistanceToNode(
                     0, otherGfx, oppositeLat, oppositeLon);
                  sqDistOppositeNodeOtherSSI[1] = 
                     GMSUtility::getDistanceToNode(
                     1, otherGfx, oppositeLat, oppositeLon);

                  // Make sure that the distances not are equal
                  const float64 eps = 0.000001;
                  if (sqDistCurNodeOtherSSI[0] == sqDistOppositeNodeOtherSSI[0])
                     sqDistOppositeNodeOtherSSI[0] += eps;
                  if (sqDistCurNodeOtherSSI[1] == sqDistOppositeNodeOtherSSI[1])
                     sqDistOppositeNodeOtherSSI[1] += eps;

                  if ( (sqDistCurNodeOtherSSI[0] <= minSqDist) &&
                       (sqDistCurNodeOtherSSI[1] <= minSqDist) &&
                       (otherGfx->getNbrCoordinates(0) < 4)  ) {
                     // This is a strange case -- both nodes close to
                     // curNode and less than 4 coordinates.
                     // (e.g. virtual 0-length ssi)
                     mc2dbg8 << "Found segment shorter than minSqDist=" 
                        << minSqDist << " dist0=" << sqDistCurNodeOtherSSI[0]
                        << " dist1=" << sqDistCurNodeOtherSSI[1]
                        << " nbr coords=" << otherGfx->getNbrCoordinates(0)
                        << " otherssiID=" << otherSSI->getID() << endl;
                  } else {
                     mc2dbg4 << "otherSSIid=" << closeIDs->getElementAt(i)
                             << endl;
                     for ( uint32 otherNodeNbr=0;   
                           otherNodeNbr < 2; 
                           otherNodeNbr++) {
                        if ( ( sqDistCurNodeOtherSSI[otherNodeNbr] <= 
                               minSqDist) &&
                             ( sqDistCurNodeOtherSSI[otherNodeNbr] <
                               sqDistOppositeNodeOtherSSI[otherNodeNbr] ) ) {
                           // The other node is close to this node!
                           mc2dbg4 << "Add connection" << endl;
                  
                           // Check if otherItem is a boundary segment
                           if (m_segmentsOnTheBoundry != NULL) {
                              // Note that we don't want otherItem to be
                              // a boundrysegment (ie. virtual 0-length
                              // segment) or connected to a boundrysegment.
                              
                              OldBoundrySegment* bs;
                              
                              // The otherSSI may be a boundrysegment.
                              bs = m_segmentsOnTheBoundry->
                                    getBoundrySegment(otherSSI->getID());
                              if ((bs == NULL) &&
                                  !(itemIsForeign && !fakeUpdate)) {
                                 // Ok otherSSI itself was not a
                                 // boundrysegment. Check if it's
                                 // connected to a boundrysegment then.
                                 //(unless we are adding conns to extraitems)
                                 OldNode* otherNode = 
                                    otherSSI->getNode(otherNodeNbr);
                                 uint32 j = 0;
                                 while ((bs == NULL) && 
                                        (j < otherNode
                                             ->getNbrConnections())) {
                                    uint32 potentialBSNodeID =
                                       otherNode->getEntryConnection(j)
                                        ->getConnectFromNode();
                                    bs = m_segmentsOnTheBoundry->
                                       getBoundrySegment(
                                          potentialBSNodeID & 0x7fffffff);
                                    j++;
                                 }
                              }
                              
                              if (bs != NULL) {
                                 OldBoundrySegment::closeNode_t closeNodeVal =
                                    bs->getCloseNodeValue();
                                 if ( ((closeNodeVal ==
                                        OldBoundrySegment::node0close) &&
                                       (otherNodeNbr == 0)) ||
                                      ((closeNodeVal ==
                                        OldBoundrySegment::node1close) &&
                                       (otherNodeNbr == 1)) ) {
                                    if ((m_itemsFromExtraItemMap == NULL) ||
                                        (m_itemsFromExtraItemMap->binarySearch(
                                          otherSSI->getID()) != MAX_UINT32)) {
                                       // the bs is originated from a extra
                                       // item map -> ok to connect to this ssi
                                       mc2dbg2 << "otherSSI (" 
                                               <<  otherSSI->getID()
                                               << ") is a bs, but was "
                                               << "added from extraitemmap"
                                               << endl;
                                    } else {
                                       // otherSSI doesn't fit - "exit" method
                                       mc2dbg2 << otherSSI->getID() 
                                               << " is a bs or connected "
                                               << "to a bs"  << endl;
                                         // Fixme: Don't know why this is commented!
//                                       fitsThisMap = false;
                                    }
                                 }
                              }
                           }

                           if (fitsThisMap) {
                              if (!fakeUpdate) {
                                 addConnection( 
                                    curNode, otherSSI->getNode(otherNodeNbr),
                                    (uint32)sqrt(
                                      sqDistCurNodeOtherSSI[otherNodeNbr]));
                                 // Set the location to be the same as the
                                 // location for the connecting segment.
                                 replaceRegions( item, otherSSI, 
                                             ItemTypes::builtUpAreaItem);
                                 replaceRegions(item, otherSSI, 
                                             ItemTypes::municipalItem);
                              }
                              addedConnections = true;
                           }
                        }
                     }
                  }
               } // if otherGfx != NULL
               i++;
            } // for closeIDs++
            
         }// if closeIDs != NULL
         // Clean up closeIDs !?
         if (killCloseIDs) {
            delete closeIDs;
            closeIDs = NULL;
         }
      } // if tmpCloseIDs != NULL
      if (killTmpCloseIDs) {
         delete tmpCloseIDs;
         tmpCloseIDs = NULL;
      }

      nodeNbr++;
   } //for nodeNbr
   delete[] sqDistCurNodeOtherSSI ;
   delete[] sqDistOppositeNodeOtherSSI;
   
   // Return if any connections were added
   return addedConnections;

}


uint32
GMSMap::updateNodeLevels()
{
   
   mc2dbg4 << "GSC: updateNodeLevels()" << endl;
   uint32 nbrUpdatedNodes = 0;
   
   for(uint32 z=0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for(uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z, i);
         if ((item != NULL) && 
             (item->getItemType() == ItemTypes::streetSegmentItem) &&
             (item->getGfxData() != NULL)) {
            OldStreetSegmentItem* ssi = dynamic_cast<OldStreetSegmentItem*> (item);
            
            for (uint32 n = 0; n < 2; n++) { // Iterate over the two nodes.
               OldNode* curNode = ssi->getNode( n );
               int8 level = curNode->getLevel();
               if (level != 0) {
                  uint16 nbrCon = curNode->getNbrConnections();
                  for ( uint16 j = 0; j < nbrCon; j++ ) {
                     OldConnection* con = curNode->getEntryConnection( j );
                     uint32 conFromNode = con->getConnectFromNode();
                     OldNode* oppositeNode = nodeLookup(conFromNode ^ 0x80000000);
                     if (oppositeNode->getLevel() != level) {
                        oppositeNode->setLevel(level);
                        mc2dbg8 << "GSC: Setting level =" << int32(level) << endl;
                        nbrUpdatedNodes++;                  
                     }
                  }
               }
            }
         }
      }   
   }
   mc2dbg4 << "Number updated nodes: " <<  nbrUpdatedNodes << endl;
   return nbrUpdatedNodes;
}


OldRouteableItem*
GMSMap::createVirtualItem( OldRouteableItem* rItem, 
                           OldBoundrySegment::closeNode_t closeNodeVal )
{
   bool node1Close = (closeNodeVal == OldBoundrySegment::node1close);

   // This one is returned from the metod.
   OldRouteableItem* virtualItem = NULL;

   if ( rItem != NULL ) {

      // Check the type of the routeable item to find out if
      // the virtual item should be a street segment or a ferry
      ItemTypes::itemType type = rItem->getItemType();
      

      // Log what we are about to do.
      uint32 itemID = rItem->getID();
      uint32 nodeClose = 0;
      if (node1Close == true){
         nodeClose = 1;
      }
      mc2dbg4 << "To create \"virtual\" "
              << StringTable::getString(ItemTypes::getItemTypeSC(type),
                                        StringTable::ENGLISH)
              <<" for itemID 0x"
              << hex << itemID << "=" << dec << itemID
              << " for addition to the BondrySegments-vector." << endl;
      mc2dbg4 << " OldNode " << nodeClose << " close." << endl;
      

      //create a "virtual" item to add to the BS vector
      if (type == ItemTypes::streetSegmentItem) {
         virtualItem = new GMSStreetSegmentItem(
                     this, static_cast<GMSStreetSegmentItem*>(rItem));
      } else if (type == ItemTypes::ferryItem) {
         virtualItem = new GMSFerryItem(
                     static_cast<GMSFerryItem*>(rItem));
      } else {
         mc2log << error << "Can not create a virtual item"
                << " for item that is of type " << int(type) << endl;
         exit(1);
      }


      //create and set a gfxData
      GMSGfxData* gfx = (GMSGfxData*) rItem->getGfxData();
      MC2_ASSERT(gfx != NULL);
      GMSGfxData* newgfx = GMSGfxData::createNewGfxData(NULL, true);
      int c = 0;
      if ( node1Close ) {
         c = gfx->getNbrCoordinates(0) - 1;
      }
      newgfx->addCoordinate(gfx->getLat(0,c), gfx->getLon(0,c));
      newgfx->addCoordinate(gfx->getLat(0,c), gfx->getLon(0,c));
      virtualItem->setGfxData(newgfx);

      //add the virtual item to the map.
      uint32 zoom = (itemID & 0x7fffffff) >> 27;
      uint32 newID = addItem(virtualItem, zoom);
      MC2_ASSERT(newID != MAX_UINT32);
      mc2dbg4 << "virtual itemid=" << newID << endl;

      //setNodeIDs
      virtualItem->getNode(0)->setNodeID(newID & 0x7FFFFFFF);
      virtualItem->getNode(1)->setNodeID(newID | 0x80000000);
      
      // Set the level to be the same as for the original node.
      OldNode* originalNode = rItem->getNode(nodeClose);
      //OldNode* originalNode = nodeLookup( nodeID );
      virtualItem->getNode( 0 )->setLevel( originalNode->getLevel() );
      virtualItem->getNode( 1 )->setLevel( originalNode->getLevel() );

      // use the groups/regions from the parent item
      replaceRegions(virtualItem, rItem);

      //add the virtual item to the boundrySegmVec
      if (getBoundrySegments()->addBoundrySegment(newID, 
                                                  closeNodeVal)) {
         mc2dbg << "GMSMap::createVirtualItem " 
                << "Added virtual item 0x" << hex << newID 
                << "=" << dec << newID << "(node " << nodeClose
                << "), type: " 
                << StringTable::getString(ItemTypes::getItemTypeSC(type),
                                          StringTable::ENGLISH)
                << " to bs vector," << " for item 0x" << hex 
                << itemID << "=" << dec << itemID << endl;
      } else {
         mc2log << warn << "GMSMap::createVirtualItem " 
                <<"Failed to add virtual item 0x" << hex 
                << newID << "=" << dec << newID << "(node " << nodeClose
                << ") type: " 
                << StringTable::getString(ItemTypes::getItemTypeSC(type),
                                          StringTable::ENGLISH)
                << " to bs vector," << " for item 0x" << hex 
                << itemID << "=" << dec << itemID << endl;
         virtualItem = NULL;
      }
   }
   return virtualItem;
} // createVirtualItem



uint32
GMSMap::addToBoundaryNoConn( uint32 nodeID )
{
   //get the routeable item on the boundry
   uint32 itemID = 0x7FFFFFFF & nodeID;
   
   //which node is close to the boundry?
   uint32 n = 0;
   uint32 m = 1;
   bool node0closest = (((0x80000000 & nodeID) >> 31) == 0);
   OldBoundrySegment::closeNode_t closeNodeVal;
   if (node0closest) {
      closeNodeVal = OldBoundrySegment::node0close;
   } else {
      closeNodeVal = OldBoundrySegment::node1close;
      n = 1;
      m = 0;
   }

   OldRouteableItem* curRI = 
      dynamic_cast<OldRouteableItem*>(itemLookup(itemID));
   if ( curRI == NULL ){
      mc2log << error << "GMSMap::addToBoundaryNoConn" 
             << " Could not find item to create vitual item from." << endl;
      return MAX_UINT32;
   }
      
   OldRouteableItem* virtualItem = 
      createVirtualItem( curRI, closeNodeVal );
   if ( virtualItem == NULL ) {
      mc2log << error << "GMSMap::addToBoundaryNoConn" 
             << " Could not add virtual item." << endl;
      return MAX_UINT32;
   }


   // Return the node not close to the boundry.
   uint32 result;
   if ( node0closest ){
      result = virtualItem->getNode(1)->getNodeID();
   }
   else {
      result = virtualItem->getNode(0)->getNodeID();
   }
   
   return result;

} // addToBoundaryNoConn

uint32
GMSMap::addToBoundary( uint32 nodeID )
{
   //get the routeable item on the boundry
   uint32 itemID = 0x7FFFFFFF & nodeID;
   
   //which node is close to the boundry?
   uint32 n = 0;  // close node
   uint32 m = 1;  // far node
   bool node0closest = (((0x80000000 & nodeID) >> 31) == 0);
   OldBoundrySegment::closeNode_t closeNodeVal;
   if (node0closest) {
      closeNodeVal = OldBoundrySegment::node0close;
   } else {
      closeNodeVal = OldBoundrySegment::node1close;
      n = 1;
      m = 0;
   }
   
   // Create the boundry segments vector if it does not exist
   if ( m_segmentsOnTheBoundry == NULL ) {
      m_segmentsOnTheBoundry = new OldBoundrySegmentsVector();
   }
      
   // First make sure that the node is not already present on the boundary.
   bool found = false;
   uint32 i = 0;
   while ( ( ! found ) && ( i < m_segmentsOnTheBoundry->getSize() ) ) {
      OldBoundrySegment* bs = static_cast<OldBoundrySegment*> 
         ( m_segmentsOnTheBoundry->getElementAt( i ) );
      OldRouteableItem* virtualBS = static_cast<OldRouteableItem*> 
         ( itemLookup( bs->getConnectRouteableItemID() ) );
      for ( uint32 j = 0; j < 2; ++j ) {
         OldNode* virtualNode = virtualBS->getNode( j );
         for ( uint32 k = 0; k < virtualNode->getNbrConnections(); ++k ) {
            if ( ( ( virtualNode->getEntryConnection( k )
                    ->getConnectFromNode() ) & 0x7fffffff ) == itemID ) {
               // This item is already present on the boundary.
               // What about the node?
               if ( bs->getCloseNodeValue() == closeNodeVal ) {
                  // The node is already present on the boundary!
                  // (= has a virtual bs connected to it)
                  found = true;
               }
            }
         }
      }
      ++i;
   }
   if ( found ) {
      mc2dbg2 << "addToBoundary: Not adding " << nodeID << " since it is "
              << " already present on the boundary." << endl;
      return MAX_UINT32;
   }



   OldRouteableItem* curRI = 
      dynamic_cast<OldRouteableItem*>(itemLookup(itemID));
   if ( curRI == NULL ){
      mc2log << error << "GMSMap::addToBoundary" 
             << " Could not find item to create vitual item from." << endl;
      return MAX_UINT32;
   }
      
   OldRouteableItem* virtualItem = 
      createVirtualItem( curRI, closeNodeVal );
   if ( virtualItem == NULL ) {
      mc2log << error << "GMSMap::addToBoundary" 
             << " Could not add virtual item for external node: "
             << nodeID << endl;
      return MAX_UINT32;
   }

   //create connection between curRI and nodeRI
   addConnection(curRI->getNode(n), virtualItem->getNode(m), 0);
      
   //set turndescription followroad for these connections.
   uint16 curNbrCon = curRI->getNode(n)->getNbrConnections();
   
   MC2_ASSERT(virtualItem->getNode(m)->getNbrConnections() == 1);
   
   MC2_ASSERT(curRI->getNode(n)->getEntryConnection(curNbrCon-1)->
              getConnectFromNode() == 
              virtualItem->getNode(n)->getNodeID());
   curRI->getNode(n)->getEntryConnection(curNbrCon-1)->
      setTurnDirection(ItemTypes::FOLLOWROAD);
   curRI->getNode(n)->getEntryConnection(curNbrCon-1)->
      setCrossingKind(ItemTypes::NO_CROSSING);
   virtualItem->getNode(m)->getEntryConnection(0)->
      setTurnDirection(ItemTypes::FOLLOWROAD);
   virtualItem->getNode(m)->getEntryConnection(0)->
      setCrossingKind(ItemTypes::NO_CROSSING);
   
   

   return virtualItem->getID();
} // addToBoundary

void 
GMSMap::addMultiConnection( uint32 firstMultiNodeID,
                            OldNode* lastMultiNode,
                            const list<uint32>& expandedNodeIDs,
                            uint32 vehicleRestriction,
                            ItemTypes::turndirection_t turnDirection )
{
   uint32 lastMultiNodeID = lastMultiNode->getNodeID();   
   if ( firstMultiNodeID == lastMultiNodeID ) {
      // Something is wrong. Perhaps a bug in the GDF data with
      // too many features in the 2103 relation record
      mc2log << error << "GMSMap::addMultiConnection "
             << "firstMultiNodeID same as lastMultiNodeID "
             << firstMultiNodeID << endl;
      mc2log << error << "Comment out assert to generate the mcm map"
             << " ignoring the prohibited manouver"
             << " causing this strange multi-connection"
             << endl;
      MC2_ASSERT(false);
      // Comment out the assert, if we want to generate mcm maps 
      // Then, the prohibited manouver relation will be ignored
      // Need comment for NAM 2009.02, EU 2009.02: gr(+pt)
      return;
   }

   // Check if the node to connect to is a virtual segment and in that
   // case, create a new one since we only can have one connection to each
   // virtual segment.
   if ( m_segmentsOnTheBoundry != NULL ) {
      bool found = false;
      uint32 virtualNodeID = MAX_UINT32;
      uint32 i = 0;
      while ( ( ! found ) && ( i < m_segmentsOnTheBoundry->getSize() ) ) {
         OldBoundrySegment* bs = static_cast<OldBoundrySegment*> 
            ( m_segmentsOnTheBoundry->getElementAt( i ) );
         OldRouteableItem* virtualBS = static_cast<OldRouteableItem*> 
            ( itemLookup( bs->getConnectRouteableItemID() ) );
         for ( uint32 j = 0; j < 2; ++j ) {
            OldNode* virtualNode = virtualBS->getNode( j );
            if ( virtualNode->getNodeID() == lastMultiNodeID ){
               found = true;
               virtualNodeID= virtualNode->getNodeID();
            }            
         }
         ++i;
      }
      if ( found ) {
         // Create a new virtual segment from the one we originally wanted
         // to connect to.
         lastMultiNodeID = addToBoundaryNoConn( virtualNodeID );
         if ( lastMultiNodeID == MAX_UINT32 ){
            mc2log << error << "GMSMap::addMultiConnection" 
                   << " Failed to add multi connection, last node invalid"
                   << " (first node ID: " << firstMultiNodeID << ")" 
                   << endl;
            exit(1);
         }
         else {
            lastMultiNode = this->nodeLookup(lastMultiNodeID);
         }
      }
   }



   // Create the connection.
   OldConnection* conn = 
      new OldConnection( firstMultiNodeID );
   conn->setVehicleRestrictions( vehicleRestriction );
   conn->setTurnDirection( turnDirection );
   lastMultiNode->addConnection( conn, *this );

   // Add the multi connection to the maps node expansin table for multi
   // connections.
   m_nodeExpansionTable[ make_pair( firstMultiNodeID, lastMultiNodeID ) ] =
      expandedNodeIDs;

   // Print the result to the log.
   mc2dbg << "GMSMap::addMultiConnection - " << firstMultiNodeID
          << ", " << lastMultiNodeID << " => ";
   copy( expandedNodeIDs.begin(), expandedNodeIDs.end(), 
         ostream_iterator<uint32>( mc2log, " " ) );
   mc2log << endl;

} // addMultiConnection

uint32
GMSMap::removeConnsWithDiffLevel()
{
   uint32 nbrRemoved = 0;
   
   for(uint32 z=0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for(uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z, i);
         if (item != NULL) {
            OldRouteableItem* ri = dynamic_cast<OldRouteableItem*> ( item );

            if ( ri != NULL ) {
               // For each node:
               for ( uint32 j = 0; j < 2; ++j ) {
                  OldNode* node = ri->getNode( j );
                  
                  int level = node->getLevel();
                  uint32 k = 0;
                  while ( k < node->getNbrConnections() ) {
                     OldConnection* conn = node->getEntryConnection( k );
                     // Note that we need to swap the node that the
                     // connection is coming from in other to be able
                     // to compare levels.
                     OldNode* oppositeNode = nodeLookup( 
                        conn->getConnectFromNode() ^ 0x80000000 );
                     if ( oppositeNode->getLevel() != level ) {
                        // The nodes were not on the same level!
                        // Remove connection
                        if (! isPartOfMultiConnection( 
                                 conn->getConnectFromNode(),
                                 node->getNodeID() ) ) {
                           node->deleteConnection( k, *this );
                           ++nbrRemoved;
                        } else {
                           // The connection is actually part of a 
                           // multi connection. Therefore just set that
                           // noone is allowed to pass through the
                           // connection.
                           conn->addVehicleRestrictionsAll();
                           ++k;
                        }
                     } else {
                        ++k;
                     }
                  }
               }
            }
         }
      }
   }
   
   return nbrRemoved;
}


uint32
GMSMap::updateForeignBuaNames()
{
   uint32 nbrAddedNames = 0;
   for ( uint32 curZoom = 0; 
         curZoom < NUMBER_GFX_ZOOMLEVELS; curZoom++ ) {
      // Go through all items
      for ( uint32 itemNbr = 0;
            itemNbr < getNbrItemsWithZoom( curZoom ); itemNbr++ ) {
         OldBuiltUpAreaItem* bua = dynamic_cast<OldBuiltUpAreaItem*> 
            ( getItem( curZoom, itemNbr ) );
         if ( bua != NULL ) {
            // Handles BUA in municipal.
            
            // Get the municipal of the bua.
            uint32 munID = getRegionID( bua, ItemTypes::municipalItem );
            OldMunicipalItem* mun = static_cast<OldMunicipalItem*> 
               ( itemLookup( munID ) );
            if ( mun != NULL ) {
               nbrAddedNames += updateBUANamesFromMun(bua, mun);
            }
         }
         else {
            OldMunicipalItem* mun = dynamic_cast<OldMunicipalItem*> 
               ( getItem( curZoom, itemNbr ) );
            if ( mun != NULL ){
               // Handles municipal in BUA.
               
               // Get the the bua of the municipal.
               uint32 buaID = getRegionID( mun, ItemTypes::builtUpAreaItem );
               OldBuiltUpAreaItem* bua = static_cast<OldBuiltUpAreaItem*> 
                  ( itemLookup( buaID ) );
               if ( bua != NULL ) {
                  nbrAddedNames += updateBUANamesFromMun(bua, mun);
               }
            }  
         }
      }
   }
   return nbrAddedNames;
}

uint32
GMSMap::updateBUANamesFromMun(OldBuiltUpAreaItem* bua, OldMunicipalItem* mun){
   uint32 nbrAddedNames = 0;

   if ( bua->hasCommonLangName( mun ) ) {
      // Bua and municipal have common names.
      // Add all the names of the municipal to the bua unless
      // they are already present.
      for ( uint32 i = 0; i < mun->getNbrNames(); ++i ) {
         // Check if the name is present in among the bua names
         bool found = false;
         uint32 j = 0;
         while ( ( j < bua->getNbrNames() ) && ! found ) {
            if ( ( bua->getNameLanguage( j ) == 
                   mun->getNameLanguage( i ) ) &&
                 ( bua->getNameType( j ) ==
                   mun->getNameType( i ) ) &&
                 ( bua->getStringIndex( j ) ==
                   mun->getStringIndex( i ) ) ) {
               found = true;
            } else {
               ++j;
            }
         }
         // Not found. Add the name to the bua.
         if ( ! found ) {
            addNameToItem( bua, 
                           getName( mun->getStringIndex( i ) ),
                           mun->getNameLanguage( i ),
                           mun->getNameType( i ) );
            mc2dbg << here << " Added names from muicipal to BUA: "
                   << bua->getID() << endl;
            ++nbrAddedNames;
         }
      }
   }
   return nbrAddedNames;
}


uint32
GMSMap::updateCongestionSpeedLimits()
{
   mc2dbg << "Started updateCongestionSpeedLimits" << endl;
   uint32 nbrChangedSpeeds = 0;
   
   // Key: name of city.
   // Value: pair.first - square max dist to cc in meters
   //        pair.second - percent of normal speed in the cc
   map<MC2String, congestionInfo_t> congestedCities;
   
   congestionInfo_t londonInfo;

   // --- LONDON ---
   
   // 30% of normal speed in cc.
   londonInfo.m_percentOfMaxSpeedInCC = 0.3;
   // Maximum distance to citycentre for a road is 15 km.
   londonInfo.m_sqCongestionRadiusMeter = SQUARE( 15000.0 ); 
   londonInfo.m_ccCoord.lat = 614421282;
   londonInfo.m_ccCoord.lon = -1505578;
   congestedCities.insert( make_pair( MC2String( "London" ), londonInfo ) );

   // The built up areas to set congestion speed limits for are put in this
   // one.
   typedef map< uint32, congestionInfo_t> cgstInfoByBuaID_t;
   cgstInfoByBuaID_t cgstInfoByBuaID;
      
   // Minimum city area is a square with 10 km sides.
   const float64 minCityAreaMC2 = 
      SQUARE( 10000.0 * GfxConstants::METER_TO_MC2SCALE );
  
   uint32 buaZoom = 0;

   for ( uint32 i = 0; i < getNbrItemsWithZoom( buaZoom ); ++i ) {
      OldBuiltUpAreaItem* bua = dynamic_cast<OldBuiltUpAreaItem*> 
         ( getItem( buaZoom, i ) );
      if ( ( bua != NULL ) && ( bua->getGfxData() != NULL ) ) {
         // Bua with gfx data

         // If we use index areas, we have to use the index area buas
         // for the congestion speed limit
         // No ssi is located in the other display-buas
         if ( NationalProperties::useIndexAreas(
                  getCountryCode(), getMapOrigin()) &&
              itemNotInSearchIndex(bua->getID()) ) {
            continue;
         }
         
         // Calculate BUA area
         float64 buaAreaMC2 = 0;
         uint32 poly = 0;
         while ( poly < bua->getGfxData()->getNbrPolygons() ) {
            buaAreaMC2 += fabs( bua->getGfxData()->polygonArea( poly ) );
            ++poly;
         }

         if ( buaAreaMC2 > minCityAreaMC2 ) {
            // Large enough area of BUA.

            const char* buaName = getBestItemName( bua,
                                               LangTypes::english );

            mc2dbg << "BUA: " << bua->getID() << " "
                   << buaName << " area:" << buaAreaMC2 
                   << " is large enough" << endl;
            
            map<MC2String, congestionInfo_t>::const_iterator cgstInfoIt;
            if ( buaName != NULL ){
               cgstInfoIt = 
                  congestedCities.find( MC2String( buaName ));
            }
            else {
               cgstInfoIt = congestedCities.end();
            }

            if ( cgstInfoIt != congestedCities.end() ){
               const congestionInfo_t cgstInfo = cgstInfoIt->second;
               mc2dbg << "BUA name found among congestion cities." << endl;
               
               GfxData* buaGfx = bua->getGfxData();
               if ( buaGfx->insidePolygon( cgstInfo.m_ccCoord ) );
               {
                  // This is one of the congested cities.
                  mc2dbg << "BUA city center coordinate is inside this"
                         << "BUA. Use it for setting congestion speed."
                         << endl;   
                  cgstInfoByBuaID[ bua->getID() ] = cgstInfo;
               }
            }
         }
      }
   }


   // Print some info.
   mc2log << info << "Will set congestion speed for " 
          << cgstInfoByBuaID.size() << " following BUAs:" << endl;
   for (cgstInfoByBuaID_t::const_iterator infoIt = cgstInfoByBuaID.begin();
        infoIt != cgstInfoByBuaID.end();
        ++infoIt )
   {
      mc2log << info << "BUA ID:" << infoIt->first << " center:(" 
             << infoIt->second.m_ccCoord.lat << ", " 
             << infoIt->second.m_ccCoord.lon << ") radious:" 
             << sqrt(infoIt->second.m_sqCongestionRadiusMeter)
             << " center speed percentage:" 
             << infoIt->second.m_percentOfMaxSpeedInCC << endl;
   }


   // Change the speedlimits for all roads close to the congested 
   // citycentres.
   
   for ( uint32 curZoom = 0; curZoom < NUMBER_GFX_ZOOMLEVELS; curZoom++ ) {
      for ( uint32 itemNbr = 0;
            itemNbr < getNbrItemsWithZoom( curZoom ); itemNbr++ ) {
         OldStreetSegmentItem* ssi = dynamic_cast<OldStreetSegmentItem*>
            ( getItem( curZoom, itemNbr ) );
         if ( ssi != NULL ) {

            // Check if the ssi is located in any bua which is among
            // the congested cities.
            uint32 congestedCityID = MAX_UINT32;
            for ( uint32 g = 0; g < ssi->getNbrGroups(); g++ ) {
               OldItem* group = itemLookup(ssi->getGroup(g));
               MC2_ASSERT(group != NULL);
               if (group->getItemType() != ItemTypes::builtUpAreaItem) {
                  continue;
               }
               cgstInfoByBuaID_t::const_iterator it =
                  cgstInfoByBuaID.find( group->getID() );
               if ( it != cgstInfoByBuaID.end() ) {
                  // the ssi is in a congested city
                  if ( congestedCityID != MAX_UINT32 ) {
                     // was already in another congested city, same?
                     if ( cgstInfoByBuaID.find(congestedCityID) == it ) {
                        // ok
                     } else {
                        mc2log << warn << "Item " << ssi->getID()
                             << " in different congested cities "
                             << congestedCityID << " and "
                             << group->getID() << endl;
                        // accept, use the bua you first found
                     }
                  }
                  congestedCityID = group->getID();
               }
            }
            if ( congestedCityID == MAX_UINT32 ) {
               continue;
            }
            cgstInfoByBuaID_t::const_iterator it =
               cgstInfoByBuaID.find( congestedCityID );

            //cgstInfoByBuaID_t::const_iterator it =
            //   cgstInfoByBuaID.find( getRegionID( ssi, 
            //                           ItemTypes::builtUpAreaItem ) );
            if ( it != cgstInfoByBuaID.end() ) {
               MC2Coordinate coord = it->second.m_ccCoord;
               float64 sqRadius = 
                  it->second.m_sqCongestionRadiusMeter;
               float64 percentMaxSpeed = 
                  it->second.m_percentOfMaxSpeedInCC;
               
               // This ssi is located inside one of the buas.
               // Adjust the speedlimit based on how far it is from
               // the citycentre.
               float64 sqDistToCC = ssi->getGfxData()
                  ->squareDistTo( coord.lat, coord.lon );
               if ( sqDistToCC < sqRadius ) { 
                  float64 penaltyFactor = sqDistToCC / sqRadius;

                  // Adjust speed limits for both nodes.
                  for ( uint32 i = 0; i < 2; ++i ) {
                     OldNode* node = ssi->getNode( i );
                     // If in the middle of the citycentre, then only keep
                     // 30% of the original speed. If furthest away
                     // then keep the original speed. 
                     byte newSpeedLimit = byte( rint(
                        node->getSpeedLimit() * 
                        ( percentMaxSpeed + 
                          (1 - percentMaxSpeed) * penaltyFactor ))); 
                     mc2dbg8 << "Speedlimit reduced for 0x" << hex 
                             << node->getNodeID() << dec << " from " 
                             << (int) node->getSpeedLimit() << " to "
                             << (int) newSpeedLimit << endl;
                     node->setSpeedLimit( newSpeedLimit );
                  }
                  ++nbrChangedSpeeds;
               }
            }
         }
      }
   }
   return nbrChangedSpeeds;
}
     
uint32
GMSMap::calcRegionStats( OldItem* item, 
                         map<uint32, uint32>& regionByNbrOccasions,
                         ItemTypes::itemType type )
{
   uint32 totalNbr = 0;
   // Reversed map. Key: region id, Value: occassions
   map<uint32, uint32> occasionsByRegion;
   for ( uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++ ) {
      for ( uint32 i = 0;
            i < getNbrItemsWithZoom( z ); i++ ) {
         OldItem* ssi = getItem( z, i );
         if ( ( ssi != NULL ) && 
              ( ssi->getItemType() == ItemTypes::streetSegmentItem ) ) {
            if ( ssi->memberOfGroup(item->getID()) ){
               vector<OldItem*> groups = getRegions(ssi, type);
               for ( uint32 g=0; g<groups.size(); g++){
                  uint32 groupID = groups[g]->getID();
                  //getRegionID( ssi, type );
                  if ( occasionsByRegion.find( groupID ) ==
                       occasionsByRegion.end() ) {
                     occasionsByRegion[ groupID ] = 1;
                  } else {
                     occasionsByRegion[ groupID ]++;
                  }
               }
               ++totalNbr;              
            }
         }
      }
   }

   // Reverse occasionsByRegion into regionByNbrOccasions.
   for ( map<uint32, uint32>::const_iterator it = 
            occasionsByRegion.begin(); it != occasionsByRegion.end();
         ++it ) {
      regionByNbrOccasions[ it->second ] = it->first;
   }
   return totalNbr;
}

   
uint32
GMSMap::updateMetropolitanBuaMunLoc()
{
   mc2dbg << "Running updateMetropolitanBuaMunLoc" << endl;
   uint32 nbrChanged = 0;

   // Municipals
   mc2dbg << "updateMetropolitanBuaMunLoc: Handling municipals" << endl;
   for ( uint32 curZoom = 0; curZoom < NUMBER_GFX_ZOOMLEVELS; curZoom++ ) {
      for ( uint32 itemNbr = 0;
            itemNbr < getNbrItemsWithZoom( curZoom ); itemNbr++ ) {
         OldItem* mun = getItem( curZoom, itemNbr );
         if ( ( mun != NULL ) && 
              ( mun->getItemType() == ItemTypes::municipalItem )  &&
              ( !itemNotInSearchIndex(mun->getID()) ) ){
            // No need to set location of municipals that are not part of 
            // search index.
            map<uint32, uint32> buaByNbrOccasions;
            
            uint32 totalMun = 
               calcRegionStats( mun, buaByNbrOccasions, 
                                ItemTypes::builtUpAreaItem );

            if ( buaByNbrOccasions.size() > 0 ) {
               uint32 mostCoveredBuaOccations = 
                  buaByNbrOccasions.rbegin()->first;
               float percentage = mostCoveredBuaOccations / (float) totalMun;
               // 1. In case all the municipals streets
               //    belong to a bua, then the municipal is considered
               //    to belong to that bua.
               mc2dbg << "Mun(" << mun->getID() <<  ") in BUA(" 
                      << buaByNbrOccasions.rbegin()->second << "): " 
                      << percentage*100 << "% (" 
                      << mostCoveredBuaOccations << " / " << totalMun << ")" 
                      << endl;
               if ( percentage > 0.99 ) {
                  OldItem* bua = itemLookup( 
                        buaByNbrOccasions.rbegin()->second );
                  if ( bua != NULL ) {
                     if (itemNotInSearchIndex(bua->getID()) ){
                        mc2dbg << "BUA(" << bua->getID() 
                               << ") not in search index." << endl;
                     }
                     else {
                        bool addBuaAsGroupToMun = false;
                        if ( ( bua->getGfxData() != NULL ) &&
                             ( mun->getGfxData() != NULL ) ){
                           
                           // In case gfxdatas are present, require the
                           // bua:s gfx to be larger than the municipal's.
                           float buaBBArea = 
                              bua->getGfxData()->getBBoxArea();
                           float munBBArea = 
                              mun->getGfxData()->getBBoxArea();
                           float eps = buaBBArea / pow( 10.0, 16.0 );
                           
                           if ( ( munBBArea - buaBBArea ) < eps ){
                              // BUA area larger than the municipal area.
                              addBuaAsGroupToMun = true;
                           }
                        }
                        else {
                           addBuaAsGroupToMun = true;
                        }
                        
                        if ( addBuaAsGroupToMun ){
                           
                           clearRegionsForItem( bua, 
                                                ItemTypes::municipalItem );
                           clearRegionsForItem( mun, 
                                                ItemTypes::builtUpAreaItem );
                           bool result = addRegionToItem( mun, bua );
                           if ( result ) {
                              mc2dbg1 << "[GMap] Metro: 1. " << mun->getID()  
                                      << " => " 
                                      << bua->getID() << endl;
                              ++nbrChanged;
                           }
                           else {
                              mc2dbg1 << "[GMap] Metro: 1. NOT " 
                                      << mun->getID()  
                                      << " => " 
                                      << bua->getID() << endl;
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
   
   // Buas
   mc2dbg << "updateMetropolitanBuaMunLoc: Handling BUAs" << endl;
   for ( uint32 curZoom = 0; curZoom < NUMBER_GFX_ZOOMLEVELS; curZoom++ ) {
      for ( uint32 itemNbr = 0;
            itemNbr < getNbrItemsWithZoom( curZoom ); itemNbr++ ) {
         OldItem* bua = getItem( curZoom, itemNbr );
         if ( ( bua != NULL ) && 
              ( bua->getItemType() == ItemTypes::builtUpAreaItem ) &&
              ( !itemNotInSearchIndex(bua->getID()))  ) {

            map<uint32, uint32> munByNbrOccasions;
            uint32 totalBua = 
               calcRegionStats( bua, munByNbrOccasions, 
                                ItemTypes::municipalItem );
            
            // Whether the bua is considered small or not.
            bool smallBua = totalBua < 100;

            // 2. Similar name, and more of the municipal lies
            //    in the bua than the other way around:
            //    municipal => bua.
            //
            // 3. Similar name, and more of the bua lies
            //    in the municipal than the other way around:
            //    bua => municipal.
            //    
            map<uint32, uint32>::reverse_iterator it = 
               munByNbrOccasions.rbegin();
            bool regionSet = false;
            bool buaRegionSet = false; // Don't set any more municipal 
            // locations to the BUA if this one is true.
            while ( it != munByNbrOccasions.rend() ) {
               // while (! regionSet ) ) 
               OldItem* mun = itemLookup( it->second );
               mc2dbg << "Handling mun " << it->second << " of bua: " 
                      << bua->getID() << endl;
               float buaInMunPercentage = it->first / (float) totalBua;
               if ( ( mun != NULL ) && 
                    oneNameSameCase( bua, mun ) &&
                    !itemNotInSearchIndex(mun->getID()) ) {
                  mc2dbg << "Mun and BUA share name." << endl;
                  if ( hasRegion( mun, bua ) ) {
                     // Municipal already belongs to bua.
                     regionSet = true;
                  } else {

                     // Bua and municipal with same name.
                     // Check which one that belongs most to the other.
                     map<uint32, uint32> buaByNbrOccasions;
                     uint32 totalMun = 
                        calcRegionStats( mun, buaByNbrOccasions, 
                                         ItemTypes::builtUpAreaItem );
                     map<uint32, uint32>::reverse_iterator buaIt = 
                        buaByNbrOccasions.rbegin();
                     bool found = false;
                     while ( buaIt != buaByNbrOccasions.rend() &&
                             ! found ) {
                        if ( buaIt->second == bua->getID() ) {
                           found = true;
                        } else {
                           ++buaIt;
                        }
                     }

                     bool addMunToBua = false;
                     if ( found == true ) {
                        float munInBuaPercentage = 
                              buaIt->first / (float) totalMun;
                        if ( munInBuaPercentage > buaInMunPercentage ) {
                           addMunToBua = true;
                        }
                     }
                    
                     if ( addMunToBua ) {
                        // Mun -> Bua
                        // Clear old regions.
                        clearRegionsForItem( bua, 
                                           ItemTypes::municipalItem );
                        clearRegionsForItem( mun, 
                                           ItemTypes::builtUpAreaItem );
                        bool result = addRegionToItem( mun, bua );
                        if ( result ){
                           mc2dbg1 << "[GMap] Metro: 2 " << mun->getID()  
                                   << " => " 
                                   << bua->getID() << endl;
                           ++nbrChanged;
                        }
                        else {
                           mc2dbg1 << "[GMap] Metro: 2 NOT " << mun->getID()  
                                   << " => " 
                                   << bua->getID() << endl;  
                        }
                     } else if ( !buaRegionSet ){
                        // Bua -> Mun
                        // Clear old regions.
                        clearRegionsForItem( bua, 
                                             ItemTypes::municipalItem );
                        bool result = addRegionToItem( bua, mun );
                        if ( result ){
                           mc2dbg1 << "[GMap] Metro: 3 " << bua->getID()  
                                   << " => " 
                                   << mun->getID() << endl;
                           ++nbrChanged;
                        }
                        else {
                           mc2dbg1 << "[GMap] Metro: 3 NOT " << bua->getID()  
                                   << " => " 
                                   << mun->getID() << endl;
                        }
                        buaRegionSet = true;
                     }
                     else {
                        mc2dbg1 << "[GMap] Metro: mun region of BUA already "
                                << "set." << endl;
                     }
                     regionSet = true;
                  }
               }
               ++it;
            }

            if ( ! regionSet ) {
               // Bua and municipal doesn't have the same name.
               if ( munByNbrOccasions.size() > 0 ) {
                  
                  float percentage = munByNbrOccasions.rbegin()->first / 
                                     (float) totalBua;
                  // 4. In case more than 80 % of the bua:s streets
                  // belong to the municpal OR
                  // the bua is very small: bua => municipal
                  if ( percentage > 0.80 || smallBua ) {
                     OldItem* mun = itemLookup( 
                           munByNbrOccasions.rbegin()->second );
                     if ( ( mun != NULL ) && 
                          (! hasRegion( mun, bua ) ) ) {
                        clearRegionsForItem( bua, 
                                             ItemTypes::municipalItem );
                        addRegionToItem( bua, mun );
                        ++nbrChanged;
                        mc2dbg1 << "[GMap]: Metro 4. " 
                                << bua->getID() << " => " 
                                << mun->getID() << endl;
                     }
                  }
               }
            }
         }
      }
   }

   return nbrChanged;
} // updateMetropolitanBuaMunLoc

uint32 
GMSMap::getMultiConnsFromNode( uint32 fromNodeID, 
                               list<uint32>& toNodes )
{
   uint32 nbrFound = 0;
   map<multiNodes_t, expandedNodes_t>::const_iterator expandIt = 
      m_nodeExpansionTable.begin();
   while ( expandIt != m_nodeExpansionTable.end() ) {
      if ( expandIt->first.first == fromNodeID ) {
         toNodes.push_back( expandIt->first.second );
         ++nbrFound;
      } 
      ++expandIt;
   }
   return nbrFound;
}

bool 
GMSMap::isPartOfMultiConnection( uint32 firstNodeID,
                                 uint32 secondNodeID )
{
   // Note that this method is linear in number of multi connection nodes.
   
   map<multiNodes_t, expandedNodes_t>::const_iterator expandIt = 
      m_nodeExpansionTable.begin();
   
   while ( expandIt != m_nodeExpansionTable.end() ) {
      
      const expandedNodes_t& expNodes = expandIt->second;
      // First check the start and end of the list.
      if ( ( ( expandIt->first.first == firstNodeID ) &&
             ( expNodes.front() == secondNodeID ) ) ||
           ( ( expNodes.back() == firstNodeID ) && 
             ( expandIt->first.second == secondNodeID ) ) ) {
         // Found the nodes in the start / end of the list.
         return true;
      } else {
         // Check the expanded node list.
         expandedNodes_t::const_iterator it = expNodes.begin();
         expandedNodes_t::const_iterator nextIt = it;
         ++nextIt;
         while ( nextIt != expNodes.end() ) {
            if ( ( *it == firstNodeID ) && ( *nextIt == secondNodeID ) ) {
               // Found the nodes in the list.
               return true;
            }
            
            it = nextIt;
            ++nextIt;
         }
      }
      ++expandIt;
   }
  
   // If we got this far, the nodes where not part of a multi connection.
   return false;
}


bool 
GMSMap::extendAbbreviations( ItemTypes::itemType type ){
   bool result = true;  // Returned by the method.
   uint32 nbrExtendedNames = 0;

   // Loop all names of all items of type.
   for(uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z, i);
         if ( (item != NULL) && (item->getItemType() == type ) ){

            // Used as outparameters.
            LangTypes::language_t strLang;
            ItemTypes::name_t strType;
            uint32 strIndex;

            for ( uint32 n=0; n < item->getNbrNames(); n++){
               item->getNameAndType(n, strLang, strType, strIndex);

               vector<MC2String> expandedNames = 
                  AbbreviationTable::fullExpand( this->getName(strIndex),
                                                 strLang,
                                                 this->getCountryCode() );
               
               for ( uint32 i = 0; i<expandedNames.size(); ++i){
                  nbrExtendedNames++;
                  this->addNameToItem( item,
                                       expandedNames[i].c_str(),
                                       strLang,
                                       ItemTypes::synonymName );
               } // for extended name

            } // for original name
         }
      } // for item
   } // for zoomlevel

   mc2log << info << "extendAbbreviations: added " << nbrExtendedNames
          << " extended names" << endl;
   return result;
} // extendAbbreviations


bool
GMSMap::getGDFRef(GDFRef& gdfRef){
   bool result = true;
   // Compose gdf ref file name.
   //
   mc2log << info << "GMSMap::getGDFRef: " 
          << "Reding gdf ref file." << endl;
   // Find last slash.
   MC2String mapFileName( getFilename() );
   mc2TextIterator it = mapFileName;
   mc2TextIterator beginIt = mapFileName;
   mc2TextIterator lastSlashIt = it;
   while (*it != 0){
      if ( *it == '/' ){
         lastSlashIt = it;
      }
      ++it;
   }
   mc2TextIterator endIt = it;
   // Put gdf ref file name together.
   MC2String gdfRefFileName = 
      copyMC2String( beginIt,
                     ++lastSlashIt );
   MC2String mapBaseName = copyMC2String( lastSlashIt,
                                          endIt);
   MC2String gdfRefFileNameTemp = gdfRefFileName+"temp/"; // look in temp 
   // directory if not found locally.
   
   gdfRefFileNameTemp+=mapBaseName;
   gdfRefFileName+=mapBaseName;
   gdfRefFileNameTemp+=".gdf_ref";
   gdfRefFileName+=".gdf_ref";

   mc2dbg << "GDF ref file name: " << gdfRefFileName << endl;

   // Read GDF IDs table.
   result = gdfRef.readGDFRefFile(gdfRefFileName);
   if (! result ){
      result = gdfRef.readGDFRefFile(gdfRefFileNameTemp);
      if (!result){
         mc2log << error << "Failed to read"
                << " gdf ref file. Filename:" << gdfRefFileName << endl;
         exit(1);
      }
   }
   return result;
}

bool
GMSMap::addZipsFromZipPOIFile( ZipSideFile zipSideFile, 
                               MC2String sectionedZipsPath )
{
   // Format of the data rows in the zip POI file
   // "||zipcode|mc2-lat|mc2-lon"
   // example:
   // ||8360223|-706558699|-334613299
   // ||8360224|-706554400|-334608400
   
   // There can be only one zip poi file with all zipcodes for a country
   // in the sectionedZipsPath directory.
   //
   // Fixme: if you have several zip poi files, need to adjust this method.
   // Several zip poi files, in some way sectionized, e.g. one file per
   // GDF section or any other possible way to sectionize the zip poi file 
   // data not to have too many zip points to work with at the same time.
   // The name of the sectionized files, might be used to select which files
   // that shoudl be applied to this mcm map, no need loading a zip poi file
   // for this map, if it does not contain data that fits this map.
   
   bool result = true;
   

   // One and only one file in the sectionedZipsPath directory
   // find the one and only zip poi file
   MC2String filePath = "";
   bool foundOneFile = false;
   char** entries = NULL;
   int nbr = Utility::directory(sectionedZipsPath.c_str(), &entries);
   if ( nbr < 0 ) {
      mc2log << error << "[GMSMap::addZipsFromZipPOIFile]:"
             << " Could not read dir for "
             << "sectionedZipsPath" << " : " << strerror(errno) << endl;
      MC2_ASSERT(false);
   } else {
      for( int i = 0; i < nbr; ++i ) {
         if ( StringUtility::endsWithStr( entries[i], ".txt", true ) ) {
            // This is a zip-by-section-file.
            if ( ! foundOneFile ) {
               filePath = entries[i];
               foundOneFile = true;
            } else {
               mc2log << error << "More than one txt file in "
                      << "sectionedZipsPath: " << filePath 
                      << " and " << entries[i] << endl;
               MC2_ASSERT(false);
            }
         }
         mc2dbg8 << "Entry : " << entries[i] << endl;
      }
   }
   Utility::deleteStrArray(entries, nbr);
   
   // Load the zip poi file
   MC2String secZipFilePath = sectionedZipsPath + "/" + filePath;
   mc2dbg8 << "zip file " << secZipFilePath << endl;
   zipSideFile.loadZipPOIFile(secZipFilePath);
   
   
    // Remove all old zips.
   set<uint32>removedZipsIDs;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
         OldZipCodeItem* zipItem = 
            dynamic_cast<OldZipCodeItem*>(getItem(z, i));
         if ( zipItem != NULL ){
            uint32 zipID = zipItem->getID();
            if (removeItem(zipID,
                           false, // Not updating hash table
                           false  // Not updating admin centers
                           )){
               removedZipsIDs.insert(zipID);
            }
            else {
               mc2dbg << "Problem with removing item ID: " 
                      << zipItem->getID() << endl;
               exit(1);
            }
         }
      }
   }
   mc2dbg << "Building hash table." << endl;
   buildHashTable();
   mc2dbg << "Removing zips from admin centers table." << endl;
   removeAdminCenters(removedZipsIDs);
   mc2dbg << "Removed " << removedZipsIDs.size() 
          << " old zip codes." << endl;



   // Add the new zips.
   set<MC2String> notAddedZips;
   uint32 nbrNew = 0;
   vector<MC2String> zipNames = 
      zipSideFile.getMapZipItemNames();
   for (uint32 i=0; i<zipNames.size(); i++){
      bool add = true;
      if ( add ){
         addZipFromName(NULL, zipNames[i]);
         nbrNew++;
      }
      else {
         notAddedZips.insert(zipNames[i]);
      }
   }
   mc2dbg << "Added " << nbrNew << " new zip codes to map." << endl;
   if (nbrNew == 0){
      mc2log << "Did not add any zips. This is not sane, exits." << endl;
      MC2_ASSERT(false);
   }
   set<MC2String>::const_iterator notAddedIt = notAddedZips.begin();
   while (notAddedIt != notAddedZips.end()){
      mc2dbg << "   Zip not added: " << (*notAddedIt) << endl;
      ++notAddedIt;
   }

   // Set center coordinates of the zips.
   adminAreaCentreTable_t adminAreaCenters = getAdminAreaCentres();
      // here you got a copy of the centre coordinates.
   mc2dbg << "Setting admin area centers from zips" << endl;
   mc2dbg << "Size of adminAreaCenters before: " 
          << adminAreaCenters.size() << endl;
   uint32 nbrCentersAdded = 0;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
         OldZipCodeItem* zip = 
            dynamic_cast<OldZipCodeItem*>(getItem(z, i));
         if (zip == NULL ){
            continue;
         }
         if (zip->getNbrNames() != 1){
            mc2dbg << "Zip code ID: " << zip->getID() << " got " 
                   << zip->getNbrNames() << " names." << endl;
            exit(1);
         }
         MC2String zipName = getFirstItemName(zip);
         MC2Coordinate centerCoord = 
            zipSideFile.getZipCenterCoord(zipName);
         if ( !centerCoord.isValid() ){
            mc2dbg << "Zip code ID: " << zip->getID() << ":" << zipName 
                   << " missing in zip side file coordinates." << endl;
            mc2dbg << "Coord: " << centerCoord << endl;
            exit(1);
         }
         adminAreaCenters.insert(make_pair(zip->getID(), centerCoord));
         nbrCentersAdded++;
      }
   }
   // Set the centres
   mc2dbg << "Size of adminAreaCenters after: " << adminAreaCenters.size() 
          << endl;
   setAminAreaCentres(adminAreaCenters);
   mc2dbg << "Tried to add " << nbrCentersAdded 
          << " admin area centers from zip codes" << endl;
         
   // Zips must have gfx data for setZipForSSIs to work.
   setZipGfxDataFromAdminCenters();

   // Updating, expanding the SSI connection.
   setZipForSSIs();


   // Remove all zipcodes that doesn't have any streetsegments.
   mc2dbg << "Removing zipcodes (outside section), wich are not used as"
          << " group" << endl;
   mc2dbg << "   Collecting SSI groups." << endl;
   // Collect all items being groups to SSIs.
   set<uint32>allSSIGroupIDs;
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldStreetSegmentItem* ssi = 
            dynamic_cast<OldStreetSegmentItem*>(getItem(z, i));
         if ( ssi == NULL ){
            // We are only interested in SSIs.
            continue;
         }
         for (uint32 g=0; g<ssi->getNbrGroups(); g++){
            allSSIGroupIDs.insert(ssi->getGroup(g));
         }
      }
   }
   mc2dbg << "   Detecting zips to remove." << endl;
   set<uint32>removedIDs;
   uint32 processedZips = 0;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; ++z) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); ++i) {
         OldZipCodeItem* zip = dynamic_cast<OldZipCodeItem*>
            (getItem(z, i));
         if ( ( zip != NULL ) && ( zip->getNbrItemsInGroup() == 0 ) ) {
            processedZips++;
            if ( (processedZips % 10000) == 0 ){
               mc2dbg << "      " << "Processed " << processedZips 
                      << " zips, total to remove: " 
                      << removedIDs.size() << endl;
            }
            MC2String zipName = getFirstItemName( zip->getID() );
            mc2dbg8 << "Zip to rm name: " << zipName << endl;
            if (!zipSideFile.zipInSectionOfMap(zipName)){
               if (allSSIGroupIDs.find(zip->getID()) == allSSIGroupIDs.end()){
                  mc2dbg8 << " Removing zipcode "
                          << getFirstItemName( zip->getID() ) 
                          << " [" << zip->getID() << "]"
                          << endl;
                  removedIDs.insert(zip->getID());
               }
            }
         }
      }
   }

   // Remove the zips first
   // then remove from admin centers and rebuild hash table.
   mc2dbg << "   Removing zips." << endl;
   set<uint32>::const_iterator zipIt=removedIDs.begin();
   processedZips=0;
   while (zipIt != removedIDs.end()){

      // Print status.
      processedZips++;
      if ( (processedZips % 1000) == 0 ){
         mc2dbg << "      " << "Processed " << processedZips 
                << " of  " << removedIDs.size() << endl;
      }
      removeItem(*zipIt,
                 false, // Not updating hash table
                 false, // Not updating admin centers
                 true   // unusedUkZips optimization
                 );
      ++zipIt;
   }
   mc2log << info << "   Removed " << removedIDs.size() 
          << " not group used or not in true section zips." << endl;
   mc2dbg << "   Removing from admin centers." << endl;
   removeAdminCenters(removedIDs);
   mc2log << info << "   Rebuilding hash table." << endl;
   buildHashTable();
   


   return result;
} // addZipsFromZipPOIFile



void
GMSMap::setZipGfxDataFromAdminCenters(){
   mc2log << info << "Called setZipGfxDataFromAdminCenters" << endl;
   
   adminAreaCentreTable_t adminCenters = getAdminAreaCentres();
   uint32 nbrUpdated = 0;

   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
         OldZipCodeItem* zip = 
            dynamic_cast<OldZipCodeItem*>(getItem(z, i));
         if (zip == NULL ){
            continue;
         }
         adminAreaCentreTable_t::const_iterator acIt = 
            adminCenters.find(zip->getID());
         if ( acIt == adminCenters.end() ){
            continue;
         }
         // Set the coordinate as GfxData.
         if(zip->getGfxData() != NULL){
            mc2log << error << "Found zip with GfxData, exits" << endl;
            exit(1);
         }
         zip->setGfxData(GMSGfxData::createNewGfxData(NULL, // theMap
                                                      true  // create poly
                                                      ));
         zip->getGfxData()->addCoordinate( acIt->second.lat,
                                           acIt->second.lon );
         nbrUpdated++;
      }
   }   
   
   mc2dbg << "Rebuilds hash table" << endl;
   buildHashTable();
   
   mc2log << info << "setZipGfxDataFromAdminCenters: Updated " 
          << nbrUpdated 
          << " zip codes." << endl;

} // setZipGfxDataFromAdminCenters



bool
GMSMap::addZipFromName(OldItem* item, MC2String zipCodeName ){
   bool result = true;

   // Fill the m_zipByName map if needed
   if (m_zipByName.size() == 0){
      // Loop all items
      for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
         for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
            OldZipCodeItem* zipItem = 
               dynamic_cast<OldZipCodeItem*>(getItem(z, i));
            if (zipItem == NULL ){
               continue;
            }
            // Add all names of all zip codes.
            for ( uint32 n=0; n<zipItem->getNbrNames(); n++){
               MC2String name = this->getName(zipItem->getStringIndex(n));
               if ( !m_zipByName.insert(make_pair(name, zipItem)).second){
                  mc2log << error << "1: Zip code with name: " << name 
                         << " already found in m_zipByName." << endl;
                  exit(1);
               }
            }
         }
      }
   } // fill zipByName map

   // Get the zip item to add as group.
   zipByName_t::const_iterator zipIt = 
      m_zipByName.find(zipCodeName);
   uint32 zipID = MAX_UINT32;
   if ( zipIt != m_zipByName.end() ){
      // Zip exists
      zipID = zipIt->second->getID();
   }
   else {
      // Create new zip item
      OldZipCodeItem* zipItem = new OldZipCodeItem( MAX_UINT32 );
      
      // Add the zip item to the map.
      uint32 zoomLevel = 10; // zip code zoom level.
      zipID = addItem(zipItem, zoomLevel);
      if ( zipID == MAX_UINT32 ){
         mc2log << error << "Could not add zip item to map."
                << endl;
         result = false;
         exit(1);
      }

      // Set name of new zip item.
      LangTypes::language_t nameLang = LangTypes::invalidLanguage;
      ItemTypes::name_t nameType = ItemTypes::roadNumber;
      addNameToItem( zipID, zipCodeName.c_str(), 
                     nameLang, nameType );

      // Add it to the m_zipByName member.
      if ( !m_zipByName.insert(make_pair(zipCodeName, zipItem)).second){
         mc2log << error << "2: Zip code with name: " << zipCodeName
                << " already found in m_zipByName." << endl;
         exit(1);
      }


      mc2dbg8 << "Added zip item from zip side file. ID:0x"
              << hex << zipID << dec << " name:" << zipCodeName << endl;
   }


   // Add the zip as a group to the given item
   if ( item != NULL ){
      // Not setting group if we got no item.
      if ( !addRegionToItem(item, zipID) ){
         result = false;
         mc2log << error << "GMSMap::addZipFromName failed adding "
                << "zip item 0x"
                << hex << zipID << dec << " as group" << endl;
         exit(1);
      }
      else {
         mc2dbg8 << "Side file zip item: 0x" << hex << zipID << " -> 0x" 
                 << item->getID() << dec << endl;
      }
   }
   

   return result;
} //addZipFromName




bool 
GMSMap::removeGroups(ItemTypes::itemType itemType)
{
   bool result = true;

   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z,i);
         if ( item != NULL ){

            for( int32 g = item->getNbrGroups()-1; g>=0; g--){
               uint32 groupID = item->getGroup(g);
               OldItem* group = itemLookup(groupID);
               if ( group->getItemType() == itemType ){
                  result = result && item->removeGroup(g);
               }
            } // for each group

         } 
      } // for each item
   } // for each zoom
   return result;
}

bool 
GMSMap::addIndexGroupToItem(OldItem* item, 
                            pair<MC2String, MC2String> indexPost,
                            map< pair<MC2String, MC2String>, uint32>&
                            groupIDsByIndex,
                            bool onlySearchable )
{
   bool result = true;
   
   // Find group to add to this item.
   map< pair<MC2String, MC2String>, uint32>::const_iterator
      groupIt = groupIDsByIndex.find( indexPost );
   if ( groupIt == groupIDsByIndex.end() ){
      // No group created for this index post, create it.
      
      // Create new group item.
      OldItem* groupItem = new OldBuiltUpAreaItem( MAX_UINT32 );
      
      // Add the group item to the map.
      uint32 zoomLevel = 0;
      uint32 groupItemID = 
         this->addItem(groupItem, zoomLevel);
      if ( groupItemID == MAX_UINT32 ){
         mc2log << warn << "Could not add new group item to map."
                << endl;
         result = false;
         exit(1);
      }
      
      // Set name of new group item.
      LangTypes::language_t nameLang = LangTypes::english;
      ItemTypes::name_t nameType = ItemTypes::officialName;
      MC2String name = indexPost.first;
      if ( indexPost.second != "" ){
         // Adding locality distinction.
         name += " ";
         name += indexPost.second;
         mc2dbg << "Added locality dist:\"" << indexPost.second << "\"" 
                << endl;
      }

      mc2dbg8 << "item->getID():" << item->getID() << endl;
      mc2dbg8 << "indexPost.first:" << indexPost.first << endl;
      mc2dbg8 << "indexPost.second:" << indexPost.second << endl;

      addNameToItem( groupItemID, name.c_str(), 
                     nameLang, nameType );
      mc2dbg << "Added index grp item: 0x" << hex << groupItemID << dec 
             << " " << MC2CITE(name) << "" << endl; 
      
      // Set group of new group item.
      for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
         for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
            OldMunicipalItem* munItem = 
               dynamic_cast<OldMunicipalItem*>(getItem(z, i));
            if ( munItem == NULL ){
               // We are only interested in municipals, which in USA
               // are couties actually.
               continue;
            }
            if ( oneNameSameCase( groupItem, munItem ) ){
               addRegionToItem(groupItem, munItem->getID() );
            }
         }
      }


         

      groupIt = 
         groupIDsByIndex.insert( make_pair(indexPost, 
                                            groupItemID) ).first;
   }
   
   // Bind item to the group.
   uint32 groupID = groupIt->second;
   OldGroupItem* group = dynamic_cast<OldGroupItem*>(itemLookup(groupID));
   if ( group != NULL ){
      // Set high bit to make this group not show as location in a hit.
      if ( !addRegionToItem(item,groupID, onlySearchable) ){
         result = false;
         mc2log << error << "GMSMap::addIndexGroupToItem failed adding " 
                << "group" << endl;
      }
      else {
         mc2log << "Index group: 0x" << hex << groupID << " -> 0x" 
                << item->getID() << endl;
      }
      // Not adding the item to the group item.
      //if ( !group->addItem( item->getID() ) ){
      //   result = false;
      //}
   }
   else {
      mc2log << error << "GMSMap::addIndexGroupToItem: Missing group id " 
             << groupID << endl;
      MC2_ASSERT(false);
   }

   return result;
} // addIndexGroupToItem

uint32 
GMSMap::setZipForSSIs(){ // rename
   uint32 nbrGroups = 0;
   mc2log << info << "Called setZipForSSIs" << endl;

   // Inclusion bounding box size distances.
   //
   // first:  square meter
   // second: distance factor
   vector<pair<uint64, float> > distFactors;
   distFactors.push_back(make_pair(50, 4));
   distFactors.push_back(make_pair(100, 3));
   distFactors.push_back(make_pair(150, 2.5));
   distFactors.push_back(make_pair(250, 2));
   distFactors.push_back(make_pair(500, 1.75));
   distFactors.push_back(make_pair(750, 1.50));
   distFactors.push_back(make_pair(1000, 1.25));
   distFactors.push_back(make_pair(2000, 1.20));
   distFactors.push_back(make_pair(4000, 1.15));
   distFactors.push_back(make_pair(7500, 1.07));
   distFactors.push_back(make_pair(10000, 1));
   // Max and min distances for the bounding box possible size.
   const uint32 minDistMeter = distFactors[0].first;
   const uint32 maxDistMeter = distFactors[distFactors.size()-1].first;
   mc2dbg << "   maxDistMeter: " << maxDistMeter << endl;
   mc2dbg << "   minDistMeter: " << minDistMeter << endl;
 


   mc2dbg << "Removing zip groups from all items" << endl;
   bool result = removeGroups(ItemTypes::zipCodeItem);
   if ( !result ){
      mc2log << error << "GMSMap::setZipForSSIs. Removing zip groups failed"
             << endl;
      exit(1);
   }


   // Loop all SSIs.
   uint32 nbrSsiGotZips = 0;
   uint32 nbrSsiProcessed = 0;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      mc2dbg << "Set zip location entering zoom: " << z << endl;
      for (uint32 i=0; i<getNbrItemsWithZoom(z); i++) {
         OldStreetSegmentItem* ssi = 
            dynamic_cast<OldStreetSegmentItem*>(getItem(z, i));
         if ( ssi == NULL ){
            // We are only interested in SSIs.
            continue;
         }
         mc2dbg8 << "DBG: " << "SSI ID: 0x" << hex << ssi->getID() << dec 
                << endl;

         // Create a bounding box to look for zips in.
         MC2BoundingBox ssiBox;
         getItemBoundingBox  ( ssiBox, ssi );
         MC2BoundingBox maxBox = ssiBox;
         maxBox.increaseMeters( maxDistMeter );

         
         // OldItem types to look for.
         set<ItemTypes::itemType> zipItemType;
         zipItemType.insert(ItemTypes::zipCodeItem);


         // Set the inclusion bounding box addition distance.
         MC2Coordinate ssiCoord = getOneGoodCoordinate(ssi);
         if (!ssiCoord.isValid()){
            mc2log << error << here 
                   << "getOneGoodCoordinate for item ID: " 
                   << ssi->getID() << " failed." << endl;
            exit(1);
         }
         MC2BoundingBox densBox(ssiCoord.lat, ssiCoord.lon,
                                ssiCoord.lat, ssiCoord.lon );
         uint32 includeDistMeter = MAX_UINT32;
         uint32 foundZipDistMeter = MAX_UINT32;
         uint32 prevDistMeter = 0;
         set<uint32> densZipIDs;
         for (uint32 i=0; 
              (i < distFactors.size()) && (includeDistMeter == MAX_UINT32);
              i++){
            uint32 distMeter = distFactors[i].first;
            densBox.increaseMeters(distMeter-prevDistMeter);
            mc2dbg8 << "DBG: " << "   distMeter-prevDistMeter: " 
                   << distMeter-prevDistMeter << endl;
            prevDistMeter += distMeter-prevDistMeter;
            
            getIDsWithinBBox(densZipIDs, densBox, zipItemType);
            if ( densZipIDs.size() > 0 ){
               foundZipDistMeter = distMeter;
            }
            if ( densZipIDs.size() > 1 ){
               includeDistMeter = distMeter;
               includeDistMeter = 
                  static_cast<uint32>
                  (floor(includeDistMeter * distFactors[i].second)+0.5);
            }


         }
         mc2dbg8 << "DBG: " << "foundZipDistMeter: " << foundZipDistMeter
                << endl
                << "     " << "includeDistMeter: " << includeDistMeter
                << endl
                << "     " << "nbr density zips: " << densZipIDs.size() 
                << endl;
                        


         // Handle max and min values.
         if ( includeDistMeter < minDistMeter ){
            includeDistMeter = minDistMeter;
         }
         else if ( includeDistMeter > maxDistMeter ){
            includeDistMeter = maxDistMeter;
         }
         mc2dbg8 << "DBG: " << "Include box expanding distance: " 
                << includeDistMeter << " meters." << endl;
         
         // Get the items to add as group to this zip.
         MC2BoundingBox includeBox = ssiBox;
         includeBox.increaseMeters( includeDistMeter );
         mc2dbg8 << "DBG: " << "Include box width: " 
                << includeBox.getWidth() * GfxConstants::MC2SCALE_TO_METER
                << " height: " 
                << includeBox.getHeight() * GfxConstants::MC2SCALE_TO_METER
                << endl;
         
         set<OldItem*> groupZips;
         getItemsWithinBBox(groupZips,
                            includeBox,
                            zipItemType  // allowed types
                            );
         mc2dbg8 << "DBG: " << "Found: " <<  groupZips.size() 
                << " zip items to add as group to SSI ID: 0x" 
                << hex << ssi->getID() << dec
                << endl;
         if ( groupZips.size() == 0 ){
            //DBG:
            //includeBox.dump();
         }
         else {
            nbrSsiGotZips++;
         }
         nbrSsiProcessed++;
         
         const uint32 maxNbrZipGroups = 10;
         if ( groupZips.size() > maxNbrZipGroups ){
            mc2dbg8 << "DBG: Too many zips, only keep the " 
                   << maxNbrZipGroups
                   << " closest ones." << endl;
            GfxData* ssiGfx = ssi->getGfxData();
            multimap<uint64, OldItem*> zipByDist;
            set<OldItem*>::const_iterator groupIt = groupZips.begin();
            while (groupIt != groupZips.end()){
               MC2Coordinate zipCoord = getOneGoodCoordinate(*groupIt);
               if (!zipCoord.isValid()){
                  mc2log << error << here 
                         << "getOneGoodCoordinate for item ID: " 
                         << (*groupIt)->getID() << " failed." << endl;
                  exit(1);
               }
               
               uint64 sqDist = ssiGfx->squareDistTo(zipCoord.lat,
                                                    zipCoord.lon);
               zipByDist.insert(make_pair(sqDist, (*groupIt)));
               ++groupIt;
            }
            // Take the first maxNbrZipGroups
            groupZips.clear();
            multimap<uint64, OldItem*>::const_iterator zipByDistIt = 
               zipByDist.begin();
            uint32 nbrAdded = 0;
            while ((nbrAdded < maxNbrZipGroups) && 
                   (zipByDistIt != zipByDist.end()) ){
               groupZips.insert(zipByDistIt->second);
               ++nbrAdded;
               ++zipByDistIt;
            }
         }


         set<OldItem*>::const_iterator groupIt = groupZips.begin();
         while (groupIt != groupZips.end()){
                  
            // Add the zip as a group
            if ( !addRegionToItem(ssi, (*groupIt)->getID()) ){
               mc2log << error << "GMSMap::setZipForSSIs failed adding "
                      << "zip item 0x"
                      << hex << (*groupIt)->getID() 
                      << " as group to 0x" 
                      << ssi->getID() << dec << endl;
               exit(1);
            }
            else {
               mc2dbg8 << "DBG: Zip group item: 0x" << hex 
                      << (*groupIt)->getID()
                      << " -> 0x" 
                      << ssi->getID() << dec << endl;
            }
            ++groupIt;
         }

      } // for all items.
   } // for zooms

   mc2log << info << "Processed SSIs: " << nbrSsiProcessed
          << " added zips to " << nbrSsiGotZips << " SSIs." << endl;
   
   return nbrGroups;
} // setZipForSSIs



uint32 
GMSMap::changeGroupOfItems(uint32 origGroupItemID, 
                           uint32 newGroupItemID)
{
   uint32 result = 0;
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldItem* item = getItem(z, i);
         if ( item == NULL) {
            continue;
         }
         for (uint32 i=0; i<item->getNbrGroups(); ++i) {
            if (item->getGroup(i) == origGroupItemID){
               if (!item->removeGroup(i)){
                  mc2log << error << "Could not remove group idx:" 
                         << i << " from item " << item->getID();
                  exit(1);
               }
               if (! addRegionToItem(item, newGroupItemID)) {
                  mc2dbg8 << "Could not add group item ID:" 
                          << newGroupItemID << " to item " 
                          << item->getID() << endl;
                  // This is perhaps because the group is already present
                  // for this item
               }
               result++;
            }
         }
      }
   }
   return result;
}


uint32
GMSMap::removeShorterZipCodes(uint32 minLength){
   if ( minLength == MAX_UINT32){
      // Most common minLength value, no need to check for it.
      return 0;
   }
   uint32 result = 0;
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldZipCodeItem* zipItem = 
            dynamic_cast<OldZipCodeItem*>(getItem(z, i));
         if ( zipItem == NULL) {
            continue;
         }
         if (zipItem->getNbrNames() != 1){
            mc2dbg << "Zip code with " << zipItem->getNbrNames() 
                   << " names, should have been 1, exits!." << endl;
            MC2_ASSERT(false);
         }
         MC2String name = getFirstItemName(zipItem);
         if ( name.size() < minLength ){
            removeItem(zipItem->getID());
            result++;
         }
      }
   }
   return result;
}


uint32
GMSMap::removeNonNumericZipCodeNames(){
   uint32 result = 0;
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldZipCodeItem* zipItem = 
            dynamic_cast<OldZipCodeItem*>(getItem(z, i));
         if ( zipItem == NULL) {
            continue;
         }
         bool removedName = false;
         for (int32 n=zipItem->getNbrNames()-1; n >= 0; n-- ){
            if ( zipItem->getNameType(n) != ItemTypes::roadNumber ){
               zipItem->removeNameWithOffset(n);
               removedName = true;
            }
         }
         if ( zipItem->getNbrNames() == 0 ){
            mc2log << error << "Zip code ID: " << zipItem->getID()
                   << " no longer have any names." << endl;
            MC2_ASSERT(false);
         }
         if ( removedName ){
            result++;
         }
         
      }
   }
   return result;
}


uint32
GMSMap::updateMunicipalNames()
{
   uint32 nbrUpdated = 0;

   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldMunicipalItem* munItem = 
            dynamic_cast<OldMunicipalItem*>(getItem(z, i));
         if ( munItem == NULL) {
            continue;
         }

         bool addCountryName = false;
         MC2String reason = "reason";
         if ( munItem->getNbrNames() == 0 ) {
            addCountryName = true;
            reason = "no name";
            mc2dbg8 << "Municipal " << munItem->getID() << " has no name"
                    << endl;
         } else {
            for (int32 n=munItem->getNbrNames()-1; n >= 0; n-- ){
               MC2String name = getName(munItem->getStringIndex(n));
               if ( name == "$$$$$" ) {
                  munItem->removeNameWithOffset(n);
                  reason = "$-name";
                  mc2dbg8 << "Municipal " << munItem->getID()
                          << " has $$ name" << endl;
               }
            }
            if (munItem->getNbrNames() == 0) {
               addCountryName = true;
            }
         }
         
         if (addCountryName) {
            nbrUpdated++;
            // add name for all native langs + english
            bool englishIsNative = false;
            for (uint32 l = 0; l < getNbrNativeLanguages(); l++ ) {
               LangTypes::language_t lang = getNativeLanguage(l);
               if ( lang == LangTypes::english ) {
                  englishIsNative = true;
               }
               const char* name = 
                  StringTable::getStringToDisplayForCountry(
                  getCountryCode(),
                  ItemTypes::getLanguageTypeAsLanguageCode(lang) );
               addNameToItem (munItem,
                              name, lang, ItemTypes::officialName); 
               mc2dbg1 << "Municipal " << munItem->getID() << " added name "
                       << name << " because of " << reason << endl;
            }
            if ( ! englishIsNative ) {
               const char* name = 
                  StringTable::getStringToDisplayForCountry(
                  getCountryCode(), StringTable::ENGLISH );
               addNameToItem (munItem,
                              name,
                              LangTypes::english,
                              ItemTypes::officialName); 
               mc2dbg1 << "Municipal " << munItem->getID() << " added name "
                       << name << " (english) because of: " << reason << endl;
            }
         }
      }
   }

   return nbrUpdated;
} // updateMunicipalNames

uint32
GMSMap::setNamesOnNoNameBuas()
{
   uint32 nbrUpdatedFromMun = 0;
   uint32 nbrUpdatedFromCountry = 0;
   
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldBuiltUpAreaItem* buaItem = 
            dynamic_cast<OldBuiltUpAreaItem*>(getItem(z, i));
         if ( buaItem == NULL) {
            continue;
         }

         // For noname buas, add the names from the municipal it is
         // located in.
         bool addCountryName = false;
         if ( buaItem->getNbrNames() == 0 ) {
            OldItem* mun = getRegion(buaItem, ItemTypes::municipalItem);
            if ( mun == NULL ) {
               // No municipal for the bua - add the country name
               // if that is a good idea
               if ( NationalProperties::useCountryNameForNoNameBuas(
                        getCountryCode(), getMapOrigin()) ) {
                  addCountryName = true;
               } else {
                  mc2log << error << here << "No municipal to pick name from "
                         << "for bua " << buaItem->getID() << endl;
                  MC2_ASSERT(false);
               }
            } else {
               mc2dbg << "Noname bua " << buaItem->getID() 
                      << " located in mun " << mun->getID() << endl;
               
               nbrUpdatedFromMun++;
               LangTypes::language_t lang;
               ItemTypes::name_t type;
               uint32 strIdx;
               for (uint32 n = 0; n < mun->getNbrNames(); n++) {
                  if ( mun->getNameAndType(n, lang, type, strIdx) ) {
                     const char* name = getName(strIdx);
                     addNameToItem (buaItem, name, lang, type);
                     mc2dbg1 << "Bua " << buaItem->getID() << " added name "
                             << name << endl;
                  }
               }
            }
         }
         if (addCountryName) {
            nbrUpdatedFromCountry++;
            // add name for all native langs + english
            bool englishIsNative = false;
            for (uint32 l = 0; l < getNbrNativeLanguages(); l++ ) {
               LangTypes::language_t lang = getNativeLanguage(l);
               if ( lang == LangTypes::english ) {
                  englishIsNative = true;
               }
               const char* name = 
                  StringTable::getStringToDisplayForCountry(
                  getCountryCode(),
                  ItemTypes::getLanguageTypeAsLanguageCode(lang) );
               addNameToItem (buaItem,
                              name, lang, ItemTypes::officialName); 
               mc2dbg1 << "Bua " << buaItem->getID() 
                       << " added country name " << name << endl;
            }
            if ( ! englishIsNative ) {
               const char* name = 
                  StringTable::getStringToDisplayForCountry(
                  getCountryCode(), StringTable::ENGLISH );
               addNameToItem (buaItem,
                              name,
                              LangTypes::english,
                              ItemTypes::officialName); 
               mc2dbg1 << "Bua " << buaItem->getID() 
                       << " added country name " << name << " (english)"
                       << endl;
            }
         }
      }
   }
   mc2dbg1 << "Added names to buas, from municipal for " 
           << nbrUpdatedFromMun << ", from country for "
           << nbrUpdatedFromCountry << endl;
   return (nbrUpdatedFromMun + nbrUpdatedFromCountry);
} // setNamesOnNoNameBuas

uint32
GMSMap::removeNoNameBuasInNoNameMunicipals()
{
   mc2dbg1 << "GMSMap::removeNoNameBuasInNoNameMunicipals" << endl;
   uint32 nbrRemoved = 0;
   
   set<uint32> buasToRemove;
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldBuiltUpAreaItem* buaItem = 
            dynamic_cast<OldBuiltUpAreaItem*>(getItem(z, i));
         if ( buaItem == NULL) {
            continue;
         }

         if ( buaItem->getNbrNames() > 0 ) {
            continue;
         }

         // FIXME: Possible special action if using index area
         // - e.g. if not in search index: ok to keep the bua

         // For noname buas, identify the ones that are located in a no-name 
         // municipal, or no municipal at all.
         bool removeBua = false;
         MC2String reason = "";
         OldItem* mun = getRegion(buaItem, ItemTypes::municipalItem);
         if ( mun == NULL ) {
            removeBua = true;
            reason = "no municipal";
         }
         else {
            if ( mun->getNbrNames() == 0 ) {
               removeBua = true;
               reason = "no-name municipal";
            }
         }
         
         if (removeBua) {
            mc2dbg1 << "Will remove bua " 
                    << getMapID() << ":" << buaItem->getID()
                    << " because: " << reason << endl;
            buasToRemove.insert(buaItem->getID());
         }
      }
   }

   if ( buasToRemove.size() > 0 ) {
      if ( removeItems(buasToRemove) ) {
         nbrRemoved = buasToRemove.size(); 
      }
   }
   return nbrRemoved;
} // removeNoNameBuasInNoNameMunicipals

uint32
GMSMap::removeNoNameCityParts()
{
   mc2dbg1 << "GMSMap::removeNoNameCityParts" << endl;
   uint32 nbrRemoved = 0;
   
   set<uint32> cpToRemove;
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldCityPartItem* cpItem = 
            dynamic_cast<OldCityPartItem*>(getItem(z, i));
         if ( cpItem == NULL) {
            continue;
         }
         if ( cpItem->getNbrNames() > 0 ) {
            continue;
         }
         mc2dbg1 << "Will remove no-name city part " 
                 << getMapID() << ":" << cpItem->getID() << endl;
         cpToRemove.insert(cpItem->getID());
      }
   }

   if ( cpToRemove.size() > 0 ) {
      if ( removeItems(cpToRemove) ) {
         nbrRemoved = cpToRemove.size(); 
      }
   }
   return nbrRemoved;
} // removeNoNameCityParts

bool
GMSMap::isRightExit(uint32 fromNodeID, uint32 toNodeID){
   OldNode* fromNode = nodeLookup(fromNodeID);
   OldNode* toNode = nodeLookup(toNodeID);

   OldRouteableItem* fromItem = dynamic_cast<OldRouteableItem*>
      (itemLookup(fromNodeID));
   OldRouteableItem* toItem = dynamic_cast<OldRouteableItem*>
      (itemLookup(toNodeID));   

   GfxData* fromGfx = fromItem->getGfxData();
   GfxData* toGfx = toItem->getGfxData();
   
   // Returns angle from north.
   float64 fromAngle = 
      Crossing::getAdjustedAngle(fromGfx, !fromNode->isNode0());
   // We flip nodes of from item since it's the other one that is located at
   // the crossing.
   float64 toAngle = 
      Crossing::getAdjustedAngle(toGfx, toNode->isNode0());
   
   // Algorithm here:
   //    angle from north of from item - angel from north of to item is
   //    ( > 0 and < 180 ) or < -180 when the exit is on the right side.
   float64 angleDiff = fromAngle - toAngle;

   Vector toNodeIDVec;
   getConnectToNodeIDs(fromNodeID, &toNodeIDVec);
   for ( uint32 i=0; i<toNodeIDVec.getSize(); i++){
      uint32 tmpToNodeID = toNodeIDVec[i];
      if ( tmpToNodeID == toNodeID ){
         // We don't calculate the same to node again.
         continue;
      }
      OldNode* tmpToNode = nodeLookup(tmpToNodeID);
      OldRouteableItem* tmpToItem = dynamic_cast<OldRouteableItem*>
         (itemLookup(tmpToNodeID));   
      GfxData* tmpToGfx = tmpToItem->getGfxData();
      float64 tmpToAngle = 
         Crossing::getAdjustedAngle(tmpToGfx, tmpToNode->isNode0());
      float64 tmpAngleDiff = fromAngle - tmpToAngle;
      if ( ( tmpAngleDiff >= 0 && angleDiff >= 0 ) ||
           ( tmpAngleDiff < 0 && angleDiff < 0) ){ 
         if ( tmpAngleDiff > angleDiff ){
            // We found a road to the left, this one is to the right.
            //return true;
         }
         else {
            mc2dbg << "Found road to the right A" << endl;
            return false;
         }
      }
      else if ( tmpAngleDiff < 0 && angleDiff > 0 ){
         //return true;
      }
      else {
         mc2dbg << "Found road to the right B" << endl;
         return false;
      }
   }
   return true;

}


GMSLane::laneDirection_t 
GMSMap::getLaneDirCatFromTurnDir(ItemTypes::turndirection_t turnDirection,
                                 uint32 fromNodeID, uint32 toNodeID){
   GMSLane::laneDirection_t result = GMSLane::noDirectionIndicated;
   switch (turnDirection){
   case ItemTypes::LEFT: {
      result = GMSLane::left;
   } break;
   case ItemTypes::AHEAD: {
      result = GMSLane::ahead;
   } break;
   case ItemTypes::RIGHT: {
      result = GMSLane::right;
   } break;
   case ItemTypes::FOLLOWROAD: {
      result = GMSLane::ahead;
   } break;
   case ItemTypes::ON_RAMP: {
      result = GMSLane::ahead;
   } break;
   case ItemTypes::OFF_RAMP: {
      if ( isRightExit(fromNodeID, toNodeID) ){
         result = GMSLane::slightRight;
         if ( ! driveOnRightSide() ) {
            mc2dbg << "Set right exit of " << fromNodeID << "->" << toNodeID 
                   << endl;
         }
      }
      else {
         result = GMSLane::slightLeft;
         if ( driveOnRightSide() ) {
            mc2dbg << "Set left exit of " << fromNodeID << "->" << toNodeID 
                   << endl;
         }
      }
   } break;
   case ItemTypes::KEEP_RIGHT: {
      result = GMSLane::slightRight;
   } break;
   case ItemTypes::KEEP_LEFT: {
      result = GMSLane::slightLeft;
   } break;
   case ItemTypes::OFF_RAMP_LEFT: {
      result = GMSLane::slightLeft;
   } break;
   case ItemTypes::OFF_RAMP_RIGHT: {
      result = GMSLane::slightRight;
   } break;
   
   
   
   case ItemTypes::AHEAD_ROUNDABOUT: 
   case ItemTypes::RIGHT_ROUNDABOUT: 
   case ItemTypes::LEFT_ROUNDABOUT: {
      // Can we do wnyting?
   } break;
   
   
   case ItemTypes::ENTER_ROUNDABOUT: 
   case ItemTypes::MULTI_CONNECTION_TURN: 
   case ItemTypes::EXIT_ROUNDABOUT: 
   case ItemTypes::ENTER_BUS: 
   case ItemTypes::EXIT_BUS: 
   case ItemTypes::CHANGE_BUS: 
   case ItemTypes::ENTER_FERRY: 
   case ItemTypes::EXIT_FERRY: 
   case ItemTypes::CHANGE_FERRY:
   case ItemTypes::UNDEFINED:
   case ItemTypes::UTURN: {
      // Nothing to do!
   } break;
   
   } // switch 
   
   return result;
}


uint32
GMSMap::setUndefiendLaneConnDirCat(){
   uint32 nbrSetDirCat = 0;
   uint32 nbrLanesWithConn = 0;

   // Collect the node IDs that already have lane direction category set from 
   // the source data.
   //
   // First:  Node ID of items with at least one lane direction category 
   //         defined.
   // Second: Lane index of lane with direction defined.
   multimap<uint32, uint32> definedLaneDirs;
   for (ItemMap< vector<GMSLane> >::iterator nodeLanesIt = 
           m_nodeLaneVectors.begin(); 
        nodeLanesIt != m_nodeLaneVectors.end(); ++nodeLanesIt){
      uint32 fromNodeID = nodeLanesIt->first;
      vector<GMSLane>& lanes = nodeLanesIt->second;
      for (uint32 l=0; l<lanes.size(); l++){
         if ( lanes[l].directionUseable() ){
            mc2dbg8 << "definedLaneDirs: " << fromNodeID << "[" << l << "]" 
                    << endl;
            definedLaneDirs.insert(make_pair(fromNodeID, l));
         }
      }
   }

   // Go through the table of lane connectivity and set lane direction category
   // from corresponding turn descriptions.
   mc2dbg << "setUndefiendLaneConnDirCat A (lane direction cat from lane connectivity turn directions)" << endl;
   const ConnectingLanesTable& lanesTable = getConnectingLanes();
   ConnectingLanesTable::connLanesByFromAndToNode_t clByToAndFromNode =
      lanesTable.getFirstConnectingLanes();
   ConnectingLanesTable::connLanesByFromAndToNode_t noMoreData =
      make_pair( make_pair(MAX_UINT32,MAX_UINT32), MAX_UINT32 );
   while ( clByToAndFromNode != noMoreData ){
      uint32 fromNodeID = clByToAndFromNode.first.first;
      uint32 lanesBitfield = clByToAndFromNode.second;

      OldNode* fromNode = nodeLookup(fromNodeID);
      if ( fromNode == NULL ) {
         mc2log << error << "From node is NULL." << endl;
         MC2_ASSERT(false);
      }
      vector<GMSLane> lanes = fromNode->getLanes(*this);
      for (uint32 l=0; l<lanes.size(); l++){
         GMSLane& lane = lanes[l];
         if (ConnectingLanesTable::laneConnects(l, lanesBitfield)){
            // We can use the turn description of this connection to set this
            // lane direction category.
            nbrLanesWithConn++;

            // Check if this lane direction category was defined in the data.
            bool definedLaneCat = false;
            multimap<uint32, uint32>::const_iterator defIt = 
               definedLaneDirs.lower_bound(fromNodeID);
            while ( defIt != definedLaneDirs.upper_bound(fromNodeID) ){
               if ( defIt->second == l ){
                  // The direction category of this items was already defined
                  definedLaneCat = true;
               }
               ++defIt;
            }
            if ( definedLaneCat ){
               // Already set, nothing to do.
               mc2dbg8 << "Not setting lane cat A of: " << fromNodeID 
                       << "[" << l << "]" << endl;
               continue;
            }

            //uint16 laneDirBits = lane.getLaneDirBits();
            uint32 toNodeID = clByToAndFromNode.first.second;
            OldNode* toNode = nodeLookup(toNodeID);
            OldConnection* conn = 
               toNode->getEntryConnectionFrom(fromNodeID);
            ItemTypes::turndirection_t turnDirection = 
               conn->getTurnDirection();
            GMSLane::laneDirection_t newLaneDir = 
               getLaneDirCatFromTurnDir(turnDirection, fromNodeID, toNodeID);
            if ( newLaneDir == GMSLane::noDirectionIndicated ){
               mc2dbg8 << "GMSLane::noDirectionIndicated of " 
                       <<  fromNode->getNodeID() 
                       << "[" << l << "]: " << turnDirection << endl;
            }
            else {
               mc2dbg << "Setting A " << fromNode->getNodeID() 
                      << "[" << l << "]"
                      << " dir cat: " 
                      << newLaneDir << endl;
               if ( ! lane.directionUseable() ){
                  lane.clearDirCatValue();
               }
               lane.addDirCatValue(newLaneDir);
               fromNode->setLane(lane, l, *this);
               nbrSetDirCat++;
            }
         }
      }
      clByToAndFromNode = lanesTable.getNextConnectingLanes();
   }

   // Go through all nodes with lane info and set lane direction categories
   // from the turn description of all nodes that only connects to one other
   // node.
   mc2dbg << "setUndefiendLaneConnDirCat B (lane direction category from single connection turn directions)" << endl;
   uint32 nbrSingleConnLanes = 0;
   uint32 nbrSetDirCatB = 0;
   for (ItemMap< vector<GMSLane> >::iterator nodeLanesIt = 
           m_nodeLaneVectors.begin(); 
        nodeLanesIt != m_nodeLaneVectors.end(); ++ nodeLanesIt){
      uint32 fromNodeID = nodeLanesIt->first;
      Vector toNodeIDVec;
      if ( !getConnectToNodeIDs(fromNodeID, &toNodeIDVec) ){
         MC2_ASSERT(false);
      }
      if ( toNodeIDVec.getSize() == 1 ){
         // There is only one connection from this node. Otherwise we could not
         // do anything here.
         OldConnection* conn;
         uint32 toNodeID = toNodeIDVec[0];
         getConnection((fromNodeID & 0x7fffffff), // Mask to create item ID.
                       (toNodeID & 0x7fffffff),   // Mask to create item ID.
                       conn);
         if ( conn == NULL ){
            mc2dbg << "Could not find connection from " << fromNodeID << " to "
                   << toNodeID << endl;
               MC2_ASSERT(false);
         }
         ItemTypes::turndirection_t turnDirection = conn->getTurnDirection();
         GMSLane::laneDirection_t newLaneDir = 
            getLaneDirCatFromTurnDir(turnDirection, fromNodeID, toNodeID);
         vector<GMSLane>& lanes = nodeLanesIt->second;
         for ( uint32 l=0; l<lanes.size(); l++){
            
            nbrSingleConnLanes++;

            // Check if this lane direction category was defined in the data.
            bool definedLaneCat = false;
            multimap<uint32, uint32>::const_iterator defIt = 
               definedLaneDirs.lower_bound(fromNodeID); 
            while ( defIt != definedLaneDirs.upper_bound(fromNodeID) ){
               if ( defIt->second == l ){
                  // The direction category of this items was already defined
                  definedLaneCat = true;
               }
               ++defIt;
            }
            if ( definedLaneCat ){
               // Already set, nothing to do.
               mc2dbg << "Not setting lane cat B of: " << fromNodeID 
                      << "[" << l << "]" << endl;
               continue;
            }

            if ( newLaneDir != GMSLane::noDirectionIndicated ){
               OldNode* fromNode = nodeLookup(fromNodeID);
               mc2dbg << "Setting B " << fromNode->getNodeID() 
                      << "[" << l << "]"
                      << " dir cat: " 
                      << newLaneDir << endl;
               if ( ! lanes[l].directionUseable() ){
                  lanes[l].clearDirCatValue();
               }
               lanes[l].addDirCatValue(newLaneDir);
               nbrSetDirCatB++;
            }
         }
      }
   }

   mc2dbg << "GMSMap::setUndefiendLaneConnDirCat: Updated A: " << nbrSetDirCat
          << " of " << nbrLanesWithConn << " lanes." << endl;
   mc2dbg << "GMSMap::setUndefiendLaneConnDirCat: Updated B: " << nbrSetDirCatB
          << " of " << nbrSingleConnLanes << " lanes." << endl;

   return nbrSetDirCat;
} // setUndefiendLaneConnDirCat


bool 
GMSMap::validateIndexAreaOrderTable() const
{
   bool result = true;
   for (ItemMap< uint32 >::const_iterator indexAreaIt = 
           m_indexAreasOrder.begin();
        indexAreaIt != m_indexAreasOrder.end(); ++indexAreaIt){
      uint32 orderValue = indexAreaIt->second;
      if ( orderValue > MAX_UINT32-2 ){
         uint32 itemID = indexAreaIt->first;
         mc2log << error << "Item ID:" << itemID 
                << " has invalid index area order value:" << orderValue 
                << endl;
         result = false;
      }
   }
   return result;
}

uint32 
GMSMap::setIdxAreaGeometryConvexHull()
{
   if ( ! NationalProperties::useIndexAreas(
            getCountryCode(), getMapOrigin()) ) {
      mc2dbg1 << "Not using index areas for country - exit" << endl;
      return 0;
   }
   
   uint32 nbrConvHullGeometries = 0;
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldItem* item = (getItem(z, i));
         if ( item == NULL) {
            continue;
         }
         if ( ! isIndexArea(item->getID()) ){
            // Not an index area
            continue;
         }

         GfxDataFull* itemCoords = GMSGfxData::createNewGfxData(NULL, true);
         GMSGfxData* itemGfx = item->getGfxData();
         if ( itemGfx != NULL ){
            // The IA has geometry, simplify it to convex hull
            for ( uint32 p=0; p<itemGfx->getNbrPolygons(); p++ ){
               GfxData::const_iterator cIt = itemGfx->polyBegin(p);
               while ( cIt != itemGfx->polyEnd(p) ){
                  itemCoords->addCoordinate(cIt->lat, cIt->lon);
                  ++cIt;
               }
            }         
         } else {
            // Get coordinates from all items that belong to this IA
            // - recursive get or not?
            vector<uint32> items = getItemsInGroup(item->getID());
            for ( vector<uint32>::const_iterator itemsIT = items.begin();
                  itemsIT != items.end(); itemsIT++ ) {
               uint32 itemID = *itemsIT;
               OldItem* tmpItem = itemLookup(itemID);
               GMSGfxData* gfx = tmpItem->getGfxData();
               if (tmpItem->getItemType() == ItemTypes::streetSegmentItem) {
                  itemCoords->addCoordinate(gfx->getLat(0,0), gfx->getLon(0,0));
                  itemCoords->addCoordinate(gfx->getLat(0,1), gfx->getLon(0,1));
                  itemCoords->addCoordinate(
                           gfx->getLastLat(0), gfx->getLastLon(0));
               }
               else if ( tmpItem->getItemType() == 
                              ItemTypes::pointOfInterestItem ) {
                  itemCoords->addCoordinate(gfx->getLat(0,0), gfx->getLon(0,0));
               }
               else {
                  for ( uint32 p=0; p<gfx->getNbrPolygons(); p++ ){
                     GfxData::const_iterator cIt = gfx->polyBegin(p);
                     while ( cIt != gfx->polyEnd(p) ){
                        itemCoords->addCoordinate(cIt->lat, cIt->lon);
                        ++cIt;
                     }
                  }         
               }
            }
            
         }
         
         // Calculate the convex hull.
         Stack* stack = new Stack;
         if (itemCoords->getConvexHull(stack, 0)) {
            GMSGfxData* newItemGfx = GMSGfxData::createNewGfxData(NULL, true);
            uint32 size = stack->getStackSize();
            for (uint32 i = 0; i < size; i++) {
               uint32 idx = stack->pop();
               newItemGfx->addCoordinate(itemCoords->getLat(0,idx), 
                                         itemCoords->getLon(0,idx));
            }
            newItemGfx->setClosed(0, true);
            newItemGfx->updateLength();
            item->setGfxData(newItemGfx);
            nbrConvHullGeometries++;
         } else {
            // Something went wrong.
            mc2log << warn << "GMSMap::setIdxAreaGeometryConvexHull "
                   << "Could not create convex hull for IA "
                   << getMapID() << ":" << item->getID() << endl;
            return false;
         }
         
         delete stack;
         delete itemCoords;
      }
   }
   if ( nbrConvHullGeometries > 0 ) {
      buildHashTable();
   }
   return nbrConvHullGeometries;
} // setIdxAreaGeometryConvexHull

class IndexHierarchy {

public:
   IndexHierarchy (const OldItem* indexItem, const OldGenericMap* theMap) :
      m_baseItem(indexItem), 
      m_map(theMap)
   {
      // Check validity.
      MC2_ASSERT(m_baseItem != NULL);
      MC2_ASSERT(m_map != NULL);
      MC2_ASSERT((m_map->itemLookup(m_baseItem->getID())) == m_baseItem );
   };

   const OldItem* getBaseItem() const
   {
      return m_baseItem;
   };

   const OldGenericMap* getMap() const
   {
      return m_map;
   };

private:
   // Hide default constructor.
   IndexHierarchy();

   const OldItem* m_baseItem;
   const OldGenericMap* m_map;

}; // class IndexHierarchy

// Forward declarations
ostream& operator << (ostream& stream, const set<IndexHierarchy>& indexHchs );
bool operator> ( const IndexHierarchy& lhih, const IndexHierarchy& rhih );
//bool operator< ( const IndexHierarchy& lhih, const IndexHierarchy& rhih );


void fillIndexAreaGroups(set<IndexHierarchy>& indexHchys, 
                         const OldItem* item,
                         const OldGenericMap* theMap){
   for ( uint32 g = 0; g < item->getNbrGroups(); g++ ){

      

      // Debug prints.
      uint32 groupID =  item->getGroup(g);
      mc2dbg8 << "fillIndexAreaGroups(" << item->getID() << "):" 
             << groupID << endl;
      if ( theMap->itemLookup(groupID) == NULL ){
         mc2dbg8 << "Group:" << groupID << " is NULL" << endl;
      }
      if ( theMap->getIndexAreaOrder(groupID) == MAX_UINT32 ){
         mc2dbg8 << "Group:" << groupID << " not index area." << endl;
      }

      // Actual processing.
      const OldItem* group = theMap->itemLookup(item->getGroup(g));
      if ( group != NULL &&
           theMap->getIndexAreaOrder(group->getID()) != MAX_UINT32 ){
         // This is an index area.
         IndexHierarchy ih(group, theMap);
         pair<set<IndexHierarchy>::const_iterator, bool> result = 
            indexHchys.insert(ih);
         if ( !result.second ){
            set<IndexHierarchy> dummy;
            dummy.clear();
            dummy.insert(*result.first);
            mc2dbg8 << "fillIndexAreaGroups Existing: "<< endl << dummy 
                    << endl;
            dummy.clear();
            dummy.insert(ih);
            mc2dbg8 << "fillIndexAreaGroups To insert:" << endl << dummy
                    << endl;
         }
         
      }
   }
}

/// Gets the index areas that are group of this item.
void getIndexAreas(const OldItem* baseItem, const OldGenericMap* theMap, 
                   vector<uint32>& indexAreas)
{
   for (uint32 g=0; g<baseItem->getNbrGroups(); g++){
      const OldItem* group = theMap->itemLookup(baseItem->getGroup(g));
      if ( group != NULL &&
           theMap->getIndexAreaOrder(group->getID()) != MAX_UINT32 ){
         indexAreas.push_back(group->getID());
      }
   }
}

/// Get all index areas in the hierarchy of this item.
void getAllIndexAreas(const OldItem* baseItem, const OldGenericMap* theMap, 
              set<uint32>& indexAreas)
{
   for (uint32 g=0; g<baseItem->getNbrGroups(); g++){
      const OldItem* group = theMap->itemLookup(baseItem->getGroup(g));
      if ( group != NULL &&
           theMap->getIndexAreaOrder(group->getID()) != MAX_UINT32 ){
         indexAreas.insert(group->getID());
         getAllIndexAreas(group, theMap, indexAreas);
      }
   }
}
   

ostream& operator << (ostream& stream, const IndexHierarchy& indexHch ){
   uint32 itemID = indexHch.getBaseItem()->getID();
   return stream << "IA: " << indexHch.getMap()->getIndexAreaOrder(itemID) 
                 << " " << itemID;
}

uint32 indentWidth = 0;
uint32 indentDiff = 1;
ostream& operator << (ostream& stream, 
                      const set<IndexHierarchy>& indexHchs ){
   indentWidth += indentDiff;
   stream.width(indentWidth);
   stream.fill(' ');
   stream << "";
   for (set<IndexHierarchy>::const_iterator ihIt = indexHchs.begin();
        ihIt != indexHchs.end(); ++ ihIt ){
      stream << *ihIt << endl;
      set<IndexHierarchy> subIndexHchys;
      fillIndexAreaGroups( subIndexHchys, ihIt->getBaseItem(), ihIt->getMap());
      stream << subIndexHchys;
   }
   indentWidth -= indentDiff;
   stream.width(indentWidth);
   return stream;

} // operator << 


bool operator< ( const IndexHierarchy& lhih, const IndexHierarchy& rhih ) {
   const OldItem* lItem =  lhih.getBaseItem();
   const OldItem* rItem =  rhih.getBaseItem();
   const OldGenericMap* lMap = lhih.getMap();
   const OldGenericMap* rMap = rhih.getMap();

   mc2dbg8 << "Compare " << lItem->getID() << ":" << rItem->getID() << endl;

   /////////////////////////////////////////////////////////////////////////
   // Basic checking

   if ( lItem == rItem ){
      // Same item, not less than.
      return false;
   }

   /*
   else if (lItem->getID() < rItem->getID()){
      return true;
   }
   else if (lItem->getID() > rItem->getID()){
      return false;
   }
   */


   /////////////////////////////////////////////////////////////////////////
   // Check if different number of index area groups.

   // Left
   set<uint32> lIdxAreaIDs;
   for (uint32 g=0; g<lItem->getNbrGroups(); g++){
      const OldItem* group = lMap->itemLookup(lItem->getGroup(g));
      if ( group != NULL &&
           lMap->getIndexAreaOrder(group->getID()) != MAX_UINT32 ){
         lIdxAreaIDs.insert(group->getID());
      }
   }

   // Right 
   set<uint32> rIdxAreaIDs;
   for (uint32 g=0; g<rItem->getNbrGroups(); g++){
      const OldItem* group = rMap->itemLookup(rItem->getGroup(g));
      if ( group != NULL &&
           rMap->getIndexAreaOrder(group->getID()) != MAX_UINT32 ){
         rIdxAreaIDs.insert(group->getID());
      }
   }

   // Compare size
   if ( lIdxAreaIDs.size()  < rIdxAreaIDs.size() ){
      mc2dbg8 << "   Cmp:t1 " << lItem->getID() << ":" << rItem->getID() 
             << endl;
      return true;
   }
   else if ( lIdxAreaIDs.size() > rIdxAreaIDs.size() ){
      mc2dbg8 << "   Cmp:f1 " << lItem->getID() << ":" << rItem->getID() 
             << endl;
      return false;
   }

   // Equal number of index area groups checking the groups.
   for (set<uint32>::const_iterator lIaIt = lIdxAreaIDs.begin(),
           rIaIt = rIdxAreaIDs.begin();
        lIaIt != lIdxAreaIDs.end() && rIaIt != rIdxAreaIDs.end();
        ++lIaIt, ++rIaIt ){
      mc2dbg8 << "      " << *lIaIt << " < " << *rIaIt << endl;
      if ( *lIaIt < *rIaIt ){
         mc2dbg8 << "   Cmp:t2 " << lItem->getID() << ":" << rItem->getID() 
                << endl;
         return true;
      }
      else if ( *lIaIt > *rIaIt ){
         mc2dbg8 << "   Cmp:f2 " << lItem->getID() << ":" << rItem->getID() 
                << endl;
         return false;
      }
   }
   mc2dbg8 << "   Cmp:f3 " << lItem->getID() << ":" << rItem->getID() << endl;
   return false; // Items have equal groups.
};

bool operator> ( const IndexHierarchy& lhih, const IndexHierarchy& rhih ) {
   return rhih < lhih;
};


bool
GMSMap::finishIndexAreas()
{
   mc2dbg1 << "GMSMap::finishIndexAreas" << endl;
   if ( ! NationalProperties::useIndexAreas(
            getCountryCode(), getMapOrigin()) ) {
      mc2dbg1 << "Not using index areas for country - exit" << endl;
      return false;
   }
   
   // Set convex hull as gfx data of all index areas.
   mc2log << info << "[finishIndexAreas] index area geometries convex hull"
          << endl;
   uint32 convHullGeometries = setIdxAreaGeometryConvexHull();
   mc2log << info << "[finishIndexAreas] Conv hull geometry for "
          << convHullGeometries << " index areas." << endl;

   bool customizedOK = customizeIndexAreaHierarchies();

   return ( (convHullGeometries > 0) || customizedOK );
}

bool
GMSMap::customizeIndexAreaHierarchies()
{
   mc2dbg1 << "GMSMap::customizeIndexAreaHierarchies in map " 
           << getMapID() << endl;
   if ( ! NationalProperties::useIndexAreas(
            getCountryCode(), getMapOrigin()) ) {
      mc2dbg1 << "Not using index areas for country - exit" << endl;
      return false;
   }


   // --------------------------------------------------------
   // * Remove index areas of order we do not want
   //    Collect the other index areas, and all items that are
   //    located in the wanted index areas
   // * Remove index areas that have a same-name index area as group
   // * Complete the index area hierarchies for all items.
   //    Set all index areas as group for the items that got only the most
   //    detailed index area as group in setItemLocation
   //    

   
   // --------------------------------------------------------
   // Remove index areas of orders we don't want in our maps, 
   // e.g. for UK order 0 - 6  (Put in NationalProperties)
   // Collect the other index areas.
   mc2dbg1 << "Remove index areas of unwanted orders" << endl;
   set<uint32> itemsToRemove;
   set<OldItem*> indexAreaItems; // Collected for further use.
   set<uint32> indexAreaOrders; // All orders that exists for this map
   set<OldItem*> otherItems;
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < getNbrItemsWithZoom(z); i++) {
         OldItem* item = (getItem(z, i));
         if ( item == NULL) {
            continue;
         }
         uint32 indexAreaOrder = getIndexAreaOrder(item->getID());
         if ( indexAreaOrder == MAX_UINT32 ) {
            // not an index area

            
            // Collect interesting items, that will have their index 
            // area hierarchy updated.
            // Items with at least one level 1 index area.
            vector<uint32> indexAreaIds;
            getIndexAreas(item, this, indexAreaIds);
            if ( indexAreaIds.size() > 0 ) {
               otherItems.insert(item);
               if ( itemNotInSearchIndex(item->getID()) ) {
                  cout << " Item " << item->getID() 
                       << " - is it really interesting??" << endl;
                  MC2_ASSERT(false);
               }
            }
            // Items that do not have index areas as their group
            // check (only for debug)
            else {
               if ( itemNotInSearchIndex(item->getID()) ) {
                  mc2dbg8 << " Item not in search index " << item->getID() 
                          << " (map " << getMapID() << ") "
                          << ItemTypes::getItemTypeAsString(
                                 item->getItemType())
                          << " without index area set" << endl;
               } else {
                  if ((item->getItemType() == ItemTypes::nullItem) ||
                      (item->getItemType() == ItemTypes::zipCodeItem) ||
                      (item->getItemType() == ItemTypes::builtUpAreaItem) ||
                      (item->getItemType() == ItemTypes::municipalItem) ||
                      (item->getItemType() == ItemTypes::pointOfInterestItem) ) {
                     // ok, not to have an index area.
                     // POIs added from WASP have no bua/mun groups
                  } else {
                     cout << " Item " << item->getID() << " "
                          << ItemTypes::getItemTypeAsString(
                                 item->getItemType())
                          << " without index area set - what to do??" << endl;
                     // no index area coverage some parts of the country
                     if ( item->getNbrGroups() > 0 ) {
                        // ok, located in something
                     } else {
                        mc2log << warn << "The item " 
                          << this->getMapID() << ":" << item->getID() << " "
                          << ItemTypes::getItemTypeAsString(
                                 item->getItemType())
                          << " has no IAgroup and no other group either"
                          << endl;
                        MC2_ASSERT(false);
                     }
                  }
               }
            }
            // end of check (for debug)
         }
         else {
            // index area
            if ( ! NationalProperties::indexAreaToUse(getCountryCode(),
                                                      getMapOrigin(),
                                                      indexAreaOrder) ){
               // This is an index area to remove.
               itemsToRemove.insert(item->getID());
            }
            else {

               // This indicates an index area with no index area order set.
               // No such thing should exist.
               MC2_ASSERT(indexAreaOrder != (MAX_UINT32-1) );
               
               // This is an index area to keep.
               indexAreaItems.insert(item);
               indexAreaOrders.insert(indexAreaOrder);
            }
         }

      }
   }
   mc2dbg1 << "removeItems itemsToRemove.size=" 
           << itemsToRemove.size() << endl;
   removeItems(itemsToRemove);


   // --------------------------------------------------------
   // Remove index areas that have a same-name index area as group
   // All items with the index-area-to-remove as group, will have
   // the same-name index area as group instead.
   // Loop the indexAreaItems,
   // Start with the most detailed index areas (UK: order 10)
   mc2dbg1 << "Remove index areas that have a same-name index area as group"
           << endl;
   itemsToRemove.clear();
   set<uint32>::const_reverse_iterator setIt;
   for ( setIt = indexAreaOrders.rbegin(); 
         setIt != indexAreaOrders.rend(); setIt++ ) {
      uint32 order = *setIt;
      for ( set<OldItem*>::const_iterator itemIt = indexAreaItems.begin();
           itemIt != indexAreaItems.end(); ++itemIt ){
         OldItem* indexAreaItem = *itemIt;
         uint32 indexAreaOrder = getIndexAreaOrder(indexAreaItem->getID());
         if ( indexAreaOrder != order ) {
            continue;
         }
         mc2dbg8 << "index area (map " << getMapID() << ") "
                 << indexAreaOrder << ":" << indexAreaItem->getID() 
                 << " " << getFirstItemName(indexAreaItem) << endl;
         
         // Loop my regions, check if same name
         // An index area should have max one region
         if ( indexAreaItem->getNbrGroups() > 1 ) {
            // If the groups have the same name and index area order,
            // then ok to have more than one group ??
            if ( ! indexAreaGroupsSameNameAndOrder( indexAreaItem ) ) {
               MC2_ASSERT(indexAreaItem->getNbrGroups() <= 1);
            } else {
               // OK 
               // possibly remove all but the first group ??
            }
         }
         uint32 sameNameRegionID = MAX_UINT32;
         for ( uint32 g = 0; g < indexAreaItem->getNbrGroups(); g++ ) {
            OldItem* group = itemLookup(indexAreaItem->getGroup(g));
            MC2_ASSERT(group != NULL);
            uint32 groupOrder = getIndexAreaOrder(group->getID());
            MC2_ASSERT(groupOrder != MAX_UINT32);
            MC2_ASSERT(groupOrder != indexAreaOrder);
            mc2dbg8 << "  group " << groupOrder << ":" << group->getID()
                    << " " << getFirstItemName(group) << endl;
            if ( indexAreaItem->hasSameNames(group) ) {
               if ( sameNameRegionID != MAX_UINT32 ) {
                  mc2log << warn << "More than one same name region for "
                         << indexAreaOrder << ":" << indexAreaItem->getID()
                         << " no1=" << sameNameRegionID
                         << " no2=" << group->getID() << endl;
               }
               MC2_ASSERT(sameNameRegionID==MAX_UINT32);
               mc2dbg8 << " same name region "
                       << groupOrder << ":" << group->getID() << endl;
               sameNameRegionID = group->getID();
            }
            // If hasSameNames did not do the job, try something else that
            // does not require exactly the same names, but is still
            // good enough
            if ( sameNameRegionID == MAX_UINT32 ) {
               // hasAllLangNames
               // All names of (other) item are present in this item, i.e. the 
               // names of (other) item is a subset of the names in this item.
               // Pays respect to language but not to name type.
               
               // OK if all names of indexAreaItem exists in group.
               // -> we are not removing any names from the map.
               if ( group->hasAllLangNames(indexAreaItem) ) {
                  mc2dbg1 << "indexAreaItem "
                          << indexAreaOrder << ":" << indexAreaItem->getID()
                          << " hasAllLangNames (inv) group " 
                          << group->getID() << endl;
                  MC2_ASSERT(sameNameRegionID==MAX_UINT32);
                  mc2dbg8 << " enough same name region "
                          << groupOrder << ":" << group->getID() << endl;
                  sameNameRegionID = group->getID();
               }
               // If the indexAreaItem has more names than group -> no OK.
               // - we do not want to remove names from the map.
            }
         }
         
         // If same name region, do changes
         if ( sameNameRegionID != MAX_UINT32 ) {
            vector<uint32> itemsInGroup = 
               getItemsInGroup(indexAreaItem->getID());
            mc2dbg1 << "index area "
                    << indexAreaOrder << ":" << indexAreaItem->getID() 
                    << " " << getBestItemName(indexAreaItem->getID())
                    << " same name as it's region " << sameNameRegionID
                    << ", group of " << itemsInGroup.size()
                    << " items" << endl;
            for (uint32 i=0; i<itemsInGroup.size(); i++){
               OldItem* item = itemLookup( itemsInGroup[i]);
               MC2_ASSERT(item != NULL);
               removeRegionFromItem(item, indexAreaItem);
               addRegionToItem(item, sameNameRegionID);
            }

            itemsToRemove.insert(indexAreaItem->getID());
         }
         
      }
   }
   mc2dbg1 << "removeItems itemsToRemove.size=" 
           << itemsToRemove.size() << endl;
   removeItems(itemsToRemove);



   // --------------------------------------------------------
   // Complete the index area hierarchies for all items
   // e.g. the parks and other items have, after setItemLocation,
   // only the index area with the highest index area order
   // (most detailed index area order) as their group.
   mc2dbg1 << "Complete index area hierarchies for all " 
           << otherItems.size() << " items" << endl;
   uint32 nbrItems = 0;
   for ( set<OldItem*>::const_iterator itemIt = otherItems.begin();
        itemIt != otherItems.end(); ++itemIt ){
      OldItem* item = (*itemIt);
      if ( item == NULL) {
         continue;
      }

      // Collect all index areas that are groups of this item
      // iterative, which means it collects all groups of groups
      // that are index areas
      set<uint32> allIndexAreaIds;
      getAllIndexAreas(item, this, allIndexAreaIds);
      // Compare with the level 1 index areas, 
      // (that is all index areas with direct links from the item)
      vector<uint32> indexAreaIds;
      getIndexAreas(item, this, indexAreaIds);
      if (allIndexAreaIds.size() == indexAreaIds.size()) {
         continue;
      }
      mc2dbg8 << " Complete item " << item->getID() << " nbr idxAreas="
              << allIndexAreaIds.size() << " (was " 
              << indexAreaIds.size() << ")" << endl;

      // Add all the index areas as regions for this item
      for ( set<uint32>::const_iterator setIt = allIndexAreaIds.begin();
            setIt != allIndexAreaIds.end(); setIt++ ) {
         addRegionToItem( item, *setIt );
      }
      nbrItems++;
   }
   mc2dbg1 << "Completed index area hierarchies for "
           << nbrItems << " items" << endl;


   // Exit here to keep all index area orders in the maps.
   // Items have all index areas as groups (not only the most detailed one)
   return true;

} // customizeIndexAreaHierarchies



uint32
GMSMap::getMostDetailedIndexArea( vector<OldItem*> indexAreaItems, 
                  multimap<uint32,uint32> indexAreasBasedOnOrder,
                  set<uint32> indexAreaOrders )
{

   set<uint32> candidates;
   for ( uint32 i=0; i<indexAreaItems.size(); i++){
      uint32 groupID = indexAreaItems[i]->getID();
      candidates.insert(groupID);
   }

   set<uint32>::const_iterator candIt;
   bool found = false;
   set<uint32>::const_reverse_iterator setIt = indexAreaOrders.rbegin();
   while ( !found && ( setIt != indexAreaOrders.rend()) ) {
      uint32 order = *setIt;
      multimap<uint32,uint32>::const_iterator it =
         indexAreasBasedOnOrder.lower_bound(order);
      while ( !found && (it != indexAreasBasedOnOrder.upper_bound(order)) ) {
         uint32 id = it->second;
         candIt = candidates.find(id);
         if (candIt != candidates.end() ) {
            found = true;
         } else {
            ++it;
         }
      }
      setIt++;
   }
   if ( found ) {
      return (*candIt);
   }

   return MAX_UINT32;
}

void
GMSMap::areaFeatureDrawDisplayClassesInit( mapItemIdToBB_t& mapItemIdToBB,
                                           mapTypeToItemId_t& mapTypeToItemId ) {

   vector<bool> dispClassItemTypes( (int)ItemTypes::numberOfItemTypes, false );
   
   // For each area feature draw display class, store 
   // interesting ItemTypes types.
   for( int i=0; i < ItemTypes::nbrAreaFeatureDrawDisplayClasses; i++) {
      if( i == ItemTypes::waterInCityPark ) {
         dispClassItemTypes[ ItemTypes::waterItem ] = true;
         dispClassItemTypes[ ItemTypes::parkItem ] = true;
      } else if ( i == ItemTypes::waterInCartographic ) {
         dispClassItemTypes[ ItemTypes::waterItem ] = true;
         dispClassItemTypes[ ItemTypes::cartographicItem ] = true;
      } else if ( i == ItemTypes::waterInBuilding ) {
         dispClassItemTypes[ ItemTypes::waterItem ] = true;
         dispClassItemTypes[ ItemTypes::buildingItem ] = true;
      } else if ( i == ItemTypes::buaOnIsland ) {
         dispClassItemTypes[ ItemTypes::builtUpAreaItem ] = true;
         dispClassItemTypes[ ItemTypes::islandItem ] = true;
      } else if ( i == ItemTypes::waterOnIsland ) {
         dispClassItemTypes[ ItemTypes::waterItem ] = true;
         dispClassItemTypes[ ItemTypes::islandItem ] = true;
      } else if ( i == ItemTypes::islandInBua ) {
         dispClassItemTypes[ ItemTypes::islandItem ] = true;
         dispClassItemTypes[ ItemTypes::builtUpAreaItem ] = true;
      } else if ( i == ItemTypes::cartographicInCityPark ) {
         dispClassItemTypes[ ItemTypes::cartographicItem ] = true;
         dispClassItemTypes[ ItemTypes::parkItem ] = true;
      } else if ( i == ItemTypes::cartographicInForest ) {
         dispClassItemTypes[ ItemTypes::cartographicItem ] = true;
         dispClassItemTypes[ ItemTypes::forestItem ] = true;
      } else if ( i == ItemTypes::IIWIPOutsideParkOutsideBua ) {
         dispClassItemTypes[ ItemTypes::islandItem ] = true;
         dispClassItemTypes[ ItemTypes::waterItem ] = true;
      } else if ( i == ItemTypes::IIWIPOutsideParkInsideBua ) {
         // Only used when we already have found an
         // island in water in park now want to determine whether
         // it is also inside a bua.
         dispClassItemTypes[ ItemTypes::islandItem ] = true;
         dispClassItemTypes[ ItemTypes::builtUpAreaItem ] = true;
      } else if ( i == ItemTypes::IIWIPInsidePark ) {
         // Only used when we already have found an
         // island in water in park now want to determine whether
         // it is also inside a bua.
         dispClassItemTypes[ ItemTypes::islandItem ] = true;
         dispClassItemTypes[ ItemTypes::parkItem ] = true;
      } else {
         mc2log << error 
                << "GMSMap::computeAreaFeatureDrawDisplayClasses "
                << "unknown areaFeatureDrawDisplayClass_t type" 
                << endl;
         MC2_ASSERT(false);
      }
   }
 
   /*
    * Map items to type and corresponding pre-computed bounding boxes.
    */
   
   ItemTypes::itemType curItemType;
   for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i=0; i<getNbrItemsWithZoom(z); ++i) {

         // Get info from current iterator item
         OldItem* curItem = getItem(z, i);
         curItemType = curItem->getItemType();

         // If item empty, go to next
         if ( curItem == NULL ){
            continue;
         } else if( dispClassItemTypes[ curItemType ] ) {

            // Park items are a special case where we only consider
            // city parks.
            if( curItemType == ItemTypes::parkItem ) {
               ItemTypes::parkType curParkSubType = 
                  (ItemTypes::parkType)((OldParkItem*)curItem)->getParkType();
            
               if ( curParkSubType != ItemTypes::cityPark ) {
                  continue;
               }
            }

            // For buas, do not include index area buas!
            if( curItemType == ItemTypes::builtUpAreaItem ) {
               if ( isIndexArea(curItem->getID()) ) {
                  continue;
               }
            }
               
            // Item ok, store info
            GMSGfxData* gfxData = curItem->getGfxData();  
            MC2BoundingBox bbox;
            if( gfxData != NULL ) {
               gfxData->getMC2BoundingBox(bbox); 

               mapItemIdToBB.insert(
                  make_pair(curItem->getID(), bbox));   

               mapTypeToItemId.insert(
                  make_pair( (ItemTypes::itemType)curItem->getItemType(), 
                              curItem));

               mc2dbg8 << info << "INIT: curItem: " << curItem->getID()
                      << endl;
            }
         }
      }
   }

   // Info abt collected candidates
   mc2dbg2 << "areaFeatureDrawDisplayClassesInit: collected bboxes for "
           << mapItemIdToBB.size() << " items" << endl;
   for ( uint32 i = 0; i < (uint32)(ItemTypes::numberOfItemTypes); i++ ) {
      if ( mapTypeToItemId.count(ItemTypes::itemType(i)) > 0 ) {
         mc2dbg2 << "  " << mapTypeToItemId.count(ItemTypes::itemType(i))
                 << " "
                 << ItemTypes::getItemTypeAsString(ItemTypes::itemType(i))
                 << endl;
      }
   }
} // areaFeatureDrawDisplayClassesInit

void 
GMSMap::itemInItemToDisplayClass( 
               ItemTypes::areaFeatureDrawDisplayClass_t displayClass,
               uint32 interiorItemMaxLength,
               uint32 exteriorItemMinLength,
               bool compareIntWithExt,
               float quotaRequired,
               mapTypeToItemId_t* interiorItemsToUse,
               mapTypeToItemId_t* exteriorItemsToUse,
               mapItemIdToBB_t mapItemIdToBB,
               mapTypeToItemId_t mapTypeToItemId,
               mapTypeToItemId_t* foundInteriorItems ) {

   // variables
   ItemTypes::itemType interiorItemType;
   ItemTypes::itemType exteriorItemType;
   OldItem* curInteriorItem;
   OldItem* curExteriorItem;

   // For each area feature draw display class, determine
   // which is the interior and exterior item types.

   if( displayClass == ItemTypes::waterInCityPark ) {
      interiorItemType = ItemTypes::waterItem;
      exteriorItemType = ItemTypes::parkItem;
   } else if ( displayClass == ItemTypes::waterInCartographic ) {
      interiorItemType = ItemTypes::waterItem;
      exteriorItemType = ItemTypes::cartographicItem;
   } else if ( displayClass == ItemTypes::waterInBuilding ) {
      interiorItemType = ItemTypes::waterItem;
      exteriorItemType = ItemTypes::buildingItem;
   } else if ( displayClass == ItemTypes::waterOnIsland ) {
      interiorItemType = ItemTypes::waterItem;
      exteriorItemType = ItemTypes::islandItem;
   } else if ( displayClass == ItemTypes::buaOnIsland ) {
      interiorItemType = ItemTypes::builtUpAreaItem;
      exteriorItemType = ItemTypes::islandItem;
   } else if ( displayClass == ItemTypes::islandInBua ) {
      interiorItemType = ItemTypes::islandItem;
      exteriorItemType = ItemTypes::builtUpAreaItem;
   } else if ( displayClass == ItemTypes::cartographicInCityPark ) {
      interiorItemType = ItemTypes::cartographicItem;
      exteriorItemType = ItemTypes::parkItem;
   } else if ( displayClass == ItemTypes::cartographicInForest ) {
      interiorItemType = ItemTypes::cartographicItem;
      exteriorItemType = ItemTypes::forestItem;
   } else if ( displayClass == ItemTypes::IIWIPOutsideParkOutsideBua ) {
      interiorItemType = ItemTypes::islandItem;
      exteriorItemType = ItemTypes::waterItem;
   } else if ( displayClass == ItemTypes::IIWIPOutsideParkInsideBua ) {
      // Only used when we already have found an
      // island in water in park, and thus, now want to determine whether
      // it is also inside a bua.
      interiorItemType = ItemTypes::islandItem;
      exteriorItemType = ItemTypes::builtUpAreaItem;
   } else if ( displayClass == ItemTypes::IIWIPInsidePark ) {
      // Only used when we already have found an
      // island in water in park, and thus, now want to determine whether
      // it is also actually inside the park (it may be located in a part
      // of water that is outside the park).
      interiorItemType = ItemTypes::islandItem;
      exteriorItemType = ItemTypes::parkItem;
   } else {
      mc2log << error 
             << "GMSMap::itemInItemToDisplayClass "
             << "unknown areaFeatureDrawDisplayClass_t type" 
             << endl;
      MC2_ASSERT(false);
   }
   mc2dbg2 << "itemInItemToDisplayClass "
           << ItemTypes::getStringForAreaFeatureDrawDisplayClass(displayClass)
           << " interior items = " << mapTypeToItemId.count(interiorItemType)
           << " exterior items = " << mapTypeToItemId.count(exteriorItemType)
           << endl;

   // Check if there are any items of the desired types
   if( mapTypeToItemId.find( interiorItemType ) == mapTypeToItemId.end() ||
       mapTypeToItemId.find( exteriorItemType ) == mapTypeToItemId.end() ) {
      return;
   }


   // Create iterators
   mapTypeToItemId_t::iterator interiorIt; 
   mapTypeToItemId_t::iterator exteriorIt; 

   if( interiorItemsToUse !=NULL ) {
      interiorIt =
         interiorItemsToUse->lower_bound( interiorItemType );
   } else {
      interiorIt =  
         mapTypeToItemId.lower_bound( interiorItemType );
   }

   if( exteriorItemsToUse != NULL ) {
      exteriorIt = 
         exteriorItemsToUse->lower_bound( exteriorItemType );
   } else {
      exteriorIt = 
         mapTypeToItemId.lower_bound( exteriorItemType );
   }


   // Create iterator upper bounds
   mapTypeToItemId_t::iterator interiorItUpperBound;
   if( interiorItemsToUse !=NULL ) {
      interiorItUpperBound =
         interiorItemsToUse->upper_bound( interiorItemType );
   } else {
      interiorItUpperBound =  
         mapTypeToItemId.upper_bound( interiorItemType );
   }

   mapTypeToItemId_t::iterator exteriorItUpperBound;
   if( exteriorItemsToUse != NULL ) {
      exteriorItUpperBound = 
         exteriorItemsToUse->upper_bound( exteriorItemType );
   } else {
      exteriorItUpperBound = 
         mapTypeToItemId.upper_bound( exteriorItemType );
   }


   // Loop through all interior items
   for ( ; 
         interiorIt != interiorItUpperBound;
         interiorIt++ ) {
      
      // Extract current int item and gfx data
      curInteriorItem = interiorIt->second;
      GfxDataFull* curInteriorGfxData = curInteriorItem->getGfxData();

      mc2dbg8 << info << "curInteriorItem: " << curInteriorItem->getID() 
             << endl;

      // Check that we have the desired item length
      if( !compareIntWithExt ) {
         if( (uint32)curInteriorGfxData->getLengthAll() > 
              interiorItemMaxLength ) {
            continue;
         } 
      } else {
         // Have to wait with length check until we have an exterior item
         // to compare with.
      }

      // reset external item iterator
      if( exteriorItemsToUse != NULL ) {
         exteriorIt = 
            exteriorItemsToUse->lower_bound( exteriorItemType );
      } else {
         exteriorIt = 
            mapTypeToItemId.lower_bound( exteriorItemType );
      }
 
      // Loop through all external items
      for ( ; 
            exteriorIt != exteriorItUpperBound;
            exteriorIt++ ) {
        
         // Extract current ext item and gfx data
         curExteriorItem = exteriorIt->second; 
         GfxDataFull* curExteriorGfxData = curExteriorItem->getGfxData();
  
         // Check that we have the desired length
         if( !compareIntWithExt ) {
            if( (uint32)curExteriorGfxData->getLengthAll() < 
                 exteriorItemMinLength ) {
               continue;
            }            
         } else {
            if( (uint32)curExteriorGfxData->getLengthAll() <=
                (uint32)curInteriorGfxData->getLengthAll() ) {
               continue;
            }
         }

         /*
          *  Check if interior item is inside exterior item
          * and compute area feature draw display class
          */
         
         // First get various info needed before checking item in item
         MC2BoundingBox interiorBox = 
            ( mapItemIdToBB.find( curInteriorItem->getID() ) )->second;
         MC2BoundingBox exteriorBox = 
            ( mapItemIdToBB.find( curExteriorItem->getID() ) )->second;
        
         GMSGfxData* gfxDataInterior = curInteriorItem->getGfxData();  
         GMSGfxData* gfxDataExterior = curExteriorItem->getGfxData();  

         uint32 nbrOfIntPolys = gfxDataInterior->getNbrPolygons();
         uint32 nbrOfExtPolys = gfxDataExterior->getNbrPolygons();

         uint32 intPolyIdx = 0;
         uint32 extPolyIdx = 0;

         uint32 nbrOfInteriorCoords = 0;
         uint32 coordIdx = 0;

         // If quotaRequired > 0, we need the total nbr
         // of coords over all polys in the interior item
         uint nbrOfIntCoordsOfAllPolys = 0;
         for( uint32 i = 0; i < nbrOfIntPolys; i++ ) {
            nbrOfIntCoordsOfAllPolys += 
               gfxDataInterior->getNbrCoordinates( i );
         }

         mc2dbg8 << info << "nbrOfIntCoordsOfAllPolys: "
                << nbrOfIntCoordsOfAllPolys << endl;

         uint32 totalNbrIntCoordsFound = 0;
         float quotaFound = 0;   


         // Do the check
         bool foundItemInItem = false;
         if( interiorBox.overlaps( exteriorBox ) ) {
            mc2dbg8 << "Overlapping " << endl;

            // Loop through all polys of the external item
            while ( extPolyIdx < nbrOfExtPolys && !foundItemInItem ) {
               mc2dbg8 << "extPolyIdx " << extPolyIdx << endl;

               // Loop through all polys of the internal item
               intPolyIdx = 0;
               while ( intPolyIdx < nbrOfIntPolys && !foundItemInItem  ) {
                  mc2dbg8 << "intPolyIdx: " << intPolyIdx << endl;   

                  // Check that both internal and external poly is closed
                  // and that both are not holes.
                  if( ! ( gfxDataInterior->getClosed( intPolyIdx ) &&
                          gfxDataExterior->getClosed( extPolyIdx ) ) ) {
                     intPolyIdx++;
                     continue;
                  }
         
                  mc2dbg8 << "Not closed " << endl;

                  if( !( gfxDataInterior->clockWise( intPolyIdx ) &&
                         gfxDataExterior->clockWise( extPolyIdx ) ) ) {
                     intPolyIdx++;
                     continue;
                  }

                  mc2dbg8 << "Not holes " << endl;

                  // Loop through all coords of the interior poly. 
                  nbrOfInteriorCoords = 
                     gfxDataInterior->getNbrCoordinates( intPolyIdx );
                
                  // Check that a sufficient number of coordinates of the
                  // interior item is inside the exterior item. 
                  if( quotaRequired > 0 ) {
                     mc2dbg8 << "quotaRequired > 0" << endl;
                     coordIdx = 0;
                     while ( coordIdx < nbrOfInteriorCoords && 
                             quotaFound < quotaRequired ) {
                        mc2dbg8 << "coordIdx " << coordIdx << endl;

                        // 0 = outside, 1 = on the boundry, 2 = inside
                        if( gfxDataExterior->insidePolygon ( 
                              gfxDataInterior->getLat( intPolyIdx, coordIdx ),
                              gfxDataInterior->getLon( intPolyIdx, coordIdx ),
                              extPolyIdx ) ) {  
                           totalNbrIntCoordsFound++; 
                           mc2dbg8 << "totalNbrIntCoordsFound " 
                                  << totalNbrIntCoordsFound << endl; 
                           mc2dbg8 << "nbrOfIntCoordsOfAllPolys " 
                                  << nbrOfIntCoordsOfAllPolys << endl; 

                           quotaFound = 
                              (float)totalNbrIntCoordsFound /
                                 (float)nbrOfIntCoordsOfAllPolys;
                           mc2dbg8 << "quotaFound: " << quotaFound << endl;
                        }
                        coordIdx++;
                     }
                     
                     // Check if we found more than required
                     if( quotaFound >= quotaRequired ) {
                        foundItemInItem = true;
                        mc2dbg8 << " foundItemInItem "
                                << ItemTypes::getStringForAreaFeatureDrawDisplayClass(displayClass)
                                << " quotaFound=" << quotaFound
                                << " intItem=" << curInteriorItem->getID()
                                << " extItem=" << curExteriorItem->getID()
                                << " coord "
                                << gfxDataInterior->getLat(intPolyIdx, coordIdx)
                                << " " 
                                << gfxDataInterior->getLon(intPolyIdx, coordIdx)
                                << endl;
                        setAreaFeatureDrawDisplayClass( 
                                    curInteriorItem->getID(), 
                                    displayClass,
                                    true );
                        if( foundInteriorItems != NULL ) {
                           foundInteriorItems->insert(
                              make_pair( interiorItemType, curInteriorItem ) );
                        }
                     }
                  } else {
                     mc2dbg8 << info << "display class NOT island in bua" 
                             << endl;
                     coordIdx = 0;
                     while (coordIdx < nbrOfInteriorCoords && 
                            !foundItemInItem ) {
                        mc2dbg8 << info << "coordIdx " << coordIdx << endl; 
                        // If a coord of the interior poly is found inside the
                        // exterior poly, we found an item in an item.
                        int inside = gfxDataExterior->insidePolygon ( 
                              gfxDataInterior->getLat( intPolyIdx, coordIdx ),
                              gfxDataInterior->getLon( intPolyIdx, coordIdx ),
                              extPolyIdx );
                        if( inside == 2 ) {
                           foundItemInItem = true;
                           mc2dbg8 << info << "FOUND " << endl; 
                           mc2dbg8 << " foundItemInItem "
                                   << ItemTypes::getStringForAreaFeatureDrawDisplayClass(displayClass)
                                   << " intItem=" << curInteriorItem->getID()
                                   << " extItem=" << curExteriorItem->getID()
                                   << " coord " 
                                   << gfxDataInterior->getLat(intPolyIdx, coordIdx)
                                   << " " 
                                   << gfxDataInterior->getLon(intPolyIdx, coordIdx)
                                   << endl;
                           setAreaFeatureDrawDisplayClass( 
                                 curInteriorItem->getID(), 
                                 displayClass, 
                                 true );
                           if( foundInteriorItems != NULL ) {
                              foundInteriorItems->insert(
                                 make_pair( interiorItemType, 
                                            curInteriorItem ) );
                           }
                        }
                        coordIdx++;
                     }  
                  }     
                  intPolyIdx++;
               } 
               extPolyIdx++;
            }
         }
      }
   }  
} // itemInItemToDisplayClass


void
GMSMap::computeAreaFeatureDrawDisplayClasses() {

   // Mappings needed
   mapItemIdToBB_t mapItemToBB;
   mapTypeToItemId_t mapTypeToItemId;

   areaFeatureDrawDisplayClassesInit(mapItemToBB, mapTypeToItemId );

   /*
    * Go through all area feature draw display classes, and identify
    * all items of each such class. 
    *
    */

              
   // Determine max water length and min park lenght. Used
   // when identifying waterInCityPark.
   uint32 maxWaterSize = 
      NationalProperties::AFDDCInteriorItemMaxLength( 
         getCountryCode(),
         ItemTypes::waterInCityPark );

   uint32 minParkLength = 
      NationalProperties::AFDDCExteriorItemMinLength( 
         getCountryCode(),
         ItemTypes::waterInCityPark );

   /*
    * compute the afddc for various item types
    *
    */

   mapTypeToItemId_t foundWaterInCityParkItems;
   itemInItemToDisplayClass( ItemTypes::waterInCityPark,
                             maxWaterSize,
                             minParkLength,
                             false,
                             0,
                             NULL,
                             NULL,
                             mapItemToBB,
                             mapTypeToItemId,
                             &foundWaterInCityParkItems );

   itemInItemToDisplayClass( ItemTypes::waterInCartographic,
                             4000,
                             0,
                             false,
                             0,
                             NULL,
                             NULL,
                             mapItemToBB,
                             mapTypeToItemId,
                             NULL );

   itemInItemToDisplayClass( ItemTypes::waterInBuilding,
                             35000,
                             0,
                             false,
                             0,
                             NULL,
                             NULL,
                             mapItemToBB,
                             mapTypeToItemId,
                             NULL );

   itemInItemToDisplayClass( ItemTypes::waterOnIsland,
                             MAX_UINT32, // disregarded
                             MAX_UINT32, // disregarded
                             true,
                             0,
                             NULL,
                             NULL,
                             mapItemToBB,
                             mapTypeToItemId,
                             NULL );

   // Fixme: Can't set buaOnIsland, until server 
   // can display buaOnIsland good (not over-drawing parks etc)
//   itemInItemToDisplayClass( ItemTypes::buaOnIsland,
//                             MAX_UINT32, // disregarded
//                             MAX_UINT32, // disregarded
//                             true,
//                             0,
//                             NULL,
//                             NULL,
//                             mapItemToBB,
//                             mapTypeToItemId,
//                             NULL );
   
   itemInItemToDisplayClass( ItemTypes::islandInBua,
                             MAX_UINT32, // disregarded
                             MAX_UINT32, // disregarded
                             true,
                             0.7,
                             NULL,
                             NULL,
                             mapItemToBB,
                             mapTypeToItemId,
                             NULL );

   // NOTE; 
   // For the IIWIPOutsideParkOutsideBua (IIWIPOPOB), 
   // IIWIPOutsideParkInsideBua (IIWIPOPIB), and IIWIPInsidePark (IIWIPIP)
   // the order of the below function calls matter. When identifying
   // IIWIPOPOB actually all IIWIP are identified, and initially 
   // classified as IIWIPOPOB, which may be wrong. However, since
   // IIWIPOPO and IIWIPIP have higher priority than the islands that
   // were wrongly classified as IIWIPOPOB, these will be correctly 
   // classified once IIWIPOPO and IIWIPIP are identified.
   mapTypeToItemId_t foundIIWIPItems;
   itemInItemToDisplayClass( ItemTypes::IIWIPOutsideParkOutsideBua,
                             MAX_UINT32, // disregarded 
                             MAX_UINT32, // disregarded
                             true,
                             0.9,
                             NULL,
                             &foundWaterInCityParkItems, 
                             mapItemToBB,
                             mapTypeToItemId,
                             &foundIIWIPItems );

   itemInItemToDisplayClass( ItemTypes::IIWIPOutsideParkInsideBua,
                             MAX_UINT32, // disregarded 
                             MAX_UINT32, // disregarded
                             true,
                             0.9,
                             &foundIIWIPItems,
                             NULL,
                             mapItemToBB,
                             mapTypeToItemId,
                             NULL );
   
   itemInItemToDisplayClass( ItemTypes::IIWIPInsidePark,
                             MAX_UINT32, // disregarded 
                             MAX_UINT32, // disregarded
                             true,
                             0.9,
                             &foundIIWIPItems,
                             NULL,
                             mapItemToBB,
                             mapTypeToItemId,
                             NULL );

   // No support implemented for
   // ItemTypes::cartographicInCityPark
   // ItemTypes::cartographicInForest

} // computeAreaFeatureDrawDisplayClasses

