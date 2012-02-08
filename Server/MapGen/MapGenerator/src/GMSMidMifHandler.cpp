/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSMidMifHandler.h"
#include "GfxUtility.h"
#include "NationalProperties.h"
#include "OldMapHashTable.h"
#include "CharEncoding.h"

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
#include "GMSMunicipalItem.h"
#include "GMSParkItem.h"
#include "GMSPedestrianAreaItem.h"
#include "GMSPointOfInterestItem.h"
#include "GMSRailwayItem.h"
#include "GMSStreetSegmentItem.h"
#include "GMSWaterItem.h"
#include "GMSZipCodeItem.h"
#include "GMSZipCodeItemNotice.h"
#include "GMSCartographicItem.h"

#include "GMSItem.h"

#include "Utility.h"
#include "Stack.h"


GMSMidMifHandler::GMSMidMifHandler(GMSMap* theMap)
{
   m_map = theMap;
   m_mapGfxData = NULL;
   m_midIdRefLoaded = false;
};

GMSMidMifHandler::~GMSMidMifHandler()
{
}

const uint32
GMSMidMifHandler::maxMidLineLength = 255;



int
GMSMidMifHandler::createItemsFromMidMif(
      const char* fileName,
      uint32 &nbrItemsInFile,
      bool addToExistingMap,
      bool setLocations,
      bool setTurnDesc,
      bool updateNodeLevelsInMap,
      bool useCoordToFindCorrectMap,
      bool addAllMidMifItems,
      bool tryToBuildMapGfxFromMunicipals)
{
   const int maxLineLength =16380; //perhaps not enough for reading ssi line
   nbrItemsInFile = 0;
   
   // Extract the itemtype from the filename
   m_itemType = ItemTypes::getItemTypeFromString(fileName);

   if ( (int) m_itemType > ( (int) ItemTypes::numberOfItemTypes -1) ) {
      mc2log << fatal << "ItemType could not "
             << "be extracted from the filename." << endl;
      MC2_ASSERT(false);
   } else {
      mc2dbg1 << "ItemType is " << (int) m_itemType << " "
              << StringTable::getString(
                  ItemTypes::getItemTypeSC(m_itemType),
                  StringTable::ENGLISH ) << endl;
   }

   uint32 nbrCreatedItems = 0;

   uint32 nbrOkMifFeatures = 0;
   uint32 nbrOkMidRows = 0;
   uint32 nbrNotOkMidRows = 0;

   // Create and open files for reading
   char* midFileName = new char[strlen(fileName) + 5];
   char* mifFileName = new char[strlen(fileName) + 5];

   strcpy(midFileName, fileName);
   strcat(midFileName, ".mid");
   strcpy(mifFileName, fileName);
   strcat(mifFileName, ".mif");

   ifstream midFile(midFileName);
   ifstream mifFile(mifFileName);

   delete [] midFileName;
   delete [] mifFileName;

   // Create mc2 <-> midmif references for the previous items added
   // to the map.
   m_midmifRefFilename = m_map->getFilename();
   m_midmifRefFilename += MC2String(".midmif_ref");
   m_midIdRefLoaded = false;

   // Check if an ESRI turntable exist for the ssi
   bool turntableExists = false;
   char* turntableName = new char[strlen(fileName) + 20];
   strcpy(turntableName, fileName);
   strcat(turntableName, "turntable.txt");
   ifstream turnTable(turntableName);
   if (turnTable && (m_itemType == ItemTypes::streetSegmentItem)) {
      mc2dbg8 << "Turntable for ssi exists: " << turntableName << endl;
      turntableExists = true;
   }
   delete [] turntableName;

   // If the mid/mif file contains routeable items to be added to 
   // m_map, loop over all items already in the map and for every routeable
   // item mark in its nodes that turndirections have been set.
   if ( setTurnDesc && 
        ( (m_itemType == ItemTypes::streetSegmentItem) ||
          (m_itemType == ItemTypes::ferryItem) ) ) {
      OldRouteableItem* curItem;
      for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
         for (uint32 i = 0; i < m_map->getNbrItemsWithZoom(z); i++) {
            curItem = dynamic_cast<OldRouteableItem*>(m_map->getItem(z,i));
            if ( (curItem != NULL) && 
                 (curItem->getItemType() == m_itemType) )
            {
               ((GMSNode*) curItem->getNode(0))->setTurnSet();
               ((GMSNode*) curItem->getNode(1))->setTurnSet();
            }
         }
      }
   }
   
   // Prepare creation of a country gfxData,
   // to be used when checking indsidePolygon for any items that are 
   // located on the boarder of m_map.
   // Fixme: This name string is not correct for all countries!!!
   //        Use country map name instead!!!!
   m_countryGfx = NULL; //new GfxData();
   char* countryMifName = new char[strlen(fileName) + 30];
   MC2String countryName = StringTable::getString(
      StringTable::getCountryStringCode(m_map->getCountryCode()),
      StringTable::ENGLISH);
   MC2String lowerName = StringUtility::copyLower(countryName);
   strcpy(countryMifName, "./");
   strcat(countryMifName, countryName.c_str());
   strcat(countryMifName, ".mif");
   if (addToExistingMap && !useCoordToFindCorrectMap) {
      mc2dbg2 << "If necessary for checking items on the boundry "
              << "a country gfx will be created from file "
              << countryMifName << endl;
   }
   ifstream countryMifFile(countryMifName);
   delete[] countryMifName;
  
   // Get the map gfxData and bounding box
   m_mapGfxData = m_map->getGfxData();
   if (addToExistingMap) {
      m_mapGfxData->getMC2BoundingBox(m_mapBBox);
   }
   // The map gfx data is NULL if running initMapFromMidMif
   // It is not NULL if running createItemsFromMidMif, and it is when
   // running createItemsFromMidMif that the map gfx data must be properly 
   // in order to get the items added to the correct map.
   if ( m_mapGfxData != NULL ) {
      mc2dbg8 << "map gfx nbr coords = " 
              << m_mapGfxData->getNbrCoordinates(0) << endl;
      if ( m_mapGfxData->getNbrCoordinates(0) == 4 ) {
         // The map gfxdata is bounding box
         // Update it temporarily to convex hull
         // to get faster insidePolygon calculations
         mc2dbg1 << "MapGfx is bbox, temp set it to conv hull" << endl;
         GMSGfxData* convHullGfx = getConvexHullForMapGfxData();
         if ( convHullGfx != NULL ) {
            m_mapGfxData = convHullGfx;
            mc2dbg8 << "map gfx nbr coords = " 
                    << m_mapGfxData->getNbrCoordinates(0) << endl;
         }
      }
      // Try to build a better gfxData from the municipals in the map
      if ( tryToBuildMapGfxFromMunicipals ) {
         mc2dbg1 << "tryToBuildMapGfxFromMunicipals" << endl;
         // skip noname municipals, not likely that we have something to add
         // in municipals with no-name = dummy features
         GMSGfxData* betterGfx = buildMapGfxFromMunicipals(true);
         if ( betterGfx != NULL ) {
            mc2dbg1 << "Temp set mapGfx to merge of municipals" << endl;
            m_mapGfxData = betterGfx;
            mc2dbg8 << "map gfx nbr coords = " 
                    << m_mapGfxData->getNbrCoordinates(0) << endl;
            // For debug
            //char tmpstr[128];
            //sprintf(tmpstr, "__mapGfxData_%d.txt", m_map->getMapID());
            //ofstream fileX( tmpstr );
            //betterGfx->printMif( fileX );
         } else {
            mc2dbg << "Failed to create a good mapgfx from the municipals"
                   << endl;
         }
      }
   }
   
   // Read the mif header.
   CoordinateTransformer::format_t coordsys;
   bool normalCoordinateOrder;
   uint32 utmzone;
   int32 falseNorthing, falseEasting;
   mc2dbg1 << "Reading mif-header of the item-file." << endl;
   if (!GMSGfxData::readMifHeader(mifFile, 
                                  coordsys, normalCoordinateOrder,
                                  utmzone, falseNorthing, falseEasting)) {
      mc2log << fatal << "Could not read mif header" << endl;
      MC2_ASSERT(false);
      return -1;
   }


   // Get the char encoding of the midmif files!
   // Defined in NationalProperties
   StringTable::countryCode coCode = m_map->getCountryCode();
   const char* mapOrigin = m_map->getMapOrigin();
   CharEncoding* chEncToMc2 = 
      NationalProperties::getMapToMC2ChEnc( coCode, mapOrigin );
   
   // Check if we are using index areas in the search model in this country
   // for this map version. - Defined in NationalProperties
   bool useIndexAreas =
            NationalProperties::useIndexAreas( coCode, mapOrigin);

   // Read the files and create items.
   char inbuffer[maxLineLength];
   inbuffer[0] = '\0';
   char coordbuffer[maxLineLength];
   coordbuffer[0] = '\0';
   m_midLineNbr = 1;
   while ( !midFile.eof() && !mifFile.eof() ) {
      
      // read the id-column from the mid-file, check that something
      // is read to go on.
      
      midFile.getline(inbuffer, maxLineLength, ',');
      if (strlen(inbuffer) < 1 && !midFile.eof()){
         mc2log << warn << here << "Strange mid feature:" << endl;
         mc2dbg << " nbrItemsInFile=" << nbrItemsInFile
                << " m_midLineNbr=" << m_midLineNbr << endl;
         nbrNotOkMidRows++;
      }
      else if (strlen(inbuffer) > 0) {
         nbrItemsInFile++;
         
         // extract mid id
         mc2dbg8 << "midId buffer=\"" << inbuffer << "\"" << endl;
         char* dest;
         uint32 midId = MAX_UINT32;
         if ( ! Utility::getUint32(inbuffer, dest, midId) ) {
            mc2log << warn << here << "Strange mid feature:" << endl;
            mc2dbg << " nbrItemsInFile=" << nbrItemsInFile
                   << " m_midLineNbr=" << m_midLineNbr << endl;
            mc2log << warn << here << "Couldn't get id from inbuffer=\""
                   << inbuffer << "\"" << endl;
            nbrNotOkMidRows++;
         }
         mc2dbg8 << "midId: " << midId << endl;
         
         // mid id must exist if we have a turn table
         if ( turntableExists && (midId == MAX_UINT32) ) {
            mc2log << warn << here << "Couldn't get midId for "
               << "ssi om midline " << m_midLineNbr << endl;
         }
         
         // skip officialName
         // (read the two '"' first if a ',' is included in the name)
         midFile.getline(inbuffer, maxLineLength, '"');
         midFile.getline(inbuffer, maxLineLength, '"');
         midFile.getline(inbuffer, maxLineLength, ',');
         
         // Read all names, nametypes and languages (including roadNumber).
         // The inbuffer is saved for now and processed after the item 
         // has been created and added to the map.
         // read past the first '"'
         midFile.getline(inbuffer, maxLineLength, '"');
         // all names
         midFile.getline(inbuffer, maxLineLength, '"');
         
         // create the midmif item gfxData
         GMSGfxData* curGfx = GMSGfxData::createNewGfxData(m_map, false);
         mc2dbg8 << "Creating gfxData for item "<< m_midLineNbr << "." << endl;
         bool gfxOK = curGfx->createGfxFromMif(mifFile,
                                  coordsys, normalCoordinateOrder,
                                  utmzone, falseNorthing, falseEasting);
         if ( ! gfxOK ) {
            mc2log << warn << "Could not create gfx from mif of mid line " 
                   << m_midLineNbr << endl;
         }

         // Create item with correct type and decide zoomlevel
         // Needs GMSItem for accessing MidMifData
         GMSItem* curGMSItem = NULL;
         OldItem* curItem = NULL;
         uint32 zoomLevel = 0;
         switch (m_itemType) {
            case (ItemTypes::streetSegmentItem) :
               zoomLevel = 0; // default
               mc2dbg8 << "new ssi, mid line " << m_midLineNbr << endl;
               curItem = new GMSStreetSegmentItem(m_map, 0, GMSItem::MIDMIF);
               break;
            
            case (ItemTypes::municipalItem) :
               zoomLevel = 0;
               //gfxData must be closed to be able to set locations
               if ( !curGfx->closed() ) {
                  curGfx->setClosed(0,true);
                  curGfx->updateLength();
               }
               mc2dbg8 << "new municipal item, " 
                      << "mid line " << m_midLineNbr << endl;
               curItem = new GMSMunicipalItem(GMSItem::MIDMIF);
               break;

            case (ItemTypes::waterItem) :
               zoomLevel = 1;
               curItem = new GMSWaterItem(GMSItem::MIDMIF);
               break;

            case (ItemTypes::parkItem) :
               zoomLevel = 1;
               curItem = new GMSParkItem(GMSItem::MIDMIF);
               break;

            case (ItemTypes::forestItem) :
               zoomLevel = 1;
               curItem = new GMSForestItem(GMSItem::MIDMIF);
               break;

            case (ItemTypes::buildingItem) :
               zoomLevel = 1;
               curItem = new GMSBuildingItem(GMSItem::MIDMIF);
               break;

            case (ItemTypes::railwayItem) :
               zoomLevel = 1;
               curItem = new GMSRailwayItem(GMSItem::MIDMIF);
               break;

            case (ItemTypes::islandItem) :
               zoomLevel = 1;
               curItem = new GMSIslandItem(GMSItem::MIDMIF);
               break;

            case (ItemTypes::builtUpAreaItem) :
               zoomLevel = 0;
               //gfxData must be closed to be able to set locations
               if ( !curGfx->closed() ) {
                  curGfx->setClosed(0,true);
                  curGfx->updateLength();
               }
               curItem = new GMSBuiltUpAreaItem(GMSItem::MIDMIF);
               break;

            case (ItemTypes::cityPartItem) :
               zoomLevel = ItemTypes::zoomConstCPI;
               curItem = new GMSCityPartItem(GMSItem::MIDMIF);
               break;

            case (ItemTypes::airportItem) :
               zoomLevel = 1;
               curItem = new GMSAirportItem(GMSItem::MIDMIF);
               break;

            case (ItemTypes::aircraftRoadItem) :
               zoomLevel = 1;
               curItem = new GMSAircraftRoadItem(GMSItem::MIDMIF);
               break;

            case (ItemTypes::militaryBaseItem) :
               zoomLevel = 4;
               curItem = new GMSMilitaryBaseItem(GMSItem::MIDMIF);
               break;

            case (ItemTypes::individualBuildingItem) :
               zoomLevel = 4;
               mc2dbg8 << "new individual building item, " 
                      << "mid line " << m_midLineNbr << endl;
               curItem = new GMSIndividualBuildingItem(GMSItem::MIDMIF);
               break;
        
            case (ItemTypes::ferryItem) :
               zoomLevel = 1;
               curItem = new GMSFerryItem(GMSItem::MIDMIF);
               break;
        
            case (ItemTypes::cartographicItem) :
               zoomLevel = 4;
               curItem = new GMSCartographicItem(GMSItem::MIDMIF);
               break;

            default :
               mc2log << error << here << "unsupported itemType " 
                      << int (m_itemType) << endl;
         } // switch

         if ( curItem == NULL ) {
            mc2log << error << "No item was created,"
                   << " maybe unsupported type - exit" << endl;
            MC2_ASSERT(false);
         }

         // create the item and set gfxData
         // if useCoordToFindCorrectMap don't read to EOL from mid file
         if ( curItem->createFromMidMif(
                  midFile, !useCoordToFindCorrectMap) ) {
            curItem->setGfxData(curGfx);
            nbrOkMidRows++;
         } else {
            delete curItem;
            curItem = NULL;
            mc2log << warn << here << "Strange mid feature:" << endl;
            mc2dbg << " nbrItemsInFile=" << nbrItemsInFile
                   << " m_midLineNbr=" << m_midLineNbr << endl;
            nbrNotOkMidRows++;
            //MC2_ASSERT(false); // ???
         }

         // If the gfx data could not be properly created, skip this item
         // (e.g. if there is a mif feature we can not handle)
         if ( ! gfxOK ) {
            mc2dbg << "item midId " << midId 
                   << " has bad gfx data - skip" << endl;
            delete curItem;
            curItem = NULL;
         }
         else {
            nbrOkMifFeatures++;
         }

         // Check if this midmif item will be added in this map
         uint32 curItemRegionId = MAX_UINT32; // if using left|right settlement
         bool itemRegionIsGroupItem = false;  // if using left|right settl
         curGMSItem = dynamic_cast<GMSItem*>(curItem);
         if ( addAllMidMifItems ) {
            // Always add this item
            // Read to EOL if there is a map ssi coordinate in the mid file
            if ( useCoordToFindCorrectMap ) {
               midFile.getline(coordbuffer, maxLineLength);
            }
         } else if ( addToExistingMap && ( curItem != NULL ) ) {
            
            // Map ssi coordinates?
            int32 mapSsiLat = MAX_INT32;
            int32 mapSsiLon = MAX_INT32;
            bool mapSsiCoords = false; // mapSsiLat and mapSsiLon is not 
                                       // used unless this is true.
            if ( useCoordToFindCorrectMap ) {
               // Check that this mid row contains only 2 more attributes
               uint32 nbrAttributes = 
                  getNumberExtraAttributes( midFile, false);
               if ( nbrAttributes != 2 ) {
                  mc2log << error << "Reading map ssi coord for midId="
                         << midId << " has " << nbrAttributes
                         << " attributes left on mid row - should be 2!"
                         << endl;
                  MC2_ASSERT(false);
               }
               
               // Read the rest of the line, expect to find mapssicoord
               midFile.getline(coordbuffer, maxLineLength);
               // Check if here are coordinates at the end of the mid line
               // Both ",lat,lon" or "lat,lon" is possible depending on if
               // the prior attribute value was a string or not..
               
               if ((strlen(coordbuffer) > 1) &&
                    (strstr(coordbuffer, ",") != NULL) ) {
                  char* coordPos = coordbuffer;
                  char* latStr = coordPos;
                  if ( strstr(coordbuffer, ",") == coordbuffer ) {
                     // initial commasign
                     latStr = strchr(coordPos, ',')+1;
                  }
                  char* lonStr = strchr(latStr, ',')+1;
                  if ( ! Utility::getNumber(latStr, dest, mapSsiLat) ) {
                     mc2log << error << "midId " << midId
                            << " problem with mapSsiLat buffer='"
                            << latStr << "'" << endl;
                  }
                  else if ( ! Utility::getNumber(lonStr, dest, mapSsiLon) ) {
                     mc2log << error << "midId " << midId
                            << " problem with mapSsiLon buffer='"
                            << lonStr << "'" << endl;
                  } else {
                     mapSsiCoords = true;
                     mc2dbg8 << "Searching for ssi within 2 meters from ("
                             << mapSsiLat << "," << mapSsiLon << ")" << endl;
                  }
               } else {
                  mc2log << error << "No map ssi coord string at the "
                         << "end of mid row for midId=" << midId << endl;
               }
               if ( ! mapSsiCoords ) {
                  mc2log << error << "No map ssi coord for midId "
                         << midId << endl;
                  MC2_ASSERT(false);
               }
            }

            uint32 leftSettlement = curGMSItem->getLeftSettlement();
            uint32 rightSettlement = curGMSItem->getRightSettlement();
            uint32 settlementOrder = curGMSItem->getSettlementOrder();
            mc2dbg8 << "midId " << midId << " settlement left="
                    << leftSettlement << " right=" << rightSettlement
                    << " order " << settlementOrder << endl;
            if ( (leftSettlement == 0) and (rightSettlement == 0) ) {
               // print warning???
               mc2log << warn 
                      << "The left+right settlement is 0, looking for "
                      << "municipal with midId=0. Correct?" << endl;
            }
            
            bool correctMap = 
                  correctMapForItem ( curGfx,
                                      mapSsiCoords, mapSsiLat, mapSsiLon,
                                      leftSettlement, rightSettlement,
                                      settlementOrder,
                                      curItemRegionId, /*outparam*/ 
                                      itemRegionIsGroupItem /*outparam*/ );
            if ( !correctMap ) {
               // The midmif item should not be added to this map
               delete curItem;
               curItem = NULL;
            }
         }
         
         // if ssi, find the zoomlevel from the roadclass
         if ((curItem != NULL) && 
             (m_itemType == ItemTypes::streetSegmentItem)) {
            int roadClass = 
               ((GMSStreetSegmentItem*) curItem)->getRoadClass();
            zoomLevel = roadClass + 2;
            mc2dbg4 << "setting zoomlevel=" << zoomLevel << endl;
         }
         
         // add the item
         bool added = false;
         if ( curItem != NULL ) {
            uint32 newID = m_map->addItem(curItem, zoomLevel);
            if ( newID != MAX_UINT32 ) {
               // OldItem added to map.
               mc2dbg1 << "IATM: map=" << m_map->getMapID();
               if (midId != MAX_UINT32) {
                  mc2dbg1 << " midid=" << midId;
               }
               mc2dbg1 << " newid=" << newID << endl;
               nbrCreatedItems++;
               added = true;
               //store the id
               m_addedItems.insert ( make_pair(midId, newID) );
               if (m_itemType == ItemTypes::streetSegmentItem) {
                  //set the nodeIDs
                  mc2dbg8 << "setting nodeIDs" << endl;
                  ((GMSStreetSegmentItem*)curItem)->getNode(0)
                     ->setNodeID(newID & 0x7FFFFFFF);
                  ((GMSStreetSegmentItem*)curItem)->getNode(1)
                     ->setNodeID(newID | 0x80000000);
               }
               
               // Set group if provided from the correctMapForItem method
               if ( curItemRegionId != MAX_UINT32 ) {
                  if ( itemRegionIsGroupItem ) {
                     m_map->addRegionToItem( curItem, curItemRegionId );
                  }
                  mc2dbg8 << "Adding region " << curItemRegionId << " to "
                          << newID << endl;
               }


               if ((m_itemType == ItemTypes::builtUpAreaItem) &&
                   useCoordToFindCorrectMap) {
                  // Check that there is at least one ssi inside the bua
                  // (geographicly).
                  // If not print a warning, a bua is of no use
                  // if there is nothing in it.
                  uint32 result = 
                     m_map->polygonHasItemsInside( curItem, 
                              ItemTypes:: streetSegmentItem);
                  if ( result == 0 ) {
                     mc2dbg << "There are no street segments "
                            << "in midmif-bua id=" << newID
                            << " mapid=" << m_map->getMapID() 
                            << " midID=" << midId << endl;
                  }
                  else if ( result == 1 ){
                     mc2dbg << "Only adjecent street segments "
                            << "in midmif-bua id=" << newID
                            << " mapid=" << m_map->getMapID() 
                            << " midID=" << midId << endl;
                  }
               }

               // Index areas
               if ( useIndexAreas ){
                  // If we have index areas for items from midmif
                  // only buas are index areas, if they have a
                  // index area order set in the bua mid file
                  // The municipals should never be kept in search index
                  if ( m_itemType == ItemTypes::builtUpAreaItem ) {
                     uint32 tempIndexAreaOrder = 
                              curGMSItem->getTempIndexAreaOrder();
                     if ( tempIndexAreaOrder != MAX_UINT32 ) {
                        mc2dbg1 << "set index area order " 
                                << tempIndexAreaOrder << " to bua " 
                                << newID << endl;
                        m_map->setIndexAreaOrder(newID,tempIndexAreaOrder);
                     } else {
                        mc2dbg1 << "set bua not in search idx " 
                                << newID << endl;
                        m_map->setItemNotInSearchIndex(newID);
                     }
                  } else if ( m_itemType == ItemTypes::municipalItem ) {
                     mc2dbg1 << "set municipal not in search idx " 
                             << newID << endl;
                     m_map->setItemNotInSearchIndex(newID);
                  }
               }

               // Road display class
               if ( m_itemType == ItemTypes::streetSegmentItem ) {
                  uint32 tmpRoadDisplayClass = 
                           curGMSItem->getTempRoadDisplayClass();
                  if ( tmpRoadDisplayClass != MAX_UINT32 ) {
                     ItemTypes::roadDisplayClass_t dp = 
                        ItemTypes::roadDisplayClass_t(tmpRoadDisplayClass);
                     m_map->setRoadDisplayClass(newID, dp);
                     mc2dbg8 << "Road disp class "
                             << ItemTypes::getStringForRoadDisplayClass(dp)
                             << " for ssi " << newID << endl;
                  }
               }
            }
         }
         
         // add the names (including roadNumber)
         if ( added ) { 
            uint32 nbrNames = 0;
            char* inbufferPos = inbuffer;
            if ( strlen(inbufferPos) == 0 )
               inbufferPos = NULL;
            while (inbufferPos != NULL) {
               // Read the strings representing name, type, language
               const char* nameStr = inbufferPos;
               const char* typeStr = strchr(inbufferPos, ':')+1;
               const char* languageStr = strchr(typeStr, ':')+1;
               // Update inbufferPos-pointer and Null-terminate the strings
               inbufferPos = StringUtility::strchr(languageStr, ' ');
               if (inbufferPos != NULL)
                  inbufferPos++;
               char* tmpStr = StringUtility::strchr(nameStr, ':');
               tmpStr[0] = '\0';
               tmpStr = StringUtility::strchr(typeStr, ':');
               tmpStr[0] = '\0';
               tmpStr = StringUtility::strchr(languageStr, ' ');
               if (tmpStr != NULL)
                  tmpStr[0] = '\0';
               
               mc2dbg8 << "nameStr=\"" << nameStr << "\", typeStr=\""
                       << typeStr << "\", langStr=\"" << languageStr
                       << "\", inbufferPos=\"" << inbufferPos << "\""
                       << endl;
                

               // If needed, convert name to char encoding type of mc2.
               MC2String mc2Dst;
               if ( chEncToMc2 != NULL ){
                  chEncToMc2->convert( MC2String(nameStr), mc2Dst );
                  nameStr = mc2Dst.c_str();
               }
                  
               
               //Match typeStr and languageStr with name_t and language_t
               ItemTypes::name_t nameType = 
                  ItemTypes::getStringAsNameType(typeStr);
               LangTypes::language_t language =
                  LangTypes::getStringAsLanguage(languageStr);

               if ( nameType == ItemTypes::invalidName ) {
                  mc2log << error << "nameType is invalid \"" 
                         << typeStr << "\"" << endl;
               }
               else{
            
                  if ( (language == LangTypes::invalidLanguage) &&
                           (nameType != ItemTypes::roadNumber)) {
                     mc2log << warn << "language is invalid \"" 
                            << languageStr << "\"" << endl;
                  }
               
                  mc2dbg8 << "nameType=" << (int) nameType 
                          << ", language=" << (int) language 
                          << endl;
                  //Don't add a null name
                  if ( (strlen(nameStr) > 0) && 
                       (StringUtility::strcasecmp(
                          nameStr, "Missing") != 0)) {
                     mc2dbg1 << "name " << nbrNames 
                             << ", nameStr= \"" << nameStr << "\""
                             << " (" << typeStr << ")" << endl;
                     m_map->addNameToItem(curItem, nameStr, 
                                          language, nameType);
                     nbrNames++;
                  }
               }
            } // while
            
            mc2dbg4 << "Added " << nbrNames 
                    << " names to this item." << endl;
         }

      } // if strlen > 0
      else {
         // We could have reached eof, but could also be that a space is 
         // leftout before the first "," (midID)
         // In that case: read the rest of the line to avoid errors due
         // to continuing reading the file from after the first "," 
         midFile.getline(inbuffer, maxLineLength);
         if (strlen(inbuffer) > 1) {
            mc2log << warn << here << "Check your mid file!!!! " 
                   << " midline=" << m_midLineNbr << endl;
         }
      }
      m_midLineNbr++;
   } // while
   
   if ( !midFile.eof() ) {
      mc2log << error << "Mid file not end-of-file!" << endl
             << " Can be different number of midrows/number of miffeatures"
             << endl;
      mc2dbg << "nbrItemsInFile=" << nbrItemsInFile
             << " m_midLineNbr=" << m_midLineNbr << endl;
      MC2_ASSERT(false);
   }
   if ( !mifFile.eof() ) {
      // we have not yet read the eof-character, test to read it
      // to see if there really are more thing to read, or not
      char tmp[maxLineLength];
      tmp[0] = '\0';
      mifFile >> tmp;
      if ( !mifFile.eof() ) {
         mc2log << error << "Mif file not end-of-file!" << endl
                << " Can be different number of midrows/number of miffeatures"
                << endl;
         mc2dbg << "Remaining data(within \" signs): \"" << tmp << "\"" 
                << endl;
         mc2dbg << "OK mid rows read:     " << nbrOkMidRows << endl;
         mc2dbg << "Not OK mid rows read: " << nbrNotOkMidRows << endl;
         mc2dbg << "OK mif rows features: " << nbrOkMifFeatures << endl;
         mc2dbg << "nbrItemsInFile=" << nbrItemsInFile
                << " m_midLineNbr=" << m_midLineNbr << endl;
         mc2dbg << " Search for \"Strange ....\"" << endl;
         MC2_ASSERT(false);
      }
   }
   
   if (m_countryGfx != NULL) {
      //delete m_countryGfx;
      m_countryGfx = NULL;
   }
   
   // Print result
   mc2log << info << "Created " << nbrCreatedItems << " items with itemtype " 
          << (int) m_itemType << " ("
          << StringTable::getString(
               ItemTypes::getItemTypeSC(m_itemType), StringTable::ENGLISH )
          << ") in map " << m_map->getMapID() << endl;

   // Write mid id - mc2 id to the mid ref file
   if ( nbrCreatedItems > 0 ) {
      writeMidMifReferences( m_midmifRefFilename );
   }


   // If built up areas were added perhaps set location
   // Don't do any merging of admin areas here
   if ((m_itemType == ItemTypes::builtUpAreaItem) &&
       (nbrCreatedItems > 0)) {

      // If given, set locations for the buas and items in the buas.
      if (setLocations) {
         m_map->setAllItemLocation(true);
      }
   }

   
   // If routeable items (ssi or ferry) were added we need e.g. to update
   // the connections and perhaps set turndescriptions for the added items
   if ( (nbrCreatedItems > 0) && 
        ( (m_itemType == ItemTypes::streetSegmentItem) ||
          (m_itemType == ItemTypes::ferryItem) ) ) {

      // Re-build hashtable
      mc2log << info << "Building hash table." << endl;
      m_map->buildHashTable();
     
      // Update the connections
      mc2log << info << "Updating connections for routeable items." << endl;
      uint32 nbrConnectionUpdates = 0;
      u32map::const_iterator cit;
      for ( cit = m_addedItems.begin(); cit != m_addedItems.end(); cit++ ) {
         OldRouteableItem* rItem = static_cast<OldRouteableItem*>
               (m_map->itemLookup( (*cit).second ));
         if (m_map->updateConnections(rItem)) {
            nbrConnectionUpdates++;
         }
      }
      mc2log << info << "Updated connections for " << nbrConnectionUpdates
             << " routeable items." << endl;
      
      // Update node levels for the new routeable items 
      // (necessary if the midmif supplier had level on street segments)
      if ( updateNodeLevelsInMap ) {
         uint32 nbrUpdates = m_map->updateNodeLevels();
         mc2log << info << "Updated node level in " << nbrUpdates 
                << " nodes." << endl;
      }

      // extract the restrictions from the ESRI turntable
      if (turntableExists) {
         mc2log << info << "Extract restrictions from turntable." << endl;
         extractRestrictionsFromTurnTable( turnTable );
      }
      
      // If requested, set turndescriptions for the new items
      // This works due to the for-loop in the beginning of this function,
      // that set TurnSet = true for all "old" routeable items.
      if ( setTurnDesc ) {
         mc2log << info << "Setting turn descriptions" << endl;
         m_map->initTurnDescriptions();
      }
      
      if (setLocations) {
         mc2log << info << "Setting location for the added items" << endl;
         //setItemLocation(ItemTypes::municipalItem, true);
         //setItemLocation(ItemTypes::builtUpAreaItem, true);
         m_map->setAllItemLocation(true);
      }
      
      // Create virtual items for boundry segments from nodes that have
      // junctionType borderCrossing
      mc2dbg1 << "Create virtual items for borderNodes." << endl;
      uint32 nbrVirtuals = 0;
      for ( cit = m_addedItems.begin(); cit != m_addedItems.end(); cit++ ) {
         OldRouteableItem* rItem = static_cast<OldRouteableItem*>
               (m_map->itemLookup( (*cit).second ));
         for ( uint32 n = 0; n < 2; n++ ) {
            if ( rItem->getNode(n)->getJunctionType() 
                  == ItemTypes::borderCrossing ) {
               uint32 virtualID = 
                     m_map->addToBoundary( rItem->getNode(n)->getNodeID() );
               if ( virtualID != MAX_UINT32 ) {
                  nbrVirtuals++;
                  mc2dbg4 << "Virtual item id " << virtualID << " created for "
                          << (*cit).second << " node " << n << endl;
               }
            }
         }
      }
      if ( nbrVirtuals > 0 ) {
         mc2log << info << "Created " << nbrVirtuals << " virtual items"
                << " because of borderNodes" << endl;
      }

      // Close SSIs with control access for pedestrians.
      // Use m_addedItems: first=midID, second=itemID.
      mc2dbg << "Closing controlled access SSIs for pedestrians" << endl;
      uint32 closePedestrianMask = 0xffffff7f;
      uint32 nbrSSIs = 0;
      uint32 nbrSSIsCtrlAccess = 0;
      uint32 nbrSSIsAlreadyClosed = 0;
      uint32 nbrSSIsClosed = 0;
      uint32 nbrConnsClosed = 0;
      for ( cit = m_addedItems.begin(); cit != m_addedItems.end(); cit++ ) {
         uint32 itemID = cit->second;
         OldStreetSegmentItem* ssi = 
            dynamic_cast<OldStreetSegmentItem*>(m_map->itemLookup(itemID));
         if ( ssi == NULL ){
            continue;
         }
         nbrSSIs++;
         if ( !ssi->isControlledAccess()){
            continue;
         }
         nbrSSIsCtrlAccess++;

         // This is a ssi with controlled access set.
         //
         // Handle both nodes.
         bool alreadyClosed = false;
         for (uint32 n=0; n<2; n++){
            OldNode* node = ssi->getNode(n);
            // Handle all connections.
            for (uint32 c=0; c<node->getNbrConnections(); c++){
               OldConnection* conn = node->getEntryConnection(c);
               uint32 oldVclRestr = conn->getVehicleRestrictions();
               uint32 newVclRestr = oldVclRestr & closePedestrianMask;
               if ( oldVclRestr == newVclRestr ){
                  alreadyClosed = true;
               }
               else {
                  // Close the entry connection for pedestrians.
                  conn->setVehicleRestrictions(newVclRestr);
                  nbrConnsClosed++;
               }
            }
         }
         if ( alreadyClosed ){
            nbrSSIsAlreadyClosed++;
         }
         else {
            nbrSSIsClosed++;
         }
      }
      mc2dbg << setfill(' ');
      mc2dbg << "   Nbr SSIs:                              " << setw(6) 
             << nbrSSIs << endl;
      mc2dbg << "   Nbr ctrl access SSIs:                  " << setw(6) 
             << nbrSSIsCtrlAccess << endl;
      mc2dbg << "   Nbr ctrl access SSIs already closed:   " << setw(6) 
             << nbrSSIsAlreadyClosed << endl;
      mc2dbg << "   Nbr ctrl access SSIs changed to closed:" << setw(6) 
             << nbrSSIsClosed << endl;
      mc2dbg << endl;
      mc2dbg << "   Nbr connections  changed to closed:    " << setw(6) 
             << nbrConnsClosed << endl;

   }


   // If street segment items were added, check if to create zip code items,
   // or if to update map gfx data
   if ( (nbrCreatedItems > 0) && 
        ( m_itemType == ItemTypes::streetSegmentItem) ) {
      
      //  build OldZipCodeItems from any tmpZipCodes added to ssi
      uint32 nbrZipCodeItems = m_map->createZipsFromGMSStreetSegmentItems();
      mc2log << info << "Created " << nbrZipCodeItems << " zipCodeItems "
             << "from gms ssi." << endl;
      
      if (useCoordToFindCorrectMap) {
         // rebuild the map gfxData, force convex hull
         mc2log << info << "Rebuilding the mapgfx using convex hull" << endl;
         m_map->setMapGfxDataConvexHull();
      }

   }

   delete chEncToMc2;
   chEncToMc2 = NULL;

   return nbrCreatedItems;
}

