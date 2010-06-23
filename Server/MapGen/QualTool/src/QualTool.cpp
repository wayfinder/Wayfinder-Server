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

#include "CommandlineOptionHandler.h"
#include "OldGenericMap.h"
#include <iomanip>

#include <cmath>

#include "OldStreetSegmentItem.h"
#include "OldPointOfInterestItem.h"
#include "GMSGfxData.h"
#include "OldNode.h"
#include "StringUtility.h"
#include "CategoryTree.h"
#include "XMLParserHelper.h"
#include "MapBits.h"


char*
stripMapOrigin(const char* mapOrigin)
{
   // If the map has a map origin, skip the origin supplier name and
   // anything after the first point in the origin map name.
   // mappsupplier_origin.mapname
   //
   char* tmp = new char[strlen(mapOrigin+1)];
   
   if ((mapOrigin != NULL) && strlen(mapOrigin) > 0) {
      // Check if the map has a maporigin
      // skip the supplier name   
      char* begin = StringUtility::strchr(mapOrigin, '_') + 1;

      uint32 i = 0;
      bool found = false;
      while ((i < strlen(begin)) && !found) {
         if (begin[i] != '.')
            tmp[i] = begin[i];
         else {
            found = true;
            tmp[i] = '\0';
         }
         i++;
      }
      if (!found) {
         tmp[0] = '\0';
      }
   } else {
      tmp[0]=0;
   }

   return tmp;
}


/** Help class, making it easier to count things by a key value.
 *  first: any integer key.
 *  second: counted value of the key.
 */ 
class CountMap : public map<uint32,uint32> {
public:
   /** Increase the count of key with one.
    */
   void increase(uint32 key){
      map<uint32,uint32>::iterator findIt = find(key);
      if (findIt == end()){
         insert(make_pair(key, 1));
      }
      else {
         findIt->second++;
      }
   };
}; // class countMap