bool
GMSMidMifHandler::correctMapForItem( GfxData* itemGfx,
                                     bool mapSsiCoords,
                                     int32 itemLat, int32 itemLon,
                                     uint32 leftSettlementId,
                                     uint32 rightSettlementId,
                                     uint32 settlementOrder,
                                     uint32& itemRegionId,
                                     bool& itemRegionIsGroupItem )
{
   bool correctMap = false;

   // Use map ssi coordinate if provided
   if ( mapSsiCoords ) {
   
      OldMapHashTable* mht = m_map->getHashTable();
      MC2_ASSERT(mht != NULL);
      mht->clearAllowedItemTypes();
      mht->addAllowedItemType(ItemTypes::streetSegmentItem);
      // Find closest street segment
      uint64 dist;
      uint32 closestSSI = mht->getClosest(itemLon, itemLat, dist);
      mc2dbg8 << "Found closest ssi " << closestSSI << " dist="
              << sqrt(dist)*GfxConstants::MC2SCALE_TO_METER 
              << " meters" << endl;

      if (sqrt(dist)*GfxConstants::MC2SCALE_TO_METER < 2) {
         // correct map
         mc2dbg8 << " -> correct map." << endl;
         correctMap = true;
      } else {
         mc2dbg8 << " -> not the correct map." << endl;
      }
      return correctMap;
   }

   // Use left and right settlement
   if ( (leftSettlementId != MAX_UINT32) ||
        (rightSettlementId != MAX_UINT32) ) {
      
      // Load midid references if not read before
      readMidMifReferences( m_midmifRefFilename );

      // Either one or both settlements were set for this midmif item.
      // Use left before right.
      uint32 orderMidId = leftSettlementId;
      if ( orderMidId == MAX_UINT32 ) {
         orderMidId = rightSettlementId;
      }
      
      // Get item type from settlement order
      uint32 orderItemType = MAX_UINT32;
      switch ( settlementOrder ) {
         case ( 8 ) :
            orderItemType = uint32(ItemTypes::municipalItem);
            break;
         case ( 9 ) :
            orderItemType = uint32(ItemTypes::cityPartItem);
            break;
         case ( 99 ) :
            orderItemType = uint32(ItemTypes::builtUpAreaItem);
            break;
         default :
            break;
      }

      // Check if there is an orderItemType item with midid orderMidId
      uint32 orderMc2Id = MAX_UINT32;
      if ( orderItemType != MAX_UINT32 ) {
         bool itemTypeExist = true;
         orderMc2Id = getMc2Id(orderItemType, orderMidId, itemTypeExist);

         if ( ! itemTypeExist || ( orderMc2Id == MAX_UINT32) ) {
            // There were no items with type orderItemType
            // If looked for order9 try order8..
            if ( orderItemType == uint32(ItemTypes::cityPartItem) ) {
               orderItemType = uint32(ItemTypes::municipalItem);
               orderMc2Id = getMc2Id(orderItemType, orderMidId, itemTypeExist);
            }
         }
      }

      mc2dbg8 << " mid item " << m_midLineNbr << " settlement=" << orderMidId
              << " ordertype=" << orderItemType
              << " -> orderMc2Id=" << orderMc2Id << endl;
      
      // This is the correct map for the mid item
      if ( orderMc2Id != MAX_UINT32 ) {
         correctMap = true;
         // Use orderMc2Id to set group of the mid item if the order is
         // either municipal, bua or city part.
         if ( (orderItemType == uint32(ItemTypes::municipalItem)) ||
              (orderItemType == uint32(ItemTypes::builtUpAreaItem)) ||
              (orderItemType == uint32(ItemTypes::cityPartItem)) ) {
            itemRegionId = orderMc2Id;
         }
         itemRegionIsGroupItem = false;
         if ( dynamic_cast<OldGroupItem*>(
                  m_map->itemLookup(itemRegionId)) != NULL ) {
            itemRegionIsGroupItem = true;
         }
      }

      return correctMap;
   }

   // Use item gfx
   // Start with assuming that m_map is the correct map.
   correctMap = true;
   if ( itemGfx != NULL ) {
      if ((m_itemType == ItemTypes::streetSegmentItem) && 
          ((itemGfx->getNbrCoordinates(0) == 1) ||
           (itemGfx->getLength(0) == 0))) {
         // Don't add any 0-length street segments to the map, since 
         // connections etc will not be set correctly then.
         correctMap = false;
         mc2dbg2 << "OldItem on midline " << m_midLineNbr << " is a ssi with "
                 << itemGfx->getNbrCoordinates(0) << " coordinates and "
                 << "length=" << itemGfx->getLength(0) << ". Don't add." 
                 << endl;
      } else {
         // Check if the item fits m_map using the mapGfx.
         int32 lat, lon;
         uint32 coord = 0;
         itemGfx->getCoordinate(coord, lat, lon);
         if ( ! m_mapBBox.contains(lat, lon)) {
            // outside map bbox
            correctMap = false;
         } else {
            // inside the bounding box, might fit in m_map
            // check if inside the m_mapGfxData
            int inside = m_mapGfxData->insidePolygon(lat, lon);
            coord++;
            uint32 curPoly = 0;
            while ( (inside==1) && (curPoly<itemGfx->getNbrPolygons())) {
               if (curPoly > 0) {
                  coord = 0;
               }
               while ( (inside == 1) &&
                       ( (uint32) coord < 
                           itemGfx->getNbrCoordinates(curPoly))) {
                  inside = m_mapGfxData->insidePolygon(
                        itemGfx->getLat(curPoly, coord), 
                        itemGfx->getLon(curPoly, coord));
                  coord++;
               }
               ++curPoly;
            }
            if (inside == 0) {
               correctMap = false;
            } else if (inside == 1) {
               // all coords of the item are on the map border
               mc2dbg2 << "The item is located on the map border" 
                       << endl;
               int32 checklat = 1, checklon = 1;
               GfxUtility::generatePointInMap(
                  itemGfx->getLat(0,0), itemGfx->getLon(0,0),
                  itemGfx->getLat(0,1), itemGfx->getLon(0,1),
                  checklat, checklon);
               if (!m_mapGfxData->insidePolygon(checklat, checklon)) {
                  // Possible fixme: 
                  // Use the country polygon to check if the point is inside 
                  // the country, then the item will be inside another mcm map
                  // Else if the point is outside the country, this is 
                  // the correct map
                  if (m_countryGfx == NULL) {
                     mc2dbg2 << "Creating the countryGfx! for midline=" 
                             << m_midLineNbr << endl;
                     m_countryGfx = GMSGfxData::createNewGfxData(NULL);
                     //m_countryGfx->createFromMif(countryMifFile);
                  }
                  //if (m_countryGfx->insidePolygon(checklat, checklon)) {
                  //   correctMap = false;
                  //}
               }
               // else: this is the correct map
            }
            // else inside == 2 -> this is the correct map
         
         }
      }
   }

   return correctMap;
}

bool
GMSMidMifHandler::extractRestrictionsFromTurnTable(
                  ifstream& turnTable)
{
   // The turntable, use ARC1_ ARC2_ IMPEDANCE (fromid toid restriction)
   // (the order and number of attribues may vary between turntables)
   // This is the turntable for Manama Bahrain:
   // 0. KEY
   // 1. NODE_
   // 2. ARC1_
   // 3. ARC2_
   // 4. AZIMUTH
   // 5. ANGLE
   // 6. ARC1_ID
   // 7. ARC2_ID
   // 8. IMPEDANCE

   uint32 MAXLENGTH = 255;
   char buf[MAXLENGTH];
   buf[0] = '\0';

   uint32 fromCol = MAX_UINT32, toCol = MAX_UINT32, restrCol = MAX_UINT32;
   bool allColsFound = false;
   
   // Read the first line and find the column of fromId, toId and restriction
   // (the name of toid and fromid is always the same in any turntable)
   uint32 col = 0;
   while (!turnTable.eof() && !allColsFound) {
      turnTable >> buf;
      if (strcasecmp(buf, "ARC1_") == 0)
         fromCol = col;
      else if(strcasecmp(buf, "ARC2_") == 0)
         toCol = col;
      else if (strcasecmp(buf, "IMPEDANCE") == 0)
         restrCol = col;
      
      if ( (fromCol != MAX_UINT32) && (toCol != MAX_UINT32) &&
           (restrCol != MAX_UINT32) ) {
         allColsFound = true;
      }
      
      col++;
   }
   
   if (!allColsFound) {
      // Did not find the columns
      mc2log << error << "Did not find all the columns for toid, fromid "
             << "and restriction." << endl
             << "from: " << fromCol << " to: " << toCol 
             << " restriction: " << restrCol << endl;
      MC2_ASSERT(false);
   }
   mc2dbg4 << "The turntable has at least " << col << " columns: "
           << "from=" << fromCol << " to=" << toCol 
           << " restriction=" << restrCol << endl;
   // Read to end of the first line
   turnTable.getline(buf, MAXLENGTH);

   // Loop the turntable and for every line note the fromid, toid and 
   // restriction.
   // If fromid != toid and the turn is restricted ( < 0 ), get the
   // gmsSSIID from the m_addedItems, get the connection between the 
   // segments and set restrictions.

   uint32 nbrCols = col;
   uint32 nbrRestrictions = 0, nbrRestrictionsSet = 0, nbrBifurcationsSet = 0,
          nbrRestrictionsSetAllToId = 0, nbrNoConn = 0;
   while ( !turnTable.eof() ) {

      nbrRestrictions++;
      int32 restr = 0, fromid = -1, toid = -1;
      col = 0;
      while (col < nbrCols) {
         turnTable >> buf;

         if (col == fromCol) {
            fromid = strtol(buf, NULL, 10);
         } else if (col == toCol) {
            toid = strtol(buf, NULL, 10);
         } else if (col == restrCol) {
            restr = strtol(buf, NULL, 10);
         }
         
         col++;
      }
      turnTable.getline(buf, MAXLENGTH);

      if ( (fromid == -1) && (toid == -1) ) {
         mc2log << warn << "Restriction " << nbrRestrictions 
              << " not valid: fromid == toid == -1" << endl;
         continue; // read next row of the turn table file
      }

      // Ok, we have the toid, fromid and restriction.
      // Check if this is a restriction we want to set.
      if ( fromid != toid ) {
         mc2dbg8 << " Restriction from " << fromid << " to " << toid 
                 << " restr " << restr
                 << ", restnbr " << nbrRestrictions << endl;

         u32map::iterator fromit = m_addedItems.find(fromid);
         u32map::iterator toit = m_addedItems.find(toid);
         
         if ( restr < 0 ) {
            // If fromId is not given - set restrictions for all connections 
            // leading to toId
            if ( (fromid == -1) && (toit != m_addedItems.end()) ) {
               // Vehicle restriction
               if ( restr == -1 ) {
                  mc2dbg8 << "Setting restr for all conns to midItem " << toid
                          << " itemID " << (*toit).second << endl;
                  OldRouteableItem* ri = dynamic_cast<OldRouteableItem*>
                        (m_map->itemLookup((*toit).second));
                  if (ri != NULL) {
                     OldConnection* conn = NULL;
                     for ( uint32 n = 0; n < 2; n++ ) {
                        OldNode* node = ri->getNode(n);
                        for (uint16 c = 0; c < node->getNbrConnections(); c++) {
                           conn = node->getEntryConnection(c);
                           conn->setVehicleRestrictions(0x80);
                        }
                     }
                     nbrRestrictionsSetAllToId++;
                  }
               }
               else {
                  mc2log << error << "Cannot handle restriction val="
                         << restr << " with only toSegment given" << endl;
                  MC2_ASSERT(false);
               }
            }
            
            // The ids exists, get the connection and set restriction or
            // bifurcation.
            // If a junction coordinate was provided to mark the toNode of 
            // the connections, check that it is ok (there might be 2 conns 
            // from fromit to toit if they go in a circle)
            else if ( (fromit != m_addedItems.end()) &&
                      (toit != m_addedItems.end()) ) {
               
               OldConnection* conn = NULL;
               uint32 toNodeId = m_map->getConnection(
                                    (*fromit).second, (*toit).second, conn);
               if (conn != NULL) {
                  mc2dbg4 << "Restriction for conn from " << (*fromit).second
                          << " (" << fromid << ") to " << (*toit).second
                          << " (" << toid << "), restriction nbr "
                          << nbrRestrictions << endl;
                  
                  // Vehicle restriction
                  if ( restr == -1 ) {
                     conn->setVehicleRestrictions(0x80);
                     nbrRestrictionsSet++;
                  }
                  // Bifurcation
                  else if ( restr == -2 ) {
                     // Get the toNode of the connection
                     OldNode* toNode = m_map->nodeLookup( toNodeId );
                     if ( toNode != NULL ) {
                        mc2dbg1 << "Setting bifurcation to node "
                                << toNodeId << " (" << (*fromit).second
                                << "->" << (*toit).second << ")" << endl;
                        toNode->setJunctionType( ItemTypes::bifurcation );
                        nbrBifurcationsSet++;
                     }
                  }
                  
               } else {
                  mc2log << warn << "No connection from "
                         << (*fromit).second << " to " << (*toit).second << ":"
                         << " turntable: fromid=" << fromid << " toid=" << toid
                         << ", forbidden turn nbr " << nbrRestrictions << endl;
               }
            }

            // One of the ids exists in this map, the other is probably
            // added to another map, so this restriction is valid for
            // an external connection..
            else if ( (fromit != m_addedItems.end()) ||
                      (toit != m_addedItems.end()) ) {
               mc2dbg1 << "Restriction on external connection fromid="
                       << fromid << " toid=" << toid;
               if (fromit != m_addedItems.end())
                  mc2dbg1 << " fromMc2=" << fromit->second;
               else
                  mc2dbg1 << " toMc2=" << toit->second;
               mc2dbg1 << endl;
            }
            
            // else Road ids not found (not in this map..)
      
         } else {
            
            // No restriction, check that the connection exists in the map
            if ( (fromit != m_addedItems.end()) &&
                 (toit != m_addedItems.end()) ) {
               // The ids exists, check the connection
               OldConnection* conn = NULL;
               m_map->getConnection((*fromit).second, (*toit).second, conn);
               if (conn == NULL) {
                  mc2log << error << "No connection from "
                         << (*fromit).second << " to " << (*toit).second << ":"
                         << " allowed turn nbr " << nbrRestrictions << endl;
               }
            }
         }
      
      } else if ( !turnTable.eof() ) {
         // fromid=toid, no connection we want.
         nbrNoConn++;
      }
   
   }
   
   mc2log << info << "Added " << nbrRestrictionsSet << "+" 
           << nbrRestrictionsSetAllToId << "+" << nbrBifurcationsSet
           << " of " << nbrRestrictions-1
           << " restrictons from the turntable (nbr conn "
           << nbrRestrictions - 1 - nbrNoConn << ")." << endl;
   
   return true;
}