bool
printMapQuality(CommandlineOptionHandler coh)
{
   
   map<const char*, uint32> maplinkage;
   
   // Counters to hold statistics for origin maps and totals.
   CountMap totalNbrPoisPerCat; // nubmer of POIs each category is used for.
   uint32 totalNbrItemsPerType[ItemTypes::numberOfItemTypes];
   uint32 totalNbrPerPoiType[ItemTypes::nbr_pointOfInterest];
   uint32 totalNbrItemsInMaps = 0;
   uint32 totalNbrGfxData = 0;
   uint32 totalNbrLandmarks = 0;
   uint32 totalNbrOldConnections = 0;
   uint32 totalNbrSignPosts = 0;
   uint32 totalNbrEntryRestrictions[4]; // noRest, noThrou, noEntry, noWay
   float64 totalMeterRoadsAllMaps = 0;
   float64 totalMeterRoadsUVMaps = 0;
   
   // Reset the counters
   for (uint32 t = 0; t < ItemTypes::numberOfItemTypes; t++) {
      totalNbrItemsPerType[t] = 0;
   }
   for (uint32 t = 0; t < ItemTypes::nbr_pointOfInterest; t++){
      totalNbrPerPoiType[t] = 0;
   }
   for (uint32 r = 0; r < 4; r++) {
      totalNbrEntryRestrictions[r] = 0;
   }
   
   // Initialize XML
   try { XMLPlatformUtils::Initialize(); } 
   catch(const XMLException& toCatch) {
      cerr << "Error during Xerces-c Initialization.\n"
           << "  Exception message:"
           << toCatch.getMessage() << endl;
      MC2_ASSERT(false);
   }
   // Load category file.
   CategoryTreeUtils::CategoryTree catTree;
   catTree.load("/home/is/devel/Maps/genfiles/xml/poi_category_tree.xml", "");
   


   // Loop the maps
   int nbrMaps = coh.getTailLength();
   for (int i=0; i<nbrMaps; i++) {

      const char* mcmName = coh.getTail(i);
      OldGenericMap* theMap = OldGenericMap::createMap(mcmName);

      if (theMap != NULL) {
         // Print mcm-file
         cout << "QUAL1: "
              << "=========================================================="
              << endl 
              << "QUAL1: NEWMCMMAP: true" << endl
              << "QUAL1: file: " << mcmName << endl;

         // Counters to hold the statistics for this map
         uint32 nbrItemsInMap = 0;
         uint32 nbrGfxData = 0;
         uint32 nbrItemsWithLocation = 0;
         uint32 nbrItems[NUMBER_GFX_ZOOMLEVELS];
         uint32 nbrItemsPerType[ItemTypes::numberOfItemTypes];
         uint32 nbrItemsPerZoom[NUMBER_GFX_ZOOMLEVELS]
                               [ItemTypes::numberOfItemTypes];
         uint32 nbrPois[ItemTypes::nbr_pointOfInterest];
         uint32 nbrGfxDataPerType[ItemTypes::numberOfItemTypes];
         uint32 nbrLocationPerType[ItemTypes::numberOfItemTypes];
         CountMap nbrPoisPerCat; // number of POIs each category is used for.

                // any location set.
         
         uint32 nbrLandmarks = 0;
         uint32 nbrSsiPerRC[5];     // roadclass (ssi)
         uint32 nbrVirtualRi = 0;   // 0-length segments (ri = ssi+ferry)
         uint32 nbrSsiBua = 0;      // ssi with bua location
         uint32 nbrSsiNoBua = 0;    // ssi without bua location
         uint32 nbrNodes = 0;
         uint32 nbrOldConnections = 0;
         uint32 nbrSignPosts = 0;
         uint32 nbrEntryRestrictions[4]; // noRest, noThrou, noEntry, noWay
         uint32 nbrNodesRoadToll = 0;
         uint32 nbrSsiWithHnbrs = 0;
         uint32 nbrSsiRamps = 0;
         uint32 nbrSsiRbt = 0;
         uint32 nbrSsiRbtish = 0;
         uint32 nbrSsiMulti = 0;
         uint32 nbrSsiCtrlAcc = 0;
         float64 meterRoads = 0;
         float64 meterRoadsPerRC[5];     // roadclass (ssi)

         uint32 nbrNTPoi = 0;
         uint32 nbrTAPoi = 0;
         uint32 nbrPARPoi = 0;
         uint32 nbrOtherPoi = 0;
         
         
         // Reset the counters
         for(uint32 i = 0; i < NUMBER_GFX_ZOOMLEVELS; i++ ) {
            nbrItems[i] = 0;
            for (uint32 t = 0; t < ItemTypes::numberOfItemTypes; t++) {
               nbrItemsPerZoom[i][t] = 0;
            }
         }
         for (uint32 t = 0; t < ItemTypes::numberOfItemTypes; t++) {
            nbrItemsPerType[t] = 0;
            nbrGfxDataPerType[t] = 0;
            nbrLocationPerType[t] = 0;
         }
         for (uint32 n = 0; n < ItemTypes::nbr_pointOfInterest; n++) {
            nbrPois[n] = 0;
         }
         for (uint32 rc = 0; rc < 5; rc++) {
            nbrSsiPerRC[rc] = 0;
         }
         for (uint32 r = 0; r < 4; r++) {
            nbrEntryRestrictions[r] = 0;
         }
         for (uint32 rc = 0; rc < 5; rc++) {
            meterRoadsPerRC[rc] = 0;
         }

         // get the map name. Make sure country maps start with country_
         // in order to seperate them from the overview maps.
         string tmpMapName = string( "" );
         
         if ( MapBits::isCountryMap( theMap->getMapID() ) ) {
            tmpMapName = string( "country_" );
         }
         tmpMapName += string( theMap->getMapName() );
         const char* mapName = tmpMapName.c_str();
         
         // get the map origin
         const char* mapOrigin = stripMapOrigin(theMap->getMapOrigin());
         cout << "QUAL1: MAPNAME: " << mapName << " maporigin: " 
              << mapOrigin 
              << " (" << theMap->getMapOrigin() << ")" << endl;


         // Count items etc.
         for (uint32 z=0; z<NUMBER_GFX_ZOOMLEVELS; z++) {
            for (uint32 i=0; i<theMap->getNbrItemsWithZoom(z); i++) {
               OldItem* item = theMap->getItem(z, i);
               if ( (item != NULL) ) {
                  nbrItems[z]++;
                  ItemTypes::itemType type = item->getItemType();
                  ++nbrItemsPerZoom[z][type];

                  if (item->getGfxData() != NULL) {
                     ++nbrGfxData;
                     nbrGfxDataPerType[type]++;
                  }
                  uint32 nbrLoc = 
                     theMap->getNbrRegions( item, 
                           ItemTypes::builtUpAreaItem ) +
                     theMap->getNbrRegions( item, 
                           ItemTypes::municipalItem );
                  if ( nbrLoc != 0 ) {
                     nbrItemsWithLocation++;
                     nbrLocationPerType[type]++;
                  }

                  // Routeable
                  if ((type == ItemTypes::streetSegmentItem) /*||
                      (type == ItemTypes::ferryItem)*/) {
                     OldRouteableItem* ri = static_cast<OldRouteableItem*>(item);
                     
                     for (uint32 n = 0; n < 2; n++) {
                        nbrNodes++;
                        OldNode* node = ri->getNode(n);
                        nbrEntryRestrictions[node->getEntryRestrictions()]++;
                        if ( node->hasRoadToll() ) {
                           nbrNodesRoadToll++;
                        }
                        for (uint32 c = 0; c < node->getNbrConnections(); c++) {
                           nbrOldConnections++;
                           OldConnection* conn = node->getEntryConnection(c);
                           nbrSignPosts += 
                              node->getNbrSignPosts(*theMap, 
                                                    conn->getFromNode());
                        }
                     }

                     if (ri->getGfxData()->getLength(0) == 0 ) {
                        nbrVirtualRi++;
                     }
                     if (type == ItemTypes::streetSegmentItem) {
                        nbrSsiPerRC[ri->getRoadClass()]++;
                        if (theMap->getRegionID( ri, 
                                 ItemTypes::builtUpAreaItem) != MAX_UINT32)
                           nbrSsiBua++;
                        else
                           nbrSsiNoBua++;
                        
                        // hnbrs
                        OldStreetSegmentItem* ssi = 
                            static_cast<OldStreetSegmentItem*>(item);
                        if ( ( ssi->getLeftSideNbrStart() != 0 ) ||
                             ( ssi->getLeftSideNbrEnd() != 0 ) ||
                             ( ssi->getRightSideNbrStart() != 0 ) ||
                             ( ssi->getRightSideNbrEnd() != 0 ) ) {
                           nbrSsiWithHnbrs++;
                        }

                        if ( ssi->isRamp() ) { nbrSsiRamps++; }
                        if ( ssi->isRoundabout() ) { nbrSsiRbt++; }
                        if ( ssi->isRoundaboutish() ) { nbrSsiRbtish++; }
                        if ( ssi->isMultiDigitised() ) { nbrSsiMulti++; }
                        if ( ssi->isControlledAccess() ) { nbrSsiCtrlAcc++; }

                        if ( ssi->getGfxData() != NULL ) {
                           meterRoads += ssi->getGfxData()->getLength(0);
                           meterRoadsPerRC[ri->getRoadClass()] +=
                                    ssi->getGfxData()->getLength(0);
                        }
                     }
                  }

                  if (type == ItemTypes::pointOfInterestItem) {
                     OldPointOfInterestItem* poi = 
                        static_cast<OldPointOfInterestItem*>(item);
                     
                     // Count POIs by type
                     ItemTypes::pointOfInterest_t type = 
                        poi->getPointOfInterestType();
                     totalNbrPerPoiType[type]++;
                     nbrPois[type]++;
                     
                     // Count POis by category
                     set<uint16> categories = poi->getCategories(*theMap);
                     for ( set<uint16>::const_iterator catIt = 
                              categories.begin(); catIt != categories.end();
                           ++catIt ){
                        nbrPoisPerCat.increase(*catIt);
                        totalNbrPoisPerCat.increase(*catIt);
                     }



                     // Check the source of the pois
                     if (poi->getSource() == SearchTypes::NAV_TECH)
                        nbrNTPoi++;
                     else if (poi->getSource() == SearchTypes::TELE_ATLAS)
                        nbrTAPoi++;
                     else if (poi->getSource() == SearchTypes::PAR)
                        nbrPARPoi++;
                     else
                        nbrOtherPoi++;
                  }
                  
               }
            }
            nbrItemsInMap += nbrItems[z];
            for (uint t = 0; t < ItemTypes::numberOfItemTypes; t++) {
               nbrItemsPerType[t] += nbrItemsPerZoom[z][t];
               totalNbrItemsPerType[t] += nbrItemsPerZoom[z][t];
            }
         }

         //count Landmarks
         nbrLandmarks = theMap->getLandmarkTable().size();
         
         // sum up some statstics
         totalNbrItemsInMaps += nbrItemsInMap;
         totalNbrGfxData += nbrGfxData;
         totalNbrLandmarks += nbrLandmarks;
         totalNbrOldConnections += nbrOldConnections;
         totalNbrSignPosts += nbrSignPosts;
         for (uint32 r = 0; r < 4; r++) {
            totalNbrEntryRestrictions[r] += nbrEntryRestrictions[r];
         }
         if ( MapBits::isUnderviewMap(theMap->getMapID()) ) {
            totalMeterRoadsUVMaps += meterRoads;
         }
         totalMeterRoadsAllMaps += meterRoads;

         // Print the statistics for this map to standard out
         //
         // MCM totals
         cout << "QUAL1: " << mapName 
              << ": MCM TOTALS " << 3 //number of MCM TOTALS
              << "   -  -  -  -  -  -  -  -  -  -  -" << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Total number of items in the map: " 
              << setw(12) << nbrItemsInMap << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Number of items with location: " 
              << setw(15) << nbrItemsWithLocation << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Total number of gfx data in the map: "
              << setw(9) << nbrGfxData << endl;

         
         // Number items, gfx data, location(any)
         cout << "QUAL1: " << mapName << ": ITEMS " 
              << (int(ItemTypes::numberOfItemTypes) ) * 3  // nbr ITEMS
              << " -  -  -  -  -  -  -  -  -  -  -  -  -" << endl;
         for (uint32 t = 0; t < ItemTypes::numberOfItemTypes; t++) {
            cout << "QUAL1: " << mapName << ": "
                 << setiosflags(ios::left) << setw(30)
                 << StringTable::getString(
                        ItemTypes::getItemTypeSC(ItemTypes::itemType(t)), 
                        StringTable::ENGLISH)
                 << resetiosflags(ios::left)
                 << " (it " << setw(2) << t << "): " 
                 << setw(6) << nbrItemsPerType[t] << endl;
            cout << "QUAL2: " << mapName << ": "
                 << setiosflags(ios::left) << setw(30)
                 << "  nbr gfx data"
                 << resetiosflags(ios::left)
                 << " (it " << setw(2) << t << "): " 
                 << setw(6) << nbrGfxDataPerType[t] << endl;
            cout << "QUAL2: " << mapName << ": "
                 << setiosflags(ios::left) << setw(30)
                 << "  nbr items with location"
                 << resetiosflags(ios::left)
                 << " (it " << setw(2) << t << "): " 
                 << setw(6) << nbrLocationPerType[t] << endl;
         }
         
         // Attributes
         cout << "QUAL1: " << mapName 
              << ": ATTRIBUTES ETC "
              // nbr ATTRIBUTES:  5rc 7misc 4entryrestr 7misc 5km-rc 1tollroad
              << (5 + 7 + 4 + 7 + 5 + 1)
              << " -  -  -  -  -  -  -  -  -  -" << endl;
         for (uint32 rc = 0; rc < 5; rc++) {
            cout << "QUAL1: " << mapName << ": "
                 << "Nbr ssi roadClass " << rc << ": "
                 << setw(12) << nbrSsiPerRC[rc] << endl;
         }
         cout << "QUAL1: " << mapName << ": "
              << "Nbr virtual ri: " << setw(17) << nbrVirtualRi << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Nbr ssi in bua: " << setw(17) << nbrSsiBua << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Nbr ssi in no bua: " << setw(14) << nbrSsiNoBua << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Nbr nodes (ri): " << setw(17) << nbrNodes << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Nbr connections (ri): " << setw(11) << nbrOldConnections << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Nbr landmarks: " << setw(18) << nbrLandmarks << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Nbr signposts: " << setw(18) << nbrSignPosts << endl;
         for (uint32 r = 0; r < 4; r++) {
            cout << "QUAL1: " << mapName << ": "
                 << "Nbr entry restrictions " << r << ": " 
                 << setw(7) << nbrEntryRestrictions[r] << endl;
         }
         cout << "QUAL1: " << mapName << ": "
              << "Nbr ssi with hnbrs: " << setw(13) << nbrSsiWithHnbrs << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Nbr ssi ramps: " << setw(18) << nbrSsiRamps << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Nbr ssi rbt: " << setw(20) << nbrSsiRbt << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Nbr ssi rbtish: " << setw(17) << nbrSsiRbtish << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Nbr ssi multi: " << setw(18) << nbrSsiMulti << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Nbr ssi ctrl acc: " << setw(15) << nbrSsiCtrlAcc << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Kilometer roads: " << setw(16) 
              << int (meterRoads / 1000.0) << endl;
         for (uint32 rc = 0; rc < 5; rc++) {
            cout << "QUAL1: " << mapName << ": "
                 << "Km roads roadClass " << rc << ": "
                 << setw(11) << int (meterRoadsPerRC[rc] / 1000.0) << endl;
         }
         cout << "QUAL1: " << mapName << ": "
              << "Nbr tollRoad nodes: " << setw(13) << nbrNodesRoadToll << endl;

         // if adding more count-prints also increase the nbr ATTRIBUTES ETC
         // else comparison will not work
         
         
         // Items per zoomlevel    
         cout << "QUAL4: " << mapName 
              << ": ZOOMLEVEL ------------------------------------" << endl;
         for(uint32 i = 0; i < NUMBER_GFX_ZOOMLEVELS; i++ ) {
            if ( nbrItems[i] != 0 ) {
               cout << "QUAL4: " << mapName << ": "
                    << "Zoomlevel [" << i << "], nbr items = " << nbrItems[i]
                    << endl;
            }
            for (uint32 t = 0; t < ItemTypes::numberOfItemTypes; t++){
               if (nbrItemsPerZoom[i][t] != 0) {
                  cout << "QUAL4: " << mapName << ": "
                       << "   " << StringTable::getString(
                              ItemTypes::getItemTypeSC(ItemTypes::itemType(t)), 
                              StringTable::ENGLISH)
                       << ", nbrItems = " << nbrItemsPerZoom[i][t] << endl;
               }
            }
         }

         // Number different pois
         cout << "QUAL1: " << mapName 
              << ": POINTOFINTERESTS " << 4 // nbr poi totals
              << " ---------------------------" << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Nbr POIs from PAR:       " << setw(6) << nbrPARPoi << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Nbr POIs from NavTech:   " << setw(6) << nbrNTPoi << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Nbr POIs from others:    " << setw(6) << nbrOtherPoi << endl;
         cout << "QUAL1: " << mapName << ": "
              << "Nbr POIs from TeleAtlas: " << setw(6) << nbrTAPoi << endl;
         
         cout << "QUAL1: " << mapName << ": POI TYPES " 
              << int(ItemTypes::nbr_pointOfInterest) // nbr poi types
              << "   -  -  -  -  -  -  -  -  -  -  -" << endl;
         for (uint32 n = 0; n < ItemTypes::nbr_pointOfInterest; n++) {
            cout << "QUAL1: " << mapName << ": "
                 << setiosflags(ios::left) << setw(31)
                 << StringUtility::copyUpper(
                    StringTable::getString(ItemTypes::getPOIStringCode(
                                           ItemTypes::pointOfInterest_t (n)),
                                           StringTable::ENGLISH))
                 << resetiosflags(ios::left)
                 << " (pt " << setw(2) << n << "): " 
                 << setw(5) << nbrPois[n] << endl;
         }
         for (map<uint32, uint32>::const_iterator catIt = 
                 nbrPoisPerCat.begin(); catIt != nbrPoisPerCat.end(); 
              ++catIt ){
            uint16 catID = catIt->first;
            uint32 count = catIt->second;
            MC2String catName = 
               catTree.getTranslation( catID, LangTypes::english);
            cout << "QUAL1: " << mapName << ": "
                 << setiosflags(ios::left) << setw(32)
                 << catName
                 << resetiosflags(ios::left)
                 << " (cat " << setw(3) << catID << "): " 
                 << setw(5) << count << endl;  
         }

         // Print the finishing line, needed in compareMapQuality.
         // It stops when mapname can't be read any more between the ':'
         cout << "QUAL1: " << "endofmap" << ": " << endl;
   
      } else {
         MC2WARNING("Failed to load map");
      }

      delete theMap;

   }
   // no more mcm maps to check
   cout << "QUAL1: "
        << "==========================================================" << endl
        << "QUAL1: NEWMCMMAP: false" << endl;


   // Print the total statistics
   cout << "QUAL1: TOTALS " 
        // nbr TOTALS 3misc itemTypes poiTypes 8misc
        << (3 + ItemTypes::numberOfItemTypes +
            ItemTypes::nbr_pointOfInterest + 8)
        << ": Total statistics for " << setw(2) << nbrMaps
        << " maps =================="
        << endl;

   cout << "QUAL1: TOTALS: " << "Total number of items:      " 
        << setw(19) << totalNbrItemsInMaps << endl;
   cout << "QUAL1: TOTALS: " << "Total number of gfx data's: " 
        << setw(19) << totalNbrGfxData << endl;
   cout << "QUAL1: TOTALS: " << "Total number of landmarks:  " 
        << setw(19) << totalNbrLandmarks << endl;

   for (uint32 t = 0; t < ItemTypes::numberOfItemTypes; t++) {
      cout << "QUAL1: TOTALS: " 
           << setiosflags(ios::left) << setw(30)
           << StringTable::getString(
                  ItemTypes::getItemTypeSC(ItemTypes::itemType(t)), 
                  StringTable::ENGLISH)
           << resetiosflags(ios::left)
           << " (it " << setw(2) << t << "): " 
           << setw(7) << totalNbrItemsPerType[t] << endl;
   }
   for (uint32 t = 0; t < ItemTypes::nbr_pointOfInterest; t++) {
      cout << "QUAL1: TOTALS: " 
           << setiosflags(ios::left) << setw(30)
           << StringUtility::copyUpper(StringTable::getString
                                       (ItemTypes::getPOIStringCode
                                        (ItemTypes::pointOfInterest_t(t)), 
                                        StringTable::ENGLISH ))
           << resetiosflags(ios::left)
           << " (pt " << setw(3) << t << "): " 
           << setw(7) << totalNbrPerPoiType[t] << endl;
   }
   for (map<uint32, uint32>::const_iterator catIt = 
           totalNbrPoisPerCat.begin(); catIt != totalNbrPoisPerCat.end(); 
        ++catIt ){
      uint16 catID = catIt->first;
      uint32 count = catIt->second;
      MC2String catName = 
         catTree.getTranslation( catID, LangTypes::english);
      cout << "QUAL1: TOTALS: "
           << setiosflags(ios::left) << setw(32)
           << catName
           << resetiosflags(ios::left)
           << " (cat " << setw(3) << catID << "): " 
           << setw(5) << count << endl;  
   }


   cout << "QUAL1: TOTALS: " << "Total number of connections (ri):  " 
        << setw(12) << totalNbrOldConnections << endl;
   cout << "QUAL1: TOTALS: " << "Total number of signposts:  " 
        << setw(19) << totalNbrSignPosts << endl;
   for (uint32 r = 0; r < 4; r++) {
      cout << "QUAL1: TOTALS: " << "Total number of entry restrictions " 
           << r <<":  " << setw(8) << totalNbrEntryRestrictions[r] << endl;
   }
   cout << "QUAL1: TOTALS: " << "Total kilometer roads in all maps:  " 
        << setw(11) << int (totalMeterRoadsAllMaps / 1000.0) << endl;
   cout << "QUAL1: TOTALS: " << "Total kilometer roads in underviews:  " 
        << setw(9) << int (totalMeterRoadsUVMaps / 1000.0) << endl;
   // if adding more count-prints also increase the nbr TOTALS
   // else comparison will not work


   // Shut down the XML system
   XMLPlatformUtils::Terminate();         


   return true;
}