bool
GMSMidMifHandler::writeMidMifReferences( const MC2String& fileName )
{
   // Load all, so far written references
   readMidMifReferences( fileName );

   // Add the items in m_addedItems to m_midIdsRef
   midIdsRef_t::iterator mit = m_midIdsRef.find( int(m_itemType) );
   if ( mit != m_midIdsRef.end() ) {
   
      // mid id mc2 id
      for ( u32map::const_iterator cit = m_addedItems.begin();
            cit != m_addedItems.end(); cit++ ) {
         
         mit->second.insert( make_pair( cit->first, cit->second) );
      }
      mc2dbg8 << "Added " << m_addedItems.size()
              << " midId-mc2Ids to reference for existing item type "
              << int(m_itemType) << endl;
   } else {
      mc2ByMid_t mc2ByMid;
      
      // mid id mc2 id
      for ( u32map::const_iterator cit = m_addedItems.begin();
            cit != m_addedItems.end(); cit++ ) {
         
         mc2ByMid.insert( make_pair( cit->first, cit->second) );
      }
      mc2dbg8 << "Added " << m_addedItems.size()
              << " midId-mc2Ids to reference for new item type "
              << int(m_itemType) << endl;

      m_midIdsRef.insert( make_pair( int(m_itemType), mc2ByMid ) );
      
   }

   // Open the ref file for writing
   ofstream file( fileName.c_str() );
   if ( ! file ) {
      mc2log << error << " Could not open " << fileName << endl;
      MC2_ASSERT(false);
      return false;
   }

      
   for ( midIdsRef_t::const_iterator mit = m_midIdsRef.begin();
         mit != m_midIdsRef.end(); mit++ ) {
      mc2ByMid_t mc2ByMid = mit->second;
      file << int (mit->first) << " " << mc2ByMid.size() << endl;
      for (mc2ByMid_t::const_iterator it = mc2ByMid.begin();
           it != mc2ByMid.end(); it++ ) {
         file << "  " << it->first << " " << it->second << endl;
      }
   }


   return true;
}

bool
GMSMidMifHandler::readMidMifReferences( 
   const MC2String& fileName, bool printOut )
{

   // The midmif references alread loaded?
   if (m_midIdRefLoaded) {
      return true;
   }

   ifstream file( fileName.c_str(), ios::in );
   if ( ! file ) {
      mc2log << error << here << " Could not open " << fileName << endl;
      return false;
   }

   uint32 itemType, nbrItems;

   file >> itemType;
   while ( ! file.eof() ) {

      mc2ByMid_t mc2ByMid;
      
      file >> nbrItems;

      for ( uint32 i = 0; i < nbrItems; i++ ) {
         uint32 midID, mc2ID;
         file >> midID >> mc2ID;
         
         mc2ByMid.insert( make_pair( midID, mc2ID) );
      }
      if (printOut ) {
         mc2dbg1 << "Read " << mc2ByMid.size() 
                 << " midId-mc2Ids for item type " << itemType << endl;
      }

      m_midIdsRef.insert( make_pair( itemType, mc2ByMid ) );
      
      // Try read next item type
      file >> itemType;
   }

   if (printOut ) {
      mc2dbg1 << "Read midId-mc2Ids for " << m_midIdsRef.size()
              << " item types (map " << m_map->getMapID()
              << ")" << endl;
   }

   m_midIdRefLoaded = true;
   return true;
}