bool
readPastKey(ifstream &file, uint32 pos, const char* key)
{
   // Read from one row in file until pos, and check if it contains key.
   // Pos is the key's place in the row record (separated by ':'),
   // e.g. in the row "QUAL1: TOTALS:" the key "TOTALS" is in the 2nd pos
   
   bool retVal = false;
   uint32 MAXLENGTH = 255;
   char* buf = new char[MAXLENGTH];

   for (uint32 k = 0; k < pos; k++) {
      file.getline(buf, MAXLENGTH, ':');
   }
   // check if key exists in pos
   if (strstr(buf, key))
      retVal = true;

   delete buf;
   return retVal;
}

char*
findMapName(ifstream &file)
{
   // Read from file until the key MAPNAME is found, then 
   // read and return the map name that follows the key.
   
   uint32 MAXLENGTH = 255;
   char* buf = new char[MAXLENGTH];
   char* mapname = new char[MAXLENGTH];

   file.getline(buf, MAXLENGTH, ':');
   while (!file.eof() && (strstr(buf, "MAPNAME") == NULL)) {
      file.getline(buf, MAXLENGTH, ':');
   }
   if (!file.eof()) {
      file >> mapname;
      file.getline(buf, MAXLENGTH);
   } else {
      mc2log << fatal << "Did not find MAPNAME in quality file" << endl;
      exit (1);
   }

   delete buf;
   return mapname;
}