uint32
GMSMidMifHandler::getMc2Id( uint32 type, uint32 midId, bool& itemTypeExist )
{
   uint32 retVal = MAX_UINT32;

   // Get all the items with correct item type
   midIdsRef_t::const_iterator mit = m_midIdsRef.find( type );
   if ( mit != m_midIdsRef.end() ) {
      itemTypeExist = true;
      mc2ByMid_t mc2ByMid = mit->second;

      // Check if there is an item with the correct mid id
      mc2ByMid_t::const_iterator it = mc2ByMid.find( midId );
      if ( it != mc2ByMid.end() ) {
         retVal = it->second;
      }
   } else {
      itemTypeExist = false;
   }

   return retVal;
}

uint32
GMSMidMifHandler::getNumberExtraAttributes(
               ifstream &midFile, bool mapSsiCoords)
{
   uint32 nbrExtraAttributes = 0;
   char inbuffer[maxMidLineLength];
   
   // Check if we have any extra attributes to read from a mid row
   // Read the rest of the mid row to a buffer, then reset the ifstream 
   // pointer so reading will continue at the correct position
   streampos filePos = midFile.tellg();
   midFile.getline(inbuffer, maxMidLineLength);
   midFile.seekg(filePos);
   
   char* inPos = inbuffer;
   if ( strlen(inPos) == 0 ) {
      inPos = NULL;
      return 0;
   }
   mc2dbg8 << "getNumberExtraAttributes '" << inPos << "'" << endl;
   int32 nbrAttributes = 0;
   if ( strstr(inPos, ",") == inPos )
      nbrAttributes = -1;
   while ( inPos != NULL ) {
      nbrAttributes++;
      inPos = strchr(inPos, ',');
      if ( inPos != NULL )
         inPos++;
   }

   // Count nbr extra item attributes (not including map ssi coord
   nbrExtraAttributes = nbrAttributes;
   if ( mapSsiCoords ) {
      nbrExtraAttributes -= 2;
   }

   return nbrExtraAttributes;
}