uint32
getOriginalNbrRecords(const char* string)
{
   // Original means the number of statistics records in the
   // first version of QualTool
   // To enable some version handling..
   
   uint32 retVal = 0;
   if (StringUtility::strcmp(string, "MCM TOTALS") == 0)
      return 3;
   else if (StringUtility::strcmp(string, "ITEMS") == 0)
      return 75;
   else if (StringUtility::strcmp(string, "ATTRIBUTES ETC") == 0)
      return 11;
   else if (StringUtility::strcmp(string, "POINTOFINTERESTS") == 0)
      return 3;
   else if (StringUtility::strcmp(string, "POI TYPES") == 0)
      return 63;
   else if (StringUtility::strcmp(string, "TOTALS") == 0)
      return 28;
   
   return retVal;
}

void
goToLineWithoutString(ifstream &file, const char* string ){

   // Read rows from the file until one row does not  contain "string"

   uint32 MAXLENGTH = 255;
   char* buf = new char[MAXLENGTH];
   
   bool cont = true;
   while ( !file.eof() && cont ) {
      int filePos = file.tellg();
      file.getline(buf, MAXLENGTH);
      if (strstr(buf, string)) {
         file.seekg( filePos ); // Reset file.
         cont = false;
      }
   }
   delete[] buf;
}


uint32
findLineWithString(ifstream &file, const char* string)
{
   // Read rows from the file until one row contains "string"

   uint32 MAXLENGTH = 255;
   char* buf = new char[MAXLENGTH];
   uint32 retVal = 0;
   bool cont = true;
   while ( !file.eof() && cont ) {
      file.getline(buf, MAXLENGTH);
      if (strstr(buf, string)) {
         cont = false;
         
         // find the number statistics records in this "section"
         char* tmp = strstr(buf, string);
         char* tmp2 = &tmp[strlen(string)];
         char* dest;
         uint32 nbr2 = strtoul(tmp2, &dest, 10);
         if ( dest != tmp2 ) {
            retVal = nbr2;
         } else {
            // no number was given in the qual file, use original
            retVal = getOriginalNbrRecords(string);
         }
      }
   }
   delete[] buf;
   return retVal;
}

bool
moreMCMMaps(ifstream &file)
{
   // Read the file until the key NEWMCMMAP is found,
   // then find out if there is another map in the quality 
   // statistics file (true) or not (false).
   
   bool retVal = false;

   uint32 MAXLENGTH = 255;
   char* buf = new char[MAXLENGTH];
   bool cont = true;
   while ( !file.eof() && cont ) {
      file.getline(buf, MAXLENGTH);
      if (strstr(buf, "NEWMCMMAP")) {
         cont = false;
         if (strstr(buf, "true"))
            retVal = true;
         else if (strstr(buf, "false"))
            retVal = false;
      }
   }
   if (file.eof())
      mc2log << warn << "Reached end of file!!" << endl;

   return retVal;
}