GMSGfxData*
GMSMidMifHandler::getConvexHullForMapGfxData()
{
   mc2dbg1 << "Calculate convex hull for map gfx data" << endl;
   GMSGfxData* returnGfx = NULL;

   // Use street segmens and points of interest
   GfxData* gfx;
   GfxDataFull* allItems = GMSGfxData::createNewGfxData(NULL, true);
   for (uint32 curZoom=0; curZoom < NUMBER_GFX_ZOOMLEVELS; curZoom++) {
      for ( uint32 itemNbr = 0;
            itemNbr < m_map->getNbrItemsWithZoom(curZoom); itemNbr++) {
         
         OldItem* item = m_map->getItem(curZoom, itemNbr);
         
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
   
   
   // Calculate the convex hull.
   Stack* stack = new Stack;
   if (allItems->getConvexHull(stack, 0)) {
      returnGfx = GMSGfxData::createNewGfxData(NULL, true);
      
      uint32 size = stack->getStackSize();
      for (uint32 i = 0; i < size; i++) {
         uint32 idx = stack->pop();
         returnGfx->addCoordinate(allItems->getLat(0,idx), 
                                  allItems->getLon(0,idx));
      }
   } else {
      // Something went wrong.
      mc2log << warn << "GMSMap::setMapGfxDataConvexHull "
             << "Could not create convex hull" << endl;
      return false;
   }
   returnGfx->setClosed(0, true);
   returnGfx->updateLength();
   
   delete stack;
   delete allItems;
   return returnGfx;
} // getConvexHullForMapGfxData

GMSGfxData*
GMSMidMifHandler::buildMapGfxFromMunicipals( bool skipNoNameMunicipals )
{
   uint32 nbrmuns = 0;
   uint32 nbrNoNameMunicipals = 0;
   uint32 nbrMunsUsed = 0;
   GMSGfxData* allMunGfx = GMSGfxData::createNewGfxData(m_map, false);
   for (uint32 z = 0; z < NUMBER_GFX_ZOOMLEVELS; z++) {
      for (uint32 i = 0; i < m_map->getNbrItemsWithZoom(z); i++) {
         OldMunicipalItem* mun = 
            dynamic_cast<OldMunicipalItem*>(m_map->getItem(z, i));
         if (mun == NULL){
            continue;
         }
         GfxData* munGfx = mun->getGfxData();
         if (munGfx==NULL) {
            mc2log << error << "No municipal gfx data for mun "
                   << mun->getID() << endl;
            MC2_ASSERT(false);
         }

         nbrmuns++;
         if ( mun->getNbrNames() < 1 ) {
            nbrNoNameMunicipals++;
            mc2dbg8 << "mun " << m_map->getMapID() << ":" << mun->getID()
                    << " has no name" << endl;
            if ( skipNoNameMunicipals ) {
               continue;
            }
         }

         //if ( ! allMunGfx->add(munGfx) ) {
         //   cout << "problem adding gfx " << mun->getID()
         //        << " to allMunGfx" << endl;
         //   MC2_ASSERT(false);
         //}

         nbrMunsUsed++;
         for (uint32 p = 0; p < munGfx->getNbrPolygons(); p++ ) {
            if ( ! allMunGfx->addPolygon(munGfx, false, p) ) {
               cout << "problem adding gfx " << mun->getID()
                    << " poly " << p
                    << " to allMunGfx" << endl;
               MC2_ASSERT(false);
            }
         }
         
         //for (uint32 p = 0; p < munGfx->getNbrPolygons(); p++ ) {
         //   for (uint32 c = 0; munGfx->getNbrCorodinates(p); c++ ) {
         //      
         //   }
         //}
   
      }
   }
   allMunGfx->setClosed(0, true);
   allMunGfx->updateLength();
   mc2dbg1 << "Map has " << nbrmuns << " municipals, " 
           << nbrNoNameMunicipals << " with no name" 
           << ", skip noname=" << int(skipNoNameMunicipals) << endl;
   mc2dbg8 << "Combined " << nbrMunsUsed << " municipals to one gfx with "
           << allMunGfx->getNbrPolygons() << " polys and "
           << allMunGfx->getTotalNbrCoordinates() << " coords" << endl;

   // Now, we have all polygons in the allMunGfx
   // Try to merge the polygons
   // See GMSPolyUtility::mergeItemPolygons
   GMSGfxData* mergedGfx = GMSGfxData::mergePolygons(allMunGfx);
   if ( mergedGfx != NULL ) {
      mc2dbg << "Merged polygons of allMunGfx"
              << " p=" << mergedGfx->getNbrPolygons()
                    << "(" << allMunGfx->getNbrPolygons() << ")"
              << " c=" << mergedGfx->getTotalNbrCoordinates() 
                    << "(" << allMunGfx->getTotalNbrCoordinates() << ")"
              << endl;
      return mergedGfx;
   }
   else {
      mc2log << warn << "Failed to merge allMunGfx" << endl;
   }
   
   return NULL;
} //buildMapGfxFromMunicipals

uint32
GMSMidMifHandler::fillDistancesForItemsFromMidMif(
            ItemTypes::itemType itemType,
            set<uint32> midmifIDs, 
            map<uint32, mapDistances_t> &distToSSIinMap)
{
   uint32 nbrDistSet = 0;

   // load midmif ref
   m_midmifRefFilename = m_map->getFilename();
   m_midmifRefFilename += MC2String(".midmif_ref");
   m_midIdRefLoaded = false;
   if ( ! readMidMifReferences(m_midmifRefFilename, true) ) {
      mc2log << warn << "Could not load midmif ref from " 
             << m_midmifRefFilename << endl;
      // try temp dir
      m_midmifRefFilename = MC2String("temp/") + m_midmifRefFilename;
      mc2dbg4 << "Trying " << m_midmifRefFilename << endl;
      if ( ! readMidMifReferences(m_midmifRefFilename, true) ) {
         mc2log << warn << "Could not load midmif ref from " 
                << m_midmifRefFilename << endl;
         // try ../temp dir
         m_midmifRefFilename = MC2String("../") + m_midmifRefFilename;
         mc2dbg4 << "Trying " << m_midmifRefFilename << endl;
         if ( ! readMidMifReferences(m_midmifRefFilename, true) ) {
            mc2log << error << "Could not load midmif ref from " 
                   << m_midmifRefFilename << endl;
            // No items added from midmif to this map
            return nbrDistSet;
            //MC2_ASSERT(false);
         }
      }
   }
   mc2dbg4 << "Loaded midmif ref for map " << m_map->getMapID() 
           << " from " << m_midmifRefFilename << endl;

   // Get the map gfxData and bounding box
   m_mapGfxData = m_map->getGfxData();
   m_mapGfxData->getMC2BoundingBox(m_mapBBox);
   mc2dbg8 << "map gfx nbr coords = " 
           << m_mapGfxData->getNbrCoordinates(0) << endl;
   if ( m_mapGfxData->getNbrCoordinates(0) == 4 ) {
      // The map gfxdata is bounding box
      // Update it temporarily to convex hull
      // to get faster insidePolygon calculations
      GMSGfxData* convHullGfx = getConvexHullForMapGfxData();
      if ( convHullGfx != NULL ) {
         mc2dbg1 << "MapGfx was bbox, temp set it to conv hull" << endl;
         m_mapGfxData = convHullGfx;
         mc2dbg8 << "map gfx nbr coords = " 
                 << m_mapGfxData->getNbrCoordinates(0) << endl;
      }
   }
   
   uint32 nbrLooped = 0;
   uint32 nbrInThisMap = 0;
   uint32 nbrInsideThisMap = 0;
   uint32 nbrWithSSiInThisMap = 0;
   bool itemTypeExist = true;
   set<ItemTypes::itemType> allowedTypes;
   allowedTypes.insert( ItemTypes::streetSegmentItem);
   uint64 dist = MAX_UINT64-1;
   map<uint32, mapDistances_t>::iterator distIt;
   mapDistances_t::iterator mapDistIt;
   for ( set<uint32>::const_iterator sit = midmifIDs.begin();
         sit != midmifIDs.end(); sit++ ) {
      nbrLooped++;
      // was this midId added to this map?
      uint32 midId = *sit;
      mc2dbg8 << "check for id " << midId << endl;
      uint32 itemID = getMc2Id(itemType, midId, itemTypeExist);
      mc2dbg8 << "  itemID=" << itemID 
              << " itemTypeExist=" << int(itemTypeExist) << endl;
      if ( ! itemTypeExist ) {
         // break for-loop
         cout << " itemType does not exist in midmifref" << endl;
         break;
      }
      if (itemID != MAX_UINT32) {
         nbrInThisMap++;
         // Look up item, check if item is inside convHull of map gfx data
         // then get distance to closest street segment item
         OldItem* item = m_map->itemLookup(itemID);
         if (item != NULL) {
            // 
            GfxData* itemGfx = item->getGfxData();
            if (itemGfx != NULL) {
               MC2Coordinate coord(itemGfx->getLat(0,0),itemGfx->getLon(0,0));
               
               // default dist is large
               dist = MAX_UINT64-1;

               // check if item inside map
               int insideMap = m_mapGfxData->insidePolygon(coord);
               if ( insideMap ) {
                  nbrInsideThisMap++;
                  uint32 closestSSIID =
                     m_map->getClosestItemID(coord, dist, allowedTypes);
                  if (closestSSIID != MAX_UINT32) {
                     OldStreetSegmentItem* ssi = 
                        dynamic_cast<OldStreetSegmentItem*>
                        (m_map->itemLookup(closestSSIID));
                     if ( ssi != NULL ) {
                        nbrWithSSiInThisMap++;
                     }
                  } 
               }
               
               // add the distance to the distToSSIinMap
               distIt = distToSSIinMap.find(midId);
               if ( distIt != distToSSIinMap.end() ) {
                  // already exist distances for some maps for this midId
                  distIt->second.insert(make_pair(m_map->getMapID(), dist));
                  nbrDistSet++;
                  // list all for debug
                  for ( mapDistIt = distIt->second.begin();
                        mapDistIt != distIt->second.end();
                        mapDistIt++ ) {
                     mc2dbg8 << "listing " << distIt->first 
                             << mapDistIt->first << ":"
                             << mapDistIt->second << endl;
                  }
               }
               else {
                  // insert first info
                  mapDistances_t mapDist;
                  mapDist.insert(make_pair(m_map->getMapID(), dist));
                  distToSSIinMap.insert(
                     make_pair (midId, mapDist) );
                  nbrDistSet++;
               }
            }
         }
      }
      if ( (nbrLooped % 5000) == 0 ) {
         cout << " f processed " << nbrLooped 
              << " nbrInThisMap=" << nbrInThisMap
              << " nbrInsideThisMap=" << nbrInsideThisMap
              << " " << m_map->getMapID() << endl;
      }
   }
   cout << " INFO map=" << m_map->getMapID() 
        << " checked " << nbrLooped << " midIds, " 
        << nbrInThisMap << " in map, "
        << nbrInsideThisMap << " inside map, "
        << "distToSSIinMap.size=" << distToSSIinMap.size()
        << endl;
   if ( ! itemTypeExist ) {
      cout << " INFO BREAK for map " << m_map->getMapID() << endl;
   }

   return nbrDistSet;
} // fillDistancesForItemsFromMidMif

uint32
GMSMidMifHandler::removeDupItemsFromMidMif(
            ItemTypes::itemType itemType,
            set<uint32> midmifIDs, 
            map<uint32, mapDistances_t> distToSSIinMap)
{
   uint32 nbrRemoved = 0;
   set<uint32> itemsToRemove;
   itemsToRemove.clear();
   
   // load midmif ref
   m_midmifRefFilename = m_map->getFilename();
   m_midmifRefFilename += MC2String(".midmif_ref");
   m_midIdRefLoaded = false;
   if ( ! readMidMifReferences(m_midmifRefFilename, true) ) {
      mc2log << warn << "Could not load midmif ref from " 
             << m_midmifRefFilename << endl;
      // try temp dir
      m_midmifRefFilename = MC2String("temp/") + m_midmifRefFilename;
      mc2dbg4 << "Trying " << m_midmifRefFilename << endl;
      if ( ! readMidMifReferences(m_midmifRefFilename, true) ) {
         mc2log << warn << "Could not load midmif ref from " 
                << m_midmifRefFilename << endl;
         // try ../temp dir
         m_midmifRefFilename = MC2String("../") + m_midmifRefFilename;
         mc2dbg4 << "Trying " << m_midmifRefFilename << endl;
         if ( ! readMidMifReferences(m_midmifRefFilename, true) ) {
            mc2log << error << "Could not load midmif ref from " 
                   << m_midmifRefFilename << endl;
            // Don't want to crash... since perhaps no items 
            // added from midmif to this map
            return nbrRemoved;
            //MC2_ASSERT(false);
         }
      }
   }
   mc2dbg4 << "Loaded midmif ref for map " << m_map->getMapID() << endl;

   uint32 nbrLooped = 0;
   uint32 nbrInThisMap = 0;
   uint32 nbrToKeep = 0;
   bool itemTypeExist = true;
   map<uint32, mapDistances_t>::iterator distIt;
   mapDistances_t::iterator mapDistIt;
   for ( set<uint32>::const_iterator sit = midmifIDs.begin();
         sit != midmifIDs.end(); sit++ ) {
      nbrLooped++;
      uint32 midId = *sit;
      mc2dbg8 << "check for id " << midId << endl;
      // was this midId added to this map?
      // 1. Check in the distToSSIinMap
      distIt = distToSSIinMap.find(midId);
      if ( distIt != distToSSIinMap.end() ) {
         // Check if in this map
         bool inThisMap = false;
         uint32 closestMap = MAX_UINT32;
         uint64 closestDist = MAX_UINT64;
         for ( mapDistIt = distIt->second.begin();
               mapDistIt != distIt->second.end();
               mapDistIt++ ) {
            if ( mapDistIt->first == m_map->getMapID() ) {
               inThisMap = true;
            }
            if ( mapDistIt->second < closestDist ) {
               closestDist = mapDistIt->second;
               closestMap = mapDistIt->first;
            }
            mc2dbg8 << "midId=" << midId
                 << " closestDist=" << closestDist
                 << " closestMap=" << closestMap 
                 << " thisMap=" << m_map->getMapID() << endl;
         }

         if ( inThisMap ) {
            nbrInThisMap++;
            if ( closestMap == m_map->getMapID() ) {
               nbrToKeep++;
            } else {
               mc2dbg8 << "Remove " << midId << " from map " 
                       << m_map->getMapID() << endl;
               // get the itemID from midmif ref
               uint32 itemID = getMc2Id(itemType, midId, itemTypeExist);
               mc2dbg8 << "  itemID=" << itemID 
                       << " itemTypeExist=" << int(itemTypeExist) << endl;
               if ( ! itemTypeExist ) {
                  // break for-loop
                  cout << " itemType does not exist in midmifref" << endl;
                  break;
               }
               if (itemID != MAX_UINT32) {
                  itemsToRemove.insert(itemID);
                  nbrRemoved++;
               }
               else {
                  mc2log << error << "Did not find mcm itemID for midId "
                         << midId << ", to be removed from map "
                         << m_map->getMapID() << endl;
               }
            }
         }
      }
      else {
         mc2log << error << "No distances stored for midId="
                << midId << endl;
         MC2_ASSERT(false);
      }

      if ( (nbrLooped % 5000) == 0 ) {
         cout << " r processed " << nbrLooped 
              << " nbrInThisMap=" << nbrInThisMap
              << " " << m_map->getMapID() << endl;
      }
   }
   cout << " INFO To remove=" << nbrRemoved 
        << "=" << itemsToRemove.size()
        << " items from map " << m_map->getMapID() 
        << " (keep " << nbrToKeep << " of "
        << nbrInThisMap << ")" << endl;
   if ( nbrRemoved != itemsToRemove.size() ) {
      mc2log << error << "Porblem with nbrRemoved" << endl;
      MC2_ASSERT(false);
   }
   
   m_map->removeItems(itemsToRemove);

   // Fixme: Improvement, remove from midmif-ref
   
   return nbrRemoved;
} // removeDupItemsFromMidMif