void
printQualDiff( const char* mapName, const char* diffDesc, 
               uint32 value1, uint32 value2)
{
   const uint32 textfill = 40;
   const uint32 nbrfill = 9;

   int32 diff = value2 - value1;
   int32 diffPercent = MAX_INT32;
   if ( value1 != 0 ){
      diffPercent = static_cast<int32>(rint(100 * abs(diff) / value1));
   }
   const char* sign;
   if ( diff < 0 ){
      sign = " -";
   }
   else if ( diff > 0 ){
      sign = " +";
   }
   else {
      sign = "  ";
   }

   char* diffPercentStr = new char[4096];
   if ( diffPercent != MAX_INT32 ){
      sprintf( diffPercentStr, "%i", diffPercent );
   }
   else{
      sprintf( diffPercentStr, "x");
   }

   cout << "DIFF MCM MAP: " << mapName << ":"
        << setiosflags(ios::left)
        << setw(textfill) << diffDesc
        << resetiosflags(ios::left)
        << setw(nbrfill) << value1 << setw(nbrfill) << value2 
        << sign << setw(nbrfill) << abs(diff)
        << sign << setw(nbrfill) << diffPercentStr
        << endl;

}

bool
compareMapQuality(ifstream &map1, ifstream &map2)
{
   uint32 MAXLENGTH = 255;
   char* buf1 = new char[MAXLENGTH];
   char* buf2 = new char[MAXLENGTH];
   uint32 textfill = 40;
   uint32 nbrfill = 9;
   uint32 signfill = 2;
   uint32 nbr1, nbr2;

   // Compare statistics, mcm map vs. mcm map.
   
   // Loop while there are more mcm maps to read from the files
   bool moreMCMMaps1 = moreMCMMaps(map1);
   bool moreMCMMaps2 = moreMCMMaps(map2);

   while (moreMCMMaps1 && moreMCMMaps2) {
    
      // find map name
      const char* mapname1 = findMapName(map1);
      const char* mapname2 = findMapName(map2);
      mc2dbg8 << "MapNames: " << mapname1 << ", " << mapname2 << endl;
      if (StringUtility::strcmp(mapname1, mapname2) != 0) {
         const char* origMapName2 = mapname2; // Used for looking in map1 
         int origMapPos2 = map2.tellg();      // later

         // Go forward in map2 until map name from map1 is found.
         vector<const char*> noCmpMaps2;
         while ( ( ! map2.eof() ) && 
                 (StringUtility::strcmp(mapname1, mapname2) != 0) ){
            mc2dbg8 << "mapname1: " << mapname1 << ", cmpval: " 
                   << StringUtility::strcmp(mapname1, mapname2) << endl;
            goToLineWithoutString( map2, mapname2 );
            noCmpMaps2.push_back( mapname2 );
            mc2dbg8 << "Non-matching map from 2: " << mapname2 << endl;
            if ( moreMCMMaps(map2) ){
               mapname2 = findMapName(map2);
            }
         }
         if ( StringUtility::strcmp(mapname1, mapname2) == 0 ) {
              // Write names of maps not compared.
            cout << "Not comparing following maps from map2 file." 
                 << endl;
            for ( uint32 i=0; i<noCmpMaps2.size(); i++){
               cout << "   " << noCmpMaps2[i] << endl;
            }
         }
         else {
            // Perhaps more maps in maps1 than maps2.
            mapname2 = origMapName2;
            //map2.seekg(origMapPos2);
            map2.clear();
            mc2dbg8 << "pos1: " << map2.tellg() <<endl;
            map2.seekg(origMapPos2, ios::beg);
            mc2dbg8 << "pos2: " << origMapPos2 <<endl;
            mc2dbg8 << "tellg: " << map2.tellg() <<endl;

            // Go forward in map1 until map name from map2 is found.
            vector<const char*> noCmpMaps1;
            while ( ( ! map1.eof() ) && 
                    (StringUtility::strcmp(mapname1, mapname2 ) != 0) ){
               goToLineWithoutString( map1, mapname1 );
               noCmpMaps1.push_back( mapname1 );
               mc2dbg8 << "Non-matching map from 1: " << mapname1 << endl;
               if ( moreMCMMaps(map1) ){
                  mapname1 = findMapName(map1);
               }
            }
            if ( StringUtility::strcmp(mapname1, mapname2) == 0 ) {
               // Write names of maps not compared.
               cout << "Not comparing following maps from map1 file."
                    << endl;
               for ( uint32 i=0; i<noCmpMaps1.size(); i++){
                  cout << "   " << noCmpMaps1[i] << endl;
               }
            }
            else {
               mc2log << error << "Not the same maps!" << endl
                      << "map1=" << mapname1 << " map2=" << mapname2 
                      << endl;
               return false;
            }
         }
      }
     
      cout << "DIFF MCM MAP: " << mapname1 << ": "
           << setw(textfill-1) << setfill('-') << " " 
           << setfill(' ')
           << setw(nbrfill) << "map1" << setw(nbrfill) << "map2" 
           << setw(nbrfill+signfill) << "diff" 
           << setw(nbrfill+signfill) << "diff(%)" 
           << endl;
   
     
      // MCM TOTALS
      uint32 nbrRec1 = findLineWithString(map1, "MCM TOTALS");
      uint32 nbrRec2 = findLineWithString(map2, "MCM TOTALS");
      uint32 rows = 0;
      bool cont1 = readPastKey(map1, 2, mapname1);
      bool cont2 = readPastKey(map2, 2, mapname2);
      while ( cont1 && cont2 && (rows < nbrRec1) && (rows < nbrRec2)) {
         map1.getline(buf1, MAXLENGTH, ':');
         map2.getline(buf2, MAXLENGTH, ':');
         map1 >> nbr1;
         map2 >> nbr2;
         if (nbr1 != nbr2) {
            // The files differ, print
            printQualDiff( mapname1, buf1, nbr1, nbr2);
         }
         rows++;
         if (rows < nbrRec1)
            cont1 = readPastKey(map1, 2, mapname1);
         if (rows < nbrRec2)
            cont2 = readPastKey(map2, 2, mapname2);
      }
     
      // ITEMS
      nbrRec1 = findLineWithString(map1, "ITEMS");
      nbrRec2 = findLineWithString(map2, "ITEMS");
      rows = 0;
      cont1 = readPastKey(map1, 2, mapname1);
      cont2 = readPastKey(map2, 2, mapname2);
      while ( cont1 && cont2 && (rows < nbrRec1) && (rows < nbrRec2)) {
         map1.getline(buf1, MAXLENGTH, ':');
         map2.getline(buf2, MAXLENGTH, ':');
         map1 >> nbr1;
         map2 >> nbr2;
         if (nbr1 != nbr2) {
            // The files differ, print
            printQualDiff( mapname1, buf1, nbr1, nbr2);
         }
         rows++;
         if (rows < nbrRec1)
            cont1 = readPastKey(map1, 2, mapname1);
         if (rows < nbrRec2)
            cont2 = readPastKey(map2, 2, mapname2);
      }
      
      // ATTRIBUTES ETC
      nbrRec1 = findLineWithString(map1, "ATTRIBUTES ETC");
      nbrRec2 = findLineWithString(map2, "ATTRIBUTES ETC");
      rows = 0;
      cont1 = readPastKey(map1, 2, mapname1);
      cont2 = readPastKey(map2, 2, mapname2);
      while ( cont1 && cont2 && (rows < nbrRec1) && (rows < nbrRec2)) {
         map1.getline(buf1, MAXLENGTH, ':');
         map2.getline(buf2, MAXLENGTH, ':');
         map1 >> nbr1;
         map2 >> nbr2;
         if (nbr1 != nbr2) {
            // The files differ, print
            printQualDiff( mapname1, buf1, nbr1, nbr2);
         }
         rows++;
         if (rows < nbrRec1)
            cont1 = readPastKey(map1, 2, mapname1);
         if (rows < nbrRec2)
            cont2 = readPastKey(map2, 2, mapname2);
      }
      
      // ZOOMLEVELS
      // skip
      
      // POINTOFINTERESTS
      nbrRec1 = findLineWithString(map1, "POINTOFINTERESTS");
      nbrRec2 = findLineWithString(map2, "POINTOFINTERESTS");
      rows = 0;
      cont1 = readPastKey(map1, 2, mapname1);
      cont2 = readPastKey(map2, 2, mapname2);
      while ( cont1 && cont2 && (rows < nbrRec1) && (rows < nbrRec2)) {
         // nbr poi types + 3 for general poi info
         map1.getline(buf1, MAXLENGTH, ':');
         map2.getline(buf2, MAXLENGTH, ':');
         map1 >> nbr1;
         map2 >> nbr2;
         if (nbr1 != nbr2) {
            // The files differ, print
            printQualDiff( mapname1, buf1, nbr1, nbr2);
         }
         rows++;
         if (rows < nbrRec1)
            cont1 = readPastKey(map1, 2, mapname1);
         if (rows < nbrRec2)
            cont2 = readPastKey(map2, 2, mapname2);
      }

      // POI TYPES
      if (nbrRec1 != getOriginalNbrRecords("POINTOFINTERESTS"))
         nbrRec1 = findLineWithString(map1, "POI TYPES");
      else 
         nbrRec1 = getOriginalNbrRecords("POI TYPES");
      if (nbrRec2 != getOriginalNbrRecords("POINTOFINTERESTS"))
         nbrRec2 = findLineWithString(map2, "POI TYPES");
      else 
         nbrRec2 = getOriginalNbrRecords("POI TYPES");
      rows = 0;
      cont1 = readPastKey(map1, 2, mapname1);
      cont2 = readPastKey(map2, 2, mapname2);
      while ( cont1 && cont2 && (rows < nbrRec1) && (rows < nbrRec2)) {
         // nbr poi types + 3 for general poi info
         map1.getline(buf1, MAXLENGTH, ':');
         map2.getline(buf2, MAXLENGTH, ':');
         map1 >> nbr1;
         map2 >> nbr2;
         if (nbr1 != nbr2) {
            // The files differ, print
            printQualDiff( mapname1, buf1, nbr1, nbr2);
         }
         rows++;
         if (rows < nbrRec1)
            cont1 = readPastKey(map1, 2, mapname1);
         if (rows < nbrRec2)
            cont2 = readPastKey(map2, 2, mapname2);
      }

      
      cout << "DIFF MCM MAP: " << mapname1 << ": "
           << setw(textfill-1) << " "
           << setw(nbrfill) << "-" << setw(nbrfill) << "-" 
           << setw(nbrfill+signfill) << "-" 
           << setw(nbrfill+signfill) << "-" << endl;
      
      delete mapname1;
      delete mapname2;

      moreMCMMaps1 = moreMCMMaps(map1);
      moreMCMMaps2 = moreMCMMaps(map2);
   }

   // if one file still has more mcm maps
   if (moreMCMMaps1) {
      mc2log << error << "Quality file #1 still has more mcm maps, "
                      << "but file #2 has not." << endl;
      return false;
   }
   else if (moreMCMMaps2) {
      mc2log << error << "Quality file #2 still has more mcm maps, "
                      << "but file #1 has not." << endl;
      return false;
   }


   // TOTALS
   // Compare the total statistics in both files.
   cout << "DIFF TOTALS: " << setw(textfill-1) << setfill('-') << " " 
        << setfill(' ')
        << setw(nbrfill) << "map1" << setw(nbrfill) << "map2" << endl;
   
   uint32 nbrRec1 = findLineWithString(map1, "TOTALS");
   uint32 nbrRec2 = findLineWithString(map2, "TOTALS");
   uint32 rows = 0;
   bool cont1 = readPastKey(map1, 2, "TOTALS");
   bool cont2 = readPastKey(map2, 2, "TOTALS");
   while ( cont1 && cont2 && (rows < nbrRec1) && (rows < nbrRec2)) {
      map1.getline(buf1, MAXLENGTH, ':');
      map2.getline(buf2, MAXLENGTH, ':');
      map1 >> nbr1;
      map2 >> nbr2;
      if (nbr1 != nbr2) {
         // The files differ, print
         cout << "DIFF TOTALS:" 
              << setiosflags(ios::left)
              << setw(textfill) << buf1 
              << resetiosflags(ios::left)
              << setw(nbrfill) << nbr1 << setw(nbrfill) << nbr2 << endl;
      }
      rows++;
      if (rows < nbrRec1)
         cont1 = readPastKey(map1, 2, "TOTALS");
      if (rows < nbrRec2)
         cont2 = readPastKey(map2, 2, "TOTALS");
   }
   map1.getline(buf1, MAXLENGTH);
   map2.getline(buf2, MAXLENGTH);
   cout << "DIFF TOTALS: " 
        << setw(textfill-1) << " "
        << setw(nbrfill) << "-" << setw(nbrfill) << "-" << endl;
   

   // ORIGIN
   // skip
   
   delete buf1;
   delete buf2;
   return true;
}



int main(int argc, char* argv[])
{

   CommandlineOptionHandler coh(argc, argv, 0);
   coh.setTailHelp("mcm-file(s) | text-files");
   coh.setSummary("Generates quality statistics for maps.\n"
                  "Also compares such statistics from two map generations "
                  "to find differences.");

   // --------- print Quality
   bool CL_printMapQuality = false;
   coh.addOption("-M", "--printMapQuality",
                 CommandlineOptionHandler::presentVal,
                 1, &CL_printMapQuality, "F",
                 "print qualty statistics for mcm maps given in tail.");
   
   // --------- compare map quality, file 1
   char* CL_compareMaps1 = NULL;
   coh.addOption("-c", "--compareMaps1",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_compareMaps1, "\0",
                 "One of two quality files (generated with "
                 "--printMapQualty), when comparing the quality "
                 "statistics for two sets of mcm maps.\n"
                 "It is important that the two sets consist of the "
                 "same map areas.");
   
   // --------- compare map quality, file 2
   char* CL_compareMaps2 = NULL;
   coh.addOption("-C", "--compareMaps2",
                 CommandlineOptionHandler::stringVal,
                 1, &CL_compareMaps2, "\0",
                 "The second of two quality files (generated with "
                 "--printMapQualty), when comparing the quality "
                 "statistics for two sets of mcm maps.\n"
                 "It is important that the two sets consist of the "
                 "same map areas.");

   
   if (!coh.parse()) {
      cerr << argv[0] << ": Error on commandline, (-h for help)" << endl;
      exit(1);
   }
   

   // print quality statistics
   if (CL_printMapQuality) {
      mc2dbg << "To print map quality statistics" << endl;
      if (coh.getTailLength() == 0) {
         mc2log << error << "Must give mcm maps in tail!" << endl;
      } else {
         printMapQuality(coh);
      }
   }

   // compare quality files
   if ((CL_compareMaps1 != NULL) && (CL_compareMaps2 != NULL)) {
      mc2dbg << "Comparing quality files, map1: " << CL_compareMaps1 << endl
             << "                         map2: " << CL_compareMaps2 << endl;
      ifstream file1(CL_compareMaps1);
      ifstream file2(CL_compareMaps2);
      if ( file1 && file2 ) {
         compareMapQuality(file1, file2);
      } else {
         mc2log << error << "Could not open files." << endl;
      }
   }

   exit (0);
}


